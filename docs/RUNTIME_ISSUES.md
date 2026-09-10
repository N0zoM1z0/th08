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
| RT-001 | closed | Remilia's X-bomb trail did not render, while Sakuya's did. | Native production owned a zero-filled `g_EffectTemplates[66]`; target row 53 requires script 88 plus `UpdateFadingRadialTrail` and `InitializeRadialTrail`. The complete target table is now owned by `EffectManager.cpp`, passes linked-data verification, and the user confirmed Remilia's X-bomb effect on native hash `a583f9a5...56dcad`. |
| RT-002 | closed | Selecting a team/run exited the native game at the gameplay transition. | Native hash `c394035e...f9ce7` reproduced `0xC0000005` at linked `Gui::CopyEnemyNameTexture + 0x77` (`0x00429589`). The function selected sprite `0x10` from six-sprite `stg1txt.anm`; the resulting pointer lay in reserved, uncommitted memory. Target instructions instead read `g_Gui + 0x0C` (`frontAnm`), whose two entries provide 33 sprites. Production now uses `frontAnm`, all eight DIR32 relocations name the real `g_Gui @ 0x0160F428` base, and the linked verifier guards the owner. The user entered and continued gameplay on repaired hash `a583f9a5...56dcad`, closing the original transition path. |
| RT-003 | fixed / confirmation pending | No specific symptom was isolated; the owner audit found additional latent data defects. | `g_LastSpellCount`, nine stage-clear bonuses, and twelve dialogue palettes were zero-filled in the native link. Their target initializers now live in `Spellcard.cpp`/`Gui.cpp` and match the target bytes. Exercise Last Spell selection, stage-clear calculation, and dialogue across multiple shot types. |
| RT-004 | closed | The normal VC7 reconstruction exited about three seconds after startup. | CDB caught `0xC0000005` at linked CRT `strncmp + 0x1F` (`0x004ABA5F`), called by `Supervisor::CheckVersion + 0xC4`. Archive open/decryption returned valid data. The normal executable's non-retail size/checksum made the original whitelist scan pass the final `0100d` record; its unchecked `strchr(..., '\n') + 1` then produced `0x1`. The native `bugfix` build uses the already reconstructed `FIX_REALLY_BAD_BUGS` version-string acceptance path. Hash `c394035e...f9ce7` remained alive at title for a 12-second Windows smoke test. |
| RT-005 | fixed / confirmation pending | Gameplay exited later, after the repaired selection and X-bomb paths had run. | Native hash `a583f9a5...56dcad` reproduced `0xC0000005` at linked `AnmLoaded::SetAndExecuteScriptIdx + 0x24` (`0x00406994`) with null `this`. CDB traced the call through `Spellcard::StartSpell`: source referenced a never-assigned standalone `g_SpellcardBackgroundAnm`, but target `0x00577EB8` is `g_EffectManager + 0x8B058`, the asserted `stageEffectAnm` field populated by `EffectManager::LoadEffectResources`. Production and the relocation manifest now use the aggregate owner, the standalone storage/mapping is gone, and the linked verifier guards the load. Repeat a spell-card start on hash `e8b7107a...161d73d`. |

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
0x0160F428` base. The user subsequently entered and continued gameplay on hash
`a583f9a5...56dcad`, so the originally reported selection transition is closed
independently of RT-005.

## Native spell-background exit

The later gameplay exit on native hash `a583f9a5...56dcad` was a second owner
defect, not a recurrence of the selection crash. Windows Error Reporting
recorded exception `0xC0000005` at module RVA `0x6994`. CDB stopped at linked
`AnmLoaded::SetAndExecuteScriptIdx + 0x24` (`0x00406994`) and captured null
`ecx`, VM `0x004F1DA8`, and script index zero. The caller chain was
`Spellcard::StartSpell + 0x25F`, `StartEnemySpell`, `EclManager::RunEcl`,
`EnemyManager::OnUpdate`, and the normal calc/render chain.

The VM belongs to `g_Background.spellVms[0]`; the invalid receiver came from
linked standalone `g_SpellcardBackgroundAnm @ 0x004F78D0`, which had no writer
and remained zero. Target `Spellcard::StartSpell @ 0x004152A0` instead loads
absolute `0x00577EB8`. That address is exactly canonical
`g_EffectManager @ 0x004ECE60 + stageEffectAnm @ 0x8B058`, and target
`EffectManager::LoadEffectResources @ 0x004284B0` populates that field.

Production now names the aggregate member directly. The relocation at
`StartSpell + 0x256` names canonical `g_EffectManager` and carries addend
`0x8B058`; the resolved target address remains `0x00577EB8`. The false global
and its global-ledger row are removed. Focused replay remains **2,483 / 2,483
exact**, and the linked verifier rejects a resurrected standalone symbol or a
missing aggregate-field load.

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
`Spellcard::StartSpell` replay (**2,483 / 2,483 exact**), the linked runtime
owner/data verifier, and a single-job cold rebuild/replay of all 75 configured
objects (**1,106 / 1,106 exact**). The fresh normal link is a 902,144-byte PE32
i386 GUI executable with SHA-256
`beab5f36302c8334551dd6f86fa4f65d3fbc0d2e5055f4a73f86dcc5bd77240c`.
The fresh bugfix runtime link is an 898,048-byte PE32 i386 GUI executable with
SHA-256
`e8b7107a0d45c9e319345d131ec5d7f713d38d7a2707d61f52eed0d12161d73d`.
It passes linked runtime verification, including all eight enemy-name owner
loads and the spell-background aggregate load. The preserve-lives launcher is
repinned to this hash and the unchanged RVA/instruction bytes; its patch/read-
back gate passed and the process remained responsive through a 12-second
Windows startup smoke. Spell-card entry is the RT-005 confirmation gate on the
repaired image; broader manual confirmation uses the isolated deployment in
`WINDOWS_I386_RUNTIME.md`.
