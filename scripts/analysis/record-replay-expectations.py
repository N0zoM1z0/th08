#!/usr/bin/env python3
"""Verify retained replay evidence and add reviewed expectations to the fixture manifest.

Existing expectations must agree. Raw captures and reports remain under build/.
"""
import argparse
import importlib.util
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
spec = importlib.util.spec_from_file_location("comparison", ROOT / "scripts/compare-replay-traces.py")
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--suite", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, default=ROOT / "config/replay-fixtures.json")
    parser.add_argument("--cases", help="Comma-separated case IDs; default: every completed equal case")
    parser.add_argument("--write", action="store_true", help="Add newly verified expectations; default: verify only")
    args = parser.parse_args()
    suite = args.suite.resolve()
    if not suite.is_relative_to(ROOT / "build"):
        parser.error("--suite must be under build/")
    ledger = json.loads((suite / "suite.json").read_text())
    manifest = json.loads(args.manifest.read_text())
    if ledger["identity"]["targetSha256"] != manifest["targetSha256"]:
        raise ValueError("Suite target differs from the fixture manifest")
    fixtures = {f["id"]: f for f in manifest["fixtures"]}
    selected = args.cases.split(",") if args.cases else [k for k, v in ledger["cases"].items() if v["result"] == "equal"]
    if not selected:
        parser.error("No completed equal cases")
    added = []
    calculations = 0
    for case in selected:
        fixture = fixtures[case]
        entry = ledger["cases"][case]
        if entry["result"] != "equal" or entry["fixtureSha256"] != fixture["sha256"]:
            raise ValueError(f"Case has no matching completed result: {case}")
        reference = ROOT / entry["reference"]
        candidate = suite / case / "candidate"
        report = comparison.compare(reference, candidate)
        if report["result"] != "equal" or report["comparedFrames"] != entry["comparedFrames"]:
            raise ValueError(f"Retained traces differ from the suite result: {case}")
        comparison.verify_expectation(report, fixture)
        ref = json.loads((reference / "metadata.json").read_text())
        got = json.loads((candidate / "metadata.json").read_text())
        if (ref["product"] != "retail" or ref["executableSha256"] != manifest["targetSha256"]
                or got["executableSha256"] != ledger["identity"]["candidateSha256"]
                or ref["fixture"] != fixture["sha256"] or ref["gameDataSha256"] != ledger["identity"]["gameDataSha256"]
                or got["mapSha256"] != ledger["identity"]["mapSha256"]):
            raise ValueError(f"Capture identity differs from the recorded checkpoint: {case}")
        expected = dict(schemaVersion=ref["schema"]["version"], calculationFrames=report["comparedFrames"],
                        sha256=report["referenceTraceSha256"])
        if "retailTrace" not in fixture:
            added.append(case)
        fixture["retailTrace"] = expected
        calculations += report["comparedFrames"]
        print(f"{case}: verified {report['comparedFrames']} calculations, {expected['sha256']}", flush=True)
    if args.write:
        temporary = args.manifest.with_suffix(".tmp")
        temporary.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")
        temporary.replace(args.manifest)
    print(json.dumps(dict(verifiedCases=len(selected), calculations=calculations, newExpectations=added, written=args.write)))


if __name__ == "__main__":
    raise SystemExit(main())
