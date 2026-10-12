#!/usr/bin/env python3
"""Build and run asset-free pixel contracts against the Linux D3D8 backend."""
import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "build/portable-renderer-test")
    parser.add_argument("--baseline-ref", help="Compile the renderer from a repository commit for a regression control")
    parser.add_argument("--expect-failures", action="store_true", help="Require the four known baseline assertions to fail")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("--output-dir must be under this repository's build/.")
    if args.expect_failures and not args.baseline_ref:
        parser.error("--expect-failures requires --baseline-ref.")
    for command in (args.compiler, "pkg-config", "xvfb-run", "timeout"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    output.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ)
    environment.setdefault("PKG_CONFIG_LIBDIR", "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
    flags = shlex.split(subprocess.check_output(
        ["pkg-config", "--cflags", "--libs", "sdl2", "SDL2_image", "SDL2_ttf", "fontconfig", "gl"],
        env=environment, text=True))
    binary = output / "portable-renderer-test"
    source = ROOT / "src/modern/linux/d3d8_compat.cpp"
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
    controls = {"locks": "flag-0 lock sees current GPU pixels",
                "perspective": "XYZRHW uses reciprocal-depth texture interpolation",
                "clipping": "vertices behind the camera are clipped",
                "fog": "CPU upload ignores current fog and viewport"}
    for case, assertion in controls.items():
        result = subprocess.run(["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", "timeout", "30s", str(binary), case],
                                cwd=output, env=dict(environment, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                                                     GALLIUM_DRIVER="llvmpipe"), capture_output=True, text=True)
        combined = result.stdout + result.stderr
        print(combined, end="", flush=True)
        if args.expect_failures:
            if result.returncode != 1 or "FAIL: " + assertion not in combined:
                raise RuntimeError("Baseline did not fail at its expected assertion: " + case)
            print("PASS: reproduced baseline defect for " + case, flush=True)
        elif result.returncode:
            raise RuntimeError("Renderer contract failed: " + case)


if __name__ == "__main__":
    main()
