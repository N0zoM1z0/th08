#!/usr/bin/env python3
"""Check float-vector rounding against independent high-precision values."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--baseline-ref", help="Use ZunMath.hpp from this Git revision as a negative control")
    parser.add_argument("--expect-failures", action="store_true", help="Require both historical helper failures")
    parser.add_argument("--candidate", type=Path, help="Also check VectorAngle in this Debug ELF, without game assets")
    args = parser.parse_args()
    if args.expect_failures and not args.baseline_ref:
        parser.error("--expect-failures requires --baseline-ref")
    build = ROOT / "build"
    build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="portable-math.", dir=build) as name:
        temporary = Path(name)
        include = ROOT / "src"
        if args.baseline_ref:
            revision = subprocess.check_output(["git", "rev-parse", "--verify", args.baseline_ref + "^{commit}"], cwd=ROOT, text=True).strip()
            source = subprocess.check_output(["git", "show", revision + ":src/ZunMath.hpp"], cwd=ROOT)
            (temporary / "ZunMath.hpp").write_bytes(source)
            include = temporary
        executable = temporary / "portable-math"
        subprocess.run([args.compiler, "-std=c++98", "-O0", "-DTH08_MODERN_PORT", "-I", str(include),
                        "-I", str(ROOT / "src"), str(ROOT / "tests/portable-math.cpp"), "-o", str(executable)], check=True)
        result = subprocess.run([str(executable)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        print(result.stdout, end="", flush=True)
        angle_result = None
        if args.candidate:
            commands = temporary / "angle.gdb"
            commands.write_text("""set pagination off
set confirm off
set debuginfod enabled off
start
python
import gdb, struct
angle = gdb.parse_and_eval('th08::VectorAngle(-0.3900333344936371f, 1.9535869359970093f)')
bits = struct.unpack('<I', struct.pack('<f', float(angle)))[0]
if bits != 0xbe49c9cc:
    print('FAIL: VectorAngle: expected be49c9cc, observed %08x' % bits)
    gdb.execute('quit 1')
print('PASS: VectorAngle matches the retail stage-4A angle sample')
end
quit 0
""")
            environment = {key: value for key, value in os.environ.items() if not key.startswith("TH08_REPLAY_TRACE_")}
            environment.update(LC_ALL="C", SDL_AUDIODRIVER="dummy")
            angle_result = subprocess.run(["gdb", "--batch", "--quiet", "-x", str(commands), "--args",
                                           str(args.candidate.resolve(strict=True)), "--data-dir", str(temporary)],
                                          cwd=temporary, env=environment, stdout=subprocess.PIPE,
                                          stderr=subprocess.STDOUT, text=True, timeout=30)
            messages = [line for line in angle_result.stdout.splitlines() if line.startswith(("PASS:", "FAIL:"))]
            print("\n".join(messages), flush=True)
            if not messages:
                raise RuntimeError("VectorAngle probe did not complete:\n" + angle_result.stdout)
        if args.expect_failures:
            if result.returncode != 1 or not all("FAIL: " + name in result.stdout for name in ("FromAngleMagnitude", "FromRotatedVec2")):
                raise RuntimeError("Historical vector precision failures were not reproduced")
            if angle_result and (angle_result.returncode != 1 or "FAIL: VectorAngle:" not in angle_result.stdout):
                raise RuntimeError("Historical VectorAngle failure was not reproduced")
            print("Historical vector precision failures reproduced", flush=True)
            return 0
        return result.returncode or (angle_result.returncode if angle_result else 0)


if __name__ == "__main__":
    raise SystemExit(main())
