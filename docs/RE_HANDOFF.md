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

The published baseline native VC7, Linux x86_64, and Linux i386 products each pass all 108 credited
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

Continue laser validation on `test/portable-laser-visual-regression`. The tested
GCC 8 x86_64 recorder is
`e0a57a43520524713b70dbaa38329155fd6ab4a09006869b4f09db8dbcc27649`, built
from source checkpoint `897decb2`. All 132 recorded build-input hashes agree
after LF normalization. Evidence is under
`build/laser-spawn-origin-20261014/production-final/`.

The ECL opcode 32/33 repair passes 390 paired spawn-input records and seven
Final B laser/VM packets. All seven images pass the original beam limit.
The complete 108-recording gate is running under
`build/laser-spawn-origin-20261014/x64-suite/`, with retained-evidence verification
queued after it. Continue the existing job and output identity. Earlier
full-corpus results apply to their published products, as recorded in
[REPLAY_RESULTS.md](REPLAY_RESULTS.md).

The native backport is on `fix/native-spell-background-owner`, checkpoint
`5dfd5197`, in `.analysis/native-main-owner-20261014/`. All three affected
units and the full 1,106-unit cold gate pass. Fresh normal and bugfix links
pass the owner/data/callee checks. The new bugfix product is
`9e699dd2c596acb090a5d44e9aa32b6c0bb19f0d5e11fe6a0babb0eb1469fd9b`.
Its five fresh Final A captures at clock rate 1, with rasterization and
`csmt=0`, have identical retail laser/VM packets and pixel-identical complete
playfields. The paired 22,590-calculation diagnostic prefix agrees on all
25 fields. Its full native replay gate remains pending before a main PR/merge.
Products and reports are under `build/laser-native-20261014/main-production/`
and `main-finala-paired/`.

The two-checkout cold gates also verify the anonymous-namespace comparison
repair. The comparator normalizes only VC7's generated discriminator and
records the actual COFF names. Ambiguity, signature, extent and relocation
checks remain strict. The controlled two-path compiler experiment and ten
contract tests are documented in [RE_WORKFLOW.md](RE_WORKFLOW.md).

The i386 refresh is prepared under `build/laser-i386-refresh-20261014/`.
It includes the script-owner, font, surface-filter and ECL trigonometry
repairs. Its borrowed GCC 12 container is limited to one CPU, 768 MiB and one
compiler job. Preserve the compiler working directory for CMake try-compiles
and Ninja output paths. The fresh build, 640 surface API cases, renderer/FPU
contracts, paired laser images and full replay gate remain separate checks.
The earlier i386 product's results remain in
[PORTABLE_LASER_VISUALS.md](PORTABLE_LASER_VISUALS.md#i386-refresh-october-14-2026).

Next finish the current x86_64 gate, exercise the new native product's complete
replays, validate the refreshed i386 product and capture Extra laser images.
Issue #20's reported scene remains unspecified. Small font/sampling residuals,
the Stage 5 color observations and native RT-007 retain their measured scope
in the laser, rendering and runtime reports. AArch64 hardware and audio are
separate checks; web work remains deferred.

Detailed prior checkpoints are preserved in the
[laser session archive](RE_HANDOFF_HISTORY.md#laser-validation-checkpoints-before-the-final-platform-refresh-october-14-2026).
For exact reconstruction, the sole authored near match and bounded whole-link
or library work remain independent of these runtime gates. The ANM semantic
queue keeps opcodes 25, 31 and 88 neutral until complete TH08 consumer evidence
supports a shared-layout rename.

<a id="documentation-batch-for-local-review"></a>

The completed human/agent documentation batch is retained in the
[handoff archive](RE_HANDOFF_HISTORY.md#replay-investigation-handoff--2026-10-13).
