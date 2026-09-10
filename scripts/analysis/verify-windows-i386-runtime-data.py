#!/usr/bin/env python3
"""Verify target-initialized data restored for the native Windows i386 image.

This is a whole-link runtime prerequisite check, not an authored-function
comparison and not a source of exact-match credit.  It verifies the canonical
Japanese TH08 1.00d target, then checks that the rebuilt PE owns the four data
families recovered during native Windows playtesting and selects the correct
aggregate fields in the enemy-name and spell-background paths.  Function
pointers in the Effect table are compared through linker-map symbols instead
of preferred virtual addresses.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import re
import struct
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from pe_image import PEImage  # noqa: E402


EFFECT_TEMPLATES_VA = 0x004C6D30
EFFECT_TEMPLATE_COUNT = 66
LAST_SPELL_COUNT_VA = 0x004C6C3C
GUI_STAGE_CLEAR_BONUSES_VA = 0x004C7158
GUI_MESSAGE_TEXT_COLORS_VA = 0x004C7180
GUI_COPY_ENEMY_NAME_TEXTURE_VA = 0x00437F5C
GUI_COPY_ENEMY_NAME_TEXTURE_SIZE = 0xEA
GUI_FRONT_ANM_OFFSET = 0x0C
GUI_STAGE_TEXT_ANM_OFFSET = 0x10
SPELLCARD_START_SPELL_VA = 0x004152A0
SPELLCARD_START_SPELL_SIZE = 0x9B3
EFFECT_MANAGER_VA = 0x004ECE60
EFFECT_MANAGER_STAGE_EFFECT_ANM_OFFSET = 0x8B058

MAP_PUBLIC_RE = re.compile(
    r"^\s+[0-9A-Fa-f]{4}:[0-9A-Fa-f]{8}\s+(\S+)\s+([0-9A-Fa-f]{8})(?:\s|$)"
)
UNINITIALIZED_OWNER_RE = re.compile(
    r"DIFFABLE_STATIC(?:_ARRAY)?\s*\(([^;\n]+)\)\s*;?"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--target", type=Path, default=ROOT / "resources" / "th08.exe"
    )
    parser.add_argument(
        "--rebuild", type=Path, default=ROOT / "build" / "th08.exe"
    )
    parser.add_argument(
        "--map", dest="map_path", type=Path, default=ROOT / "build" / "th08.map"
    )
    return parser.parse_args()


def verify_target(image: PEImage) -> None:
    with (ROOT / "config" / "target.toml").open("rb") as stream:
        expected = tomllib.load(stream)["target"]
    if len(image.data) != int(expected["size"]) or image.sha256 != expected["sha256"]:
        raise ValueError(
            "target identity mismatch: expected "
            f"{expected['size']} bytes/{expected['sha256']}, got "
            f"{len(image.data)} bytes/{image.sha256}"
        )


def read_va(image: PEImage, va: int, size: int) -> bytes:
    return image.read_rva(va - image.image_base, size)


def parse_publics(path: Path) -> dict[str, set[int]]:
    publics: dict[str, set[int]] = {}
    for line in path.read_text(encoding="cp1252", errors="replace").splitlines():
        match = MAP_PUBLIC_RE.match(line)
        if match:
            publics.setdefault(match.group(1), set()).add(int(match.group(2), 16))
    if not publics:
        raise ValueError(f"no public symbols found in linker map {path}")
    return publics


def public_va(publics: dict[str, set[int]], unqualified_name: str) -> int:
    prefix = f"?{unqualified_name}@"
    values = {
        value
        for symbol, addresses in publics.items()
        if symbol.startswith(prefix)
        for value in addresses
    }
    if len(values) != 1:
        rendered = ", ".join(f"{value:#010x}" for value in sorted(values)) or "none"
        raise ValueError(
            f"expected one linked address for {unqualified_name}, found {rendered}"
        )
    return next(iter(values))


def mapped_target_names() -> dict[int, list[str]]:
    result: dict[int, list[str]] = {}
    with (ROOT / "config" / "mapping.csv").open(newline="") as stream:
        for row in csv.reader(stream):
            result.setdefault(int(row[1], 16), []).append(row[0])
    return result


def verify_plain_data(
    target: PEImage,
    rebuild: PEImage,
    publics: dict[str, set[int]],
    target_va: int,
    rebuild_name: str,
    size: int,
) -> None:
    rebuilt_va = public_va(publics, rebuild_name)
    expected = read_va(target, target_va, size)
    actual = read_va(rebuild, rebuilt_va, size)
    if actual != expected:
        raise ValueError(
            f"{rebuild_name} differs from target {target_va:#010x} "
            f"({size:#x} bytes)"
        )


def verify_gui_enemy_name_owner(
    target: PEImage, rebuild: PEImage, publics: dict[str, set[int]]
) -> None:
    """Reject a relocation-normalized match that selects the wrong Gui field."""
    rebuilt_function_va = public_va(publics, "CopyEnemyNameTexture")
    rebuilt_gui_va = public_va(publics, "g_Gui")
    cases = (
        (
            "target",
            read_va(
                target,
                GUI_COPY_ENEMY_NAME_TEXTURE_VA,
                GUI_COPY_ENEMY_NAME_TEXTURE_SIZE,
            ),
            0x0160F428,
        ),
        (
            "rebuild",
            read_va(
                rebuild,
                rebuilt_function_va,
                GUI_COPY_ENEMY_NAME_TEXTURE_SIZE,
            ),
            rebuilt_gui_va,
        ),
    )
    for label, body, gui_va in cases:
        front_load = b"\x8b\x0d" + struct.pack("<I", gui_va + GUI_FRONT_ANM_OFFSET)
        stage_text_load = b"\x8b\x0d" + struct.pack(
            "<I", gui_va + GUI_STAGE_TEXT_ANM_OFFSET
        )
        front_count = body.count(front_load)
        stage_text_count = body.count(stage_text_load)
        if front_count != 8 or stage_text_count != 0:
            raise ValueError(
                f"{label} Gui::CopyEnemyNameTexture has {front_count} frontAnm "
                f"loads and {stage_text_count} stageTextAnm loads; expected 8/0"
            )


def verify_spellcard_background_owner(
    target: PEImage, rebuild: PEImage, publics: dict[str, set[int]]
) -> None:
    """Require spell backgrounds to use EffectManager's loaded stage ANM."""
    legacy_prefix = "?g_SpellcardBackgroundAnm@"
    legacy_symbols = [name for name in publics if name.startswith(legacy_prefix)]
    if legacy_symbols:
        raise ValueError(
            "linked image still owns standalone g_SpellcardBackgroundAnm: "
            + ", ".join(sorted(legacy_symbols))
        )

    rebuilt_function_va = public_va(publics, "StartSpell")
    rebuilt_effect_manager_va = public_va(publics, "g_EffectManager")
    cases = (
        (
            "target",
            read_va(target, SPELLCARD_START_SPELL_VA, SPELLCARD_START_SPELL_SIZE),
            EFFECT_MANAGER_VA,
        ),
        (
            "rebuild",
            read_va(rebuild, rebuilt_function_va, SPELLCARD_START_SPELL_SIZE),
            rebuilt_effect_manager_va,
        ),
    )
    for label, body, effect_manager_va in cases:
        stage_anm_load = b"\x8b\x0d" + struct.pack(
            "<I", effect_manager_va + EFFECT_MANAGER_STAGE_EFFECT_ANM_OFFSET
        )
        count = body.count(stage_anm_load)
        if count != 1:
            raise ValueError(
                f"{label} Spellcard::StartSpell has {count} "
                "EffectManager::stageEffectAnm loads; expected 1"
            )


def verify_effect_templates(
    target: PEImage, rebuild: PEImage, publics: dict[str, set[int]]
) -> None:
    size = EFFECT_TEMPLATE_COUNT * 12
    target_rows = struct.iter_unpack("<III", read_va(target, EFFECT_TEMPLATES_VA, size))
    rebuilt_va = public_va(publics, "g_EffectTemplates")
    rebuilt_rows = struct.iter_unpack("<III", read_va(rebuild, rebuilt_va, size))
    names_by_va = mapped_target_names()

    for index, (expected, actual) in enumerate(zip(target_rows, rebuilt_rows)):
        expected_script, expected_update, expected_initialize = expected
        actual_script, actual_update, actual_initialize = actual
        if actual_script != expected_script:
            raise ValueError(
                f"g_EffectTemplates[{index}] script is {actual_script}, "
                f"expected {expected_script}"
            )

        for role, target_callback, rebuilt_callback in (
            ("update", expected_update, actual_update),
            ("initialize", expected_initialize, actual_initialize),
        ):
            if target_callback == 0:
                expected_callback = 0
                expected_name = "NULL"
            else:
                mapped = names_by_va.get(target_callback, [])
                if len(mapped) != 1:
                    raise ValueError(
                        f"Effect row {index} {role} target {target_callback:#010x} "
                        f"has {len(mapped)} mapping names"
                    )
                expected_name = mapped[0].rsplit("::", 1)[-1]
                # Target row 41 stores the ABI-compatible AnmVm member body.
                # A normal VC7 C++ function-pointer initializer cannot express
                # that member/free-function conversion, so production owns a
                # relocatable Effect callback adapter which invokes the exact
                # member implementation.
                if target_callback == 0x0040EB50:
                    expected_name = "UpdatePulsingRadialTrailEffectCallback"
                expected_callback = public_va(publics, expected_name)

            if rebuilt_callback != expected_callback:
                raise ValueError(
                    f"g_EffectTemplates[{index}] {role} is "
                    f"{rebuilt_callback:#010x}, expected {expected_name} at "
                    f"{expected_callback:#010x}"
                )


def raw_backed_section(image: PEImage, va: int):
    rva = va - image.image_base
    for section in image.sections:
        if section.rva <= rva < section.rva + section.raw_size:
            return section
    return None


def uninitialized_raw_owner_candidates(target: PEImage) -> list[str]:
    addresses: dict[str, set[int]] = {}
    with (ROOT / "config" / "reccmp-globals.csv").open(newline="") as stream:
        for row in csv.DictReader(stream):
            name = row["name"].rsplit("::", 1)[-1]
            addresses.setdefault(name, set()).add(int(row["address"], 16))

    candidates: list[str] = []
    for path in sorted((ROOT / "src").glob("*")):
        if path.suffix not in {".cpp", ".inl"}:
            continue
        source = path.read_text(encoding="utf-8", errors="replace")
        for match in UNINITIALIZED_OWNER_RE.finditer(source):
            macro = match.group(0)
            if "_ASSIGN" in macro:
                continue
            name_match = re.search(r"\b(g_[A-Za-z0-9_]+)\s*$", match.group(1))
            if name_match is None:
                continue
            name = name_match.group(1)
            for va in sorted(addresses.get(name, set())):
                section = raw_backed_section(target, va)
                if section is not None:
                    line = source.count("\n", 0, match.start()) + 1
                    candidates.append(
                        f"{path.relative_to(ROOT)}:{line}: {name} at {va:#010x} "
                        f"lies in raw-backed target {section.name}"
                    )
    return candidates


def main() -> int:
    args = parse_args()
    try:
        target = PEImage(args.target)
        rebuild = PEImage(args.rebuild)
        verify_target(target)
        publics = parse_publics(args.map_path)

        candidates = uninitialized_raw_owner_candidates(target)
        if candidates:
            raise ValueError(
                "uninitialized production owner(s) overlap target raw data:\n  "
                + "\n  ".join(candidates)
            )

        verify_plain_data(
            target, rebuild, publics, LAST_SPELL_COUNT_VA, "g_LastSpellCount", 4
        )
        verify_effect_templates(target, rebuild, publics)
        verify_plain_data(
            target,
            rebuild,
            publics,
            GUI_STAGE_CLEAR_BONUSES_VA,
            "g_GuiStageClearBonuses",
            9 * 4,
        )
        verify_plain_data(
            target,
            rebuild,
            publics,
            GUI_MESSAGE_TEXT_COLORS_VA,
            "g_GuiMessageTextColors",
            12 * 16,
        )
        verify_gui_enemy_name_owner(target, rebuild, publics)
        verify_spellcard_background_owner(target, rebuild, publics)
    except (OSError, UnicodeError, ValueError, struct.error) as exc:
        print(f"Windows i386 runtime-data verification failed: {exc}", file=sys.stderr)
        return 1

    print(
        "Windows i386 runtime data OK: 0 raw-data zero-owner candidates; "
        "Last Spell count, 66 Effect templates, 9 stage bonuses, and "
        "12 dialogue palettes match the target; enemy-name copy selects "
        "Gui::frontAnm in all 8 linked loads; spell backgrounds select "
        "EffectManager::stageEffectAnm"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
