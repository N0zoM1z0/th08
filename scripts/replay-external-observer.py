"""GDB observer for ordinary replay files. Gameplay memory is read-only.

Menu input uses XTest key events on the capture's isolated X display.
"""
import gdb
import json
import struct
import subprocess
import gzip
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from replay_file import valid_input_tail

output = Path.cwd().parent
settings = json.loads((output / "observer.json").read_text())
FIELDS = json.loads((output / "metadata.json").read_text())["schema"]["fields"]
stream = (output / "rows.jsonl").open("w")
menu_log = (output / "menu.jsonl").open("w")
events = (output / "rng-events.jsonl").open("w")
releases = (output / "effect-events.jsonl").open("w")
score_events = (output / "score-events.jsonl").open("w")
item_events = (output / "item-events.jsonl").open("w")
pool_stream = gzip.open(output / "effect-pool.jsonl.gz", "wt") if settings.get("watchEffectPool") else None
GM = settings["gameManager"]


def block(address, size):
    return bytes(gdb.selected_inferior().read_memory(address, size))


def read(address):
    return struct.unpack("<I", block(address, 4))[0]


def snapshot():
    state = {name: block(settings[name], size).hex() for name, size in (
        ("player", 0xe2b30), ("spellcard", 0x2644), ("bulletManager", 0x6ba578),
        ("enemyManager", 0x9dcf10), ("itemManager", 0x17b094),
        ("effectManager", 0x8b05c), ("background", 0x6600)
    )}
    if hasattr(gdb, "execute"):
        state["floatingPointState"] = gdb.execute("info float", to_string=True)
    if settings.get("normalizeDispatch"):
        state["normalizeImplementation"] = hex(read(settings["normalizeDispatch"]))
    return state


for address_key, bytes_key in (("attestAddress", "attestBytes"),
                               ("completionAttestAddress", "completionAttestBytes")):
    expected = bytes.fromhex(settings[bytes_key])
    if block(settings[address_key], len(expected)) != expected:
        raise RuntimeError("Mapped replay observer instructions differ from the executable")
for attestation in settings["extraAttestations"]:
    expected = bytes.fromhex(attestation["bytes"])
    if block(attestation["address"], len(expected)) != expected:
        raise RuntimeError("Mapped stage initialization/completion instructions differ from the executable")


class Observer(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['boundary']:08X}", gdb.BP_HARDWARE_BREAKPOINT)
        self.count = 0
        self.stage = None
        self.stages = []
        self.stage_frames = {}
        self.rng_origins = {}
        self.end_scores = {}
        self.menu_tick = 0
        self.key = None
        self.key_until = 0
        self.next_key = 0
        self.menu_state = None
        self.completion_observer = None
        self.previous_rows = []
        self.reference_stream = None
        self.diagnostic_breakpoints = []
        if settings.get("referenceTrace"):
            path = Path(settings["referenceTrace"]) / "rows.jsonl"
            self.reference_stream = (path.open() if path.exists() else gzip.open(str(path) + ".gz", "rt"))

    def finish(self, error=None):
        stream.close()
        menu_log.close()
        score_events.close()
        item_events.close()
        if pool_stream:
            pool_stream.close()
        if self.reference_stream:
            if not error and self.reference_stream.readline():
                error = "Candidate ended before the retail trace"
            self.reference_stream.close()
        expected = [stage["index"] for stage in settings["replay"]["stages"]
                    if stage["index"] >= settings["startStage"]]
        errors = [error] if error else []
        if self.stages != expected:
            errors.append(f"Stage sequence {self.stages} differs from {expected}")
        for stage in settings["replay"]["stages"]:
            key = str(stage["index"])
            if stage["index"] not in self.stages or key not in self.stage_frames:
                continue
            remaining = stage["inputRecords"] - self.stage_frames[key]
            if not valid_input_tail(remaining, stage["index"] == expected[-1]):
                errors.append(f"Stage {key} ended with {remaining} unconsumed input records")
            if self.end_scores.get(key) != stage["endScore"]:
                errors.append(f"Stage {key} score differs from the original recording")
        (output / "complete.json").write_text(json.dumps(dict(
            rows=self.count, ended=not errors, errors=errors, stages=self.stages,
            stageFrames=self.stage_frames, endScores=self.end_scores, rngOrigins=self.rng_origins)) + "\n")
        self.enabled = False
        self.completion_observer.enabled = False
        begin_observer.enabled = False
        delete_observer.enabled = False

    def menu(self):
        self.menu_tick += 1
        if self.key and self.menu_tick >= self.key_until:
            subprocess.run(["xdotool", "keyup", self.key], check=True)
            self.key = None
        if self.key or self.menu_tick < self.next_key:
            return
        title = read(settings["titleScreen"])
        if not title:
            return
        screen, timer, _, state = struct.unpack("<4I", block(title + 0x14428, 16))
        cursor, _, _, substate, sub_timer = struct.unpack("<5I", block(title, 20))
        current = (screen, substate, cursor)
        if current != self.menu_state:
            menu_log.write(json.dumps(dict(tick=self.menu_tick, screen=screen, state=substate,
                                           cursor=cursor, timer=sub_timer)) + "\n")
            menu_log.flush()
            self.menu_state = current
        if state or sub_timer < 12:
            return
        key = None
        if screen == 0 and substate == 1:
            key = "z" if cursor == 4 else "Down"
        elif screen == 7:
            if substate == 1:
                if not read(title + 0xc288):
                    raise RuntimeError("Retail Replay menu rejected the supplied fixture")
                key = "z"
            elif substate == 2:
                key = "z" if cursor == settings["startStage"] else "Down"
            elif substate == 3:
                key = "z" if cursor == settings["playbackMode"] else "Down"
        if key:
            # Xvfb is dedicated to this capture, so its only game window owns focus.
            subprocess.run(["xdotool", "keydown", key], check=True)
            self.key = key
            self.key_until = self.menu_tick + 3
            self.next_key = self.menu_tick + 12

    def stop(self, completion=False):
        try:
            if completion:
                if self.stage is None:
                    self.menu()
                elif not read(settings["replayManager"]):
                    self.finish()
                return False
            manager = read(settings["replayManager"])
            if not manager:
                return False
            gm = block(GM + 0x3dba8, 0x220)
            flags = struct.unpack_from("<I", gm, 4)[0]
            if not flags & 8 or flags & 2:
                return False
            stage = struct.unpack_from("<I", gm, 0x21c)[0]
            if stage != self.stage:
                if stage in self.stages:
                    raise RuntimeError("Replay revisited an already captured stage")
                self.stage = stage
                self.stages.append(stage)
                if str(stage) not in self.rng_origins:
                    raise RuntimeError("Stage began without its initialization callback")
                self.completion_observer.enabled = stage == settings["replay"]["stages"][-1]["index"]
                begin_observer.enabled = False
                if self.key:
                    subprocess.run(["xdotool", "keyup", self.key], check=True)
                    self.key = None
            frame = read(manager)
            if pool_stream:
                data = block(settings["effectManager"], 0x8b05c)
                slots = bytes(value for slot in range(512) for value in
                              data[0x1c + slot * 0x360 + 0x350:0x1c + slot * 0x360 + 0x352])
                pool_stream.write(json.dumps(dict(stage=stage, frame=frame,
                    nextIndex=struct.unpack_from("<I", data)[0], activeCount=struct.unpack_from("<I", data, 8)[0],
                    slots=slots.hex())) + "\n")
            if self.stage_frames.get(str(stage)) == frame:
                begin_observer.enabled = True
            if settings.get("detailStart") and stage == settings["detailStage"]:
                if frame == settings["detailStart"] - 1:
                    if settings.get("watchNormalization"):
                        self.diagnostic_breakpoints = [NormalizationObserver()]
                    elif settings.get("watchScore"):
                        self.diagnostic_breakpoints = [ScoreObserver(), ItemSpawnObserver()]
                    elif not (settings.get("watchItems") or settings.get("watchEnemies")):
                        self.diagnostic_breakpoints = [RngObserver(), ReleaseObserver()]
                if frame == settings["detailEnd"]:
                    with gzip.open(output / "window-state.json.gz", "wt") as state_stream:
                        json.dump(snapshot(), state_stream)
                    for handle in (stream, menu_log, events, releases, score_events, item_events):
                        handle.close()
                    if pool_stream:
                        pool_stream.close()
                    (output / "complete.json").write_text(json.dumps(dict(rows=self.count, ended=False,
                        diagnosticComplete=True, stage=stage, frame=frame, errors=[])) + "\n")
                    self.enabled = False
                    for breakpoint in (begin_observer, delete_observer, self.completion_observer):
                        breakpoint.enabled = False
                    for breakpoint in self.diagnostic_breakpoints:
                        breakpoint.enabled = False
                    return False
            recorded = next(s for s in settings["replay"]["stages"] if s["index"] == stage)
            if settings.get("watchEnemies") and stage == settings["detailStage"] and settings["detailStart"] <= frame < settings["detailEnd"]:
                for slot in settings["watchEnemies"]:
                    raw = block(settings["enemyManager"] + (slot + 1) * 0x53d0, 0x53d0)
                    motion = dict(stage=stage, frame=frame, slot=slot, flags1=read(settings["enemyManager"] + (slot + 1) * 0x53d0 + 0x3324))
                    for name, offset, count in (("position", 0x2d34, 3), ("positionOffset", 0x2d40, 3),
                        ("velocity", 0x2d4c, 3), ("previousPosition", 0x2d58, 3),
                        ("worldPosition", 0x2d88, 3), ("movementAngle", 0x2d94, 1),
                        ("angularVelocity", 0x2d98, 1), ("speed", 0x2da8, 1), ("acceleration", 0x2dac, 1)):
                        motion[name] = list(struct.unpack_from("<" + "I" * count, raw, offset))
                    with (output / "enemy-motion.jsonl").open("a") as handle:
                        handle.write(json.dumps(motion) + "\n")
            if settings.get("watchItems") and stage == settings["detailStage"] and settings["detailStart"] <= frame < settings["detailEnd"]:
                for slot in settings["watchItems"]:
                    raw = block(settings["itemManager"] + slot * 0x2e4, 0x2e4)
                    motion = dict(stage=stage, frame=frame, slot=slot,
                        position=list(struct.unpack_from("<3I", raw, 0x2a4)),
                        velocity=list(struct.unpack_from("<3I", raw, 0x2b0)),
                        timer=list(struct.unpack_from("<3I", raw, 0x2c8)), flags=list(raw[0x2d4:0x2d9]),
                        player=list(struct.unpack("<3I", block(settings["player"] + 0x2b4, 12))))
                    with (output / "item-motion.jsonl").open("a") as handle:
                        handle.write(json.dumps(motion) + "\n")
            if frame > recorded["inputRecords"]:
                raise RuntimeError(f"Stage {stage} read beyond its recorded input at frame {frame}")
            globals_data = block(read(GM + 8), 0xa0)
            player = block(settings["player"] + 0x2b4, 12)
            rng = block(settings["rng"], 8)
            u32 = lambda offset: struct.unpack_from("<I", globals_data, offset)[0]
            row = [self.count + 1, stage, frame, struct.unpack_from("<I", gm, 0x218)[0],
                   u32(8), u32(12), *[u32(offset) for offset in (0x64, 0x74, 0x80, 0x98)],
                   *struct.unpack("<3I", player), block(settings["player"], 1)[0],
                   struct.unpack_from("<H", rng)[0],
                   (struct.unpack_from("<I", rng, 4)[0] - self.rng_origins[str(stage)]) & 0xffffffff,
                   struct.unpack("<H", block(settings["input"], 2))[0], u32(0x30), u32(0x3c),
                   struct.unpack_from("<h", globals_data, 0x22)[0] & 0xffffffff,
                   globals_data[0x28], u32(0x24), gm[0], gm[1], read(GM + 0x30)]
            stream.write(json.dumps(row) + "\n")
            self.count += 1
            self.stage_frames[str(stage)] = frame
            self.end_scores[str(stage)] = row[4]
            if self.reference_stream:
                line = self.reference_stream.readline()
                if not line:
                    raise RuntimeError("Candidate continued beyond the retail trace")
                expected = json.loads(line)
                if expected != row:
                    difference = dict(calc=row[0], stage=stage, frame=frame,
                        differingFields=[field for field, a, b in zip(FIELDS, expected, row) if a != b],
                        reference=dict(zip(FIELDS, expected)),
                        candidate=dict(zip(FIELDS, row)), previous=self.previous_rows)
                    (output / "first-difference.json").write_text(json.dumps(difference, indent=2) + "\n")
                    # Preserve the relevant object state at the first differing frame.
                    # Raw pointers remain image-specific; compare positions/flags by slot.
                    state = snapshot()
                    state.update(globals=globals_data.hex(), gameManager=gm.hex(), rng=rng.hex())
                    with gzip.open(output / "first-difference-state.json.gz", "wt") as state_stream:
                        json.dump(state, state_stream)
                    raise RuntimeError("Replay diverged; see first-difference.json")
                self.previous_rows = (self.previous_rows + [dict(zip(FIELDS, row))])[-2:]
            if self.count % 120 == 0:
                stream.flush()
        except Exception as error:
            self.finish(str(error))
        return False


class CompletionObserver(gdb.Breakpoint):
    def __init__(self, observer):
        super().__init__(f"*0x{settings['completionBoundary']:08X}", gdb.BP_HARDWARE_BREAKPOINT)
        self.observer = observer

    def stop(self):
        return self.observer.stop(completion=True)


observer = Observer()
observer.completion_observer = CompletionObserver(observer)


class BeginStageObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['stageStartBoundary']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        manager = read(settings["replayManager"])
        if manager and not read(manager):
            stage = read(GM + 0x3ddc4)
            observer.rng_origins[str(stage)] = read(settings["rng"] + 4)
        return False


class DeleteObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['deleteReplay']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        if observer.stage is not None:
            observer.finish()
        return False


begin_observer = BeginStageObserver()
delete_observer = DeleteObserver()


class NormalizationObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['normalizationBoundary']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        bp = int(gdb.parse_and_eval("$ebp"))
        effect = read(bp - 0x44)
        camera = settings["background"] + 0x6394
        record = dict(stage=read(GM + 0x3ddc4), frame=read(read(settings["replayManager"])),
                      slot=(effect - settings["effectManager"] - 0x1c) // 0x360,
                      position=struct.unpack("<3I", block(effect + 0x2a4, 12)),
                      camera=struct.unpack("<3I", block(camera, 12)),
                      normalized=struct.unpack("<3I", block(bp - 0xc, 12)),
                      forward=struct.unpack("<3I", block(camera + 36, 12)))
        with (output / "normalization-events.jsonl").open("a") as handle:
            handle.write(json.dumps(record) + "\n")
        return False


class RngObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['randomU16']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        sp = int(gdb.parse_and_eval("$esp"))
        bp = int(gdb.parse_and_eval("$ebp"))
        callers = [hex(read(sp))]
        vm_info = None
        for _ in range(5):
            if not bp:
                break
            try:
                caller = read(bp + 4)
                callers.append(hex(caller))
                next_bp = read(bp)
                if settings["anmExecute"] <= caller < settings["anmExecute"] + 0x366d:
                    vm = read(next_bp + 8)
                    kind, slot, offset = "other", 0, vm
                    for name, base, stride, count in (
                        ("bullet", settings["bulletManager"] + 0x1a880, 0x10b8, 1536),
                        ("effect", settings["effectManager"] + 0x1c, 0x360, 654),
                        ("enemy", settings["enemyManager"] + 0x53d0, 0x53d0, 480),
                        ("player", settings["player"], 0xe2b30, 1),
                    ):
                        if base <= vm < base + stride * count:
                            kind = name
                            slot, offset = divmod(vm - base, stride)
                            break
                    vm_info = dict(kind=kind, slot=slot, offset=offset,
                        file=int.from_bytes(block(vm + 0x216, 2), "little", signed=True),
                        script=int.from_bytes(block(vm + 0x21a, 2), "little", signed=True),
                        instructionOffset=read(vm + 0x220) - read(vm + 0x21c))
                if next_bp <= bp or next_bp > bp + 0x100000:
                    break
                bp = next_bp
            except gdb.MemoryError:
                break
        events.write(json.dumps(dict(stage=read(GM + 0x3ddc4), frame=read(read(settings["replayManager"])),
                                     callers=callers, vm=vm_info)) + "\n")
        return False


class ItemSpawnObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['spawnItem']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        sp = int(gdb.parse_and_eval("$esp"))
        caller, position, kind, state = struct.unpack("<4I", block(sp, 16))
        item_events.write(json.dumps(dict(stage=read(GM + 0x3ddc4),
            frame=read(read(settings["replayManager"])), caller=hex(caller),
            kind=kind, state=state, position=struct.unpack("<3f", block(position, 12)))) + "\n")
        return False


class ScoreObserver(gdb.Breakpoint):
    def __init__(self):
        self.address = read(GM + 8) + 8
        self.previous = read(self.address)
        super().__init__(f"*(unsigned int*)0x{self.address:08X}", gdb.BP_WATCHPOINT, wp_class=gdb.WP_WRITE)

    def stop(self):
        score = read(self.address)
        pc = int(gdb.parse_and_eval("$eip"))
        bp = int(gdb.parse_and_eval("$ebp"))
        callers = []
        for _ in range(6):
            if not bp:
                break
            callers.append(hex(read(bp + 4)))
            next_bp = read(bp)
            if next_bp <= bp or next_bp > bp + 0x100000:
                break
            bp = next_bp
        score_events.write(json.dumps(dict(stage=read(GM + 0x3ddc4),
            frame=read(read(settings["replayManager"])), pc=hex(pc), callers=callers,
            score=score, delta=score - self.previous)) + "\n")
        score_events.flush()
        self.previous = score
        return False


class ReleaseObserver(gdb.Breakpoint):
    def __init__(self):
        super().__init__(f"*0x{settings['releaseEffects']:08X}", gdb.BP_HARDWARE_BREAKPOINT)

    def stop(self):
        enemy = int(gdb.parse_and_eval("$ecx"))
        sp = int(gdb.parse_and_eval("$esp"))
        count = read(enemy + 0x53c0)
        releases.write(json.dumps(dict(stage=read(GM + 0x3ddc4), frame=read(read(settings["replayManager"])),
                                      caller=hex(read(sp)), enemy=(enemy - settings["enemyManager"] - 0x53d0) // 0x53d0,
                                      count=count)) + "\n")
        return False
