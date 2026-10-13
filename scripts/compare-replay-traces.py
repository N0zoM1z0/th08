#!/usr/bin/env python3
"""Compare complete replay traces captured at the same calculation boundary."""
import argparse
import json
import struct
import gzip
import hashlib
from collections import deque
from itertools import zip_longest
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


def verify_suite_capture(metadata, identity, fixture_sha, candidate=False):
    """Bind retained captures to the suite's executable, assets, and playback settings."""
    expected = {field: identity[field] for field in
                ("targetSha256", "gameDataSha256", "bgmDataSha256", "clockRate", "rasterization")}
    expected.update(fixture=fixture_sha, product="native-vc7-bugfix" if candidate else "retail",
                    executableSha256=identity["candidateSha256" if candidate else "targetSha256"])
    if candidate:
        expected["mapSha256"] = identity["mapSha256"]
    schema = Path(__file__).with_name("replay-external-schema.json").read_bytes()
    if hashlib.sha256(schema).hexdigest() != identity["traceSchemaSha256"]:
        raise ValueError("Suite schema differs from the current observer schema")
    expected["schema"] = json.loads(schema)
    for field, value in expected.items():
        if metadata.get(field) != value:
            raise ValueError(f"Capture identity differs from the suite: {field}")


class Capture:
    """Validate and hash rows while retaining only the current stage boundary."""
    def __init__(self, directory):
        self.directory = directory
        self.metadata = json.loads((directory / "metadata.json").read_text())
        self.complete = json.loads((directory / "complete.json").read_text())
        self.version = self.metadata["schema"].get("version")
        if self.version not in (2, 4) or (self.version == 2 and not self.metadata["schema"].get("demoEndFrames")):
            raise ValueError(f"Capture uses an older schema; recapture it: {directory}")
        if not self.complete.get("ended") or self.complete.get("errors"):
            raise ValueError(f"Incomplete or failed capture: {directory}")
        if not self.metadata.get("muted"):
            raise ValueError(f"Capture was not muted: {directory}")
        self.count = 0
        self.digest = hashlib.sha256()
        self.digest.update(json.dumps(self.metadata["schema"], sort_keys=True, separators=(",", ":")).encode() + b"\n")
        self.pack = struct.Struct("<" + "I" * len(self.metadata["schema"]["fields"])).pack

    def check_stage_end(self, row, stages):
        stage = stages[row[1]]
        key = str(row[1])
        if self.complete["stageFrames"].get(key) != row[2]:
            raise ValueError(f"Stage completion frame differs from trace: {self.directory}")
        if not valid_input_tail(stage["inputRecords"] - row[2], row[1] == list(stages)[-1]):
            raise ValueError(f"Stage ended before consuming its recorded inputs: {self.directory}")
        if row[4] != stage["endScore"] or self.complete["endScores"].get(key) != stage["endScore"]:
            raise ValueError(f"Stage end score differs from the original recording: {self.directory}")

    def rows(self):
        directory, metadata, complete = self.directory, self.metadata, self.complete
        trace = directory / "rows.jsonl"
        opener = open
        if not trace.exists():
            trace, opener = directory / "rows.jsonl.gz", gzip.open
        if self.version == 4:
            selected = [stage for stage in metadata["replay"]["stages"] if stage["index"] >= metadata["startStage"]]
            expected = [stage["index"] for stage in selected]
            stages = {stage["index"]: stage for stage in selected}
            if len(stages) != len(expected):
                raise ValueError(f"Missing or out-of-order replay stages: {directory}")
        else:
            expected = metadata.get("demoIndexes", [0])
            demo_column = metadata["schema"]["fields"].index("demo")
        seen, end_frames, previous = [], {}, None
        with opener(trace, "rt") as stream:
            for index, line in enumerate(stream, 1):
                row = json.loads(line)
                if not isinstance(row, list) or any(type(value) is not int or not 0 <= value <= 0xffffffff for value in row):
                    raise ValueError(f"Invalid trace values: {directory}")
                if len(row) != len(metadata["schema"]["fields"]):
                    raise ValueError(f"Invalid row width: {directory}")
                if self.version == 4:
                    if row[0] != index:
                        raise ValueError(f"Missing or out-of-order calculation frames: {directory}")
                    if previous is None or row[1] != previous[1]:
                        if len(seen) >= len(expected) or row[1] != expected[len(seen)]:
                            raise ValueError(f"Missing or out-of-order replay stages: {directory}")
                        if previous is not None:
                            self.check_stage_end(previous, stages)
                        seen.append(row[1])
                        if row[2] != 1 or row[3] != 1:
                            raise ValueError(f"Stage did not start at replay frame 1: {directory}")
                    elif row[2] not in (previous[2], previous[2] + 1):
                        raise ValueError(f"Missing or out-of-order replay input frames: {directory}")
                    elif row[3] != previous[3] + 1:
                        raise ValueError(f"Missing gameplay calculation frames: {directory}")
                else:
                    new_demo = previous is None or row[demo_column] != previous[demo_column]
                    if new_demo:
                        if len(seen) >= len(expected) or row[demo_column] != expected[len(seen)]:
                            raise ValueError(f"Missing or out-of-order demos: {directory}")
                        seen.append(row[demo_column])
                    if row[0] != (1 if new_demo else previous[0] + 1):
                        raise ValueError(f"Missing, duplicated, or out-of-order frames: {directory}")
                    end_frames[str(row[demo_column])] = row[0]
                self.count = index
                self.digest.update(self.pack(*row))
                previous = row
                yield row
        if not self.count or complete["rows"] != self.count:
            raise ValueError(f"Missing trace rows: {directory}")
        if self.version == 4:
            if seen != expected or complete.get("stages") != expected:
                raise ValueError(f"Missing or out-of-order replay stages: {directory}")
            self.check_stage_end(previous, stages)
        else:
            if complete.get("lastFrame") != previous[0]:
                raise ValueError(f"Completion frame differs from trace: {directory}")
            if seen != expected or complete.get("completedDemos", expected) != expected:
                raise ValueError(f"Missing or out-of-order demos: {directory}")
            if any(end_frames[str(demo)] != metadata["schema"]["demoEndFrames"][str(demo)] for demo in expected):
                raise ValueError(f"Demo ended before its expected terminal frame: {directory}")


def summarize(directory):
    capture = Capture(directory)
    for _ in capture.rows():
        pass
    return capture.metadata, capture.count, capture.digest.hexdigest()


def load(directory):
    """Compatibility helper for callers that need the actual row list."""
    capture = Capture(directory)
    return capture.metadata, list(capture.rows())


def compare(reference, candidate, allow_pacing_difference=False):
    ref, got = Capture(reference), Capture(candidate)
    ref_meta, got_meta = ref.metadata, got.metadata
    for field in ("schema", "fixture", "targetSha256", "gameDataSha256", "bgmDataSha256", "configSha256", "muted"):
        if field not in ref_meta or field not in got_meta:
            raise ValueError(f"Capture metadata is missing {field}; recapture it")
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
    first_index = None
    ref_context, got_context = deque(maxlen=2), deque(maxlen=2)
    for index, (expected, observed) in enumerate(zip_longest(ref.rows(), got.rows())):
        if first is not None and index <= first_index + 2:
            if expected is not None:
                first["referenceContext"].append(decoded(expected))
            if observed is not None:
                first["candidateContext"].append(decoded(observed))
        if expected is None or observed is None:
            continue
        differences = {field: [expected[col], observed[col]] for col, field in enumerate(fields)
                       if expected[col] != observed[col]}
        if differences and first is None:
            first_index = index
            expected_values, observed_values = decoded(expected), decoded(observed)
            first = dict(frame=expected[0], differingFields=list(differences), rawBits=differences,
                         reference=expected_values, candidate=observed_values,
                         referenceContext=[decoded(row) for row in ref_context] + [expected_values],
                         candidateContext=[decoded(row) for row in got_context] + [observed_values])
        for field in differences:
            counts[field] += 1
        ref_context.append(expected)
        got_context.append(observed)
    return dict(result="equal" if first is None and ref.count == got.count else "different",
                comparedFrames=min(ref.count, got.count), referenceFrames=ref.count, candidateFrames=got.count,
                referenceTraceSha256=ref.digest.hexdigest(), candidateTraceSha256=got.digest.hexdigest(),
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
