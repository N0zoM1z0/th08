#!/usr/bin/env python3
"""Compare captured sprite vertex words and localize differences in a screen region."""
import argparse
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
FIELDS = ("x", "y", "z", "rhw", "diffuse", "u", "v")


def floating(word):
    return struct.unpack("<f", struct.pack("<I", word))[0]


def load_quads(folder, frames):
    rows = json.loads((folder / "quads.json").read_text())
    if [row["frame"] for row in rows] != frames:
        raise ValueError("Submitted-quad frame coverage differs")
    for row in rows:
        if not 0 < len(row["quads"]) <= 10000:
            raise ValueError("Submitted-quad count is empty or exceeds the capture bound")
        for quad in row["quads"]:
            if len(quad) != 4 or any(len(vertex) != 7 for vertex in quad):
                raise ValueError("Submitted-quad layout differs from four 28-byte vertices")
            for vertex in quad:
                if any(type(word) is not int or not 0 <= word <= 0xffffffff for word in vertex):
                    raise ValueError("Submitted-quad word is not an unsigned 32-bit integer")
                if any(not math.isfinite(floating(word)) for index, word in enumerate(vertex) if index != 4):
                    raise ValueError("Submitted-quad float is not finite")
    return rows


def select_region(quads, roi, selection="thin-horizontal"):
    selected = []
    for index, quad in enumerate(quads):
        xs, ys = ([floating(vertex[axis]) for vertex in quad] for axis in (0, 1))
        bounds = [min(xs), min(ys), max(xs), max(ys)]
        if selection == "covering-bounds":
            matches = (bounds[0] <= roi[0] and bounds[1] <= roi[1]
                       and bounds[2] >= roi[2] and bounds[3] >= roi[3])
        elif selection == "intersecting-bounds":
            matches = (bounds[0] <= roi[2] and bounds[2] >= roi[0]
                       and bounds[1] <= roi[3] and bounds[3] >= roi[1])
        elif selection == "thin-horizontal":
            matches = (bounds[2] - bounds[0] > 30 and 0 < bounds[3] - bounds[1] <= 16
                       and bounds[0] < roi[2] and bounds[2] >= roi[0]
                       and bounds[1] < roi[3] and bounds[3] >= roi[1])
        else:
            raise ValueError("Unknown quad selection: " + selection)
        if matches:
            selected.append((index, quad, bounds))
    return selected


def differences(left, right):
    result = []
    for vertex, (a, b) in enumerate(zip(left, right)):
        for index, (x, y) in enumerate(zip(a, b)):
            if x != y:
                item = dict(vertex=vertex, field=FIELDS[index], referenceWord=f"{x:08x}", candidateWord=f"{y:08x}")
                if index != 4:
                    item.update(reference=floating(x), candidate=floating(y), delta=floating(y) - floating(x))
                result.append(item)
    return result


def compare(reference, candidate, roi, selection="thin-horizontal", frame=None):
    spec = importlib.util.spec_from_file_location("laser_visuals", ROOT / "scripts/analysis/compare-laser-visuals.py")
    visuals = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(visuals)
    image_report = visuals.compare(reference, candidate)
    if any(row["stateDifferences"] for row in image_report["frames"]):
        raise ValueError("Raw laser state differs; vertex localization requires the aligned scene")
    frames = [row["frame"] for row in image_report["frames"]]
    left, right = load_quads(reference, frames), load_quads(candidate, frames)
    if frame is not None and frame not in frames:
        raise ValueError("Requested frame is absent from the validated capture")
    report = dict(evidenceClass="Observed AddSpriteToDrawBuffer inputs; target bytes and gameplay memory unmodified",
                  vertexLayout=list(FIELDS), vertexBytes=28, roi=list(roi),
                  captureFrames=frames, selectedFrame=frame,
                  selector=("sprite quads whose bounds touch ROI in submission order; conservative spatial selection"
                            if selection == "intersecting-bounds" else
                            "sprite quads with bounds covering ROI in submission order" if selection == "covering-bounds"
                            else "thin horizontal sprite quads intersecting ROI in submission order"),
                  imageComparison=image_report, frames=[])
    for a, b, expected in zip(left, right, image_report["frames"]):
        if a["stage"] != b["stage"] or a["stage"] != expected["stage"]:
            raise ValueError("Submitted-quad stage differs")
        if frame is not None and a["frame"] != frame:
            continue
        selected_a, selected_b = select_region(a["quads"], roi, selection), select_region(b["quads"], roi, selection)
        if not selected_a or len(selected_a) != len(selected_b):
            raise ValueError(f"Region quad coverage cannot be paired in submission order: "
                             f"reference {len(selected_a)}, candidate {len(selected_b)}")
        pairs = []
        for x, y in zip(selected_a, selected_b):
            pairs.append(dict(referenceIndex=x[0], candidateIndex=y[0], referenceBounds=x[2], candidateBounds=y[2],
                              differences=differences(x[1], y[1])))
        report["frames"].append(dict(frame=a["frame"], referenceQuads=len(a["quads"]),
                                     candidateQuads=len(b["quads"]), selectedQuads=len(pairs), pairs=pairs,
                                     regionVertexWordsEqual=all(not pair["differences"] for pair in pairs)))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--roi", default="201,236,248,245", help="x0,y0,x1,y1; default: Stage 3 horizontal-edge region")
    parser.add_argument("--selection", choices=("thin-horizontal", "covering-bounds", "intersecting-bounds"), default="thin-horizontal",
                        help="Select thin edges, bounds covering the ROI, or conservative bounds touching it")
    parser.add_argument("--frame", type=int, help="Localize one frame after validating the complete paired capture")
    parser.add_argument("--check", action="store_true", help="Fail if any selected vertex words differ")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    roi = tuple(int(value) for value in args.roi.split(","))
    if len(roi) != 4 or not (0 <= roi[0] < roi[2] <= 640 and 0 <= roi[1] < roi[3] <= 480):
        parser.error("ROI must be a nonempty rectangle inside the 640x480 frame")
    report = compare(args.reference, args.candidate, roi, args.selection, args.frame)
    report["comparatorSha256"] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    for row in report["frames"]:
        count = sum(len(pair["differences"]) for pair in row["pairs"])
        print(f"Frame {row['frame']}: {row['selectedQuads']} region quads, {count} differing vertex words")

    if args.check and any(not row["regionVertexWordsEqual"] for row in report["frames"]):
        raise SystemExit("Selected sprite vertex words differ")


if __name__ == "__main__":
    main()
