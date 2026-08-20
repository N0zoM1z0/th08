#!/usr/bin/env python3
"""Rank current production objects by detailed whole-image anchor disorder.

This consumes the JSON produced by ``compare-whole-image.py`` with
``--include-anchor-details``. It is a read-only routing aid: inversions and
drift jumps can identify a current source file that merged target translation
units, but they do not prove a boundary by themselves.

Examples:
  python3 scripts/analysis/report-tu-partition-candidates.py \
    build/whole-image-anchors.json
  python3 scripts/analysis/report-tu-partition-candidates.py \
    build/whole-image-anchors.json --object AsciiManager.obj
  python3 scripts/analysis/report-tu-partition-candidates.py \
    build/whole-image-anchors.json --json > build/tu-candidates.json
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REPORT = ROOT / "build" / "whole-image-anchors.json"


def relative_display(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def load_anchors(path: Path) -> list[dict[str, object]]:
    try:
        report = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ValueError(f"whole-image report does not exist: {path}") from exc
    except json.JSONDecodeError as exc:
        raise ValueError(f"invalid whole-image JSON: {path}: {exc}") from exc

    layout = report.get("link_layout")
    if not isinstance(layout, dict) or not isinstance(layout.get("anchors"), list):
        raise ValueError(
            "report has no detailed anchors; regenerate it with "
            "scripts/compare-whole-image.py --json --include-anchor-details"
        )
    anchors = [
        item
        for item in layout["anchors"]
        if isinstance(item, dict)
        and item.get("kind") == "authored"
        and str(item.get("object", "")).lower().endswith(".obj")
    ]
    if not anchors:
        raise ValueError("report contains no authored production-object anchors")
    return anchors


def summarize_object(object_name: str, anchors: list[dict[str, object]]) -> dict[str, object]:
    linked = sorted(anchors, key=lambda item: int(item["linked_address"]))
    targets = [int(item["target_address"]) for item in linked]
    drifts = [int(item["drift"]) for item in linked]
    inversions = sum(
        targets[left] > targets[right]
        for left in range(len(targets))
        for right in range(left + 1, len(targets))
    )
    descents = sum(targets[index] < targets[index - 1] for index in range(1, len(targets)))
    drift_jumps = [abs(drifts[index] - drifts[index - 1]) for index in range(1, len(drifts))]
    return {
        "object": object_name,
        "anchor_count": len(linked),
        "target_order_inversions": inversions,
        "target_order_runs": descents + 1,
        "minimum_drift": min(drifts),
        "maximum_drift": max(drifts),
        "drift_span": max(drifts) - min(drifts),
        "maximum_absolute_drift_jump": max(drift_jumps, default=0),
        "first_linked_address": int(linked[0]["linked_address"]),
        "last_linked_address": int(linked[-1]["linked_address"]),
        "first_target_address": min(targets),
        "last_target_address": max(targets),
        "anchors": linked,
    }


def build_report(path: Path, minimum_anchors: int) -> dict[str, object]:
    grouped: dict[str, list[dict[str, object]]] = {}
    for anchor in load_anchors(path):
        grouped.setdefault(str(anchor["object"]), []).append(anchor)
    objects = [
        summarize_object(name, items)
        for name, items in grouped.items()
        if len(items) >= minimum_anchors
    ]
    objects.sort(
        key=lambda item: (
            -int(item["target_order_inversions"]),
            -int(item["drift_span"]),
            str(item["object"]).lower(),
        )
    )
    return {
        "schema_version": 1,
        "source_report": relative_display(path),
        "minimum_anchors": minimum_anchors,
        "object_count": len(objects),
        "objects": objects,
    }


def print_summary(objects: list[dict[str, object]], limit: int) -> None:
    print(
        f"{'object':28} {'anchors':>7} {'inversions':>10} {'runs':>5} "
        f"{'drift span':>12} {'max drift jump':>15}"
    )
    for item in objects[:limit]:
        print(
            f"{str(item['object']):28} {int(item['anchor_count']):7d} "
            f"{int(item['target_order_inversions']):10d} "
            f"{int(item['target_order_runs']):5d} "
            f"{int(item['drift_span']):12,d} "
            f"{int(item['maximum_absolute_drift_jump']):15,d}"
        )


def print_details(item: dict[str, object]) -> None:
    print(f"\n{item['object']} linked-order anchors:")
    previous_target: int | None = None
    previous_drift: int | None = None
    for anchor in item["anchors"]:
        target = int(anchor["target_address"])
        linked = int(anchor["linked_address"])
        drift = int(anchor["drift"])
        markers: list[str] = []
        if previous_target is not None and target < previous_target:
            markers.append("target-order reset")
        if previous_drift is not None and drift != previous_drift:
            markers.append(f"drift change {drift - previous_drift:+d}")
        suffix = f"  [{'; '.join(markers)}]" if markers else ""
        print(
            f"  L {linked:08X}  T {target:08X}  D {drift:+8d}  "
            f"{anchor['unit']}{suffix}"
        )
        previous_target = target
        previous_drift = drift


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "report",
        nargs="?",
        type=Path,
        default=DEFAULT_REPORT,
        help="detailed whole-image JSON (default: build/whole-image-anchors.json)",
    )
    parser.add_argument(
        "--object",
        action="append",
        default=[],
        help="show linked-order details for this object basename; repeatable",
    )
    parser.add_argument("--min-anchors", type=int, default=2)
    parser.add_argument("--limit", type=int, default=20)
    parser.add_argument("--json", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.min_anchors < 1:
        print("error: --min-anchors must be at least 1", file=sys.stderr)
        return 2
    if args.limit < 1:
        print("error: --limit must be at least 1", file=sys.stderr)
        return 2
    path = args.report if args.report.is_absolute() else ROOT / args.report
    try:
        report = build_report(path, args.min_anchors)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    if args.object:
        wanted = {Path(name).name.lower() for name in args.object}
        selected = [
            item for item in report["objects"] if Path(str(item["object"])).name.lower() in wanted
        ]
        found = {Path(str(item["object"])).name.lower() for item in selected}
        missing = sorted(wanted - found)
        if missing:
            print("error: object not found in detailed anchors: " + ", ".join(missing), file=sys.stderr)
            return 1
    else:
        selected = report["objects"]

    if args.json:
        output = dict(report)
        output["objects"] = selected
        print(json.dumps(output, indent=2, sort_keys=True))
        return 0

    print(f"source: {report['source_report']}")
    print_summary(selected, args.limit)
    if args.object:
        for item in selected:
            print_details(item)
    print(
        "\nInterpretation: rank disorder selects a bounded investigation; "
        "confirm every proposed TU boundary against target neighborhoods and exact object replay."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
