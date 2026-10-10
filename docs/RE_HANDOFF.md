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

## Documentation batch for local review

The `docs/human-agent-reading-paths` branch implements the documentation scope
of [issue #24](https://github.com/N0zoM1z0/th08/issues/24). The homepage retains
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

Review the documentation branch locally before publication. Start at
[docs/README.md](README.md), follow a human route and the agent session route,
and inspect the homepage's retained workflow section.

For later reconstruction work, select one evidence-backed family at a time.
The ANM queue keeps opcodes 25, 31, and 88 neutral until their
complete TH08 consumer sets justify a shared-layout rename. Whole-image/library
work remains independent and should resume only for a bounded link dependency.
