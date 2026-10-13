#!/usr/bin/env python3
"""Measure TH08's requested GDI font and direct SDL_ttf point-size metrics."""
import argparse
import ctypes
import ctypes.util
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HEIGHTS = [14, 18, 22, 26, 30, 32]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ttf_metrics(font_path):
    library = ctypes.util.find_library("SDL2_ttf")
    if not library:
        raise RuntimeError("SDL2_ttf is unavailable")
    ttf = ctypes.CDLL(library)
    ttf.TTF_Init.restype = ctypes.c_int
    if ttf.TTF_Init() != 0:
        raise RuntimeError("SDL2_ttf initialization failed")
    ttf.TTF_OpenFont.argtypes = [ctypes.c_char_p, ctypes.c_int]
    ttf.TTF_OpenFont.restype = ctypes.c_void_p
    for name in ("TTF_FontHeight", "TTF_FontAscent", "TTF_FontDescent"):
        function = getattr(ttf, name)
        function.argtypes = [ctypes.c_void_p]
        function.restype = ctypes.c_int
    ttf.TTF_FontFaceFamilyName.argtypes = [ctypes.c_void_p]
    ttf.TTF_FontFaceFamilyName.restype = ctypes.c_char_p
    ttf.TTF_SetFontStyle.argtypes = [ctypes.c_void_p, ctypes.c_int]
    ttf.TTF_SetFontHinting.argtypes = [ctypes.c_void_p, ctypes.c_int]
    ttf.TTF_SizeUTF8.argtypes = [ctypes.c_void_p, ctypes.c_char_p,
                              ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
    ttf.TTF_CloseFont.argtypes = [ctypes.c_void_p]
    rows = []
    try:
        for height in HEIGHTS:
            font = ttf.TTF_OpenFont(os.fsencode(font_path), height)
            if not font:
                raise RuntimeError("Cannot open font: " + str(font_path))
            try:
                ttf.TTF_SetFontStyle(font, 1)  # TTF_STYLE_BOLD
                ttf.TTF_SetFontHinting(font, 1)  # TTF_HINTING_LIGHT
                width, text_height = ctypes.c_int(), ctypes.c_int()
                if ttf.TTF_SizeUTF8(font, b"Stage 3", ctypes.byref(width), ctypes.byref(text_height)):
                    raise RuntimeError("SDL_ttf text measurement failed")
                rows.append(dict(requestedHeight=height,
                                 face=ttf.TTF_FontFaceFamilyName(font).decode("utf-8"),
                                 height=ttf.TTF_FontHeight(font), ascent=ttf.TTF_FontAscent(font),
                                 descent=-ttf.TTF_FontDescent(font),
                                 textWidth=width.value, textHeight=text_height.value))
            finally:
                ttf.TTF_CloseFont(font)
    finally:
        ttf.TTF_Quit()
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    for command in ("i686-w64-mingw32-g++", "wine", "wineserver", "xvfb-run", "timeout", "fc-match"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    override = os.environ.get("TH08_FONT")
    font_path = Path(override) if override and os.access(override, os.R_OK) else Path(
        subprocess.check_output(["fc-match", "-f", "%{file}", "VL Gothic:lang=ja:spacing=mono"], text=True))
    font_path = font_path.resolve(strict=True)
    output.mkdir(parents=True)
    source = ROOT / "tests/font-metrics-probe.cpp"
    binary = output / "font-metrics.exe"
    with (output / "build.log").open("w") as log:
        subprocess.run(["i686-w64-mingw32-g++", "-O2", "-static", str(source), "-o", str(binary),
                        "-lgdi32", "-luser32"], stdout=log, stderr=subprocess.STDOUT, check=True)
    prefix = Path(tempfile.mkdtemp(prefix="font-metrics-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned font metrics probe prefix\n")
    environment = dict(os.environ, WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
                       WINEDLLOVERRIDES="winepulse.drv,winealsa.drv=d")
    try:
        with (output / "wine.log").open("w") as log:
            raw = subprocess.check_output(["xvfb-run", "-a", "timeout", "--kill-after=5s", "60s",
                                           "wine", str(binary)], env=environment, stderr=log, text=True)
        (output / "wine.stdout").write_text(raw)
        rows, _ = json.JSONDecoder().raw_decode(raw[raw.index("[\n{"):])
        if [row["requestedHeight"] for row in rows] != HEIGHTS:
            raise RuntimeError("Wine font measurement coverage differs")
        (output / "wine.json").write_text(json.dumps(rows, indent=2) + "\n")
    finally:
        subprocess.run(["wineserver", "-k"], env=environment, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=environment, timeout=15, check=False)
        shutil.rmtree(prefix)
        binary.unlink()
    portable = ttf_metrics(font_path)
    (output / "portable.json").write_text(json.dumps(portable, indent=2) + "\n")
    provenance = dict(evidenceClass="Wine GDI and direct SDL_ttf metric probes; not a TH08 game capture",
                      probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                      fontPath=str(font_path), fontSha256=sha(font_path), fontOverride=override,
                      locale={name: os.environ.get(name) for name in ("LANG", "LC_ALL", "LC_CTYPE")},
                      nativeParameters="TH08 CreateFontA face, semibold, Shift-JIS, antialias, fixed pitch",
                      portableParameters="Direct TTF point size, synthetic bold and light hinting; no GDI height mapping",
                      compiler=subprocess.check_output(["i686-w64-mingw32-g++", "--version"], text=True).splitlines()[0],
                      wine=subprocess.check_output(["wine", "--version"], text=True).strip(),
                      temporaryPrefixRemoved=True)
    (output / "provenance.json").write_text(json.dumps(provenance, indent=2) + "\n")
    for native, candidate in zip(rows, portable):
        print(f"Height {native['requestedHeight']}: Wine {native['face']} "
              f"width {native['textWidth']}, direct SDL_ttf {candidate['face']} width {candidate['textWidth']}")


if __name__ == "__main__":
    main()
