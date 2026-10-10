#!/usr/bin/env python3
"""Guard against false passes when replay evidence is missing or mismatched."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("compare", Path(__file__).with_name("compare-replay-traces.py"))
compare = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compare)


class ComparisonTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.reference = Path(self.temp.name) / "reference"
        self.candidate = Path(self.temp.name) / "candidate"
        schema = json.loads(Path(__file__).with_name("replay-schema.json").read_text())
        schema["demoEndFrames"] = {"0": 3, "1": 3}
        self.metadata = dict(schema=schema, fixture="demo/demorpy0.rpy", gameDataSha256="data",
                             targetSha256="target", configSha256="config", muted=True)
        self.rows = [[frame] + [0] * (len(schema["fields"]) - 1) for frame in range(1, 4)]
        self.write(self.reference)
        self.write(self.candidate)

    def write(self, directory, rows=None, ended=True, metadata=None):
        directory.mkdir(exist_ok=True)
        rows = self.rows if rows is None else rows
        (directory / "metadata.json").write_text(json.dumps(metadata or self.metadata))
        (directory / "complete.json").write_text(json.dumps(dict(rows=len(rows), lastFrame=rows[-1][0], ended=ended)))
        (directory / "rows.jsonl").write_text("\n".join(map(json.dumps, rows)) + "\n")

    def test_equal_complete_capture(self):
        self.assertEqual(compare.compare(self.reference, self.candidate)["result"], "equal")

    def test_one_float_bit_is_reported(self):
        rows = [row[:] for row in self.rows]
        rows[1][self.metadata["schema"]["fields"].index("x")] = 1
        self.write(self.candidate, rows=rows)
        result = compare.compare(self.reference, self.candidate)
        self.assertEqual(result["firstDifference"]["frame"], 2)
        self.assertEqual(result["firstDifference"]["differingFields"], ["x"])
        self.assertNotEqual(result["referenceTraceSha256"], result["candidateTraceSha256"])

    def test_trace_digest_ignores_json_whitespace(self):
        (self.candidate / "rows.jsonl").write_text("\n".join(json.dumps(row, separators=(",", ":")) for row in self.rows))
        result = compare.compare(self.reference, self.candidate)
        self.assertEqual(result["referenceTraceSha256"], result["candidateTraceSha256"])

    def test_published_expectation_rejects_equal_but_changed_traces(self):
        result = compare.compare(self.reference, self.candidate)
        fixture = dict(retailTrace=dict(calculationFrames=3, sha256=result["referenceTraceSha256"]))
        compare.verify_expectation(result, fixture)
        self.rows[1][self.metadata["schema"]["fields"].index("score")] += 1
        self.write(self.reference)
        self.write(self.candidate)
        changed = compare.compare(self.reference, self.candidate)
        self.assertEqual(changed["result"], "equal")
        with self.assertRaisesRegex(ValueError, "published retail expectation"):
            compare.verify_expectation(changed, fixture)

    def test_missing_or_duplicate_frame_is_rejected(self):
        for rows in (self.rows[1:], [self.rows[0], self.rows[2]], [self.rows[0]] * 3):
            with self.subTest(rows=rows):
                self.write(self.candidate, rows=rows)
                with self.assertRaises(ValueError):
                    compare.compare(self.reference, self.candidate)

    def test_timeout_is_rejected(self):
        self.write(self.candidate, ended=False)
        with self.assertRaises(ValueError):
            compare.compare(self.reference, self.candidate)

    def test_mismatched_data_is_rejected(self):
        self.write(self.candidate, metadata={**self.metadata, "gameDataSha256": "other"})
        with self.assertRaises(ValueError):
            compare.compare(self.reference, self.candidate)

    def test_different_duration_cannot_pass(self):
        self.write(self.candidate, rows=self.rows[:2])
        with self.assertRaisesRegex(ValueError, "terminal frame"):
            compare.compare(self.reference, self.candidate)

    def test_identically_truncated_captures_cannot_pass(self):
        self.write(self.reference, rows=self.rows[:2])
        self.write(self.candidate, rows=self.rows[:2])
        with self.assertRaisesRegex(ValueError, "terminal frame"):
            compare.compare(self.reference, self.candidate)

    def test_older_schema_is_rejected(self):
        metadata = {**self.metadata, "schema": {**self.metadata["schema"], "version": 1}}
        self.write(self.reference, metadata=metadata)
        self.write(self.candidate, metadata=metadata)
        with self.assertRaisesRegex(ValueError, "older schema"):
            compare.compare(self.reference, self.candidate)

    def test_wrong_completion_frame_is_rejected(self):
        (self.candidate / "complete.json").write_text(json.dumps(dict(rows=3, lastFrame=2, ended=True)))
        with self.assertRaises(ValueError):
            compare.compare(self.reference, self.candidate)

    def test_unmuted_capture_is_rejected(self):
        self.write(self.candidate, metadata={**self.metadata, "muted": False})
        with self.assertRaises(ValueError):
            compare.compare(self.reference, self.candidate)

    def test_multiple_demos_have_separate_frame_sequences(self):
        rows = [row[:] for row in self.rows] + [row[:] for row in self.rows]
        demo_column = self.metadata["schema"]["fields"].index("demo")
        for row in rows[3:]:
            row[demo_column] = 1
        metadata = {**self.metadata, "demoIndexes": [0, 1],
                    "fixture": ["demo/demorpy0.rpy", "demo/demorpy1.rpy"]}
        self.write(self.reference, rows=rows, metadata=metadata)
        self.write(self.candidate, rows=rows, metadata=metadata)
        self.assertEqual(compare.compare(self.reference, self.candidate)["result"], "equal")
        self.write(self.candidate, rows=rows[:3], metadata=metadata)
        with self.assertRaisesRegex(ValueError, "Missing or out-of-order demos"):
            compare.compare(self.reference, self.candidate)


class ExternalComparisonTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.reference = Path(self.temp.name) / "reference"
        self.candidate = Path(self.temp.name) / "candidate"
        schema = json.loads(Path(__file__).with_name("replay-external-schema.json").read_text())
        self.metadata = dict(schema=schema, fixture="pinned replay", gameDataSha256="data",
                             targetSha256="target", configSha256="config", muted=True, startStage=0,
                             replay=dict(stages=[dict(index=0, inputRecords=5, endScore=99),
                                                 dict(index=1, inputRecords=10, endScore=199)]))
        self.rows = []
        for stage, frames, score in ((0, 2, 99), (1, 3, 199)):
            for frame in range(1, frames + 1):
                self.rows.append([len(self.rows) + 1, stage, frame, frame, score] + [0] * (len(schema["fields"]) - 5))
        self.complete = dict(rows=5, ended=True, stages=[0, 1], stageFrames={"0": 2, "1": 3},
                             endScores={"0": 99, "1": 199})
        self.write(self.reference)
        self.write(self.candidate)

    def write(self, directory):
        directory.mkdir(exist_ok=True)
        (directory / "metadata.json").write_text(json.dumps(self.metadata))
        (directory / "complete.json").write_text(json.dumps(self.complete))
        (directory / "rows.jsonl").write_text("\n".join(map(json.dumps, self.rows)) + "\n")

    def test_complete_cross_stage_trace(self):
        self.assertEqual(compare.compare(self.reference, self.candidate)["result"], "equal")

    def test_missing_stage(self):
        self.rows = self.rows[:2]
        self.complete.update(rows=2, stages=[0])
        self.write(self.candidate)
        with self.assertRaisesRegex(ValueError, "stages"):
            compare.compare(self.reference, self.candidate)

    def test_premature_end_with_matching_scores(self):
        self.rows = self.rows[:3]
        self.complete.update(rows=3, stageFrames={"0": 2, "1": 1})
        self.write(self.candidate)
        with self.assertRaisesRegex(ValueError, "recorded inputs"):
            compare.compare(self.reference, self.candidate)

    def test_second_retail_trailer_length(self):
        for stage in self.metadata["replay"]["stages"]:
            stage["inputRecords"] -= 1
        self.write(self.reference)
        self.write(self.candidate)
        self.assertEqual(compare.compare(self.reference, self.candidate)["result"], "equal")

    def test_missing_last_row_in_shorter_trailer_cannot_match_retail(self):
        for stage in self.metadata["replay"]["stages"]:
            stage["inputRecords"] -= 1
        self.write(self.reference)
        self.rows.pop()
        self.complete.update(rows=4, stageFrames={"0": 2, "1": 2})
        self.write(self.candidate)
        self.assertEqual(compare.compare(self.reference, self.candidate)["result"], "different")

    def test_frozen_input_does_not_hide_missing_calc(self):
        self.rows[1][2] = 1
        self.rows[1][3] = 3
        self.write(self.candidate)
        with self.assertRaisesRegex(ValueError, "calculation frames"):
            compare.compare(self.reference, self.candidate)

    def test_end_score_must_match_recording(self):
        self.rows[-1][4] = 200
        self.write(self.candidate)
        with self.assertRaisesRegex(ValueError, "end score"):
            compare.compare(self.reference, self.candidate)

    def test_pacing_difference_requires_explicit_validation(self):
        self.metadata["clockRate"] = 128
        self.write(self.candidate)
        with self.assertRaisesRegex(ValueError, "clockRate"):
            compare.compare(self.reference, self.candidate)
        self.assertEqual(compare.compare(self.reference, self.candidate, allow_pacing_difference=True)["result"], "equal")


if __name__ == "__main__":
    unittest.main()
