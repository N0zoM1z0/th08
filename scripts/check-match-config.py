#!/usr/bin/env python3
"""Statically validate comparator manifests without requiring the target EXE."""

from __future__ import annotations

import json
from pathlib import Path
import re
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[1]
RECCMP = ROOT / "reccmp-project.yml"
OBJDIFF = ROOT / "objdiff.json"
RECCMP_FILENAME = re.compile(r"^\s{4}filename:\s*([^#\s]+)\s*$", re.MULTILINE)
RECCMP_SHA256 = re.compile(
    r"^\s{6}sha256:\s*['\"]?([0-9a-fA-F]{64})['\"]?\s*$", re.MULTILINE
)
RECCMP_DATA_SOURCE = re.compile(r"^\s{6}-\s+([^#\s]+)\s*$", re.MULTILINE)


def fail(message: str) -> None:
    raise ValueError(message)


def validate_reccmp(target: dict[str, object]) -> None:
    text = RECCMP.read_text(encoding="utf-8")
    filenames = RECCMP_FILENAME.findall(text)
    hashes = RECCMP_SHA256.findall(text)
    sources = RECCMP_DATA_SOURCE.findall(text)
    if filenames != [str(target["filename"])]:
        fail(f"reccmp filename does not match target.toml: {filenames}")
    if [digest.lower() for digest in hashes] != [str(target["sha256"]).lower()]:
        fail("reccmp sha256 does not match target.toml")
    if not sources:
        fail("reccmp data_sources list is empty")
    missing = [path for path in sources if not (ROOT / path).is_file()]
    if missing:
        fail("reccmp data source does not exist: " + ", ".join(missing))


def validate_objdiff() -> None:
    manifest = json.loads(OBJDIFF.read_text(encoding="utf-8"))
    if manifest.get("custom_make") != "python":
        fail("objdiff custom_make must be python")
    args = manifest.get("custom_args")
    if not isinstance(args, list) or args[:2] != [
        "./scripts/build.py",
        "--build-type=objdiffbuild",
    ]:
        fail("objdiff custom_args do not invoke the canonical objdiff build")
    if not (ROOT / "scripts" / "build.py").is_file():
        fail("objdiff build wrapper is missing")

    units = manifest.get("units")
    if not isinstance(units, list) or not units:
        fail("objdiff units must be a non-empty list")
    seen_names: set[str] = set()
    seen_targets: set[str] = set()
    seen_bases: set[str] = set()
    for index, unit in enumerate(units):
        if not isinstance(unit, dict):
            fail(f"objdiff unit {index} is not an object")
        name = unit.get("name")
        target_path = unit.get("target_path")
        base_path = unit.get("base_path")
        values = (name, target_path, base_path)
        if not all(isinstance(value, str) and value for value in values):
            fail(f"objdiff unit {index} lacks name/target_path/base_path")
        if name in seen_names or target_path in seen_targets or base_path in seen_bases:
            fail(f"objdiff unit {index} duplicates a name or object path")
        if not target_path.startswith(
            "build/objdiff/orig/"
        ) or not target_path.endswith(".obj"):
            fail(f"objdiff unit {name!r} has a noncanonical target path")
        if not base_path.startswith("build/objdiff/reimpl/") or not base_path.endswith(
            ".obj"
        ):
            fail(f"objdiff unit {name!r} has a noncanonical base path")
        seen_names.add(name)
        seen_targets.add(target_path)
        seen_bases.add(base_path)


def main() -> int:
    try:
        with (ROOT / "config" / "target.toml").open("rb") as stream:
            target = tomllib.load(stream)["target"]
        validate_reccmp(target)
        validate_objdiff()
    except (
        OSError,
        KeyError,
        TypeError,
        ValueError,
        json.JSONDecodeError,
        tomllib.TOMLDecodeError,
    ) as exc:
        print(f"error: match configuration: {exc}", file=sys.stderr)
        return 1
    print(
        "match configuration OK: reccmp target and objdiff units are internally consistent"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
