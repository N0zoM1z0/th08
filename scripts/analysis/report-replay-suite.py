#!/usr/bin/env python3
"""Show recorded replay-suite results and the latest capture progress."""
import argparse
from collections import Counter
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def progress(suite, case):
    logs = [p for p in (suite / case / "reference.log", suite / case / "candidate.log") if p.exists()]
    if not logs:
        return "capture starting"
    log = max(logs, key=lambda p: p.stat().st_mtime_ns)
    lines = log.read_text().splitlines()
    last = lines[-1] if lines else ""
    if last.startswith("Captured "):
        return f"{log.stem}: {last}"
    try:
        completion = json.loads(last)
    except (ValueError, TypeError):
        return f"{log.stem}: see {log.relative_to(ROOT)}" if last else f"{log.stem}: capture starting"
    return f"{log.stem}: captured {completion['rows']} calculation frames"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--suite", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, default=ROOT / "config/replay-fixtures.json")
    args = parser.parse_args()
    suite = args.suite.resolve()
    if not suite.is_relative_to(ROOT / "build"):
        parser.error("--suite must be under build/")
    fixtures = {f["id"] for f in json.loads(args.manifest.read_text())["fixtures"]}
    cases = json.loads((suite / "suite.json").read_text())["cases"]
    if set(cases) - fixtures:
        parser.error("Suite contains cases absent from the manifest")
    counts = Counter(c["result"] for c in cases.values())
    calculations = sum(c["comparedFrames"] for c in cases.values() if c["result"] == "equal")
    print(f"Recorded results: {counts['equal']}/{len(fixtures)} equal; {counts['different']} different, "
          f"{counts['error']} errors, {counts['running']} running, {counts['fetched']} fetched")
    print(f"Compared calculations in equal cases: {calculations}")
    for case, result in cases.items():
        if result["result"] == "running":
            print(f"Running: {case}; {progress(suite, case)}")
        elif result["result"] in ("different", "error"):
            print(f"Review: {case}; {result['result']}; see {(suite / case).relative_to(ROOT)}")


if __name__ == "__main__":
    raise SystemExit(main())
