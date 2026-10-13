#!/usr/bin/env python3
"""Locate the first effect allocation difference in two replay diagnostics."""
import argparse
import gzip
import json
from pathlib import Path

TARGET_SHA = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"


def load(directory):
    records = {}
    with gzip.open(directory / "effect-pool.jsonl.gz", "rt") as source:
        for line in source:
            record = json.loads(line)
            if len(bytes.fromhex(record["slots"])) != 1024:
                raise ValueError("Expected 512 effect slots with active/ID bytes")
            if record["frame"] < 1:
                continue
            key = record["stage"], record["frame"]
            if key in records and records[key] != record:
                raise ValueError("Conflicting effect pool records at the same stage/frame")
            records[key] = record
    return records


def compare(reference, candidate):
    metadata = [json.loads((directory / "metadata.json").read_text()) for directory in (reference, candidate)]
    if any(value.get("targetSha256") != TARGET_SHA for value in metadata):
        raise ValueError("Diagnostics must identify the canonical Japanese TH08 1.00d target")
    if any(value.get("muted") is not True for value in metadata):
        raise ValueError("Effect pool diagnostics require muted playback")
    for field in ("fixture", "gameDataSha256", "bgmDataSha256", "playbackMode"):
        if field not in metadata[0] or metadata[0][field] != metadata[1].get(field):
            raise ValueError("Diagnostic identity differs: " + field)
    left, right = load(reference), load(candidate)
    shared = sorted(left.keys() & right.keys())
    if not shared:
        raise ValueError("Diagnostics have no shared positive input frames")
    report = dict(result="equal-prefix", sharedFrames=len(shared), referenceFrames=len(left), candidateFrames=len(right))
    for stage, frame in shared:
        a, b = left[stage, frame], right[stage, frame]
        if a != b:
            fields = [name for name in ("nextIndex", "activeCount", "slots") if a[name] != b[name]]
            native, portable = bytes.fromhex(a["slots"]), bytes.fromhex(b["slots"])
            slots = [dict(slot=index, reference=list(native[index * 2:index * 2 + 2]),
                          candidate=list(portable[index * 2:index * 2 + 2]))
                     for index in range(512) if native[index * 2:index * 2 + 2] != portable[index * 2:index * 2 + 2]]
            report.update(result="different", firstDifference=dict(stage=stage, frame=frame, fields=fields,
                reference={name: a[name] for name in ("nextIndex", "activeCount")},
                candidate={name: b[name] for name in ("nextIndex", "activeCount")}, slots=slots))
            break
    if report["result"] == "equal-prefix" and left.keys() != right.keys():
        report["result"] = "coverage-diff"
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    args = parser.parse_args()
    report = compare(args.reference, args.candidate)
    print(json.dumps(report, indent=2))
    return 0 if report["result"] == "equal-prefix" else 1


if __name__ == "__main__":
    raise SystemExit(main())
