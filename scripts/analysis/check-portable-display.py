#!/usr/bin/env python3
"""Inspect portable window geometry and HOME screenshots using a muted replay."""
from __future__ import annotations

import argparse
import importlib.util
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


def load_script(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


background = load_script("background_capture", Path(__file__).with_name("check-stage5-background.py"))
native_capture = load_script("native_capture", ROOT / "scripts/capture-replay.py")


def changed_pixels(first, second):
    from PIL import Image, ImageChops
    with Image.open(first) as a, Image.open(second) as b:
        if a.size != (640, 480) or b.size != (640, 480):
            raise RuntimeError("Expected complete 640x480 captures.")
        delta = ImageChops.difference(a.convert("RGB"), b.convert("RGB"))
        return sum(pixel != (0, 0, 0) for pixel in delta.getdata())


def capture(binary, data, replay, output, stage, frame, windowed):
    output.mkdir()
    runtime = Path(tempfile.mkdtemp(prefix="portable-display.", dir=ROOT / "build"))
    metadata = dict(executableSha256=background.digest(binary), replaySha256=background.digest(replay),
                    observerSha256=background.digest(ROOT / "scripts/gdb/portable-display.gdb"),
                    gameDataSha256=background.digest(data / "th08.dat"),
                    bgmDataSha256=background.digest(data / "thbgm.dat"),
                    stage=stage, requestedFrame=frame, muted=True, windowed=windowed,
                    fastReplay=False, keepLives=False, cosmeticEnemyNameCopySuppressed=False,
                    screenshotInput="Physical HOME key through XTest",
                    displayEnvironment="Xvfb 1024x768, without a window manager")
    try:
        for archive in ("th08.dat", "thbgm.dat"):
            (runtime / archive).symlink_to((data / archive).resolve())
        cfg = bytearray(native_capture.muted_config())
        cfg[0x22] = int(windowed)
        (runtime / "th08.cfg").write_bytes(cfg)
        (runtime / "replay").mkdir()
        shutil.copyfile(replay, runtime / "replay/audit.rpy")
        environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                           GALLIUM_DRIVER="llvmpipe", TH08_AUTOPLAY_REPLAY="./replay/audit.rpy",
                           TH08_AUTOPLAY_STAGE=str(stage), TH08_RENDER_AUDIT_FAST="0",
                           TH08_DISPLAY_OUTPUT=str(output), TH08_DISPLAY_STAGE=str(stage),
                           TH08_DISPLAY_FRAME=str(frame))
        command = ["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", "timeout",
                   "--signal=TERM", "--kill-after=5s", "90s", "gdb", "--batch", "--quiet",
                   "-x", str(ROOT / "scripts/gdb/portable-display.gdb"),
                   "--args", str(binary), "--data-dir", str(runtime)]
        print(f"Inspecting {output.name}: stage index {stage}, HOME after frame {frame}.", flush=True)
        with (output / "capture.log").open("w") as log:
            result = subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
        metadata["runnerExitCode"] = result.returncode
        snapshot = runtime / "snapshot/th000.bmp"
        if result.returncode or not snapshot.is_file():
            raise RuntimeError("Display or HOME capture failed; inspect the retained log.")
        from PIL import Image
        metadata["snapshotBmpSha256"] = background.digest(snapshot)
        with Image.open(snapshot) as image:
            image.convert("RGB").save(output / "home-snapshot.png")
        report = dict(
            windowed=windowed,
            framebufferVersusWindowChangedPixels=changed_pixels(
                output / "before-home-framebuffer.png", output / "before-home-window.png"),
            snapshotVersusOldCpuCacheChangedPixels=changed_pixels(
                output / "home-snapshot.png", output / "before-home-cpu-cache.png"),
            snapshotVersusPresentedFrameChangedPixels=changed_pixels(
                output / "home-snapshot.png", output / "after-home-framebuffer.png"),
            nextFrameVersusOldCpuCacheChangedPixels=changed_pixels(
                output / "next-frame-framebuffer.png", output / "before-home-cpu-cache.png"))
        report["captures"] = {name: json.loads((output / (name + ".json")).read_text())
                              for name in ("before-home", "after-home", "next-frame")}
        (output / "summary.json").write_text(json.dumps(report, indent=2) + "\n")
        # These CPU captures duplicate the first cache in the observed failure.
        for name in ("after-home", "next-frame"):
            path = output / (name + "-cpu-cache.png")
            if background.digest(path) == background.digest(output / "before-home-cpu-cache.png"):
                path.unlink()
        return report
    finally:
        for name in ("modern-files.txt", "modern-crash.txt"):
            path = runtime / name
            if path.exists():
                shutil.copyfile(path, output / name)
                background.compress(output / name)
        shutil.rmtree(runtime)
        metadata["temporaryRuntimeRemoved"] = True
        (output / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
        background.compress(output / "capture.log")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--portable", type=Path, required=True, help="Native-layout x86_64 Debug ELF")
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--modes", choices=("both", "windowed", "fullscreen"), default="both")
    args = parser.parse_args()
    binary, data, output = (path.resolve() for path in
                            (args.portable, args.game_data_dir, args.output_dir))
    if not output.is_relative_to(ROOT / "build") or output.exists():
        parser.error("--output-dir must be a fresh directory under this repository's build/.")
    if not binary.is_file() or any(not (data / name).is_file() for name in ("th08.dat", "thbgm.dat")):
        parser.error("Portable executable and both original data archives are required.")
    fixture = next(item for item in json.loads((ROOT / "config/replay-fixtures.json").read_text())["fixtures"]
                   if item["id"] == "lunatic-0-b")
    replay = ROOT / "tests/fixtures/replays/th08-1.00d/accepted" / fixture["file"]
    verify_fixture(replay, fixture)
    output.mkdir(parents=True)
    reports = []
    for name, windowed in (("windowed", True), ("fullscreen", False)):
        if args.modes in ("both", name):
            reports.append(capture(binary, data, replay, output / name, 0, 600, windowed))
    manifest = dict(sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT,
                                                        text=True).strip(), fixture=fixture["id"],
                    runnerSha256=background.digest(Path(__file__)),
                    workingTreeDirty=bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
                    replaySha256=fixture["sha256"], reports=reports)
    (output / "summary.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print("Display and HOME evidence retained; temporary runtimes removed.", flush=True)


if __name__ == "__main__":
    main()
