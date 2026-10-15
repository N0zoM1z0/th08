#!/usr/bin/env python3
"""Measure warmed low-alpha draw cost with pinned renderer source variants."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-ref", required=True, help="Immutable renderer baseline commit")
    parser.add_argument("--renderer-control", type=Path, help="Candidate source; default: current renderer")
    parser.add_argument("--reference-control", type=Path, help="Optional independent source with the candidate's expected pixels")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--pkg-config", default="pkg-config")
    parser.add_argument("--iterations", type=int, default=3, choices=range(1, 10))
    parser.add_argument("--legacy-gl", action="store_true")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    for command in (args.compiler, args.pkg_config, "git", "xvfb-run", "timeout"):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    candidate = (args.renderer_control or ROOT / "src/modern/linux/d3d8_compat.cpp").resolve()
    for path in (candidate, args.reference_control):
        if path and not path.is_file():
            parser.error("Renderer source is missing: " + str(path))
    commit = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                      args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
    output.mkdir(parents=True)
    baseline = output / "baseline.cpp"
    baseline.write_bytes(subprocess.check_output(["git", "show", commit + ":src/modern/linux/d3d8_compat.cpp"], cwd=ROOT))
    sources = dict(baseline=baseline, candidate=candidate)
    if args.reference_control:
        sources["reference"] = args.reference_control.resolve()
    source = ROOT / "tests/alpha-render-benchmark.cpp"
    compiler = str(Path(shutil.which(args.compiler)).resolve())
    package = str(Path(shutil.which(args.pkg_config)).resolve())
    flags = shlex.split(subprocess.check_output([package, "--cflags", "--libs", "sdl2", "SDL2_image",
                                               "SDL2_ttf", "fontconfig", "gl"], text=True))
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                       GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    environment.pop("LP_NO_RAST", None)
    if args.legacy_gl:
        environment.update(MESA_GL_VERSION_OVERRIDE="2.1", MESA_EXTENSION_OVERRIDE="-GL_ARB_clip_control",
                           TH08_RENDERER_REQUIRE_LEGACY_GL="1")
    report = dict(evidenceClass=__doc__, baselineCommit=commit, legacyOpenGL=args.legacy_gl,
                  compiler=subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
                  frames=20, layers=12, warmupFrames=3, quad=[32, 16, 416, 464],
                  probeSha256=sha(source), runnerSha256=sha(Path(__file__)),
                  rendererSha256={key: sha(path) for key, path in sources.items()},
                  compileCommands={}, binarySha256={}, runs=[])
    binaries = []
    try:
        for name, renderer in sources.items():
            binary = output / ("benchmark-" + name)
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
        for iteration in range(args.iterations):
            for name in sources:
                text = subprocess.check_output(["xvfb-run", "-a", "timeout", "90s",
                                                str(output / ("benchmark-" + name))], env=environment, text=True)
                (output / f"{name}-{iteration}.stdout").write_text(text)
                row = json.loads(next(line for line in text.splitlines() if line.startswith("{")))
                if type(row.get("seconds")) not in (int, float) or not 0 < row["seconds"] < 90:
                    raise RuntimeError("Benchmark duration is invalid")
                if type(row.get("checksum")) is not int or not 0 <= row["checksum"] <= 0xffffffff:
                    raise RuntimeError("Benchmark checksum is invalid")
                report["runs"].append(dict(row, variant=name, iteration=iteration))
        for name in sources:
            rows = [row for row in report["runs"] if row["variant"] == name]
            if len({row["checksum"] for row in rows}) != 1:
                raise RuntimeError("Benchmark image changed between iterations: " + name)
        report["medianSeconds"] = {name: statistics.median(row["seconds"] for row in report["runs"]
                                                         if row["variant"] == name) for name in sources}
        report["candidateMatchesReference"] = (None if "reference" not in sources else
            {row["checksum"] for row in report["runs"] if row["variant"] == "candidate"} ==
            {row["checksum"] for row in report["runs"] if row["variant"] == "reference"})
        (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
        for name, seconds in report["medianSeconds"].items():
            print(f"{name}: median {seconds:.6f} seconds across {args.iterations} warmed runs", flush=True)
        if report["candidateMatchesReference"] is False:
            raise SystemExit("Candidate benchmark pixels differ from the reference control")
    finally:
        for binary in binaries:
            binary.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
