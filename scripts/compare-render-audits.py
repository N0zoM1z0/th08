#!/usr/bin/env python3
"""Compare semantic render-audit observations across modern TH08 ports."""

from __future__ import annotations

import argparse
import collections
import csv
import math
from pathlib import Path


KEY_FIELDS = ("stage", "frame", "enemy_index")
EXACT_FIELDS = (
    "boss",
    "draw_group",
    "script",
    "sprite",
    "anm_file",
    "loaded_anm",
    "visible",
    "draw_enabled",
    "flag17",
    "color1",
    "color2",
    "render_color",
    "anchor",
    "source_pixels",
    "source_visible",
    "source_colorful",
    "source_white",
    "source_edge",
    "expected_visible",
    "expected_colorful",
    "expected_white",
    "draw_result",
)
FLOAT_FIELDS = (
    "x",
    "y",
    "width",
    "height",
    "rotation",
    "screen_x",
    "screen_y",
    "u0",
    "v0",
    "u1",
    "v1",
)
HARD_STATUSES = {
    "missing-sprite",
    "missing-texture",
    "invalid-geometry",
    "source-unavailable",
    "empty-source",
}
SUSPECT_STATUSES = {
    "not-queued",
    "no-pixel-delta",
    "color-loss-suspect",
    "white-output-suspect",
}
CLIPPED_STATUSES = {"offscreen", "partially-offscreen"}
TINTED_STATUSES = {"vm-transparent-output", "vm-tinted-output"}
KNOWN_STATUSES = (
    HARD_STATUSES
    | SUSPECT_STATUSES
    | CLIPPED_STATUSES
    | TINTED_STATUSES
    | {"ok", "unprobed"}
)
SCHEMA_FIELDS = {
    "schema_version",
    "status",
    "probe_pixels",
    "changed_pixels",
    "changed_colorful",
    "changed_chromatic",
    "changed_white",
    "absolute_rgb_difference",
    *KEY_FIELDS,
    *EXACT_FIELDS,
    *FLOAT_FIELDS,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("--min-overlap", type=float, default=0.95)
    parser.add_argument("--float-tolerance", type=float, default=0.001)
    parser.add_argument("--max-differences", type=int, default=0)
    return parser.parse_args()


def load(path: Path) -> dict[tuple[str, str, str], dict[str, str]]:
    if not path.is_file():
        raise SystemExit(f"render audit does not exist: {path}")
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        fieldnames = reader.fieldnames or []
        fields = set(fieldnames)
        missing = SCHEMA_FIELDS - fields
        unknown = fields - SCHEMA_FIELDS
        duplicate = sorted(
            field for field, count in collections.Counter(fieldnames).items()
            if count > 1
        )
        if missing or unknown or duplicate:
            raise SystemExit(
                f"render audit {path} schema columns differ from version 1: "
                f"missing={sorted(missing)} unknown={sorted(unknown)} "
                f"duplicate={duplicate}"
            )
        rows = list(reader)
    versions = {row["schema_version"] for row in rows}
    if versions != {"1"}:
        raise SystemExit(f"render audit {path} has unsupported schema versions: {sorted(versions)}")
    indexed: dict[tuple[str, str, str], dict[str, str]] = {}
    for row in rows:
        if row["status"] not in KNOWN_STATUSES:
            raise SystemExit(f"render audit {path} has unknown status: {row['status']}")
        key = tuple(row[field] for field in KEY_FIELDS)
        if key in indexed:
            raise SystemExit(f"render audit {path} has duplicate sample key: {key}")
        indexed[key] = row
    if not indexed:
        raise SystemExit(f"render audit has no samples: {path}")
    return indexed


def status_class(status: str) -> str:
    if status in HARD_STATUSES:
        return "hard-failure"
    if status in SUSPECT_STATUSES:
        return "suspect"
    if status in CLIPPED_STATUSES:
        return "clipped"
    if status in TINTED_STATUSES:
        return "tinted"
    if status == "ok":
        return "ok"
    if status == "unprobed":
        return "unprobed"
    return f"unknown:{status}"


def framebuffer_visible(row: dict[str, str]) -> bool | None:
    if status_class(row["status"]) in {"clipped", "tinted", "unprobed"}:
        return None
    if int(row["probe_pixels"]) == 0:
        return None
    return int(row["changed_pixels"]) > 0


def main() -> int:
    args = parse_args()
    if not 0.0 <= args.min_overlap <= 1.0:
        raise SystemExit("--min-overlap must be between 0 and 1")
    if args.float_tolerance < 0.0 or args.max_differences < 0:
        raise SystemExit("tolerances and difference limits must be non-negative")

    baseline = load(args.baseline)
    candidate = load(args.candidate)
    common = sorted(set(baseline) & set(candidate))
    overlap = len(common) / max(len(baseline), len(candidate))
    print(
        f"Render-audit overlap: {len(common)}/{max(len(baseline), len(candidate))} "
        f"({overlap:.2%})"
    )
    if overlap < args.min_overlap:
        raise SystemExit(
            f"render-audit overlap {overlap:.2%} is below {args.min_overlap:.2%}"
        )

    differences: list[str] = []
    for key in common:
        expected = baseline[key]
        actual = candidate[key]
        for field in EXACT_FIELDS:
            if expected[field] != actual[field]:
                differences.append(
                    f"{key} {field}: baseline={expected[field]} candidate={actual[field]}"
                )
        for field in FLOAT_FIELDS:
            expected_float = float(expected[field])
            actual_float = float(actual[field])
            if not math.isfinite(expected_float) or not math.isfinite(actual_float):
                differences.append(
                    f"{key} {field}: non-finite baseline={expected[field]} "
                    f"candidate={actual[field]}"
                )
            elif abs(expected_float - actual_float) > args.float_tolerance:
                differences.append(
                    f"{key} {field}: baseline={expected[field]} candidate={actual[field]}"
                )
        expected_class = status_class(expected["status"])
        actual_class = status_class(actual["status"])
        if expected_class != actual_class:
            differences.append(
                f"{key} status: baseline={expected['status']} candidate={actual['status']}"
            )
        expected_visible = framebuffer_visible(expected)
        actual_visible = framebuffer_visible(actual)
        if expected_visible is not None and actual_visible is not None and expected_visible != actual_visible:
            differences.append(
                f"{key} framebuffer visibility: baseline={expected_visible} "
                f"candidate={actual_visible}"
            )

    if differences:
        print("Semantic/pixel-presence differences:")
        for difference in differences[:30]:
            print(f"  {difference}")
        if len(differences) > 30:
            print(f"  ... {len(differences) - 30} more")
    if len(differences) > args.max_differences:
        raise SystemExit(
            f"render audits differ in {len(differences)} fields; "
            f"limit is {args.max_differences}"
        )
    print("Cross-port render-audit semantic parity: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
