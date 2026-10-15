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

## Latest validation checkpoint

The replay and laser repair batch is merged into `port/portable-64bit` through
[PR #32](https://github.com/N0zoM1z0/th08/pull/32), checkpoint `c7d27932`.
It includes main PR #31. All 440 existing non-document inputs remain unchanged
after that history integration; the final 1,106-unit single-job cold comparison
and repository CI pass. The tested
GCC 8 x86_64 recorder is
`e0a57a43520524713b70dbaa38329155fd6ab4a09006869b4f09db8dbcc27649`, built
from source checkpoint `897decb2`. All 132 recorded build-input hashes agree
after LF normalization. Evidence is under
`build/laser-spawn-origin-20261014/production-final/`.

The ECL opcode 32/33 repair passes 390 paired spawn-input records and seven
Final B laser/VM packets. All seven images pass the original beam limit.
The complete 108-recording gate and retained-evidence verification pass under
`build/laser-spawn-origin-20261014/x64-suite/`: 12,311,460 calculations, all
25 fields, scores, fingerprints and natural exits agree. All temporary
runtimes are removed, and equal traces share reference storage. Earlier
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
25 fields. Its full native replay gate and separate retained-evidence pass
also pass all 108 recordings and 12,311,460 calculations, with matching fields,
scores, fingerprints and completion boundaries. The native backport is merged
in [PR #31](https://github.com/N0zoM1z0/th08/pull/31), main checkpoint `08c7f00f`.
Products and reports are under `build/laser-native-20261014/main-production/`
and `main-finala-paired/`.

The two-checkout cold gates also verify the anonymous-namespace comparison
repair. The comparator normalizes only VC7's generated discriminator and
records the actual COFF names. Ambiguity, signature, extent and relocation
checks remain strict. The controlled two-path compiler experiment and ten
contract tests are documented in [RE_WORKFLOW.md](RE_WORKFLOW.md).

The i386 refresh is prepared under `build/laser-i386-refresh-20261014/`.
Its fresh GCC 12 ELF32 recorder is
`bd8b148c0ac6a478391ee4c1fea7334ac1f473466418131715cda4e1a900ce40`, built
from `ed8ff8df`. All 132 recorded build-input hashes agree. It includes the
script-owner, font, surface-filter and ECL trigonometry repairs. ELF32/fixed
layout verification and all 640 default-context surface API cases pass. Seven
renderer/FPU contract groups pass in each of the default and forced OpenGL 2.1
contexts. All seven Final B
image/raw-state checks and 390 paired spawn inputs pass in each context. The
`finalb-kaguya` manifest pins the original state file and passes retained checks
for x86_64 and both i386 contexts. The complete 108-recording corpus and
separate retained-evidence pass also succeed: 12,311,460 calculations agree
on all 25 fields, scores, fingerprints and natural exits. All temporary
runtimes are removed; equal traces share reference storage. Evidence is under
`full-suite/`, `full-suite-revalidation.log` and `full-suite-audit.json` in that
phase directory. CMake, package discovery and
Ninja run together inside the borrowed GCC 12 container, limited to one CPU,
768 MiB and one job. The compiler tree has been removed. Use the container
commands in [PORTABLE_REPLAY_TESTING.md](PORTABLE_REPLAY_TESTING.md) to reproduce
that environment.
The earlier i386 product's results remain in
[PORTABLE_LASER_VISUALS.md](PORTABLE_LASER_VISUALS.md#i386-refresh-october-14-2026).

A serial worker completed the x86_64, new native and refreshed i386
108-recording gates and all three retained-evidence passes. Product hashes are
pinned. The owned native Wine prefix was removed after native verification,
before i386 captures began. The completed worker's state is under
`build/laser-i386-refresh-20261014/queued-platform-gates.json`.
Four Extra startup/active/despawn frames now pass against
retail, with exact laser/VM packets and beam errors 0.001472–0.004902. The
`extra-mokou` case pins their retail state hash; evidence is under
`build/laser-extra-20261014/paired/`. The second serial worker also completed
the refreshed i386 Stage 3, Stage 4B, Final A and Extra cases in default and
OpenGL 2.1 contexts. All eight groups and 34 new image/raw-state checks pass,
including separate retained comparisons. With Final B, each context covers
24 selected frames. Candidate, observer, comparator, case-manifest and
reference hashes remain pinned. Its completed state is in
`build/laser-i386-refresh-20261014/queued-scene-gates.json`; evidence is under
`scene-default-*/` and `scene-legacy-*/` in the same directory. All temporary
game runtimes are removed. Tested binaries, source/build provenance and
compressed comparison evidence remain under their recorded phase directories.

## Next bounded work

Continue the measured title and boundary-sampling residuals in
[the laser report](PORTABLE_LASER_VISUALS.md#remaining-sampling-and-title-error).
Start with the retained, unmodified quad inputs and their D3D8/OpenGL API
control; the selected gameplay images already pass their recorded thresholds.
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
