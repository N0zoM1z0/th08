"""Capture retail laser state and images using attested read-only breakpoints."""
import gdb
import json
import os
from pathlib import Path
import struct
import subprocess

root = Path(os.environ["TH08_STAGE5_REPOSITORY"])
original = root / "scripts/replay-external-observer.py"
ns = {"__file__": str(original), "__name__": "laser_replay_observer"}
exec(compile(original.read_text(), str(original), "exec"), ns)
output, observer = ns["output"], ns["observer"]
settings = json.loads(Path(os.environ["TH08_LASER_SETTINGS"]).read_text())
stage = settings["stage"]
frames = set(settings["frames"])
records = []
address = settings["postPresentAddress"]
expected = bytes.fromhex(settings["postPresentBytes"])
if ns["block"](address, len(expected)) != expected:
    raise RuntimeError("Retail post-Present instructions differ from the canonical target")

# TH08 1.00d: BulletManager::OnDraw 0x432B50 walks +0x660938 with stride 0x59C.
base = 0x00F54E90 + 0x660938
stride = 0x59C
fields = {"position": (0x548, "3I"), "angle": (0x554, "I"),
          "startOffset": (0x558, "I"), "endOffset": (0x55C, "I"),
          "startLength": (0x560, "I"), "width": (0x564, "I"),
          "currentWidth": (0x568, "I"), "speed": (0x56C, "I"),
          "startTime": (0x570, "i"), "duration": (0x578, "i"),
          "despawnDuration": (0x57C, "i"), "inUse": (0x584, "i"),
          "timer.current": (0x590, "i"), "flags": (0x594, "H"),
          "color": (0x596, "h"), "state": (0x598, "B"),
          "hideCapDuringStartup": (0x599, "B")}
for vm, start in (("bodyVm", 0), ("startCapVm", 0x2A4)):
    for field, offset, format_ in (("scale", 0x18, "2I"), ("spriteSize", 0x28, "2I"), ("pos", 0x208, "3I"),
                                   ("rotation", 0, "3I"), ("color1.d3dColor", 0x1F0, "I"),
                                   ("color2.d3dColor", 0x1F4, "I"), ("flagsWord", 0x1F8, "I"),
                                   ("activeSpriteIndex", 0x214, "h"), ("anmFileIndex", 0x216, "h"),
                                   ("scriptIndex", 0x21A, "h")):
        fields[vm + "." + field] = (start + offset, format_)
(output / "retail-layout.json").write_text(json.dumps(dict(
    base=base, stride=stride, offsets={name: item[0] for name, item in fields.items()}), indent=2) + "\n")


def laser_state():
    raw = ns["block"](base, stride * 256)
    result = []
    for slot in range(256):
        start = slot * stride
        if struct.unpack_from("<i", raw, start + 0x584)[0] == 0:
            continue
        item = dict(slot=slot)
        for name, (offset, format_) in fields.items():
            values = struct.unpack_from("<" + format_, raw, start + offset)
            item[name] = list(values) if len(values) > 1 else values[0]
        result.append(item)
    return result


class Screenshot(gdb.Breakpoint):
    def __init__(self):
        super().__init__("*0x%08X" % address, gdb.BP_HARDWARE_BREAKPOINT)
        self.enabled = False

    def stop(self):
        try:
            return self.capture()
        except Exception as error:
            self.enabled = False
            observer.finish("Laser frame capture failed: " + str(error))
            return False

    def capture(self):
        current_stage = ns["read"](ns["GM"] + 0x3DDC4)
        frame = ns["read"](ns["GM"] + 0x3DDC0)
        if current_stage != stage or frame < min(frames):
            return False
        if frame not in frames:
            raise RuntimeError("Retail presentation skipped a requested laser frame")
        records.append(dict(stage=stage, frame=frame, lasers=laser_state()))
        (output / "lasers.json").write_text(json.dumps(records, indent=2) + "\n")
        windows = subprocess.check_output([
            "xdotool", "search", "--onlyvisible", "--pid", str(gdb.selected_inferior().pid)], text=True).split()
        if len(windows) != 1:
            raise RuntimeError("Retail capture requires exactly one visible game window")
        subprocess.run(["import", "-silent", "-window", windows[0], str(output / f"frame-{frame}.png")],
                       check=True, timeout=15)
        frames.remove(frame)
        if not frames:
            for name in ("stream", "menu_log", "events", "releases", "score_events", "item_events"):
                ns[name].flush()
                ns[name].close()
            for bp in (observer, observer.completion_observer, ns["begin_observer"], ns["delete_observer"], self):
                bp.enabled = False
            (output / "complete.json").write_text(json.dumps(dict(
                ended=False, diagnosticComplete=True, errors=[], rows=observer.count,
                stage=stage, capturedFrames=[record["frame"] for record in records],
                gameplayMemoryReadOnly=True, executableBytesUnmodified=True)) + "\n")
        return False


screenshot = Screenshot()
ordinary_stop = observer.stop


def observe_and_arm(completion=False):
    result = ordinary_stop(completion=completion)
    if observer.stage == stage and ns["read"](ns["GM"] + 0x3DDC0) >= max(1, min(frames) - 16):
        observer.completion_observer.enabled = False
        screenshot.enabled = True
    return result


observer.stop = observe_and_arm
