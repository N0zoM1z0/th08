#!/usr/bin/env python3
"""Build and replay accepted exact comparisons for a unit, object, or ledger."""

from __future__ import annotations

import argparse
import csv
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tomllib
from types import ModuleType


ROOT = Path(__file__).resolve().parents[2]
CONFIG = ROOT / "config"


def load_comparator() -> ModuleType:
    scripts = ROOT / "scripts"
    sys.path.insert(0, str(scripts))
    spec = importlib.util.spec_from_file_location(
        "th08_compare_function", scripts / "compare-function.py"
    )
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load scripts/compare-function.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def accepted_units() -> set[str]:
    with (CONFIG / "matches.csv").open(newline="", encoding="utf-8") as stream:
        return {
            row["unit"]
            for row in csv.DictReader(stream)
            if row["status"] == "matching"
        }


def object_matches(value: str, requested: set[str]) -> bool:
    path = Path(value)
    return value in requested or path.name in requested


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "Examples:\n"
            "  python3 scripts/analysis/verify-exact-units.py "
            "--unit player-option-orbit-0044ee70\n"
            "  python3 scripts/analysis/verify-exact-units.py "
            "--object build/probes/PlayerOptionProbe.obj\n"
            "  python3 scripts/analysis/verify-exact-units.py --all\n\n"
            "--all performs a single-job cold objdiff build before replay. "
            "Use --reuse-build only for a non-attesting diagnostic replay."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    selection = parser.add_mutually_exclusive_group(required=True)
    selection.add_argument(
        "--unit", action="append", default=[], help="unit name; repeat for several"
    )
    selection.add_argument(
        "--object",
        action="append",
        default=[],
        help="object path or basename; repeat for several",
    )
    selection.add_argument(
        "--all", action="store_true", help="check every accepted exact unit"
    )
    parser.add_argument(
        "--include-unaccepted",
        action="store_true",
        help="allow explicitly selected configured units absent from matches.csv",
    )
    parser.add_argument("--fail-fast", action="store_true")
    parser.add_argument("--json", action="store_true")
    parser.add_argument(
        "--reuse-build",
        action="store_true",
        help=(
            "compare existing objects without building; diagnostic only and "
            "not sufficient for an aggregate exact-state claim"
        ),
    )
    parser.add_argument(
        "--target", type=Path, default=ROOT / "resources" / "th08.exe"
    )
    args = parser.parse_args()
    try:
        with (CONFIG / "match-units.toml").open("rb") as stream:
            units = tomllib.load(stream)["units"]
        accepted = accepted_units()
        known = {str(unit["name"]) for unit in units}
        missing = sorted(set(args.unit) - known)
        if missing:
            raise ValueError("unknown units: " + ", ".join(missing))
        selected = list(units)
        if args.unit:
            selected = [unit for unit in selected if unit["name"] in args.unit]
        elif args.object:
            requested = set(args.object)
            selected = [
                unit
                for unit in selected
                if object_matches(str(unit["object"]), requested)
            ]
        if not args.include_unaccepted:
            selected = [unit for unit in selected if unit["name"] in accepted]
        selected.sort(key=lambda unit: (int(unit["target_address"]), unit["name"]))
        if not selected:
            raise ValueError("selection contains no comparison units")
        target = args.target.expanduser().resolve()
        if not args.reuse_build:
            build_command = [
                sys.executable,
                "scripts/build.py",
                "--build-type=objdiffbuild",
            ]
            if args.all:
                build_command.append("--fresh")
            else:
                build_command.extend(sorted({str(unit["object"]) for unit in selected}))
            print(
                "exact-unit batch: "
                + ("cold-building all configured objects" if args.all else "building selected objects"),
                file=sys.stderr if args.json else sys.stdout,
                flush=True,
            )
            subprocess.run(
                build_command,
                cwd=ROOT,
                check=True,
                stdout=sys.stderr if args.json else None,
            )
        elif args.all:
            print(
                "warning: --reuse-build skips cold-build attestation; "
                "do not publish aggregate exact totals from this run",
                file=sys.stderr,
            )
        comparator = load_comparator()
    except (
        OSError,
        KeyError,
        TypeError,
        ValueError,
        RuntimeError,
        subprocess.CalledProcessError,
        tomllib.TOMLDecodeError,
    ) as exc:
        print(f"error: exact-unit batch: {exc}")
        return 1

    reports: list[dict[str, object]] = []
    for index, unit in enumerate(selected, start=1):
        try:
            report = comparator.compare(unit, target)
        except (OSError, KeyError, TypeError, ValueError) as exc:
            report = {"unit": unit["name"], "result": "error", "error": str(exc)}
        reports.append(report)
        if not args.json:
            detail = ""
            if report.get("result") == "exact":
                detail = f" {report.get('matched_bytes', 0)}/{report.get('size', 0)}"
            elif report.get("error"):
                detail = f" {report['error']}"
            print(f"[{index}/{len(selected)}] {unit['name']}: {report.get('result')}{detail}")
        if args.fail_fast and report.get("result") != "exact":
            break

    failures = [report for report in reports if report.get("result") != "exact"]
    payload = {
        "result": "exact" if not failures and len(reports) == len(selected) else "failed",
        "build": (
            "reused"
            if args.reuse_build
            else ("cold" if args.all else "incremental")
        ),
        "selected": len(selected),
        "checked": len(reports),
        "exact": len(reports) - len(failures),
        "failures": failures,
    }
    if args.json:
        print(json.dumps(payload, indent=2))
    else:
        print(
            f"exact-unit batch: {payload['exact']}/{payload['checked']} exact "
            f"({payload['selected']} selected)"
        )
    return 0 if payload["result"] == "exact" else 1


if __name__ == "__main__":
    raise SystemExit(main())
