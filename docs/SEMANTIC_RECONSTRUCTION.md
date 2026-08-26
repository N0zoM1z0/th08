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
and the target absolute source `g_EnemyManager.firstEnemy + 0x2E24` agree on
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
