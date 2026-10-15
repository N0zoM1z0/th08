#!/usr/bin/env python3
"""Compare packed-texture filtering and blending through Wine D3D8 and Linux."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "tests/texture-filter-probe.cpp"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(path, alpha_states=False):
    text = path.read_text()
    start = text.find("[\n")
    if start < 0:
        raise ValueError("Texture filter probe JSON is missing")
    rows, _ = json.JSONDecoder().raw_decode(text[start:])
    expected = [(f, t, b, p, d) for f in range(4) for t in (1, 2) for b in range(3)
                for p in range(4) for d in (1, 4)]
    keys = ("format", "filter", "blend", "phase", "draws")
    if alpha_states:
        expected = [(m, c, r, f) for m in range(6) for c in range(1, 9)
                    for r in (0, 4, 128, 255) for f in range(2)]
        keys = ("material", "comparison", "reference", "fog")
    actual = [tuple(row[key] for key in keys) for row in rows]
    if actual != expected:
        raise ValueError("Texture filter case coverage differs")
    for row in rows:
        if len(row["pixels"]) != (16 if alpha_states else 32) or any(type(x) is not int or not 0 <= x <= 0xffffff for x in row["pixels"]):
            raise ValueError("Texture filter pixel coverage differs")
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--renderer-control", type=Path, help="Also test a private renderer source copy")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--pkg-config", default="pkg-config")
    parser.add_argument("--legacy-gl", action="store_true")
    profile = parser.add_mutually_exclusive_group()
    profile.add_argument("--alpha-test", action="store_true",
                        help="Use the scene's alpha test (>= 4/255) and diffuse alpha 40/255")
    profile.add_argument("--alpha-states", action="store_true",
                         help="Test eight alpha comparisons, four references, six materials and linear fog")
    parser.add_argument("--check", action="store_true", help="Require every production pixel to match Wine")
    args = parser.parse_args()
    source = ROOT / "tests/alpha-test-probe.cpp" if args.alpha_states else SOURCE
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    if args.renderer_control and not args.renderer_control.is_file():
        parser.error("Renderer control source is missing")
    for command in (args.compiler, args.pkg_config, "i686-w64-mingw32-g++", "wine", "wineserver", "xvfb-run", "timeout"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    compiler = str(Path(shutil.which(args.compiler)).resolve())
    package = str(Path(shutil.which(args.pkg_config)).resolve())
    output.mkdir(parents=True)
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                       GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    environment.pop("LP_NO_RAST", None)
    environment.pop("TH08_PROBE_ALPHA_TEST", None)
    if args.alpha_test:
        environment["TH08_PROBE_ALPHA_TEST"] = "1"
    binary = output / "filter-probe.exe"
    with (output / "native-build.log").open("w") as log:
        subprocess.run(["i686-w64-mingw32-g++", "-std=gnu++98", "-O2", "-static", str(source),
                        "-o", str(binary), "-ld3d8", "-lgdi32", "-luser32"],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    prefix = Path(tempfile.mkdtemp(prefix="texture-filter-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned texture filtering probe prefix\n")
    native_env = dict(environment, WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
                      WINEDLLOVERRIDES="winepulse.drv,winealsa.drv=d")
    binaries = dict(native=sha(binary))
    try:
        with (output / "native.stdout").open("w") as log:
            subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "90s", "wine", str(binary)],
                           env=native_env, stdout=log, stderr=subprocess.STDOUT, check=True)
        reference = decode(output / "native.stdout", args.alpha_states)
    finally:
        subprocess.run(["wineserver", "-k"], env=native_env, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=native_env, timeout=15, check=False)
        shutil.rmtree(prefix)
        binary.unlink()
    sources = dict(production=ROOT / "src/modern/linux/d3d8_compat.cpp")
    if args.renderer_control:
        path = output / "control-d3d8.cpp"
        path.write_bytes(args.renderer_control.read_bytes())
        sources["control"] = path
    package_command = [package, "--cflags", "--libs", "sdl2", "SDL2_image", "SDL2_ttf", "fontconfig", "gl"]
    flags = shlex.split(subprocess.check_output(package_command, text=True))
    report = dict(evidenceClass="Synthetic D3D8 texture filtering and blending API probe",
                  probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                  rendererSha256={key: sha(path) for key, path in sources.items()},
                  compiler=subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
                  wineVersion=subprocess.check_output(["wine", "--version"], text=True).strip(),
                  packageMetadataCommand=package_command, legacyOpenGL=args.legacy_gl,
                  alphaStateMatrix=args.alpha_states,
                  alphaTest=(dict(enabled=True, references=[0, 4, 128, 255], comparisons=list(range(1, 9)),
                                  diffuseArgb="material-dependent") if args.alpha_states else
                             dict(enabled=args.alpha_test, reference=4, comparison="greater-or-equal",
                                  diffuseArgb="28ffffff" if args.alpha_test else "d0c0f080")),
                  binarySha256=binaries, nativeSamplesSha256=sha(output / "native.stdout"),
                  formats=["A8R8G8B8"] if args.alpha_states else ["A1R5G5B5", "R5G6B5", "A4R4G4B4", "A8R8G8B8"],
                  samples=6144, compileCommands={}, variants={})
    for name, renderer in sources.items():
        binary = output / ("filter-probe-" + name)
        command = [compiler, "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
                   "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                   "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
                   "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux", str(source),
                   str(renderer), "src/modern/linux/linux_compat.cpp", "-o", str(binary), *flags, "-ldl"]
        report["compileCommands"][name] = command
        with (output / (name + "-build.log")).open("w") as log:
            subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        binaries[name] = sha(binary)
        local_env = dict(environment)
        if args.legacy_gl:
            local_env.update(MESA_GL_VERSION_OVERRIDE="2.1", MESA_EXTENSION_OVERRIDE="-GL_ARB_clip_control",
                             TH08_RENDERER_REQUIRE_LEGACY_GL="1")
        try:
            with (output / (name + ".stdout")).open("w") as log:
                subprocess.run(["xvfb-run", "-a", "timeout", "60s", str(binary)], env=local_env,
                               stdout=log, stderr=subprocess.STDOUT, check=True)
            candidate = decode(output / (name + ".stdout"), args.alpha_states)
        finally:
            binary.unlink()
        changes, equal, maximum = [], 0, 0
        for a, b in zip(reference, candidate):
            for index, (expected, actual) in enumerate(zip(a["pixels"], b["pixels"])):
                equal += expected == actual
                error = max(abs(((expected >> shift) & 255) - ((actual >> shift) & 255)) for shift in (0, 8, 16))
                maximum = max(maximum, error)
                if error:
                    changes.append(dict(**{key: a[key] for key in a if key != "pixels"},
                                        pixel=index, referenceRgb=expected, candidateRgb=actual,
                                        maximumChannelError=error))
        report["variants"][name] = dict(equalPixels=equal, maximumChannelError=maximum,
                                        candidateSamplesSha256=sha(output / (name + ".stdout")), differences=changes)
        print(f"{name}: {equal}/6144 identical pixels; maximum channel error {maximum}/255", flush=True)
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    if args.check and report["variants"]["production"]["equalPixels"] != 6144:
        raise SystemExit("Production texture filtering differs from Wine D3D8")


if __name__ == "__main__":
    main()
