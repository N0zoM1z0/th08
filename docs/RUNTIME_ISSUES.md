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
| RT-002 | open | The earlier modern game process exited after a title/gameplay transition. | The symbolized fault belongs to the MinGW modern executable: an access violation in `Gui::CopyEnemyNameTexture`, consistent with a stale `AnmLoadedSprite` view. It is separate from RT-004 and is not yet a proven VC7 reconstruction defect. Preserve the modern crash evidence, then reproduce or disprove that transition on the native artifact before changing production lifetime logic. |
| RT-003 | fixed / confirmation pending | No specific symptom was isolated; the owner audit found additional latent data defects. | `g_LastSpellCount`, nine stage-clear bonuses, and twelve dialogue palettes were zero-filled in the native link. Their target initializers now live in `Spellcard.cpp`/`Gui.cpp` and match the target bytes. Exercise Last Spell selection, stage-clear calculation, and dialogue across multiple shot types. |
| RT-004 | closed | The normal VC7 reconstruction exited about three seconds after startup. | CDB caught `0xC0000005` at linked CRT `strncmp + 0x1F` (`0x004ABA5F`), called by `Supervisor::CheckVersion + 0xC4`. Archive open/decryption returned valid data. The normal executable's non-retail size/checksum made the original whitelist scan pass the final `0100d` record; its unchecked `strchr(..., '\n') + 1` then produced `0x1`. The native `bugfix` build uses the already reconstructed `FIX_REALLY_BAD_BUGS` version-string acceptance path. Hash `c394035e...f9ce7` remained alive at title for a 12-second Windows smoke test. |

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

## Native normal-build startup exit

The native startup failure is independently reproduced and closed. Its fault
address happened to be in a linked VC7 CRT function, but the caller arguments
identified the source condition:

1. `FileSystem::OpenFile("th08_0100d.ver")` returned a valid decrypted heap
   pointer and `Supervisor::LoadDat` stored it with size `0x168`;
2. `Supervisor::CheckVersion` iterated the valid version records through
   `0100d 840704 2724749753`;
3. a reconstruction cannot match that retail size/checksum pair, so the normal
   build took the target loop's next-record path; and
4. the table had no next newline, so `strchr` returned null, `+1` became `0x1`,
   and the next `strncmp` faulted.

This does not justify altering the exact-facing implementation or pretending
that the reconstruction has the retail checksum. Runtime deployment uses the
repository's native VC7 `bugfix` mode, whose existing evidence-backed branch
accepts a matching `0100d` version. The normal mode remains the comparator/link
lane; the bugfix mode remains native prerequisite evidence, not port evidence.

## Current static checkpoint

The current source checkpoint has passed the focused native link and
`EffectManager.obj`/`Gui.obj`/`SpellCard.obj` replay (**125 / 125 exact**), the
linked runtime-data verifier, and a single-job cold rebuild/replay of all 75
configured objects (**1,106 / 1,106 exact**). The fresh normal link is a
902,144-byte PE32 i386 GUI executable with SHA-256
`db11de130f007bdcb793550c2a4b937f30968d17787e5acf511fe89b80bf9a20`.
The fresh bugfix runtime link is an 898,048-byte PE32 i386 GUI executable with
SHA-256
`c394035eb81237dd1fa8884549d7cea4f4e9901348a1a4e6c2b80e1cd02f9ce7`.
It passed linked runtime-data verification and remained alive for the native
Windows startup smoke test. The hash-pinned preserve-lives launcher also
installed its one-byte process patch successfully and the spawned process
remained alive for a separate 12-second smoke test; the old normal artifact was
rejected before launch by its hash gate. Broader manual confirmation uses the
isolated deployment described in `WINDOWS_I386_RUNTIME.md`.
