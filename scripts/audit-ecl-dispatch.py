#!/usr/bin/env python3
"""Audit TH08 RunEcl opcode coverage and resolver-call leverage."""

from __future__ import annotations

from collections import Counter
import json
from pathlib import Path
import re
import struct

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM

from typed_re.facts import DEFAULT_TARGET, load_target


ROOT = Path(__file__).resolve().parents[1]
RUN_ECL = 0x004184B0
RUN_ECL_SIZE = 0x680E
JUMP_TABLE = 0x0041ECBE
OPCODE_COUNT = 184
RESOLVERS = {
    0x0041F420: "ResolveInt",
    0x0041FE10: "ResolveIntLValue",
    0x00420120: "ResolveFloat",
    0x00420950: "ResolveFloatLValue",
}


def source_cases(path: Path, first: int, last: int) -> dict[str, object]:
    values = [
        int(value, 0)
        for value in re.findall(r"\bcase\s+(0x[0-9A-Fa-f]+|\d+)\s*:", path.read_text())
    ]
    counts = Counter(value for value in values if first <= value <= last)
    expected = set(range(first, last + 1))
    return {
        "path": str(path.relative_to(ROOT)),
        "range": [first, last],
        "case_count": sum(counts.values()),
        "unique_cases": len(counts),
        "missing": sorted(expected - counts.keys()),
        "duplicates": sorted(value for value, count in counts.items() if count != 1),
    }


def main() -> int:
    image, target = load_target(DEFAULT_TARGET)
    entries = struct.unpack("<184I", image.read(JUMP_TABLE, OPCODE_COUNT * 4))

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    calls = Counter()
    body = image.read(RUN_ECL, RUN_ECL_SIZE)
    instructions = list(decoder.disasm(body, RUN_ECL))
    if sum(instruction.size for instruction in instructions) != RUN_ECL_SIZE:
        raise ValueError("RunEcl did not decode through its target-attested extent")
    for instruction in instructions:
        if instruction.mnemonic == "call" and instruction.operands and instruction.operands[0].type == X86_OP_IMM:
            calls[instruction.operands[0].imm & 0xFFFFFFFF] += 1

    low = source_cases(ROOT / "src/EclRunLow.inl", 1, 92)
    high = source_cases(ROOT / "src/EclRunHigh.inl", 93, 184)
    resolver_calls = {RESOLVERS[address]: calls[address] for address in RESOLVERS}
    report = {
        "target_sha256": target["sha256"],
        "run_ecl": {"address": f"0x{RUN_ECL:08X}", "size": RUN_ECL_SIZE},
        "jump_table": {
            "address": f"0x{JUMP_TABLE:08X}",
            "slots": len(entries),
            "unique_handlers": len(set(entries)),
            "first_handler": f"0x{entries[0]:08X}",
            "last_handler": f"0x{entries[-1]:08X}",
        },
        "source": {"low": low, "high": high},
        "direct_calls": sum(calls.values()),
        "resolver_calls": resolver_calls,
        "resolver_call_total": sum(resolver_calls.values()),
    }
    print(json.dumps(report, indent=2))
    complete = not low["missing"] and not low["duplicates"] and not high["missing"] and not high["duplicates"]
    return 0 if complete and len(entries) == OPCODE_COUNT else 1


if __name__ == "__main__":
    raise SystemExit(main())
