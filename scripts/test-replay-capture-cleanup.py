#!/usr/bin/env python3
"""Verify that replay capture cleanup reaps responsive and stalled children."""
import importlib.util
import os
from pathlib import Path
import signal
import subprocess
import sys
import unittest


spec = importlib.util.spec_from_file_location("capture", Path(__file__).with_name("capture-replay.py"))
capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(capture)


@unittest.skipUnless(os.name == "posix", "Process cleanup tests require POSIX signals")
class CaptureCleanupTests(unittest.TestCase):
    def check_child(self, ignore_termination, expected_signal):
        script = (
            "import signal\n"
            + ("signal.signal(signal.SIGTERM, signal.SIG_IGN)\n" if ignore_termination else "")
            + "print('ready', flush=True)\nsignal.pause()\n"
        )
        process = subprocess.Popen([sys.executable, "-c", script], stdout=subprocess.PIPE, text=True)
        try:
            self.assertEqual(process.stdout.readline(), "ready\n")
            capture.stop_process(process, timeout=0.1)
            self.assertEqual(process.returncode, -expected_signal)
        finally:
            if process.poll() is None:
                process.kill()
            process.wait(timeout=5)
            process.stdout.close()

    def test_responsive_child_exits_on_termination(self):
        self.check_child(False, signal.SIGTERM)

    def test_stalled_child_is_killed_and_reaped(self):
        self.check_child(True, signal.SIGKILL)

    def test_completed_child_keeps_its_exit_status(self):
        process = subprocess.Popen([sys.executable, "-c", "pass"])
        process.wait(timeout=5)
        capture.stop_process(process, timeout=0.1)
        self.assertEqual(process.returncode, 0)


if __name__ == "__main__":
    unittest.main()
