#!/usr/bin/env python3
"""Fail when closed TH08 semantic protocol surfaces regain raw literals."""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
SOURCE_SUFFIXES = {".cpp", ".hpp", ".inl"}


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def source_files() -> list[Path]:
    return sorted(
        path
        for path in SRC.rglob("*")
        if path.is_file() and path.suffix.lower() in SOURCE_SUFFIXES
    )


def reject_pattern(paths: list[Path], label: str, pattern: str) -> None:
    matcher = re.compile(pattern, re.MULTILINE | re.DOTALL)
    hits: list[str] = []
    for path in paths:
        text = path.read_text(encoding="utf-8")
        for match in matcher.finditer(text):
            line = text.count("\n", 0, match.start()) + 1
            relative = path.relative_to(ROOT).as_posix()
            spelling = " ".join(match.group(0).split())
            hits.append(f"{relative}:{line}: {spelling[:120]}")
    if hits:
        fail(f"{label}:\n  " + "\n  ".join(hits))


def explicit_enum(
    path: Path, enum_name: str, prefix: str, expected_values: list[int]
) -> list[str]:
    text = path.read_text(encoding="utf-8")
    match = re.search(rf"enum {enum_name}\s*\{{(.*?)\n\}};", text, re.DOTALL)
    if match is None:
        fail(f"could not find enum {enum_name}")

    entries = re.findall(
        rf"\b({prefix}[A-Z0-9_]+)\s*=\s*(0x[0-9a-fA-F]+|\d+)",
        match.group(1),
    )
    names = [name for name, _ in entries]
    values = [int(value, 0) for _, value in entries]
    if sorted(values) != sorted(expected_values) or len(entries) != len(expected_values):
        expected = ", ".join(f"0x{value:X}" for value in expected_values)
        fail(f"{enum_name} must contain one explicit, unique entry for: {expected}")
    if len(set(names)) != len(names):
        fail(f"{enum_name} contains a duplicate name")
    return names


def check_ecl_opcode_protocol() -> None:
    header = (SRC / "EclManager.hpp").read_text(encoding="utf-8")
    match = re.search(r"enum EclOpcode\s*\{(.*?)\n\};", header, re.DOTALL)
    if match is None:
        fail("could not find enum EclOpcode")

    entries = re.findall(
        r"\b(ECL_OPCODE_[A-Z0-9_]+)\s*=\s*(\d+)", match.group(1)
    )
    names = [name for name, _ in entries]
    values = [int(value) for _, value in entries]
    if len(entries) != 184 or sorted(values) != list(range(1, 185)):
        fail("EclOpcode must contain one explicit, unique entry for every value 1..184")
    if len(set(names)) != len(names):
        fail("EclOpcode contains a duplicate name")

    dispatch_paths = [SRC / "EclRunLow.inl", SRC / "EclRunHigh.inl"]
    dispatch = "\n".join(path.read_text(encoding="utf-8") for path in dispatch_paths)
    reject_pattern(dispatch_paths, "numeric ECL dispatch label", r"\bcase\s+(?:0x[0-9a-f]+|\d+)\s*:")
    dispatched = re.findall(r"\bcase\s+(ECL_OPCODE_[A-Z0-9_]+)\s*:", dispatch)
    if len(dispatched) != 184 or set(dispatched) != set(names):
        fail("ECL dispatch must reference every named opcode exactly once")


def check_ecl_operand_protocol() -> None:
    names = explicit_enum(
        SRC / "EclManager.hpp",
        "EclOperandId",
        "ECL_OPERAND_",
        list(range(0x2710, 0x2775)),
    )
    resolver_paths = [SRC / "EclOperandsInt.cpp", SRC / "EclOperandsFloat.cpp"]
    reject_pattern(
        resolver_paths,
        "numeric ECL operand dispatch label",
        r"\bcase\s+0x27[0-9a-fA-F]{2}\s*:",
    )
    dispatch = "\n".join(path.read_text(encoding="utf-8") for path in resolver_paths)
    dispatched = re.findall(r"\bcase\s+(ECL_OPERAND_[A-Z0-9_]+)\s*:", dispatch)
    if set(dispatched) != set(names):
        fail("ECL operand resolvers must collectively reference every named selector")
    unknown = sorted(set(dispatched) - set(names))
    if unknown:
        fail(f"ECL operand resolvers reference unknown selectors: {', '.join(unknown)}")


def check_background_protocol() -> None:
    opcode_names = explicit_enum(
        SRC / "Background.hpp",
        "BackgroundOpcode",
        "BACKGROUND_OPCODE_",
        list(range(35)),
    )
    explicit_enum(
        SRC / "Background.hpp",
        "BackgroundInterpolationMode",
        "BACKGROUND_INTERPOLATION_",
        list(range(8)),
    )
    explicit_enum(
        SRC / "Background.hpp",
        "BackgroundCameraMotionMode",
        "BACKGROUND_CAMERA_MOTION_",
        list(range(4)),
    )
    path = SRC / "Background.cpp"
    reject_pattern([path], "numeric Background dispatch label", r"\bcase\s+(?:0x[0-9a-f]+|\d+)\s*:")
    dispatched = re.findall(
        r"\bcase\s+(BACKGROUND_OPCODE_[A-Z0-9_]+)\s*:",
        path.read_text(encoding="utf-8"),
    )
    if len(dispatched) != 35 or set(dispatched) != set(opcode_names):
        fail("Background dispatch must reference every named opcode exactly once")


def check_ecl_timeline_protocol() -> None:
    names = explicit_enum(
        SRC / "EnemyManager.hpp",
        "EclTimelineOpcode",
        "ECL_TIMELINE_OPCODE_",
        list(range(17)),
    )
    path = SRC / "EnemyTimeline.cpp"
    reject_pattern([path], "numeric ECL timeline dispatch label", r"\bcase\s+(?:0x[0-9a-f]+|\d+)\s*:")
    dispatched = re.findall(
        r"\bcase\s+(ECL_TIMELINE_OPCODE_[A-Z0-9_]+)\s*:",
        path.read_text(encoding="utf-8"),
    )
    if len(dispatched) != 17 or set(dispatched) != set(names):
        fail("ECL timeline dispatch must reference every named opcode exactly once")


def check_replay_frame_event_protocol(paths: list[Path]) -> None:
    explicit_enum(
        SRC / "ReplayManager.hpp",
        "ReplayFrameEventFlag",
        "REPLAY_FRAME_EVENT_",
        [0x0000, 0x0001, 0x0002, 0x0004, 0x0020, 0x0040,
         0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000],
    )
    reject_pattern(
        paths,
        "numeric replay frame-event flag",
        r"\bframeEventFlags\s*(?:\|=|=)\s*(?:0x[0-9a-f]+|\d+)",
    )


def check_closed_small_dispatches() -> None:
    explicit_enum(
        SRC / "EclManager.hpp",
        "EclEasingMode",
        "ECL_EASING_",
        list(range(7)),
    )
    reject_pattern(
        [SRC / "EclRun.cpp", SRC / "EnemyManager.cpp"],
        "numeric ECL easing dispatch label",
        r"\bcase\s+(?:0x[0-9a-f]+|\d+)\s*:",
    )

    names = explicit_enum(
        SRC / "AnmManager.hpp",
        "AnmPlayerBulletDrawMode",
        "ANM_PLAYER_BULLET_DRAW_",
        list(range(6)),
    )
    text = (SRC / "AnmManager.cpp").read_text(encoding="utf-8")
    match = re.search(
        r"void AnmManager::DrawPlayerBullet\(AnmVm \*vm\)\s*\{(.*?)^\}",
        text,
        re.DOTALL | re.MULTILINE,
    )
    if match is None:
        fail("could not find AnmManager::DrawPlayerBullet")
    reject_pattern(
        [SRC / "AnmManager.cpp"],
        "numeric player-bullet draw dispatch label",
        r"void AnmManager::DrawPlayerBullet\(AnmVm \*vm\).*?\bcase\s+(?:0x[0-9a-f]+|\d+)\s*:",
    )
    dispatched = re.findall(
        r"\bcase\s+(ANM_PLAYER_BULLET_DRAW_[A-Z0-9_]+)\s*:", match.group(1)
    )
    if len(dispatched) != 6 or set(dispatched) != set(names):
        fail("player-bullet draw dispatch must reference every named mode exactly once")

    replay_menu_names = explicit_enum(
        SRC / "TitleScreen.hpp",
        "TitleReplayMenuState",
        "TITLE_REPLAY_MENU_",
        list(range(5)),
    )
    for path in [SRC / "TitleScreen.cpp", SRC / "TitleReplayMenuProbe.cpp"]:
        dispatched = re.findall(
            r"\bcase\s+(TITLE_REPLAY_MENU_[A-Z0-9_]+)\s*:",
            path.read_text(encoding="utf-8"),
        )
        if len(dispatched) != 5 or set(dispatched) != set(replay_menu_names):
            fail(f"{path.name} must reference every named replay-menu state exactly once")

    phase_names = explicit_enum(
        SRC / "ResultScreen.hpp",
        "ResultScreenPhase",
        "RESULT_SCREEN_PHASE_",
        list(range(2)),
    )
    category_names = explicit_enum(
        SRC / "ResultScreen.hpp",
        "ResultScreenCategory",
        "RESULT_SCREEN_CATEGORY_",
        list(range(4)),
    )
    result_text = (SRC / "ResultScreen.cpp").read_text(encoding="utf-8")
    result_phases = set(
        re.findall(r"\bcase\s+(RESULT_SCREEN_PHASE_[A-Z0-9_]+)\s*:", result_text)
    )
    result_categories = set(
        re.findall(r"\bcase\s+(RESULT_SCREEN_CATEGORY_[A-Z0-9_]+)\s*:", result_text)
    )
    if result_phases != set(phase_names):
        fail("ResultScreen dispatch must reference every named phase")
    if result_categories != set(category_names):
        fail("ResultScreen dispatch must reference every named category")

    portrait_names = explicit_enum(
        SRC / "Gui.hpp",
        "GuiPortraitSlot",
        "GUI_PORTRAIT_",
        list(range(4)),
    )
    gui_text = (SRC / "Gui.cpp").read_text(encoding="utf-8")
    portrait_cases = set(
        re.findall(r"\bcase\s+(GUI_PORTRAIT_[A-Z0-9_]+)\s*:", gui_text)
    )
    if portrait_cases != set(portrait_names):
        fail("GUI portrait dispatches must reference every named slot")
    reject_pattern(
        [SRC / "Gui.cpp"],
        "numeric GUI portrait slot",
        r"\bportraits\s*\[\s*[0-3]\s*\]",
    )

    death_names = explicit_enum(
        SRC / "EnemyManager.hpp",
        "EnemyDeathMode",
        "ENEMY_DEATH_MODE_",
        list(range(4)),
    )
    enemy_text = "\n".join(
        path.read_text(encoding="utf-8")
        for path in [SRC / "EnemyManager.cpp", SRC / "EnemyManagerUpdate.cpp"]
    )
    death_cases = set(
        re.findall(r"\bcase\s+(ENEMY_DEATH_MODE_[A-Z0-9_]+)\s*:", enemy_text)
    )
    if death_cases != set(death_names):
        fail("Enemy death dispatch must reference every named mode")

    boss_marker_names = explicit_enum(
        SRC / "AsciiManager.hpp",
        "BossMarkerState",
        "BOSS_MARKER_",
        list(range(5)),
    )
    boss_marker_text = (SRC / "AsciiManager.cpp").read_text(encoding="utf-8")
    boss_marker_cases = set(
        re.findall(r"\bcase\s+(BOSS_MARKER_[A-Z0-9_]+)\s*:", boss_marker_text)
    )
    if boss_marker_cases != set(boss_marker_names):
        fail("Boss marker dispatch must reference every named state")

    enemy_direction_names = explicit_enum(
        SRC / "EnemyManager.hpp",
        "EnemyAnmDirection",
        "ENEMY_ANM_DIRECTION_",
        [0, 1, 2, 0xFF],
    )
    ecl_dependencies = (SRC / "EclDependencies.cpp").read_text(encoding="utf-8")
    enemy_direction_uses = set(
        re.findall(r"\b(ENEMY_ANM_DIRECTION_[A-Z0-9_]+)\b", ecl_dependencies)
    )
    if enemy_direction_uses != set(enemy_direction_names):
        fail("Enemy animation logic must reference every named direction state")

    music_input_names = explicit_enum(
        SRC / "MusicRoom.hpp",
        "MusicRoomInputState",
        "MUSIC_ROOM_INPUT_",
        list(range(2)),
    )
    music_text = (SRC / "MusicRoom.cpp").read_text(encoding="utf-8")
    music_input_cases = set(
        re.findall(r"\bcase\s+(MUSIC_ROOM_INPUT_[A-Z0-9_]+)\s*:", music_text)
    )
    if music_input_cases != set(music_input_names):
        fail("MusicRoom input dispatch must reference every named state")

    shake_axis_names = explicit_enum(
        SRC / "ScreenEffect.cpp",
        "ScreenShakeAxisSample",
        "SCREEN_SHAKE_AXIS_",
        list(range(3)),
    )
    shake_text = (SRC / "ScreenEffect.cpp").read_text(encoding="utf-8")
    shake_axis_cases = set(
        re.findall(r"\bcase\s+(SCREEN_SHAKE_AXIS_[A-Z0-9_]+)\s*:", shake_text)
    )
    if shake_axis_cases != set(shake_axis_names):
        fail("Screen shake dispatches must reference every named axis sample")


def main() -> int:
    paths = source_files()
    check_ecl_opcode_protocol()
    check_ecl_operand_protocol()
    check_background_protocol()
    check_ecl_timeline_protocol()
    check_replay_frame_event_protocol(paths)
    check_closed_small_dispatches()

    reject_pattern(
        [path for path in paths if path.name != "ZunMath.hpp"],
        "raw Float3/D3DXVECTOR3 pointer cast",
        r"reinterpret_cast\s*<\s*(?:const\s+)?(?:Float3|D3DXVECTOR3)\s*\*\s*>",
    )
    reject_pattern(
        paths,
        "numeric fixed Effect ID",
        r"\.SpawnEffect(?:InSecondaryPool|InFixedSlot|InFixedSlotWithVelocity|WithVelocity)?"
        r"\s*\(\s*(?:0x[0-9a-f]+|\d+)",
    )
    reject_pattern(
        paths,
        "numeric fixed SoundIdx cast",
        r"static_cast\s*<\s*SoundIdx\s*>\s*\(\s*(?:0x[0-9a-f]+|\d+)\s*\)",
    )
    reject_pattern(
        paths,
        "numeric ANM resource slot",
        r"\b(?:LoadAnm|PreloadAnm|ReleaseAnm|GetAnm)\s*\(\s*(?:0x[0-9a-f]+|\d+)",
    )

    print("TH08 semantic protocol checks passed")
    print("  ECL opcode dispatch: 184/184 named")
    print("  ECL operand selectors: 101/101 named")
    print("  Background opcodes: 35/35 named")
    print("  ECL timeline opcodes: 17/17 named")
    print("  replay frame-event writers: 0 raw masks")
    print("  closed UI/gameplay state protocols: guarded")
    print("  raw vector pointer casts: 0 outside exact-safe views")
    print("  fixed numeric effect/sound/resource IDs: 0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
