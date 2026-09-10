#!/usr/bin/env python3
"""Regression tests for literal detection in the semantic protocol guard."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "check_semantic_protocols", ROOT / "scripts" / "check-semantic-protocols.py"
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load semantic protocol guard")
GUARD = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GUARD)


class StableIdGuardTests(unittest.TestCase):
    def assert_detected(self, pattern: str, source: str) -> None:
        self.assertIsNotNone(re.search(pattern, source, re.MULTILINE | re.DOTALL))

    def assert_allowed(self, pattern: str, source: str) -> None:
        self.assertIsNone(re.search(pattern, source, re.MULTILINE | re.DOTALL))

    def test_sound_literals_cover_all_cast_spellings(self) -> None:
        for source in (
            "g_SoundPlayer.PlaySoundByIdx(14, 0);",
            "g_SoundPlayer.PlaySoundByIdx(-1, 0);",
            "g_SoundPlayer.PlaySoundByIdx((SoundIdx)0x23, 0);",
            (
                "g_SoundPlayer.PlaySoundPositionedByIdx("
                "static_cast<SoundIdx>(15), x);"
            ),
        ):
            with self.subTest(source=source):
                self.assert_detected(GUARD.NUMERIC_SOUND_CALL, source)

    def test_dynamic_sound_indices_remain_valid(self) -> None:
        for source in (
            "g_SoundPlayer.PlaySoundByIdx((SoundIdx)soundIndex, 0);",
            "g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(entry->soundIndex), 0);",
            "g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);",
        ):
            with self.subTest(source=source):
                self.assert_allowed(GUARD.NUMERIC_SOUND_CALL, source)

    def test_screen_effect_literals_cover_all_cast_spellings(self) -> None:
        for source in (
            "ScreenEffect::RegisterChain(4, frames, color, 0, 0, priority);",
            (
                "ScreenEffect::RegisterChain((ScreenEffectType)4, frames, "
                "color, 0, 0, priority);"
            ),
            (
                "ScreenEffect::RegisterChain(static_cast<ScreenEffectType>(4), "
                "frames, color, 0, 0, priority);"
            ),
        ):
            with self.subTest(source=source):
                self.assert_detected(GUARD.NUMERIC_SCREEN_EFFECT_CALL, source)

    def test_dynamic_screen_effect_indices_remain_valid(self) -> None:
        for source in (
            (
                "ScreenEffect::RegisterChain((ScreenEffectType)(idx + "
                "SCREEN_EFFECT_FULL_FADE_HOLD), 60, 0, 0, 0, 1);"
            ),
            (
                "ScreenEffect::RegisterChain(effectType, frames, color, "
                "0, 0, priority);"
            ),
            (
                "ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, frames, "
                "color, 0, 0, priority);"
            ),
        ):
            with self.subTest(source=source):
                self.assert_allowed(GUARD.NUMERIC_SCREEN_EFFECT_CALL, source)


if __name__ == "__main__":
    unittest.main()
