"""Loaded by GDB, through capture-replay.py. Reads process state only."""
import gdb
import json
import struct
from pathlib import Path

output = Path.cwd().parent
stream = (output / "rows.jsonl").open("w")
detail_stream = (output / "items.jsonl").open("w")
score_stream = (output / "score-events.jsonl").open("w")
rng_stream = (output / "rng-events.jsonl").open("w")
effect_stream = (output / "effects.jsonl").open("w")
effect_event_stream = (output / "effect-events.jsonl").open("w")
settings = json.loads((output / "observer.json").read_text())
GM = settings["gameManager"]
PLAYER = settings["player"]
RNG = settings["rng"]
seen_effect_slots = {}


def read(address, size=4, signed=False):
    return int.from_bytes(gdb.selected_inferior().read_memory(address, size), "little", signed=signed)


def capture_effects(watched_only=False):
    effects = []
    data = bytes(gdb.selected_inferior().read_memory(settings["effectManager"] + 0x1c, 654 * 0x360))
    for index in range(654):
        base = index * 0x360
        effect_id = data[base + 0x351]
        if effect_id in settings.get("watchEffects", []) and data[base + 0x350]:
            seen_effect_slots[index] = effect_id
        elif seen_effect_slots.get(index) != effect_id:
            seen_effect_slots.pop(index, None)
        # ID 0 also occurs in never-used, zero-initialized slots.
        watched = index in seen_effect_slots
        if not (watched if watched_only else data[base + 0x350] or watched):
            continue
        beginning, current = struct.unpack_from("<II", data, base + 0x21c)
        effects.append(dict(slot=index, id=data[base + 0x351], active=data[base + 0x350],
                            position=list(struct.unpack_from("<3f", data, base + 0x2a4)),
                            timer=struct.unpack_from("<i", data, base + 0x340)[0],
                            releaseRequested=data[base + 0x352], releaseTimer=data[base + 0x353],
                            anmTime=struct.unpack_from("<i", data, base + 0x40)[0],
                            flags=struct.unpack_from("<I", data, base + 0x1f8)[0],
                            pendingInterrupt=struct.unpack_from("<h", data, base + 0x1fe)[0],
                            instructionOffset=current - beginning if current else None,
                            file=struct.unpack_from("<h", data, base + 0x216)[0],
                            script=struct.unpack_from("<h", data, base + 0x21a)[0]))
    return effects


def attached_effects(enemy):
    count = read(enemy + 0x53c0)
    if count > 24:
        raise RuntimeError("Invalid attached-effect count")
    return [(read(enemy + 0x5360 + index * 4) - settings["effectManager"] - 0x1c) // 0x360
            if read(enemy + 0x5360 + index * 4) else None for index in range(count)]


def capture_effect_owners():
    owners = []
    for slot in range(8):
        enemy = read(settings["enemyManager"] + 0x9dcda0 + slot * 4)
        if enemy:
            owners.append(dict(boss=slot, enemySlot=(enemy - settings["enemyManager"] - 0x53d0) // 0x53d0,
                               attachedSlots=attached_effects(enemy), flags=read(enemy + 0x3324)))
    return owners


# Attest the frame callback and the Render completion boundary before trapping.
for prefix in ("", "completion"):
    address_key = "attestAddress" if not prefix else "completionAttestAddress"
    bytes_key = "attestBytes" if not prefix else "completionAttestBytes"
    expected = bytes.fromhex(settings[bytes_key])
    if bytes(gdb.selected_inferior().read_memory(settings[address_key], len(expected))) != expected:
        raise RuntimeError("Mapped TH08 capture bytes do not match the supplied executable")


class Observer(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['boundary']:08X}", gdb.BP_HARDWARE_BREAKPOINT)
        self.count = 0
        self.last_frame = 0
        self.score_observer = None
        self.rng_observer = None
        self.demo = None
        self.completed_demos = []
        self.completion_observer = None

    def stop(self, completion_only=False):
        try:
            return self.capture_frame(completion_only)
        except Exception as error:
            for handle in (stream, detail_stream, score_stream, rng_stream, effect_stream, effect_event_stream):
                if not handle.closed:
                    handle.close()
            (output / "complete.json").write_text(json.dumps(dict(
                rows=self.count, lastFrame=self.last_frame, ended=False, errors=[str(error)])) + "\n")
            self.enabled = False
            if self.completion_observer:
                self.completion_observer.enabled = False
            return False

    def capture_frame(self, completion_only=False):
        flags = read(GM + 0x3DBAC)
        frame = read(GM + 0x3DBB8)
        if not flags & 8 or not frame:
            if self.demo is not None:
                if self.last_frame != settings["demoEndFrames"][str(self.demo)]:
                    raise RuntimeError(f"Demo {self.demo} ended early at frame {self.last_frame}")
                self.completed_demos.append(self.demo)
                self.demo = None
            if self.completed_demos == settings.get("demoIndexes", [0]):
                stream.close()
                detail_stream.close()
                score_stream.close()
                rng_stream.close()
                effect_stream.close()
                effect_event_stream.close()
                (output / "complete.json").write_text(json.dumps(dict(
                    rows=self.count, lastFrame=self.last_frame, completedDemos=self.completed_demos, ended=True)) + "\n")
                self.enabled = False
                self.completion_observer.enabled = False
            return False
        if completion_only and frame == self.last_frame:
            return False
        demo = read(GM + 0x3DBB4, 1)
        if demo not in settings.get("demoIndexes", [0]) or demo in self.completed_demos:
            return False
        # At the terminal frame GameManager breaks the chain before the
        # playback-control callback. Read that exit state after RunCalcChain.
        if completion_only and frame != settings["demoEndFrames"][str(demo)]:
            raise RuntimeError(f"Playback-control callback missed frame {frame} of demo {demo}")
        if self.demo is None:
            if frame != 1 or not flags & 2:
                raise RuntimeError("Expected frame 1 of a bundled demo")
            if demo == 0 and read(GM + 0x3DDC4) != 5:
                raise RuntimeError("Expected Stage 5 for demo 0")
            self.demo = demo
            self.last_frame = 0
            seen_effect_slots.clear()
        if frame != self.last_frame + 1:
            raise RuntimeError(f"Missing or duplicated logical frame: expected {self.last_frame + 1}, observed {frame}")
        globals_address = read(GM + 8)
        row = [frame, read(GM + 0x3DDC4), read(GM + 0x3DBB4, 1), read(GM + 0x3DDC0),
               read(globals_address + 8), read(globals_address + 12)]
        row += [read(globals_address + offset) for offset in (0x64, 0x74, 0x80, 0x98)]
        row += [read(PLAYER + offset) for offset in (0x2B4, 0x2B8, 0x2BC)]
        row += [read(PLAYER, 1), read(RNG, 2), read(RNG + 4), read(settings["input"], 2),
                read(globals_address + 0x30), read(globals_address + 0x3C),
                read(globals_address + 0x22, 2, signed=True) & 0xffffffff,
                read(globals_address + 0x28, 1), (flags >> 1) & 1]
        stream.write(json.dumps(row) + "\n")
        if settings.get("watchEffects"):
            effect_stream.write(json.dumps(dict(frame=frame, demo=demo, effects=capture_effects(True),
                                                owners=capture_effect_owners())) + "\n")
        if settings["detailStart"] and frame == settings["detailStart"]:
            self.score_observer = ScoreObserver()
            self.rng_observer = RngObserver()
        if self.score_observer and frame >= settings["detailEnd"]:
            self.score_observer.enabled = False
            self.rng_observer.enabled = False
        if settings["detailStart"] and settings["detailStart"] <= frame <= settings["detailEnd"]:
            # Item slots retain allocation identity across the two PE images.
            # Item layout: 0x2e4; position +0x2a4, type +0x2d4, next +0x2dc.
            manager = settings["itemManager"]
            item = read(manager + 2097 * 0x2e4 + 8 + 0x2dc)
            items = []
            visited = set()
            while item:
                if item in visited or len(visited) > 2097:
                    raise RuntimeError("Invalid item list")
                visited.add(item)
                data = bytes(gdb.selected_inferior().read_memory(item + 0x2a4, 0x40))
                items.append(dict(slot=(item - manager) // 0x2e4,
                                  position=list(struct.unpack_from("<3f", data)),
                                  velocity=list(struct.unpack_from("<3f", data, 12)),
                                  timer=struct.unpack_from("<i", data, 0x2c)[0],
                                  type=data[0x30], state=data[0x33], maxValue=data[0x34]))
                item = struct.unpack_from("<I", data, 0x38)[0]
            shots = []
            data = bytes(gdb.selected_inferior().read_memory(PLAYER + 0xbe838, 128 * 0x484))
            for index in range(128):
                base = index * 0x484
                state = struct.unpack_from("<H", data, base + 0x462)[0]
                if state:
                    shots.append(dict(slot=index, state=state, type=struct.unpack_from("<h", data, base + 0x464)[0],
                                      position=list(struct.unpack_from("<3f", data, base + 0x2a4)),
                                      velocity=list(struct.unpack_from("<3f", data, base + 0x43c)),
                                      angle=struct.unpack_from("<f", data, base + 0x450)[0]))
            enemies = []
            manager = settings["enemyManager"]
            for group in range(4):
                enemy = read(manager + 0x9dcedc + group * 4)
                seen = set()
                while enemy:
                    if enemy in seen or len(seen) > 480:
                        raise RuntimeError("Invalid enemy draw list")
                    seen.add(enemy)
                    enemies.append(dict(slot=(enemy - manager - 0x53d0) // 0x53d0,
                                        position=list(struct.unpack("<3f", gdb.selected_inferior().read_memory(enemy + 0x2d88, 12))),
                                        life=read(enemy + 0x2dfc, signed=True)))
                    enemy = read(enemy)
            regions = []
            data = bytes(gdb.selected_inferior().read_memory(PLAYER + 0xb8834, 384 * 0x40))
            for index in range(384):
                region = data[index * 0x40:(index + 1) * 0x40]
                if region[0x3c]:
                    regions.append(dict(slot=index, bytes=region.hex()))
            bomb = PLAYER + 0xfdc
            work_items = []
            for index in range(128):
                item = bomb + 0x4c + index * 0x16f0
                if read(item):
                    work_items.append(dict(slot=index, state=read(item), timer=read(item + 0x16e4),
                                           position=list(struct.unpack("<3f", gdb.selected_inferior().read_memory(item + 0x14, 12))),
                                           motion=list(struct.unpack("<3f", gdb.selected_inferior().read_memory(item + 0x1a0, 12))),
                                           angle=read(item + 0x10), speed=read(item + 0xc)))
            bullets = []
            data = bytes(gdb.selected_inferior().read_memory(settings["bulletManager"] + 0x1a880, 1536 * 0x10b8))
            for index in range(1536):
                base = index * 0x10b8
                state = struct.unpack_from("<H", data, base + 0xdb8)[0]
                if state:
                    bullets.append(dict(slot=index, state=state,
                                        position=list(struct.unpack_from("<3f", data, base + 0xd44)),
                                        velocity=list(struct.unpack_from("<3f", data, base + 0xd50)),
                                        angle=struct.unpack_from("<f", data, base + 0xd74)[0],
                                        grazed=data[base + 0xdbd]))
            effects = capture_effects()
            detail_stream.write(json.dumps(dict(frame=frame, demo=demo, pointValue=read(globals_address + 0x24),
                                               fpuControl=int(gdb.parse_and_eval("$fctrl")),
                                               bombState=[read(bomb + offset) for offset in (0, 4, 8, 0x20)],
                                               workItems=work_items, regions=regions, bullets=bullets,
                                               effects=effects,
                                               items=items, shots=shots, enemies=enemies)) + "\n")
        self.count += 1
        self.last_frame = frame
        if self.count % 120 == 0:
            stream.flush()
            detail_stream.flush()
            score_stream.flush()
            rng_stream.flush()
            effect_stream.flush()
            effect_event_stream.flush()
        return False


class ScoreObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['addScore']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        sp = int(gdb.parse_and_eval("$esp"))
        score_stream.write(json.dumps(dict(frame=read(GM + 0x3dbb8), demo=read(GM + 0x3dbb4, 1), scoreArgument=read(sp + 4, signed=True),
                                           caller=hex(read(sp)), scoreBefore=read(read(GM + 8) + 8))) + "\n")
        return False


class RngObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['randomU16']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        sp = int(gdb.parse_and_eval("$esp"))
        bp = int(gdb.parse_and_eval("$ebp"))
        callers = [hex(read(sp))]
        vm_info = None
        for _ in range(3):
            if not bp:
                break
            try:
                caller = read(bp + 4)
                callers.append(hex(caller))
                next_bp = read(bp)
                anm = settings.get("anmExecute", 0)
                if anm and anm <= caller < anm + 0x366d:
                    vm = read(next_bp + 8)
                    kind, slot, offset = "other", 0, vm
                    for name, base, stride, count in (
                        ("bullet", settings["bulletManager"] + 0x1a880, 0x10b8, 1536),
                        ("effect", settings["effectManager"] + 0x1c, 0x360, 654),
                        ("enemy", settings["enemyManager"] + 0x53d0, 0x53d0, 480),
                        ("player", PLAYER, 0xe2b2c, 1),
                    ):
                        if base <= vm < base + stride * count:
                            kind = name
                            slot, offset = divmod(vm - base, stride)
                            break
                    vm_info = dict(kind=kind, slot=slot, offset=offset,
                                   file=read(vm + 0x216, 2, signed=True), script=read(vm + 0x21a, 2, signed=True),
                                   instructionOffset=read(vm + 0x220) - read(vm + 0x21c))
                if next_bp <= bp or next_bp > bp + 0x100000:
                    break
                bp = next_bp
            except gdb.MemoryError:
                break
        rng_stream.write(json.dumps(dict(frame=read(GM + 0x3dbb8), demo=read(GM + 0x3dbb4, 1),
                                         seed=read(RNG, 2), generation=read(RNG + 4), callers=callers, vm=vm_info)) + "\n")
        return False


class CompletionObserver(gdb.Breakpoint):
    def __init__(self, observer):
        super().__init__(f"*0x{settings['completionBoundary']:08X}", gdb.BP_HARDWARE_BREAKPOINT)
        self.observer = observer

    def stop(self):
        return self.observer.stop(completion_only=True)


class EffectReleaseObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['releaseEffects']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        if effect_event_stream.closed or not read(GM + 0x3dbac) & 8:
            return False
        demo = read(GM + 0x3dbb4, 1)
        if demo not in settings.get("demoIndexes", [0]):
            return False
        enemy = int(gdb.parse_and_eval("$ecx"))
        sp = int(gdb.parse_and_eval("$esp"))
        effect_event_stream.write(json.dumps(dict(frame=read(GM + 0x3dbb8), demo=demo, caller=hex(read(sp)),
                                                 enemySlot=(enemy - settings["enemyManager"] - 0x53d0) // 0x53d0,
                                                 attachedSlots=attached_effects(enemy))) + "\n")
        return False


observer = Observer()
observer.completion_observer = CompletionObserver(observer)
if settings.get("watchEffects"):
    EffectReleaseObserver()
