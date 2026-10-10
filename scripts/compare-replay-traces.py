#!/usr/bin/env python3
"""Compare complete replay traces captured at the same calculation boundary."""
import argparse
import json
import struct
from pathlib import Path


def load(directory):
    metadata = json.loads((directory / "metadata.json").read_text())
    complete = json.loads((directory / "complete.json").read_text())
    rows = [json.loads(line) for line in (directory / "rows.jsonl").read_text().splitlines()]
    if not complete.get("ended") or complete.get("errors"):
        raise ValueError(f"Incomplete or failed capture: {directory}")
    if not rows or complete["rows"] != len(rows):
        raise ValueError(f"Missing trace rows: {directory}")
    if not metadata.get("muted"):
        raise ValueError(f"Capture was not muted: {directory}")
    if any(not isinstance(row, list) or any(type(value) is not int or not 0 <= value <= 0xffffffff
                                           for value in row) for row in rows):
        raise ValueError(f"Invalid trace values: {directory}")
    if any(len(row) != len(metadata["schema"]["fields"]) for row in rows):
        raise ValueError(f"Invalid row width: {directory}")
    if complete.get("lastFrame") != rows[-1][0]:
        raise ValueError(f"Completion frame differs from trace: {directory}")
    demo_column = metadata["schema"]["fields"].index("demo")
    indexes = metadata.get("demoIndexes", [0])
    seen = []
    last_demo = None
    last_frame = 0
    for row in rows:
        if row[demo_column] != last_demo:
            seen.append(row[demo_column])
            last_demo = row[demo_column]
            last_frame = 0
        if row[0] != last_frame + 1:
            raise ValueError(f"Missing, duplicated, or out-of-order frames: {directory}")
        last_frame = row[0]
    if seen != indexes or complete.get("completedDemos", indexes) != indexes:
        raise ValueError(f"Missing or out-of-order demos: {directory}")
    return metadata, rows


def compare(reference, candidate):
    ref_meta, ref = load(reference)
    got_meta, got = load(candidate)
    for field in ("schema", "fixture", "targetSha256", "gameDataSha256", "configSha256", "muted"):
        if ref_meta[field] != got_meta[field]:
            raise ValueError(f"Capture metadata differs: {field}")
    fields = ref_meta["schema"]["fields"]
    def decoded(row):
        result = dict(zip(fields, row))
        for field in ref_meta["schema"]["floatFields"]:
            result[field] = struct.unpack("<f", struct.pack("<I", result[field]))[0]
        if result["gauge"] & 0x80000000:
            result["gauge"] -= 0x100000000
        return result
    counts = dict.fromkeys(fields, 0)
    first = None
    for index, (expected, observed) in enumerate(zip(ref, got)):
        differences = {field: [expected[col], observed[col]] for col, field in enumerate(fields)
                       if expected[col] != observed[col]}
        if differences and first is None:
            expected_values, observed_values = decoded(expected), decoded(observed)
            first = dict(frame=expected[0], differingFields=list(differences), rawBits=differences,
                         reference=expected_values, candidate=observed_values,
                         referenceContext=[decoded(row) for row in ref[max(0, index - 2):index + 3]],
                         candidateContext=[decoded(row) for row in got[max(0, index - 2):index + 3]])
        for field in differences:
            counts[field] += 1
    return dict(result="equal" if first is None and len(ref) == len(got) else "different",
                comparedFrames=min(len(ref), len(got)), referenceFrames=len(ref), candidateFrames=len(got),
                firstDifference=first, differingFramesByField=counts,
                floatComparison="exact IEEE-754 bits; no tolerance or ignored fields")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path)
    parser.add_argument("candidate", type=Path)
    args = parser.parse_args()
    try:
        result = compare(args.reference, args.candidate)
    except (ValueError, OSError, KeyError, json.JSONDecodeError) as error:
        result = dict(result="incomplete", error=str(error))
    if args.candidate.is_dir():
        (args.candidate / "comparison.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({key: value for key, value in result.items() if key != "firstDifference"}))
    if result.get("firstDifference"):
        print(json.dumps(result["firstDifference"]))
    return 0 if result["result"] == "equal" else 1


if __name__ == "__main__":
    raise SystemExit(main())
