#!/usr/bin/env python3
"""Validate TH08 tracking ledgers, optionally including private target bytes."""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path
import re
import struct
import tomllib


ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "config"
MAPPING_FIELDS = (
    "name",
    "address",
    "size",
    "calling_convention",
    "varargs",
    "return_type",
)
CLAIM_FIELDS = ["address", "owner", "started_utc", "branch", "notes"]
MATCH_FIELDS = [
    "address",
    "name",
    "size",
    "status",
    "match_percent",
    "unit",
    "evidence",
]
RELOCATION_FIELDS = [
    "coff_symbol",
    "address",
    "data_hex",
    "addends",
    "evidence",
    "validation",
]
HEX_DIGEST = re.compile(r"^[0-9a-f]+$")
RECCMP_SHA256 = re.compile(
    r"^\s+sha256:\s*['\"]?([0-9a-fA-F]{64})['\"]?\s*$", re.MULTILINE
)


@dataclass(frozen=True)
class MappingRow:
    line: int
    name: str
    address: int
    size: int

    @property
    def end(self) -> int:
        return self.address + self.size


def fail(message: str) -> None:
    raise ValueError(message)


def parse_address(value: str) -> int:
    return int(value, 0)


def load_target(errors: list[str]) -> dict[str, object] | None:
    try:
        with (CONFIG / "target.toml").open("rb") as stream:
            manifest = tomllib.load(stream)
        target = manifest["target"]
        sha256 = str(target["sha256"]).lower()
        md5 = str(target["md5"]).lower()
        if len(sha256) != 64 or not HEX_DIGEST.fullmatch(sha256):
            fail("sha256 must be 64 lowercase hexadecimal digits")
        if len(md5) != 32 or not HEX_DIGEST.fullmatch(md5):
            fail("md5 must be 32 lowercase hexadecimal digits")
        if int(target["size"]) <= 0:
            fail("target size must be positive")
        return target
    except (OSError, KeyError, TypeError, ValueError, tomllib.TOMLDecodeError) as exc:
        errors.append(f"target.toml: {exc}")
        return None


def validate_reccmp_hash(target: dict[str, object] | None, errors: list[str]) -> None:
    try:
        text = (ROOT / "reccmp-project.yml").read_text(encoding="utf-8")
        digests = RECCMP_SHA256.findall(text)
        if len(digests) != 1:
            fail(f"expected exactly one sha256 entry, found {len(digests)}")
        if target is not None and digests[0].lower() != str(target["sha256"]).lower():
            fail("th08 sha256 differs from config/target.toml")
    except (OSError, ValueError) as exc:
        errors.append(f"reccmp-project.yml: {exc}")


def target_bytes_at(data: bytes, address: int, size: int) -> bytes:
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[:2] != b"MZ" or data[pe_offset : pe_offset + 4] != b"PE\0\0":
        fail("resources/th08.exe is not a PE image")
    section_count, optional_size = struct.unpack_from("<H12xH", data, pe_offset + 6)
    optional = pe_offset + 24
    image_base = struct.unpack_from("<I", data, optional + 28)[0]
    section_table = optional + optional_size
    rva = address - image_base
    for index in range(section_count):
        offset = section_table + index * 40
        virtual_size, section_rva, raw_size, raw_offset = struct.unpack_from(
            "<IIII", data, offset + 8
        )
        if section_rva <= rva and rva + size <= section_rva + max(virtual_size, raw_size):
            relative = rva - section_rva
            if relative + size > raw_size:
                fail("relocation ledger range extends beyond PE raw data")
            return data[raw_offset + relative : raw_offset + relative + size]
    fail(f"relocation ledger range 0x{address:08X}+0x{size:X} is outside the PE")


def validate_relocation_ledger(errors: list[str], *, check_target_bytes: bool) -> int:
    count = 0
    seen_symbols: set[str] = set()
    seen_addresses: set[int] = set()
    try:
        target = (
            (ROOT / "resources" / "th08.exe").read_bytes()
            if check_target_bytes
            else None
        )
        with (CONFIG / "reccmp-relocations.csv").open(
            newline="", encoding="utf-8"
        ) as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != RELOCATION_FIELDS:
                fail(
                    "reccmp-relocations.csv: unexpected columns: "
                    f"{reader.fieldnames}"
                )
            for line, row in enumerate(reader, start=2):
                symbol = row["coff_symbol"]
                address = parse_address(row["address"])
                canonical = f"0x{address:08X}"
                if not symbol or symbol in seen_symbols:
                    fail(
                        f"reccmp-relocations.csv:{line}: empty or duplicate COFF symbol"
                    )
                if address in seen_addresses or row["address"] != canonical:
                    fail(
                        f"reccmp-relocations.csv:{line}: duplicate or noncanonical "
                        f"address {row['address']!r}"
                    )
                literal = bytes.fromhex(row["data_hex"])
                addends = [
                    int(value, 0)
                    for value in row["addends"].split(";")
                    if value
                ]
                validation = row["validation"]
                if not literal or not addends or validation not in {"literal", "address"}:
                    fail(f"reccmp-relocations.csv:{line}: invalid row")
                if not row["evidence"]:
                    fail(f"reccmp-relocations.csv:{line}: evidence is required")
                if target is not None:
                    actual = target_bytes_at(target, address, len(literal))
                    if actual != literal:
                        fail(
                            f"reccmp-relocations.csv:{line}: target bytes differ at "
                            f"{canonical}"
                        )
                seen_symbols.add(symbol)
                seen_addresses.add(address)
                count += 1
    except (OSError, KeyError, TypeError, ValueError, struct.error) as exc:
        errors.append(str(exc))
    return count


def load_mapping(errors: list[str]) -> tuple[list[MappingRow], set[str], set[int]]:
    rows: list[MappingRow] = []
    names: set[str] = set()
    addresses: set[int] = set()
    try:
        with (CONFIG / "mapping.csv").open(newline="", encoding="utf-8") as stream:
            for line, raw in enumerate(csv.reader(stream), start=1):
                if len(raw) < len(MAPPING_FIELDS):
                    fail(
                        f"mapping.csv:{line}: expected at least {len(MAPPING_FIELDS)} "
                        f"columns ({', '.join(MAPPING_FIELDS)}), got {len(raw)}"
                    )
                name, address_text, size_text, convention, _varargs, return_type, *_ = (
                    raw
                )
                if not name or not convention or not return_type:
                    fail(
                        f"mapping.csv:{line}: name, convention, and return type are required"
                    )
                address = parse_address(address_text)
                canonical = f"0x{address:x}"
                if address_text != canonical:
                    fail(
                        f"mapping.csv:{line}: noncanonical address {address_text!r}; "
                        f"expected {canonical}"
                    )
                size = int(size_text, 0)
                if size <= 0:
                    fail(f"mapping.csv:{line}: size must be positive")
                if address in addresses:
                    fail(f"mapping.csv:{line}: duplicate address {canonical}")
                if rows and address <= rows[-1].address:
                    fail(f"mapping.csv:{line}: addresses are not strictly sorted")
                rows.append(MappingRow(line, name, address, size))
                names.add(name)
                addresses.add(address)
        if not rows:
            fail("mapping.csv is empty")
    except (OSError, ValueError) as exc:
        errors.append(str(exc))
    return rows, names, addresses


def validate_implemented(
    mapping_names: set[str], errors: list[str]
) -> tuple[int, set[str]]:
    count = 0
    seen: set[str] = set()
    try:
        with (CONFIG / "implemented.csv").open(newline="", encoding="utf-8") as stream:
            for line, row in enumerate(csv.reader(stream), start=1):
                if len(row) != 1 or not row[0]:
                    fail(f"implemented.csv:{line}: expected one non-empty symbol")
                name = row[0]
                if name in seen:
                    fail(f"implemented.csv:{line}: duplicate symbol {name!r}")
                if name not in mapping_names:
                    fail(
                        f"implemented.csv:{line}: symbol absent from mapping.csv: {name!r}"
                    )
                seen.add(name)
                count += 1
    except (OSError, ValueError) as exc:
        errors.append(str(exc))
    return count, seen


def validate_claims(_mapping_addresses: set[int], errors: list[str]) -> int:
    try:
        with (CONFIG / "claims.csv").open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != CLAIM_FIELDS:
                fail(f"claims.csv: unexpected columns: {reader.fieldnames}")
            for line, _row in enumerate(reader, start=2):
                fail(
                    f"claims.csv:{line}: claims are retired in the single-agent "
                    "workflow; keep this file header-only"
                )
    except (OSError, KeyError, TypeError, ValueError) as exc:
        errors.append(str(exc))
    return 0


def load_match_units(errors: list[str]) -> dict[str, tuple[int, int]]:
    units: dict[str, tuple[int, int]] = {}
    try:
        with (CONFIG / "match-units.toml").open("rb") as stream:
            manifest = tomllib.load(stream)
        if manifest.get("schema_version") != 1:
            fail("match-units.toml: unsupported schema_version")
        for index, unit in enumerate(manifest["units"], start=1):
            name = str(unit["name"])
            if not name or name in units:
                fail(f"match-units.toml: unit {index} has an empty or duplicate name")
            address = int(unit["target_address"])
            size = int(unit["size"])
            if size <= 0:
                fail(f"match-units.toml: unit {name!r} has a non-positive size")
            compare_size = int(unit.get("compare_size", size))
            if compare_size < size:
                fail(
                    f"match-units.toml: unit {name!r} has comparison extent "
                    f"smaller than its coverage size"
                )
            units[name] = (address, size)
    except (OSError, KeyError, TypeError, ValueError, tomllib.TOMLDecodeError) as exc:
        errors.append(str(exc))
    return units


def validate_matches(
    mapping: list[MappingRow],
    implemented_names: set[str],
    errors: list[str],
) -> int:
    count = 0
    seen_addresses: set[int] = set()
    seen_units: set[str] = set()
    mapping_by_address = {row.address: row for row in mapping}
    units = load_match_units(errors)
    try:
        with (CONFIG / "matches.csv").open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != MATCH_FIELDS:
                fail(f"matches.csv: unexpected columns: {reader.fieldnames}")
            for line, row in enumerate(reader, start=2):
                address = parse_address(row["address"])
                canonical = f"0x{address:08X}"
                if row["address"] != canonical:
                    fail(
                        f"matches.csv:{line}: noncanonical address {row['address']!r}; "
                        f"expected {canonical}"
                    )
                if address in seen_addresses:
                    fail(f"matches.csv:{line}: duplicate address {canonical}")
                mapping_row = mapping_by_address.get(address)
                if mapping_row is None:
                    fail(f"matches.csv:{line}: address absent from mapping.csv: {canonical}")
                size = int(row["size"], 0)
                if row["name"] != mapping_row.name or size != mapping_row.size:
                    fail(
                        f"matches.csv:{line}: name/size differs from mapping.csv for "
                        f"{canonical}"
                    )
                if row["name"] not in implemented_names:
                    fail(f"matches.csv:{line}: exact symbol is not implemented")
                if row["status"] != "matching" or row["match_percent"] != "100.00":
                    fail(f"matches.csv:{line}: exact ledger accepts only matching 100.00")
                unit = row["unit"]
                if not unit or unit in seen_units:
                    fail(f"matches.csv:{line}: empty or duplicate comparison unit")
                if unit not in units or units[unit] != (address, size):
                    fail(f"matches.csv:{line}: comparison unit does not describe this range")
                if not row["evidence"]:
                    fail(f"matches.csv:{line}: evidence command is required")
                seen_addresses.add(address)
                seen_units.add(unit)
                count += 1
    except (OSError, KeyError, TypeError, ValueError) as exc:
        errors.append(str(exc))
    return count


def seed_overlaps(rows: list[MappingRow]) -> list[tuple[MappingRow, MappingRow, int]]:
    """Return upstream seed overlaps without treating imported evidence as clean."""
    overlaps: list[tuple[MappingRow, MappingRow, int]] = []
    if not rows:
        return overlaps
    furthest = rows[0]
    for row in rows[1:]:
        if row.address < furthest.end:
            overlaps.append((furthest, row, furthest.end - row.address))
        if row.end > furthest.end:
            furthest = row
    return overlaps


def report_seed_overlaps(
    overlaps: list[tuple[MappingRow, MappingRow, int]],
) -> None:
    if not overlaps:
        return
    print(
        f"warning: imported upstream mapping seed contains {len(overlaps)} "
        "overlapping address ranges"
    )
    for left, right, amount in overlaps:
        print(
            f"  mapping.csv:{left.line}/{right.line}: {left.name} overlaps "
            f"{right.name} at 0x{right.address:06x} by 0x{amount:x} bytes"
        )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "Examples:\n"
            "  python3 scripts/validate-tracking.py --require-target\n"
            "  python3 scripts/validate-tracking.py --skip-target-bytes  # public CI"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    target_mode = parser.add_mutually_exclusive_group()
    target_mode.add_argument(
        "--require-target",
        action="store_true",
        help="fail unless resources/th08.exe exists, then validate relocation bytes",
    )
    target_mode.add_argument(
        "--skip-target-bytes",
        action="store_true",
        help="validate static ledger structure without reading the untracked executable",
    )
    args = parser.parse_args()

    errors: list[str] = []
    target = load_target(errors)
    validate_reccmp_hash(target, errors)
    target_path = ROOT / "resources" / "th08.exe"
    if args.require_target and not target_path.is_file():
        errors.append(f"required target is missing: {target_path}")
    check_target_bytes = not args.skip_target_bytes and target_path.is_file()
    relocations = validate_relocation_ledger(
        errors, check_target_bytes=check_target_bytes
    )
    mapping, names, addresses = load_mapping(errors)
    implemented, implemented_names = validate_implemented(names, errors)
    claims = validate_claims(addresses, errors)
    matches = validate_matches(mapping, implemented_names, errors)
    overlaps = seed_overlaps(mapping)
    report_seed_overlaps(overlaps)

    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1

    print(
        f"tracking data OK: {len(mapping):,} mapping rows, "
        f"{implemented:,} implemented symbols, {matches:,} exact matches, "
        f"{claims:,} claim rows, {relocations:,} relocation-only symbols; "
        f"target bytes {'checked' if check_target_bytes else 'skipped'}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
