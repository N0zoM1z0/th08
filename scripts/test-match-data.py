#!/usr/bin/env python3
"""Synthetic PE/COFF mutation tests; no game, toolchain or Wine is required."""

from __future__ import annotations

import contextlib
import copy
import importlib.util
import io
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

from coff import AuxRecordFunctionDef, ObjectModule, Relocation, Section, SymbolRecord
import match_data as data
import match_evidence as evidence
from pe_image import PEImage


ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    "data_test_comparator", ROOT / "scripts/compare-function.py"
)
comparator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparator)


def pe(base=0x400000):
    raw = bytearray(0x800)
    raw[:2] = b"MZ"
    struct.pack_into("<I", raw, 0x3C, 0x80)
    raw[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", raw, 0x84, 0x14C, 3, 0, 0, 0, 224, 0x102)
    optional = 0x98
    struct.pack_into("<H", raw, optional, 0x10B)
    struct.pack_into(
        "<9I",
        raw,
        optional + 4,
        0x200,
        0x400,
        0,
        0x1000,
        0x1000,
        0x2000,
        base,
        0x1000,
        0x200,
    )
    struct.pack_into("<II", raw, optional + 56, 0x4000, 0x200)
    struct.pack_into("<I", raw, optional + 92, 16)
    for index, (name, rva, offset, flags) in enumerate(
        [
            (b".text", 0x1000, 0x200, 0x60000020),
            (b".data", 0x2000, 0x400, 0xC0000040),
            (b".rdata", 0x3000, 0x600, 0x40000040),
        ]
    ):
        struct.pack_into(
            "<8sIIIIIIHHI",
            raw,
            optional + 224 + index * 40,
            name,
            0x200,
            rva,
            0x200,
            offset,
            0,
            0,
            0,
            0,
            flags,
        )
    raw[0x200:0x206] = b"\xa1" + struct.pack("<I", base + 0x2000) + b"\xc3"
    for index in range(6):
        struct.pack_into("<I", raw, 0x400 + index * 4, base + 0x3000 + index * 16)
        raw[0x600 + index * 16 : 0x603 + index * 16] = f"A{index}".encode() + b"\0"
    struct.pack_into("<I", raw, 0x500, 43)
    return PEImage.from_bytes(bytes(raw), "<synthetic fixture>")


def changed(image, address, content):
    raw = bytearray(image.data)
    offset = image.rva_to_offset(address - image.image_base, len(content))
    raw[offset : offset + len(content)] = content
    return PEImage.from_bytes(bytes(raw), "<mutated fixture>")


def symbol(module, name, section, value, *, function=False):
    encoded = name.encode()
    if len(encoded) > 8:
        encoded = struct.pack("<II", 0, module.string_table.append(encoded))
    item = SymbolRecord(
        encoded, typ=0x20 if function else 0, section_number=section, storage_class=2
    )
    item.value = value
    if function:
        item.aux_records = [AuxRecordFunctionDef(total_size=6)]
    index = module.symbols.cur_insertion_idx
    module.symbols.append(item)
    return index


def relocation(section, offset, index, kind=6):
    row = Relocation()
    row.virtual_address, row.symbol_table_index, row.type = offset, index, kind
    section.relocations.append(row)


def coff():
    module = ObjectModule()
    module.sections = [
        Section(b".text", flags=0x60000020, data=b"\xa1\0\0\0\0\xc3"),
        Section(b".data", flags=0xC0000040, data=bytes(0x200)),
        Section(b".rdata", flags=0x40000040, data=pe().read_rva(0x3000, 0x200)),
    ]
    for section in module.sections:
        section.size_of_raw_data = len(section.data)
    symbol(module, "func", 1, 0, function=True)
    table = symbol(module, "table", 2, 0)
    relocation(module.sections[0], 1, table)
    for index in range(6):
        item = symbol(module, f"str{index}", 3, index * 16)
        relocation(module.sections[1], index * 4, item)
    symbol(module, "scalar", 2, 0x100)
    body = bytearray(module.sections[1].data)
    struct.pack_into("<I", body, 0x100, 43)
    module.sections[1].data = bytes(body)
    for section in module.sections:
        section.number_of_relocations = len(section.relocations)
    return module


class DataTests(unittest.TestCase):
    def setUp(self):
        (ROOT / "build").mkdir(exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(
            prefix="data-contract-test-", dir=ROOT / "build"
        )
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name) / "fixture.obj"
        self.relative = str(self.path.relative_to(ROOT))
        self.module, self.target, self.rebuild = coff(), pe(), pe(0x500000)
        self.contract = dict(
            name="six-labels",
            kind="pointer_array",
            object=self.relative,
            symbol="table",
            target_address=0x402000,
            size=24,
            evidence="Synthetic fixture: six explicit pointer slots.",
            entries=[
                dict(
                    kind="cstring",
                    target=0x403000 + index * 16,
                    symbol=f"str{index}",
                    addend=0,
                    data_hex=(f"A{index}".encode() + b"\0").hex(),
                )
                for index in range(6)
            ],
        )
        self.publics = {
            "func": {0x501000},
            "table": {0x502000},
            "scalar": {0x502100},
            **{f"str{index}": {0x503000 + index * 16} for index in range(6)},
        }
        self.unit = dict(
            name="fixture-function",
            object=self.relative,
            symbol="func",
            target_address=0x401000,
            size=6,
            relocations=[dict(offset=1, type="DIR32", symbol="table", target=0x402000)],
        )
        self.write()

    def write(self):
        for section in self.module.sections:
            section.size_of_raw_data = len(section.data)
            section.number_of_relocations = len(section.relocations)
        self.path.write_bytes(self.module.get_buffer())

    def check(self, contract=None, **kwargs):
        self.write()
        return data.check(
            contract or self.contract, self.target, object_path=self.path, **kwargs
        )

    def code(self):
        self.write()
        with (
            patch.object(comparator, "verify_target", return_value=self.target.data),
            patch.object(evidence, "load_contracts", return_value=[self.contract]),
        ):
            return comparator.compare(self.unit, Path("synthetic"))

    def test_coff_and_relocated_pe_pass_the_same_contract(self):
        report = self.check(linked=(self.rebuild, self.publics))
        self.assertEqual(report["result"], "verified")
        self.assertEqual(report["coff"]["status"], "verified")
        self.assertEqual(report["linked"]["status"], "verified")

    def test_missing_tail_is_rejected_even_when_code_is_exact(self):
        self.module.sections[1].data = bytes(20)
        report = self.code()
        self.assertEqual(report["result"], "mismatch")
        self.assertEqual(report["evidence"]["code"]["status"], "exact")
        self.assertEqual(report["findings"][0]["code"], "DATA_OWNER_EXTENT")

    def test_six_slots_with_missing_initializer_do_not_pass(self):
        self.module.sections[1].relocations.pop()
        self.assertEqual(self.check()["findings"][0]["code"], "DATA_RELOCATION_SET")

    def test_changed_string_is_rejected(self):
        raw = bytearray(self.module.sections[2].data)
        raw[5 * 16] = ord("X")
        self.module.sections[2].data = bytes(raw)
        self.assertEqual(self.check()["findings"][0]["code"], "DATA_CONTENT_MISMATCH")

    def test_data_is_not_taken_from_a_different_good_object(self):
        good = Path(self.temporary.name) / "good.obj"
        good.write_bytes(self.module.get_buffer())
        self.contract["object"] = str(good.relative_to(ROOT))
        self.module.sections[1].relocations.pop()
        report = self.code()
        self.assertEqual(report["result"], "mismatch")
        self.assertEqual(
            report["evidence"]["data"]["contracts"][0]["checked_object"], self.relative
        )

    def test_contract_cannot_absorb_the_next_data_owner(self):
        symbol(self.module, "next", 2, 20)
        self.assertEqual(self.check()["findings"][0]["code"], "DATA_OWNER_EXTENT")

    def test_duplicate_or_non_dir32_relocations_are_rejected(self):
        first = self.module.sections[1].relocations[0]
        for kind in (6, 20):
            with self.subTest(kind=kind):
                relocation(self.module.sections[1], 0, first.symbol_table_index, kind)
                self.assertEqual(
                    self.check()["findings"][0]["code"], "DATA_RELOCATION_INVALID"
                )
                self.module.sections[1].relocations.pop()

    def test_invalid_coff_symbol_reference_is_a_structured_failure(self):
        self.module.sections[1].relocations[0].symbol_table_index = 999999
        report = self.check()
        self.assertEqual(report["result"], "mismatch")
        self.assertEqual(report["findings"][0]["code"], "DATA_INPUT_INVALID")

    def test_plain_scalar_contents_are_checked(self):
        contract = dict(
            name="scalar",
            kind="bytes",
            object=self.relative,
            symbol="scalar",
            target_address=0x402100,
            size=4,
            data_hex="2b000000",
            evidence="Synthetic scalar.",
        )
        self.assertEqual(self.check(contract)["result"], "verified")
        raw = bytearray(self.module.sections[1].data)
        struct.pack_into("<I", raw, 0x100, 42)
        self.module.sections[1].data = bytes(raw)
        self.assertEqual(
            self.check(contract)["findings"][0]["code"], "DATA_CONTENT_MISMATCH"
        )

    def test_same_size_same_timestamp_object_change_is_not_cached_as_good(self):
        self.assertEqual(self.check()["result"], "verified")
        old = self.path.stat()
        raw = bytearray(self.module.sections[2].data)
        raw[0] = ord("X")
        self.module.sections[2].data = bytes(raw)
        self.write()
        os.utime(self.path, ns=(old.st_atime_ns, old.st_mtime_ns))
        self.assertEqual(
            data.check(self.contract, self.target, object_path=self.path)["findings"][
                0
            ]["code"],
            "DATA_CONTENT_MISMATCH",
        )

    def test_bad_linked_pointer_is_not_hidden_by_matching_code(self):
        broken = changed(self.rebuild, 0x502014, struct.pack("<I", 1))
        self.assertEqual(
            self.check(linked=(broken, self.publics))["findings"][0]["code"],
            "DATA_POINTER_INVALID",
        )

    def test_linked_string_contents_are_followed(self):
        broken = changed(self.rebuild, 0x503050, b"X")
        self.assertEqual(
            self.check(linked=(broken, self.publics))["findings"][0]["code"],
            "DATA_CONTENT_MISMATCH",
        )

    def test_equal_text_at_a_different_owner_does_not_hide_wrong_pointer(self):
        broken = changed(self.rebuild, 0x503100, b"A0\0")
        broken = changed(broken, 0x502000, struct.pack("<I", 0x503100))
        self.assertEqual(
            self.check(linked=(broken, self.publics))["findings"][0]["code"],
            "DATA_POINTER_IDENTITY",
        )

    def test_explicit_null_slot_is_checked(self):
        contract = copy.deepcopy(self.contract)
        contract["entries"][0] = {"kind": "null"}
        self.target = changed(self.target, 0x402000, bytes(4))
        self.module.sections[1].relocations.pop(0)
        self.assertEqual(self.check(contract)["result"], "verified")
        raw = bytearray(self.module.sections[1].data)
        raw[0] = 1
        self.module.sections[1].data = bytes(raw)
        self.assertEqual(
            self.check(contract)["findings"][0]["code"], "DATA_POINTER_MISMATCH"
        )

    def test_symbol_pointer_requires_identity_and_addend(self):
        contract = copy.deepcopy(self.contract)
        contract["entries"][0] = dict(
            kind="symbol", target=0x401000, symbol="func", addend=0
        )
        self.target = changed(self.target, 0x402000, struct.pack("<I", 0x401000))
        self.assertEqual(
            self.check(contract)["findings"][0]["code"], "DATA_POINTER_IDENTITY"
        )
        self.module.sections[1].relocations[0].symbol_table_index = 0
        self.assertEqual(self.check(contract)["result"], "verified")
        wrong = changed(self.rebuild, 0x502000, struct.pack("<I", 0x501010))
        self.assertEqual(
            data.check_linked(
                contract,
                self.target,
                changed(self.rebuild, 0x502000, struct.pack("<I", 0x501000)),
                self.publics,
            )["status"],
            "verified",
        )
        with self.assertRaises(data.DataError) as raised:
            data.check_linked(contract, self.target, wrong, self.publics)
        self.assertEqual(raised.exception.finding["code"], "DATA_POINTER_IDENTITY")

    def test_ambiguous_link_symbol_is_rejected(self):
        self.publics["table"].add(0x502010)
        self.assertEqual(
            self.check(linked=(self.rebuild, self.publics))["findings"][0]["code"],
            "LINK_SYMBOL_UNRESOLVED",
        )

    def test_unchecked_contract_cannot_pass(self):
        with self.assertRaisesRegex(data.DataError, "choose COFF or linked"):
            data.check(self.contract, self.target)

    def test_compiler_literal_content_is_verified_from_coff(self):
        literal = "??_C@_02@FIXTURE@A0?$AA@"
        index = symbol(self.module, literal, 3, 0)
        self.module.sections[0].relocations[0].symbol_table_index = index
        self.unit["relocations"][0].update(symbol=literal, target=0x403000)
        self.target = changed(self.target, 0x401001, struct.pack("<I", 0x403000))
        self.assertEqual(self.code()["evidence"]["data"]["literal_check_count"], 1)
        raw = bytearray(self.module.sections[2].data)
        raw[0] = ord("B")
        self.module.sections[2].data = bytes(raw)
        report = self.code()
        self.assertEqual(report["evidence"]["code"]["status"], "exact")
        self.assertEqual(report["result"], "mismatch")
        self.assertEqual(report["findings"][0]["code"], "LITERAL_CONTENT_MISMATCH")

    def test_linked_code_replays_actual_relocations(self):
        raw, relocations = comparator.object_function(self.path, "func")
        report = evidence.linked_code(
            self.unit, "func", raw, relocations, (self.rebuild, self.publics)
        )
        self.assertEqual(report["status"], "verified")
        bad = changed(self.rebuild, 0x501001, struct.pack("<I", 0x502004))
        report = evidence.linked_code(
            self.unit, "func", raw, relocations, (bad, self.publics)
        )
        self.assertEqual(report["findings"][0]["code"], "LINK_CODE_MISMATCH")

    def test_missing_link_symbols_remain_partial_without_masking(self):
        raw, relocations = comparator.object_function(self.path, "func")
        report = evidence.linked_code(
            self.unit, "func", raw, relocations, (self.rebuild, {"func": {0x501000}})
        )
        self.assertEqual(report["status"], "partial")
        self.assertEqual(report["findings"][0]["code"], "LINK_SYMBOL_UNRESOLVED")

    def test_strict_data_mode_has_json_and_distinct_incomplete_exit(self):
        output = io.StringIO()
        with (
            patch.object(
                comparator, "load_units", return_value={self.unit["name"]: self.unit}
            ),
            patch.object(comparator, "verify_target", return_value=self.target.data),
            patch.object(evidence, "load_contracts", return_value=[]),
            patch.object(
                sys,
                "argv",
                ["compare-function.py", self.unit["name"], "--json", "--require-data"],
            ),
            contextlib.redirect_stdout(output),
        ):
            result = comparator.main()
        report = json.loads(output.getvalue())
        self.assertEqual((result, report["result"]), (2, "incomplete"))
        self.assertEqual(
            report["evidence"]["data"]["uncovered"][0]["code"],
            "INITIALIZED_DATA_UNCOVERED",
        )

    def test_json_unit_discovery_is_bounded_and_paginated(self):
        units = {
            f"unit-{index:02}": {**self.unit, "name": f"unit-{index:02}"}
            for index in range(45)
        }
        for extra, expected_count, next_offset in [
            ([], 40, 40),
            (["--offset", "40"], 5, None),
            (["--filter", "unit-44"], 1, None),
        ]:
            output = io.StringIO()
            with (
                patch.object(comparator, "load_units", return_value=units),
                patch.object(
                    sys, "argv", ["compare-function.py", "--list", "--json", *extra]
                ),
                contextlib.redirect_stdout(output),
            ):
                result = comparator.main()
            report = json.loads(output.getvalue())
            self.assertEqual(result, 0)
            self.assertEqual(len(report["units"]), expected_count)
            self.assertEqual(report["next_offset"], next_offset)

    def test_unknown_unit_returns_json_finding_and_discovery_command(self):
        output = io.StringIO()
        with (
            patch.object(comparator, "load_units", return_value={}),
            patch.object(sys, "argv", ["compare-function.py", "unknown", "--json"]),
            contextlib.redirect_stdout(output),
        ):
            result = comparator.main()
        report = json.loads(output.getvalue())
        self.assertEqual((result, report["result"]), (1, "error"))
        self.assertEqual(report["findings"][0]["code"], "UNIT_UNKNOWN")
        self.assertIn("--list", report["next_commands"][0])

    def test_manifest_rejects_unknown_fields_short_extent_and_overlap(self):
        document = dict(
            schema_version=1, target_sha256="fixture", contracts=[self.contract]
        )
        self.assertEqual(len(data.validate_manifest(document, "fixture")), 1)
        for mutate in [
            lambda row: row.update(unknown=True),
            lambda row: row.update(size=20),
            lambda row: row["entries"][0].update(data_hex="413000ff"),
        ]:
            candidate = copy.deepcopy(document)
            mutate(candidate["contracts"][0])
            with self.assertRaises(data.DataError):
                data.validate_manifest(candidate, "fixture")
        duplicate = copy.deepcopy(document)
        duplicate["contracts"].append({**self.contract, "name": "overlap"})
        with self.assertRaisesRegex(data.DataError, "overlapping"):
            data.validate_manifest(duplicate, "fixture")

    def test_manifest_rejects_path_escape_wrong_target_and_bad_shapes(self):
        document = dict(
            schema_version=1, target_sha256="fixture", contracts=[self.contract]
        )
        with self.assertRaises(data.DataError):
            data.validate_manifest(document, "other")
        for broken in [
            None,
            {**document, "contracts": [None]},
            {**document, "contracts": [{**self.contract, "object": "../bad.obj"}]},
            {**document, "contracts": [{**self.contract, "entries": [None] * 6}]},
        ]:
            with self.assertRaises(data.DataError):
                data.validate_manifest(broken, "fixture")

    def test_schema_validation_cli_does_not_read_private_inputs(self):
        spec = importlib.util.spec_from_file_location(
            "data_test_cli", ROOT / "scripts/compare-data.py"
        )
        cli = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cli)
        output = io.StringIO()
        with (
            patch.object(
                cli, "PEImage", side_effect=AssertionError("private input read")
            ),
            patch.object(sys, "argv", ["compare-data.py", "--validate-only", "--json"]),
            contextlib.redirect_stdout(output),
        ):
            result = cli.main()
        self.assertEqual(result, 0)
        self.assertEqual(json.loads(output.getvalue())["target_status"], "not_checked")

    def test_foreign_target_is_rejected(self):
        with self.assertRaises(data.DataError) as raised:
            data.attest_target(self.target)
        self.assertEqual(raised.exception.finding["code"], "TARGET_IDENTITY_INVALID")


if __name__ == "__main__":
    unittest.main()
