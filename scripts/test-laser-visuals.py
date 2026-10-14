#!/usr/bin/env python3
"""Exercise laser evidence identity checks and missing-beam detection."""
import importlib.util
import hashlib
import json
from pathlib import Path
import struct
import shutil
import tempfile
import unittest
from unittest import mock

from PIL import Image

spec = importlib.util.spec_from_file_location("laser_visuals", Path(__file__).parent / "analysis/compare-laser-visuals.py")
visuals = importlib.util.module_from_spec(spec)
spec.loader.exec_module(visuals)
spec = importlib.util.spec_from_file_location("laser_errors", Path(__file__).parent / "analysis/analyze-laser-errors.py")
errors = importlib.util.module_from_spec(spec)
spec.loader.exec_module(errors)
spec = importlib.util.spec_from_file_location("laser_capture", Path(__file__).parent / "analysis/check-laser-visuals.py")
capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(capture)
spec = importlib.util.spec_from_file_location("sprite_quads", Path(__file__).parent / "analysis/compare-sprite-quads.py")
quads = importlib.util.module_from_spec(spec)
spec.loader.exec_module(quads)
spec = importlib.util.spec_from_file_location("subpixel_probe", Path(__file__).parent / "analysis/probe-subpixel-quads.py")
subpixel = importlib.util.module_from_spec(spec)
spec.loader.exec_module(subpixel)
spec = importlib.util.spec_from_file_location("portable_draws", Path(__file__).parent / "analysis/compare-portable-draws.py")
portable_draws = importlib.util.module_from_spec(spec)
spec.loader.exec_module(portable_draws)
spec = importlib.util.spec_from_file_location("laser_spawns", Path(__file__).parent / "analysis/compare-laser-spawns.py")
laser_spawns = importlib.util.module_from_spec(spec)
spec.loader.exec_module(laser_spawns)


def bits(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


class LaserSpawnTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.reference, self.candidate = (Path(temporary.name) / name for name in ("retail", "portable"))
        common = dict(gameDataSha256="data", bgmDataSha256="bgm", configSha256="config", startStage=7, muted=True)
        reference = dict(common, product="retail", executableSha256=laser_spawns.TARGET_SHA256, fixture="replay",
                         laserCapture=dict(gameplayMemoryReadOnly=True, executableBytesUnmodified=True, watchLaserSpawns=[80, 100]))
        candidate = dict(common, product="portable-linux", executableSha256="candidate", replaySha256="replay",
                         runnerExitCode=0, watchLaserSpawns=[80, 100])
        row = dict(stage=7, frame=90, enemySlot=2, firstFreeSlot=12, caller="caller")
        for field in ("descriptor.position", "enemy.position", "enemy.worldPosition", "enemy.shootOffset"):
            row[field] = [bits(1), bits(2), bits(0)]
        row.update({"descriptor.angle": bits(0.5), "descriptor.speed1": bits(4), "descriptor.bulletType": 4,
                    "descriptor.color": 4, "descriptor.laserDuration": 800, "ecl.subId": 52,
                    "ecl.time": 5, "ecl.instruction": "00" * 12, "ecl.floatVariables": [0] * 8})
        for directory, metadata in ((self.reference, reference), (self.candidate, candidate)):
            directory.mkdir()
            (directory / "metadata.json").write_text(json.dumps(metadata))
            (directory / "laser-spawns.json").write_text(json.dumps(dict(
                boundary="SpawnLaserPattern entry", frameCounter="GameManager.gameplayFrameCounter",
                range=[80, 100], complete=True, records=[row])))
        (self.reference / "complete.json").write_text(json.dumps(dict(diagnosticComplete=True, errors=[])))

    def mutate(self, filename, action):
        path = self.candidate / filename
        data = json.loads(path.read_text())
        action(data)
        path.write_text(json.dumps(data))

    def test_one_ulp_is_detected_and_platform_caller_name_is_ignored(self):
        self.mutate("laser-spawns.json", lambda d: d["records"][0].update(caller="portable caller"))
        self.assertEqual(laser_spawns.compare(self.reference, self.candidate)["result"], "equal")
        self.mutate("laser-spawns.json", lambda d: d["records"][0]["descriptor.position"].__setitem__(1, bits(2) + 1))
        result = laser_spawns.compare(self.reference, self.candidate)
        self.assertEqual(result["result"], "different")
        self.assertEqual(result["differences"][0]["field"], "descriptor.position")

    def test_missing_raw_state_cannot_pass(self):
        self.mutate("laser-spawns.json", lambda d: d["records"][0].pop("enemy.shootOffset"))
        with self.assertRaisesRegex(ValueError, "raw float"):
            laser_spawns.compare(self.reference, self.candidate)

    def test_incomplete_range_or_mismatched_replay_cannot_pass(self):
        self.mutate("laser-spawns.json", lambda d: d.update(complete=False))
        with self.assertRaisesRegex(ValueError, "incomplete"):
            laser_spawns.compare(self.reference, self.candidate)
        self.mutate("laser-spawns.json", lambda d: d.update(complete=True))
        self.mutate("metadata.json", lambda d: d.update(replaySha256="another recording"))
        with self.assertRaisesRegex(ValueError, "identities"):
            laser_spawns.compare(self.reference, self.candidate)

    def test_different_record_counts_are_detected(self):
        self.mutate("laser-spawns.json", lambda d: d["records"].append(dict(d["records"][0])))
        result = laser_spawns.compare(self.reference, self.candidate)
        self.assertEqual(result["result"], "different")
        self.assertEqual(result["differences"][0]["field"], "recordCount")


class FontEvidenceTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.font = Path(temporary.name) / "font with spaces.ttf"
        self.font.write_bytes(b"font identity fixture")

    def test_explicit_override_records_file_and_hash(self):
        evidence = capture.font_environment(str(self.font))
        self.assertEqual(evidence["file"], str(self.font))
        self.assertEqual(evidence["sha256"], hashlib.sha256(self.font.read_bytes()).hexdigest())
        self.assertEqual(evidence["selection"], "TH08_FONT")

    def test_relative_or_missing_overrides_fail(self):
        with self.assertRaisesRegex(ValueError, "must be absolute"):
            capture.font_environment("relative.ttf")
        with self.assertRaisesRegex(ValueError, "unreadable"):
            capture.font_environment(str(self.font.with_name("missing.ttf")))

    def test_fallback_records_the_fontconfig_selection(self):
        with mock.patch.object(capture.subprocess, "check_output", return_value=str(self.font)) as query:
            evidence = capture.font_environment(None)
        self.assertEqual(evidence["file"], str(self.font))
        self.assertEqual(query.call_args.args[0],
                         ["fc-match", "-f", "%{file}", "VL Gothic:lang=ja:spacing=mono"])


class DrawEvidenceTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.reference, self.candidate = (Path(temporary.name) / name for name in ("reference", "candidate"))
        inputs = dict(primitiveType=5, primitiveCount=2, stride=28, fvf=0x144, vertices=[0] * 28,
                      world=[0] * 16, view=[0] * 16, projection=[0] * 16, textureTransform=[0] * 16,
                      viewport=[0, 0, 640, 480, 0, bits(1)], renderStates=[0] * 256,
                      textureStates=[0] * 32, texture=None)
        metadata = dict(product="portable-linux", watchDraws=True, runnerExitCode=0, temporaryRuntimeRemoved=True,
                        rasterization=True, scout=False, muted=True, stage=4, frames=[100], executableSha256="a" * 64)
        metadata.update(frameCounter="GameManager.gameplayFrameCounter after Present", playbackMode=0,
                        startStage=4, keepLives=False)
        for key in ("replaySha256", "gameDataSha256", "bgmDataSha256", "configSha256"):
            metadata[key] = "b" * 64
        self.rows = [dict(stage=4, frame=100, draws=[dict(inputs=inputs, cpuFloatingPoint=dict(x87=895, mxcsr=8192))])]
        for folder in (self.reference, self.candidate):
            folder.mkdir()
            (folder / "metadata.json").write_text(json.dumps(metadata))
            (folder / "draws.json").write_text(json.dumps(self.rows))

    def write_candidate(self):
        (self.candidate / "draws.json").write_text(json.dumps(self.rows))

    def test_cpu_environment_is_separate_from_input_equality(self):
        self.rows[0]["draws"][0]["cpuFloatingPoint"]["x87"] = 127
        self.write_candidate()
        report = portable_draws.compare(self.reference, self.candidate)
        self.assertEqual(report["result"], "equal")
        self.assertNotEqual(report["frames"][0]["referenceFpu"], report["frames"][0]["candidateFpu"])

    def test_single_word_difference_cannot_pass(self):
        self.rows[0]["draws"][0]["inputs"]["projection"][5] = 1
        self.write_candidate()
        report = portable_draws.compare(self.reference, self.candidate)
        self.assertEqual(report["result"], "different")
        self.assertEqual(report["frames"][0]["differences"][0]["field"], "[0].projection[5]")

    def test_incomplete_draw_layout_is_rejected(self):
        self.rows[0]["draws"][0]["inputs"]["vertices"].pop()
        self.write_candidate()
        with self.assertRaisesRegex(ValueError, "word layout"):
            portable_draws.compare(self.reference, self.candidate)

    def test_experimental_draw_control_is_not_an_ordinary_capture(self):
        path = self.candidate / "metadata.json"
        metadata = json.loads(path.read_text())
        metadata["renderInputControl"] = True
        path.write_text(json.dumps(metadata))
        with self.assertRaisesRegex(ValueError, "Experimental"):
            portable_draws.compare(self.reference, self.candidate)

    def test_rgb_control_preserves_all_meaningful_vertex_words(self):
        current = self.rows[0]["draws"][0]["inputs"]
        reference = json.loads(json.dumps(current))
        reference["vertices"][4] = 0x00123456
        result, changed = portable_draws.borrow_transparent_rgb(current, reference)
        self.assertEqual(changed, [4])
        self.assertEqual(result, reference["vertices"])
        reference["vertices"][4] = 0xff123456
        with self.assertRaisesRegex(ValueError, "zero-alpha"):
            portable_draws.borrow_transparent_rgb(current, reference)
        reference["vertices"][4] = 0x00123456
        reference["vertices"][0] = 1
        with self.assertRaisesRegex(ValueError, "zero-alpha"):
            portable_draws.borrow_transparent_rgb(current, reference)


class QuadEvidenceTests(unittest.TestCase):
    def quad(self):
        return [[bits(x), bits(y), bits(0.06), bits(1), 0xffffffff, bits(u), bits(v)]
                for x, y, u, v in [(200, 236, 0, 0), (280, 236, 1, 0),
                                  (200, 244, 0, 1), (280, 244, 1, 1)]]

    def test_single_float_ulp_is_reported(self):
        a, b = self.quad(), self.quad()
        b[0][1] += 1
        result = quads.differences(a, b)
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]["field"], "y")
        self.assertGreater(result[0]["delta"], 0)

    def test_region_excludes_full_frame_and_offscreen_sprites(self):
        body = self.quad()
        background = [[bits(x), bits(y), *vertex[2:]] for vertex, (x, y) in zip(body,
                       [(0, 0), (640, 0), (0, 480), (640, 480)])]
        offscreen = [[bits(-100), *vertex[1:]] for vertex in body]
        self.assertEqual([x[0] for x in quads.select_region([background, body, offscreen], (201, 236, 248, 245))], [1])

    def test_missing_frame_or_invalid_layout_cannot_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            path = folder / "quads.json"
            path.write_text(json.dumps([dict(stage=2, frame=100, quads=[self.quad()])]))
            self.assertEqual(len(quads.load_quads(folder, [100])), 1)
            with self.assertRaisesRegex(ValueError, "coverage"):
                quads.load_quads(folder, [101])
            path.write_text(json.dumps([dict(stage=2, frame=100, quads=[[self.quad()[0]]])]))
            with self.assertRaisesRegex(ValueError, "layout"):
                quads.load_quads(folder, [100])

    def test_background_selection_uses_bounds_and_preserves_submission_order(self):
        body = self.quad()
        rotated = [[bits(x), bits(y), *vertex[2:]] for vertex, (x, y) in zip(body,
                   [(300, 650), (-185, 325), (635, 155), (140, -170)])]
        partial = [[bits(x), bits(y), *vertex[2:]] for vertex, (x, y) in zip(body,
                   [(40, 0), (640, 0), (40, 480), (640, 480)])]
        rows = quads.select_region([body, rotated, partial, rotated], (32, 16, 416, 464), "covering-bounds")
        self.assertEqual([row[0] for row in rows], [1, 3])
        with self.assertRaisesRegex(ValueError, "Unknown quad selection"):
            quads.select_region([rotated], (32, 16, 416, 464), "unknown")

    def test_grid_control_preserves_depth_color_and_uv_words(self):
        vertices = self.quad()
        vertices[0][0] = bits(200.00001525878906)
        vertices[0][1] = bits(235.99989318847656)
        data = struct.pack("<I", 1) + b"".join(struct.pack("<7I", *v) for v in vertices)
        result, evidence = subpixel.snap_positions(data, 256)
        self.assertEqual(result[:4], data[:4])
        for vertex in range(4, len(data), 28):
            self.assertEqual(result[vertex + 8:vertex + 28], data[vertex + 8:vertex + 28])
        self.assertEqual(struct.unpack_from("<2f", result, 4), (200, 236))
        self.assertEqual(evidence["changedWords"], 2)
        self.assertGreater(evidence["maximumPositionDelta"], 0)
        self.assertEqual(evidence["kind"], "synthetic XY grid control")
        with self.assertRaises(ValueError):
            subpixel.snap_positions(data, 0)


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

    def test_native_images_require_mapped_observer_and_completed_frames(self):
        self.portable.update(product="native-vc7-bugfix", clockRate=1, targetSha256=visuals.TARGET_SHA256,
                             nativeMapSha256="a" * 64, laserCapture=self.common["laserCapture"])
        complete = dict(errors=[], diagnosticComplete=True, capturedFrames=[100])
        self.write(self.candidate, "complete.json", complete)
        self.metadata()
        self.assertEqual(self.compare()["result"], "equal")
        complete["capturedFrames"] = [101]
        self.write(self.candidate, "complete.json", complete)
        with self.assertRaisesRegex(ValueError, "Native laser capture"):
            self.compare()
        complete["capturedFrames"] = [100]
        self.write(self.candidate, "complete.json", complete)
        self.portable["nativeMapSha256"] = ""
        self.metadata()
        with self.assertRaisesRegex(ValueError, "Native laser capture"):
            self.compare()
        self.portable["nativeMapSha256"] = "a" * 64
        self.common["wineCsmt"] = self.portable["wineCsmt"] = 0
        self.metadata()
        self.assertEqual(self.compare()["result"], "equal")
        self.portable["wineCsmt"] = 1
        self.metadata()
        with self.assertRaisesRegex(ValueError, "Native laser capture"):
            self.compare()

    def test_experimental_render_input_cannot_pass_the_ordinary_gate(self):
        self.portable["renderInputControl"] = True
        self.metadata()
        with self.assertRaisesRegex(ValueError, "Experimental render-input"):
            self.compare()

    def test_legacy_profile_requires_observed_context_evidence(self):
        self.portable["legacyOpenGL"] = True
        for renderer in ({}, dict(version="2.1 Mesa test", clipControlLoaded=True),
                         dict(version="4.5 Mesa test", clipControlLoaded=False)):
            self.portable["environment"] = dict(openGL=renderer)
            self.metadata()
            with self.assertRaisesRegex(ValueError, "Legacy OpenGL evidence"):
                self.compare()
        self.portable["environment"] = dict(openGL=dict(version="2.1 Mesa test", clipControlLoaded=False))
        self.metadata()
        self.assertEqual(self.compare()["result"], "equal")

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

    def test_case_pins_reference_state_coverage_and_phase(self):
        case = dict(stage=2, frames=[dict(frame=100, expectedLaserState=1)],
                    retailLaserStateSha256=hashlib.sha256((self.reference / "lasers.json").read_bytes()).hexdigest())
        self.write(self.candidate, "case.json", case)
        self.assertEqual(self.compare()["result"], "equal")
        for update, message in ((dict(retailLaserStateSha256="0" * 64), "packet hash"),
                                (dict(stage=3), "coverage"),
                                (dict(frames=[dict(frame=100, expectedLaserState=0)]), "phase")):
            with self.subTest(update=update):
                self.write(self.candidate, "case.json", dict(case, **update))
                with self.assertRaisesRegex(ValueError, message):
                    self.compare()

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


class ErrorPartitionTests(unittest.TestCase):
    def laser(self):
        return {"state": 0, "speed": bits(0), "startOffset": bits(0), "hideCapDuringStartup": True,
                "bodyVm.flagsWord": 3, "bodyVm.color1.d3dColor": 0xffffffff,
                "bodyVm.scale": [bits(1), bits(1)], "bodyVm.spriteSize": [bits(12), bits(40)],
                "bodyVm.pos": [bits(200), bits(240), bits(0.06)], "bodyVm.rotation": [0, 0, 0]}

    def test_regions_preserve_the_complete_original_error(self):
        laser = self.laser()
        normal = Image.new("RGB", (640, 480), (9, 6, 3))
        black = Image.new("RGB", (640, 480))
        regions = errors.partition(black, normal, black, dict(lasers=[laser]))
        pixels = visuals.geometry_mask([laser])[0].histogram()[255]
        self.assertEqual(sum(region["pixels"] for region in regions.values()), pixels)
        self.assertEqual(sum(region["absoluteRgbSum"] for region in regions.values()), pixels * 18)
        self.assertEqual(regions["influencedBeam"]["pixels"], pixels)
        self.assertEqual(regions["influencedBeam"]["meanSignedRgbError"], [9, 6, 3])

    def test_header_overlapping_a_cap_is_counted_once(self):
        laser = self.laser()
        laser["bodyVm.pos"] = [bits(200), bits(40), bits(0.06)]
        laser["hideCapDuringStartup"] = False
        laser.update({key.replace("bodyVm.", "startCapVm."): value for key, value in tuple(laser.items())
                      if key.startswith("bodyVm.")})
        normal = Image.new("RGB", (640, 480), (9, 6, 3))
        black = Image.new("RGB", (640, 480))
        regions = errors.partition(black, normal, black, dict(lasers=[laser]))
        pixels = visuals.geometry_mask([laser])[0].histogram()[255]
        self.assertEqual(regions["header"]["pixels"], pixels)
        self.assertEqual(sum(region["pixels"] for name, region in regions.items() if name != "header"), 0)

    def test_unchanged_control_pixels_are_not_attributed_to_beam_omission(self):
        normal = Image.new("RGB", (640, 480), (9, 6, 3))
        black = Image.new("RGB", (640, 480))
        regions = errors.partition(black, normal, normal, dict(lasers=[self.laser()]))
        self.assertEqual(regions["influencedBeam"]["pixels"], 0)
        self.assertGreater(regions["remainder"]["pixels"], 0)
        self.assertEqual(regions["remainder"]["errorSharePercent"], 100)


if __name__ == "__main__":
    unittest.main()
