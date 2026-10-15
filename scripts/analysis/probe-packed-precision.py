#!/usr/bin/env python3
"""Compare exhaustive A4R4G4B4 sampling, modulation and blending with Wine D3D8."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SAMPLES = 24 * 65536


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def compare(reference, candidate):
    if any(path.stat().st_size != SAMPLES * 4 for path in (reference, candidate)):
        raise ValueError("Packed precision sample coverage differs")
    cases = []
    with reference.open("rb") as left, candidate.open("rb") as right:
        for material in range(4):
            for blend in range(3):
                for draws in (1, 4):
                    a, b = left.read(65536 * 4), right.read(65536 * 4)
                    count = maximum = 0
                    examples = []
                    for texel, ((expected,), (actual,)) in enumerate(zip(struct.iter_unpack("<I", a), struct.iter_unpack("<I", b))):
                        if expected > 0xffffff or actual > 0xffffff:
                            raise ValueError("Packed precision output contains a non-RGB word")
                        if expected == actual:
                            continue
                        error = max(abs(((expected >> shift) & 255) - ((actual >> shift) & 255)) for shift in (0, 8, 16))
                        count += 1
                        maximum = max(maximum, error)
                        if len(examples) < 16:
                            examples.append(dict(sampleIndex=f"{texel:04x}", referenceRgb=f"{expected:06x}",
                                                 candidateRgb=f"{actual:06x}", maximumChannelError=error))
                    cases.append(dict(material=material, blend=blend, draws=draws,
                                      differentPixels=count, maximumChannelError=maximum, examples=examples))
    return dict(differentPixels=sum(row["differentPixels"] for row in cases),
                maximumChannelError=max(row["maximumChannelError"] for row in cases), cases=cases)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--filter", choices=("point", "linear"), default="point")
    parser.add_argument("--renderer-control", type=Path, help="Also compare a private renderer source")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--pkg-config", default="pkg-config")
    parser.add_argument("--legacy-gl", action="store_true")
    parser.add_argument("--check", action="store_true", help="Require every production RGB pixel to match")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    if args.renderer_control and not args.renderer_control.is_file():
        parser.error("Renderer control source is missing")
    for command in (args.compiler, args.pkg_config, "i686-w64-mingw32-g++", "wine", "wineserver", "xvfb-run", "timeout"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    output.mkdir(parents=True)
    source = ROOT / "tests/packed-texture-precision.cpp"
    compiler = str(Path(shutil.which(args.compiler)).resolve())
    package = str(Path(shutil.which(args.pkg_config)).resolve())
    flags = shlex.split(subprocess.check_output([package, "--cflags", "--libs", "sdl2", "SDL2_image",
                                               "SDL2_ttf", "fontconfig", "gl"], text=True))
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                       GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    environment.pop("LP_NO_RAST", None)
    environment.pop("TH08_RENDERER_REQUIRE_LEGACY_GL", None)
    prefix = Path(tempfile.mkdtemp(prefix="packed-precision-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned packed-texture precision probe prefix\n")
    native_env = dict(environment, WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
                      WINEDLLOVERRIDES="winepulse.drv,winealsa.drv=d")
    sources = dict(production=ROOT / "src/modern/linux/d3d8_compat.cpp")
    if args.renderer_control:
        control = output / "control.cpp"
        control.write_bytes(args.renderer_control.read_bytes())
        sources["control"] = control
    report = dict(evidenceClass=__doc__, samples=SAMPLES, filter=args.filter, legacyOpenGL=args.legacy_gl,
                  probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                  rendererSha256={name: sha(path) for name, path in sources.items()},
                  compiler=subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
                  wineVersion=subprocess.check_output(["wine", "--version"], text=True).strip(),
                  materials=["ffffffff", "28ffffff", "80c0f080", "ffc0f080"],
                  textureFormat="A4R4G4B4", textureSize=[256, 256],
                  quadOrigin=-0.5 if args.filter == "point" else -0.25,
                  outputPacking="little-endian 00RRGGBB",
                  alphaTest=dict(enabled=True, reference=4, comparison="greater-or-equal"),
                  compileCommands={}, binarySha256={}, sampleSha256={}, variants={})
    binaries, samples = [], []
    try:
        native = output / "probe.exe"
        binaries.append(native)
        command = ["i686-w64-mingw32-g++", "-std=gnu++98", "-O2", "-static", str(source),
                   "-o", str(native), "-ld3d8", "-lgdi32", "-luser32"]
        report["compileCommands"]["native"] = command
        with (output / "native-build.log").open("w") as log:
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
        reference = output / "native.bin"
        samples.append(reference)
        report["binarySha256"]["native"] = sha(native)
        with (output / "native.stdout").open("w") as log:
            subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "90s", "wine", str(native),
                            "Z:" + str(reference), args.filter], env=native_env, stdout=log, stderr=subprocess.STDOUT, check=True)
        report["sampleSha256"]["native"] = sha(reference)
        for name, renderer in sources.items():
            binary = output / ("probe-" + name)
            binaries.append(binary)
            command = [compiler, "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
                       "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                       "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
                       "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux", str(source),
                       str(renderer), "src/modern/linux/linux_compat.cpp", "-o", str(binary), *flags, "-ldl"]
            report["compileCommands"][name] = command
            with (output / (name + "-build.log")).open("w") as log:
                subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
            report["binarySha256"][name] = sha(binary)
            local_env = dict(environment)
            if args.legacy_gl:
                local_env.update(MESA_GL_VERSION_OVERRIDE="2.1", MESA_EXTENSION_OVERRIDE="-GL_ARB_clip_control",
                                 TH08_RENDERER_REQUIRE_LEGACY_GL="1")
            sample = output / (name + ".bin")
            samples.append(sample)
            with (output / (name + ".stdout")).open("w") as log:
                subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "90s", str(binary),
                                str(sample), args.filter], env=local_env, stdout=log, stderr=subprocess.STDOUT, check=True)
            report["sampleSha256"][name] = sha(sample)
            report["variants"][name] = compare(reference, sample)
            print(f"{name}: {report['variants'][name]['differentPixels']}/{SAMPLES} different pixels; "
                  f"maximum channel error {report['variants'][name]['maximumChannelError']}/255", flush=True)
        (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
        if args.check and report["variants"]["production"]["differentPixels"]:
            raise SystemExit("Production packed-texture pixels differ from Wine D3D8")
    finally:
        subprocess.run(["wineserver", "-k"], env=native_env, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=native_env, timeout=15, check=False)
        shutil.rmtree(prefix)
        for path in binaries + samples:
            path.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
