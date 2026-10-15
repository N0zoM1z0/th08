#!/usr/bin/env python3
"""Check portable sincos rounding against observed and high-precision values."""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--baseline-ref", help="Immutable header revision for a failure control")
    parser.add_argument("--expect-failure", action="store_true", help="Require the old sincos rounding defect")
    parser.add_argument("--single-precision", action="store_true", help="Exercise the i386 device's single-precision x87 mode")
    args = parser.parse_args()
    if args.expect_failure and not args.baseline_ref:
        parser.error("--expect-failure requires --baseline-ref")
    (ROOT / "build").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="portable-sincos.", dir=ROOT / "build") as name:
        temporary = Path(name)
        include = ROOT / "src"
        if args.baseline_ref:
            revision = subprocess.check_output(["git", "rev-parse", "--verify", "--end-of-options",
                                               args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
            (temporary / "ZunMath.hpp").write_bytes(subprocess.check_output(
                ["git", "show", revision + ":src/ZunMath.hpp"], cwd=ROOT))
            include = temporary
        binary = temporary / "portable-sincos"
        definitions = ["-DTH08_TEST_SINGLE_PRECISION"] if args.single_precision else []
        subprocess.run([args.compiler, "-std=c++98", "-O0", "-DTH08_MODERN_PORT", *definitions,
                        "-I", str(include), "-I", str(ROOT / "src"), str(ROOT / "tests/portable-sincos.cpp"),
                        "-o", str(binary)], cwd=ROOT, check=True)
        result = subprocess.run([str(binary)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        print(result.stdout, end="", flush=True)
        if args.expect_failure:
            if result.returncode != 1 or "FAIL: sincos angle 3f879c1a" not in result.stdout:
                raise RuntimeError("The observed laser's historical sincos failure was not reproduced")
            print("Historical sincos rounding failure reproduced", flush=True)
            return 0
        return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
