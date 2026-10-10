#!/usr/bin/env python3
"""Validate the full replay corpus and the shape of published expectations."""
import argparse
import json
from pathlib import Path
import re
import tomllib
from replay_file import RETAIL_IDENTITY, SHOTS, DIFFICULTIES

ROOT = Path(__file__).resolve().parents[1]


def validate(manifest):
    target = tomllib.loads((ROOT / "config/target.toml").read_text())["target"]
    if manifest["version"] != 1 or manifest["targetSha256"] != target["sha256"]:
        raise ValueError("Replay manifest target or version differs")
    expected = {(shot, difficulty, final) for shot in range(12) for difficulty in range(4) for final in (6, 7)}
    expected.update((shot, 4, 8) for shot in range(12))
    seen = set()
    ids = set()
    hashes = set()
    accepted = 0
    for fixture in manifest["fixtures"]:
        case = fixture["id"]
        shot, difficulty = fixture["shot"], fixture["difficulty"]
        stages = [s["index"] for s in fixture["stages"]]
        if not stages or (shot, difficulty, stages[-1]) not in expected:
            raise ValueError(f"Unknown replay coverage cell: {case}")
        cell = shot, difficulty, stages[-1]
        if cell in seen or case in ids or fixture["sha256"] in hashes:
            raise ValueError(f"Duplicate replay cell, ID, or recording: {case}")
        seen.add(cell)
        ids.add(case)
        hashes.add(fixture["sha256"])
        stage4 = 4 if shot in (0, 3, 4, 5, 10, 11) else 3
        route = [8] if difficulty == 4 else [0, 1, 2, stage4, 5, stages[-1]]
        if stages != route:
            raise ValueError(f"Missing stage or wrong Stage 4 route: {case}")
        if fixture["shotName"] != SHOTS[shot] or fixture["difficultyName"] != DIFFICULTIES[difficulty]:
            raise ValueError(f"Shot or difficulty label differs: {case}")
        identity = fixture["executable"]
        if (identity["size"], identity["checksum"], identity["version"]) != RETAIL_IDENTITY:
            raise ValueError(f"Recording executable identity differs: {case}")
        if fixture["extended"] or fixture["practice"] or fixture["spellcard"] != -1:
            raise ValueError(f"Expected a standard full-run recording: {case}")
        if not re.fullmatch(r"[a-z0-9-]+", case) or Path(fixture["file"]).name != fixture["file"]:
            raise ValueError(f"Unsafe replay ID or filename: {case}")
        if not re.fullmatch(r"[0-9a-f]{64}", fixture["sha256"]) or fixture["bytes"] < 0x68:
            raise ValueError(f"Invalid recording hash or size: {case}")
        if not fixture.get("recordedBy") or not fixture["url"].startswith("https://"):
            raise ValueError(f"Missing recording provenance: {case}")
        for stage in fixture["stages"]:
            if type(stage["inputRecords"]) is not int or stage["inputRecords"] < 1 or not 0 <= stage["endScore"] <= 0xffffffff:
                raise ValueError(f"Invalid stage input count or score: {case}")
        if "retailTrace" in fixture:
            trace = fixture["retailTrace"]
            if (trace["schemaVersion"] != 4 or type(trace["calculationFrames"]) is not int
                    or trace["calculationFrames"] < 1 or not re.fullmatch(r"[0-9a-f]{64}", trace["sha256"])):
                raise ValueError(f"Invalid published trace expectation: {case}")
            accepted += 1
    if seen != expected:
        raise ValueError(f"Replay corpus is missing coverage cells: {sorted(expected - seen)}")
    return accepted


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", nargs="?", type=Path, default=ROOT / "config/replay-fixtures.json")
    args = parser.parse_args()
    accepted = validate(json.loads(args.manifest.read_text()))
    print(f"Replay corpus verified: 108 fixtures, 12 shots, both Final routes on four difficulties, Extra; {accepted} published expectations")


if __name__ == "__main__":
    raise SystemExit(main())
