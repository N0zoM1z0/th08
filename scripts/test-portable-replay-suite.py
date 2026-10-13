#!/usr/bin/env python3
"""Run the credited portable replay corpus serially, with validated resume."""
import argparse
import gzip
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

from replay_file import CORPUS, inspect as inspect_replay, verify_fixture

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("comparison", ROOT / "scripts/compare-replay-traces.py")
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)
spec = importlib.util.spec_from_file_location("portable_capture", ROOT / "scripts/capture-portable-replay.py")
portable_capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(portable_capture)


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def write_json(path, value):
    temporary = path.with_suffix(".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def share_equal_trace(reference, candidate):
    """Share storage only after equality and published expectations passed."""
    source, destination = reference / "rows.jsonl.gz", candidate / "rows.jsonl.gz"
    if source.stat().st_dev != destination.stat().st_dev:
        return
    if source.stat().st_ino == destination.stat().st_ino:
        return
    def content_sha(path):
        with gzip.open(path, "rb") as stream:
            return hashlib.file_digest(stream, "sha256").hexdigest()
    if content_sha(source) != content_sha(destination):
        return
    temporary = destination.with_suffix(".shared")
    temporary.unlink(missing_ok=True)
    os.link(source, temporary)
    temporary.replace(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--reference-dir", type=Path, help="Existing retail suite; missing references are captured afresh")
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--cases", help="Comma-separated fixture IDs; default: all 108 accepted recordings")
    parser.add_argument("--backend", choices=("recorder", "gdb"), default="recorder")
    parser.add_argument("--timeout", type=int, default=1800, help="Deadline per portable or retail capture, seconds")
    parser.add_argument("--paced", action="store_true", help="Use ordinary portable pacing and rasterization")
    parser.add_argument("--keep-going", action="store_true", help="Record failures and continue the remaining fixtures")
    parser.add_argument("--verify-only", action="store_true",
                        help="Revalidate a completed suite without launching or changing captures")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("--output-dir must be under build/")
    fixtures = json.loads((ROOT / "config/replay-fixtures.json").read_text())["fixtures"]
    if args.cases:
        selected = set(args.cases.split(","))
        fixtures = [entry for entry in fixtures if entry["id"] in selected]
        if selected != {entry["id"] for entry in fixtures}:
            parser.error("Unknown portable replay fixture ID")
    candidate, data = args.candidate.resolve(strict=True), args.game_data_dir.resolve(strict=True)
    if args.backend == "recorder":
        symbols = subprocess.check_output(["nm", "-C", str(candidate)], text=True)
        if "th08::modern::ReplayTraceRecord(" not in symbols:
            parser.error("Recorder backend requires TH08_ENABLE_REPLAY_TRACE=ON")
    identity = dict(candidateSha256=sha(candidate), targetSha256=sha(args.target),
                    gameDataSha256=sha(data / "th08.dat"), bgmDataSha256=sha(data / "thbgm.dat"),
                    backend=args.backend, paced=args.paced,
                    observerSha256=sha(ROOT / ("scripts/gdb/portable-replay-observer.py" if args.backend == "gdb"
                                              else "src/modern/linux/replay_trace.cpp")),
                    schemaSha256=sha(ROOT / "scripts/replay-external-schema.json"))
    if identity["targetSha256"] != "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924" or args.target.stat().st_size != 840704:
        parser.error("Target must be the canonical Japanese TH08 1.00d executable")
    if args.verify_only and not output.is_dir():
        parser.error("--verify-only requires an existing output directory")
    output.mkdir(parents=True, exist_ok=True)
    ledger_path = output / "suite.json"
    if args.verify_only and not ledger_path.is_file():
        parser.error("--verify-only requires an existing suite.json")
    ledger = json.loads(ledger_path.read_text()) if ledger_path.exists() else dict(version=1, cases={})
    if any(ledger.get("identity", identity).get(field) != value for field, value in identity.items()):
        parser.error("Suite identity changed; use a fresh output directory")
    ledger["identity"] = identity
    wine = output / "wine"
    failed = False
    verified = 0
    verified_calculations = 0
    try:
        for index, fixture in enumerate(fixtures, 1):
            case_id = fixture["id"]
            print(f"[{index}/{len(fixtures)}] {case_id}", flush=True)
            replay = CORPUS / "accepted" / fixture["file"]
            verify_fixture(replay, fixture)
            case = output / case_id
            if args.verify_only and not case.is_dir():
                raise ValueError("Completed portable case is missing: " + case_id)
            case.mkdir(exist_ok=True)
            reference = args.reference_dir.resolve() / case_id / "reference" if args.reference_dir else case / "reference"
            if not reference.exists():
                if args.verify_only:
                    raise ValueError("Completed retail reference is missing: " + case_id)
                reference = case / "reference"
                command = [sys.executable, str(ROOT / "scripts/capture-replay.py"), "--target", str(args.target.resolve()),
                           "--replay", str(replay), "--game-data", str(data / "th08.dat"), "--bgm-data", str(data / "thbgm.dat"),
                           "--output-dir", str(reference), "--wine-prefix", str(wine), "--clock-rate", "128",
                           "--no-rasterization", "--observer-backend", "ptrace", "--timeout", str(args.timeout)]
                with (case / "reference.log").open("w") as log:
                    subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
                for name in ("rows.jsonl", "gdb.log"):
                    path = reference / name
                    if path.exists():
                        with path.open("rb") as source, gzip.open(str(path) + ".gz", "wb") as destination:
                            shutil.copyfileobj(source, destination)
                        path.unlink()
            previous = ledger["cases"].get(case_id, {})
            if args.verify_only:
                if previous.get("result") != "equal" or previous.get("fixtureSha256") != fixture["sha256"]:
                    raise ValueError("Completed suite result is missing or changed: " + case_id)
                if Path(previous.get("reference", "")).resolve() != reference.resolve():
                    raise ValueError("Completed suite reference changed: " + case_id)
                result = previous
            else:
                result = dict(fixtureSha256=fixture["sha256"], reference=str(reference), result="running")
                ledger["cases"][case_id] = result
                write_json(ledger_path, ledger)
            capture = case / "candidate"
            try:
                observer_path = ROOT / ("scripts/gdb/portable-replay-observer.py" if args.backend == "gdb"
                                        else "src/modern/linux/replay_trace.cpp")
                if sha(observer_path) != identity["observerSha256"] or sha(candidate) != identity["candidateSha256"]:
                    raise ValueError("Portable executable or observer changed during the suite")
                portable_capture.validate_reference(reference, inspect_replay(replay), data, args.target)
                if capture.exists():
                    metadata = json.loads((capture / "metadata.json").read_text())
                    for field, value in dict(executableSha256=identity["candidateSha256"], backend=args.backend,
                                             fastPortableClock=not args.paced,
                                             gameDataSha256=identity["gameDataSha256"], bgmDataSha256=identity["bgmDataSha256"],
                                             targetSha256=identity["targetSha256"]).items():
                        if metadata.get(field) != value:
                            raise ValueError("Completed portable capture identity differs: " + field)
                    if metadata.get("observer", {}).get("sha256") != identity["observerSha256"]:
                        raise ValueError("Completed portable observer identity differs")
                    complete_path = capture / "complete.json"
                    complete = json.loads(complete_path.read_text()) if complete_path.exists() else {}
                    if (not complete.get("ended") or complete.get("errors") or
                            not complete.get("naturalReplayExit")):
                        if args.verify_only:
                            raise ValueError("Completed portable capture is incomplete: " + case_id)
                        attempt = 1
                        while (case / f"attempt-{attempt}").exists():
                            attempt += 1
                        capture.rename(case / f"attempt-{attempt}")
                    elif metadata.get("runnerExitCode") != 0 or not metadata.get("temporaryRuntimeRemoved"):
                        raise ValueError("Completed portable runner failed or retained its runtime")
                if not capture.exists():
                    if args.verify_only:
                        raise ValueError("Completed portable capture is missing: " + case_id)
                    command = [sys.executable, str(ROOT / "scripts/capture-portable-replay.py"), "--candidate", str(candidate),
                               "--target", str(args.target.resolve()), "--game-data-dir", str(data), "--replay", str(replay),
                               "--reference", str(reference), "--output-dir", str(capture), "--backend", args.backend,
                               "--timeout", str(args.timeout)]
                    if args.paced:
                        command.append("--paced")
                    with (case / "candidate.log").open("w") as log:
                        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
                report = comparison.compare(reference, capture, allow_pacing_difference=True)
                comparison.verify_expectation(report, fixture)
                metadata = json.loads((capture / "metadata.json").read_text())
                if metadata["executableSha256"] != identity["candidateSha256"] or metadata["observer"]["sha256"] != identity["observerSha256"]:
                    raise ValueError("Portable executable or observer identity differs")
                if metadata["fixture"] != fixture["sha256"] or metadata["product"] != "portable-linux":
                    raise ValueError("Portable fixture/product identity differs")
                if len(metadata["schema"]["fields"]) != 25:
                    raise ValueError("Portable trace does not contain the expected 25 fields")
                if args.verify_only:
                    if report != json.loads((case / "comparison.json").read_text()):
                        raise ValueError("Saved comparison report changed: " + case_id)
                    if (result.get("comparedFrames") != report["comparedFrames"] or
                            result.get("traceSha256") != report["candidateTraceSha256"]):
                        raise ValueError("Completed suite ledger changed: " + case_id)
                    verified += 1
                    verified_calculations += report["comparedFrames"]
                else:
                    write_json(case / "comparison.json", report)
                    share_equal_trace(reference, capture)
                    result.update(result=report["result"], comparedFrames=report["comparedFrames"],
                                  traceSha256=report["candidateTraceSha256"])
                print(f"  {report['result']}: {report['comparedFrames']} calculations", flush=True)
            except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
                failed = True
                result.update(result="error", error=str(error))
                difference_path = capture / "first-difference.json"
                if difference_path.exists():
                    result.update(result="different", firstDifference=json.loads(difference_path.read_text()))
                print(f"  {result['result']}: {error}", flush=True)
            if not args.verify_only:
                write_json(ledger_path, ledger)
            if failed and not args.keep_going:
                break
    finally:
        if not args.verify_only and wine.exists() and (wine / ".th08-replay-owned").exists():
            shutil.rmtree(wine)
    if args.verify_only:
        print(f"Verified {verified}/{len(fixtures)} portable recordings; "
              f"{verified_calculations} equal calculations", flush=True)
        return 1 if failed or verified != len(fixtures) else 0
    results = [ledger["cases"].get(entry["id"], {}) for entry in fixtures]
    passed = sum(result.get("result") == "equal" for result in results)
    calculations = sum(result.get("comparedFrames", 0) for result in results if result.get("result") == "equal")
    print(f"Passed {passed}/{len(fixtures)} portable recordings; {calculations} equal calculations", flush=True)
    return 1 if failed or passed != len(fixtures) else 0


if __name__ == "__main__":
    raise SystemExit(main())
