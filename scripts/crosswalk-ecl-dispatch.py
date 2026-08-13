#!/usr/bin/env python3
"""Crosswalk TH08 RunEcl target handlers against one VC7 probe object.

This is a diagnostic only.  It verifies the private target first, pairs the
184 target jump-table slots with the first contiguous COFF DIR32 table, and
reports physical handler span deltas.  It never updates mapping or progress.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from coff import ObjectModule  # noqa: E402


RUN_ECL_ADDRESS = 0x004184B0
RUN_ECL_CODE_SIZE = 0x680E
RUN_ECL_SLOTS = 184
DIR32 = 0x0006
RUN_ECL_SYMBOL = "?RunEcl@EclManager@th08@@QAE?AW4ZunResult@@PAUEnemy@2@@Z"


def target_bytes_at(data: bytes, address: int, size: int) -> bytes:
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise ValueError("target lacks a PE header")
    section_count, optional_size = struct.unpack_from("<H12xH", data, pe_offset + 6)
    optional = pe_offset + 24
    image_base = struct.unpack_from("<I", data, optional + 28)[0]
    rva = address - image_base
    for index in range(section_count):
        offset = optional + optional_size + index * 40
        _name, virtual_size, section_rva, raw_size, raw_offset = struct.unpack_from(
            "<8sIIII", data, offset
        )
        if section_rva <= rva and rva + size <= section_rva + max(virtual_size, raw_size):
            relative = rva - section_rva
            if relative + size > raw_size:
                raise ValueError("target extent leaves raw PE data")
            return data[raw_offset + relative : raw_offset + relative + size]
    raise ValueError("target extent is outside PE sections")


def verified_target(path: Path) -> bytes:
    expected = tomllib.loads((ROOT / "config" / "target.toml").read_text(encoding="utf-8"))["target"]
    data = path.read_bytes()
    if len(data) != int(expected["size"]) or hashlib.sha256(data).hexdigest() != expected["sha256"]:
        raise ValueError("target identity mismatch")
    return data


def function_symbol(module: ObjectModule):
    symbols = [
        symbol
        for symbol in module.symbols
        if symbol.get_name(module.string_table).decode("ascii", errors="strict") == RUN_ECL_SYMBOL
    ]
    if len(symbols) != 1 or not symbols[0].aux_records:
        raise ValueError("RunEcl COFF function definition is unavailable")
    return symbols[0]


def object_table(module: ObjectModule, section, body_start: int, body_end: int):
    relocations = sorted(
        (relocation.virtual_address, relocation)
        for relocation in section.relocations
        if relocation.type == DIR32
    )
    candidates = []
    for index in range(len(relocations) - RUN_ECL_SLOTS + 1):
        run = relocations[index : index + RUN_ECL_SLOTS]
        start = run[0][0]
        if not all(offset == start + slot * 4 for slot, (offset, _relocation) in enumerate(run)):
            continue
        destinations = []
        for offset, relocation in run:
            symbol = module.symbols[relocation.symbol_table_index]
            addend = struct.unpack_from("<I", section.data, offset)[0]
            destinations.append(symbol.value + addend)
        if all(body_start <= value < start for value in destinations):
            candidates.append((start, destinations))
    if not candidates:
        raise ValueError("no isolated 184-entry RunEcl COFF jump table")
    return min(candidates, key=lambda row: row[0])


def spans(starts: list[int], end: int) -> dict[int, int]:
    ordered = sorted(set(starts))
    return {
        start: (ordered[index + 1] if index + 1 < len(ordered) else end) - start
        for index, start in enumerate(ordered)
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--object", type=Path, default=ROOT / "build" / "probes" / "EclRun.obj")
    parser.add_argument("--target", type=Path, default=ROOT / "resources" / "th08.exe")
    parser.add_argument("--top", type=int, default=20)
    args = parser.parse_args()
    if args.top <= 0:
        raise ValueError("--top must be positive")

    target = verified_target(args.target.resolve())
    target_table_address = RUN_ECL_ADDRESS + RUN_ECL_CODE_SIZE
    target_slots = [
        struct.unpack_from("<I", target_bytes_at(target, target_table_address + slot * 4, 4))[0]
        for slot in range(RUN_ECL_SLOTS)
    ]

    module = ObjectModule()
    module.unpack(args.object.resolve().read_bytes())
    symbol = function_symbol(module)
    section = module.sections[symbol.section_number - 1]
    body_start = symbol.value
    table_start, object_slots = object_table(module, section, body_start, symbol.value + symbol.aux_records[0].total_size)
    target_spans = spans(target_slots, target_table_address)
    object_spans = spans(object_slots, table_start)

    rows = []
    for slot, (target_start, object_start) in enumerate(zip(target_slots, object_slots), start=1):
        if target_slots.index(target_start) != slot - 1:
            continue
        rows.append(
            {
                "opcode": slot,
                "aliases": [index + 1 for index, value in enumerate(target_slots) if value == target_start],
                "target_start": f"0x{target_start:08X}",
                "target_size": target_spans[target_start],
                "object_start": f"0x{object_start:X}",
                "object_size": object_spans[object_start],
                "delta": object_spans[object_start] - target_spans[target_start],
            }
        )
    rows.sort(key=lambda row: (-int(row["delta"]), int(row["opcode"])))
    print(json.dumps({
        "target_sha256": hashlib.sha256(target).hexdigest(),
        "target": {"address": f"0x{RUN_ECL_ADDRESS:08X}", "code_size": RUN_ECL_CODE_SIZE, "slots": RUN_ECL_SLOTS},
        "object": {"path": str(args.object), "function_size": symbol.aux_records[0].total_size, "jump_table_offset": table_start, "code_size": table_start - body_start},
        "physical_handler_delta": sum(int(row["delta"]) for row in rows),
        "largest_positive_spans": rows[: args.top],
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
