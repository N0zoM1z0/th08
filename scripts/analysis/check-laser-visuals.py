#!/usr/bin/env python3
"""Discover and capture laser scenes from credited replays, with bounded resources."""
import argparse
import gzip
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from replay_file import verify_fixture

TARGET_SHA256 = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"
FRAME_COUNTER = "GameManager.gameplayFrameCounter after Present"


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def compress(path):
    if not path.exists():
        return
    with path.open("rb") as source, gzip.open(str(path) + ".gz", "wb", compresslevel=9) as destination:
        shutil.copyfileobj(source, destination)
    with gzip.open(str(path) + ".gz", "rb") as stream:
        if hashlib.file_digest(stream, "sha256").hexdigest() != sha(path):
            raise RuntimeError("Compressed evidence differs: " + str(path))
    path.unlink()


def source_identity():
    return dict(sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                sourceTreeDirty=bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
                runnerSha256=sha(Path(__file__)))


def validate_spawn_capture(output, selected_range):
    capture = json.loads((output / "laser-spawns.json").read_text())
    if capture.get("range") != selected_range or capture.get("complete") is not True:
        raise RuntimeError("Laser spawn observation did not complete its requested range")


def font_environment(override):
    if override:
        path = Path(override)
        if not path.is_absolute():
            raise ValueError("TH08_FONT must be absolute for reproducible laser captures; use --font")
        if not path.is_file() or not os.access(path, os.R_OK):
            raise ValueError("Portable font override is unreadable: " + str(path))
    else:
        path = Path(subprocess.check_output([
            "fc-match", "-f", "%{file}", "VL Gothic:lang=ja:spacing=mono"], text=True))
    path = path.resolve(strict=True)
    return dict(override=override, file=str(path), sha256=sha(path),
                selection="TH08_FONT" if override else "Fontconfig Japanese monospace fallback")


def capture_portable(candidate, data, replay, output, settings):
    observer = ROOT / "scripts/gdb/laser-visual-observer.py"
    selected_font = font_environment(settings.get("fontOverride") or os.environ.get("TH08_FONT"))
    runtime = Path(tempfile.mkdtemp(prefix="laser-visual.", dir=ROOT / "build"))
    settings_path = output / "settings.json"
    settings_path.write_text(json.dumps(dict(settings, output=str(output)), indent=2) + "\n")
    native = module("native_capture", ROOT / "scripts/capture-replay.py")
    configuration = native.muted_config()
    metadata = dict(source_identity(), product="portable-linux", executableSha256=sha(candidate),
                    replaySha256=sha(replay), gameDataSha256=sha(data / "th08.dat"),
                    bgmDataSha256=sha(data / "thbgm.dat"), observerSha256=sha(observer),
                    stage=settings["stage"], frames=settings["frames"], scout=settings["scout"],
                    frameCounter=FRAME_COUNTER, startStage=settings["stage"], playbackMode=0,
                    configSha256=hashlib.sha256(configuration).hexdigest(),
                    fastPortableClock=not settings["paced"],
                    rasterization=not (settings["scout"] or settings.get("geometryOnly")),
                    environment=dict(host=platform.platform(),
                                     gdb=subprocess.check_output(["gdb", "--version"], text=True).splitlines()[0],
                                     font=selected_font),
                    muted=True, keepLives=False, suppressLaserBodies=settings["suppressLaserBodies"])
    metadata["watchQuads"] = settings.get("watchQuads", False)
    metadata["geometryOnly"] = settings.get("geometryOnly", False)
    metadata["watchDraws"] = settings.get("watchDraws", False)
    metadata["watchRotatingHitboxes"] = settings.get("watchRotatingHitboxes", False)
    metadata["watchLaserSpawns"] = settings.get("watchLaserSpawns")
    metadata["renderInputControl"] = bool(settings.get("drawColorsFrom"))
    metadata["legacyOpenGL"] = settings.get("legacyOpenGL", False)
    try:
        for archive in ("th08.dat", "thbgm.dat"):
            (runtime / archive).symlink_to((data / archive).resolve())
        (runtime / "th08.cfg").write_bytes(configuration)
        (runtime / "replay").mkdir()
        shutil.copyfile(replay, runtime / "replay/audit.rpy")
        environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                           GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0",
                           TH08_AUTOPLAY_REPLAY="./replay/audit.rpy",
                           TH08_AUTOPLAY_STAGE=str(settings["stage"]),
                           TH08_RENDER_AUDIT_FAST="0" if settings["paced"] else "1", TH08_RENDER_AUDIT="0",
                           TH08_LASER_SETTINGS=str(settings_path))
        for name in ("TH08_REPLAY_TRACE_REFERENCE", "TH08_REPLAY_TRACE_ROWS", "TH08_REPLAY_TRACE_COMPLETION", "TH08_REPLAY_TRACE_MAXIMUM"):
            environment.pop(name, None)
        environment.pop("LP_NO_RAST", None)
        if settings.get("geometryOnly"):
            environment.update(LP_NO_RAST="true", LC_ALL="C")
        if settings.get("legacyOpenGL"):
            environment.update(MESA_GL_VERSION_OVERRIDE="2.1", MESA_EXTENSION_OVERRIDE="-GL_ARB_clip_control")
        if settings.get("fontOverride"):
            environment["TH08_FONT"] = settings["fontOverride"]
        if settings["scout"]:
            environment["LP_NO_RAST"] = "1"
        command = ["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", "timeout",
                   "--signal=TERM", "--kill-after=5s", "900s", "gdb", "--batch", "--quiet",
                   "-x", str(observer), "--args", str(candidate), "--data-dir", str(runtime)]
        with (output / "capture.log").open("w") as stream:
            result = subprocess.run(command, env=environment, stdout=stream, stderr=subprocess.STDOUT)
        metadata["runnerExitCode"] = result.returncode
        if result.returncode or not (output / "lasers.json").is_file():
            raise RuntimeError("Laser capture failed: " + str(output / "capture.log.gz"))
        if settings["scout"]:
            summary = json.loads((output / "scout-summary.json").read_text())
            if not summary.get("complete"):
                raise RuntimeError("Laser scout did not reach its requested boundary")
        if not settings["scout"] and not settings.get("geometryOnly"):
            for frame in settings["frames"]:
                if not (output / f"frame-{frame}.png").is_file():
                    raise RuntimeError("Requested laser image is missing")
        if settings.get("watchTextFonts") and not json.loads((output / "text-fonts.json").read_text()):
            raise RuntimeError("No bold-text font calls were captured in the selected stage")
        if settings.get("watchLaserSpawns"):
            validate_spawn_capture(output, settings["watchLaserSpawns"])
        if settings.get("watchQuads"):
            quads = json.loads((output / "quads.json").read_text())
            if [row["frame"] for row in quads] != settings["frames"] or any(not row["quads"] for row in quads):
                raise RuntimeError("Portable quad coverage differs from the requested frames")
        if settings.get("watchDraws"):
            draws = json.loads((output / "draws.json").read_text())
            if [row["frame"] for row in draws] != settings["frames"] or any(not row["draws"] for row in draws):
                raise RuntimeError("Portable draw coverage differs from the requested frames")
        if settings.get("drawColorsFrom"):
            control = json.loads((output / "draw-color-control.json").read_text())
            if not control["changedWords"] or control["restoredCalls"] != control["modifiedCalls"]:
                raise RuntimeError("Draw-color control did not complete its substitution and restoration")
            metadata["drawColorControl"] = control
        if settings.get("legacyOpenGL"):
            renderer = json.loads((output / "renderer.json").read_text())
            if renderer["clipControlLoaded"] or not renderer["version"].startswith("2.1 "):
                raise RuntimeError("Legacy capture did not exercise OpenGL 2.1 without clip control")
            metadata["environment"]["openGL"] = renderer
    finally:
        for name in ("modern-crash.txt", "modern-files.txt"):
            path = runtime / name
            if path.exists():
                shutil.copyfile(path, output / name)
        shutil.rmtree(runtime)
        metadata["temporaryRuntimeRemoved"] = True
        (output / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
        compress(output / "capture.log")
        for name in ("modern-files.txt", "modern-crash.txt"):
            compress(output / name)


def capture_retail(target, data, replay, output, stage, frames, watch_text_fonts=False, watch_quads=False,
                   candidate=None, map_path=None, wine_csmt=0, watch_laser_spawns=None):
    from pe_image import PEImage
    if target.stat().st_size != 840704 or sha(target) != TARGET_SHA256:
        raise ValueError("Retail input must be the canonical Japanese TH08 1.00d executable")
    image = PEImage(candidate or target)
    if image.machine != 0x14C:
        raise ValueError("Windows laser capture requires an x86 PE32 executable")
    label = "native" if candidate else "retail"
    post_present, quad_address = 0x442082, 0x462F10
    if candidate:
        owners = module("native_owners", ROOT / "scripts/analysis/verify-windows-i386-runtime-data.py")
        publics = owners.parse_publics(map_path)
        post_present = owners.public_va(publics, "Present@GameWindow") + 0x22
        quad_address = owners.public_va(publics, "AddSpriteToDrawBuffer@AnmManager")
        if image.read_rva(post_present - image.image_base, 6) != bytes.fromhex("85c07d308b0d"):
            raise ValueError("Native post-Present boundary differs from the target instruction shape")
    settings = dict(stage=stage, frames=frames, postPresentAddress=post_present,
                    postPresentBytes=image.read_rva(post_present - image.image_base, 13).hex(),
                    layoutFile=label + "-layout.json")
    if watch_laser_spawns:
        if candidate:
            raise ValueError("Spawn diagnostics currently require the canonical retail image")
        settings.update(watchLaserSpawns=watch_laser_spawns, spawnAddress=0x430F20,
                        spawnBytes=image.read_rva(0x430F20 - image.image_base, 9).hex())
    if watch_text_fonts:
        settings.update(watchTextFonts=True, fontCreatedAddress=0x43FA12,
                        fontCreatedBytes=image.read_rva(0x43FA12 - image.image_base, 11).hex())
    if watch_quads:
        settings.update(watchQuads=True, quadAddress=quad_address,
                        quadBytes=image.read_rva(quad_address - image.image_base, 12).hex())
    provenance = dict(source_identity(), observerSha256=sha(ROOT / "scripts/gdb/laser-visual-native.py"),
                      baseObserverSha256=sha(ROOT / "scripts/replay-external-observer.py"),
                      frameCounter=FRAME_COUNTER, frames=frames,
                      gameplayMemoryReadOnly=True, executableBytesUnmodified=True)
    provenance["watchQuads"] = watch_quads
    provenance["watchLaserSpawns"] = watch_laser_spawns
    stem = output.name + "-native" if candidate else "retail"
    settings_path = output.parent / (stem + "-settings.json")
    settings_path.write_text(json.dumps(settings, indent=2) + "\n")
    environment = dict(os.environ, TH08_STAGE5_REPOSITORY=str(ROOT),
                       TH08_STAGE5_NATIVE_OBSERVER=str(ROOT / "scripts/gdb/laser-visual-native.py"),
                       TH08_LASER_SETTINGS=str(settings_path), LIBGL_ALWAYS_SOFTWARE="1",
                       GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    environment.pop("LP_NO_RAST", None)
    environment.pop("WINE_D3D_CONFIG", None)
    if wine_csmt is not None:
        environment["WINE_D3D_CONFIG"] = "csmt=" + str(wine_csmt)
    prefix = output.parent / (stem + "-wine")
    command = [sys.executable, str(ROOT / "scripts/analysis/stage5-background-native-runner.py"),
               "--target", str(target), "--game-data", str((data / "th08.dat").resolve()),
               "--bgm-data", str((data / "thbgm.dat").resolve()), "--replay", str(replay),
               "--start-stage", str(stage), "--playback-mode", "0", "--timeout", "900",
               "--wine-prefix", str(prefix), "--output-dir", str(output)]
    if candidate:
        command.extend(["--candidate", str(candidate), "--map", str(map_path)])
    if watch_text_fonts:
        command.append("--trace-fonts")
    try:
        with (output.parent / (stem + ".log")).open("w") as stream:
            result = subprocess.run(command, env=environment, stdout=stream, stderr=subprocess.STDOUT)
        if result.returncode:
            raise RuntimeError(f"{label.capitalize()} laser capture exited with code {result.returncode}; see {stem}.log.gz")
        complete = json.loads((output / "complete.json").read_text())
        if complete.get("capturedFrames") != frames or not complete.get("diagnosticComplete") or complete.get("errors"):
            raise RuntimeError("Retail laser frames did not complete")
        if watch_text_fonts and not json.loads((output / "text-fonts.json").read_text()):
            raise RuntimeError("No retail bold-text font calls were captured in the selected stage")
        if watch_laser_spawns:
            validate_spawn_capture(output, watch_laser_spawns)
        if watch_quads:
            quads = json.loads((output / "quads.json").read_text())
            if [row["frame"] for row in quads] != frames or any(not row["quads"] for row in quads):
                raise RuntimeError("Retail quad coverage differs from the requested frames")
        metadata_path = output / "metadata.json"
        metadata = json.loads(metadata_path.read_text())
        metadata["laserCapture"] = provenance
        metadata["wineCsmt"] = "default" if wine_csmt is None else wine_csmt
        if candidate:
            metadata.update(replaySha256=sha(replay), stage=stage, frames=frames, scout=False,
                            keepLives=False, runnerExitCode=0, temporaryRuntimeRemoved=not (output / "runtime").exists(),
                            frameCounter=FRAME_COUNTER, suppressLaserBodies=False,
                            nativeMapSha256=sha(map_path))
        metadata_path.write_text(json.dumps(metadata, indent=2) + "\n")
    finally:
        if prefix.exists():
            if not (prefix / ".th08-replay-owned").is_file():
                raise RuntimeError("Refusing to remove an unowned Wine prefix")
            cleanup = dict(environment, WINEPREFIX=str(prefix), WINEARCH="win32")
            subprocess.run(["wineserver", "-k"], env=cleanup, timeout=15, check=False)
            subprocess.run(["wineserver", "-w"], env=cleanup, timeout=15, check=False)
            shutil.rmtree(prefix)
        for folder in (output.parent, output):
            if folder.exists():
                for path in list(folder.glob("*.log")) + list(folder.glob("*.jsonl")):
                    compress(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True, help="Portable Debug ELF or native VC7 bugfix PE32 with --native-map")
    parser.add_argument("--native-map", type=Path, help="VC7 linker map; capture the Windows candidate at ordinary pacing")
    parser.add_argument("--wine-csmt", type=int, choices=(0, 1), help="Native/retail Wine command-stream threading (default: 0 for synchronous images)")
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--case", help="ID in config/laser-visual-fixtures.json")
    parser.add_argument("--fixture", help="ID in config/replay-fixtures.json; default: lunatic-1-a")
    parser.add_argument("--stage", type=int, help="Internal stage index, 0 through 8")
    parser.add_argument("--frames", help="Comma-separated active frame numbers")
    parser.add_argument("--scout", action="store_true", help="Discover laser state at presented frames without rasterization")
    parser.add_argument("--sample-interval", type=int, default=30, help="Scout interval in presented gameplay frames; 1 checks every distinct frame")
    parser.add_argument("--paced", action="store_true", help="Capture portable images with ordinary pacing")
    parser.add_argument("--legacy-gl", action="store_true", help="Capture portable through Mesa OpenGL 2.1 without clip control")
    parser.add_argument("--font", type=Path, help="Portable font override; recorded with its file hash")
    parser.add_argument("--watch-text-fonts", action="store_true", help="Record bold text parameters and retail font diagnostics")
    parser.add_argument("--watch-quads", action="store_true", help="Read every submitted sprite quad in the selected frames")
    parser.add_argument("--geometry-only", action="store_true", help="Capture portable quad inputs with rasterization disabled")
    parser.add_argument("--geometry-reference", type=Path, help="Verify geometry-only inputs against an ordinary capture of the same binary and frames")
    parser.add_argument("--watch-draws", action="store_true", help="Read portable draw inputs, matrices, states and texture identities")
    parser.add_argument("--watch-rotating-hitboxes", action="store_true",
                        help="Scout the three enemy rotating-laser collision helpers and their VM state")
    parser.add_argument("--watch-laser-spawns", help="Read spawn inputs in an inclusive START:END gameplay-frame range")
    parser.add_argument("--draw-colors-from", type=Path, help="Experimental zero-alpha RGB control using another completed draw capture")
    parser.add_argument("--draw-color-index", type=int, default=2, help="Draw index for the experimental color control")
    parser.add_argument("--maximum-frame", type=int, default=30000)
    parser.add_argument("--suppress-laser-bodies", action="store_true", help="Isolated missing-laser negative control")
    parser.add_argument("--retail", action="store_true", help="Also capture the original executable at the same frames")
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    args = parser.parse_args()
    spawn_range = None
    if args.watch_laser_spawns:
        try:
            spawn_range = [int(part) for part in args.watch_laser_spawns.split(":")]
        except ValueError:
            parser.error("Spawn range must be START:END")
        if len(spawn_range) != 2 or not 1 <= spawn_range[0] <= spawn_range[1]:
            parser.error("Spawn range must have positive, ordered START:END frames")
        if args.native_map or args.watch_text_fonts or args.watch_quads or args.watch_draws or args.suppress_laser_bodies:
            parser.error("Spawn diagnostics require a separate portable/retail capture")
    if args.wine_csmt is not None and not (args.native_map or args.retail):
        parser.error("--wine-csmt requires --native-map or --retail")
    if args.wine_csmt is None and (args.native_map or args.retail):
        args.wine_csmt = 0
    if args.native_map and (args.scout or args.legacy_gl or args.font or args.watch_text_fonts or args.watch_draws
                            or args.draw_colors_from or args.suppress_laser_bodies):
        parser.error("Native captures support ordinary images and --watch-quads")
    if args.sample_interval < 1:
        parser.error("Sample interval must be positive")
    if args.watch_rotating_hitboxes and (not args.scout or args.native_map):
        parser.error("Rotating-hitbox discovery requires a portable --scout capture")
    case = None
    if args.case:
        config = json.loads((ROOT / "config/laser-visual-fixtures.json").read_text())
        case = next((entry for entry in config["cases"] if entry["id"] == args.case), None)
        if case is None or config.get("frameCounter") != FRAME_COUNTER:
            parser.error("Unknown laser case or incompatible frame counter")
        if args.fixture is not None or args.stage is not None or args.frames is not None or args.scout:
            parser.error("--case supplies --fixture, --stage and --frames")
        args.fixture, args.stage = case["fixture"], case["stage"]
        args.frames = ",".join(str(entry["frame"]) for entry in case["frames"])
    args.fixture = args.fixture or "lunatic-1-a"
    if args.stage is None or not 0 <= args.stage <= 8:
        parser.error("Select --case or a --stage index from 0 through 8")
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    try:
        frames = sorted(set(int(value) for value in (args.frames or "").split(",") if value))
    except ValueError:
        parser.error("--frames must contain comma-separated integers")
    if not args.scout and (not frames or min(frames) < 1):
        parser.error("Capture requires positive --frames")
    if args.scout and (args.retail or args.suppress_laser_bodies or args.paced):
        parser.error("Scout mode does not capture retail images or run missing-laser controls")
    if args.scout and args.watch_text_fonts:
        parser.error("Text-font diagnostics require rasterized frame captures")
    if args.scout and args.legacy_gl:
        parser.error("Legacy OpenGL validation requires rasterized frame captures")
    if args.geometry_reference and not args.geometry_only:
        parser.error("--geometry-reference requires --geometry-only")
    if args.geometry_only and (not args.watch_quads or args.scout or args.retail or args.native_map or args.paced):
        parser.error("--geometry-only requires a portable --watch-quads capture")
    if args.geometry_only and len(frames) > 16:
        parser.error("Geometry-only captures support at most 16 selected frames per run")
    if args.watch_quads and (args.scout or args.watch_text_fonts or args.suppress_laser_bodies):
        parser.error("Quad diagnostics require an ordinary frame capture, separate from font and omission probes")
    if args.watch_draws and (args.scout or args.watch_quads or args.watch_text_fonts or args.suppress_laser_bodies or args.retail):
        parser.error("Draw diagnostics require a separate portable frame capture")
    if args.draw_colors_from and (not args.watch_draws or len(frames) != 1 or args.draw_color_index < 0):
        parser.error("Draw-color controls require --watch-draws, one frame and a nonnegative draw index")
    if args.retail and args.suppress_laser_bodies:
        parser.error("Compare a missing-laser control against separate normal and retail captures")
    fixture = next((entry for entry in json.loads((ROOT / "config/replay-fixtures.json").read_text())["fixtures"]
                    if entry["id"] == args.fixture), None)
    if fixture is None or args.stage not in [entry["index"] for entry in fixture["stages"]]:
        parser.error("Fixture is unknown or does not contain the requested stage")
    header = next(entry for entry in fixture["stages"] if entry["index"] == args.stage)
    maximum = min(args.maximum_frame, header["inputRecords"] - 8)
    if spawn_range and spawn_range[1] > (maximum if args.scout else max(frames)):
        parser.error("Spawn range must end within the requested capture")
    if maximum < 1 or any(frame > header["inputRecords"] - 8 for frame in frames):
        parser.error("Requested frame is outside the recording's complete gameplay range")
    replay = ROOT / "tests/fixtures/replays/th08-1.00d/accepted" / fixture["file"]
    verify_fixture(replay, fixture)
    candidate, data = args.candidate.resolve(strict=True), args.game_data_dir.resolve(strict=True)
    if (args.retail or args.native_map) and (args.target.stat().st_size != 840704 or sha(args.target) != TARGET_SHA256):
        parser.error("Retail input must be the canonical Japanese TH08 1.00d executable")
    if args.geometry_reference:
        geometry = module("geometry_preflight", ROOT / "scripts/analysis/compare-sprite-geometry.py")
        reference_meta = geometry.profile(args.geometry_reference.resolve(strict=True), True)
        if (reference_meta["executableSha256"] != sha(candidate) or reference_meta["frames"] != frames
                or reference_meta["stage"] != args.stage or reference_meta["replaySha256"] != sha(replay)):
            parser.error("Geometry reference must use the same executable, recording, stage and frames")
    for name in ("th08.dat", "thbgm.dat"):
        if not (data / name).is_file():
            parser.error("Original archive is missing: " + name)
    if args.native_map:
        output.parent.mkdir(parents=True, exist_ok=True)
    else:
        output.mkdir(parents=True)
    color_capture = None
    if args.draw_colors_from:
        color_capture = args.draw_colors_from.resolve(strict=True)
        draw_module = module("draw_comparison", ROOT / "scripts/analysis/compare-portable-draws.py")
        color_meta, color_rows = draw_module.load(color_capture)
        if color_meta["stage"] != args.stage or color_meta["frames"] != frames or color_meta["replaySha256"] != sha(replay):
            parser.error("Draw-color source capture does not match the requested recording and frame")
    settings = dict(stage=args.stage, frames=frames, scout=args.scout,
                    sampleInterval=args.sample_interval,
                    maximumFrame=maximum, paced=args.paced, suppressLaserBodies=args.suppress_laser_bodies,
                    watchTextFonts=args.watch_text_fonts,
                    watchQuads=args.watch_quads,
                    geometryOnly=args.geometry_only,
                    watchDraws=args.watch_draws,
                    watchRotatingHitboxes=args.watch_rotating_hitboxes,
                    watchLaserSpawns=spawn_range,
                    drawColorsFrom=str(color_capture) if color_capture else None,
                    drawColorIndex=args.draw_color_index,
                    legacyOpenGL=args.legacy_gl,
                    fontOverride=str(args.font.resolve(strict=True)) if args.font else None)
    if case and not args.native_map:
        (output / "case.json").write_text(json.dumps(case, indent=2) + "\n")
    print(f"Reading laser scene: {fixture['id']}, stage index {args.stage}", flush=True)
    if args.native_map:
        capture_retail(args.target.resolve(strict=True), data, replay, output, args.stage, frames,
                       watch_quads=args.watch_quads, candidate=candidate, map_path=args.native_map.resolve(strict=True),
                       wine_csmt=args.wine_csmt)
        if case:
            (output / "case.json").write_text(json.dumps(case, indent=2) + "\n")
    else:
        capture_portable(candidate, data, replay, output, settings)
    if args.geometry_reference:
        geometry = module("sprite_geometry", ROOT / "scripts/analysis/compare-sprite-geometry.py")
        report = geometry.compare(args.geometry_reference.resolve(strict=True), output)
        (output / "geometry-comparison.json").write_text(json.dumps(report, indent=2) + "\n")
        print("Sprite geometry comparison: " + report["result"], flush=True)
        if report["result"] != "equal":
            raise SystemExit(1)
    records = json.loads((output / "lasers.json").read_text())
    if case:
        for sample, expected in zip(records, case["frames"]):
            if expected["expectedLaserState"] not in {laser["state"] for laser in sample["lasers"]}:
                raise RuntimeError("Selected laser phase is missing at frame " + str(expected["frame"]))
    kind = "geometry" if args.geometry_only else "laser"
    print(f"Captured {len(records)} {kind} samples in {output}", flush=True)
    if args.retail:
        print("Capturing retail laser frames with ordinary pacing and rasterization", flush=True)
        capture_retail(args.target.resolve(strict=True), data, replay, output / "retail", args.stage, frames,
                       args.watch_text_fonts, args.watch_quads, wine_csmt=args.wine_csmt,
                       watch_laser_spawns=spawn_range)
        comparator = module("laser_comparison", ROOT / "scripts/analysis/compare-laser-visuals.py")
        report = comparator.compare(output / "retail", output)
        (output / "comparison.json").write_text(json.dumps(report, indent=2) + "\n")
        print("Laser visual comparison: " + report["result"], flush=True)
        spawn_failed = False
        if spawn_range:
            spawn_comparator = module("laser_spawns", ROOT / "scripts/analysis/compare-laser-spawns.py")
            spawn_report = spawn_comparator.compare(output / "retail", output)
            (output / "spawn-comparison.json").write_text(json.dumps(spawn_report, indent=2) + "\n")
            print("Laser spawn comparison: " + spawn_report["result"], flush=True)
            spawn_failed = spawn_report["result"] != "equal"
        if report["result"] != "equal" or spawn_failed:
            raise SystemExit(1)


if __name__ == "__main__":
    main()
