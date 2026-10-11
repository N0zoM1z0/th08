#!/usr/bin/env python3
"""Keep relocation normalization from concealing a wrong switch destination."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location("compare_function", Path(__file__).with_name("compare-function.py"))
comparator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparator)


class LocalRelocationTests(unittest.TestCase):
    def compare(self, source_offset, destination=8, addend=0, symbol="$L123"):
        code = bytearray(16)
        struct.pack_into("<I", code, 0, addend)
        actual = [dict(offset=0, type_id=6, symbol=symbol, local_symbol_offset=source_offset)]
        expected = [dict(offset=0, type="DIR32", symbol="$L*", target=0x400000 + destination)]
        target = struct.pack("<I", 0x400000 + destination + addend) + bytes(12)
        comparator.apply_relocations(code, actual, expected, 0x400000, target)
        return code

    def test_renumbered_label_keeps_its_destination(self):
        self.assertEqual(struct.unpack_from("<I", self.compare(8, symbol="$L9876"))[0], 0x400008)

    def test_wrong_case_destination_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "local relocation"):
            self.compare(6)

    def test_addend_does_not_hide_wrong_label(self):
        with self.assertRaisesRegex(ValueError, "local relocation"):
            self.compare(6, addend=2)

    def test_correct_label_preserves_addend(self):
        self.assertEqual(struct.unpack_from("<I", self.compare(8, addend=2))[0], 0x40000a)

    def test_external_symbol_is_normalized(self):
        self.assertEqual(struct.unpack_from("<I", self.compare(None))[0], 0x400008)


if __name__ == "__main__":
    unittest.main()
