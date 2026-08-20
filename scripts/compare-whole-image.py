#!/usr/bin/env python3
"""Compare a rebuilt TH08 PE with the hash-attested Japanese 1.00d image.

This is a whole-link diagnostic, not an exact-match ledger.  It reports PE
metadata, data directories, sections, imports, resources, debug data, and
accepted-unit address anchors from the linker map.  It never edits tracking
state or copies bytes out of the target.

Examples:
  python3 scripts/compare-whole-image.py
  python3 scripts/compare-whole-image.py --json > build/whole-image-report.json
  python3 scripts/compare-whole-image.py --json --include-anchor-details
  python3 scripts/compare-whole-image.py --require-exact
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import sys
import tomllib

from pe_image import DIRECTORY_NAMES, PEFormatError, PEImage, Section


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TARGET = ROOT / "resources" / "th08.exe"
DEFAULT_REBUILD = ROOT / "build" / "th08.exe"
DEFAULT_MAP = ROOT / "build" / "th08.map"
TARGET_CONFIG = ROOT / "config" / "target.toml"

HEADER_FIELDS = (
    "size",
    "machine",
    "section_count",
    "timestamp",
    "characteristics",
    "linker_version",
    "size_of_code",
    "size_of_initialized_data",
    "size_of_uninitialized_data",
    "entry_rva",
    "entry_address",
    "base_of_code",
    "base_of_data",
    "image_base",
    "section_alignment",
    "file_alignment",
    "os_version",
    "image_version",
    "subsystem_version",
    "size_of_image",
    "size_of_headers",
    "checksum",
    "subsystem",
    "dll_characteristics",
    "stack_reserve",
    "stack_commit",
    "heap_reserve",
    "heap_commit",
    "loader_flags",
)


def relative_display(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def verify_target_identity(image: PEImage) -> dict[str, object]:
    with TARGET_CONFIG.open("rb") as handle:
        expected = tomllib.load(handle)["target"]
    size_ok = len(image.data) == int(expected["size"])
    hash_ok = image.sha256 == str(expected["sha256"])
    if not size_ok or not hash_ok:
        raise ValueError(
            "target identity mismatch: "
            f"expected {expected['size']} bytes/{expected['sha256']}, "
            f"got {len(image.data)} bytes/{image.sha256}"
        )
    return {"verified": True, "expected_size": expected["size"], "expected_sha256": expected["sha256"]}


def byte_diff(left: bytes, right: bytes, sample_limit: int = 8) -> dict[str, object]:
    common = min(len(left), len(right))
    differing = 0
    samples: list[int] = []
    prefix = 0
    while prefix < common and left[prefix] == right[prefix]:
        prefix += 1
    suffix = 0
    while suffix < common - prefix and left[len(left) - 1 - suffix] == right[len(right) - 1 - suffix]:
        suffix += 1
    for index in range(common):
        if left[index] != right[index]:
            differing += 1
            if len(samples) < sample_limit:
                samples.append(index)
    return {
        "exact": left == right,
        "left_size": len(left),
        "right_size": len(right),
        "size_delta": len(right) - len(left),
        "common_size": common,
        "differing_common_bytes": differing,
        "common_prefix": prefix,
        "common_suffix": suffix,
        "first_difference_offsets": samples,
    }


def section_record(image: PEImage, section: Section) -> dict[str, object]:
    raw = image.raw_section_bytes(section)
    return {
        "name": section.name,
        "virtual_size": section.virtual_size,
        "rva": section.rva,
        "address": image.image_base + section.rva,
        "raw_size": section.raw_size,
        "raw_offset": section.raw_offset,
        "characteristics": section.characteristics,
        "sha256": hashlib.sha256(raw).hexdigest(),
    }


def compare_sections(target: PEImage, rebuild: PEImage) -> list[dict[str, object]]:
    target_sections = {section.name: section for section in target.sections}
    rebuild_sections = {section.name: section for section in rebuild.sections}
    order = [section.name for section in target.sections]
    order.extend(name for name in rebuild_sections if name not in target_sections)
    result: list[dict[str, object]] = []
    for name in order:
        target_section = target_sections.get(name)
        rebuild_section = rebuild_sections.get(name)
        entry: dict[str, object] = {
            "name": name,
            "target": section_record(target, target_section) if target_section else None,
            "rebuild": section_record(rebuild, rebuild_section) if rebuild_section else None,
        }
        if target_section and rebuild_section:
            entry["raw_diff"] = byte_diff(
                target.raw_section_bytes(target_section), rebuild.raw_section_bytes(rebuild_section)
            )
        result.append(entry)
    return result


def import_identity(dll: str, symbol: dict[str, object]) -> str:
    if symbol["kind"] == "ordinal":
        name = f"#{symbol['ordinal']}"
    else:
        name = str(symbol["name"])
    return f"{dll.lower()}!{name}"


def compare_imports(target: PEImage, rebuild: PEImage) -> dict[str, object]:
    target_imports = target.imports()
    rebuild_imports = rebuild.imports()
    target_dlls = [str(entry["dll"]) for entry in target_imports]
    rebuild_dlls = [str(entry["dll"]) for entry in rebuild_imports]
    target_symbols = {
        import_identity(str(entry["dll"]), symbol)
        for entry in target_imports
        for symbol in entry["symbols"]
    }
    rebuild_symbols = {
        import_identity(str(entry["dll"]), symbol)
        for entry in rebuild_imports
        for symbol in entry["symbols"]
    }
    return {
        "target": target_imports,
        "rebuild": rebuild_imports,
        "dll_order_equal_casefolded": [name.lower() for name in target_dlls]
        == [name.lower() for name in rebuild_dlls],
        "target_dll_order": target_dlls,
        "rebuild_dll_order": rebuild_dlls,
        "target_symbol_count": len(target_symbols),
        "rebuild_symbol_count": len(rebuild_symbols),
        "target_only_symbols": sorted(target_symbols - rebuild_symbols),
        "rebuild_only_symbols": sorted(rebuild_symbols - target_symbols),
    }


def compare_resources(target: PEImage, rebuild: PEImage) -> dict[str, object]:
    target_leaves = target.resources()
    rebuild_leaves = rebuild.resources()
    target_by_path = {str(entry["raw_path"]): entry for entry in target_leaves}
    rebuild_by_path = {str(entry["raw_path"]): entry for entry in rebuild_leaves}
    changed: list[dict[str, object]] = []
    for path in sorted(target_by_path.keys() & rebuild_by_path.keys()):
        target_entry = target_by_path[path]
        rebuild_entry = rebuild_by_path[path]
        if target_entry != rebuild_entry:
            changed.append({"path": path, "target": target_entry, "rebuild": rebuild_entry})
    return {
        "target": target_leaves,
        "rebuild": rebuild_leaves,
        "target_only_paths": sorted(target_by_path.keys() - rebuild_by_path.keys()),
        "rebuild_only_paths": sorted(rebuild_by_path.keys() - target_by_path.keys()),
        "changed": changed,
    }


MAP_PUBLIC_RE = re.compile(
    r"^\s*[0-9A-Fa-f]{4}:[0-9A-Fa-f]{8}\s+(\S+)\s+([0-9A-Fa-f]{8})\s+(.+?)\s*$"
)


def parse_map_publics(path: Path) -> dict[str, list[dict[str, object]]]:
    publics: dict[str, list[dict[str, object]]] = {}
    in_publics = False
    for line in path.read_text(encoding="cp1252", errors="replace").splitlines():
        if "Publics by Value" in line:
            in_publics = True
            continue
        if in_publics and line.lstrip().startswith("entry point at"):
            break
        if not in_publics:
            continue
        match = MAP_PUBLIC_RE.match(line)
        if not match:
            continue
        symbol, address_text, remainder = match.groups()
        object_name = remainder.split()[-1]
        publics.setdefault(symbol, []).append(
            {"address": int(address_text, 16), "object": object_name}
        )
    return publics


def accepted_rows(path: Path) -> set[str]:
    with path.open(newline="", encoding="utf-8") as handle:
        return {row["unit"] for row in csv.DictReader(handle) if row.get("unit")}


def map_aliases_by_address(
    publics: dict[str, list[dict[str, object]]],
) -> dict[int, set[tuple[str, str]]]:
    aliases: dict[int, set[tuple[str, str]]] = {}
    for symbol, items in publics.items():
        for item in items:
            aliases.setdefault(int(item["address"]), set()).add(
                (symbol, str(item["object"]))
            )
    return aliases


def address_anchors(
    map_path: Path | None, include_details: bool = False
) -> dict[str, object] | None:
    if map_path is None or not map_path.is_file():
        return None
    publics = parse_map_publics(map_path)
    aliases_by_address = map_aliases_by_address(publics)
    anchors: list[dict[str, object]] = []
    missing: list[dict[str, str]] = []

    with (ROOT / "config" / "match-units.toml").open("rb") as handle:
        authored_units = {
            unit["name"]: unit for unit in tomllib.load(handle).get("units", [])
        }
    for name in sorted(accepted_rows(ROOT / "config" / "matches.csv")):
        unit = authored_units.get(name)
        if not unit or "symbol" not in unit or "target_address" not in unit:
            continue
        object_path = Path(str(unit.get("object", "")))
        if object_path.parent.as_posix() != "build":
            continue
        candidates = [
            item
            for item in publics.get(str(unit["symbol"]), [])
            if str(item["object"]).lower().endswith(object_path.name.lower())
        ]
        if len(candidates) != 1:
            missing.append({"kind": "authored", "unit": name, "reason": f"map candidates={len(candidates)}"})
            continue
        linked = int(candidates[0]["address"])
        target = int(unit["target_address"])
        linked_aliases = aliases_by_address.get(linked, set())
        anchors.append(
            {
                "kind": "authored",
                "unit": name,
                "symbol": unit["symbol"],
                "object": candidates[0]["object"],
                "target_address": target,
                "linked_address": linked,
                "drift": linked - target,
                "folded_alias": len(linked_aliases) > 1,
                "linked_alias_count": len(linked_aliases),
                "linked_alias_object_count": len(
                    {object_name for _, object_name in linked_aliases}
                ),
            }
        )

    with (ROOT / "config" / "library-match-units.toml").open("rb") as handle:
        library_units = {
            unit["name"]: unit for unit in tomllib.load(handle).get("units", [])
        }
    for name in sorted(accepted_rows(ROOT / "config" / "library-matches.csv")):
        unit = library_units.get(name)
        if not unit or "symbol" not in unit or "target_address" not in unit:
            continue
        candidates = publics.get(str(unit["symbol"]), [])
        if len(candidates) != 1:
            missing.append({"kind": "library", "unit": name, "reason": f"map candidates={len(candidates)}"})
            continue
        linked = int(candidates[0]["address"])
        target = int(unit["target_address"])
        linked_aliases = aliases_by_address.get(linked, set())
        anchors.append(
            {
                "kind": "library",
                "unit": name,
                "symbol": unit["symbol"],
                "object": candidates[0]["object"],
                "target_address": target,
                "linked_address": linked,
                "drift": linked - target,
                "folded_alias": len(linked_aliases) > 1,
                "linked_alias_count": len(linked_aliases),
                "linked_alias_object_count": len(
                    {object_name for _, object_name in linked_aliases}
                ),
            }
        )

    grouped: dict[str, list[dict[str, object]]] = {}
    for anchor in anchors:
        grouped.setdefault(str(anchor["object"]), []).append(anchor)
    object_summaries: list[dict[str, object]] = []
    for object_name, items in grouped.items():
        layout_items = [item for item in items if not bool(item["folded_alias"])]
        layout_metrics_available = bool(layout_items)
        metric_items = layout_items or items
        by_target = sorted(metric_items, key=lambda entry: int(entry["target_address"]))
        drifts = [int(entry["drift"]) for entry in by_target]
        object_summaries.append(
            {
                "object": object_name,
                "anchor_count": len(items),
                "layout_anchor_count": len(layout_items),
                "folded_alias_count": len(items) - len(layout_items),
                "layout_metrics_available": layout_metrics_available,
                "first_target_address": by_target[0]["target_address"],
                "first_linked_address": by_target[0]["linked_address"],
                "first_drift": by_target[0]["drift"],
                "last_target_address": by_target[-1]["target_address"],
                "last_linked_address": by_target[-1]["linked_address"],
                "last_drift": by_target[-1]["drift"],
                "minimum_drift": min(drifts),
                "maximum_drift": max(drifts),
            }
        )
    object_summaries.sort(key=lambda entry: int(entry["first_target_address"]))
    layout_summaries = [
        entry for entry in object_summaries if bool(entry["layout_metrics_available"])
    ]
    linked_order = sorted(layout_summaries, key=lambda entry: int(entry["first_linked_address"]))
    target_order_names = [str(entry["object"]) for entry in layout_summaries]
    linked_order_names = [str(entry["object"]) for entry in linked_order]
    result: dict[str, object] = {
        "map": relative_display(map_path),
        "anchor_count": len(anchors),
        "missing_count": len(missing),
        "object_target_anchor_order": target_order_names,
        "object_linked_anchor_order": linked_order_names,
        "object_anchor_order_equal": target_order_names == linked_order_names,
        "objects": object_summaries,
    }
    if include_details:
        result["missing"] = missing
        result["anchors"] = sorted(anchors, key=lambda entry: int(entry["target_address"]))
    return result


def compare_directories(target: PEImage, rebuild: PEImage) -> list[dict[str, object]]:
    count = max(len(target.directories), len(rebuild.directories), len(DIRECTORY_NAMES))
    result: list[dict[str, object]] = []
    for index in range(count):
        name = DIRECTORY_NAMES[index] if index < len(DIRECTORY_NAMES) else str(index)
        target_rva, target_size = target.directory(index)
        rebuild_rva, rebuild_size = rebuild.directory(index)
        result.append(
            {
                "index": index,
                "name": name,
                "target": {"rva": target_rva, "size": target_size},
                "rebuild": {"rva": rebuild_rva, "size": rebuild_size},
                "equal": (target_rva, target_size) == (rebuild_rva, rebuild_size),
            }
        )
    return result


def build_report(
    target: PEImage,
    rebuild: PEImage,
    map_path: Path | None,
    include_anchor_details: bool = False,
) -> dict[str, object]:
    target_metadata = target.metadata()
    rebuild_metadata = rebuild.metadata()
    target_metadata["path"] = relative_display(target.path)
    rebuild_metadata["path"] = relative_display(rebuild.path)
    header_differences = [
        {"field": field, "target": target_metadata[field], "rebuild": rebuild_metadata[field]}
        for field in HEADER_FIELDS
        if target_metadata[field] != rebuild_metadata[field]
    ]
    return {
        "schema_version": 1,
        "status": "exact" if target.data == rebuild.data else "different",
        "target": target_metadata,
        "rebuild": rebuild_metadata,
        "file_diff": byte_diff(target.data, rebuild.data),
        "header_raw_diff": byte_diff(
            target.data[: target.size_of_headers], rebuild.data[: rebuild.size_of_headers]
        ),
        "header_differences": header_differences,
        "data_directories": compare_directories(target, rebuild),
        "sections": compare_sections(target, rebuild),
        "imports": compare_imports(target, rebuild),
        "resources": compare_resources(target, rebuild),
        "debug": {"target": target.debug_entries(), "rebuild": rebuild.debug_entries()},
        "link_layout": address_anchors(map_path, include_anchor_details),
    }


def hex_value(value: object) -> str:
    return f"0x{int(value):X}"


def print_human(report: dict[str, object]) -> None:
    target = report["target"]
    rebuild = report["rebuild"]
    print(f"whole image: {report['status']}")
    print(f"target:  {target['path']} ({target['size']} bytes, {target['sha256']})")
    print(f"rebuild: {rebuild['path']} ({rebuild['size']} bytes, {rebuild['sha256']})")
    print(
        "entry:   "
        f"target={hex_value(target['entry_address'])} "
        f"rebuild={hex_value(rebuild['entry_address'])}"
    )
    print(f"header fields changed: {len(report['header_differences'])}")
    for difference in report["header_differences"]:
        field = difference["field"]
        if field in {
            "timestamp",
            "size_of_code",
            "size_of_initialized_data",
            "entry_rva",
            "entry_address",
            "base_of_data",
            "size_of_image",
        }:
            target_value = hex_value(difference["target"])
            rebuild_value = hex_value(difference["rebuild"])
        else:
            target_value = str(difference["target"])
            rebuild_value = str(difference["rebuild"])
        print(f"  {field}: {target_value} -> {rebuild_value}")

    print("sections:")
    for section in report["sections"]:
        target_section = section["target"]
        rebuild_section = section["rebuild"]
        if target_section and rebuild_section:
            raw_diff = section["raw_diff"]
            print(
                f"  {section['name']}: virtual {hex_value(target_section['virtual_size'])}"
                f" -> {hex_value(rebuild_section['virtual_size'])}, raw "
                f"{hex_value(target_section['raw_size'])} -> {hex_value(rebuild_section['raw_size'])}, "
                f"common-byte differences={raw_diff['differing_common_bytes']}"
            )
        else:
            print(f"  {section['name']}: missing from one image")

    imports = report["imports"]
    print(
        "imports: "
        f"DLL order equal={imports['dll_order_equal_casefolded']}, "
        f"symbols target={imports['target_symbol_count']} rebuild={imports['rebuild_symbol_count']}, "
        f"target-only={len(imports['target_only_symbols'])} "
        f"rebuild-only={len(imports['rebuild_only_symbols'])}"
    )
    if imports["target_only_symbols"]:
        print("  target-only: " + ", ".join(imports["target_only_symbols"]))
    if imports["rebuild_only_symbols"]:
        print("  rebuild-only: " + ", ".join(imports["rebuild_only_symbols"]))

    resources = report["resources"]
    print(
        "resources: "
        f"leaves target={len(resources['target'])} rebuild={len(resources['rebuild'])}, "
        f"changed={len(resources['changed'])}, "
        f"target-only={len(resources['target_only_paths'])}, "
        f"rebuild-only={len(resources['rebuild_only_paths'])}"
    )
    for changed in resources["changed"]:
        print(
            f"  {changed['path']}: size {changed['target']['size']}"
            f" -> {changed['rebuild']['size']}"
        )

    layout = report["link_layout"]
    if layout is None:
        print("link layout: no linker map supplied")
    else:
        print(
            "link layout: "
            f"accepted anchors={layout['anchor_count']}, missing={layout['missing_count']}, "
            f"anchor-derived object order equal={layout['object_anchor_order_equal']}"
        )
        production_objects = [
            entry for entry in layout["objects"] if ":" not in str(entry["object"])
        ]
        layout_objects = [
            entry for entry in production_objects if bool(entry["layout_metrics_available"])
        ]
        folded_only_objects = len(production_objects) - len(layout_objects)
        ranked = sorted(
            layout_objects,
            key=lambda entry: abs(int(entry["maximum_drift"]) - int(entry["minimum_drift"])),
            reverse=True,
        )
        print(
            f"  production objects anchored={len(production_objects)}, "
            f"layout-ranked={len(layout_objects)}, folded-only={folded_only_objects}; "
            "largest intra-object drift spans:"
        )
        for entry in ranked[:12]:
            print(
                f"  {entry['object']}: anchors={entry['anchor_count']} "
                f"first drift={int(entry['first_drift']):+#x}, "
                f"last drift={int(entry['last_drift']):+#x}, "
                f"span={int(entry['maximum_drift']) - int(entry['minimum_drift']):+#x}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--target", type=Path, default=DEFAULT_TARGET)
    parser.add_argument("--rebuild", type=Path, default=DEFAULT_REBUILD)
    parser.add_argument(
        "--map",
        dest="map_path",
        type=Path,
        default=DEFAULT_MAP,
        help="linker map for accepted-unit address anchors; omit with --no-map",
    )
    parser.add_argument("--no-map", action="store_true", help="skip linker-map address anchors")
    parser.add_argument("--json", action="store_true", help="emit deterministic JSON")
    parser.add_argument(
        "--include-anchor-details",
        action="store_true",
        help="include every accepted/missing address anchor in JSON (large; summaries are default)",
    )
    parser.add_argument(
        "--skip-target-identity",
        action="store_true",
        help="diagnostic only: do not require the configured TH08 1.00d size/hash",
    )
    parser.add_argument(
        "--require-exact",
        action="store_true",
        help="return exit status 1 when the complete files differ",
    )
    args = parser.parse_args()

    try:
        target = PEImage(args.target)
        rebuild = PEImage(args.rebuild)
        target_identity = (
            {"verified": False} if args.skip_target_identity else verify_target_identity(target)
        )
        map_path = None if args.no_map else args.map_path
        report = build_report(target, rebuild, map_path, args.include_anchor_details)
        report["target_identity"] = target_identity
    except (OSError, PEFormatError, ValueError, KeyError, tomllib.TOMLDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    if args.json:
        json.dump(report, sys.stdout, indent=2, sort_keys=True)
        sys.stdout.write("\n")
    else:
        print_human(report)
    if args.require_exact and report["status"] != "exact":
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
