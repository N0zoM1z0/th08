# Current reconstruction handoff

Use this page for the current phase and next task. Detailed runtime results are
in [Replay validation results](REPLAY_RESULTS.md); dated investigation notes
are in [RE_HANDOFF_HISTORY.md](RE_HANDOFF_HISTORY.md).

## Current status

The target is the original Japanese TH08 1.00d executable, size 840,704 bytes,
SHA-256 `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`.

Authored source covers 1,107 / 1,107 functions; the accepted exact ledger covers
1,106 / 1,107. The sole authored near match remains
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`. Read live figures
from the ledgers:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
```

Whole-executable layout and target-linked library recovery remain in progress.
The native VC7 `normal` build serves exact comparisons; `bugfix` serves runtime
testing. The Windows i386 prerequisite is complete. Modern Windows packaging
and macOS runtime work have their own platform guides.

## Completed replay validation

The native VC7, Linux x86_64, and Linux i386 products each pass all 108 credited
recordings and 12,311,460 calculations. All 25 fields, recorded stage-end
scores, natural replay exits, and published fingerprints agree with retail.
The native gate used fresh cold builds and fresh paired captures. Both portable
gates launched isolated processes and reused validated retail references.
Retained-evidence verification also passes for all three products.

The native changes are merged in
[PR #26](https://github.com/N0zoM1z0/th08/pull/26). The portable branch includes
those repairs through [PR #27](https://github.com/N0zoM1z0/th08/pull/27) and the
renderer, precision, and replay-tooling changes through
[PR #28](https://github.com/N0zoM1z0/th08/pull/28). Portable gameplay source
checkpoint `af9cb1d3` retains all 1,106 accepted VC7 units after a cold build,
with fresh normal/bugfix links and owner/data/callee checks.

The native ordinary-paced, rasterized demo gate passes all three demos:
16,080 frames and all 22 fields. Portable checks include four retail frames,
four renderer contract groups, actual HOME screenshots in both display modes,
six bounded replay/render windows, focused GDB controls, and sanitizer smoke
and Stage 5 audits. The results page records each check's scope and artifacts.

Use [REPLAY_TESTING.md](REPLAY_TESTING.md) for native reproduction and the
[portable replay guide](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_REPLAY_TESTING.md)
for Linux reproduction and `--verify-only`. Generated evidence stays under
`build/`; credited recordings, provenance, and published expectations are
committed.

## Remaining runtime evidence

Small portable pixel differences and nine unconfirmed Stage 5 color suspects
remain in the
[rendering report](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_RENDERING.md).
The color observations are heuristic; same-frame review found no obvious
anomaly in the initial sample. Native
[RT-007](RUNTIME_ISSUES.md#rt-007) still needs the reported visual interaction
repeated. Gameplay trace equality alone does not close that image-evidence task.

AArch64 has cross-build and initialization evidence; hardware gameplay remains
to be exercised. Replay tests ran muted, so audio output needs its own checks.
Web work is deferred and lives in the separate `th08-web` repository.

## Session and resource rules

Start at [the documentation index](README.md#agent-session-route), check
`git status`, and run the status command above. Before reconstruction changes,
follow [AGENTS.md](../AGENTS.md), [ARCHITECTURE.md](ARCHITECTURE.md), and
[RE_WORKFLOW.md](RE_WORKFLOW.md).

Use one writable session, one bounded task, and serial captures/builds.
Preserve tested binaries and their provenance before cold builds clear link
outputs. Keep completed metadata, comparison reports, and compressed traces;
remove owned temporary runtimes and compiler trees after their checks finish.
A changed executable, observer, schema, assets, or playback profile requires a
new replay output directory.

## Next bounded work

Laser image work is active on `test/portable-laser-visual-regression`.
Eight Stage 3 and Stage 4B frames retain identical retail laser/VM packets.
The color repair passes 297 Wine D3D8 samples, 144 D3DX copy samples, and
196,608 exhaustive 16-bit copies; its complete `lunatic-0-b` recording passes
113,221 calculations on all 25 fields. Earlier 108-recording gates remain
scoped to their published binaries.

The original-game font capture now confirms Droid Sans Fallback, GDI height
28 mapped to 21 ppem, and 81 bold-text parameter records equal to portable.
The Linux height/baseline repair uses OpenType Windows metrics. Fresh Stage 3
captures pass three of four strict image checks with both the default VL Gothic
and matched Droid fonts. Stage 4B passes all four. The tested Debug recorder
is `75f785111922b27bedb71a2d8142a23b71f6c98cd0ac854a47d5b978539fc566`;
its complete `lunatic-0-b` recording passes 113,221 calculations and all
25 fields with a natural exit, under `build/laser-font-20261014/`.
Retained-evidence verification, renderer contracts, and texture/copy gates pass.
The completed compiler tree and owned Wine prefixes have been removed.

The horizontal-edge capture observes 16 region quads with all 448 vertex
words identical. The independent API probe localizes the difference to edge
coverage: the old renderer omits the top row and includes the bottom row.
The upper-left clip-origin repair passes all 34 solid cases and 53/68 total
synthetic image cases. Fresh game captures now pass all eight strict pixel
checks with unchanged raw laser/VM packets. Frame 12,900's outside-header mean
falls to 0.003603 and its 49 high-error pixels disappear. The tested recorder
is `f0138506b35e9849a2877ee6a8ebe6df66c933534b1375b40c35944af2d653d6`;
its complete `lunatic-0-b` recording and retained-evidence verification pass
113,221 calculations and all 25 fields under `build/laser-vertices-20261014/`.
CI and the named edge contract/failure control pass.

Fifteen synthetic striped-material cases retain sampling differences. Font
bold-width/grayscale differences, the legacy context path without clip control,
later-boss laser cases, and refreshed i386 gates remain to be checked.
See [the laser workflow](PORTABLE_LASER_VISUALS.md) for observed quad capture,
API replay, and the named old-source edge failure control. Issue #20 remains
open; its original scene is unspecified.

For runtime work, select a concrete remaining visual or hardware scenario and
retain the stage, route, recording, frame, and comparison evidence. For exact
reconstruction, resume the authored near match or one bounded whole-link/library
dependency. The ANM semantic queue keeps opcodes 25, 31, and 88 neutral until
complete TH08 consumer evidence supports a shared-layout rename.

<a id="documentation-batch-for-local-review"></a>

The completed human/agent documentation batch and earlier repair investigations
are retained in the [handoff archive](RE_HANDOFF_HISTORY.md#replay-investigation-handoff--2026-10-13).
