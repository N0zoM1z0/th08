#!/usr/bin/env python3
"""Capture muted retail/native demos sequentially and compare every recorded frame."""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--bgm-data", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--candidate", type=Path, default=ROOT / "build/th08.exe")
    parser.add_argument("--map", dest="map_path", type=Path, default=ROOT / "build/th08.map")
    parser.add_argument("--reference", type=Path, help="Reuse an existing complete retail capture")
    parser.add_argument("--demos", default="0,1,2", help="Ordered bundled demo indexes; default: all three")
    parser.add_argument("--timeout", type=float, help="Deadline for each capture")
    parser.add_argument("--display", help="Existing X display; otherwise each capture starts private Xvfb")
    args = parser.parse_args()
    args.output_dir = args.output_dir.resolve()
    if not args.output_dir.is_relative_to(ROOT / "build"):
        parser.error("Generated evidence must be under build/")
    args.output_dir.mkdir(parents=True, exist_ok=False)
    common = ["--target", str(args.target.resolve()), "--game-data", str(args.game_data.resolve()),
              "--bgm-data", str(args.bgm_data.resolve()), "--demos", args.demos]
    if args.timeout:
        common += ["--timeout", str(args.timeout)]
    if args.display:
        common += ["--display", args.display]
    capture = [sys.executable, str(ROOT / "scripts/capture-replay.py")]
    reference = args.reference.resolve() if args.reference else args.output_dir / "reference"
    if args.reference is None:
        subprocess.run(capture + common + ["--output-dir", str(reference)], cwd=ROOT, check=True)
    candidate = args.output_dir / "candidate"
    subprocess.run(capture + common + ["--candidate", str(args.candidate.resolve()), "--map", str(args.map_path.resolve()),
                                       "--output-dir", str(candidate)], cwd=ROOT, check=True)
    return subprocess.run([sys.executable, str(ROOT / "scripts/compare-replay-traces.py"),
                           str(reference), str(candidate)], cwd=ROOT).returncode


if __name__ == "__main__":
    raise SystemExit(main())
