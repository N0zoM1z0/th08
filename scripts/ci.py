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
            "Test relocation literal validation",
            [sys.executable, "scripts/test-match-literals.py"],
        )
        run("Test math callee validation", [sys.executable, "scripts/test-match-callees.py"])
        run("Test replay input binding validation", [sys.executable, "scripts/test-match-inputs.py"])
        run("Test local relocation validation", [sys.executable, "scripts/test-match-local-relocations.py"])
        run("Test replay trace comparison", [sys.executable, "scripts/test-replay-trace-comparison.py"])
        run("Test portable replay reference validation", [sys.executable, "scripts/test-portable-replay-validation.py"])
        run("Test laser visual evidence validation", [sys.executable, "scripts/test-laser-visuals.py"])
        run("Test texture color probe coverage", [sys.executable, "scripts/test-texture-color-probe.py"])
        run("Test packed precision sample coverage", [sys.executable, "scripts/test-packed-texture-precision.py"])
        run("Test replay file validation", [sys.executable, "scripts/test-replay-file.py"])
        run("Test replay capture cleanup", [sys.executable, "scripts/test-replay-capture-cleanup.py"])
        run("Test replay suite progress and process ownership", [sys.executable, "scripts/test-replay-suite-report.py"])
        run("Validate replay route coverage", [sys.executable, "scripts/validate-replay-fixtures.py"])
        run("Smoke-test portable replay repair CLI", [sys.executable, "scripts/test-portable-replay-repairs.py", "--help"])
        run("Smoke-test portable replay capture CLI", [sys.executable, "scripts/capture-portable-replay.py", "--help"])
        run("Smoke-test portable replay suite CLI", [sys.executable, "scripts/test-portable-replay-suite.py", "--help"])
        run("Smoke-test portable vector precision CLI", [sys.executable, "scripts/test-portable-math.py", "--help"])
        run("Smoke-test portable sincos precision CLI", [sys.executable, "scripts/test-portable-sincos.py", "--help"])
        run("Smoke-test portable normalization CLI", [sys.executable, "scripts/test-portable-normalization.py", "--help"])
        run("Smoke-test effect pool comparison CLI", [sys.executable, "scripts/analysis/compare-effect-pools.py", "--help"])
        run("Smoke-test replay frame inspection CLI", [sys.executable, "scripts/analysis/check-stage5-background.py", "--help"])
        run("Smoke-test laser frame inspection CLI", [sys.executable, "scripts/analysis/check-laser-visuals.py", "--help"])
        run("Smoke-test laser visual comparison CLI", [sys.executable, "scripts/analysis/compare-laser-visuals.py", "--help"])
        run("Smoke-test sprite quad comparison CLI", [sys.executable, "scripts/analysis/compare-sprite-quads.py", "--help"])
        run("Smoke-test subpixel quad probe CLI", [sys.executable, "scripts/analysis/probe-subpixel-quads.py", "--help"])
        run("Smoke-test alpha pipeline benchmark CLI", [sys.executable, "scripts/analysis/benchmark-alpha-pipeline.py", "--help"])
        run("Smoke-test laser error localization CLI", [sys.executable, "scripts/analysis/analyze-laser-errors.py", "--help"])
        run("Smoke-test laser pixel summary CLI", [sys.executable, "scripts/analysis/summarize-laser-pixels.py", "--help"])
        run("Smoke-test texture color probe CLI", [sys.executable, "scripts/analysis/probe-texture-colors.py", "--help"])
        run("Smoke-test texture filtering probe CLI", [sys.executable, "scripts/analysis/probe-texture-filtering.py", "--help"])
        run("Smoke-test packed texture precision CLI", [sys.executable, "scripts/analysis/probe-packed-precision.py", "--help"])
        run("Smoke-test surface scaling probe CLI", [sys.executable, "scripts/analysis/probe-surface-scaling.py", "--help"])
        run("Smoke-test surface codec observer CLI", [sys.executable, "scripts/analysis/observe-surface-codecs.py", "--help"])
        run("Smoke-test font metrics probe CLI", [sys.executable, "scripts/analysis/probe-font-metrics.py", "--help"])
        run("Smoke-test font pixel probe CLI", [sys.executable, "scripts/analysis/probe-font-pixels.py", "--help"])
        run("Smoke-test font rasterization analysis CLI", [sys.executable, "scripts/analysis/analyze-font-rasterization.py", "--help"])
        run("Smoke-test FreeType architecture probe CLI", [sys.executable, "scripts/analysis/probe-freetype-architecture.py", "--help"])
        run("Smoke-test font blending probe CLI", [sys.executable, "scripts/analysis/probe-font-blending.py", "--help"])
        run("Smoke-test portable display inspection CLI", [sys.executable, "scripts/analysis/check-portable-display.py", "--help"])
        run("Smoke-test portable renderer contract CLI", [sys.executable, "scripts/test-portable-renderer.py", "--help"])
        run("Smoke-test portable rendering regression CLI", [sys.executable, "scripts/test-portable-rendering.py", "--help"])
        run(
            "Test render-audit checker contract",
            [sys.executable, "scripts/test-render-audit.py"],
        )
        run(
            "Test semantic protocol guards",
            [sys.executable, "scripts/test-semantic-protocols.py"],
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
        run(
            "Check semantic protocol readability",
            [sys.executable, "scripts/check-semantic-protocols.py"],
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
