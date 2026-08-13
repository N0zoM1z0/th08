#!/usr/bin/env python3
"""Extract reproducible low-level facts from the hash-attested TH08 image.

This initial port deliberately stays below a semantic decompiler.  It reads
only target PE bytes and repository ledgers, and never edits source, analysis
databases, or tracking state.  IDA/Ghidra-derived types and library recovery
are outside this module's supported boundary.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import csv
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tomllib
from typing import Any

try:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_AC_READ, CS_AC_WRITE
    from capstone.x86 import (
        X86_OP_IMM,
        X86_OP_MEM,
        X86_OP_REG,
        X86_REG_EBP,
        X86_REG_EBX,
        X86_REG_ECX,
        X86_REG_EDI,
        X86_REG_EDX,
        X86_REG_ESI,
        X86_REG_ESP,
    )
except ImportError as error:  # pragma: no cover - environment diagnostic
    raise SystemExit("capstone is required: python3 -m pip install capstone") from error


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_TARGET = ROOT / "resources" / "th08.exe"
TARGET_CONFIG = ROOT / "config" / "target.toml"
MAPPING = ROOT / "config" / "mapping.csv"
RECCMP_FUNCTIONS = ROOT / "config" / "reccmp-functions.csv"
RULES = ROOT / "config" / "typed-re-rules.toml"
MATCH_UNITS = ROOT / "config" / "match-units.toml"
COMPARE = ROOT / "scripts" / "compare-function.py"


def canonical(value: str | int) -> str:
    return f"0x{int(value, 0) if isinstance(value, str) else value:08X}"


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


class PeImage:
    def __init__(self, path: Path):
        self.data = path.read_bytes()
        if len(self.data) < 0x40 or self.data[:2] != b"MZ":
            raise ValueError("target is not an MZ executable")
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        if pe + 24 > len(self.data) or self.data[pe : pe + 4] != b"PE\0\0":
            raise ValueError("target lacks a PE signature")
        count = struct.unpack_from("<H", self.data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", self.data, pe + 20)[0]
        optional = pe + 24
        if optional + optional_size > len(self.data) or optional_size < 60:
            raise ValueError("target has a truncated PE optional header")
        self.base = struct.unpack_from("<I", self.data, optional + 28)[0]
        self.image_size = struct.unpack_from("<I", self.data, optional + 56)[0]
        table = optional + optional_size
        if table + count * 40 > len(self.data):
            raise ValueError("target has a truncated PE section table")
        self.sections: list[tuple[int, int, int, int]] = []
        for index in range(count):
            offset = table + 40 * index
            virtual_size, rva, raw_size, raw_pointer = struct.unpack_from(
                "<IIII", self.data, offset + 8
            )
            self.sections.append((rva, max(virtual_size, raw_size), raw_pointer, raw_size))

    def contains(self, address: int) -> bool:
        return self.base <= address < self.base + self.image_size

    def read(self, address: int, size: int) -> bytes:
        if size <= 0:
            raise ValueError("target read size must be positive")
        rva = address - self.base
        for section_rva, mapped_size, raw_pointer, raw_size in self.sections:
            if section_rva <= rva and rva + size <= section_rva + mapped_size:
                within = rva - section_rva
                if within + size > raw_size:
                    raise ValueError("target function extends beyond PE raw data")
                start = raw_pointer + within
                data = self.data[start : start + size]
                if len(data) != size:
                    raise ValueError("target PE section is truncated")
                return data
        raise ValueError(f"target range {canonical(address)}+{size:#x} is unmapped")


def load_target(path: Path) -> tuple[PeImage, dict[str, Any]]:
    with TARGET_CONFIG.open("rb") as stream:
        target = tomllib.load(stream).get("target")
    if not isinstance(target, dict):
        raise ValueError("config/target.toml lacks [target]")
    actual_size = path.stat().st_size
    actual_hash = file_sha256(path)
    if actual_size != int(target["size"]) or actual_hash != str(target["sha256"]):
        raise ValueError(
            f"target identity mismatch: size={actual_size} sha256={actual_hash}"
        )
    return PeImage(path), target


def load_ledgers() -> tuple[dict[int, dict[str, Any]], dict[int, dict[str, str]]]:
    mapping: dict[int, dict[str, Any]] = {}
    with MAPPING.open(newline="", encoding="utf-8") as stream:
        for line, row in enumerate(csv.reader(stream), 1):
            if len(row) < 6:
                raise ValueError(f"mapping.csv:{line}: expected at least 6 fields")
            address = int(row[1], 0)
            if address in mapping:
                raise ValueError(f"mapping.csv:{line}: duplicate address {canonical(address)}")
            mapping[address] = {
                "name": row[0],
                "address": canonical(address),
                "size": int(row[2], 0),
                "calling_convention": row[3],
                "varargs": row[4],
                "return_type": row[5],
                "parameters": row[6:],
            }

    reccmp: dict[int, dict[str, str]] = {}
    with RECCMP_FUNCTIONS.open(newline="", encoding="utf-8") as stream:
        reader = csv.reader(stream)
        header = next(reader, None)
        if header != ["name", "address", "type"]:
            raise ValueError(f"invalid reccmp-functions.csv header: {header!r}")
        for line, row in enumerate(reader, 2):
            if len(row) != 3:
                raise ValueError(f"reccmp-functions.csv:{line}: expected 3 fields")
            address = int(row[1], 0)
            if address in reccmp:
                raise ValueError(
                    f"reccmp-functions.csv:{line}: duplicate address {canonical(address)}"
                )
            reccmp[address] = {
                "name": row[0],
                "address": canonical(address),
                "kind": row[2],
            }
    return mapping, reccmp


def access_name(access: int) -> str:
    names = []
    if access & CS_AC_READ:
        names.append("read")
    if access & CS_AC_WRITE:
        names.append("write")
    return "+".join(names) or "unspecified"


def width_name(size: int) -> str:
    return {1: "byte", 2: "word", 4: "dword", 8: "qword", 10: "tbyte"}.get(
        size, f"{size}-byte"
    )


def load_rules(target_sha256: str, features: set[str]) -> list[dict[str, Any]]:
    with RULES.open("rb") as stream:
        rules = tomllib.load(stream)
    if rules.get("schema_version") != 1:
        raise ValueError("unsupported typed-re rule schema")
    if rules.get("target_sha256") != target_sha256:
        raise ValueError("typed-re rules target digest mismatch")
    result = []
    for rule in rules.get("rules", []):
        if not isinstance(rule, dict) or not isinstance(rule.get("requires", []), list):
            raise ValueError("typed-re rules contain an invalid rule")
        if set(rule.get("requires", [])).issubset(features):
            result.append(rule)
    return result


def find_match_unit(address: int, size: int) -> str | None:
    with MATCH_UNITS.open("rb") as stream:
        manifest = tomllib.load(stream)
    if manifest.get("schema_version") != 1 or not isinstance(manifest.get("units"), list):
        raise ValueError("unsupported match-unit schema")
    matches = []
    names: set[str] = set()
    for unit in manifest["units"]:
        if not isinstance(unit, dict):
            raise ValueError("match-units.toml contains a non-table unit")
        name = str(unit.get("name", ""))
        if not name or name in names:
            raise ValueError(f"invalid or duplicate match unit: {name!r}")
        names.add(name)
        if int(unit["target_address"]) == address:
            if int(unit["size"]) != size:
                raise ValueError("match-unit size disagrees with mapping.csv")
            matches.append(name)
    if len(matches) > 1:
        raise ValueError(f"multiple match units own {canonical(address)}")
    return matches[0] if matches else None


def run_compare(address: int, size: int, target_path: Path) -> dict[str, Any]:
    unit = find_match_unit(address, size)
    if unit is None:
        return {"state": "not_configured"}
    completed = subprocess.run(
        [
            sys.executable,
            str(COMPARE),
            unit,
            "--target",
            str(target_path),
            "--json",
        ],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    try:
        report = json.loads(completed.stdout)
    except json.JSONDecodeError as error:
        raise ValueError(
            f"compare-function.py returned non-JSON output: "
            f"{completed.stderr.strip() or completed.stdout.strip()}"
        ) from error
    if report.get("unit") != unit:
        raise ValueError("compare-function.py returned the wrong match unit")
    if completed.returncode not in {0, 1}:
        raise ValueError(f"compare-function.py failed: {report.get('error', report)!r}")
    return {"state": "compared", "unit": unit, "report": report}


def analyze(address: str, target_path: Path, compare: bool) -> dict[str, Any]:
    image, target_config = load_target(target_path)
    mapping, reccmp = load_ledgers()
    start = int(address, 0)
    row = mapping.get(start)
    if row is None:
        raise ValueError(f"address is absent from mapping.csv: {address}")
    size = int(row["size"])
    body = image.read(start, size)

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(body, start))
    decoded = sum(insn.size for insn in instructions)
    if decoded != len(body):
        raise ValueError(f"instruction decode stops at +{decoded:#x} of {len(body):#x}")

    frame_size = 0
    frame_instruction: dict[str, Any] | None = None
    saved_registers: list[str] = []
    register_homes: list[dict[str, Any]] = []
    stack: dict[int, dict[str, Any]] = {}
    absolute_operands: dict[tuple[int, str, int], dict[str, Any]] = {}
    calls: list[dict[str, Any]] = []
    returns: list[int] = []
    features: set[str] = set()

    for index, insn in enumerate(instructions):
        mnemonic = insn.mnemonic
        if mnemonic in {"movzx", "movsx", "idiv"}:
            features.add(mnemonic)
        if mnemonic.startswith("set"):
            features.add("setcc")
        if (
            mnemonic == "jmp"
            and insn.operands
            and insn.operands[0].type == X86_OP_MEM
            and insn.operands[0].mem.index != 0
        ):
            features.add("jump_table_dispatch")
        if index < 16 and mnemonic == "sub" and len(insn.operands) == 2:
            left, right = insn.operands
            if left.type == X86_OP_REG and left.reg == X86_REG_ESP and right.type == X86_OP_IMM:
                frame_size = int(right.imm)
                frame_instruction = {
                    "address": canonical(insn.address),
                    "size": insn.size,
                    "bytes": bytes(insn.bytes).hex(" "),
                }
        if index < 16 and mnemonic == "push" and insn.operands:
            operand = insn.operands[0]
            if operand.type == X86_OP_REG and operand.reg in {
                X86_REG_EBX,
                X86_REG_ESI,
                X86_REG_EDI,
            }:
                name = insn.reg_name(operand.reg)
                if name not in saved_registers:
                    saved_registers.append(name)
        if index < 20 and mnemonic == "mov" and len(insn.operands) == 2:
            destination, source = insn.operands
            if (
                destination.type == X86_OP_MEM
                and destination.mem.base == X86_REG_EBP
                and destination.mem.index == 0
                and destination.mem.disp < 0
                and source.type == X86_OP_REG
                and source.reg in {X86_REG_ECX, X86_REG_EDX}
            ):
                register_homes.append(
                    {
                        "register": insn.reg_name(source.reg),
                        "stack_offset": int(destination.mem.disp),
                        "instruction": canonical(insn.address),
                    }
                )
                features.add("register_home")

        for operand in insn.operands:
            if operand.type == X86_OP_MEM and operand.mem.base == X86_REG_EBP and operand.mem.index == 0:
                displacement = int(operand.mem.disp)
                slot = stack.setdefault(
                    displacement,
                    {
                        "offset": displacement,
                        "region": "local" if displacement < 0 else "argument_or_frame",
                        "widths": set(),
                        "uses": [],
                    },
                )
                slot["widths"].add(width_name(operand.size))
                if len(slot["uses"]) < 12:
                    slot["uses"].append(
                        {
                            "address": canonical(insn.address),
                            "mnemonic": mnemonic,
                            "access": access_name(getattr(operand, "access", 0)),
                        }
                    )

            absolute: int | None = None
            kind = ""
            if operand.type == X86_OP_MEM and operand.mem.base == 0 and operand.mem.index == 0:
                absolute = int(operand.mem.disp) & 0xFFFFFFFF
                kind = "absolute_memory"
            elif operand.type == X86_OP_IMM and mnemonic != "call" and not mnemonic.startswith("j"):
                value = int(operand.imm) & 0xFFFFFFFF
                if image.contains(value):
                    absolute = value
                    kind = "image_immediate"
            if absolute is not None and image.contains(absolute):
                features.add("absolute_memory")
                key = (absolute, kind, operand.size)
                record = absolute_operands.setdefault(
                    key,
                    {
                        "address": canonical(absolute),
                        "kind": kind,
                        "operand_width": width_name(operand.size),
                        "uses": [],
                    },
                )
                if len(record["uses"]) < 12:
                    record["uses"].append(
                        {"instruction": canonical(insn.address), "mnemonic": mnemonic}
                    )

        if mnemonic == "call" and insn.operands and insn.operands[0].type == X86_OP_IMM:
            destination = int(insn.operands[0].imm) & 0xFFFFFFFF
            calls.append(
                {
                    "instruction": canonical(insn.address),
                    "destination": canonical(destination),
                    "mapping_name": mapping.get(destination, {}).get("name"),
                    "reccmp_name": reccmp.get(destination, {}).get("name"),
                }
            )
        if mnemonic == "ret":
            returns.append(int(insn.operands[0].imm) if insn.operands else 0)

    if len(calls) >= 32:
        features.add("large_direct_call_surface")

    stack_rows = []
    for displacement in sorted(stack):
        item = stack[displacement]
        item["widths"] = sorted(item["widths"])
        stack_rows.append(item)

    comparison = run_compare(start, size, target_path) if compare else {"state": "not_requested"}
    ecx_role_hint = None
    if any(item["register"] == "ecx" for item in register_homes):
        convention = row.get("calling_convention")
        if convention == "__thiscall":
            ecx_role_hint = "this_receiver"
        elif convention == "__fastcall":
            ecx_role_hint = "fastcall_argument_zero"
        else:
            ecx_role_hint = "unknown"
    return {
        "schema_version": 1,
        "address": canonical(start),
        "analysis_backend": "hash-attested PE bytes (no IDA/Ghidra state)",
        "target_sha256": target_config["sha256"],
        "ledger": {"mapping": row, "reccmp": reccmp.get(start)},
        "exact_observations": {
            "target_bytes": body.hex(" "),
            "target_bytes_sha256": hashlib.sha256(body).hexdigest(),
            "decoded_bytes": decoded,
            "instruction_count": len(instructions),
            "frame_size": frame_size,
            "frame_instruction": frame_instruction,
            "saved_registers": saved_registers,
            "register_homes": register_homes,
            "stack_accesses": stack_rows,
            "absolute_operands": sorted(
                absolute_operands.values(), key=lambda item: (item["address"], item["kind"])
            ),
            "direct_calls": calls,
            "return_cleanup_bytes": sorted(set(returns)),
        },
        "inferences": {
            "features": sorted(features),
            "ecx_role_hint": ecx_role_hint,
            "compiler_recommendations": load_rules(str(target_config["sha256"]), features),
            "warning": "Recommendations are probe hints; only compare-function.py exact output is acceptance evidence.",
        },
        "comparison": comparison,
        "unsupported": [
            "semantic decompilation",
            "IDA database reads or writes",
            "Ghidra headless type recovery",
            "VC7 prebuilt-library candidate promotion",
            "indirect-call destination recovery",
            "automatic source or ledger mutation",
        ],
    }


def self_check(target_path: Path) -> None:
    report = analyze("0x004413E0", target_path, True)
    observed = report["exact_observations"]
    failures = []
    if observed["decoded_bytes"] != 0x61:
        failures.append("decoded-size regression")
    if observed["target_bytes_sha256"] != "94f8d95180648b9231430ebc4926c0ab8b5507d154f771b2465eb3dee65238e5":
        failures.append("target-byte regression")
    if observed["frame_size"] != 0x14:
        failures.append("frame-size regression")
    if not any(
        item["register"] == "ecx" and item["stack_offset"] == -0x14
        for item in observed["register_homes"]
    ):
        failures.append("ECX-home regression")
    if [item["destination"] for item in observed["direct_calls"]] != ["0x00404720"]:
        failures.append("direct-call regression")
    comparison = report["comparison"]
    if comparison.get("state") != "compared" or comparison.get("report", {}).get("result") != "exact":
        failures.append("canonical comparator regression")
    callback = analyze("0x00437A2F", target_path, True)
    if callback["inferences"]["ecx_role_hint"] != "fastcall_argument_zero":
        failures.append("fastcall ECX-role regression")
    callback_comparison = callback["comparison"]
    if (
        callback_comparison.get("state") != "compared"
        or callback_comparison.get("report", {}).get("result") != "exact"
    ):
        failures.append("fastcall callback comparator regression")
    if failures:
        raise ValueError("; ".join(failures))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("address", nargs="?")
    parser.add_argument("--target", type=Path, default=DEFAULT_TARGET)
    parser.add_argument("--compare", action="store_true")
    parser.add_argument("--json", action="store_true", help="retained for CLI symmetry")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    target_path = args.target.expanduser().resolve()
    try:
        if args.check:
            self_check(target_path)
            print("typed reconstruction facts OK: thiscall and fastcall ECX-role corpus plus strict comparators")
            return 0
        if not args.address:
            parser.error("address is required unless --check is selected")
        print(json.dumps(analyze(canonical(args.address), target_path, args.compare), indent=2))
        return 0
    except (OSError, ValueError, KeyError, TypeError, struct.error, tomllib.TOMLDecodeError) as error:
        print(
            json.dumps({"result": "error", "failure": {"message": str(error)}}, indent=2),
            file=sys.stderr,
        )
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
