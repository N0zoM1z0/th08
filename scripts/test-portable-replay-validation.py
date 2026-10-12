#!/usr/bin/env python3
"""Reject stale or misidentified retail evidence before portable playback."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("capture", Path(__file__).with_name("capture-portable-replay.py"))
capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(capture)


class ValidationTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        (self.root / "scripts").mkdir()
        (self.root / "config").mkdir()
        self.schema = json.loads(Path(__file__).with_name("replay-external-schema.json").read_text())
        (self.root / "scripts/replay-external-schema.json").write_text(json.dumps(self.schema))
        fixture = dict(sha256="fixture", retailTrace=dict(calculationFrames=1, sha256="published"))
        (self.root / "config/replay-fixtures.json").write_text(json.dumps(dict(fixtures=[fixture])))
        self.replay = dict(sha256="fixture", stages=[dict(index=0)])
        self.metadata = dict(product="retail", executableSha256="target.exe", targetSha256="target.exe",
                             schema=self.schema, fixture="fixture", replay=self.replay, startStage=0, playbackMode=2,
                             gameDataSha256="th08.dat", bgmDataSha256="thbgm.dat", muted=True,
                             configSha256=hashlib.sha256(b"config").hexdigest())
        self.fingerprint = "published"
        comparison = SimpleNamespace(load=lambda _: (self.metadata, [[0] * 25]),
                                     trace_sha256=lambda *_: self.fingerprint)
        native = SimpleNamespace(muted_config=lambda: b"config")
        for item in (patch.object(capture, "ROOT", self.root),
                     patch.object(capture, "sha", lambda path: path.name),
                     patch.object(capture, "module", lambda name: comparison if name == "compare-replay-traces" else native)):
            item.start()
            self.addCleanup(item.stop)

    def validate(self):
        return capture.validate_reference(self.root, self.replay, self.root, self.root / "target.exe")

    def test_identified_reference_is_accepted(self):
        self.assertEqual(self.validate()[2], "published")

    def test_each_identity_field_is_required(self):
        for field in tuple(self.metadata):
            with self.subTest(field=field):
                value = self.metadata.pop(field)
                with self.assertRaisesRegex(ValueError, field):
                    self.validate()
                self.metadata[field] = value

    def test_wrong_archive_is_rejected(self):
        for field in ("gameDataSha256", "bgmDataSha256"):
            with self.subTest(field=field):
                self.metadata[field] = "other"
                with self.assertRaisesRegex(ValueError, field):
                    self.validate()
                self.metadata[field] = "th08.dat" if field == "gameDataSha256" else "thbgm.dat"

    def test_native_candidate_cannot_be_a_retail_reference(self):
        self.metadata["product"] = "native-vc7-bugfix"
        with self.assertRaisesRegex(ValueError, "product"):
            self.validate()

    def test_different_playback_mode_is_rejected(self):
        self.metadata["playbackMode"] = 0
        with self.assertRaisesRegex(ValueError, "playbackMode"):
            self.validate()

    def test_changed_trace_is_rejected(self):
        self.fingerprint = "different"
        with self.assertRaisesRegex(ValueError, "published"):
            self.validate()


if __name__ == "__main__":
    unittest.main()
