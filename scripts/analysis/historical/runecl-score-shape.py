#!/usr/bin/env python3
"""Reproduce the completed RunEcl handler-shape score.

This is a non-mutating historical companion to runecl-crosswalk-dispatch.py.
RunEcl is already exact; use this only to reproduce its checked-in evidence,
not to select new work.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[3]
DEFAULT_OBJECT = ROOT / "build" / "probes" / "EclRun.obj"
EXPECTED_FUNCTION_SIZE = 0x6B06
EXPECTED_CODE_SIZE = 0x680E


def run_crosswalk(object_path: Path, top: int) -> dict[str, Any]:
    output = subprocess.check_output(
        [
            sys.executable,
            str(
                ROOT
                / "scripts"
                / "analysis"
                / "historical"
                / "runecl-crosswalk-dispatch.py"
            ),
            "--object",
            str(object_path),
            "--top",
            str(top),
        ],
        text=True,
    )
    return json.loads(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--object", type=Path, default=DEFAULT_OBJECT)
    parser.add_argument("--top", type=int, default=25)
    parser.add_argument(
        "--focus",
        default="34,39,75,89,106,117,119,137,140,164",
        help="comma-separated opcodes to summarize separately",
    )
    parser.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    args = parser.parse_args()

    object_path = args.object if args.object.is_absolute() else ROOT / args.object
    data = run_crosswalk(object_path, max(args.top, 200))
    entries = data["largest_positive_spans"]
    by_opcode = {entry["opcode"]: entry for entry in entries}
    focus = [int(part, 0) for part in args.focus.split(",") if part.strip()]

    positive_delta = sum(max(0, entry["delta"]) for entry in entries)
    absolute_delta = sum(abs(entry["delta"]) for entry in entries)
    max_positive_delta = max((max(0, entry["delta"]) for entry in entries), default=0)
    obj = data["object"]
    summary = {
        "object": str(object_path),
        "function_size": obj["function_size"],
        "code_size": obj["code_size"],
        "jump_table_offset": obj["jump_table_offset"],
        "physical_handler_delta": data["physical_handler_delta"],
        "expected_function_size": EXPECTED_FUNCTION_SIZE,
        "expected_code_size": EXPECTED_CODE_SIZE,
        "extent_ok": obj["function_size"] == EXPECTED_FUNCTION_SIZE
        and obj["code_size"] == EXPECTED_CODE_SIZE
        and data["physical_handler_delta"] == 0,
        "positive_delta": positive_delta,
        "absolute_delta": absolute_delta,
        "max_positive_delta": max_positive_delta,
        "focus": {
            str(opcode): by_opcode.get(
                opcode,
                {
                    "opcode": opcode,
                    "aliases": [opcode],
                    "target_size": None,
                    "object_size": None,
                    "delta": 0,
                },
            )
            for opcode in focus
        },
        "top": entries[: args.top],
    }

    if args.json:
        print(json.dumps(summary, indent=2))
        return 0

    print(f"object: {summary['object']}")
    print(
        "extent: "
        f"function=0x{summary['function_size']:X}/0x{EXPECTED_FUNCTION_SIZE:X} "
        f"code=0x{summary['code_size']:X}/0x{EXPECTED_CODE_SIZE:X} "
        f"physical_delta={summary['physical_handler_delta']} "
        f"ok={summary['extent_ok']}"
    )
    print(
        "shape score: "
        f"positive_delta={positive_delta} "
        f"absolute_delta={absolute_delta} "
        f"max_positive_delta={max_positive_delta}"
    )
    print("focus:")
    for opcode in focus:
        entry = summary["focus"][str(opcode)]
        print(
            f"  op {opcode:3d}: "
            f"target={entry['target_size']} object={entry['object_size']} "
            f"delta={entry['delta']} aliases={entry['aliases']}"
        )
    print("top positive spans:")
    for entry in entries[: args.top]:
        print(
            f"  op {entry['opcode']:3d}: "
            f"target=0x{int(entry['target_start'], 16):08X}+0x{entry['target_size']:X} "
            f"object={entry['object_start']}+0x{entry['object_size']:X} "
            f"delta={entry['delta']} aliases={entry['aliases']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
