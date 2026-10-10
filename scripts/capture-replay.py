#!/usr/bin/env python3
"""Capture a bundled demo from the retail target or a native VC7 reconstruction."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import queue
import re
import shutil
import socket
import struct
import subprocess
import threading
import time
from pe_image import PEImage

TARGET_SHA = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"


def observer_config(candidate, map_path):
    if candidate is None:
        return dict(gameManager=0x0160F508, player=0x017D5EF8, rng=0x0164D520, input=0x0164D52C,
                    itemManager=0x01653648,
                    bulletManager=0x00F54E90,
                    effectManager=0x004ECE60, anmExecute=0x0045EA00,
                    releaseEffects=0x0042A820,
                    enemyManager=0x00577F20, addScore=0x004181F0, randomU16=0x0043ECC0,
                    boundary=0x00452490, attestAddress=0x00452490,
                    attestBytes="558bec51894dfca1b4d06401c1",
                    completionBoundary=0x00441F52, completionAttestAddress=0x00441F48,
                    completionAttestBytes="b948f56401e8feaaffff8945fc")
    image = PEImage(candidate)
    symbols = {}
    for line in map_path.read_text(encoding="cp1252").splitlines():
        match = re.match(r"\s+[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s", line, re.I)
        if match:
            symbols.setdefault(match[1], set()).add(int(match[2], 16))
    def address(name):
        values = {value for symbol, addresses in symbols.items() if symbol.startswith("?" + name + "@")
                  for value in addresses}
        if len(values) != 1:
            raise ValueError(f"Expected one linked address for {name}, found {values}")
        return values.pop()
    render, present, calc, chain = [address(name) for name in ("Render@GameWindow", "Present@GameWindow", "RunCalcChain@Chain", "g_Chain")]
    if not render < present <= render + 1024:
        raise ValueError("Unexpected Render/Present extent in linker map")
    code = image.read_rva(render - image.image_base, present - render)
    calls = [render + index for index in range(5, len(code) - 7)
             if code[index] == 0xe8 and render + index + 5 + struct.unpack_from("<i", code, index + 1)[0] == calc
             and code[index - 5:index] == b"\xb9" + struct.pack("<I", chain)]
    if len(calls) != 1:
        raise ValueError("Expected one mov ecx,g_Chain / call RunCalcChain in Render")
    call = calls[0]
    attest = call - 5
    frame_control = address("ControlPlaybackFrameAdvance@ReplayManager")
    return dict(gameManager=address("g_GameManager"), player=address("g_Player"), rng=address("g_Rng"),
                input=address("g_GuiMessageInputCurrent"), itemManager=address("g_ItemManager"),
                bulletManager=address("g_BulletManager"),
                effectManager=address("g_EffectManager"), anmExecute=address("ExecuteScript@AnmManager"),
                releaseEffects=address("ReleaseAttachedEffects@Enemy"),
                enemyManager=address("g_EnemyManager"), addScore=address("AddScore@GameManager"),
                randomU16=address("GetRandomU16@Rng"),
                boundary=frame_control, attestAddress=frame_control,
                attestBytes=image.read_rva(frame_control - image.image_base, 13).hex(),
                completionBoundary=call + 5, completionAttestAddress=attest,
                completionAttestBytes=image.read_rva(attest - image.image_base, 13).hex())


def sha(path):
    with open(path, "rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def muted_config():
    # Supervisor::LoadConfig defaults, with BGM/SFX off and a windowed display.
    cfg = bytearray(60)
    struct.pack_into("<10hI2h", cfg, 0, 0, 1, 2, 4, -1, -1, -1, -1, 3, 0,
                     0x80001, 600, 600)
    cfg[0x1c:0x27] = bytes([2, 3, 0, 0, 0, 1, 1, 0, 2, 0, 0])
    return bytes(cfg)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--target", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, help="Playable VC7 bugfix PE32 image; omitted for retail reference")
    parser.add_argument("--map", dest="map_path", type=Path, help="Linker map produced with --candidate")
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--bgm-data", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--wine", default="/usr/lib/wine/wine",
                        help="Wine ELF loader (not the shell wrapper)")
    parser.add_argument("--timeout", type=float, help="Capture deadline in seconds; default: 240 per demo slot")
    parser.add_argument("--demos", default="0", help="Bundled demo indexes, e.g. 0 or 0,1,2")
    parser.add_argument("--keep-runtime", action="store_true", help="Keep the isolated Wine prefix for debugging")
    parser.add_argument("--detail-start", type=int, default=0, help="First frame for item/point-value diagnostics")
    parser.add_argument("--detail-end", type=int, default=0, help="Last frame for item/point-value diagnostics")
    parser.add_argument("--watch-effect", type=int, action="append", default=[],
                        help="Trace this effect ID's lifetime, including inactive slots; repeat as needed")
    parser.add_argument("--display", help="Existing X display; default: a private Xvfb display")
    args = parser.parse_args()
    try:
        demos = [int(value) for value in args.demos.split(",")]
        if not demos or demos != sorted(set(demos)) or any(index not in (0, 1, 2) for index in demos):
            raise ValueError()
    except ValueError:
        parser.error("--demos must be an ordered subset of 0,1,2")
    if args.timeout is None:
        args.timeout = 240 * (max(demos) + 1)
    if bool(args.candidate) != bool(args.map_path):
        parser.error("--candidate and --map must be supplied together")
    if (args.detail_start or args.detail_end) and not 1 <= args.detail_start <= args.detail_end <= 6120:
        parser.error("Detail window must satisfy 1 <= start <= end <= 6120")
    if any(value not in range(66) for value in args.watch_effect):
        parser.error("--watch-effect requires an ID from 0 to 65")
    if args.watch_effect and args.detail_start:
        parser.error("Use separate captures for --watch-effect and detail windows: x86 has four hardware breakpoint slots")
    if args.candidate:
        args.candidate = args.candidate.resolve()
        args.map_path = args.map_path.resolve()
    for name in ("target", "game_data", "bgm_data", "output_dir"):
        setattr(args, name, getattr(args, name).resolve())
    if args.target.stat().st_size != 840704 or sha(args.target) != TARGET_SHA:
        parser.error("Target must be the original Japanese TH08 1.00d executable")
    args.output_dir.mkdir(parents=True, exist_ok=False)
    runtime = args.output_dir / "runtime"
    runtime.mkdir()
    executable = args.candidate or args.target
    observer_settings = observer_config(args.candidate, args.map_path)
    observer_settings.update(detailStart=args.detail_start, detailEnd=args.detail_end, demoIndexes=demos,
                             watchEffects=args.watch_effect)
    shutil.copyfile(executable, runtime / "th08.exe")
    for name, data in (("th08.dat", args.game_data), ("thbgm.dat", args.bgm_data)):
        (runtime / name).symlink_to(data)
    config = muted_config()
    (runtime / "th08.cfg").write_bytes(config)
    schema = json.loads(Path(__file__).with_name("replay-schema.json").read_text())
    observer_settings["demoEndFrames"] = schema["demoEndFrames"]
    fixture = "demo/demorpy0.rpy" if demos == [0] else [f"demo/demorpy{index}.rpy" for index in demos]
    metadata = dict(schema=schema, fixture=fixture, demoIndexes=demos, targetSha256=TARGET_SHA,
                    gameDataSha256=sha(args.game_data), configSha256=hashlib.sha256(config).hexdigest(),
                    muted=True, executableSha256=sha(executable),
                    product="native-vc7-bugfix" if args.candidate else "retail",
                    observer=dict(type="GDB hardware breakpoints",
                                  frame=f"0x{observer_settings['boundary']:08X}",
                                  completion=f"0x{observer_settings['completionBoundary']:08X}"))
    if args.candidate:
        metadata["mapSha256"] = sha(args.map_path)
        metadata["sourceCommit"] = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
        metadata["sourceTreeDirty"] = bool(subprocess.check_output(["git", "status", "--porcelain", "--", "src", "config"]))
    (args.output_dir / "observer.json").write_text(json.dumps(observer_settings, indent=2) + "\n")
    (args.output_dir / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    env = os.environ.copy()
    # Each capture owns its Wine prefix; user saves and Wine sessions are isolated.
    env.update(WINEPREFIX=str(args.output_dir / "wine"), WINEDEBUG="-all")
    xvfb = None
    xvfb_log = None
    if args.display:
        env["DISPLAY"] = args.display
    else:
        # An explicit display also works on WSL, where /tmp/.X11-unix may be
        # read-only and Xvfb -displayfd cannot allocate its filesystem socket.
        xvfb_log = (args.output_dir / "xvfb.log").open("w")
        for number in range(300 + os.getpid() % 2000, 330 + os.getpid() % 2000):
            probe = socket.socket(socket.AF_UNIX)
            try:
                probe.connect(f"\0/tmp/.X11-unix/X{number}")
                continue
            except OSError:
                pass
            finally:
                probe.close()
            xvfb = subprocess.Popen(["Xvfb", f":{number}", "-screen", "0", "800x600x24",
                                     "-nolisten", "tcp", "-noreset"], stdout=subprocess.DEVNULL,
                                    stderr=xvfb_log)
            time.sleep(.5)
            if xvfb.poll() is None:
                env["DISPLAY"] = f":{number}"
                break
        else:
            raise RuntimeError("Unable to start private Xvfb; inspect xvfb.log")
    # Initialize Wine before debugging so its first-run setup is outside capture.
    try:
        subprocess.run(["wineboot", "-u"], env=env, check=True, timeout=90,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        events = queue.Queue()
        stopped = threading.Event()
        with (args.output_dir / "gdb.log").open("w") as log:
            proc = subprocess.Popen(["gdb", "-q", "--interpreter=mi2", "--args", args.wine,
                                     str(runtime / "th08.exe")], cwd=runtime, env=env, text=True,
                                    stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            def read_output():
                for line in proc.stdout:
                    log.write(line)
                    log.flush()
                    if line.startswith("*stopped"):
                        stopped.set()
                    elif line.startswith("*running"):
                        stopped.clear()
                    events.put(line.rstrip())
            reader = threading.Thread(target=read_output, daemon=True)
            reader.start()
            token = 0
            def command(command_text):
                nonlocal token
                token += 1
                proc.stdin.write(f"{token}{command_text}\n")
                proc.stdin.flush()
                deadline = time.monotonic() + 20
                while time.monotonic() < deadline:
                    line = events.get(timeout=max(.01, deadline - time.monotonic()))
                    if line.startswith(f"{token}^error"):
                        raise RuntimeError(line)
                    if line.startswith(f"{token}^"):
                        return line
                raise TimeoutError(command_text)
            try:
                for option in ("pagination off", "confirm off", "mi-async on", "disable-randomization off"):
                    command("-gdb-set " + option)
                command("-gdb-set debuginfod enabled off")
                for sig in ("SIGUSR1", "SIGUSR2"):
                    command('-interpreter-exec console ' + json.dumps(f"handle {sig} nostop noprint pass"))
                command("-exec-run")
                # Wine execs its loader during startup. Install the hardware
                # breakpoint after that exec so the debug registers survive.
                load_deadline = time.monotonic() + 90
                while time.monotonic() < load_deadline:
                    time.sleep(2)
                    command("-exec-interrupt --all")
                    if not stopped.wait(20):
                        raise TimeoutError("Wine did not stop for observer installation")
                    try:
                        mapped = command(f"-data-read-memory-bytes {observer_settings['attestAddress']} 13")
                    except RuntimeError:
                        mapped = ""
                    if 'contents="' + observer_settings["attestBytes"] + '"' in mapped:
                        break
                    command("-exec-continue")
                else:
                    raise RuntimeError("Executable did not map its attested capture instructions")
                observer = Path(__file__).with_name("replay-observer.py")
                command('-interpreter-exec console ' + json.dumps("source " + str(observer)))
                command("-exec-continue")
                deadline = time.monotonic() + args.timeout
                last_count = -1
                while time.monotonic() < deadline and proc.poll() is None:
                    done = args.output_dir / "complete.json"
                    if done.exists():
                        report = json.loads(done.read_text())
                        if not report.get("ended") or report.get("errors"):
                            raise RuntimeError(f"Observer failed: {report}")
                        print(json.dumps(report))
                        return
                    rows = args.output_dir / "rows.jsonl"
                    if rows.exists():
                        count = sum(1 for _ in rows.open())
                        if count // 600 != last_count // 600:
                            print(f"Captured {count} frames", flush=True)
                        last_count = count
                    time.sleep(1)
                raise RuntimeError("Incomplete capture; inspect gdb.log and rows.jsonl")
            finally:
                if proc.poll() is None:
                    try:
                        command("-exec-interrupt --all")
                        time.sleep(.2)
                        command('-interpreter-exec console "kill"')
                        command("-gdb-exit")
                    except (RuntimeError, OSError, queue.Empty):
                        proc.kill()
                proc.wait(timeout=15)
                reader.join(timeout=5)
    finally:
        # This prefix was created by this invocation and contains no other work.
        subprocess.run(["wineserver", "-k"], env=env, check=False, timeout=15)
        if not args.keep_runtime:
            subprocess.run(["wineserver", "-w"], env=env, check=False, timeout=15)
            shutil.rmtree(args.output_dir / "wine", ignore_errors=True)
            shutil.rmtree(runtime, ignore_errors=True)
        if xvfb:
            if xvfb.poll() is None:
                xvfb.terminate()
                xvfb.wait(timeout=5)
        if xvfb_log:
            xvfb_log.close()


if __name__ == "__main__":
    main()
