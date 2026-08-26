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
`callbackVariant`, `duration`, `bombsConsumed`, `updateCallbacks`,
`drawCallbacks`, and `deathbombEffectVm` are high-confidence dataflow names.
`bombsConsumed` is supported by all three writes but has no independent read in
the authored source, so its downstream purpose remains unknown.  The later
PlayerBomb callback reconstruction identifies slots 0..4 as primary,
secondary, their two deathbomb variants, and the special callback.  The
`PlayerBombState + 0xC` and `Player + 0xE2B20/+0xE2B24` fields remain
deliberately unnamed.

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

Scope: the transform dispatcher at `Bullet::AdvanceTransformProgram @ 0x0042FFC0`, the
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

### PlayerBomb callback and work-item protocol — 2026-08-26

Scope: the complete Bomb callback cluster at `0x0040BC60..0x004142C0`, its
24-row callback table at `0x004C7AD0`, the shared `PlayerBombState` and
`PlayerBombWorkItem` layouts, and the three timer predicates used by these
state machines and adjacent Enemy/ECL/Player consumers.

Observed: `Player::AddedCallback` selects two adjacent callback rows with
`shotType * 2`; the first is copied to `updateCallbacks` and the second to
`drawCallbacks`.  Each row contains five function pointers selected by
`callbackVariant`.  Bomb acceptance establishes variants 0/1 as the primary
and secondary Bombs, adds two for the corresponding deathbomb callbacks, and
uses variant 4 for the special path.  The 16 update callbacks independently
embed their Shift-JIS cut-in names, directly identifying Fantasy Orb, Fantasy
Seal -Blink-, Quadruple Barrier, Eternal Night Quadruple Barrier, Master
Spark, Artful Sacrifice, Final Spark, Return Inanimateness, Killing Doll,
Night Mist Phantom Killer, Red the Nightless Castle, Scarlet Devil, Slash of
the Present World, Slash of Future Eternity, Ghastly Dream, and Eternal Sleep
in Dreamland.  Their paired draw callbacks and table relocations establish the
remaining names without relying on adjacent-version labels.

The `0x16F0`-byte work item now pins `state/stateTimer @ +0x0/+0x4`,
`motionStep/speed/angle @ +0x8/+0xC/+0x10`, current `position @ +0x14`, 32
path points at `+0x20`, generic motion vectors at `+0x1A0/+0x1AC`, eight VMs
at `+0x1B8`, the effect VM at `+0x16D8`, its timer at `+0x16DC`, and damage
and cancellation regions at `+0x16E8/+0x16EC`.  The generic `motion` and
`auxiliaryMotion` names are deliberate because different character callbacks
reuse those vectors as velocity, interpolation endpoints, or radial state.
`stateTimer` is proven by its reset and 30-frame finishing transition; the
three-state enum records only the observed inactive, active, and finishing
values.

The callback aggregate is now `PlayerBombCallbackSet`, and
`g_PlayerBombCallbacksByShotType` makes the shot-type ownership explicit.
`ZunTimer::HasTicked`, `JustReached`, and `IsPeriodic` state their exact
predicates: `current != previous`, the same gate plus equality to one frame,
and the same gate plus a zero interval remainder.  TH06's independently named
`HasTicked` is corroboration only; all three TH08 bodies and call boundaries
remain target-attested.

Unknowns: `PlayerBombState +0xC` remains opaque, and the downstream purpose of
its saved tail position is not yet visible in authored source.  Effect fields
accessed through an `AnmVm *` at `+0x2A4..+0x35C` belong to the adjacent shared
Effect/ANM semantic batch and are not guessed here.

Oracle status at the focused checkpoint: accepted replay passes **59 / 59**
for PlayerBomb, **77 / 77** for Player, **39 / 39** for EnemyManager, and
**33 / 33** for the ECL extension object.  The renamed table relocations,
timer call relocations, and all spell callbacks remain exact.  The final cold
aggregate passes **1,105 / 1,105** in a single-job cold VC7 replay, the normal
VC7 image links, and the complete i386 Linux build and fixed-layout verifier
pass.

Result: the PlayerBomb/Player header router falls from 168 to 160 raw-member
candidates and from 11 to 10 opaque-storage candidates.  The principal gain
is that all 24 callback rows and their 32 distinct update/draw functions are
now readable by gameplay role instead of address.  These counts remain routing
aids, not completion percentages.  The next coherent family is the Effect and
ANM interpolation state consumed by these callbacks.

### Effect and ANM interpolation state — 2026-08-26

Scope: `EffectManager @ 0x004ECE60`, its 654 pooled `Effect` rows and five
draw-list sentinels, all accepted Effect callbacks at
`0x004253E0..0x00428880`, the Player/PlayerBomb Effect callbacks, and the four
ANM interpolation setters at `0x0040EC30..0x0040EE10`.

Observed: every Effect is exactly `0x360` bytes.  Its leading `AnmVm` ends at
`+0x2A4`; the target then constructs nine distinct `Float3` members through
`+0x310`, followed by primary radius/angle, strip thickness and segment count,
pool slot index, secondary radius/angle, radial wave count, timer, update/draw
callbacks, eight signed lifecycle/draw bytes, a vertex buffer pointer, and a
draw-list link.  Target `movsx` consumers prove that release request/timer,
alternate draw group, dirty flag, and update-during-freeze state are signed
bytes.  Player and ECL callers now use the same asserted owner instead of
treating an allocated Effect as an open-ended `AnmVm`.

`EffectManager` is exactly `0x8B05C` bytes.  The pool begins at `+0x1C`; the
five sentinels begin at `+0x89F5C` and are independent objects, followed by
five draw-list tails, the tamper-check counter, and the two loaded ANM banks.
Constructor lowering independently confirms this layout: one vector
constructor covers the 654 pool rows, while each sentinel receives a separate
constructor call.

The four ANM helpers are now `StartPositionInterpolation`,
`StartColor1RgbInterpolation`, `StartColor1AlphaInterpolation`, and
`StartScaleInterpolation`.  Their two timer arrays, mode-byte array, and typed
initial/final payloads are shared by ECL, Player, and Bomb callers.  The
dissolve ring's target byte 5 is `EaseOutCubic`; the deathbomb ring's byte 4 is
`EaseOut`.  RGB channel unpacking follows the target `ZunColor` byte order and
is not replaced with a packed assignment.

VC7 source-shape limits: the `+0x304` member must remain a `Float3` axis plus a
separate `orientationW`, not `D3DXQUATERNION`, because the former's constructor
relocation is target-visible.  The five manager sentinels cannot be collapsed
into an array for the same reason.  The two fixed-slot spawners retain an
explicit `sizeof(Effect)`/`offsetof(EffectManager, effects)` address expression;
natural typed array indexing is semantically equivalent but two bytes shorter
under VC7 `/Od`.

Unknowns: `vector1..vector7` are deliberately generic because effect kinds
reuse them for velocities, interpolation endpoints, camera-relative anchors,
and trail state.  The dword at `+0x31C`, dword at `+0x344`, callback signatures,
and exact gameplay role of the four orientation components remain unclaimed.
The compatibility-facing callbacks therefore retain their existing `AnmVm *`
ABI and use an explicit `Effect` view internally.

Oracle status: focused replay passes Player/PlayerBomb **136 / 136**, ECL,
EnemyManagerUpdate, and EffectManager **86 / 86**, including all four renamed
setter bodies and caller relocations.  A single-job non-reuse cold VC7 replay
passes **1,105 / 1,105**, the normal VC7 image links, and the complete i386
Linux build plus fixed-layout verifier passes.  `PlayerBomb.cpp` has no
remaining Effect-tail raw access; its sole raw-member candidate is the
separate Background tint flag.  The nine EffectManager candidates are Replay,
Supervisor, and the two intentional exact-source fixed-slot expressions, not
untyped Effect fields.  These are routing observations, not completion
percentages.  The next high-value family is the remaining Player core state,
followed by the Background camera/stage object model.

### Remaining Player core state and option protocol — 2026-08-26

Scope: the Player state-machine functions at `0x0044C5B0..0x0044D52A`, the
two collision-region pools and their allocators at
`0x0044DE60..0x0044E393`, the option callbacks at
`0x0044E3A0..0x0044FB6A`, and the adjacent GameManager frame statistics read
by Player and GUI.

Observed: the contiguous 384 `0x40`-byte records split into two 192-entry
pools.  One pool is consumed as per-Enemy Player-shot damage and owns the
`damage` field; the other is consumed by bullet cancellation and owns
`collisionValue`.  Each has rectangular and circular producers, distinguished
by size versus radius/radius-growth writes.  They are therefore
`damageRegions` and `cancelRegions` of `PlayerCollisionRegion`, with
`CreateRectDamageRegion`, `CreateCircleDamageRegion`,
`CreateRectCancelRegion`, and `CreateCircleCancelRegion` as their four
allocation paths.  PlayerBomb work-item pointers and every direct Enemy caller
now share that owner.

The state controller is named from its complete behavior rather than its old
addresses: `UpdateCollisionRegions`, `UpdateBombState`,
`UpdateDeathAndRespawn`, `UpdateRespawnAnimation`, `UpdateInvulnerability`,
and `UpdateGaugePosition`.  State 3 is `PLAYER_STATE_INVULNERABLE`: entry
follows respawn, it suppresses lethal collision, counts down, flashes the
Player VM, follows the state Effect, and returns to `ALIVE`.  The deathbomb
pending flag remains unsigned because the target reads it with `movzx`; an
initial signed-byte declaration changed two instructions and was rejected.

The option callback protocol now uniformly takes `PlayerOptionState *`.
`UpdateOptionHomingToPlayer` and `UpdateOptionHomingToTarget` use typed
position, velocity, timer, substate, and VM members, while `DrawPlayerOption`
uses the embedded VM and world position directly.  The table callbacks are
named conservatively as homing, Bomb-anchor, orbiting, mode-sensitive orbiting,
facing-trail, mode-sensitive facing, and twin-orbiting controllers; those
labels describe their proven motion/control behavior without assigning a
character or shot identity to table rows that are shared.  The uniform type
removes all callback casts without changing the fastcall ABI.

Adjacent owners: `AnmVmBase` now asserts scale at `+0x18`, colors at
`+0x1F0/+0x1F4`, the complete flags word at `+0x1F8`, and `AnmVm::pos` at
`+0x208`.  `GameManager` names the run/stage active-frame counters and the
parallel run/stage extremely-youkai/extremely-human frame counters through
`+0x3DE24`; setup, Player, and GUI consumers agree on those roles.

Oracle status: focused replay across Player, PlayerBomb, EnemyManager,
Spellcard, GameManager, GameManagerSetup, and GUI passes **258 / 258** accepted
units.  The single-job non-reuse cold VC7 replay passes **1,105 / 1,105**, the
normal VC7 production image links, and the complete i386 Linux build plus
fixed-layout verifier passes.  Source/header/config COFF identities and unit
names moved together; no authored or accepted-exact ledger row changed.

Result: the whole-source router reports 345 raw-member, 82 absolute-address,
570 anonymous-identifier, and 56 opaque-storage candidates.  In
`Player.cpp`/`Player.hpp`/`PlayerBomb.cpp`, only one raw-member candidate
remains: the separate Background tint byte.  The remaining Player candidates
are explicit unknown serialized fields, global-address owners not yet
recovered, and neutral callback operands.  These counts are routing aids, not
completion percentages.  The next high-value family is the Background camera,
stage-object, and spell-tint model.

### Background camera, stage, and spell-background model — 2026-08-26

Scope: `Background @ 0x004E4030`, its constructor and camera constructor at
`0x004071A0/0x004073B0`, update/draw lifecycle at
`0x00407400..0x0040984F`, stage loading and rendering at
`0x00409CE0..0x0040B42F`, and the spell-background transitions at
`0x00415CE0/0x00416AD0`.

Observed: the `0x6600`-byte owner now exposes the loaded stage buffer, object
and quad counts, object-offset and instance tables, stage instruction stream,
instruction timer/index, stage position interpolation, clear color, stage
text VM, stage Effect, sky-fog interpolation, spell VM bank, draw callback,
and the stage-script label selected by ECL opcode 147.  File records are
bounded as `RawStageHeader`, `RawStageObject`, `RawStageObjectInstance`,
`RawStageQuadBasic/Type1`, and `RawStageInstr`; neutral names are retained where
the serialized role is not proven.

The five `0x4C` camera records are now `BackgroundCamera`: target,
interpolation start, Hermite end/start tangents, and current.  Each record owns
position, look-at offset, up/forward/right vectors, position offset, and field
of view.  Target instructions in `OnUpdate`, `SetCamera1/2`, stage rendering,
ANM projection, and Effect camera-relative callbacks establish the individual
roles.  Five parallel duration/timer/mode lanes control position, look-at, up,
field-of-view, and camera-motion interpolation.  The scalar interpolator at
`0x00408FC0` has target-observed `__stdcall` cleanup of 20 bytes and is named
`CubicHermiteInterpolate`; the vector dispatcher is
`InterpolateCameraVector`.

Spell background state values 0, 1, and 2 are named inactive, fading in, and
active.  Tint accumulation, the one-frame retain flag, culling distance, camera
motion mode, and the 32 special-effect points now share asserted fields across
Background, PlayerBomb, Spellcard, ANM, Effect, GameManager, and GUI.  The old
`g_EclGlobal004EA290` was an overlapping analysis alias: target address
`0x004EA290` is exactly `g_Background + 0x6260`, so RunEcl now writes
`pendingStageScriptLabel` and the duplicate VC7/Linux storage identity is
retired.

VC7 source-shape limit: the stage-7 check at target `OnUpdate + 0x4D` retains
an explicitly documented absolute view of `0x0164D2CC`, even though that
address is `g_GameManager.currentStage`.  A natural member expression changes
the COFF relocation shape; restoring the target operand is required for strict
accepted replay.  Snapshotting the stage Effect VM also retains a raw
`BackgroundAnmVmSnapshot` carrier because adding convenient typed local
pointers changes `/Od` stack/code shape.  These are localized compiler-shape
constraints, not unknown field ownership.

Oracle status: focused `Background.obj` replay passes **29 / 29** and all
selected affected consumer objects pass **487 / 487**.  RunEcl remains
**26,638 / 26,638** authored bytes exact after its relocation owner changes to
`g_Background + 0x6260`.  The non-reuse single-job cold VC7 replay passes
**1,105 / 1,105**, the normal VC7 image links, and the complete i386 Linux
build plus fixed-layout verifier passes.

Result: the whole-source router reports 207 raw-member, 82 absolute-address,
432 anonymous-identifier, and 54 opaque-storage candidates.  Background source
and header have zero raw-member candidates; the one PlayerBomb Background tint
candidate is also gone.  Counts remain routing aids, not completion
percentages.  The next largest coherent semantic owner is GameManager and its
GUI/setup consumers.

### GameManager core runtime and setup state — 2026-08-26

Scope: `GameManager::OnUpdate @ 0x00439BC7`, the target-pinned setup callback
at `0x0043ABD7`, score-data initialization at `0x0043BBE1`, and the Replay,
Player, GUI, Background, Enemy, Effect, Spellcard, ResultScreen, Ending, and
Supervisor consumers of the same state.  The two primary typed-RE packets are
`build/typed-re-00439bc7.json` and `build/typed-re-0043abd7.json`; both attest
the canonical 1.00d image and exact target extents.

Observed: setup state `0/1/2` is complete/in-progress/failed, while the
stage-start music selector `0/1/2` is none/play/without-music.  The owner now
names the setup wait count, skip-current-frame latch, playtime and gameplay
frame counters, replay pause recording, the next Supervisor state, run
humanity-rate numerator/denominator, character-list index, stage RNG seed,
stage-at-start, current-stage clear bit, aggregate stage playtime, frame-skip
counter, and the startup selector.  Flag roles shared across update and setup
are replay-input enabled, game cleared, stage-transition state,
deathbomb-freeze active, final-stage route, and player-shot suppression.

Setup's former raw global/config views are now asserted `ZunGlobals` and
`GameConfiguration` fields.  A layout assertion rejected an initially
plausible bomb-field interpretation: target offsets `+0x80`, `+0x84`, and
`+0x88` are `bombsRemaining`, `bombsUsed`, and `bombsUsedInStage`, not three
interchangeable counters.  `scoreDisplayStep @ +0x10`, life count, power,
score/high-score state, time orbs, graze, and retries likewise use common
owners throughout setup and normal gameplay.

The complete routine at `0x0043BBE1` initializes all CATK records, opens and
parses score.dat, selects the displayed high score, updates practice attempts,
copies the active CATK table, initializes the HSCR record, and resets run
playtime.  It is therefore named `InitializeScoreData`; the source symbol,
mapping, implemented/reccmp ledgers, accepted match row, relocation metadata,
and comparison-unit name moved together.

VC7 source-shape limit: packed `GameManagerFlags` writes in setup retain the
ownership-aware `GM_FLAGS_WORD(gameManager)` view.  Expressing each mask as
separate bitfield assignments changes the target-visible mask-operation
sequence even though the logical state is equivalent.  The stage RNG seed is
written as the observed low `u16` of `g_Rng`, with a layout assertion at
`GameManager + 0x3DDBC`.  This is typed ownership with a compiler-shape view,
not an unowned raw offset.

Unknowns: the bytes at `+0x3DBB6/+0x3DBB7`, the high halfword at `+0x3DDBE`,
and fields at `+0x3DDCC/+0x3DDD2/+0x3DE0C` remain neutral because their full
cross-state roles are not established.  They were not renamed from adjacency
alone.

Oracle status: focused `GameManager.obj` replay passes **42 / 42**, including
the complete 3,644-byte update, 3,423-byte setup callback, and 552-byte
`InitializeScoreData`.  Selected affected consumers pass **493 / 493**.  A
single-job non-reuse cold VC7 replay passes **1,105 / 1,105**, the normal VC7
production image links (including setup/score/option probes), and the complete
i386 Linux container build plus fixed-layout verifier passes.

Result: the whole-source router reports 201 raw-member, 82 absolute-address,
310 anonymous-identifier, and 54 opaque-storage candidates.  GameManager's
remaining seven anonymous fields are the explicit unknowns above; the raw
offset concentration now belongs to GUI.  Counts remain routing aids, not
completion percentages.  The next bounded owner is GUI boss/status/message
state.

### GUI message, Boss HUD, and stage-clear model — 2026-08-27

Scope: `GuiImpl @ g_Gui.impl`, message start/update/draw at
`0x0043396D..0x004358BB`, stage/HUD update and drawing at
`0x00435900..0x004390D6`, stage-clear initialization at `0x004396B8`, and the
Boss-gauge setters at `0x004230C0..0x00423130`.  The accepted callers span
ECL, GameManager, Enemy, Player, Item, Spellcard, Replay, Background, and the
portable renderer.

Observed: `GuiImpl` remains exactly `0x230B8` bytes.  Its sixteen front VMs,
Boss-life-bar presentation state, four stage-text VMs, clock/loading/capture
VMs, eight motion-blur VMs, spell-nullify VM, 168 transition VMs, stage-rank
and clock VMs, and active-transition count now have asserted owners.  The
message controller is exactly `0x1570` bytes at `GuiImpl + 0x21814`; portraits,
dialogue and intro lines, text/shadow colors, instruction/timer state, wait and
skip state, current portrait/line/color, textbox visibility, and selected
route are named through its last target-used byte.  This corrects the former
`0x1578` overlay: the following dwords at `+0x22D84/+0x22D88` are the
stage-clear screen state and clear-bonus total, not message fields.

The `+0x22DEC..+0x22E13` stage-clear owner now names stage, power, point-item,
graze, time-orb, clock-increment, and old/target/displayed clock values.  The
target's indexed dword read at VA `0x004C7158` proves the old
`i32 *[MAX_STAGES]` declaration was a type lie; it is now the integer array
`g_GuiStageClearBonuses`, with its COFF identity and Linux fixed-layout symbol
moved together.  `Gui::bossLifeBarTargetSize` is the ECL/Enemy-written value,
while `bossLifeBarDisplayedSize` is the interpolated value consumed by the
draw path.  Supervisor `+0x164/+0x168/+0x16C` are respectively initial-stage
load, release-on-restart, and keep-stage-resources flags across every producer
and consumer.

Behavior-backed names replace the public address labels for message reading,
stage-element update/draw, stage completion/skipping, bonus and status popups,
Boss timer/life/gauge/marker setters, enemy-name texture selection, stage-clear
draw and initialization, ASCII popup draw, arcade capture, and clock flashing
or hiding.  The source, mappings, implemented/reccmp ledgers, accepted rows,
and COFF relocation manifest moved together.  GensokyoClub's current GUI model
corroborated the layout and roles; acceptance remains the TH08 1.00d target
replay below.

Unknowns: the unused final message byte, `Gui + 0x38`, and the prefix of the
Background stage-data view used only for music paths remain explicit neutral
storage.  Opcode payloads are still byte-oriented in this batch; their widths
and behavior are visible, but promoting the serialized instruction union is a
separate source-shape change.

Oracle status: focused `Gui.obj` replay passes **41 / 41** and the selected
caller set passes **224 / 224**, including complete `RunEcl` at **26,638 /
26,638** bytes.  A single-job non-reuse cold VC7 replay of all 75 comparison
objects passes **1,106 / 1,106**.  The normal VC7 production image links, the
complete Linux container build links as ELF32/i386, and
`verify-modern-linux.sh` reports all fixed target-owned layout symbols OK.

Result: `Gui.cpp` has zero raw-member, absolute-address, or anonymous-identifier
candidates; its one opaque candidate is the retained Background-owned music
view.  The whole-source router now reports 116 raw-member, 82 absolute-address,
280 anonymous-identifier, and 49 opaque-storage candidates.  These are routing
observations, not completion percentages.  The next dense owners are the
remaining GameManager/Spellcard cross-state fields, followed by
EnemyManagerUpdate and AsciiManager presentation state.

### GUI serialized message instruction protocol — 2026-08-27

Scope: `GuiImpl::StartMessage @ 0x0043396D`, `RunMsg @ 0x00433DB3`, message
text decryption at `0x004353EC`, and `Gui::LoadMsg @ 0x00439710`.  Shared
wire-format declarations live in `src/Gui.hpp`; the interpreter remains in
`src/Gui.cpp` under its exact `/Os` profile.

Observed: the target dispatches the complete byte-opcode range 0 through 22
through a 23-entry jump table.  Each variable-size record starts with
`u16 time`, `u8 opcode`, and `u8 instructionSize`; the target advances from
the payload address by `instructionSize`.  Payload accesses independently
establish signed 16-bit portrait/line/color indices, unsigned one-byte ANM
interrupt and boolean controls, dword waits/music indices, a five-dword
all-portrait descriptor, a two-dword single-portrait descriptor, and encrypted
text beginning at payload offset zero or four.  `LoadMsg` reads a dword count
and fixes each following dword offset into an in-memory instruction pointer.
The decoder at `0x004353EC` XORs every byte, including the terminator, with
`0x77` and is now named `DecryptGuiMessageText` in source and all ledgers.

Corroborated: GensokyoClub's current `MsgOpcode`, payload union, raw record,
and file-header declarations agree with these TH08 widths and the interpreter's
behavior.  They supplied the naming hypothesis only; the final declarations
were checked against the canonical target packet and strict comparator rather
than copied as an exactness claim.

Inference: opcode names describe the target-observed action: portrait ANM or
sprite selection, dialogue/speaker/top/bottom text, waits and selection,
music/fade, stage result/end, screen fade, textbox/skipping controls, and
selected-message routing.  `GUI_MSG_RESUME_ECL` and the intro-text label retain
upstream corroboration because their target cases respectively adjust the
message wait gate and initialize the intro ANM without independently naming
the external protocol.  The unused trailing message byte and the opaque prefix
of Background stage music data remain outside this batch.

Layout: assertions pin every relied-on payload size/offset,
`GuiMessageInstruction::args @ +0x4`, its maximum compile-time extent of
`0x18`, `GuiMessageFile::messages @ +0x4`, and the unchanged
`sizeof(GuiMsgVm) == 0x1570`.  Comments explicitly preserve the variable-size
record and in-place pointer-fixup semantics instead of pretending the serialized
stream is a fixed C++ array.

VC7 oracle: focused replay across both `build/Gui.obj` and the canonical
transformed GUI object passes **41 / 41**.  `RunMsg` matches all **5,597 / 5,597**
authored bytes and the complete **5,689 / 5,689** code-plus-jump-table extent;
the renamed decoder is **63 / 63**, `StartMessage` **1,094 / 1,094**, and
`LoadMsg` **197 / 197**.  The required single-job non-reuse cold build of all
75 comparison objects passes **1,106 / 1,106 exact**, and the normal VC7 image
links.

Portable oracle: `scripts/build-modern-linux-container.sh` compiles and links
the complete portable target.  `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verifies ELF32/ET_EXEC/i386 and every
fixed target-owned layout symbol.

Result: all numeric message opcode cases, local raw-instruction facades, and
payload reinterpret casts in `RunMsg` are replaced by the asserted typed
protocol without changing instruction order, state transitions, or accepted
bytes.  This closes the deferred message-protocol portion of the GUI model; it
does not claim meanings for unconsumed payload bytes or whole-program semantic
completion.

### Spellcard effect and reward lifecycle — 2026-08-27

Scope: `Spellcard::Init @ 0x00414590`, `StartSpell @ 0x004152A0`,
`EndSpell @ 0x004161B0`, `OnUpdateImpl @ 0x00416B90`, and the ECL-facing
`SetStoredVector @ 0x0041F040`.  The owner boundary spans `Spellcard.hpp`,
`Spellcard.cpp`, and the one `EclManager.cpp` consumer.

Observed: `Spellcard +0xF4/+0xF8` are pointers to the existing asserted
`Effect` object, whose `AnmVm` prefix occupies `+0x000..+0x2A3`.  The target
uses the VM position, RGB1, and scale interpolation timer/mode banks; initial,
final, and current position/scale/color fields; active sprite and visibility
state; and the Effect position, tracking vector, radius, angle, thickness,
segment count, secondary radius, radial-wave count, lifetime timer, and active
byte.  `SetStoredVector` independently confirms the three dword writes to
`Effect::position @ +0x2A4/+0x2A8/+0x2AC`.

The transition from the active spell ring to its capture-reward effect is now
expressed through those owners: position and color interpolation are restarted,
the ring tracks the enemy while active and the player after capture, and its
timer drives scale changes, time-orb emission, and final release.  Target-
visible dword copies between float fields deliberately retain bit-copy source
shape instead of introducing x87 loads/stores.  The eight VM script resets in
`Init` use `currentInstruction` and `flagsWord` directly.  An unused local raw
effect overlay was removed; it had no consumers and supplied no independent
layout evidence.

Unknowns: `Spellcard::unknown_044` remains neutral storage because this batch
does not establish its contents.  No new interpretation is claimed for the
packed Spellcard flag bits outside their already-observed behavior.

VC7 oracle: focused `SpellCard.obj` replay passes **29 / 29**.  In particular,
`StartSpell` is **2,483 / 2,483**, `EndSpell` **2,331 / 2,331**, and
`OnUpdateImpl` **3,269 / 3,269**.  Focused `EclManager.obj` replay passes
**10 / 10**, including `SetStoredVector` at **67 / 67**.  The required
single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies ELF32/ET_EXEC/i386 plus every fixed
target-owned layout symbol.

Result: `Spellcard.cpp` now has zero raw-member, absolute-address, anonymous-
identifier, or opaque-storage candidates.  The sole Spellcard-header candidate
is the intentionally retained unknown range.  The whole-source router reports
87 raw-member, 82 absolute-address, 271 anonymous-identifier, and 49 opaque-
storage candidates.  These counts route the next bounded owner; they are not a
semantic-completion percentage.

### Enemy update and render ownership — 2026-08-27

Scope: `EnemyManager::OnUpdate @ 0x0042C660`,
`EnemyManager::OnDrawImpl @ 0x0042E140`, the phase-capture helper at
`0x0042BC50`, and the shared `Enemy` prefix consumed by the update and four
draw lists.

Observed: `Enemy +0x000` is the singly linked next pointer installed when an
active visible Enemy is assigned to one of the manager's four draw groups and
consumed by `OnDrawImpl`; it is now `Enemy::nextInDrawGroup`.  The primary VM
at `+0x00C` and two secondary VMs at `+0x2B0` are the existing asserted
`AnmVm` objects.  Their loaded-sprite pixel width/height, colors, visibility,
script, position, and auxiliary trail VMs replace the update TU's private raw
facades and every render-side offset.  The target's damage flash writes the
individual `color2` channels, while normal display tint uses `color1`.

The same update body now uses the shared owners for Enemy life, timers,
hitboxes, flags, phase callbacks, movement, effects, Boss state, Player-shot
damage, and draw-list publication.  Its external state reads are likewise
typed: total play time and humanity-rate counters, deathbomb freeze and shot-
suppression flags, Player Bomb activity, Player option mode, replay state, and
the Player state machine.  In particular the target access at `Player +0x000`
is `Player::playerState`, not the unrelated character-type concept from the
discarded local overlay.  The death path therefore starts the 90-frame
invulnerable state through the real owner.  The helper at `0x0042BC50` retains
its target ABI while expressing its three operations as Spellcard flag and
`bonusProgress` field accesses.

Unknowns: the absolute dword at `0x018B8A24`, sampled once at the start of the
update, has no independent global-owner or behavioral evidence and remains an
explicit address instead of receiving a speculative name.  The unused bytes
at `Enemy +0x004..+0x00B`, unobserved flag bits, and neutral ranges elsewhere
in `Enemy.hpp` remain unknown.  Existing method names and field roles outside
this bounded owner family are not strengthened by this batch.

VC7 oracle: focused `EnemyManagerUpdate.obj` replay matches the complete
`OnUpdate` body at **6,198 / 6,198 bytes**.  The canonical transformed
`EnemyManager.obj` replay passes **23 / 23**, including `OnDrawImpl` at
**2,504 / 2,504** and the phase-capture helper at **50 / 50**.  The production
object's configured subset also passes **16 / 16**.  Because `Enemy.hpp` is a
shared layout, the required single-job non-reuse cold build of all 75
comparison objects was run and passes **1,106 / 1,106 exact**; the normal VC7
production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies ELF32/ET_EXEC/i386 plus every fixed
target-owned layout symbol.

Result: `EnemyManagerUpdate.cpp` has zero candidates in every semantic-router
category.  Across the update source, draw source, and Enemy header, the only
non-header-layout candidate is the deliberately unresolved absolute dword.
The whole-source router reports 60 raw-member, 82 absolute-address, 255
anonymous-identifier, and 49 opaque-storage candidates.  These are routing
counts, not a whole-program completion claim.

### Ascii presentation and night-blindness protocol — 2026-08-27

Scope: the asserted `AsciiManager` presentation owner, its update and low/high
priority draw paths, Boss-marker helpers, the ECL extension callback at
`0x00423390`, and the shared effect-ANM resource used by Bullet, PlayerBomb,
Spellcard, ECL extensions, and the night-blindness renderer.

Observed: `AsciiManager +0x8284` advances once per manager update and supplies
the modulo-8/4/2 Boss-marker flicker phases; it is `frameTimer`.
`+0x2254` is the four-dword state array paired with the four Boss-marker VMs,
and the helper at `0x0042F2D0` is now `SetBossMarkerState`.  The helper at
`0x00422BB0` applies the marker VM interrupt and is
`SetBossMarkerInterrupt`.  The repeated VM execution body at `0x00406FD0` is
the real out-of-line `UpdateVms`, not a second anonymous operation.  The retry
path now reads the already-asserted `PlayerRawShtFile::initialBombCount`
instead of a local two-field overlay.  The cross-owner dword at
`Supervisor +0x174`, set around menu/stage transitions and decremented each
frame, is conservatively named `screenTransitionCountdown`; no consumer
effect beyond that observed lifecycle is claimed.

The Ascii tail is Mystia's night-blindness presentation protocol.
`+0x16F04` is the clear radius around Player position, `+0x16F08` is its
integer alpha parameter, and `+0x16F0C` is the script-105 center VM.  The draw
path shades the four regions outside the clear square, scales the center VM by
`radius / 63`, and applies the same low-byte alpha.  ECL callback
`ConfigureNightBlindness @ 0x00423390` publishes float variable 0 by an exact
dword bit copy and integer variable 0 as the alpha.  Game setup, Spellcard end,
and teardown reset that owner directly.

This also closes two overlapping-analysis aliases.  Absolute
`0x004E3D24/0x004E3D28` are `g_AsciiManager +0x16F04/+0x16F08`, not standalone
ECL globals.  Absolute `0x00577EB4` is
`g_EffectManager.effectAnm @ +0x8B054`, not an Ascii-owned ANM pointer.  All
source consumers, relocation identities, global ledgers, and Linux fixed-
layout aliases now use the true aggregate owners.  GensokyoClub's current
`frameTimer` and `nightBlindness*` declarations corroborate the behavioral
names, but TH08 target accesses and this repository's replay are the accepting
evidence.  In particular this source keeps the target-observed integer alpha
rather than importing upstream's `ZunColor` interpretation.

Unknowns: the VM at `AsciiManager +0x1520` is initialized with script 9 and
executed each frame, but this batch does not prove its presentation role.  The
bool at `+0x829C` is only reset by authored code, and
`AsciiManagerPopup +0x34` has no authored consumer.  All three retain neutral
names.  No semantic claim is made for unused Supervisor transition-countdown
values after they are decremented.

VC7 oracle: the rebuilt focused affected-object selection passes **353 / 353**
accepted units.  The Ascii object contributes **63 / 63**; important complete
bodies include `OnUpdate` **460 / 460**, `OnDrawLowPrioImpl`
**1,698 / 1,698**, `OnDrawHighPrioImpl` **2,538 / 2,538**, `UpdateVms`
**217 / 217**, `SetBossMarkerInterrupt` **42 / 42**, and
`SetBossMarkerState` **29 / 29**.  `ConfigureNightBlindness` is **52 / 52**,
and the separately regenerated canonical Ascii/Bullet/PlayerBomb/Spellcard
selection passes **72 / 72**.  The required single-job non-reuse cold build of
all 75 comparison objects passes **1,106 / 1,106 exact**, and the normal VC7
production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies ELF32/ET_EXEC/i386 plus every fixed
target-owned layout symbol after the three redundant aliases are removed.

Result: the Ascii source/header/marker-state router falls to five candidates,
all explicitly retained unknowns above.  The whole-source router reports 60
raw-member, 82 absolute-address, 218 anonymous-identifier, and 48 opaque-
storage candidates.  These counts select future work; they do not express a
semantic-completion percentage.

### GameManager music, setup, and scripted-freeze ownership — 2026-08-27

Scope: the spell-practice music helpers at `0x00439916/0x00439961`,
`GameManager::OnUpdate @ 0x00439BC7`, `GameplaySetupThread @ 0x0043ABD7`,
the ECL extension callback at `0x00425070`, and the Bullet, Enemy, GUI,
Player, Spellcard, ScreenEffect, Supervisor, ReplayManager, and Background
consumers of the same runtime state.

Observed: target data at `0x004C7670` is exactly nineteen 0x14-byte records:
a signed spell-number ceiling, song number, song-path pointer, song-name
sprite index, and dword pause policy, terminated by spell number `-1`.
`0x00439916` returns the pause policy and `0x00439961` returns the sprite
index from the first record whose ceiling contains the current spell.
`g_GuiStageMusicContexts @ 0x004C7240` is a nine-row, three-dword song-number
table.  The loaded STD header independently places four 128-byte song-name
rows at `+0x90` and four song-path rows at `+0x290`; setup and GUI playback
now share that asserted `RawStageHeader` rather than private offset views.

The byte at absolute `0x0160F534` is `g_GameManager + 0x2C`, not a standalone
ECL global.  `EclExIns::SetScriptedUpdateFreeze @ 0x00425070` copies the
signed instruction byte at `+0x10` into it and selects barrier interrupts 1
or 2.  Independent target users at Spellcard `0x00416B90`, Enemy update
`0x0042C660`, Bullet update `0x00431240`, GUI update `0x004338CA`, Player
death/update `0x0044AB40/0x0044C390`, and ScreenEffect
`0x0045BDC0/0x0045BF10` prove a scripted whole-gameplay update freeze.  Every
COFF relocation now names `g_GameManager` with addend `0x2C`; the redundant
global ledger row, storage definition, and Linux linker alias were removed.

Additional target ownership in `GameManager::OnUpdate` establishes
`Supervisor::viewport @ +0xC8`, replay-recorded FPS at `+0x198`, loading-VM
setup state at `+0x2FC`, `Background::skyFog.color @ +0xAF4`,
`BulletManager::activeBulletCount @ +0x6BA538`, and the RNG saved seed at
`g_Rng + 0x2`.  Setup dataflow names the random malloc/free slot at
`GameManager +0x0` as an anti-tamper heap-jitter allocation, the Supervisor
worker flag at `+0x290` as `subthreadActive`, GameManager flag bit 9 as the
stage-clear sequence gate used by replay control, and play-count `+0x34` as
`restarts`: it increments only for
`SupervisorState_GameManagerRestartFromBeginning`.  Reset-only GameManager
tail fields remain neutral.

Corroborated: GensokyoClub's current spell-practice record/table and the two
music helper names agree with the TH08 records and control flow.  As with its
previous `TitleScreen::RegisterChain` fix, this was treated only as a strong
source hypothesis.  Acceptance came from target data inspection, canonical
owner/addend reconciliation, and the strict repository oracle.  The setup
error paths and resource names are now ordinary source literals whose COFF
relocations replay the original target strings.

Layout: assertions pin `sizeof(SpellcardMusicEntry) == 0x14`,
`sizeof(GuiStageMusicContextSet) == 0x0C`,
`sizeof(RawStageHeader) == 0x490`, its song arrays at `+0x90/+0x290`,
`GameManager::scriptedUpdateFreeze @ +0x2C`, Supervisor recorded FPS,
subthread state, and loading-VM state at `+0x198/+0x290/+0x2FC`, and the
existing manager extents.  No field width, class size, calling convention, or
state transition changed.

VC7 oracle: focused replay passed `EclExIns.obj` **33 / 33**,
`Background.obj` **24 / 24**, `GameManager.obj` **33 / 33**, the broad
freeze/music caller selection **333 / 333**, `SpellCard.obj` **29 / 29**, and
the final setup/Supervisor/replay/title selection **251 / 251**.  The normal
production image and probe graph link.  The required single-job cold rebuild
of all 75 configured comparison objects passes **1,106 / 1,106 exact**;
`TitleScreen::RegisterChain` is independently present in that replay at
**281 / 281**.

Portable oracle: `scripts/build-modern-linux-container.sh` compiles and links
the complete ELF32/i386 target after source-owned initialization replaces the
Linux table-copy shim.  `verify-modern-linux.sh
build/modern-linux-container/th08-modern` verifies the executable and every
fixed target-owned layout symbol.  No isolated automated gameplay smoke exists
for spell-practice music or the ECL whole-update freeze, so no runtime smoke is
claimed.

Result: `GameManager.cpp` and its setup probe now have zero raw-member,
absolute-address, or anonymous-identifier candidates; the six unused/reset-
only GameManager tail fields remain explicit unknowns.  The whole-source router
reports 60 raw-member, 18 absolute-address, 194 anonymous-identifier, and 45
opaque-storage candidates.  These are routing observations, not a semantic-
completion percentage or a new authored-exact claim.

### Supervisor frame-time ownership and Bullet transform semantics — 2026-08-27

Scope: the Bullet spawn, transform-program, per-frame transform, deactivation,
and draw-bucket helpers at `0x0042F5F0`, `0x0042FFC0`, `0x00430E10`, and
`0x00432170` through `0x00432AA0`; the ECL extension callbacks that manipulate
the same bullets and effects; and every authored reader of absolute
`0x017CE8E0` in Bullet, Enemy, Effect, Item, Player, PlayerBomb, and Spellcard.

Observed ownership: `g_Supervisor @ 0x017CE758` plus `0x188` is exactly
`0x017CE8E0`.  Supervisor initialization and the ECL slow-time path write this
float, while the independent gameplay systems above multiply their movement
or timers by it.  The existing `Supervisor::framerateMultiplier` field at
`+0x188` is therefore the single aggregate owner; the formerly declared
`g_EclGameTimeScale` was an overlapping false global.  The separate state word
`g_EclGameTimeScaleFlags @ 0x017CE8FC` remains independent and is not folded
into Supervisor.

All 43 pre-existing time-scale relocation manifests now require
`g_Supervisor + 0x188`.  Eleven source expressions which formerly embedded the
absolute address gained explicit owner/addend relocation records: nine across
the Bullet transform helpers and two in `Player::UpdateShots`.  The duplicate
global ledger row, storage definition, declaration, redundant extern, and
Linux linker alias were removed together.  `offsetof(Supervisor,
framerateMultiplier) == 0x188` pins the source interpretation.

Bullet semantics: the helpers are now `SpawnSingleBullet @ 0x0042F5F0`,
`AdvanceTransformProgram @ 0x0042FFC0`, `SpawnBulletPattern @ 0x00430E10`,
`Deactivate @ 0x00432170`, `ClearDrawBuckets @ 0x004321B0`,
`UpdateDeceleration @ 0x00432210`, `UpdateVectorAcceleration @ 0x004322B0`,
`UpdatePolarAcceleration @ 0x00432390`,
`UpdateRelativeDirectionChange @ 0x00432460`,
`UpdateAbsoluteDirectionChange @ 0x004325A0`,
`UpdateAimedDirectionChange @ 0x004326E0`,
`UpdateBoundaryBounce @ 0x00432830`, `UpdateHorizontalWrap @ 0x004329F0`, and
`UpdateVerticalWrap @ 0x00432AA0`.  Source, decorated symbols, mappings,
implemented/exact ledgers, and match-unit identifiers moved together.

The same target bodies establish ordinary field ownership for
`AnmVm::activeSpriteIndex`, `AnmVm::loadedSprite`, sprite height,
`AnmVm::zWriteDisabled`, additive/normal blend mode, VM alpha,
`ReplayManager::flags`, `Effect::active`, and
`Player::bombState.isInUse`.  The ECL extension instruction view now asserts a
value/byte union at `+0x10`; its four bytes at `+0x0C` remain explicitly
unknown.  Natural VC7 bitfield assignment for the ANM blend mode reproduces
the target dword read/modify/write, so no byte-oriented compatibility shim is
needed.

Unknowns: the Bullet field at `+0xDBC` and bullet-template field at `+0xD40`
do not yet have independent behavioral evidence and retain neutral names.  No
meaning is inferred for the ECL instruction's `+0x08/+0x0C` fields.  The
ReplayManager flag write caused by bullet-pattern spawn is named only at the
known aggregate field; this batch does not guess a narrower event-bit enum.

VC7 oracle: the final Bullet production/reimplementation plus ECL-extension
selection passes **70 / 70 exact** after all owner, field, and symbol changes.
The normal production image links.  The required single-job cold rebuild of
all 75 configured comparison objects passes **1,106 / 1,106 exact**, with no
accepted authored unit lost.

Portable oracle: `scripts/build-modern-linux-container.sh` builds and links
the complete ELF32/i386 executable without the removed time-scale alias.
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
portable executable and every fixed target-owned layout symbol.  No isolated
gameplay smoke harness exists for the transform program or slow-time path, so
none is claimed.

Result: the whole-source semantic router now reports 31 raw-member, 7
absolute-address, 195 anonymous-identifier, and 45 opaque-storage candidates.
These are work-selection observations only, not a semantic-completion
percentage.  ReplayManager serialized state and Supervisor timing/subthread
status are the next core owners.
