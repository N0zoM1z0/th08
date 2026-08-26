# Semantic reconstruction

This phase turns layout-shaped reconstruction source into evidence-backed C++
without giving up either accepted VC7 code generation or the playable modern
ports.  Typical work replaces byte-pointer arithmetic, absolute field views,
`unk...` members, and opaque storage with named fields, aggregates, enums, and
canonical object owners.

It is not a second authored-progress ledger.  Candidate counts are routing
signals, a readable name is not proof, and preserving compilation alone does
not establish the meaning of a field.

## Evidence boundary

Keep these classes distinct in the batch note and commit message:

- **Observed:** TH08 1.00d access offset and width, signedness, bit operations,
  callers/callees, target strings, relocations, exact object code, and verified
  runtime object identity or behavior.
- **Corroborated:** several independent TH08 functions use the field for the
  same role, or TH06/TH07/upstream source names the same layout and TH08 target
  evidence agrees.
- **Inferred:** the role follows from dataflow or adjacent-version structure but
  is not independently named by TH08 evidence.  Use a neutral role name and
  state the confidence.
- **Unknown:** only storage, width, or alignment is known.  Keep it opaque; do
  not convert uncertainty into a confident-looking identifier.

IDA remains unavailable until the active database passes `docs/IDA_MCP.md`.
Semantic cleanup usually has enough source, strict VC7 replay, `typed-re.py`,
and portable runtime evidence, but lack of IDA does not lower the evidence bar.

## Select one bounded batch

Start with the read-only candidate report:

```bash
python3 scripts/analysis/report-semantic-debt.py
python3 scripts/analysis/report-semantic-debt.py \
  --category raw-member-access --details
```

The report intentionally excludes `*Probe*` files and `src/modern/` by default.
Use `--include-probes` only when a canonical exact unit still lives in probe
source.  Use `--include-modern` to audit a portability boundary, not to infer
original TH08 field semantics from backend implementation details.

Choose one structure plus one coherent field family.  Good early batches have
several independent reads/writes, a small caller set, existing exact units in
one object, and no need to change behavior.  Avoid combining unrelated renames
across managers merely because the edits are mechanical.

Before editing:

1. record the target functions and addresses that use the fields;
2. confirm those rows in `config/mapping.csv`, `config/reccmp-functions.csv`,
   and the affected units in `config/match-units.toml`;
3. inspect the current declarations, every source use, access widths, and the
   smallest relevant target fact packet or disassembly;
4. capture the affected accepted-unit baseline;
5. identify whether the declaration is shared through the PCH or by optimized
   callers, which expands the VC7 replay scope.

## Source rules

- Preserve the VC7 x86 ABI: exact field offsets, widths, signedness, packing,
  bitfield behavior, class size, construction order, and translation-unit
  visibility.
- Add `C_ASSERT(sizeof(Type) == ...)` and focused
  `C_ASSERT(offsetof(Type, member) == ...)` checks for every layout fact the
  batch relies on.  An assertion attests layout only, not the member's name.
- Prefer a real aggregate owner and a member/index expression.  Do not create a
  second global for an address inside an existing object.  The Linux port has
  demonstrated that equal symbol values without shared object identity can
  duplicate callback state and corrupt gameplay.
- Replace raw expressions with the narrowest natural typed form.  Do not mix a
  field recovery with loop cleanup, branch inversion, local reordering, or API
  redesign unless target evidence requires those changes together.
- Keep serialization buffers, instruction byte streams, tagged unions, and
  platform ABI glue byte-oriented when byte addressing is their actual
  semantics.  The candidate reporter is expected to include false positives.
- Do not rename a field solely from its numeric offset, a decompiler label, one
  visual observation, or an adjacent game's name.  Preserve a neutral name or
  opaque range until independent TH08 evidence closes the role.
- Do not add aliases, union overlays, accessors, or casts merely to make the
  source look typed.  They must express a real ownership or representation
  relationship and preserve both oracles.

## Two-oracle acceptance

The VC7 and portable builds answer different questions:

| Change | VC7 oracle | Portable oracle |
| --- | --- | --- |
| Private field rename or typed expression in one object | Build the smallest object and replay every accepted unit in that object | Compile and link the configured modern target |
| Shared header, class layout, inline body, PCH, or object-owner change | Replay every affected object, then run the cold `verify-exact-units.py --all` gate | Clean compile/link plus `verify-modern-linux.sh` when the linker layout or fixed-address ownership is involved |
| Behavior, initialization, callback state, persistence, or rendering interpretation | Strictly compare every touched function; do not accept a byte regression as a semantic improvement | Run the smallest state-transition smoke that exercises the interpretation |

VC7 exactness proves code/data identity for configured units, not the chosen
English name.  Linux success proves that the shared source remains usable on a
second implementation, not that the original binary used the same abstraction.
Acceptance requires the evidence record in addition to both applicable oracle
results.

Do not rebuild the Linux container for every spelling-only edit when an existing
configured modern tree can compile and link the same shared declarations.
Conversely, a change to target-address aliases, global ownership, initialization,
or fixed layout requires the Linux layout verifier and a relevant runtime smoke.

## Batch record

Add a concise completed entry below only after validation.  Keep experiments in
`.analysis/`; do not turn this file into a live scratchpad.

```text
### Owner / field family — YYYY-MM-DD
Scope: functions @ addresses; source files; VC7 object/profile
Observed: widths, offsets, reads/writes, calls, strings, object identity
Corroborated: independent TH08 users; labeled TH06/TH07/upstream evidence
Inference: chosen names/types and confidence; unknowns retained
Layout: sizeof/offsetof assertions added or already present
VC7 oracle: focused build/replay commands and exact unit count
Portable oracle: build/layout/runtime command and result, or why not applicable
Result: raw forms removed for this family; no repository-wide percentage claim
```

Promote a pattern to `docs/VC7_ZUN_PATTERNS.md` or
`docs/BUILD_MATCHING.md` only when it generalizes beyond the batch.  Update
`docs/RE_HANDOFF.md` only when the active family, phase, or blocker changes.

## Completed batches

### ScreenEffect variant parameters — 2026-08-26

Scope: `RegisterChain @ 0x0045B8B0`, the draw/calc callbacks at
`0x0045BB50..0x0045C0DB`, and `src/ScreenEffect.cpp` under the
`/Od /Yu"th_pch.h"` ScreenEffect profile.  `src/ScreenEffect.hpp` remains
byte-for-byte identical to the branch parent.

Observed: `RegisterChain` stores its three generic dword parameters at
`ScreenEffect + 0x18/+0x1C/+0x20`.  Full/partial/arcade fades read the first
dword as RGB color.  The `0x0045BC90/0x0045BD70` pair decrements the first
dword as a repeat count and splits alpha/RGB from the second dword.  `CalcShake`
interpolates between signed amplitudes in the first two dwords.  The
`0x0045BF10` callback treats all three as ramp-up, hold, and ramp-down frame
counts and multiplies the resulting envelope by `duration` as the amplitude.
Independent TH08 callers pass count/color pairs for effect 3, amplitude pairs
for effect 1, and three phase lengths for effect 7.

Corroborated: TH06 calls the first two slots a generic effect parameter and a
shake parameter; the bounded TH07 fade reconstruction independently identifies
the `+0x18` slot as the fade RGB parameter.  Those adjacent versions support
the stable layout, while the TH08 callbacks and callers establish the variant-
specific roles used here.

Inference: a TU-local 0x0C `ScreenEffectParameters` overlay now exposes raw
initialization, fade color, shake amplitudes, pulse repeat/color, and shake-
envelope views from one cast boundary anchored to the existing `+0x18` member.
Keeping the overlay private avoids changing the widely included VC7 header.
These role names have high dataflow confidence.  The still-unknown effect enum
names and `ScreenEffect::unk24` were deliberately left outside this batch.

Layout: assertions pin the overlay/raw sizes, its inner `+0x4/+0x8` members,
and the existing `ScreenEffect::arcadeFadeColor` anchor at `+0x18`; the existing
header assertion continues to pin `sizeof(ScreenEffect) == 0x34`.

VC7 oracle: `verify-exact-units.py --object build/ScreenEffect.obj` passed
**16 / 16**; the canonical objdiff path passed **5 / 5**.  The latter baseline
first required restoring two target-observed `g_GameManager + 0x3DBAC` DIR32
manifest entries at relocation offset 10; that ledger repair is the separate
commit immediately before this batch.

Cold aggregate diagnostic: `verify-exact-units.py --all --json` rebuilt all 75
configured objects and checked all 1,105 selected units, but returned
**1,017 / 1,105** with 88 failures outside ScreenEffect.  Repeating after
restoring `ScreenEffect.hpp` byte-for-byte reproduced the identical failure
set.  This is an aggregate branch-baseline blocker, not an aggregate exact
claim or a ScreenEffect regression; the generated report remains under
`build/accepted-unit-replay-semantic-screen-effect.json`.  The maintenance
batch below subsequently resolved this blocker; these numbers remain the
historical observation that routed that work.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, then `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 and all
required fixed target-owned layout symbols.

Result: all 21 scattered raw byte-offset expressions for the three ScreenEffect
parameter slots were replaced by one asserted typed boundary plus role-specific
members.  The semantic-debt router reports zero raw-member-access candidates in
this source.  Authored/exact progress ledgers are unchanged.

### Post-port exact-oracle reconciliation — 2026-08-26

Scope: the 88-unit cold failure set exposed after the playable-port owner
canonicalization; `EclExIns`, `EffectManager`, `EnemyManager`, `GameManager`,
`Gui`, `Player`, and the shared `ZUN_MIN` source shape.  This was one bounded
oracle-maintenance batch, not a new authored recovery lane.

Observed: 74 initial failures were relocation-manifest disagreements and 14
were function-extent disagreements.  After restoring the target source shapes,
the remaining 79 relocation-only units all had zero non-relocation byte
differences when relocation fields were masked.  Independent target-value plus
COFF-addend inference for all 338 changed/new entries agreed with the existing
canonical target bases, with no conflicts.  The two real code differences were
`Gui::FUN_00438fe9 @ 0x00438FE9`, whose target directly reads
`g_Supervisor.unk164`, and `AnmManager::DrawInner @ 0x004628B0`, whose float
minimum follows the Windows `min` predicate `x < y ? x : y` and therefore has
distinct NaN/code-generation behavior from the former equivalent-on-normal-
values expression.

Corroborated: the target-pinned fact packet for
`GameManager::GameplaySetupThread @ 0x0043ABD7` confirms the deliberate mix of
the cached member owner and direct `g_GameManager` accesses.  Existing mapping,
function, global, and accepted-relocation entries corroborate the inferred
owners.  The Player data lane identifies six callback arrays plus the mixed
collision-callback/difficulty-name table at `0x004C7E00..0x004C7F24`.

Inference: target-only ECL extension storage remains directly named in VC7
translation units, while TU-local `TH08_MODERN_PORT` bridges route the same
logical state through `Background` in the portable build.  The target can clear
the adjacent `g_EclManager` and call-parameter storage with one typed `memset`;
the portable build uses two clears because its linker does not promise that
adjacency.  `GameManagerFlags::playerDeathDissolveMode` is a two-bit state at
bits 7..8; its gameplay role is supported by the alternate death-dissolve path,
while the individual values remain unknown.

Layout: existing assertions continue to pin the ECL barrier VM offsets and
manager sizes.  No class size, field width, calling convention, or vtable order
changed.  Six target-backed Player table owners were added to the global
ledger; `config/claims.csv` remains header-only.

VC7 oracle: affected-object replay passed **238 / 238** after the cold build.
The required non-reuse command `verify-exact-units.py --all --json` cleared and
rebuilt all 75 configured objects and passed **1,105 / 1,105** with no failures.
The normal VC7 production image also linked successfully.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target.  `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 and all
fixed target-owned layout symbols.

Result: the branch-baseline blocker is closed without weakening a comparison.
The manifest now records the evidence-backed semantic owners/addends emitted by
the shared source, while all configured accepted target bytes remain exact.

### Player deathbomb and Bomb lifecycle — 2026-08-26

Scope: `Player::Die @ 0x0044AB40`, the movement/update paths at
`0x0044AEC0` and `0x0044C650`, the death transition at `0x0044CBF0`, the
respawn/draw/add callbacks at `0x0044D180`, `0x0044D530`, and `0x0044D650`,
`Spellcard::EndSpell @ 0x004161B0`, and the exact Player shot/collision users
of the Bomb-active state.  The shared layout lives in `src/Player.hpp`; source
users span Player, PlayerBomb, SpellCard, GameManager, EnemyManager, and the
non-canonical Player option probe.

Observed: TH08 uses dword accesses for `Player + 0xFDC/+0xFE0/+0xFE4/+0xFEC`
and calls two five-entry callback groups at `Player + 0x1000/+0x1014`, indexed
by the `+0xFE0` value.  `AddedCallback` copies adjacent 0x14-byte rows from the
target callback table into those groups; the update path calls the first group
and `OnDrawHighPrio` calls the second.  `Player + 0xE2A68` is initialized from
SHT header `+0x8`, recomputed by `Die`, decremented once per dying frame, and
must remain nonzero for Bomb acceptance.  `Player + 0xE2A6C` is written to 16
by `Spellcard::EndSpell`, decremented by the Bomb-input path, and blocks that
input until zero.  `Player + 0xE2B28` stores the effect VM created for the
deathbomb window and is disabled/cleared when the window expires or a Bomb is
accepted.  Target-pinned fact packets confirm the full exact extents and ABIs
of `0x0044AB40`, `0x0044C650`, `0x0044CBF0`, `0x0044D650`, and
`0x004161B0`; the latter independently records the `0x018B8964` dword write.

Corroborated: TH06 independently models the corresponding Bomb aggregate with
`isInUse`, `duration`, `timer`, calculation, and draw callbacks.  TH08 changes
the callback representation substantially, so only those stable roles are
borrowed; TH08's own table copies, indirect calls, gates, and state transitions
establish the two five-callback groups and all final offsets.

Inference: `deathbombWindowFrames`, `bombInputLockFrames`, `isInUse`,
`callbackSetIndex`, `duration`, `bombsConsumed`, `calcCallbacks`,
`drawCallbacks`, and `deathbombEffectVm` are high-confidence dataflow names.
`bombsConsumed` is supported by all three writes but has no independent read in
the authored source, so its downstream purpose remains unknown.  The
individual meanings of callback slots 0..4, `PlayerBombState + 0xC`, and
`Player + 0xE2B20/+0xE2B24` remain deliberately unnamed.

Layout: assertions pin the SHT deathbomb default at `+0x8`, the 0x14 callback
group size, Bomb state fields/groups through `workItems + 0x4C`, Player fields
at `+0xE2A68/+0xE2A6C/+0xE2B28`, `sizeof(PlayerBombState) == 0xB7858`, and
`sizeof(Player) == 0xE2B30`.  No field width, class size, calling convention,
vtable, global identity, or initialization order changed.

VC7 oracle: serial focused object builds followed by accepted replay passed
**249 / 249** across Player, PlayerBomb, SpellCard, GameManager, and
EnemyManager production/reimplementation objects.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  The normal VC7 production image
also linked successfully; no match manifest or exact ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  A runtime state-transition smoke is not
claimed: this batch changes only names/typed member expressions, and the
portable callback identities, state operations, and initialization sequence
are unchanged.

Result: 79 raw-member candidates in `src/Player.cpp` were replaced by asserted
typed fields and callback-group expressions (442 to 363 in the routing report).
That delta is a review aid, not a semantic-completion percentage; the retained
unknown storage and unrelated Player offsets remain future bounded families.

### BulletManager Laser lifecycle — 2026-08-26

Scope: `BulletManager::RemoveAllBullets @ 0x00430830`, `DespawnBullets @
0x00430AA0`, `SpawnLaserPattern @ 0x00430F20`, `OnUpdate @ 0x00431240`, and
`OnDraw @ 0x00432B50`, plus the Laser-control cases inside
`EclManager::RunEcl @ 0x004184B0`.  The shared layout is `Laser` in
`src/BulletManager.hpp`; its target size remains 0x59C.

Observed: TH08 accesses `Laser + 0x548/+0x554..+0x588` as dword position,
angle, length, width, speed, timing, active-state, and `ZunTimer` storage;
`+0x594/+0x596` are word accesses, while `+0x598/+0x599` are byte accesses.
`SpawnLaserPattern` initializes position, angle, length/width/timing fields,
sets `inUse`, and chooses state 0 or 1 from whether `startTime` is nonzero.
`OnUpdate` advances the beam endpoints, builds its collision box, changes
state 0 to 1 after `startTime`, changes state 1 to 2 after `duration`, and
clears `inUse` after the despawn duration.  Both clear paths force state 2,
reset the timer, preserve the current rendered width bit-for-bit, and remove
the remaining hitbox delay.  `OnDraw` uses `+0x599` only to suppress the cap
VM while the beam is still in state 0.  RunEcl opcodes 117/118/167 update the
angle, opcode 119 updates position, opcode 170 writes that cap gate, opcode
120 reads `inUse`, and opcode 121 performs the same active-to-despawning
transition.  Target-pinned packets cover all six complete exact function
extents; target disassembly independently confirms each access width.

Corroborated: TH06 independently lays out Laser as two ANM VMs followed by
position, angle, start/end offsets, start length, width, speed, start/hitbox/
duration/despawn timing, `inUse`, timer, flags, color, and state.  Its update
and draw paths use the same starting/active/despawning sequence.  TH08 itself
establishes the added `currentWidth` and cap-gate fields and remains the source
of truth for all final offsets and behavior.

Inference: `LASER_STATE_STARTING`, `LASER_STATE_ACTIVE`, and
`LASER_STATE_DESPAWNING` are high-confidence transition names.
`hideCapDuringStartup` is a high-confidence dataflow name: a nonzero value
suppresses only the secondary/cap VM while state is starting, and never
suppresses the active or despawning cap.  The individual meanings of flag bits
`0x1` and `0x4`, the 16-bit `color` convention, and `Laser + 0x59A/+0x59B`
remain deliberately unnamed or unrefined.

Layout: focused assertions pin the second VM at `+0x2A4`, every relied-on
Laser field from position `+0x548` through the cap gate `+0x599`, and
`sizeof(Laser) == 0x59C`.  Field widths, object size, construction order,
calling conventions, vtables, and global ownership are unchanged.

VC7 oracle: serial focused builds and replay passed **24 / 24** accepted
BulletManager units and **1 / 1** complete RunEcl unit.  The required
non-reuse `verify-exact-units.py --all --json` then cold-built all 75
configured objects and passed **1,105 / 1,105** with no failures.  The normal
VC7 production image linked successfully; no match manifest or exact ledger
changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Laser gameplay smoke
exists, so no runtime smoke is claimed; the state operations, callback/global
identity, initialization order, and rendering call sequence are unchanged.

Result: the five core BulletManager functions and seven RunEcl Laser-control
opcodes now use the asserted `Laser` owner, named lifecycle states, and typed
members instead of raw Laser offsets.  The routing report for
`src/BulletManager.cpp` falls from 323 to 294 raw-member candidates.  That
29-location delta is a work-selection aid, not a semantic-completion metric;
unrelated Bullet runtime, sprite-template, and manager-tail storage remains
future bounded work.

### Bullet core lifecycle and ECL controls — 2026-08-26

Scope: `BulletManager::Initialize @ 0x0042F360`, single-bullet spawn at
`0x0042F5F0`, the transform dispatcher at `0x0042FFC0`, removal/despawn paths
at `0x00430830..0x00430E10`, `OnUpdate @ 0x00431240`, bullet reset and draw-
bucket reset at `0x00432170/0x004321B0`, the transform handlers at
`0x00432210..0x00432AA0`, `OnDraw @ 0x00432B50`, `DrawSingleBullet @
0x00432F20`, and `AddedCallback @ 0x00433070`.  Nine exact ECL extension
handlers at `0x00423A60`, `0x00423E20`, `0x004241E0`, and
`0x00424A20..0x00425290` supply independent freeze, unfreeze, zone-transition,
and bullet-control users.  Shared declarations live in
`src/BulletManager.hpp`; users changed in `src/BulletManager.cpp` and
`src/EclExIns.cpp`.

Observed: TH08 uses a 0x10B8-byte `Bullet` pool with 0x600 usable entries and a
sentinel entry whose 16-bit state is 6.  The target reads/writes position,
velocity, speed, and angle as floats; two adjacent `ZunTimer` values begin at
`+0xD80`; the configured and currently active transform masks at
`+0xDB0/+0xDAC` are dwords.  State and consecutive-offscreen-frame count are
words at `+0xDB8/+0xDBA`; graze and spawn-cancellation flags are bytes at
`+0xDBD/+0xDBE`; the collision gate at `+0x10B4` is a signed byte.  A pointer
at `+0xDC0` chains each live bullet into one of six manager draw buckets.
Spawn, update, collision, removal, transform dispatch, ECL controls, and draw
independently agree on those widths and owners.  The update path selects one
of five sprite VMs from the 0..5 state machine, advances the relevant timer,
performs collision/graze and offscreen culling, and links surviving bullets
into the template-selected draw bucket.  Target-pinned fact packets cover
spawn, update, draw, freeze, and a zone-transition handler; target disassembly
independently confirms the relied-on access widths.

Corroborated: TH06 independently models five bullet VMs followed by collision
size, position, velocity, speed, angle, timers, lifecycle state, graze state,
and transform flags.  It supports those stable roles only.  TH08's own pool
walks, state transitions, ECL handlers, draw lists, and exact code generation
establish the final offsets, widths, and TH08-specific manager tail.

Inference: `BULLET_STATE_UNUSED`, `FIRED`, the three spawn-speed states,
`DESPAWNING`, and `SENTINEL` are high-confidence transition names.
`offscreenCullDelayFrames`, `activeTransformFlags`, `transformFlags`,
`offscreenFrames`, `isGrazed`, `cancelledDuringSpawn`,
`nextInDrawBucket`, `zoneTransitionCooldownFrames`, `collisionDisabled`, the
five sprite-VM roles, `collisionSize`, and `drawBucketIndex` are
high-confidence dataflow names.  `Bullet + 0xD5C` remains neutrally named
`unknownVectorD5C`: constructor code generation proves that its type is
`Float3`, but this batch does not establish its gameplay role.  The byte at
`+0xDBC`, template byte `+0xD40`, transform-specific `BulletExState` meanings,
individual transform-bit names, and ANM VM internals remain deliberately raw
or opaque.

Layout: assertions pin all relied-on `BulletTypeSprites`, `Bullet`, and
`BulletManager` members, including `sizeof(BulletTypeSprites) == 0xD44`,
`sizeof(Bullet) == 0x10B8`, the Bullet/Laser pool starts, the six draw buckets,
and `sizeof(BulletManager) == 0x6BA578`.  Field widths, pool capacity,
construction order, calling conventions, vtables, and global ownership are
unchanged.

VC7 oracle: focused production and canonical objdiff builds replayed **70 / 70**
accepted units across `BulletManager.obj` and `EclExIns.obj`.  The first cold
pass exposed three code-generation-visible implicit `Float3::operator float*`
call shapes in spawn, update, and draw; restoring those natural typed calls
made their canonical units exact.  The final required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully; no match manifest or exact ledger changed.

Portable oracle: after the final source-shape correction,
`scripts/build-modern-linux-container.sh` compiled and linked the complete
i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Bullet gameplay
smoke is claimed; this batch preserves the state operations, callback/global
identity, initialization order, collision calls, and rendering sequence.

Result: the routing report for `src/BulletManager.cpp` falls from 294 to 97
raw-member candidates and `src/EclExIns.cpp` from 25 to 18.  Those deltas are
review aids, not semantic-completion percentages.  The retained candidates are
primarily transform-specific state, ANM VM internals, and explicitly unknown
bytes rather than the core Bullet lifecycle family completed here.

### Bullet transform runtime state — 2026-08-26

Scope: the transform dispatcher at `Bullet::FUN_0042ffc0 @ 0x0042FFC0`, the
bullet update path at `BulletManager::OnUpdate @ 0x00431240`, and the nine
transform handlers at `0x00432210..0x00432AA0`.  Single-bullet and pattern
spawn at `0x0042F5F0`, `0x00430E10`, and `0x00430F20` provide the adjacent
flag users.  Shared declarations live in `src/BulletManager.hpp`; this batch
changes only that header and `src/BulletManager.cpp`.

Observed: each `Bullet` contains seven consecutively constructed 0x2C-byte
`BulletExState` objects at `+0xF80`.  TH08's dispatcher and handlers assign
those slots to deceleration, vector acceleration, polar acceleration, the
three direction-change modes, boundary bounce, wait, and X/Y wrap.  Within a
slot, target accesses establish a `ZunTimer @ +0x0`, two floats at `+0xC/+0x10`,
a `Float3 @ +0x14`, and three dwords at `+0x20/+0x24/+0x28`.  The paired
dispatcher/handler dataflow establishes acceleration magnitude and angle,
speed and angle deltas, duration, direction-change speed/angle/interval/repeat
and completion counts, bounce speed/count/limit, and the wait/wrap countdowns.
Target disassembly independently confirms every relied-on width and offset;
target-pinned typed-re packets cover the dispatcher, update path, and all nine
handlers.

Corroborated: TH06 independently uses the same spawn-speed bits, vector and
polar acceleration bits, three direction-change bits, two boundary-bounce
bits, and spawn-sound bit with the same stable behavior.  This supports the
cross-version names only; TH08's target dataflow establishes the final slot
layout, added action bits, cancel-immunity behavior, and program sequencing.

Inference: the transform constants are high-confidence behavior names from
their TH08 dispatch actions and update-side gates.  `BULLET_TRANSFORM_CANCEL_IMMUNE`
names the bit that bypasses spawn suppression and every collision-result-2
cancel path.  The seven `BulletTransformStateSlot` names and role-specific
union aliases are high-confidence because each is initialized by one transform
case and consumed by its corresponding handler.  The generic
`BulletTransformRecord::float0/float1/int0/int1` fields remain deliberately
neutral because their meaning depends on the tagged transform kind.  The
deceleration slot's bitwise-zeroed `Float3::z` field remains unnamed because no
consumer establishes its gameplay role.

Layout: assertions pin `sizeof(BulletExState) == 0x2C`, every payload member
offset, `Bullet::exStates @ +0xF80`, and the existing `sizeof(Bullet) ==
0x10B8`.  The seven-element array is retained because its nontrivial VC7 vector
construction is target-visible.  Field widths, construction order, calling
conventions, pool layout, and state operations are unchanged.

VC7 oracle: the focused production/canonical replay passed **37 / 37** accepted
`BulletManager.obj` units after both the state typing and the final flag-name
cleanup.  The required non-reuse `verify-exact-units.py --all` then cold-built
all 75 configured objects and passed **1,105 / 1,105** with no failures.  A
subsequent normal VC7 production image linked successfully; no match manifest
or exact ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated transform gameplay
smoke exists, so no runtime smoke is claimed.

Result: raw byte-offset views of the complete `BulletExState[7]` transform
runtime family are replaced by asserted typed owners, slot names, and
role-specific fields without changing accepted VC7 bytes or portable behavior.
The semantic router for `src/BulletManager.cpp` falls from 97 to 29 candidates.
That delta is a review aid, not a completion percentage; retained candidates
are primarily ANM internals, two explicitly unknown bytes/vectors, replay
storage, and the adjacent ECL descriptor family.

### Bullet spawn and ECL descriptor family — 2026-08-26

Scope: `EclManager::RunEcl @ 0x004184B0`, in particular shot opcodes 96..104
and descriptor opcodes 109, 111, and 113..115; the shot dispatcher at
`0x00422720`; Enemy initialization, construction, phase reset, and update at
`0x00429E00`, `0x0042A280`, `0x0042B490`, `0x0042B930`, and `0x0042C660`; and
the bullet/laser spawn consumers at `0x0042F5F0`, `0x00430E10`, and
`0x00430F20`.  Shared declarations live in `src/BulletManager.hpp` and
`src/EnemyManager.hpp`; typed producers and reset users span
`src/EclRunHigh.inl`, `src/EclDependencies.cpp`, `src/EnemyManager.cpp`, and
`src/EnemyManagerUpdate.cpp`.

Observed: `Enemy` owns two separately constructed 0x210-byte
`BulletSpawnDescriptor` members: the bullet pattern descriptor at `+0x2E24`
and the laser descriptor at `+0x3070`.  Constructor order, phase-reset copies,
and the target absolute source `g_EnemyManager.spawnTemplate + 0x2E24` agree on
those owners.  The descriptor contains an 18-element array of 0x18-byte
`BulletTransformRecord` values at `+0x20`, followed by the laser geometry and
timing block at `+0x1D0`, count/aim fields at `+0x1F4`, transform flags at
`+0x1FC`, sound fields at `+0x200/+0x204`, the starting transform index at
`+0x208`, and the resolved sprite-template pointer at `+0x20C`.

TH08 ECL dataflow establishes the protocol.  Opcodes 96..104 decode a
0x20-byte `ShotArgs` packet into the bullet descriptor and select aim mode by
`opcode - 0x60`.  Opcode 109 spawns the stored bullet descriptor; opcode 111
writes all six members of one indexed transform record; opcode 113 controls
the spawn sound flag/index and transform sound index.  Opcodes 114/115 decode
a 0x34-byte `LaserSpawnArgs` packet into the separate laser descriptor and
select aimed/fixed laser angle behavior.  The shot dispatcher also proves the
two player-state conditional flags: one suppresses the shot unless the player
is youkai, and the other suppresses it unless the player is human.

Corroborated: TH06 independently supplies the nine fan/circle/offset/random aim
mode names and the same stable spawn-sound behavior.  TH08's own switch,
position arithmetic, rank scaling, transform-record writes, laser packet
decode, and consumers establish the final field ownership and all TH08
offsets.  The adjacent version is not used to infer the two Enemy member
locations or the laser packet layout.

Inference: `bulletSpawnDescriptor`, `laserSpawnDescriptor`, `BulletAimMode`,
`LaserSpawnArgs`, and the two player-state conditional transform flags are
high-confidence names from paired TH08 producers and consumers.  The tagged
record operands `float0/float1/int0/int1` remain neutral because their meaning
varies by transform kind.  `BulletSpawnDescriptor::unknown1FA` remains unknown:
the target clears it before bullet spawn, but this family provides no
independent nonzero use from which to recover a role.

Layout: assertions pin every `BulletTransformRecord` member, the relied-on
`BulletSpawnDescriptor` fields and 0x210-byte size, both descriptor positions
inside `Enemy`, the 0x20-byte shot packet, and the 0x34-byte laser packet.
Construction order, copy sizes, field widths, calling conventions, and object
sizes are unchanged.

VC7 oracle: focused production/canonical replay across `EclRun.obj`,
`EclDependencies.obj`, `BulletManager.obj`, `EnemyManager.obj`, and
`EnemyManagerUpdate.obj` passed **102 / 102** accepted units.  An intermediate
pass found a two-byte source-shape mismatch in `EnemyManager::Initialize` when
the typed write bypassed the target's existing `enemy` local; expressing the
same typed owner through that local restored **1,029 / 1,029** exact bytes.
The required final non-reuse replay cold-built all 75 configured objects and
passed **1,105 / 1,105**.  A subsequent normal VC7 production image linked
successfully; no match manifest or exact ledger changed.

Portable oracle: the complete Linux i386 container build linked, and
`scripts/verify-modern-linux.sh build/modern-linux-container/th08-modern`
verified ELF32/ET_EXEC/i386 plus every fixed target-owned layout symbol.  No
isolated automated ECL-shot gameplay smoke exists, so no runtime smoke is
claimed.

Result: all known bullet/laser spawn-descriptor producers, reset/copy paths,
and consumers now share asserted typed owners; the corresponding ECL transform
record, sound, aim, and laser operand offsets are gone from authored gameplay
code.  Together with the accepted Player deathbomb/Bomb, Laser lifecycle,
Bullet core lifecycle, and Bullet transform-state batches, this closes the
first semantic milestone: **core bullet-gameplay loop semantic closure**.
This milestone is a bounded subsystem claim backed by the listed target and
portable oracles, not a whole-program semantic-completion percentage.

### Enemy bullet-rank and repeated-shot scheduling — 2026-08-26

Scope: the complete `EclManager::RunEcl @ 0x004184B0`, especially shot
opcodes 96..106 and rank-influence opcode 152; the shot dispatcher at
`0x00422720`; the post-ECL shot scheduler at `0x00423150`; Enemy template
initialization and phase reset at `0x00429E00`, `0x0042B490`, and
`0x0042B930`; the spell-phase reset at `0x00415C80`; and the death-callback
reset inside `EnemyManager::OnUpdate @ 0x0042C660`.  Shared layout is in
`src/EnemyManager.hpp`; typed users span `src/EclRunHigh.inl`,
`src/EclDependencies.cpp`, `src/EnemyManager.cpp`, `src/Spellcard.cpp`, and
`src/EnemyManagerUpdate.cpp`.

Observed: TH08 stores two float rank endpoints at `Enemy + 0x2DEC/+0x2DF0`
and two signed-word endpoint pairs at `+0x2DF4..+0x2DFA`.  The shot dispatcher
passes the float pair to `ScaleFloatBasedOnRank` for both bullet speeds and
the two word pairs to `ScaleIntBasedOnRank` for the two pattern counts.
Opcode 152 writes those six values with float operand resolution for the
speed endpoints and integer operand resolution followed by 16-bit stores for
the count endpoints.  The spell-phase reset writes `-0.5/+0.5` and zeroes all
four count endpoints; the template initializer writes `-0.15/+0.15`.

When Enemy flag bit 17 is set, opcodes 96..104 copy exactly 11 dwords
(0x2C bytes) of the current ECL instruction to `Enemy + 0x3034` instead of
dispatching it.  Opcodes 105/106 write the dword at `+0x3060`, adjust it by
rank using `interval / 5` and `-interval / 5`, and initialize the `ZunTimer`
at `+0x3064` either to zero or a random value below the adjusted interval.
`Enemy::FUN_00423150` advances that timer while the Enemy is alive and, on
expiry, redispatches the stored instruction and resets the timer.  Template,
phase, and death-callback paths independently clear the interval.

Corroborated: TH06 independently names the corresponding six endpoints
`bulletRankSpeedLow/High`, `bulletRankAmount1Low/High`, and
`bulletRankAmount2Low/High`, and uses the same rank-scaled repeat-shot
interval formula.  TH08's own producer/consumer pairs establish all final
offsets, widths, signedness, storage size, and scheduling behavior; the
adjacent version supplies names only.

Inference: `EnemyBulletRankInfluence`, its six endpoint members,
`shootIntervalFrames`, and `shootIntervalTimer` are high-confidence behavior
names.  `pendingShotInstruction` is intentionally a byte array rather than an
embedded `EclRawInstruction`: TH08 performs a fixed-size protocol copy and the
instruction type has a variable operand tail, so imposing a normal C++ object
would overstate the recovered representation.

Layout: assertions pin `sizeof(EnemyBulletRankInfluence) == 0x10`, the rank
aggregate at `+0x2DEC`, the 0x2C-byte instruction snapshot at `+0x3034`, the
interval at `+0x3060`, its timer at `+0x3064`, and the existing descriptor and
Enemy sizes.  No field width, copy size, construction order, calling
convention, or state operation changed.

VC7 oracle: focused replay passed **71 / 71** accepted units across
`EclRun.obj`, `EclDependencies.obj`, `EnemyManager.obj`, `SpellCard.obj`, and
`EnemyManagerUpdate.obj`.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully; no match manifest or exact ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated repeat-shot runtime
smoke exists, so no runtime smoke is claimed.

Result: raw offset views of the full bullet-rank influence aggregate, delayed
shot snapshot, repeat interval, and interval timer are replaced by one
asserted Enemy owner while the serialized instruction remains honestly
byte-oriented.  The routing report falls from 1,449 to 1,438 raw-member
candidates; that delta is a review aid, not a semantic-completion percentage.

### Enemy combat identity and life state — 2026-08-26

Scope: Enemy template initialization at `0x00429E00`; spawning at
`0x0042A4E0/0x0042A680`; linked-enemy damage and phase transitions at
`0x0042B370`, `0x0042B490`, and `0x0042B930`; the complete update at
`0x0042C660`; the complete ECL dispatcher at `0x004184B0`; integer/float ECL
operand resolvers at `0x0041F420`, `0x0041FE10`, and `0x00420120`; Spellcard
start/update; Player enemy-hit accumulation at `0x00451670`; and three ECL EX
laser-hit callbacks at `0x00424730..0x00424910`.  Shared declarations live in
`src/EnemyManager.hpp` and `src/Spellcard.hpp`; consumers span ten production
objects.

Observed: TH08 reads and writes `Enemy + 0x2DFC` as signed current life and
uses `+0x2E00` as the divisor for the Boss life ratio and gauge-segment
fractions.  Spawn and ECL opcode 131 initialize both from the same value;
damage and death paths decrement/test/clear current life.  `+0x2E08` is the
spawn-configurable value awarded by both scored death modes and is exposed as
an ECL integer/float lvalue.  `+0x2E14` is a complete `ZunTimer`: the main
update advances it, pause paths decrement it, phase changes clear it, ECL
opcodes set/clear it, ECL operands expose its `current` member, and three EX
callbacks use its interval predicate.

Spawn assigns the pool cursor index to `+0x2E0C`.  Linked-child visual paths
use its low bit to mirror alternating effects, while Spellcard start snapshots
the full dword and the update path compares that snapshot against the active
Enemy.  `+0x2E10` is passed by address as the per-Enemy hit accumulator to
`Player::CalcDamageToEnemy`, which consumes thresholds and adds capped damage; the
Player added callback initializes the template copy from
`Player::damageAccumulatorThreshold`.  Finally, `+0x2E20` is initialized from
the primary ANM VM color and copied into/out of that VM around script
execution.

The dword at `+0x2E04` is initialized from maximum life on spawn/opcode 131,
updated to current life at phase transitions, and independently writable by
opcode 177.  No authored TH08 consumer currently reads it.  Those writes prove
phase-local life ownership and width, but not a narrower display or damage
role.

Corroborated: TH06 independently places current life, maximum life, score,
Boss timer, and display color together after the same bullet-rank aggregate.
It supports the stable names only.  TH08's own spawn, ECL, damage, death,
Spellcard, ANM, and Player-hit dataflow establishes all final TH08 offsets and
the added index/accumulator/phase fields.

Inference: `life`, `maxLife`, `score`, `enemyIndex`,
`playerShotHitAccumulator`, `bossTimer`, and `displayColor` are
high-confidence behavior names.  `phaseStartingLife` is deliberately neutral
and moderate-confidence: it records the common spawn/phase write pattern
without claiming an unobserved consumer.  The Spellcard snapshot is renamed
`activeEnemyIndexSnapshot`; its target type and StartSpell ABI remain
unchanged.

Layout: assertions pin all eight consecutive fields at `Enemy +
0x2DFC..+0x2E20`, including the complete 0x0C `ZunTimer`, while the existing
rank/descriptor and `sizeof(Enemy) == 0x53D0` assertions still pass.  No
calling convention, decorated function type, construction order, global
identity, field width, or state operation changed.

VC7 oracle: focused replay passed **163 / 163** accepted units across
`EclRun`, `EclDependencies`, both operand resolvers, `EclExIns`,
`EnemyTimeline`, `EnemyManager`, `EnemyManagerUpdate`, `SpellCard`, and
`Player`.  Target-pinned fact packets independently replayed exact spawn,
linked-damage, update, and Player-hit functions.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully; no match manifest or exact ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Enemy combat smoke
exists, so no runtime smoke is claimed.

Result: every authored raw offset view in the `Enemy + 0x2DFC..+0x2E20`
combat-state range is replaced by the asserted Enemy owner, including ECL
resolver lvalues and cross-subsystem Player/Spellcard users.  The routing
report falls from 1,438 to 1,406 raw-member candidates and from 75 to 73 opaque
storage candidates; those deltas are review aids, not completion percentages.

### Enemy Boss phase callbacks and child ECL ownership — 2026-08-26

Scope: `EclManager::RunEcl @ 0x004184B0`, `PopEclContext @ 0x00421CB0`,
`Enemy::FUN_0042B370`, the life transition at `0x0042B490`, the timer
transition at `0x0042B930`, child-block cleanup at `0x0042BC90`,
`EnemyManager::OnUpdate @ 0x0042C660`, and the Spellcard/GUI/operand consumers
of `Enemy + 0x3354..+0x3393`.

Observed: `+0x3354` is cleared before Player collision processing, receives
the final applied damage, and is exposed as ECL operand `0x2763`.  Opcode 133
writes four dword life thresholds at `+0x3358` and four parallel dword
subroutine identifiers at `+0x3368`.  The phase transition scans the threshold
table, clamps life to the crossed threshold, and invokes the corresponding
subroutine by loading the low signed 16 bits of its dword slot.  Operands
`0x2768..0x276B` expose the four thresholds.

Opcode 134 writes a frame threshold at `+0x3378` and a dword subroutine
identifier at `+0x337C`; the timer transition compares the Boss timer against
the threshold, calls through the identifier's low signed 16 bits, and resets
both timer state and phase resources.  Opcode 153 copies the existing signed
16-bit death callback identifier into the same dword slot.  Spellcard and GUI
consumers use the threshold as the phase duration.

The dword at `+0x3380` is incremented by all three linked-child spawn paths,
decremented when an attached child dies, and displayed/converted into item
drops by the parent.  The four pointers at `+0x3384` own separately allocated
`0x24B0` child ECL blocks.  Opcode 135 allocates, clears, seeds, and starts a
block; RunEcl rotates through the blocks; PopEclContext and every phase/death
cleanup path free the owning slot and set it to null.

Corroborated: no adjacent-version name is needed for this batch.  The imported
TH08 baseline supplied control-flow hypotheses only; the exact TH08 1.00d
instruction widths, indexed displacements, direct calls, allocation extent,
and all authored producers/consumers establish the final layout and roles.

Inference: `lastDamage`, `lifeCallbackThresholds`,
`timerCallbackThresholdFrames`, `linkedChildCount`, and `childEclBlocks` are
high-confidence behavior/ownership names.  `lifeCallbackSubIds` and
`timerCallbackSubId` deliberately remain dword fields because their ECL
opcodes write 32 bits, while the call sites explicitly view only their low
signed 16 bits.  `childEclBlocks` remains byte-oriented: the target proves the
allocation ownership and several internal offsets, but not a complete
`0x24B0` C++ object layout.

Layout: assertions pin the complete consecutive family at
`+0x3354/+0x3358/+0x3368/+0x3378/+0x337C/+0x3380/+0x3384`, the following
trail array at `+0x3394`, and the unchanged `sizeof(Enemy) == 0x53D0`.
No field width, array extent, pointer ownership operation, call ABI, timer
operation, or ECL presentation-write condition changed.

VC7 oracle: focused replay passed **110 / 110** accepted units across
`EclRun`, `EclDependencies`, both operand resolvers, `EnemyManager`,
`EnemyManagerUpdate`, `SpellCard`, and `Gui`.  Target-pinned fact packets for
the life transition, timer transition, and child-block cleanup independently
replayed exact.  The required non-reuse `verify-exact-units.py --all --json`
cold-built all 75 configured objects and passed **1,105 / 1,105** with no
failures.  A subsequent normal VC7 production image linked successfully; no
match manifest or exact ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Boss-phase gameplay
smoke exists, so no runtime smoke is claimed.

Result: every authored raw view of `Enemy + 0x3354..+0x3393` is replaced by
the asserted owner, including the Spellcard and GUI cross-subsystem users.
The routing report falls from 1,406 to 1,377 raw-member candidates; opaque
storage remains 73 because the now-owned child blocks correctly retain their
unknown byte-oriented representation.  Those counts are routing aids, not a
semantic completion percentage.  The next coherent Enemy/ECL family is the
active ECL context/subroutine state around `+0x2CA0..+0x2D30`, followed by the
motion vectors and phase/control flags that consume it.

### Enemy ECL interpreter and subroutine state — 2026-08-26

Scope: `EclManager::RunEcl @ 0x004184B0`, operand resolvers at
`0x0041F420..0x00420950`, `CallSubOnEnemy @ 0x00421BD0`,
`PopEclContext @ 0x00421CB0`, the ECL extension handlers, both Enemy spawn
paths, the Enemy timeline pending-call opcode, and all phase/death users of
`Enemy + 0x07F8..+0x2D33`.

Observed: the constructor-bearing context at `Enemy + 0x07F8` and sixteen
`0x228`-byte snapshots at `+0x0A20` form the primary interpreter context and
its call stack.  `+0x2CA0` points at the currently executing primary or child
context; `+0x2CA4` points at that context's snapshot stack.  RunEcl selects
child blocks by replacing both pointers, preserves each context's signed
16-bit call depth, then restores the primary pointers before its post-dispatch
Enemy updates.

Within each `EnemyEclContext`, the target accesses the current instruction and
two timers, a fastcall callback/opaque argument pair, eight integer and eight
floating-point script variables, four extra integers and two extra floats,
four integer and four floating call parameters, eight `0x30` interpolation
slots, a child-context slot, and the signed 16-bit current subroutine ID.  The
interpolation slots contain callback storage, a timer, duration, callback
index, easing mode, four parameters, and the affected-variable selector.

The per-Enemy arrays at `+0x2CA8/+0x2CC8` are a second set of eight integer and
eight floating ECL variables exposed by operand IDs `0x2718..0x271F` and
`0x2728..0x272F`.  SpawnEnemy2 copies the exact `0x78`-byte context-variable
range beginning at a parent's local integer variables into the child's
primary context.  `+0x2CE8/+0x2CEA` retain primary/current signed call depths;
`+0x2CEE` is the phase/death callback subroutine ID.  Opcodes 126 and 125 write
the 32-entry signed-16-bit subroutine table at `+0x2CF0` and its pending index
at `+0x2D30`; RunEcl consumes the pending index through the table and clears it
to `-1`.

Corroborated: no adjacent-version layout was imported.  The exact TH08 1.00d
indexed displacements, `0x78` variable copy, `0x228` context-copy/stride, the
child slot at context `+0x220`, signed word loads/stores, direct CallEclSub
calls, and the complete authored producer/consumer set establish the
structure.  The imported class names
`EnemyUnkStruct2/3` are retained because their constructor symbols are part of
the target ABI; their formerly opaque interiors now mirror the proven context
and interpolation layout.

Inference: pointer ownership, variable families, callback/call-parameter
roles, call depths, death callback, subroutine table, and pending index are
high-confidence behavior names.  `extraIntVariables` and
`extraFloatVariables` deliberately describe only their ECL namespace rather
than assigning stage-specific gameplay meanings.  The 2-byte gap at
`Enemy + 0x2CEC`, context dword `+0x21C`, callback argument payload, and the
individual interpolation parameter meanings remain explicitly unknown.

Layout: assertions pin both `0x228` context representations, every known
context subrange through `subId @ +0x224`, primary storage at
`Enemy + 0x07F8/+0x0A20`, and all outer fields from `+0x2CA0` through the
pending index at `+0x2D30`.  The existing `sizeof(Enemy) == 0x53D0` remains
unchanged.  No constructor symbol, callback convention, stack depth width,
copy extent, ECL operand identity, or context-switch operation changed.

VC7 oracle: focused replay passed **124 / 124** accepted units across
`EclManager`, `EclRun`, `EclDependencies`, both operand resolvers, `EclExIns`,
`EnemyTimeline`, `EnemyManager`, and `EnemyManagerUpdate`.  Target-pinned fact
packets for CallSubOnEnemy and PopEclContext independently replayed exact.
The required non-reuse `verify-exact-units.py --all --json` cold-built all 75
configured objects and passed **1,105 / 1,105** with no failures.  A subsequent
normal VC7 production image linked successfully; no match manifest or exact
ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated ECL gameplay smoke
exists, so no runtime smoke is claimed.

Result: authored raw views of the active context, its proven internal fields,
and the outer `Enemy + 0x2CA0..+0x2D30` interpreter state are replaced by the
asserted owners.  The routing report falls from 1,377 to 1,344 raw-member
candidates, anonymous identifiers from 601 to 600, and opaque storage from 73
to 72.  Those counts are routing aids, not completion percentages.  The next
coherent Enemy/ECL family is the motion controller at `+0x2D34..+0x2DEB`,
followed by its control flags and boundary/presentation state.

### Enemy motion controller — 2026-08-26

Scope: `EclManager::RunEcl @ 0x004184B0`, the motion helpers at
`0x00420D10/0x00420F40/0x004222B0`, `Enemy::UpdateMovement @ 0x00422C40`,
`EnemyManager::OnUpdate @ 0x0042C660`, `Enemy::IntegrateVelocity @
0x0042DEB0`, both operand resolvers, the ECL extension/timeline paths, and all
Player, Effect, Spellcard, and Enemy users of `Enemy + 0x2D34..+0x2DEB`.

Observed: `position @ +0x2D34` is the logical/local position manipulated by
ECL motion opcodes.  `positionOffset @ +0x2D40` is added by the attachment
path, and `worldPosition @ +0x2D88` is the resulting collision/render
position.  `velocity @ +0x2D4C` is integrated into the logical position;
`previousPosition @ +0x2D58` retains its pre-update value, while
`lastFrameDisplacement @ +0x2D64` records the completed-frame delta before
the next integration.  `hitboxDimensions @ +0x2D70` is the primary collision
box and `secondaryHitboxDimensions @ +0x2D7C` gates the optional secondary
Player-shot damage test.

The linear controller uses `movementAngle @ +0x2D94`,
`angularVelocity @ +0x2D98`, `speed @ +0x2DA8`, and
`acceleration @ +0x2DAC`.  The polar
controller uses `orbitAngle @ +0x2D9C`, `orbitAngularVelocity @ +0x2DA0`,
`orbitRadius @ +0x2DB0`, `radialVelocity @ +0x2DB4`, and the shared origin at
`movementInterpolationOrigin @ +0x2DD0`.  `parentEnemy @ +0x2DA4` links child
spawn, follow, and death behavior.  `shootOffset @ +0x2DB8` feeds bullet and
laser spawn positions.  Timed displacement uses
`movementInterpolationDelta @ +0x2DC4`, the same origin,
`movementTimer @ +0x2DDC`, and `movementDuration @ +0x2DE8`.

Corroborated: TH06 preserves the earlier sequence of position, primary
hitbox, axis speed, angle/angular velocity, speed/acceleration, shoot offset,
interpolation state, timer, and duration.  TH08 target dataflow independently
establishes the additional prior/displacement/world position, secondary
hitbox, orbit, and parent layers; no TH06 offset or field extent is imported.

Inference: the movement, collision, ownership, and timing names are
high-confidence behavior names because every producer and consumer agrees.
The flags at `Enemy + 0x3324` remain locally viewed in this batch: their
movement-mode, easing, mirroring, and attachment bits need to be recovered as
one adjacent control-state family rather than hidden inside this continuous
field layout.

Layout: assertions pin every field from `+0x2D34` through `+0x2DE8`, including
all `Float3` extents, the parent pointer, `ZunTimer`, and the unchanged
`sizeof(Enemy) == 0x53D0`.  No member width, arithmetic order, callback ABI,
timer operation, or ECL operand identity changed.

VC7 source-shape note: ECL opcode 169 in `RunEcl` retains a named,
layout-asserted byte view of `position` through
`TH08_ECL_ENEMY_POSITION_OFFSET`.  A direct C++ member expression changes the
target's chained x87 comparison by six bytes under VC7 even though its value
and offset are identical.  The exception is confined to that handler and is
not evidence for an anonymous layout.

VC7 oracle: post-rename focused replay passed **287 / 287** accepted units.
Target-pinned fact packets for `UpdateMovement`, `IntegrateVelocity`, and the
polar-motion helper independently replayed exact.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully; the two formerly anonymous core functions are now
tracked consistently as `Enemy::UpdateMovement` and
`Enemy::IntegrateVelocity`.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Enemy-motion
gameplay smoke exists, so no runtime smoke is claimed.

Result: every authored raw view of the continuous Enemy motion range
`+0x2D34..+0x2DEB` is replaced by an asserted owner, apart from the documented
VC7 source-shape view of the same named `position` field.  The routing report
falls from 1,344 to 1,295 raw-member candidates; anonymous identifiers remain
600 and opaque storage falls from 72 to 70.  Those counts are routing aids,
not completion percentages.  The next coherent Enemy/ECL family is the
control flags at `+0x3324/+0x3328` and their adjacent boundary/presentation
state.

### Enemy control and presentation state — 2026-08-26

Scope: `EclManager::RunEcl @ 0x004184B0`,
`Enemy::SetPrimaryAnmScripts @ 0x00421DE0`, the shot dispatcher at
`0x00422720`, `Enemy::UpdateShotAndAnm @ 0x00423150`, initialization and both
spawn paths at `0x00429E00/0x0042A4E0/0x0042A680`, the life/timer callbacks at
`0x0042B490/0x0042B930`, despawn/item-drop/bounds/collision/alignment/update/
draw paths at `0x0042BCF0/0x0042BEA0/0x0042C180/0x0042C290/0x0042C420/
0x0042C660/0x0042E010/0x0042E140`, and all Player, Bomb, Spellcard, Effect,
timeline, extension, helper, and operand-resolver consumers of
`Enemy + 0x3304..+0x3353`.

Observed: `itemDropType @ +0x3304` selects an explicit item for nonnegative
values, schedules the normal drop policy at `-1`, and suppresses it at `-2`.
`pointItemDropCount @ +0x3308` and `powerOrPointItemDropCount @ +0x330C`
control the two repeated drop groups.  The latter emits small power below the
Player power cap and point items otherwise.  Three death-animation bytes,
`bossSlot`, and the one-byte `damageFlashTimer` retain their target widths.

The two dword flag owners at `+0x3324/+0x3328` now provide named masks and
shift constants for every target-observed role.  These include active/Boss,
collision and damage gates, sprite/primary-ANM visibility, linked-child and
parent-position behavior, death-effect and death-mode policy, human/youkai
alignment, movement mode/easing/mirroring/clamping, deferred shot dispatch,
timer and movement pauses, offscreen persistence, timeout-spell state, and
the smaller secondary bank/form/death/damage-feedback controls.  Bits without
an authored producer or consumer remain unnamed.

`anmDirection @ +0x332E` and `drawGroup @ +0x332F` drive animation selection
and draw routing.  `eclDifficultyMaskOverride @ +0x3330` gates ECL execution.
The six-script `EnemyAnmScripts @ +0x3332` owner records initial idle, the
left/right idle returns, left/right movement, and the special script.
`movementBounds @ +0x3340` supplies asserted lower/upper `Float2` limits, and
`minimumPlayerDistanceSquared @ +0x3350` is written by ECL opcode 82 after
squaring its distance operand and suppresses shot dispatch while the Player
is inside that radius.

Corroborated: TH06 preserves the earlier item/death/Boss/timer/flag sequence,
the default/left/right animation-script family, and lower/upper movement
limits.  TH08 target producers and consumers independently establish every
field offset, width, and role used here; no TH06 offset or extent is imported.

Inference and unknowns: `hidePrimaryAnm` and `specialInteraction` deliberately
use behavior-neutral names because the target establishes their gates but not
a narrower game-design label.  `timer3318`, bytes `+0x3315..+0x3317`,
`+0x332C..+0x332D`, `+0x3331`, and `+0x333E..+0x333F`, plus every unobserved
flag bit, remain explicitly unknown.  Splitting one broad opaque range into
those retained gaps increases the router's opaque-storage count without
adding unknown bytes.

Layout: assertions pin every recovered scalar, both flag dwords, the
six-word script table, both bounds vectors, and the unchanged
`sizeof(Enemy) == 0x53D0`.  Twelve target-backed anonymous methods are now
tracked consistently as `ReleaseAttachedEffects`, `UpdateShotAndAnm`,
`DetachFromParentChain`, `ApplyDamageToParent`, `HandleLifeCallback`,
`HandleTimerCallback`, `ReleaseChildEclBlocks`, `Despawn`, `DropItems`,
`CheckPlayerCollision`, `UpdateYoukaiAlignment`, and `UpdateEffects`.

VC7 source-shape note: target reads of individual flag bits use a right shift
followed by `& 1`.  Replacing them with semantically equivalent mask tests
changed 26 accepted units under VC7.  Shared named shift constants preserve
that observed expression shape; mask constants remain natural for writes and
multi-bit updates.

VC7 oracle: post-source-shape focused replay passed **328 / 328** accepted
units.  Target-pinned packets for `OnUpdate`, `ClampPosition`, and
`SetPrimaryAnmScripts` independently replayed exact.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Enemy-control
gameplay smoke exists, so no runtime smoke is claimed.

Result: all target-backed raw accesses in the bounded control/presentation
range are routed through asserted owners.  The semantic router falls from
1,295 to 1,185 raw-member candidates and anonymous identifiers from 600 to
593; opaque-storage candidates rise from 70 to 73 solely because the broad
unknown span was split into three explicit retained gaps.  These are routing
aids, not completion percentages.  The next coherent Enemy/ECL family is the
trail/effect/death tail at `+0x534C..+0x53CC`, followed by the EnemyManager
pool/list owners needed to close the orchestration milestone.

### Enemy trail and attached-effect tail — 2026-08-26

Scope: `EclManager::RunEcl @ 0x004184B0`,
`Enemy::ReleaseAttachedEffects @ 0x0042A820`, parent-damage and phase/death
paths at `0x0042B370/0x0042B490/0x0042BCF0`,
`EnemyManager::OnUpdate @ 0x0042C660`, alignment/effect update at
`0x0042C420/0x0042E010`, `EnemyManager::OnDrawImpl @ 0x0042E140`, the score-
drop sweep at `0x0042EFB0`, and all linked-child/opcode users of
`Enemy + 0x3394..+0x53CC`.  The adjacent shared `Effect + 0x314/+0x318/
+0x350/+0x352` fields are included because they are the concrete objects
owned by this Enemy tail.

Observed: the 96-element `EnemyTrailSample` history stores world position,
velocity, and movement angle at each frame.  `trailHistoryLength @ +0x534E`
controls history shifting, culling, drawing, and score-drop traversal;
`trailCollisionLength @ +0x5350` bounds secondary collision sampling, and
`trailSampleStride @ +0x5352` controls render sampling and strip allocation.
The flag byte at `+0x534C` enables the trail and independently selects taper,
alpha fade, strip rendering, and head-ANM suppression.

`damageReductionTimer @ +0x5354` is set by ECL opcode 160, decremented by the
Enemy update, and reduces Boss damage to one ninth while suppressing ordinary
Enemy damage.  `attachedEffects[24] @ +0x5360`, count at `+0x53C0`, and target
distance at `+0x53C4` own ECL opcode 128's orbit-effect family.  Their
`Effect::vector5/vector6` members are the target-observed center and axis
storage; the shared `Effect::radius/angle` fields drive growth and rotation.
`alignmentEffect @ +0x53C8` follows world position and receives human/youkai,
collision, and death interrupts.  `phaseEndTimeRemainingSeconds @ +0x53CC`
records `(timer threshold - Boss timer) / 60` on either a phase callback or
death.

Corroborated: TH06 preserves the earlier `effectArray`, effect index, and
effect-distance sequence and the same grow-toward-distance update.  TH08
target producers and consumers independently establish the larger 24-pointer
array, every trail field, the damage timer, alignment effect, and phase-end
value; no TH06 offset or array extent is imported.

Inference and unknowns: `phaseEndTimeRemainingSeconds` is a producer-backed
behavior name because no current authored TH08 reader is present.  The one
padding byte at `+0x534D`, unobserved trail-flag bits, generic meanings of
`Effect::vector5/vector6` outside this attached-effect protocol, and the
remaining Effect tail bytes stay unknown.  Splitting the Enemy and Effect
opaque spans accounts for the routing increase in opaque-storage candidates;
unknown byte coverage did not grow.

Layout: assertions pin all three `EnemyTrailSample` members and its `0x1C`
extent, both arrays, every tail scalar/pointer, the selected Effect fields and
unchanged `sizeof(Effect) == 0x360`, and unchanged
`sizeof(Enemy) == 0x53D0`.  The formerly anonymous trail-record constructor at
`0x0042A490` is now tracked as `EnemyTrailSample::EnemyTrailSample`.

VC7 source-shape note: ECL opcodes 80/81 must assign the semantic
`AnmVm::flag17` bitfield.  Treating the overlapping `flags` member as a
16-bit mask enlarged exact `RunEcl` by four bytes; the bitfield expression
emits the target dword mask operation.  All other direct field and array
expressions in this batch reproduced the accepted bytes naturally.

VC7 oracle: focused replay passed **93 / 93** accepted units across
`EclRun`, `EffectManager`, `EnemyManager`, and `EnemyManagerUpdate`.
Target-pinned packets for `RunEcl`, `ReleaseAttachedEffects`, `OnUpdate`, and
`OnDrawImpl` independently replayed exact.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated trail/effect
gameplay smoke exists, so no runtime smoke is claimed.

Result: target-backed raw Enemy-tail accesses are replaced by asserted owners,
and the Effect radius/angle/release state used by that protocol is exposed.
The semantic router falls from 1,185 to 1,127 raw-member candidates and
anonymous identifiers from 593 to 592; opaque-storage candidates rise from
73 to 75 because two broad ranges became narrower explicit gaps.  These are
routing aids, not completion percentages.  The EnemyManager pool/list and
global update-state fields are the remaining coherent family before declaring
Enemy/ECL orchestration closure.

### EnemyManager pool and orchestration state — 2026-08-26

Scope: `EnemyManager::Initialize @ 0x00429E00`, both spawn paths at
`0x0042A4E0/0x0042A680`, `EclTimeline::Run @ 0x0042A8A0`,
`EnemyManager::OnUpdate @ 0x0042C660`, `OnDrawImpl @ 0x0042E140`, the
added/deleted callbacks and non-Boss cleanup at `0x0042EBF0/0x0042EE80/
0x0042EFB0`, the complete `EclManager::RunEcl @ 0x004184B0`, and every
Player, Bomb, Spellcard, Effect, GUI, helper, and dependency user of
`EnemyManager + 0x000000..+0x9DCF0F`.

Observed: `g_EnemyManager @ 0x00577F20` owns one spawn template at `+0`, one
nontrivial `Enemy enemies[481]` array at `+0x53D0`, and the orchestration tail
beginning at `+0x9DCDA0`.  Spawn scans only elements 0..479; element 480 is
the target-returned failure sentinel.  The 481-element array deliberately
remains intact because its VC7 vector construction is target-visible.

The tail contains eight Boss pointers, the two 16-bit Enemy drop scheduler
values, active-Enemy count, ECL opcode 163 state, sixteen timeline lanes, the
manager timer, four draw-list heads, the normal and alternate Enemy ANM banks,
the last-spawn-failed flag, four timeline event slots, and the timeline-spawn
suppression flag.  Producers and consumers establish the distinct integer,
pointer, timer, and array widths; this is not one homogeneous pointer table.

The earlier analysis identity `g_EclEnemyTableF54CC0 @ 0x00F54CC0` is now
retired.  That address is exactly `g_EnemyManager + 0x9DCDA0`: its indices
0..7 happened to name the Boss pointers, but index 11 reached opcode-163
state, indices 87..90 reached the four timeline event integers, and index 91
reached timeline-spawn suppression.  The old declaration therefore described
an overlapping analysis view, not a standalone production array.  The former
drop-counter globals at `0x00F54CE0/0x00F54CE2` are likewise adjacent members
of the same manager object.  Their standalone storage declarations, Linux
fixed aliases, global-ledger rows, and relocation identities are removed;
accepted relocations now name `g_EnemyManager` with the target field addend.

Inference and unknowns: `opcode163Value` is intentionally named for its only
proven producer/consumer protocol rather than a guessed stage-specific role.
The four bytes at `+0x9DCDC8` and `+0x9DCEF4` remain unknown.  The failure and
suppression names are behavior-backed by all spawn/child and timeline gates;
no narrower game-design meaning is asserted.

Layout: assertions pin the spawn template, pool, every tail owner, and the
unchanged `sizeof(EnemyManager) == 0x9DCF10`.  Three high-confidence anonymous
methods are now tracked as `UpdateSubrank @ 0x0042C3B0`,
`KillAllNonBossEnemies @ 0x0042EFB0`, and `HasBoss @ 0x0042F1F0`.

VC7 source-shape note: remote Boss-register opcodes retain target-observed
repeated selector resolution; no Boss pointer is cached across those reads.
Direct manager-member expressions otherwise reproduce the original
instructions naturally.  During migration, the first focused comparison
correctly rejected relocation metadata that combined the new manager symbol
with the old field address as its base; rebasing those entries to
`g_EnemyManager @ 0x00577F20` restored the field addend and exact target
resolution without masking a byte difference.

VC7 oracle: post-rename focused replay passed **316 / 316** accepted units.
Target-pinned packets for `Initialize`, `OnUpdate`, `OnDrawImpl`, both spawn
paths, `EclTimeline::Run`, and `RunEcl` independently replayed exact.  The
required non-reuse `verify-exact-units.py --all --json` cold-built all 75
configured objects and passed **1,105 / 1,105** with no failures.  A
subsequent normal VC7 production image linked successfully.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated EnemyManager
gameplay smoke exists, so no runtime smoke is claimed.

Result: the semantic router falls from 1,127 to 1,091 raw-member candidates;
absolute-address, anonymous-identifier, and opaque-storage candidates remain
82, 592, and 75.  These are routing aids, not completion percentages.  This
formally closes the **Enemy/ECL orchestration semantic milestone**: combat,
phase callbacks, interpreter context, motion, control/presentation, trail and
attached effects, spawn/pool ownership, Boss routing, timelines, and manager
update/draw state are typed and dual-oracle locked.  The claim is limited to
those recovered families and does not imply whole-program semantic
completion.  The next high-value milestone should recover the still-dense
Player core state and its adjacent PlayerBomb protocol.

### Player movement, collision, options, and shooting — 2026-08-26

Scope: `Player::CheckBulletCancelCollision @ 0x00449FF0`, the lethal and
graze collision family at `0x0044A230..0x0044A930`,
`UpdateMovementAndOptions @ 0x0044AEC0`, `AngleToPoint @ 0x0044C1B0`, Player
initialization/update/draw, `SpawnShots @ 0x00450F60`,
`UpdateShooting @ 0x00451500`, `StartShooting @ 0x00451640`, and all linked
Bullet, Enemy, Item, ECL, option, and Bomb consumers of the recovered fields.

Observed: `positionHistory[16] @ +0x2CC` is initialized from Player position,
shifted only on movement, and consumed by the route-3 trailing options.  The
range `+0x38C..+0x3F7` contains hurtbox, graze, and item-collection AABB
minimum/maximum pairs followed by their three half-sizes.  Initialization reads
the corresponding SHT sizes, while every movement frame recomputes all six
bounds from `position` and the half-sizes.

`velocity @ +0x3F8` is the time-scaled per-frame displacement added to Player
position.  `horizontalSpeedMultiplier/verticalSpeedMultiplier @ +0x404/+0x408`
are independently written by Bomb callbacks and multiply the selected SHT
speed before animation and movement.  `currentHorizontalSpeed/currentVerticalSpeed
@ +0xE2A9C/+0xE2AA0` retain the unscaled result for facing-animation
transitions.  The nine-value `PlayerMovementDirection` enum covers none, four
cardinal, and four diagonal input states without changing its dword ABI.

The focus path now owns `focusTransitionFrames @ +0x8`, typed option update and
render callbacks, `focusEffect @ +0xBE834`, and
`extremeGaugeEffect @ +0xE2B24`.  Both effects share the target-observed VM
flags at `+0x1F8`; the extreme-gauge effect also exposes position at `+0x2A4`
and active state at `+0x350`.  `shotTimer @ +0xE2AC4` starts at zero on fire
input, drives shot spawning for twenty frames, and returns to `-1`; option and
shot callbacks use the same owner to gate firing behavior.

The loaded `PlayerRawShtFile` header is pinned through `+0x33`: initial Bomb
count at `+0x4`, deathbomb window at `+0x8`, hurtbox/graze sizes at
`+0xC/+0x10`, item autocollect speed at `+0x14`, item-collection size at
`+0x18`, point-item value line at `+0x1C`, an unknown dword at `+0x20`, and
normal/focused axis and diagonal speeds at `+0x24..+0x30`.  The first typed
layout hypothesis placed the point line at `+0x20`; strict ItemManager replay
rejected three units with only the field displacement differing.  Restoring
the target `+0x1C` owner and retaining `+0x20` as unknown returned all three to
exact, so the correction is compiler- and consumer-backed rather than an
upstream-name assumption.

Names: the collision family is tracked as `CheckBulletCancelCollision`,
`CheckBulletCollision`, `CheckLethalCollision`, `CheckGrazeCollision`, and
`AwardGraze`; the adjacent core helpers are `UpdateMovementAndOptions`,
`AngleToPoint`, `SpawnShots`, `UpdateShooting`, `StartShooting`, and
`IsBombShotSuppressed`.  Each mapping, accepted match unit, caller relocation,
and decorated COFF symbol was updated together.

Inference and unknowns: `position2 @ +0x2C0`, `velocity.z`, the detailed roles
of `timerE2AD0/timerE2ADC/timerE2AE8`, option state/substate values outside the
observed transitions, SHT `+0x20`, and most PlayerShot/PlayerBomb work-item
internals remain deliberately neutral.  The effect type name describes the
shared target layout used here; it does not assert that all EffectManager
objects use these fields identically.

Layout: assertions pin the complete SHT prefix, callback widths, effect fields,
all movement/collision members, both effect pointers, both current speeds,
`shotTimer`, and unchanged `sizeof(Player) == 0xE2B30`.

VC7 source-shape note: the collision range-test negations, nested graze reward
ternary, laser-overlap gates, and movement switch lexical order remain intact.
Direct named member expressions reproduced the accepted instructions.  The
shooting timer remains lexically repeated rather than cached in a pointer local,
preserving the target hidden-`this` stack ownership.

VC7 oracle: focused replay across the nine affected objects passed **242 /
242** accepted units.  Target-pinned packets for all eleven renamed helpers
independently replayed exact.  The required non-reuse
`verify-exact-units.py --all --json` cold-built all 75 configured objects and
passed **1,105 / 1,105** with no failures.  A subsequent normal VC7 production
image linked successfully.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  No isolated automated Player gameplay
smoke exists, so no runtime smoke is claimed.

Result: the semantic router falls from 1,091 to 850 raw-member candidates and
opaque-storage candidates from 75 to 73; absolute-address and anonymous-
identifier candidates remain 82 and 592.  Within `Player.cpp`, `Player.hpp`,
and `PlayerBomb.cpp`, raw-member candidates fall from 555 to 319.  These are
routing aids, not completion percentages.  The next coherent batch is the
PlayerShot runtime/SHT descriptor family, followed by the PlayerBomb work-item
protocol; whole-program semantic completion remains open.

### PlayerShot runtime and SHT descriptors — 2026-08-26

Scope: `Player::LoadShtFile @ 0x0044DD70`, the descriptor-driven spawn family
at `0x0044FB70..0x00450240`, shot update/draw/hit callbacks at
`0x00450320..0x00450EE0`, `SpawnShots @ 0x00450F60`, the runtime controller at
`0x00451150..0x004514F1`, and `CalcDamageToEnemy @ 0x00451670` together with
its EnemyManager callers.

Observed: the variable SHT header contains a count at `+0x2` and an array of
eight-byte power-level rows at `+0x38`; each row owns a descriptor pointer and
minimum-power threshold.  Loader relocation proves every shot descriptor is
`0x38` bytes.  Its fields are the fire interval/frame, position offset,
hitbox size, angle, speed, damage, extreme-gauge behavior, source option,
shot type, animation, sound, and spawn/update/draw/collision callbacks at
`+0x28/+0x2C/+0x30/+0x34`.

The runtime `PlayerShot` remains `0x484` bytes and now exposes its VM, position,
32-entry position history, hitbox, velocity, auxiliary value, speed, angle,
timer, damage/state/type, persistent-timeline metadata, option-mode snapshot,
animation/tint state, three runtime callbacks, and originating descriptor.
The owning Player now names the four persistent-descriptor slots at
`+0xE2A80`, the shared shot-hit effect counter at `+0xE2A94`, and the base
shot angle at `+0xE2B0C`.  Layout assertions pin all descriptor and runtime
callback offsets without changing `sizeof(Player) == 0xE2B30`.

Names: the member helpers are `InitializeShot`, `SpawnShotOnSchedule`,
`SpawnShotOnScheduleUnlessBombing`, `SpawnPersistentShot`,
`SpawnShotAimedAtTrackedPoint`, `UpdateShots`, `DrawActiveShots`,
`DrawHitShots`, and `CalcDamageToEnemy`.  Descriptor callbacks use
behavior-backed names for player/option-relative, randomized, and homing
spawns; homing/falling/persistent/trail updates; trail drawing; and two hit
behaviors.  All mapping, implementation, accepted-match, match-unit, decorated
COFF, table-relocation, and EnemyManager caller identities moved together.

VC7 source-shape evidence: typed `PlayerShot *` iteration and direct named
fields preserve the accepted loop bodies.  `vm.zWriteDisabled = 1` is the
natural typed expression that retains the target dword `OR 0x2000`; assigning
through the public 16-bit aggregate `flags` instead emitted two extra bytes.
The tint selector is target-observed as signed `i8`: an unsigned byte changed
the target `movsx` to `movzx`.  The SHT loader also requires its renamed
descriptor local in `#pragma var_order` to retain the original stack homes.

Inference and unknowns: the first SHT header word, header `+0x20/+0x34`,
`PlayerShot::auxiliaryValue @ +0x448`, the unconsumed byte at `+0x46D`, and
three bytes at `+0x471` remain deliberately neutral.  The descriptor
`extremeGaugeBehavior` name records its two proven consumers—positive values
enable youkai tint and negative values enable human item drops—but does not
assert a serialized enum.  `velocity.z` is copied from the descriptor offset
and used as the persistent-shot horizontal increment; no broader 3D motion
claim is made.

Oracle status at the focused checkpoint: all 22 directly edited Player/SHT
units replay exact, including the `1,646`-byte damage function; the complete
Player plus EnemyManagerUpdate object set passes **52 / 52** accepted units.
The final single-job cold VC7 replay passes **1,105 / 1,105** accepted units,
the normal VC7 image links, and the complete i386 Linux build plus fixed-layout
verifier passes.  During that replay, using `Float3` directly for shot velocity
was rejected because its implicit constructor enlarged `PlayerShot::PlayerShot`;
the accepted layout therefore uses a named three-float POD and an explicit
`Float3` method view at the behavior call sites.

Result: the heuristic whole-source router falls from 850 to 779 raw-member
candidates and from 73 to 66 opaque-storage candidates; anonymous identifiers
rise from 592 to 595 because three byte ranges remain explicitly unknown, and
absolute-address candidates remain 82.  In `Player.cpp`/`Player.hpp`, the raw
candidate count falls from 151 to 80 and opaque storage from 18 to 11.  These
are routing aids, not completion percentages.  The next coherent family is the
PlayerBomb work-item protocol.
