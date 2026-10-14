#!/usr/bin/env python3
"""Test replay progress reporting and process ownership boundaries without games."""
import importlib.util
import os
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("report", ROOT / "scripts/analysis/report-replay-suite.py")
report = importlib.util.module_from_spec(spec)
spec.loader.exec_module(report)


class ReplaySuiteReport(unittest.TestCase):
    def test_portable_inner_log_progress(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "build") as temporary:
            suite = Path(temporary)
            capture = suite / "case/candidate"
            capture.mkdir(parents=True)
            (suite / "case/candidate.log").write_text("")
            (capture / "gdb.log").write_text(
                "Portable replay: 30000 equal calculations, stage index 2, frame 3792\n"
                "An unrelated diagnostic line\n")
            self.assertEqual(report.progress(suite, "case"),
                             "candidate: Portable replay: 30000 equal calculations, stage index 2, frame 3792")

    def test_native_wrapper_progress(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "build") as temporary:
            suite = Path(temporary)
            (suite / "case").mkdir()
            (suite / "case/reference.log").write_text("Captured 45000 calculation frames\n")
            self.assertEqual(report.progress(suite, "case"), "reference: Captured 45000 calculation frames")

    def test_process_selection_excludes_other_suites_and_other_processes(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "build") as temporary:
            root = Path(temporary)
            suite, other, proc = root / "suite", root / "other", root / "proc"
            suite.mkdir(); other.mkdir(); proc.mkdir()
            runtime = root / "portable-state.runtime"
            runtime.mkdir()
            records = [
                (100, "th08.exe", suite / "case/candidate/runtime", b""),
                (101, "th08-modern", runtime, os.fsencode("TH08_PORTABLE_CAPTURE=" + str(suite / "case/candidate"))),
                (102, "th08-modern", runtime, os.fsencode("TH08_PORTABLE_CAPTURE=" + str(other / "case/candidate"))),
                (103, "th08-modern", runtime, b"NO_CAPTURE=1"),
                (104, "unrelated", suite, b""),
            ]
            for pid, name, cwd, environment in records:
                cwd.mkdir(parents=True, exist_ok=True)
                folder = proc / str(pid); folder.mkdir()
                (folder / "comm").write_text(name + "\n")
                (folder / "cwd").symlink_to(cwd, target_is_directory=True)
                (folder / "statm").write_text("100 10 0 0 0 0 0\n")
                (folder / "environ").write_bytes(environment + b"\0")
            selected = report.live_games(suite, proc)
            self.assertEqual([r[0] for r in selected], [100, 101])
            self.assertEqual([str(r[2]) for r in selected], ["case/candidate/runtime", "case/candidate"])
            self.assertEqual(selected[0][1], 10 * os.sysconf("SC_PAGE_SIZE"))


if __name__ == "__main__":
    unittest.main()
