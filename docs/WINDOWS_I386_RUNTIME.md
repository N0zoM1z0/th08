# Native Windows i386 reconstruction runtime

This is the prerequisite whole-program validation lane for the reconstructed
Japanese TH08 1.00d source. It builds the production translation units with the
pinned Visual C++ .NET 2002 toolchain, links a real PE32 GUI executable, and
runs that executable with the original game data on Windows.

It must precede modern Windows/Linux/macOS port work. A modern compiler can
accept different declarations, choose different owners, add compatibility
initialization, or tolerate a link graph that the VC7 production image does
not. A modern executable can therefore be useful portability evidence while
still hiding a missing VC7 data owner, translation-unit boundary, static
initialization, callback ABI, or lifetime defect.

## Completion status

The first prerequisite pass completed on 2026-09-10 on branch
`reconstruction/windows-i386-runtime`. The accepted authored ledger cold-replays
**1,106 / 1,106 exact** units, both production modes link as PE32 i386, the
final-link owner/callee verifier passes, and the phase exercised title,
selection, story/practice, dialogue, stage, X-bomb, normal-death, score, and
graze paths on attributable native artifacts. The final clean, unpatched image
also passed the reported score/graze and all three team X-bomb checks. The phase
found RT-001 through RT-008; the issue ledger keeps each causal repair and its
runtime disposition. RT-007's old Final/Last Spell screenshot path retains a
non-blocking confirmation follow-up, but there is no known unfixed native defect
at this checkpoint.

Completion means later port work may proceed. It does not make the VC7 image a
supported download, and it does not permanently waive this gate. Repeat the
serial procedure below after any shared owner/layout/TU/PCH/compiler-profile or
production link-graph change, or when a new native failure is reported.

## Environment bootstrap

The exact target and retail data are private inputs and must never be committed.
Place the original Japanese 1.00d executable at `resources/th08.exe`; its
required size and SHA-256 are listed in `docs/ARCHITECTURE.md`.

On Linux or macOS, install Python 3.11 or newer, Wine, and `msiextract`, then
create the pinned Visual Studio .NET 2002/DirectX 8 environment:

```bash
git submodule update --init --recursive
./scripts/create_th08_prefix
```

The helper downloads hash-pinned historical inputs into ignored paths, prepares
`scripts/prefix`, and uses `~/.wineth08` as its default Wine prefix. Set `WINE`
before running it only when a different compatible runner is required. On a
native Windows development host, prepare the same ignored environment with:

```text
git submodule update --init --recursive
python scripts/create_devenv.py scripts/dls scripts/prefix
```

See `docs/BUILD_MATCHING.md` for dependency details and focused comparison
workflows. Use one writable session and one Wine/VC7 job throughout this gate.

## Artifact boundary

| Artifact | Purpose | What it can prove |
| --- | --- | --- |
| `resources/th08.exe` | Canonical Japanese 1.00d target | Target bytes and behavior; never modified or distributed |
| `build/th08.exe` from `normal` | Exact-facing native VC7 reconstruction | Native i386 compile/link and comparator input; not a playable artifact when its non-retail file identity reaches the original version whitelist |
| `build/th08.exe` from `bugfix` | Native VC7 runtime reconstruction | The same native production graph with the repository's narrow `FIX_REALLY_BAD_BUGS` branches enabled; use this artifact for Windows playtesting |
| `th08-modern.exe` or a Linux/macOS binary | Portability product | Behavior under its own compiler/backend; cannot substitute for `build/th08.exe` |
| preserve-lives launcher | Process-only endurance aid | Wider runtime coverage for one hash-pinned reconstruction; no correctness or exact-match credit |

Do not make the native image runnable with unresolved-symbol forcing, blanket
aliases, duplicate storage, fixed preferred-base function addresses, fake
returns, or modern startup `memcpy` initialization. Repair the real semantic
owner, ABI, translation unit, link input, or lifetime instead.

## Reproducible build and static gates

Run this sequence from the repository root. Start by proving the input and
ledger state; `config/claims.csv` must contain only its header:

```bash
git status --short
python3 scripts/verify-target.py resources/th08.exe
python3 scripts/validate-tracking.py --require-target
python3 scripts/analysis/report-reconstruction-status.py --summary
test "$(wc -l < config/claims.csv)" -eq 1
```

First cold-build and replay every accepted normal comparison object. This step
cleans generated Ninja and known VC7/linker side outputs, so it intentionally
precedes both complete executable links:

```bash
python3 scripts/analysis/verify-exact-units.py --all --json \
  > build/accepted-unit-replay.json
```

Require `result: exact`, `checked: 1106`, `exact: 1106`, and no failures for
the recorded checkpoint. Those counts are a snapshot; later contributors must
use the live ledgers rather than hard-code them into tools.

Next cold-build the exact-facing production image with one VC7 job, identify the
artifact, and run the final-link owner/callee verifier against its map:

```bash
python3 scripts/build.py --build-type normal --fresh -j 1
file build/th08.exe
sha256sum build/th08.exe
python3 scripts/analysis/verify-windows-i386-runtime-data.py
```

The link must produce `build/th08.exe` without unresolved-symbol forcing. The
output is expected to be a PE32 i386 GUI executable. This is the artifact used
for normal-object comparison and link evidence; merely producing it is not a
runtime pass. On native Windows, use `Get-Item` and `Get-FileHash -Algorithm
SHA256` instead of `file` and `sha256sum`.

Run the repository gates while the normal build evidence is current:

```bash
python3 scripts/ci.py
python3 scripts/progress.py --check
git diff --check
```

Finally, cold-build the native runtime artifact and verify the actual file that
will be copied to Windows:

```bash
python3 scripts/build.py --build-type bugfix --fresh -j 1
file build/th08.exe
sha256sum build/th08.exe
python3 scripts/analysis/verify-windows-i386-runtime-data.py
```

The bugfix build is still a Microsoft VC7 PE32/i386 compile and link of the
production translation units. It is not a modern port, binary patch, forced
link, or compatibility startup shim. `FIX_REALLY_BAD_BUGS` is required here
because the original `Supervisor::CheckVersion` accepts serialized score and
replay headers only when their version, executable size, and checksum occur in
the retail `th08_0100d.ver` table. A reconstructed executable cannot possess a
retail executable identity. In a normal reconstruction the final `0100d`
record therefore fails its size/checksum comparison; the original loop then
walks beyond the table and dereferences `0x1`. The bugfix branch accepts the
matching version string before that impossible file-identity comparison.

Exact status continues to come only from the normal comparison objects. Never
claim target exactness from a bugfix object or from the playable executable.
Because the cold aggregate replay and both production modes reuse and clean the
same `build/` graph, do not reorder or parallelize them. Build `bugfix` last and
copy only that final `build/th08.exe` to the isolated Windows directory.

The linked verifier checks the target hash, rejects mapped uninitialized
production owners in target raw-backed sections, compares the Last Spell count,
stage bonuses, and dialogue palettes byte-for-byte, and resolves all 66 Effect-table
callback pointers through the current linker map. It also decodes the linked
`Gui::CopyEnemyNameTexture` body and requires all eight GUI pointer loads to
select `frontAnm @ Gui + 0x0C`, never `stageTextAnm @ Gui + 0x10`. It also
rejects standalone `g_SpellcardBackgroundAnm` storage and requires
`Spellcard::StartSpell` to select
`stageEffectAnm @ g_EffectManager + 0x8B058`. Finally, it decodes all three
stage-layer gates in the linked Background draw callbacks and requires them to
call `Gui::IsStageFinished`, never `Gui::IsDialoguePresent`. The same verifier
checks the eight other REL32 callees uncovered by that audit and all 20 entries
of the target-owned player-shot spawn/update/draw/collision callback tables. It
also rejects standalone `g_PlayerGaugeBounds` storage and checks all 21 setup
writes against the six gauge fields at `g_GameManager + 0x3DDF8..0x3DE02`.
It must pass for the final bugfix artifact as well as the normal link. A reused
object tree or `--reuse-build` result cannot support aggregate exact status
after an owner, header, compiler-profile, or link-graph change.

### Completed checkpoint identities

The completed pass used the source checkpoint at commit `318fec8b` plus the
documentation-only closure that follows it. These hashes make later runtime
reports attributable; they are not release downloads:

| File | Size | SHA-256 |
| --- | ---: | --- |
| canonical `resources/th08.exe` | 840,704 | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| fresh `normal` reconstruction | 902,144 | `5e217f010c78d3fb1c1459f7127c917dfe039d24a9253f90badb00b8cf556527` |
| fresh `bugfix` reconstruction | 898,048 | `cf32bd1f5202f866a749b40c2aaced94b9c7fe357df0dc5bb20867e1726c6e26` |
| tested retail `th08.dat` | 46,838,025 | `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb` |
| tested retail `thbgm.dat` | 449,961,024 | `2c2ef05f9ff6f43f752dae5da519a2477372c3f8875b7b44996a8f69fdad4d89` |

Only the executable hash/size in `docs/ARCHITECTURE.md` defines the accepted
reverse-engineering target. The two DAT hashes identify the legally owned data
used for this runtime pass; they are recorded for reproduction, not promoted to
the target ledger.

## Isolated Windows deployment

Never test by overwriting the canonical installation. Create a separate
directory, for example:

```text
D:\Entertainment\Game\Touhou\th08-reconstruct
```

Copy the freshly built **bugfix** `build/th08.exe` there as
`th08-reconstructed.exe`, then copy the legally owned `th08.dat` and
`thbgm.dat`. Do not commit any of those files. Do not deploy the normal
exact-facing artifact as the playtest executable: its retail-identity check is
expected to reject the reconstruction and can run off the end of the version
table.

For a new empty directory, the following PowerShell example is deliberately
fail-closed: it refuses an existing destination instead of deleting or mixing
old artifacts. Run it from a Windows-visible repository root and replace the
two example paths:

```powershell
$Repo = (Resolve-Path '.').Path
$Retail = 'D:\path\to\the\original Japanese TH08 1.00d'
$Deploy = 'D:\path\to\a\new\th08-reconstruct'

if (Get-Process -Name 'th08*' -ErrorAction SilentlyContinue) {
    throw 'Close every TH08 process before deployment.'
}
if (Test-Path -LiteralPath $Deploy) {
    throw "Choose a new or separately emptied deployment directory: $Deploy"
}

New-Item -ItemType Directory -Path $Deploy | Out-Null
Copy-Item -LiteralPath (Join-Path $Repo 'build\th08.exe') `
    -Destination (Join-Path $Deploy 'th08-reconstructed.exe')
Copy-Item -LiteralPath (Join-Path $Repo 'scripts\run-windows-i386-reconstruction.bat') `
    -Destination $Deploy
Copy-Item -LiteralPath (Join-Path $Retail 'th08.dat') -Destination $Deploy
Copy-Item -LiteralPath (Join-Path $Retail 'thbgm.dat') -Destination $Deploy
Copy-Item -LiteralPath (Join-Path $Retail 'th08.cfg') -Destination $Deploy

Get-ChildItem -LiteralPath $Deploy
Get-FileHash -Algorithm SHA256 -LiteralPath `
    (Join-Path $Deploy 'th08-reconstructed.exe'), `
    (Join-Path $Deploy 'th08.dat'), `
    (Join-Path $Deploy 'thbgm.dat')
```

The expected clean baseline contains exactly five files: the renamed bugfix
EXE, two DATs, one configuration, and the ordinary launcher. It contains no
previous score, replay, backup, log, crash record, modern-port executable, SDK
debug DLL, or runtime-patch helper. When investigating a failure instead of
establishing a clean baseline, move the old directory aside first so its exact
EXE hash and artifacts remain attributable.

TH08 stores fullscreen/windowed selection in `th08.cfg`; the reconstructed VC7
image does not need a modern command-line override. Use a known windowed
configuration in the isolated directory for compatibility testing. In the
60-byte retail configuration used by this pass, byte `0x22` was `1` for windowed
mode. Copy an already configured file; do not modify the original installation
as part of deployment.

Launch only `run-windows-i386-reconstruction.bat` for the ordinary acceptance
run. It sets the deployment directory as the working directory, checks the EXE
and both DATs, waits for the process, and reports a nonzero exit. It passes no
modern-port arguments and installs no runtime patch.

Before replacing or launching the executable, make sure no prior TH08 process
is running. Do not rebuild with Wine/VC7 while a Windows-host runtime test is
active; this repository uses one writable build/runtime session at a time.

## Why function exactness does not replace this pass

RT-002 demonstrated a relocation-specific false sense of safety. The stale
source selected `g_Gui.stageTextAnm @ +0x10`, while the match unit declared a
`g_Gui` target base four bytes before the independently mapped object. The two
errors canceled to the target absolute address, so relocation replay could
accept every instruction byte even though the rebuilt linker resolved the
reference to the wrong field. On Windows, Stage 1 supplied only six stage-text
sprites and the first selection requested sprite `0x10`, producing an access
violation in reserved memory.

The repair names `frontAnm @ +0x0C`, restores the match relocation base to
canonical `g_Gui @ 0x0160F428`, and checks the linked loads separately. Treat
every relocation as a three-part claim: semantic owner, field addend, and
target base. A byte-exact normalized function proves none of those parts in
isolation when compensating errors are possible.

RT-005 showed a second failure mode: a target address already inside a mapped
aggregate had been modeled as standalone storage. The old
`g_SpellcardBackgroundAnm @ 0x00577EB8` symbol could normalize an exact
`Spellcard::StartSpell` relocation, but the production linker allocated a new
zero-filled pointer because no target-address layout exists in an ordinary PE
link. The target address is actually `g_EffectManager @ 0x004ECE60 +
stageEffectAnm @ 0x8B058`; the manager's resource loader initializes that
field. Before accepting any apparent global, test its address against the
known extents and member offsets of adjacent aggregate owners.

RT-006 exposed the REL32 form of the same false assurance. Source
`Background::OnDrawHighPrio` and `Background::OnDrawLowPrio` called
`Gui::IsDialoguePresent @ 0x004358BB`, but their three relocation entries
declared target `Gui::IsStageFinished @ 0x00437D87`. The object comparator
therefore replayed the target displacement and accepted the callbacks, while
the production linker followed the actual source symbol. The rebuilt game
stopped drawing all stage layers during dialogue, leaving solid clear colors
or black and accumulating moving player/portrait images. Native verification
now checks the three final linked call instructions as well as data owners.

The follow-up REL32 audit found eight more call sites with the same
source-symbol/target-address contradiction. Target item collection sends the
two full-power notices to `CreateScorePopup` but point-star and time-orb values
to `CreatePlayerPointPopup`; the stale source selected the opposite popup pool
at all four sites. Target `AnmVm::UpdatePulsingRadialTrail` reads the current
timer value for its parity pulse, not `HasTicked`; `UpdateFantasyOrbBomb` uses
plain timer equality at frame 40, not the edge-detecting `JustReached` helper;
`SpawnRandomizedShot` uses `GetRandomF32Signed`; and `RetryMenu::OnDraw` gates
the fourth menu VM on Spell Practice rather than the broader practice-mode
flag. Their signatures let every wrong call preserve surrounding instruction
shape while relocation replay substituted the declared target. The tracking
validator now rejects any decorated REL32 symbol assigned to multiple target
addresses, and the linked verifier checks all eight repaired instructions.

The player-shot callback global ledger previously shifted three table names:
target `0x004C7EE0`, `0x004C7F04`, and `0x004C7F1C` are respectively the
9-entry spawn, 6-entry update, and 2-entry draw tables, followed by the 3-entry
collision table at `0x004C7F24`. The final-link verifier resolves all 20
entries, including the ABI adapters used where a serialized SHT callback index
selects a Player member implementation.

RT-008 exposed the same aggregate-owner failure in mutable setup data. Target
`g_PlayerGaugeBounds @ 0x0164D300` is exactly the first of six contiguous
signed 16-bit fields at `g_GameManager @ 0x0160F508 + 0x3DDF8`; it is not an
independent array. `Player::AddedCallback` writes 21 default and shot-specific
values across those six fields, while the gauge predicates read the same
manager members. The old native link allocated the apparent array separately,
leaving the real manager thresholds zero. Gauge zero then qualified as extreme
human, awarding 100 visible score points every active frame and tripling the
ordinary graze count. The production fix names all six manager members,
removes the false global and fixed-layout port alias, and requires the final
linked writes to resolve through `g_GameManager`.

## Runtime matrix

A minimum manual pass should exercise ownership and lifetime boundaries, not
only reach the title screen:

1. start in windowed mode and enter gameplay with the copied retail data;
2. exercise the Reimu/Yukari, Marisa/Alice, and Sakuya/Remilia X-bomb paths;
3. die normally, observe death effects, item/power changes, respawn, and the
   life display;
4. return from gameplay to the title, start another run, and repeat;
5. visit Music Room, Options, practice/story menus, replay selection, and
   return paths;
6. cross stage and dialogue transitions, including enemy-name and portrait
   texture replacement;
7. save and replay a run when the preceding paths are stable; and
8. close the game normally and record whether the process exits unexpectedly.

The earlier long-run regression replay saved during RT-007 was slot 3
(`th8_03.rpy`, SHA-256 `1ec94058...4fbc7`). The later explicitly requested
clean deployment did not carry any prior replay or score state forward; repeat
the Final/Last Spell self-shot and dense popup paths in a fresh run.

At either real extreme of the correctly initialized human/youkai gauge, the
original game awards 100 visible score points per active frame, or approximately
6,000 points per second at 60 FPS, even while the player is stationary. Gauge
zero at the start of an ordinary team run is not extreme and must not receive
that award. Large awards are also presented through a short display-score
catch-up animation. Record the authoritative score, display score, gauge, and
six initialized thresholds when distinguishing these paths.

Track every observation in [the runtime issue ledger](RUNTIME_ISSUES.md).
Record the executable SHA-256, exact interaction, last visible frame, whether
Windows still reports the process alive, and any crash/event-log evidence.

## Optional historical preserve-lives diagnostic

`scripts/run-preserve-lives-test.bat` and its PowerShell helper are
intentionally narrower than a "no-death" patch. The player must still enter
the normal death, effect, power drop, respawn, invulnerability, replay-event,
score, and UI paths. The launcher changes only the immediate argument of the reconstructed
`GameManager::AddLives(-1)` call to zero in the live process, so the call and
all surrounding bookkeeping still execute without consuming a life.

The helper must be regenerated or reviewed after every executable change. It
is pinned to a SHA-256, a map-derived RVA, and expected instruction bytes; it
must refuse the canonical target and every unknown reconstruction. It never
modifies the executable on disk and is never a production or release mode.
The completed final deployment intentionally omitted both helper files and used
ordinary life decrement. The checked-in helper remains pinned to an earlier
artifact and must refuse the completed `cf32bd1f...6c6e26` image; do not copy it
for reproduction of the final acceptance run.

## Acceptance language

- A successful VC7 compile/link is a **native production build**, not a runtime
  pass or a whole-image exact claim.
- `normal` is the exact-facing build mode; `bugfix` is the native playable mode.
  Both use VC7 and the production link graph, but only normal objects are exact
  evidence.
- A manually exercised interaction is a **runtime observation** tied to one
  artifact hash.
- A repaired issue is **fixed / confirmation pending** until the originally
  reported path has been repeated on the repaired native image.
- Function exactness remains governed only by the configured target comparator.
- A runtime repair does not add exact credit, and a function-level exact result
  does not prove correct whole-program ownership or lifetime.
- The 2026-09-10 prerequisite checkpoint is complete; later work may call it
  complete only while the source/link assumptions listed above remain unchanged
  or after this serial gate is repeated.
