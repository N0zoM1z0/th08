#!/usr/bin/env python3
"""Run the target-independent validation suite used by GitHub Actions.

This entry point deliberately does not build or compare TH08: public CI lacks
the copyrighted target and the pinned local VC7 environment.  Run
``scripts/verify-target.py`` plus the normal/object builds locally for binary
claims.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def tracked(pattern: str) -> list[str]:
    result = subprocess.run(
        ["git", "ls-files", "-z", pattern],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [entry.decode() for entry in result.stdout.split(b"\0") if entry]


def run(label: str, command: list[str]) -> None:
    print(f"\n==> {label}", flush=True)
    environment = os.environ.copy()
    environment["PYTHONPYCACHEPREFIX"] = str(ROOT / "build" / "pycache")
    subprocess.run(command, cwd=ROOT, check=True, env=environment)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog="Example:\n  python3 scripts/ci.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.parse_args()

    try:
        python_files = tracked("scripts/*.py")
        if not python_files:
            raise RuntimeError("no tracked Python scripts found")
        run("Compile tracked Python", [sys.executable, "-m", "py_compile", *python_files])
        run(
            "Test render-audit checker contract",
            [sys.executable, "scripts/test-render-audit.py"],
        )

        shell_files = tracked("scripts/*.sh")
        shell_files.extend(
            path
            for path in ("scripts/create_th08_prefix", "scripts/wineth08")
            if (ROOT / path).is_file()
        )
        if shell_files:
            run("Check shell syntax", ["bash", "-n", *sorted(set(shell_files))])

        run(
            "Validate static reconstruction ledgers",
            [sys.executable, "scripts/validate-tracking.py", "--skip-target-bytes"],
        )
        run("Validate exact-match configuration", [sys.executable, "scripts/check-match-config.py"])
        run("Validate library reconstruction ledgers", [sys.executable, "scripts/validate-library.py"])
        run("Check generated progress", [sys.executable, "scripts/progress.py", "--check"])
        run("Check generated library progress", [sys.executable, "scripts/library-progress.py", "--check"])
        run("Check tracked documentation", [sys.executable, "scripts/check-docs.py"])
        run(
            "Smoke-test status reporting",
            [
                sys.executable,
                "scripts/analysis/report-reconstruction-status.py",
                "--summary",
            ],
        )
        run(
            "Smoke-test library missing-size routing",
            [
                sys.executable,
                "scripts/analysis/report-reconstruction-status.py",
                "--category",
                "library",
                "--state",
                "missing-size",
                "--sort",
                "address",
            ],
        )
        run(
            "Smoke-test whole-image CLI routing",
            [sys.executable, "scripts/compare-whole-image.py", "--help"],
        )
        run(
            "Smoke-test import-provenance CLI routing",
            [sys.executable, "scripts/analysis/report-import-provenance.py", "--help"],
        )
        run(
            "Smoke-test TU partition candidate CLI routing",
            [sys.executable, "scripts/analysis/report-tu-partition-candidates.py", "--help"],
        )
        run(
            "Smoke-test semantic-debt CLI routing",
            [sys.executable, "scripts/analysis/report-semantic-debt.py", "--help"],
        )
        with tempfile.TemporaryDirectory(prefix="th08-ci-icon-") as directory:
            generated_icon = Path(directory) / "icon.ico"
            run(
                "Smoke-test deterministic resource icon generation",
                [
                    sys.executable,
                    "scripts/prepare-icon.py",
                    "resources/placeholder.ico",
                    str(generated_icon),
                ],
            )
            if generated_icon.stat().st_size != 3262:
                raise RuntimeError(
                    "prepared 32x32 24-bit icon has unexpected size: "
                    f"{generated_icon.stat().st_size}"
                )
        run("Check whitespace", ["git", "diff", "--check"])
    except (OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"error: CI validation failed: {exc}", file=sys.stderr)
        return 1
    print("\nTH08 target-independent CI checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
