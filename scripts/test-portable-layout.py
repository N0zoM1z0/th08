#!/usr/bin/env python3
"""Check native-layout fixes through asset-free production state transitions."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = {"effect", "bullet", "gui", "ecl", "player"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--expect-baseline", action="store_true",
                        help="Require the five known defects in the pre-repair x86_64 product")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    candidate = args.candidate.resolve(strict=True)
    header = candidate.read_bytes()[:20]
    if header[:4] != b"\x7fELF" or header[5] != 1 or struct.unpack_from("<H", header, 18)[0] not in (3, 62):
        parser.error("--candidate must be a Linux x86 or x86_64 Debug ELF")
    if args.expect_baseline and header[4] != 2:
        parser.error("--expect-baseline requires the old native x86_64 layout")
    (ROOT / "build").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="layout-checks.", dir=ROOT / "build") as directory:
        commands = Path(directory) / "checks.gdb"
        helper = ROOT / "scripts/gdb/portable-layout-checks.py"
        commands.write_text("set pagination off\nset confirm off\nset debuginfod enabled off\nstart\n"
                            "python\nimport runpy\nrunpy.run_path(%r, run_name='__main__')\nend\nquit 0\n" % str(helper))
        env = {key: value for key, value in os.environ.items() if not key.startswith("TH08_")}
        env.update(LC_ALL="C", SDL_AUDIODRIVER="dummy")
        result = subprocess.run(["gdb", "--batch", "--quiet", "-x", str(commands),
                                 "--args", str(candidate)], cwd=directory, env=env,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, timeout=90)
        checks = [json.loads(line.split(" ", 1)[1]) for line in result.stdout.splitlines()
                  if line.startswith("LAYOUT_CHECK ")]
        if result.returncode or not checks or checks[-1]["name"] != "player-solo-added-11":
            raise RuntimeError("Production state-transition checks did not complete:\n" + result.stdout)
        failures = [check for check in checks if not check["passed"]]
        if args.expect_baseline:
            passed = {check["family"] for check in failures} == FAMILIES
        else:
            passed = not failures
        report = dict(candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest(),
                      helperSha256=hashlib.sha256(helper.read_bytes()).hexdigest(),
                      runnerSha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                      pointerBits=64 if header[4] == 2 else 32,
                      mode="old-baseline" if args.expect_baseline else "fixed",
                      result="passed" if passed else "failed", checks=checks)
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(report, indent=2) + "\n")
        if args.expect_baseline:
            print("%s: all five baseline defect families; %d/%d checks fail as expected" %
                  (report["result"].upper(), len(failures), len(checks)))
        else:
            print("%s: %d/%d production state-transition checks" %
                  (report["result"].upper(), len(checks) - len(failures), len(checks)))
        if not passed:
            print(json.dumps(failures, indent=2))
            return 1
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
