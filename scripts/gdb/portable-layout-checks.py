"""Asset-free production state transitions, executed inside an isolated GDB inferior."""
import json
import struct

import gdb


inferior = gdb.selected_inferior()


def integer(expression):
    return int(gdb.parse_and_eval(expression))


def set_value(expression):
    gdb.execute("set " + expression, to_string=True)


def zero(expression):
    value = gdb.parse_and_eval(expression)
    inferior.write_memory(int(value.address), bytes(value.type.sizeof))


def allocate(type_name, variable):
    address = integer("(void*)calloc(1, sizeof(%s))" % type_name)
    if not address:
        raise RuntimeError("Cannot allocate fixture " + type_name)
    set_value("$%s = (%s*)%d" % (variable, type_name, address))
    return address


def call(expression):
    return gdb.execute("call " + expression, to_string=True)


def check(name, family, passed, **observed):
    print("LAYOUT_CHECK " + json.dumps(dict(name=name, family=family,
                                           passed=bool(passed), observed=observed),
                                      sort_keys=True))


native = integer("sizeof(void*)") == 8

# A reset is also used when effects are re-registered and cached stage resources
# are reused. Seed valid activity, rather than assuming static zero initialization.
zero("th08::g_EffectManager")
set_value("th08::g_EffectManager.nextEffectIndex = 17")
set_value("th08::g_EffectManager.effects[640].active = 1")
call("th08::g_EffectManager.ResetEffects()")
active = integer("th08::g_EffectManager.effects[640].active")
check("effect-reset", "effect", active == 0 and
      integer("th08::g_EffectManager.nextEffectIndex") == 0, fixedSlotActive=active)

# Empty, cached ANM tables are sufficient for real resource callbacks. Their
# methods execute normally; no game code, return value, or callback is stubbed.
allocate("th08::AnmManager", "anm")
set_value("th08::g_AnmManager = $anm")
set_value("th08::g_Supervisor.keepStageResources = 1")
set_value("th08::g_Supervisor.isInitialStageLoad = 0")
set_value("th08::g_Supervisor.framerateMultiplier = 1")
for iteration in range(2):
    set_value("th08::g_EffectManager.effects[640].active = 1")
    call("th08::EffectManager::ReleaseEffectResources(&th08::g_EffectManager)")
    call("th08::EffectManager::LoadEffectResources(&th08::g_EffectManager)")
    active = integer("th08::g_EffectManager.effects[640].active")
    resources = integer("th08::g_EffectManager.effectAnm != 0 && "
                        "th08::g_EffectManager.stageEffectAnm != 0")
    check("effect-cached-reload-%d" % iteration, "effect", active == 0 and resources,
          fixedSlotActive=active, resourcesReinstalled=bool(resources))

# Both item-clear arms use an out-of-bounds item position so the real item
# producer exits before using textures. The mode-4 control must still despawn.
zero("th08::g_Player")
zero("th08::g_BulletManager")
set_value("$bullet = &th08::g_BulletManager.bullets[0]")
for collision in (False, True):
    zero("th08::g_Player")
    zero("*$bullet")
    set_value("$bullet->state = 1")
    set_value("$bullet->position.x = -100")
    set_value("$bullet->collisionDisabled = 1")
    set_value("$bullet->exStates[6].int2 = 123")
    if collision:
        set_value("th08::g_Player.cancelRegions[0].active = 1")
        set_value("th08::g_Player.cancelRegions[0].center.x = -100")
        set_value("th08::g_Player.cancelRegions[0].radius = 1")
    call("th08::g_BulletManager.RemoveAllBullets(0)")
    state = integer("$bullet->state")
    disabled = integer("$bullet->collisionDisabled")
    tail = integer("$bullet->exStates[6].int2")
    check("bullet-clear-" + ("collision" if collision else "item"), "bullet",
          state == 0 and disabled == 0 and tail == 0,
          state=state, collisionDisabled=disabled, transformTail=tail)

zero("th08::g_Player")
set_value("$bullet->state = 1")
set_value("$bullet->position.x = -100")
set_value("$bullet->collisionDisabled = 1")
call("th08::g_BulletManager.RemoveAllBullets(4)")
check("bullet-despawn-control", "control", integer("$bullet->state") == 5 and
      integer("$bullet->collisionDisabled") == 1)

# Reuse the deactivated slot through the actual production spawner. Null script
# entries and matching sprite indices avoid texture-dependent sprite selection.
call("$bullet->Deactivate()")
allocate("th08::BulletSpawnDescriptor", "descriptor")
allocate("th08::BulletTypeSprites", "sprites")
set_value("$descriptor->templateSprites = $sprites")
set_value("$descriptor->count1 = 1")
set_value("$descriptor->count2 = 1")
set_value("$descriptor->speed1 = 1")
set_value("$descriptor->aimMode = 1")
set_value("$descriptor->transformSound = -1")
set_value("th08::g_BulletManager.bulletCursor = $bullet")
set_value("th08::g_BulletManager.spawnSuppressionFrames = 0")
call("th08::g_BulletManager.SpawnSingleBullet($descriptor, 0, 0, 0.0f)")
check("bullet-slot-reuse", "control", integer("$bullet->state") == 1 and
      integer("$bullet->collisionDisabled") == 0 and
      integer("$bullet->transformFlags") == 0)
# Keep later GUI transitions from attempting to create an item for this bullet.
zero("th08::g_BulletManager")

# A repeated ordinary message start must reset wait/selection/intro state while
# retaining the file pointer and avoiding writes into the following GUI state.
allocate("th08::GuiImpl", "gui")
set_value("th08::g_Gui.impl = $gui")
message = integer("(void*)calloc(1, 64)")
set_value("$gui->message.msgFile = (th08::GuiMessageFile*)%d" % message)
set_value("$gui->message.msgFile->messageCount = 2")
if native:
    set_value("$gui->message.msgFile->messageOffsets[1] = 16")
else:
    set_value("$gui->message.msgFile->messages[1] = (th08::GuiMessageInstruction*)%d" %
              (message + 16))
set_value("th08::g_GameManager.currentStage = 0")
set_value("th08::g_GameManager.shotType = 0")
for iteration in range(2):
    set_value("$gui->message.ignoreWaitCounter = 123")
    set_value("$gui->message.selectedOption = 1")
    set_value("$gui->message.introLines[1].visible = 1")
    set_value("$gui->stageClearScreenState = 77")
    call("$gui->StartMessage(1)")
    wait = integer("$gui->message.ignoreWaitCounter")
    selection = integer("$gui->message.selectedOption")
    visible = integer("$gui->message.introLines[1].visible")
    check("gui-message-restart-%d" % iteration, "gui",
          wait == 0 and selection == 0 and visible == 0,
          ignoreWaitCounter=wait, selectedOption=selection, introVisible=visible)
    check("gui-neighbors-%d" % iteration, "control",
          integer("$gui->message.msgFile") == message and
          integer("$gui->stageClearScreenState") == 77 and
          integer("$gui->message.currentMsgIdx") == 1)

# Actual interpreter, branch boundaries, unchanged RNG initial state, and an
# independent old-offset perturbation in the native layout.
set_value("$enemy = &th08::g_EnemyManager.enemies[0]")
enemy = integer("$enemy")
stream = integer("(void*)malloc(32)")
outputs = {}
pairs = [(100.0, x) for x in (80.0, 96.0, 97.0, 288.0, 289.0, 320.0)]
pairs += [(0.0, 96.0), (0.0, 97.0), (300.0, 288.0), (300.0, 289.0)]
for player_x, x in pairs:
    for raw in (0.0, 320.0):
        zero("*$enemy")
        inferior.write_memory(stream,
            struct.pack("<ihhBBHf", 0, 169, 16, 0, 255, 1, 10016.0) +
            struct.pack("<ihhBBHf", 1000, 0, 16, 0, 255, 0, 0.0))
        if native:
            inferior.write_memory(enemy + 0x2d34, struct.pack("<f", raw))
        set_value("$enemy->position.x = %s" % x)
        set_value("th08::g_Player.position.x = %s" % player_x)
        set_value("$enemy->pendingEclSubroutineIndex = -1")
        set_value("$enemy->mainEclContextStorage.currentInstr = "
                  "(th08::EclRawInstruction*)%d" % stream)
        zero("th08::g_Rng")
        call("th08::g_EclManager.RunEcl($enemy)")
        angle = float(gdb.parse_and_eval("$enemy->mainEclContextStorage.floatVariables[0]"))
        outputs[player_x, x, raw] = angle
        left_branch = (player_x < x and 96 < x) or 288 < x
        passed = abs(angle) > 2.35 if left_branch else -0.785399 <= angle <= 0.785399
        check("opcode-169-player%s-x%s-raw%s" % (player_x, x, raw), "ecl", passed,
              playerX=player_x, x=x, oldOffsetValue=raw if native else None, angle=angle)
for player_x, x in pairs:
    check("opcode-169-invariance-player%s-x%s" % (player_x, x), "ecl",
          outputs[player_x, x, 0.0] == outputs[player_x, x, 320.0])

# Complete Player methods with empty ANM scripts and null option callback data.
# This tests all three clear sites, including every solo-character initializer;
# actual game callback behavior is covered separately by replay tests.
allocate("th08::ZunGlobals", "globals")
set_value("th08::g_GameManager.globals = $globals")
allocate("th08::PlayerRawShtFile", "sht")
scripts = integer("(void*)calloc(16, sizeof(void*))")
set_value("$anm->anmFiles[5].scripts = (th08::AnmRawInstr**)%d" % scripts)
zero("th08::g_PlayerOptionUpdateCallbacks")
zero("th08::g_PlayerOptionRenderCallbacks")
zero("th08::g_PlayerRoute3ExitUpdateCallbacks")
zero("th08::g_PlayerRoute3ExitRenderCallbacks")


def prepare_player(shot):
    zero("th08::g_Player")
    set_value("th08::g_GameManager.shotType = %d" % shot)
    set_value("th08::g_Player.anmFile = &$anm->anmFiles[5]")
    set_value("th08::g_Player.primaryShtFile = $sht")
    set_value("th08::g_Player.secondaryShtFile = $sht")
    set_value("th08::g_GameManager.youkaiGaugeHumanEffectsThreshold = -8000")
    set_value("th08::g_GameManager.youkaiGaugeYoukaiEffectsThreshold = 8000")
    for i in range(4):
        set_value("th08::g_Player.optionStates[%d].orbitAngle = 1.25" % i)
        set_value("th08::g_Player.optionStates[%d].facingAngle = 2.5" % i)
        set_value("th08::g_Player.optionStates[%d].unconsumedDword2D4 = 123" % i)


def options_reset(indices):
    return all(float(gdb.parse_and_eval("th08::g_Player.optionStates[%d].%s" % (i, field))) == 0
               for i in indices for field in ("orbitAngle", "facingAngle", "unconsumedDword2D4"))


for shot in range(4):
    prepare_player(shot)
    set_value("th08::g_GuiMessageInputCurrent = 4")
    set_value("th08::g_Player.focusMode = 0")
    set_value("th08::g_Player.focusEffect = &th08::g_EffectManager.effects[0]")
    call("th08::g_Player.UpdateMovementAndOptions()")
    check("player-focus-entry-%d" % shot, "player", options_reset(range(4)),
          completed=True)

prepare_player(3)
set_value("th08::g_GuiMessageInputCurrent = 0")
set_value("th08::g_Player.focusMode = 1")
call("th08::g_Player.UpdateMovementAndOptions()")
check("player-route3-exit", "player", options_reset((2,)), completed=True)

for shot in range(4, 12):
    prepare_player(shot)
    call("th08::Player::AddedCallback(&th08::g_Player)")
    check("player-solo-added-%d" % shot, "player", options_reset(range(4)),
          completed=True)
