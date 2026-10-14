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

Laser image work is active on `test/portable-laser-visual-regression`.
The preceding x86_64 Debug recorder has SHA-256
`62386854ab8e486a8ad742c97f27b7f2063d39e008aefc4af8600dd083b9aecc`,
with build identities under `build/laser-legacy-20261014/production/`.
Earlier 108-recording gates remain scoped to their published binaries.
That binary's complete `lunatic-0-b` recording passes 113,221
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
bold-width/grayscale differences now have an offline rasterization control:
all 27 production font images reproduce exactly before changing settings.
Normal hinting reduces average error by 2.7%; disabling kerning changes none
of the control pixels, and disabling bold worsens 20 of 27 images. SDL_ttf's
two-pixel synthetic-bold advance at 21 ppem differs from Wine's one-pixel
advance. A direct FreeType control of glyph advance, outline emboldening and
grayscale transfer now reproduces 26 of 27 Wine
images exactly, with all 27 visible-pixel masks equal. The remaining pixel is
`(208,22)` in height-28 probe text 1: gray 217 versus Wine's 208. The system
i386/x86_64 API control now localizes it to FreeType rasterization: pre/post-bold
outlines agree, four raw coverage values differ by one, and only coverage
165/166 crosses a gray-level boundary. The production scalable-font adapter now
uses those GDI rules. Fresh API checks give 26/27 exact text images, 96/96 exact
regular blend cases, and 96 semibold blend cases with differences confined to
measured coverage locations and bounds. RGB555 pen/destination quantization
also repairs 2,712 pixels found by the first blend repetition. The recorder is
`4cce0fe8ee633fa2b37713214793fef078395edbb303f52c0cdd98e4bc914cc6`, with
build identity under `build/font-production-20261014/production/`. All eight
default-font Stage 3/4B laser images and four matched-Droid Stage 3 images pass
the original limits with exact raw laser/VM packets. Matched-font game-image
residuals now have a scaling candidate: the text helper requests filter 4 for
half-size copies, while portable discards that argument and uses nearest-neighbor
coordinates. Canonical D3DX8 forwards the filter through its fallback and
validates types 1–5. A fresh VC7 probe linked to the pinned D3DX8 archive now
confirms the API defect: 72/160 surfaces match, all eight half-size filter-4
cases differ, and portable output ignores all five filters. Equal-size and
scaled point-filter copies match. An independent repetition retains identical
pixel records. The production filter repair now matches all 640 extended API
cases under both the system compiler and GCC 8, including defaults, edge flags,
cross-format conversions, offset rectangles and rounding-state preservation.
The previous backend still reproduces the 72/160 failure control. Fresh color,
exhaustive-copy and six-group renderer checks pass. The new GCC 8 recorder is
`7eb846a665867ed9e8a1c19484d2c0fe1ee586954f0ecca3b1b756223e0e0853` under
`build/surface-filter-repair-20261014/production/`. All four matched-Droid Stage 3
images pass with exact laser/VM packets; frame 12,900's complete header changes
from 1,081 differing pixels to three. Stage 4B's four beam results remain equal
to the prior product. Default-font Stage 3 also passes all four limits with
exact packets. Final A's five frames also pass with exact packets; frame
15,090's beam error improves to 0.812111. All seventeen image checks pass.
The complete `lunatic-0-b` and `lunatic-1-a` recordings pass 232,906 calculations
on all 25 fields, end scores and natural exits. Retained-evidence verification
also passes both. The same product's 108-recording gate and retained-evidence
verification now pass 12,311,460 calculations on all 25 fields, end scores,
fingerprints and natural exits. Evidence is under
`build/surface-filter-repair-20261014/x64-suite/`, reusing validated retail
references with fresh isolated recorder processes. The i386 refresh remains
separate. Compiled source checkpoint is `183cd755`;
all 132 recorded build-input hashes agree with its normalized Git blobs. See
[the scaling observation](PORTABLE_LASER_VISUALS.md#remaining-title-texture-scaling).
The complete `lunatic-0-b` recording and
retained-evidence gate pass 113,221 calculations on all 25 fields, end scores
and natural exit. All six renderer contract groups pass. The new product's complete corpus and i386
refresh remain separate gates. See the
[font controls](PORTABLE_LASER_VISUALS.md#font-rasterization-controls-october-14-2026).

The earlier clip-control API probe retains 15 striped-material sampling
differences; the legacy probe retains 16. An XY grid control passes 59/68
images, with nine first-row differences coinciding with Wine's offset-viewport
sensitivity. Private viewport mapping does not resolve them. Its evidence
is under `build/laser-sampling-20261014/`; `--position-grid 256` reproduces
the control. Final B's seven added frames exposed one-ULP origin-Y differences
in slots 12 and 19 at 8,400. Paired spawn observations locate them in the ECL
standalone sine/cosine outputs: eight differing offsets among 390 calls, with
both failing origins created at 8,242. The opcode 32/33 repair now passes all
390 input records and all seven laser/VM packets with unchanged beam errors.
GCC 8 recorder SHA-256 is
`ee78ef7efa7f9e79bd89f1b82320d266f8d63725eefd0298a4a3b2330b4aca10`, under
`build/laser-spawn-origin-20261014/production/`. Its affected native RunEcl
unit passes a fresh build and exact comparison. The full replay refresh is
next; i386 remains separate. See the
[Final B observation](PORTABLE_LASER_VISUALS.md#final-b-precision-observation-october-14-2026).
Extra images remain to be captured. Issue #20's reported scene is unspecified.

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

The Magic Team Lunatic Final A case `finala-eirin` adds five paired frames.
The five baseline failures lead to a shared-owner repair: target address
`0x004E4B64` is `Background::spellVmScriptBase @ +0xB34`, previously split into
standalone `g_GuiMessageStageMode` storage in native and portable. All production
and probe writers now use the field. Seven relocation entries migrate to the
canonical Background base with their `0xB34` addend; masked instruction bytes
remain equal. The cold gate passes all 1,106 accepted units. Fresh normal and
bugfix links pass the new seven-write/one-read owner guard; the retained older
native image fails that guard.

The initial repaired x86_64 product `c421597e…` passes all five original image
limits, with means 0.003248–0.862803 and the pinned retail laser/VM hash. At
frame 9,570, the two background submissions now match retail words exactly;
the baseline differed in 22 position/UV words. The final x86_64 recorder is
`34bcdd33b2d31a52e418e9f806314fb00c68ac2fd5cea2bad8259f6a5de9fab9`, with build
identity under `build/laser-background-owner-20261014/production/`. Its allocated
ELF sections match the initial repaired product. All five final captures pass
the original limits and retain pixel-identical playfields to the initial repair.
Its complete `lunatic-1-a` recording and retained-evidence check pass 119,685
calculations on all 25 fields, recorded end scores and natural exit.
Native bugfix SHA-256 is
`1b8e217a7fe48b74304b1cbf01600385e78f13273e63a6b90bcc1c32436e45f4`.
RT-014 is closed after native Final A repetition: four default-profile playfields
are pixel-identical to retail, and frame 15,090 becomes pixel-identical in paired
`csmt=0` captures. Its default screenshot lagged despite exact laser packets and
all 22,590 recorded calculations. The explicit Wine scheduling control resolves
that capture failure; both profiles and the failed image remain retained.
The native complete `lunatic-1-a` recording passes 119,685 calculations on all
25 fields, end scores and natural exit at clock rate 128 without rasterization.
Native evidence is under `build/laser-native-20261014/`. The runner supports
`--native-map` and paired `--wine-csmt 0`; the comparator requires matching Wine
settings and completed mapped-observer evidence.
The native refresh under `build/laser-native-20261014/full-state128/` is complete:
108/108 recordings and 12,311,460 calculations agree on all 25 fields, recorded
end scores, fingerprints and natural replay exits. Retained-evidence verification
also passes all 108. This gate uses fresh serial candidate captures against
validated retail references, clock rate 128, no rasterization and ptrace
observations. Owned runtime directories and the shared Wine prefix are removed.
The summary pins the native product above to owner-repair checkpoint `096c5a87`.

Next run the repaired ECL product's full replay corpus, refresh i386 and
capture Extra visual cases. Evidence is under
`build/laser-background-owner-20261014/` and `build/laser-stage4a-20261014/`; see the
[Final A observation](PORTABLE_LASER_VISUALS.md#final-a-observation-october-14-2026).
The scout now supports interval 1, samples at Present entry, records counter
gaps, and enforces a 32 MiB snapshot capacity. This recording's Stage 4A survey
observed no active lasers in 22,947 presented samples; its older report predates
gap accounting and supports no claim about unobserved calculations.

For runtime work, select a concrete remaining visual or hardware scenario and
retain the stage, route, recording, frame, and comparison evidence. For exact
reconstruction, resume the authored near match or one bounded whole-link/library
dependency. The ANM semantic queue keeps opcodes 25, 31, and 88 neutral until
complete TH08 consumer evidence supports a shared-layout rename.

<a id="documentation-batch-for-local-review"></a>

The completed human/agent documentation batch and earlier repair investigations
are retained in the [handoff archive](RE_HANDOFF_HISTORY.md#replay-investigation-handoff--2026-10-13).
