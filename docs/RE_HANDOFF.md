# Current reconstruction handoff

This file records only the current durable state. Historical investigations
belong in focused notes such as `RUNECL_FUNCTION_EXACT_NOTES.md`; live counts
come from the ledgers, not this prose.

## Active semantic reconstruction branch

`semantic/typed-reconstruction` starts from `main@4cffb2a` and makes semantic
source recovery the active bounded lane.  The bootstrap scope changes no game
layout, function mapping, authored ledger, or accepted exact unit.  It adds the
evidence/acceptance guide in `docs/SEMANTIC_RECONSTRUCTION.md`, the
`$th08-semantic` workflow, and the read-only
`scripts/analysis/report-semantic-debt.py` candidate router.

The verified starting baseline is authored **1,107 / 1,107** and strict exact
**1,105 / 1,107**; `config/claims.csv` is header-only.  Work one structure and
one coherent field family at a time.  VC7 focused/aggregate replay remains the
exact-code oracle, while the modern build and relevant Linux layout/runtime
checks remain the portable behavior oracle.  Do not publish candidate counts as
semantic progress.

The portable Linux package workflow is again automatic on `push` and
`pull_request`, as well as available through `workflow_dispatch`.  It was
temporarily manual-only during early semantic iteration, but its release-lane
triggers were restored before publishing this phase to `main`.

The first accepted source batch is the `ScreenEffect +0x18/+0x1C/+0x20`
variant-parameter family.  A TU-local typed overlay records fade color, shake
amplitude, pulse repeat/color, and three-phase shake-envelope views while
preserving both the generic `RegisterChain` initialization ABI and the shared
header byte-for-byte.  Focused VC7 replay passed `ScreenEffect.obj` **16 / 16**
and its canonical objdiff path **5 / 5**; the normal VC7 image linked, the
complete i386 container build linked, and the fixed-layout verifier passed.

The required cold aggregate diagnostic initially exposed a reproducible
branch-baseline blocker at **1,017 / 1,105**.  The follow-up bounded maintenance
batch has now closed it.  It restored target-observed direct-global and source-
expression shapes where the portable-owner pass had changed VC7 extent/codegen,
introduced target/modern storage bridges where the two linkers intentionally
differ, and migrated only relocation entries whose masked function bytes were
otherwise identical.  All 79 final relocation-only failures had zero
non-relocation byte differences; all 338 changed/new relocation entries inferred
the existing target bases without conflict and had mapping/global-ledger
evidence.

The current attested result is a non-reuse cold build of all 75 configured
comparison objects followed by **1,105 / 1,105 exact** accepted units with no
failures.  A normal VC7 production link passes.  The complete i386 Linux
container build and fixed-layout verifier also pass.  This closes the aggregate
blocker without changing authored progress or weakening the comparator; see the
post-port reconciliation entry in `docs/SEMANTIC_RECONSTRUCTION.md` for the
evidence split and reusable compatibility pattern.

The next accepted source batch recovers the Player deathbomb/Bomb lifecycle.
It replaces the raw `Player + 0xFDC..+0x1014` active/type/duration/timer and
calculation/draw callback views, plus the `+0xE2A68/+0xE2A6C/+0xE2B28`
deathbomb window, Bomb-input lock, and effect pointer, with asserted typed
fields.  TH08 target users cover `Die`, the Bomb update/draw paths, the dying
transition, initialization, shot/collision gates, and `Spellcard::EndSpell`;
TH06 only corroborates the stable active/duration/timer/calc/draw roles.  The
individual callback-slot meanings and retained opaque fields remain unknown.
Focused replay passed **249 / 249**, a new non-reuse cold replay passed **1,105
/ 1,105**, the normal VC7 link passed, and the complete i386 Linux build/layout
verifier passed.  No match manifest, authored/exact ledger, global identity, or
runtime state operation changed.

The next accepted batch closes the `Laser` lifecycle family across
`BulletManager::RemoveAllBullets`, `DespawnBullets`, `SpawnLaserPattern`,
`OnUpdate`, `OnDraw`, and seven Laser-control opcodes in the complete exact
`EclManager::RunEcl`.  Raw `+0x548..+0x599` views now use the asserted Laser
owner for position, beam geometry, speed, timers, hitbox windows, active state,
and the starting/active/despawning state machine.  The `+0x599` byte is named
`hideCapDuringStartup` from its RunEcl write and draw-side gate; flag-bit
meanings, the color convention, and the final two bytes remain unknown.
Focused replay passed BulletManager **24 / 24** and RunEcl **1 / 1**; a new
non-reuse cold replay passed **1,105 / 1,105**, the normal VC7 link passed, and
the complete i386 Linux build/layout verifier passed.  No match manifest,
ledger, global identity, initialization order, or state operation changed.

The latest accepted batch closes the core `Bullet` lifecycle across spawn,
transform dispatch, removal/despawn, update/collision/culling, six draw
buckets, rendering, sprite-template setup, and nine exact ECL extension
handlers.  Raw `Bullet + 0xD44..+0x10B4` and manager-tail views now use asserted
owners for kinematics, timers, state, transform masks, graze/collision gates,
draw links, and pool bookkeeping.  The transform-specific `BulletExState`
payloads, ANM VM internals, `Bullet + 0xDBC`, template `+0xD40`, and the
gameplay role of the proven-`Float3` member at `+0xD5C` remain explicitly
unknown.  Focused production/canonical replay passed **70 / 70**; a final
non-reuse cold replay passed **1,105 / 1,105**, the normal VC7 image linked,
and the final complete i386 Linux build/layout verifier passed.  The semantic
router falls from 294 to 97 candidates in `BulletManager.cpp` and 25 to 18 in
`EclExIns.cpp`; these are review deltas, not progress percentages.  No match
manifest, ledger, global identity, state operation, or rendering sequence
changed.

The next accepted batch closes `BulletExState[7]`, the transform-specific
runtime family at `Bullet + 0xF80..+0x10B3`.  The dispatcher, update path, and
nine handlers now use asserted state slots and named timer, acceleration,
direction-change, bounce, wait, and wrap fields; the transform flag namespace
also names spawn modes, runtime actions, spawn sound, and cancel immunity.
The seven-element nontrivial array remains intact because its VC7 vector
construction is target-visible.  Generic tagged-record operands and the
deceleration slot's unconsumed `Float3::z` role remain explicitly neutral.
Focused `BulletManager.obj` replay passed **37 / 37**; a required non-reuse cold
replay passed **1,105 / 1,105**, the normal VC7 image linked, and the complete
i386 Linux build/layout verifier passed.  The semantic router for
`BulletManager.cpp` falls from 97 to 29 candidates.  No match manifest, ledger,
layout, construction order, or state operation changed.  The adjacent bullet
spawn/ECL descriptor family is the next bounded milestone-closing batch.

The following accepted batch closes that adjacent bullet/laser spawn and ECL
descriptor family.  `Enemy + 0x2E24/+0x3070` are now asserted
`bulletSpawnDescriptor` and `laserSpawnDescriptor` owners; ECL opcodes 109,
111, and 113..115 use typed descriptor, transform-record, sound, aim, and laser
operand fields, while the exact shot dispatcher at `0x00422720` uses the same
bullet owner.  The nine target-observed aim modes and player-human/youkai shot
conditions are named; tagged transform operands and descriptor `unknown1FA`
remain deliberately neutral.  Focused replay across the five affected objects
passed **102 / 102**, a required non-reuse cold replay passed **1,105 / 1,105**,
the normal VC7 image linked, and the complete i386 Linux build/layout verifier
passed.  No match manifest, ledger, ABI, construction order, or state operation
changed.

This formally closes the first semantic milestone, **core bullet-gameplay loop
semantic closure**: Player deathbomb/Bomb, Laser lifecycle, Bullet core
lifecycle, Bullet transform runtime state, and the adjacent spawn/ECL
descriptor family are all typed and dual-oracle locked.  The claim is limited
to those bounded families and does not imply whole-program semantic
completion.  A useful next milestone is Enemy/ECL orchestration closure,
starting from the still-raw Enemy motion/rank/phase fields adjacent to these
descriptors rather than reopening the now-closed bullet loop.

The first accepted batch toward Enemy/ECL orchestration closure types the
Enemy bullet-rank and repeated-shot scheduling family.  The six
`Enemy + 0x2DEC..+0x2DFA` rank endpoints now form an asserted
`EnemyBulletRankInfluence`; the `+0x3034` delayed-shot snapshot remains an
explicit 0x2C-byte ECL protocol buffer, followed by named
`shootIntervalFrames @ +0x3060` and `shootIntervalTimer @ +0x3064` fields.
Producers, consumers, and reset paths across RunEcl, the exact shot dispatcher,
Enemy phase transitions, Spellcard reset, and EnemyManager update share those
owners.  Focused replay passed **71 / 71**, a non-reuse cold replay passed
**1,105 / 1,105**, the normal VC7 image linked, and the complete i386 Linux
build/layout verifier passed.  The next coherent family is the adjacent Enemy
combat identity/life/score/boss-timer state at `+0x2DFC..+0x2E20`, followed by
its phase threshold/callback owners.

The second Enemy/ECL orchestration batch closes that adjacent combat-state
range.  `Enemy + 0x2DFC..+0x2E20` now exposes current/maximum life, a neutral
phase-starting-life value, death score, pool index, per-Enemy Player-shot hit
accumulator, Boss timer, and ANM display color.  All spawn, ECL, phase, damage,
death, Player, Spellcard, operand-resolver, and EX callback users share the
asserted owner; `phaseStartingLife` remains explicitly inference-limited
because the target proves its writes but no current authored reader.
Focused replay passed **163 / 163**, a non-reuse cold replay passed **1,105 /
1,105**, the normal VC7 image linked, and the complete i386 Linux build/layout
verifier passed.  The next coherent family is the Boss life/timer threshold and
callback table at `Enemy + 0x3358..+0x3384`, together with the adjacent ECL
subroutine identifiers and phase-gauge state that consume it.

The third Enemy/ECL orchestration batch closes the Boss phase callback and
child-ECL ownership range at `Enemy + 0x3354..+0x3393`.  The shared owner now
names last applied damage, four life thresholds and their parallel dword ECL
subroutine identifiers, the timer threshold/subroutine pair, linked-child
count, and four owned child ECL block pointers.  The dword callback IDs retain
their exact write width while call sites explicitly consume their low signed
16 bits; each `0x24B0` child block remains byte-oriented because only its
allocation protocol and partial internal offsets are proven.  All opcode,
RunEcl, PopEclContext, phase/death cleanup, Spellcard, GUI, resolver, spawn,
and update consumers share the asserted fields.  Focused replay passed **110 /
110**, target-pinned packets for both transitions and cleanup replayed exact,
a non-reuse cold replay passed **1,105 / 1,105**, the normal VC7 image linked,
and the complete i386 Linux build/layout verifier passed.  The next coherent
family is active ECL context/subroutine state around `+0x2CA0..+0x2D30`, then
the motion vectors and phase/control flags that consume it.

The fourth Enemy/ECL orchestration batch closes the interpreter/context state.
The constructor-bearing primary context at `Enemy + 0x07F8`, its sixteen-entry
call stack at `+0x0A20`, active context/stack pointers, local and per-Enemy ECL
variable families, call parameters, callback/interpolation state, primary and
active call depths, death callback, 32-entry subroutine table, and pending
subroutine index now share asserted owners.  The constructor-bearing storage
uses the same `EnemyEclContext` and `EnemyEclInterpolationSlot` types as the
interpreter instead of duplicate imported `UnkStruct` shells.  The `+0x2CEC` word, context `+0x21C`
dword, callback argument payload, and stage-specific meanings of extra ECL
variables remain explicitly unknown.  Focused replay passed **124 / 124**,
target-pinned CallSubOnEnemy/PopEclContext packets replayed exact, a non-reuse
cold replay passed **1,105 / 1,105**, the normal VC7 image linked, and the
complete i386 Linux build/layout verifier passed.  The next coherent family is
the Enemy motion controller at `+0x2D34..+0x2DEB`, then its phase/control flags
and boundary/presentation state.

The fifth Enemy/ECL orchestration batch closes the continuous motion
controller at `Enemy + 0x2D34..+0x2DEB`.  Logical, offset, previous,
last-frame-displacement, and world positions; velocity; both hitboxes; linear
and polar movement parameters; parent linkage; shot offset; and timed
interpolation state now share asserted owners across ECL, Enemy, Player,
Effect, and Spellcard users.  The two anonymous core functions are tracked as
`Enemy::UpdateMovement @ 0x00422C40` and
`Enemy::IntegrateVelocity @ 0x0042DEB0`.  ECL opcode 169 retains one
named/layout-asserted offset constant
because a direct member expression changes six VC7 x87 bytes; the complete
RunEcl function remains exact.  Focused replay passed **287 / 287**, three
target-pinned motion packets replayed exact, a non-reuse cold replay passed
**1,105 / 1,105**, the normal VC7 image linked, and the complete i386 Linux
build/layout verifier passed.  The next coherent family is the control flags
at `+0x3324/+0x3328` and the adjacent boundary/presentation state.

The sixth Enemy/ECL orchestration batch closes the adjacent control and
presentation range at `Enemy + 0x3304..+0x3353`.  Item-drop policy/counts,
death animations, Boss slot, damage flash, both flag dwords, animation
direction/draw group/difficulty override, the six-script animation table,
movement bounds, and minimum Player shot distance now share asserted owners
across ECL, Enemy, Player, Bomb, Spellcard, Effect, and timeline users.  Named
shift constants preserve target-observed VC7 shift-and-test source shape; a
mask-test trial changed 26 units and was rejected.  Twelve important Enemy
methods now carry behavior-backed names.  Focused replay passed **328 / 328**,
three target-pinned packets replayed exact, a non-reuse cold replay passed
**1,105 / 1,105**, the normal VC7 image linked, and the complete i386 Linux
build/layout verifier passed.  Retained padding, `timer3318`, neutral flag
labels, and unobserved bits remain explicit unknowns.  The next coherent
family is the trail/effect/death tail at `+0x534C..+0x53CC`, followed by the
EnemyManager pool/list owners needed to close the Enemy/ECL orchestration
milestone.

The seventh Enemy/ECL orchestration batch closes the Enemy trail and attached-
effect tail.  The 96 position/velocity/angle samples, trail flags and three
length/stride controls, damage-reduction timer, 24 attached Effect pointers,
Effect count/target distance, alignment Effect, and phase-end seconds now
share asserted owners across RunEcl, update, draw, collision, culling, linked-
child, phase/death, and score-drop paths.  The shared Effect radius, angle,
active, and release-requested fields are also asserted.  The anonymous
`0x1C` record constructor is tracked as `EnemyTrailSample::EnemyTrailSample`.
Focused replay passed **93 / 93**, four target-pinned packets replayed exact,
a non-reuse cold replay passed **1,105 / 1,105**, the normal VC7 image linked,
and the complete i386 Linux build/layout verifier passed.  The one retained
padding byte, unused trail bits, generic cross-effect vector roles, and a
producer-only phase-end value remain evidence-limited.  The EnemyManager
pool/list and global update-state fields are the final coherent family before
declaring Enemy/ECL orchestration closure.

The eighth Enemy/ECL orchestration batch closes that final manager family.
The spawn template, 481-element Enemy array (480 active pool entries plus the
target-visible failure sentinel), eight Boss slots, drop scheduler, active
count, opcode-163 state, sixteen timelines, manager timer, four draw-list
heads, two Enemy ANM banks, spawn-failure state, four timeline event slots,
and timeline-spawn suppression now share asserted `EnemyManager` owners.  The
old `g_EclEnemyTableF54CC0` and two drop-counter globals were overlapping
analysis aliases inside `g_EnemyManager`, not separate production storage;
their declarations, Linux aliases, ledgers, and relocation identities are
retired.  Important helpers are tracked as `UpdateSubrank`,
`KillAllNonBossEnemies`, and `HasBoss`.  Focused replay passed **316 / 316**,
seven target-pinned packets replayed exact, a non-reuse cold replay passed
**1,105 / 1,105**, the normal VC7 image linked, and the complete i386 Linux
build/layout verifier passed.  This formally closes the bounded **Enemy/ECL
orchestration semantic milestone**.  The next high-value milestone is Player
core state together with its adjacent PlayerBomb protocol; whole-program
semantic completion remains open.

The first accepted Player-core batch closes movement, collision/graze/item
bounds, option focus transitions, and the 20-frame shooting cycle.  The Player
owner now exposes its 16-position movement history, three AABB pairs and half-
sizes, time-scaled velocity, axis multipliers, current speeds, movement
direction enum, focus/extreme-gauge effects, typed option callbacks, and
`shotTimer`.  The linked SHT header now names the target-proven initial Bomb,
deathbomb, hitbox/graze/autocollect/item/point-line, and normal/focused speed
fields.  Strict replay caught and corrected the point-line displacement from
the initial `+0x20` hypothesis to target `+0x1C`; the accepted layout keeps
`+0x20` unknown.  Eleven important helpers now carry behavior-backed collision,
graze, movement, angle, and shooting names.  Focused replay passed **242 / 242**,
eleven target-pinned packets replayed exact, a non-reuse cold replay passed
**1,105 / 1,105**, the normal VC7 image linked, and the complete i386 Linux
build/layout verifier passed.  `position2 @ +0x2C0`, unconsumed vector
components, several gauge timers, PlayerShot internals, and most PlayerBomb
work-item protocols remain open.  The next coherent batch is PlayerShot runtime
and SHT descriptors, followed by the adjacent PlayerBomb work-item callbacks.

The second accepted Player-core batch closes that PlayerShot/SHT family.  The
SHT owner now exposes its power-level rows and `0x38`-byte shot descriptors,
including schedule, geometry, damage/type/animation/sound metadata and four
typed callbacks.  `PlayerShot` remains `0x484` bytes while naming its VM,
position history, hitbox/velocity/speed/angle, timer/state/type, persistent
timeline metadata, tint, callbacks, and originating descriptor.  Player owns
the persistent descriptor slots, shot-hit effect counter, and base shot angle.
Twenty-one helpers now carry behavior-backed initialization, scheduled spawn,
homing/falling/persistent/trail, update/draw, hit-behavior, and damage names;
`CalcDamageToEnemy @ 0x00451670` matches TH07 only as corroboration and remains
accepted against TH08 target bytes.  Focused Player plus EnemyManagerUpdate
replay passes **52 / 52**, including all 22 directly edited units.  The shared
header replay then passed **1,105 / 1,105** accepted units from a single-job
cold VC7 build; the normal VC7 image linked, and the complete i386 Linux build
and fixed-layout verifier passed.  Remaining neutral fields include SHT
header `+0x20/+0x34`, shot `+0x448/+0x46D/+0x471`, and the wider PlayerBomb
work-item protocol.  PlayerBomb is the next coherent Player-core family.

The third accepted Player-core batch closes the PlayerBomb callback protocol.
The 24-row table is now `g_PlayerBombCallbacksByShotType`, with typed
primary/secondary/deathbomb/special variants and paired update/draw rows.  The
16 Shift-JIS cut-in strings directly name the Fantasy Orb, Quadruple Barrier,
Master Spark/Artful Sacrifice, Killing Doll/Red Nightless Castle, and
Present-World Slash/Ghastly Dream families plus their deathbomb forms.  The
`0x16F0` work item now exposes its state timer, motion scalar/vector state,
path points, eight VMs, effect VM, timer, and damage/cancellation regions with
assertions through `+0x16EC`.  Timer predicates are now `HasTicked`,
`JustReached`, and `IsPeriodic`.  Focused replay passes PlayerBomb **59 / 59**,
Player **77 / 77**, EnemyManager **39 / 39**, and ECL extensions **33 / 33**.
The single-job cold VC7 replay passes **1,105 / 1,105**, the normal VC7 image
links, and the complete i386 Linux build and fixed-layout verifier pass.
`PlayerBombState +0xC` remains unknown; the next coherent batch is shared
Effect/ANM interpolation state.

The fourth accepted Player-core batch closes that shared Effect/ANM family.
`Effect` remains exactly `0x360` bytes and now exposes its leading VM, nine
constructor-bearing vectors, radius/angle and strip geometry, pool slot,
secondary radial state, timer, callbacks, signed lifecycle/draw bytes, vertex
storage, and draw-list link.  `EffectManager` now asserts the 654-row pool,
five individually constructed sentinels, five draw tails, tamper counter, and
two ANM banks through its exact `0x8B05C` extent.  Player/PlayerBomb/ECL/Enemy
users share those owners.  The four interpolation setters are behavior-named,
and their timer/mode/initial/final fields replace open-coded offsets; mode 5 is
recorded as `EaseOutCubic` and mode 4 as `EaseOut`.  Focused replay passes
Player/PlayerBomb **136 / 136** and ECL/EnemyManagerUpdate/EffectManager
**86 / 86**.  The single-job cold VC7 replay passes **1,105 / 1,105**, the
normal VC7 image links, and the complete i386 Linux build and fixed-layout
verifier pass.  Constructor evidence requires `Float3 + f32` at Effect
`+0x304`, five separate sentinel members, and signed lifecycle bytes; the two
fixed-slot spawners retain their target-shaped `sizeof`/`offsetof` expression.
Generic per-effect vectors and callback signatures remain evidence-limited.
The next coherent family is remaining Player core state, then Background.

The fifth accepted Player-core batch closes that remaining state/controller
family.  The two 192-entry collision pools are now asserted
`damageRegions`/`cancelRegions` of `PlayerCollisionRegion`; their four
allocators distinguish rectangular/circular damage and cancellation regions,
and every PlayerBomb/Enemy consumer shares the same type.  The Player state
machine now names Bomb/deathbomb update, death and respawn, respawn animation,
invulnerability, collision-region lifetime, and gauge-position updates.  State
3 is `PLAYER_STATE_INVULNERABLE`, matching its timer, flashing, and collision
suppression behavior rather than the former `DEAD` label.

The option controller now has a uniform `PlayerOptionState *` callback ABI.
Homing helpers use typed position/velocity/timer/VM members, the draw callback
uses the embedded VM directly, and seven table callbacks carry conservative
behavior names for homing, Bomb anchoring, orbit, facing, and paired-option
motion.  `AnmVmBase` exposes the scale/color/flag word used by Player, and the
six run/stage active/extreme-gauge frame counters in `GameManager` are named
across setup, Player, and GUI consumers.  Focused replay across the seven
directly affected production objects passes **258 / 258**.  A non-reuse
single-job cold VC7 replay passes **1,105 / 1,105**, the normal VC7 image
links, and the complete i386 Linux build plus fixed-layout verifier passes.
The Player source/header/Bomb router is down to one raw-member candidate, the
separate Background tint byte.  The next high-value family is the Background
camera/stage object model; whole-program semantic completion remains open.

The first accepted Background batch closes the camera, stage-object,
stage-script, fog, spell-background, and tint model.  `Background` retains its
exact `0x6600` extent while naming the loaded stage buffer and serialized
records, object/quad tables, stage position and script state, text and Effect
owners, five `BackgroundCamera` records, interpolation lanes, spell VM bank,
culling/tint state, and special-effect points.  Important helpers are now
`InterpolateCameraVector`, `CubicHermiteInterpolate`, `AccumulateTint`,
`UpdateStageObjectVms`, `StartSpellBackground`, and `StopSpellBackground`.

ECL opcode 147's former `g_EclGlobal004EA290` identity is proven to be
`g_Background.pendingStageScriptLabel @ +0x6260`; the overlapping VC7/Linux
storage alias is retired and RunEcl remains **26,638 / 26,638 exact**.  The
later replay-runtime batch also closes the former stage-7 absolute view as
`g_GameManager.currentStage == STAGE6B`, with its target owner/addend recorded
as a COFF relocation.  Focused replay passes Background **29 / 29** and the selected affected
consumer objects **487 / 487**.  A non-reuse single-job cold VC7 replay passes
**1,105 / 1,105**, the normal VC7 image links, and the complete i386 Linux
build plus fixed-layout verifier passes.  The whole-source router now reports
207 raw-member, 82 absolute-address, 432 anonymous-identifier, and 54 opaque-
storage candidates; Background itself has zero raw-member candidates.  These
are routing observations, not completion percentages.  GameManager and its
GUI/setup consumers are the next high-value coherent family.

The first accepted GameManager-core batch recovers the runtime/setup state
used by `OnUpdate @ 0x00439BC7`, `GameplaySetupThread @ 0x0043ABD7`, replay,
Player, GUI, and stage teardown.  The typed owner now names gameplay setup and
startup states, frame skipping, run/stage playtime, humanity-rate counters,
stage-at-start and clear-mask state, replay pause recording, next Supervisor
state, character-list index, and the stage RNG seed.  Setup writes now use
asserted `ZunGlobals` and `GameConfiguration` members; in particular the
target's `+0x80/+0x84/+0x88` bomb triplet is respectively bombs remaining,
bombs used, and bombs used in the current stage.  The score.dat/CATK/CLRD/PSCR
initializer at `0x0043BBE1` is now `InitializeScoreData` in source and every
mapping/comparison ledger.  Packed flag masks retain one ownership-aware word
view where VC7's emitted mask operations are target-visible.

Focused GameManager replay passes **42 / 42**, selected affected consumers
pass **493 / 493**, a new single-job non-reuse cold replay passes **1,105 /
1,105**, the normal VC7 production image links, and the complete i386 Linux
build/layout verifier passes.  Evidence-limited bytes at
`+0x3DBB6/+0x3DBB7/+0x3DDBE`, plus unresolved fields at
`+0x3DDCC/+0x3DDD2/+0x3DE0C`, remain explicitly unknown.  The whole-source
router now reports 201 raw-member, 82 absolute-address, 310 anonymous-
identifier, and 54 opaque-storage candidates.  GUI's boss/status/message
state is the next coherent owner.

The adjacent authored-exact side investigation resolves
`TitleScreen::RegisterChain @ 0x0047146D`.  GensokyoClub commit `1b630bb`
supplied the decisive natural source-shape hypothesis:
`ZUN_NEW(TitleScreen, "TitleInf")` instead of raw `new TitleScreen()`.  In the
retail build the inline registry wrapper returns the allocation directly, but
the macro expression changes VC7's hidden-new/EH local lifetime and restores
the target `0x40` frame.  The dedicated comparator replays the complete **281
/ 281 bytes** and all 20 relocations, including local EH metadata.  The
`"TitleInf"` debug registry label is compiled out and therefore remains
upstream provenance, not a target-observed string claim.  A new single-job
non-reuse cold replay passes **1,106 / 1,106**, the normal VC7 image links,
and the complete i386 Linux build/layout verifier passes.  Authored exact is
now **1,106 / 1,107 functions** and **459,396 / 459,757 bytes**; only
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0` remains unaccepted.

The first accepted GUI-core batch closes its message, Boss HUD, stage-element,
stage-clear, popup, capture, transition, and clock ownership.  `GuiImpl` is
asserted through its exact `0x230B8` extent; the message state is correctly
bounded at `0x1570`, leaving the following stage-clear state/total dwords as
their own fields.  The `+0x22DEC` stage-clear record, all important VM banks,
Boss-life target/display interpolation, formatted-popup mode, and the three
Supervisor stage-resource flags now have shared typed owners.  The clear-stage
bonus global at `0x004C7158` is corrected from a pointer array to the target-
observed integer array `g_GuiStageClearBonuses`.

Twenty-five high-use GUI and helper symbols now carry behavior-backed names,
including `StartMessage`, `UpdateStageElements`, `DrawStageElements`,
`DrawStageClearScreen`, `ShowBonusScore`, `ShowPopupText`,
`ShowSpellcardBonus`, `SetBossLifeBarTarget`, `SetBossTimerSeconds`,
`CopyCurrentStageEnemyNameTexture`, `CaptureArcade`, and the clock controls.
All mapping/implemented/reccmp/match and relocation identities moved with the
source.  Focused GUI replay passes **41 / 41**; selected affected callers pass
**224 / 224**; a single-job non-reuse cold build of all 75 comparison objects
passes **1,106 / 1,106**.  The normal VC7 image links, and the complete i386
Linux build plus fixed-layout verifier passes.  `Gui.cpp` now has zero raw-
member candidates; the whole-source router is 116 raw-member, 82 absolute,
280 anonymous, and 49 opaque candidates.  These counts are routing aids only.

The adjacent GUI message-protocol batch is now accepted.  The complete 0..22
opcode set has behavior names, and the variable-size record/file format now
uses asserted typed payloads and pointers while retaining its real byte-stream
advance and in-place offset fixups.  Target evidence proves the signed-word,
unsigned-byte, dword, portrait, wait/music, and encrypted-text accesses;
GensokyoClub's current declarations are corroboration only.  The XOR helper at
`0x004353EC` is now `DecryptGuiMessageText` in source and all ledgers.
Focused GUI replay passes **41 / 41**; `RunMsg` is exact over **5,597 authored
bytes** and its full **5,689-byte code-plus-table extent**.  A single-job
non-reuse cold build passes **1,106 / 1,106**, the normal VC7 image links, and
the complete i386 Linux build plus fixed-layout verifier passes.

The Spellcard effect/reward lifecycle is now accepted across `Init`,
`StartSpell`, `EndSpell`, `OnUpdateImpl`, and the ECL-facing
`SetStoredVector`.  `Spellcard +0xF4/+0xF8` are typed `Effect *` owners; their
ANM position/RGB/scale interpolation banks, sprite/visibility state, effect
position/tracking vector, ring geometry, lifetime timer, and active state
replace every raw effect offset in `Spellcard.cpp`.  Target-visible float
field transfers remain explicit dword bit copies.  Focused Spellcard replay
passes **29 / 29**, focused EclManager replay **10 / 10**, and a required
single-job non-reuse cold replay passes **1,106 / 1,106**.  The normal VC7
image links, and the complete i386 Linux build plus fixed-layout verifier
passes.  `Spellcard.cpp` is now zero-candidate in all router categories; the
unproven `Spellcard::unknown_044` range remains neutral.  The whole-source
router is 87 raw-member, 82 absolute, 271 anonymous, and 49 opaque candidates.

The Enemy update/render batch is now accepted.  The complete
`EnemyManager::OnUpdate @ 0x0042C660` uses the shared Enemy, Player,
GameManager, ReplayManager, VM, sprite, and draw-list owners instead of its
private raw overlay family; `EnemyManager::OnDrawImpl @ 0x0042E140` consumes
the same typed VM and `Enemy::nextInDrawGroup` fields.  Target replay also
caught two important false hypotheses: `Player +0x000` is the Player state
machine rather than character type, and loaded-sprite `+0x30/+0x34` are the
pixel height/width fields.  The phase-capture helper at `0x0042BC50` now uses
Spellcard flags and `bonusProgress` directly while retaining its ABI.

Focused replay matches OnUpdate **6,198 / 6,198**, canonical EnemyManager
**23 / 23** including OnDrawImpl **2,504 / 2,504**, and the production subset
**16 / 16**.  The required single-job non-reuse cold replay passes **1,106 /
1,106**, the normal VC7 image links, and the complete i386 Linux build plus
fixed-layout verifier passes.  `EnemyManagerUpdate.cpp` is zero-candidate in
all router categories.  The later replay-runtime batch proves former absolute
`0x018B8A24` as `g_Player.damageAccumulatorThreshold @ +0xE2B2C` from its
Player compare/subtract consumers and Enemy initialization.  The whole-source
router is now 60 raw-member, 82 absolute, 255 anonymous, and 49 opaque.

The Ascii presentation batch is now accepted.  Boss-marker state/interrupts,
the shared VM update, frame timer, retry initial-Bomb read, and Supervisor
transition countdown now have behavior-backed owners and names.  The
`AsciiManager +0x16F04/+0x16F08/+0x16F0C` tail is the Mystia night-blindness
radius, integer alpha, and center VM protocol, with
`ConfigureNightBlindness @ 0x00423390` publishing its ECL parameters.
GensokyoClub's current names corroborated this interpretation; target access
patterns and strict replay accepted it, while the target-observed integer
alpha was retained instead of upstream's `ZunColor` view.

The former globals at `0x004E3D24/0x004E3D28` are retired as overlapping
aliases inside `g_AsciiManager`.  The former
`g_AsciiManagerDemoAnm0577EB4` is likewise retired: `0x00577EB4` is exactly
`g_EffectManager.effectAnm @ +0x8B054`, now shared directly by Bullet,
PlayerBomb, Spellcard, ECL extensions, and Ascii rendering.  Their source,
relocation, global-ledger, and Linux layout identities moved together.

The rebuilt affected-object selection passes **353 / 353**, with the Ascii
object at **63 / 63** and a separately regenerated canonical selection at
**72 / 72**.  A required single-job non-reuse cold replay passes **1,106 /
1,106**, the normal VC7 image links, and the complete i386 Linux build plus
fixed-layout verifier passes.  Only five Ascii candidates remain: the
unidentified script-9 VM, reset-only bool, and unused popup tail field.  The
whole-source router is now 60 raw-member, 82 absolute, 218 anonymous, and 48
opaque candidates.

The GameManager setup/runtime batch is now accepted.  The target's nineteen
0x14-byte spell-practice music records at `0x004C7670`, nine three-song stage
contexts at `0x004C7240`, and the STD header's `+0x90/+0x290` song-name/path
arrays now have shared typed owners.  The helpers at `0x00439916/0x00439961`
are `GameManager::ShouldPauseMusicInSpellPractice` and
`GetSongNameSpriteIdx`; setup error/resource addresses are source literals
with target-replayed relocations.  GensokyoClub supplied strong table/name
hypotheses, but canonical TH08 data and VC7 replay remain the acceptance
evidence.

The byte at `0x0160F534` is now correctly owned as
`g_GameManager.scriptedUpdateFreeze @ +0x2C`.  The ECL setter at `0x00425070`
and all Bullet, Enemy, GUI, Player, Spellcard, and ScreenEffect consumers use
that owner; eight manifest relocations now require `g_GameManager + 0x2C`, and
the false standalone global/storage/Linux alias is gone.  The same batch names
the Supervisor viewport, recorded replay FPS, loading-VM and worker-thread
state; active Bullet count; Background sky-fog color; RNG saved seed;
stage-clear replay gate; anti-tamper heap-jitter allocation; and restart play
count.  Unused/reset-only GameManager tail fields remain neutral.

Focused replay passed EclExIns **33 / 33**, Background **24 / 24**,
GameManager **33 / 33**, the broad affected selection **333 / 333**,
SpellCard **29 / 29**, and the final setup/Supervisor/replay/title selection
**251 / 251**.  The normal VC7 image and probes link, and a required single-job
cold replay of all 75 comparison objects passes **1,106 / 1,106 exact**.  The
complete i386 Linux container build links and its fixed-layout verifier passes
after removing the duplicate initialization/alias shims.  No isolated runtime
smoke exists for spell-practice music or scripted whole-update freeze, so none
is claimed.  The semantic router now reports 60 raw-member, 18 absolute, 194
anonymous, and 45 opaque candidates; these remain routing counts only.

The Gensokyo `TitleScreen::RegisterChain` side hypothesis is conclusively
confirmed in this cold environment: the existing `ZUN_NEW(TitleScreen,
"TitleInf")` source replays all **281 / 281** bytes and 20 relocations at
`0x0047146D`, including its `0x40` target frame.  It is already committed in
the earlier GameManager/title checkpoint; no new merge is required.

The Bullet transform/runtime-owner batch is now accepted.  Absolute
`0x017CE8E0` is `g_Supervisor.framerateMultiplier @ +0x188`, not an
independent ECL global: all 43 existing relocations and 11 newly manifested
raw-address uses replay with the aggregate owner/addend, and the duplicate
storage, global row, and Linux linker alias are gone.  Bullet transform
helpers at `0x0042F5F0`, `0x0042FFC0`, `0x00430E10`, and
`0x00432170` through `0x00432AA0` now carry lifecycle/action names; their ANM,
ReplayManager, Effect, Player bomb-state, and ECL-instruction accesses use
asserted fields.  Focused replay passes **70 / 70**, the required cold replay
passes **1,106 / 1,106 exact**, the normal VC7 image links, and the complete
i386 Linux build plus fixed-layout verifier passes.  The remaining
`Bullet +0xDBC` and template `+0xD40` observations stay neutral.

The Replay/Supervisor runtime batch is now accepted.  Replay stage payloads and
live state distinguish captured spellcards, input bytes, playback and recording
cursors/end tables, FPS samples, per-frame RNG seed, and event flags.
Supervisor now exposes startup-worker result, FPS-display suppression, and
recording/playback FPS warnings.  Effect initialization/update callbacks,
quality selection, pool slots, Spellcard time limits, ANM flags, and Item
collection bounds use their asserted owners.

All safely attributable absolute views are closed: Background stage 6B,
GameManager gameplay-frame reads, Player singleton Bomb/input reads,
`g_Player.damageAccumulatorThreshold`, and ECL opcode 148's nested CATK maximum
bonus now use aggregate members.  Their COFF owner/addend relocations are
explicit in the manifests; signed target comparisons remain signed.  Broad
focused replay passes **319 / 319**, a required single-job cold replay passes
**1,106 / 1,106 exact**, the normal VC7 image links, and the complete i386
Linux build plus fixed-layout verifier passes.  The router now reports 5 raw,
0 absolute, 166 anonymous, and 44 opaque candidates.  The five raw candidates
are deliberately retained evidence gaps in Bullet/template and SHT item
collection state.  MIDI and SoundPlayer are the next dense semantic owners.

The MIDI/SoundPlayer protocol batch is now accepted.  MIDI track and output
state distinguish data/running-status cursors, event ticks, loop points, tempo
epochs, elapsed ticks/milliseconds, pending long messages, controller/meta
events, device/transposition state, and fade handling.  SoundPlayer command
records use typed preload/load/stop/release/fade/pause/unpause/volume opcodes;
their arguments, paths, per-effect queue counts, preloaded BGM storage, loaded
slot, and streaming-file base offset now have shared owners across GameManager,
Supervisor, SoundPlayer, and zwave.  Fields with only reset or no authored
consumer remain explicitly neutral.

Focused replay across the five affected objects passes **169 / 169 exact**,
including `MidiOutput::ProcessMsg` at **1,871 / 1,871** bytes and
`SoundPlayer::ProcessQueues` at **2,358 / 2,358** bytes.  A required single-job
cold replay passes **1,106 / 1,106 exact**, the normal VC7 image links, and the
complete i386 Linux build plus fixed-layout verifier passes.  The router now
reports 5 raw, 0 absolute, 121 anonymous, and 44 opaque candidates.  The next
batch should continue from behavior-backed UI/result or remaining manager
state; do not invent meanings for the residual reset-only audio words.

The Replay file-envelope/callback batch is now focused-accepted.  Generic
callback names now state whether a callback captures frame sync,
records input/FPS, plays ordinary or extended input/FPS, or controls playback
frame advancement.  Recording/playback stage setup and manager teardown have
matching lifecycle names, and their chain members share those roles.  Header
`+0x06/+0x07/+0x15` now expose extended-record selection, the appended USER
section marker, and the byte-obfuscation key; the random header/payload bytes
remain deliberately role-neutral.

Focused ReplayManager replay passes **17 / 17 exact**, including RegisterChain
**777 / 777**, recording setup **1,315 / 1,315**, playback setup **787 / 787**,
and all accepted callback bodies.  The sole authored backlog entry is still
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`: its behaviorally
reconstructed six-byte-record source emits 362 bytes versus the 361-byte target
because of the documented VC7 register-allocation phase.  It is not added to
the exact ledger.  The required single-job cold replay passes **1,106 / 1,106
exact**, the normal VC7 image links, and the complete i386 Linux build plus
fixed-layout verifier passes.  The whole-source router is now 5 raw, 0
absolute, 116 anonymous, and 44 opaque candidates.

The ScreenEffect lifecycle/mode batch is now accepted.  The formerly numeric
mode family now distinguishes playfield pulses, full/playfield loading-overlay
holds, and amplitude-envelope shake.  The pulse, draw, envelope, initialization,
deletion, and release functions all have behavior-based names; `+0x10` is the
shared overlay alpha and `+0x24` is the loading-overlay release request.  The
three variant words retain neutral storage names underneath their already
typed TU-local views, and unused `+0x0C` remains unknown.

ScreenEffect's two focused selections pass **16 / 16** and **5 / 5 exact**;
the affected ECL/Player/Supervisor production selection passes **132 / 132**.
A required single-job cold replay passes **1,106 / 1,106 exact**, the normal
VC7 image links, and the complete i386 Linux build plus fixed-layout verifier
passes.  `ScreenEffect.cpp` has zero router candidates; the whole-source router
is now 5 raw, 0 absolute, 113 anonymous, and 44 opaque candidates.

The Effect factory/draw/trail batch is now accepted.  Factory names distinguish
the primary 0x200-entry ring, secondary `[0x200, 0x280)` pool, and fixed slots
at `slotIndex + 0x280`, including their velocity-taking variants.  Bullet and
Background callers now name their draw-layer ownership; stage-origin
compensation, stage-position adjustment, resource load/release, and the
animation-end predicate use behavior-backed names.  Directional-offset and
radial-trail callbacks now expose their exact initialization, synchronization,
timed/fading, anchored, and alternate-layer roles.

The complete EffectManager selection passes **52 / 52 exact**.  A required
single-job cold replay passes **1,106 / 1,106 exact**, the normal VC7 image
links, and the complete i386 Linux build plus fixed-layout verifier passes.
Six stage-specific particle callbacks remain address-named because their exact
bodies prove motion and camera relations but not stable visual identities.  The
whole-source router remains at 5 raw, 0 absolute, 113 anonymous, and 44 opaque
candidates; `EffectManager.cpp` itself has zero candidates.

The accepted ANM render batch names the central 2D, camera-facing, and 3D quad
projection/draw paths at `0x00463470..0x004640E0`, the horizontal/vertical
texture-strip initializers, and the buffered sprite-quad queue.  It also closes
`AnmVm::IsStopped @ 0x004396F8` and the effect-table callback
`UpdatePulsingRadialTrail @ 0x0040EB50`.  Target dataflow proves the projection,
UV-axis, submission, and pulsing-strip roles; unused ANM storage remains
unknown.  All nine central functions and both VM helpers pass focused exact
comparison, the required cold replay passes **1,106 / 1,106 exact**, the normal
VC7 image links, and the complete i386 Linux build plus fixed-layout verifier
passes.  The old `QueueSpriteQuad` ledger ABI was also corrected from two byte
arguments to its target-proven VM/vertex pointer types.

The complete ECL EX callback table is now behavior-readable.  Twenty-nine
remaining address names at `0x004233D0..0x00425390` now expose the narrow,
medium, and wide bullet-warp barriers, bouncing-enemy motion, rotating laser
hitboxes, child-orbit formation, Reisen bullet phases, screen effects, spell
state publishers, marked-bullet triggers, scaled bullet time, and conditional
item spawning.  All 33 table functions were first replayed through target-
pinned typed packets and then pass the focused `EclExIns.obj` selection
**33 / 33 exact**.  The shared Bullet flags name `0x100000` only as an ECL EX
trigger marker; instruction-header bytes and artistic identities remain
unknown.  The required cold replay passes **1,106 / 1,106 exact**, the normal
VC7 image links, and the complete i386 Linux build plus fixed-layout verifier
passes.

The accepted Spellcard control batch closes fifteen remaining address-named
helpers and their shared flag/presentation ownership.  Capture validity,
timer-callback transition, Bomb damage, pending/captured reward state, effect
tracking, bonus updates, alternate effect style, and bonus-presentation
suppression now use named shifts and masks while preserving the target-visible
shift tests and bitfield assignments.  Twelve offset-derived VMs now identify
the player/enemy portraits, name/frame presentation, layered enemy name, and
bonus digit/frame roles; two draw-only VMs and flag bit 4 remain neutral.

Important helpers now include `HidePlayerSpellPresentation @ 0x00416130`,
`HideEnemySpellPresentation @ 0x00416160`, `DeactivateWithoutCleanup @
0x00416AF0`, `AddBonusProgress @ 0x00416B10`, `IsCaptureValid @ 0x0041FD90`,
`PrepareSpellcardForTimerCallback @ 0x0042BC50`,
`IsBombDamageEnabled @ 0x0042DFF0`, and the Bomb/death capture invalidators at
`0x0044CBA0/0x0044D150`.  The timer helper now has its proven `Spellcard *`
fastcall receiver rather than `void *`.  Fifteen target-pinned packets and the
affected eleven-object selection pass **192 / 192 exact**; the typed fastcall
helper separately passes **50 / 50**.  The required cold replay passes **1,106
/ 1,106 exact**, the normal VC7 image links, and the complete i386 Linux build
plus fixed-layout verifier passes.  The remaining evidence gaps are flag bit 4,
`unknown_044`, and the two draw-only VMs.

The accepted Ending/score-record batch makes the complete ending-script path
behavior-readable.  `ReadScriptParameter @ 0x00428890`,
`UpdateAndDrawFade @ 0x00428910`, `RunEndingScript @ 0x00428B80`, and
`LoadEndingScript @ 0x004295E0` now name their actual protocol, while the
Ending owner exposes its script buffer/cursor, ending ANM/VMs, line/page wait
timers, skip-lock countdowns, configured line delays, and next text-VM index.
`canSkipChainedEnding @ +0x2A5C` is set from the already-unlocked route in
`AddedCallback @ 0x00429980` and copied into the active skip gate after opcode
`@F` loads another ending script.

The persisted score chapter header now exposes `chapterSize` and the second
size word that every authored producer writes equally as `chapterSizeCopy`;
CATK `historyChecksum @ +0x0E` names the byte checksum
seeded by the spell name and accumulated over both history arrays.  CLRD
`pendingEndingSkip @ +0x20` is deliberately conservative: target-wide `.text`
inspection finds only the non-clear Ending read followed by an immediate clear,
and no TH08 1.00d producer.  Ending `+0x2A98` and the seven remaining ScoreDat
fields stay neutral because current evidence does not prove their roles.

Focused replay passes Ending **11 / 11 exact** and the six directly affected
ScoreDat/SpellCard/ResultScreen/GameManager/Supervisor/TitleScreen objects
**172 / 172 exact**.  The required final single-job cold replay passes **1,106
/ 1,106 exact**, the normal VC7 image links, and the complete i386 Linux build
plus fixed-layout verifier passes.  The four-file Ending/ScoreDat router falls
from 14 anonymous identifiers plus one opaque range to seven anonymous
identifiers plus the retained opaque range; the whole-source router is now 5
raw-member, 0 absolute-address, 106 anonymous-identifier, and 44 opaque-storage
candidates.  These remain routing counts, not a completion percentage.

The accepted ResultScreen/high-score batch closes the current-run score marker,
PSCR serialization gate, score-list operations, menu phase timer, shared name
keyboard selection, saved-name presence, spell-card exit, and displayed-time
refresh state.  Target writes/reads prove that `Th8k::runtimeMarker @ +0x09`
marks the current run only in memory and is cleared immediately before HSCR
serialization.  `Pscr::shouldSerialize @ +0x175` gates copying the full PSCR
chapter, but its nonzero gameplay producer remains unknown.  ResultScreen
offset assertions pin `statePhase @ +0x10`, `statePhaseTimer @ +0x18`,
`keyboardSelection @ +0x2C`, `hasSavedLastName @ +0x50`,
`isExitingSpellcardResults @ +0x54`, and
`lastDisplayedTotalSeconds @ +0x19C`.

`ScoreDat::InsertScore @ 0x0045A500` and the ResultScreen wrapper now state the
descending-list insertion/rank behavior; both release helpers explicitly name
that they free score-list nodes.  Eight post-rename typed packets replay exact.
Fresh focused replay passes ResultScreen **30 / 30 exact** and ScoreDat **13 /
13 exact**.  The required final single-job cold replay passes **1,106 / 1,106
exact**, the normal VC7 image links, and the complete i386 Linux build plus
fixed-layout verifier passes.  Four-file anonymous candidates fall from 15 to
12 and the whole-source router is now 5 raw, 0 absolute, 103 anonymous, and 44
opaque.  `Hscr +0x166` and the reset-only/unused ResultScreen fields remain
unknown rather than receiving speculative names.

The residual raw-member batch closes all five remaining router-selected raw
offset expressions.  `ItemManager::OnUpdate @ 0x00440500` now reads the
asserted `PlayerRawShtFile::itemMovementSpeed @ +0x34` from the primary or
secondary SHT according to `Player::focusMode`; target dataflow uses it as
the ordinary/scattered item movement and gravity step, distinct from
autocollection speed.  `SpawnSingleBullet @ 0x0042F5F0` addresses the remaining
`Bullet +0xDBC` and template `+0xD40` bytes through asserted direct members.
Target-wide inspection still finds only one initialization and one copy with
no consumer, so both Bullet names remain explicitly unknown.

Post-change typed packets for both complete functions replay exact.  Fresh
focused replay passes ItemManager **19 / 19 exact** and BulletManager **24 / 24
exact**.  The required final single-job cold replay passes **1,106 / 1,106
exact**, the normal VC7 image links, and the complete i386 Linux build plus
fixed-layout verifier passes.  Whole-source raw-member and absolute-address
candidates are now both **0**.  Replacing two opaque Bullet bytes with direct
neutral members intentionally migrates repeated occurrences to the anonymous
category, so the remaining router totals are 110 anonymous and 42 opaque; no
semantic meaning is claimed for those bytes.

The address-name/ECL-type closure is now accepted.  `EclRawHeader::timelineCount`
names the word consumed by the exact ECL load and accessor path.  The loading
completion transition, Enemy attachment-chain teardown, wrapped-angle
interpolator, and random-biased ECL move helper now have behavior-backed names.
Ten Player Bomb callbacks expose their radial geometry and lifecycle roles,
with the portable callback bridge updated to the same symbols.  Duplicate
`EnemyUnkStruct2/3` shells are gone: the primary context, sixteen-entry call
stack, and target constructors now directly share `EnemyEclContext` and
`EnemyEclInterpolationSlot`.

Target-pinned packets and focused comparison replay every renamed function
exact, including the 1,336-byte Enemy chain helper, 630-byte movement helper,
and all ten Bomb callbacks.  The required single-job cold replay passes
**1,106 / 1,106 exact**, the normal VC7 image links, and the complete i386
Linux build plus fixed-layout verifier passes.  The final router totals are
**0 raw-member**, **0 absolute-address**, **107 anonymous-identifier**, and
**41 opaque-storage** candidates.

The follow-up production/probe/modern audit found and closed a gap outside the
router's categories: `TitleScreen.cpp` still contained the repository's only
two game-source `STUB` bodies while their complete accepted implementations
lived in `TitleReplayMenuProbe.cpp`.  `UnlockLastWordSpellCards @ 0x0046CBBB`,
`ConvertToFullWidthDigits @ 0x0046D763`, and `FormatSpellCardInfo @ 0x0046D7F9`
now have shared source owners consumed by both production and the exact probe.
The production Title owner also restores the full-width number buffer, 18
Last Word comment records, and six difficulty strings.  The target-pinned
`verify-title-spell-card-data.py` check passes **18 / 18 records and 6 / 6
strings exact**.

The same batch names the ECL interpolation slot's callback index and four float
parameters, the Title spell-info reveal countdown, and the GameManager
practice-mode bit used by score initialization.  Four modern-runtime literal
target-data writes now use mapped Linux-local ABI aliases.  Focused replay
passes all **11 / 11** Title probe units, RunEcl **26,638 / 26,638**, and score
initialization **552 / 552**.  The required non-reuse cold replay passes
**1,106 / 1,106 exact**, the normal VC7 image links, and the complete i386
Linux build plus fixed-layout verifier passes.

This closes the current whole-source semantic-readability milestone more
strictly: game production source contains no `STUB` marker, and scans including
probes and modern sources contain **0 raw-member** and **0 absolute-address**
candidates.  They retain 100 anonymous and 41 opaque candidates; production
alone retains 99 and 41.  Those entries were reviewed as reset-only,
unconsumed, padding, or reserved storage and remain neutral.  The six
stage-specific Effect callbacks also remain address-named because target
bodies do not prove stable visual identities.  Future semantic work must start
from new producer/consumer or runtime evidence, not from heuristic category
size.  Re-audit the evidence boundary with:

```bash
python3 scripts/analysis/report-semantic-debt.py --include-probes --include-modern
python3 scripts/analysis/verify-title-spell-card-data.py
```

A final production-symbol audit also corrects twelve ledger-only namespace
identities.  Eleven PlayerShot free callbacks and `InitializeScoreData` were
already emitted in `Player.obj`/`GameManager.obj` as `th08::...`, but four
canonical CSVs still used unqualified names.  Qualifying those rows removes all
twelve false "not present in detour input objects" warnings.  Focused replay
remains Player **51 / 51 exact** and GameManager **33 / 33 exact**; no source,
address, layout, match-unit symbol, or accepted count changed.  The remaining
20 detour warnings are limited to the generator's unsupported operator and
compiler-generated deleting-destructor spellings.

The scheduler follow-up names the complete calc/draw priority protocol instead
of leaving engine ordering as literals `0..21`.  Every normal owner
registration now identifies its execution/draw layer, including Replay's four
separate calc roles and the paired Background, Enemy, Player, and Ascii draw
layers.  Normal gameplay ScreenEffects identify draw priority 21; the special
Supervisor-only priority 1 path stays literal pending stronger evidence.
`ChainElem +0x18` is now the narrowly evidenced `releaseTarget`, with size and
offset assertions.  TH08 target instructions show it initialized to self,
filled with live nodes in `ReleaseSingleChain @ 0x0043CC60`, and read back for
the release pass.

GensokyoClub's priority table was used only as a naming hypothesis; each name
was checked against the current exact registrant and immediate value.  Focused
Global replay passes **44 / 44 exact**.  Because `Global.hpp` is shared, a new
single-job cold build of all 75 comparison objects passes **1,106 / 1,106
exact**.  The normal VC7 image links, and the complete i386 Linux container
build plus fixed-layout verifier passes.  Router totals remain **0 / 0 / 100 /
41** with probes and modern source, or **0 / 0 / 99 / 41** for production.

Three live Supervisor flag roles are also closed.  Bit 1 is
`lockableBackbuffer`: `InitD3DRendering @ 0x004424C0` sets the corresponding
D3D presentation flag and the bit, while all consumers gate pause/retry menu
captures.  Bit 6 is `dummyMidiTimerEnabled`: the startup-info branch controls
it and the startup thread uses it only to create/start that timer.  Bit 8 is
`scoreBackupPending`: it guards one startup `score.dat` backup rotation and is
then cleared.  The four-byte flags word and its `Supervisor + 0x1A4` offset are
asserted; bit 5 remains unknown because no writer has been observed.

A stale focused PCH first rejected the new names, as expected for this shared
header.  The proof run discarded it: a fresh single-job build of all 75 objects
passes **1,106 / 1,106 exact**.  Rebuilt AsciiManager/main/Supervisor diagnostic
replay passes **134 / 134 exact**, the normal VC7 image links, and the complete
i386 Linux container build plus fixed-layout verifier passes.

The ResultScreen entry protocol is now explicit at every caller.  Mode 0 opens
the standalone result browser, mode 1 handles the post-game result flow, and
mode 2 performs synchronous score-data persistence without installing chains.
`RegisterChain @ 0x004582A0` keeps its original `u32` ABI while named enum
values replace all raw mode arguments and comparisons.  The function passes
**395 / 395 exact**; the required cold 75-object replay passes **1,106 / 1,106
exact**, the normal VC7 image links, and the complete i386 Linux container
build plus fixed-layout verifier passes.

TitleScreen's two registration arguments are also explicit, with a narrower
claim: standard entry passes 0 and replay-completion return passes 1, but
`RegisterChain @ 0x0047146D` only homes and never reads the `i32` argument.
`TitleScreenRegistrationReason` documents those caller reasons without
claiming different callee behavior.  Focused replay passes Title **281 / 281
exact** and Supervisor **1,831 / 1,831 exact**.  The required cold 75-object
replay passes **1,106 / 1,106 exact**, the normal VC7 image links, and the
complete i386 Linux container build plus fixed-layout verifier passes.

The PBG archive loader now names its encoded entry count, encoded table offset,
and encoded table decompressed size, and preserves the per-entry fourth word as
neutral `unconsumedMetadata` because retail copies but never reads it.  Header
and entry sizes/offsets are asserted.  Focused `PbgArchive.obj` replay passes
**15 / 15 exact**.  The required cold 75-object replay passes **1,106 / 1,106
exact**, the normal VC7 image links, and the complete i386 Linux container build
plus fixed-layout verifier passes.

The Enemy/ECL attachment and laser-slot family is now typed end to end.
`Enemy + 0x04/+0x08` expose the doubly linked attachment chain used by ECL
insertion, traversal, and unlinking; `+0x3280` is a 32-entry Laser pointer
array and `+0x3300` its selected slot.  ECL Laser controls and primary/secondary
ANM VM users now use their asserted owners instead of raw byte views.  Opcode
169's position view remains intentionally offset-shaped to preserve six
target VC7 x87 bytes, and child ECL blocks remain byte-oriented beyond their
three proven internal offsets.  Focused replay passes **74 / 74 exact**, the
parent-chain cleanup packet passes **1,336 / 1,336 bytes exact**, and the
required cold 75-object replay passes **1,106 / 1,106 exact**.  The normal VC7
image links; the complete i386 Linux build and fixed-layout verifier pass.
The semantic router now reports **0 / 0 / 100 / 39** including probes and
modern source, or **0 / 0 / 99 / 39** for production; these are candidate
counts, not semantic completion totals.

Supervisor's BGM playback interface now exposes its actual score-data
protocol.  Target PlayMusic/PlayAudio instructions treat the second argument
as an integer index into `g_GameManager.plst.bgmUnlocked`, not as a pointer;
the owner offsets `GameManager::plst @ +0x3D804` and
`Plst::bgmUnlocked @ +0x208` are asserted.  Production/probe callers no longer
cast song numbers to `char *`, replay/demo gates use named flags, and the exact
manifest records the corrected PlayMusic symbol plus the target-equivalent
`g_GameManager + 0x3DA0C` relocations.  Focused replay passes **189 / 189
exact**, including PlayMusic **218 / 218** and PlayAudio **352 / 352**; the
required cold 75-object replay passes **1,106 / 1,106 exact**.  The normal VC7
image links, and the complete i386 Linux build plus fixed-layout verifier pass.

The residual Item/ANM owner pass replaces byte views of Player shot/item
timers and option mode, GameManager character, the Item sprite z-write bit,
and AnmManager's texture factor with existing named fields.  Assertions pin
`Player::timeOrbGaugeChangeSuppressionTimer @ +0xE2ADC` and
`AnmManager::currentTextureFactor @ +0x24B8`.  A direct Float3 `.y` trial was
rejected because retail Item OnDraw calls `Float3::operator float *()` twice;
the conversion-shaped element accesses remain documented to preserve its
451-byte extent.  Focused AnmManager/ItemManager replay passes **106 / 106
exact**, and the required cold 75-object replay passes **1,106 / 1,106 exact**.
The normal VC7 image links; the complete i386 Linux build and verifier pass.

The Enemy chain-helper ownership follow-up removes the final fake helper class
from this family.  `HasParentChain @ 0x0041F000`, `HasAttachedEnemy @
0x0041FD20`, and `CountParentChain @ 0x0041FD40` are now real `Enemy` members:
their complete target bodies consume only the asserted attachment fields, and
all Enemy/ECL/GUI callers pass an `Enemy *`.  Their target-neighborhood
placement in EclManager is retained as TU evidence rather than being confused
with class ownership.  Both ECL operand resolvers also use typed Enemy chain
links and `Player::position.x/y/z`; unused raw field macros and a private vector
shell are gone.  The temporary resolver/detach adapter described at this
checkpoint was removed by the later Enemy/ECL owner migration.

Focused replay across the seven affected objects passes **127 / 127 exact**;
the helper bodies pass **53 / 53**, **28 / 28**, and **70 / 70**, and the
1,336-byte cleanup caller remains exact.  The required non-reuse cold replay
passes **1,106 / 1,106 exact**, the normal VC7 image links, and the complete
i386 Linux container build plus fixed-layout verifier passes.  Mapping and
accepted ledgers now record the real Enemy identities without changing any
address, size, status, or count.

The Player focus/option/gauge pass closes three gameplay timer roles and the
shared option state machine.  `focusMode @ +0x03` names unfocused, focused, and
the initialization sentinel.  `gaugeShiftDelayTimer @ +0xE2AD0`,
`timeOrbGaugeChangeSuppressionTimer @ +0xE2ADC`, and
`shootingGaugeChangeRampTimer @ +0xE2AE8` now express the complete observed
shooting/idle gauge ramp and time-orb suppression protocols across Player,
Enemy attachment rewards, and Item collection.  The option dwords at
`+0x2C8/+0x2CC` expose the common inactive/initializing/active/exiting
lifecycle and the homing-specific behavior state.  Production, Bomb, and
probe switches use named enums; three unaccessed alignment ranges are marked
as padding, while unconsumed storage remains neutral.

Focused replay across the five affected production objects passes **200 / 200
exact**, including `UpdateMovementAndOptions` **4,769 / 4,769**, every option
callback, and Enemy cleanup **1,336 / 1,336**; the PlayerOption probe compiles.
The required non-reuse cold replay passes **1,106 / 1,106 exact**, the normal
VC7 image links, and the complete i386 Linux build plus fixed-layout verifier
passes.  No ledger identity/count changed.

Whole-executable TU/layout work below remains deferred, not invalidated.

## Active playable-port branch

`port/modern-windows-linux` is the independent playable reconstruction lane.
It does not replace the VC7 exact build or change authored/library ledgers.

Playable-port state on 2026-08-24:

- CMake compiles and links the complete production-authored source set with the
  32-bit MinGW toolchain into `build/modern-windows/th08-modern.exe`;
- the output is an i386 Windows GUI PE and has no MinGW support-DLL dependency,
  but the user reports that the current native BAT/startup path does not launch
  a usable application. Prior smoke observations are insufficient to call the
  Windows product verified; Windows remains in progress;
- `--data-dir <directory>` is the intended Windows interface for a Unicode
  directory containing `th08.dat` and `thbgm.dat`, but it must be revalidated
  as part of the native startup fix;
- the playable lane restores target-proven global ownership for the active
  `GameManager` state and playfield bounds, uses relocatable function symbols
  for Player option/shot/bomb callback tables, and connects the complete
  `EnemyManager::OnUpdate` implementation to the calculation chain;
- the MinGW bring-up build temporarily uses the non-redistributable SDK
  `d3dx8d.dll`. A distributable Windows build must replace the remaining D3DX
  calls;
- CMake also builds `th08-modern` as a native i386 ELF using repository-owned
  SDL2/OpenGL compatibility backends. `scripts/build-modern-linux.sh` is the
  default host multilib entry; the i386 container is optional CI/build
  isolation rather than a runtime dependency;
- native i386 ELF execution under WSLg has completed a full Lunatic story
  endurance pass, with player/enemy/bullet simulation, score awards, Boss
  bars, Bomb animations, focus marker, Stage 2 fog, Japanese dialogue,
  keyboard input, sound effects, streamed WAV BGM, and the ending transition
  active. The endurance run used an external GDB command that suppressed life
  decrement only; that test aid is ignored and never shipped. No fatal signal
  occurred. A real Linux desktop with hardware OpenGL remains additional
  release coverage;
- `th08-layout.ld` now aliases target-owned manager, callback-chain, and known
  aggregate-field views to their original addresses. This prevents duplicate
  spell/ECL callbacks across reloads, which had manifested as negative spell
  bonuses, saturated scores, missing scripted visuals, and later-stage exits;
- the OpenGL backend uses a texture-backed FBO plus a stable dialogue-entry
  snapshot, explicit eye-space linear fog, and independent RGB/alpha
  texture-stage combiners. A Stage 4-to-5 transition can still tile the dynamic
  player-name texture across the frame/HUD; it is documented as a deferred
  Linux renderer issue rather than a release blocker. Fatal Linux signals
  write a symbolizable `modern-crash.txt`; MIDI remains a follow-up;
- `scripts/setup-modern-linux.sh <data-directory>` is the one-command
  Debian/Ubuntu setup/build/run entry, while `scripts/play-modern-linux.sh`
  performs the normal incremental build and launch. Both accept only the
  original data directory and require `th08.dat` plus `thbgm.dat`;
- `.github/workflows/portable-linux.yml` builds and verifies the i386 ELF and
  uploads a tar archive with a standalone data-directory launcher. No original
  executable or archives enter the artifact. The project-owned Touhou Lab icon
  is copied beside the ELF and loaded as the SDL window icon;
- after the endurance pass, a fresh normal container build was linked without
  GDB or runtime patches, passed the ELF/layout/dependency/package checks, and
  was user-smoke-tested successfully under WSLg. The CI portable package was
  subsequently user-tested in a Kali Linux x86-64 GUI VM with native filesystem
  data, low memory, and no 3D acceleration. Normal life decrement remains
  active in this build;
- the Kali artifact's clean-data-directory exit was reproduced locally with
  the exact CI package: status 139, fault address `0xffffffff`, and a stack from
  `FindClose` into `Supervisor::StartupThread`. The report was written under
  `backup/modern-crash.txt` because the authored score-rotation code had already
  changed directories. The Linux `FindClose` backend now returns `FALSE` for
  null, `INVALID_HANDLE_VALUE`, or wrong-kind handles instead of deleting an
  invalid pointer. A clean two-DAT regression with an empty backup directory
  stayed alive, created the first score backup, and continued through the title
  assets. `th08.exe` is not opened; an existing `th08.cfg` is only useful for
  avoiding slow fullscreen calibration on unaccelerated VMs.

Reproduce the build with:

```bash
cmake -S . -B build/modern-windows -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/modern-windows --parallel 1
```

Run the executable without copying or linking the original archives:

```text
th08-modern.exe --data-dir "D:\path\to\the\original\TH08\directory"
```

Build and run the native Linux target with:

```bash
scripts/setup-modern-linux.sh "/path/to/the/original/TH08 directory"
```

After first-time dependency setup, use
`scripts/play-modern-linux.sh "/path/to/the/original/TH08 directory"`.

The optional isolated build is `scripts/build-modern-linux-container.sh` and
outputs `build/modern-linux-container/th08-modern`.

See `docs/LINUX_PORTING.md` for the platform boundary, environment overrides,
validation matrix, and reusable lessons from the Linux reconstruction.

The selected directory becomes the runtime working directory, matching the
original game's relative-file behavior for configuration, score, replay, and
BGM files. Keep the executable, SDK DLL, generated test data, and runtime
screenshots under `build/` or outside the repository; never commit the original
archives.

## Current status

As cold-built and replayed on 2026-08-26 against the original Japanese TH08
1.00d target:

- authored source: **1,107 / 1,107 functions**, **459,757 / 459,757 bytes**;
- strict authored exact: **1,106 / 1,107 functions**, **459,396 / 459,757 bytes**;
- library inventory: **1,119 classified functions**; all **1,119 / 1,119** now have
  mapping sizes totaling **217,165 bytes**; the independent library exact ledger currently accepts **260 functions / 52,955 body bytes**;
- `config/claims.csv` is header-only;
- a cold normal VC7 build links `build/th08.exe` successfully;
- a cold objdiff build followed by full replay passes **1,106 / 1,106**
  accepted units;
- the completed production-layout passes cover the Ascii target clusters,
  Player bomb/main regions, misplaced `main.cpp` helpers, Anm render helpers,
  Global math helpers, SoundPlayer helpers/tail, and GameManager helpers/main
  region. `Player.obj`, `GameManager.obj`, `Global.obj`, and `SoundPlayer.obj`
  now each have zero target-order inversions. Canonical owners record the
  section-defined production copies rather than stale probe/objdiff donors;
- the whole-image comparator is in place. After the first evidence-backed link
  contract repairs, the rebuild has the target's DLL descriptor order, no debug
  directory, and the same two resource paths and resource section extent. The
  placeholder icon payload intentionally remains different;
- repository-owned target-independent CI passes.

Regenerate or verify these values with:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
python3 scripts/validate-tracking.py --require-target
python3 scripts/analysis/verify-exact-units.py --all --json \
  > build/accepted-unit-replay.json
python3 scripts/progress.py --check
python3 scripts/ci.py
```

`verify-exact-units.py --all` performs its own single-job cold build. Do not add
`--reuse-build` when attesting aggregate totals.

## Authored exact backlog

One authored function remains unaccepted. Do not count it as exact or
restart broad brute-force matrices without a new target-backed hypothesis.

- `ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`, 361 bytes: the natural
  source object is one byte longer and the residual is a register-allocation
  phase difference. Its adjacent callback family is exact.  The ordinary
  pointer/cast/increment/local-order variants are exhausted and recorded in
  `BUILD_MATCHING.md`; resume only from a new allocator/TU hypothesis.
`TitleScreen::RegisterChain @ 0x0047146D` is no longer backlog.  The
GensokyoClub `ZUN_NEW(TitleScreen, "TitleInf")` hypothesis restores the target
`0x40` frame and replays the full 281-byte body plus all 20 relocations exactly.

The source for the remaining function is present and behaviorally
reconstructed. Exact coverage
will increase only when the canonical comparator returns `exact` for a natural,
evidence-backed C++ form.

The other fourteen cold regressions have been repaired and accepted. Four
TitleScreen functions required production-source restoration. Seven other
functions were already byte-exact but had stale relocation identities in
`match-units.toml`. `Supervisor::IsFogDisabled` required restoring its original
header-inline definition: VC7 then emits the target `0x1A` COMDAT in
`main.obj`, while the calling `AsciiManager` function remains exact. The
`AnmVm::SetZRotation` and `GameManager::CollectExtend` then required bounded
production-source owner/value-flow fixes. The 2026-08-20 single-job cold replay
verifies all **1,105 / 1,105** accepted units.
See `BUILD_MATCHING.md` before changing a shared inline/out-of-line contract.

## Library foundation (paused except for whole-link blockers)

The seven previously missing library extents are repaired.  The canonical
selector now reports no missing-size rows:

```bash
python3 scripts/analysis/report-reconstruction-status.py \
  --category library --state missing-size --sort address
```

The six D3DX rows were reconciled against the target CPU dispatch tables and
the SHA-pinned VC7 PlatformSDK prerelease `D3DX8.LIB`
(`0d4a2b642485dcaa7671926a9a1a545c656d5eb73f160fe971b3deebf0b516b5`).
Their archive identities are SSE/SSE2 Vec3, Plane, Vec4, and Quaternion
normalize members; target body extents stop before archive alignment padding.
`operator delete @ 0x004A43CF` is the five-byte VC7 `/MT` `LIBCMT.LIB`
`delete.obj` tail jump to `_free`.  These boundary/provenance facts now feed the separate accepted library ledger; they are not authored claims.

Inventory integrity is now reconciled.  Four imported extents were stale and
were shortened at target-proven return/next-function boundaries (`0x0049E3CD`,
`0x004A51FD`, `_strchr @ 0x004A5BE0`, and `0x004A6890`).  The remaining nine
overlaps are VC7 CRT nested SEH cleanup funclets and are explicitly represented
in `config/mapping-overlaps.csv`; `validate-tracking.py` rejects stale exception
rows and reports any new unclassified overlap.

Separate library acceptance infrastructure is now in place:

- `library-provenance.toml` pins archive identity and SHA-256;
- `library-match-units.toml` records target body/compare extents, archive member,
  COFF symbol, and every relocation target base;
- `compare-library.py` verifies archive hash/member identity and replays DIR32/REL32
  relocations before comparing the complete target range;
- `library-matches.csv` is the independent accepted ledger;
- `validate-library.py` is the public CI gate, with `--require-archives` for local
  hash attestation; `library-progress.py` generates `LIBRARY_PROGRESS.md`.

Library acceptance now has **260 / 260 configured units** and **52,955 body bytes**
after canonical zero-difference replay.  The accepted set contains six SSE/SSE2
D3DX normalize helpers plus seven exact `x3d_quat.obj` quaternion helpers (including the 0xA59-byte `x3d_D3DXQuaternionSquadSetup` with 66 replayed relocations) and six exact `x3d_matx.obj` matrix helpers (including 0x1198/0x1154-byte transformation implementations), VC7 `/MT` `operator delete`, seven CRT string/memory
helpers (including strict shared-section `strchr`/`strcpy`/`strcat`), six
isolated compiler-runtime arithmetic helpers (`_ftol2`, `_chkstk`, `_aulldiv`,
`_allmul`, `_ftol`, `_aullshr`), and the `LIBCMT malloc.obj` allocation wrappers
`_nh_malloc @ 0x004A423D` / `malloc @ 0x004A4269` with explicit DIR32/REL32 replay, plus the complete currently mapped `heapinit.obj`/`sbheap.obj` core accepted so far: `_heap_init`, `__sbh_heap_init`, `__sbh_find_block`, `__sbh_free_block`, `__sbh_alloc_new_region`, `__sbh_alloc_new_group`, the 735-byte `__sbh_resize_block`, and the 764-byte `__sbh_alloc_block`.  The next accepted family also covers eight `trnsctrl.obj` C++ EH helpers, including `_UnwindNestedFrames`, `__CxxFrameHandler`, `_GetRangeOfTrysToCheck`, `_CreateFrameInfo`, `IsExceptionObjectToBeDestroyed`, `_FindAndUnlinkFrame`, `_CallCatchBlock2`, and `_CallSETranslator`.  Shared-section `exsup.obj` coverage now also accepts `_global_unwind2`, `_local_unwind2`, `_abnormal_termination`, `_NLG_Notify1`, `_NLG_Notify`, plus the isolated `__EH_prolog`.  The EH dependency chain now also accepts `_getptd`, `__InternalCxxFrameHandler`, `_CallSettingFrame@12`, and `_inconsistency`; the latter exposed and fixed a stale 0x26 mapping extent, with `hooks.obj` and target control flow proving the full 0x2D body.  `__FrameUnwindToState` is accepted with a 0xAD main-body extent and a 0xCE comparison extent so its compiler-owned cleanup alternate-entry/funclet bytes are replayed without double-counting the separately mapped `0x004AA267` funclet.  Auxless whole-section COMDAT support is now explicit/fail-closed, and `FindHandler @ 0x004AA72E` is accepted as a 0x1E2-byte static code COMDAT with a unique offset-zero function owner and 13 replayed REL32 relocations.  `tidtable.obj` thread-runtime coverage now also accepts `_mtterm @ 0x004AA049` and `_mtinit @ 0x004AA0D8` with their TLS/lock/calloc relocation graph replayed explicitly.  `mlock.obj` coverage now accepts `_mtinitlocks`, `_mtdeletelocks`, `_unlock`, `_mtinitlocknum`, and `_lock`, including the dynamic critical-section table and lazy lock initialization paths.  Its immediate dependencies now also accept `_errno`, `_amsg_exit`, and `__crtInitCritSecAndSpinCount`; the latter two required target-proven extent repairs from 0x22 to 0x25 and 0x67 to 0x8B respectively.  The message/critical-section leaf layer now additionally accepts `__crtInitCritSecNoSpinCount@8`, `_FF_MSGBANNER`, and `_NMSG_WRITE`; the 0x177-byte message writer replays 30 relocations including CRT string helpers, Win32 I/O, and the security-cookie check.  Its immediate code dependencies now also accept `strncpy`, `__crtMessageBoxA`, and the 14-byte `seccook.obj` security-check-cookie helper at `0x004AB43C`; that last row remains conservatively named `FUN_004ab43c` until a repository-wide fastcall-decoration naming rule is fixed.  `_report_failure` is now accepted with its full 0x31-byte auxless COMDAT extent, and `_calloc` with its full 0xBB-byte VC7 function-definition extent; `calloc` deliberately retains the legal nested cleanup-funclet overlap rather than truncating the parent epilogue.  `trnsctrl.obj` also exposed a missing `CatchGuardHandler @ 0x004A44A1` inventory row and an undersized `TranslatorGuardHandler @ 0x004A44C5`; both auxless COMDATs are now accepted at 0x24/0x71 bytes.  `frame.obj` coverage now also accepts `TypeMatch`, the repaired 0x45-byte `__DestructExceptionObject`, `CatchIt`, and `FindHandlerForForeignException`.  `_JumpToContinuation` now replays a 0x30-byte COFF section while counting its 0x2B main body, and the linked 0x004A4412 helper is accepted through the existing `CallMemberFunction1` row; archive Function0/1/2 symbols are identical 7-byte COMDAT aliases of that linked copy.  `CallCatchBlock` and `BuildCatchObject` are now accepted with 0x97/0x170 body extents while replaying their complete 0x1A7/0x17C auxless COMDAT sections, which include separately mapped EH funclets/tails.  Their direct support layer now also accepts `AdjustPointer`, `ValidateRead`, corrected `ValidateWrite`, `ValidateExecute`, and the repaired full 0x33D-byte `memmove`; `FUN_004AA427` is now accepted through the explicit tail-local-funclet schema, pinned to `$L19653` and its `CallCatchBlock` parent section rather than forced through the normal function schema.  The same schema now accepts `FUN_004AA267` via `$L19568` at the tail of `__FrameUnwindToState`; `terminate @ 0x004AA9B2` is also repaired from 0x2E to the full 0x35-byte `hooks.obj` function and accepted with two `__getptd` calls plus the final `_abort @ 0x004B05BD` tail jump.  The locale/thread pair now accepts `__updatetlocinfo_lk @ 0x004A9F4C`, `__updatetlocinfo @ 0x004AA00E` with a 0x32 body/0x3B comparison extent, and its 9-byte `FUN_004AA040` tail-local cleanup; `_abort @ 0x004B05BD` is also accepted as a 0x17 body with the final `int3` included only in its 0x18-byte comparison extent.  This chain now continues through `__freetlocinfo @ 0x004A9E82`; `_abort` dependencies `_exit @ 0x004A6B6A` and `_raise @ 0x004B17F2` are accepted as well, with `_raise` repaired from 0x164 to its full 0x179-byte parent extent while retaining the legal nested 0x004B192D cleanup funclet.  The locale teardown chain now also accepts `__free_lc_time @ 0x004AFA38`, `__free_lconv_num @ 0x004AFBC8`, and `__free_lconv_mon @ 0x004AFC27`, closing the direct heap-release dependencies of `__freetlocinfo`.  Signal/ctype coverage accepts static `siglookup @ 0x004B17C4` and the `isdigit/isxdigit/isspace` leaf family.  Do **not** accept `signal @ 0x004B196B`: `winsig.obj::_signal` has a 0x1A9-byte function-definition section, but replay at that target start has hundreds of non-relocation byte differences; the current row remains an unresolved inventory/provenance item rather than an exact claim.  Time/locale formatting coverage accepts `_Strftime_mt`, `_Strftime`, `strftime`, `localtime`, and `time`; the archive-proven VC7 symbol corrects the imported `_time32` mapping name to `_time`. Formatting/small-CRT coverage also accepts `vsprintf`, `sprintf`, `sscanf`, `atol`, `rand`, and `purecall` from their pinned `LIBCMT` members; the stdio wrappers replay their `__output`/`__flsbuf` or `strlen`/`__input` relocations explicitly rather than masking them. Time-runtime dependencies now also accept `__tzset`, `_isindst`, and `gmtime`; termination coverage accepts `_onexit_lk`, `_onexit`, its separately mapped 6-byte tail cleanup funclet, and `atexit`. The internal timezone lane now also accepts `_tzset_lk` and `_isindst_lk` as auxless static `tzset.obj` COMDATs, replaying 58 and 30 relocations respectively; `_isindst_lk` exposes `_cvtdate @ 0x004AADC8` as the next direct dependency. That dependency is now accepted as the full 0x1B8-byte static `_cvtdate` COMDAT; `_tzset_lk` also closes through exact `_getenv_lk @ 0x004B05D5`. The environment conversion chain now continues through exact `_mbsnbicoll @ 0x004B1FC8` and archive-proven `__wtomb_environ @ 0x004B2016`, each with four explicit relocations.  String/heap support now accepts the repaired 0x88-byte `strcmp` plus `_msize` as a 0x6A main body with a 0x76 comparison extent and a separately accepted 9-byte cleanup tail. Startup/termination coverage now also accepts `__crtExitProcess`, `_lockexit`, `_unlockexit`, `_cinit`, `doexit`, public `exit`, `_cexit`, and the previously missing `_c_exit @ 0x004A6B8A`; `__crtExitProcess` counts 0x2F body bytes while replaying the final `int3` in its 0x30-byte COFF extent. `initsect.obj` coverage now repairs `_RTC_Initialize @ 0x004ACBFE` from 0x3D to 0x44 and adds the previously missing `_RTC_Terminate @ 0x004ACC42`, both exact 0x44-byte SEH iterator functions. File-I/O wrapper coverage now repairs `_lseek`, `_write`, and `_read` from imported 0xA0 extents to their full 0xAB VC7 function-definition extents.  Each parent still contains a separately mapped nested cleanup funclet, but control flow resumes in the parent afterward; the overlap exceptions remain valid and the child does not define the parent boundary. Stdio support also accepts `write_char`, `write_multi_char`, `write_string`, `input::__inc`, and archive-proven `_ungetc_lk`; the latter replaces the imported `FID_conflict:_ungetc` label after exact `ungetc.obj` replay. File-handle support now also accepts `_lseek_lk`, `_write_lk`, `_read_lk`, `_lock_fhandle`, and `_unlock_fhandle`, closing the direct lock/worker dependencies of the repaired `_lseek/_write/_read` wrappers. Error/handle support now also accepts `_doserrno`, `_dosmaperr`, and `_get_osfhandle`, replacing the anonymous doserrno row with its archive-proven identity. Stdio/MBC conversion coverage now also accepts `_getbuf`, `_filbuf`, `wctomb`, `mbtowc`, `__wctomb_mt`, and `__mbtowc_mt`, closing the direct buffer and multibyte conversion dependencies of the accepted `output`/`input` code. File/stdio lock internals now also accept `_free_osfhnd`, `_lseeki64_lk`, `_flush`, `_fflush_lk`, `_fclose_lk`, and `_close_lk`, providing archive-proven worker identities for the remaining public wrapper lane. `_isatty` is also accepted from `isatty.obj`, closing the simple terminal-test branch used by formatted and file output paths. Public-wrapper coverage now adds the previously missing `fclose @ 0x004B2609` and repairs `_commit @ 0x004B265A` / `_close @ 0x004B2E0D` to their full 0xBC / 0x9B VC7 extents while preserving their nested cleanup overlaps. `fflush.obj` inventory repair now also adds static `flsall @ 0x004B1C69` (0xD5) and `_flushall @ 0x004B1D3E` (0x9), which exactly fill the unmapped gap after `_fflush_lk` and before `___ansicp`. X87 `common.obj` coverage now accepts nine bounded functions in one shared 0x18C code section, including the previously missing `_check_overflow_exit @ 0x004A7F35`; each match pins its section offset instead of pretending the archive member contains nine independent sections.  Continue pinning provenance/match units for
remaining VC7 CRT/runtime, standard-library, D3DX, and compiler-runtime inventory
only when a bounded whole-image difference identifies the member as a dependency.
Keep library progress separate from authored totals; do not resume broad archive
scanning merely to increase its percentage.

Keep authored `implemented.csv`, `matches.csv`, and authored percentages
unchanged.  There is intentionally no whole-library scanner until archive
identity, relocation policy, COMDAT/padding rules, and failure modes are encoded.

## Current milestone: whole-executable reconstruction

The normal executable links, so whole-image differences now select the work.
Run the canonical cold baseline with:

```bash
python3 scripts/build.py --fresh
python3 scripts/compare-whole-image.py --json \
  > build/whole-image-report.json
```

The 2026-08-20 baseline after restoring the target-proven ICF link contract is:

- file size: target **840,704**, rebuild **817,152** bytes, so the rebuild is
  **23,552 bytes (`0x5C00`) smaller**;
- entry point: target `0x004A619E`, rebuild `0x00498EAB`;
- `.text` virtual/raw: target `0xB1B78 / 0xB1C00`, rebuild
  `0xA8C4F / 0xA8E00`; the raw-code deficit is `0x8E00`;
- `.rdata` virtual/raw: target `0x11284 / 0x11400`, rebuild
  `0x14FDC / 0x15000`;
- `.data1` raw bytes and extent are exact; `.data` and the rest of the image are
  not exact;
- `.rsrc` virtual/raw extents are both `0xD60 / 0xE00`, and its two paths and
  group-icon leaf agree. The 3,240-byte icon leaf retains repository-owned
  placeholder artwork, so its payload is intentionally different;
- both images have no debug-directory entries;
- DLL descriptor order agrees exactly. The target imports 155 symbols and the
  rebuild imports 172: there are no target-only imports and 17 rebuild-only
  `KERNEL32` symbols. Nine are referenced by D3DX `cd3dxfile.obj` /
  `cd3dxresource.obj`; the remaining set is referenced by six linked VC7 CRT
  members recorded in `KNOWLEDGE_BASE.md`. The configured prerelease
  `d3dx8tex.obj` is now directly attested by exact auxless-COMDAT units for
  `D3DXLoadSurfaceFromFileInMemory @ 0x0047AAF3` (0xB8, four REL32 relocations)
  and `D3DXCreateTextureFromFileInMemoryEx @ 0x0047B72E` (0x40, one REL32).
  Both have zero non-relocation differences. An isolated `mssdk/lib/d3dx8.lib`
  link kept all 17 extra imports, reduced located anchors to 930 with 86 missing,
  and shrank `.text` raw size to `0x96C00`, so do not replace the pinned
  prerelease archive. The live D3DX import issue is member retention/splitting
  under `/OPT:NOREF`, not a production source API mismatch;
- 952 accepted address anchors are found in the linker map and 66 accepted units
  lack a unique map candidate. Twelve located anchors are ICF-folded aliases;
  they remain exact evidence but are excluded from TU-layout ranking because a
  shared linked RVA does not identify the contributing object's placement.
  Thirty-five production objects have accepted anchors, 34 retain at least one
  non-folded layout anchor, and the only folded-only production object is
  `utils.obj`. Anchor-derived current-object order still differs from target.
  The largest genuine intra-object drift is now `BulletManager.obj` at
  **104,240 bytes**, followed by `EnemyManager.obj` at **89,792 bytes**;
  `AnmManager.obj` falls from the former 359,792-byte apparent span to only
  **160 bytes** once its folded vertex-constructor alias is excluded.

The first bounded target translation-unit/object-partition recovery is now
complete. The old `AsciiManager.obj` placed seven exact functions totaling
`0x220` bytes between `InitializeVms` and `RegisterChain`, although the target
places them in four distant clusters: `0x00406FD0`, `0x00422BB0`,
`0x0042F2D0`, and `0x004398FF`. Their definitions now live in
`AsciiManagerGauge.cpp`, `AsciiManagerBossMarker.cpp`,
`AsciiManagerScale.cpp`, and `AsciiManagerGuiMode.cpp`, compiled under the same
`/Od` PCH profile. Canonical focused replay passed all **45 / 45** accepted
units across the donor and recipients; the subsequent full cold replay passed
**1,105 / 1,105**.

This split reduced `AsciiManager.obj` from **189 inversions / 3 target-order
runs / 225,802 bytes of drift span** to **0 inversions / 1 run / 2,800 bytes of
drift span**. Its first 21 anchors, through `PauseMenu::OnUpdate @ 0x004037B0`,
now have zero linked-address drift. The extracted clusters each have uniform
internal drift; their final placement depends on later splits in neighboring
GameManager, EnemyManager, and Gui ownership and is not yet exact.
The executable layout proves that these bodies cannot remain in the early
AsciiManager object; it does not yet prove that all four clusters correspond
one-for-one to standalone original source files.

The second bounded investigation separated a genuine Player TU boundary from
ordinary lexical disorder. The callback family at
`0x0040BC20..0x004142C0` was merged into the same source file as the main
Player region at `0x00449CA0..0x00451D50`, despite the large target gap and
many intervening subsystems. Its real definitions now live in
`PlayerBomb.cpp`, under the same `/Od` PCH profile, and all **56** accepted
early-range units have canonical ownership in `PlayerBomb.obj`. The main
target neighborhood is otherwise substantially continuous, so its remaining
seven runs were definition-order drift rather than evidence for six more
standalone TUs. `Player.cpp` is now ordered by its mapped target functions;
two forward declarations preserve natural C++ visibility without adding
definitions or shims.

Focused donor/recipient replay passed all **116 / 116** accepted units. The
subsequent full single-job cold replay passed **1,105 / 1,105**. The original
merged `Player.obj` had **286 inversions / 8 runs / 275,088 bytes of drift
span**. The repaired `Player.obj` has **0 / 1 / 7,760**, and
`PlayerBomb.obj` has **0 / 1 / 544**. The name `PlayerBomb.cpp` is a
repository description of the proven callback cluster, not a claim about the
original filename.

Continue target translation-unit recovery one production object at a time.
Generate and rank the detailed anchors rather than manually scanning the whole
report:

```bash
python3 scripts/compare-whole-image.py --json --include-anchor-details \
  > build/whole-image-anchors.json
python3 scripts/analysis/report-tu-partition-candidates.py \
  build/whole-image-anchors.json
```

The third bounded pass repaired a different ownership error rather than
inventing another source-file split. Fourteen small `GameManager`/`ZunTimer`
helpers had explicit definitions appended to `main.cpp`, even though target
neighborhoods and production undefined references place twelve with the early
`AsciiManager` consumer cluster, `ZunTimer::operator--(int) @ 0x00418110`
beside `SpellCard`, and `GameManager::SetClockTime @ 0x00453C60` beside replay
loading. Restoring their natural header-inline bodies makes VC7 emit exact
COMDATs from `AsciiManager.obj`, `SpellCard.obj`, and `ReplayManager.obj` under
the actual consumer profiles. Focused donor/recipient replay passed **133 /
133** accepted units, and the subsequent full single-job cold replay passed
**1,105 / 1,105**.

This reduced `main.obj` from **277 inversions / 5 runs / 316,960 bytes of drift
span** to **16 / 2 / 146,336**. All 25 anchors in the retained main region are
continuous; the remaining reset is solely the exact but currently uncalled
`ZunTimer::operator+= @ 0x0041FDF0`. It remains explicitly defined in
`main.cpp` until a target-backed production consumer or owner is recovered;
do not assign it to an unrelated ECL/SpellCard TU just to improve the metric.
The current Ascii helper cluster retains a bounded lexical/emission-order issue
(**59 anchors / 154 inversions / 7 runs / 9,600 span**) but no exact-code
regression. The later natural emissions of
`Float3::Float3(float,float,float) @ 0x00404720` and eight Sound fade/pause
COMDATs all have target-neighbor evidence; the pairwise inversion count grows
with the number of anchors and is not directly comparable to the earlier
45-anchor measurement.

The fourth bounded pass addressed `AnmManager.obj`. Seven exact helper bodies
were mixed into the donor despite target-neighbor production consumers:
`AnmVmBase::Initialize` and `SetTextureCaptureParams` route to the early
Ascii cluster, `SetZRotation` to `PlayerBomb`, `IsStopped` to `Gui`, and
`SetCameraMode`/`Draw2DAndFlush`/`GetAnm` to `Background`. Six can remain
natural consumer-emitted header bodies. `Initialize` must instead be an
explicit `/Od` definition in `AsciiManager.cpp`: exposing its body globally
changed five accepted `/Os /Ob1` Title functions during cold replay. The
expanded seven-object focused replay passed **294 / 294** accepted units and
the final full cold replay passed **1,105 / 1,105**.

The retained `0x00462270..0x00464EC0` Anm render cluster was continuous, so its
definitions were reordered by target function address instead of split into
invented TUs. `SpriteHasTexture @ 0x004622C0` deliberately remains
header-inline: changing it to out-of-line made the Anm metric perfect but
broke exact `TitleScreen::OnDraw @ 0x0047087F`, whose target contains the
inlined body and no call relocation. The accepted result reduces
`AnmManager.obj` from **195 inversions / 10 runs / 400,208 bytes of drift
span** to **20 / 2 / 359,840**. All 20 residual inversions are that required
deferred COMDAT placement; the main/render definitions themselves are in
target order.

The fifth bounded pass repaired `Global.obj`. Six exact shared math bodies were
appended between a substantially continuous Global main region and its final
global helpers. Target neighborhoods plus production undefined references
place `Float3::operator+`, `operator-`, `operator*`, and `operator/=` in
`Background.cpp`, and `VectorAngle` plus `Float3::operator/` in
`PlayerBomb.cpp`. Their explicit definitions now sit at the corresponding
target-local lexical positions without making the bodies globally visible.
`Chain::RunDrawChain` and `Chain::ReleaseSingleChain` remain in the continuous
Global TU but are lexically ordered as in the target.

Moving the six callers also stopped `Global.obj` from implicitly emitting
`Float3::Float3(float,float,float) @ 0x00404720`. The target constructor is
adjacent to `PauseMenu::OnDraw`, and `AsciiManager.obj` naturally emits the
same exact COMDAT, so its canonical match-unit owner now records that
section-defined production copy. Focused donor/recipient replay passed **191 /
191**, and the subsequent full single-job cold replay passed **1,105 /
1,105**. `Global.obj` improved from **194 inversions / 4 runs / 220,816 bytes
of drift span** to **0 / 1 / 96**.

The sixth bounded pass repaired `SoundPlayer.obj`. Eight exact fade/pause
helpers appended to the Sound donor had target addresses
`0x00406AC0..0x00406C50`, directly between the already recovered Anm and
GameManager Ascii-emitted clusters. `AsciiManager.obj` was the only production
object with undefined references to the five outer SoundPlayer wrappers. Once
the wrapper bodies in `SoundPlayer.hpp` and their three nested stream-helper
bodies in `zwave.hpp` were restored inline, VC7 emitted the exact target order:
each SoundPlayer wrapper immediately followed by its CStreamingSound helper,
then `Pause` and `UnPause`. Focused Ascii/Sound replay passed **86 / 86** and
the required full cold replay passed **1,105 / 1,105**.

The remaining Sound tail at `0x0045E2D0..0x0045E3F0` was continuous but its
three deferred header accessors appeared after the explicit constructor/free/
fade functions. All accessor callers are in the same `/Od` Sound TU and retain
target-proven call boundaries. Making `CWaveFile::GetFormat`,
`CSoundManager::GetDirectSound`, and `CSound::GetWaveFile` explicit definitions
immediately before the constructor restored the complete target lexical order;
focused replay passed **25 / 25**, followed by another **1,105 / 1,105** cold
aggregate replay. `SoundPlayer.obj` improved from **181 inversions / 4 runs /
358,864 bytes of drift span** to **0 / 1 / 0**.

The seventh bounded pass repaired `GameManager.obj`, which initially had **37
anchors / 174 inversions / 9 runs / 291,648 bytes of drift span**. It required
several kinds of evidence-backed repair rather than one artificial source-file
split:

- `SetLives`, `UpdateAntiTamper`, `SetBombCount`, and `SetPower` now emit from
  the early Ascii consumer cluster. The target places Sound helper COMDATs
  before these GameManager COMDATs, so `th_pch.h` includes `SoundPlayer.hpp`
  before `GameManager.hpp`; PCH include order is part of the VC7 emission
  contract.
- `IsSoloHuman`, `IsSoloYoukai`, and `GetLives` now emit from their real
  `EnemyManager.obj` consumer at the target `0x0042F230..0x0042F2B0`
  neighborhood.
- the retained GameManager main definitions were reordered by target address.
  `AddLives @ 0x0043C641` is an explicit definition between the constructor and
  `InitArcadeRegionParams`, matching the continuous target sequence without
  changing its exact body.
- the Player-local sequence at `0x0044E140..0x0044E348` is now explicit in
  `Player.cpp`: `SetYoukaiGauge`, `RandomizeAntiTamper`, `AddToDeaths`, and
  `AddToBombsUsed`. This both restores the target order and replaces the stale
  objdiff-only `RandomizeAntiTamper` owner with the exact production copy.
- `ScaleFloatBasedOnRank @ 0x00422B80` is the first definition in the existing
  target-contiguous `AsciiManagerBossMarker.cpp` block, immediately before the
  two Ascii functions at `0x00422BB0..0x00422C13`.

Focused replay passed **65 / 65** Ascii units for the setter move, **44 / 44**
Enemy units for the solo move, **62 / 62** Player units for the Player-local
block, **3 / 3** Ascii boss-marker units for the final cluster, and **40 / 40**
units in the final GameManager donor. Shared-header checkpoints and the final
cold aggregate replay pass **1,105 / 1,105**. The final linked metrics are
`GameManager.obj` **30 / 0 / 1 / 3,970**, `Player.obj` **36 / 0 / 1 / 7,264**,
and `AsciiManagerBossMarker.obj` **3 / 0 / 1 / 0**.

The eighth bounded pass repaired `Gui.obj`. The target neighborhood from
`Gui::OnUpdate @ 0x004338CA` through `Gui::FUN_00439810 @ 0x00439810` is
substantially continuous, so the twelve linked-order runs were lexical
definition disorder rather than evidence for twelve original source files. The
40 explicit definitions in `Gui.cpp` now follow target address order; only the
file-local helpers needed by earlier callers received forward declarations.

One residual remained after that reorder: header-inline
`AnmVm::IsStopped @ 0x004396F8` emitted at its first Gui use, placing it far
earlier than the target. Production callers are confined to Gui, while the two
PlayerOption probe callers retain target-proven REL32 calls. Making the header
declaration-only and placing the unchanged body explicitly between
`Gui::FUN_004396b8 @ 0x004396B8` and `Gui::LoadMsg @ 0x00439710` preserves all
call boundaries and the established Gui production owner. Focused replay
passed **41 / 41** Gui units and **15 / 15** PlayerOption probe units. The final
cold aggregate replay passed **1,105 / 1,105**. `Gui.obj` improved from **28
anchors / 167 inversions / 12 runs / 23,133 drift span** to **28 / 0 / 1 /
101**.

The ninth bounded pass repaired the safe lexical part of `SpellCard.obj`. Its
explicit function definitions now follow target address order across the current
production TU. Focused replay passed **34 / 34** accepted units, and the
subsequent cold aggregate replay passed **1,105 / 1,105**. The linked object
improved from **31 anchors / 122 inversions / 7 runs / 167,120 span** to **31 /
18 / 2 / 151,552**.

The 18 residual inversions are intentionally retained. The continuous Spellcard
main range through `CutChain @ 0x004180F0` is followed in the target by the
consumer-emitted `ZunTimer::operator--(int)` and GameManager spell-number
helpers at `0x00418110..0x00418180`. Current source also contains explicit
Spellcard helpers whose target homes are far away (`0x0041F040..0x0041FDD0`,
`0x0042DFF0`, and later Player-region helpers). They have no bounded production
caller/owner evidence that justifies moving them into another TU, while the
existing consumer COMDATs are already exact. Do not change header visibility or
invent owners solely to erase this metric residual.

The tenth bounded pass repaired `BulletManager.obj`. The production source had
real definitions in a highly non-target lexical order spanning the early
`0x00415C60` clear wrapper, the `0x0042A410..0x00432F20` bullet body, and the
`0x00433820..0x004338C0` lifecycle/resource tail. Sorting the existing
definitions by mapped target address required only forward declarations for
`CopyBulletAnmVmCore` and `SelectBulletSprite`; no behavior or owner changed.
Focused replay passed **38 / 38**, and the subsequent cold aggregate replay
passed **1,105 / 1,105**. Linked metrics improved from **25 anchors / 85
inversions / 4 runs / 121,968 span** to **25 / 0 / 1 / 104,240**. The large
remaining span is the real target distance between `0x00415C60` and the main
`0x0042F360+` block, not a target-order residual.

The eleventh bounded pass repaired `Background.obj`. Two tiny Background
methods were not Background-TU bodies at all in the shipped layout: the only
production callers are in `Spellcard.cpp`, and target addresses place
`background_fun_00415ce0 @ 0x00415CE0` between `StartSpell` and the cut-in
sequence, and `background_fun_00416ad0 @ 0x00416AD0` between `EndSpell` and
`DeactivateWithoutCleanup`. Their unchanged definitions and canonical match-unit
owners now live in `SpellCard.obj`. Background donor/SpellCard recipient
focused replay passed **29 / 29** and **36 / 36** respectively.

The retained Background definitions were then repaired with targeted lexical
moves rather than a whole-file sort, preserving the interspersed file-scope
arrays/globals: constructor/OnUpdate/interpolation helpers, stage-data load/
update/render, and the late resource-reload predicate now follow target order.
`Background.obj` improved from **26 anchors / 61 inversions / 6 runs / 68,640
span** to **24 / 0 / 1 / 528**. The subsequent cold aggregate replay passed
**1,105 / 1,105**.

The twelfth bounded pass repaired `Midi.obj`. Its only target-order reset was
caused by two header-defined bodies whose correct owner was already Midi but
whose deferred emission positions were wrong. `MidiTimer::OnTimerElapsed @
0x0043EF40` is the empty virtual base callback and belongs before the main
`0x00443C60+` Midi implementation; `MidiOutput::Ntohl @ 0x004444E0` belongs
between `ParseFile` and `LoadFile`. Both caller sets are confined to Midi and
the target preserves real call/virtual boundaries, so the header declarations
are now bodyless and the unchanged definitions sit explicitly at their target
lexical positions in `Midi.cpp`. Focused replay passed **33 / 33**, and the
subsequent shared-header cold aggregate replay passed **1,105 / 1,105**.
`Midi.obj` improved from **33 anchors / 43 inversions / 2 runs / 25,888 span**
to **33 / 0 / 1 / 19,728**; the remaining span is the real target gap from the
base virtual callback to the main Midi TU.

The thirteenth bounded pass repaired `EnemyManager.obj`. Its five runs mixed
ordinary explicit-definition disorder with one target-contiguous cross-class
tail. Sorting only function bodies by mapped target address (leaving file-scope
statics in place) required one natural forward declaration for
`InterpolateWrappedAngle`; focused replay remained **44 / 44** exact and reduced the
linked object to three residual inversions.

Those three inversions proved a real owner boundary. The target sequence is
`GameManager::IsSoloHuman/IsSoloYoukai/GetLives @ 0x0042F230..0x0042F2B0`,
then the recovered `AsciiManagerScale` block at `0x0042F2D0..0x0042F315`,
then `Gui::IsBossPresent @ 0x0042F320` and `Gui::FUN_0042f340 @ 0x0042F340`,
followed immediately by `BulletManager::Initialize @ 0x0042F360`. Both donor
and recipient use the same `/Od` PCH profile. Moving the unchanged Gui bodies
onto the end of `AsciiManagerScale.cpp` and canonicalizing their match-unit
owners produced an exact four-function `0x0042F2D0..0x0042F355` block. Donor
replay passed **42 / 42**, recipient replay **4 / 4**, and the cold aggregate
replay passed **1,105 / 1,105**. `EnemyManager.obj` improved from **18 anchors /
32 inversions / 5 runs / 111,744 span** to **17 / 0 / 1 / 89,616**;
`AsciiManagerScale.obj` is **4 / 0 / 1 / 0**.

The fourteenth bounded pass repaired `Supervisor.obj`. Three target-order
resets had two distinct causes. `SoundPlayer::UpdateFades @ 0x00447764` and
`GameManager::IsExtraUnlockedWithAllTeams @ 0x00447D04` already had the correct
Supervisor production owner, but header visibility made VC7 emit their exact
standalone bodies at earlier first-use positions. Their target locations are
respectively between `TakeSnapshot`/`LoadConfig` and `LoadConfig`/`LoadMusic`.
Both headers are now declaration-only and the unchanged bodies are explicit at
those target-local positions in `Supervisor.cpp`. The GameManager helper has
multiple Title callers, but their accepted units already contain real REL32
calls; focused replay passed **49 / 49** Supervisor units, **23 / 23** TitleScreen
units, and **11 / 11** TitleReplay probe units. The third reset was ordinary
lexical disorder: `UpdatePlayTime @ 0x004482A1` now precedes
`UpdateGameTime @ 0x00448418`. The cold aggregate replay passed **1,105 /
1,105**, and `Supervisor.obj` improved from **43 anchors / 29 inversions / 4
runs / 58,804 span** to **43 / 0 / 1 / 51,658**.

The fifteenth bounded pass repaired `ReplayManager.obj`. The object mixed plain
lexical disorder with two same-owner deferred header bodies. `LoadReplayData @
0x00451D90` is now the first Replay definition, the constructor moved from the
file head to its target slot after `DeleteReplayManager @ 0x00453080`, and the
existing `AppendFormat @ 0x00453B80` tail is followed by explicit
`GameManager::SetClockTime @ 0x00453C60` and `ReplayManager::IsDemo @
0x00453CC0`. The latter two were declaration-only header repairs; each has only
ReplayManager production callers. Focused replay passed **18 / 18**, the cold
aggregate replay passed **1,105 / 1,105**, and linked metrics improved from **15
anchors / 23 inversions / 4 runs / 12,352 span** to **15 / 0 / 1 / 192**.

The sixteenth bounded pass repaired `PbgFile.obj`. The sole target-order reset
was caused by the inline base-class `IPbgFile` constructor/destructor emitting
immediately after the derived `CPbgFile` constructor. The target instead keeps
the complete `CPbgFile` implementation through `GetFullFilePath @ 0x00473B40`,
then emits `IPbgFile::IPbgFile @ 0x00473C40` and `~IPbgFile @ 0x00473C60` before
the following PBG/LZSS code. Making those base bodies declaration-only in the
header and defining the unchanged bodies at the tail of `PbgFile.cpp` preserved
**15 / 15** focused units. The cold aggregate replay passed **1,105 / 1,105**,
and `PbgFile.obj` improved from **13 anchors / 20 inversions / 2 runs / 1,456
span** to **13 / 0 / 1 / 48**.

The seventeenth bounded pass repaired `EffectManager.obj`. This TU had 49
mapped function definitions in heavily scrambled lexical order while file-scope
statics and the `g_EffectTemplates` data definition were interspersed between
them. Rebuilding the source as the same declarations/data in their original
relative order followed by all real function bodies sorted by mapped target
address required only natural forward declarations for
`AdjustStageEffectDrawPosition` and `HasAnimationEnded`. No behavior or owner
changed. Focused replay passed **52 / 52**
and the cold aggregate replay passed **1,105 / 1,105**. `EffectManager.obj`
improved from **8 anchors / 17 inversions / 4 runs / 23,424 span** to **8 / 0 /
1 / 304**.

The eighteenth bounded pass repaired `ResultScreen.obj` and the adjacent
ScoreDat owner boundary. `AnmManager::ReplaceSurface @ 0x0045A3FD` has only a
ResultScreen production caller, so its header body is now declaration-only and
the unchanged explicit definition follows `MoveCursorHorizontally @
0x0045A2F2` in `ResultScreen.cpp`. `ScoreListNode::ScoreListNode @ 0x0045A4DC`
required stronger evidence: a naive move into `/Od` `ScoreDat.cpp` produced a
0x2B-byte body instead of the accepted 0x20 bytes, but placing it immediately
after `Catk::WasAttemptedWithShot @ 0x0045A4A0` under that function's existing
`#pragma optimize("s", on)` reproduced the exact body. The canonical owner now
is `ScoreDat.obj`. Focused replay passed **33 / 33** ResultScreen and **14 / 14**
ScoreDat units; cold aggregate replay passed **1,105 / 1,105**. Linked metrics
are `ResultScreen.obj` **30 / 0 / 1 / 0** and `ScoreDat.obj` **13 / 0 / 1 / 9**.

The nineteenth bounded pass repaired `PbgArchive.obj`. Two same-owner header bodies were deferred at first use rather than their target tail positions: `CPbgFile::ReadInt @ 0x004751E0` and `PbgArchiveEntry::~PbgArchiveEntry @ 0x00475270`. Making both declaration-only and defining the unchanged bodies after `CopyFileName` / after `SeekPastString` respectively preserved **15 / 15** focused units. Cold aggregate replay passed **1,105 / 1,105**, and `PbgArchive.obj` improved from **14 anchors / 11 inversions / 3 runs / 2,032 span** to **14 / 0 / 1 / 32**.

The twentieth bounded pass repaired `TitleScreen.obj`. Two target-late helper
bodies were emitted at their first users because they were still header-inline:
`TitleScreen::SetKeyNumberSprite @ 0x00469FA9` and
`GameManager::IsLastWordSpellCardAttempted @ 0x0046FD5F`. Both are now
header declarations with unchanged explicit definitions after
`OnUpdateKeyConfig` and `DrawSpellStageSelect` respectively. Focused replay
passed **23 / 23** TitleScreen units and **11 / 11** TitleReplay probe units;
cold aggregate replay passed **1,105 / 1,105**. `TitleScreen.obj` improved from
**23 anchors / 4 inversions / 3 runs / 15,726 span** to **23 / 0 / 1 / 11,979**.

The twenty-second bounded pass repaired `MusicRoom.obj`. The first global
visibility probe for `Supervisor::IsMusicPreloadEnabled @ 0x00449C79` correctly
reproduced the standalone body but changed two accepted Supervisor callers by
1-2 bytes, so it was rejected. The accepted repair makes the header
**declaration-only only while compiling MusicRoom** via a TU-local preprocessor
gate; every other TU still sees the original inline body and therefore keeps its
caller codegen. The explicit MusicRoom-tail definition is itself `inline`, so it
remains a COMDAT and coexists with the other header copies while production
object order selects the target-local MusicRoom copy. Focused replay passed
MusicRoom **11 / 11**, Supervisor **49 / 49**, and SoundPlayer **25 / 25**; cold
aggregate replay passed **1,105 / 1,105**. `MusicRoom.obj` improved from **10
anchors / 7 inversions / 2 runs / 2,621 span** to **10 / 0 / 1 / 0**.

The twenty-first bounded pass repaired `ScreenEffect.obj`. Its only reset was ordinary lexical disorder: source emitted `DrawArcadeFade @ 0x0045BC40` before `DrawPartialFade @ 0x0045BBF0`, opposite the target. Swapping those complete bodies preserved **21 / 21** focused units; cold aggregate replay passed **1,105 / 1,105**. `ScreenEffect.obj` improved from **14 anchors / 1 inversion / 2 runs / 160 span** to **14 / 0 / 1 / 48**.

The twenty-third bounded pass repaired the production ECL owner lane that had
been split across `EnemyManager.obj`, `SpellCard.obj`, and `main.obj`. Target
mapping shows `EclManager::RunEcl @ 0x004184B0` followed by the ECL operand/run
neighborhood at `0x0041F000..0x0041FDF0`; all four donor/recipient TUs use the
same `/Od /Yu"th_pch.h"` profile. The unchanged `HasParentChain`,
`SetStoredVector`, spellcard ECL flag helpers, attached/parent-chain helpers,
`IsCaptureValid`, `GetTimerFrames`, and `ZunTimer::operator+=` bodies now live
in production `EclManager.cpp`, with `Spellcard::IsBombDamageEnabled @ 0x0042DFF0`
also following the EclManager timeline getters. Canonical match-unit owners were
updated with the bodies. Focused replay passed **18 / 18** EclManager, **39 /
39** EnemyManager, **30 / 30** SpellCard, and **30 / 30** main units. Cold
aggregate replay passed **1,105 / 1,105**. `SpellCard.obj` improved from **33 /
18 / 2 / 151,472** to **27 / 0 / 1 / 61,920**, and `main.obj` from **26 / 16 /
2 / 146,336** to **25 / 0 / 1 / 16**; recipient `EclManager.obj` is **10 / 0 /
1 / 60,848**. The large EclManager span is target distance, not an order reset.

The twenty-fourth bounded pass repaired `AnmManager.obj`. Its only reset was
`AnmManager::SpriteHasTexture @ 0x004622C0`, a PCH-defined inline body emitted
after `DrawTriangleFan @ 0x00464EC0` instead of between `SetInterruptArray @
0x00462270` and `ExecuteScriptArray @ 0x00462310`. A MusicRoom-style TU-local
header gate cannot work inside a `/Yu` consumer because `AnmManager.hpp` is
already fixed inside the PCH. The accepted PCH-safe shape therefore makes the
header declaration-only, provides the target-local `inline` COMDAT body in
`AnmManager.cpp`, and provides the same early inline definition in
`TitleScreen.cpp` so its optimized caller still compiles with body visibility.
Focused replay passed **83 / 83** AnmManager and **23 / 23** TitleScreen units;
cold aggregate replay passed **1,105 / 1,105**. `AnmManager.obj` improved from
**56 anchors / 19 inversions / 2 runs / 359,840 span** to **56 / 0 / 1 /
359,792**. The large span is the real distance from its early target COMDATs at
`0x00406700/0x0040B580` to the main `0x0045E430+` region, not an order reset.

The twenty-fifth bounded pass repaired the last remaining production layout,
`AsciiManager.obj`. Four independent fixes were required. First,
`AnmVmBase::Initialize @ 0x004068E0` became an `inline` COMDAT so it could join
the target helper emission queue. Second, `Float3::Float3(f32,f32,f32) @
0x00404720` moved from its header body to an unchanged ordinary definition
between `PauseMenu::OnUpdate` and `PauseMenu::OnDraw`, matching the target-local
slot. Third, only lexical order changed inside shared headers: `ZunTimer`
`operator=`/`SetCurrent` now precede `<`/`>`, and `GameManager::IsSpellPractice`
precedes `IsReplay`, matching the observed COMDAT order. Finally,
`GetGaugeInterrupt`, `ResetStrings`, and `SetSpaceWidth @ 0x00407140..7180`
were made declaration-only and their unchanged bodies/canonical owners moved to
the already recovered `/Od /Yu` `AsciiManagerGauge.obj` target cluster. The
relevant accepted callers contain real REL32 calls. Final focused replay passed
**62 / 62** AsciiManager, **5 / 5** AsciiManagerGauge, **41 / 41** Gui, and
**62 / 62** Player units. Moving the `Float3` constructor out of the shared
header exposed one final caller-visibility dependency under a full cold build:
Supervisor, TitleScreen, and the TitleReplay probe compiled larger until the
same unchanged constructor body was restored as an early TU-local `inline`
definition in those callers. This preserves their former inline compilation
context without changing the target-local Ascii owner. The final cold aggregate
replay then passed **1,105 / 1,105**, and a fresh normal link retained zero
inversions. `AsciiManager.obj` improved from **63 anchors / 86 inversions / 6
runs / 9,600 span** to **60 / 0 / 1 / 736**; `AsciiManagerGauge.obj` is **5 / 0 /
1 / 32**.

The twenty-sixth bounded pass restored the `Spellcard::StartSpell` service
cluster at `0x00415C60..0x00415CE0`. Two exact helper bodies had been placed in
their class implementation TUs during early local matching even though target
layout puts them immediately after `StartSpell`:
`BulletManager::ClearBulletsForTransition @ 0x00415C60` and
`Enemy::ResetBulletRankInfluence @ 0x00415C80`, followed by the already-correct
`Background::background_fun_00415ce0 @ 0x00415CE0` in `Spellcard.cpp`. Moving
the unchanged bodies into `Spellcard.cpp` and rebinding their canonical exact
owners preserved focused replay at **32 / 32** SpellCard, **37 / 37**
BulletManager, and **38 / 38** EnemyManager units. A fresh production link
reduced `BulletManager.obj` from **25 anchors / 104,240 span** to **24 / 176**
and `EnemyManager.obj` from **14 / 89,792** to **13 / 36,704**. The SpellCard
cluster from `StartSpell` through the cut-in helpers now has one uniform linked
drift; its object remains **29 anchors / 0 inversions / 1 run / 128 span**.

The twenty-seventh bounded pass closed the production-owner side of the main
ECL dispatcher dependency lane. `EclManager::RunEcl @ 0x004184B0` remains
**26,638 / 26,638 authored bytes** and **27,398 / 27,398 compared bytes**
strict exact while provisional service/global identities were rebound to their
natural production owners. The same pass canonicalized the exact integer and
float operand resolvers, restored `Enemy::ClampPosition @ 0x0042C180` to
`EnemyManager.cpp`, and promoted the target ECL call-parameter/interpolator/EX
dispatch data owners into `EclGlobals.cpp`. Focused replay passed **24 / 24**
EclDependencies, **33 / 33** EclExIns, **4 / 4** EnemyTimeline, **39 / 39**
EnemyManager, **32 / 32** SpellCard, and all four resolver units. A no-`/FORCE`
diagnostic link of production objects plus the exact ECL Run/Dependencies/
Helpers/Operands/Timeline/ExIns group now resolves completely; its only
remaining unresolved object is the still-probe-only `EnemyManagerUpdate.obj`,
with **77** provisional owner symbols. Treat that object as the next bounded
promotion blocker rather than reopening already-closed RunEcl/resolver owners.

The twenty-eighth bounded pass closed that final authored ECL owner blocker.
`EnemyManagerUpdate::OnUpdate @ 0x0042C660` remains **6,198 / 6,198 authored bytes**
and **6,214 / 6,214 compared bytes** strict exact while its 77 provisional
external identities were routed back to natural production owners. The pass
restored the real `g_EclManager @ 0x004ECCB8` data owner; its `0x168`-byte
extent ends exactly at `g_EclCallParameters @ 0x004ECE20`. It also rebound
GameManager/Player/subsystem singletons, ZunTimer calls, Enemy methods, the ECL
timeline lane, canonical `fabsf`, and the target-exact `0x0041FD20` /
`0x0042ADB0` helpers. The last data alias was proven to be the member now named
`g_EnemyManager.spawnTemplate.bulletSpawnDescriptor`: `BulletSpawnDescriptor` is
exactly `0x210` bytes and its address is `0x00577F20 + 0x2E24 = 0x0057AD44`, matching
the target `memcpy` source. A no-`/FORCE` diagnostic link of production objects
plus EclRun, EclDependencies, EclHelpers, EclOperandsInt/Float, EnemyTimeline,
EclExIns, and EnemyManagerUpdate now exits successfully with **zero unresolved
externals**. Cold authored replay remains **1,105 / 1,105 exact**. The next
bounded step is production promotion and fresh whole-image measurement, not
further owner canonicalization.

The twenty-ninth bounded pass moved the closed ECL lane into the real production
link without changing its compiler contract: EclRun, EclDependencies, EclHelpers,
EclOperandsInt/Float, EnemyTimeline, EclExIns, and EnemyManagerUpdate now build as
`build/*.obj` with the same generic `/Oi /Gr` profile that produced their accepted
probe objects. All **69 / 69** promoted match units replay strict exact from the
production paths. Fresh whole-image routing then exposed three earlier probe lanes as
missing bodies inside existing target TUs rather than standalone objects.
`PauseMenu::OnDraw` / `RetryMenu::OnDraw` were restored to their real `void` ABI and
inserted into `AsciiManager.cpp`, which now replays **65 / 65 exact** and reduces its
former `0 / -304 / -752` drift staircase to only `0 / -80`-class alignment/COMDAT
noise. `GameManager::GameplaySetupThread @ 0x0043ABD7` and `FUN_0043BBE1` returned to
`GameManager.cpp`; the target `g_TimeRequirementParams @ 0x004C77F0` storage is a
semantic `9 x 4` i32 table, the setup member has the observed `__fastcall void(void *)`
ABI, and `GameManager.obj` now replays **42 / 42 exact** with the former 3,418-byte
drift jump removed. Fifteen Player option/shot functions were restored into the two
target gaps in `Player.cpp`; after preserving their VC7 `#pragma var_order` contracts,
`Player.obj` replays **77 / 77 exact** and the former `-63,824 -> -67,152 -> -70,960`
staircase collapses to one approximately constant drift. The resulting fresh linked
PBG lane is only **4,896..4,848 bytes early**, down from 16,672 bytes before these
owner restorations.

The promoted ECL code is target-backed but its object placement is not yet target-like.
The current normal image has `.text` raw size `0xBA800` versus target `0xB1C00`,
`.rdata` `0x15200` versus `0x11400`, `.data` `0x6400` versus `0x6E00`, and file-size
delta `+49,152` bytes. Do not interpret the `.text` overshoot as evidence that the
exact ECL bodies are spurious: their target anchors are valid, but the eight promoted
objects are currently appended after the main authored lane. In particular
`EclDependencies.obj` now reports **160 target-order inversions / 7 runs** and must be
partitioned/routed by target neighborhoods; `EclExIns.obj` has a small two-run kink.
The next bounded whole-image task is ECL object order/partition, followed by the small
remaining authored gap (PBG about 4.9 KB early), not further ECL owner canonicalization.

The production link contains **44 authored/PBG object files**. The fold-aware
whole-image report now includes the promoted ECL objects as production layout
evidence. Existing settled objects such as AsciiManager, GameManager, Player, and the
PBG lane retain one target-order run, while the newly promoted mixed ECL dependency TU
is intentionally the active partition candidate. `utils.obj` remains folded-only and
`EclGlobals.obj` remains data-only; neither should be used as TU-placement evidence. The final cold authored replay remains **1,105 /
1,105 exact**, `config/claims.csv` remains header-only, and the final
tracking/progress/CI/doc/whitespace gates pass. Do not infer whole-image byte
identity from this milestone: section drift, imports/resources, and non-layout
reconstruction work remain separate evidence classes. For future layout
changes, preserve the same acceptance rule: target-neighborhood evidence,
focused donor/recipient/caller replay, a cold normal linked-order measurement,
and a cold aggregate exact replay before committing.
Large intra-object drift means today's source combines or orders target TUs
differently, so permuting the existing object list alone cannot solve it. A
run reset is a routing clue, not automatically a TU boundary: the Player pass
showed that one investigation may require both a real split and target-order
restoration inside the retained TU. The 17
extra imports already have bounded contributing archive members; do not hide
them with `/OPT:REF`. A trial of that flag made the import set exact while
shrinking `.text` to `0x72B6F` and losing 212 located accepted anchors, so
`/OPT:NOREF` remains required by stronger whole-image evidence. ICF is a
separate target-proven contract: constructor-iterator sites across multiple
subsystems load the same `0x0040B580` body for four differently decorated
vertex constructors, and the linked image also contains the reviewed folded
sound-manager constructor at `0x004716E0`. Keep `/OPT:ICF` enabled. Return to
CRT/D3DX exact recovery only if a subsequent diff identifies a specific member
operation. Do not copy the target icon payload and do not claim whole-image
exactness from successful linking or same-size sections.

Use `docs/TOOLS.md` to choose commands. Keep `.analysis/` disposable and update
this file whenever the active milestone or a durable blocker changes.

The latest semantic checkpoint types the Replay serialization boundary without
inventing meanings for unconsumed storage.  `StageReplayData::inputStream` owns
the variable input tail, while `ReplayDataHeader::stageFpsData` is correctly a
nine-entry table of byte-stream pointers.  Zero-filled serialized gaps are
reserved storage; the payload constant at `+0x120` and manager reset word at
`+0x4E` are named only for their proven write protocols.  Four manager
`Float3`s remain explicit unknown vectors because an opaque-storage trial
removed their target-visible implicit construction (constructor 58 -> 14
bytes); the ranges at `+0x48/+0x7C/+0xCC` remain opaque for lack of consumers.
Focused Replay replay passes **18 / 18 exact**, the required non-reuse cold
aggregate passes **1,106 / 1,106 exact**, the normal VC7 image links, and the
complete i386 Linux build plus fixed-layout verifier pass.  This is a bounded
semantic checkpoint, not a whole-program semantic-completion claim.

PR #2 merged the first semantic reconstruction phase into `main` as merge
commit `4fb955225ff9621d2eaa5ff2113a709ff5576407`; the continuing
`semantic/typed-reconstruction` branch was then fast-forwarded from that exact
mainline state.  The next checkpoint classifies residual ANM/Ascii/Sound/MIDI
state without inventing consumers.  ANM `+0x0C` is proven to count scripts
started during the frame, Ascii script 9 belongs to an auxiliary gauge VM, and
several Sound/MIDI fields are now explicitly marked unconsumed or reset-only
where the authored corpus supplies no stronger behavior.  Serialized ANM
unknowns retain neutral offset-bearing names and layout assertions.

Focused replay across the six affected production objects passes **259 / 259
exact**.  The required non-reuse cold aggregate passes **1,106 / 1,106 exact**,
the normal VC7 image links, and the complete i386 Linux container build plus
fixed-layout verifier pass.  Continue treating the remaining heuristic
anonymous/opaque list as evidence-routing work: it is neither an exactness
regression nor proof that every residual byte has a recoverable semantic name.

The following score/result checkpoint classifies the remaining ResultScreen,
TitleScreen replay-enumeration, `Hscr`, and `Catk` tail state.  Reset-only and
unconsumed runtime fields are named only for their observed protocols, while
serialized alignment/tail bytes remain explicit reserved storage.  The byte at
`Hscr + 0x166` is now `defaultScoreMarker`: only the synthetic leaderboard
producer writes it, and no unobserved consumer is claimed.  All renamed owner
offsets are asserted.

Focused replay across ResultScreen, ScoreDat, TitleScreen, and GameManager
passes **113 / 113 exact**; cold aggregate replay passes **1,106 / 1,106
exact**; the normal VC7 image and complete i386 Linux image both link; and the
Linux fixed-layout verifier passes.  `TitleScreen::RegisterChain @ 0x0047146D`
is currently **281 / 281 exact** under the repository Oracle.  The sole
authored/non-accepted row is instead
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0` (361 bytes), which
has no configured match unit.  Treat adding and proving that unit as a
separate Oracle-ledger closure task, not as evidence of a current mismatch.

The current Bullet residual checkpoint replaces generic transform payload
reads with kind-specific union views while preserving opcode 111's generic raw
write boundary.  Descriptor `+0x1FA`, template `+0xD40`, live-bullet `+0xDBC`,
and the remaining unaccessed ranges are classified only as unconsumed or
alignment storage; sprite height `+0xD41` is named from its exact loaded-sprite
producer.  `BulletManager.hpp/.cpp` now has zero semantic-router candidates.
Focused replay passes **94 / 94** across BulletManager/EclDependencies/EclExIns
plus **1 / 1** for EclRun; the cold aggregate passes **1,106 / 1,106 exact**;
the normal VC7 image and complete Linux i386 image link; and the Linux fixed-
layout verifier passes.  Continue with Enemy/ANM residual storage without
weakening the sole Replay near-match blocker.

The Enemy residual checkpoint classifies the remaining flag bits, neutral
words/dwords, and structural alignment ranges in the shared `Enemy` and
`EnemyManager` layouts without inventing gameplay meanings.  Explicit
alignment arrays are retained where removing them would alter the
target-visible Enemy aggregate-copy source shape, and every classified range
is offset-asserted.  Focused replay across EnemyManager, EnemyManagerUpdate,
EclRun, EclDependencies, EclExIns, Gui, and SpellCard passes **171 / 171
exact**; the required cold aggregate passes **1,106 / 1,106 exact**; the normal
VC7 image and complete Linux i386 image link; and the Linux fixed-layout
verifier passes.  `EnemyManager.hpp` now has no semantic-router candidates.
Continue with the ANM residual family while preserving the sole Replay
near-match as a separately documented Oracle limitation.

The ANM residual checkpoint closes the remaining `AnmManager.hpp` router
candidates.  Serialized record fields with no consumer are named reserved;
runtime tails that are only zeroed or aggregate-copied remain explicitly
unconsumed; and manager `+0x24C6..+0x24C7` is now compiler-owned alignment
before the asserted `currentSprite @ +0x24C8`.  Focused AnmManager replay
passes **82 / 82 exact** before and after the edit; the required cold aggregate
passes **1,106 / 1,106 exact**; the normal VC7 image and complete Linux i386
image link; and the Linux fixed-layout verifier passes.  Continue on
`semantic/typed-reconstruction` with Player/ECL/Game residual state.  Do not
open or merge another PR until explicitly requested; preserve the sole Replay
near-match as a separate Oracle limitation.

The Player residual checkpoint classifies the last neutral SHT, collision,
option, Bomb, shot, and Player bytes without inventing consumers.  Four proven
alignment ranges are now compiler-owned padding, while serialized and runtime
dwords remain reserved/unconsumed and offset-asserted.  Focused Player plus
PlayerBomb replay passes **136 / 136 exact** before and after the edit; the
required cold aggregate passes **1,106 / 1,106 exact**; the normal VC7 image
and complete Linux i386 image link; and the Linux fixed-layout verifier
passes.  `Player.hpp` now has no semantic-router candidates.  Continue on the
branch with ECL/Game residual state, without opening or merging a PR until the
user explicitly requests it.

The ECL/Effect residual checkpoint classifies only storage with no independent
consumer and serialized instruction bytes skipped by all exact dispatch
views.  Context tail alignment stays explicit because whole-context assignment
is target-visible.  Focused replay across EclManager, EclRun,
EclDependencies, EclExIns, and EffectManager passes **128 / 128 exact** before
and after the edit, including full RunEcl; the required cold aggregate passes
**1,106 / 1,106 exact**; the normal VC7 image and complete Linux i386 image
link; and the Linux fixed-layout verifier passes.  Continue with Game/GUI/
Ending residual state on the branch.  Do not open or merge a PR until the user
explicitly asks.

The GameManager residual checkpoint proves three formerly anonymous ranges as
compiler alignment and retains only two independently unconsumed dwords.  All
surrounding replay, RNG, stage, playtime, frame-skip, and active-frame fields
are offset-asserted.  Focused GameManager replay passes **42 / 42 exact**
before and after the edit; the required cold aggregate passes **1,106 / 1,106
exact**; the normal VC7 image and complete Linux i386 image link; and the Linux
fixed-layout verifier passes.  Continue with the bounded GUI/Ending residuals
and audit the Replay constructor-sensitive opaque ranges separately.  Keep all
work on the branch until the user explicitly requests another PR/merge.

The GUI/Ending residual checkpoint removes the fictitious final message byte
in favor of compiler-owned tail alignment and classifies the two remaining
aggregate-cleared dwords as explicitly unconsumed.  Target-pinned packets for
`GuiMsgVm::GuiMsgVm @ 0x00437CE2`, `Gui::RegisterChain @ 0x00437AD0`,
`Ending::Ending @ 0x004297B0`, and `Ending::OnUpdate @ 0x00429860` are exact.
Focused GUI plus Ending replay passes **52 / 52 exact** before and after the
edit; the required cold aggregate passes **1,106 / 1,106 exact**; the normal
VC7 image and complete Linux i386 image link; and the Linux fixed-layout
verifier passes.  Continue with the Replay constructor-sensitive residual
storage and keep all work on the branch until the user explicitly requests a
PR/merge.

The Replay residual checkpoint preserves the four target-observed `Float3`
constructor calls while classifying those objects and the three manager byte
ranges as explicitly unconsumed; serialized stage byte `+0x23` is reserved
wire storage rather than false compiler padding.  The now-unused
`unknown_fields` macro is removed.  Target-pinned Replay constructor and
RegisterChain packets are exact, focused Replay replay passes **18 / 18
exact** before and after the edit, the required cold aggregate passes **1,106
/ 1,106 exact**, the normal VC7 image and complete Linux i386 image link, and
the Linux fixed-layout verifier passes.  The four-category source router is
now zero, but this is a layout-routing milestone rather than a whole-program
semantic-completion claim.  Continue by auditing router-invisible neutral
function names and evidence-limited state.  Keep all work on the branch until
the user explicitly requests a PR/merge.

The Effect camera-particle naming checkpoint replaces the last six authored
`FUN_00426...` identities with three evidence-backed initializer/update pairs:
tinted Boss-tracking, rising Boss-tracking, and spinning camera particles.
Template slots 51, 63, and 19 respectively establish the pairings; names stop
short of guessing the artwork.  Source, portable template mirror, tracking
ledgers, accepted rows, and VC7 decorated-symbol units move together.  Focused
`EffectManager.obj` replay passes **52 / 52 exact**, the required cold
aggregate passes **1,106 / 1,106 exact**, the normal VC7 image and complete
Linux i386 image link, and the Linux fixed-layout verifier passes.  Continue
the router-invisible neutral-field audit on the branch; do not open or merge a
PR until explicitly requested.

The neutral-identifier checkpoint replaces the remaining production-source
`unknown*`/`unk*` names with evidence-bounded `unconsumed`,
`constructorCleared`, or `serializedReserved` identities across AsciiManager,
Background, Midi, SoundPlayer, ScoreDat, ResultScreen, and zwave.  New offset
assertions pin the affected public and wire layouts.  Focused cold replay of
the seven affected objects passes **238 / 238 exact** before and after the
edit; the required cold aggregate passes **1,106 / 1,106 exact**; the normal
VC7 image and complete Linux i386 image link; and the Linux fixed-layout
verifier passes.  Production source now has no `unknown*`, `unk*`, or authored
address-style `FUN_*` identifiers, while the four-category semantic router
remains zero.  Continue auditing router-invisible weak names and intentionally
neutral state on `semantic/typed-reconstruction`; do not open or merge a PR
until explicitly requested.

The Item spawn-state checkpoint replaces the remaining unknown item enums with
behavior-backed death-drop spread, ordinary time-orb rise, and apex-gated
time-orb request/state identities; the two unproduced values remain explicitly
reserved.  All authored constant SpawnItem calls now use typed item/state
names, while genuinely data-driven ECL/drop/cancel values retain casts.
Target-pinned packets cover `SpawnItem @ 0x004400A0` and
`OnUpdate @ 0x00440500`.  Focused cold replay of the eight affected objects
passes **244 / 244 exact** before and after the edit; the required cold
aggregate passes **1,106 / 1,106 exact**; the normal VC7 image and complete
Linux i386 image link; and the Linux fixed-layout verifier passes.  Continue
the router-invisible semantic audit on `semantic/typed-reconstruction`; do not
open or merge a PR until explicitly requested.

The Player collision-region API checkpoint replaces all four allocator
signatures' `value1..value4` parameters with width/height or initial-radius/
growth plus damage/collision-value/lifetime roles.  It preserves the
target-observed circle-cancel argument-order asymmetry.  Target-pinned packets
cover `0x0044DE60`, `0x0044DF00`, `0x0044DFA0`, and `0x0044E040`; focused
Player replay passes **77 / 77 exact** before and after the edit, with each
allocator at **153 / 153**; the required cold aggregate passes **1,106 / 1,106
exact**; the normal VC7 image and complete Linux i386 image link; and the Linux
fixed-layout verifier passes.  Continue the weak-parameter audit on
`semantic/typed-reconstruction`; do not open or merge a PR until explicitly
requested.

The shared-helper parameter checkpoint removes residual `paramN`/decompiler
locals from seven exact helpers in AsciiManager, TextHelper, Supervisor,
ScreenEffect, and TitleScreen.  Names are bounded to directly observed roles:
two popup digit sequences, music preload slot/fade seconds, ScreenEffect
duration/variant parameters/draw priority, TextHelper's A1R5G5B5 gentle
falloff selector, and pie fraction/diameter.  Target-pinned packets cover all
seven addresses; focused replay passes **178 / 178 exact** before and after
the edit; the required cold aggregate passes **1,106 / 1,106 exact**; the
normal VC7 image and complete Linux i386 image link; and the Linux fixed-
layout verifier passes.  Continue the remaining generic-local/API audit on
`semantic/typed-reconstruction`; do not open or merge a PR until explicitly
requested.

The background/MIDI/spell-portrait checkpoint closes three high-signal weak-
identifier families.  Background cubic interpolation now exposes its
start/end values and tangents, and RegisterChain's stage index is named from
both production callers and the `registeredStage` write.  MidiOutput::ProcessMsg
now reads as a running-status MIDI event decoder, including its TH08 loop
checkpoint/jump controllers and fade-adjusted channel volume.  Spellcard's
`+0x90C` and `+0xE54` VMs are named only as enemy-portrait auxiliaries with
their observed draw modes; no absent asset role is guessed.  Target packets
cover all nine affected exact functions; focused post-edit replay passes
**94 / 94 exact**; the required cold aggregate passes **1,106 / 1,106 exact**;
the normal VC7 image and complete Linux i386 image link; and the Linux fixed-
layout verifier passes.  Continue on `semantic/typed-reconstruction`; commit
and push stable checkpoints, but do not open or merge a PR until explicitly
requested.

The runtime-queue checkpoint replaces the last intentionally opaque local
names in Chain::ReleaseSingleChain and Controller::GetControllerInput with a
release-snapshot model and explicit WinMM/DirectInput state.  Readable names
initially exposed VC7's identifier-sensitive stack allocation; target stack
evidence and `#pragma var_order` now preserve the exact 0x54/0x160 frames
without returning to `a0/aa/aaa`.  GetControllerState retains and documents
the original discarded-GetDeviceState-HRESULT quirk.  ResultScreen's VM and
keyboard animation locals and SoundPlayer's command/SFX queue locals are also
named by observed roles.  Target packets cover all six functions; focused
replay passes **106 / 106 exact**; the cold aggregate passes **1,106 / 1,106
exact**; VC7 and Linux i386 images link; and the Linux fixed-layout verifier
passes.  Continue on `semantic/typed-reconstruction`; do not open or merge a
PR until explicitly requested.

The runtime-owner checkpoint names Laser's midpoint-rendered `bodyVm` and
start-offset `startCapVm`; enemy-spawn ECL subroutine/life/drop/score arguments;
and DirectSound's current play/write cursors.  The ECL global at `0x004E4B60`
is proven to be the `g_Background + 0xB30` spell-background view and now exposes
`spellVmCount`, `spellVmScriptBase`, and `spellVms[2]`.  The synthetic
`g_TargetPlayerPosition017D61AC` symbol is proven to be `g_Player.position @
+0x2B4`; all production users and COFF relocation manifests now name the real
owner, and the obsolete Linux linker alias is gone.  ECL helpers at
`0x0041EFC0`, `0x0041F110`, and `0x0041F280` are now
`FindAttachmentChainTail`, `SpawnChildAtScriptPosition`, and
`SpawnChildAtParentOffset`, with source and every tracking ledger migrated
together.  Focused replays pass **190 / 190**, **182 / 182**, and **25 / 25
exact** for the three bounded groups; the required cold aggregate passes
**1,106 / 1,106 exact**; the normal VC7 image and complete Linux i386 image
link; and the Linux fixed-layout verifier passes.  Continue on
`semantic/typed-reconstruction`; commit and push stable checkpoints, but do
not open or merge a PR until explicitly requested.

The ECL spell-background owner checkpoint removes three remaining synthetic
global identities.  Target address `0x004ECCA8` is
`g_Spellcard.scoreLimit @ +0x2638`; ECL opcode 155 now writes the same readable
`99999990` timeout limit as `Spellcard::StartSpell`.  Addresses `0x004E4B60`
and `0x004EA28C` are respectively `g_Background.spellVmCount @ +0xB30` and
`g_Background.spellBackgroundDrawCallback @ +0x625C`; all ECL extension and
Effect users now name the real Background fields directly, and the callback
slot carries the only assigned target function's `__fastcall` type.  The stale
DIFFBUILD `_g_Stage` label at the Background base is also corrected.  The
obsolete
target/modern bridge declarations, Linux linker alias, global-ledger rows, and
three synthetic relocation identities are gone; COFF addends continue to
prove the same absolute target addresses through the real owners.  Focused
replay passes **86 / 86 exact**, and the required non-reuse cold aggregate
passes **1,106 / 1,106 exact** with zero failures.  The normal VC7 image and
complete Linux i386 image link, and the Linux fixed-layout verifier passes.
Continue on `semantic/typed-reconstruction`; commit and push stable
checkpoints, but do not open or merge a PR until explicitly requested.

The ECL runtime-type checkpoint promotes the exact low/high interpreter helper
families from the temporary `EclRunLowProposal` / `EclRunHighProposal`
namespaces to `EclRunLow` / `EclRunHigh`, migrating all 17 helper identities,
the call-parameter global, and every source/configuration/COFF/linker reference
together.  The dead private Player overlay is gone.  `ApplyRandomBiasedMove @
0x004224A0` and `DispatchShotInstruction @ 0x00422720` now take the real
`Enemy *` owner and shared `EclRawInstruction`; RunEcl and the high dispatcher no
longer carry duplicate raw-instruction or vector shells.  The public
`EnemyEclInterpolationSlot` is now shared by the ECL context, RunEcl,
`InterpolateLinear @ 0x00421120`, `InterpolateHermite @ 0x00421180`, and slot
installation, with the callback table at `0x004C6C90` carrying its observed
`__fastcall` ABI.  Focused replay passes **68 / 68 exact** and the required
non-reuse cold aggregate passes **1,106 / 1,106 exact** with zero failures.
The normal VC7 image and complete Linux i386 image link, and the Linux fixed-
layout verifier passes.  Continue the semantic audit on
`semantic/typed-reconstruction`; commit and push stable checkpoints, but do
not open or merge a PR until explicitly requested.

The Enemy/ECL owner checkpoint removes the dormant standalone low/high
dispatcher harness and its unused `Services`/`TargetApi` abstractions; the two
`.inl` files now contain only the production lexical opcode bodies compiled by
`EclManager::RunEcl`.  The temporary `EclOperands::EnemyOverlay` is gone:
`Enemy::ResolveFloat @ 0x00420120`, `Enemy::DetachEnemyChain @ 0x0042ADB0`,
both operand resolvers, all ECL helpers, and the EX-instruction family now use
the real `Enemy *` owner.  Fifty-three function identities and 465 configured
COFF relocation references move to the corresponding VC7 `Enemy *` symbols
without changing their target addresses.

`EclExInstruction` is now a shared serialized record, and `g_EclExInsn @
0x004C6CB0` is a typed `__fastcall` callback table.  Opcode 136 calls that ABI
directly; opcode 137 keeps an explicit compatibility cast because the target
reuses the same table with a different second-argument interpretation.
`StartEnemySpell @ 0x00421280`, `EndEnemySpell @ 0x004212E0`, and
`Spellcard::StartSpell @ 0x004152A0` carry typed instruction/Enemy arguments,
while `Spellcard::activeEnemy @ +0x004` names the same owner directly.  Focused
replay across the 11 affected objects passes **213 / 213 exact**.  The required
non-reuse cold aggregate passes **1,106 / 1,106 exact** with zero failures;
the normal VC7 image and complete Linux i386 image link, and the Linux fixed-
layout verifier passes.  Continue on `semantic/typed-reconstruction`; commit
and push stable checkpoints, but do not open a PR or merge until explicitly
requested.

The Enemy spawn/lifecycle checkpoint makes `EnemyManager::SpawnEnemy1 @
0x0042A4E0` and `SpawnEnemy2 @ 0x0042A680` return `Enemy *`, matching the
storage they scan and the timeline/ECL callers that immediately consume the
result as an Enemy.  Their COFF-defined VC7 symbols are
`QAEPAUEnemy@2@...`, so all six configured references to each old `QAEPAX...`
identity move with the declaration.  `EnemyManager::Initialize @ 0x00429E00`
and `KillAllNonBossEnemies @ 0x0042EFB0` now traverse `Enemy *` records rather
than maintaining a byte cursor and repeating casts; chain cleanup and ECL
context calls likewise use their real fields directly.  Focused replay across
the five affected objects passes **101 / 101 exact**.  The required cold
aggregate passes **1,106 / 1,106 exact** with zero failures; the normal VC7
image and complete Linux i386 image link, and the fixed-layout verifier passes.
Continue on `semantic/typed-reconstruction`; commit and push stable
checkpoints, but do not open a PR or merge until explicitly requested.

The runtime-pointer checkpoint types two residual owner fields with complete
production-use evidence.  `Spellcard::lifetimeObject @ +0x263C` is a
`ChainElem *` produced by `Chain::CreateElem`, submitted to the calc chain, and
cleaned through ChainElem lifetime state; RegisterChain and DeletedCallback no
longer reinterpret it.  `AnmManager::currentSprite @ +0x24C8` is an
`AnmLoadedSprite *`, matching the sole value compared and assigned in
`Draw3D`.  Focused replay passes **175 / 175**, **157 / 157**, and **1,318 /
1,318 exact** for `Spellcard::RegisterChain @ 0x00417F60`,
`Spellcard::DeletedCallback @ 0x00418050`, and `AnmManager::Draw3D @
0x00464470`.  The required cold aggregate passes **1,106 / 1,106 exact** with
zero failures; the normal VC7 image and complete Linux i386 image link, and
the fixed-layout verifier passes.  Continue on `semantic/typed-reconstruction`;
commit and push stable checkpoints, but do not open a PR or merge until
explicitly requested.

The Effect lifecycle/ownership checkpoint replaces the remaining false
`AnmVm *` view of full Effect objects.  Target `OnUpdate @ 0x00427BF0` and
`OnDraw @ 0x00427F00` pass the Effect address in ECX to their indirect
callbacks, so the instance and 66-row template tables now carry typed Effect
update/initialize/draw callbacks.  The splash/orbit and player radial/barrier
families consume `Effect *` directly; the one
`AnmVm::UpdatePulsingRadialTrail` table entry remains an explicit member-ABI
compatibility case.

All five spawn APIs at `0x00425430..0x00425B70` now return the Effect they
initialize, and `GetFixedSlotEffect @ 0x004253E0` names the same fixed-pool
owner.  Player deathbomb/work-item fields and ECL/Background/Spellcard callers
retain `Effect *`, taking `&effect->vm` only at genuine ANM boundaries.  The
post-change `QAEPAUEffect@2@...` spawn/getter and `PAUEffect@1@@Z` callback
decorated identities were read from new VC7 objects before migrating every
configured relocation.  Focused replay passes **26 / 26 exact**; the required
non-reuse cold aggregate passes **1,106 / 1,106 exact** with zero failures; the
normal VC7 image and complete Linux i386 image link; and the Linux fixed-layout
verifier passes.  Continue on `semantic/typed-reconstruction`; commit and push
stable checkpoints, but do not open a PR or merge until explicitly requested.

The core-callback/update-owner checkpoint gives the projected-position ANM
callback its target-observed `void __fastcall(AnmVm *, D3DXVECTOR3 *)` ABI and
removes its `void *` call boundary.  Target references also prove that
`g_PlayerShotCollisionCallbacks @ 0x004C7F24` is a three-entry typed callback
array and that the replay difficulty strings beginning at `0x004C7F30` are a
separate `ReplayManager.cpp` owner, retiring the false nine-entry mixed table.

`EnemyManager::OnUpdate @ 0x0042C660` is now the real non-static member proven
by target ECX use and the rebuilt `?OnUpdate@EnemyManager@th08@@QAEHXZ` COFF
definition.  The one-byte overlay and whole-object casts are gone; a distinct
portable Chain callback adapter preserves `RegisterChain` behavior.  All nine
focused units pass exact, including OnUpdate **6,214 / 6,214 compared bytes**
and RegisterChain **203 / 203**.  The required cold aggregate passes **1,106 /
1,106 exact** with zero failures; the normal VC7 image and complete Linux i386
image link; and the Linux fixed-layout verifier passes.  Continue on
`semantic/typed-reconstruction`; commit and push stable checkpoints, but do
not open a PR or merge until explicitly requested.

The residual-owner/Title-parity checkpoint removes Background's last authored
raw field access, gives the ECL context's opcode-137 per-frame callback and
instruction real types, preserves EnemyManager's ECL header/subtable through
their fields, types ReplayManager::LoadReplayData as `ReplayData *`, and moves
the three Title spell-history queries onto `Catk`.  The mapping ledger now also
reflects the already exact `ZunResult`/`Enemy *` signatures for RenderObjects,
RunEcl, and AddedCallback.

A complete production/probe duplicate audit found three real portable Title
drifts: completion status used `&& cursor > 3` instead of the target's
independent `|| cursor > 3`, and spell-card selection reversed two cursor
upper-bound comparisons.  Production now follows the exact probe, and
completion rendering is single-owned by a shared `.inl`.  Player option/shot,
score initialization, Stage-menu draw, and Gameplay setup duplicates have no
behavioral drift.  Focused replay passes **11 / 11 exact**; the required cold
aggregate passes **1,106 / 1,106 exact** with zero failures; the normal VC7
image and complete Linux i386 image link; and the Linux fixed-layout verifier
passes.  Continue the router-invisible API/config signature and intentionally
neutral-state audit on `semantic/typed-reconstruction`.  Commit and push stable
checkpoints, but do not open a PR or merge until explicitly requested.

The child-ECL-runtime checkpoint replaces the raw `0x24B0` opcode-135 block
and its `+0x8/+0x230/+0x6` arithmetic with `EnemyChildEclBlock`: one active
`EnemyEclContext`, a 16-context call stack, and signed call-stack depth, all
pinned by layout assertions.  The word at `+0x4` remains explicitly
unconsumed, and serialized ECL operands remain byte-oriented.  The stale
AddSpriteToDrawBuffer mapping now records its exact
`ZunResult (VertexTex1DiffuseXyzrhw *)` API.  The exact Title DrawPieChart probe
also corrected production's center vertex `w` initialization.  Focused replay
passes **26,638 / 26,638**, **211 / 211**, and **510 / 510 exact**; the required
non-reuse cold aggregate passes **1,106 / 1,106 exact** with zero failures;
the normal VC7 image and complete Linux i386 image link; and the Linux
fixed-layout verifier passes.  Continue on `semantic/typed-reconstruction`;
commit and push stable checkpoints, but do not open a PR or merge until
explicitly requested.

The authored-API-ledger checkpoint aligns 119 stale `config/mapping.csv`
signatures with the existing typed exact source and rebuilt VC7 COFF symbols.
It corrects receiver ownership, member/static conventions, return widths,
const pointer types, missing arguments, and varargs boundaries across the
major gameplay, UI, rendering, audio, and data APIs.  Focused comparison
passes **119 / 119 exact**; the required non-reuse cold aggregate passes
**1,106 / 1,106 exact** with zero failures; the normal VC7 image and complete
Linux i386 image link; and the fixed-layout verifier passes.  None of the 119
gameplay/API rows in scope retains an `unknown` convention.  Sixteen accepted
exact rows remain in the bundled CSound/CWaveFile, PBG, and LZSS helper-source
families; compiler-runtime, D3DX, initializer, and unselected rows remain
separate audit lanes.  The single authored allocator residual is unchanged.
Continue with a bounded semantic-completion audit on
`semantic/typed-reconstruction`; commit and push stable checkpoints, but do
not open a PR or merge until explicitly requested.

The bundled-helper closure aligns 54 accepted CSound/CWaveFile, PBG, and LZSS
mapping signatures with their exact source and VC7 decorated symbols.  The
accepted implementation/mapping intersection now contains zero `unknown`
calling conventions.  The two remaining production `m_unk*` fields in
`CSound` are renamed to honest `unconsumedDword28/2C`; target-pinned evidence
only proves that Play clears `+0x2C`, so no stronger meaning is invented.
Focused replay passes **54 / 54 exact**, including Play **236 / 236**; the
required non-reuse cold aggregate passes **1,106 / 1,106 exact** with zero
failures; the normal VC7 image and complete Linux i386 image link; and the
fixed-layout verifier passes.  Continue the repository-wide semantic
completion audit on `semantic/typed-reconstruction`; commit and push stable
checkpoints, but do not open a PR or merge until explicitly requested.
