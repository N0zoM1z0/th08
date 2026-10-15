# Called by probe-freetype-architecture.py. The inferior is system libc,
# used only to call public FreeType APIs; no game or Wine process is started.
set pagination off
set confirm off
set debuginfod enabled off
set startup-with-shell off
set auto-load libthread-db off
set auto-load python-scripts off
starti
python
import gdb, json, os, re, struct, subprocess
from pathlib import Path

params = json.loads(Path(os.environ['TH08_FONT_API_PARAMETERS']).read_text())
entry = int(re.search(r'Entry point: (0x[0-9a-f]+)', gdb.execute('info files', to_string=True)).group(1), 16)
gdb.execute('tbreak *' + hex(entry), to_string=True)
gdb.execute('continue', to_string=True)

def read(address, size):
    return bytes(gdb.selected_inferior().read_memory(address, size))

def u32(address):
    return struct.unpack('<I', read(address, 4))[0]

def symbols(path):
    result = {}
    for line in subprocess.check_output(['nm', '-D', path], text=True).splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1].lower() == 't':
            name = parts[2].split('@')[0]
            if name not in result or '@@' in parts[2]:
                result[name] = int(parts[0], 16)
    return result

libc = Path(params['libc'])
libc_base = entry - struct.unpack_from('<I', libc.read_bytes(), 24)[0]
lc, ft_symbols = symbols(str(libc)), symbols(params['freetype'])

def call(address, result, types, args):
    expression = '((' + result + ' (*)(' + ','.join(types) + '))' + hex(address) + ')(' + ','.join(args) + ')'
    value = gdb.parse_and_eval(expression)
    return None if result == 'void' else int(value)

def ptr(value):
    return '(void*)' + hex(value)

handle = call(libc_base + lc['dlopen'], 'void*', ['const char*', 'int'], [json.dumps(params['freetype']), '2'])
if not handle:
    raise ValueError('System i386 FreeType could not be loaded')
# GNU link_map starts with the relocated base address.
ft_base = u32(handle)
if read(ft_base, 4) != b'\x7fELF':
    raise ValueError('FreeType load base is not an ELF image')
packet = call(libc_base + lc['malloc'], 'void*', ['unsigned int'], ['64'])
if not packet:
    raise ValueError('Inferior parameter allocation failed')
face = library = 0

def api(name, result, types, args):
    return call(ft_base + ft_symbols[name], result, types, args)

def checked(name, types, args):
    result = api(name, 'int', types, args)
    if result:
        raise ValueError(name + ' failed: ' + str(result))

def outline_packet(slot):
    # Public FreeType ILP32 layout: GlyphSlotRec.outline @ 108.
    nc, np = struct.unpack('<hh', read(slot + 108, 4))
    if not (0 < nc < 1024 and 0 < np < 4096):
        raise ValueError('Outline dimensions exceed the probe limit')
    pp, tp, cp = struct.unpack('<III', read(slot + 112, 12))
    points = struct.unpack('<' + 'i' * (np * 2), read(pp, np * 8))
    return dict(points=list(zip(points[::2], points[1::2])), tags=list(read(tp, np)),
                contours=list(struct.unpack('<' + 'h' * nc, read(cp, nc * 2))))

try:
    checked('FT_Init_FreeType', ['void*'], [ptr(packet)])
    library = u32(packet)
    api('FT_Library_Version', 'void', ['void*'] * 4, [ptr(library), ptr(packet + 8), ptr(packet + 12), ptr(packet + 16)])
    version = [u32(packet + i) for i in (8, 12, 16)]
    checked('FT_New_Face', ['void*', 'const char*', 'long', 'void*'], [ptr(library), json.dumps(params['font']), '0', ptr(packet + 4)])
    face = u32(packet + 4)
    checked('FT_Set_Pixel_Sizes', ['void*', 'unsigned int', 'unsigned int'], [ptr(face), '0', str(params['ppem'])])
    index = api('FT_Get_Char_Index', 'unsigned int', ['void*', 'unsigned long'], [ptr(face), str(params['codepoint'])])
    # FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH.
    checked('FT_Load_Glyph', ['void*', 'unsigned int', 'int'], [ptr(face), str(index), str((1 << 3) | (1 << 9))])
    # FaceRec.glyph @ 84; GlyphSlotRec.library @ 0, format @ 72.
    slot = u32(face + 84)
    if u32(slot) != library or u32(slot + 72) != 0x6f75746c:
        raise ValueError('FreeType glyph owner or outline format differs')
    before = outline_packet(slot)
    checked('FT_Outline_Embolden', ['void*', 'long'], [ptr(slot + 108), str(params['strength'])])
    after = outline_packet(slot)
    checked('FT_Render_Glyph', ['void*', 'int'], [ptr(slot), '0'])  # FT_RENDER_MODE_NORMAL
    rows, width, pitch, buffer = struct.unpack('<IIiI', read(slot + 76, 16))
    left, top = struct.unpack('<ii', read(slot + 100, 8))
    if not (0 < width < 128 and 0 < rows < 128 and width <= pitch < 512) or read(slot + 94, 1) != b'\x02':
        raise ValueError('Bitmap dimensions or grayscale mode differ')
    raw = read(buffer, pitch * rows)
    result = dict(architecture='i386', freetypeVersion=version, glyphIndex=index,
                  width=width, rows=rows, pitch=pitch, bitmapLeft=left, bitmapTop=top,
                  outlineBefore=before, outlineAfter=after, bitmap=list(raw))
    Path(params['result']).write_text(json.dumps(result, indent=2) + '\n')
    print('System i386 FreeType API capture completed.')
finally:
    if face:
        api('FT_Done_Face', 'int', ['void*'], [ptr(face)])
    if library:
        api('FT_Done_FreeType', 'int', ['void*'], [ptr(library)])
    call(libc_base + lc['free'], 'void', ['void*'], [ptr(packet)])
end
quit
