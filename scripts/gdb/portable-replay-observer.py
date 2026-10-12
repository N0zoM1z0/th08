"""Read portable replay state at the retail schema's calculation boundary."""
import gzip
import json
import os
from pathlib import Path
import struct

import gdb

output = Path(os.environ["TH08_PORTABLE_CAPTURE"])
settings = json.loads((output / "settings.json").read_text())
fields = json.loads((output / "metadata.json").read_text())["schema"]["fields"]
reference = gzip.open(Path(settings["reference"]) / "rows.jsonl.gz", "rt")
reference_origin = 0
while True:
    first = json.loads(reference.readline())
    if first[1] == settings["startStage"]:
        reference_origin = first[0] - 1
        break
pending_reference = first
stream = gzip.open(output / "rows.jsonl.gz", "wt", compresslevel=6)
pool_stream = gzip.open(output / "effect-pool.jsonl.gz", "wt") if settings.get("watchEffectPool") else None


def address(expression):
    return int(gdb.parse_and_eval(expression).address)


def offsets(name):
    return {field.name: field.bitpos // 8 for field in gdb.lookup_type(name).fields()}


def block(location, size):
    return bytes(gdb.selected_inferior().read_memory(location, size))


def u32(data, offset=0):
    return struct.unpack_from("<I", data, offset)[0]


gm = address("th08::g_GameManager")
gm_offsets = offsets("th08::GameManager")
globals_offsets = offsets("th08::ZunGlobals")
player_offsets = offsets("th08::Player")
rng_offsets = offsets("th08::Rng")
player = address("th08::g_Player")
rng = address("th08::g_Rng")
manager_pointer = address("th08::g_ReplayManager")
input_address = address("th08::g_GuiMessageInputCurrent")
pointer_format = "<Q" if gdb.lookup_type("void").pointer().sizeof == 8 else "<I"
pointer_size = struct.calcsize(pointer_format)
gm_begin = gm_offsets["character"]
gm_end = gm_offsets["currentStage"] + 4


def pointer(location):
    return struct.unpack(pointer_format, block(location, pointer_size))[0]


def save_state(name):
    snapshot = {}
    for symbol in ("g_Player", "g_Spellcard", "g_BulletManager", "g_EnemyManager", "g_ItemManager", "g_EffectManager", "g_Background"):
        value = gdb.parse_and_eval("th08::" + symbol)
        snapshot[symbol] = block(int(value.address), value.type.sizeof).hex()
    snapshot.update(globals=block(pointer(gm + gm_offsets["globals"]), gdb.lookup_type("th08::ZunGlobals").sizeof).hex(),
                    gameManager=block(gm + gm_begin, gm_end - gm_begin).hex(),
                    rng=block(rng, gdb.lookup_type("th08::Rng").sizeof).hex())
    snapshot["floatingPointState"] = gdb.execute("info float", to_string=True)
    snapshot["sseControl"] = str(gdb.parse_and_eval("$mxcsr"))
    snapshot["layouts"] = {}
    for type_name in ("Player", "PlayerShot", "PlayerCollisionRegion", "Enemy", "EnemyManager", "Item", "ItemManager",
                      "Bullet", "BulletManager", "Effect", "EffectManager", "Background", "BackgroundCamera", "AnmVm", "AnmVmBase"):
        try:
            snapshot["layouts"][type_name] = [dict(name=field.name, offset=field.bitpos // 8,
                bytes=field.type.sizeof, type=str(field.type)) for field in gdb.lookup_type("th08::" + type_name).fields()]
        except gdb.error:
            pass
    with gzip.open(output / name, "wt") as handle:
        json.dump(snapshot, handle)


class Observer(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::ReplayManager::ControlPlaybackFrameAdvance", internal=True)
        self.count = 0
        self.stages = []
        self.stage_frames = {}
        self.end_scores = {}
        self.rng_origins = {}
        self.previous = []
        self.finished = False

    def finish(self, error=None, diagnostic=False):
        if self.finished:
            return True
        self.finished = True
        errors = [error] if error else []
        if diagnostic and pool_stream:
            save_state("window-state.json.gz")
        expected = [entry["index"] for entry in settings["replay"]["stages"]]
        if not diagnostic and not error:
            if reference.readline():
                errors.append("Portable playback ended before the retail trace")
            if self.stages != expected:
                errors.append(f"Stage sequence {self.stages} differs from {expected}")
            import sys
            sys.path.insert(0, str(Path(os.environ["TH08_PORTABLE_REPOSITORY"]) / "scripts"))
            from replay_file import valid_input_tail
            for entry in settings["replay"]["stages"]:
                key = str(entry["index"])
                remaining = entry["inputRecords"] - self.stage_frames.get(key, 0)
                if not valid_input_tail(remaining, entry["index"] == expected[-1]):
                    errors.append(f"Stage {key} has {remaining} unconsumed input records")
                if self.end_scores.get(key) != entry["endScore"]:
                    errors.append(f"Stage {key} score differs from the recording")
        stream.close()
        reference.close()
        if pool_stream:
            pool_stream.close()
        (output / "complete.json").write_text(json.dumps(dict(
            rows=self.count, ended=not errors and not diagnostic, diagnosticComplete=diagnostic,
            errors=errors, stages=self.stages, stageFrames=self.stage_frames,
            endScores=self.end_scores, rngOrigins=self.rng_origins)) + "\n")
        print(f"Portable observer stopped after {self.count} calculations: " +
              ("; ".join(errors) if errors else "diagnostic prefix" if diagnostic else "natural replay exit"), flush=True)
        return True

    def stop(self):
        global pending_reference
        try:
            manager = pointer(manager_pointer)
            if not manager:
                return False
            state = block(gm + gm_begin, gm_end - gm_begin)
            def gm32(name):
                return u32(state, gm_offsets[name] - gm_begin)
            flags = gm32("flags")
            if not flags & 8 or flags & 2:
                return False
            stage, frame = gm32("currentStage"), u32(block(manager, 4))
            key = str(stage)
            if pool_stream:
                manager_value = gdb.parse_and_eval("th08::g_EffectManager")
                layout = offsets("th08::EffectManager")
                effect_layout = offsets("th08::Effect")
                size = gdb.lookup_type("th08::Effect").sizeof
                raw = block(int(manager_value.address), manager_value.type.sizeof)
                slots = bytes(value for slot in range(512) for value in
                              raw[layout["effects"] + slot * size + effect_layout["active"]:
                                  layout["effects"] + slot * size + effect_layout["active"] + 2])
                pool_stream.write(json.dumps(dict(stage=stage, frame=frame,
                    nextIndex=u32(raw, layout["nextEffectIndex"]), activeCount=u32(raw, layout["activeCount"]),
                    slots=slots.hex())) + "\n")
            rng_events.enabled = bool(settings.get("watchRng") and stage == settings["detailStage"]
                                      and settings["detailStart"] - 1 <= frame < settings["detailEnd"])
            normalization_events.enabled = bool(settings.get("watchNormalization") and stage == settings["detailStage"]
                                                and settings["detailStart"] - 1 <= frame < settings["detailEnd"])
            if settings.get("watchEnemy") is not None:
                slot = settings["watchEnemy"]
                enemy = gdb.parse_and_eval(f"th08::g_EnemyManager.enemies[{slot}]")
                layout = offsets("th08::Enemy")
                raw = block(int(enemy.address), enemy.type.sizeof)
                motion = dict(stage=stage, frame=frame, slot=slot, flags1=u32(raw, layout["flags1"]))
                for name, count in (("position", 3), ("positionOffset", 3), ("velocity", 3), ("previousPosition", 3),
                    ("worldPosition", 3), ("movementAngle", 1), ("angularVelocity", 1), ("speed", 1), ("acceleration", 1)):
                    motion[name] = list(struct.unpack_from("<" + "I" * count, raw, layout[name]))
                with (output / "enemy-motion.jsonl").open("a") as handle:
                    handle.write(json.dumps(motion) + "\n")
            if settings.get("watchItem") is not None and frame >= settings["watchItemStart"]:
                item_spawn.enabled = True
                slot = settings["watchItem"]
                item = gdb.parse_and_eval(f"th08::g_ItemManager.items[{slot}]")
                item_layout = offsets("th08::Item")
                raw = block(int(item.address), item.type.sizeof)
                motion = dict(stage=stage, frame=frame, slot=slot,
                    position=list(struct.unpack_from("<3I", raw, item_layout["currentPosition"])),
                    velocity=list(struct.unpack_from("<3I", raw, item_layout["startPositionOrVelocity"])),
                    timer=list(struct.unpack_from("<3I", raw, item_layout["timer"])),
                    flags=list(raw[item_layout["itemType"]:item_layout["itemType"] + 5]),
                    player=list(struct.unpack("<3I", block(player + player_offsets["position"], 12))))
                with (output / "item-motion.jsonl").open("a") as handle:
                    handle.write(json.dumps(motion) + "\n")
            if settings.get("watchScoreStage") == stage and frame >= settings["watchScoreFrame"] - 1:
                score_events.enabled = True
            if not self.stages or self.stages[-1] != stage:
                if stage in self.stages or key not in self.rng_origins:
                    raise RuntimeError("Stage initialization is missing or repeated")
                self.stages.append(stage)
                initialization.enabled = False
            if self.stage_frames.get(key) == frame:
                initialization.enabled = True
            globals_data = block(pointer(gm + gm_offsets["globals"]), gdb.lookup_type("th08::ZunGlobals").sizeof)
            def glob(name):
                return u32(globals_data, globals_offsets[name])
            rng_data = block(rng, gdb.lookup_type("th08::Rng").sizeof)
            row = [self.count + 1, stage, frame, gm32("gameplayFrameCounter"), glob("score"), glob("graze"),
                   *[glob(name) for name in ("deaths", "livesRemaining", "bombsRemaining", "playerPower")],
                   *struct.unpack("<3I", block(player + player_offsets["position"], 12)),
                   block(player + player_offsets["playerState"], 1)[0],
                   struct.unpack_from("<H", rng_data, rng_offsets["seed"])[0],
                   (u32(rng_data, rng_offsets["generationCount"]) - self.rng_origins[key]) & 0xffffffff,
                   struct.unpack("<H", block(input_address, 2))[0], glob("pointItemsCollected"),
                   glob("currentTimeOrbs"),
                   struct.unpack_from("<h", globals_data, globals_offsets["youkaiGauge"])[0] & 0xffffffff,
                   globals_data[globals_offsets["clockTime"]], glob("pointItemValue"),
                   state[gm_offsets["character"] - gm_begin], state[gm_offsets["shotType"] - gm_begin],
                   u32(block(gm + gm_offsets["difficulty"], 4))]
            stream.write(json.dumps(row) + "\n")
            self.count += 1
            self.stage_frames[key], self.end_scores[key] = frame, row[4]
            if pending_reference is not None:
                expected, pending_reference = pending_reference, None
            else:
                line = reference.readline()
                expected = json.loads(line) if line else None
            if expected:
                expected[0] -= reference_origin
            if expected != row:
                difference = dict(calc=self.count, stage=stage, frame=frame,
                    differingFields=[field for field, a, b in zip(fields, expected or [], row) if a != b],
                    reference=dict(zip(fields, expected or [])), candidate=dict(zip(fields, row)), previous=self.previous)
                (output / "first-difference.json").write_text(json.dumps(difference, indent=2) + "\n")
                save_state("first-difference-state.json.gz")
                return self.finish("Replay diverged; see first-difference.json")
            self.previous = (self.previous + [dict(zip(fields, row))])[-2:]
            if self.count % 10000 == 0:
                print(f"Portable replay: {self.count} equal calculations, stage index {stage}, frame {frame}", flush=True)
                stream.flush()
            if settings.get("maxCalculations") and self.count >= settings["maxCalculations"]:
                return self.finish(diagnostic=True)
        except Exception as error:
            return self.finish(str(error))
        return False


observer = Observer()


class Initialization(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::GameManager::OnUpdate", internal=True)
        self.enabled = False

    def stop(self):
        manager = pointer(manager_pointer)
        if manager and not u32(block(manager, 4)):
            stage = u32(block(gm + gm_offsets["currentStage"], 4))
            observer.rng_origins[str(stage)] = u32(block(rng + rng_offsets["generationCount"], 4))
        return False


initialization = Initialization()


class BeginStage(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::ReplayManager::BeginPlaybackStage", internal=True)

    def stop(self):
        # Configure the same public playback mode selected in the retail menu.
        gdb.execute("set th08::g_GameManager.replayMode = " + str(settings["playbackMode"]))
        initialization.enabled = True
        return False


class Delete(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::ReplayManager::DeleteReplayManager", internal=True)

    def stop(self):
        return observer.finish() if observer.stages else False


begin_stage = BeginStage()
delete = Delete()


class ScoreEvents(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::GameManager::AddScore", internal=True)
        self.enabled = False

    def stop(self):
        current = gdb.newest_frame()
        record = dict(stage=int(gdb.parse_and_eval("th08::g_GameManager.currentStage")),
                      frame=int(gdb.parse_and_eval("th08::g_ReplayManager->frameCounter")),
                      points=int(current.read_var("score")),
                      scoreBefore=int(gdb.parse_and_eval("th08::g_GameManager.globals->score")), callers=[])
        caller = current.older()
        for _ in range(5):
            if caller is None:
                break
            record["callers"].append(dict(name=caller.name(), pc=hex(caller.pc())))
            if caller.name() == "th08::EnemyManager::OnUpdate":
                for name in ("damage", "extraDamage", "bombHit", "rankAmount"):
                    try:
                        record[name] = int(caller.read_var(name))
                    except gdb.error:
                        pass
            caller = caller.older()
        with (output / "score-calls.jsonl").open("a") as handle:
            handle.write(json.dumps(record) + "\n")
        return False


score_events = ScoreEvents()


class ItemSpawn(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::ItemManager::SpawnItem", internal=True)
        self.enabled = False

    def stop(self):
        frame = gdb.newest_frame()
        if int(frame.read_var("this")["nextIndex"]) != settings["watchItem"]:
            return False
        if int(frame.read_var("this")["items"][settings["watchItem"]]["isInUse"]):
            return False
        position = frame.read_var("position")
        record = dict(stage=int(gdb.parse_and_eval("th08::g_GameManager.currentStage")),
                      frame=int(gdb.parse_and_eval("th08::g_ReplayManager->frameCounter")),
                      slot=settings["watchItem"], itemType=int(frame.read_var("itemType")),
                      state=int(frame.read_var("state")),
                      position=list(struct.unpack("<3I", block(int(position), 12))), callers=[])
        caller = frame.older()
        for _ in range(5):
            if caller is None:
                break
            record["callers"].append(dict(name=caller.name(), pc=hex(caller.pc())))
            if caller.name() in ("th08::BulletManager::RemoveAllBullets", "th08::BulletManager::DespawnBullets"):
                if int(position) == int(caller.read_var("position").address):
                    for name in ("sine", "cosine", "radius"):
                        value = caller.read_var(name)
                        record[name] = u32(block(int(value.address), 4))
                    laser = caller.read_var("laser")
                    record["laserPosition"] = list(struct.unpack("<3I", block(int(laser["position"].address), 12)))
            if caller.name() == "th08::Enemy::DropItems":
                enemy = caller.read_var("this")
                record["enemySlot"] = int(enemy - gdb.parse_and_eval("&th08::g_EnemyManager.enemies[0]"))
                for name in ("position", "positionOffset", "worldPosition", "velocity"):
                    record[name] = list(struct.unpack("<3I", block(int(enemy[name].address), 12)))
                record["flags1"] = int(enemy["flags1"])
            caller = caller.older()
        with (output / "item-spawn.jsonl").open("a") as handle:
            handle.write(json.dumps(record) + "\n")
        return False


item_spawn = ItemSpawn()


class RngEvents(gdb.Breakpoint):
    def __init__(self):
        super().__init__("th08::Rng::GetRandomU16", internal=True)
        self.enabled = False

    def stop(self):
        data = block(rng, gdb.lookup_type("th08::Rng").sizeof)
        record = dict(stage=u32(block(gm + gm_offsets["currentStage"], 4)),
                      frame=u32(block(pointer(manager_pointer), 4)),
                      seed=struct.unpack_from("<H", data, rng_offsets["seed"])[0],
                      generation=u32(data, rng_offsets["generationCount"]), callers=[])
        caller = gdb.newest_frame().older()
        for _ in range(7):
            if caller is None:
                break
            record["callers"].append(dict(name=caller.name(), pc=hex(caller.pc())))
            caller = caller.older()
        with (output / "rng-events.jsonl").open("a") as handle:
            handle.write(json.dumps(record) + "\n")
        return False


rng_events = RngEvents()


class NormalizationEvents(gdb.Breakpoint):
    def __init__(self):
        super().__init__("D3DXVec3Dot", internal=True)
        self.enabled = False

    def stop(self):
        frame = gdb.newest_frame()
        caller = frame.older()
        if caller.name() != "th08::UpdateTintedBossTrackingCameraParticle":
            return False
        effect = caller.read_var("effect")
        camera = gdb.parse_and_eval("th08::g_Background.cameraCurrent")
        record = dict(stage=u32(block(gm + gm_offsets["currentStage"], 4)),
                      frame=u32(block(pointer(manager_pointer), 4)),
                      slot=int(effect - gdb.parse_and_eval("&th08::g_EffectManager.effects[0]")),
                      position=list(struct.unpack("<3I", block(int(effect["position"].address), 12))),
                      camera=list(struct.unpack("<3I", block(int(camera["position"].address), 12))),
                      normalized=list(struct.unpack("<3I", block(int(frame.read_var("right")), 12))),
                      forward=list(struct.unpack("<3I", block(int(frame.read_var("left")), 12))))
        with (output / "normalization-events.jsonl").open("a") as handle:
            handle.write(json.dumps(record) + "\n")
        return False


normalization_events = NormalizationEvents()
