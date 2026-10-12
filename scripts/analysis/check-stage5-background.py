#!/usr/bin/env python3
"""Inspect Stage 5 frame 6166 using the credited lunatic-0-b replay."""
from __future__ import annotations

import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from replay_file import verify_fixture


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def compress(path):
    destination = path.with_suffix(path.suffix + ".gz")
    with path.open("rb") as source, gzip.open(destination, "wb", compresslevel=9) as output:
        shutil.copyfileobj(source, output)
    with gzip.open(destination, "rb") as stream:
        if hashlib.sha256(stream.read()).hexdigest() != digest(path):
            raise RuntimeError("Compressed evidence differs: " + str(path))
    path.unlink()


def capture_portable(binary, data, replay, output, fast):
    output.mkdir()
    runtime = Path(tempfile.mkdtemp(prefix="background-capture.", dir=ROOT / "build"))
    metadata = dict(executableSha256=digest(binary), replaySha256=digest(replay),
                    gameDataSha256=digest(data / "th08.dat"), bgmDataSha256=digest(data / "thbgm.dat"),
                    stage=5, activeFrame=6166, muted=True, fastReplay=fast,
                    keepLives=False, cosmeticEnemyNameCopySuppressed=True)
    try:
        for archive in ("th08.dat", "thbgm.dat"):
            (runtime / archive).symlink_to((data / archive).resolve())
        (runtime / "replay").mkdir()
        shutil.copyfile(replay, runtime / "replay/audit.rpy")
        environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                           GALLIUM_DRIVER="llvmpipe", TH08_AUTOPLAY_REPLAY="./replay/audit.rpy",
                           TH08_AUTOPLAY_STAGE="5", TH08_RENDER_AUDIT="1",
                           TH08_RENDER_AUDIT_FAST="1" if fast else "0",
                           TH08_BACKGROUND_CAPTURE_DIR=str(output))
        command = ["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", "timeout",
                   "--signal=TERM", "--kill-after=5s", "210s", "gdb", "--batch", "--quiet",
                   "-x", str(ROOT / "scripts/gdb/stage5-background.gdb"),
                   "--args", str(binary), "--data-dir", str(runtime)]
        print("Capturing " + output.name + " Stage 5 frame 6166.", flush=True)
        with (output / "capture.log").open("w") as log:
            result = subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
        metadata["runnerExitCode"] = result.returncode
        if result.returncode or not (output / "camera.json").is_file() or not (output / "stage5-frame-6166.png").is_file():
            raise RuntimeError("Frame capture failed; inspect " + str(output / "capture.log.gz"))
    finally:
        for name in ("modern-enemy-render.csv", "modern-files.txt", "modern-crash.txt"):
            path = runtime / name
            if path.exists():
                shutil.copyfile(path, output / name)
                compress(output / name)
        shutil.rmtree(runtime)
        metadata["temporaryRuntimeRemoved"] = True
        (output / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
        log = output / "capture.log"
        if log.exists():
            compress(log)


def capture_retail(target, data, replay, output):
    from pe_image import PEImage
    image = PEImage(target)
    settings = dict(postPresentAddress=0x442082, cameraExitAddress=0x40B7E7,
                    background=0x4E4030, supervisor=0x17CE758)
    for name in ("postPresent", "cameraExit"):
        address = settings[name + "Address"]
        settings[name + "Bytes"] = image.read_rva(address - image.image_base, 13).hex()
    (output.parent / "native-capture-settings.json").write_text(json.dumps({"retail": settings}, indent=2) + "\n")
    environment = dict(os.environ, TH08_STAGE5_REPOSITORY=str(ROOT),
                       TH08_STAGE5_NATIVE_OBSERVER=str(ROOT / "scripts/gdb/stage5-background-native.py"),
                       LIBGL_ALWAYS_SOFTWARE="1", GALLIUM_DRIVER="llvmpipe")
    prefix = output.parent / "native-wine"
    command = [sys.executable, str(Path(__file__).with_name("stage5-background-native-runner.py")),
               "--target", str(target), "--game-data", str((data / "th08.dat").resolve()),
               "--bgm-data", str((data / "thbgm.dat").resolve()), "--replay", str(replay),
               "--start-stage", "5", "--playback-mode", "0", "--timeout", "240",
               "--wine-prefix", str(prefix), "--output-dir", str(output)]
    print("Capturing retail Stage 5 frame 6166.", flush=True)
    try:
        with (output.parent / "retail-capture.log").open("w") as log:
            result = subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
        if result.returncode or not (output / "camera.json").is_file():
            raise RuntimeError("Retail capture failed; inspect its retained log.")
    finally:
        if prefix.exists():
            if not (prefix / ".th08-replay-owned").is_file():
                raise RuntimeError("Temporary Wine prefix has no ownership marker.")
            cleanup_env = dict(environment, WINEPREFIX=str(prefix), WINEARCH="win32")
            subprocess.run(["wineserver", "-k"], env=cleanup_env, timeout=15, check=False)
            subprocess.run(["wineserver", "-w"], env=cleanup_env, timeout=15, check=False)
            shutil.rmtree(prefix)
        for folder in (output.parent, output):
            if folder.exists():
                for path in list(folder.glob("*.log")) + list(folder.glob("*.jsonl")):
                    compress(path)


def build_clip_w_probe(output):
    """Build a private renderer variant while retaining repository source."""
    source_dir, build_dir = output / "probe-source", output / "probe-build"
    source_dir.mkdir()
    original = ROOT / "src/modern/linux/d3d8_compat.cpp"
    source = original.read_text()
    replacements = (
        ("float *zOut, float *fogCoordinateOut)", "float *zOut, float *wOut, float *fogCoordinateOut)"),
        ("*fogCoordinateOut = 0.0f;\n            return;", "*fogCoordinateOut = 0.0f;\n            *wOut = 1.0f;\n            return;"),
        ("        float reciprocal = fabsf(vector[3]) > 1.0e-8f ? 1.0f / vector[3] : 1.0f;\n"
         "        *xOut = viewport.X + (vector[0] * reciprocal + 1.0f) * viewport.Width * 0.5f;\n"
         "        *yOut = viewport.Y + (1.0f - vector[1] * reciprocal) * viewport.Height * 0.5f;\n"
         "        *zOut = viewport.MinZ + vector[2] * reciprocal * (viewport.MaxZ - viewport.MinZ);",
         "        *wOut = vector[3];\n"
         "        *xOut = viewport.X * vector[3] + (vector[0] + vector[3]) * viewport.Width * 0.5f;\n"
         "        *yOut = viewport.Y * vector[3] + (vector[3] - vector[1]) * viewport.Height * 0.5f;\n"
         "        *zOut = viewport.MinZ * vector[3] + vector[2] * (viewport.MaxZ - viewport.MinZ);"),
        ("float x, y, z, fogCoordinate;", "float x, y, z, w, fogCoordinate;"),
        ("&fogCoordinate);", "&w, &fogCoordinate);"),
        ("glVertex3f(x, y, 1.0f - 2.0f * z);", "glVertex4f(x, y, w - 2.0f * z, w);"),
    )
    for before, after in replacements:
        if source.count(before) != 1:
            raise RuntimeError("Clip-W experiment source pattern differs: " + before.splitlines()[0])
        source = source.replace(before, after)
    corrected = source_dir / "corrected-d3d8.cpp"
    corrected.write_text(source)
    for name in ("src", "scripts", "config", "resources"):
        (source_dir / name).symlink_to(ROOT / name, target_is_directory=True)
    cmake = (ROOT / "CMakeLists.txt").read_text() + """
# Private clip-W experiment; game translation units use the current source.
set_source_files_properties(src/modern/linux/d3d8_compat.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(th08-modern PRIVATE corrected-d3d8.cpp)
set_source_files_properties(corrected-d3d8.cpp PROPERTIES COMPILE_OPTIONS "-O2;-fno-strict-aliasing")
target_include_directories(th08-modern PRIVATE "${CMAKE_SOURCE_DIR}/src/modern/linux")
"""
    (source_dir / "CMakeLists.txt").write_text(cmake)
    metadata = dict(originalBackendSha256=digest(original), counterfactualBackendSha256=digest(corrected),
                    change="Preserve homogeneous W for XYZ vertices; keep existing XYZRHW behavior",
                    productionSourceUnchanged=True)
    (output / "probe-source-metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    with (output / "probe-build.log").open("w") as log:
        subprocess.run(["cmake", "-S", str(source_dir), "-B", str(build_dir), "-G", "Ninja",
                        "-DCMAKE_BUILD_TYPE=Debug"], stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run(["cmake", "--build", str(build_dir), "--parallel", "1"],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run([str(ROOT / "scripts/verify-portable-linux.sh"), str(build_dir / "th08-modern"), "x86_64"],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    return build_dir / "th08-modern"


def summarize(output):
    from PIL import Image, ImageChops, ImageStat
    report = dict(stage=5, activeFrame=6166, cameras={}, pixelComparisons=[], clipW=[])
    names = [name for name in ("retail", "portable-fast", "portable-paced", "clip-w-probe")
             if (output / name / "camera.json").exists()]
    cameras = {name: json.loads((output / name / "camera.json").read_text())[0] for name in names}
    reference = "retail" if "retail" in names else names[0]
    for name in names:
        report["cameras"][name] = {key: max(abs(a-b) for a, b in zip(cameras[reference][key], cameras[name][key]))
                                   for key in ("cameraCurrent", "stagePosition", "view", "projection", "viewport")}
        report["cameras"][name]["scriptStateEqual"] = all(cameras[reference][key] == cameras[name][key]
            for key in ("stageScriptTimerHex", "cameraInterpolationTimersHex",
                        "stageScriptInstructionIndex", "frameCounter", "cameraMotionMode"))
        with Image.open(output / reference / "stage5-frame-6166.png") as a, Image.open(output / name / "stage5-frame-6166.png") as b:
            if a.size != (640,480) or b.size != (640,480):
                raise RuntimeError("Expected complete 640x480 images for the pixel comparison.")
            delta = ImageChops.difference(a.convert("RGB"), b.convert("RGB")).crop((32,16,416,464))
            report["pixelComparisons"].append(dict(reference=reference, candidate=name,
                playfieldChangedPixels=sum(pixel != (0,0,0) for pixel in delta.getdata()),
                playfieldMeanAbsoluteRgbDifference=sum(ImageStat.Stat(delta).mean)/3.0))
    for name in names:
        path = output / name / "draws.json"
        if not path.exists():
            continue
        for index, draw in enumerate(json.loads(path.read_text())):
            values=[]
            for vertex in draw["vertices"]:
                vector=vertex["position"]+[1.0]
                for key in ("world", "view", "projection"):
                    matrix=draw[key]
                    vector=[sum(vector[row]*matrix[row*4+column] for row in range(4)) for column in range(4)]
                values.append(vector[3])
            report["clipW"].append(dict(capture=name, draw=index, fvf=draw["fvf"], computedClipW=values))
    (output / "summary.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report["pixelComparisons"], indent=2), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--portable", type=Path, required=True, help="Native-layout portable Debug ELF with GDB symbols")
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--profiles", choices=("both", "fast", "paced"), default="both")
    parser.add_argument("--retail", action="store_true", help="Also capture the hash-attested retail image using Wine")
    parser.add_argument("--clip-w-probe", action="store_true", help="Build and capture an isolated XYZ clip-W experiment")
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    args = parser.parse_args()
    binary, data, output, target = (path.resolve() for path in (args.portable, args.game_data_dir, args.output_dir, args.target))
    if not output.is_relative_to(ROOT / "build") or output.exists():
        parser.error("--output-dir must be a fresh directory under this repository's build/.")
    if not binary.is_file():
        parser.error("Portable Debug executable is missing.")
    for archive in ("th08.dat", "thbgm.dat"):
        if not (data / archive).is_file():
            parser.error("Original data archive is missing: " + archive)
    if args.retail and (not target.is_file() or target.stat().st_size != 840704 or digest(target) != "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"):
        parser.error("Retail input must be the canonical Japanese TH08 1.00d executable.")
    fixture = next(item for item in json.loads((ROOT / "config/replay-fixtures.json").read_text())["fixtures"] if item["id"] == "lunatic-0-b")
    replay = ROOT / "tests/fixtures/replays/th08-1.00d/accepted" / fixture["file"]
    verify_fixture(replay, fixture)
    output.mkdir(parents=True)
    manifest = dict(fixture=fixture["id"], replaySha256=fixture["sha256"], stage=5, activeFrame=6166,
                    sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                    portableSha256=digest(binary), profiles=args.profiles, retail=args.retail,
                    clipWProbe=args.clip_w_probe,
                    observerScope="Bounded camera, 3D draw, and rendered-frame inspection")
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    for name, fast in (("portable-fast", True), ("portable-paced", False)):
        if args.profiles == "both" or args.profiles == ("fast" if fast else "paced"):
            capture_portable(binary, data, replay, output / name, fast)
    if args.retail:
        capture_retail(target, data, replay, output / "retail")
    if args.clip_w_probe:
        probe = build_clip_w_probe(output)
        try:
            capture_portable(probe, data, replay, output / "clip-w-probe", True)
        finally:
            if probe.is_file():
                shutil.copy2(probe, output / "clip-w-probe.elf")
                if digest(probe) != digest(output / "clip-w-probe.elf"):
                    raise RuntimeError("Retained probe executable differs from its tested build.")
            shutil.rmtree(output / "probe-build")
            compress(output / "probe-build.log")
    summarize(output)
    print("Stage 5 background evidence retained in " + str(output), flush=True)


if __name__ == "__main__":
    main()
