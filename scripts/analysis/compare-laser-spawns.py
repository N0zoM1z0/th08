#!/usr/bin/env python3
"""Compare bounded, read-only retail and portable laser spawn inputs."""
import argparse
import hashlib
import json
from pathlib import Path

TARGET_SHA256 = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"


def load(directory):
    metadata = json.loads((directory / "metadata.json").read_text())
    path = directory / "laser-spawns.json"
    capture = json.loads(path.read_text())
    if (capture.get("boundary") != "SpawnLaserPattern entry"
            or capture.get("frameCounter") != "GameManager.gameplayFrameCounter"
            or capture.get("complete") is not True):
        raise ValueError("Laser spawn observation is incomplete or has a different boundary")
    records = capture["records"]
    if not records or len(records) > 4096:
        raise ValueError("Expected 1 through 4096 laser spawn records")
    start, end = capture["range"]
    if not 1 <= start <= end:
        raise ValueError("Invalid laser spawn range")
    for row in records:
        if not start <= row["frame"] <= end or row["enemySlot"] is None:
            raise ValueError("Spawn record is outside the selected range or typed enemy pool")
        if not 0 <= row["enemySlot"] < 481:
            raise ValueError("Spawn enemy slot is outside the typed pool")
        for name, length in (("descriptor.position", 3), ("enemy.position", 3),
                             ("enemy.worldPosition", 3), ("enemy.shootOffset", 3), ("ecl.floatVariables", 8)):
            words = row.get(name)
            if not isinstance(words, list) or len(words) != length or any(
                    type(word) is not int or not 0 <= word <= 0xFFFFFFFF for word in words):
                raise ValueError("Invalid or missing raw float words: " + name)
        required = {"stage", "firstFreeSlot", "descriptor.angle", "descriptor.speed1", "descriptor.bulletType",
                    "descriptor.color", "descriptor.laserDuration", "ecl.subId", "ecl.time", "ecl.instruction", "caller"}
        if not required <= row.keys():
            raise ValueError("Spawn record lacks required fields")
        if row["stage"] != metadata.get("startStage"):
            raise ValueError("Spawn record has a different stage")
    if any(a["frame"] > b["frame"] for a, b in zip(records, records[1:])):
        raise ValueError("Laser spawn frame counter went backwards")
    if metadata.get("muted") is not True:
        raise ValueError("Expected muted replay captures")
    return metadata, capture, hashlib.sha256(path.read_bytes()).hexdigest()


def compare(reference, candidate):
    ref_meta, ref, ref_sha = load(reference)
    got_meta, got, got_sha = load(candidate)
    if ref_meta.get("product") != "retail" or ref_meta.get("executableSha256") != TARGET_SHA256:
        raise ValueError("Reference must be the canonical Japanese TH08 1.00d executable")
    if got_meta.get("product") != "portable-linux" or got_meta.get("runnerExitCode") != 0:
        raise ValueError("Candidate must be a completed portable capture")
    provenance = ref_meta.get("laserCapture", {})
    complete = json.loads((reference / "complete.json").read_text())
    if (not provenance.get("gameplayMemoryReadOnly") or not provenance.get("executableBytesUnmodified")
            or complete.get("errors") or not complete.get("diagnosticComplete")):
        raise ValueError("Retail spawn capture lacks completed read-only evidence")
    for left, right in (("fixture", "replaySha256"), ("gameDataSha256", "gameDataSha256"),
                        ("bgmDataSha256", "bgmDataSha256"), ("configSha256", "configSha256"),
                        ("startStage", "startStage")):
        if ref_meta.get(left) != got_meta.get(right):
            raise ValueError("Spawn capture identities differ: " + left)
    if (ref["range"] != got["range"] or provenance.get("watchLaserSpawns") != ref["range"]
            or got_meta.get("watchLaserSpawns") != got["range"]):
        raise ValueError("Spawn capture ranges differ from their provenance")
    differences = []
    if len(ref["records"]) != len(got["records"]):
        differences.append(dict(field="recordCount", reference=len(ref["records"]), candidate=len(got["records"])))
    for index, (left, right) in enumerate(zip(ref["records"], got["records"])):
        # Caller identity is platform-specific: retail return VA vs portable DWARF name.
        for key in sorted((set(left) | set(right)) - {"caller"}):
            if left.get(key) != right.get(key):
                differences.append(dict(record=index, frame=left["frame"], enemySlot=left["enemySlot"],
                                        firstFreeSlot=left["firstFreeSlot"], field=key,
                                        reference=left.get(key), candidate=right.get(key)))
    return dict(result="different" if differences else "equal", floatComparison="exact IEEE-754 bits",
                referenceExecutableSha256=TARGET_SHA256, candidateExecutableSha256=got_meta["executableSha256"],
                referenceObserverSha256=provenance.get("observerSha256"),
                candidateObserverSha256=got_meta.get("observerSha256"),
                comparatorSha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                replaySha256=got_meta["replaySha256"], range=ref["range"],
                referenceRecordCount=len(ref["records"]), candidateRecordCount=len(got["records"]),
                referenceSpawnSha256=ref_sha, candidateSpawnSha256=got_sha, differences=differences)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = compare(args.reference, args.candidate)
    encoded = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(encoded)
    print(encoded, end="")
    raise SystemExit(report["result"] != "equal")


if __name__ == "__main__":
    main()
