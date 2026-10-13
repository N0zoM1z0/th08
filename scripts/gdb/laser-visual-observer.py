"""Read typed laser state and select or capture bounded replay frames."""
import gdb
import json
import os
from pathlib import Path
import struct
import subprocess
import struct

settings = json.loads(Path(os.environ["TH08_LASER_SETTINGS"]).read_text())
output = Path(settings["output"])
stage = settings["stage"]
frames = set(settings.get("frames", []))
scout = settings.get("scout", False)
maximum = settings.get("maximumFrame", max(frames, default=30000))
sample_interval = settings.get("sampleInterval", 30)
records = []
scout_summary = dict(sampledFrames=0, framesWithLasers=0, firstFrame=None, lastFrame=None,
                     stopReason=None, complete=False)


def value(expression):
    return gdb.parse_and_eval(expression)


def number(expression):
    return int(value(expression))


def current():
    global counter_addresses
    if counter_addresses is None:
        counter_addresses = (int(value("th08::g_GameManager.currentStage").address),
                             int(value("th08::g_GameManager.gameplayFrameCounter").address))
    inferior = gdb.selected_inferior()
    return tuple(struct.unpack("<i", bytes(inferior.read_memory(address, 4)))[0]
                 for address in counter_addresses)


fields = {"position": ("3I", 3), "angle": ("I", 1), "startOffset": ("I", 1),
          "endOffset": ("I", 1), "startLength": ("I", 1), "width": ("I", 1),
          "currentWidth": ("I", 1), "speed": ("I", 1), "startTime": ("i", 1),
          "duration": ("i", 1), "despawnDuration": ("i", 1), "inUse": ("i", 1),
          "timer.current": ("i", 1), "flags": ("H", 1), "color": ("h", 1),
          "state": ("B", 1), "hideCapDuringStartup": ("B", 1)}
for vm in ("bodyVm", "startCapVm"):
    for field, format_, count in (("scale", "2I", 2), ("spriteSize", "2I", 2), ("pos", "3I", 3),
                                  ("rotation", "3I", 3), ("color1.d3dColor", "I", 1),
                                  ("color2.d3dColor", "I", 1), ("flagsWord", "I", 1),
                                  ("activeSpriteIndex", "h", 1), ("anmFileIndex", "h", 1),
                                  ("scriptIndex", "h", 1)):
        fields[vm + "." + field] = (format_, count)
layouts = None
counter_addresses = None
last_sampled_frame = None
last_progress_frame = None


def progress(selected_stage, frame):
    global last_progress_frame
    if selected_stage == stage and frame > 0 and frame % 3000 == 0 and frame != last_progress_frame:
        last_progress_frame = frame
        print(f"Laser replay: stage index {stage}, gameplay frame {frame}", flush=True)


def laser_state():
    global layouts
    expression = "th08::g_BulletManager.lasers"
    lasers = value(expression)
    base = int(lasers.address)
    stride = lasers.type.target().sizeof
    if layouts is None:
        layouts = {name: int(value(expression + "[0]." + name).address) - base for name in fields}
        (output / "portable-layout.json").write_text(json.dumps(
            dict(stride=stride, offsets=layouts), indent=2) + "\n")
    raw = bytes(gdb.selected_inferior().read_memory(base, stride * 256))
    result = []
    for index in range(256):
        offset = index * stride
        if struct.unpack_from("<i", raw, offset + layouts["inUse"])[0] == 0:
            continue
        item = {"slot": index}
        for name, (format_, count) in fields.items():
            decoded = struct.unpack_from("<" + format_, raw, offset + layouts[name])
            item[name] = list(decoded) if count > 1 else decoded[0]
        result.append(item)
    return result


def save_records():
    (output / "lasers.json").write_text(json.dumps(records, indent=2) + "\n")
    if scout:
        (output / "scout-summary.json").write_text(json.dumps(scout_summary, indent=2) + "\n")


class Scout(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::BulletManager::OnDraw(th08::BulletManager*)", internal=True)

    def stop(self):
        global last_sampled_frame
        selected_stage, frame = current()
        progress(selected_stage, frame)
        if selected_stage != stage or frame > maximum:
            if scout_summary["sampledFrames"] and (selected_stage != stage or frame > maximum):
                scout_summary.update(stopReason="stage changed" if selected_stage != stage else "frame limit",
                                     complete=True)
                save_records()
                return True
            return False
        if frame > 0 and frame % sample_interval == 0 and frame != last_sampled_frame:
            last_sampled_frame = frame
            active = laser_state()
            scout_summary["sampledFrames"] += 1
            scout_summary["framesWithLasers"] += bool(active)
            if scout_summary["firstFrame"] is None:
                scout_summary["firstFrame"] = frame
            scout_summary["lastFrame"] = frame
            if active:
                records.append(dict(stage=stage, frame=frame, lasers=active))
                save_records()
        if frame == maximum:
            scout_summary.update(stopReason="frame limit", complete=True)
            save_records()
            return True
        return False


class Present(gdb.Breakpoint):
    def __init__(self):
        super().__init__("'(anonymous namespace)::LinuxDevice::Present'", internal=True)

    def stop(self):
        selected_stage, frame = current()
        progress(selected_stage, frame)
        if selected_stage == stage and frame in frames:
            gdb.execute("set $laser_device = this")
            Presented(internal=True)
        return False


text_fonts = []
quad_frames = {}


class Quad(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::AnmManager::AddSpriteToDrawBuffer(th08::VertexTex1DiffuseXyzrhw*)", internal=True)
        self.enabled = False

    def stop(self):
        selected_stage, frame = current()
        if selected_stage != stage or frame not in frames:
            raise RuntimeError("Portable quad observer was armed outside its selected frame")
        pointer = number("vertices")
        if value("sizeof(th08::VertexTex1DiffuseXyzrhw)") != 28:
            raise RuntimeError("Portable quad vertex stride differs")
        raw = bytes(gdb.selected_inferior().read_memory(pointer, 112))
        rows = quad_frames.setdefault(frame, [])
        if len(rows) >= 10000:
            raise RuntimeError("Portable quad capture exceeds its bounded frame capacity")
        rows.append([list(struct.unpack_from("<7I", raw, offset)) for offset in range(0, 112, 28)])
        return False


class ArmQuads(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::AnmManager::ClearVertexBuffer()", internal=True)

    def stop(self):
        selected_stage, frame = current()
        quad.enabled = selected_stage == stage and frame in frames
        return False


class TextFont(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::TextHelper::RenderTextToTextureBold", internal=True)

    def stop(self):
        selected_stage, frame = current()
        if selected_stage != stage or frame < 1:
            return False
        item = dict(stage=stage, frame=frame, bold=True)
        for name in ("xPos", "yPos", "spriteWidth", "spriteHeight", "fontHeight", "fontWidth", "textColor", "outlineType"):
            item[name] = number(name)
        pointer = number("string")
        raw = bytearray()
        for index in range(256):
            byte = bytes(gdb.selected_inferior().read_memory(pointer + index, 1))[0]
            if byte == 0:
                break
            raw.append(byte)
        else:
            raise RuntimeError("Portable text exceeds the diagnostic string bound")
        item.update(textCp932=raw.hex(), text=raw.decode("cp932"),
                    requestedGdiHeight=item["fontHeight"] * 2 - 2)
        text_fonts.append(item)
        (output / "text-fonts.json").write_text(json.dumps(text_fonts, indent=2) + "\n")
        return False


class Presented(gdb.FinishBreakpoint):
    def stop(self):
        frame = current()[1]
        records.append(dict(stage=stage, frame=frame, lasers=laser_state()))
        if settings.get("watchQuads"):
            quad.enabled = False
            (output / "quads.json").write_text(json.dumps([
                dict(stage=stage, frame=row["frame"], quads=quad_frames.get(row["frame"], []))
                for row in records], indent=2) + "\n")
        save_records()
        return True


def screenshot(frame):
    gdb.execute("set scheduler-locking off")
    gdb.execute("set $laser_access = (LinuxSurfaceAccess*)malloc(sizeof(LinuxSurfaceAccess))")
    gdb.execute("call th08_linux_surface_access($laser_device->backbuffer, $laser_access, true)")
    access = value("*$laser_access")
    width, height, pitch = (int(access[key]) for key in ("width", "height", "pitch"))
    if int(access["format"]) != 22 or (width, height, pitch) != (640, 480, 2560):
        raise RuntimeError("Unexpected laser capture framebuffer format")
    pixels = bytes(gdb.selected_inferior().read_memory(int(access["pixels"]), pitch * height))
    gdb.execute("call free($laser_access)")
    converter = ('from PIL import Image; import sys; Image.frombytes("RGB",(640,480),'
                 'sys.stdin.buffer.read(),"raw","BGRX",2560,1).save(sys.argv[1])')
    subprocess.run(["python3", "-c", converter, str(output / f"frame-{frame}.png")],
                   input=pixels, check=True, timeout=15)


class SkipLaser(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::AnmManager::Draw2D(th08::AnmVm*)", internal=True)

    def stop(self):
        selected_stage, frame = current()
        if selected_stage != stage or frame not in frames:
            return False
        base = int(value("th08::g_BulletManager.lasers").address)
        stride = value("th08::g_BulletManager.lasers").type.target().sizeof
        pointer = number("vm")
        if base <= pointer < base + 256 * stride and (pointer - base) % stride == 0:
            SuppressionExit(internal=True)
            suppression["active"] = True
        return False


suppression = dict(active=False, skippedQuads=0)


class SuppressionExit(gdb.FinishBreakpoint):
    def stop(self):
        suppression["active"] = False
        return False


class SkipQuad(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::AnmManager::AddSpriteToDrawBuffer(th08::VertexTex1DiffuseXyzrhw*)", internal=True)

    def stop(self):
        if suppression["active"]:
            suppression["skippedQuads"] += 1
            gdb.execute("return 0")
        return False


class ControlDrawExit(gdb.FinishBreakpoint):
    def stop(self):
        skip_laser.enabled = skip_quad.enabled = False
        return False


class ControlDraw(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::BulletManager::OnDraw(th08::BulletManager*)", internal=True)

    def stop(self):
        selected_stage, frame = current()
        if selected_stage == stage and frame in frames:
            skip_laser.enabled = skip_quad.enabled = True
            ControlDrawExit(internal=True)
        return False


for command in ("set pagination off", "set confirm off", "set breakpoint pending on",
                "set debuginfod enabled off", "set print thread-events off",
                "set direct-call-timeout 10", "set unwind-on-timeout on"):
    gdb.execute(command)
if scout:
    Scout()
else:
    Present()
    if settings.get("watchQuads"):
        quad = Quad()
        ArmQuads()
    if settings.get("watchTextFonts"):
        TextFont()
    if settings.get("suppressLaserBodies", False):
        skip_laser, skip_quad = SkipLaser(), SkipQuad()
        skip_laser.enabled = skip_quad.enabled = False
        ControlDraw()
gdb.execute("run")
if scout and not scout_summary["complete"]:
    raise RuntimeError("Laser scout stopped before its stage or frame boundary")
if not scout:
    while True:
        frame = current()[1]
        if frame not in frames:
            raise RuntimeError("Laser replay exited before every requested frame")
        screenshot(frame)
        frames.remove(frame)
        if not frames:
            break
        gdb.execute("continue")
save_records()
(output / "control.json").write_text(json.dumps(suppression, indent=2) + "\n")
gdb.execute("kill")
print("Laser scout completed" if scout else "All requested laser frames captured")
