#!/usr/bin/env python3
"""Compare complete replay traces captured at the same calculation boundary."""
import argparse
import json
import struct
import gzip
import hashlib
from pathlib import Path
from replay_file import valid_input_tail


def trace_sha256(metadata, rows):
    """Hash the schema and little-endian uint32 rows, independent of JSON spacing."""
    digest = hashlib.sha256()
    digest.update(json.dumps(metadata["schema"], sort_keys=True, separators=(",", ":")).encode() + b"\n")
    pack = struct.Struct("<" + "I" * len(metadata["schema"]["fields"])).pack
    for row in rows:
        digest.update(pack(*row))
    return digest.hexdigest()


def verify_expectation(report, fixture):
    expected = fixture.get("retailTrace")
    if expected is None:
        return
    if report["comparedFrames"] != expected["calculationFrames"]:
        raise ValueError("Calculation count differs from the published retail expectation")
    if any(report.get(field) != expected["sha256"] for field in ("referenceTraceSha256", "candidateTraceSha256")):
        raise ValueError("Trace differs from the published retail expectation")


def load(directory):
    metadata = json.loads((directory / "metadata.json").read_text())
    complete = json.loads((directory / "complete.json").read_text())
    trace = directory / "rows.jsonl"
    opener = open
    if not trace.exists():
        trace = directory / "rows.jsonl.gz"
        opener = gzip.open
    with opener(trace, "rt") as stream:
        rows = [json.loads(line) for line in stream]
    version = metadata["schema"].get("version")
    if version not in (2, 4) or (version == 2 and not metadata["schema"].get("demoEndFrames")):
        raise ValueError(f"Capture uses an older schema; recapture it: {directory}")
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
    if version == 4:
        expected = [stage["index"] for stage in metadata["replay"]["stages"] if stage["index"] >= metadata["startStage"]]
        stages = []
        previous = None
        for index, row in enumerate(rows, 1):
            if row[0] != index:
                raise ValueError(f"Missing or out-of-order calculation frames: {directory}")
            if previous is None or row[1] != previous[1]:
                stages.append(row[1])
                if row[2] != 1 or row[3] != 1:
                    raise ValueError(f"Stage did not start at replay frame 1: {directory}")
            elif row[2] not in (previous[2], previous[2] + 1):
                raise ValueError(f"Missing or out-of-order replay input frames: {directory}")
            elif row[3] != previous[3] + 1:
                raise ValueError(f"Missing gameplay calculation frames: {directory}")
            previous = row
        if stages != expected or complete.get("stages") != expected:
            raise ValueError(f"Missing or out-of-order replay stages: {directory}")
        for stage in metadata["replay"]["stages"]:
            if stage["index"] not in expected:
                continue
            captured = [row for row in rows if row[1] == stage["index"]]
            if complete["stageFrames"].get(str(stage["index"])) != captured[-1][2]:
                raise ValueError(f"Stage completion frame differs from trace: {directory}")
            if not valid_input_tail(stage["inputRecords"] - captured[-1][2], stage["index"] == expected[-1]):
                raise ValueError(f"Stage ended before consuming its recorded inputs: {directory}")
            if captured[-1][4] != stage["endScore"] or complete["endScores"].get(str(stage["index"])) != stage["endScore"]:
                raise ValueError(f"Stage end score differs from the original recording: {directory}")
        return metadata, rows
    if complete.get("lastFrame") != rows[-1][0]:
        raise ValueError(f"Completion frame differs from trace: {directory}")
    demo_column = metadata["schema"]["fields"].index("demo")
    indexes = metadata.get("demoIndexes", [0])
    seen = []
    end_frames = {}
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
        end_frames[str(last_demo)] = last_frame
    if seen != indexes or complete.get("completedDemos", indexes) != indexes:
        raise ValueError(f"Missing or out-of-order demos: {directory}")
    expected_ends = metadata["schema"]["demoEndFrames"]
    if any(end_frames[str(demo)] != expected_ends[str(demo)] for demo in indexes):
        raise ValueError(f"Demo ended before its expected terminal frame: {directory}")
    return metadata, rows


def compare(reference, candidate, allow_pacing_difference=False):
    ref_meta, ref = load(reference)
    got_meta, got = load(candidate)
    for field in ("schema", "fixture", "targetSha256", "gameDataSha256", "configSha256", "muted"):
        if ref_meta[field] != got_meta[field]:
            raise ValueError(f"Capture metadata differs: {field}")
    for field, default in (("startStage", None), ("playbackMode", None),
                           ("clockRate", 1), ("rasterization", True)):
        if allow_pacing_difference and field in ("clockRate", "rasterization"):
            continue
        if ref_meta.get(field, default) != got_meta.get(field, default):
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
                referenceTraceSha256=trace_sha256(ref_meta, ref), candidateTraceSha256=trace_sha256(got_meta, got),
                firstDifference=first, differingFramesByField=counts,
                floatComparison="exact IEEE-754 bits; no tolerance or ignored fields")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("--allow-pacing-difference", action="store_true",
                        help="Validate an accelerated capture against ordinary pacing")
    args = parser.parse_args()
    try:
        result = compare(args.reference, args.candidate, args.allow_pacing_difference)
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
