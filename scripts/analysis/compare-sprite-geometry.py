#!/usr/bin/env python3
"""Verify geometry-only sprite inputs against the same product's rasterized capture."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("sprite_quads", Path(__file__).with_name("compare-sprite-quads.py"))
quads = importlib.util.module_from_spec(spec)
spec.loader.exec_module(quads)
FRAME_COUNTER = "GameManager.gameplayFrameCounter"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def profile(folder, rasterized):
    metadata = json.loads((folder / "metadata.json").read_text())
    for key, expected in dict(product="portable-linux", muted=True, scout=False, keepLives=False,
                              runnerExitCode=0, temporaryRuntimeRemoved=True, watchQuads=True,
                              frameCounter=FRAME_COUNTER, playbackMode=0, rasterization=rasterized).items():
        if metadata.get(key) != expected:
            raise ValueError("Invalid geometry capture profile: " + key)
    if metadata.get("geometryOnly", False) != (not rasterized):
        raise ValueError("Geometry-only metadata differs from the requested comparison scope")
    if metadata.get("suppressLaserBodies") or metadata.get("renderInputControl"):
        raise ValueError("Controlled draw inputs require a separate diagnostic comparison")
    frames = metadata.get("frames", [])
    if (not frames or any(type(frame) is not int or frame < 1 for frame in frames)
            or frames != sorted(set(frames))):
        raise ValueError("Geometry capture requires ordered unique positive frames")
    if type(metadata.get("stage")) is not int or not 0 <= metadata["stage"] <= 8:
        raise ValueError("Geometry capture has an invalid stage index")
    if metadata.get("startStage") != metadata["stage"]:
        raise ValueError("Geometry capture start stage differs")
    if not rasterized:
        observation = json.loads((folder / "geometry-observation.json").read_text())
        if (observation.get("rasterizationEnvironment") != "true"
                or observation.get("inferiorRasterEnvironment") != "LP_NO_RAST = true"
                or observation.get("capturedFrames") != frames
                or observation.get("imageCapture") is not False):
            raise ValueError("Geometry observer did not confirm the nonrasterized frame coverage")
        if list(folder.glob("frame-*.png")):
            raise ValueError("Geometry-only capture contains unexpected frame images")
    return metadata


def compare(reference, candidate, roi=(32, 16, 416, 464)):
    left, right = profile(reference, True), profile(candidate, False)
    for key in ("executableSha256", "replaySha256", "gameDataSha256", "bgmDataSha256", "configSha256"):
        value = left.get(key, "")
        if (len(value) != 64 or any(character not in "0123456789abcdef" for character in value)
                or value != right.get(key)):
            raise ValueError("Geometry capture identities differ: " + key)
    for key in ("stage", "frames", "legacyOpenGL"):
        if left.get(key) != right.get(key):
            raise ValueError("Geometry capture profiles differ: " + key)
    font = left.get("environment", {}).get("font", {}).get("sha256")
    if not font or font != right.get("environment", {}).get("font", {}).get("sha256"):
        raise ValueError("Geometry capture font identities differ")
    expected = [(left["stage"], frame) for frame in left["frames"]]
    laser_rows = [json.loads((folder / "lasers.json").read_text()) for folder in (reference, candidate)]
    for rows in laser_rows:
        if [(row["stage"], row["frame"]) for row in rows] != expected:
            raise ValueError("Laser snapshot frame coverage differs")
    if laser_rows[0] != laser_rows[1]:
        raise ValueError("Laser/VM state differs; geometry requires aligned frames")
    a, b = (quads.load_quads(folder, left["frames"]) for folder in (reference, candidate))
    report = dict(result="equal", evidenceClass="Observed sprite input words and laser/VM state",
                  imageComparison=False, roi=list(roi), rawLaserPacketsEqual=True, frames=[],
                  analyzerSha256=digest(Path(__file__)), inputSha256={})
    for folder in (reference, candidate):
        for name in ("metadata.json", "lasers.json", "quads.json"):
            report["inputSha256"][str((folder / name).resolve())] = digest(folder / name)
    report["inputSha256"][str((candidate / "geometry-observation.json").resolve())] = digest(candidate / "geometry-observation.json")
    for x, y, stage_frame in zip(a, b, expected):
        if (x["stage"], x["frame"]) != stage_frame or (y["stage"], y["frame"]) != stage_frame:
            raise ValueError("Submitted quad stage/frame differs")
        image = reference / f"frame-{stage_frame[1]}.png"
        with Image.open(image) as frame:
            if frame.size != (640, 480):
                raise ValueError("Rasterized reference image dimensions differ")
            frame.verify()
        report["inputSha256"][str(image.resolve())] = digest(image)
        selected_a = quads.select_region(x["quads"], roi, "intersecting-bounds")
        selected_b = quads.select_region(y["quads"], roi, "intersecting-bounds")
        if not selected_a or not selected_b:
            raise ValueError("Geometry region has no submitted quads")
        equal = len(selected_a) == len(selected_b) and [row[1] for row in selected_a] == [row[1] for row in selected_b]
        examples, different_words = [], 0
        for u, v in zip(selected_a, selected_b):
            differences = quads.differences(u[1], v[1])
            different_words += len(differences)
            if differences and len(examples) < 16:
                examples.append(dict(referenceIndex=u[0], candidateIndex=v[0], differences=differences))
        report["frames"].append(dict(stage=stage_frame[0], frame=stage_frame[1],
            referenceQuads=len(selected_a), candidateQuads=len(selected_b), vertexWordsEqual=equal,
            pairedDifferingWords=different_words, differenceExamples=examples))
        if not equal:
            report["result"] = "mismatch"
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True, help="Rasterized portable capture with --watch-quads")
    parser.add_argument("--candidate", type=Path, required=True, help="Geometry-only capture of the same executable and frames")
    parser.add_argument("--output", type=Path, required=True, help="Fresh JSON file under build/")
    parser.add_argument("--check", action="store_true", help="Return failure for differing input words")
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh file under build/")
    report = compare(args.reference, args.candidate)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n")
    print("Sprite geometry comparison: " + report["result"])
    for row in report["frames"]:
        print(f"Stage index {row['stage']}, frame {row['frame']}: "
              f"{row['referenceQuads']}/{row['candidateQuads']} playfield quads; words equal: {row['vertexWordsEqual']}")
    return 1 if args.check and report["result"] != "equal" else 0


if __name__ == "__main__":
    raise SystemExit(main())
