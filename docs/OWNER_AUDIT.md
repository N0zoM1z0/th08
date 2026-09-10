# Native runtime data-owner audit

This document records the initialized-data owner audit opened by native
Windows i386 playtesting on 2026-09-10. The only target is the original Japanese
TH08 1.00d executable, SHA-256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`.

The audit is whole-program reconstruction evidence. It does not grant new
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
load `EffectManager::stageEffectAnm`. This is deliberately stricter than
checking source text alone.

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

## Static verification checkpoint

- the native VC7 link succeeds without unresolved-symbol forcing;
- the linked table contains all 66 target script IDs;
- every non-null callback resolves through `build/th08.map` to its intended
  semantic function, with the documented row-41 adapter;
- the Last Spell count, nine stage bonuses, and twelve dialogue palettes match
  the target byte-for-byte;
- `Spellcard::StartSpell` loads the stage-effect ANM through its canonical
  manager field and has no standalone background-ANM storage; and
- focused `Spellcard::StartSpell` replay passes **2,483 / 2,483 exact**.

A subsequent single-job cold replay rebuilt all 75 configured objects and
passed **1,106 / 1,106 exact**. The fresh normal link produced a 902,144-byte
PE32 i386 GUI executable with SHA-256
`beab5f36302c8334551dd6f86fa4f65d3fbc0d2e5055f4a73f86dcc5bd77240c`.
That normal artifact remains exact-facing build evidence. The native playtest
artifact is rebuilt with `--build-type bugfix` so reconstructed score/replay
headers are accepted by version rather than an impossible retail executable
size/checksum identity. The current bugfix link is an 898,048-byte PE32 i386
GUI executable with SHA-256
`e8b7107a0d45c9e319345d131ec5d7f713d38d7a2707d61f52eed0d12161d73d`; it
passes the expanded linked owner/data verifier. Its preserve-lives patch gate
passed on Windows and the process remained alive for a 12-second startup smoke;
spell-card entry remains the RT-005 runtime confirmation gate.

The user confirmed Remilia's X bomb and continued gameplay past the repaired
selection transition on hash `a583f9a5...56dcad`, closing RT-001 and RT-002.
The later spell-background repair remains **fixed / confirmation pending** as
RT-005 in `RUNTIME_ISSUES.md`.

## Audit limits

This initialized-data scan catches one important owner class; it cannot prove
that two valid objects with identical layouts are not confused, that an
asynchronous publication/teardown is ordered correctly, or that a private
switch-table relocation names the correct destination. Native runtime testing
must continue across repeated title/gameplay transitions, stage loads, replay
save/load, and process shutdown. New failures are evidence for a bounded owner,
lifetime, ABI, or TU investigation—not permission to add speculative guards or
duplicate storage.
