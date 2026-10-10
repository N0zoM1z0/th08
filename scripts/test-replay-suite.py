#!/usr/bin/env python3
"""Fetch pinned 1.00d replays and compare native playback with retail, serially.

The ledger is updated after each case. Re-running the same command reuses
completed captures and comparisons whose executable and fixture hashes agree.
"""
import argparse
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import urllib.request

from replay_file import inspect

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("comparison", ROOT / "scripts/compare-replay-traces.py")
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)


def sha(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def write_json(path, value):
    temporary = path.with_suffix(".tmp")
    temporary.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n")
    temporary.replace(path)


def fetch(fixture, directory):
    path = directory / fixture["file"]
    if not path.exists():
        request = urllib.request.Request(fixture["url"], headers={"User-Agent": "TH08-replay-parity/1"})
        with urllib.request.urlopen(request, timeout=60) as response:
            data = response.read(4 * 1024 * 1024 + 1)
        if hashlib.sha256(data).hexdigest() != fixture["sha256"]:
            raise ValueError(f"Downloaded replay hash differs: {fixture['id']}")
        path.write_bytes(data)
    info = inspect(path)
    if info["sha256"] != fixture["sha256"]:
        raise ValueError(f"Cached replay hash differs: {path}")
    for field in ("shot", "difficulty", "extended", "practice", "spellcard", "executable"):
        if info[field] != fixture[field]:
            raise ValueError(f"Replay metadata differs: {fixture['id']} / {field}")
    if info["bytes"] != fixture["bytes"]:
        raise ValueError(f"Replay size differs: {fixture['id']}")
    if [{key: s[key] for key in ("index", "inputRecords", "endScore")} for s in info["stages"]] != [
        {key: s[key] for key in ("index", "inputRecords", "endScore")} for s in fixture["stages"]
    ]:
        raise ValueError(f"Replay stages differ: {fixture['id']}")
    return path


def compress(directory):
    for name in ("rows.jsonl", "gdb.log"):
        path = directory / name
        if not path.exists():
            continue
        with path.open("rb") as source, gzip.open(str(path) + ".gz", "wb", compresslevel=6) as target:
            shutil.copyfileobj(source, target)
        path.unlink()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=ROOT / "config/replay-fixtures.json")
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--candidate", type=Path, default=ROOT / "build/th08.exe")
    parser.add_argument("--map", type=Path, default=ROOT / "build/th08.map")
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--bgm-data", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--fixtures-dir", type=Path, default=ROOT / "build/replay-fixtures")
    parser.add_argument("--wine-prefix", type=Path, help="Reuse a runner-owned prefix for this serial batch")
    parser.add_argument("--reference-dir", type=Path, help="Retail captures from an earlier suite directory")
    parser.add_argument("--cases", help="Comma-separated case IDs; default: every manifest case")
    parser.add_argument("--claims-only", action="store_true", help="Reproduce cases with published retail trace expectations")
    parser.add_argument("--fetch-only", action="store_true")
    parser.add_argument("--clock-rate", type=int, default=1)
    parser.add_argument("--no-rasterization", action="store_true")
    parser.add_argument("--observer-backend", choices=("gdb", "ptrace"), default="gdb")
    parser.add_argument("--timeout", type=int, default=3600, help="Deadline per capture, seconds")
    parser.add_argument("--keep-going", action="store_true", help="Continue other cases after a difference or capture error")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("--output-dir must be under build/")
    output.mkdir(parents=True, exist_ok=True)
    args.fixtures_dir.mkdir(parents=True, exist_ok=True)
    manifest = json.loads(args.manifest.read_text())
    fixtures = manifest["fixtures"]
    if any(not re.fullmatch(r"[a-z0-9-]+", f["id"]) or Path(f["file"]).name != f["file"] for f in fixtures):
        raise ValueError("Unsafe fixture ID/path in manifest")
    if len({f["id"] for f in fixtures}) != len(fixtures):
        raise ValueError("Duplicate replay fixture ID")
    if args.cases:
        selected = args.cases.split(",")
        fixtures = [f for f in fixtures if f["id"] in selected]
        if {f["id"] for f in fixtures} != set(selected):
            parser.error("Unknown replay case ID")
    if args.claims_only:
        fixtures = [f for f in fixtures if "retailTrace" in f]
        if not fixtures:
            parser.error("No published retail trace expectations in the selected cases")
    ledger_path = output / "suite.json"
    ledger = json.loads(ledger_path.read_text()) if ledger_path.exists() else dict(version=1, cases={})
    if not args.fetch_only:
        if sha(args.target) != manifest["targetSha256"]:
            raise ValueError("Retail target hash differs from the manifest")
        identity = dict(candidateSha256=sha(args.candidate), mapSha256=sha(args.map),
                        gameDataSha256=sha(args.game_data), bgmDataSha256=sha(args.bgm_data),
                        targetSha256=sha(args.target), traceSchemaSha256=sha(ROOT / "scripts/replay-external-schema.json"),
                        clockRate=args.clock_rate, rasterization=not args.no_rasterization)
        if ledger.get("identity", identity) != identity:
            raise ValueError("Suite identity changed; use a fresh output directory and --reference-dir for retail captures")
        ledger["identity"] = identity
    failed = False
    for index, fixture in enumerate(fixtures, 1):
        case_id = fixture["id"]
        previous = ledger["cases"].get(case_id, {})
        if previous.get("result") == "equal" and previous.get("fixtureSha256") == fixture["sha256"]:
            reference = ROOT / previous["reference"]
            report = comparison.compare(reference, output / case_id / "candidate")
            comparison.verify_expectation(report, fixture)
            if report["result"] != "equal" or report["comparedFrames"] != previous["comparedFrames"]:
                raise ValueError(f"Completed comparison is missing or differs: {case_id}")
            write_json(output / case_id / "comparison.json", report)
            print(f"[{index}/{len(fixtures)}] {case_id}: verified cached equal", flush=True)
            continue
        print(f"[{index}/{len(fixtures)}] {case_id}", flush=True)
        result = dict(fixtureSha256=fixture["sha256"], shot=fixture["shot"], difficulty=fixture["difficulty"],
                      expectedStages=[s["index"] for s in fixture["stages"]], result="running")
        ledger["cases"][case_id] = result
        write_json(ledger_path, ledger)
        case = output / case_id
        case.mkdir(exist_ok=True)
        try:
            replay = fetch(fixture, args.fixtures_dir)
            if args.fetch_only:
                result["result"] = "fetched"
            else:
                reference = case / "reference"
                if args.reference_dir:
                    cached_reference = args.reference_dir.resolve() / case_id / "reference"
                    if cached_reference.exists():
                        reference = cached_reference
                result["reference"] = str(reference.relative_to(ROOT))
                for product in ("reference", "candidate"):
                    capture = reference if product == "reference" else case / product
                    if capture.exists():
                        try:
                            meta, _ = comparison.load(capture)
                            expected_sha = identity["targetSha256" if product == "reference" else "candidateSha256"]
                            if (meta["fixture"] != fixture["sha256"] or meta["executableSha256"] != expected_sha
                                    or meta["gameDataSha256"] != identity["gameDataSha256"]
                                    or meta.get("clockRate", 1) != args.clock_rate
                                    or meta.get("rasterization", True) == args.no_rasterization):
                                raise ValueError("Cached capture identity differs")
                        except (OSError, ValueError, KeyError):
                            if capture != case / product:
                                raise ValueError(f"Reused retail capture is missing or invalid: {capture}")
                            shutil.rmtree(capture)
                        else:
                            continue
                    command = [sys.executable, str(ROOT / "scripts/capture-replay.py"),
                               "--target", str(args.target.resolve()), "--replay", str(replay.resolve()),
                               "--game-data", str(args.game_data.resolve()), "--bgm-data", str(args.bgm_data.resolve()),
                               "--output-dir", str(capture), "--clock-rate", str(args.clock_rate),
                               "--observer-backend", args.observer_backend,
                               "--timeout", str(args.timeout)]
                    if args.no_rasterization:
                        command.append("--no-rasterization")
                    if args.wine_prefix:
                        command += ["--wine-prefix", str(args.wine_prefix.resolve())]
                    if product == "candidate":
                        command += ["--candidate", str(args.candidate.resolve()), "--map", str(args.map.resolve())]
                        command += ["--reference-trace", str(reference)]
                    with (case / f"{product}.log").open("w") as log:
                        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
                    compress(capture)
                report = comparison.compare(reference, case / "candidate")
                comparison.verify_expectation(report, fixture)
                write_json(case / "comparison.json", report)
                result.update(result=report["result"], comparedFrames=report["comparedFrames"],
                              firstDifference=report["firstDifference"])
                print(f"  {report['result']}: {report['comparedFrames']} calculation frames", flush=True)
        except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
            result.update(result="error", error=str(error))
            difference_path = case / "candidate" / "first-difference.json"
            if difference_path.exists():
                result.update(result="different", firstDifference=json.loads(difference_path.read_text()))
                compress(case / "candidate")
            print(f"  error: {error}", flush=True)
        write_json(ledger_path, ledger)
        if result["result"] not in ("equal", "fetched"):
            failed = True
            if not args.keep_going:
                break
    counts = {state: sum(case["result"] == state for case in ledger["cases"].values())
              for state in ("equal", "different", "error", "running", "fetched")}
    print(json.dumps(dict(cases=len(manifest["fixtures"]), completed=counts)), flush=True)
    return int(failed)


if __name__ == "__main__":
    raise SystemExit(main())
