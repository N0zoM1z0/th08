#!/usr/bin/env python3
"""Compare one VC7 COFF function with the hash-attested TH08 target."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shlex
import struct
import sys
import tomllib

from coff import ObjectModule
from match_literals import real_literal_bytes
from match_callees import validate_math_callee
from match_inputs import validate_input_global
from match_data import DataError, file_identity, literal_bytes, parse_publics
from match_evidence import build_command, scopes
from pe_image import PEImage


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "config" / "match-units.toml"
TARGET_MANIFEST = ROOT / "config" / "target.toml"
RELOCATION_TYPES = {"DIR32": 0x0006, "REL32": 0x0014}


def stable_namespace(name: str) -> str:
    # VC7's anonymous-namespace discriminator changes with the checkout path.
    # Preserve every other component of the decorated name and its signature.
    return re.sub(r"(?<=@)\?A0x[0-9a-fA-F]{8}(?=@)", "?A0x*", name)


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
    path: Path,
    decorated_name: str,
    *,
    allow_auxless_comdat: bool = False,
    expected_size: int | None = None,
    identity: dict[str, str] | None = None,
) -> tuple[bytearray, list[dict[str, object]]]:
    module = ObjectModule()
    module.unpack(path.read_bytes())
    matches = []
    for symbol in module.symbols:
        name = symbol.get_name(module.string_table).decode("ascii", errors="strict")
        if stable_namespace(name) == stable_namespace(decorated_name) and symbol.section_number > 0:
            matches.append(symbol)
    if len(matches) != 1:
        raise ValueError(
            f"expected one section-defined COFF symbol {decorated_name!r}, found {len(matches)}"
        )
    symbol = matches[0]
    if identity is not None:
        identity["object_symbol"] = symbol.get_name(module.string_table).decode("ascii", errors="strict")
    section = module.sections[symbol.section_number - 1]
    function_aux = (
        symbol.aux_records[0]
        if symbol.aux_records and hasattr(symbol.aux_records[0], "total_size")
        else None
    )
    if function_aux is not None:
        size = int(function_aux.total_size)
    else:
        if not allow_auxless_comdat:
            raise ValueError(f"COFF symbol lacks a function definition: {decorated_name}")
        if expected_size is None:
            raise ValueError("auxless COMDAT comparison requires an expected size")
        if section.data is None:
            raise ValueError("auxless COMDAT section has no raw data")
        # This opt-in is reserved for compiler-generated isolated COMDAT bodies
        # such as scalar/vector deleting destructors and VC7 iterator helpers.
        # Require a code COMDAT section, an offset-zero function symbol, and
        # exactly one external offset-zero function owner.  The manifest
        # comparison extent must consume the entire section, so section bounds
        # cannot silently absorb adjacent authored code.
        IMAGE_SCN_CNT_CODE = 0x00000020
        IMAGE_SCN_LNK_COMDAT = 0x00001000
        if not (section.flags & IMAGE_SCN_CNT_CODE) or not (section.flags & IMAGE_SCN_LNK_COMDAT):
            raise ValueError("auxless symbol is not in a code COMDAT section")
        if symbol.value != 0:
            raise ValueError("auxless COMDAT function does not begin at section offset zero")
        external_owners = []
        for candidate in module.symbols:
            if (
                candidate.section_number == symbol.section_number
                and candidate.value == 0
                and candidate.storage_class == 2
                and candidate.type == 0x20
            ):
                external_owners.append(
                    candidate.get_name(module.string_table).decode("ascii", errors="strict")
                )
        if [stable_namespace(name) for name in external_owners] != [stable_namespace(decorated_name)]:
            raise ValueError(
                f"auxless COMDAT section has ambiguous external owners: {external_owners!r}"
            )
        if len(section.data) != expected_size:
            raise ValueError(
                f"auxless COMDAT section size {len(section.data):#x} differs from "
                f"manifest comparison extent {expected_size:#x}"
            )
        size = expected_size
    if section.data is None or symbol.value + size > len(section.data):
        raise ValueError("COFF function extends beyond its section")
    function = bytearray(section.data[symbol.value : symbol.value + size])
    relocations = []
    for relocation in section.relocations:
        local_offset = relocation.virtual_address - symbol.value
        if 0 <= local_offset < size:
            referenced = module.symbols[relocation.symbol_table_index]
            record = {
                    "offset": local_offset,
                    "type_id": relocation.type,
                    "symbol": referenced.get_name(module.string_table).decode(
                        "ascii", errors="strict"
                    ),
                    "local_symbol_offset": referenced.value - symbol.value
                    if referenced.section_number == symbol.section_number else None,
                }
            if relocation.type == 6:
                addend = struct.unpack_from("<I", function, local_offset)[0]
                try:
                    data = literal_bytes(module, referenced, addend)
                    if data is not None:
                        record["literal_hex"] = data.hex()
                except (ValueError, IndexError, struct.error) as exc:
                    record["literal_error"] = str(exc)
            relocations.append(record)
    return function, relocations


def apply_relocations(
    code: bytearray,
    actual: list[dict[str, object]],
    expected: list[dict[str, object]],
    target_address: int,
    target_code: bytes,
) -> list[dict[str, object]]:
    def stable_symbol(name: str) -> str:
        # VC7 renumbers compiler-owned local labels when an earlier function in
        # the same translation unit changes. Their relocation offset and
        # resolved target are stable evidence; the generated numeric suffix is
        # not. Preserve a named local label's semantic stem (for example
        # $updateBullet$) while ignoring only its unstable trailing number.
        if name.startswith("$L"):
            return "$L*"
        if name.startswith("$"):
            stem, separator, suffix = name.rpartition("$")
            if separator and stem and suffix.isdigit():
                return stem + "$*"
        return stable_namespace(name)

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
        (
            int(row["offset"]),
            int(row["type_id"]),
            stable_symbol(str(row["symbol"])),
        )
        for row in actual
    )
    expected_key = sorted(
        (row["offset"], row["type_id"], stable_symbol(row["symbol"]))
        for row in normalized_expected
    )
    if actual_key != expected_key:
        raise ValueError(
            f"COFF relocations differ from manifest: actual={actual_key!r} "
            f"expected={expected_key!r}"
        )

    report = []
    actual_by_offset = {int(row["offset"]): row for row in actual}
    for relocation in normalized_expected:
        offset = relocation["offset"]
        if offset < 0 or offset + 4 > len(code):
            raise ValueError(f"relocation offset outside function: {offset:#x}")
        object_field_before = struct.unpack_from("<I", code, offset)[0]
        addend = object_field_before
        # Internal labels must keep their actual offset in the COFF function.
        # Substituting a manifest address here can conceal a wrong case entry.
        local_offset = actual_by_offset[offset].get("local_symbol_offset")
        if local_offset is not None and target_address <= relocation["target"] < target_address + len(code):
            actual_target = target_address + int(local_offset)
            if actual_target != relocation["target"]:
                raise ValueError(
                    f"local relocation at {offset:#x} resolves to {actual_target:#x}, "
                    f"manifest declares {relocation['target']:#x}"
                )
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
                "object_symbol": str(actual_by_offset[offset]["symbol"]),
                "target": f"0x{relocation['target']:08X}",
                "addend": f"0x{addend:08X}",
                "addend_signed": struct.unpack("<i", struct.pack("<I", addend))[0],
                "object_field_before": f"0x{object_field_before:08X}",
                "resolved_value": f"0x{value & 0xFFFFFFFF:08X}",
                "target_field": f"0x{struct.unpack_from('<I', target_code, offset)[0]:08X}",
            }
        )
    return report


def attest_relocation_literals(
    expected: list[dict[str, object]], target_data: bytes
) -> list[dict[str, object]]:
    report = []
    for index, relocation in enumerate(expected, start=1):
        try:
            declared = real_literal_bytes(relocation)
        except (TypeError, ValueError) as exc:
            raise ValueError(f"relocation {index}: {exc}") from exc
        if declared is None:
            continue
        address = int(relocation["target"])
        actual = pe_bytes_at(target_data, address, len(declared))
        if actual != declared:
            raise ValueError(
                f"relocation {index}: target literal at 0x{address:08X} is "
                f"{actual.hex()}, not declared {declared.hex()}"
            )
        report.append(
            {
                "symbol": str(relocation["symbol"]),
                "target": f"0x{address:08X}",
                "data_hex": declared.hex(),
                "recorded": "data_hex" in relocation,
            }
        )
    return report


def compare(unit: dict[str, object], target_path: Path, *, linked=None, details=False) -> dict[str, object]:
    target_data = verify_target(target_path)
    object_path = repository_path(str(unit["object"]))
    coverage_size = int(unit["size"])
    compare_size = int(unit.get("compare_size", coverage_size))
    if compare_size < coverage_size:
        raise ValueError(
            f"comparison extent {compare_size:#x} is smaller than coverage "
            f"size {coverage_size:#x}"
        )
    identity: dict[str, str] = {}
    code, actual_relocations = object_function(
        object_path,
        str(unit["symbol"]),
        allow_auxless_comdat=bool(unit.get("allow_auxless_comdat", False)),
        expected_size=compare_size,
        identity=identity,
    )
    if len(code) != compare_size:
        raise ValueError(
            f"object function size {len(code):#x} differs from manifest "
            f"comparison extent {compare_size:#x}"
        )
    target_address = int(unit["target_address"])
    target = pe_bytes_at(target_data, target_address, compare_size)
    expected_relocations = list(unit.get("relocations", []))
    for relocation in expected_relocations:
        validate_math_callee(relocation)
        validate_input_global(relocation)
    literal_attestations = attest_relocation_literals(
        expected_relocations, target_data
    )
    raw_code = bytes(code)
    relocations = apply_relocations(
        code,
        actual_relocations,
        expected_relocations,
        target_address,
        target,
    )
    differences = [
        {"offset": f"0x{index:X}", "object": left, "target": right}
        for index, (left, right) in enumerate(zip(code, target))
        if left != right
    ]
    coverage_differences = sum(
        int(row["offset"], 0) < coverage_size for row in differences
    )
    code_result = "exact" if not differences else "mismatch"
    target_image = PEImage.from_bytes(target_data, target_path)
    evidence = scopes(unit, target_image, raw_code,
                      actual_relocations, relocations, code_result, identity["object_symbol"],
                      linked=linked, details=details)
    failed = code_result != "exact" or any(evidence[scope]["status"] == "mismatch" for scope in ("data", "linkage"))
    findings = list(evidence["data"]["findings"])
    if evidence["linkage"]["status"] in {"mismatch", "partial"}:
        findings.extend(evidence["linkage"].get("findings", []))
    next_commands = evidence["data"]["next_commands"] + evidence["linkage"].get("next_commands", [])
    for proof in evidence["data"]["contracts"]:
        if proof["result"] != "verified":
            next_commands.extend([build_command(proof["object"]), ["python3", "scripts/compare-data.py", proof["contract"], "--json"]])
    return {
        "schema_version": 2,
        "target_sha256": target_image.sha256,
        "object_identity": file_identity(object_path),
        "unit": unit["name"],
        "result": "mismatch" if failed else "exact",
        "result_scope": "configured code plus performed literal/declared-data/link checks; uncovered dependencies are not passes",
        "target_address": f"0x{target_address:08X}",
        "size": coverage_size,
        "matched_bytes": coverage_size - coverage_differences,
        "compared_size": compare_size,
        "matched_compared_bytes": compare_size - len(differences),
        "object": str(object_path.relative_to(ROOT)),
        "symbol": unit["symbol"],
        "object_symbol": identity["object_symbol"],
        "relocations": relocations,
        "literal_attestations": literal_attestations,
        "first_differences": differences[:16],
        "evidence": evidence,
        "findings": findings,
        "next_commands": next_commands,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("unit", nargs="?", help="unit name; omit with --list")
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--filter", help="narrow --list by unit, object or COFF symbol")
    parser.add_argument("--limit", type=int, help="--list page size; JSON defaults to 40")
    parser.add_argument("--offset", type=int, default=0, help="--list pagination offset")
    parser.add_argument("--target", type=Path, default=ROOT / "resources" / "th08.exe")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--details", action="store_true", help="include every data gap and literal observation")
    parser.add_argument("--rebuild", type=Path, help="also verify the final linked PE32 function and contracted data")
    parser.add_argument("--map", dest="map_path", type=Path, help="linker map paired with --rebuild")
    parser.add_argument("--require-data", action="store_true", help="exit 2 if direct initialized-data coverage is incomplete")
    parser.add_argument("--require-linkage", action="store_true", help="exit 2 unless linked code and declared data are verified")
    args = parser.parse_args()
    units = {}
    try:
        units = load_units()
        if args.list:
            if args.offset < 0 or args.limit is not None and args.limit <= 0:
                raise DataError("REQUEST_INVALID", "--offset must be nonnegative and --limit positive")
            query = (args.filter or "").casefold()
            found = sorted((unit for unit in units.values() if query in " ".join(str(unit[key]) for key in ("name", "object", "symbol")).casefold()), key=lambda unit: (unit["target_address"], unit["name"]))
            limit = args.limit or (40 if args.json else len(found))
            page = found[args.offset:args.offset + limit]
            if args.json:
                print(json.dumps({"schema_version": 2, "result": "listed", "total": len(found), "offset": args.offset,
                                  "next_offset": args.offset + len(page) if args.offset + len(page) < len(found) else None,
                                  "units": [{key: row[key] for key in ("name", "object", "target_address", "size")} for row in page]}, indent=2))
            else:
                print("\n".join(row["name"] for row in page))
            return 0
        if args.filter or args.limit is not None or args.offset:
            raise DataError("REQUEST_INVALID", "--filter, --limit and --offset are list options")
        if not args.unit or args.unit not in units:
            raise DataError("UNIT_UNKNOWN", f"unknown or missing unit: {args.unit!r}")
        if bool(args.rebuild) != bool(args.map_path):
            raise DataError("REQUEST_INVALID", "--rebuild and --map must be supplied together")
        linked = (PEImage(args.rebuild), parse_publics(args.map_path)) if args.rebuild else None
        report = compare(units[args.unit], args.target.expanduser().resolve(), linked=linked, details=args.details)
        if linked is not None:
            report["linked_inputs"] = {"image": file_identity(args.rebuild), "map": file_identity(args.map_path)}
        if report["result"] == "exact" and (
            args.require_data and report["evidence"]["data"]["status"] != "verified"
            or args.require_linkage and report["evidence"]["linkage"]["status"] != "verified"
        ):
            report["result"] = "incomplete"
    except (OSError, KeyError, IndexError, TypeError, ValueError, struct.error) as exc:
        next_commands = ([build_command(units[args.unit]["object"]), ["python3", "scripts/compare-function.py", args.unit, "--json"]]
                         if args.unit in units else [["python3", "scripts/compare-function.py", "--list", "--json"]])
        report = {"schema_version": 2, "unit": args.unit, "result": "error", "error": str(exc),
                  "findings": [exc.finding if isinstance(exc, DataError) else {"code": "COMPARISON_ERROR", "message": str(exc)}],
                  "next_commands": next_commands}
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
        if "evidence" in report:
            evidence = report["evidence"]
            print(f"  code={evidence['code']['status']} data={evidence['data']['status']} linkage={evidence['linkage']['status']}; "
                  f"{evidence['data']['uncovered_count']} uncovered initialized-data dependencies")
        for finding in report.get("findings", [])[:3]:
            print(f"  {finding['code']}: {finding['message']}")
        for command in report.get("next_commands", [])[:3]:
            print("  next: " + shlex.join(command))
    return 0 if report["result"] == "exact" else 2 if report["result"] == "incomplete" else 1


if __name__ == "__main__":
    raise SystemExit(main())
