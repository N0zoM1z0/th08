# Current reconstruction handoff

This page records the current phase and next task. Earlier checkpoints are in
[RE_HANDOFF_HISTORY.md](RE_HANDOFF_HISTORY.md); the ledgers supply live counts.

## Current status

The target is the original Japanese TH08 1.00d executable, size 840,704 bytes,
with SHA-256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`.

Authored source covers **1,107 / 1,107** authored functions and the accepted
exact ledger covers **1,106 / 1,107**. The sole authored near match remains
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`. Recompute these
figures with:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
```

The native Windows i386 prerequisite is complete and merged. Its clean VC7
bugfix build was playtested through Final with normal score/graze behavior,
dialogue/background rendering, Stage 4 Reimu rendering, spell effects, and
replay save. The exact-facing native `normal` build and playable `bugfix` build
serve comparison and runtime testing respectively. See the
[runtime procedure](WINDOWS_I386_RUNTIME.md), [owner audit](OWNER_AUDIT.md),
and [issue ledger](RUNTIME_ISSUES.md) for evidence.

Native 64-bit work is on `port/portable-64bit`; Web work is in `th08-web`.
Each port has its own build and runtime validation alongside the VC7 path.

## Portable replay repair synchronization

This branch integrates [main PR #26](https://github.com/N0zoM1z0/th08/pull/26),
including the native replay repairs, comparison guards, and credited offline
corpus. Native pointer layouts, 32-bit serialized formats, backend optimization,
and modern parser/lifetime repairs remain part of the portable product.

The 2026-10-12 synchronization gate passed at source commit `2c253eb2`: a fresh
native-layout x86_64 build and ownership verifier, all 1,106 accepted VC7 units
from a cold build, and fresh normal/bugfix links with their owner/data/callee
checks. All six muted portable replay windows passed the structural render
checks, retaining 5,231 samples. Normal and AddressSanitizer builds each passed
a 60-second title/demo/Stage 5 smoke check. Repository CI passed.

Stage 5 has one `color-loss-suspect` sample at frame 6,166, enemy 14, sprite 77.
The same animation/script/sprite has 72 `ok` samples; this observation remains
unconfirmed and is retained in the report. The portable gate checks bounded
runtime/render windows; the native 108-case parity result below belongs to the
VC7 product. See [PORTABLE_64BIT.md](PORTABLE_64BIT.md) for reproduction and
coverage. Local evidence is in `build/replay-cold-20261011/portable-replays/`;
build and smoke logs are compressed alongside it. The earlier v0.2.1 checkpoint
is in [RE_HANDOFF_HISTORY.md](RE_HANDOFF_HISTORY.md). Web work remains deferred.

## Portable rendering repairs

The Linux backend now retains homogeneous clip `W` for XYZ and XYZRHW draws,
aligns Direct3D/OpenGL pixel centers, refreshes flag-0 backbuffer reads, and
commits writable unlocks immediately. Read-only unlocks preserve subsequent
rendering. Fullscreen windows start at desktop dimensions, fixing the Xvfb
size mismatch. Target-facing source, headers, and ledgers retain the native
checkpoint.

Fresh x86_64 and i386 builds passed ownership/layout verification. Independent
pixel contracts pass with the repaired backend and reproduce all four expected
failures at source `9cf5eae8`. Actual HOME screenshots equal their presented
frames in both windowed and fullscreen modes. Four retail frame comparisons
pass; the mean RGB errors are 0.005193 (Stage 1), 0.004768 (Stage 3), 0.292928
(Stage 5), and 0.323372 (Final B). Stage 5 ordinary/accelerated playfields and
camera packets agree exactly.

All six muted replay windows pass with normal life rules and the normal
boss-name copy, retaining 8,810 enemy-render samples. Nine Stage 5 color suspects
remain heuristic observations. The 60-second sanitizer title/demo/Stage 5 smoke
and 90-second direct Stage 5 sanitizer replay audit pass; the latter retains
1,891 samples. Local evidence is in `build/portable-render-fix-20261012/`.

Use [PORTABLE_RENDERING.md](PORTABLE_RENDERING.md) and
`scripts/test-portable-rendering.py` for reproduction. The earlier
[background study](PORTABLE_BACKGROUND.md) and
[runtime review](PORTABLE_RUNTIME_REVIEW.md) preserve the pre-fix observations.
The complete portable 108-recording state-parity run, remaining pixel
differences, and AArch64 hardware gameplay remain separate validation tasks.

## Portable replay state gate

The serial portable runner now compares the credited corpus against validated
retail traces, using the same 25-field schema and published fingerprints as
the native gate. Its optional compiled recorder reads typed fields and exits
through normal shutdown after natural replay deletion. A GDB backend provides
independent checks and bounded enemy/item motion diagnostics.

Trace validation and comparison now stream rows. Revalidation of all 108
retained retail/native pairs reproduces their preceding reports and published
fingerprints over 12,311,460 calculations. The `easy-1-b` comparison's peak
resident size falls from 227 MiB to 17.5 MiB; 29 comparison tests, seven
reference-identity tests, and repository CI pass. Reproduction and cleanup
commands are in [PORTABLE_REPLAY_TESTING.md](PORTABLE_REPLAY_TESTING.md).
This revalidation checks the comparator; it reuses the completed cold captures.

The first batch exposed a Magic Team Lunatic Final A score difference at
Stage 4A frame 14,564: internal score +12. Modern C++ selected the float
`atan2` overload in `VectorAngle @ 0x0040C7B0`; the portable branch now selects
the original double path. Two Float3 helpers also retain trigonometric precision
through multiplication. The native branches preserve their preceding emission.

The vector-precision checkpoint at `5b537b4c` passed six full x86_64 recordings
and 667,371 calculations,
including all recorded end scores and natural replay exits. The Stage 4A GDB
prefix passes 14,566 calculations; 14,463 enemy motion observations agree with
retail. Asset-free math contracts pass and reproduce the earlier failures.
The accepted VC7 ledger passes all 1,106 units from a cold build.

Use [PORTABLE_REPLAY_TESTING.md](PORTABLE_REPLAY_TESTING.md) for the full suite
and [PORTABLE_VECTOR_PRECISION.md](PORTABLE_VECTOR_PRECISION.md) for diagnosis
and controls. Evidence is in `build/portable-state-20261012/`. The next batch
completed 29 recordings and found three differences before it was paused.
`matrix-fixed/` retains that candidate's results. Border Team Normal Stage 1
traced to [D3DX normalization rounding](PORTABLE_NORMALIZATION.md); the repaired
recording passes all 100,979 calculations. Ordinary GDB playback passes its
2,517-calculation prefix. The two Solo Youmu failures traced to
[polar motion precision](PORTABLE_POLAR_MOTION.md). The latest x86_64 candidate
passes all three complete recordings over 358,140 calculations. Its GDB
Youmu Easy prefix passes 6,826 calculations and 6,824 shared enemy-motion
records. The final x86_64 build is running all 108 recordings in `matrix-final/`
with `final-x86_64-candidate/th08-modern`.
`build/portable-state-20261012/final-full-suite.log` records progress.
The current x86_64 candidate passes all 25 fields for 36 Easy, Normal, and
Hard Final B recordings over 4,479,937 calculations, including stage-end scores
and natural exits.
The candidate uses gameplay source `af9cb1d3`. The batch continues with
Lunatic, Extra, and Final A recordings.
The detached serial queue records its status in `serial-replay-queue.json`
under the same evidence directory. It runs the i386 corpus after a complete
x86_64 pass, reusing the completed Border Team check in `i386-matrix-fpu/`.

The i386 [device precision repair](PORTABLE_I386_FLOATING_POINT.md) resolves
the Stage 5 frame 3,789 drift. Its full Border Team recording passes 100,979
calculations in `i386-matrix-fpu/`; GDB independently passes 60,797 calculations
and records retail's `0x007F` control word. Asset-free device defaults and
FPU-preservation contracts pass and reproduce the old backend's failure.
The queued i386 corpus follows the x86_64 batch; captures remain serial.

The accepted VC7 ledger passes all 1,106 units from a new cold build after
the motion repair. Fresh normal/bugfix links and their owner/data/callee
checks pass. All four retail frame checks pass after the motion repair.
Retain tested binaries and provenance when removing owned compiler trees.

At that checkpoint the ordinary build passed the 14,566-calculation Stage 4A prefix.
All four retail frame/camera checks pass after the math repair, and Stage 5
ordinary/accelerated playfields remain identical. Fresh normal/bugfix links
and their owner/data/callee checks pass. Repository CI and the remote i386,
x86_64, and AArch64 build jobs pass at `5b537b4c`. This checkpoint is pushed;
Web work remains deferred.

## Native replay parity investigation

The 2026-10-11 cold gate is recorded under `build/replay-cold-20261011/`.
Fresh normal/bugfix links and all 1,106 accepted authored units passed.
All 108 fresh retail/native pairs agree on 25 fields over 12,311,460
calculations, including recorded stage-end scores and natural replay exits.
All three fresh ordinary-paced demos, with rasterization, agree on 22 fields
over 16,080 frames. Their published trace fingerprints remain unchanged.
Use this checkpoint when syncing `port/portable-64bit`; validate the portable
builds and runtime separately.

A completed retail capture hit an Xvfb shutdown timeout; cleanup now kills and
reaps a stalled child. The resumed pair passed all 113,660 calculations.
Process tests cover graceful, stalled, and completed children. Resuming
validates retained traces and compresses completed captures. The cold batch
retains both its original interruption log and the successful resume.

The [committed replay corpus](../tests/fixtures/replays/th08-1.00d/README.md)
preserves all 108 accepted recordings and three retail-incomplete diagnostic
recordings, with per-file author credits and provenance. The suite reads these
files offline; CI verifies their bytes, metadata, and generated credit index.

The current playable VC7 build passes all three bundled demos against a fresh
muted retail capture with ordinary pacing and rasterization: 16,080 frames,
with all 22 recorded fields equal. Its executable SHA-256 is
`19b24c3a1d06de5414114ff5f224576f5047d9a0d9c38d7bf5c4ec3612d6723e`.
Its executable/map is in `build/replay-cold-20261011/candidate/`, with demo
evidence in `build/replay-cold-20261011/demos/`. Earlier demo checkpoints are
in `build/replay-suite/current-build-demos/` and `build/native-replay-parity-final/`.
Use the [unattended replay procedure](REPLAY_TESTING.md) to reproduce the gate.
Web investigation remains deferred for this batch.

The first repair restores the sine/cosine call order in Fantasy Seal bomb and
deathbomb and corrects four relocation bindings that had concealed it.
PlayerBomb's 54 accepted units pass, the fresh playable VC7 link passes its
owner/callee check, and the diagnostic window at demo 0 frames 5200–5270
agrees for all collected pools and collision regions. See
[RT-009](RUNTIME_ISSUES.md#rt-009).

The second repair moves death mode 2 past the boss cleanup block, preserving
six spellcard orbits. Their premature release caused the extra RNG consumption
at demo 0 frame 5908. COFF local-label destinations are now checked before
relocation normalization; all 1,106 accepted authored units passed a cold
build and replay with this check. The fresh playable link also verifies the
four death-mode switch entries and Fantasy Seal math callees. See
[RT-010](RUNTIME_ISSUES.md#rt-010).

The observer records each calculation pass during dialogue fast-forward and
the terminal demo state. Schema version 2 rejects older baselines and requires
the complete per-demo frame counts. Input now records the replay-fed player
value. Temporary Wine prefixes were removed; the current and historical evidence
locations are listed above. The trace-comparison tests, repository CI,
and whitespace checks pass.

The full route suite now pins 108 public canonical-version replays: 12 shots,
four main difficulties through both Final routes, and Extra. It runs serially,
muted, with automated menu input, complete-stage guards, compressed traces,
resume support, and candidate stop at the first difference. The accelerated
profile agrees with ordinary playback for all three bundled demos.

The first complete retail fixture (`easy-0-b`, `th8_ud2cdc.rpy`) records
107,296 calculations through Stage 1/2/3/4B/5/Final B. It exposed a standalone
`g_SpellcardCalcChain` allocation that failed to remove the previous stage's
spellcard callback. `CutChain @ 0x004180F0` now reads
`g_Spellcard + 0x263C` (`lifetimeObject`); all 29 SpellCard units and the cold
replay of all 1,106 accepted authored units pass. See
[RT-011](RUNTIME_ISSUES.md#rt-011).

The next divergence, at Stage 3 frame 14,350, came from an empty `fsincos`
runtime helper. It left laser-cancel item positions uninitialized. A C++
sin/cos implementation preserves all 37 accepted BulletManager comparisons
and restores complete playback parity for `easy-0-b`: all 25 fields over
107,296 calculations, six recorded end scores, and a natural replay exit.
The helper remains an unaccepted library entry: its runtime implementation is
44 bytes, while target `0x00433880` is 33 bytes; original archive provenance
is unresolved. See [RT-012](RUNTIME_ISSUES.md#rt-012).

The complete corpus now passes on the repaired build: all 108 cases and all
25 fields over 12,311,460 calculations, including every recorded stage-end score
and natural replay exit. Source checkpoint `25c101f9` supplies the C++ repairs.
Each fixture has a published retail trace fingerprint and count; `--claims-only`
captures both images afresh and checks those expectations. The acceptance tool
revalidates complete traces and the executable, map, DAT/BGM, observer schema,
and playback settings before publishing them. The earliest four retail traces
were recaptured with BGM hashes and reproduced their existing fingerprints.
Regression tests reject mismatched or missing BGM identities, equal traces with
changed suite settings, and stale candidate maps.

The current complete batch is in `build/replay-cold-20261011/matrix/`.
Use it as the retail reference directory when testing a new build. The first
full checkpoint remains in `build/replay-suite/matrix-v2/`, with its executable/map
in `build/replay-suite/candidate-replay-input/`. Identical old and fresh trace
files share storage after byte verification; both batches revalidate. The older
`matrix-v1/` stopped at Yukari and remains historical evidence.
`scripts/analysis/report-replay-suite.py` prints a concise ledger summary.

The earlier batch stopped on Solo Yukari (`easy-5-b`): Stage 3 frame 6,751, score
minus 7 and RNG generations minus 8. Target `UpdateHomingOption @ 0x0044E3A0`
uses gameplay input at `0x0164D52C`; source read physical input at
`0x0164D528`, concealed by a relocation override. Source and the manifest
now select `g_GuiMessageInputCurrent`. The focused comparison and a cold
replay of all 1,106 accepted units pass. The old manifest and playable link
fail the new input-binding guards. Fresh normal/bugfix links and the Linux32
build/layout check pass. The complete native Yukari replay now agrees for all
25 fields over 132,629 calculations, closing
[RT-013](RUNTIME_ISSUES.md#rt-013). The new playable executable/map is archived
in `build/replay-suite/candidate-replay-input/`; executable SHA-256
`3cf82cf0345c9de8f7aed883f144526df6e170c3a08f6e41c800a1b44b04916d`.

The replay guide and `config/replay-rejected-fixtures.json` retain three
recordings that fail completeness in retail: Easy Sakuya Final B,
Normal Yuyuko Final A, and Lunatic Scarlet Team Final A. Ordinary pacing with
rasterization reproduces each failing stage's accelerated prefix. Their causes
remain unresolved in the recorded Wine environment. Replacement recordings
complete every recorded stage and score in retail and pass the native comparison.
Failure evidence is in `build/replay-suite/sakuya-*/`,
`yuyuko-normal-stage1-*/`, and `scarlet-lunatic-*/`.

Complete retail playback of the first two fixtures leaves different input
tails: 3/7 and 2/6 intermediate/final records. The trailer guard accepts those
observed variants while retaining continuous frames, recorded scores, natural
exit, and complete paired-trace comparison. Regression tests cover a missing
terminal row and two equal captures that differ from a published expectation.

Repair diagnostics include `build/replay-suite/ptrace-reference-v4/`,
`chain-fix-candidate/`, `chain-fix-exact.json`, and the score/item diagnostic
windows. Both GDB and ptrace diagnostic prefixes agree for 37,016 frames.
Keep captures and VC7 builds sequential. Preserve a tested executable/map
under the suite before cold builds, which clear link outputs. The completed
batch retains compressed traces and reports; its owned 581 MiB Wine prefix
and temporary clock libraries were removed. Leave GitHub issues without
comments. Web testing remains deferred. Rendering reports, including RT-007,
retain their separate image-evidence requirements.

<a id="documentation-batch-for-local-review"></a>

## Completed human and agent documentation batch

The documentation update for [issue #24](https://github.com/N0zoM1z0/th08/issues/24)
has passed local review. The homepage retains
its AI agent workflow and explains exact reconstruction, reimplementation,
and ports. [docs/README.md](README.md) provides human and agent
reading routes; [PROJECT_GUIDE.md](PROJECT_GUIDE.md) explains runtime concepts
and terminology. Source/semantic indexes link directly to files and evidence,
Linux instructions identify their branch/product scope, and whole-image case
studies have moved from the knowledge map to
[WHOLE_IMAGE_RECONSTRUCTION.md](WHOLE_IMAGE_RECONSTRUCTION.md). Active guides
use direct, concise wording; experimental records retain their evidence and
checkpoint scope. The homepage also invites bug reports across gameplay routes.

This is a documentation-only batch. The reconstruction results below describe
the earlier checkpoint.

## Completed maintainer-navigation batch

The 2026-09-19 batch makes repository knowledge easier to enter without
changing target behavior:

- `SOURCE_MAP.md` maps production owners, exact probes, shared includes, build
  selectors, and validation entry points;
- `SEMANTIC_INDEX.md` routes current subsystem/owner/evidence questions;
- `ANM_RESOURCE_INDEX.md` separates file-slot, script, sprite, and VM-index
  namespaces and records the remaining opcode evidence queue;
- `EFFECT_STORAGE.md` records manager ownership, pool ranges, sentinel
  behavior, and callback-local scratch-vector roles;
- source headers identify ECL/probe/Effect compile roles;
- Replay, Effect, and ScreenEffect declarations carry their ownership or
  tagged-parameter contracts;
- `EclRun` uses the named enemy-position operand selectors and remains exact;
- ANM opcode 83 and its VM field now name the player-bullet draw mode and
  `AnmManager::ExecuteScript @ 0x0045EA00` remains exact.

## Validation

The following reconstruction results were established in the 2026-09-19
maintainer-navigation batch.

Focused results from that checkpoint:

- `EclManager::RunEcl @ 0x004184B0`: exact, 26,638 authored bytes and 27,398
  compared bytes;
- `AnmManager::ExecuteScript @ 0x0045EA00`: exact, 13,933 authored bytes and
  14,349 compared bytes.

The shared-header gate was run from a cold state with one build job and passed
**1,106 / 1,106** accepted units. The complete portable Linux i386 container
target compiled and linked with one job; `verify-modern-linux.sh` confirmed
ELF32/ET_EXEC/i386 and every fixed target-owned layout symbol. The normal VC7
production image linked successfully. `python3 scripts/ci.py`, documentation
link validation, and `git diff --check` passed.

### Documentation validation

The local documentation batch passed target identity verification,
`python3 scripts/ci.py`, local file/heading link checks, and `git diff --check`.
Existing section headings remain available, and the extracted whole-image
cases retain their recorded evidence. Source, configuration, and ledgers are
unchanged.

## Next bounded work

Start future sessions at [docs/README.md](README.md) and use the appropriate
reading route. For reconstruction work, select one evidence-backed family at a time.
The ANM queue keeps opcodes 25, 31, and 88 neutral until their
complete TH08 consumer sets justify a shared-layout rename. Whole-image/library
work remains independent and should resume only for a bounded link dependency.
