#!/usr/bin/env python3
"""Target-independent corruption and layout checks for the replay reader."""
import copy
import importlib.util
import json
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
from replay_file import CORPUS, decode, inspect, RETAIL_IDENTITY, verify_fixture


def pack_replay(change=None):
    data = bytearray(0x134 + 36 + 6 + 2)
    data[:6] = b"T8RP\x06\0"
    struct.pack_into("<I", data, 0x20, 0x134)
    struct.pack_into("<I", data, 0x44, 0x134 + 42)
    struct.pack_into("<h", data, 0x7c, -1)
    struct.pack_into("<II", data, 0x124, *RETAIL_IDENTITY[:2])
    data[0x12c:0x132] = b"0100d\0"
    if change:
        change(data)
    payload = data[0x68:]
    bits = "".join("1" + format(value, "08b") for value in payload) + "0" * 14
    bits += "0" * (-len(bits) % 8)
    compressed = int(bits, 2).to_bytes(len(bits) // 8, "big")
    header = data[:0x68]
    struct.pack_into("<I", header, 0x0c, 0x68 + len(compressed))
    header[0x15] = 127
    struct.pack_into("<II", header, 0x18, len(compressed), len(payload))
    raw = header + compressed
    struct.pack_into("<I", raw, 0x10, (0x3f000318 + sum(raw[0x15:])) & 0xffffffff)
    key = raw[0x15]
    for index in range(0x18, len(raw)):
        raw[index] = (raw[index] + key) & 255
        key = (key + 7) & 255
    return bytes(raw)


class ReplayFileTests(unittest.TestCase):
    def inspect(self, data):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.rpy"
            path.write_bytes(data)
            return inspect(path)

    def test_metadata_and_stage_extent(self):
        result = self.inspect(pack_replay())
        self.assertEqual(result["shotName"], "Border Team")
        self.assertEqual(result["stages"][0]["inputRecords"], 3)
        self.assertEqual(result["executable"]["version"], "0100d")

    def test_checksum_corruption(self):
        raw = bytearray(pack_replay())
        raw[-1] ^= 1
        with self.assertRaisesRegex(ValueError, "checksum"):
            decode(raw)

    def test_truncated_file(self):
        with self.assertRaisesRegex(ValueError, "extent"):
            decode(pack_replay()[:-1])

    def test_other_executable(self):
        with self.assertRaisesRegex(ValueError, "identity"):
            self.inspect(pack_replay(lambda data: struct.pack_into("<I", data, 0x124, 840705)))

    def test_slow_mode(self):
        with self.assertRaisesRegex(ValueError, "Slow-mode"):
            self.inspect(pack_replay(lambda data: data.__setitem__(0xd9, 1)))

    def test_invalid_offsets(self):
        with self.assertRaisesRegex(ValueError, "offsets"):
            self.inspect(pack_replay(lambda data: struct.pack_into("<I", data, 0x20, 0xffffff00)))

    def test_duplicate_streams(self):
        with self.assertRaisesRegex(ValueError, "offsets"):
            self.inspect(pack_replay(lambda data: struct.pack_into("<I", data, 0x44, 0x134)))

    def test_invalid_input_encoding(self):
        with self.assertRaisesRegex(ValueError, "encoding"):
            self.inspect(pack_replay(lambda data: data.__setitem__(6, 2)))


class CommittedReplayTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root = Path(__file__).resolve().parents[1]
        cls.fixture = json.loads((root / "config/replay-fixtures.json").read_text())["fixtures"][0]
        spec = importlib.util.spec_from_file_location("suite", root / "scripts/test-replay-suite.py")
        cls.suite = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.suite)

    def test_offline_fixture_reuse(self):
        with tempfile.TemporaryDirectory() as directory, patch(
            "urllib.request.urlopen", side_effect=AssertionError("Network access attempted")
        ):
            path = self.suite.fetch(self.fixture, Path(directory))
            self.assertEqual(path, CORPUS / "accepted" / self.fixture["file"])
            self.assertEqual(list(Path(directory).iterdir()), [])

    def test_invalid_cache_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / self.fixture["file"]
            path.write_bytes(b"damaged recording")
            with self.assertRaises(ValueError):
                self.suite.fetch(self.fixture, Path(directory))

    def test_manifest_stage_score_is_verified(self):
        fixture = copy.deepcopy(self.fixture)
        fixture["stages"][0]["endScore"] += 1
        with self.assertRaisesRegex(ValueError, "stages differ"):
            verify_fixture(CORPUS / "accepted" / fixture["file"], fixture)


if __name__ == "__main__":
    unittest.main()
