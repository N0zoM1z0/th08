#!/usr/bin/env python3
"""Validate a modern-port semantic + framebuffer enemy render audit."""

from __future__ import annotations

import argparse
import collections
import csv
from pathlib import Path


HARD_FAILURES = {
    "missing-sprite",
    "missing-texture",
    "invalid-geometry",
    "source-unavailable",
    "empty-source",
}
SUSPECTS = {
    "not-queued",
    "no-pixel-delta",
    "color-loss-suspect",
    "white-output-suspect",
}
INFORMATIONAL = {
    "ok",
    "offscreen",
    "partially-offscreen",
    "unprobed",
    "vm-transparent-output",
    "vm-tinted-output",
}
KNOWN_STATUSES = HARD_FAILURES | SUSPECTS | INFORMATIONAL
SCHEMA_COLUMNS = {
    "schema_version",
    "stage",
    "frame",
    "enemy_index",
    "boss",
    "draw_group",
    "anm_file",
    "script",
    "sprite",
    "loaded_anm",
    "visible",
    "draw_enabled",
    "flag17",
    "color1",
    "color2",
    "render_color",
    "x",
    "y",
    "width",
    "height",
    "anchor",
    "rotation",
    "screen_x",
    "screen_y",
    "u0",
    "v0",
    "u1",
    "v1",
    "source_pixels",
    "source_visible",
    "source_colorful",
    "source_white",
    "source_edge",
    "expected_visible",
    "expected_colorful",
    "expected_white",
    "draw_result",
    "probe_pixels",
    "changed_pixels",
    "changed_colorful",
    "changed_chromatic",
    "changed_white",
    "absolute_rgb_difference",
    "status",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("audit", type=Path)
    parser.add_argument("--require-stage", type=int, action="append", default=[])
    parser.add_argument("--min-samples", type=int, default=1)
    parser.add_argument("--strict-suspects", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.min_samples < 1:
        raise SystemExit("--min-samples must be positive")
    if not args.audit.is_file():
        raise SystemExit(f"render audit was not produced: {args.audit}")

    with args.audit.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        fieldnames = reader.fieldnames or []
        columns = set(fieldnames)
        missing_columns = SCHEMA_COLUMNS - columns
        unknown_columns = columns - SCHEMA_COLUMNS
        duplicate_columns = sorted(
            column for column, count in collections.Counter(fieldnames).items()
            if count > 1
        )
        if missing_columns or unknown_columns or duplicate_columns:
            raise SystemExit(
                "render audit schema columns differ from version 1: "
                f"missing={sorted(missing_columns)} "
                f"unknown={sorted(unknown_columns)} "
                f"duplicate={duplicate_columns}"
            )
        rows = list(reader)
    if len(rows) < args.min_samples:
        raise SystemExit(
            f"render audit has {len(rows)} samples; expected at least {args.min_samples}"
        )

    versions = {row["schema_version"] for row in rows}
    if versions != {"1"}:
        raise SystemExit(f"unsupported render audit schema versions: {sorted(versions)}")

    stages = {int(row["stage"]) for row in rows}
    missing_stages = sorted(set(args.require_stage) - stages)
    if missing_stages:
        raise SystemExit(f"render audit did not sample stages: {missing_stages}")

    counts = collections.Counter(row["status"] for row in rows)
    unknown_statuses = sorted(set(counts) - KNOWN_STATUSES)
    if unknown_statuses:
        raise SystemExit(f"render audit contains unknown statuses: {unknown_statuses}")
    hard_rows = [row for row in rows if row["status"] in HARD_FAILURES]
    suspect_rows = [row for row in rows if row["status"] in SUSPECTS]

    # A single no-delta sample can be a transient depth/overlap event. Treat a
    # VM/sprite whose zero-delta observations dominate its successful samples
    # by at least 3:1 as a deterministic failure.
    samples_by_sprite: dict[tuple[str, str, str, str], list[dict[str, str]]] = (
        collections.defaultdict(list)
    )
    for row in rows:
        samples_by_sprite[
            (row["stage"], row["anm_file"], row["script"], row["sprite"])
        ].append(row)
    repeated_no_delta = []
    for key, samples in samples_by_sprite.items():
        no_delta = [row for row in samples if row["status"] == "no-pixel-delta"]
        okay = [row for row in samples if row["status"] == "ok"]
        if len(no_delta) >= 3 and len(no_delta) >= 3 * max(1, len(okay)):
            repeated_no_delta.append((key, len(no_delta), len(okay)))

    print(f"Render audit: {len(rows)} samples across stages {sorted(stages)}")
    for status, count in sorted(counts.items()):
        print(f"  {status}: {count}")
    if suspect_rows:
        print("Suspect samples (stage/frame/enemy/sprite/status):")
        for row in suspect_rows[:20]:
            print(
                f"  {row['stage']}/{row['frame']}/{row['enemy_index']}/"
                f"{row['sprite']}/{row['status']}"
            )
        if len(suspect_rows) > 20:
            print(f"  ... {len(suspect_rows) - 20} more")

    if hard_rows:
        first = hard_rows[0]
        raise SystemExit(
            "structural render failures found; first is "
            f"stage={first['stage']} frame={first['frame']} "
            f"enemy={first['enemy_index']} status={first['status']}"
        )
    if repeated_no_delta:
        raise SystemExit(f"repeated zero-pixel enemy draws: {repeated_no_delta[:10]}")
    if args.strict_suspects and suspect_rows:
        raise SystemExit("render audit suspects are fatal in --strict-suspects mode")

    print("Enemy texture regions and framebuffer draw deltas: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
