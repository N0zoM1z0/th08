#!/usr/bin/env python3
"""Check D3DX rounding against replay goldens and an optional pinned VC7 oracle."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import random
import shlex
import struct
import subprocess
import tempfile
import tomllib

ROOT = Path(__file__).resolve().parents[1]
KINDS = {"quaternion": (0, 4, 16), "transform": (1, 19, 3), "project": (2, 57, 3),
         "lookat": (3, 9, 16), "perspective": (4, 4, 16)}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def word(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def run(command, cases, directory):
    lines = [str(len(cases))]
    for case in cases:
        kind, inputs, _ = KINDS[case["kind"]]
        if len(case["inputWords"]) != inputs:
            raise ValueError("Incorrect input width for " + case["id"])
        lines.append(" ".join(map(str, [kind, *case["inputWords"]])))
    output = subprocess.check_output(command, cwd=directory, input="\n".join(lines) + "\n", text=True, timeout=60)
    rows = [list(map(int, line.split())) for line in output.splitlines()]
    if len(rows) != len(cases):
        raise ValueError("Probe output does not cover every requested case")
    for index, (case, row) in enumerate(zip(cases, rows)):
        if len(row) != KINDS[case["kind"]][2] + 1 or row[0] != index:
            raise ValueError("Probe output order or width differs")
        if any(not 0 <= value <= 0xFFFFFFFF for value in row[1:]):
            raise ValueError("Probe output is not a binary32 word")
    return [row[1:] for row in rows]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--pkg-config", default="pkg-config")
    parser.add_argument("--baseline-ref", help="Compile the D3DX adapter from this revision")
    parser.add_argument("--expect-failure", action="store_true", help="Require both historical matrix failures")
    parser.add_argument("--expect-perspective-failure", action="store_true", help="Require only the three historical perspective failures")
    parser.add_argument("--native", action="store_true", help="Also compare generated inputs with the SHA-pinned VC7 D3DX8 library")
    parser.add_argument("--attest-target", action="store_true", help="Also compare both pinned library sections with resources/th08.exe")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "build/d3dx-rounding")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build") or output.exists():
        parser.error("Output must be a fresh directory under build/")
    if args.attest_target and not args.native:
        parser.error("--attest-target requires --native")
    if args.expect_failure and not args.baseline_ref:
        parser.error("--expect-failure requires --baseline-ref")
    if args.expect_perspective_failure and not args.baseline_ref:
        parser.error("--expect-perspective-failure requires --baseline-ref")
    if args.expect_failure and args.expect_perspective_failure:
        parser.error("Select one historical failure control")
    output.mkdir(parents=True)
    fixture = ROOT / "tests/fixtures/d3dx-rounding.json"
    fixture_data = json.loads(fixture.read_text())
    cases = fixture_data["cases"]
    report = dict(result="running", nativeOracle=args.native, baseline=args.baseline_ref,
                  runnerSha256=sha(Path(__file__)), fixtureSha256=sha(fixture),
                  probeSha256=sha(ROOT / "tests/portable-d3dx-rounding.cpp"),
                  compiler=subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
                  compileCommands={})
    try:
        with tempfile.TemporaryDirectory(prefix="d3dx-rounding.", dir=output) as temporary:
            temporary = Path(temporary)
            source = ROOT / "src/modern/linux/d3dx8_compat.cpp"
            if args.baseline_ref:
                revision = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                    args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
                source = temporary / "baseline-d3dx.cpp"
                source.write_bytes(subprocess.check_output(["git", "show", revision + ":src/modern/linux/d3dx8_compat.cpp"], cwd=ROOT))
                report["baseline"] = revision
            flags = shlex.split(subprocess.check_output([args.pkg_config, "--cflags", "--libs", "sdl2", "SDL2_image"], text=True))
            executable = temporary / "portable"
            command = [args.compiler, "-std=gnu++98", "-O0", "-ffunction-sections", "-fdata-sections",
                "-fno-strict-aliasing", "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
                "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include", "-Isrc/modern/linux",
                "-include", "src/modern/linux/linux_compat.hpp", str(ROOT / "tests/portable-d3dx-rounding.cpp"),
                str(source), "-Wl,--gc-sections", "-o", str(executable), *flags]
            report["compileCommands"]["portable"] = command
            report["adapterSha256"] = sha(source)
            with (output / "portable-build.log").open("w") as log:
                subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
            report["portableBinarySha256"] = sha(executable)
            if args.native:
                from library_match import verify_archive
                from winhelpers import get_windows_path
                archives = tomllib.loads((ROOT / "config/library-provenance.toml").read_text())["archives"]
                archive = next(row for row in archives if row["id"] == "vc7-d3dx8-prerelease")
                archive_bytes = verify_archive(ROOT / archive["path"], archive["sha256"])
                report["archiveSha256"] = archive["sha256"]
                if args.attest_target:
                    from library_match import archive_members, compare_member, pe_bytes_at
                    target = (ROOT / "resources/th08.exe").read_bytes()
                    if len(target) != 840704 or hashlib.sha256(target).hexdigest() != fixture_data["targetSha256"]:
                        raise ValueError("Target is not the canonical Japanese TH08 1.00d executable")
                    members = archive_members(archive_bytes)
                    report["targetArchiveComparisons"] = []
                    for unit in fixture_data["referenceUnits"]:
                        candidates = members[unit["member"]]
                        if len(candidates) != 1:
                            raise ValueError("Reference member is ambiguous")
                        result = compare_member(candidates[0], unit,
                            pe_bytes_at(target, unit["target_address"], unit["compare_size"]))
                        report["targetArchiveComparisons"].append(dict(unit=unit, comparison=result))
                        if result["result"] != "exact":
                            raise ValueError("Reference member differs from the canonical target")
                    if pe_bytes_at(target, 0x4B42EC, 4) != bytes.fromhex("00000040"):
                        raise ValueError("Target quaternion constant is not binary32 2.0")
                    if pe_bytes_at(target, 0x4B847C, 4) != bytes.fromhex("0000003f"):
                        raise ValueError("Target perspective constant is not binary32 0.5")
                native = temporary / "reference.exe"
                command = [str(ROOT / "scripts/wineth08"), str(ROOT / "scripts/th08run.bat"), "cl.exe", "/nologo", "/MT", "/Od",
                    "/Fo" + get_windows_path(temporary / "reference.obj"), "/Fe" + get_windows_path(native),
                    get_windows_path(ROOT / "tests/portable-d3dx-rounding.cpp"), "/link",
                    get_windows_path(ROOT / archive["path"]), "advapi32.lib"]
                report["compileCommands"]["native"] = command
                with (output / "native-build.log").open("w") as log:
                    subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=120)
                reference = run([str(ROOT / "scripts/wineth08"), str(native)], cases, ROOT)
                if reference != [case["expectedWords"] for case in cases]:
                    raise ValueError("Pinned VC7 library does not reproduce the committed goldens")
                report["nativeBinarySha256"] = sha(native)
                randomizer = random.Random(0x54483038)
                extra = []
                for index in range(128):
                    vector = [randomizer.uniform(-2, 2) for _ in range(4)]
                    magnitude = math.sqrt(sum(value * value for value in vector))
                    extra.append(dict(id=f"generated-quaternion-{index}", kind="quaternion",
                                      inputWords=[word(value / magnitude) for value in vector]))
                    extra.append(dict(id=f"generated-transform-{index}", kind="transform",
                        inputWords=[word(randomizer.uniform(-256, 256)) for _ in range(3)] +
                                   [word(randomizer.uniform(-2, 2)) for _ in range(16)]))
                camera_randomizer = random.Random(0x54483038)
                for index in range(128):
                    eye = [camera_randomizer.uniform(-256, 256) for _ in range(3)]
                    at = [eye[axis] + camera_randomizer.uniform(-512, 512) for axis in range(3)]
                    up = [camera_randomizer.uniform(-1, 1), -1.0, camera_randomizer.uniform(-1, 1)]
                    extra.append(dict(id=f"generated-lookat-{index}", kind="lookat", inputWords=list(map(word, eye + at + up))))
                    fov = camera_randomizer.uniform(0.15, 2.5)
                    near = camera_randomizer.uniform(0.1, 50)
                    far = camera_randomizer.uniform(1800, 10000)
                    extra.append(dict(id=f"generated-perspective-{index}", kind="perspective",
                                      inputWords=list(map(word, [fov, 384.0 / 448, near, far]))))
                expected = run([str(ROOT / "scripts/wineth08"), str(native)], extra, ROOT)
                for case, expected_words in zip(extra, expected):
                    case["expectedWords"] = expected_words
                cases += extra
            actual = run([str(executable)], cases, ROOT)
            report["cases"] = [dict(id=case["id"], kind=case["kind"], inputWords=case["inputWords"],
                expectedWords=case["expectedWords"], actualWords=words, equal=words == case["expectedWords"])
                for case, words in zip(cases, actual)]
            failures = [case for case in report["cases"] if not case["equal"]]
            if args.expect_perspective_failure:
                historical = {f"perspective-oracle-{index}" for index in (1, 2, 3)}
                if historical != {case["id"] for case in failures}:
                    raise ValueError("Expected only the three historical perspective failures")
                report["result"] = "historical perspective failures reproduced"
            elif args.expect_failure:
                historical = {"finalb-11490-quaternion", "finalb-17460-quaternion"}
                if not historical <= {case["id"] for case in failures}:
                    raise ValueError("Both historical quaternion failures were not reproduced")
                report["result"] = "historical failures reproduced"
            else:
                report["result"] = "mismatch" if failures else "equal"
            print(f"D3DX rounding: {len(cases) - len(failures)}/{len(cases)} cases equal; {report['result']}")
            return 0 if args.expect_failure or args.expect_perspective_failure or not failures else 1
    except BaseException as error:
        report.update(result="error", error=str(error))
        raise
    finally:
        (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    raise SystemExit(main())
