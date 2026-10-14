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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", type=Path, required=True, help="Completed probe-font-pixels.py output")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
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
        for variant, hinting, kerning, bold in (("light", 1, True, True), ("normal", 0, True, True),
                                                ("normal-no-kerning", 0, False, True), ("light-no-bold", 1, True, False)):
            entries = []
            folder = output / variant
            folder.mkdir()
            for height in HEIGHTS:
                for text, image in enumerate(renderer.images(height, hinting, kerning, bold)):
                    name = f"text-{height}-{text}.png"
                    with Image.open(capture / "native" / name) as reference, Image.open(capture / "production" / name) as production:
                        reference, production = reference.convert("RGB"), production.convert("RGB")
                        error = sum(ImageStat.Stat(ImageChops.difference(image, reference)).mean) / 3
                        entries.append(dict(requestedHeight=height, text=text, bounds=image.getbbox(),
                                            meanRgbError=error, productionPixelEqual=image.tobytes() == production.tobytes(),
                                            nonzeroGrayLevels=len({p[0] for p in image.getdata() if p[0]})))
                    image.save(folder / name)
            if variant == "light" and not all(e["productionPixelEqual"] for e in entries):
                raise ValueError("Baseline rendering differs; controls cannot be attributed to hinting or bold alone")
            baseline = report["variants"].get("light", {}).get("images", entries)
            summary = dict(meanRgbError=sum(e["meanRgbError"] for e in entries) / len(entries),
                           improved=sum(e["meanRgbError"] < b["meanRgbError"] for e, b in zip(entries, baseline)),
                           worsened=sum(e["meanRgbError"] > b["meanRgbError"] for e, b in zip(entries, baseline)))
            report["variants"][variant] = dict(hinting=hinting, kerning=kerning, bold=bold, summary=summary, images=entries)
            print(f"{variant}: 27 images; mean RGB error {summary['meanRgbError']:.6f}; "
                  f"improved {summary['improved']}, worsened {summary['worsened']}")
    finally:
        renderer.quit()
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
