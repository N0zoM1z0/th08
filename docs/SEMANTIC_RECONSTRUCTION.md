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
These role names have high dataflow confidence.  The then-unknown effect enum
names and `ScreenEffect +0x24` were deliberately left outside this batch and
are recovered by the later ScreenEffect lifecycle batch below.

Layout: assertions pin the overlay/raw sizes, its inner `+0x4/+0x8` members,
and the current `ScreenEffect::rawParameter0` anchor at `+0x18`; the existing
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
structure.  The constructor-bearing storage now directly uses the proven
`EnemyEclContext` and `EnemyEclInterpolationSlot` types; their renamed VC7
constructor symbols and all caller relocations are tracked explicitly.

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

Inference and unknowns: `position2 @ +0x2C0`, `velocity.z`, SHT `+0x20`, and
most PlayerShot/PlayerBomb work-item internals remain deliberately neutral.
The later focus/gauge pass closes the three timer roles and option state
protocol that were still unknown at this point.  The effect type name describes the
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

The stage-7 check at target `OnUpdate + 0x4D` was initially retained as an
absolute view of `0x0164D2CC`.  The later replay-runtime ownership batch proved
that a natural `g_GameManager.currentStage == STAGE6B` expression emits the
same instruction bytes once the resulting `g_GameManager + 0x3DDC4` COFF
relocation is represented in the match manifest.  Snapshotting the stage
Effect VM still retains a raw
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

The dword at `0x018B8A24` was conservatively retained during this bounded
batch.  Later cross-function reconciliation proved it is
`g_Player.damageAccumulatorThreshold @ +0xE2B2C`: Player compares and
subtracts the same threshold while the Enemy initializer publishes it.  The
unused bytes
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

### Replay runtime protocol and residual absolute-owner closure — 2026-08-27

Scope: ReplayManager's per-stage serialized payload and live playback/recording
cursors; Supervisor's worker/FPS control state; Effect spawn callbacks; and
the remaining safe absolute-address consumers in Background, Player,
ScreenEffect, Enemy, ANM, and ECL opcode 148.

Replay protocol: `StageReplayData +0x21` is the captured-spellcard byte and
`+0x24..+0x3F` is the stage input payload.  ReplayManager now distinguishes the
playback input cursor/end table, recording cursor, FPS-sample cursor/end table,
per-frame RNG seed, and per-frame event flags.  The second stage-data pointer
table is named `stageReplayFpsData` from its production and playback use.  The
reset-only word at `ReplayManager +0x4E` and the replay-header obfuscation fields
remain neutral.

Supervisor state: `+0x294` is a three-state startup-worker result
(idle/running/failed), `+0x178` suppresses FPS display, and `+0x338/+0x33C`
are recording/playback FPS warnings.  The other four dwords cleared by
`ResetUnknownStuff` remain unnamed.  Effect templates now expose separate
initialization and per-frame update callbacks; the two raw pool-slot address
expressions use the real `effects[]` owner, and quality gates use
`g_Supervisor.cfg.effectQuality` with the existing quality enum.

Absolute owners: target `0x0164D2CC` is
`g_GameManager.currentStage @ +0x3DDC4` and the special Background path is
stage 6B.  Player and ScreenEffect reads of `0x0164D2C8` are
`gameplayFrameCounter`; the target performs signed comparisons, preserved by
explicit `i32` casts.  `Player::UpdateShooting` also proves the target's
singleton-shaped `g_Player.bombState.isInUse` read and the GUI shoot input at
`g_GuiMessageInputCurrent`.  Target `0x018B8A24` is
`g_Player.damageAccumulatorThreshold @ +0xE2B2C`.  ECL opcode 148 addresses
`g_GameManager.catkData[SPELLCARD_ST2_BOSS_3L].inGameHistory.maxBonus[SHOT_MARISA_ALICE]`;
layout assertions pin each nested offset.  ANM visibility/alpha tests, the
Enemy VM color write, Spellcard time remaining/limit, and the Item collection
box now use their asserted owners as well.

Compiler evidence: natural aggregate-member expressions introduce genuine
COFF relocations even where the old raw source embedded absolute immediates.
The manifests now require the corresponding target base/addend pairs; no
comparison range or mask was weakened.  The Background stage constant and the
signed frame-counter branches were caught as ordinary byte mismatches before
acceptance.

Unknowns: only five raw-member candidates remain.  They are the independently
unproven Bullet field at `+0xDBC`, bullet-template field at `+0xD40` (two
consumers), and the primary/secondary SHT value at `+0x34` used by Item
collection.  They retain explicit raw views rather than speculative names.

VC7 oracle: the broad focused affected-object replay passes **319 / 319**
accepted units, including RunEcl **26,638 / 26,638**, Background OnUpdate
**6,291 / 6,291**, Enemy OnUpdate **6,198 / 6,198**,
Player::UpdateShooting **311 / 311**, and the ScreenEffect reimplementation
**459 / 459**.  The required single-job cold build of all 75 configured
comparison objects passes **1,106 / 1,106 exact**; the normal VC7 production
image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh` verifies ELF32/ET_EXEC/i386 plus every fixed
target-owned layout symbol.  No isolated automated gameplay smoke exists for
replay recording/playback or FPS-warning transitions, so none is claimed.

Result: the whole-source semantic router reports 5 raw-member, 0
absolute-address, 166 anonymous-identifier, and 44 opaque-storage candidates.
The counts are work-selection observations, not a semantic-completion
percentage.  MIDI and SoundPlayer are the next dense behavior-backed owners.

### MIDI timeline and streaming-audio protocol — 2026-08-27

Scope: the Standard MIDI File track cursor/timing state, MIDI output scheduling
and controller/meta-event dispatch, and SoundPlayer's asynchronous sound/BGM
command protocol and preloaded streaming buffers.  The affected target anchors
include `MidiOutput::ProcessMsg @ 0x00444A90`,
`MidiOutput::FadeOutSetVolume @ 0x00445340`,
`SoundPlayer::LoadBGM @ 0x0045D0F0`, and
`SoundPlayer::ProcessQueues @ 0x0045D790`.

MIDI protocol: each `MidiTrack` now distinguishes data size, byte cursor,
running status, next-event tick, and loop cursor/tick.  `MidiOutput` names the
active file, pending long-message headers, file format, ticks per quarter note,
tempo, elapsed ticks/milliseconds across tempo changes and loop points, output
device, note transposition, fade state, and the target-observed volume-update
gate.  Controller and meta-event constants replace numeric dispatch literals
for bank select, loop start/end, channel volume, pan, effects, end-of-track,
and set-tempo messages.  The reset-only track/output words and
`DummyMidiTimer +0x10` remain neutral because current target-backed source does
not establish their roles.

Streaming-audio protocol: queued operations now use a typed command opcode and
named argument, step, and path fields for preload, load, stop, release, fade,
pause, unpause, and volume changes.  SoundPlayer's per-effect request counts,
preload allocations/data, loaded BGM slot, and BGM file-base offset use shared
owners across GameManager, Supervisor, SoundPlayer, and zwave.  The file-base
offset is independently supported by Supervisor's BGM selection and zwave's
`SetFilePointer` use.  Reset-only/unconsumed SoundPlayer fields, including the
remaining `unk408`, `unk61c`, and `unk5210`, retain neutral names.

Evidence boundary: GensokyoClub TH08 and the adjacent TH07/TH06 sources
corroborate stable class/record layouts, but they retain most of the same
anonymous audio fields.  The semantic names above therefore come from TH08
producer/consumer behavior and target-exact code generation; no adjacent source
name is treated as proof.

VC7 oracle: the final affected-object selection across Midi, SoundPlayer,
Supervisor, GameManager, and zwave passes **169 / 169 exact**.
`MidiOutput::ProcessMsg` reproduces **1,871 / 1,871** target bytes and
`SoundPlayer::ProcessQueues` reproduces **2,358 / 2,358**.  The required
single-job cold build of all 75 configured comparison objects passes
**1,106 / 1,106 exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  There is no
isolated automated MIDI-device or streaming-BGM runtime harness, so no live
audio-output claim is made.

Result: the whole-source semantic router now reports 5 raw-member, 0
absolute-address, 121 anonymous-identifier, and 44 opaque-storage candidates.
These remain work-selection counts, not a semantic-completion percentage.

### Replay file envelope and callback protocol — 2026-08-27

Scope: the replay header's format-selection and obfuscation fields, the
six-byte extended input record, ReplayManager's callback-chain ownership, and
the record/playback/frame-control callback family at
`0x004522A0..0x00453080`.

Callback roles: `CaptureFrameSyncState @ 0x004522A0` snapshots the frame RNG
seed and event flags before the gameplay update;
`RecordInputAndFps @ 0x00452310` appends ordinary two-byte input samples and
periodic FPS samples; `ControlPlaybackFrameAdvance @ 0x00452490` throttles
dialogue and replay-mode frame advancement; and
`PlaybackInputAndFps @ 0x00452550` consumes the ordinary input stream.
`PlaybackExtendedInputAndFps @ 0x004526C0` consumes six-byte records instead.
The lifetime callbacks are now `BeginRecordingStage @ 0x00452830`,
`BeginPlaybackStage @ 0x00452D60`, and
`DeleteReplayManager @ 0x00453080`; the frame-control and frame-sync chain
members use the same roles.

File protocol: `ReplayDataHeader::usesExtendedInputRecords @ +0x06` selects
the six-byte playback callback and its frame-sync chain.
`hasUserDataSection @ +0x07` changes from zero to one only when SaveReplay
appends the trailing `USER` record.  `obfuscationKey @ +0x15` seeds the target's
seven-step byte addition/subtraction and is also the first byte in the checksum
range.  The independently randomized header and payload bytes are named only
as random bytes; no cryptographic or compatibility purpose is inferred.
`ReplayInputSync` is asserted as input/event-flags/RNG-seed words at
`+0/+2/+4`; the event-field role is inference-limited because the current
authored target path initializes it but does not consume it.

Exact boundary: `PlaybackExtendedInputAndFps` remains the sole authored
non-exact function.  The target is **361 bytes** and uses the post-cursor
register phase EAX/ECX/EDX; the natural VC7 object is **362 bytes** and uses
EDX/EAX/ECX, making signed `% 8` choose the six-byte generic `and` instead of
the five-byte EAX form.  The existing syntax matrix remains exhausted; this
semantic batch does not add an exact unit or claim the function.  The adjacent
accepted functions and all callers prove the six-byte record and observable
behavior independently.

VC7 oracle: after every function, decorated symbol, mapping, accepted ledger,
and relocation reference moved together, the focused ReplayManager selection
passes **17 / 17 exact**.  The accepted callbacks reproduce 102, 372, 184, 362,
1,315, 787, and 209 bytes respectively; RegisterChain reproduces **777 / 777**.
The required single-job cold build of all 75 comparison objects passes
**1,106 / 1,106 exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  No isolated
automated playback harness exercises both replay record formats, so no live
runtime claim is made.

Result: the whole-source router reports 5 raw-member, 0 absolute-address, 116
anonymous-identifier, and 44 opaque-storage candidates.  This five-candidate
drop reflects names made explicit in the replay protocol; it is not a semantic
completion percentage.

### Result and score persistence tails — 2026-08-27

Scope: `ResultScreen::AddedCallback @ 0x0045964D`,
`TitleScreen::OnUpdateReplayMenu @ 0x0046E136`, and the serialized `Hscr` and
`Catk` score chapters.

The ResultScreen dword at `+0x20` is cleared immediately before `OpenScore` and
has no observed reader, so it is `scoreLoadResetWord20`; the VM at `+0x10EF8`
has only its active sprite reset to -1 and is `resetOnlyVm10EF8`.  Two completely
unconsumed ResultScreen dwords remain explicitly classified as such.  The
TitleScreen dword at `+0xC284` is cleared after replay-file enumeration and has
no later consumer, giving it the bounded `replayEnumerationResetState` name;
the unaccessed `+0x14` dword is not assigned a menu meaning.

`Hscr` and `Catk` are copied as complete score.dat chapters, so their residual
bytes cannot be discarded merely because gameplay does not read them.  The
alignment byte before `Hscr::cfg` and final byte are named reserved serialized
storage, the `+0x6C` Hscr dword and `+0x228` Catk tail remain unconsumed, and
`Hscr + 0x166` is `defaultScoreMarker` because the synthetic leaderboard
records are its sole producer and write 1.  This does not claim a load-time
consumer that the authored corpus does not contain.  Assertions pin every
renamed owner and serialized offset.

VC7 oracle: focused replay of ResultScreen, ScoreDat, TitleScreen, and
GameManager passes **113 / 113 exact**.  The required single-job cold build of
all 75 comparison objects passes **1,106 / 1,106 exact**, and the normal VC7
production image links.  In particular, the current
`TitleScreen::RegisterChain @ 0x0047146D` source independently replays **281 /
281 exact**; the old 1105-era blocker snapshot is no longer current.

Portable oracle: the complete i386 Linux container build links, and its ELF32
and fixed-layout verifier passes.  The only authored row not represented by an
accepted exact unit is now `ReplayManager::PlaybackExtendedInputAndFps @
0x004526C0` (361 bytes), because it has no configured match unit; it is not a
reported byte mismatch.  That ledger gap remains a separate bounded Oracle
task.

### ScreenEffect lifecycle and visual modes — 2026-08-27

Scope: `CalcFadeHold @ 0x0045B800`, `RegisterChain @ 0x0045B8B0`,
`CalcArcadePulse @ 0x0045BC90`, `DrawArcadePulse @ 0x0045BD70`,
`CalcShakeEnvelope @ 0x0045BF10`, the lifecycle callbacks at
`0x0045C0E0/0x0045C100`, and `BeginFadeRelease @ 0x0045C160`, together with
the Player, PlayerBomb, ECL, and Supervisor callers that select those modes.

Observed modes: effect 3 takes a repeat count and ARGB color, fades the
playfield overlay's alpha to zero, decrements the count, and repeats; it is now
`SCREEN_EFFECT_ARCADE_PULSE`.  Effect 7 treats the first registration value as
shake amplitude and the three variant parameters as ramp-up, hold, and
ramp-down frames; it is `SCREEN_EFFECT_SHAKE_ENVELOPE`.  Effects 5 and 6 raise
alpha from zero to 128 over the requested duration and then retain it until an
external release request.  Supervisor creates those two adjacent modes for
the full-screen and playfield loading overlays, so they are
`SCREEN_EFFECT_FULL_FADE_HOLD` and `SCREEN_EFFECT_ARCADE_FADE_HOLD`.

Lifecycle: `HideLoadingVms` and the parallel loading transition call
`BeginFadeRelease`, which writes `fadeReleaseRequested @ +0x24`, resets the
timer, and lets `CalcFadeHold` lower alpha from 128 to zero over eight frames
before removing the job.  The chain's add callback is now `InitializeTimer`;
its delete callback cuts the paired draw element and destroys the object as
`DeleteScreenEffect`.  The common alpha at `+0x10` is `overlayAlpha`, the
discriminator at `+0x00` is a typed `ScreenEffectType`, and the variant storage
at `+0x18/+0x1C/+0x20` has neutral raw owners beneath the existing typed
TU-local overlay.  `+0x0C` remains `unk0c` because no authored read establishes
a role.

Evidence boundary: the names come from exact TH08 callback behavior and all
authored call sites.  They do not rely on an adjacent-version label.  Layout
assertions pin `ScreenEffect` at 0x34 bytes and the discriminator, alpha,
variant storage, release flag, and timer at `+0x00/+0x10/+0x18/+0x24/+0x28`.

VC7 oracle: the two ScreenEffect comparison-object selections pass **16 / 16**
and **5 / 5 exact**, including RegisterChain **628 / 628**, pulse calculation
**210 / 210**, shake-envelope calculation **459 / 459**, and release
**34 / 34**.  A focused production-object selection across ECL, Player,
Supervisor, and their accepted dependencies passes **132 / 132 exact**.  The
required single-job cold build of all 75 comparison objects passes
**1,106 / 1,106 exact**, including all 54 PlayerBomb accepted units, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  There is no
isolated automated visual-effect smoke, so no live-rendering claim is made.

Result: `ScreenEffect.cpp` now has zero candidates in every semantic-router
category.  The whole-source router reports 5 raw-member, 0 absolute-address,
113 anonymous-identifier, and 44 opaque-storage candidates.  These counts are
work-selection observations, not a semantic-completion percentage.

### Effect factory, draw layers, and radial-trail protocol — 2026-08-27

Scope: the EffectManager factory block at `0x004253E0..0x00425D6C`,
directional and radial-trail callbacks at `0x00426B20..0x00427BEF`, stage-origin
and draw routing at `0x00426D10/0x00428100..0x004284AF`, and the resource and
animation-lifetime helpers at `0x004284B0..0x0042873F`.  All mapped names,
decorated symbols, match-unit selectors, accepted ledgers, callback tables, and
production callers moved together; no implementation, layout, callback value,
or state transition changed.

Factory ownership: `SpawnEffect` scans the primary 0x200-entry ring,
`SpawnEffectInSecondaryPool` scans `[0x200, 0x280)`, and the fixed-slot helpers
address entries at `slotIndex + 0x280`.  The velocity-taking siblings are now
`SpawnEffectWithVelocity` and `SpawnEffectInFixedSlotWithVelocity`;
`GetFixedSlotVm` exposes the same fixed-slot address rule to ECL extension
callbacks.  These pool boundaries come directly from exact target loops and
address arithmetic, not from an adjacent-version name.

Draw ownership: BulletManager calls `DrawBulletLayerEffects` for the linked
group whose default Z is 0.04.  Background calls `DrawBackgroundEffects`, which
applies the target's minimum/moderate effect-quality gates and dispatches the
stage-effect draw modes.  `AdjustStageEffectDrawPosition` is the callback used
for effect ids 0x33 and 0x3F; its name is limited to the proven stage-effect
call path and position adjustment.  `ShiftStageEffectOrigins` updates the
stored origin of active id-0x33 effects after a compensated camera jump.

Directional and trail ownership: the `0x00426B20..0x004271FF` initializers
store an origin plus a normalized XY direction, and their updates apply the
observed 60-frame, 240-frame, or eased 90-frame displacement.  The
`0x004272E0..0x00427BEF` family allocates and owns a textured-vertex strip,
builds circular, elliptical, or radially modulated geometry, marks it dirty
when VM parameters change, and optionally selects the alternate draw group.
The source therefore names initialization, synchronization, timed/fading, and
anchored radial-trail roles while leaving overlapping generic Effect storage
neutral outside this mode.

Lifecycle: `LoadEffectResources` resets the pool and resolves/preloads the
common and stage ANM owners; `ReleaseEffectResources` frees per-effect vertex
buffers and releases the stage ANM when reload is enabled.  RegisterChain uses
those functions as its add/delete callbacks.  `HasAnimationEnded` is only the
exact `vm.currentInstruction == NULL` predicate and is not generalized into a
broader effect-state claim.

Evidence boundary: six stage-specific particle callbacks remain deliberately
address-named: the init/update pairs `0x00426280/0x004264F0`,
`0x00426720/0x00426990`, and `0x00426D70/0x00426E70`.  Their exact bodies prove
camera-relative random initialization, motion, visibility, and draw-group
behavior, but the authored corpus does not prove a stable visual identity for
any pair.  Renaming them from appearance guesses would reduce readability
honesty rather than semantic debt.

VC7 oracle: twelve representative factory/draw/lifecycle targets were first
replayed through target-pinned typed packets, and the complete EffectManager
selection then passed **52 / 52 exact**.  Representative exact extents include
`SpawnEffect` **529 / 529**, `SpawnEffectWithVelocity` **533 / 533**,
`SpawnEffectInFixedSlot` **365 / 365**,
`SpawnEffectInFixedSlotWithVelocity` **394 / 394**,
`SpawnEffectInSecondaryPool` **509 / 509**, `DrawBulletLayerEffects`
**219 / 219**, `DrawBackgroundEffects` **290 / 290**, and
`AdjustStageEffectDrawPosition` **411 / 411**.  The required single-job cold
build of all 75 comparison objects passes **1,106 / 1,106 exact**, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  Its reconstructed
Effect callback table was updated to the same semantic symbols.  There is no
isolated automated particle-rendering smoke, so no live visual claim is made.

Result: `EffectManager.cpp` has zero semantic-router candidates.  The
whole-source router remains at 5 raw-member, 0 absolute-address, 113
anonymous-identifier, and 44 opaque-storage candidates because the six
evidence-limited callbacks are outside the heuristic's current identifier
pattern.  These counts are work-selection observations, not a semantic-
completion percentage.

### ANM projection, draw, and texture-strip protocol — 2026-08-27

Scope: the central ANM render lane at `0x00463470..0x00464EB0`, together with
`AnmVm::IsStopped @ 0x004396F8` and the effect-table callback
`AnmVm::UpdatePulsingRadialTrail @ 0x0040EB50`.  Source declarations,
production callers, decorated symbols, mapping/accepted ledgers, relocation
manifests, and the modern callback table moved together.  Implementations and
render-state order did not change.

Projection and draw roles: `Draw2DRotatedOrAxisAligned @ 0x00463470` is the
rotated/scaled 2D draw path selected beside the existing non-rotation paths.
`ProjectCameraFacingQuad @ 0x004639E0` projects the VM origin and the
Background camera-right vector, then scales the sprite quad by their projected
unit distance; `DrawCameraFacingQuad @ 0x00463CF0` submits that result.
`ProjectCameraFacingQuadWithCallback @ 0x004640E0` performs the same projection
but calls the supplied position callback before constructing the vertices.
`Project3DQuad @ 0x00463D60` applies translation, scale, rotation, and anchor
state to the manager's four 3D quad vertices, projects them, and caches the
matrix; `DrawProjected3DQuad @ 0x00464070` submits that result.

Strip and queue roles: `InitializeHorizontalTextureStrip @ 0x004649A0` and
`InitializeVerticalTextureStrip @ 0x00464B00` initialize diffuse/RHW vertices
while advancing UVs along U or V respectively.  Independent TH08 callers use
the same helpers for ECL enemy trails, Effect radial trails, and PlayerBomb
geometry.  `QueueSpriteQuad @ 0x00464DD0` rejects hidden/disabled/transparent
VMs, flushes on texture or shader-state changes, applies the target render
state, and appends four vertices to the sprite buffer.  Its old ledger ABI of
two `u8` arguments was corrected to `AnmVm *` and
`VertexTex1DiffuseXyzrhw *`: target stack dword reads and the exact VC7
decorated symbol independently require those pointer types.

VM helpers: `IsStopped` is exactly the VM `stopped` bit predicate.
`UpdatePulsingRadialTrail` marks the Effect vertex strip dirty and configures
48 segments, thickness 32, angle zero, and an alternating radius of 64/72 for
effect-table id 82.  Its class membership and effect callback use are both
target-visible; the Linux ABI bridge retains that shared object identity.

Evidence boundary: unused `AnmTextureHeader`, `AnmLoadedSprite`, `AnmVm`, and
`AnmManager` storage with no authored behavioral reads remains neutral or
opaque.  No visual identity is inferred for those fields, and the successful
portable link is not treated as a live-rendering observation.

VC7 oracle: all nine central functions pass focused strict comparison:
**912 / 912**, **771 / 771**, **107 / 107**, **779 / 779**, **98 / 98**,
**786 / 786**, **352 / 352**, **352 / 352**, and **225 / 225** bytes.
`IsStopped` passes **24 / 24** and `UpdatePulsingRadialTrail` passes
**112 / 112**.  After the shared-header and caller renames, the required
single-job cold build of all 75 comparison objects passes **1,106 / 1,106
exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  The first link
caught a stale Itanium name-length in the renamed callback bridge; correcting
that mechanical alias to the emitted member symbol made the oracle pass without
changing target source or VC7 bytes.

Result: all eleven bounded address-named functions now expose their proven
render or VM roles.  The whole-source router remains at 5 raw-member,
0 absolute-address, 113 anonymous-identifier, and 44 opaque-storage
candidates; those remaining counts are work-selection observations, not a
semantic-completion percentage.

### ECL EX gameplay callback protocol — 2026-08-27

Scope: the complete 32-entry `g_EclExInsn @ 0x004C6CB0` callback table and its
33 exact functions at `0x00423390..0x004253D9`.  Twenty-nine remaining address
names, their declarations, table entries, decorated symbols, accepted ledgers,
relocation manifests, and unit selectors moved together.  The four already
semantic handlers—`ConfigureNightBlindness`, `ReisenFreezeBullets`,
`MokouResurrection`, and `SetScriptedUpdateFreeze`—retain their source names.

Barrier protocol: three start handlers allocate paired fixed-slot effects and
install `DrawBulletWarpBarrier`; the narrow, medium, and wide update handlers
detect crossings between concentric rectangular zones, invert velocity, scale
position between the zones, rotate the bullet angle by pi, and apply a
two-frame transition cooldown.  `StopBulletWarpBarrier` deactivates the paired
effects and changes the shared render mode.  The size qualifiers come from the
exact bounds and scale ratios; no spell-card or visual identity is inferred.

Enemy and hitbox protocol: `UpdateBouncingEnemyMotion` reflects horizontal or
top-boundary motion, accelerates vertical velocity toward the ECL limit, and
clears the offscreen permission after the bottom boundary.  The three rotating
laser handlers publish narrow, medium, and wide inner/outer hitboxes through
`Player::CalcLaserHitbox`.  `SynchronizeOrbitingChildFormation` enumerates
same-group children, publishes group count/change state, and eases each
child's orbit angle toward uniform spacing.

Stage-specific control: the table now directly names short/long arcade pulses,
the combined pulse-and-shake envelope, a shake-only envelope, red Background
tint accumulation, the screen-effect counter setter, spell-number and capture-
count publishers, spell-presentation dismissal, marked-bullet child triggers,
enemy spawning from marked bullets, and Bomb-versus-Extend item spawning.
`BULLET_TRANSFORM_ECL_EX_TRIGGER_MARKER` names the target-observed
`0x100000` bit only as this callback family's marker; its presentation meaning
remains deliberately unspecified.

Reisen and time-scale protocol: `AdvanceReisenBulletPhase` cycles the selected
bullets through additive/collision-disabled, alpha-restoring, and normal
states beside the already named freeze handler.  `SetFrameRateDivisor`,
`EnterScaledBulletTime`, and `ExitScaledBulletTime` scale active bullet
velocities, preserve and restore their sprite indices, select the paired
barrier interrupts, and restore the global multiplier.  These names describe
the exact state transitions and do not claim that the global time-scale flag's
other bits are understood.

Evidence boundary: target-pinned typed packets were generated for all 33
addresses before renaming, including stack homes, direct calls, relocations,
and exact compiler comparison.  The instruction header bytes at `+0x08` and
`+0x0C` remain unknown, as do unrelated ECL globals and the artistic identity
of the generic barrier effects.

VC7 oracle: the baseline and final focused `EclExIns.obj` selections both pass
**33 / 33 exact**.  Representative extents include the barrier renderer
**1,203 / 1,203**, each zone transformer **784 / 784**, orbit formation
**494 / 494**, Reisen phase update **443 / 443**, bullet freeze
**536 / 536**, and scaled-time entry/exit **213 / 213** and **255 / 255**.
After the shared Bullet flag enum changed, the required single-job cold build
of all 75 comparison objects passes **1,106 / 1,106 exact**, and the normal
VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  There is no
automated stage-script/render smoke for all EX callbacks, so no live visual or
spell-specific claim is made.

Result: every entry in the ECL EX callback table now has a behavior-readable
function name, with unknown instruction bytes and effect identities retained.
The whole-source router remains at 5 raw-member, 0 absolute-address,
113 anonymous-identifier, and 44 opaque-storage candidates because its current
heuristic does not count address-named functions; these are routing counts,
not a semantic-completion percentage.

### Spellcard control, capture, and presentation protocol — 2026-08-27

Scope: fifteen exact helpers at `0x00405260`, `0x00415C60..0x00416B85`,
`0x00417860`, `0x0041F0B0..0x0041F107`, `0x0041FD90`, `0x0042BC50`,
`0x0042DFF0`, and `0x0044CBA0..0x0044D174`, together with every production
caller and the shared Spellcard flag and presentation-VM owners.  Target-pinned
typed packets were captured for all fifteen addresses before renaming.  The
source, declarations, mapping/implemented/reccmp ledgers, accepted selectors,
and decorated symbols then moved together.

Control protocol: the shared flag namespace now distinguishes active state,
capture validity, timer-callback transition, alternate effect style, disabled
effect tracking, enabled Bomb damage, pending capture reward, captured state,
suppressed bonus presentation, and disabled bonus updates.  The exact shift-
and-test expressions and three VC7-visible bitfield assignments remain intact.
In particular, `InvalidateCaptureAndEnableBombDamage @ 0x0044CBA0` still copies
the active bit into the Bomb-damage bit through one-bit fields; simplifying it
to whole-word arithmetic is known to change VC7 register ownership.

Lifecycle names: `HidePlayerSpellPresentation @ 0x00416130` and
`HideEnemySpellPresentation @ 0x00416160` interrupt the paired name/frame VMs;
`DeactivateWithoutCleanup @ 0x00416AF0` is the active-enemy-loss path;
`AddBonusProgress @ 0x00416B10` updates the capped bonus and decay rate;
`SetEffectTrackingDisabled @ 0x0041F0B0` and
`SetBonusUpdatesDisabled @ 0x0041F0E0` expose the two ECL controls.
`IsCaptureValid`, `WasCaptured`, `UsesAlternateEffectStyle`, and
`IsBombDamageEnabled` name the four exact state queries.

Transition helpers: `ClearBulletsForTransition @ 0x00415C60` is the mode-1
bullet-clear wrapper shared by dialogue, full-power collection, ECL, and spell
start; `ResetBulletRankInfluence @ 0x00415C80` restores the six per-Enemy rank
endpoints.  `PrepareSpellcardForTimerCallback @ 0x0042BC50` has a target-proven
`Spellcard *` fastcall receiver: its sole caller is the non-timeout Boss timer
transition, and its three stores invalidate capture, mark the transition so
`EndSpell` skips duplicate clear/reward work, and zero the bonus.

Presentation ownership: twelve offset-derived VM members now expose player and
enemy portraits, their backdrop/overlay, player/enemy spell-name text and
frames, two extra enemy-name layers, and the bonus frame/digit renderer.  The
two draw-only VMs at `+0x90C/+0xE54`, flag bit 4, and `unknown_044` retain
neutral names because the authored corpus does not prove their producers or
stable presentation roles.

VC7 oracle: the directly affected eleven-object selection passes **192 / 192
exact**, including `StartSpell`, `EndSpell`, `OnUpdateImpl`, `OnDrawImpl`,
RunEcl, Player Bomb/death, Enemy timer update, and all fifteen renamed helpers.
The typed `Spellcard *` fastcall refinement independently replays **50 / 50**
bytes.  The required single-job cold build of all 75 comparison objects passes
**1,106 / 1,106 exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  No layout,
callback ordering, gameplay state operation, or initialization sequence
changed.  The whole-source router remains at 5 raw-member, 0 absolute-address,
113 anonymous-identifier, and 44 opaque-storage candidates because it does not
count these address names or offset-derived VM identifiers; these are routing
counts, not a semantic-completion percentage.

### Ending script and persisted score protocol — 2026-08-27

Scope: the exact Ending script functions at `0x00428890..0x00429694`, callbacks
through `AddedCallback @ 0x00429980`, TH8K chapter iteration and construction
across `ScoreDat::OpenScore @ 0x0045A5E0` through `ParsePLST @ 0x0045AF30`,
`ResultScreen::WriteScore @ 0x00453D0D`, `InitializeScoreData @ 0x0043BBE1`,
and the CATK checksum consumers in `Spellcard::StartSpell @ 0x004152A0` and
`EndSpell @ 0x004161B0`.  Target-pinned typed packets were captured before
renaming, and the canonical executable was also inspected with target-safe
objdump for the persisted CLRD byte.

Ending protocol: `RunEndingScript` owns the ending bytecode cursor, dispatches
the `@` opcode family, renders text through the next VM, and enforces separate
line/page waits with independent skip-lock countdowns.  The buffer, cursor,
ending ANM, sixteen VMs, elapsed timer, configured normal/minimum line waits,
and next text-VM index now expose those roles.  `ReadScriptParameter`,
`UpdateAndDrawFade`, and `LoadEndingScript` replace file-format-shaped names
with their exact behavior.  Layout assertions pin every relied-on member,
including `scriptData @ +0x2A54`, `canSkipChainedEnding @ +0x2A5C`,
`pageWaitTimer @ +0x2A7C`, and `scriptCursor @ +0x2AB4`.

Skip evidence: `AddedCallback` clears `canSkipChainedEnding`, then sets it only
when entering stage 6B with Extra already unlocked or stage 6A with Spell
Practice already unlocked.  Opcode `@F` loads the chained script and copies
that value into `hasSeenEnding`, which gates SELECT/SKIP acceleration.  The
persisted `Clrd::pendingEndingSkip @ +0x20` is read into the same gate and
immediately cleared only on the non-clear ending path.  Full target `.text`
inspection finds exactly those two references and no producer, so the name
states its one-shot consumer protocol without guessing an artistic ending
identity.

Score record evidence: `Th8k::chapterSize @ +0x04` is checked for zero and
advances every chapter iterator.  All authored chapter producers write the
same `sizeof(...)` value at `+0x06`, so that field is conservatively
`chapterSizeCopy`; no stronger purpose is claimed.  `Catk::historyChecksum @
+0x0E` is compared with the low byte of the spell-name-plus-history sum,
causes both history arrays to reset on mismatch, and is recomputed after
attempt/capture changes.  At this batch boundary, `Th8k +0x09`, PSCR/Hscr
residual bytes, CATK `+0x228`, and Ending `+0x2A98` remained neutral because
their producers or stable roles were not yet proven.  The ResultScreen batch
below resolves the first byte and PSCR `+0x175` without extending the claim to
the other residual fields.

VC7 oracle: focused Ending replay passes **11 / 11 exact**.  The directly
affected ScoreDat, SpellCard, ResultScreen, GameManager, Supervisor, and
TitleScreen selection passes **172 / 172 exact**, including all checksum and
chapter-size consumers.  After adding the shared-header offset assertions, a
fresh single-job cold build of all 75 comparison objects passes **1,106 / 1,106
exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and all fixed target-owned layout symbols.  The focused
four-file router falls from 14 anonymous identifiers plus one opaque range to
seven anonymous identifiers plus that retained range; the whole-source router
is now 5 raw-member, 0 absolute-address, 106 anonymous-identifier, and 44
opaque-storage candidates.  These counts are routing evidence only.

### ResultScreen high-score, keyboard, and phase protocol — 2026-08-27

Scope: `ResultScreen::WriteScore @ 0x00453D0D`, the score-list wrappers at
`0x00454C59/0x00454C87`, category and spell-card state handlers beginning at
`0x00454CB2`, `HandleResultKeyboard @ 0x0045621E`,
`HandleOtherStatsScreen @ 0x004578AA`, `OnDraw @ 0x004586B4`,
`AddedCallback @ 0x0045964D`, and `ScoreDat` list operations at
`0x0045A500/0x0045A5A0`.  Eight post-rename target-pinned packets replay these
important functions exact.  Target-safe objdump against the canonical image
supplies the field-level observations below; IDA was not used.

High-score marker protocol: `HandleResultKeyboard @ 0x004563B7` stores byte 1
at `ResultScreen::hscr.base + 0x09` before inserting the current run into its
difficulty/character list.  `OnDraw @ 0x004588BC/0x0045892B` reads that same
byte from each list entry to choose the active name-entry color and draw the
edit cursor.  `WriteScore @ 0x00453E4C` clears it immediately before copying
the complete 0x168-byte HSCR chapter to the persisted buffer.  The shared
header byte is therefore `Th8k::runtimeMarker`, with named values for no marker
and current-run score; the name deliberately does not assign it a file-format
meaning.

PSCR serialization protocol: `WriteScore @ 0x00453FA3` performs the sole
authored read of byte `Pscr +0x175` and copies the complete 0x178-byte chapter
only when it is nonzero.  `ParsePSCR` initializes the byte to zero, then an
accepted persisted chapter can replace the full record.  No authored producer
sets a fresh nonzero value, so `shouldSerialize` states only the observed gate;
the gameplay condition or older-version producer remains unknown.

ResultScreen state: target dword accesses at `+0x10/+0x18` select a handler's
entry/interactive phase and time its transition, so they are `statePhase` and
`statePhaseTimer`.  `+0x2C` is the shared alphabet-grid selection for both
high-score and replay names.  `+0x50` stores the success result of parsing LSNM
and selects the END key when a saved name exists.  `+0x54` is set only while
the spell-card list is exiting and is consumed with the ten-frame delay.
`HandleOtherStatsScreen @ 0x0045796F/0x00457CF7/0x00457D49` stores and compares
the byte at `+0x19C` solely to refresh displayed play time when the seconds
value changes.  Assertions pin all six offsets without changing
`sizeof(ResultScreen) == 0x477B0`.

List behavior: `ScoreDat::InsertScore` scans the descending score list,
allocates and links one node, and returns its zero-based insertion rank;
`ResultScreen::InsertScore` selects the difficulty/character head.
`FreeScoreNodes` names the fact that only list nodes, not the referenced HSCR
records, are released.  Mapping rows now record the source/target ABI for these
functions and for the inspected ResultScreen callbacks.  Addresses, extents,
relocation targets, authored coverage, and accepted exact counts are unchanged.

Evidence limits: `Hscr +0x166` has one default-score write and no consumer;
ResultScreen `+0x20`, its unused `+0x24/+0x11448`, and the reset-only anonymous
VM remain unknown.  They were not promoted from presentation guesses.

VC7 oracle: fresh focused replay passes ResultScreen **30 / 30 exact** and
ScoreDat **13 / 13 exact**.  A required single-job cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.  The one authored unit outside the accepted
ledger remains unchanged.

Portable oracle: `scripts/build-modern-linux-container.sh` compiles and links
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verifies ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  The focused four-file router falls from 15
to 12 anonymous-identifier candidates; the whole-source router is now 5 raw,
0 absolute, 103 anonymous, and 44 opaque candidates.  These are routing deltas,
not completion percentages.

### Residual raw-member closure — 2026-08-27

Scope: the final five router-selected raw member expressions in
`ItemManager::OnUpdate @ 0x00440500` and
`BulletManager::SpawnSingleBullet @ 0x0042F5F0`, together with their asserted
`PlayerRawShtFile`, `BulletTypeSprites`, and `Bullet` owners.  Post-change
target-pinned packets replay both complete functions exact.  This is a source-
readability closure for raw member arithmetic, not a claim that every retained
anonymous byte is semantically understood.

SHT evidence: target `0x0044054D` reads `Player::focusMode @ +0x03`, then
`0x00440558..0x0044056F` selects the dword at `PlayerRawShtFile +0x34` from the
secondary or primary SHT owner.  `0x00440578` immediately loads it as a float
and multiplies it by the frame-rate multiplier.  The resulting step scales
ordinary/scattered item displacement and gravity; auto-collection instead
uses the independent `itemAutoCollectSpeed @ +0x14`.  The field is therefore
`itemMovementSpeed @ +0x34`, extending the previously asserted SHT prefix
without guessing a more specific file-format label.

Bullet evidence limit: full target `.text` inspection finds exactly one access
to `Bullet +0xDBC`, the byte-one store at `0x0042F8E2`, and exactly two accesses
to `BulletTypeSprites +0xD40`, the template-to-live byte copy at
`0x0042FA2C/0x0042FA32`.  No consumer exists in the authored target corpus.
The bytes are now directly addressable, offset-asserted `unknownDBC` and
`unknownD40` members so the spawn code no longer needs nested byte-pointer
casts, but their names and documentation remain explicitly neutral.

VC7 oracle: fresh ItemManager replay passes **19 / 19 exact** and fresh
BulletManager replay passes **24 / 24 exact**.  A required single-job cold
build of all 75 comparison objects passes **1,106 / 1,106 exact** with zero
failures, and the normal VC7 production image links.  No mapping, match,
authored, global, or relocation ledger changed.

Portable oracle: `scripts/build-modern-linux-container.sh` compiles and links
the complete i386 target, and `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verifies ELF32/ET_EXEC/i386 plus all
fixed target-owned layout symbols.  Whole-source raw-member candidates fall
from 5 to **0** while absolute-address candidates remain **0**.  Explicitly
representing the two unknown Bullet bytes moves their repeated occurrences
from opaque storage into the anonymous-identifier category, yielding 110
anonymous and 42 opaque candidates; this category migration is not semantic
progress for those bytes.

### Address-name and ECL type closure — 2026-08-27

Scope: the remaining behavior-recoverable address-named authored helpers, the
ECL timeline header word used by `EclManager::Load @ 0x00418330` and its two
accessors at `0x0042DFB0/0x0042DFD0`, and the duplicate constructor-bearing ECL
context shells.  Each renamed function was first checked with a target-pinned
typed packet, then replayed as its smallest configured VC7 comparison unit.

ECL and lifecycle behavior: `EclRawHeader::timelineCount @ +0x06` now names the
word that bounds and indexes the file's timeline-offset table.
`Supervisor::BeginLoadingCompletion @ 0x00448972` begins the loading-finish
transition, `Enemy::DetachEnemyChain @ 0x0042ADB0` separates an Enemy
attachment chain and conditionally awards its death rewards,
`InterpolateWrappedAngle @ 0x0042EB10` interpolates across the shortest wrapped
angular path, and `EclRunLow::ApplyRandomBiasedMove @ 0x004224A0`
applies the ECL instruction's randomized, bias-adjusted movement.  The names
state directly observed state transitions and arithmetic rather than inferred
script or presentation identities.

Player Bomb callbacks: the exact functions at `0x0040E040..0x0040E2D0`,
`0x00410BB0`, `0x004114E0..0x00411A80`, and `0x00413070` now distinguish the
wavy, positive/negative diagonal, octagonal, twelve-segment, barrier, rotating
barrier, and orthogonal radial update/initialization paths.  Their callback
table entries and the portable Linux callback bridge use the same symbols.
These are geometry- and lifecycle-based names; no character, spell-card, or
artistic effect identity is asserted.

ECL ownership: the storage formerly represented by duplicate imported
`EnemyUnkStruct2/3` shells now directly uses `EnemyEclContext` and
`EnemyEclInterpolationSlot`.  Their target constructors at
`0x0042A450/0x0042A4C0`, all constructor relocations, the primary context, and
the sixteen-entry Enemy call stack therefore share one asserted type model.
This removes duplicate opaque types without changing their layout or VC7 ABI.

Evidence boundary: the six stage-specific Effect callbacks at
`0x00426280`, `0x004264F0`, `0x00426720`, `0x00426990`, `0x00426D70`, and
`0x00426E70` deliberately remain address-named.  Their bodies prove particle
motion and camera relationships but not stable visual identities.  The two
comments that retain target `FUN_00422720` and `FUN_00422020` are provenance
for already named dispatcher helpers, not anonymous function definitions.
Reset-only, unused, serialization-reserved, and producerless fields likewise
remain neutral rather than receiving presentation guesses.

VC7 oracle: the ECL load/accessor units replay **239**, **20**, and **25** exact
bytes; the four lifecycle/math helpers replay **88**, **1,336**, **114**, and
**630** exact bytes.  The ten Bomb callbacks independently replay **209**,
**209**, **196**, **119**, **76**, **567**, **139**, **708**, **139**, and
**204** exact bytes, and the unified ECL constructors plus `Enemy::Enemy`
replay **62**, **25**, and **394** exact bytes.  A required single-job cold
build of all 75 comparison objects passes **1,106 / 1,106 exact** with zero
failures, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links and
`verify-modern-linux.sh build/modern-linux-container/th08-modern` verifies the
ELF32 executable and every fixed target-owned layout symbol.  The whole-source
router now reports **0 raw-member**, **0 absolute-address**, **107 anonymous-
identifier**, and **41 opaque-storage** candidates.  With all recoverable
address-named authored definitions closed except the six evidence-limited
Effect callbacks, this closes the current source-readability milestone.  The
remaining counts are an evidence-directed backlog, not a completion
percentage and not permission to guess meanings for inert storage.

### Probe, production, and portable semantic closure — 2026-08-27

Scope: a follow-up audit of production source, exact probe TUs, and the modern
runtime together.  This audit deliberately checked source markers outside the
heuristic debt categories.  It found that `TitleScreen.cpp` still linked empty
`UnlockLastWordSpellCards @ 0x0046CBBB` and
`FormatSpellCardInfo @ 0x0046D7F9` bodies even though their exact
implementations, plus `ConvertToFullWidthDigits @ 0x0046D763`, were accepted in
`TitleReplayMenuProbe.cpp`.  The router did not classify ordinary empty
functions, so its prior 0/0 raw/absolute result was necessary but not
sufficient for production semantic closure.

The three Title implementations now have single source owners in shared `.inl`
files used by both the production and probe TUs.  Production also owns the
64-byte full-width-number buffer and the target-backed Shift-JIS spell-card
tables.  `g_TitleLastWordCommentFormats @ 0x004C82C8` is an asserted array of
18 0x30-byte records, each containing two format pointers and two groups of
five integer arguments; `g_TitleSpellDifficultyNames @ 0x004C8628` is the
six-entry difficulty-name table.  The reproducible
`scripts/analysis/verify-title-spell-card-data.py` oracle compares every
integer, every pointed-to string byte, and the canonical target hash: **18 / 18
records and 6 / 6 strings exact**.

Adjacent cleanup closes four type-system escapes.  The common ECL interpolation
slot now names `callbackIndex @ +0x14` and `parameters[4] @ +0x1C`, matching the
already exact installer/callback protocol.  `TitleScreen +0xC29C` is
`spellCardInfoRevealCountdown`, proven by its 21-frame start and the formatter's
11/9/7/5/3 staged reveal gates.  `GameManager::InitializeScoreData @
0x0043BBE1` reads the asserted `flags.isPracticeMode` bit instead of a raw
`GameManager +0x3DBAC` cast.  The Linux target-data bootstrap now reaches the
four already mapped Last Spell, Effect-template, stage-bonus, and GUI-color
owners through local ABI aliases rather than writing literal addresses.

VC7 oracle: focused replay passes ECL RunEcl **26,638 / 26,638**, GameManager
score initialization **552 / 552**, and every Title probe unit **11 / 11**,
including **2,984**, **150**, and **2,365** exact bytes for the three promoted
Title implementations.  A required single-job cold build of all 75 comparison
objects passes **1,106 / 1,106 exact** with zero failures, and the normal VC7
production image links.

Portable oracle: the complete i386 Linux container build links the promoted
Title behavior and initialized data, and the fixed-layout verifier passes.  A
scan including probes and modern sources reports **0 raw-member**, **0 absolute-
address**, **100 anonymous-identifier**, and **41 opaque-storage** candidates;
production-only is **0 / 0 / 99 / 41**.  The remaining anonymous/opaque entries
were reviewed as padding, reset-only or unconsumed slots, reserved serialized
bytes, or otherwise evidence-limited storage.  They are retained neutrally;
these heuristic totals are not a completion percentage.

### Production free-function identity closure — 2026-08-27

Scope: compare every authored `implemented.csv` identity against the symbols
actually exported by the normal production object set through `build/th08.def`.
This is a ledger-identity batch; it changes no source expression, ABI, object,
address, or accepted unit.

Evidence: VC7 emits the eleven PlayerShot callbacks from `Player.cpp` and the
score initializer from `GameManager.cpp` as namespace members such as
`?UpdateHomingShot@th08@@...` and `?InitializeScoreData@th08@@...`.
`mapping.csv`, `reccmp-functions.csv`, `implemented.csv`, and `matches.csv`
had retained unqualified names for exactly those twelve functions.  The
detour generator therefore reported them absent even though `build/th08.def`
contained their real mangled symbols.  All four ledgers now use
`th08::SpawnShotAlongPlayerAngle` through
`th08::SpawnPeriodicShotHitEffect` and `th08::InitializeScoreData`, matching
the source namespace and production COFF identity.

Oracle: production-symbol audit warnings fall from 32 to 20; none of the
remaining warnings names an ordinary authored free function.  They are the
generator's existing unsupported operator/deleting-destructor spellings.
Focused accepted replay remains Player **51 / 51 exact** and GameManager
**33 / 33 exact**, including all twelve corrected identities.  Tracking still
reports **1,106 implemented / 1,106 accepted exact**, with no claim rows.  The
normal VC7 image and previously built identical-source Linux image remain
valid; this batch only makes the canonical semantic names agree with their
already verified machine symbols.

### Calc/draw scheduler priority protocol — 2026-08-27

Scope: the shared `Chain` scheduler in `Global.hpp`/`Global.cpp`, every
production calc/draw registration, and every ordinary gameplay ScreenEffect
registration.  This is a source-readability batch: it changes no priority
value, list operation, callback ABI, object layout, target address, or ledger.

Observed: exact TH08 registration bodies pass integer priorities `0..21` to
the two sorted scheduler lists.  The registered owner and callback identify
each stable slot: Supervisor starts calc at 0, Ascii at 1, GameManager at 2,
ScreenEffect at 3, the gameplay systems follow in execution order, and Replay
uses distinct playback, frame-sync, record, and frame-control slots.  The draw
list similarly distinguishes the two Background, Enemy, Player, and Ascii
layers.  All normal gameplay ScreenEffects use draw priority 21.  The
Supervisor-only effect construction at priority 1 is deliberately left as a
literal because no ordinary draw-slot identity is proved for that special
path.

Upstream evidence: GensokyoClub already proposed the complete constant table.
It was treated as a naming hypothesis, not an exact claim.  Each imported name
was reconciled with the current registered object/callback and its already
exact TH08 immediate value before replacing the literal.  The resulting named
`ChainCalcPriority` and `ChainDrawPriority` enums make the complete engine
ordering visible at every registration site.

The last anonymous `ChainElem +0x18` pointer is now `releaseTarget`.  Target
`ChainElem::ChainElem @ 0x0043C760` initializes it to self.  Target
`Chain::ReleaseSingleChain @ 0x0043CC60` writes each live node to scratch-node
`+0x18`, then reads that same field while cutting the captured nodes.  The
self value on the stack sentinel and terminal scratch node makes their cuts
no-ops.  Assertions pin `sizeof(ChainElem) == 0x20` and
`releaseTarget @ +0x18`; the name claims only this observed release-snapshot
role, not general ownership.

VC7 oracle: focused replay passes `Global.obj` **44 / 44 exact**, including the
99-byte constructor and 408-byte release routine.  Because the priority table
and layout assertions live in a shared header, the required single-job cold
build of all 75 comparison objects was run and passes **1,106 / 1,106 exact**.
The normal VC7 production image also links.

Portable oracle: the complete i386 Linux container build links all named
registrations and the fixed-layout verifier passes, including the asserted
32-byte ChainElem layout.  The semantic router remains **0 raw-member**, **0
absolute-address**, **100 anonymous-identifier**, and **41 opaque-storage**
candidates when probes and modern source are included; production-only remains
**0 / 0 / 99 / 41**.  Those residual totals remain routing hints rather than a
completion percentage.

### Supervisor startup and capture flags — 2026-08-27

Scope: three live bits in `SupervisorFlags @ Supervisor + 0x1A4`, together
with their producers in `GameWindow`, the startup thread, and their Ascii/MIDI
consumers.  The remaining anonymous flag bit 5 is outside this batch: it has
timer consumers but no observed writer, so naming it would overstate the
evidence.

Observed: `GameWindow::InitD3DRendering @ 0x004424C0` sets
`D3DPRESENTFLAG_LOCKABLE_BACKBUFFER`, then target instructions OR mask `0x2`
into `g_Supervisor + 0x1A4`.  The only consumers gate pause/retry menu capture
setup, animation, and drawing, so bit 1 is `lockableBackbuffer`.  This agrees
with the upstream GensokyoClub name, but the TH08 D3D producer/consumer chain
is the acceptance evidence.

`Supervisor::Supervisor @ 0x00445B7A` initializes masks `0x40` and `0x100`.
`GameWindow::CheckForRunningGameInstance @ 0x00443420` subsequently clears or
sets mask `0x40` from the startup-info `lpTitle` branch; the sole runtime
consumer at `Supervisor::StartupThread @ 0x004464C8` creates and starts the
`DummyMidiTimer`.  Bit 6 is therefore named narrowly as
`dummyMidiTimerEnabled`.  In the same startup thread, mask `0x100` guards the
one-time `score.dat` backup rotation and is cleared afterward, so bit 8 is
`scoreBackupPending`.

Layout: assertions pin `sizeof(SupervisorFlags) == 0x4` and the flags word at
`Supervisor + 0x1A4`.  No flag order, width, mask, branch, initialization, or
side effect changed.

VC7 oracle: after discarding the stale precompiled header, the required
single-job cold build of all 75 comparison objects passes **1,106 / 1,106
exact**.  A diagnostic focused replay of the rebuilt AsciiManager, main, and
Supervisor objects passes **134 / 134 exact**, including the 1,472-byte D3D
initializer, 625-byte startup-info producer, 70-byte Supervisor constructor,
1,391-byte startup thread, and all four menu update/draw consumers.  The normal
VC7 production image links.

Portable oracle: the complete i386 Linux container build and fixed-layout
verifier pass with the same named flags and asserted owner offset.  Router
totals remain **0 / 0 / 100 / 41** including probes and modern source, or **0 /
0 / 99 / 41** for production; the scanner intentionally does not treat every
anonymous bitfield spelling as a completion count.

### ResultScreen registration modes — 2026-08-27

Scope: `ResultScreen::RegisterChain @ 0x004582A0` and all production/probe
callers.  The previously anonymous `u32` parameter and the raw `0`, `1`, and
`2` arguments now expose one three-mode entry protocol.  The parameter remains
`u32` in the declaration and definition to preserve the original VC7 symbol
and call ABI; `ResultScreenRegistrationMode` supplies semantic values to the
body and callers only.

Observed: target instructions compare the incoming word first with `1`, then
with `2`.  Mode 0 leaves the zero-initialized state at
`RESULT_SCREEN_STATE_INIT` and installs the normal calc/draw chains, matching
the Supervisor path that enters the standalone result browser.  Mode 1 selects
high-score name entry, ordinary practice results, or spell-practice results
from the current GameManager mode before installing those chains.  Mode 2 sets
`RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE`, calls `AddedCallback` synchronously,
and returns without registering either chain.  That callback opens and writes
the score container through `DeletedCallback`; the exact callers use this path
for score initialization, pause-menu exit, and final shutdown persistence.
The names are therefore `BROWSE`, `GAME_RESULT`, and the deliberately broad
`SAVE_DATA`, rather than claiming mode 2 is used only for first-file creation.

VC7 oracle: the target-pinned 395-byte registration function replays **395 /
395 exact**.  Because the enum and declaration live in a shared header, the
required single-job cold build of all 75 comparison objects passes **1,106 /
1,106 exact** with zero failures.  The normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links every named
caller, and `verify-modern-linux.sh` verifies the ELF32 image and all fixed
target-owned layout symbols.  No state value, branch, callback, priority,
layout, target mapping, or accepted ledger entry changed.

### TitleScreen registration reasons — 2026-08-27

Scope: the `i32` argument to `TitleScreen::RegisterChain @ 0x0047146D` and its
two Supervisor call shapes.  The argument remains `i32` to preserve the target
ABI; `TitleScreenRegistrationReason` documents the standard entry value 0 and
the replay-completion return value 1.

Observed limitation: unlike ResultScreen's live mode selector, the retail
Title function only homes this fastcall argument at `[ebp-0x48]`.  No later
instruction reads it, and both values execute the identical allocation,
initialization, and calc/draw registration path.  The enum therefore names the
target-proven caller reason, not a behavioral Title mode.  The source comment
records that the callee ignores it so future work does not invent a missing
branch.

VC7 focused oracle: the rebuilt 281-byte Title registration function passes
**281 / 281 exact**, and the rebuilt 1,831-byte Supervisor dispatcher passes
**1,831 / 1,831 exact**.  Because the enum and declaration live in a shared
header, the required single-job cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures.  The normal VC7 production image
links.

Portable oracle: the complete i386 Linux container build links both named
call shapes, and the fixed-layout verifier passes.  No argument value, branch,
allocation shape, callback, priority, ABI, mapping, or accepted ledger entry
changed.

### PBG archive table metadata — 2026-08-27

Scope: `PbgArchive::ParseHeader @ 0x00474CE0`, `AllocEntries @ 0x00474FA0`,
and the two serialized record layouts in `PbgArchive.hpp`.

Observed: after decrypting the twelve-byte archive header, the retail loader
subtracts fixed biases `123456`, `345678`, and `567891`.  The three results
directly become the entry count, compressed-table file offset, and the output
size passed to `Lzss::Decode` for the file table.  The stored words are now
`encodedEntryCount`, `encodedFileTableOffset`, and
`encodedFileTableDecompressedSize`; the local decode bound is
`fileTableDecompressedSize`.  Assertions pin the header to 0x0C bytes and its
third word to +0x08.

Each decoded entry contains filename, data offset, decompressed size, and one
fourth word.  Target `AllocEntries` copies that word at entry +0x0C, but no
retail loader path reads it.  It is therefore named the deliberately neutral
`unconsumedMetadata`; no CRC, flag, timestamp, or compression meaning is
claimed.  Assertions pin the entry to 0x10 bytes and that word to +0x0C.

VC7 focused oracle: all 15 accepted `PbgArchive.obj` units pass **15 / 15
exact**, including ParseHeader **701 / 701** and AllocEntries **420 / 420**.
Because these declarations live in a shared header, the required cold build of
all 75 comparison objects was also replayed and passes **1,106 / 1,106 exact**;
the normal VC7 image links.  Portable oracle: the complete i386 Linux container
build links and its fixed-layout verifier passes.

### Enemy attachment chains and ECL laser slots — 2026-08-27

Scope: the doubly linked Enemy attachment chain at `Enemy + 0x04/+0x08`, the
32 ECL-owned Laser pointers at `+0x3280`, their selected slot at `+0x3300`,
and the primary/secondary ANM VM fields used by the same interpreter lane.

Observed: ECL opcodes 90..92 splice children through the two attachment links;
`FindLinkedChildTail`, `DetachFromParentChain`, `DetachEnemyChain`, and the EX
handlers traverse or unlink the same pointers.  They are therefore named
`previousInAttachmentChain` and `nextInAttachmentChain`, with asserted offsets.
Opcodes 114/115 store newly spawned Lasers in the selected slot, opcode 116
changes that slot, opcodes 117..121/167/170..172 operate on typed Laser members,
and opcode 154 clears all 32 pointers.  This producer/consumer set establishes
`laserSlots[32]` and `selectedLaserSlot`; assertions pin them to `+0x3280` and
`+0x3300` without changing `sizeof(Enemy) == 0x53D0`.

The same pass replaces byte views of `Enemy::vm`, its pending interrupt and
rotation, and `secondaryVms[index]` with their existing typed owners.  The
standalone RunEcl adapter also drops unused address-named API proposals and
retains only the five services its body actually calls.  Two raw forms remain
deliberately: opcode 169 keeps its layout-asserted position view because the
direct member spelling changes the target VC7 x87 compare, and owned child ECL
blocks remain byte-oriented because only offsets `+0x06`, `+0x08`, and
`+0x230` are target-proven, not their full serialized layout.

VC7 oracle: focused replay across EclRun, EclDependencies, EclExIns, and
EnemyManager passes **74 / 74 exact**; the target-pinned parent-chain cleanup
unit passes **1,336 / 1,336 bytes exact**.  Because `Enemy` is shared, the
required single-job cold build of all 75 comparison objects passes **1,106 /
1,106 exact** with zero failures.  The normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  No opcode, state transition, call ABI, mapping, accepted unit,
or target-owned layout changed.

The post-pass router reports **0 / 0 / 100 / 39** candidates including probes
and modern source, or **0 / 0 / 99 / 39** for production.  The two opaque-range
reductions are review routing only, not a whole-program completion percentage.

### Supervisor BGM unlock index — 2026-08-27

Scope: `Supervisor::PlayMusic @ 0x00447E47`, `PlayAudio @ 0x00447F21`,
their production/probe callers, and the score-data owner reached by their
second argument.

Observed: both target functions load the second stack argument as a dword and
emit `mov byte ptr [eax + 0x0164CF14], 1` after excluding replay and demo mode.
`0x0164CF14` is `g_GameManager + 0x3DA0C`, which decomposes as
`GameManager::plst @ +0x3D804` plus `Plst::bgmUnlocked @ +0x208`.  MusicRoom
copies that same array to decide which tracks are selectable, and every
nonzero PlayMusic caller supplies a stage/spell song number.  The former
`char *` declaration was therefore an artifact of expressing base-plus-index
machine code, not a pointer-bearing ABI.

The interface now names `musicIndex` and `bgmUnlockIndex` as `i32`; callers no
longer cast song numbers to pointers, and both functions write
`g_GameManager.plst.bgmUnlocked[bgmUnlockIndex]`.  Replay/demo gates use the
existing named flag bits.  Layout assertions pin both owner offsets.  VC7
still emits the same target instruction bytes, while the COFF object now
honestly records `g_GameManager + 0x3DA0C` relocations; the exact manifest
tracks those relocations and the corrected `PlayMusic(i32, i32)` decorated
symbol.

VC7 oracle: focused replay across Supervisor, GameManager, Gui, Ending,
TitleScreen, MusicRoom, and the replay-menu probe passes **189 / 189 exact**,
including PlayMusic **218 / 218** and PlayAudio **352 / 352**.  The required
single-job cold build of all 75 comparison objects passes **1,106 / 1,106
exact** with zero failures, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and the
fixed-layout verifier passes with the new `Plst` and `GameManager` assertions.
No playback branch, unlock index, return value, score-data byte, calling
convention, target address, or accepted unit changed.

### Item and ANM residual owner views — 2026-08-27

Scope: the remaining non-serialized byte views in ItemManager update,
collection, and draw paths, plus `AnmManager::AnmManager @ 0x00465070`.

The Item paths now use the already established `Player::shotTimer`,
`timeOrbGaugeChangeSuppressionTimer`, and `focusMode` owners,
`GameManager::character`, and
`AnmVm::zWriteDisabled`.  The last field is the target-observed bit 13 of
`AnmVmBase::flagsWord @ +0x1F8`; assigning the named bitfield naturally emits
the retail dword load/OR/store sequence.  The AnmManager constructor now
initializes `currentTextureFactor` directly, with a new assertion pinning that
member to `+0x24B8`.  `Player::timeOrbGaugeChangeSuppressionTimer @ +0xE2ADC`
is also asserted.

One apparent Float3 cleanup was tested and rejected.  Retail
`ItemManager::OnDraw @ 0x004415A0` calls `Float3::operator float *()` twice
before reading element 1.  Replacing those expressions with
`currentPosition.y` removes both calls and shortens the function from 451 to
434 bytes.  The conversion-shaped accesses therefore remain, now with an
exact-source-shape comment; they are not evidence that the field is unknown.

VC7 oracle: focused replay of the complete AnmManager and ItemManager objects
passes **106 / 106 exact**, including Item OnUpdate **1,989 / 1,989**,
CollectPowerSmall **334 / 334**, CollectTimeOrb **293 / 293**, Item OnDraw
**451 / 451**, and the AnmManager constructor **466 / 466**.  The required
single-job cold build of all 75 comparison objects passes **1,106 / 1,106
exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and its
fixed-layout verifier passes with both new owner assertions.  No item state,
score/gauge update, rendering flag, timer operation, constructor value, or
accepted unit changed.

### Enemy chain helper ownership — 2026-08-27

Scope: `HasParentChain @ 0x0041F000`, `HasAttachedEnemy @ 0x0041FD20`,
`CountParentChain @ 0x0041FD40`, their Enemy/ECL/GUI callers, and the integer
and float ECL operand resolvers.

The three functions had been modeled as methods of a one-byte
`EclOperands::TargetEnemyHelpersOverlay`, even though every observed `this`
value is an `Enemy *` and their complete bodies read only the asserted Enemy
attachment fields.  They now belong to `Enemy` and traverse
`parentEnemy`/`nextInAttachmentChain` directly.  Their historical placement in
the EclManager target neighborhood is translation-unit evidence, not semantic
class ownership.  The qualified identities in every mapping and exact ledger
were updated without changing an address, size, status, or accepted count.

The same pass removes unused raw resolver macros and its private vector shell.
ECL player-position operands now use `Player::position.x/y/z`, and operand
`0x2770` follows the typed Enemy attachment chain.  The temporary resolver and
detach-chain adapter described at this checkpoint was removed by the later
Enemy/ECL owner migration; both functions now belong directly to `Enemy`.

VC7 oracle: focused replay of EclManager, both operand resolvers,
EclDependencies, GUI, EnemyManager, and EnemyManagerUpdate passes **127 / 127
exact**.  The three moved helpers pass **53 / 53**, **28 / 28**, and **70 / 70
bytes exact**, while parent-chain cleanup remains **1,336 / 1,336**.  The
required single-job cold build of all 75 comparison objects passes **1,106 /
1,106 exact**, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and fixed target-owned layout
symbols.  No attachment traversal, branch, ECL operand value, call target,
calling convention, target extent, or accepted unit changed.

### Player focus, option, and gauge state — 2026-08-27

Scope: `Player::UpdateMovementAndOptions @ 0x0044AEC0`, the nine option update
callbacks at `0x0044E3A0..0x0044F4F0`, Bomb paths that reactivate options,
Enemy attachment rewards, and Item update/collection consumers.

The byte at `Player + 0x03` is now `focusMode`.  Its complete producer set uses
0 for unfocused, 1 for focused, and writes 2 only during Player initialization
so the first movement update must enter one of the two setup paths.  The same
mode selects primary/secondary SHT movement, item behavior, Bomb callback
variants, Enemy rank/gauge direction, and is copied into spawned PlayerShots.
`PlayerFocusMode` names all three observed values without changing the byte
field's ABI.

The three adjacent timers now describe their complete observed protocols.
`gaugeShiftDelayTimer @ +0xE2AD0` counts down before shooting can move the
youkai gauge and counts up to the 30-frame idle recenter threshold.
`shootingGaugeChangeRampTimer @ +0xE2AE8` is reset on focus changes and after
four idle frames; while shooting it raises the signed gauge delta up to its
cap.  `timeOrbGaugeChangeSuppressionTimer @ +0xE2ADC` is decremented by
ItemManager and is the sole gate on a collected time orb changing the gauge;
Enemy attachment reward paths set or clear that gate.  All three offsets are
asserted, and redundant casts around their existing `ZunTimer` owners are gone.

`PlayerOptionState +0x2C8` is also a uniform four-state lifecycle across every
option callback: inactive, initializing, active, and exiting.  The homing
option's `+0x2CC` behavior state distinguishes following the player, left/right
motion, and target tracking.  Named lifecycle/homing enums replace the raw
0/1/2/3 switches in production, Bomb, and probe sources; assertions pin both
dword fields.  Three unaccessed alignment ranges in Player/PlayerShot are
marked as padding, while unconsumed SHT/work-item dwords remain explicitly
unknown.

VC7 oracle: focused replay across Player, PlayerBomb, ItemManager,
EnemyManager, and EnemyManagerUpdate passes **200 / 200 exact**, including
Player movement/options **4,769 / 4,769**, all option callbacks, and Enemy
chain cleanup **1,336 / 1,336**.  The PlayerOption probe compiles under its
target profile.  The required single-job cold build of all 75 comparison
objects passes **1,106 / 1,106 exact**, and the normal VC7 image links.

Portable oracle: the complete i386 Linux container build links, and the ELF32
plus fixed-layout verifier passes.  No focus transition, option callback,
gauge formula, timer value, item reward, Enemy rank/reward branch, field width,
or accepted unit changed.

### Replay serialization and stream ownership — 2026-08-27

Scope: `ReplayManager::LoadReplayData @ 0x00451B80`, recording/playback stage
setup at `0x00452830` and `0x00452D60`, `SaveReplay @ 0x004531E0`, and the
serialized `ReplayDataHeader`, `ReplayData`, and `StageReplayData` layouts.

The target writer and loader establish two parallel per-stage streams.  The
first pointer table owns a `StageReplayData` prefix followed by input records,
so its tail is now `inputStream`.  The second table is copied and advanced in
single bytes and is therefore `u8 *stageFpsData[MAX_STAGES]`, rather than a
second table of fictitious `StageReplayData` objects.  Recording/playback local
pointers and save-time size arithmetic use those owners directly.  Header and
payload gaps that are covered by the initial whole-`ReplayData` zero fill and
have no target consumer are named reserved storage.  The payload dword at
`+0x120` is written as 30 immediately before serialization but has no observed
reader, so it remains the deliberately neutral `unconsumedConstant30` rather
than receiving a guessed gameplay meaning.  The manager word at `+0x4E` is
likewise named only for its proven stage-start reset protocol.

Evidence limits remain explicit.  The four `Float3` objects at manager offsets
`+0x18`, `+0x24`, `+0x30`, and `+0x3C` have no authored scalar consumers that
would justify behavioral names.  They cannot be collapsed into byte storage:
that trial removed four implicit constructors and shortened the retail
constructor shape from 58 bytes to 14.  They therefore remain honest
`unknownVector18/24/30/3C` objects.  The unconsumed manager ranges at `+0x48`,
`+0x7C`, and `+0xCC` also stay opaque; in particular, no unsupported parallel
end-pointer array is inferred for `+0x7C`.

VC7 oracle: focused replay of `ReplayManager.obj` passes **18 / 18 exact**,
including LoadReplayData **511 / 511**, BeginRecordingStage **1,315 / 1,315**,
BeginPlaybackStage **787 / 787**, the constructor **58 / 58**, and SaveReplay
**2,445 / 2,445**.  Because the serialized layouts are shared headers, the
required single-job cold build of all 75 comparison objects passes **1,106 /
1,106 exact** with zero failures, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  The Replay two-file router falls from 14 anonymous identifiers
plus 5 opaque ranges to 1 alignment identifier plus 3 evidence-limited opaque
ranges; that is a review-routing delta, not a semantic completion percentage.
No replay byte, pointer fixup, stream extent, checksum/obfuscation operation,
constructor call, field offset, or accepted unit changed.

### Presentation and audio residual-state classification — 2026-08-27

Scope: ANM frame diagnostics and serialized records, Ascii gauge VMs, the
DirectSound queue metadata lane, and the MIDI output object's remaining
constructor/reset-only fields.

`AnmManager + 0x0C` is now `scriptsStartedThisFrame`: every successful
`AnmLoaded::SetAndExecuteScript` increments it and `ResetFrameDebugInfo` clears
it alongside the established execution/render/flush counters.  The ANM VM at
`+0x1C64` has no observed consumer and is therefore `unconsumedVm1C64` rather
than being assigned a speculative rendering role.  Unconsumed fields in the
on-disk `AnmRawEntry`/`AnmTextureHeader` records and `AnmLoadedSprite` likewise
retain neutral offset-bearing names, with their sizes and offsets asserted.

Ascii script 9 is loaded into `auxiliaryGaugeVm @ +0x1520` and executed every
frame.  That establishes its ownership by the gauge group, but not which
specific artwork it represents; the name deliberately stops at the strongest
available evidence.  The dword at `+0x829C` is only zeroed by `Reset`, so it is
classified as `resetOnlyState829C`.  Sound-buffer table metadata is copied to
a per-sound array when a queue request is accepted but has no authored reader,
and the startup BGM attenuation curve is written without a current consumer.
Both lanes are now explicitly `unconsumed` instead of pretending their values
affect the reconstructed playback path.

The MIDI byte array at `+0x144` is structurally per-channel, while the remaining
dwords have no evidence-backed purpose.  The field at `+0x2DC` is cleared by
construction, track loading, and fade-out setup, which supports only the
`resetOnlyFadeState2DC` protocol.  No MIDI event, channel meaning, or fade
calculation is inferred from those writes.

VC7 oracle: focused replay of AnmManager, AsciiManager, AsciiManagerGauge,
SoundPlayer, Midi, and Supervisor passes **259 / 259 exact**.  Because these
changes touch shared layouts, the required single-job cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and fixed target-owned layout
symbols.  The heuristic debt router now reports zero raw-member accesses and
zero absolute-address field views repository-wide; its remaining unknown and
opaque entries are evidence-routing leads, not failed exact units or a semantic
completion percentage.

### Supervisor and residual layout-state classification — 2026-08-27

Scope: the serialized `GameConfiguration` tail, Supervisor timer and title-entry
state, `RawStageHeader`, Background alignment, and the remaining neutral fields
in ScreenEffect and Spellcard.

The 60-byte configuration record is copied from disk and written back as one
complete aggregate.  Its unreferenced `+0x29..+0x37` bytes are therefore named
`serializedReserved29`, without assigning them an option meaning.  Assertions
pin the record size, reserved range, and `opts @ +0x38`.  Supervisor flag bit 5
is read only by `ZunTimer::Increment` and `Decrement`, where it first advances
one whole frame and resets fractional history before the ordinary timer step.
It is now `forceExtraTimerStep`.  The ECL scaled-time exit path's direct
`g_EclGameTimeScaleFlags` access still retains its target-visible global
identity, but its bit-5 write aliases the same target word and independently
corroborates the timer behavior.

Target `Supervisor::ClearRecordingFpsWarningState @ 0x00470DD5` clears the
recording warning and four otherwise unconsumed dwords in the exact observed
order `+0x338,+0x340,+0x34C,+0x344,+0x348`.  The former anonymous method and
all mapping, implementation, accepted-match, relocation, and decorated-symbol
identities moved together.  The four trailing fields stop at the strongest
available evidence as `resetOnlyDword*`; the unused `+0x170` dword and
`+0x304..+0x337` range remain explicitly `unconsumed`.  The two-byte gap after
`recordedFps` is ordinary alignment and is now compiler-owned, with
`textAnm @ +0x19C` asserted.

The on-disk stage header dword at `+0x0C` has no authored consumer and is named
`unconsumedDword0C`.  Four three-byte Background gaps are proven by the
following four-byte-aligned members and are now implicit C++ padding rather
than fictitious unknown state; the following timer, spell state, tint, and
special-effect offsets remain asserted.  `ScreenEffect +0x0C` and
`Spellcard +0x044..+0x073` likewise remain neutral `unconsumed` storage.  In
particular, adjacent-game source and TH08 call sites do not prove that the
Spellcard range contains an owner name: TH08 decodes the owner into a local
buffer and copies it directly into CATK instead.

VC7 oracle: focused replay across Supervisor, TitleScreen, Background,
ScreenEffect, SpellCard, and EclExIns passes **188 / 188 exact**, including the
renamed Supervisor clear helper at **59 / 59**.  Because the batch changes
shared PCH layouts, the required single-job non-reuse cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  The review-only semantic router moves from 54 to 40 anonymous
identifiers and from 37 to 32 opaque ranges; these are routing deltas, not a
semantic-completion percentage.  No configuration byte, timer branch, field
width, serialized extent, aggregate size, target address, or accepted-unit
count changed.

### Bullet transform payload and residual storage closure — 2026-08-27

Scope: `Bullet::AdvanceTransformProgram @ 0x0042FFC0`, ECL opcode 111 inside
`EclManager::RunEcl @ 0x004184B0`, the shot descriptor producer at
`0x00422720`, bullet spawn/setup at `0x0042F5F0` and `0x00433070`, and the
shared Bullet/Laser layouts.

Opcode 111 still writes one generic four-value transform payload because its
meaning is selected later by the record's `kind`.  That on-wire/raw view is now
one member of `BulletTransformPayload`; `AdvanceTransformProgram` uses tagged
views for vector acceleration, polar acceleration, direction changes, boundary
bounces, timed wrap/wait, cull delay, sprite selection, positioned sound, and
the two-record child-pattern descriptor.  The role-specific names come from
the exact consumers and retain the same four 32-bit slots.  This supersedes the
earlier decision to expose `float0/float1/int0/int1` at every consumer while
preserving those names only at the generic ECL write boundary.

The remaining Bullet-family bytes stop at observed behavior.  Descriptor word
`+0x1FA` is explicitly cleared before spawn but has no reader and is therefore
`unconsumedWord1FA`.  `BulletTypeSprites +0xD40` is copied from template to
live bullet but has no producer beyond aggregate zeroing; `Bullet +0xDBC` is
set to one at spawn and has no consumer.  They are now
`unconsumedTemplateByteD40` and `unconsumedSpawnMarkerDBC`, not guessed flags.
The template's `+0xD41` byte is independently written from the loaded sprite
height and is now `spriteHeightPx`.  Unaccessed Bullet dword ranges retain
their exact extents as `unconsumed` storage, while the one- to three-byte
structure tails are explicitly named alignment.  The constructor-visible
`Float3 @ +0xD5C` remains typed and is merely renamed `unconsumedVectorD5C`.

Layout assertions pin the 0x10-byte tagged payload, 0x18-byte transform record,
0x210-byte spawn descriptor, 0xD44-byte sprite template, 0x59C-byte Laser, and
0x10B8-byte Bullet together with every newly classified offset.  No field
width, constructor-bearing member, array count, or aggregate extent changed.

VC7 oracle: focused replay passes **94 / 94 exact** across BulletManager,
EclDependencies, and EclExIns, and the separately rebuilt EclRun owner passes
**1 / 1 exact**, including the complete code-plus-table `RunEcl` comparison.
The required single-job non-reuse cold build of all 75 comparison objects then
passes **1,106 / 1,106 exact** with zero failures, and the normal VC7
production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  `BulletManager.hpp/.cpp` now has zero candidates in all four
review-router categories; repository-wide anonymous/opaque routing moves from
40/32 to 30/25.  Those counts are review aids, not completion percentages.  No
transform opcode, branch, sound, spawn pattern, target byte, relocation,
accepted-unit identity, or exact total changed.

### GUI and Ending residual-state closure — 2026-08-27

Scope: the final byte of `GuiMsgVm`, the otherwise unconsumed Boss-HUD dword
at `Gui +0x38`, and the otherwise unconsumed Ending dword at `+0x2A98`.

The target-pinned `GuiMsgVm::GuiMsgVm @ 0x00437CE2` constructs its timer and
three ANM-VM arrays through `+0x156E`; the following byte is the natural
four-byte-alignment tail of the `0x1570` aggregate.  It is now compiler-owned
padding rather than a fictitious field, while an assertion pins the last real
message member, `selectedOption @ +0x156E`.  `Gui::RegisterChain @ 0x00437AD0`
clears all `0x9C` GUI bytes with 39 dword stores on initial-stage load.  No
exact GUI producer or consumer independently accesses the dword at `+0x38`,
so it is retained as `unconsumedDword38` between the displayed life-bar size
and the asserted segment arrays without assigning a Boss-HUD meaning.

`Ending::Ending @ 0x004297B0` constructs the sixteen ANM VMs and three timers,
then clears all `0x2AB8` bytes with 2,734 dword stores before restoring its
observed timer/default state.  The exact ending-script and callback corpus has
no independent access to `+0x2A98`; it therefore remains
`unconsumedDword2A98` between the asserted line-wait thresholds and next text
VM index.  These classifications describe only observed aggregate treatment
and absence of authored scalar consumers; they do not invent latent format or
runtime roles.

VC7 oracle: target-fact packets for the GUI message constructor, GUI chain
registration, Ending constructor, and Ending update all report exact against
their canonical units.  Focused replay of `Gui.obj` and `Ending.obj` passes
**52 / 52 exact** before and after the edit.  Because both layouts are shared
through the PCH, the required single-job non-reuse cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  `Gui.hpp` and `Ending.hpp` now have zero review-router
candidates; repository-wide anonymous/opaque routing moves from 2/6 to 1/4.
Those counts remain evidence-routing aids, not proof of whole-program semantic
completion.  No dialogue, Boss HUD, ending-script behavior, target byte,
accepted-unit identity, or exact total changed.

### Enemy residual layout-state classification — 2026-08-27

Scope: the shared `Enemy` and `EnemyManager` layouts and their exact consumers
in EnemyManager, EnemyManagerUpdate, EclRun, EclDependencies, EclExIns, Gui,
and SpellCard.

The unassigned low bit and upper 23 bits of `EnemyFlag2Bits` have no authored
reader or writer beyond whole-word flag operations, so they remain neutral
`reserved` bits rather than receiving speculative gameplay meanings.  The
two words at `Enemy +0x2CEC` and `+0x332C`, and the manager dwords at
`+0x9DCDC8` and `+0x9DCEF4`, likewise have no independent producer or consumer
beyond aggregate clearing/copying and are now explicitly `unconsumed` storage.

The ranges at `Enemy +0x2D32`, `+0x3315`, `+0x3331`, `+0x333E`, and `+0x534D`
are structural alignment before the following position, timer, ANM-script,
movement-bound, and trail-length fields.  They remain explicit byte arrays
because the target-visible Enemy aggregate-copy source shape depends on those
bytes being members.  Offset assertions pin every classified range and all
following semantic fields; no width, bit position, aggregate extent, or copy
boundary changed.

VC7 oracle: focused replay across the seven affected comparison objects passes
**171 / 171 exact**.  Because `EnemyManager.hpp` is shared through the PCH, the
required single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures, and the normal VC7 production image
links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  `EnemyManager.hpp` now has zero candidates in all four
review-router categories; repository-wide anonymous/opaque routing moves from
30/25 to 29/16.  Those counts remain review aids, not completion percentages.
No enemy behavior, ECL dispatch, target address, relocation, accepted-unit
identity, or exact total changed.

### ANM serialized and residual storage closure — 2026-08-27

Scope: the on-disk `AnmRawEntry` and `AnmTextureHeader` records, the runtime
`AnmLoadedSprite` tail, `AnmVm` tail, and residual `AnmManager` storage.

The ANM entry dword at `+0x3C` and texture-header words at `+0x04`, `+0x0C`,
and `+0x0E` enter memory as part of their complete serialized records but have
no authored consumer.  They are therefore named `serializedReserved*` without
assigning unsupported format meanings.  `AnmLoadedSprite +0x40` is carried by
the temporary sprite aggregate into the loaded array but has no reader, so it
remains an explicitly `unconsumedDword40` rather than a guessed render flag.

`AnmVm +0x29A..+0x2A3` and `AnmManager +0x1F08..+0x2037` are covered by their
owners' complete zeroing but have no independent producer or consumer.  Their
names now state that evidence boundary.  The two bytes at manager
`+0x24C6..+0x24C7` are proven structural padding before the four-byte-aligned
`currentSprite @ +0x24C8`; compiler-owned alignment reproduces them without an
invented member.  Size and offset assertions pin every record, residual range,
following surface array, render-state byte, and pointer.  No serialized extent,
aggregate size, pointer offset, or constructor-visible zeroing range changed.

VC7 oracle: focused replay of `AnmManager.obj` passes **82 / 82 exact** before
and after the change.  Because the layout is shared through the PCH, the
required single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures, and the normal VC7 production image
links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  `AnmManager.hpp` now has zero candidates in all four
review-router categories; repository-wide anonymous/opaque routing moves from
29/16 to 19/13.  Those counts are review aids, not completion percentages.  No
ANM decoding, texture creation, sprite loading, render path, target byte,
accepted-unit identity, or exact total changed.

### Player residual storage and alignment closure — 2026-08-27

Scope: the serialized SHT header, collision/option/Bomb/shot records, and the
remaining neutral bytes in the shared `Player` layout.

`PlayerRawShtFile +0x00` and `+0x20` enter memory with the complete SHT file
and have no authored consumer, so they are now `serializedReserved00/20`
without format guesses.  The dword at `PlayerOptionState +0x2D4`, the Bomb
dword at `PlayerBombState +0x0C`, and `Player +0xE2B20` likewise have no
independent producer or reader and remain explicitly `unconsumed`.  Player
byte `+0x02` is written to one by `AddedCallback` but never read; its name now
records that limited protocol as `unconsumedAddedMarker02`.  Byte `+0x07` has
no independent access and remains neutral.

The collision-region tail `+0x3E..+0x3F`, PlayerShot bytes `+0x46D` and
`+0x471..+0x473`, and Player bytes `+0xE2A95..+0xE2A97` are completely
determined by the following structure boundary, `i16`, callback-pointer, and
dword-enum alignments.  They are now compiler-owned padding.  Existing and new
assertions pin the preceding fields, following semantic members, each neutral
dword, and every aggregate size.  No serialized offset, array stride, callback
ABI, constructor sequence, or object extent changed.

VC7 oracle: focused replay of Player and PlayerBomb passes **136 / 136 exact**
before and after the edit.  Because `Player.hpp` is shared through the PCH, the
required single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures, and the normal VC7 production image
links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  `Player.hpp` now has zero candidates in all four review-router
categories; repository-wide anonymous/opaque routing moves from 19/13 to
16/8.  Those counts are review aids, not completion percentages.  No movement,
collision, option, Bomb, shot, target byte, accepted-unit identity, or exact
total changed.

### ECL and Effect residual-state closure — 2026-08-27

Scope: shared Effect/EffectManager state, ECL raw-instruction views, the
per-Enemy interpreter context, ECL extension barrier state, and the otherwise
unconsumed timeline-state prefix.

The dwords at `Effect +0x31C/+0x344`, `EffectManager +0x04`, barrier state
`+0x04`, and interpreter context `+0x21C` have no independent authored
producer or consumer beyond their owners' aggregate clearing/copying.  They
are now explicitly `unconsumed`, preserving the evidence boundary recorded by
the earlier Effect reconstruction.  The timeline-state prefix
`+0x000..+0x0FF` is likewise unconsumed; its eight following vectors remain
separately typed at `+0x100`.

Every exact raw-instruction view now calls byte `+0x08`
`serializedReserved08`: opcode dispatch consumes the adjacent difficulty mask
and operand flags but never that byte.  The ECL extension view also retains
its skipped `+0x0C..+0x0F` slot as `serializedReserved0C`, before the arguments
actually consumed at `+0x10`.  No file-format meaning is inferred.  Context
tail `+0x226..+0x227` remains an explicit alignment array rather than implicit
padding because complete context assignment is target-visible in the ECL call
stack.  Assertions pin every neutral range, following semantic field, context
size, timeline-state size, and manager extent.

VC7 oracle: focused replay across EclManager, EclRun, EclDependencies,
EclExIns, and EffectManager passes **128 / 128 exact** before and after the
edit, including the complete RunEcl code-plus-table unit.  Because the shared
header reaches the PCH, the required single-job non-reuse cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  The four affected ECL source views now have zero candidates in
all review-router categories; repository-wide anonymous/opaque routing moves
from 16/8 to 8/6.  Those counts are review aids, not completion percentages.
No opcode selection, operand resolution, interpolation, callback, Effect
behavior, target byte, accepted-unit identity, or exact total changed.

### GameManager residual-state and alignment closure — 2026-08-27

Scope: the six evidence-limited fields retained after the GameManager runtime,
setup, score, replay, frame-counter, and stage-transition reconstruction.

Bytes `+0x3DBB6..+0x3DBB7` align `demoFrameCount @ +0x3DBB8`; bytes
`+0x3DDBE..+0x3DDBF` align `gameplayFrameCounter @ +0x3DDC0`; and bytes
`+0x3DDD2..+0x3DDD3` align `arcadeRegionTopLeftPos @ +0x3DDD4`.  These are now
compiler-owned padding rather than anonymous state.  The dwords at
`+0x3DDCC` and `+0x3DE0C` have no independent authored producer or consumer
beyond whole-manager clearing and remain explicit `unconsumed` storage.

Assertions pin both sides of every alignment range, both neutral dwords, the
replay filename/seed boundary, current-stage clear flag, playtime/frame-skip
counters, and all following active/extreme-state counters.  No replay buffer,
RNG seed, stage state, counter width, Float2 alignment, or manager extent
changed.

VC7 oracle: focused replay of `GameManager.obj` passes **42 / 42 exact** before
and after the edit, including OnUpdate, GameplaySetupThread, and
InitializeScoreData.  Because `GameManager.hpp` is shared through the PCH, the
required single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures, and the normal VC7 production image
links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  `GameManager.hpp` now has zero candidates in all four
review-router categories; repository-wide anonymous/opaque routing moves from
8/6 to 2/6.  Those counts are review aids, not completion percentages.  No
gameplay state transition, stage setup, replay behavior, target byte,
accepted-unit identity, or exact total changed.

### Replay residual storage and router closure — 2026-08-27

Scope: `StageReplayData +0x23`, the four constructor-visible vectors at
`ReplayManager +0x18..+0x47`, and the otherwise unconsumed manager ranges at
`+0x48..+0x4D`, `+0x7C..+0x9F`, and `+0xCC..+0xCF`.

The stage byte at `+0x23` is carried in the serialized stage prefix between
`clockTime` and the input stream but has no authored consumer.  It is now
`serializedReserved23`, not generic padding: the following byte stream has
one-byte alignment, so this position belongs to the wire record rather than
compiler alignment.  Its offset and the adjacent serialized fields are
asserted.

The target-pinned `ReplayManager::ReplayManager @ 0x00453160` is exactly 58
bytes and calls `Float3::Float3` four times with receivers at `+0x18`,
`+0x24`, `+0x30`, and `+0x3C`.  Those type identities and implicit constructor
calls are retained while the fields become neutral
`unconsumedVector18/24/30/3C`: the authored corpus still provides no scalar
producer or consumer that would support behavioral names.  The three byte
ranges likewise have no independent access beyond the manager-wide clear and
are now explicitly `unconsumedBytes48/7C/CC`; `+0x7C` is deliberately not
misrepresented as an unsupported parallel end-pointer array.

`ReplayManager::RegisterChain @ 0x00451F90` remains exactly 777 bytes.  Its
target sequence allocates `0xDC` bytes, invokes the constructor above, then
clears the complete manager with 55 dword stores before assigning the known
replay and chain state.  Offset assertions pin every residual range and all
following cursors or chain pointers.  With no remaining declaration using it,
the generic `unknown_fields` macro is removed from `utils.hpp`; the named
arrays now expose both the evidence boundary and their owning aggregates.

VC7 oracle: focused replay of `ReplayManager.obj` passes **18 / 18 exact**
before and after the edit, including the constructor at **58 / 58** and
RegisterChain at **777 / 777**.  Because `ReplayManager.hpp` and `utils.hpp`
are shared through the PCH, the required single-job non-reuse cold build of
all 75 comparison objects passes **1,106 / 1,106 exact** with zero failures,
and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  The source-wide heuristic router now reports zero candidates
in raw-member access, absolute-address view, anonymous-identifier, and opaque-
storage categories.  This closes the known offset/opaque-layout routing
milestone; it is not proof that every function or evidence-limited field has a
recoverable semantic name.  The sole authored non-accepted function remains
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0` (361 target bytes
versus the current natural 362-byte emission), tracked separately from this
layout closure.  No replay behavior, constructor call, target byte, accepted-
unit identity, or exact total changed.

### Effect camera-particle callback naming — 2026-08-27

Scope: the six remaining authored address-named Effect callbacks at
`0x00426280`, `0x004264F0`, `0x00426720`, `0x00426990`, `0x00426D70`, and
`0x00426E70`, plus their target and portable template-table identities.

The Effect template table supplies three exact initializer/update pairs.
Template slot 51 (ANM script 73) binds the `0x426280/0x4264F0` pair: its
initializer seeds a randomized camera-relative position, velocity and
acceleration, while its updater integrates that motion, rejects particles
outside the forward view cone, smooths `vm.pos2` toward the damageable Boss,
and multiplies the secondary color by the current stage-text tint.  These are
now `Initialize/UpdateTintedBossTrackingCameraParticle`.

Template slot 63 (ANM script 74) binds `0x426720/0x426990`.  It uses the same
camera-relative protocol with a directly observed positive vertical-velocity
base and the Boss-target smoothing path, but without the stage tint.  The pair
is therefore `Initialize/UpdateRisingBossTrackingCameraParticle`.  Template
slot 19 (ANM script 51) binds initializer `0x426E70` to updater `0x426D70`; the
initializer randomizes camera-relative motion and Z rotation, and the updater
integrates motion, advances rotation, and retires the particle at its view/Z
boundaries.  That pair is `Initialize/UpdateSpinningCameraParticle`.  The
names intentionally stop at proven motion, tracking and color behavior; no
unobserved texture subject such as leaf, snow, or spark is inferred.

The semantic identities move together through `EffectManager.cpp`, the Linux
template mirror, mapping/reccmp/implemented/matches ledgers, and all six VC7
decorated-symbol match units.  Match-unit IDs retain their stable address-
bearing CLI names, while their selected COFF symbols now use the readable C++
identities.

VC7 oracle: focused replay of all configured `EffectManager.obj` units passes
**52 / 52 exact** after the migration; the six renamed functions individually
retain their accepted extents of 624, 545, 620, 388, 251, and 587 bytes.  The
required single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures, and the normal VC7 production
image links.

Portable oracle: the complete i386 Linux container build links with the same
three template pairs, and `verify-modern-linux.sh` verifies the ELF32 image
and every fixed target-owned layout symbol.  No Effect motion, Boss test,
color operation, table slot, target address, relocation target, accepted-unit
identity, or exact total changed.

### Neutral identifier closure — 2026-08-27

Scope: the remaining production-source identifiers whose `unknown`/`unk`
spelling concealed an already bounded evidence class in AsciiManager,
Background, Midi, SoundPlayer, ScoreDat, ResultScreen, and zwave.  This pass
also pins the public or serialized layouts touched by the new names.

The Ascii popup dword at `+0x34`, SoundPlayer dwords at `+0x04`, `+0x61C`,
and `+0x5210`, and DummyMidiTimer dword at `+0x10` have no scalar consumer
beyond their enclosing initialization or otherwise observed storage role.
They are therefore named `unconsumed...`, without inventing gameplay meaning.
Midi's per-channel bytes at `+0x144` likewise have no authored access, while
the dwords at `+0x2D0` and `+0x2D8` are distinguished as
`constructorCleared...` because that is the only observed operation.

ScoreDat header byte `+0x00` is carried through the serialized header but does
not participate in the checksum, custom XOR transform, or post-decrypt
protocol.  It becomes `unconsumedHeaderByte00`, rather than an unsupported
magic/version claim.  The raw stage-object byte at `+0x02`, between the object
ID and position, is a reserved wire slot and becomes `serializedReserved02`.
Offset assertions pin both records and their adjacent fields.

ResultScreen's local performance rating still performs the target-observed
score, difficulty, completion, retry, death, bomb, spell, slowdown, item, and
graze adjustments, but its final value has no consumer; its name now states
that boundary.  The two zwave seek results are likewise stored exactly as in
the target but never tested, so they become `unconsumedSeekResult` rather than
suggesting meaningful error handling.

VC7 oracle: focused cold replay of the seven affected comparison objects
passes **238 / 238 exact** before and after the edits.  The required
single-job non-reuse cold build of all 75 comparison objects passes
**1,106 / 1,106 exact** with zero failures, and the normal VC7 production
image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  Production source now has no `unknown*`, `unk*`, or authored
address-style `FUN_*` identifiers; the two remaining `FUN_` strings are
target-name comments beside already semantic ECL functions.  The four-category
semantic router remains zero.  Neutral `unconsumed`, `serializedReserved`, and
`constructorCleared` names are deliberately retained wherever the corpus does
not justify a stronger behavioral identity, so this is an identifier-closure
checkpoint rather than a claim that every recoverable program concept has
already been named.

### Item spawn-state protocol — 2026-08-27

Scope: the remaining unknown ItemType/ItemState identities and every authored
constant `ItemManager::SpawnItem` call across ItemManager, Player, Spellcard,
BulletManager, EnemyManager, ECL high instructions, and ECL extended
instructions.

Target-pinned packets for `ItemManager::SpawnItem @ 0x004400A0` and
`ItemManager::OnUpdate @ 0x00440500` confirm the exact state values and the
following transitions.  State 2 is produced only by
`Player::UpdateDeathAndRespawn`: it records the death position, chooses a
random playfield target, interpolates there for 60 frames, and then enters the
default motion path.  It is now `ITEM_STATE_DEATH_DROP_SPREAD`.

Every `ITEM_TIME` request is normalized by SpawnItem to state 3.  That state
starts with randomized upward velocity, remains non-collectible while active,
and switches to auto-collection when its vertical velocity passes the apex or
the player's 20-frame shot cycle becomes inactive.  It is now
`ITEM_STATE_TIME_RISING`.  Item type 10 is not retained in the spawned Item:
it is a request code that is immediately converted to `ITEM_TIME` and state 5.
State 5 advances without pickup while rising and enters auto-collection only
after the apex, so the request and state are now
`ITEM_TIME_APEX_AUTOCOLLECT_REQUEST` and
`ITEM_STATE_TIME_RISING_TO_APEX`.

No authored producer or dedicated branch uses item-type slot 9 or state slot
4.  They remain explicitly `ITEM_RESERVED_9` and `ITEM_STATE_RESERVED_4`;
this does not claim that data-driven ECL can never supply those numeric values.
All constant SpawnItem call sites now use `ItemType` and `ItemState` names.
Runtime item types sourced from ECL operands, enemy drop schedules, and bullet
cancel configuration retain explicit casts because their values are data, not
compile-time semantic identities.

VC7 oracle: focused cold replay of the eight affected comparison objects
passes **244 / 244 exact** before and after the edit, including SpawnItem at
**970 / 970** and OnUpdate at **1,989 / 1,989**.  Because ItemManager.hpp is a
shared header, the required single-job non-reuse cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links with the same
state transitions, and `verify-modern-linux.sh` verifies the ELF32 image and
every fixed target-owned layout symbol.  No item value, state value, call ABI,
drop count, movement step, collection gate, target byte, accepted-unit
identity, or exact total changed.

### Player collision-region parameter protocol — 2026-08-27

Scope: the public declarations and exact bodies of the four Player collision-
region allocators at `0x0044DE60`, `0x0044DF00`, `0x0044DFA0`, and
`0x0044E040`.

The existing typed bodies provide direct ownership evidence for every formerly
generic `value1..value4` parameter.  Rectangular helpers copy `width` and
`height` into `size.x/y`; circular helpers copy `initialRadius` and
`radiusGrowthPerFrame` into `radius/radiusGrowth`.  Damage-pool allocators
write `damage`, while cancel-pool allocators write `collisionValue`, and all
four write `lifetime`.

The argument order is intentionally not normalized: circle-cancel receives
`lifetime` before `collisionValue`, whereas circle-damage receives `damage`
before `lifetime`.  The declarations now expose those target-observed orders
to every PlayerBomb, Enemy, and option caller without changing the parameter
types or decorated VC7 symbols.

VC7 oracle: target-pinned packets pass for all four addresses.  Focused cold
replay of `Player.obj` passes **77 / 77 exact** before and after the edit, with
each allocator retaining **153 / 153** bytes.  Because Player.hpp is shared,
the required single-job non-reuse cold build of all 75 comparison objects
passes **1,106 / 1,106 exact** with zero failures, and the normal VC7
production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  No call ABI, argument order, pool selection, field write,
target byte, accepted-unit identity, or exact total changed.

### Shared helper parameter closure — 2026-08-27

Scope: seven exact helper bodies and their public declarations across
AsciiManager, TextHelper, Supervisor, ScreenEffect, and TitleScreen.

AsciiManager's time/familiar popup encoders consume two independent integer
digit sequences.  Their former `number/param3` names become the evidence-
bounded `primaryNumber/secondaryNumber`; the second sequence's delimiter
sprites are preserved without guessing a stronger UI label.  ScreenEffect's
public declaration now agrees with its already typed implementation:
`durationFrames`, three variant-owned raw parameters, and `drawPriority`.

Supervisor's music loader passes its integer to either MidiOutput::ReadFileData
or the WAV preload command and is therefore `preloadSlot`.  FadeOutMusic's
float is converted to milliseconds for MIDI and frame-rate-adjusted for the
WAV command, establishing `durationSeconds`.  TextHelper::InvertAlpha exposes
its ignored first coordinate as `unusedX` and its A1R5G5B5 quarter-strength
color-decay selector as `useGentleColorFalloff`; the byte-count, byte-offset,
adjusted-channel, and format-index locals replace decompiler-style names.
The A8R8G8B8 and A4R4G4B4 alpha-inversion paths remain independent of that
selector.  DrawPieChart uses one float to scale the angular sweep and the
other as twice the radial offset, now `fraction` and `diameter`.

VC7 oracle: target-pinned packets pass at `0x00403460`, `0x00403600`,
`0x0043F31C`, `0x00447D56`, `0x004480F8`, `0x0045B8B0`, and `0x0046FDC2`.
Focused cold replay of the five affected objects passes **178 / 178 exact**
before and after the edit.  The individual extents remain 411, 420, 1,292,
241, 183, 628, and 510 bytes respectively.  Because four declarations live
in shared headers, the required single-job non-reuse cold build of all 75
comparison objects passes **1,106 / 1,106 exact** with zero failures, and the
normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  No parameter type/order, encoded digit, color decay, fade
duration, effect variant, draw priority, pie geometry, target byte, accepted-
unit identity, or exact total changed.

### Background interpolation, MIDI events, and spell portrait auxiliaries — 2026-08-27

Scope: the exact Background interpolation/registration helpers, MidiOutput's
timer-driven event decoder and fade-volume sender, and the two residual
offset-named Spellcard portrait VMs.

Background's mode-7 interpolation is the standard cubic Hermite basis.  The
four former `valueN` inputs are now the start/end values and start/end
tangents, consistent with the camera interpolation fields and all three
component call sites.  `Background::RegisterChain` receives
`GameManager::currentStage` at both production setup sites and stores it in
`registeredStage`; its parameter is therefore `stageIndex`.

MidiOutput's playback tick, status byte, message type, channel, two event data
bytes, meta-event type, event data length, next delta ticks, loop checkpoint
and reset cursors, and adjusted channel volume now describe the parser's
observed MIDI roles.  FadeOutSetVolume sends controller 7 on status `0xB0 +
channel`; those literals are expressed as the channel-volume controller and
control-change status enums, with the incoming additive term named
`volumeOffset`.

Spellcard offsets `+0x90C` and `+0xE54` are executed beside the enemy portrait
and drawn only while that portrait is visible.  They become
`enemyPortraitAuxNoRotationVm` and `enemyPortraitAux2dVm`, respectively.  The
names deliberately stop at target-observed context and draw path: no script is
assigned to either VM in the recovered TH08 code, so no asset-specific role is
claimed.

VC7 oracle: target-pinned packets pass at `0x00408D60`, `0x00408FC0`,
`0x00409B20`, `0x00414590`, `0x00416B90`, `0x004178C0`, `0x00444820`,
`0x00444A90`, and `0x00445340`.  Focused post-edit replay of Background.obj,
Midi.obj, and SpellCard.obj passes **94 / 94 exact**.  Their covered function
extents remain 577, 152, 254, 3,237, 3,269, 1,685, 619, 1,871, and 187 bytes.
Because the changes touch shared headers, the required single-job non-reuse
cold build of all 75 comparison objects passes **1,106 / 1,106 exact** with
zero failures, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  No class size/offset, parser branch, running-status behavior,
loop cursor, volume byte, camera interpolation, stage selection, target byte,
accepted-unit identity, or exact total changed.

### Chain release, controller input, result drawing, and audio queues — 2026-08-27

Scope: six exact runtime helpers across Global.cpp, ResultScreen.cpp, and
SoundPlayer.cpp whose remaining local names came directly from decompiler
placeholders or stack-matching experiments.

Chain::ReleaseSingleChain constructs a temporary snapshot list so callbacks
may unlink the real calc/draw chain without invalidating its traversal.  Its
stack object and cursors are now `releaseSnapshotHead`,
`releaseSnapshotCursor`, and `nextSnapshotEntry`.  Controller input now names
the WinMM joystick state, per-axis deadzone, shot/focus conflict result,
DirectInput state, and HRESULT path.  GetControllerState uses the same
vocabulary.  It intentionally preserves a target-observed original quirk:
GetDeviceState's HRESULT is discarded and the following check reuses the
previous Poll/Acquire result.

The old identifiers had also been accidental VC7 code-generation inputs.
Target stack evidence established explicit `#pragma var_order` contracts for
the release snapshot and controller state, separating readable source names
from their required physical homes.  The first readable build exposed this
dependency by failing the focused Oracle; the corrected order restores the
original 0x54-byte release frame and 0x160-byte controller frame without
reintroducing opaque names.

ResultScreen::OnUpdate now distinguishes the VM receiving the exit interrupt
from the normal sprite-script iterator.  Its draw callback names the keyboard
column, animated glyph position/value, and vertical glyph offset.  The reused
animation scalar remains one source local because VC7 and the target use the
same home first for scale and then for horizontal offset.  SoundPlayer's queue
state machine now exposes immediate command restart, preload/reopen buffers,
BGM format lookup, queued SFX index, averaged positional pan, and nonlinear
volume scale.

VC7 oracle: target-pinned packets pass at `0x0043CC60`, `0x0043D120`,
`0x0043D7E0`, `0x004584B0`, `0x004586B4`, and `0x0045D790`.  Focused replay of
Global.obj, ResultScreen.obj, and SoundPlayer.obj passes **106 / 106 exact**.
The six function extents remain 408, 1,457, 392, 424, 3,993, and 2,358 bytes.
The required single-job non-reuse cold aggregate passes **1,106 / 1,106
exact** with zero failures, and the normal VC7 production image links.

Portable oracle: the complete i386 Linux container build links, and
`verify-modern-linux.sh` verifies the ELF32 image and every fixed target-owned
layout symbol.  No chain callback/destructor order, input bit, retry path,
original HRESULT quirk, VM interrupt/draw behavior, audio command step, pan,
volume, target byte, accepted-unit identity, or exact total changed.

### Laser rendering, enemy spawn descriptors, and runtime owner aliases — 2026-08-27

Scope: the remaining neutral Laser VMs, enemy-spawn arguments and packet
layout, DirectSound stream cursors, the ECL-extension view of spell-background
state, the duplicated player-position symbol, and three linked-child ECL
helpers.

Laser target use separates its two animation objects without relying on asset
guesses.  `bodyVm @ +0x000` is scaled to the laser width and length, rotated,
executed, and drawn at the segment midpoint.  `startCapVm @ +0x2A4` is drawn at
`startOffset` under the cap visibility rules.  `SpawnEnemy1/2 @ 0x0042A4E0 /
0x0042A680` pass their first argument to `CallEclSub` as an `i16`, then consume
the following arguments as life, item-drop type, and score.  SpawnEnemy1's
last scalar directly supplies `mirrorMovementX`.  Both serialized
`SpawnPacketTyped` views now expose that same layout.

`CStreamingSound::HandleWaveStreamNotification @ 0x00472D30` supplies its two
formerly anonymous locals to DirectSound's play-cursor and write-cursor output
parameters; only the current write cursor participates in the overwrite guard.
The ECL extension global at `0x004E4B60` is exactly `g_Background @ 0x004E4030
+ offsetof(Background, spellVmCount) @ 0xB30`.  Its former `mode`, anonymous
dword, and `vm0/vm1` members therefore become `spellVmCount`,
`spellVmScriptBase`, and `spellVms[2]`, agreeing with the public Background
owner rather than inventing a second state model.

The old `EclOperands::g_TargetPlayerPosition017D61AC` symbol is likewise
exactly `g_Player @ 0x017D5EF8 + offsetof(Player, position) @ 0x2B4`.  All
aiming, wrap-selection, night-blindness, spell presentation, GUI, and effect
tracking sites now name `g_Player.position` directly.  The affected COFF
manifests were migrated from the synthetic position symbol to `g_Player`; the
object addend continues to encode `+0x2B4`, so the relocation-aware Oracle
still checks the same target addresses instead of bypassing symbol evidence.
The obsolete Linux fixed-address alias is removed.

Finally, target behavior at `0x0041EFC0`, `0x0041F110`, and `0x0041F280`
establishes `FindAttachmentChainTail`, `SpawnChildAtScriptPosition`, and
`SpawnChildAtParentOffset`.  The latter adds the parent's world position while
the former spawner uses the resolved script position directly; source,
mapping, implemented/reccmp ledgers, accepted rows, and decorated-symbol
manifests move together.

VC7 oracle: target-pinned packets cover the Laser lifecycle at `0x00430F20`,
`0x00431240`, and `0x00432B50`; both enemy spawners; the stream notifier; the
spell-background alias users including `0x004284B0` and `0x004235A0`; and all
three child helpers.  Focused replay passes **190 / 190 exact** for the Laser,
spawn, spell-background, and audio batch; **182 / 182 exact** for the player-
position owner migration; and **25 / 25 exact** for the child-helper ledger
migration.  The required single-job non-reuse cold build of all 75 comparison
objects passes **1,106 / 1,106 exact** with zero failures, and the normal VC7
production image links.

Portable oracle: the complete i386 Linux container image links after removal
of the synthetic player-position linker symbol, and
`verify-modern-linux.sh` verifies the ELF32 executable and every fixed target-
owned layout symbol.  No class size/offset, Laser lifecycle, enemy-spawn
behavior, stream overwrite guard, spell-background state, player coordinate,
attachment-chain behavior, target byte, accepted-unit identity, or aggregate
exact total changed.

### ECL spell-background state returns to its real owners — 2026-08-27

Scope: three residual globals that named interior addresses of already typed
production owners, plus every source, ledger, linker, and COFF relocation view
that depended on those aliases.

ECL opcode 155 writes `0x05F5E0F6` to target address `0x004ECCA8` immediately
after enabling the Enemy timeout-spell flag.  The address equation is exact:
`g_Spellcard @ 0x004EA670 + offsetof(Spellcard, scoreLimit) @ 0x2638 =
0x004ECCA8`.  `Spellcard::StartSpell @ 0x004152A0` independently writes the
same decimal value, `99999990`, to that field when the timeout flag is active.
The opcode therefore now names `g_Spellcard.scoreLimit` rather than the
synthetic `g_EclGlobal004ECCA8` identity.

The other two aliases are likewise interior Background fields:
`g_Background @ 0x004E4030 + spellVmCount @ 0xB30 = 0x004E4B60`, and
`g_Background + spellBackgroundDrawCallback @ 0x625C = 0x004EA28C`.  ECL
extension drawing, ANM interrupts, effect-resource setup, and the three barrier
startup handlers now access `g_Background.spellVms`, `spellVmCount`, and
`spellBackgroundDrawCallback` directly.  The callback field uses the
`__fastcall` ABI of its sole non-null target, `DrawBulletWarpBarrier`, so the
assignments no longer need a `void *` cast.  The stale DIFFBUILD `_g_Stage`
symbol at the Background base is corrected to `_g_Background`.  The
compatibility accessors and
duplicate two-VM overlay type are no longer needed.  The global ledgers and
Linux score-limit linker alias are removed, while the COFF match manifests now
name `g_Spellcard` or `g_Background`; their object relocation addends retain the
member offsets and therefore resolve to the same target bytes.

VC7 oracle: focused replay of EclRun.obj, EclExIns.obj, EffectManager.obj, and
EclGlobals.obj passes **86 / 86 exact**.  Because the cleanup changes shared
headers and relocation ownership, the required single-job non-reuse cold build
of all 75 comparison objects passes **1,106 / 1,106 exact** with zero failures.
The normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links after the
obsolete linker alias is removed, and `verify-modern-linux.sh` verifies the
ELF32 executable and every fixed target-owned layout symbol.  No spell timeout,
barrier draw callback, ANM VM update, target byte, accepted-unit identity, or
aggregate exact total changed.

### ECL interpreter namespaces and shared runtime types — 2026-08-27

Scope: the low/high ECL helper families that retained reconstruction-phase
namespace names, their weak raw-pointer ABIs, and three independent structural
views of the same instruction and interpolation records.

The 17 exact helpers formerly under `EclRunLowProposal` and
`EclRunHighProposal` now live under the production-facing `EclRunLow` and
`EclRunHigh` namespaces.  The migration includes source declarations and
definitions, all implementation/mapping/reccmp/match ledgers, decorated COFF
symbols, `g_EclCallParameters @ 0x004ECE20`, and the Linux fixed-address alias.
This is an identity migration only: each accepted function retains its target
address and relocation-aware comparison.  The unused private
`TargetPlayerOverlay` declaration is removed because current production users
already access the public `g_Player` owner and its exact members directly.

`ApplyRandomBiasedMove @ 0x004224A0` and `DispatchShotInstruction @
0x00422720` now express their actual fastcall inputs as
`Enemy *` and `EclRawInstruction *`, replacing `u8 *`, `void *`, and the high
dispatcher's duplicate `RawInstruction`.  The main
interpreter and opcode 169 likewise use the shared `Float3` instead of a local
three-float shell.  These types make operand flags, serialized operands, Enemy
state, and saved positions readable without changing the target-observed
loads or call convention.

The ECL context, RunEcl update loop, slot installer, and interpolation helpers
now share the public `EnemyEclInterpolationSlot`.  Its callback is typed as
`void (__fastcall *)(Enemy *, EnemyEclInterpolationSlot *, f32)`, matching the
register/stack ABI used by `InterpolateLinear @ 0x00421120` and
`InterpolateHermite @ 0x00421180`.  The callback table at `0x004C6C90` carries
that same function-pointer type.  Size `0x30` and the observed duration,
callback-index, parameter-array, and affected-variable offsets remain guarded
by compile-time assertions.

VC7 oracle: focused relocation-aware replay of EclDependencies.obj,
EclGlobals.obj, EclOperandsInt.obj, EclOperandsFloat.obj, EclRun.obj, and
EnemyManager.obj passes **68 / 68 exact**.  Because the batch changes a shared
header and decorated identities, the required single-job non-reuse cold build
of all 75 comparison objects passes **1,106 / 1,106 exact** with zero failures.
The normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links with the new
namespace symbol, and `verify-modern-linux.sh` verifies the ELF32 executable
and every fixed target-owned layout symbol.  No instruction layout,
interpolation slot offset, callback ABI, target byte, accepted-unit identity,
or aggregate exact total changed.

### Enemy-owned ECL execution and typed EX dispatch — 2026-08-27

Scope: the temporary Enemy ABI adapter, the dormant standalone low/high
dispatcher harness, shared spell/EX instruction records, and the Spellcard
field that stores the active Enemy owner.

The lexical bodies in `EclRunLow.inl` and `EclRunHigh.inl` are production code
included by `EclManager::RunEcl`; their unused standalone dispatcher branches
and `Services`/`TargetApi` proposal shells had no callers and obscured that
relationship.  They are removed.  The main interpreter still retains its
target-authored lexical handler order and source shape.

The complete target bodies at `Enemy::ResolveFloat @ 0x00420120` and
`Enemy::DetachEnemyChain @ 0x0042ADB0` operate on the asserted `Enemy` layout.
The temporary `EclOperands::EnemyOverlay` adapter is therefore retired.  Both
operand resolvers, the low/high helper family, RunEcl, the EX handlers, Enemy
updates, and Player callers now pass `Enemy *` directly.  All 53 affected
function identities and 465 configured COFF relocation references were
migrated to the actual VC7-decorated `Enemy *` symbols.  The method identity
for `DetachEnemyChain` is used where the split object defines or calls it;
external object relocations retain the exact VC7 decorated member symbol.

`EclSpellCardInstructionArgs` now gives `StartEnemySpell @ 0x00421280` and
`EndEnemySpell @ 0x004212E0` typed access to their serialized spell operands.
The EX dispatcher shares one `EclExInstruction` record and exposes
`g_EclExInsn @ 0x004C6CB0` as an array of
`void (__fastcall *)(Enemy *, EclExInstruction *)`.  Opcode 136 invokes the
table directly.  Opcode 137 deliberately retains an explicit function-pointer
cast: target evidence shows it reuses the table with an ECL-context second
argument, so erasing that distinction would assert an unsupported common ABI.

Finally, `Spellcard::activeEnemy @ +0x004` and the fifth parameter of
`Spellcard::StartSpell @ 0x004152A0` are `Enemy *`.  Spell presentation and
damage/capture logic now read the owner without byte-pointer casts while class
size, offset, calling convention, and decorated relocation identity remain
pinned.

VC7 oracle: focused relocation-aware replay of EclOperandsInt.obj,
EclOperandsFloat.obj, EclHelpers.obj, EclRun.obj, EclDependencies.obj,
EclExIns.obj, EclGlobals.obj, EnemyManager.obj, EnemyManagerUpdate.obj,
Player.obj, and SpellCard.obj passes **213 / 213 exact**.  Because the batch
changes shared headers and decorated identities, the required single-job
non-reuse cold build of all 75 comparison objects passes **1,106 / 1,106
exact** with zero failures.  The normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links with the typed
callback and Enemy/Spellcard owners, and `verify-modern-linux.sh` verifies the
ELF32 executable and every fixed target-owned layout symbol.  No serialized
record layout, callback ABI, Enemy/Spellcard offset, target byte, accepted-unit
identity, or aggregate exact total changed.

### Typed Enemy spawning and lifecycle traversal — 2026-08-27

Scope: the two EnemyManager spawn constructors, their timeline/ECL callers,
and the remaining byte cursors in EnemyManager initialization, chain cleanup,
and bulk non-boss removal.

`EnemyManager::SpawnEnemy1 @ 0x0042A4E0` and `SpawnEnemy2 @ 0x0042A680`
scan `EnemyManager::enemies[480]`, copy the typed spawn template into the first
inactive slot, initialize its ECL context, and return that same slot in EAX.
Timeline opcode 11 immediately writes Enemy drop fields through the returned
pointer; ECL child spawners use it as an attachment-chain node.  Both APIs now
return `Enemy *`, and all callers consume that type without `void *` adapters.

This source-only type change preserves the x86 calling convention but changes
VC7's decorated identity.  The compiler-emitted symbols were read from
EnemyTimeline.obj as
`?SpawnEnemy1@EnemyManager@th08@@QAEPAUEnemy@2@HPBUD3DXVECTOR3@@HHHH@Z`
and
`?SpawnEnemy2@EnemyManager@th08@@QAEPAUEnemy@2@HPBUD3DXVECTOR3@@HHHPAH@Z`.
All six configured references to each old `QAEPAX...` symbol were migrated to
those observed names; no mangling was inferred by hand.

`EnemyManager::Initialize @ 0x00429E00` now initializes its `spawnTemplate`
through an `Enemy *`.  `KillAllNonBossEnemies @ 0x0042EFB0` advances the same
typed array cursor with `enemy++`, which VC7 lowers to the existing 0x53D0-byte
stride.  `Enemy::DetachEnemyChain @ 0x0042ADB0`, life/timer callbacks, and the
spawn constructors access typed chain links and `mainEclContextStorage`
directly.  The target-sensitive lexical scopes, `#pragma var_order` names,
copy aggregates, and dword float stores remain unchanged.

VC7 oracle: focused relocation-aware replay of EnemyManager.obj,
EnemyTimeline.obj, EclRun.obj, EclDependencies.obj, and EclExIns.obj passes
**101 / 101 exact**.  Because the return type changes a shared declaration and
decorated relocation identities, the required single-job non-reuse cold build
of all 75 comparison objects passes **1,106 / 1,106 exact** with zero failures.
The normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links with the typed
spawn API and traversals, and `verify-modern-linux.sh` verifies the ELF32
executable and every fixed target-owned layout symbol.  No Enemy size/stride,
spawn behavior, ECL context, target byte, accepted-unit identity, or aggregate
exact total changed.

### Typed chain lifetime and ANM sprite cache owners — 2026-08-27

Scope: two residual `void *` fields whose complete production use gives each a
single concrete owner type.

`Spellcard::lifetimeObject @ +0x263C` is created by `Chain::CreateElem`, receives
`ChainElem::deletedCallback` and `ChainElem::arg`, is submitted to
`Chain::AddToCalcChain`, and has its lifetime callback cleared during
`Spellcard::DeletedCallback @ 0x00418050`.  It is now `ChainElem *`, matching
the adjacent draw-chain field at `+0x2640`; `Spellcard::RegisterChain @
0x00417F60` and deletion cleanup no longer reinterpret the stored owner.

`AnmManager::currentSprite @ +0x24C8` is the render-state cache compared with
and assigned only from `AnmVm::loadedSprite`.  That producer is an
`AnmLoadedSprite *`, so the cache now carries the same type.  The change makes
the state invalidation in `AnmManager::Draw3D @ 0x00464470` explicit without
changing the four-byte VC7 field or the asserted modern-port offset.

VC7 oracle: focused relocation-aware replay passes **175 / 175 exact** for
`Spellcard::RegisterChain`, **157 / 157 exact** for
`Spellcard::DeletedCallback`, and **1,318 / 1,318 exact** for
`AnmManager::Draw3D`.  Because both declarations are in shared layout headers,
the required single-job non-reuse cold build of all 75 comparison objects
passes **1,106 / 1,106 exact** with zero failures.  The normal VC7 production
image links.

Portable oracle: the complete i386 Linux container image links with both typed
pointers, and `verify-modern-linux.sh` verifies the ELF32 executable and every
fixed target-owned layout symbol.  No object offset, callback ordering, render
state, target byte, accepted-unit identity, or aggregate exact total changed.

### Typed Effect lifecycle and spawn ownership — 2026-08-27

Scope: the Effect template/instance callback protocol, all callbacks that were
still declared against the Effect's leading `AnmVm`, the five Effect allocation
APIs, the fixed-slot lookup, and callers that retain or mutate the returned
owner.

Target instructions in `EffectManager::OnUpdate @ 0x00427BF0` load the current
Effect address into ECX before the indirect update call and compare EAX with
one.  `EffectManager::OnDraw @ 0x00427F00` likewise supplies the Effect address
in ECX to the draw callback and ignores its return value.  The instance fields
at `Effect +0x348` and `+0x34C` therefore carry typed `__fastcall` update and
draw callbacks, while the template's third field carries the corresponding
initializer type.  The template storage now expresses all 66 runtime IDs
(`0..65`) rather than the old 20-row source fiction.  Initializers, update and
draw dispatchers, the splash/orbit family, and the player radial/barrier family
all receive `Effect *` directly.  `AnmVm::UpdatePulsingRadialTrail` remains the
documented compatibility entry whose member ABI intentionally views the same
leading subobject.

`EffectManager::SpawnEffect @ 0x00425430`, `SpawnEffectWithVelocity @
0x00425650`, both fixed-slot variants at `0x00425870` and `0x004259E0`, and
`SpawnEffectInSecondaryPool @ 0x00425B70` now return `Effect *`, the object
each routine actually scans and initializes.  `GetFixedSlotEffect @
0x004253E0` returns that same owner.  Callers take `&effect->vm` only when an
ANM API truly requires the leading animation subobject; Effect fields and
Enemy attachment pointers are accessed through their real owner.  This also
types `PlayerBombWorkItem::effect @ +0x16D8` and
`Player::deathbombEffect @ +0xE2B28` without changing either four-byte field or
the containing layouts.

The post-change VC7 COFF definitions were read directly from
`EffectManager.obj`: each spawn/getter identity is `QAEPAUEffect@2@...`.
Configured definition and caller relocations were migrated to those observed
symbols, as were the compiler-observed `PAUEffect@1@@Z` callback identities;
none were hand-inferred.  Focused relocation-aware replay of the getter, five
spawn routines, lifecycle dispatchers, and migrated callbacks passes **26 / 26
exact**.  Because the declarations are shared and their decorated identities
reach callers throughout the game loop, the required single-job non-reuse cold
build of all 75 comparison objects passes **1,106 / 1,106 exact** with zero
failures.  The normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links with the typed
Effect protocol and owner returns, and `verify-modern-linux.sh` verifies the
ELF32 executable and every fixed target-owned layout symbol.  No Effect or
Player layout, callback calling convention, target byte, accepted-unit
identity, or aggregate exact total changed.

### Typed core callbacks and EnemyManager update ownership — 2026-08-27

Scope: the projected-position ANM callback ABI, Player shot collision callback
storage, and the complete EnemyManager update loop at `0x0042C660`.

`AnmManager::ProjectCameraFacingQuadWithCallback @ 0x004640E0` tests the
callback, supplies the `AnmVm *` in ECX, and passes the projected position as
the sole stack argument.  The callback is therefore represented as
`void __fastcall(AnmVm *, D3DXVECTOR3 *)`; `DrawWithCallback @ 0x00464400` and
the stage-Effect draw path now carry that type instead of `void *`.  The two
new VC7 decorated identities were read from the rebuilt `AnmManager.obj` before
their configured definition/caller relocations were migrated.

The target reference in `Player::LoadShtFile @ 0x0044DCB0` indexes a
three-entry callback array beginning at `0x004C7F24`.  The next address,
`0x004C7F30`, belongs to the independently defined five-entry
`ReplayManager.cpp` difficulty-name array.  The old nine-entry `void *` source
array that concatenated three callbacks and six strings was therefore a false
reconstruction boundary.  `g_PlayerShotCollisionCallbacks` now contains only
the three correctly typed callback entries, and the replay table remains its
own owner.

Target instructions for `EnemyManager::OnUpdate @ 0x0042C660` home ECX as the
owner and address the complete EnemyManager state from it.  The one-byte
`EnemyManagerUpdateOverlay` shell and repeated whole-object reinterpret casts
are removed; the exact 0x1836-byte body is now the real non-static
`EnemyManager::OnUpdate()`.  A separately named `OnUpdateCallback` adapts the
portable `ChainElem::arg` ABI, while `RegisterChain @ 0x0042C590` continues to
resolve its callback relocation to target address `0x0042C660`.  The rebuilt
COFF definitions prove the member symbol
`?OnUpdate@EnemyManager@th08@@QAEHXZ` and the adapter symbol used by the
portable link.

VC7 oracle: all nine focused configured units pass exact, including the full
EnemyManager update comparison (**6,214 / 6,214 compared bytes**) and
RegisterChain (**203 / 203**).  Because the batch changes shared declarations
and decorated identities, the required single-job non-reuse cold build of all
75 comparison objects passes **1,106 / 1,106 exact** with zero failures.  The
normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links with the typed
callbacks and real EnemyManager owner, and `verify-modern-linux.sh` verifies
the ELF32 executable and every fixed target-owned layout symbol.  No callback
calling convention, Player/EnemyManager layout, target byte, accepted-unit
identity, or aggregate exact total changed.

### Residual runtime owners and Title portable parity — 2026-08-27

Scope: five remaining false cast boundaries in Background, ECL, EnemyManager,
Replay, and Title spell-history code; a bounded audit of every target-facing
probe that duplicates production behavior; and three Title control-flow
differences found by that audit.

`Background::RenderObjects @ 0x0040A1B0` now indexes
`specialEffectPoints` through the already offset-asserted
`specialEffectPointCount @ +0x6478` instead of reconstructing the same field
with `this + 0x6478`.  `EnemyEclContext +0x10/+0x14` now carries the typed
`EclExInstructionCallback` and the serialized `EclExInstruction *` retained
by opcode 137; `EclManager::RunEcl @ 0x004184B0` invokes that per-frame pair
without a `void *` callback boundary.  The table entry remains evidenced by
the same target-observed `__fastcall(Enemy *, EclExInstruction *)` ABI.

`EnemyManager::AddedCallback @ 0x0042EBF0` preserves and restores
`g_EclManager.eclFile` and `subTable` through their real `EclRawHeader *` and
`u32 *` fields rather than treating the manager's first two dwords as
anonymous integers.  `ReplayManager::LoadReplayData @ 0x00451D90` now accepts
the `ReplayData *` that every production caller supplies.  Its rebuilt VC7
definition is
`?LoadReplayData@ReplayManager@th08@@SIPAUReplayData@2@PAU32@H@Z`; all seven
configured definition/caller identities were migrated to that observed COFF
symbol.  The mapping ledger also catches up with the already exact
`ZunResult Background::RenderObjects(i32)`,
`ZunResult EclManager::RunEcl(Enemy *)`, and
`ZunResult EnemyManager::AddedCallback(EnemyManager *)` source ABIs.

The temporary Title-only `TitleCatkView` is removed.  Its three proven queries
are now inline `Catk` members: spell-practice capture, any capture, and any
attempt for a shot slot.  `UnlockLastWordSpellCards @ 0x0046CBBB` and
`FormatSpellCardInfo @ 0x0046D7F9` therefore read the real persisted spell
history owner directly.

The repository's complete duplicated-source audit covers all four
`*Probe.cpp` files plus `GameManagerSetup.cpp`.  The fifteen Player option/shot
bodies are token-identical to production after normalizing only the local
parameter spelling; `InitializeScoreData` and both Stage-menu draw bodies are
source-identical; Gameplay setup differs only between a cached
`gameManager == &g_GameManager` spelling and the same global owner.  The Title
probe exposed three genuine portable-source differences.  Production
`DrawCompletionStatusText @ 0x0047052D` incorrectly bound `cursor > 3` only to
the Lunatic clear test; the exact behavior is the four clear tests **or**
`cursor > 3`.  Production `OnUpdateSpellCardSelect @ 0x0046BBC0` also reversed
two upper-bound tests as `count >= cursor`; both now use the exact
`cursor >= count` relation.  Completion rendering is single-owned by
`TitleCompletionStatus.inl`, shared by production and its strict VC7 probe,
with a target-shaped inline ANM initializer.  The target-facing spell-card
select probe remains exact at **4,047 / 4,047** compared bytes.

VC7 oracle: focused relocation-aware replay passes all **11 / 11** selected
units, including Background RenderObjects (**4,746 / 4,746**), complete RunEcl
(**26,638 / 26,638**), EnemyManager AddedCallback (**642 / 642**), Replay load
(**511 / 511**), Last Word unlock (**2,984 / 2,984**), spell-card formatting
(**2,365 / 2,365**), and completion status (**850 / 850**).  Because the batch
changes shared headers and inline bodies, the required single-job non-reuse
cold build of all 75 comparison objects passes **1,106 / 1,106 exact** with
zero failures; `TitleScreen::RegisterChain` remains **281 / 281 exact**.  The
normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links after the
Title behavior corrections and typed callback/API changes, and
`verify-modern-linux.sh` verifies the ELF32 executable and every fixed
target-owned layout symbol.  The audit does not turn source similarity into an
exact claim: only configured target-facing units carry that status.  The sole
authored-but-unaccepted function remains
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`; its natural
361-byte target versus 362-byte object allocator residual is unchanged and is
not hidden by this semantic batch.

### Typed child ECL runtime storage and Title pie parity — 2026-08-27

Scope: the raw allocation installed by ECL opcode 135, all RunEcl child-context
selection/depth accesses, one stale ANM mapping signature, and a production
Title rendering drift exposed by the exact replay-menu probe.

Target instructions in `EclManager::RunEcl @ 0x004184B0` allocate and clear
`0x24B0` bytes, store the requested subroutine at `+0x0`, initialize an
`EnemyEclContext` at `+0x8`, and later select its call stack at `+0x230` while
loading and storing a signed word at `+0x6`.  `EnemyChildEclBlock` now expresses
that complete runtime owner as `subId`, an explicitly unconsumed word at
`+0x4`, signed `callStackDepth`, one active context, and 16 call-stack
contexts.  Size and offset assertions pin `0x24B0`, `+0x8`, and `+0x230`.
`Enemy::childEclBlocks @ +0x3384` and both low/high RunEcl paths use that type
directly; the main context and main call stack likewise no longer pass through
redundant same-type casts.  The copied variable span is described by the
`EnemyEclContext` member boundaries rather than the old `0x1E` dword literal.
Serialized ECL instruction operands remain intentionally byte-oriented, and no
meaning is claimed for the unconsumed word at `+0x4`.

The mapping ledger for `AnmManager::AddSpriteToDrawBuffer @ 0x00462F10` now
matches its already exact declaration and configured VC7 symbol:
`ZunResult (VertexTex1DiffuseXyzrhw *)`, replacing the stale `u8 (void *)`
description.  `DrawPieChart @ 0x0046FDC2` names its two scalar parameters as
`fraction` and `diameter`.  Its exact probe also exposed that portable
production initialized `vertices[1].w`, which the following loop immediately
overwrote, while leaving the center vertex's reciprocal-homogeneous coordinate
undefined.  Production now initializes `vertices[0].w`, matching the exact
target-facing body.

VC7 oracle: focused relocation-aware replay passes RunEcl **26,638 / 26,638**,
AddSpriteToDrawBuffer **211 / 211**, and DrawPieChart **510 / 510 exact**.
Because the ECL and Enemy declarations are shared layout headers, the required
single-job non-reuse cold build of all 75 comparison objects passes **1,106 /
1,106 exact** with zero failures; `TitleScreen::RegisterChain` remains **281 /
281 exact**.  The normal VC7 production image links.

Portable oracle: the complete i386 Linux container image links with the typed
child-runtime owner and corrected center vertex initialization, and
`verify-modern-linux.sh` verifies the ELF32 executable and every fixed
target-owned layout symbol.  No Enemy/EnemyEclContext layout, accepted-unit
identity, target byte, or aggregate exact total changed.

### Authored API ledger closure — 2026-08-27

Scope: the stale function signatures in `config/mapping.csv` for 119 accepted
authored functions.  This is a ledger-only correction: the typed production
declarations and definitions were already present and exact.  Their current
VC7 COFF decorated symbols, configured comparison identities, and source call
sites provide the evidence; target addresses alone do not establish C++ types
or calling conventions.

The corrected rows cover the Background, ECL, GameManager, Effect,
Spellcard, Gui, global resource, TextHelper, ItemManager, MusicRoom, Player,
SoundPlayer, ResultScreen, TitleScreen, Supervisor, ScreenEffect, AnmManager,
AsciiManager, MIDI, and ScoreDat API families.  Material stale descriptions
included a false `MidiOutput *` receiver for `GameManager::DeletedCallback`,
missing `const char *` ResultScreen getters, incorrect TitleScreen member and
static conventions, a missing `Supervisor::LoadMusic` preload-slot parameter,
the old `AnmFileDesc *` owner for `AnmManager::LoadTextureData`, and flattened
AsciiManager varargs.  Return values and parameters now preserve the existing
`ZunResult`, `ZunBool`, `ChainCallbackResult`, typed pointer, constness, and
real argument boundaries.

None of the 119 gameplay/API rows in this scope now uses the `unknown`
convention.  Sixteen accepted exact rows still do so in the bundled helper
source families: CSound/CWaveFile, CPbgFile/PbgArchive, and LZSS.  Compiler
runtime, D3DX, static-initializer, and otherwise unselected mapping rows also
retain their existing unknown descriptions.  They are separate audit lanes;
this checkpoint does not turn a scoped cleanup into a repository-wide
completion claim or invent unsupported provenance for them.

VC7 oracle: relocation-aware focused comparison passes all **119 / 119**
affected accepted units.  The required single-job non-reuse cold build of all
75 comparison objects passes **1,106 / 1,106 exact** with zero failures;
`TitleScreen::RegisterChain` remains **281 / 281 exact**.  The normal VC7
production image links.

Portable oracle: the complete i386 Linux container image links, and
`verify-modern-linux.sh` verifies the ELF32 executable and every fixed
target-owned layout symbol.  Because this batch changes only the semantic
ledger, no source body, object identity, accepted-unit result, or aggregate
exact total changed.  The sole authored-but-unaccepted function remains the
unchanged natural allocator residual in
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`.
