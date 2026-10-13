#!/usr/bin/env python3
"""Compare complete portable draw inputs from matching presented replay frames."""
import argparse
import hashlib
import json
from pathlib import Path


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def borrow_transparent_rgb(current, reference):
    """A diagnostic substitution: preserve every input except zero-alpha RGB."""
    if current["fvf"] != 0x144 or current["stride"] != 28:
        raise ValueError("Color control requires 28-byte XYZRHW/diffuse/TEX1 vertices")
    if any(current[key] != reference.get(key) for key in current if key != "vertices"):
        raise ValueError("Color control requires identical nonvertex inputs")
    a, b = current["vertices"], reference["vertices"]
    if len(a) != len(b):
        raise ValueError("Color control vertex counts differ")
    changed = []
    for index, (left, right) in enumerate(zip(a, b)):
        if left == right:
            continue
        if index % 7 != 4 or left >> 24 or right >> 24:
            raise ValueError("Color control may replace only zero-alpha RGB words")
        changed.append(index)
    return list(b), changed


def load(folder):
    metadata = json.loads((folder / "metadata.json").read_text())
    if (metadata.get("product") != "portable-linux" or not metadata.get("watchDraws")
            or metadata.get("runnerExitCode") != 0 or not metadata.get("temporaryRuntimeRemoved")
            or not metadata.get("rasterization") or metadata.get("scout")
            or metadata.get("suppressLaserBodies") or not metadata.get("muted")):
        raise ValueError("Expected a completed, rasterized portable draw capture")
    if metadata.get("renderInputControl"):
        raise ValueError("Experimental render-input controls are separate from ordinary draw captures")
    if (metadata.get("frameCounter") != "GameManager.gameplayFrameCounter after Present"
            or metadata.get("playbackMode") != 0 or metadata.get("startStage") != metadata.get("stage")
            or metadata.get("keepLives") is not False):
        raise ValueError("Invalid draw playback profile")
    for key in ("executableSha256", "replaySha256", "gameDataSha256", "bgmDataSha256", "configSha256"):
        value = metadata.get(key, "")
        if not isinstance(value, str) or len(value) != 64 or any(c not in "0123456789abcdef" for c in value):
            raise ValueError("Missing draw capture identity: " + key)
    rows = json.loads((folder / "draws.json").read_text())
    expected = metadata["frames"]
    if not expected or expected != sorted(set(expected)) or [row["frame"] for row in rows] != expected:
        raise ValueError("Draw frame coverage differs from metadata")
    for row in rows:
        if row["stage"] != metadata["stage"] or not 0 < len(row["draws"]) <= 10000:
            raise ValueError("Draw stage or count is invalid")
        for draw in row["draws"]:
            data = draw["inputs"]
            kind, count, stride = (data[name] for name in ("primitiveType", "primitiveCount", "stride"))
            vertices = count if kind == 1 else count * 2 if kind == 2 else count + 1 if kind == 3 else count * 3 if kind == 4 else count + 2
            if kind not in range(1, 7) or not 0 < vertices <= 30000 or not 0 < stride <= 256 or stride % 4:
                raise ValueError("Invalid draw primitive layout")
            lengths = dict(vertices=vertices * stride // 4, world=16, view=16, projection=16,
                           textureTransform=16, viewport=6, renderStates=256, textureStates=32)
            for name, length in lengths.items():
                values = data[name]
                if len(values) != length or any(type(word) is not int or not 0 <= word <= 0xffffffff for word in values):
                    raise ValueError("Invalid draw word layout: " + name)
    return metadata, rows


def differences(a, b, path=""):
    if type(a) is not type(b):
        yield dict(field=path, reference=a, candidate=b)
    elif isinstance(a, dict):
        for key in sorted(a.keys() | b.keys()):
            field = path + "." + key if path else key
            if key not in a or key not in b:
                yield dict(field=field, reference=a.get(key), candidate=b.get(key))
            else:
                yield from differences(a[key], b[key], field)
    elif isinstance(a, list):
        if len(a) != len(b):
            yield dict(field=path + ".length", reference=len(a), candidate=len(b))
        for index, (left, right) in enumerate(zip(a, b)):
            yield from differences(left, right, f"{path}[{index}]")
    elif a != b:
        yield dict(field=path, reference=a, candidate=b)


def compare(reference, candidate):
    left, a = load(reference)
    right, b = load(candidate)
    for key in ("replaySha256", "gameDataSha256", "bgmDataSha256", "configSha256", "stage", "frames",
                "frameCounter", "playbackMode", "startStage", "fastPortableClock", "legacyOpenGL"):
        if left.get(key) != right.get(key):
            raise ValueError("Draw capture identities differ: " + key)
    report = dict(evidenceClass="Portable API input comparison; CPU FPU state is recorded separately",
                  referenceExecutableSha256=left["executableSha256"], candidateExecutableSha256=right["executableSha256"],
                  referenceDrawSha256=digest(reference / "draws.json"), candidateDrawSha256=digest(candidate / "draws.json"),
                  comparatorSha256=digest(Path(__file__)), frames=[])
    for x, y in zip(a, b):
        changes, count = [], 0
        for change in differences([d["inputs"] for d in x["draws"]], [d["inputs"] for d in y["draws"]]):
            count += 1
            if len(changes) < 1000:
                changes.append(change)
        report["frames"].append(dict(frame=x["frame"], referenceDraws=len(x["draws"]), candidateDraws=len(y["draws"]),
                                    differingValues=count, differences=changes,
                                    detailsTruncated=count > 1000,
                                    referenceFpu=sorted({(d["cpuFloatingPoint"]["x87"], d["cpuFloatingPoint"]["mxcsr"]) for d in x["draws"]}),
                                    candidateFpu=sorted({(d["cpuFloatingPoint"]["x87"], d["cpuFloatingPoint"]["mxcsr"]) for d in y["draws"]})))
    report["result"] = "different" if any(row["differingValues"] for row in report["frames"]) else "equal"
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    report = compare(args.reference, args.candidate)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    for row in report["frames"]:
        print(f"Frame {row['frame']}: {row['referenceDraws']}/{row['candidateDraws']} draws; {row['differingValues']} differing input values")
    return int(report["result"] != "equal")


if __name__ == "__main__":
    raise SystemExit(main())
