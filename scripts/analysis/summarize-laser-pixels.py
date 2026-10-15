#!/usr/bin/env python3
"""Measure laser-mask and optional full-playfield differences across the title band."""
import argparse
import heapq
import importlib.util
import json
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("laser_comparison", Path(__file__).with_name("compare-laser-visuals.py"))
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)


def partition(reference, candidate, lasers, top_pixels=0, full_playfield=False):
    if full_playfield:
        mask = Image.new("L", (640, 480))
        mask.paste(255, (32, 16, 416, 464))
    else:
        mask, _ = comparison.geometry_mask(lasers)
    regions = {name: dict(pixels=0, absoluteRgbSum=0, differentPixels=0,
                          pixelsAboveMean32=0, maximumChannelError=0)
               for name in ("header", "outsideHeader")}
    examples = {name: [] for name in regions}
    for index, (expected, actual, covered) in enumerate(zip(reference.getdata(), candidate.getdata(), mask.getdata())):
        if not covered:
            continue
        y, x = divmod(index, 640)
        name = "header" if 32 <= x < 416 and 16 <= y < 64 else "outsideHeader"
        region = regions[name]
        errors = [abs(a - b) for a, b in zip(expected, actual)]
        total = sum(errors)
        region["pixels"] += 1
        region["absoluteRgbSum"] += total
        region["differentPixels"] += total != 0
        region["pixelsAboveMean32"] += total > 3 * 32
        region["maximumChannelError"] = max(region["maximumChannelError"], *errors)
        if top_pixels and total:
            # Prefer earlier coordinates when errors tie.
            item = (max(errors), total, -index, expected, actual)
            heapq.heappush(examples[name], item)
            if len(examples[name]) > top_pixels:
                heapq.heappop(examples[name])
    for name, region in regions.items():
        region["meanRgbError"] = (region["absoluteRgbSum"] / (3 * region["pixels"])
                                  if region["pixels"] else None)
        if top_pixels:
            region["largestDifferences"] = [
                dict(x=(-negative_index) % 640, y=(-negative_index) // 640, referenceRgb=list(expected),
                     candidateRgb=list(actual), maximumChannelError=maximum,
                     absoluteRgbSum=total)
                for maximum, total, negative_index, expected, actual in sorted(examples[name], reverse=True)]
    return regions


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True, help="Completed retail laser capture")
    parser.add_argument("--candidate", type=Path, required=True, help="Completed reconstructed laser capture")
    parser.add_argument("--output", type=Path, required=True, help="Fresh JSON file under build/")
    parser.add_argument("--top-pixels", type=int, default=0,
                        help="Retain up to this many largest differences per region (0 through 32)")
    parser.add_argument("--full-playfield", action="store_true",
                        help="Also measure the complete playfield, including pixels outside laser geometry")
    args = parser.parse_args()
    if not 0 <= args.top_pixels <= 32:
        parser.error("Top pixel count must be between 0 and 32")
    output = args.output.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh file under build/")
    strict = comparison.compare(args.reference, args.candidate)
    records = json.loads((args.reference / "lasers.json").read_text())
    report = dict(strictComparison=strict, headerRectangle=[32, 16, 416, 64],
                  geometry="Retail laser geometry clipped to the playfield; includes other layers drawn there",
                  reference=str(args.reference.resolve()), candidate=str(args.candidate.resolve()),
                  maximumExamplesPerRegion=args.top_pixels,
                  fullPlayfieldRectangle=[32, 16, 416, 464] if args.full_playfield else None,
                  analyzerSha256=comparison.digest(Path(__file__)), frames=[])
    for record, paired in zip(records, strict["frames"]):
        filename = f"frame-{record['frame']}.png"
        with Image.open(args.reference / filename) as expected, Image.open(args.candidate / filename) as actual:
            expected, actual = expected.convert("RGB"), actual.convert("RGB")
            regions = partition(expected, actual, record["lasers"], args.top_pixels)
            if args.full_playfield:
                playfield_regions = partition(expected, actual, record["lasers"], args.top_pixels, True)
        pixels = sum(region["pixels"] for region in regions.values())
        total = sum(region["absoluteRgbSum"] for region in regions.values())
        if pixels != paired["laserGeometryPixels"]:
            raise ValueError("Pixel regions do not partition the strict laser mask")
        if pixels and abs(total / (3 * pixels) - paired["laserMeanAbsoluteRgbDifference"]) > 1e-12:
            raise ValueError("Region errors do not reproduce the strict image comparison")
        report["frames"].append(dict(stage=record["stage"], frame=record["frame"], regions=regions))
        if args.full_playfield:
            count = sum(region["pixels"] for region in playfield_regions.values())
            error = sum(region["absoluteRgbSum"] for region in playfield_regions.values())
            if count != 384 * 448 or abs(error / (3 * count) - paired["playfieldMeanAbsoluteRgbDifference"]) > 1e-12:
                raise ValueError("Full playfield regions do not reproduce the strict playfield measurement")
            report["frames"][-1]["fullPlayfieldRegions"] = playfield_regions
            remaining = playfield_regions["outsideHeader"]
            print(f"Stage index {record['stage']}, frame {record['frame']}: "
                  f"{remaining['differentPixels']} changed full-playfield pixels below the title band; "
                  f"maximum channel error {remaining['maximumChannelError']}/255")
        outside = regions["outsideHeader"]
        print(f"Stage index {record['stage']}, frame {record['frame']}: "
              f"{outside['differentPixels']} changed laser-mask pixels outside the title band; "
              f"maximum channel error {outside['maximumChannelError']}/255")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
