#!/usr/bin/env python3
"""Exercise laser evidence identity checks and missing-beam detection."""
import importlib.util
import json
from pathlib import Path
import struct
import shutil
import tempfile
import unittest

from PIL import Image

spec = importlib.util.spec_from_file_location("laser_visuals", Path(__file__).parent / "analysis/compare-laser-visuals.py")
visuals = importlib.util.module_from_spec(spec)
spec.loader.exec_module(visuals)


def bits(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


class LaserVisualTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.reference, self.candidate = self.root / "retail", self.root / "portable"
        self.reference.mkdir()
        self.candidate.mkdir()
        self.common = dict(product="retail", executableSha256=visuals.TARGET_SHA256,
            targetSha256=visuals.TARGET_SHA256, fixture="replay", gameDataSha256="data", bgmDataSha256="bgm",
            muted=True, rasterization=True, startStage=2, playbackMode=0, clockRate=1, configSha256="configuration",
            laserCapture=dict(frameCounter=visuals.FRAME_COUNTER, gameplayMemoryReadOnly=True,
                              executableBytesUnmodified=True, frames=[100]))
        self.portable = dict(product="portable-linux", executableSha256="portable", replaySha256="replay",
            gameDataSha256="data", bgmDataSha256="bgm", muted=True, rasterization=True, scout=False,
            keepLives=False, runnerExitCode=0, temporaryRuntimeRemoved=True, frameCounter=visuals.FRAME_COUNTER,
            startStage=2, stage=2, playbackMode=0, configSha256="configuration", frames=[100], suppressLaserBodies=False)
        self.laser = {"slot": 0, "state": 1, "bodyVm.flagsWord": 3, "bodyVm.color1.d3dColor": 0xffffffff,
            "bodyVm.scale": [bits(1), bits(1)], "bodyVm.spriteSize": [bits(12), bits(200)],
            "bodyVm.pos": [bits(200), bits(240), bits(0.06)], "bodyVm.rotation": [0, 0, 0]}
        self.rows = [dict(stage=2, frame=100, lasers=[self.laser])]
        for folder in (self.reference, self.candidate):
            self.write(folder, "lasers.json", self.rows)
            Image.new("RGB", (640, 480), (120, 80, 200)).save(folder / "frame-100.png")
        self.write(self.reference, "complete.json", dict(errors=[], diagnosticComplete=True, capturedFrames=[100]))
        self.metadata()
        self.normal = self.root / "normal"
        shutil.copytree(self.candidate, self.normal)

    def write(self, folder, name, value):
        (folder / name).write_text(json.dumps(value) + "\n")

    def metadata(self):
        self.write(self.reference, "metadata.json", self.common)
        self.write(self.candidate, "metadata.json", self.portable)

    def compare(self):
        return visuals.compare(self.reference, self.candidate,
            normal_candidate=self.normal if self.portable.get("suppressLaserBodies") else None)

    def test_matching_scene_passes(self):
        result = self.compare()
        self.assertEqual(result["result"], "equal")
        self.assertGreater(result["frames"][0]["laserGeometryPixels"], 50)

    def test_missing_beam_is_detected(self):
        Image.new("RGB", (640, 480)).save(self.candidate / "frame-100.png")
        self.portable["suppressLaserBodies"] = True
        self.metadata()
        self.write(self.candidate, "control.json", dict(active=False, skippedQuads=1))
        result = self.compare()
        self.assertEqual(result["result"], "different")
        self.assertTrue(result["negativeControl"]["detected"])

    def test_control_must_actually_suppress_a_beam(self):
        self.portable["suppressLaserBodies"] = True
        self.metadata()
        self.write(self.candidate, "control.json", dict(active=False, skippedQuads=0))
        with self.assertRaisesRegex(ValueError, "control did not suppress"):
            self.compare()

    def test_control_cannot_pass_on_normal_renderer_differences(self):
        self.portable["suppressLaserBodies"] = True
        self.metadata()
        self.write(self.candidate, "control.json", dict(active=False, skippedQuads=1))
        result = self.compare()
        self.assertEqual(result["result"], "different")
        self.assertFalse(result["negativeControl"]["detected"])

    def test_pixels_outside_beam_do_not_hide_visibility_result(self):
        with Image.open(self.candidate / "frame-100.png") as image:
            image.paste((0, 0, 0), (420, 0, 640, 480))
            image.save(self.candidate / "frame-100.png")
        self.assertEqual(self.compare()["result"], "equal")

    def test_negative_scale_anchors_follow_signed_target_dimensions(self):
        laser = dict(self.laser, **{"bodyVm.flagsWord": 3 | (1 << 11),
                                   "bodyVm.scale": [bits(-1), bits(1)]})
        mask, visible = visuals.geometry_mask([laser])
        self.assertEqual(visible, 1)
        self.assertEqual(mask.getpixel((189, 240)), 255)
        self.assertEqual(mask.getpixel((210, 240)), 0)

    def test_empty_frame_list_cannot_pass(self):
        self.portable["frames"] = []
        self.metadata()
        with self.assertRaisesRegex(ValueError, "nonempty"):
            self.compare()

    def test_incomplete_or_missing_frame_coverage_is_rejected(self):
        self.write(self.reference, "complete.json", dict(errors=["capture stopped"], diagnosticComplete=True, capturedFrames=[100]))
        with self.assertRaisesRegex(ValueError, "did not complete"):
            self.compare()
        self.write(self.reference, "complete.json", dict(errors=[], diagnosticComplete=True, capturedFrames=[100]))
        self.write(self.candidate, "lasers.json", [])
        with self.assertRaisesRegex(ValueError, "coverage differs"):
            self.compare()

    def test_wrong_counter_or_nonrasterized_capture_is_rejected(self):
        for field, wrong in (("frameCounter", "GameManager.stageActiveFrames"), ("rasterization", False),
                             ("scout", True), ("runnerExitCode", 1)):
            with self.subTest(field=field):
                original = self.portable[field]
                self.portable[field] = wrong
                self.metadata()
                with self.assertRaisesRegex(ValueError, field):
                    self.compare()
                self.portable[field] = original

    def test_wrong_executable_or_assets_is_rejected(self):
        self.common["executableSha256"] = "other executable"
        self.metadata()
        with self.assertRaisesRegex(ValueError, "canonical"):
            self.compare()
        self.common["executableSha256"] = visuals.TARGET_SHA256
        self.portable["gameDataSha256"] = "different data"
        self.metadata()
        with self.assertRaisesRegex(ValueError, "identities differ"):
            self.compare()

    def test_state_mismatch_cannot_pass_as_matching_pixels(self):
        changed = dict(self.laser, state=2)
        self.write(self.candidate, "lasers.json", [dict(stage=2, frame=100, lasers=[changed])])
        result = self.compare()
        self.assertEqual(result["result"], "different")
        self.assertEqual(result["frames"][0]["stateDifferences"][0]["field"], "state")

    def test_absent_or_offscreen_lasers_are_not_evidence(self):
        for laser in (None, dict(self.laser, **{"bodyVm.pos": [bits(900), bits(900), bits(0.06)]}),
                      dict(self.laser, **{"bodyVm.color1.d3dColor": 0x00ffffff}),
                      dict(self.laser, **{"bodyVm.flagsWord": 3 | (1 << 17), "bodyVm.color2.d3dColor": 0x00ffffff})):
            with self.subTest(laser=laser):
                rows = [dict(stage=2, frame=100, lasers=[] if laser is None else [laser])]
                for folder in (self.reference, self.candidate):
                    self.write(folder, "lasers.json", rows)
                with self.assertRaisesRegex(ValueError, "insufficient"):
                    self.compare()


class CaseManifestTests(unittest.TestCase):
    def test_cases_select_complete_credited_recording_ranges(self):
        root = Path(__file__).resolve().parents[1]
        manifest = json.loads((root / "config/laser-visual-fixtures.json").read_text())
        fixtures = {entry["id"]: entry for entry in json.loads((root / "config/replay-fixtures.json").read_text())["fixtures"]}
        self.assertEqual(manifest["frameCounter"], visuals.FRAME_COUNTER)
        self.assertTrue(manifest["cases"])
        self.assertEqual(len({case["id"] for case in manifest["cases"]}), len(manifest["cases"]))
        for case in manifest["cases"]:
            with self.subTest(case=case["id"]):
                header = next(entry for entry in fixtures[case["fixture"]]["stages"] if entry["index"] == case["stage"])
                frames = [entry["frame"] for entry in case["frames"]]
                self.assertTrue(frames)
                self.assertEqual(frames, sorted(set(frames)))
                self.assertGreater(min(frames), 0)
                self.assertLessEqual(max(frames), header["inputRecords"] - 8)
                self.assertTrue(case["boss"])
                for entry in case["frames"]:
                    self.assertTrue(entry["phase"])
                    self.assertIn(entry["expectedLaserState"], (0, 1, 2))


if __name__ == "__main__":
    unittest.main()
