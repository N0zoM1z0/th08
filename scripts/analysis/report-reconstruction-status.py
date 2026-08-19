#!/usr/bin/env python3
"""Report current authored coverage and classified library inventory.

Use this before selecting work.  Authored sizes come from mapping.csv.  Library
rows may not have accepted function boundaries yet, so unknown sizes are shown
explicitly instead of being inferred from adjacent addresses.
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import tomllib


ROOT = Path(__file__).resolve().parents[2]
CONFIG = ROOT / "config"


def read_mapping() -> dict[int, dict[str, object]]:
    mapping: dict[int, dict[str, object]] = {}
    with (CONFIG / "mapping.csv").open(newline="", encoding="utf-8") as stream:
        for line, row in enumerate(csv.reader(stream), start=1):
            if len(row) < 3:
                raise ValueError(f"mapping.csv:{line}: expected at least three columns")
            address = int(row[1], 0)
            if address in mapping:
                raise ValueError(f"mapping.csv:{line}: duplicate address {address:#x}")
            mapping[address] = {"name": row[0], "size": int(row[2], 0)}
    return mapping


def read_single_column(path: Path) -> set[str]:
    values: set[str] = set()
    with path.open(newline="", encoding="utf-8") as stream:
        for line, row in enumerate(csv.reader(stream), start=1):
            if len(row) != 1 or not row[0]:
                raise ValueError(f"{path.name}:{line}: expected one non-empty value")
            if row[0] in values:
                raise ValueError(f"{path.name}:{line}: duplicate value {row[0]!r}")
            values.add(row[0])
    return values


def load_rows() -> list[dict[str, object]]:
    mapping = read_mapping()
    implemented = read_single_column(CONFIG / "implemented.csv")
    with (CONFIG / "matches.csv").open(newline="", encoding="utf-8") as stream:
        matched = {
            int(row["address"], 0)
            for row in csv.DictReader(stream)
            if row["status"] == "matching"
        }
    with (CONFIG / "match-units.toml").open("rb") as stream:
        manifest = tomllib.load(stream)
    units: dict[int, list[str]] = {}
    for unit in manifest.get("units", []):
        units.setdefault(int(unit["target_address"]), []).append(str(unit["name"]))

    rows: list[dict[str, object]] = []
    with (CONFIG / "reccmp-functions.csv").open(
        newline="", encoding="utf-8"
    ) as stream:
        for line, row in enumerate(csv.DictReader(stream), start=2):
            address = int(row["address"], 0)
            category = "authored" if row["type"] == "function" else row["type"]
            mapped = mapping.get(address)
            if category == "authored" and mapped is None:
                raise ValueError(
                    f"reccmp-functions.csv:{line}: authored row has no mapping at "
                    f"{address:#x}"
                )
            rows.append(
                {
                    "address": address,
                    "size": int(mapped["size"]) if mapped is not None else None,
                    "name": row["name"],
                    "mapping_name": mapped["name"] if mapped is not None else None,
                    "category": category,
                    "source_present": row["name"] in implemented
                    if category == "authored"
                    else False,
                    "exact": address in matched,
                    "units": sorted(units.get(address, [])),
                }
            )
    return rows


def summary(rows: list[dict[str, object]]) -> dict[str, object]:
    authored = [row for row in rows if row["category"] == "authored"]
    library = [row for row in rows if row["category"] == "library"]
    return {
        "authored": {
            "functions": len(authored),
            "bytes": sum(int(row["size"]) for row in authored),
            "source_present_functions": sum(
                bool(row["source_present"]) for row in authored
            ),
            "source_present_bytes": sum(
                int(row["size"]) for row in authored if row["source_present"]
            ),
            "exact_functions": sum(bool(row["exact"]) for row in authored),
            "exact_bytes": sum(
                int(row["size"]) for row in authored if row["exact"]
            ),
        },
        "library": {
            "functions": len(library),
            "sized_functions": sum(row["size"] is not None for row in library),
            "known_bytes": sum(
                int(row["size"]) for row in library if row["size"] is not None
            ),
            "with_match_units": sum(bool(row["units"]) for row in library),
        },
    }


def selected_rows(
    rows: list[dict[str, object]], category: str, state: str
) -> list[dict[str, object]]:
    selected = [
        row
        for row in rows
        if category == "all" or row["category"] == category
    ]
    if state == "source-missing":
        selected = [
            row
            for row in selected
            if row["category"] == "authored" and not row["source_present"]
        ]
    elif state == "non-exact":
        selected = [row for row in selected if not row["exact"]]
    elif state == "without-unit":
        selected = [row for row in selected if not row["units"]]
    return selected


def text_summary(report: dict[str, object]) -> str:
    authored = report["authored"]
    library = report["library"]
    assert isinstance(authored, dict) and isinstance(library, dict)
    return "\n".join(
        [
            "Authored: "
            f"source {authored['source_present_functions']}/{authored['functions']} "
            f"({authored['source_present_bytes']}/{authored['bytes']} bytes), "
            f"exact {authored['exact_functions']}/{authored['functions']} "
            f"({authored['exact_bytes']}/{authored['bytes']} bytes)",
            "Library: "
            f"{library['functions']} classified functions; mapping sizes for "
            f"{library['sized_functions']} ({library['known_bytes']} known bytes); "
            f"{library['with_match_units']} configured match units",
        ]
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "Examples:\n"
            "  python3 scripts/analysis/report-reconstruction-status.py --summary\n"
            "  python3 scripts/analysis/report-reconstruction-status.py\n"
            "  python3 scripts/analysis/report-reconstruction-status.py "
            "--category library --state without-unit --sort size"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--category",
        choices=("authored", "library", "all"),
        default="authored",
    )
    parser.add_argument(
        "--state",
        choices=("all", "source-missing", "non-exact", "without-unit"),
        default="non-exact",
    )
    parser.add_argument("--summary", action="store_true", help="print totals only")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--sort", choices=("address", "size"), default="address")
    args = parser.parse_args()

    try:
        rows = load_rows()
        report = summary(rows)
        selected = selected_rows(rows, args.category, args.state)
        selected.sort(
            key=(
                (lambda row: int(row["address"]))
                if args.sort == "address"
                else (
                    lambda row: (
                        row["size"] is None,
                        -int(row["size"] or 0),
                        int(row["address"]),
                    )
                )
            )
        )
    except (OSError, KeyError, TypeError, ValueError, tomllib.TOMLDecodeError) as exc:
        print(f"error: inventory report: {exc}")
        return 1

    if args.json:
        payload: dict[str, object] = {"summary": report}
        if not args.summary:
            payload["rows"] = [
                {**row, "address": f"0x{int(row['address']):08X}"}
                for row in selected
            ]
        print(json.dumps(payload, indent=2))
        return 0
    if args.summary:
        print(text_summary(report))
        return 0
    for row in selected:
        units = ",".join(str(unit) for unit in row["units"]) or "-"
        print(
            f"0x{int(row['address']):08X}\t"
            f"{row['size'] if row['size'] is not None else '?'}\t"
            f"{row['category']}\tsource={int(bool(row['source_present']))}\t"
            f"exact={int(bool(row['exact']))}\tunit={units}\t{row['name']}"
        )
    print(f"rows={len(selected)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
