# Reproduce the native-layout defects recorded in the portable layout audit.
# Requires a Linux x86_64 Debug ELF and GDB with Python; no game assets.
# This diagnoses the unfixed baseline and deliberately asserts retained stale
# fields. It is not an acceptance test for the eventual repairs.
# The player case stops at a source breakpoint after memset; no game code is
# patched and the interrupted inferior is killed when this script quits.
set pagination off
set confirm off
set debuginfod enabled off
start
python
import gdb, json, struct
inferior = gdb.selected_inferior()
results = []
def integer(expression):
    return int(gdb.parse_and_eval(expression))
assert integer('sizeof(void*)') == 8, 'This diagnostic requires the native 64-bit layout'
def setvalue(expression):
    gdb.execute('set ' + expression, to_string=True)
def zero(expression):
    value = gdb.parse_and_eval(expression)
    inferior.write_memory(int(value.address), bytes(value.type.sizeof))
def record(name, **fields):
    fields['case'] = name
    results.append(fields)
    print('LAYOUT_AUDIT ' + json.dumps(fields, sort_keys=True))

# 1. Full manager reset leaves fixed-pool effects active.
zero('th08::g_EffectManager')
setvalue('th08::g_EffectManager.nextEffectIndex = 17')
setvalue('th08::g_EffectManager.effects[640].active = 1')
gdb.execute('call th08::g_EffectManager.ResetEffects()', to_string=True)
assert integer('th08::g_EffectManager.nextEffectIndex') == 0
active = integer('th08::g_EffectManager.effects[640].active')
assert active == 1
record('effect-reset', runtimeSize=integer('sizeof(th08::EffectManager)'),
       clearSize=0x8b05c, fixedSlotActiveAfterReset=active,
       activeOffset=integer('(char*)&th08::g_EffectManager.effects[640].active - (char*)&th08::g_EffectManager'))

# 2. Real bullet-removal call. Offscreen position makes SpawnItem return
# before touching animation assets; no collision region is enabled.
zero('th08::g_Player')
zero('th08::g_BulletManager')
setvalue('th08::g_BulletManager.bullets[0].state = 1')
setvalue('th08::g_BulletManager.bullets[0].position.x = -100')
setvalue('th08::g_BulletManager.bullets[0].collisionDisabled = 1')
gdb.execute('call th08::g_BulletManager.RemoveAllBullets(0)', to_string=True)
assert integer('th08::g_BulletManager.bullets[0].state') == 0
collision = integer('th08::g_BulletManager.bullets[0].collisionDisabled')
assert collision == 1
record('bullet-reset', runtimeSize=integer('sizeof(th08::Bullet)'),
       clearSize=0x10b8, stateAfterRemoval=0, collisionDisabledAfterRemoval=collision,
       collisionOffset=integer('(char*)&th08::g_BulletManager.bullets[0].collisionDisabled - (char*)&th08::g_BulletManager.bullets[0]'))

# Exercise the cancellation-collision arm of RemoveAllBullets as well.
setvalue('th08::g_BulletManager.bullets[0].state = 1')
setvalue('th08::g_BulletManager.bullets[0].position.x = -100')
setvalue('th08::g_Player.cancelRegions[0].active = 1')
setvalue('th08::g_Player.cancelRegions[0].center.x = -100')
setvalue('th08::g_Player.cancelRegions[0].radius = 1')
gdb.execute('call th08::g_BulletManager.RemoveAllBullets(0)', to_string=True)
assert integer('th08::g_BulletManager.bullets[0].state') == 0
assert integer('th08::g_Player.cancelRegions[0].hitAccumulator') == 2
assert integer('th08::g_BulletManager.bullets[0].collisionDisabled') == 1
record('bullet-collision-reset', collisionDisabledAfterRemoval=1,
       stateAfterRemoval=0, cancellationHitAccumulator=2)
zero('th08::g_Player')

# 3. Ordinary message 1 avoids the stage-specific resource branches.
gui = integer('(void*)malloc(sizeof(th08::GuiImpl))')
message = integer('(void*)malloc(64)')
inferior.write_memory(gui, bytes(integer('sizeof(th08::GuiImpl)')))
inferior.write_memory(message, bytes(64))
setvalue('th08::g_Gui.impl = (th08::GuiImpl*)%d' % gui)
setvalue('th08::g_Gui.impl->message.msgFile = (th08::GuiMessageFile*)%d' % message)
setvalue('th08::g_Gui.impl->message.msgFile->messageCount = 2')
setvalue('th08::g_Gui.impl->message.msgFile->messageOffsets[1] = 16')
setvalue('th08::g_Gui.impl->message.ignoreWaitCounter = 123')
setvalue('th08::g_Gui.impl->message.selectedOption = 1')
setvalue('th08::g_GameManager.currentStage = 0')
setvalue('th08::g_GameManager.shotType = 0')
gdb.execute('call th08::g_Gui.impl->StartMessage(1)', to_string=True)
wait = integer('th08::g_Gui.impl->message.ignoreWaitCounter')
selection = integer('th08::g_Gui.impl->message.selectedOption')
assert wait == 123 and selection == 1
record('gui-message-reset', runtimeSize=integer('sizeof(th08::GuiMsgVm)'),
       clearSize=0x1570, ignoreWaitCounterAfterStart=wait, selectedOptionAfterStart=selection)

# 4. A fixed i386 member offset changes opcode 169's branch on 64-bit.
enemy = integer('&th08::g_EnemyManager.enemies[0]')
stream = integer('(void*)malloc(32)')
size = integer('sizeof(th08::Enemy)')
values = []
for raw in [0.0, 320.0]:
    inferior.write_memory(enemy, bytes(size))
    inferior.write_memory(stream, struct.pack('<ihhBBHf', 0, 169, 16, 0, 255, 1, 10016.0) + struct.pack('<ihhBBHf', 1000, 0, 16, 0, 255, 0, 0.0))
    inferior.write_memory(enemy + 0x2d34, struct.pack('<f', raw))
    setvalue('th08::g_EnemyManager.enemies[0].position.x = 320')
    setvalue('th08::g_Player.position.x = 100')
    setvalue('th08::g_EnemyManager.enemies[0].pendingEclSubroutineIndex = -1')
    setvalue('th08::g_EnemyManager.enemies[0].mainEclContextStorage.currentInstr = (th08::EclRawInstruction*)%d' % stream)
    rng = gdb.parse_and_eval('th08::g_Rng')
    inferior.write_memory(int(rng.address), bytes(rng.type.sizeof))
    gdb.execute('call th08::g_EclManager.RunEcl(&th08::g_EnemyManager.enemies[0])', to_string=True)
    values.append(float(gdb.parse_and_eval('th08::g_EnemyManager.enemies[0].mainEclContextStorage.floatVariables[0]')))
assert -0.785399 <= values[0] <= 0.785399
assert abs(values[1]) > 2.35
record('ecl-opcode-169', enemyPositionX=320, playerPositionX=100, runtimePositionOffset=integer('(char*)&th08::g_EnemyManager.enemies[0].position - (char*)&th08::g_EnemyManager.enemies[0]'), fixedReadOffset=0x2d34, rawOffsetValues=[0,320], angleOutputs=values, sameRngInitialState=True)

# 5. Stop immediately after the real focus-entry memset. Subsequent
# animation/callback initialization requires assets and is outside this probe.
zero('th08::g_Player')
setvalue('th08::g_GuiMessageInputCurrent = 4')
setvalue('th08::g_Player.focusMode = 0')
setvalue('th08::g_Player.optionStates[0].orbitAngle = 1.25')
setvalue('th08::g_Player.optionStates[0].facingAngle = 2.5')
setvalue('th08::g_Player.optionStates[0].timer.current = 123')
bp = gdb.Breakpoint('src/Player.cpp:671', internal=True)
try:
    gdb.parse_and_eval('th08::g_Player.UpdateMovementAndOptions()')
except gdb.error:
    assert bp.hit_count == 1, 'Expected the post-memset breakpoint'
else:
    raise RuntimeError('The post-memset breakpoint did not interrupt the call')
assert float(gdb.parse_and_eval('th08::g_Player.optionStates[0].orbitAngle')) == 1.25
assert float(gdb.parse_and_eval('th08::g_Player.optionStates[0].facingAngle')) == 2.5
timer = integer('th08::g_Player.optionStates[0].timer.current')
assert timer == 123
record('player-option-reset-boundary', runtimeSize=integer('sizeof(th08::PlayerOptionState)'),
       clearSize=0x2f4, orbitAngleAfterMemset=1.25, facingAngleAfterMemset=2.5,
       timerAfterMemset=timer, scope='stopped after memset, before callback assignment')
assert len(results) == 6
end
quit 0
