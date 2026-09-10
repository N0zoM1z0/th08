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
| RT-002 | fixed / confirmation pending | Selecting a team/run exited the native game at the gameplay transition. | Native hash `c394035e...f9ce7` reproduced `0xC0000005` at linked `Gui::CopyEnemyNameTexture + 0x77` (`0x00429589`). The function selected sprite `0x10` from six-sprite `stg1txt.anm`; the resulting pointer lay in reserved, uncommitted memory. Target instructions instead read `g_Gui + 0x0C` (`frontAnm`), whose two entries provide 33 sprites. Production now uses `frontAnm`, all eight DIR32 relocations name the real `g_Gui @ 0x0160F428` base, and the linked verifier guards the owner. Repeat the selection on hash `a583f9a5...56dcad`. |
| RT-003 | fixed / confirmation pending | No specific symptom was isolated; the owner audit found additional latent data defects. | `g_LastSpellCount`, nine stage-clear bonuses, and twelve dialogue palettes were zero-filled in the native link. Their target initializers now live in `Spellcard.cpp`/`Gui.cpp` and match the target bytes. Exercise Last Spell selection, stage-clear calculation, and dialogue across multiple shot types. |
| RT-004 | closed | The normal VC7 reconstruction exited about three seconds after startup. | CDB caught `0xC0000005` at linked CRT `strncmp + 0x1F` (`0x004ABA5F`), called by `Supervisor::CheckVersion + 0xC4`. Archive open/decryption returned valid data. The normal executable's non-retail size/checksum made the original whitelist scan pass the final `0100d` record; its unchecked `strchr(..., '\n') + 1` then produced `0x1`. The native `bugfix` build uses the already reconstructed `FIX_REALLY_BAD_BUGS` version-string acceptance path. Hash `c394035e...f9ce7` remained alive at title for a 12-second Windows smoke test. |

## Native enemy-name texture exit

The earlier MinGW report was candidate evidence only. The same transition was
then reproduced on the native VC7/i386 artifact, making the failure an RT-002
production defect. Windows Application Error recorded fault RVA `0x29589` for
hash `c394035e...f9ce7`; CDB stopped at linked VA `0x00429589` and resolved the
stack to `Gui::CopyEnemyNameTexture`, called with sprite index `0x10`.

The stopped process separated a stale lifetime hypothesis from the actual
owner error:

1. `g_Gui.stageTextAnm` was live and identified `data/text/stg1txt.png`, but
   its ANM header declared only six sprites;
2. `GetSprite(0x10)` therefore returned an address in an uncommitted part of
   the allocation, and the first coordinate read faulted;
3. `g_Gui.frontAnm` identified `data/front/front.png`; its two ANM entries
   declared `0x10 + 0x11 = 33` sprites, and sprite `0x10` was committed and
   initialized; and
4. target `Gui::CopyEnemyNameTexture @ 0x00437F5C` loads absolute address
   `0x0160F434` eight times: canonical `g_Gui @ 0x0160F428` plus `0x0C`, the
   asserted offset of `frontAnm`.

The accepted match unit had encoded the false base `0x0160F424`. Together with
the stale source addend `+0x10`, that produced the right target field address
and concealed the semantic error. The corrected source emits addend `+0x0C`,
and all eight unit relocations now use the independently mapped `g_Gui @
0x0160F428` base. This remains **fixed / confirmation pending** until the user
repeats the exact selection on the repaired artifact.

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

The current source checkpoint has passed focused
`Gui::CopyEnemyNameTexture` replay (**234 / 234 exact**), the linked runtime
owner/data verifier, and a single-job cold rebuild/replay of all 75 configured
objects (**1,106 / 1,106 exact**). The fresh normal link is a 902,144-byte PE32
i386 GUI executable with SHA-256
`46b7fa54b96e76ad11dbde86d56f582e96dfd5121c758f90caea8f865efd70e7`.
The fresh bugfix runtime link is an 898,048-byte PE32 i386 GUI executable with
SHA-256
`a583f9a5748ae2d112243c957e013e04f7265224e97ab9d911429a6f5756dcad`.
It passes linked runtime verification, including all eight enemy-name owner
loads. The preserve-lives launcher is repinned to this hash and the unchanged
RVA/instruction bytes; its patch/read-back gate passed and the process remained
alive through a 12-second Windows startup smoke. The exact RT-002 selection
remains the runtime confirmation gate on the repaired image; broader manual
confirmation uses the isolated deployment described in
`WINDOWS_I386_RUNTIME.md`.
