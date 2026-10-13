#!/usr/bin/env python3
"""Compare portable texture-color samples with the Wine D3D8 API."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shlex
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(path):
    text = path.read_text()
    start = text.find("[\n")
    if start < 0:
        raise ValueError("Color probe JSON is missing: " + str(path))
    rows, _ = json.JSONDecoder().raw_decode(text[start:])
    expected = [(sample_kind, value) for kind, count in ((0, 32), (1, 64), (2, 16), (3, 32))
                for value in range(count) for sample_kind in (kind, kind + 10)]
    expected += [(4, value) for value in (1, 17, 31, 64, 127, 128, 191, 254, 255)]
    if [(row["kind"], row["value"]) for row in rows] != expected:
        raise ValueError("Color probe sample coverage differs")
    return rows


def expected_samples():
    """Pinned input/output contract, independently checked against Wine D3D8."""
    rows = []
    for kind, count in ((0, 32), (1, 64), (2, 16), (3, 32)):
        for value in range(count):
            low5 = value & 31
            expanded5 = (low5 << 3) | (low5 >> 2)
            if kind == 0:
                rgb = expanded5 * 0x010101
                word = 0x8000 | (value << 10) | (value << 5) | value
            elif kind == 1:
                expanded6 = (value << 2) | (value >> 4)
                rgb = (expanded5 << 16) | (expanded6 << 8) | expanded5
                word = (low5 << 11) | (value << 5) | low5
            elif kind == 2:
                rgb = value * 17 * 0x010101
                word = 0xf000 | (value << 8) | (value << 4) | value
            else:
                rgb = value * 8 * 0x010101
                word = 0xff000000 | rgb
            rows.extend((dict(kind=kind, value=value, rgb=rgb), dict(kind=kind + 10, value=value, rgb=word)))
    # Observed Wine/llvmpipe values include separate source/destination rounding.
    for alpha, rgb in ((1, 0x8040f0), (17, 0x8041ee), (31, 0x8142ed), (64, 0x8244e9),
                       (127, 0x8448e2), (128, 0x8549e3), (191, 0x874ddc),
                       (254, 0x8951d5), (255, 0x8951d5)):
        rows.append(dict(kind=4, value=alpha, rgb=rgb))
    return rows


def counterfactual(source):
    """Keep high bits when packing and replicate those bits when expanding."""
    replacements = {
        "((pixel >> 11) & 31) * 255 / 31": "((((pixel >> 11) & 31) << 3) | (((pixel >> 11) & 31) >> 2))",
        "((pixel >> 10) & 31) * 255 / 31": "((((pixel >> 10) & 31) << 3) | (((pixel >> 10) & 31) >> 2))",
        "((pixel >> 5) & 31) * 255 / 31": "((((pixel >> 5) & 31) << 3) | (((pixel >> 5) & 31) >> 2))",
        "(pixel & 31) * 255 / 31": "(((pixel & 31) << 3) | ((pixel & 31) >> 2))",
        "((pixel >> 5) & 63) * 255 / 63": "((((pixel >> 5) & 63) << 2) | (((pixel >> 5) & 63) >> 4))",
        "rgba[0] * 31 / 255": "rgba[0] >> 3", "rgba[1] * 31 / 255": "rgba[1] >> 3",
        "rgba[2] * 31 / 255": "rgba[2] >> 3", "rgba[1] * 63 / 255": "rgba[1] >> 2"}
    counts = {}
    for before, after in replacements.items():
        counts[before] = source.count(before)
        if not counts[before]:
            raise ValueError("Counterfactual source pattern is missing: " + before)
        source = source.replace(before, after)
    return source, counts


def run_native(output, environment):
    binary = output / "color-probe.exe"
    with (output / "native-build.log").open("w") as log:
        subprocess.run(["i686-w64-mingw32-g++", "-std=gnu++98", "-O2", "-static",
                        str(ROOT / "tests/texture-color-probe.cpp"), "-o", str(binary),
                        "-ld3d8", "-lgdi32", "-luser32"], stdout=log, stderr=subprocess.STDOUT, check=True)
    prefix = Path(tempfile.mkdtemp(prefix="texture-colors-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned texture color probe prefix\n")
    env = dict(environment, WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
               WINEDLLOVERRIDES="winepulse.drv,winealsa.drv=d")
    try:
        with (output / "native.stdout").open("w") as stdout, (output / "native.log").open("w") as stderr:
            subprocess.run(["xvfb-run", "-a", "timeout", "--kill-after=5s", "90s", "wine", str(binary)],
                           env=env, stdout=stdout, stderr=stderr, check=True)
        rows = decode(output / "native.stdout")
    finally:
        subprocess.run(["wineserver", "-k"], env=env, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=env, timeout=15, check=False)
        shutil.rmtree(prefix)
    (output / "native.json").write_text(json.dumps(rows, indent=2) + "\n")
    return rows


def build_game(output, compiler, environment):
    source = output / "game-source"
    source.mkdir()
    for name in ("src", "scripts", "config", "resources"):
        (source / name).symlink_to(ROOT / name, target_is_directory=True)
    for stem in ("d3d8", "d3dx8"):
        shutil.copyfile(output / f"corrected-{stem}.cpp", source / f"corrected-{stem}.cpp")
    cmake = (ROOT / "CMakeLists.txt").read_text() + '''
# Private color-conversion counterfactual; production source is unchanged.
set_source_files_properties(src/modern/linux/d3d8_compat.cpp src/modern/linux/d3dx8_compat.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(th08-modern PRIVATE corrected-d3d8.cpp corrected-d3dx8.cpp)
set_source_files_properties(corrected-d3d8.cpp corrected-d3dx8.cpp PROPERTIES COMPILE_OPTIONS "-O2;-fno-strict-aliasing")
target_include_directories(th08-modern PRIVATE "${CMAKE_SOURCE_DIR}/src/modern/linux")
'''
    (source / "CMakeLists.txt").write_text(cmake)
    directory = output / "game-build"
    with (output / "game-build.log").open("w") as log:
        subprocess.run(["cmake", "-S", str(source), "-B", str(directory), "-G", "Ninja",
                        "-DCMAKE_CXX_COMPILER=" + shutil.which(compiler), "-DCMAKE_BUILD_TYPE=Debug",
                        "-DTH08_ENABLE_REPLAY_TRACE=ON"], env=environment, stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run(["cmake", "--build", str(directory), "--parallel", "1"],
                       env=environment, stdout=log, stderr=subprocess.STDOUT, check=True)
    binary = output / "th08-color-counterfactual"
    shutil.copyfile(directory / "th08-modern", binary)
    binary.chmod(0o755)
    subprocess.run([str(ROOT / "scripts/verify-portable-linux.sh"), str(binary), "x86_64"], check=True)
    shutil.rmtree(directory)
    return dict(executable=str(binary), executableSha256=sha(binary),
                compiler=subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
                compilerTreeRemoved=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--wine", action="store_true", help="Also compile and run a Windows D3D8 reference probe")
    parser.add_argument("--baseline-ref", help="Also observe the codecs at this immutable Git revision")
    parser.add_argument("--check", action="store_true", help="Require production API samples and exhaustive copies to pass")
    parser.add_argument("--expect-baseline-failure", action="store_true", help="Require old 5/6-bit expansion and copy defects")
    parser.add_argument("--counterfactual", action="store_true", help="Also test private corrected codecs; production source stays unchanged")
    parser.add_argument("--build-game", action="store_true", help="Build the private game counterfactual with one compiler job")
    parser.add_argument("--game-compiler", default="g++-8", help="Game compiler; default: the recorded GCC 8")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    if args.build_game and not args.counterfactual:
        parser.error("--build-game requires --counterfactual")
    if args.expect_baseline_failure and not args.baseline_ref:
        parser.error("--expect-baseline-failure requires --baseline-ref")
    if args.counterfactual and not args.baseline_ref:
        parser.error("--counterfactual requires the pre-repair --baseline-ref")
    for command in ("g++", "pkg-config", "xvfb-run", "timeout", *( ("i686-w64-mingw32-g++", "wine", "wineserver") if args.wine else ())):
        if not shutil.which(command):
            parser.error("Required command is missing: " + command)
    if args.build_game:
        for command in ("cmake", "ninja", args.game_compiler):
            if not shutil.which(command):
                parser.error("Required command is missing: " + command)
    output.mkdir(parents=True)
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                       GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    environment.pop("LP_NO_RAST", None)
    environment.setdefault("PKG_CONFIG_LIBDIR", "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
    flags = shlex.split(subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl2", "SDL2_image",
                                                "SDL2_ttf", "fontconfig", "gl"], env=environment, text=True))
    observations = {}
    if args.wine:
        observations["native"] = run_native(output, environment)
        if observations["native"] != expected_samples():
            raise SystemExit("Wine D3D8 samples differ from the pinned color contract")
    else:
        observations["native"] = expected_samples()
    sources = {"production": ROOT / "src/modern/linux/d3d8_compat.cpp"}
    baseline = None
    if args.baseline_ref:
        baseline = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                            args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
        for stem in ("d3d8", "d3dx8"):
            path = output / f"baseline-{stem}.cpp"
            path.write_bytes(subprocess.check_output(["git", "show", baseline + f":src/modern/linux/{stem}_compat.cpp"], cwd=ROOT))
        sources["baseline"] = output / "baseline-d3d8.cpp"
    if args.counterfactual:
        for stem in ("d3d8", "d3dx8"):
            source, _ = counterfactual((output / f"baseline-{stem}.cpp").read_text())
            path = output / f"corrected-{stem}.cpp"
            path.write_text(source)
            if stem == "d3d8":
                sources["counterfactual"] = path
    copy_sources = {}
    for name, source in sources.items():
        binary = output / ("color-probe-" + name)
        d3dx = (output / "corrected-d3dx8.cpp" if name == "counterfactual" else
                output / "baseline-d3dx8.cpp" if name == "baseline" else ROOT / "src/modern/linux/d3dx8_compat.cpp")
        copy_sources[name] = d3dx
        command = ["g++", "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
                   "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                   "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
                   "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux",
                   "tests/texture-color-probe.cpp", str(source), str(d3dx), "src/modern/linux/linux_compat.cpp",
                   "-o", str(binary), *flags, "-ldl"]
        with (output / (name + "-build.log")).open("w") as log:
            subprocess.run(command, cwd=ROOT, env=environment, stdout=log, stderr=subprocess.STDOUT, check=True)
        with (output / (name + ".stdout")).open("w") as stdout:
            subprocess.run(["xvfb-run", "-a", "timeout", "30s", str(binary)], env=environment,
                           stdout=stdout, stderr=subprocess.STDOUT, check=True)
        rows = decode(output / (name + ".stdout"))
        observations[name] = rows
        (output / (name + ".json")).write_text(json.dumps(rows, indent=2) + "\n")
        with (output / (name + "-d3dx.stdout")).open("w") as stdout:
            subprocess.run(["xvfb-run", "-a", "timeout", "30s", str(binary), "d3dx-copies"], env=environment,
                           stdout=stdout, stderr=subprocess.STDOUT, check=True)
        dx_rows = decode(output / (name + "-d3dx.stdout"))
        copies = [dict(d3d8=a, d3dx=b) for a, b in zip(rows, dx_rows) if a["kind"] >= 10 and a != b]
        (output / (name + "-d3dx.json")).write_text(json.dumps(dict(
            evidenceClass="Portable identical-format D3DX copies compared with bit-preserving D3D8 copies",
            samples=sum(row["kind"] >= 10 for row in rows), differences=copies), indent=2) + "\n")
        print(f"{name} D3DX copies: {len(copies)} differing raw words out of 144")
        with (output / (name + "-exhaustive.stdout")).open("w") as stdout:
            subprocess.run(["xvfb-run", "-a", "timeout", "30s", str(binary), "exhaustive-copies"],
                           env=environment, stdout=stdout, stderr=subprocess.STDOUT, check=True)
        exhaustive = json.loads((output / (name + "-exhaustive.stdout")).read_text())
        if [(entry["format"], entry["words"]) for entry in exhaustive] != [
                ("A1R5G5B5", 65536), ("R5G6B5", 65536), ("A4R4G4B4", 65536)]:
            raise ValueError("Exhaustive copy coverage differs")
        (output / (name + "-exhaustive.json")).write_text(json.dumps(exhaustive, indent=2) + "\n")
        binary.unlink()
    report = dict(evidenceClass="Portable backend probes against Wine D3D8 samples; no TH08 target execution",
                  host=platform.platform(), probeSha256=sha(ROOT / "tests/texture-color-probe.cpp"),
                  runnerSha256=sha(Path(__file__)), runnerModifiesProductionSource=False,
                  compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
                  baselineCommit=baseline,
                  backendSha256={name: sha(path) for name, path in sources.items()},
                  surfaceCopyBackendSha256={name: sha(path) for name, path in copy_sources.items()}, comparisons={})
    report["referenceMethod"] = "fresh Wine D3D8 API samples" if args.wine else "pinned Wine-verified color contract"
    for name in sources:
        differences = [dict(reference=a, candidate=b) for a, b in zip(observations["native"], observations[name]) if a != b]
        report["comparisons"][name] = dict(samples=len(observations[name]), differences=differences)
        print(f"{name}: {len(differences)} differing samples out of {len(observations[name])}")
    if args.wine:
        (output / "color-probe.exe").unlink()
    if args.build_game:
        report["privateGame"] = build_game(output, args.game_compiler, environment)
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    if args.check:
        production_copies = json.loads((output / "production-d3dx.json").read_text())
        production_exhaustive = json.loads((output / "production-exhaustive.json").read_text())
        if (report["comparisons"]["production"]["differences"] or production_copies["differences"]
                or any(entry["mismatches"] for entry in production_exhaustive)):
            raise SystemExit("Production texture-color contracts failed")
        print("PASS: production texture colors and 196608 identical-format copies")
    if args.expect_baseline_failure:
        failed = report["comparisons"].get("baseline", {}).get("differences", [])
        copies = json.loads((output / "baseline-d3dx.json").read_text())
        exhaustive = json.loads((output / "baseline-exhaustive.json").read_text())
        if (len(failed) != 59 or len(copies["differences"]) != 92
                or not exhaustive[0]["mismatches"] or not exhaustive[1]["mismatches"] or exhaustive[2]["mismatches"]):
            raise SystemExit("Baseline did not reproduce the named 5/6-bit defects")
        print("PASS: baseline 5/6-bit expansion and lossy-copy defects reproduced")


if __name__ == "__main__":
    main()
