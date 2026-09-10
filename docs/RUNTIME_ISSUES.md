# Native Windows runtime issue ledger

This ledger tracks whole-program failures in the VC7-built Windows i386
reconstruction separately from function exactness and from modern-port
behavior. A successful link or accepted object comparison does not close a
runtime issue.

Status meanings:

- `closed`: the cause is established and the repaired native path was exercised;
- `fixed / confirmation pending`: the repair is built and statically verified,
  but the exact user-observed native interaction still needs repetition;
- `open`: the cause is not established for the native VC7 artifact.

## Issue index

| ID | Status | User-visible symptom | Evidence and disposition |
| --- | --- | --- | --- |
| RT-001 | fixed / confirmation pending | Remilia's X-bomb trail did not render, while Sakuya's did. | Native production owned a zero-filled `g_EffectTemplates[66]`; target row 53 requires script 88 plus `UpdateFadingRadialTrail` and `InitializeRadialTrail`. The complete target table is now owned by `EffectManager.cpp` and passes linked-data verification. Repeat the Remilia path on the repaired VC7 image. |
| RT-002 | open | A tested game process exited unexpectedly. | The only symbolized fault so far belongs to the earlier MinGW modern executable: an access violation in `Gui::CopyEnemyNameTexture` after a title/gameplay transition, consistent with a stale `AnmLoadedSprite` view. This is not yet a proven VC7 reconstruction defect. Preserve the modern crash evidence, then reproduce or disprove it on the native artifact before changing production lifetime logic. |
| RT-003 | fixed / confirmation pending | No specific symptom was isolated; the owner audit found additional latent data defects. | `g_LastSpellCount`, nine stage-clear bonuses, and twelve dialogue palettes were zero-filled in the native link. Their target initializers now live in `Spellcard.cpp`/`Gui.cpp` and match the target bytes. Exercise Last Spell selection, stage-clear calculation, and dialogue across multiple shot types. |

## Evidence separation for the unexpected exit

The earlier crash report identified a MinGW-linked instruction in
`Gui::CopyEnemyNameTexture`, not a target or current VC7-linked address. The
modern executable also had startup initialization and ownership bridges that
the native reconstruction does not share. The report is useful as a candidate
lifetime path, but it cannot justify a VC7 source change by itself.

For the native run, record:

1. the SHA-256 of `th08-reconstructed.exe`;
2. the character/team, mode, stage, and exact transition;
3. whether the last action was death, bomb, dialogue, pause/ESC, retry, or a
   title restart;
4. the last correctly rendered frame and whether audio continued; and
5. Windows Application Error/WER or debugger exception address and module.

If the native image remains alive through the same transition, keep the modern
failure in the port lane. If it faults, resolve the exception RVA through the
matching `build/th08.map`, inspect the target lifetime, and repair only the
bounded owner/publication/teardown family supported by that evidence.

## Current static checkpoint

The current source checkpoint has passed the focused native link and
`EffectManager.obj`/`Gui.obj`/`SpellCard.obj` replay (**125 / 125 exact**), the
linked runtime-data verifier, and a single-job cold rebuild/replay of all 75
configured objects (**1,106 / 1,106 exact**). The fresh normal link is a
902,144-byte PE32 i386 GUI executable with SHA-256
`db11de130f007bdcb793550c2a4b937f30968d17787e5acf511fe89b80bf9a20`.
Manual confirmation uses the isolated deployment described in
`WINDOWS_I386_RUNTIME.md`.
