#!/usr/bin/env python3
"""Compare retail and portable laser state and pixels inside attested beam geometry."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct

from PIL import Image, ImageChops, ImageDraw, ImageStat

TARGET_SHA256 = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"
FRAME_COUNTER = "GameManager.gameplayFrameCounter after Present"


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def floating(word):
    return struct.unpack("<f", struct.pack("<I", word))[0]


def geometry_mask(lasers):
    mask = Image.new("L", (640, 480))
    draw = ImageDraw.Draw(mask)
    visible = 0
    for laser in lasers:
        flags = laser["bodyVm.flagsWord"]
        if flags & 3 != 3 or laser["bodyVm.color1.d3dColor"] >> 24 == 0:
            continue
        sx, sy = (floating(word) for word in laser["bodyVm.scale"])
        width, height = (floating(word) for word in laser["bodyVm.spriteSize"])
        x, y, _ = (floating(word) for word in laser["bodyVm.pos"])
        angle = floating(laser["bodyVm.rotation"][2])
        half_x, half_y = abs(width * sx) / 2, abs(height * sy) / 2
        if not all(math.isfinite(v) for v in (x, y, angle, half_x, half_y)):
            raise ValueError("Laser geometry is not finite")
        if half_x <= 0 or half_y <= 0:
            continue
        anchor = (flags >> 11) & 3
        x += half_x if anchor & 1 else 0
        y += half_y if anchor & 2 else 0
        sine, cosine = math.sin(angle), math.cos(angle)
        polygon = [(dx * cosine - dy * sine + x, dx * sine + dy * cosine + y)
                   for dx, dy in ((-half_x, -half_y), (half_x, -half_y),
                                  (half_x, half_y), (-half_x, half_y))]
        draw.polygon(polygon, fill=255)
        visible += 1
    bounds = Image.new("L", (640, 480))
    ImageDraw.Draw(bounds).rectangle((32, 16, 415, 463), fill=255)
    return ImageChops.multiply(mask, bounds), visible


def compare(reference, candidate, maximum_error=1.0, normal_candidate=None):
    ref_meta = json.loads((reference / "metadata.json").read_text())
    got_meta = json.loads((candidate / "metadata.json").read_text())
    for left, right in (("fixture", "replaySha256"), ("gameDataSha256", "gameDataSha256"),
                        ("bgmDataSha256", "bgmDataSha256")):
        if ref_meta[left] != got_meta[right]:
            raise ValueError("Laser capture identities differ: " + left)
    if ref_meta.get("product") != "retail" or got_meta.get("product") != "portable-linux" or not ref_meta.get("muted") or not got_meta.get("muted"):
        raise ValueError("Expected muted retail and portable captures")
    if ref_meta.get("executableSha256") != TARGET_SHA256 or ref_meta.get("targetSha256") != TARGET_SHA256:
        raise ValueError("Reference is not the canonical Japanese TH08 1.00d target")
    for key, expected in (("rasterization", True), ("scout", False), ("keepLives", False), ("runnerExitCode", 0),
                          ("temporaryRuntimeRemoved", True), ("frameCounter", FRAME_COUNTER), ("playbackMode", 0)):
        if got_meta.get(key) != expected:
            raise ValueError("Invalid portable capture profile: " + key)
    if (ref_meta.get("playbackMode") != 0 or not ref_meta.get("rasterization") or ref_meta.get("clockRate") != 1
            or ref_meta.get("configSha256") != got_meta.get("configSha256")
            or ref_meta.get("startStage") != got_meta.get("startStage")):
        raise ValueError("Retail and portable playback profiles differ")
    retail_capture = ref_meta.get("laserCapture", {})
    if (retail_capture.get("frameCounter") != FRAME_COUNTER or not retail_capture.get("gameplayMemoryReadOnly")
            or not retail_capture.get("executableBytesUnmodified")):
        raise ValueError("Retail laser observer provenance is missing")
    expected = [(got_meta["stage"], frame) for frame in got_meta["frames"]]
    if not expected or len(set(expected)) != len(expected) or sorted(expected) != expected:
        raise ValueError("Laser capture requires nonempty ordered unique frame coverage")
    complete = json.loads((reference / "complete.json").read_text())
    if complete.get("errors") or not complete.get("diagnosticComplete") or complete.get("capturedFrames") != got_meta.get("frames"):
        raise ValueError("Retail laser capture did not complete")
    references = json.loads((reference / "lasers.json").read_text())
    candidates = json.loads((candidate / "lasers.json").read_text())
    if ([(x["stage"], x["frame"]) for x in references] != expected
            or [(x["stage"], x["frame"]) for x in candidates] != expected
            or retail_capture.get("frames") != got_meta["frames"]):
        raise ValueError("Laser capture frame coverage differs")
    frames = []
    for ref, got in zip(references, candidates):
        ref_slots = {item["slot"]: item for item in ref["lasers"]}
        got_slots = {item["slot"]: item for item in got["lasers"]}
        if len(ref_slots) != len(ref["lasers"]) or len(got_slots) != len(got["lasers"]):
            raise ValueError("Duplicate laser slots")
        differences = []
        if ref_slots.keys() != got_slots.keys():
            differences.append(dict(field="activeSlots", reference=sorted(ref_slots), candidate=sorted(got_slots)))
        for slot in ref_slots.keys() & got_slots.keys():
            for field in ref_slots[slot]:
                if ref_slots[slot][field] != got_slots[slot].get(field):
                    differences.append(dict(slot=slot, field=field, reference=ref_slots[slot][field],
                                            candidate=got_slots[slot].get(field)))
        mask, visible = geometry_mask(ref["lasers"])
        pixels = mask.histogram()[255]
        if pixels < 50 or visible == 0:
            raise ValueError("Reference frame has insufficient on-screen laser geometry")
        left_path = reference / f"frame-{ref['frame']}.png"
        right_path = candidate / f"frame-{got['frame']}.png"
        with Image.open(left_path) as left, Image.open(right_path) as right:
            if left.size != (640, 480) or right.size != left.size:
                raise ValueError("Expected complete 640x480 laser images")
            delta = ImageChops.difference(left.convert("RGB"), right.convert("RGB"))
            error = sum(ImageStat.Stat(delta, mask).mean) / 3
            full_error = sum(ImageStat.Stat(delta.crop((32, 16, 416, 464))).mean) / 3
        frames.append(dict(stage=ref["stage"], frame=ref["frame"], activeLasers=len(ref_slots),
                           drawableLasers=visible, laserGeometryPixels=pixels,
                           laserMeanAbsoluteRgbDifference=error,
                           playfieldMeanAbsoluteRgbDifference=full_error,
                           referenceImageSha256=digest(left_path), candidateImageSha256=digest(right_path),
                           stateDifferences=differences,
                           result="equal" if not differences and error <= maximum_error else "different"))
    result = dict(result="equal" if all(x["result"] == "equal" for x in frames) else "different",
                floatStateComparison="exact IEEE-754 bits", maximumLaserMeanRgbError=maximum_error,
                referenceExecutableSha256=ref_meta["executableSha256"], candidateExecutableSha256=got_meta["executableSha256"],
                replaySha256=got_meta["replaySha256"], comparatorSha256=digest(Path(__file__)),
                referenceLaserStateSha256=digest(reference / "lasers.json"), candidateLaserStateSha256=digest(candidate / "lasers.json"),
                frames=frames)
    if got_meta.get("suppressLaserBodies"):
        control = json.loads((candidate / "control.json").read_text())
        if control.get("active") or control.get("skippedQuads", 0) < 1:
            raise ValueError("Missing-laser control did not suppress completed beam submissions")
        if normal_candidate is None:
            raise ValueError("Missing-laser control requires --normal-candidate evidence")
        normal_meta = json.loads((normal_candidate / "metadata.json").read_text())
        if normal_meta.get("suppressLaserBodies") or normal_meta.get("executableSha256") != got_meta["executableSha256"]:
            raise ValueError("Control needs an unmodified capture of the same executable")
        if normal_meta.get("fastPortableClock") != got_meta.get("fastPortableClock"):
            raise ValueError("Normal and control portable pacing differs")
        baseline = compare(reference, normal_candidate, maximum_error)
        if baseline["candidateLaserStateSha256"] != result["candidateLaserStateSha256"]:
            raise ValueError("Normal and control laser state differs")
        result["negativeControl"] = dict(skippedQuads=control["skippedQuads"],
            minimumErrorIncrease=10.0, minimumErrorRatio=5.0,
            normalResult=baseline["result"], normalFrames=baseline["frames"],
            detected=all(not frame["stateDifferences"] and not normal["stateDifferences"]
                         and frame["laserMeanAbsoluteRgbDifference"] >= normal["laserMeanAbsoluteRgbDifference"] + 10.0
                         and frame["laserMeanAbsoluteRgbDifference"] >= 5.0 * normal["laserMeanAbsoluteRgbDifference"]
                         for frame, normal in zip(frames, baseline["frames"])))
        result["result"] = "different"
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--normal-candidate", type=Path, help="Unmodified portable capture for the missing-beam control")
    parser.add_argument("--expect-missing", action="store_true", help="Require the missing-beam control to be detected")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = compare(args.reference, args.candidate, normal_candidate=args.normal_candidate)
    content = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(content)
    print(content, end="")
    if args.expect_missing:
        return 0 if result.get("negativeControl", {}).get("detected") else 1
    return 0 if result["result"] == "equal" else 1


if __name__ == "__main__":
    raise SystemExit(main())
