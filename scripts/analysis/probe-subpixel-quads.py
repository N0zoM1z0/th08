#!/usr/bin/env python3
"""Replay observed quad inputs through Wine D3D8 and portable synthetic materials."""
import argparse
import gzip
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import resource
import shlex
import shutil
import struct
import subprocess
import tempfile

from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def limit_reference_files():
    resource.setrlimit(resource.RLIMIT_FSIZE, (32 * 1024 * 1024, 32 * 1024 * 1024))


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


def constant_u(data, value):
    """Remove the U gradient while preserving positions, color, RHW and V."""
    if not math.isfinite(value):
        raise ValueError("Constant U must be finite")
    try:
        word = struct.pack("<f", value)
    except OverflowError as error:
        raise ValueError("Constant U must fit a finite float") from error
    actual = struct.unpack("<f", word)[0]
    if not math.isfinite(actual):
        raise ValueError("Constant U must fit a finite float")
    result = bytearray(data)
    changed = 0
    for vertex in range(4, len(data), 28):
        offset = vertex + 20
        changed += data[offset:offset + 4] != word
        result[offset:offset + 4] = word
    return bytes(result), dict(kind="synthetic constant U control", value=actual,
                               floatBits=struct.unpack("<I", word)[0], changedWords=changed)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", type=Path, required=True, help="Completed laser capture with --watch-quads")
    parser.add_argument("--frame", type=int, default=12900)
    parser.add_argument("--compiler", default="g++", help="Portable compiler command or wrapper")
    parser.add_argument("--pkg-config", default="pkg-config",
                        help="Package metadata command for the portable compiler's target architecture")
    control = parser.add_mutually_exclusive_group()
    control.add_argument("--position-grid", type=int,
                         help="Synthetic control: snap X/Y to this many grid steps per pixel in both APIs")
    control.add_argument("--constant-u", type=float,
                         help="Synthetic control: set every vertex U to this finite float in both APIs")
    parser.add_argument("--legacy-gl", action="store_true", help="Exercise the portable Mesa OpenGL 2.1 path without clip control")
    parser.add_argument("--baseline-ref", help="Also exercise an immutable pre-repair renderer")
    parser.add_argument("--renderer-control", type=Path,
                        help="Also exercise a private renderer source copy, labeled as a diagnostic control")
    parser.add_argument("--trace-wine-shaders", action="store_true",
                        help="Retain Wine shader compilation logs, with a 32 MiB file limit")
    parser.add_argument("--trace-wine-projection", action="store_true",
                        help="Observe projection matrices with GDB; requires the pinned Wine 9.0 i386 DLL")
    parser.add_argument("--check-coverage", action="store_true", help="Require exact pixels in every solid-material case")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    args = parser.parse_args()
    if args.position_grid is not None and args.position_grid <= 0:
        parser.error("Position grid must be positive")
    if args.constant_u is not None and not math.isfinite(args.constant_u):
        parser.error("Constant U must be finite")
    if args.constant_u is not None:
        try:
            struct.pack("<f", args.constant_u)
        except OverflowError:
            parser.error("Constant U must fit a finite float")
    if args.renderer_control:
        try:
            args.renderer_control = args.renderer_control.resolve(strict=True)
        except OSError:
            parser.error("Renderer control source is missing")
        if not args.renderer_control.is_file():
            parser.error("Renderer control source must be a file")
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    for command in (args.compiler, "i686-w64-mingw32-g++", args.pkg_config, "xvfb-run", "timeout", "wine", "wineserver"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    if args.trace_wine_projection and not shutil.which("gdb"):
        parser.error("Projection observation requires GDB")
    compiler = str(Path(shutil.which(args.compiler)).resolve())
    pkg_config = str(Path(shutil.which(args.pkg_config)).resolve())
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
    elif args.constant_u is not None:
        data, transformation = constant_u(data, args.constant_u)
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
    reference_environment = dict(environment)
    observer = ROOT / "scripts/analysis/observe-subpixel-projection.py"
    if args.trace_wine_shaders:
        reference_environment["WINEDEBUG"] = "-all,+d3d_shader"
    reference_command = ["wine", str(binary), "Z:" + str(output / "input.bin"), "Z:" + str(native)]
    if args.trace_wine_projection:
        reference_environment.update(TH08_SUBPIXEL_OBSERVE_PROJECTION="1",
                                     TH08_SUBPIXEL_OBSERVER_OUTPUT=str(native))
        reference_command = ["gdb", "--quiet", "--batch", "-ex", "source " + str(observer), "--args",
                             "/usr/lib/wine/wine", *reference_command[1:]]
    try:
        with (native / "capture.log").open("w") as log:
            subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "90s", *reference_command], env=reference_environment,
                           stdout=log, stderr=subprocess.STDOUT, check=True, preexec_fn=limit_reference_files)
        compact(native)
    finally:
        subprocess.run(["wineserver", "-k"], env=environment, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=environment, timeout=15, check=False)
        shutil.rmtree(prefix)
        binary.unlink()
    shader_trace = None
    projection_trace = None
    if args.trace_wine_projection:
        path = native / "projection.json"
        matrices = json.loads(path.read_text())
        if len(matrices["records"]) != 4 * (len(selected) + 1):
            raise ValueError("Reference projection observation count differs")
        projection_trace = dict(file="native/projection.json", sha256=sha(path),
                                observerSha256=sha(observer), calls=len(matrices["records"]),
                                dllSha256=matrices["dllSha256"])
    if args.trace_wine_shaders:
        path = native / "capture.log"
        log_bytes = path.read_bytes()
        if b"ffp_projection_matrix" not in log_bytes:
            raise ValueError("Wine shader trace does not contain a fixed-function projection shader")
        shader_trace = dict(debug=reference_environment["WINEDEBUG"], bytes=len(log_bytes),
                            sha256=sha(path), file="native/capture.log.gz")
        with gzip.open(path.with_suffix(".log.gz"), "wb") as stream:
            stream.write(log_bytes)
        path.unlink()
    sources = {"production": ROOT / "src/modern/linux/d3d8_compat.cpp"}
    if args.renderer_control:
        control_source = output / "control-d3d8.cpp"
        control_source.write_bytes(args.renderer_control.resolve(strict=True).read_bytes())
        sources["control"] = control_source
    baseline = None
    if args.baseline_ref:
        baseline = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                            args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
        path = output / "baseline-d3d8.cpp"
        path.write_bytes(subprocess.check_output(["git", "show", baseline + ":src/modern/linux/d3d8_compat.cpp"], cwd=ROOT))
        sources["baseline"] = path
    package_command = [pkg_config, "--cflags", "--libs", "sdl2", "SDL2_image", "SDL2_ttf", "fontconfig", "gl"]
    flags = shlex.split(subprocess.check_output(package_command, text=True))
    report = dict(evidenceClass="Wine D3D8 API and portable synthetic materials with identified input transformations",
                  quadCaptureSha256=sha(args.capture / "quads.json"), inputSha256=sha(output / "input.bin"),
                  observedInputSha256=observed_input_sha, inputTransformation=transformation,
                  referenceShaderTrace=shader_trace,
                  referenceProjectionTrace=projection_trace,
                  legacyOpenGL=args.legacy_gl,
                  frame=args.frame, selectedIndices=[index for index, _, _ in selected], baselineCommit=baseline,
                  probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                  rendererSha256={name: sha(path) for name, path in sources.items()}, variants={})
    report.update(portableCompilerCommand=compiler, packageMetadataCommand=package_command,
                  toolCommandSha256=dict(portableCompiler=sha(Path(compiler)), packageMetadata=sha(Path(pkg_config))),
                  compileCommands={})
    report["toolVersions"] = {name: subprocess.check_output(command, text=True).splitlines()[0]
                              for name, command in (("portableCompiler", [compiler, "--version"]),
                                                    ("nativeCompiler", ["i686-w64-mingw32-g++", "--version"]),
                                                    ("wine", ["wine", "--version"]))}
    for name, renderer in sources.items():
        portable_environment = dict(environment)
        if args.legacy_gl:
            portable_environment.update(MESA_GL_VERSION_OVERRIDE="2.1", MESA_EXTENSION_OVERRIDE="-GL_ARB_clip_control",
                                        TH08_RENDERER_REQUIRE_LEGACY_GL="1")
        folder = output / name
        folder.mkdir()
        binary = output / ("subpixel-" + name)
        command = [compiler, "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
                   "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                   "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
                   "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux", str(source),
                   str(renderer), "src/modern/linux/d3dx8_compat.cpp", "src/modern/linux/linux_compat.cpp",
                   "-o", str(binary), *flags, "-ldl"]
        report["compileCommands"][name] = command
        with (folder / "build.log").open("w") as log:
            subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        try:
            with (folder / "capture.log").open("w") as log:
                subprocess.run(["xvfb-run", "-a", "timeout", "60s", str(binary), str(output / "input.bin"),
                                str(folder)], env=portable_environment, stdout=log, stderr=subprocess.STDOUT, check=True)
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
    if args.check_coverage:
        for name, rows in report["variants"].items():
            if name == "baseline":
                continue
            if any(row["bounds"] is not None for row in rows if "-m0-" in row["image"]):
                raise SystemExit("Solid-material subpixel coverage differs from Wine D3D8: " + name)


if __name__ == "__main__":
    main()
