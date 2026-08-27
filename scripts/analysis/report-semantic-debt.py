#!/usr/bin/env python3
"""Report heuristic TH08 source locations that still encode layout as offsets.

This is a read-only work selector, not a semantic-completion metric.  A match
can be legitimate serialization, instruction decoding, ABI glue, or an
intentional unknown field.  Review target evidence before changing source.

Examples:
  python3 scripts/analysis/report-semantic-debt.py
  python3 scripts/analysis/report-semantic-debt.py --category raw-member-access --details
  python3 scripts/analysis/report-semantic-debt.py --path src/ScreenEffect.cpp --json
  python3 scripts/analysis/report-semantic-debt.py --include-probes --top 30
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[2]
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inl"}
CATEGORIES = (
    "raw-member-access",
    "absolute-address",
    "anonymous-identifier",
    "opaque-storage",
)

RAW_MEMBER_ACCESS = re.compile(
    r"reinterpret_cast\s*<\s*u8\s*\*>.*?\+\s*0x[0-9a-f]+",
    re.IGNORECASE,
)
ABSOLUTE_CAST = re.compile(
    r"reinterpret_cast\s*<[^>\n]+\*>\s*\(\s*0x[0-9a-f]+",
    re.IGNORECASE,
)
ABSOLUTE_MACRO = re.compile(r"\bABS_(?:I|U|F)\d+\s*\(", re.IGNORECASE)
ANONYMOUS_IDENTIFIER = re.compile(
    r"\b(?:"
    r"unk(?:nown)?(?:_?0x|_)?|"
    r"field(?:_?0x|_)?|"
    r"unused(?:_?0x|_)?|"
    r"padding(?:_?0x|_)?"
    r")[0-9a-f]{2,}\b",
    re.IGNORECASE,
)
OPAQUE_STORAGE = re.compile(r"\bunknown_fields\s*\(")


@dataclass(frozen=True)
class Finding:
    category: str
    path: str
    line: int
    match: str


def source_paths(inputs: list[str], include_probes: bool, include_modern: bool) -> list[Path]:
    paths: set[Path] = set()
    for raw in inputs:
        candidate = (ROOT / raw).resolve()
        if not candidate.is_relative_to(ROOT):
            raise ValueError(f"path escapes repository: {raw}")
        if not candidate.exists():
            raise ValueError(f"path does not exist: {raw}")
        discovered = [candidate] if candidate.is_file() else candidate.rglob("*")
        for path in discovered:
            if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
                continue
            relative = path.relative_to(ROOT)
            if not include_modern and relative.parts[:2] == ("src", "modern"):
                continue
            if not include_probes and "probe" in path.stem.lower():
                continue
            paths.add(path)
    return sorted(paths)


def line_findings(path: Path, line_number: int, line: str) -> list[Finding]:
    relative = path.relative_to(ROOT).as_posix()
    findings: list[Finding] = []
    patterns = (
        ("raw-member-access", RAW_MEMBER_ACCESS),
        ("absolute-address", ABSOLUTE_CAST),
        ("anonymous-identifier", ANONYMOUS_IDENTIFIER),
        ("opaque-storage", OPAQUE_STORAGE),
    )
    for category, pattern in patterns:
        for match in pattern.finditer(line):
            findings.append(
                Finding(
                    category=category,
                    path=relative,
                    line=line_number,
                    match=match.group(0).strip(),
                )
            )
    if not line.lstrip().startswith("#define"):
        for match in ABSOLUTE_MACRO.finditer(line):
            findings.append(
                Finding(
                    category="absolute-address",
                    path=relative,
                    line=line_number,
                    match=match.group(0).strip(),
                )
            )
    return findings


def scan(paths: list[Path]) -> list[Finding]:
    findings: list[Finding] = []
    for path in paths:
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError as exc:
            raise ValueError(f"source is not UTF-8: {path.relative_to(ROOT)}") from exc
        for line_number, line in enumerate(lines, start=1):
            findings.extend(line_findings(path, line_number, line))
    return findings


def report(paths: list[Path], findings: list[Finding]) -> dict[str, object]:
    by_category = Counter(item.category for item in findings)
    by_file = Counter(item.path for item in findings)
    return {
        "schema_version": 1,
        "scope": {
            "files_scanned": len(paths),
            "note": "Heuristic routing only; candidate counts are not semantic progress.",
        },
        "category_counts": {name: by_category.get(name, 0) for name in CATEGORIES},
        "file_counts": dict(sorted(by_file.items(), key=lambda item: (-item[1], item[0]))),
        "findings": [asdict(item) for item in findings],
    }


def print_text(data: dict[str, object], top: int, details: bool) -> None:
    scope = data["scope"]
    assert isinstance(scope, dict)
    print("TH08 semantic-debt candidates (heuristic; not a progress claim)")
    print(f"files scanned: {scope['files_scanned']}")
    print("categories:")
    category_counts = data["category_counts"]
    assert isinstance(category_counts, dict)
    for category in CATEGORIES:
        print(f"  {category}: {category_counts[category]}")

    file_counts = data["file_counts"]
    assert isinstance(file_counts, dict)
    print(f"top files (maximum {top}):")
    for path, count in list(file_counts.items())[:top]:
        print(f"  {count:5d}  {path}")

    if details:
        print("findings:")
        findings = data["findings"]
        assert isinstance(findings, list)
        for item in findings:
            assert isinstance(item, dict)
            print(
                f"  {item['path']}:{item['line']}: "
                f"{item['category']}: {item['match']}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "Candidate counts deliberately never fail CI and must not be used as "
            "semantic completion percentages."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--path",
        action="append",
        default=[],
        metavar="PATH",
        help="repository-relative source file or directory (repeatable; default: src)",
    )
    parser.add_argument(
        "--category",
        action="append",
        choices=CATEGORIES,
        help="keep only this category (repeatable)",
    )
    parser.add_argument(
        "--include-probes",
        action="store_true",
        help="include source files whose stem contains 'Probe'",
    )
    parser.add_argument(
        "--include-modern",
        action="store_true",
        help="include src/modern portability backends",
    )
    parser.add_argument("--details", action="store_true", help="print every finding")
    parser.add_argument("--json", action="store_true", help="emit the complete report as JSON")
    parser.add_argument(
        "--top",
        type=int,
        default=20,
        metavar="N",
        help="maximum files in the text summary (default: 20)",
    )
    args = parser.parse_args()

    if args.top < 1:
        parser.error("--top must be at least 1")

    try:
        paths = source_paths(args.path or ["src"], args.include_probes, args.include_modern)
        findings = scan(paths)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    if args.category:
        selected = set(args.category)
        findings = [item for item in findings if item.category in selected]
    data = report(paths, findings)
    if args.json:
        json.dump(data, sys.stdout, indent=2, sort_keys=True)
        print()
    else:
        print_text(data, args.top, args.details)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
