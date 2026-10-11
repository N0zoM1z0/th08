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

## Native replay parity investigation

A fresh cold gate is in progress under `build/replay-cold-20261011/`.
Cold normal/bugfix links and all 1,106 accepted authored units passed. The
new 108-case suite is continuing with fresh retail/native captures. A completed
retail capture hit an Xvfb shutdown timeout; cleanup now kills and reaps a
stalled child. The resumed pair passed all 113,660 calculations. Process tests
cover graceful, stalled, and completed children, and resumed captures are
validated and compressed. Resume with the archived candidate in this batch.
Merge into main and sync
`port/portable-64bit` after the full replay and ordinary-demo gates pass.

The [committed replay corpus](../tests/fixtures/replays/th08-1.00d/README.md)
preserves all 108 accepted recordings and three retail-incomplete diagnostic
recordings, with per-file author credits and provenance. The suite reads these
files offline; CI verifies their bytes, metadata, and generated credit index.

The current playable VC7 build passes all three bundled demos against a fresh
muted retail capture with ordinary pacing and rasterization: 16,080 frames,
with all 22 recorded fields equal. Its executable SHA-256 is
`3cf82cf0345c9de8f7aed883f144526df6e170c3a08f6e41c800a1b44b04916d`.
Evidence is in `build/replay-suite/current-build-demos/`; the earlier
`94b35a71...bcf58f` checkpoint is in `build/native-replay-parity-final/`.
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

The completed batch is in `build/replay-suite/matrix-v2/`. Its archived playable
executable/map is in `build/replay-suite/candidate-replay-input/`.
Use `matrix-v2/` as the retail reference directory for a fresh build. The older
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
