#!/usr/bin/env python3
"""Fail when closed TH08 semantic protocol surfaces regain raw literals."""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
SOURCE_SUFFIXES = {".cpp", ".hpp", ".inl"}


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def source_files() -> list[Path]:
    return sorted(
        path
        for path in SRC.rglob("*")
        if path.is_file() and path.suffix.lower() in SOURCE_SUFFIXES
    )


def reject_pattern(paths: list[Path], label: str, pattern: str) -> None:
    matcher = re.compile(pattern, re.MULTILINE | re.DOTALL)
    hits: list[str] = []
    for path in paths:
        text = path.read_text(encoding="utf-8")
        for match in matcher.finditer(text):
            line = text.count("\n", 0, match.start()) + 1
            relative = path.relative_to(ROOT).as_posix()
            spelling = " ".join(match.group(0).split())
            hits.append(f"{relative}:{line}: {spelling[:120]}")
    if hits:
        fail(f"{label}:\n  " + "\n  ".join(hits))


def check_ecl_opcode_protocol() -> None:
    header = (SRC / "EclManager.hpp").read_text(encoding="utf-8")
    match = re.search(r"enum EclOpcode\s*\{(.*?)\n\};", header, re.DOTALL)
    if match is None:
        fail("could not find enum EclOpcode")

    entries = re.findall(
        r"\b(ECL_OPCODE_[A-Z0-9_]+)\s*=\s*(\d+)", match.group(1)
    )
    names = [name for name, _ in entries]
    values = [int(value) for _, value in entries]
    if len(entries) != 184 or sorted(values) != list(range(1, 185)):
        fail("EclOpcode must contain one explicit, unique entry for every value 1..184")
    if len(set(names)) != len(names):
        fail("EclOpcode contains a duplicate name")

    dispatch_paths = [SRC / "EclRunLow.inl", SRC / "EclRunHigh.inl"]
    dispatch = "\n".join(path.read_text(encoding="utf-8") for path in dispatch_paths)
    reject_pattern(dispatch_paths, "numeric ECL dispatch label", r"\bcase\s+(?:0x[0-9a-f]+|\d+)\s*:")
    dispatched = re.findall(r"\bcase\s+(ECL_OPCODE_[A-Z0-9_]+)\s*:", dispatch)
    if len(dispatched) != 184 or set(dispatched) != set(names):
        fail("ECL dispatch must reference every named opcode exactly once")


def main() -> int:
    paths = source_files()
    check_ecl_opcode_protocol()

    reject_pattern(
        [path for path in paths if path.name != "ZunMath.hpp"],
        "raw Float3/D3DXVECTOR3 pointer cast",
        r"reinterpret_cast\s*<\s*(?:const\s+)?(?:Float3|D3DXVECTOR3)\s*\*\s*>",
    )
    reject_pattern(
        paths,
        "numeric fixed Effect ID",
        r"\.SpawnEffect(?:InSecondaryPool|InFixedSlot|InFixedSlotWithVelocity|WithVelocity)?"
        r"\s*\(\s*(?:0x[0-9a-f]+|\d+)",
    )
    reject_pattern(
        paths,
        "numeric fixed SoundIdx cast",
        r"static_cast\s*<\s*SoundIdx\s*>\s*\(\s*(?:0x[0-9a-f]+|\d+)\s*\)",
    )
    reject_pattern(
        paths,
        "numeric ANM resource slot",
        r"\b(?:LoadAnm|PreloadAnm|ReleaseAnm|GetAnm)\s*\(\s*(?:0x[0-9a-f]+|\d+)",
    )

    print("TH08 semantic protocol checks passed")
    print("  ECL opcode dispatch: 184/184 named")
    print("  raw vector pointer casts: 0 outside exact-safe views")
    print("  fixed numeric effect/sound/resource IDs: 0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
