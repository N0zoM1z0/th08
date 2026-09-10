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
loads in `Gui::CopyEnemyNameTexture` select `Gui::frontAnm`. This is
deliberately stricter than checking source text alone.

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

## Static verification checkpoint

- the native VC7 link succeeds without unresolved-symbol forcing;
- the linked table contains all 66 target script IDs;
- every non-null callback resolves through `build/th08.map` to its intended
  semantic function, with the documented row-41 adapter;
- the Last Spell count, nine stage bonuses, and twelve dialogue palettes match
  the target byte-for-byte; and
- focused accepted-unit replay for `EffectManager.obj`, `Gui.obj`, and
  `SpellCard.obj` passes **125 / 125 exact**.

A subsequent single-job cold replay rebuilt all 75 configured objects and
passed **1,106 / 1,106 exact**. The fresh normal link produced a 902,144-byte
PE32 i386 GUI executable with SHA-256
`46b7fa54b96e76ad11dbde86d56f582e96dfd5121c758f90caea8f865efd70e7`.
That normal artifact remains exact-facing build evidence. The native playtest
artifact is rebuilt with `--build-type bugfix` so reconstructed score/replay
headers are accepted by version rather than an impossible retail executable
size/checksum identity. The current bugfix link is an 898,048-byte PE32 i386
GUI executable with SHA-256
`a583f9a5748ae2d112243c957e013e04f7265224e97ab9d911429a6f5756dcad`; it
passes the expanded linked owner/data verifier. Its preserve-lives patch gate
passed on Windows and the process remained alive for a 12-second startup smoke;
the exact gameplay selection that exposed RT-002 remains the runtime
confirmation gate.

The exact user interactions remain runtime confirmation gates. Until Remilia's
X bomb and the gameplay selection are replayed on the repaired native VC7
executable, RT-001 and RT-002 remain **fixed / confirmation pending** in
`RUNTIME_ISSUES.md`.

## Audit limits

This initialized-data scan catches one important owner class; it cannot prove
that two valid objects with identical layouts are not confused, that an
asynchronous publication/teardown is ordered correctly, or that a private
switch-table relocation names the correct destination. Native runtime testing
must continue across repeated title/gameplay transitions, stage loads, replay
save/load, and process shutdown. New failures are evidence for a bounded owner,
lifetime, ABI, or TU investigation—not permission to add speculative guards or
duplicate storage.
