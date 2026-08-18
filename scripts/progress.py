#!/usr/bin/env python3
"""Generate conservative source and exact-match progress from the ledgers."""

from __future__ import annotations

import csv
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
FUNCTIONS = ROOT / "config" / "reccmp-functions.csv"
MAPPING = ROOT / "config" / "mapping.csv"
IMPLEMENTED = ROOT / "config" / "implemented.csv"
MATCHES = ROOT / "config" / "matches.csv"
MARKDOWN = ROOT / "docs" / "PROGRESS.md"
SVG = ROOT / "resources" / "progress.svg"


def load() -> tuple[
    list[dict[str, str]], dict[int, int], set[str], list[dict[str, str]]
]:
    with FUNCTIONS.open(newline="", encoding="utf-8") as stream:
        functions = list(csv.DictReader(stream))

    sizes: dict[int, int] = {}
    with MAPPING.open(newline="", encoding="utf-8") as stream:
        for line, row in enumerate(csv.reader(stream), start=1):
            if len(row) < 3:
                raise ValueError(f"mapping.csv:{line}: expected at least three columns")
            address = int(row[1], 0)
            if address in sizes:
                raise ValueError(f"mapping.csv:{line}: duplicate address {address:#x}")
            sizes[address] = int(row[2], 0)

    implemented = {
        line.strip()
        for line in IMPLEMENTED.read_text(encoding="utf-8").splitlines()
        if line.strip()
    }
    with MATCHES.open(newline="", encoding="utf-8") as stream:
        matches = list(csv.DictReader(stream))
    return functions, sizes, implemented, matches


def render() -> tuple[str, str]:
    functions, sizes, implemented, matches = load()
    authored = [row for row in functions if row["type"] == "function"]
    authored_names = {row["name"] for row in authored}
    unknown = implemented - authored_names
    if unknown:
        raise ValueError(
            "implemented.csv contains names absent from authored reccmp rows: "
            + ", ".join(sorted(unknown)[:5])
        )

    missing_sizes = [row for row in authored if int(row["address"], 0) not in sizes]
    if missing_sizes:
        raise ValueError(f"{len(missing_sizes)} authored functions lack mapping sizes")

    source_rows = [row for row in authored if row["name"] in implemented]
    # Authored names are not unique: overloaded members can legitimately share the
    # same logical name (for example the two Float3 constructors).  Exact evidence
    # is range evidence, so bind matches to the authored inventory by target address
    # and then require the logical name/size to agree at that address.
    authored_by_address = {int(row["address"], 0): row for row in authored}
    exact_rows = [row for row in matches if row["status"] == "matching"]
    for row in exact_rows:
        address = int(row["address"], 0)
        authored_row = authored_by_address.get(address)
        if authored_row is None:
            raise ValueError(f"exact match is absent from authored inventory: {row['name']}")
        if authored_row["name"] != row["name"] or sizes[address] != int(row["size"]):
            raise ValueError(f"exact match name/address/size differs from inventory: {row['name']}")
        if row["match_percent"] != "100.00" or not row["evidence"]:
            raise ValueError(f"exact match lacks 100% evidence: {row['name']}")
    total_bytes = sum(sizes[int(row["address"], 0)] for row in authored)
    source_bytes = sum(sizes[int(row["address"], 0)] for row in source_rows)
    exact_bytes = sum(int(row["size"]) for row in exact_rows)
    function_pct = 100 * len(source_rows) / len(authored) if authored else 0.0
    byte_pct = 100 * source_bytes / total_bytes if total_bytes else 0.0
    exact_function_pct = 100 * len(exact_rows) / len(authored) if authored else 0.0
    exact_byte_pct = 100 * exact_bytes / total_bytes if total_bytes else 0.0
    library_count = sum(row["type"] == "library" for row in functions)

    markdown = "\n".join(
        [
            "# Reconstruction progress",
            "",
            "Generated from `config/reccmp-functions.csv`, `config/mapping.csv`,",
            "and `config/implemented.csv`.",
            "",
            "> `implemented.csv` records source presence only. Exact figures below",
            "> count only reproducible 100% comparisons tracked in `config/matches.csv`.",
            "",
            f"- Source-present authored functions: **{len(source_rows):,} / {len(authored):,} ({function_pct:.2f}%)**",
            f"- Source-present authored bytes: **{source_bytes:,} / {total_bytes:,} ({byte_pct:.2f}%)**",
            f"- Exact authored functions: **{len(exact_rows):,} / {len(authored):,} ({exact_function_pct:.2f}%)**",
            f"- Exact authored bytes: **{exact_bytes:,} / {total_bytes:,} ({exact_byte_pct:.2f}%)**",
            f"- Inventory-classified library functions: **{library_count:,}**",
            "",
            "The public SVG follows the TH07 convention and visualizes exact bytes only;",
            "source presence never fills the progress bar.",
            "",
            "Function sizes and library classification are imported analysis seeds and",
            "must be reconciled against the hash-attested target before exact acceptance.",
            "",
        ]
    )

    filled = 440 * exact_byte_pct / 100
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" width="560" height="116" role="img" aria-label="TH08 reconstruction progress {exact_byte_pct:.2f}% exact authored bytes">
  <rect width="560" height="116" rx="8" fill="#1f2335"/>
  <text x="24" y="32" fill="#f4f4f5" font-family="sans-serif" font-size="16" font-weight="600">TH08 source reconstruction</text>
  <text x="536" y="32" fill="#f4f4f5" text-anchor="end" font-family="monospace" font-size="14">{exact_byte_pct:.2f}% exact bytes</text>
  <rect x="24" y="48" width="440" height="14" rx="7" fill="#3b4058"/>
  <rect x="24" y="48" width="{filled:.2f}" height="14" rx="7" fill="#9b6de3"/>
  <text x="24" y="88" fill="#c8cad2" font-family="sans-serif" font-size="13">Authored exact: {len(exact_rows):,} / {len(authored):,} functions · {exact_bytes:,} / {total_bytes:,} bytes</text>
  <text x="24" y="108" fill="#c8cad2" font-family="sans-serif" font-size="13">Library exact: not counted until TH08-specific archives are attested</text>
</svg>
'''
    return markdown, svg


def main() -> int:
    markdown, svg = render()
    if "--check" in sys.argv:
        stale = []
        if not MARKDOWN.exists() or MARKDOWN.read_text(encoding="utf-8") != markdown:
            stale.append(str(MARKDOWN.relative_to(ROOT)))
        if not SVG.exists() or SVG.read_text(encoding="utf-8") != svg:
            stale.append(str(SVG.relative_to(ROOT)))
        if stale:
            print("stale generated progress: " + ", ".join(stale))
            return 1
        print("progress artifacts are current")
        return 0

    MARKDOWN.parent.mkdir(parents=True, exist_ok=True)
    MARKDOWN.write_text(markdown, encoding="utf-8")
    SVG.write_text(svg, encoding="utf-8")
    print(f"updated {MARKDOWN.relative_to(ROOT)} and {SVG.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
