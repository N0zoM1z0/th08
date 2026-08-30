#!/usr/bin/env python3
"""Target-independent contract tests for the render-audit checker."""

from __future__ import annotations

import csv
from pathlib import Path
import subprocess
import sys
import tempfile


CHECKER = Path(__file__).with_name("check-render-audit.py")
COMPARATOR = Path(__file__).with_name("compare-render-audits.py")
FIELDS = [
    "schema_version",
    "stage",
    "frame",
    "enemy_index",
    "anm_file",
    "script",
    "sprite",
    "status",
    "boss",
    "draw_group",
    "loaded_anm",
    "visible",
    "draw_enabled",
    "flag17",
    "color1",
    "color2",
    "render_color",
    "anchor",
    "rotation",
    "screen_x",
    "screen_y",
    "x",
    "y",
    "width",
    "height",
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
]


def row(status: str, frame: int = 1) -> dict[str, str | int]:
    return {
        "schema_version": "1",
        "stage": 3,
        "frame": frame,
        "enemy_index": 7,
        "anm_file": 4,
        "script": 54,
        "sprite": 81,
        "status": status,
        "boss": 1,
        "draw_group": 0,
        "loaded_anm": 4,
        "visible": 1,
        "draw_enabled": 1,
        "flag17": 0,
        "color1": "ffffffff",
        "color2": "00000000",
        "render_color": "ffffffff",
        "anchor": 0,
        "rotation": "0.0",
        "screen_x": "192.0",
        "screen_y": "128.0",
        "x": "192.0",
        "y": "128.0",
        "width": "64.0",
        "height": "80.0",
        "u0": "0.0",
        "v0": "0.0",
        "u1": "0.25",
        "v1": "0.3125",
        "source_pixels": 5120,
        "source_visible": 2104,
        "source_colorful": 1151,
        "source_white": 281,
        "source_edge": 0,
        "expected_visible": 2104,
        "expected_colorful": 1151,
        "expected_white": 281,
        "draw_result": 0,
        "probe_pixels": 5561,
        "changed_pixels": 2104,
        "changed_colorful": 1000,
        "changed_chromatic": 1000,
        "changed_white": 20,
        "absolute_rgb_difference": 123456,
    }


def check_case(
    directory: Path,
    name: str,
    rows: list[dict[str, str | int]],
    expected_success: bool,
    expected_text: str,
    extra_args: tuple[str, ...] = (),
    fieldnames: list[str] = FIELDS,
) -> None:
    audit = directory / f"{name}.csv"
    with audit.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)
    result = subprocess.run(
        [sys.executable, str(CHECKER), str(audit), *extra_args],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    if (result.returncode == 0) != expected_success or expected_text not in result.stdout:
        raise RuntimeError(
            f"render-audit test {name!r} failed with status {result.returncode}:\n"
            f"{result.stdout}"
        )


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="th08-render-audit-test-") as raw_directory:
        directory = Path(raw_directory)
        check_case(directory, "ok", [row("ok")], True, "framebuffer draw deltas: OK")
        check_case(
            directory,
            "missing-texture",
            [row("missing-texture")],
            False,
            "structural render failures found",
        )
        check_case(
            directory,
            "repeated-no-delta",
            [row("no-pixel-delta", frame) for frame in (1, 2, 3)],
            False,
            "repeated zero-pixel enemy draws",
        )
        check_case(
            directory,
            "transient-no-delta",
            [row("no-pixel-delta", 1), row("no-pixel-delta", 2), row("ok", 3)],
            True,
            "framebuffer draw deltas: OK",
        )
        check_case(
            directory,
            "dominant-no-delta",
            [
                row("no-pixel-delta", 1),
                row("no-pixel-delta", 2),
                row("no-pixel-delta", 3),
                row("ok", 4),
            ],
            False,
            "repeated zero-pixel enemy draws",
        )
        check_case(
            directory,
            "strict-suspect",
            [row("color-loss-suspect")],
            False,
            "suspects are fatal",
            ("--strict-suspects",),
        )
        check_case(
            directory,
            "required-stage",
            [row("ok")],
            False,
            "did not sample stages",
            ("--require-stage", "4"),
        )
        unsupported = row("ok")
        unsupported["schema_version"] = "2"
        check_case(
            directory,
            "unsupported-schema",
            [unsupported],
            False,
            "unsupported render audit schema versions",
        )
        check_case(
            directory,
            "unknown-status",
            [row("mystery")],
            False,
            "unknown statuses",
        )
        unknown_column_row = row("ok")
        unknown_column_row["future_metric"] = 1
        check_case(
            directory,
            "unknown-column",
            [unknown_column_row],
            False,
            "unknown=['future_metric']",
            fieldnames=[*FIELDS, "future_metric"],
        )

        baseline_path = directory / "compare-baseline.csv"
        candidate_path = directory / "compare-candidate.csv"
        baseline_rows = [row("ok", 1), row("ok", 2)]
        for path, audit_rows in (
            (baseline_path, baseline_rows),
            (candidate_path, baseline_rows),
        ):
            with path.open("w", newline="", encoding="utf-8") as stream:
                writer = csv.DictWriter(stream, fieldnames=FIELDS)
                writer.writeheader()
                writer.writerows(audit_rows)
        result = subprocess.run(
            [sys.executable, str(COMPARATOR), str(baseline_path), str(candidate_path)],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if result.returncode != 0 or "semantic parity: OK" not in result.stdout:
            raise RuntimeError(f"identical render-audit comparison failed:\n{result.stdout}")

        changed_rows = [row("ok", 1), row("ok", 2)]
        changed_rows[1]["sprite"] = 82
        with candidate_path.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=FIELDS)
            writer.writeheader()
            writer.writerows(changed_rows)
        result = subprocess.run(
            [sys.executable, str(COMPARATOR), str(baseline_path), str(candidate_path)],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if result.returncode == 0 or "sprite:" not in result.stdout:
            raise RuntimeError(f"semantic render-audit difference was missed:\n{result.stdout}")
    print("Render-audit checker contract tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
