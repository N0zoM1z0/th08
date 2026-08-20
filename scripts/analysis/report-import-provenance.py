#!/usr/bin/env python3
"""Report object/archive references for rebuild-only PE imports.

The target and configured archives are hash-attested before analysis.  The
report scans undefined COFF symbols in production objects and pinned archives,
then uses the current linker map to distinguish members present in this link
from other archive candidates.  It is a routing diagnostic, not proof that a
whole archive member belongs in or is absent from the target.

Examples:
  python3 scripts/analysis/report-import-provenance.py
  python3 scripts/analysis/report-import-provenance.py --json > build/import-provenance.json
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from pe_image import PEFormatError, PEImage  # noqa: E402


DEFAULT_TARGET = ROOT / "resources" / "th08.exe"
DEFAULT_REBUILD = ROOT / "build" / "th08.exe"
DEFAULT_MAP = ROOT / "build" / "th08.map"
TARGET_CONFIG = ROOT / "config" / "target.toml"
PROVENANCE_CONFIG = ROOT / "config" / "library-provenance.toml"
UNDEFINED_RE = re.compile(r"^(.*):\s+U\s+(\S+)\s*$")
STDCALL_SUFFIX_RE = re.compile(r"@\d+$")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def relative_display(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def verify_target(path: Path) -> PEImage:
    with TARGET_CONFIG.open("rb") as handle:
        expected = tomllib.load(handle)["target"]
    image = PEImage(path)
    if len(image.data) != int(expected["size"]) or image.sha256 != expected["sha256"]:
        raise ValueError(
            "target identity mismatch: expected "
            f"{expected['size']} bytes/{expected['sha256']}, got "
            f"{len(image.data)} bytes/{image.sha256}"
        )
    return image


def import_names(image: PEImage) -> dict[str, str]:
    result: dict[str, str] = {}
    for descriptor in image.imports():
        dll = str(descriptor["dll"])
        for symbol in descriptor["symbols"]:
            if symbol["kind"] != "name":
                continue
            name = str(symbol["name"])
            result[f"{dll.lower()}!{name.lower()}"] = f"{dll}!{name}"
    return result


def normalized_undefined_symbol(symbol: str) -> str:
    for prefix in ("__imp__", "_imp__"):
        if symbol.startswith(prefix):
            symbol = symbol[len(prefix) :]
            break
    if symbol.startswith("_"):
        symbol = symbol[1:]
    return STDCALL_SUFFIX_RE.sub("", symbol).lower()


def configured_archives(selected_ids: set[str]) -> list[dict[str, object]]:
    with PROVENANCE_CONFIG.open("rb") as handle:
        records = tomllib.load(handle).get("archives", [])
    result: list[dict[str, object]] = []
    known_ids = {str(record["id"]) for record in records}
    unknown = sorted(selected_ids - known_ids)
    if unknown:
        raise ValueError("unknown archive id(s): " + ", ".join(unknown))
    for record in records:
        archive_id = str(record["id"])
        if selected_ids and archive_id not in selected_ids:
            continue
        path = ROOT / str(record["path"])
        if not path.is_file():
            raise ValueError(f"configured archive is unavailable: {path}")
        actual_hash = sha256(path)
        if actual_hash != record["sha256"]:
            raise ValueError(
                f"archive hash mismatch for {archive_id}: "
                f"expected {record['sha256']}, got {actual_hash}"
            )
        result.append({"id": archive_id, "path": path, "sha256": actual_hash})
    return result


def run_nm(nm: str, path: Path) -> list[tuple[str, str]]:
    completed = subprocess.run(
        [nm, "-A", "-u", str(path)],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    result: list[tuple[str, str]] = []
    for line in completed.stdout.splitlines():
        match = UNDEFINED_RE.match(line)
        if match:
            result.append((match.group(1), match.group(2)))
    return result


def member_from_origin(origin: str, archive: Path) -> str:
    prefix = str(archive) + ":"
    if origin.startswith(prefix):
        return origin[len(prefix) :]
    return origin.rsplit(":", 1)[-1]


def build_report(args: argparse.Namespace) -> dict[str, object]:
    target = verify_target(args.target)
    rebuild = PEImage(args.rebuild)
    target_names = import_names(target)
    rebuild_names = import_names(rebuild)
    extra_keys = sorted(rebuild_names.keys() - target_names.keys())
    wanted_names: dict[str, list[str]] = {}
    for key in extra_keys:
        wanted_names.setdefault(key.split("!", 1)[1], []).append(key)

    map_text = ""
    if args.map_path.is_file():
        map_text = args.map_path.read_text(encoding="cp1252", errors="replace").lower()

    references: dict[str, list[dict[str, object]]] = {key: [] for key in extra_keys}
    objects = sorted((ROOT / "build").glob("*.obj"))
    for object_path in objects:
        for _origin, symbol in run_nm(args.nm, object_path):
            for key in wanted_names.get(normalized_undefined_symbol(symbol), []):
                references[key].append(
                    {
                        "kind": "object",
                        "source": object_path.relative_to(ROOT).as_posix(),
                        "symbol": symbol,
                        "present_in_map": object_path.name.lower() in map_text,
                    }
                )

    archives = configured_archives(set(args.archive))
    for archive in archives:
        archive_path = Path(archive["path"])
        for origin, symbol in run_nm(args.nm, archive_path):
            member = member_from_origin(origin, archive_path)
            basename = Path(member).name
            for key in wanted_names.get(normalized_undefined_symbol(symbol), []):
                references[key].append(
                    {
                        "kind": "archive-member",
                        "archive": archive["id"],
                        "member": member,
                        "symbol": symbol,
                        "present_in_map": f":{basename.lower()}" in map_text,
                    }
                )

    imports: list[dict[str, object]] = []
    for key in extra_keys:
        refs = sorted(
            references[key],
            key=lambda item: (
                not bool(item["present_in_map"]),
                str(item.get("archive", "")),
                str(item.get("member", item.get("source", ""))),
            ),
        )
        imports.append(
            {
                "import": rebuild_names[key],
                "reference_count": len(refs),
                "linked_reference_count": sum(bool(item["present_in_map"]) for item in refs),
                "references": refs,
            }
        )

    return {
        "schema_version": 1,
        "target": {
            "path": relative_display(args.target),
            "size": len(target.data),
            "sha256": target.sha256,
        },
        "rebuild": {
            "path": relative_display(args.rebuild),
            "size": len(rebuild.data),
            "sha256": rebuild.sha256,
        },
        "nm": args.nm,
        "map": relative_display(args.map_path) if args.map_path.is_file() else None,
        "archives": [
            {
                "id": archive["id"],
                "path": relative_display(Path(archive["path"])),
                "sha256": archive["sha256"],
            }
            for archive in archives
        ],
        "rebuild_only_import_count": len(imports),
        "imports": imports,
    }


def print_human(report: dict[str, object]) -> None:
    print(f"rebuild-only imports: {report['rebuild_only_import_count']}")
    for entry in report["imports"]:
        print(
            f"  {entry['import']}: linked references="
            f"{entry['linked_reference_count']}, candidates={entry['reference_count']}"
        )
        linked = [item for item in entry["references"] if item["present_in_map"]]
        for item in linked:
            if item["kind"] == "archive-member":
                print(f"    {item['archive']}:{item['member']}")
            else:
                print(f"    {item['source']}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--target", type=Path, default=DEFAULT_TARGET)
    parser.add_argument("--rebuild", type=Path, default=DEFAULT_REBUILD)
    parser.add_argument("--map", dest="map_path", type=Path, default=DEFAULT_MAP)
    parser.add_argument(
        "--archive",
        action="append",
        default=[],
        help="configured archive id to scan; repeat as needed (default: all)",
    )
    parser.add_argument(
        "--nm",
        default=shutil.which("i686-w64-mingw32-nm") or shutil.which("llvm-nm") or shutil.which("nm"),
        help="COFF-capable nm executable",
    )
    parser.add_argument("--json", action="store_true", help="emit deterministic JSON")
    args = parser.parse_args()
    if not args.nm:
        parser.error("no COFF-capable nm executable found; pass --nm")

    try:
        report = build_report(args)
    except (
        OSError,
        ValueError,
        KeyError,
        PEFormatError,
        subprocess.CalledProcessError,
        tomllib.TOMLDecodeError,
    ) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    if args.json:
        json.dump(report, sys.stdout, indent=2, sort_keys=True)
        sys.stdout.write("\n")
    else:
        print_human(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
