#!/usr/bin/env python3
"""Partition paired laser-image error without changing the strict comparison gate."""
import argparse
import importlib.util
import json
from pathlib import Path

from PIL import Image, ImageDraw

spec = importlib.util.spec_from_file_location("laser_comparison", Path(__file__).with_name("compare-laser-visuals.py"))
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)


def partition(reference, normal, control, record, support_threshold=8):
    """Assign every beam-quad pixel once: header, cap, influenced beam, remainder."""
    geometry, _ = comparison.geometry_mask(record["lasers"])
    cap_lasers = []
    for laser in record["lasers"]:
        if comparison.floating(laser["speed"]) != 0 and comparison.floating(laser["startOffset"]) >= 16:
            continue
        if laser["hideCapDuringStartup"] and laser["state"] == 0:
            continue
        cap_lasers.append({"bodyVm." + key.removeprefix("startCapVm."): value
                           for key, value in laser.items() if key.startswith("startCapVm.")})
    caps, _ = comparison.geometry_mask(cap_lasers)
    header = Image.new("L", (640, 480))
    ImageDraw.Draw(header).rectangle((32, 16, 415, 63), fill=255)
    stats = {name: dict(pixels=0, absoluteRgbSum=0, signedRgbSum=[0, 0, 0],
                       pixelsAbove32=0, maximumChannelError=0)
             for name in ("header", "caps", "influencedBeam", "remainder")}
    for ref, got, suppressed, quad, cap, hud in zip(reference.getdata(), normal.getdata(), control.getdata(),
                                                  geometry.getdata(), caps.getdata(), header.getdata()):
        if not quad:
            continue
        influenced = max(abs(a - b) for a, b in zip(got, suppressed)) >= support_threshold
        name = "header" if hud else "caps" if cap else "influencedBeam" if influenced else "remainder"
        entry = stats[name]
        delta = [a - b for a, b in zip(got, ref)]
        magnitude = sum(abs(value) for value in delta)
        entry["pixels"] += 1
        entry["absoluteRgbSum"] += magnitude
        for channel, value in enumerate(delta):
            entry["signedRgbSum"][channel] += value
        entry["pixelsAbove32"] += magnitude > 32 * 3
        entry["maximumChannelError"] = max(entry["maximumChannelError"], max(abs(value) for value in delta))
    total = sum(entry["absoluteRgbSum"] for entry in stats.values())
    for entry in stats.values():
        count = entry["pixels"]
        entry["meanAbsoluteRgbError"] = entry["absoluteRgbSum"] / (3 * count) if count else None
        entry["errorSharePercent"] = 100 * entry["absoluteRgbSum"] / total if total else 0
        entry["meanSignedRgbError"] = [value / count for value in entry["signedRgbSum"]] if count else None
    return stats


def analyze(reference, normal, control):
    baseline = comparison.compare(reference, normal)
    omitted = comparison.compare(reference, control, normal_candidate=normal)
    if not omitted["negativeControl"]["detected"]:
        raise ValueError("Error localization requires a detected missing-beam control")
    records = json.loads((reference / "lasers.json").read_text())
    result = dict(strictResult=baseline["result"], strictMaximumMeanRgbError=1.0,
                  supportMaximumChannelThreshold=8, headerRectangle=[32, 16, 416, 64],
                  partitionOrder=["header", "caps", "influencedBeam", "remainder"],
                  supportMeaning="Pixels changed by portable beam omission; this is not a retail layer decomposition",
                  normalCapture=str(normal), referenceCapture=str(reference), controlCapture=str(control),
                  analyzerSha256=comparison.digest(Path(__file__)), frames=[])
    for record, original in zip(records, baseline["frames"]):
        filename = f"frame-{record['frame']}.png"
        with Image.open(reference / filename) as ref, Image.open(normal / filename) as got, Image.open(control / filename) as removed:
            regions = partition(ref.convert("RGB"), got.convert("RGB"), removed.convert("RGB"), record)
        if sum(region["pixels"] for region in regions.values()) != original["laserGeometryPixels"]:
            raise ValueError("Error regions do not cover the original beam mask")
        result["frames"].append(dict(stage=record["stage"], frame=record["frame"],
                                     originalComparison=original, regions=regions))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--normal", type=Path, required=True)
    parser.add_argument("--control", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = analyze(args.reference, args.normal, args.control)
    args.output.write_text(json.dumps(result, indent=2) + "\n")
    for frame in result["frames"]:
        parts = ", ".join(f"{name}: {region['errorSharePercent']:.1f}%" for name, region in frame["regions"].items())
        print(f"Stage index {frame['stage']}, frame {frame['frame']}: {parts}")


if __name__ == "__main__":
    main()
