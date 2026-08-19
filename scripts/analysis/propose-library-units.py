#!/usr/bin/env python3
"""Propose conservative review-only library match units from one pinned archive.

This command never edits ledgers and never proves exactness. It requires the
canonical target, a SHA-pinned archive from library-provenance.toml, a COFF
function-definition extent equal to the current mapping size, supported
DIR32/REL32 relocations, and exact equality for every non-relocation byte.
Only target addresses with a unique archive candidate are printed by default.
Final acceptance still requires an explicit reviewed unit and compare-library.py.

Examples:
  python3 scripts/analysis/propose-library-units.py --archive vc7-libcmt --min-size 0x20
  python3 scripts/analysis/propose-library-units.py --archive vc7-libcmt --json
"""
from __future__ import annotations

import argparse
import collections
import csv
import hashlib
import json
from pathlib import Path
import struct
import sys
import tomllib

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from coff import ObjectModule  # noqa: E402
from library_match import archive_members, pe_bytes_at, verify_archive  # noqa: E402

SUPPORTED_RELOCATIONS = {0x0006: "DIR32", 0x0014: "REL32"}


def load_target() -> bytes:
    data = (ROOT / "resources" / "th08.exe").read_bytes()
    expected = tomllib.loads((ROOT / "config" / "target.toml").read_text())["target"]
    digest = hashlib.sha256(data).hexdigest()
    if len(data) != int(expected["size"]) or digest != expected["sha256"]:
        raise ValueError(f"target identity mismatch: size={len(data)} sha256={digest}")
    return data


def relocation_suggestion(
    *, target_address: int, target_code: bytes, object_code: bytes, offset: int,
    type_id: int, symbol: str,
) -> dict[str, object]:
    addend_u = struct.unpack_from("<I", object_code, offset)[0]
    target_field_u = struct.unpack_from("<I", target_code, offset)[0]
    if type_id == 0x0006:
        target_base = (target_field_u - addend_u) & 0xFFFFFFFF
    elif type_id == 0x0014:
        addend = struct.unpack("<i", struct.pack("<I", addend_u))[0]
        target_field = struct.unpack("<i", struct.pack("<I", target_field_u))[0]
        target_base = (
            target_address + offset + 4 + target_field - addend
        ) & 0xFFFFFFFF
    else:
        raise ValueError(f"unsupported relocation type {type_id:#x}")
    return {
        "offset": offset,
        "type": SUPPORTED_RELOCATIONS[type_id],
        "symbol": symbol,
        "target_base": target_base,
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "--archive", required=True, help="archive id from library-provenance.toml"
    )
    parser.add_argument("--min-size", type=lambda value: int(value, 0), default=0x20)
    parser.add_argument("--include-configured", action="store_true")
    parser.add_argument("--show-ambiguous", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    provenance = tomllib.loads(
        (ROOT / "config" / "library-provenance.toml").read_text(encoding="utf-8")
    )
    archives = {entry["id"]: entry for entry in provenance.get("archives", [])}
    if args.archive not in archives:
        parser.error(f"unknown archive id: {args.archive}")
    archive = archives[args.archive]
    archive_data = verify_archive(ROOT / archive["path"], archive["sha256"])
    members = archive_members(archive_data)
    target = load_target()

    mapping: dict[int, tuple[str, int]] = {}
    with (ROOT / "config" / "mapping.csv").open(newline="", encoding="utf-8") as stream:
        for row in csv.reader(stream):
            mapping[int(row[1], 0)] = (row[0], int(row[2], 0))

    inventory: list[tuple[int, str, int]] = []
    with (ROOT / "config" / "reccmp-functions.csv").open(
        newline="", encoding="utf-8"
    ) as stream:
        for row in csv.DictReader(stream):
            if row["type"] != "library":
                continue
            address = int(row["address"], 0)
            if address in mapping and mapping[address][1] >= args.min_size:
                inventory.append((address, row["name"], mapping[address][1]))

    by_size: dict[int, list[tuple[int, str, int]]] = collections.defaultdict(list)
    for row in inventory:
        by_size[row[2]].append(row)

    manifest = tomllib.loads(
        (ROOT / "config" / "library-match-units.toml").read_text(encoding="utf-8")
    )
    configured = {int(unit["target_address"]) for unit in manifest.get("units", [])}

    proposals: list[dict[str, object]] = []
    functions_considered = 0
    for member_name, occurrences in members.items():
        for occurrence, member_data in enumerate(occurrences):
            try:
                module = ObjectModule()
                module.unpack(member_data)
            except Exception:
                continue
            for symbol in module.symbols:
                if symbol.section_number <= 0 or symbol.type != 0x20:
                    continue
                aux = (
                    symbol.aux_records[0]
                    if symbol.aux_records
                    and hasattr(symbol.aux_records[0], "total_size")
                    else None
                )
                if aux is None:
                    continue
                size = int(aux.total_size)
                if size < args.min_size or size not in by_size:
                    continue
                section = module.sections[symbol.section_number - 1]
                if section.data is None or symbol.value + size > len(section.data):
                    continue
                try:
                    symbol_name = symbol.get_name(module.string_table).decode("ascii")
                except Exception:
                    continue
                object_code = section.data[symbol.value : symbol.value + size]
                relocations: list[tuple[int, int, str]] = []
                supported = True
                for relocation in section.relocations:
                    offset = relocation.virtual_address - symbol.value
                    if not 0 <= offset < size:
                        continue
                    if relocation.type not in SUPPORTED_RELOCATIONS or offset + 4 > size:
                        supported = False
                        break
                    referenced = module.symbols[relocation.symbol_table_index]
                    relocations.append(
                        (
                            offset,
                            relocation.type,
                            referenced.get_name(module.string_table).decode(
                                "ascii", errors="strict"
                            ),
                        )
                    )
                if not supported:
                    continue
                functions_considered += 1
                relocation_bytes: set[int] = set()
                for offset, _, _ in relocations:
                    relocation_bytes.update(range(offset, offset + 4))

                for address, target_name, _ in by_size[size]:
                    if not args.include_configured and address in configured:
                        continue
                    target_code = pe_bytes_at(target, address, size)
                    if any(
                        index not in relocation_bytes and object_code[index] != target_code[index]
                        for index in range(size)
                    ):
                        continue
                    suggested_relocations = [
                        relocation_suggestion(
                            target_address=address,
                            target_code=target_code,
                            object_code=object_code,
                            offset=offset,
                            type_id=type_id,
                            symbol=relocation_symbol,
                        )
                        for offset, type_id, relocation_symbol in relocations
                    ]
                    proposals.append(
                        {
                            "target_address": address,
                            "target_name": target_name,
                            "body_size": size,
                            "archive": args.archive,
                            "member": member_name,
                            "occurrence": occurrence,
                            "symbol": symbol_name,
                            "section_offset": int(symbol.value),
                            "section_size": len(section.data),
                            "relocations": suggested_relocations,
                        }
                    )

    by_address: dict[int, list[dict[str, object]]] = collections.defaultdict(list)
    for proposal in proposals:
        by_address[int(proposal["target_address"])].append(proposal)
    unique = [items[0] for items in by_address.values() if len(items) == 1]
    ambiguous = {address: items for address, items in by_address.items() if len(items) > 1}

    if args.json:
        print(
            json.dumps(
                {
                    "archive": args.archive,
                    "functions_considered": functions_considered,
                    "unique": sorted(unique, key=lambda row: int(row["target_address"])),
                    "ambiguous": {
                        f"0x{address:08X}": items
                        for address, items in sorted(ambiguous.items())
                    }
                    if args.show_ambiguous
                    else {},
                },
                indent=2,
            )
        )
        return 0

    print(
        f"archive={args.archive} functions_considered={functions_considered} "
        f"unique={len(unique)} ambiguous={len(ambiguous)}"
    )
    for proposal in sorted(unique, key=lambda row: int(row["target_address"])):
        shared = ""
        if int(proposal["section_offset"]) != 0 or int(proposal["section_size"]) != int(
            proposal["body_size"]
        ):
            shared = (
                f" section_offset={int(proposal['section_offset']):#x}"
                f" section_size={int(proposal['section_size']):#x}"
            )
        print(
            f"0x{int(proposal['target_address']):08X}\t"
            f"{int(proposal['body_size']):#x}\t{proposal['target_name']}\t"
            f"{proposal['member']}#{proposal['occurrence']}\t{proposal['symbol']}\t"
            f"relocs={len(proposal['relocations'])}{shared}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
