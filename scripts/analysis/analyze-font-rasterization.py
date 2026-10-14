#!/usr/bin/env python3
"""Isolate SDL_ttf hinting and synthetic bold against retained Wine text images."""
import argparse
import ctypes as C
import ctypes.util
import hashlib
import json
from pathlib import Path
import sys

from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]
HEIGHTS = (14, 18, 22, 26, 28, 30, 32, -14, -28)
TEXTS = (
    "Stage 3",
    bytes.fromhex("8d9191cc81758e4f8eed82cc905f8aed81408bbd8176").decode("cp932"),
    bytes.fromhex("96a2978881758d8293568cb48176").decode("cp932"),
)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


class Color(C.Structure):
    _fields_ = [(name, C.c_ubyte) for name in ("r", "g", "b", "a")]


class Surface(C.Structure):
    # Public SDL_Surface prefix; later members are not accessed.
    _fields_ = [("flags", C.c_uint32), ("format", C.c_void_p),
                ("w", C.c_int), ("h", C.c_int), ("pitch", C.c_int),
                ("pixels", C.c_void_p)]


class Version(C.Structure):
    _fields_ = [(name, C.c_ubyte) for name in ("major", "minor", "patch")]


def bind(library, name, args, result):
    function = getattr(library, name)
    function.argtypes, function.restype = args, result
    return function


class Renderer:
    def __init__(self, font_path):
        from fontTools.ttLib import TTFont

        sdl_path = ctypes.util.find_library("SDL2")
        ttf_path = ctypes.util.find_library("SDL2_ttf")
        if not sdl_path or not ttf_path:
            raise ValueError("SDL2 and SDL2_ttf shared libraries are required")
        sdl, ttf = C.CDLL(sdl_path), C.CDLL(ttf_path)
        self.init = bind(ttf, "TTF_Init", [], C.c_int)
        self.quit = bind(ttf, "TTF_Quit", [], None)
        self.open = bind(ttf, "TTF_OpenFont", [C.c_char_p, C.c_int], C.c_void_p)
        self.close = bind(ttf, "TTF_CloseFont", [C.c_void_p], None)
        self.style = bind(ttf, "TTF_SetFontStyle", [C.c_void_p, C.c_int], None)
        self.hint = bind(ttf, "TTF_SetFontHinting", [C.c_void_p, C.c_int], None)
        self.kerning = bind(ttf, "TTF_SetFontKerning", [C.c_void_p, C.c_int], None)
        self.ascent = bind(ttf, "TTF_FontAscent", [C.c_void_p], C.c_int)
        self.render = bind(ttf, "TTF_RenderUTF8_Blended", [C.c_void_p, C.c_char_p, Color], C.POINTER(Surface))
        self.convert = bind(sdl, "SDL_ConvertSurfaceFormat", [C.POINTER(Surface), C.c_uint32, C.c_uint32], C.POINTER(Surface))
        self.free = bind(sdl, "SDL_FreeSurface", [C.POINTER(Surface)], None)
        linked = bind(ttf, "TTF_Linked_Version", [], C.POINTER(Version))().contents
        self.version = f"{linked.major}.{linked.minor}.{linked.patch}"
        self.font_path = font_path
        with TTFont(font_path) as font:
            self.units = font["head"].unitsPerEm
            self.win_ascent = font["OS/2"].usWinAscent
            self.cell = self.win_ascent + font["OS/2"].usWinDescent
        if self.cell <= 0 or self.init() != 0:
            raise ValueError("Font metrics or SDL_ttf initialization failed")

    @staticmethod
    def muldiv(a, b, c):
        return (a * b + c // 2) // c

    def images(self, height, hinting, kerning, bold):
        size = self.muldiv(self.units, height, self.cell) if height > 0 else -height
        if height > 0 and size > 1 and self.muldiv(self.cell, size, self.units) > height:
            size -= 1
        font = self.open(str(self.font_path).encode(), size)
        if not font:
            raise ValueError("SDL_ttf could not open the selected font")
        try:
            self.style(font, int(bold))
            self.hint(font, hinting)
            self.kerning(font, int(kerning))
            offset = self.muldiv(self.win_ascent, size, self.units) - self.ascent(font)
            for text in TEXTS:
                surface = self.render(font, text.encode(), Color(255, 255, 255, 255))
                if not surface:
                    raise ValueError("SDL_ttf text rendering failed")
                try:
                    # SDL_PIXELFORMAT_RGBA32 is ABGR8888 on little-endian hosts.
                    pixel_format = 0x16762004 if sys.byteorder == "little" else 0x16462004
                    rgba = self.convert(surface, pixel_format, 0)
                finally:
                    self.free(surface)
                if not rgba:
                    raise ValueError("SDL surface conversion failed")
                try:
                    row = rgba.contents
                    if not (0 < row.w <= 4096 and 0 < row.h <= 4096 and row.w * 4 <= row.pitch <= 65536):
                        raise ValueError("SDL surface dimensions exceed the probe limit")
                    data = C.string_at(row.pixels, row.pitch * row.h)
                    coverage = Image.frombytes("RGBA", (row.w, row.h), data, "raw", "RGBA", row.pitch).getchannel("A")
                finally:
                    self.free(rgba)
                canvas = Image.new("L", (768, 96))
                canvas.paste(coverage, (4, 4 + offset))
                yield canvas.convert("RGB")
        finally:
            self.close(font)


class OutlineRenderer:
    """Measure Wine's regular-face synthetic bold and grayscale rules directly."""

    # Wine 9.0 dibdrv/graphics.c: white text on a black DIB.
    GRAY_RAMP = (0, 77, 104, 124, 140, 154, 167, 178, 189,
                 199, 208, 217, 225, 233, 240, 248, 255)

    def __init__(self, font_path, metrics):
        import freetype

        self.ft, self.metrics = freetype, metrics
        self.face = freetype.Face(str(font_path))
        if self.face.style_flags & freetype.FT_STYLE_FLAG_BOLD:
            raise ValueError("The outline control requires a regular font face")
        self.version = ".".join(map(str, freetype.version()))
        self.glyphs = []

    def images(self, height):
        ft, metrics, face = self.ft, self.metrics, self.face
        size = metrics.muldiv(metrics.units, height, metrics.cell) if height > 0 else -height
        if height > 0 and size > 1 and metrics.muldiv(metrics.cell, size, metrics.units) > height:
            size -= 1
        face.set_pixel_sizes(0, size)
        ascent = metrics.muldiv(metrics.win_ascent, size, metrics.units)
        strength = metrics.muldiv(size, 64, 24)
        for text in TEXTS:
            canvas = Image.new("L", (768, 96))
            x, self.glyphs = 4, []
            for character in text:
                index = face.get_char_index(character)
                face.load_glyph(index, ft.FT_LOAD_NO_BITMAP | ft.FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)
                slot = face.glyph
                advance = (slot.advance.x + 64 + 63) // 64
                outline = slot._FT_GlyphSlot.contents.outline
                if ft.FT_Outline_Embolden(C.byref(outline), strength):
                    raise ValueError("FreeType outline emboldening failed")
                slot.render(ft.FT_RENDER_MODE_NORMAL)
                bitmap = slot.bitmap
                if bitmap.width and bitmap.rows:
                    glyph = Image.frombytes("L", (bitmap.width, bitmap.rows), bytes(bitmap.buffer),
                                            "raw", "L", bitmap.pitch)
                    left, top = x + slot.bitmap_left, 4 + ascent - slot.bitmap_top
                    # This probe measures white text on black. Retain strongest
                    # coverage where glyphs overlap; general GDI blending is a
                    # separate adapter test.
                    for row in range(glyph.height):
                        for column in range(glyph.width):
                            px, py = left + column, top + row
                            if 0 <= px < canvas.width and 0 <= py < canvas.height:
                                canvas.putpixel((px, py), max(canvas.getpixel((px, py)), glyph.getpixel((column, row))))
                self.glyphs.append(dict(index=index, advance=advance))
                x += advance
            # GGO_GRAY4 quantization; Wine's DIB painter skips levels 0 and 1.
            ramp = [0 if value * 17 // 256 <= 1 else self.GRAY_RAMP[value * 17 // 256]
                    for value in range(256)]
            self.coverage = canvas
            yield canvas.point(ramp).convert("RGB")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", type=Path, required=True, help="Completed probe-font-pixels.py output")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--outline-control", action="store_true",
                        help="Also measure FreeType outline bold and Wine grayscale; requires freetype-py")
    args = parser.parse_args()
    output, capture = args.output_dir.resolve(), args.capture.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    evidence = json.loads((capture / "report.json").read_text())
    font_path = Path(evidence["fontPath"])
    if digest(font_path) != evidence["fontSha256"] or digest(ROOT / "tests/font-pixel-probe.cpp") != evidence["probeSha256"]:
        parser.error("Font or text probe identity differs from the retained capture")
    records = evidence["variants"]["production"]
    expected = [(height, text) for height in HEIGHTS for text in range(3)]
    if [(r["requestedHeight"], r["text"]) for r in records] != expected:
        parser.error("Capture must contain all 27 production text images")
    for record in records:
        name = f"text-{record['requestedHeight']}-{record['text']}.png"
        for folder, key in (("native", "referenceImageSha256"), ("production", "candidateImageSha256")):
            if digest(capture / folder / name) != record[key]:
                parser.error("Retained image identity differs: " + name)
    renderer = Renderer(font_path)
    output.mkdir()
    report = dict(evidenceClass="SDL_ttf controls against retained Wine GDI probe images; no game execution",
                  capture=str(capture), captureReportSha256=digest(capture / "report.json"),
                  fontSha256=evidence["fontSha256"], analyzerSha256=digest(Path(__file__)),
                  sdlTtfVersion=renderer.version, boundsConvention="Exclusive right and bottom bounds", variants={})
    try:
        variants = [("light", 1, True, True), ("normal", 0, True, True),
                    ("normal-no-kerning", 0, False, True), ("light-no-bold", 1, True, False)]
        outline = None
        if args.outline_control:
            outline = OutlineRenderer(font_path, renderer)
            report["freetypeVersion"] = outline.version
            variants.append(("wine-outline", None, False, True))
        for variant, hinting, kerning, bold in variants:
            entries = []
            folder = output / variant
            folder.mkdir()
            for height in HEIGHTS:
                images = outline.images(height) if variant == "wine-outline" else renderer.images(height, hinting, kerning, bold)
                for text, image in enumerate(images):
                    name = f"text-{height}-{text}.png"
                    with Image.open(capture / "native" / name) as reference, Image.open(capture / "production" / name) as production:
                        reference, production = reference.convert("RGB"), production.convert("RGB")
                        difference = ImageChops.difference(image, reference)
                        error = sum(ImageStat.Stat(difference).mean) / 3
                        entries.append(dict(requestedHeight=height, text=text, bounds=image.getbbox(),
                                            meanRgbError=error, productionPixelEqual=image.tobytes() == production.tobytes(),
                                            referencePixelEqual=image.tobytes() == reference.tobytes(),
                                            supportPixelEqual=image.convert("L").point(lambda v: 255 if v else 0).tobytes()
                                            == reference.convert("L").point(lambda v: 255 if v else 0).tobytes(),
                                            differingPixels=sum(any(p) for p in difference.getdata()),
                                            differenceBounds=difference.getbbox(),
                                            maximumChannelError=max(channel[1] for channel in difference.getextrema()),
                                            nonzeroGrayLevels=len({p[0] for p in image.getdata() if p[0]})))
                        if variant == "wine-outline":
                            entries[-1]["glyphs"] = outline.glyphs
                            samples = []
                            for position, delta in enumerate(difference.getdata()):
                                if not any(delta):
                                    continue
                                xy = position % image.width, position // image.width
                                coverage = outline.coverage.getpixel(xy)
                                samples.append(dict(x=xy[0], y=xy[1], rawCoverage=coverage,
                                                    quantizedLevel=coverage * 17 // 256,
                                                    controlGray=image.getpixel(xy)[0], referenceGray=reference.getpixel(xy)[0]))
                                if len(samples) == 64:
                                    break
                            entries[-1]["coverageSamples"] = samples
                            entries[-1]["coverageSampleLimit"] = 64
                    image.save(folder / name)
            if variant == "light" and not all(e["productionPixelEqual"] for e in entries):
                raise ValueError("Baseline rendering differs; controls cannot be attributed to hinting or bold alone")
            baseline = report["variants"].get("light", {}).get("images", entries)
            summary = dict(meanRgbError=sum(e["meanRgbError"] for e in entries) / len(entries),
                           improved=sum(e["meanRgbError"] < b["meanRgbError"] for e, b in zip(entries, baseline)),
                           worsened=sum(e["meanRgbError"] > b["meanRgbError"] for e, b in zip(entries, baseline)))
            report["variants"][variant] = dict(hinting=hinting, kerning=kerning, bold=bold, summary=summary, images=entries)
            print(f"{variant}: 27 images; mean RGB error {summary['meanRgbError']:.6f}; "
                  f"improved {summary['improved']}, worsened {summary['worsened']}; "
                  f"pixel-identical to Wine {sum(e['referencePixelEqual'] for e in entries)}/27")
    finally:
        renderer.quit()
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
