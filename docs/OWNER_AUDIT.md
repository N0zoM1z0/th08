# Native runtime data-owner audit

This document records the initialized-data owner audit opened by native
Windows i386 playtesting on 2026-09-10. The only target is the original Japanese
TH08 1.00d executable, SHA-256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`.

The audit covers both initialized target data and mutable aggregate fields that
were incorrectly modeled as independent linker storage. It is whole-program
reconstruction evidence. It does not grant new
function exactness and it does not claim that every runtime path has been
exercised.

## Why the port did not expose the source defect cleanly

The modern Linux startup path populated four source globals with runtime
`memcpy`/assignment code. That made the port appear initialized even though the
native VC7 production link emitted zero-filled owners. The native link was
complete and individual functions remained exact, but callbacks and values
read by those exact functions were wrong at process scope.

The repair belongs in each semantic production translation unit. The modern
startup initialization is corroborating evidence for the intended values, not
the owner or the fix.

## Recovered initialized owners

| Target address/range | Canonical owner | Production TU | Runtime significance |
| --- | --- | --- | --- |
| `0x004C6C3C` | `g_LastSpellCount = 43` | `Spellcard.cpp` | Bounds the Last Spell number table |
| `0x004C6D30..0x004C7047` | `g_EffectTemplates[66]` | `EffectManager.cpp` | Selects every effect script, update callback, and initializer |
| `0x004C7158..0x004C717B` | `g_GuiStageClearBonuses[9]` | `Gui.cpp` | Supplies per-stage clear bonus values |
| `0x004C7180..0x004C723F` | `g_GuiMessageTextColors[12]` | `Gui.cpp` | Supplies four dialogue colors for every shot type |

All four target addresses are in raw-backed initialized `.data`, whose target
raw extent is `0x004C6000..0x004CCDFF`. In contrast, the unassigned manager,
chain, VM, scratch-buffer, and controller-state owners begin at target
`0x004CCE00` or later and correctly belong to zero-fill storage.

Run the audit/linked-data verifier with:

```bash
python3 scripts/analysis/verify-windows-i386-runtime-data.py
```

At this checkpoint it reports zero `DIFFABLE_STATIC`/array owners mapped into a
raw-backed target section without an initializer. It also checks the four
families above in the linked reconstruction and verifies that all eight linked
loads in `Gui::CopyEnemyNameTexture` select `Gui::frontAnm`. It also rejects a
standalone spell-background ANM symbol and requires `Spellcard::StartSpell` to
load `EffectManager::stageEffectAnm`. It also rejects standalone player gauge-
bound storage and checks all 21 linked writes into the six real `GameManager`
fields. This is deliberately stricter than checking source text alone.

## Remilia X-bomb incident

The user-observed symptom was that Sakuya's X bomb rendered normally while
Remilia's X-bomb trail did not. Remilia's normal bomb periodically spawns Effect
ID 53. Target table row 53 is:

```text
script 88, UpdateFadingRadialTrail @ 0x00427AE0,
InitializeRadialTrail @ 0x004272E0
```

The zero-filled production table instead selected script 0 with no update or
initializer. Sakuya's knife-VM path does not depend on the same table row, so
the asymmetric symptom follows directly from the missing owner.

The complete target table contains one ABI compatibility entry at row 41: its
update pointer is the exact `AnmVm::UpdatePulsingRadialTrail @ 0x0040EB50`
member body, while the table type is an Effect fastcall callback. VC7 rejects a
natural C++ member-pointer-to-free-pointer initializer. Production therefore
uses a normal relocatable Effect callback adapter which calls the exact member
implementation on `effect->vm`; it does not embed the target preferred-base
address or manufacture machine code.

## Enemy-name texture owner and the double-error trap

Native hash `c394035e...f9ce7` faulted at `Gui::CopyEnemyNameTexture + 0x77`
immediately after the user selected a run. The source used
`g_Gui.stageTextAnm`, whose loaded Stage 1 file has six sprites, while the
function requested sprite `0x10`. CDB proved that the resulting pointer was in
reserved, uncommitted memory. `front.anm` has two entries and 33 total sprites;
its sprite `0x10` was committed and initialized.

The exact target resolves every one of the function's eight GUI pointer loads
to `0x0160F434`, which is canonical `g_Gui @ 0x0160F428` plus the asserted
`frontAnm @ +0x0C`. The old match manifest instead declared `g_Gui @
0x0160F424`, while the old source emitted the `stageTextAnm @ +0x10` addend.
Those two four-byte errors canceled and produced the right absolute target
field, so relocation replay could report an exact instruction stream while
preserving the wrong semantic owner in the rebuilt link.

Production now names `frontAnm`, and the match unit names the independently
mapped `g_Gui` base. The focused result remains **234 / 234 exact**, now with
the correct base and addend. The linked verifier counts eight front-owner loads
and rejects any stage-text load. This incident is why a relocation-normalized
function result and a successful link are necessary but insufficient runtime
evidence.

## Spell-background field disguised as a global

The later native gameplay exit exposed the complementary alias failure. Source
declared `g_SpellcardBackgroundAnm` as standalone zero-fill storage and
`Spellcard::StartSpell` dereferenced it. Nothing assigned that storage, so CDB
stopped in `AnmLoaded::SetAndExecuteScriptIdx` with null `this` when the ECL
started a spell card.

Target address `0x00577EB8` had been labeled as that standalone global, but the
independently established manager layout proves
`g_EffectManager @ 0x004ECE60 + stageEffectAnm @ 0x8B058 = 0x00577EB8`.
Target `EffectManager::LoadEffectResources @ 0x004284B0` writes the field after
preloading the stage effect ANM, and target `Spellcard::StartSpell @
0x004152A0` reads the same address. The address is therefore an interior field,
not an independent storage owner.

Production now uses `g_EffectManager.stageEffectAnm`; the standalone
declaration and global-ledger row are removed. The match relocation names the
manager base and records field addend `0x8B058`, resolving to the unchanged
target address. Focused replay remains **2,483 / 2,483 exact**. The linked
verifier additionally fails if the legacy public symbol reappears or if either
the target or rebuilt `StartSpell` lacks exactly one load of the canonical
field.

## Player gauge fields disguised as an array

Normal Stage 1 and Stage Practice 2 exposed a third aggregate-owner failure:
score rose by approximately 6,000 visible points per second at gauge zero,
continued after a hit, and ordinary graze counts increased too quickly. Target
address arithmetic establishes the owner without relying on those symptoms:

```text
g_GameManager                          0x0160F508
+ offsetof(GameManager,
    youkaiGaugeHumanLimit)             0x003DDF8
= first gauge-bound field              0x0164D300
```

The next five signed 16-bit members end at `0x0164D30B`, exactly covering the
former `g_PlayerGaugeBounds[6]` range. Target `Player::AddedCallback @
0x0044D650` writes its 21 default and shot-specific values to those six
addresses. The target `GaugeIsExtremelyHuman`, `GaugeIsExtremelyYoukai`, and
moderate variants read the corresponding `GameManager` member offsets.

In native hash `87edf9dc...1832d73`, linker-map and live-process evidence
showed the false standalone array at `0x017E3128` with correct default values,
but the actual manager fields at `0x0165A5A8` were all zero. Thus gauge zero
satisfied the extreme-human comparison. `Player::OnUpdate` awarded 100 visible
points each active frame, and `Player::AwardGraze` used its extreme-human gain
of 3 instead of its ordinary gain of 1. `Player::Die` setting the gauge back to
zero could not stop the error.

A hash-pinned diagnostic wrote the 12 target bytes only to the live manager
fields and read them back. Read-only sampling then showed authoritative score
stop, display score catch up to a zero step, and the user confirmed the visible
score was normal. This isolates the owner causally but does not replace a
production rebuild. Source now writes the six named members directly; the
match unit records `g_GameManager` plus the individual field addends, the false
global and Linux fixed-layout alias are gone, and the linked verifier rejects
their return.

## Static verification checkpoint

- the native VC7 link succeeds without unresolved-symbol forcing;
- the linked table contains all 66 target script IDs;
- every non-null callback resolves through `build/th08.map` to its intended
  semantic function, with the documented row-41 adapter;
- the Last Spell count, nine stage bonuses, and twelve dialogue palettes match
  the target byte-for-byte;
- `Spellcard::StartSpell` loads the stage-effect ANM through its canonical
  manager field and has no standalone background-ANM storage; and
- `Player::AddedCallback` directs all 21 gauge-bound writes to the six
  contiguous `GameManager` fields and has no standalone gauge-bound storage;
  and
- focused `Spellcard::StartSpell` replay passes **2,483 / 2,483 exact**, while
  focused `Player::AddedCallback` replay passes **1,537 / 1,537 exact**.

A subsequent single-job cold replay rebuilt all 75 configured objects and
passed **1,106 / 1,106 exact**. The fresh normal PE32 i386 GUI link has SHA-256
`5e217f010c78d3fb1c1459f7127c917dfe039d24a9253f90badb00b8cf556527`.
That normal artifact remains exact-facing build evidence. The native playtest
artifact is rebuilt with `--build-type bugfix` so reconstructed score/replay
headers are accepted by version rather than an impossible retail executable
size/checksum identity. The current bugfix link is an 898,048-byte PE32 i386
GUI executable with SHA-256
`cf32bd1f5202f866a749b40c2aaced94b9c7fe357df0dc5bb20867e1726c6e26`; it
passes the expanded linked owner/data verifier. The isolated Windows directory
was completely cleared before this artifact, the retail DAT files, a fresh
windowed configuration, and the ordinary unpatched launcher were deployed.
No preserve-lives helper or earlier score/replay state was copied.

The user confirmed Remilia's X bomb and continued gameplay past the repaired
selection transition on hash `a583f9a5...56dcad`, closing RT-001 and RT-002.
The later completed run closed RT-005, and the current live-stage/Stage 4 test
closed RT-006. RT-007 remains **fixed / confirmation pending**. The RT-008
gauge-bound repair described above passed the fresh VC7 build, focused/cold
exact replay, linked verification, and clean deployment. The user then repeated
the score/graze path on the clean, unpatched hash `cf32bd1f...6c6e26` and
confirmed normal behavior, closing RT-008.

## Audit limits

This initialized-data scan catches one important owner class; it cannot prove
that two valid objects with identical layouts are not confused, that an
asynchronous publication/teardown is ordered correctly, or that a private
switch-table relocation names the correct destination. Native runtime testing
must continue across repeated title/gameplay transitions, stage loads, replay
save/load, and process shutdown. New failures are evidence for a bounded owner,
lifetime, ABI, or TU investigation—not permission to add speculative guards or
duplicate storage.
