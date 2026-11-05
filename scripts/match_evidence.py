"""Scoped evidence reports; unknown dependencies stay visible and unmasked."""

from __future__ import annotations

import struct

from match_data import (
    DataError,
    check,
    linked_address,
    load_contracts,
    load_object,
    owner,
    read_va,
    repository_path,
    stable_symbol,
)
from pe_image import PEImage


def build_command(object_path: str) -> list[str]:
    return ["python3", "scripts/build.py", "--build-type=objdiffbuild", object_path]


def data_scope(
    unit: dict,
    target: PEImage,
    relocations: list[dict],
    actual: list[dict],
    *,
    linked=None,
    details: bool = False,
) -> dict:
    contracts = load_contracts()
    by_offset = {row["offset"]: row for row in actual}
    proofs, literals, findings, gaps, excluded = {}, [], [], {}, {}
    for row in relocations:
        if row["type"] != "DIR32":
            continue
        offset = int(row["offset"], 0)
        source = by_offset[offset]
        address = int(row["resolved_value"], 0)
        symbol = row["object_symbol"]
        if source.get("literal_error"):
            findings.append(
                {
                    "code": "LITERAL_SOURCE_INVALID",
                    "message": source["literal_error"],
                    "offset": row["offset"],
                    "symbol": symbol,
                }
            )
            continue
        if "literal_hex" in source:
            expected = bytes.fromhex(source["literal_hex"])
            try:
                observed = read_va(target, address, len(expected))
                if observed != expected:
                    first = next(
                        i
                        for i, pair in enumerate(zip(expected, observed))
                        if pair[0] != pair[1]
                    )
                    raise DataError(
                        "LITERAL_CONTENT_MISMATCH",
                        "COFF literal differs from the target operand's data",
                        first_difference=first,
                        target_address=f"0x{address:08X}",
                        expected_hex=observed[first : first + 16].hex(),
                        actual_hex=expected[first : first + 16].hex(),
                    )
                literals.append(
                    {
                        "symbol": symbol,
                        "target_address": f"0x{address:08X}",
                        "size": len(expected),
                        "status": "verified",
                    }
                )
            except ValueError as exc:
                finding = (
                    exc.finding
                    if isinstance(exc, DataError)
                    else {"code": "LITERAL_TARGET_INVALID", "message": str(exc)}
                )
                findings.append({"offset": row["offset"], "symbol": symbol, **finding})
            continue
        candidates = [
            contract
            for contract in contracts
            if stable_symbol(contract["symbol"]) == stable_symbol(symbol)
            and contract["target_address"] == int(row["target"], 0)
            and contract["target_address"]
            <= address
            < contract["target_address"] + contract["size"]
        ]
        if candidates:
            contract = candidates[0]
            if contract["name"] not in proofs:
                # Probe copies must validate their own initializer, not a good
                # production object that happens to share the same symbol.
                source_path = repository_path(unit["object"])
                try:
                    owner(load_object(source_path), contract["symbol"])
                except DataError as exc:
                    if exc.finding["code"] != "DATA_OWNER_MISSING":
                        raise
                    source_path = repository_path(contract["object"])
                proof = check(contract, target, object_path=source_path, linked=linked)
                proof["checked_object"] = str(
                    source_path.relative_to(repository_path("."))
                )
                proofs[contract["name"]] = proof
                findings.extend(proof["findings"])
            continue
        section = next(
            (
                section
                for section in target.sections
                if section.rva
                <= address - target.image_base
                < section.rva + max(section.virtual_size, section.raw_size)
            ),
            None,
        )
        if section is not None and section.characteristics & 0x20:
            continue  # Code labels/function pointers are instruction/link evidence.
        if symbol.startswith("__imp_") or address == 0:
            kind = "import_or_abi_address"
        elif (
            section is not None
            and address - target.image_base - section.rva >= section.raw_size
        ):
            kind = "runtime_zero_fill_region"
        else:
            kind = None
        key = (stable_symbol(symbol), address)
        table = excluded if kind else gaps
        if key not in table:
            table[key] = {
                "symbol": symbol,
                "target_address": f"0x{address:08X}",
                "offsets": [],
                "reference_count": 0,
            }
            if kind:
                table[key]["reason"] = kind
            else:
                table[key].update(
                    code="INITIALIZED_DATA_UNCOVERED",
                    next_action="Review the target extent and COFF owner; add an explicit data contract. No extent is inferred automatically.",
                    configuration="config/data-contracts.toml",
                )
        table[key]["reference_count"] += 1
        if details or len(table[key]["offsets"]) < 8:
            table[key]["offsets"].append(row["offset"])
    ordered_gaps = [gaps[key] for key in sorted(gaps)]
    ordered_excluded = [excluded[key] for key in sorted(excluded)]
    status = "mismatch" if findings else "partial" if gaps else "verified"
    limit = None if details else 12
    return {
        "status": status,
        "scope": "direct raw-backed initializers and terminated literals; not runtime state or recursive pointer closure",
        "literal_checks": literals[:limit],
        "literal_check_count": len(literals),
        "contracts": [proofs[key] for key in sorted(proofs)],
        "findings": findings[:limit],
        "finding_count": len(findings),
        "uncovered": ordered_gaps[:limit],
        "uncovered_count": len(ordered_gaps),
        "out_of_scope": ordered_excluded[:limit],
        "out_of_scope_count": len(ordered_excluded),
        "details_omitted": not details
        and any(
            len(rows) > 12
            for rows in (literals, findings, ordered_gaps, ordered_excluded)
        ),
        "next_commands": [
            [
                "python3",
                "scripts/compare-function.py",
                unit["name"],
                "--json",
                "--details",
            ]
        ]
        if gaps
        else [],
    }


def linked_code(
    unit: dict, symbol: str, source: bytes, actual: list[dict], linked
) -> dict:
    if linked is None:
        return {
            "status": "not_checked",
            "scope": "final linked function and declared data contracts",
            "findings": [
                {
                    "code": "LINK_NOT_CHECKED",
                    "message": "No rebuilt PE and linker map were supplied.",
                }
            ],
            "next_commands": [
                ["python3", "scripts/build.py", "--build-type", "normal", "-j", "1"],
                [
                    "python3",
                    "scripts/compare-function.py",
                    unit["name"],
                    "--rebuild",
                    "build/th08.exe",
                    "--map",
                    "build/th08.map",
                    "--json",
                ],
            ],
        }
    image, publics = linked
    try:
        address = linked_address(publics, symbol)
    except DataError as exc:
        return {
            "status": "partial",
            "scope": "final linked function",
            "findings": [exc.finding],
        }
    expected, unresolved = bytearray(source), []
    for relocation in actual:
        offset = relocation["offset"]
        addend = struct.unpack_from("<I", expected, offset)[0]
        local = relocation.get("local_symbol_offset")
        try:
            destination = (
                address + local
                if local is not None and 0 <= local < len(source)
                else linked_address(publics, relocation["symbol"])
            )
        except DataError as exc:
            unresolved.append({"offset": f"0x{offset:X}", **exc.finding})
            continue
        value = destination + addend
        if relocation["type_id"] == 0x14:
            value -= address + offset + 4
        struct.pack_into("<I", expected, offset, value & 0xFFFFFFFF)
    if unresolved:
        # Do not compare a masked body and call it linked exact.
        return {
            "status": "partial",
            "scope": "final linked function",
            "unresolved_count": len(unresolved),
            "findings": unresolved[:12],
        }
    try:
        observed = read_va(image, address, len(source))
    except ValueError as exc:
        return {
            "status": "mismatch",
            "scope": "final linked function",
            "findings": [{"code": "LINK_EXTENT_INVALID", "message": str(exc)}],
        }
    differences = [
        index
        for index, pair in enumerate(zip(expected, observed))
        if pair[0] != pair[1]
    ]
    return {
        "status": "mismatch" if differences else "verified",
        "scope": "final linked function; callee bodies require their own units",
        "linked_address": f"0x{address:08X}",
        "compared_size": len(source),
        "difference_count": len(differences),
        "findings": [
            {
                "code": "LINK_CODE_MISMATCH",
                "message": "Linked instructions differ from the compared COFF body.",
                "first_offset": differences[0],
            }
        ]
        if differences
        else [],
    }


def scopes(
    unit: dict,
    target: PEImage,
    raw_code: bytes,
    actual: list[dict],
    relocated: list[dict],
    code_result: str,
    object_symbol: str,
    *,
    linked=None,
    details=False,
) -> dict:
    data = data_scope(unit, target, relocated, actual, linked=linked, details=details)
    linkage = linked_code(unit, object_symbol, raw_code, actual, linked)
    linked_data = [
        proof
        for proof in data["contracts"]
        if proof["linked"]["status"] != "not_checked"
    ]
    linkage["data_contracts"] = [
        {"contract": proof["contract"], **proof["linked"]} for proof in linked_data
    ]
    if any(proof["linked"]["status"] == "mismatch" for proof in linked_data):
        linkage["status"] = "mismatch"
    return {
        "schema_version": 1,
        "code": {
            "status": code_result,
            "scope": "configured function and associated compare_size, with strict relocation replay",
        },
        "data": data,
        "linkage": linkage,
    }
