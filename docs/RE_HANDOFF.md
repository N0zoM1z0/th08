# Current reconstruction handoff

Follow the lightweight workflow in [AGENTS.md](../AGENTS.md). README and the
[documentation index](README.md) remain the navigation entry points.

## Branches and current work

`main` carries the VC7 reconstruction and i386 port. `port/portable-64bit`
adds native layouts and the Linux x86_64/AArch64 paths. The completed
`semantic/readability-protocols` branch is already an ancestor of both;
its source and useful findings are incorporated. Keep these branches and
replay each branch's affected comparisons when moving source changes.

This checkout follows `main`, with the exact reconstruction and i386 builds.
The `.analysis/portable-perspective-rounding` worktree contains uncommitted
billboard/transparent-quad tools and notes. Preserve that entire worktree and
its inputs. Other experiment branches remain available through Git.

Read live reconstruction state from the ledgers:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
```

The only authored near match is
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`.
The [reference investigation](CIRNO_REFERENCE_AUDIT.md#remaining-callback-register-phase-is-explained-source-is-not)
explains its register phase, but the missing original source construct is
unknown. Empty branches and placeholder assertions are rejected approaches.
The diagnostic unit remains unaccepted.

Whole-image layout and target-linked library work should start from a concrete
link difference; see [WHOLE_IMAGE_RECONSTRUCTION.md](WHOLE_IMAGE_RECONSTRUCTION.md).
Do not resume broad library scanning merely to increase coverage.

## Verified conclusions and open runtime work

The [exact-evidence workflow](EXACT_EVIDENCE.md) checks complete owned code,
relocations, compiler literals and declared data contracts. Read code, data
and linkage scopes separately. The latest integration is main PR #39 and
portable PR #40; it repaired target strings and added initialized-data checks.
A COFF match with partial data or unchecked linkage keeps those limitations.

[Replay results](REPLAY_RESULTS.md) record the completed native, x86_64 and
i386 corpus gates and their tested revisions. Later
[native-layout repairs](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_LAYOUT_AUDIT.md) passed their narrower
state-transition and replay checks. Those historical results do not establish
that an arbitrary newly built product passed the full corpus.

Remaining work is the measured title/boundary sampling residual in
[PORTABLE_LASER_VISUALS.md](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_LASER_VISUALS.md#remaining-sampling-and-title-error),
the unconfirmed Stage 5 color observations, native
[RT-007](RUNTIME_ISSUES.md#rt-007), AArch64 hardware gameplay and audio.
ANM opcodes 25, 31 and 88 keep neutral names until all consumers support a
shared-layout interpretation. Web work lives in the separate `th08-web` repo.

## Build, checks and local state

Use [Factory tools](SHARED_TOOLS.md), the pinned VC7 compiler/SDK libraries
under `scripts/prefix`, and this game's independent Wine prefix. The target
identity is in `config/target.toml`; verify it before target-dependent work.
`normal` is the exact-comparison build; `bugfix` is the native runtime build.

`python3 scripts/ci.py` checks the whole checkout's syntax, public ledgers,
Ninja configurations and accidental private artifacts. It needs Python 3.11+
and PyYAML, without the target or private compiler. Comparator and runtime tests
are selected by the change; `--full` opts into the broad public suite.
The Linux packaging workflow is manually dispatched when those products need
validation. Documentation edits need diff review, not a compiler or replay.

Generated comparisons, traces, archives and trial builds are disposable after
useful conclusions are recorded. Retain only a current input or an unresolved
experiment whose compiler/runtime context is expensive to reproduce. Historical
paths in older notes are not a reason to retain generated files. New probes
use temporary outputs and must not replace canonical objects or receipts.
