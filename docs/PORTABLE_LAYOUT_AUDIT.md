# Native-layout audit after PR #34

Tracked in [issue #35](https://github.com/N0zoM1z0/th08/issues/35), including
an English findings record, the complete diagnostic and repair checklists.

The initial audit of `port/portable-64bit` at
`27adcb61a744c29afb121561fb4e277aa1818ef0` found five remaining defect
families: seven runtime reset calls still use their i386 byte counts, and ECL
opcode 169 still reads a runtime member through its i386 offset. The findings
below describe that original baseline; the repair checkpoint follows.

## Repair checkpoint

Implementation checkpoint `9adebf5c1b203a7042cc7df9c6eddb9bd38ca025` on
`fix/portable-layout-resets` replaces all seven reset lengths with the actual
object/member `sizeof`. Opcode 169 retains its accepted byte-view expression
tree but derives the offset with `offsetof(Enemy, position)`; the original
layout still asserts `0x2D34`. Shared layouts, translation-unit ownership,
compiler profiles, mappings and acceptance ledgers are unchanged.

The single-job cold canonical gate passes all 1,106 accepted units. Fresh
native normal and bugfix links pass the runtime-data/owner/callee checks.
The focused object replays pass every accepted unit in EffectManager (52),
BulletManager (37), Gui (41) and Player (77), plus RunEcl's complete 27,398-byte
comparison.

The new [production transition checks](../scripts/test-portable-layout.py)
pass 54/54 cases in the fresh GCC 8.4.0 x86_64 recorder. The retained old
product reproduces all five defect families (40 failing checks out of 54).
The tests include complete Player focus-entry, route-3 exit and all eight solo
initializers, rather than stopping after memset. They use empty cached ANM
tables and null option callback data; real game callbacks, assets, stage
transitions and replay behavior require their separate game checks.
The fresh GCC 12 i386 recorder also passes the fixed-layout verifier and all
54 transition checks inside its actual build container, limited to one CPU,
768 MiB and one job. Both selected replay gates and their separate retained
verifications are complete:

| Product | Complete recordings | Calculations | Scope |
| --- | --- | --- | --- |
| GCC 8 x86_64 | 7 | 694,579 | `easy-0-b`, `easy-1-a`, `easy-2-a`, `easy-3-b`, `easy-7-b`, `easy-11-a`, `extra-2-extra` |
| GCC 12 i386 | 2 | 200,700 | `easy-2-a`, `easy-3-b` |

Every calculation agrees with a hash-validated retail reference on all 25
fields. Stage-end scores, published fingerprints and natural replay exits
agree. The x86_64 selection covers all four teams, Alice/Yuyuko solo startup,
Final A/B and Extra. The recorded inputs contain focus presses/releases;
`input-coverage.json` records their counts as input evidence, not instrumented
function coverage. These repaired products have not been rerun against the
entire 108-recording corpus or on AArch64 hardware, and this work does not
attribute the contributor's extended player/item hash differences.

CI now runs the asset-free checks for x86_64 and inside the actual i386
container, retaining JSON reports. GDB is a build/test dependency in that
container and is not added to the game package. The original diagnostic below
remains a reproducer for the unfixed baseline.

The fixed products and hashes are retained under
`build/portable-layout-repair/products/`:

- `th08-x64`: `0d63babba5fa06a60a7f39a64bc8a9ca94404abfd2c5d8c8373084baa148f286`.
- `th08-i386`: `e8d46580f16ea302b7be79c396f8162f6d67f6fc937a1252b05a2421ee85918f`.
- `th08-vc7-normal.exe`: `4aeba158d49dc69681490cd2470beda9463714623f3d865a695df81dec9492be`.
- `th08-vc7-bugfix.exe`: `e70ac948797a70db901d825d65545ef2a0d02c32b9331137f66f41d2ceefe316`.

`build-provenance.json` pins the 133 production inputs, both compiler profiles,
the i386 image/package versions and the products. `validation-summary.json`,
`before-after.json`, `fixed-transitions-*.json`, `all-exact.json` and the two
`replays-*/` directories retain comparison/state/replay evidence. Only the five
expected production source files differ from the old control. The CMake
compiler trees are disposable; the copied products are independent of them.

Recheck fixed postconditions with:

```bash
python3 scripts/test-portable-layout.py \
  --candidate build/portable-layout-repair/products/th08-x64
python3 scripts/test-portable-layout.py \
  --candidate build/review-pr34/products/candidate-x86_64 --expect-baseline
```

For the exact i386 dependency environment, run the first command with
`th08-i386` inside the retained `th08-layout-repair-linux32` image, mounting the
repository at its identical absolute path. The committed CI workflow shows
the equivalent command using its ordinary `/src` mount. Its GDB checks were
also verified with the container's default security policy.

## Confirmed findings

These are runtime objects, not serialized wire records. The sizes below are
observed in the retained GCC 8.4.0 x86_64 Debug product.

| Family and source | i386 constant | x86_64 layout | Reproduction |
| --- | --- | --- | --- |
| `ECL_OPCODE_RANDOM_HORIZONTAL_ANGLE`, `EclRunHigh.inl:142,967` | position at `+0x2D34` | position at `+0x338C` | Identical player/enemy positions and RNG initial state produce different angles when unrelated data at the old offset changes. |
| `EffectManager::ResetEffects`, `EffectManager.cpp:194` | clear `0x8B05C` | size `0x940A8` | `effects[640].active` remains 1 after reset. |
| `GuiImpl::StartMessage`, `Gui.cpp:133` | clear `0x1570` | `GuiMsgVm` size `0x1698` | Starting ordinary message 1 retains `ignoreWaitCounter=123` and `selectedOption=1`. |
| `BulletManager::RemoveAllBullets`, `BulletManager.cpp:513,519` | clear `0x10B8` | `Bullet` size `0x1178` | Both clear arms set the bullet state to UNUSED but leave `collisionDisabled=1` at `+0x1170`. |
| Player option initialization, `Player.cpp:670,740,1678` | clear `0x2F4` | `PlayerOptionState` size `0x320` | At the real focus-entry clear boundary, `orbitAngle`, `facingAngle` and `timer.current` retain their previous values. |

### ECL opcode 169: wrong branch input

The raw byte-view macro uses `TH08_ECL_ENEMY_POSITION_OFFSET = 0x2D34`.
Its `C_ASSERT` is disabled in the native layout. All three reads in opcode
169 consequently address unrelated ECL stack storage on x86_64.

The probe invokes the production `RunEcl` with a two-instruction synthetic
stream. It holds `enemy.position.x=320`, `g_Player.position.x=100` and the RNG
initial state constant. Changing the unrelated float at `Enemy+0x2D34` from
0 to 320 changes the generated angle from `0.41390520334243774` to
`-2.7276875972747803`. The actual position does not change. This confirms a
branch/input error without relying on a screenshot or an extended state hash.

### Reset contracts and remaining impact questions

The effect, bullet and GUI probes call the complete production methods.
The effect probe also observes `nextEffectIndex` becoming zero, distinguishing
a partial reset from a call that failed to run. `ResetEffects` is used during
construction, chain registration and effect-resource loading, so its stale
fixed-pool state crosses real lifecycle boundaries. Resource pointers are
subsequently reinstalled by the loader; their mere survival in `ResetEffects`
is not by itself a dangling-pointer finding.

The bullet probe disables cancellation regions for one arm and enables a
region for the other. Its offscreen position makes `SpawnItem` take its real
early return before touching animation assets. `SpawnSingleBullet` explicitly
initializes `collisionDisabled`, so this observation alone does not prove
that a later reused bullet remains noncolliding.

The GUI test uses ordinary message 1 and stage 0 to avoid special route/resource
branches. `RunMsg` consumes `ignoreWaitCounter` at `Gui.cpp:292` and uses the
selection state in its option handler. The source has no ordinary-start
assignment that replaces the omitted zeroing of those fields.

The player probe stops immediately after the actual focus-entry `memset`,
before callback assignment. It does not run the entire movement/animation
method. Later callbacks and timer assignments overwrite some omitted bytes;
the other two identical clear sites were inspected statically. No attribution
to the PR author's 17 player/one item hash differences is made. A natural
gameplay, replay or visual symptom for these reset findings remains to be
measured.

## Target and native comparison evidence

The canonical Japanese TH08 1.00d executable passed size/hash verification:
840,704 bytes, SHA-256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`.
Mapping and function-ledger entries were checked before target inspection.

- `RunEcl @ 0x004184B0`: opcode 169's table entry at `0x0041EF5E`
  selects `0x0041E28F`. The handler reads `Enemy+0x2D34` at
  `0x0041E298`, `0x0041E2A8` and `0x0041E2BE`; that is the correct original
  32-bit position offset, not a native-layout contract.
- `ResetEffects @ 0x00425410`: `ECX=0x22C17; rep stosd` clears
  `0x8B05C` bytes.
- `RemoveAllBullets @ 0x00430830`: the two clear sequences at
  `0x004308EA` and `0x0043091F` use `ECX=0x42E`, or `0x10B8` bytes.
- `StartMessage @ 0x0043396D`: `ECX=0x55C` at `0x00433992` clears
  `0x1570` bytes.
- `UpdateMovementAndOptions @ 0x0044AEC0`: clear sequences at
  `0x0044B090` and `0x0044B2DB` use `ECX=0xBD`, or `0x2F4` bytes.
  `Player::AddedCallback @ 0x0044D650` repeats that count at `0x0044DB81`.

Focused VC7 object builds and strict canonical comparisons pass for the six
configured functions: `ecl-manager-run-ecl`, `effect-manager-reset-effects`,
`bullet-manager-remove-all-bullets`, `gui-message-start`,
`player-update-movement-and-options` and `player-added-callback-reconstructed`.
This is a native function-level baseline, not a fresh aggregate exactness
claim. It demonstrates why existing VC7 exactness does not detect a 64-bit
runtime layout error.

## Inspected exclusions

The source sweep included explicit memory-copy/clear calls, raw allocation
helpers including `MemAlloc`, pointer-sized allocations, byte views and
symbolic/literal object offsets. Similar-looking sites were traced through
their declarations, platform guards and consumers:

- `EnemyManager::Initialize` already uses full runtime `sizeof` values in the
  native layout and retains original constants in the i386 branch.
- ECL child blocks allocate/clear `sizeof(EnemyChildEclBlock)`; the child
  variable-region copy uses `offsetof`. Ordinary context call/return uses
  typed copies. `EnemyContextCopy`'s `0x78` span remains the pointer-free
  numeric region from `intVariables` to `secondaryTime` on both layouts.
- ANM runtime pointer tables allocate with `sizeof(void*)`; sprite and texture
  arrays use their actual element types. Background object-offset tables are
  separately expanded into native pointer tables.
- Replay and score loaders retain explicit wire sizes and separately reserve
  or move runtime storage. Those constants must not be converted wholesale
  to native `sizeof`. Their wire assertions remain enabled.
- The `STARTUPINFO` tail clear already subtracts `offsetof(lpReserved)` in
  the native branch; the `-4` spelling is confined to the original layout.
- The reviewed 64-byte text copies, eight-byte MIDI header copy, pixel-word
  copies, bitmap header reset and pointer-free effect vertex allocation have
  fixed data-format/element extents. Narrow pointer casts inspected in ECL,
  message, replay, SHT and stage loaders have native alternatives or are
  confined to the i386 path.

This is an initial audit of these patterns, not proof that all portability
defects or all lifetime/bounds errors have been found. New modern i386 builds,
AArch64 execution, full-corpus replay, sound and natural multi-stage transition
tests were not run for this read-only audit.

## Reproduction and provenance

The retained product is
`build/review-pr34/products/candidate-x86_64`, SHA-256
`65e497660ae951e28d430c63a6eed87eb45d7d9dedbb49ebc50bb3a21e59ec0f`.
All 133 recorded production source/build inputs agree with the audited tree.
Private inventories, target disassembly, six diagnostic records, product/input
provenance and focused comparison JSON are under `build/portable-layout-audit/`.

The [asset-free GDB diagnostic](../scripts/gdb/portable-layout-audit.gdb)
contains the complete setup and calls. It diagnoses the unfixed baseline and
deliberately asserts retained stale state; convert those assertions to fixed
postconditions as each family is repaired. It patches no game code.

```bash
cmake -S . -B build/layout-audit -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/layout-audit --parallel 1
SDL_AUDIODRIVER=dummy gdb --batch --quiet \
  -x scripts/gdb/portable-layout-audit.gdb --args build/layout-audit/th08-modern
```

Repairs should use natural typed members, `sizeof` or evidenced `offsetof`
while preserving the original VC7 emission, including opcode 169's expression
tree. Repair and validate one family at a time. Rerun affected exact units,
the native normal link/owner checks and the relevant portable state transitions;
run the cold aggregate gate if a shared header/layout/compiler contract changes.
