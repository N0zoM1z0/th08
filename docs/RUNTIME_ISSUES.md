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
| RT-001 | closed | Remilia's X-bomb trail did not render, while Sakuya's did. | Native production owned a zero-filled `g_EffectTemplates[66]`; target row 53 requires script 88 plus `UpdateFadingRadialTrail` and `InitializeRadialTrail`. The complete target table is now owned by `EffectManager.cpp`, passes linked-data verification, and the user confirmed Remilia's X-bomb effect on native hash `a583f9a5...56dcad`. Follow-up testing also confirmed the Reimu/Yukari, Marisa/Alice, and Sakuya/Remilia X-bomb paths. |
| RT-002 | closed | Selecting a team/run exited the native game at the gameplay transition. | Native hash `c394035e...f9ce7` reproduced `0xC0000005` at linked `Gui::CopyEnemyNameTexture + 0x77` (`0x00429589`). The function selected sprite `0x10` from six-sprite `stg1txt.anm`; the resulting pointer lay in reserved, uncommitted memory. Target instructions instead read `g_Gui + 0x0C` (`frontAnm`), whose two entries provide 33 sprites. Production now uses `frontAnm`, all eight DIR32 relocations name the real `g_Gui @ 0x0160F428` base, and the linked verifier guards the owner. The user entered and continued gameplay on repaired hash `a583f9a5...56dcad`, closing the original transition path. |
| RT-003 | fixed / confirmation pending | No specific symptom was isolated; the owner audit found additional latent data defects. | `g_LastSpellCount`, nine stage-clear bonuses, and twelve dialogue palettes were zero-filled in the native link. Their target initializers now live in `Spellcard.cpp`/`Gui.cpp` and match the target bytes. Exercise Last Spell selection, stage-clear calculation, and dialogue across multiple shot types. |
| RT-004 | closed | The normal VC7 reconstruction exited about three seconds after startup. | CDB caught `0xC0000005` at linked CRT `strncmp + 0x1F` (`0x004ABA5F`), called by `Supervisor::CheckVersion + 0xC4`. Archive open/decryption returned valid data. The normal executable's non-retail size/checksum made the original whitelist scan pass the final `0100d` record; its unchecked `strchr(..., '\n') + 1` then produced `0x1`. The native `bugfix` build uses the already reconstructed `FIX_REALLY_BAD_BUGS` version-string acceptance path. Hash `c394035e...f9ce7` remained alive at title for a 12-second Windows smoke test. |
| RT-005 | closed | Gameplay exited later, after the repaired selection and X-bomb paths had run. | Native hash `a583f9a5...56dcad` reproduced `0xC0000005` at linked `AnmLoaded::SetAndExecuteScriptIdx + 0x24` (`0x00406994`) with null `this`. CDB traced the call through `Spellcard::StartSpell`: source referenced a never-assigned standalone `g_SpellcardBackgroundAnm`, but target `0x00577EB8` is `g_EffectManager + 0x8B058`, the asserted `stageEffectAnm` field populated by `EffectManager::LoadEffectResources`. Production and the relocation manifest now use the aggregate owner, the standalone storage/mapping is gone, and the linked verifier guards the load. The user subsequently completed the repaired `e8b7107a...161d73d` run through Final and saved replay slot 3, exercising repeated spell-card starts without recurrence. |
| RT-006 | closed | Dialogue showed a flat `RGB(64,64,96)` or black playfield; later frames accumulated player and portrait trails. | Target Background draw callbacks call `Gui::IsStageFinished @ 0x00437D87` at all three stage-layer gates. Source called `Gui::IsDialoguePresent @ 0x004358BB`, while the match manifest declared the target address and normalized the wrong source call into an exact result. The linked repair passes the native call verifier, and the user confirmed the live stage background plus corrected Stage 4 Reimu rendering on native hash `87edf9dc...1832d73`. |
| RT-007 | fixed / confirmation pending | Near the final Last Spell, self-shot emission looked wrong and dense white digit-like textures covered the playfield edges. | The old source called unsigned RNG from target `SpawnRandomizedShot`'s signed-RNG site and sent point-star/time-orb values to a 720-entry score-popup pool instead of the target's three-entry player-point pool. The corrected callees are exact and pass final-link verification on hash `87edf9dc...1832d73`. The previous isolated run's replay slot 3 had SHA-256 `1ec94058...4fbc7`; it was deliberately not carried into the later clean deployment, so confirmation now requires a fresh run. |
| RT-008 | fixed / confirmation pending | Score rose by about 6,000 points per second from the start of normal and Stage Practice runs, continued after a player hit, and graze counts increased too quickly. | Target `g_PlayerGaugeBounds @ 0x0164D300` is not standalone storage: it is exactly the six contiguous gauge limit/threshold fields at `g_GameManager @ 0x0160F508 + 0x3DDF8..0x3DE02`. The native linker split the source array from those fields, leaving the predicates' manager thresholds zero. Read-only process sampling proved the split; a 12-byte process-local field repair immediately stopped authoritative score growth at gauge zero. The production repair passes focused and cold exact replay plus final-link owner verification and is deployed unpatched as native hash `cf32bd1f...6c6e26`; repeat normal Stage 1 and Stage Practice from gauge zero. |

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
missing aggregate-field load. The user then completed the repaired native run
through Final and saved replay slot 3; repeated spell-card starts did not
reproduce the crash, so RT-005 is closed.

## Native dialogue background and accumulated portraits

Four screenshots from the native hash `e8b7107a...161d73d` established one
coherent rendering failure. Stage 2 dialogue reduced the playfield to the
stage clear color `RGB(64,64,96)`; another dialogue showed black; later frames
retained many historical positions of Reimu's player sprite and moving
portraits over a white playfield. The portraits and text themselves loaded,
so this was not a missing dialogue texture.

Target-safe disassembly resolves the three stage-layer gate calls at
`Background::OnDrawHighPrio + 0x1F4`, `+0x419`, and
`Background::OnDrawLowPrio + 0x1E` to
`Gui::IsStageFinished @ 0x00437D87`. The old linked reconstruction instead
resolved the same calls to its `Gui::IsDialoguePresent`. That predicate became
true during ordinary dialogue, so all four Background object layers and the
stage VMs stopped drawing while later player/portrait layers continued to
composite onto prior backbuffer contents.

The match units had the source symbol `IsDialoguePresent` but target address
`0x00437D87`. Relocation replay therefore replaced the source displacement
with the declared target displacement and accepted the surrounding exact
instructions, concealing the wrong final-link call. Production now calls
`IsStageFinished`, the three relocation symbols agree with the independently
mapped target, and the linked verifier decodes all three calls. The modern
Linux dialogue-snapshot workaround has also been removed: it compensated for
the wrong reconstruction semantics, while the target continues drawing the
stage during dialogue.

The same-symbol REL32 audit found eight additional latent call mismatches: four
item popup-pool selections, pulsing-trail timer parity, Fantasy Orb frame-40
equality, randomized-shot signed RNG, and the Retry menu's Spell Practice
gate. Their source callees and manifests now name the target-mapped functions.
`validate-tracking.py` rejects any future decorated REL32 symbol assigned to
multiple target addresses, and the native linked verifier checks all eleven
repaired calls. These latent sites are static repairs until exercised on a new
native artifact; only the user-observed dialogue path belongs to RT-006.

## Native final-stage self-shot and popup corruption

The final screenshot from native hash `e8b7107a...161d73d` was captured at a
failed Last Spell. The user reported incorrect self-shot emission/rendering,
while the image showed dense white digit-like texture clusters around the
playfield edges. Replay slot 3 was saved as `th8_03.rpy`; its retained test-copy
SHA-256 is `1ec940587868f40f30dd1ec9249c0f3f08f08ea597bb8c79e16931179024fbc7`.

Two independently target-confirmed REL32 errors in the old artifact align with
the observation. `SpawnRandomizedShot @ 0x004501B0 + 0x37` calls
`Rng::GetRandomF32Signed @ 0x0043ED80` in the target, but the old source called
the unsigned `GetRandomF32`; this shifts the randomized self-shot angle range.
For the visual flood, target point-star pickup and `Item::CollectTimeOrb` use
`CreatePlayerPointPopup`, whose pool has only three rotating slots. The old
source used the 720-entry ordinary score-popup pool, allowing many transient
numeric sprites to remain visible simultaneously during dense collection.
The two full-power notices had the inverse pool error.

Production and all affected relocation symbols now select the target callees.
Focused replay and the cold aggregate comparison are exact. The native verifier
decodes the eight repaired REL32 calls and additionally checks the target-owned
9-entry spawn, 6-entry update, 2-entry draw, and 3-entry collision callback
tables in the final PE. `config/reccmp-globals.csv` now names those four tables
at their actual target addresses rather than the former shifted
update/render/timer labels. The screenshot-to-popup explanation remains a
strong causal inference until the repaired path is exercised again. The old
slot-3 replay was intentionally excluded when the isolated directory was
cleaned, so RT-007 is not closed yet.

## Native gauge-bound owner and inflated score/graze

Normal Stage 1 and Stage Practice 2 both reproduced stable score growth of
approximately 6,000 visible points per second from their initial gauge state.
The score continued after a player hit, and ordinary grazing increased the
counter much faster than expected. This was not display interpolation alone:
read-only `ReadProcessMemory` sampling of native hash
`87edf9dc...1832d73` showed its authoritative internal score increasing by
approximately 600 units per second while `playerState` crossed dying, spawning,
invulnerable, and alive with gauge zero.

The owner error is exact. Target `g_GameManager @ 0x0160F508` has six contiguous
signed 16-bit fields at offsets `0x3DDF8..0x3DE02`. Their addresses are
`0x0164D300..0x0164D30B`, exactly the range formerly mapped as standalone
`g_PlayerGaugeBounds[6]`. Target `Player::AddedCallback @ 0x0044D650` writes all
21 initial/default/shot-specific values to those addresses. The target gauge
predicates then read `GameManager + 0x3DDFC`, `+0x3DDFE`, `+0x3DE00`, and
`+0x3DE02`.

The old production source instead wrote a separately linked
`g_PlayerGaugeBounds` array. In the deployed PE that array lived at
`0x017E3128` and contained the correct default values
`[-10000, 10000, -8000, 8000, -2000, 2000]`, while the actual fields at
`g_GameManager + 0x3DDF8` remained `[0, 0, 0, 0, 0, 0]`. Gauge zero therefore
satisfied `GaugeIsExtremelyHuman()` (`0 <= 0`). `Player::OnUpdate` awarded 100
visible points every active frame, including after `Player::Die` restored the
gauge to zero, and `Player::AwardGraze` selected its extreme-human gain of 3
instead of the ordinary gain of 1.

A hash-pinned, process-local diagnostic wrote only the six target values into
the real manager fields and read them back. On the next samples, authoritative
score stopped at internal value `3994801`; `displayScore` caught up, its step
became zero, and score remained unchanged across the same player states at
gauge zero. This runtime experiment is causal evidence, not the production
repair. Production now writes the six named `GameManager` fields directly,
removes the false global and Linux fixed-address alias, and records the manager
base plus field addend in all 21 match relocations. The native linked verifier
rejects a resurrected standalone symbol and checks every final write. A fresh
VC7 build now passes focused `Player::AddedCallback` replay (**1,537 / 1,537
exact**), the required single-job cold replay (**1,106 / 1,106 exact**), and the
final-link owner verifier. The unpatched bugfix artifact with SHA-256
`cf32bd1f5202f866a749b40c2aaced94b9c7fe357df0dc5bb20867e1726c6e26`
is deployed for native repetition, so RT-008 is fixed with confirmation pending.

## Target-confirmed score movement while idle

A score increase while the player does not move is expected TH08 1.00d behavior
only when the correctly initialized human/youkai gauge is at either extreme.
It is not expected at the ordinary gauge-zero start of a team run. Target
`Player::OnUpdate @ 0x0044C390` calls
`GameManager::AddScore(100)` once per active, non-dialogue frame from both the
extremely-human branch (`+0x1CC`) and extremely-youkai branch (`+0x204`). At 60
frames per second, the visible score therefore rises by approximately 6,000
points per second without requiring movement, shooting, collection, or graze.

Internally, `GameManager::AddScore @ 0x004181F0` divides awards by ten. The GUI
prints that nine-digit internal score and then prints the continue count as the
visible final digit, so the on-screen value still reflects a 100-point award
per frame. `GameManager::OnUpdate @ 0x00439BC7` also advances `displayScore`
toward the authoritative `score` using a retained `scoreDisplayStep`; after a
large item, enemy, spell-card, or stage-clear award, the visible digits can
temporarily roll much faster even though no new award is being generated.

Target-safe disassembly found 18 direct calls to target `AddScore`; the deployed
native hash `e8b7107a...161d73d` likewise has 18 direct calls, all resolving to
its linked `GameManager::AddScore`. Both `Player::OnUpdate` and
`GameManager::OnUpdate` are independently configured as 100% exact functions,
and their final linked instructions retain the target scoring and display-
catch-up behavior. The deployed build's zero threshold fields made gauge zero
look extreme and created RT-008; that owner defect is separate from the real
extreme-gauge rule. A future scoring report should distinguish the authoritative
score from the animated display and record both gauge value and initialized
thresholds.

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
`Player::AddedCallback @ 0x0044D650` replay (**1,537 / 1,537 exact**), the
linked runtime owner/call verifier, and a single-job cold rebuild/replay of all
75 configured objects (**1,106 / 1,106 exact**). The fresh normal PE32 i386 GUI
link has SHA-256
`5e217f010c78d3fb1c1459f7127c917dfe039d24a9253f90badb00b8cf556527`.
The fresh bugfix runtime link is an 898,048-byte PE32 i386 GUI executable with
SHA-256
`cf32bd1f5202f866a749b40c2aaced94b9c7fe357df0dc5bb20867e1726c6e26`.
It passes linked runtime verification for the initialized owners, all eight
enemy-name loads, spell-background aggregate load, all 21 player gauge-bound
writes, three dialogue gates, eight additional corrected REL32 calls, and all
20 player-shot callback entries. The isolated Windows directory was then
completely cleared and recreated with exactly the new executable, the two
hash-verified retail DAT files, a freshly copied windowed retail configuration,
and the ordinary unpatched launcher. No old executable, score, replay, log,
modern-port artifact, or preserve-lives script was carried forward. The
optional preserve-lives helper remains pinned to the preceding hash and safely
refuses this image. RT-007 and RT-008 now require ordinary unpatched native
repetition.
