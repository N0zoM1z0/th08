#!/usr/bin/env python3
"""Keep relocation normalization from concealing a wrong switch destination."""
import importlib.util
from pathlib import Path
import struct
from types import SimpleNamespace
import unittest
from unittest import mock

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


class AnonymousNamespaceTests(unittest.TestCase):
    original = "?AppendFormat@?A0xb3f6a42d@th08@@YAPADPADPBDZZ"
    relocated = "?AppendFormat@?A0x64243aa8@th08@@YAPADPADPBDZZ"

    def function(self, names, size=16):
        symbols = [SimpleNamespace(
            get_name=lambda table, name=name: name.encode("ascii"), section_number=1,
            value=0, aux_records=[SimpleNamespace(total_size=size)]) for name in names]
        module = SimpleNamespace(symbols=symbols, string_table=b"", unpack=lambda data: None,
                                 sections=[SimpleNamespace(data=bytes(16), relocations=[])])
        path = mock.Mock()
        path.read_bytes.return_value = b"fixture"
        identity = {}
        with mock.patch.object(comparator, "ObjectModule", return_value=module):
            code, rows = comparator.object_function(path, self.original, identity=identity)
        return code, identity

    def relocation(self, actual_symbol, offset=0, type_id=20, target=0x500000, local=None):
        code = bytearray(16)
        actual = [dict(offset=offset, type_id=type_id, symbol=actual_symbol, local_symbol_offset=local)]
        expected = [dict(offset=0, type="REL32", symbol=self.original, target=target)]
        canonical = struct.pack("<I", 0x500000 - 0x400004) + bytes(12)
        report = comparator.apply_relocations(code, actual, expected, 0x400000, canonical)
        return bytes(code), canonical, report

    def test_checkout_discriminator_changes_preserve_function_identity(self):
        code, identity = self.function([self.relocated])
        self.assertEqual(len(code), 16)
        self.assertEqual(identity["object_symbol"], self.relocated)
        code, canonical, report = self.relocation(self.relocated)
        self.assertEqual(code, canonical)
        self.assertEqual(report[0]["object_symbol"], self.relocated)
        self.assertEqual(report[0]["symbol"], self.original)

    def test_ambiguous_namespace_aliases_are_rejected(self):
        with self.assertRaisesRegex(ValueError, "found 2"):
            self.function([self.original, self.relocated])

    def test_other_names_and_signatures_still_require_exact_identity(self):
        for changed in (self.relocated.replace("AppendFormat", "OtherFunction"),
                        self.relocated.replace("PBDZZ", "PADZZ"),
                        self.relocated.replace("@th08@", "@other@")):
            with self.subTest(changed=changed), self.assertRaisesRegex(ValueError, "found 0"):
                self.function([changed])
            with self.subTest(changed=changed), self.assertRaisesRegex(ValueError, "relocations differ"):
                self.relocation(changed)

    def test_relocation_offsets_and_types_are_still_checked(self):
        for offset, type_id in ((4, 20), (0, 6)):
            with self.subTest(offset=offset, type_id=type_id), self.assertRaisesRegex(ValueError, "relocations differ"):
                self.relocation(self.relocated, offset=offset, type_id=type_id)

    def test_function_extent_and_call_target_are_not_normalized(self):
        with self.assertRaisesRegex(ValueError, "extends beyond"):
            self.function([self.relocated], size=20)
        code, canonical, report = self.relocation(self.relocated, target=0x500010)
        self.assertNotEqual(code, canonical)
        with self.assertRaisesRegex(ValueError, "local relocation"):
            self.relocation(self.relocated, target=0x400008, local=6)


if __name__ == "__main__":
    unittest.main()
