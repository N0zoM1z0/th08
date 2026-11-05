#!/usr/bin/env python3
"""Verify an explicit initialized-data contract, without awarding authored credit."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shlex
import struct

from match_data import (
    ROOT,
    DataError,
    attest_target,
    check,
    file_identity,
    load_contracts,
    parse_publics,
    repository_path,
)
from match_evidence import build_command
from pe_image import PEImage


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "contract", nargs="?", help="contract name from config/data-contracts.toml"
    )
    parser.add_argument(
        "--all", action="store_true", help="verify every reviewed contract"
    )
    parser.add_argument("--list", action="store_true")
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="validate schema without private inputs",
    )
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument(
        "--object",
        dest="object_path",
        type=Path,
        help="explicit owner object for one isolated contract test",
    )
    parser.add_argument(
        "--rebuild", type=Path, help="also follow data pointers in this linked PE32"
    )
    parser.add_argument(
        "--map", dest="map_path", type=Path, help="linker map paired with --rebuild"
    )
    parser.add_argument(
        "--linked-only",
        action="store_true",
        help="verify retained PE/map evidence without reading current objects",
    )
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    report = {
        "schema_version": 1,
        "result": "error",
        "scope": "selected explicit data contracts only",
        "findings": [],
        "next_commands": [],
    }
    try:
        contracts = load_contracts()
        if args.contract and args.all:
            raise DataError("REQUEST_INVALID", "choose one contract or --all")
        selected = (
            [row for row in contracts if row["name"] == args.contract]
            if args.contract
            else contracts
        )
        if not selected:
            raise DataError("CONTRACT_UNKNOWN", f"unknown contract: {args.contract}")
        if args.list or args.validate_only:
            report.update(
                result="valid" if args.validate_only else "listed",
                target_status="not_checked",
                contracts=[
                    {
                        **{key: row[key] for key in ("name", "kind", "object", "size")},
                        "target_address": f"0x{row['target_address']:08X}",
                    }
                    for row in selected
                ],
            )
        else:
            if not args.contract and not args.all:
                raise DataError(
                    "REQUEST_INVALID",
                    "choose a contract or --all; --list shows reviewed entries",
                )
            if (
                bool(args.rebuild) != bool(args.map_path)
                or args.linked_only
                and not args.rebuild
            ):
                raise DataError(
                    "REQUEST_INVALID",
                    "--rebuild and --map must be paired; --linked-only requires both",
                )
            if args.object_path and (len(selected) != 1 or args.linked_only):
                raise DataError(
                    "REQUEST_INVALID", "--object requires one COFF contract"
                )
            target = PEImage(args.target)
            attest_target(target)
            linked = (
                (PEImage(args.rebuild), parse_publics(args.map_path))
                if args.rebuild
                else None
            )
            checks = [
                check(
                    row,
                    target,
                    object_path=None
                    if args.linked_only
                    else args.object_path or repository_path(row["object"]),
                    linked=linked,
                )
                for row in selected
            ]
            report.update(
                result="verified"
                if all(row["result"] == "verified" for row in checks)
                else "mismatch",
                target_sha256=target.sha256,
                contracts=checks,
                findings=[finding for row in checks for finding in row["findings"]],
            )
            if linked:
                report["linked_inputs"] = {
                    "image": file_identity(args.rebuild),
                    "map": file_identity(args.map_path),
                }
            for row in selected:
                if (
                    next(item for item in checks if item["contract"] == row["name"])[
                        "result"
                    ]
                    != "verified"
                ):
                    report["next_commands"].extend(
                        [
                            build_command(row["object"]),
                            [
                                "python3",
                                "scripts/compare-data.py",
                                row["name"],
                                "--json",
                            ],
                        ]
                    )
    except (OSError, KeyError, IndexError, TypeError, ValueError, struct.error) as exc:
        report.update(
            result="error",
            findings=[
                exc.finding
                if isinstance(exc, DataError)
                else {"code": "DATA_INPUT_INVALID", "message": str(exc)}
            ],
        )
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print(f"data contracts: {report['result']} ({report['scope']})")
        for row in report.get("contracts", []):
            if "coff" in row:
                print(
                    f"  {row['contract']}: coff={row['coff']['status']} linked={row['linked']['status']}"
                )
            else:
                print(
                    f"  {row['name']}: {row['kind']} {row['size']} bytes in {row['object']}"
                )
        for finding in report["findings"][:12]:
            print(f"  {finding['code']}: {finding['message']}")
        for command in report["next_commands"][:3]:
            print("  next: " + shlex.join(command))
    return 0 if report["result"] in {"verified", "valid", "listed"} else 1


if __name__ == "__main__":
    raise SystemExit(main())
