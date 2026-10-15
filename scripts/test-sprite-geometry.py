#!/usr/bin/env python3
"""Check geometry capture identities, complete frame coverage and numeric scope."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import contextlib
import io
import sys
from unittest import mock
import unittest

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("geometry_comparison", ROOT / "scripts/analysis/compare-sprite-geometry.py")
geometry = importlib.util.module_from_spec(spec)
spec.loader.exec_module(geometry)


def word(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


class GeometryTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.reference = Path(self.temporary.name) / "reference"
        self.candidate = Path(self.temporary.name) / "candidate"
        metadata = dict(product="portable-linux", muted=True, scout=False, keepLives=False,
            runnerExitCode=0, temporaryRuntimeRemoved=True, watchQuads=True,
            frameCounter="GameManager.gameplayFrameCounter", playbackMode=0,
            stage=7, startStage=7, frames=[11490, 17460], legacyOpenGL=False,
            environment=dict(font=dict(sha256="f" * 64)))
        metadata.update({name: "a" * 64 for name in (
            "executableSha256", "replaySha256", "gameDataSha256", "bgmDataSha256", "configSha256")})
        quad = [[word(x), word(y), word(0.5), word(1.0), 0xFFFFFFFF, word(u), word(v)]
                for x, y, u, v in ((40, 80, 0, 0), (42, 80, 1, 0), (40, 82, 0, 1), (42, 82, 1, 1))]
        rows = [dict(stage=7, frame=frame, quads=[copy.deepcopy(quad), copy.deepcopy(quad)]) for frame in metadata["frames"]]
        lasers = [dict(stage=7, frame=frame, lasers=[]) for frame in metadata["frames"]]
        for folder, rasterized in ((self.reference, True), (self.candidate, False)):
            folder.mkdir()
            self.write(folder, "metadata.json", dict(metadata, rasterization=rasterized, geometryOnly=not rasterized))
            self.write(folder, "quads.json", rows)
            self.write(folder, "lasers.json", lasers)
        for frame in metadata["frames"]:
            Image.new("RGB", (640, 480)).save(self.reference / f"frame-{frame}.png")
        self.write(self.candidate, "geometry-observation.json", dict(
            rasterizationEnvironment="true", inferiorRasterEnvironment="LP_NO_RAST = true",
            imageCapture=False, capturedFrames=metadata["frames"]))

    def write(self, folder, name, value):
        (folder / name).write_text(json.dumps(value))

    def edit(self, folder, name, callback):
        value = json.loads((folder / name).read_text())
        callback(value)
        self.write(folder, name, value)

    def test_matching_complete_geometry_has_no_image_claim(self):
        report = geometry.compare(self.reference, self.candidate)
        self.assertEqual(report["result"], "equal")
        self.assertIs(report["imageComparison"], False)
        self.assertEqual([row["candidateQuads"] for row in report["frames"]], [2, 2])
        self.assertIn(str((self.reference / "frame-17460.png").resolve()), report["inputSha256"])

    def test_last_word_of_last_quad_is_compared(self):
        self.edit(self.candidate, "quads.json", lambda rows: rows[-1]["quads"][-1][-1].__setitem__(6, word(0.75)))
        report = geometry.compare(self.reference, self.candidate)
        self.assertEqual(report["result"], "mismatch")
        self.assertEqual(report["frames"][-1]["pairedDifferingWords"], 1)
        self.assertEqual(report["frames"][-1]["differenceExamples"][0]["differences"][0]["field"], "v")

    def test_missing_quad_is_a_mismatch(self):
        self.edit(self.candidate, "quads.json", lambda rows: rows[-1]["quads"].pop())
        self.assertEqual(geometry.compare(self.reference, self.candidate)["result"], "mismatch")

    def test_different_binary_rejected(self):
        self.edit(self.candidate, "metadata.json", lambda value: value.update(executableSha256="b" * 64))
        with self.assertRaisesRegex(ValueError, "executableSha256"):
            geometry.compare(self.reference, self.candidate)

    def test_bad_stage_in_later_frame_rejected(self):
        self.edit(self.candidate, "quads.json", lambda rows: rows[-1].update(stage=6))
        with self.assertRaisesRegex(ValueError, "stage/frame"):
            geometry.compare(self.reference, self.candidate)

    def test_missing_frame_rejected(self):
        self.edit(self.candidate, "quads.json", lambda rows: rows.pop())
        with self.assertRaisesRegex(ValueError, "frame coverage"):
            geometry.compare(self.reference, self.candidate)

    def test_raw_state_mismatch_rejected(self):
        self.edit(self.candidate, "lasers.json", lambda rows: rows[-1]["lasers"].append(dict(slot=1)))
        with self.assertRaisesRegex(ValueError, "Laser/VM state"):
            geometry.compare(self.reference, self.candidate)

    def test_runtime_environment_must_confirm_disabled_rasterization(self):
        self.edit(self.candidate, "geometry-observation.json", lambda value: value.update(inferiorRasterEnvironment="LP_NO_RAST = false"))
        with self.assertRaisesRegex(ValueError, "nonrasterized"):
            geometry.compare(self.reference, self.candidate)

    def test_geometry_input_cannot_serve_as_the_rasterized_reference(self):
        with self.assertRaisesRegex(ValueError, "rasterization"):
            geometry.compare(self.candidate, self.candidate)

    def test_geometry_capture_cannot_retain_placeholder_images(self):
        Image.new("RGB", (640, 480)).save(self.candidate / "frame-11490.png")
        with self.assertRaisesRegex(ValueError, "unexpected frame images"):
            geometry.compare(self.reference, self.candidate)

    def test_bad_word_outside_selected_region_still_rejected(self):
        rows = json.loads((self.candidate / "quads.json").read_text())
        quad = copy.deepcopy(rows[-1]["quads"][0])
        for vertex in quad:
            vertex[0] = word(500)
        quad[-1][6] = 0x100000000
        rows[-1]["quads"].append(quad)
        self.write(self.candidate, "quads.json", rows)
        with self.assertRaisesRegex(ValueError, "unsigned 32-bit"):
            geometry.compare(self.reference, self.candidate)


class GeometryCliTests(unittest.TestCase):
    def test_invalid_modes_exit_before_launching_a_process(self):
        spec = importlib.util.spec_from_file_location("geometry_cli", ROOT / "scripts/analysis/check-laser-visuals.py")
        capture = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(capture)
        base = ["check-laser-visuals.py", "--candidate", "unused", "--game-data-dir", "unused",
                "--fixture", "lunatic-0-b", "--stage", "7", "--frames", "11490",
                "--output-dir", str(ROOT / "build/nonexistent-geometry-cli-test")]
        for options in (["--geometry-only"], ["--watch-quads", "--geometry-only", "--retail"],
                        ["--geometry-reference", "unused"],
                        ["--watch-quads", "--geometry-only", "--frames", ",".join(map(str, range(1, 18)))]):
            with self.subTest(options=options), mock.patch.object(sys, "argv", base + options), \
                    mock.patch.object(capture.subprocess, "run") as run, contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as result:
                    capture.main()
                self.assertEqual(result.exception.code, 2)
                run.assert_not_called()


if __name__ == "__main__":
    unittest.main()
