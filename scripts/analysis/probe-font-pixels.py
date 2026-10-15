#!/usr/bin/env python3
"""Compare Wine and portable GDI text images at positive and negative font heights."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import shlex
import subprocess
import tempfile

from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rows(path):
    raw = path.read_text()
    result, _ = json.JSONDecoder().raw_decode(raw[raw.index("[\n{"):])
    expected = [(height, text) for height in (14, 18, 22, 26, 28, 30, 32, -14, -28) for text in range(3)]
    if [(row["requestedHeight"], row["text"]) for row in result] != expected:
        raise ValueError("Font pixel probe coverage differs")
    if any(row["pixels"] <= 0 for row in result):
        raise ValueError("Font pixel probe has empty glyph coverage")
    return result


def compact_images(folder):
    for path in folder.glob("*.ppm"):
        original = Image.open(path).convert("RGB")
        png = path.with_suffix(".png")
        original.save(png)
        if Image.open(png).convert("RGB").tobytes() != original.tobytes():
            raise RuntimeError("Compressed font image differs")
        path.unlink()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--font", type=Path, required=True, help="Portable font file; match the reported Wine face")
    parser.add_argument("--baseline-ref", help="Also render the Linux GDI adapter at this immutable Git commit")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    font = args.font.resolve(strict=True)
    for command in ("g++", "i686-w64-mingw32-g++", "pkg-config", "wine", "wineserver", "xvfb-run", "timeout"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    output.mkdir(parents=True)
    source = ROOT / "tests/font-pixel-probe.cpp"
    native = output / "native"
    native.mkdir()
    binary = output / "font-pixel.exe"
    with (output / "native-build.log").open("w") as log:
        subprocess.run(["i686-w64-mingw32-g++", "-O2", "-static", str(source), "-o", str(binary),
                        "-lgdi32", "-luser32"], stdout=log, stderr=subprocess.STDOUT, check=True)
    prefix = Path(tempfile.mkdtemp(prefix="font-pixels-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned font pixel probe prefix\n")
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy", TH08_FONT=str(font),
                       WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
                       WINEDLLOVERRIDES="winepulse.drv,winealsa.drv=d")
    try:
        with (native / "stdout.txt").open("w") as stdout, (native / "wine.log").open("w") as log:
            subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "60s", "wine", str(binary),
                            "Z:" + native.as_posix()], env=environment, stdout=stdout, stderr=log, check=True)
        reference = rows(native / "stdout.txt")
        compact_images(native)
    finally:
        subprocess.run(["wineserver", "-k"], env=environment, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=environment, timeout=15, check=False)
        shutil.rmtree(prefix)
        binary.unlink()
    (native / "samples.json").write_text(json.dumps(reference, indent=2) + "\n")
    sources = {"production": ROOT / "src/modern/linux/linux_compat.cpp"}
    baseline = None
    if args.baseline_ref:
        baseline = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                            args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
        path = output / "baseline-linux-compat.cpp"
        path.write_bytes(subprocess.check_output(["git", "show", baseline + ":src/modern/linux/linux_compat.cpp"], cwd=ROOT))
        sources["baseline"] = path
    flags = shlex.split(subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl2", "SDL2_image",
                                                "SDL2_ttf", "fontconfig", "gl"], text=True))
    report = dict(evidenceClass="Wine GDI and portable adapter pixel probes; no TH08 target execution",
                  fontPath=str(font), fontSha256=sha(font), baselineCommit=baseline,
                  probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                  locale={name: os.environ.get(name) for name in ("LANG", "LC_ALL", "LC_CTYPE")},
                  adapterSha256={name: sha(path) for name, path in sources.items()}, variants={})
    report.update(compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
                  nativeCompiler=subprocess.check_output(["i686-w64-mingw32-g++", "--version"], text=True).splitlines()[0],
                  wine=subprocess.check_output(["wine", "--version"], text=True).strip(),
                  converterSha256={name: sha(ROOT / ("src/modern/linux/" + name + "_compat.cpp"))
                                   for name in ("d3d8", "d3dx8")})
    for name, adapter in sources.items():
        folder = output / name
        folder.mkdir()
        binary = output / ("font-pixel-" + name)
        command = ["g++", "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
                   "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                   "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
                   "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux",
                   str(source), str(adapter), "src/modern/linux/d3d8_compat.cpp", "src/modern/linux/d3dx8_compat.cpp",
                   "-o", str(binary), *flags, "-ldl"]
        with (folder / "build.log").open("w") as log:
            subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        try:
            with (folder / "stdout.txt").open("w") as stdout:
                subprocess.run(["timeout", "30s", str(binary), str(folder)], env=environment,
                               stdout=stdout, stderr=subprocess.STDOUT, check=True)
            observed = rows(folder / "stdout.txt")
            compact_images(folder)
        finally:
            binary.unlink()
        (folder / "samples.json").write_text(json.dumps(observed, indent=2) + "\n")
        comparison = []
        for a, b in zip(reference, observed):
            filename = f"text-{a['requestedHeight']}-{a['text']}.png"
            left, right = (Image.open(root / filename).convert("RGB") for root in (native, folder))
            comparison.append(dict(requestedHeight=a["requestedHeight"], text=a["text"],
                                   referenceBounds=a["bounds"], candidateBounds=b["bounds"],
                                   meanRgbError=sum(ImageStat.Stat(ImageChops.difference(left, right)).mean) / 3,
                                   referenceImageSha256=sha(native / filename), candidateImageSha256=sha(folder / filename)))
        report["variants"][name] = comparison
        selected = next(row for row in comparison if row["requestedHeight"] == 28 and row["text"] == 1)
        print(f"{name}: 27 text images; height 28 title bounds {selected['candidateBounds']}, "
              f"Wine {selected['referenceBounds']}; mean RGB error {selected['meanRgbError']:.6f}")
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
