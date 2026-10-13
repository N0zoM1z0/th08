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
The current x86_64 Debug recorder has SHA-256
`62386854ab8e486a8ad742c97f27b7f2063d39e008aefc4af8600dd083b9aecc`,
with build identities under `build/laser-legacy-20261014/production/`.
Earlier 108-recording gates remain scoped to their published binaries.
The current binary's complete `lunatic-0-b` recording passes 113,221
calculations on all 25 fields with a natural replay exit; retained-evidence
verification passes.

Both default and forced OpenGL 2.1 contexts pass six asset-free renderer
groups. The legacy path now inverts its FBO and maps CPU/GPU reads, writes,
clears, audit regions, and presentation consistently. Its eight Stage 3 and
Stage 4B frames pass the unchanged 1.0 image limit, retaining exact retail
laser/VM packets. GDB observes OpenGL 2.1 and a null clip-control pointer.
Frame 12,900's 49 high-error edge pixels disappear. The legacy API probe
matches all 34 solid images and 52/68 total images; the old-source control
differs in all 68. All 297 color samples, 144 D3DX copies, and 196,608 exhaustive
copies pass in the forced legacy profile. See
[the laser workflow](PORTABLE_LASER_VISUALS.md) for captures, source controls,
and the `--legacy-gl` reproduction commands.

The original-game font evidence confirms Droid Sans Fallback, GDI height
28 mapped to 21 ppem, and 81 bold-text parameter records equal to portable.
The Linux height/baseline repair uses OpenType Windows metrics. Remaining
bold-width/grayscale differences need separate font rasterization measurements.

The earlier clip-control API probe retains 15 striped-material sampling
differences; the legacy probe retains 16. An XY grid control passes 59/68
images, with nine first-row differences coinciding with Wine's offset-viewport
sensitivity. Private viewport mapping does not resolve them. Its evidence
is under `build/laser-sampling-20261014/`; `--position-grid 256` reproduces
the control. Later-boss laser cases and the full corpus
for the revised products remain to be checked. Issue #20 remains open; its
original scene is unspecified.

The refreshed i386 recorder is
`a82e7155eb7c71e5db645df9294cd5316e895df5f35dcbce696946ba4ba9de27`.
Both context profiles pass seven renderer/FPU groups and the same eight laser
frames each. Its complete `lunatic-0-b` recording and retained-evidence gate
pass 113,221 calculations on all 25 fields. Evidence is under
`build/laser-i386-20261014/`. The actual i386 compiler is Debian GCC 12.2.0;
the older provenance's GCC 8.4.0 label has a preserved correction addendum.
Seven playfields per profile equal the retained x86_64 pixels. Stage 4B frame
15,360 retains a small background-shading difference, with means 0.073440 and
0.074393. The [draw-input investigation](PORTABLE_DRAW_INPUTS.md) confirms that
195 zero-alpha RGB words in the background call differ while all other inputs
agree. Borrowing those words yields pixel-identical playfields. The canonical
target also writes alpha only in its far-fog branches; reconstruction behavior
is preserved. Experimental controls remain separate from ordinary image gates.

For runtime work, select a concrete remaining visual or hardware scenario and
retain the stage, route, recording, frame, and comparison evidence. For exact
reconstruction, resume the authored near match or one bounded whole-link/library
dependency. The ANM semantic queue keeps opcodes 25, 31, and 88 neutral until
complete TH08 consumer evidence supports a shared-layout rename.

<a id="documentation-batch-for-local-review"></a>

The completed human/agent documentation batch and earlier repair investigations
are retained in the [handoff archive](RE_HANDOFF_HISTORY.md#replay-investigation-handoff--2026-10-13).
