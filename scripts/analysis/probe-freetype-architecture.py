#!/usr/bin/env python3
"""Compare system i386 and x86_64 FreeType outlines and raw glyph coverage."""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--font', type=Path, required=True)
    parser.add_argument('--codepoint', type=lambda value: int(value, 0), default=0x90F7)
    parser.add_argument('--ppem', type=int, default=21)
    parser.add_argument('--strength', type=int, help='Override outline strength in 26.6 units; zero tests regular glyphs')
    parser.add_argument('--output-dir', type=Path, required=True, help='Fresh directory under build/')
    args = parser.parse_args()
    output, font = args.output_dir.resolve(), args.font.resolve(strict=True)
    if output.exists() or not output.is_relative_to(ROOT / 'build'):
        parser.error('Output must be a fresh directory under build/')
    if not (0 < args.ppem <= 96 and 0 <= args.codepoint <= 0x10FFFF):
        parser.error('Ppem or Unicode codepoint exceeds the probe limit')
    for command in ('gdb', 'nm'):
        if not shutil.which(command):
            parser.error('Required command is missing: ' + command)
    libc = Path('/usr/lib/i386-linux-gnu/libc.so.6').resolve(strict=True)
    library = Path('/usr/lib/i386-linux-gnu/libfreetype.so.6').resolve(strict=True)
    for path in (libc, library):
        data = path.read_bytes()
        if data[:6] != b'\x7fELF\x01\x01' or struct.unpack_from('<H', data, 18)[0] != 3:
            parser.error('Library must be little-endian ELF32 i386: ' + str(path))
    import freetype

    face = freetype.Face(str(font))
    face.set_pixel_sizes(0, args.ppem)
    face.load_char(chr(args.codepoint), freetype.FT_LOAD_NO_BITMAP | freetype.FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)
    slot = face.glyph

    def packet():
        return dict(points=list(map(list, slot.outline.points)), tags=list(slot.outline.tags), contours=list(slot.outline.contours))

    before = packet()
    strength = args.strength if args.strength is not None else (args.ppem * 64 + 12) // 24
    if not 0 <= strength <= 256:
        parser.error('Outline strength exceeds the probe limit')
    if freetype.FT_Outline_Embolden(ctypes.byref(slot._FT_GlyphSlot.contents.outline), strength):
        parser.error('Native FreeType outline emboldening failed')
    after = packet()
    slot.render(freetype.FT_RENDER_MODE_NORMAL)
    bitmap = slot.bitmap
    raw = bytes(bitmap.buffer)
    maps = Path('/proc/self/maps').read_text().splitlines()
    libraries = sorted({line.split()[-1] for line in maps if '/libfreetype' in line})
    if len(libraries) != 1:
        parser.error('Expected one loaded native FreeType library')
    native_library = Path(libraries[0])
    native_image = native_library.read_bytes()
    if native_image[:6] != b'\x7fELF\x02\x01' or struct.unpack_from('<H', native_image, 18)[0] != 62:
        parser.error('Native control requires little-endian ELF64 FreeType')
    native = dict(architecture='x86_64', freetypeVersion=freetype.version(), glyphIndex=face.get_char_index(chr(args.codepoint)),
                  width=bitmap.width, rows=bitmap.rows, pitch=bitmap.pitch,
                  bitmapLeft=slot.bitmap_left, bitmapTop=slot.bitmap_top,
                  outlineBefore=before, outlineAfter=after, bitmap=list(raw))
    output.mkdir()
    observer = Path(__file__).with_name('freetype-i386-api.gdb')
    params = dict(libc=str(libc), freetype=str(library), font=str(font), codepoint=args.codepoint,
                  ppem=args.ppem, strength=strength, result=str(output / 'i386.json'))
    params_path = output / 'parameters.json'
    params_path.write_text(json.dumps(params, indent=2) + '\n')
    environment = dict(os.environ, TH08_FONT_API_PARAMETERS=str(params_path))
    with (output / 'gdb.log').open('w') as log:
        subprocess.run(['gdb', '-nx', '-q', '-batch', str(libc), '-x', str(observer)],
                       cwd=ROOT, env=environment, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=30)
    other = json.loads((output / 'i386.json').read_text())
    (output / 'x86_64.json').write_text(json.dumps(native, indent=2) + '\n')
    fields = ('glyphIndex', 'width', 'rows', 'pitch', 'bitmapLeft', 'bitmapTop')
    if any(native[key] != other[key] for key in fields):
        raise ValueError('Glyph or bitmap layout differs; raw coverage comparison requires equal layouts')
    if len(other['bitmap']) != len(raw):
        raise ValueError('Bitmap byte counts differ')
    changes = [dict(x=i % bitmap.pitch, y=i // bitmap.pitch, i386=a, x86_64=b,
                    i386Level=a * 17 // 256, x86_64Level=b * 17 // 256)
               for i, (a, b) in enumerate(zip(other['bitmap'], raw)) if a != b]
    report = dict(evidenceClass='System FreeType API controls; no Wine or TH08 execution',
                  fontSha256=digest(font), codepoint=args.codepoint, ppem=args.ppem, strength26dot6=strength,
                  analyzerSha256=digest(Path(__file__)), observerSha256=digest(observer),
                  librarySha256=dict(libcI386=digest(libc), freetypeI386=digest(library), freetypeX86_64=digest(native_library)),
                  libraryPaths=dict(libcI386=str(libc), freetypeI386=str(library), freetypeX86_64=str(native_library)),
                  freetypeProperties=os.environ.get('FREETYPE_PROPERTIES'),
                  outlineBeforeEqual=before == other['outlineBefore'], outlineAfterEqual=after == other['outlineAfter'],
                  bitmapDifferences=changes, quantizedDifferences=sum(p['i386Level'] != p['x86_64Level'] for p in changes))
    (output / 'comparison.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Pre-bold outlines equal:', report['outlineBeforeEqual'])
    print('Post-bold outlines equal:', report['outlineAfterEqual'])
    print(f"Raw coverage differences: {len(changes)}; quantized differences: {report['quantizedDifferences']}")
    for change in changes:
        print(f"Pixel ({change['x']}, {change['y']}): i386 {change['i386']}, x86_64 {change['x86_64']}")


if __name__ == '__main__':
    main()
