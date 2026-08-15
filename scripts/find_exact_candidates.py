#!/usr/bin/env python3
"""Find source-defined functions that may be ready for exact-match promotion.

The script scans COFF objects, demangles MSVC symbols with the same lightweight
logic used by objdiff generation, and reports symbols that:
  * have a name present in config/mapping.csv,
  * are not in config/implemented.csv,
  * have a COFF function size equal to the mapping extent.

It does not prove exactness. Use it to choose a small batch, then add
normal match-units and run compare-function.py. Pass --clean-rebuild to
delete scanned objects and rebuild them first; this avoids stale probe
objects after temporary source-shape experiments.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from coff import ObjectModule  # noqa: E402
from generate_objdiff_objs import demangle_msvc  # noqa: E402


def load_mapping() -> dict[str, tuple[int, int]]:
    rows: dict[str, tuple[int, int]] = {}
    with (ROOT / "config" / "mapping.csv").open(newline="") as f:
        for row in csv.reader(f):
            if len(row) < 3:
                continue
            try:
                rows[row[0]] = (int(row[1], 16), int(row[2], 16))
            except ValueError:
                continue
    return rows


def load_implemented() -> set[str]:
    values: set[str] = set()
    with (ROOT / "config" / "implemented.csv").open(newline="") as f:
        for row in csv.reader(f):
            if row:
                values.add(row[0])
    return values


def iter_defined_symbols(object_path: Path):
    module = ObjectModule()
    module.unpack(object_path.read_bytes(), 0)
    for symbol in module.symbols:
        if not getattr(symbol, "aux_records", None):
            continue
        size = getattr(symbol.aux_records[0], "total_size", None)
        if not isinstance(size, int) or size <= 0:
            continue
        raw = symbol.get_name(module.string_table)
        demangled = demangle_msvc(raw).decode("utf-8", errors="ignore")
        yield demangled, raw.decode("latin1", errors="ignore"), size



def ninja_targets() -> set[str]:
    """Return explicit ninja targets accepted by this build directory."""

    result = subprocess.run(
        ["./scripts/wineth08", "./scripts/th08run.bat", "ninja", "-t", "targets", "all"],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=True,
    )
    targets: set[str] = set()
    for line in result.stdout.splitlines():
        target = line.split(":", 1)[0].strip().replace("\\", "/")
        if target:
            targets.add(target)
    return targets


def clean_rebuild_objects(objects: list[Path]) -> None:
    """Force scanned objects to reflect the current source tree."""

    accepted_targets = ninja_targets()
    normalized: list[Path] = []
    skipped: list[str] = []
    for object_path in objects:
        object_path = object_path if object_path.is_absolute() else ROOT / object_path
        rel = str(object_path.relative_to(ROOT)).replace("\\", "/")
        if object_path.name.endswith("-stripped.obj"):
            continue
        if rel not in accepted_targets:
            skipped.append(rel)
            continue
        normalized.append(object_path)

    for rel in skipped:
        print(f"warning: clean-rebuild skipped non-ninja target {rel}", file=sys.stderr)

    for object_path in normalized:
        try:
            object_path.unlink()
        except FileNotFoundError:
            pass

    if not normalized:
        return

    build_args = [str(path.relative_to(ROOT)) for path in normalized]
    subprocess.run(["./scripts/wineth08", "./scripts/th08run.bat", "ninja", "-j1", *build_args], cwd=ROOT, check=True)

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("objects", nargs="*", type=Path, default=sorted((ROOT / "build").glob("*.obj")))
    parser.add_argument("--min-size", type=lambda x: int(x, 0), default=0x10)
    parser.add_argument("--max-size", type=lambda x: int(x, 0), default=0x400)
    parser.add_argument("--limit", type=int, default=0, help="maximum rows to print; 0 means all")
    parser.add_argument("--near", action="store_true", help="also show source symbols whose size differs from mapping")
    parser.add_argument("--max-delta", type=lambda x: int(x, 0), default=0x40, help="maximum absolute size delta for --near")
    parser.add_argument("--clean-rebuild", action="store_true", help="delete scanned objects and rebuild them before scanning")
    args = parser.parse_args()

    args.objects = [obj for obj in args.objects if not obj.name.endswith("-stripped.obj")]
    if args.clean_rebuild:
        clean_rebuild_objects(args.objects)

    mapping = load_mapping()
    implemented = load_implemented()
    rows = []
    for object_path in args.objects:
        object_path = object_path if object_path.is_absolute() else ROOT / object_path
        if not object_path.exists():
            continue
        try:
            symbols = list(iter_defined_symbols(object_path))
        except Exception as exc:  # pragma: no cover - triage helper
            print(f"warning: failed to parse {object_path}: {exc}", file=sys.stderr)
            continue
        for demangled, raw, size in symbols:
            if demangled in implemented or demangled not in mapping:
                continue
            target_addr, target_size = mapping[demangled]
            if not (args.min_size <= target_size <= args.max_size):
                continue
            delta = size - target_size
            if delta != 0 and (not args.near or abs(delta) > args.max_delta):
                continue
            rows.append((abs(delta), delta, demangled, object_path.relative_to(ROOT), raw, target_addr, target_size, size))

    rows = sorted(rows)
    shown = rows if args.limit <= 0 else rows[: args.limit]
    for _abs_delta, delta, demangled, object_path, raw, target_addr, target_size, size in shown:
        delta_text = "exact" if delta == 0 else f"delta={delta:+#x} object=0x{size:X}"
        print(f"0x{target_addr:08X} size=0x{target_size:X} {delta_text} object={object_path} name={demangled} symbol={raw}")
    print(f"candidates={len(rows)} shown={len(shown)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
