#!/usr/bin/env python3
"""Run renderer contracts, HOME checks, and four retail replay frame comparisons."""
import argparse
import importlib.util
import json
import math
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("frame_capture", ROOT / "scripts/analysis/check-stage5-background.py")
frame_capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(frame_capture)
from replay_file import CORPUS, verify_fixture

TARGET_SHA = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"
CASES = (("stage0-frame600", 0, 600), ("stage2-frame600", 2, 600),
         ("stage5-frame6166", 5, 6166), ("stage7-frame1800", 7, 1800))


def reuse_reference(source, output, fixture, data, stage, frame):
    metadata = json.loads((source / "metadata.json").read_text())
    screenshot = json.loads((source / "screenshot.json").read_text())
    camera = json.loads((source / "camera.json").read_text())[0]
    if (metadata["product"] != "retail" or metadata["executableSha256"] != TARGET_SHA or
        metadata["fixture"] != fixture["sha256"] or metadata["startStage"] != stage or
        screenshot["stage"] != stage or screenshot["activeFrame"] != frame or
        camera["stage"] != stage or camera["activeFrame"] != frame):
        raise RuntimeError("Reference target, recording, or frame differs: " + str(source))
    if metadata["gameDataSha256"] != frame_capture.digest(data / "th08.dat") or metadata["bgmDataSha256"] != frame_capture.digest(data / "thbgm.dat"):
        raise RuntimeError("Reference game archives differ: " + str(source))
    output.mkdir()
    files = ("camera.json", "screenshot.json", "metadata.json", "complete.json",
             "window-geometry.txt", "rows.jsonl.gz", f"stage{stage}-frame-{frame}.png")
    hashes = {}
    for name in files:
        if (source / name).is_file():
            shutil.copyfile(source / name, output / name)
            hashes[name] = frame_capture.digest(output / name)
    (output.parent / "reference-provenance.json").write_text(json.dumps(
        dict(reusedFrom=str(source), fileSha256=hashes), indent=2) + "\n")


def verify_frame(directory):
    from PIL import Image, ImageChops
    report = json.loads((directory / "summary.json").read_text())
    for name, camera in report["cameras"].items():
        if not camera["scriptStateEqual"] or any(not math.isfinite(camera[key]) or camera[key] > 0.001
            for key in ("cameraCurrent", "stagePosition", "view", "projection", "viewport")):
            raise RuntimeError("Camera or script state differs: " + name)
    for comparison in report["pixelComparisons"]:
        error = comparison["playfieldMeanAbsoluteRgbDifference"]
        if not math.isfinite(error) or error >= 1.0:
            raise RuntimeError("Playfield RGB difference exceeds the rendering regression limit.")
    if (directory / "portable-paced").exists():
        filename = f"stage{report['stage']}-frame-{report['activeFrame']}.png"
        with Image.open(directory / "portable-fast" / filename) as a, Image.open(directory / "portable-paced" / filename) as b:
            if ImageChops.difference(a.convert("RGB"), b.convert("RGB")).crop((32, 16, 416, 464)).getbbox():
                raise RuntimeError("Ordinary and accelerated playfields differ.")
        fast = json.loads((directory / "portable-fast/camera.json").read_text())
        paced = json.loads((directory / "portable-paced/camera.json").read_text())
        if fast != paced:
            raise RuntimeError("Ordinary and accelerated camera packets differ.")
    print("PASS: retail frame, camera/script state, and pacing for " + directory.name, flush=True)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--reference-captures", type=Path, help="Reuse pinned retail captures from an earlier frame suite")
    parser.add_argument("--cases", help="Comma-separated frame directory names; default: all four")
    parser.add_argument("--frames-only", action="store_true", help="Run frame comparisons after separate renderer and HOME checks")
    args = parser.parse_args()
    candidate, data, output, target = (path.resolve() for path in
                                      (args.candidate, args.game_data_dir, args.output_dir, args.target))
    if not output.is_relative_to(ROOT / "build") or output.exists():
        parser.error("--output-dir must be a fresh directory under build/.")
    if not target.is_file() or target.stat().st_size != 840704 or frame_capture.digest(target) != TARGET_SHA:
        parser.error("--target must be the canonical Japanese TH08 1.00d executable.")
    if not candidate.is_file() or any(not (data / name).is_file() for name in ("th08.dat", "thbgm.dat")):
        parser.error("Portable executable and both original game archives are required.")
    cases = CASES
    if args.cases:
        selected = set(args.cases.split(","))
        cases = [case for case in CASES if case[0] in selected]
        if selected != {case[0] for case in cases}:
            parser.error("Unknown frame case.")
    fixture = next(item for item in json.loads((ROOT / "config/replay-fixtures.json").read_text())["fixtures"]
                   if item["id"] == "lunatic-0-b")
    replay = CORPUS / "accepted" / fixture["file"]
    verify_fixture(replay, fixture)
    output.mkdir(parents=True)
    if not args.frames_only:
        subprocess.run([sys.executable, str(ROOT / "scripts/test-portable-renderer.py"), "--output-dir",
                        str(output / "renderer-contract")], check=True)
        subprocess.run([sys.executable, str(ROOT / "scripts/analysis/check-portable-display.py"), "--portable",
                        str(candidate), "--game-data-dir", str(data), "--output-dir", str(output / "display"), "--verify"], check=True)
    reports = []
    for name, stage, frame in cases:
        directory = output / name
        directory.mkdir()
        if args.reference_captures:
            reuse_reference(args.reference_captures.resolve() / name / "retail", directory / "retail", fixture, data, stage, frame)
        else:
            frame_capture.capture_retail(target, data, replay, directory / "retail", stage, frame)
        frame_capture.capture_portable(candidate, data, replay, directory / "portable-fast", True, stage, frame)
        if stage == 5:
            frame_capture.capture_portable(candidate, data, replay, directory / "portable-paced", False, stage, frame)
        frame_capture.summarize(directory, stage, frame)
        reports.append(verify_frame(directory))
    manifest = dict(candidateSha256=frame_capture.digest(candidate), targetSha256=TARGET_SHA,
                    fixture=fixture["id"], replaySha256=fixture["sha256"], frames=reports,
                    sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                    workingTreeDirty=bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
                    runnerSha256=frame_capture.digest(Path(__file__)),
                    framesOnly=args.frames_only, playfieldRgbErrorLimit=1.0, cameraDifferenceLimit=0.001)
    (output / "report.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print("Portable rendering regression suite passed.", flush=True)


if __name__ == "__main__":
    main()
