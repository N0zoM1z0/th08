#!/usr/bin/env python3
"""Replay observed quad inputs through Wine D3D8 and portable synthetic materials."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import shutil
import struct
import subprocess
import tempfile

from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compact(folder):
    for path in folder.glob("*.ppm"):
        original = Image.open(path).convert("RGB")
        png = path.with_suffix(".png")
        original.save(png)
        if Image.open(png).convert("RGB").tobytes() != original.tobytes():
            raise RuntimeError("Compressed subpixel image differs")
        path.unlink()


def snap_positions(data, grid):
    """Build a synthetic XY control while preserving every other vertex word."""
    if grid <= 0:
        raise ValueError("Position grid must be positive")
    result = bytearray(data)
    changed = 0
    maximum = 0.0
    for vertex in range(4, len(data), 28):
        for offset in (vertex, vertex + 4):
            before = struct.unpack_from("<f", data, offset)[0]
            struct.pack_into("<f", result, offset, round(before * grid) / grid)
            after = struct.unpack_from("<f", result, offset)[0]
            changed += result[offset:offset + 4] != data[offset:offset + 4]
            maximum = max(maximum, abs(after - before))
    return bytes(result), dict(kind="synthetic XY grid control", stepsPerPixel=grid,
                               changedWords=changed, maximumPositionDelta=maximum)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", type=Path, required=True, help="Completed laser capture with --watch-quads")
    parser.add_argument("--frame", type=int, default=12900)
    parser.add_argument("--position-grid", type=int,
                        help="Synthetic control: snap X/Y to this many grid steps per pixel in both APIs")
    parser.add_argument("--baseline-ref", help="Also exercise an immutable pre-repair renderer")
    parser.add_argument("--check-coverage", action="store_true", help="Require exact pixels in every solid-material case")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    args = parser.parse_args()
    if args.position_grid is not None and args.position_grid <= 0:
        parser.error("Position grid must be positive")
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    for command in ("g++", "i686-w64-mingw32-g++", "pkg-config", "xvfb-run", "timeout", "wine", "wineserver"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    spec = importlib.util.spec_from_file_location("quads", ROOT / "scripts/analysis/compare-sprite-quads.py")
    quads = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(quads)
    frames = json.loads((args.capture / "metadata.json").read_text())["frames"]
    rows = quads.load_quads(args.capture, frames)
    sample = next((row for row in rows if row["frame"] == args.frame), None)
    if sample is None:
        parser.error("Requested frame is absent from the capture")
    selected = quads.select_region(sample["quads"], (201, 236, 248, 245))
    if not selected:
        parser.error("No horizontal sprite quads cross the selected region")
    output.mkdir(parents=True)
    data = struct.pack("<I", len(selected)) + b"".join(struct.pack("<7I", *vertex)
                                                     for _, quad, _ in selected for vertex in quad)
    observed_input_sha = hashlib.sha256(data).hexdigest()
    transformation = dict(kind="unmodified observed vertex words")
    if args.position_grid is not None:
        data, transformation = snap_positions(data, args.position_grid)
    (output / "input.bin").write_bytes(data)
    source = ROOT / "tests/subpixel-quad-probe.cpp"
    native = output / "native"
    native.mkdir()
    binary = output / "subpixel.exe"
    with (output / "native-build.log").open("w") as log:
        subprocess.run(["i686-w64-mingw32-g++", "-O2", "-static", str(source), "-o", str(binary),
                        "-ld3d8", "-lgdi32", "-luser32"], stdout=log, stderr=subprocess.STDOUT, check=True)
    prefix = Path(tempfile.mkdtemp(prefix="subpixel-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned subpixel quad probe prefix\n")
    environment = dict(os.environ, WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
                       WINEDLLOVERRIDES="winepulse.drv,winealsa.drv=d", SDL_AUDIODRIVER="dummy",
                       LIBGL_ALWAYS_SOFTWARE="1", GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    try:
        with (native / "capture.log").open("w") as log:
            subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "90s", "wine", str(binary),
                            "Z:" + str(output / "input.bin"), "Z:" + str(native)], env=environment,
                           stdout=log, stderr=subprocess.STDOUT, check=True)
        compact(native)
    finally:
        subprocess.run(["wineserver", "-k"], env=environment, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=environment, timeout=15, check=False)
        shutil.rmtree(prefix)
        binary.unlink()
    sources = {"production": ROOT / "src/modern/linux/d3d8_compat.cpp"}
    baseline = None
    if args.baseline_ref:
        baseline = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                            args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
        path = output / "baseline-d3d8.cpp"
        path.write_bytes(subprocess.check_output(["git", "show", baseline + ":src/modern/linux/d3d8_compat.cpp"], cwd=ROOT))
        sources["baseline"] = path
    flags = shlex.split(subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl2", "SDL2_image",
                                                "SDL2_ttf", "fontconfig", "gl"], text=True))
    report = dict(evidenceClass="Wine D3D8 API and portable synthetic materials with identified input transformations",
                  quadCaptureSha256=sha(args.capture / "quads.json"), inputSha256=sha(output / "input.bin"),
                  observedInputSha256=observed_input_sha, inputTransformation=transformation,
                  frame=args.frame, selectedIndices=[index for index, _, _ in selected], baselineCommit=baseline,
                  probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                  rendererSha256={name: sha(path) for name, path in sources.items()}, variants={})
    report["toolVersions"] = {name: subprocess.check_output(command, text=True).splitlines()[0]
                              for name, command in (("portableCompiler", ["g++", "--version"]),
                                                    ("nativeCompiler", ["i686-w64-mingw32-g++", "--version"]),
                                                    ("wine", ["wine", "--version"]))}
    for name, renderer in sources.items():
        folder = output / name
        folder.mkdir()
        binary = output / ("subpixel-" + name)
        command = ["g++", "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
                   "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                   "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
                   "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux", str(source),
                   str(renderer), "src/modern/linux/d3dx8_compat.cpp", "src/modern/linux/linux_compat.cpp",
                   "-o", str(binary), *flags, "-ldl"]
        with (folder / "build.log").open("w") as log:
            subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        try:
            with (folder / "capture.log").open("w") as log:
                subprocess.run(["xvfb-run", "-a", "timeout", "60s", str(binary), str(output / "input.bin"),
                                str(folder)], env=environment, stdout=log, stderr=subprocess.STDOUT, check=True)
            compact(folder)
        finally:
            binary.unlink()
        expected = {f"v{vp}-m{material}-q{index}.png" for vp in range(2) for material in range(2)
                    for index in range(len(selected) + 1)}
        if {path.name for path in native.glob("*.png")} != expected or {path.name for path in folder.glob("*.png")} != expected:
            raise ValueError("Subpixel image coverage differs")
        differences = []
        for filename in sorted(expected):
            a, b = (Image.open(path / filename).convert("RGB") for path in (native, folder))
            delta = ImageChops.difference(a, b)
            differences.append(dict(image=filename, bounds=delta.getbbox(),
                                    meanRgbError=sum(ImageStat.Stat(delta).mean) / 3,
                                    referenceSha256=sha(native / filename), candidateSha256=sha(folder / filename)))
        report["variants"][name] = differences
        print(f"{name}: {sum(row['bounds'] is None for row in differences)}/{len(expected)} pixel-identical cases")
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    if args.check_coverage and any(row["bounds"] is not None for row in report["variants"]["production"] if "-m0-" in row["image"]):
        raise SystemExit("Solid-material subpixel coverage differs from Wine D3D8")


if __name__ == "__main__":
    main()
