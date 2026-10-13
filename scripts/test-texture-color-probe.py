#!/usr/bin/env python3
"""Require complete native/portable color-probe coverage before reporting differences."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("probe", Path(__file__).parent / "analysis/probe-texture-colors.py")
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class ProbeCoverageTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.path = Path(temporary.name) / "probe.stdout"
        self.rows = []
        for kind, count in ((0, 32), (1, 64), (2, 16), (3, 32)):
            for value in range(count):
                self.rows.extend([dict(kind=kind, value=value, rgb=0), dict(kind=kind + 10, value=value, rgb=0)])
        self.rows.extend(dict(kind=4, value=value, rgb=0) for value in (1, 17, 31, 64, 127, 128, 191, 254, 255))

    def write(self):
        self.path.write_text("Wine initialization message\n" + json.dumps(self.rows, indent=2) +
                             "\nX connection closed\n")

    def test_complete_samples_survive_wine_wrapper_output(self):
        self.write()
        self.assertEqual(len(probe.decode(self.path)), 297)

    def test_missing_sample_is_rejected(self):
        self.rows.pop(7)
        self.write()
        with self.assertRaisesRegex(ValueError, "coverage differs"):
            probe.decode(self.path)

    def test_duplicated_sample_cannot_replace_a_missing_one(self):
        self.rows[8] = self.rows[6]
        self.write()
        with self.assertRaisesRegex(ValueError, "coverage differs"):
            probe.decode(self.path)

    def test_failed_launch_output_is_rejected(self):
        self.path.write_text("Unable to launch color probe\n")
        with self.assertRaisesRegex(ValueError, "JSON is missing"):
            probe.decode(self.path)


if __name__ == "__main__":
    unittest.main()
