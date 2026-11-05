#!/usr/bin/env python3
"""Crosswalk a Cirno-Labs TH08 checkout by target address, without match credit.

The reference is optional, ignored input. This checks its recorded target
checksum and function boundaries, not its source correctness or match claims.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from pe_image import PEImage  # noqa: E402


def git_revision(path: Path) -> str:
    return subprocess.check_output(
        ["git", "-C", str(path), "rev-parse", "HEAD"], text=True
    ).strip()


def compare(reference: Path, target: Path) -> dict[str, object]:
    image = PEImage(target)
    with (ROOT / "config/target.toml").open("rb") as stream:
        expected = tomllib.load(stream)["target"]
    if len(image.data) != expected["size"] or image.sha256 != expected["sha256"]:
        raise ValueError("canonical target identity mismatch")
    recorded = [
        line.split()[0]
        for line in (reference / "orig/th08_100d.sha1").read_text().splitlines()
        if len(line.split()) == 2 and Path(line.split()[1]).name == "th08.exe"
    ]
    if recorded != [hashlib.sha1(image.data).hexdigest()]:
        raise ValueError("reference executable checksum differs from the canonical target")

    with (ROOT / "config/mapping.csv").open(newline="") as stream:
        local = {
            int(row[1], 0): {"local_name": row[0], "local_size": int(row[2], 0)}
            for row in csv.reader(stream)
        }
    with (ROOT / "config/reccmp-functions.csv").open(newline="") as stream:
        categories = {int(row["address"], 0): row["type"] for row in csv.DictReader(stream)}
    authored = [address for address in local if categories[address] == "function"]
    start = min(authored)
    end = max(address + local[address]["local_size"] for address in authored)
    local = {address: row for address, row in local.items() if start <= address < end}
    peer = {}
    with (reference / "config/symbols.csv").open(newline="") as stream:
        for row in csv.DictReader(stream):
            address = int(row["address"], 0)
            if not row["size"] or not start <= address < end:
                continue
            size = int(row["size"], 0)
            if size <= 0:
                continue
            if address in peer:
                raise ValueError(f"duplicate reference function address {address:#010x}")
            peer[address] = {"reference_name": row["name"], "reference_size": size}

    rows = [
        dict(address=f"0x{address:08X}", category=categories.get(address),
             **local.get(address, {}), **peer.get(address, {}))
        for address in sorted(local.keys() | peer.keys())
    ]
    differences = [
        row for row in rows if row.get("local_size") != row.get("reference_size")
    ]
    status = subprocess.check_output(
        ["git", "-C", str(reference), "status", "--porcelain"], text=True
    ).splitlines()
    return {
        "result": "different" if differences else "inventory_agrees",
        "evidence_class": "checksum and address/extent crosswalk; no source or exact attestation",
        "target_sha256": image.sha256,
        "reference_root": str(reference),
        "reference_revision": git_revision(reference),
        "reference_dirty_paths": status,
        "range": {"start": f"0x{start:08X}", "end_exclusive": f"0x{end:08X}"},
        "local_functions": len(local),
        "reference_functions": len(peer),
        "classification_differences": [row for row in rows if row["category"] == "library"],
        "boundary_differences": differences,
        "functions": rows,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, default=ROOT / "_reference/cirno-th08")
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        report = compare(args.reference.resolve(), args.target)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as exc:
        print(f"reference inventory comparison failed: {exc}", file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print(
            f"{report['result']}: {report['local_functions']} local / "
            f"{report['reference_functions']} reference functions; "
            f"{len(report['boundary_differences'])} boundary differences; "
            f"{len(report['classification_differences'])} local library classifications"
        )
    return int(bool(report["boundary_differences"]))


if __name__ == "__main__":
    raise SystemExit(main())
