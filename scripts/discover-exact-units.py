#!/usr/bin/env python3
"""Discover conservative, review-only exact COFF function candidates.

This never changes mapping, match units, or progress.  It verifies the pinned
target, then emits only isolated VC7 functions whose target extent, relocation
replay, direct calls, ledgers, and unique object owner all agree.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import json
from pathlib import Path
import struct
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from coff import ObjectModule  # noqa: E402
from generate_objdiff_objs import demangle_msvc  # noqa: E402


DIR32 = 0x0006
REL32 = 0x0014


def fail(message: str) -> None:
    raise ValueError(message)


def symbol_name(module: ObjectModule, symbol) -> str:
    return symbol.get_name(module.string_table).decode("ascii", errors="strict")


def verify_target(path: Path) -> bytes:
    manifest = tomllib.loads((ROOT / "config" / "target.toml").read_text(encoding="utf-8"))["target"]
    data = path.read_bytes()
    if len(data) != int(manifest["size"]) or hashlib.sha256(data).hexdigest() != manifest["sha256"]:
        fail("target identity mismatch")
    return data


def pe_bytes_at(data: bytes, address: int, size: int) -> bytes:
    if data[:2] != b"MZ":
        fail("target is not a PE image")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe : pe + 4] != b"PE\0\0":
        fail("target lacks PE signature")
    count, optional_size = struct.unpack_from("<H12xH", data, pe + 6)
    optional = pe + 24
    image_base = struct.unpack_from("<I", data, optional + 28)[0]
    rva = address - image_base
    for index in range(count):
        off = optional + optional_size + index * 40
        _name, virtual_size, section_rva, raw_size, raw_off = struct.unpack_from("<8sIIII", data, off)
        if section_rva <= rva and rva + size <= section_rva + max(virtual_size, raw_size):
            relative = rva - section_rva
            if relative + size > raw_size:
                fail(f"target extent {address:#x}+{size:#x} leaves raw section")
            return data[raw_off + relative : raw_off + relative + size]
    fail(f"target extent {address:#x}+{size:#x} is outside PE sections")


def decode(code: bytes, address: int) -> tuple[Counter[int], set[int], str]:
    from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_MODE_32, Cs
    from capstone.x86_const import X86_OP_IMM

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(code, address))
    if not instructions or sum(insn.size for insn in instructions) != len(code):
        fail(f"{address:#x}: incomplete decode")
    calls: Counter[int] = Counter()
    call_fields: set[int] = set()
    for insn in instructions:
        if insn.group(CS_GRP_CALL) and insn.operands and insn.operands[0].type == X86_OP_IMM:
            calls[insn.operands[0].imm & 0xFFFFFFFF] += 1
            call_fields.add(insn.address - address + insn.imm_offset)
    return calls, call_fields, instructions[-1].mnemonic


def read_mapping() -> tuple[dict[str, list[tuple[int, int]]], list[tuple[int, int, str]]]:
    by_name: dict[str, list[tuple[int, int]]] = defaultdict(list)
    rows: list[tuple[int, int, str]] = []
    with (ROOT / "config" / "mapping.csv").open(newline="", encoding="utf-8-sig") as stream:
        for row in csv.reader(stream):
            if len(row) >= 3:
                address, size = int(row[1], 0), int(row[2], 0)
                by_name[row[0]].append((address, size))
                rows.append((address, size, row[0]))
    return by_name, rows


def function_ledger() -> set[tuple[str, int]]:
    path = ROOT / "config" / "reccmp-functions.csv"
    with path.open(newline="", encoding="utf-8-sig") as stream:
        return {(row["name"], int(row["address"], 0)) for row in csv.DictReader(stream) if row["type"] == "function"}


def relocation_starts(mapping_rows: list[tuple[int, int, str]]) -> set[int]:
    starts = {address for address, _size, _name in mapping_rows}
    for path in sorted((ROOT / "config").glob("reccmp-*.csv")):
        with path.open(newline="", encoding="utf-8-sig") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames and "address" in reader.fieldnames:
                starts.update(int(row["address"], 0) for row in reader if row.get("address"))
    return starts


def read_single_column(name: str) -> set[str]:
    with (ROOT / "config" / name).open(newline="", encoding="utf-8-sig") as stream:
        return {row[0] for row in csv.reader(stream) if row}


def matched() -> tuple[set[int], set[str]]:
    with (ROOT / "config" / "matches.csv").open(newline="", encoding="utf-8-sig") as stream:
        rows = list(csv.DictReader(stream))
    return ({int(row["address"], 0) for row in rows}, {row["name"] for row in rows})


def owner_index(paths: list[Path]) -> dict[str, set[str]]:
    owners: dict[str, set[str]] = defaultdict(set)
    for path in paths:
        module = ObjectModule()
        try:
            module.unpack(path.read_bytes())
        except (IndexError, ValueError, struct.error):
            continue
        for symbol in module.symbols:
            if symbol.section_number > 0:
                owners[symbol_name(module, symbol)].add(path.name)
    return owners


def logical_name(decorated: str) -> str | None:
    result = demangle_msvc(decorated.encode("ascii"))
    text = result.decode("ascii", errors="strict")
    return text if text.startswith("th08::") else None


def evaluate(
    module: ObjectModule,
    object_name: str,
    symbol,
    logical: str,
    address: int,
    size: int,
    target_data: bytes,
    known_starts: set[int],
) -> tuple[list[dict[str, int | str]], int] | None:
    if symbol.section_number <= 0 or not symbol.aux_records:
        return None
    section = module.sections[symbol.section_number - 1]
    emitted_size = int(symbol.aux_records[0].total_size)
    if section.data is None or symbol.value != 0 or emitted_size != size or len(section.data) != size:
        return None
    original = bytes(section.data)
    target = pe_bytes_at(target_data, address, size)
    target_calls, call_fields, target_tail = decode(target, address)
    _object_calls, _unused, object_tail = decode(original, address)
    if target_tail != "ret" or object_tail != "ret":
        return None
    replay = bytearray(original)
    covered: set[int] = set()
    rows: list[dict[str, int | str]] = []
    coff_calls: Counter[int] = Counter()
    for relocation in section.relocations:
        offset = relocation.virtual_address
        if offset < 0 or offset + 4 > size or any(index in covered for index in range(offset, offset + 4)):
            return None
        covered.update(range(offset, offset + 4))
        raw_name = symbol_name(module, module.symbols[relocation.symbol_table_index])
        if raw_name.startswith("$L") or raw_name == "__except_list" or any(token in raw_name.lower() for token in ("ehhandler", "unwind", "tryblock", "handlermap", "ip2state")):
            return None
        addend = struct.unpack_from("<I", original, offset)[0]
        target_field = struct.unpack_from("<I", target, offset)[0]
        next_instruction = address + offset + 4
        if relocation.type == DIR32:
            kind = "DIR32"
            destination = (target_field - addend) & 0xFFFFFFFF
            resolved = destination + addend
        elif relocation.type == REL32:
            kind = "REL32"
            destination = (target_field + next_instruction - addend) & 0xFFFFFFFF
            resolved = destination + addend - next_instruction
            if offset in call_fields:
                coff_calls[destination] += 1
        else:
            return None
        if destination not in known_starts:
            return None
        struct.pack_into("<I", replay, offset, resolved & 0xFFFFFFFF)
        rows.append({"offset": offset, "type": kind, "symbol": raw_name, "target": destination})
    if coff_calls != target_calls or replay != target:
        return None
    return sorted(rows, key=lambda row: int(row["offset"])), sum(target_calls.values())


def render(candidates: list[dict[str, object]], target_sha: str) -> str:
    lines = [
        "# REVIEW ARTIFACT: candidates only; do not count or accept without canonical comparison.",
        f"# Target: Japanese TH08 1.00d SHA-256 {target_sha}",
        "# Gates: isolated COFF extent, exact replay, direct-call multiset, named ledger destinations, unique owner, no local/EH relocations.",
        f"# Candidates: {len(candidates)} functions / {sum(int(row['size']) for row in candidates)} authored bytes.",
        "",
    ]
    for candidate in candidates:
        lines.extend((
            f"# Ledger name: {candidate['logical']}",
            f"# Evidence: {len(candidate['relocations'])} relocations; {candidate['calls']} direct calls; full replay exact; implemented ledger={candidate['implemented']}.",
            "[[units]]",
            f"name = \"discovered-exact-{int(candidate['address']):08x}\"",
            f"object = {json.dumps(str(candidate['object']))}",
            f"symbol = {json.dumps(str(candidate['symbol']))}",
            f"target_address = 0x{int(candidate['address']):08X}",
            f"size = 0x{int(candidate['size']):X}",
            "",
        ))
        for relocation in candidate["relocations"]:
            lines.extend((
                "[[units.relocations]]",
                f"offset = 0x{int(relocation['offset']):X}",
                f"type = {json.dumps(str(relocation['type']))}",
                f"symbol = {json.dumps(str(relocation['symbol']))}",
                f"target = 0x{int(relocation['target']):08X}",
                "",
            ))
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--object", action="append", dest="objects", help="build object name or repository-relative path; repeatable")
    parser.add_argument("--min-size", type=lambda value: int(value, 0), default=1)
    parser.add_argument("--include-implemented", action="store_true", help="also print existing exact entries (for scanner validation)")
    parser.add_argument("--allow-unimplemented", action="store_true", help="emit source-built candidates absent from implemented.csv for coordinator review")
    parser.add_argument("--output", type=Path, help="write a review-only TOML artifact")
    args = parser.parse_args()
    if args.min_size <= 0:
        fail("--min-size must be positive")
    target_path = ROOT / "resources" / "th08.exe"
    target_data = verify_target(target_path)
    target_sha = hashlib.sha256(target_data).hexdigest()
    paths = [ROOT / value if not Path(value).is_absolute() else Path(value) for value in args.objects] if args.objects else sorted((ROOT / "build").glob("*.obj"))
    paths = [path.resolve() for path in paths if path.is_file()]
    if not paths:
        fail("no usable objects")
    mapping, mapping_rows = read_mapping()
    functions = function_ledger()
    known_starts = relocation_starts(mapping_rows)
    implemented = read_single_column("implemented.csv")
    matched_addresses, matched_names = matched()
    owners = owner_index(sorted((ROOT / "build").glob("*.obj")))
    candidates: list[dict[str, object]] = []
    skipped = Counter()
    for path in paths:
        module = ObjectModule()
        try:
            module.unpack(path.read_bytes())
        except (IndexError, ValueError, struct.error) as error:
            skipped[f"unreadable:{path.name}"] += 1
            continue
        for symbol in module.symbols:
            if symbol.section_number <= 0 or not symbol.aux_records:
                continue
            decorated = symbol_name(module, symbol)
            logical = logical_name(decorated)
            if logical is None:
                continue
            pairs = mapping.get(logical, [])
            if len(pairs) != 1:
                skipped["no_unique_mapping"] += 1
                continue
            address, size = pairs[0]
            if size < args.min_size or (logical, address) not in functions:
                continue
            if not args.include_implemented and (address in matched_addresses or logical in matched_names):
                continue
            if logical not in implemented and not args.allow_unimplemented:
                skipped["not_implemented"] += 1
                continue
            if owners.get(decorated) != {path.name}:
                skipped["non_unique_owner"] += 1
                continue
            result = evaluate(module, path.name, symbol, logical, address, size, target_data, known_starts)
            if result is None:
                skipped["evidence_gate"] += 1
                continue
            relocations, calls = result
            candidates.append({"object": f"build/{path.name}", "symbol": decorated, "logical": logical, "address": address, "size": size, "relocations": relocations, "calls": calls, "implemented": logical in implemented})
    candidates.sort(key=lambda row: (-int(row["size"]), int(row["address"])))
    report = render(candidates, target_sha)
    if args.output:
        output = args.output if args.output.is_absolute() else ROOT / args.output
        output.resolve().relative_to(ROOT)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(report, encoding="utf-8", newline="\n")
    print(f"exact review candidates: {len(candidates)} functions / {sum(int(row['size']) for row in candidates)} bytes")
    for candidate in candidates:
        print(f"0x{int(candidate['address']):08X} {int(candidate['size']):#x} {candidate['logical']} ({candidate['object']})")
    if skipped:
        print("skipped: " + ", ".join(f"{key}={value}" for key, value in sorted(skipped.items())))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, struct.error, tomllib.TOMLDecodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
