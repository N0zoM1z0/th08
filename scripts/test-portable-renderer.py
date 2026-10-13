#!/usr/bin/env python3
"""Build and run asset-free pixel contracts against the Linux D3D8 backend."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--pkg-config", default="pkg-config", help="Package metadata command for the compiler's target architecture")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "build/portable-renderer-test")
    parser.add_argument("--baseline-ref", help="Compile the renderer from a repository commit for a regression control")
    parser.add_argument("--expect-failures", action="store_true", help="Require selected baseline assertions to fail; default: the four original groups")
    parser.add_argument("--cases", help="Comma-separated contracts; default: locks,perspective,clipping,fog,edges,orientation. Use fpu for i386 device precision")
    parser.add_argument("--legacy-gl", action="store_true", help="Exercise Mesa OpenGL 2.1 without ARB_clip_control")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("--output-dir must be under this repository's build/.")
    if args.expect_failures and not args.baseline_ref:
        parser.error("--expect-failures requires --baseline-ref.")
    for command in (args.compiler, args.pkg_config, "xvfb-run", "timeout"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    output.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ)
    if args.legacy_gl:
        environment.update(MESA_GL_VERSION_OVERRIDE="2.1", MESA_EXTENSION_OVERRIDE="-GL_ARB_clip_control",
                           TH08_RENDERER_REQUIRE_LEGACY_GL="1")
    environment.setdefault("PKG_CONFIG_LIBDIR", "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
    package_command = [args.pkg_config, "--cflags", "--libs", "sdl2", "SDL2_image", "SDL2_ttf", "fontconfig", "gl", "x11"]
    flags = shlex.split(subprocess.check_output(package_command, env=environment, text=True))
    binary = output / "portable-renderer-test"
    source = ROOT / "src/modern/linux/d3d8_compat.cpp"
    commit = None
    if args.baseline_ref:
        commit = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                          args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
        source = output / "baseline-d3d8.cpp"
        source.write_bytes(subprocess.check_output(["git", "show", commit + ":src/modern/linux/d3d8_compat.cpp"], cwd=ROOT))
        print("Renderer baseline: " + commit, flush=True)
    command = [args.compiler, "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
               "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
               "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
               "-include", "src/modern/linux/linux_compat.hpp", "tests/portable-renderer.cpp",
               "-Isrc/modern/linux", str(source), "src/modern/linux/linux_compat.cpp",
               "-o", str(binary), *flags, "-ldl"]
    subprocess.run(command, cwd=ROOT, env=environment, check=True)
    identities = {"renderer": source, "gdi": ROOT / "src/modern/linux/linux_compat.cpp",
                  "probe": ROOT / "tests/portable-renderer.cpp", "runner": Path(__file__)}
    report = dict(result="running", baselineCommit=commit, legacyOpenGL=args.legacy_gl,
                  expectedFailures=args.expect_failures, compileCommand=command,
                  packageMetadataCommand=package_command,
                  compiler=subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
                  binarySha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                  sourceSha256={name: hashlib.sha256(path.read_bytes()).hexdigest() for name, path in identities.items()},
                  contextOverrides={name: environment.get(name) for name in
                                    ("MESA_GL_VERSION_OVERRIDE", "MESA_EXTENSION_OVERRIDE")}, cases=[])
    def save_report():
        (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    save_report()
    controls = {"locks": "flag-0 lock sees current GPU pixels",
                "perspective": "XYZRHW uses reciprocal-depth texture interpolation",
                "clipping": "vertices behind the camera are clipped",
                "fog": "CPU upload ignores current fog and viewport",
                "edges": "horizontal top edge covers its pixel center",
                "orientation": "window presentation preserves the logical top"}
    if args.expect_failures and not args.cases:
        controls = {name: controls[name] for name in ("locks", "perspective", "clipping", "fog")}
    if args.cases:
        available = dict(controls, fpu="device uses retail single-precision x87 mode")
        requested = args.cases.split(",")
        if len(requested) != len(set(requested)) or any(name not in available for name in requested):
            parser.error("Contract names must be unique and selected from locks,perspective,clipping,fog,edges,orientation,fpu")
        controls = {name: available[name] for name in requested}
    for case, assertion in controls.items():
        result = subprocess.run(["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", "timeout", "30s", str(binary), case],
                                cwd=output, env=dict(environment, SDL_AUDIODRIVER="dummy", SDL_VIDEODRIVER="x11", LIBGL_ALWAYS_SOFTWARE="1",
                                                     GALLIUM_DRIVER="llvmpipe"), capture_output=True, text=True)
        combined = result.stdout + result.stderr
        print(combined, end="", flush=True)
        valid = result.returncode == 0
        if args.expect_failures:
            valid = result.returncode == 1 and "FAIL: " + assertion in combined
        report["cases"].append(dict(case=case, exitCode=result.returncode, output=combined, passed=valid))
        report["result"] = "running" if valid else "failed"
        save_report()
        if not valid:
            raise RuntimeError("Renderer contract failed its expected result: " + case)
        if args.expect_failures:
            print("PASS: reproduced baseline defect for " + case, flush=True)
    report["result"] = "passed"
    save_report()


if __name__ == "__main__":
    main()
