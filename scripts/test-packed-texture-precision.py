#!/usr/bin/env python3
"""Check complete packed-precision coverage and tail-sample error detection."""
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("packed_precision", Path(__file__).parent / "analysis/probe-packed-precision.py")
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class PackedCoverageTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.reference, self.candidate = [Path(temporary.name) / name for name in ("reference", "candidate")]
        for path in (self.reference, self.candidate):
            with path.open("wb") as stream:
                stream.truncate(probe.SAMPLES * 4)

    def test_last_pixel_of_last_case_is_compared(self):
        with self.candidate.open("r+b") as stream:
            stream.seek(probe.SAMPLES * 4 - 4)
            stream.write(struct.pack("<I", 0x000201))
        report = probe.compare(self.reference, self.candidate)
        self.assertEqual(report["differentPixels"], 1)
        self.assertEqual(report["maximumChannelError"], 2)
        last = report["cases"][-1]
        self.assertEqual((last["material"], last["blend"], last["draws"]), (3, 2, 4))
        self.assertEqual(last["examples"][0]["sampleIndex"], "ffff")

    def test_truncated_case_is_rejected(self):
        with self.candidate.open("r+b") as stream:
            stream.truncate(probe.SAMPLES * 4 - 1)
        with self.assertRaisesRegex(ValueError, "coverage"):
            probe.compare(self.reference, self.candidate)

    def test_unused_alpha_byte_cannot_be_mistaken_for_an_rgb_error(self):
        with self.candidate.open("r+b") as stream:
            stream.write(struct.pack("<I", 0x01000000))
        with self.assertRaisesRegex(ValueError, "non-RGB"):
            probe.compare(self.reference, self.candidate)


if __name__ == "__main__":
    unittest.main()
