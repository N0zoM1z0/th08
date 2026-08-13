#!/usr/bin/env python3
"""Compare one VC7 COFF function with the hash-attested TH08 target."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import tomllib

from coff import ObjectModule


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "config" / "match-units.toml"
TARGET_MANIFEST = ROOT / "config" / "target.toml"
RELOCATION_TYPES = {"DIR32": 0x0006, "REL32": 0x0014}


def repository_path(value: str) -> Path:
    path = (ROOT / value).resolve()
    if not path.is_relative_to(ROOT):
        raise ValueError(f"path escapes repository: {value}")
    return path


def load_units() -> dict[str, dict[str, object]]:
    with MANIFEST.open("rb") as stream:
        manifest = tomllib.load(stream)
    if manifest.get("schema_version") != 1:
        raise ValueError("unsupported match-unit schema")
    units = manifest.get("units")
    if not isinstance(units, list):
        raise ValueError("match-units.toml lacks units")
    indexed: dict[str, dict[str, object]] = {}
    for unit in units:
        name = str(unit["name"])
        if name in indexed:
            raise ValueError(f"duplicate match unit: {name}")
        indexed[name] = unit
    return indexed


def verify_target(path: Path) -> bytes:
    with TARGET_MANIFEST.open("rb") as stream:
        expected = tomllib.load(stream)["target"]
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if len(data) != int(expected["size"]) or digest != expected["sha256"]:
        raise ValueError(
            f"target identity mismatch: size={len(data)} sha256={digest}"
        )
    return data


def pe_bytes_at(data: bytes, address: int, size: int) -> bytes:
    if data[:2] != b"MZ":
        raise ValueError("target is not an MZ executable")
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise ValueError("target lacks a PE signature")
    section_count, optional_size = struct.unpack_from("<H12xH", data, pe_offset + 6)
    optional = pe_offset + 24
    image_base = struct.unpack_from("<I", data, optional + 28)[0]
    section_table = optional + optional_size
    rva = address - image_base
    for index in range(section_count):
        offset = section_table + index * 40
        name, virtual_size, section_rva, raw_size, raw_offset = struct.unpack_from(
            "<8sIIII", data, offset
        )
        span = max(virtual_size, raw_size)
        if section_rva <= rva and rva + size <= section_rva + span:
            relative = rva - section_rva
            if relative + size > raw_size:
                raise ValueError(
                    f"target range extends beyond raw {name.rstrip(bytes([0]))!r} data"
                )
            return data[raw_offset + relative : raw_offset + relative + size]
    raise ValueError(f"target range {address:#x}+{size:#x} is outside PE sections")


def object_function(
    path: Path, decorated_name: str
) -> tuple[bytearray, list[dict[str, object]]]:
    module = ObjectModule()
    module.unpack(path.read_bytes())
    matches = []
    for symbol in module.symbols:
        name = symbol.get_name(module.string_table).decode("ascii", errors="strict")
        if name == decorated_name:
            matches.append(symbol)
    if len(matches) != 1:
        raise ValueError(f"expected one COFF symbol {decorated_name!r}, found {len(matches)}")
    symbol = matches[0]
    if symbol.section_number <= 0 or not symbol.aux_records:
        raise ValueError(f"COFF symbol lacks a function definition: {decorated_name}")
    section = module.sections[symbol.section_number - 1]
    size = int(symbol.aux_records[0].total_size)
    if section.data is None or symbol.value + size > len(section.data):
        raise ValueError("COFF function extends beyond its section")
    function = bytearray(section.data[symbol.value : symbol.value + size])
    relocations = []
    for relocation in section.relocations:
        local_offset = relocation.virtual_address - symbol.value
        if 0 <= local_offset < size:
            referenced = module.symbols[relocation.symbol_table_index]
            relocations.append(
                {
                    "offset": local_offset,
                    "type_id": relocation.type,
                    "symbol": referenced.get_name(module.string_table).decode(
                        "ascii", errors="strict"
                    ),
                }
            )
    return function, relocations


def apply_relocations(
    code: bytearray,
    actual: list[dict[str, object]],
    expected: list[dict[str, object]],
    target_address: int,
    target_code: bytes,
) -> list[dict[str, object]]:
    normalized_expected = []
    for relocation in expected:
        kind = str(relocation["type"])
        if kind not in RELOCATION_TYPES:
            raise ValueError(f"unsupported relocation type: {kind}")
        normalized_expected.append(
            {
                "offset": int(relocation["offset"]),
                "type_id": RELOCATION_TYPES[kind],
                "symbol": str(relocation["symbol"]),
                "type": kind,
                "target": int(relocation["target"]),
            }
        )
    actual_key = sorted(
        (int(row["offset"]), int(row["type_id"]), str(row["symbol"]))
        for row in actual
    )
    expected_key = sorted(
        (row["offset"], row["type_id"], row["symbol"])
        for row in normalized_expected
    )
    if actual_key != expected_key:
        raise ValueError(
            f"COFF relocations differ from manifest: actual={actual_key!r} "
            f"expected={expected_key!r}"
        )

    report = []
    for relocation in normalized_expected:
        offset = relocation["offset"]
        if offset < 0 or offset + 4 > len(code):
            raise ValueError(f"relocation offset outside function: {offset:#x}")
        object_field_before = struct.unpack_from("<I", code, offset)[0]
        addend = object_field_before
        if relocation["type"] == "DIR32":
            value = relocation["target"] + addend
        else:
            next_instruction = target_address + offset + 4
            value = relocation["target"] + addend - next_instruction
        struct.pack_into("<I", code, offset, value & 0xFFFFFFFF)
        report.append(
            {
                "offset": f"0x{offset:X}",
                "type": relocation["type"],
                "symbol": relocation["symbol"],
                "target": f"0x{relocation['target']:08X}",
                "addend": f"0x{addend:08X}",
                "addend_signed": struct.unpack("<i", struct.pack("<I", addend))[0],
                "object_field_before": f"0x{object_field_before:08X}",
                "resolved_value": f"0x{value & 0xFFFFFFFF:08X}",
                "target_field": f"0x{struct.unpack_from('<I', target_code, offset)[0]:08X}",
            }
        )
    return report


def compare(unit: dict[str, object], target_path: Path) -> dict[str, object]:
    target_data = verify_target(target_path)
    object_path = repository_path(str(unit["object"]))
    code, actual_relocations = object_function(object_path, str(unit["symbol"]))
    expected_size = int(unit["size"])
    if len(code) != expected_size:
        raise ValueError(
            f"object function size {len(code):#x} differs from manifest {expected_size:#x}"
        )
    target_address = int(unit["target_address"])
    target = pe_bytes_at(target_data, target_address, expected_size)
    relocations = apply_relocations(
        code,
        actual_relocations,
        list(unit.get("relocations", [])),
        target_address,
        target,
    )
    differences = [
        {"offset": f"0x{index:X}", "object": left, "target": right}
        for index, (left, right) in enumerate(zip(code, target))
        if left != right
    ]
    return {
        "unit": unit["name"],
        "result": "exact" if not differences else "mismatch",
        "target_address": f"0x{target_address:08X}",
        "size": expected_size,
        "matched_bytes": expected_size - len(differences),
        "object": str(object_path.relative_to(ROOT)),
        "symbol": unit["symbol"],
        "relocations": relocations,
        "first_differences": differences[:16],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("unit", nargs="?", help="unit name; omit with --list")
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--target", type=Path, default=ROOT / "resources" / "th08.exe")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        units = load_units()
        if args.list:
            print("\n".join(units))
            return 0
        if not args.unit or args.unit not in units:
            raise ValueError(f"unknown or missing unit: {args.unit!r}")
        report = compare(units[args.unit], args.target.expanduser().resolve())
    except (OSError, KeyError, TypeError, ValueError, struct.error) as exc:
        report = {"unit": args.unit, "result": "error", "error": str(exc)}
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print(
            f"{report['unit']}: {report['result']}"
            + (
                f" ({report.get('matched_bytes', 0)}/{report.get('size', 0)} bytes)"
                if "size" in report
                else f": {report.get('error', '')}"
            )
        )
    return 0 if report["result"] == "exact" else 1


if __name__ == "__main__":
    raise SystemExit(main())
