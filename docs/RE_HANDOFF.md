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

Finish the current portable sampling candidate's runtime gates before its PR.
The renderer source checkpoint is `2fd6a51b`. Both tested products have the
same 132 build inputs:

- GCC 8 x86_64: `dcf14c0c8014073a15e9c1df7fcba45ccf5a6e9f1963cc2ff2e17c60fc4ed611`.
- GCC 12 i386: `240b3f9a4bae462a73129fe0d6b10b65ad44fe9a036889694e422b71cd42ca98`,
  built from `bd1f826f` in the pinned container with one CPU, 768 MiB and one job.

ELF/ownership checks pass for both products. The x86_64 candidate passes all
five laser scenes in default and OpenGL 2.1 contexts: 48 selected images,
exact laser/VM packets and retained comparisons. The i386 preflight passes
eight renderer/FPU groups and 68/68 observed-quad images per context, plus
640/640 surface API cases. The compiler trees and completed probe binaries
are removed. See [the candidate report](PORTABLE_LASER_VISUALS.md#production-candidate-october-15-2026)
for source hashes, measurements and reproduction commands.

The sampling candidate's workers are superseded. Its x86_64 attempt retains
65 completed replay comparisons and one captured case without a completed
suite comparison. Its 10 scene/context groups remain passed. The queued i386
worker was retired before scene/corpus execution. Inspect the retirement
record in `build/laser-d3dx-math-repair-20261015/obsolete-retirement.json`;
only the latest products continue through the full gates.

- `production*/build-provenance.json`: tested products and all build inputs.

Keep production inputs unchanged while these pinned jobs run. Bounded alpha
captures suspend the same scheduler at recording boundaries; the active
alpha-repair section below records their pause ownership. A missing uncompressed
capture log can mean successful compression; inspect completion metadata and
live processes before treating a job as failed.

Final B frame 14,280's maximum residual is localized to alpha-test rounding.
The 15 selected regional quads match all 420 words, and portable texture
payloads match `etama.anm` entries 0 and 5. Enabling the captured alpha-test
state in the asset-free probe exposes 16/6,144 production pixel differences;
a floating-point comparison control passes 6,144/6,144 in both GL contexts.
A shader control with eight-bit rounding reproduces production exactly.
Replaying the five low-alpha triangles at `(168, 310)` over retail's
`(30, 2, 2)` yields production `(46, 12, 12)`, exactly the screenshot residual;
floating-point comparison restores retail. This is a bounded regional replay,
not a complete-frame fix. Other residuals still need attribution.

The new profile is `probe-texture-filtering.py --alpha-test`. Its diagnostic
patch and reproduction commands are in
[the controls guide](../tests/fixtures/render-controls/README.md#floating-point-alpha-test-control).
The isolated implementation and new candidate gates are recorded below.
Keep the current products and baseline results separate from the alpha repair. Reports and regional probe source
are under `build/laser-sampling-production-20261015/`; transient original
texture bytes, Wine prefixes and executables are removed.

Use `--wine-csmt 0` for paired quad/image capture. The default-threading
retail quad observation produced a stale image despite matching raw packets;
the synchronous control resolves that failure. Preserve its failed evidence
separately. Shader, matrix, vertex and omission observations are recorded in
the laser report; they remain distinct from function-matching evidence.

Issue #20's reported scene remains unspecified. Font differences, the Stage 5
color observations and native RT-007 retain their documented scope. AArch64
hardware and audio have separate pending checks; web work remains deferred.

Detailed prior checkpoints are preserved in the
[laser session archive](RE_HANDOFF_HISTORY.md#laser-validation-checkpoints-before-the-final-platform-refresh-october-14-2026).
For exact reconstruction, the sole authored near match and bounded whole-link
or library work remain independent of these runtime gates. The ANM semantic
queue keeps opcodes 25, 31 and 88 neutral until complete TH08 consumer evidence
supports a shared-layout rename.

<a id="documentation-batch-for-local-review"></a>

The completed human/agent documentation batch is retained in the
[handoff archive](RE_HANDOFF_HISTORY.md#replay-investigation-handoff--2026-10-13).

## Active alpha-test repair

The isolated worktree `.analysis/portable-alpha-test/` holds branch
`fix/portable-alpha-test`, implementation checkpoint `80d7635d`. The original
working tree retains the earlier x86_64/i386 artifacts for the sampling
baseline. Keep those baseline results separate from the alpha candidates.

The first alpha implementation, `1514eaed`, fixes actual Final B frame 14,280:
all 12,059 beam-mask pixels agree with retail, all 92 earlier changed mask
pixels disappear, and the complete playfield below the title band is
pixel-identical. Laser/VM words agree exactly. Title text and HUD differences
retain their separate scope. Its dynamic shader has a confirmed synthetic
performance cost, so it remains a diagnostic checkpoint.

The cached implementation specializes material operations, comparison and
fog while retaining alpha-reference and texture-factor uniforms. Its two
fresh products have the same 132 pinned build inputs and pass ELF/owner checks:

- GCC 8 x86_64: `e41f8717fc26dd04373696f6e720f850137204cce4af5befe0c224b5b79fb5fb`.
- GCC 12 i386: `b09cb926b396d4c8346337a0b5b11024a9496de8a3bdbfafa8f8db25f8ffb948`.

The cached alpha-state matrix and low-alpha filtering profile each pass all
6,144 samples on both architectures in both GL contexts. The x86_64
alpha-enabled legacy observed-quad probe passes 68/68 images. A warmed,
interleaved three-round benchmark gives medians 0.360763 seconds for cached,
0.371129 fixed-function and 0.327325 narrow float control, with equal float
variant image checksums. Repeating through the public benchmark gives
0.440766 cached and 0.437427 fixed-function. The earlier dynamic program took
0.775951 versus the narrow control's 0.348062. These are synthetic rendering
measurements; whole-game gates remain required.

Reproduce from the alpha branch with
`benchmark-alpha-pipeline.py --baseline-ref 575c2368`. The matrix profile is
`probe-texture-filtering.py --alpha-states`, and `probe-subpixel-quads.py
--alpha-test` checks observed vertices with the actual alpha-test state.
See [the control guide](../tests/fixtures/render-controls/README.md) and the
[laser report](PORTABLE_LASER_VISUALS.md#alpha-pipeline-candidate-october-15-2026).

Evidence and completed/retired worker records are under `build/laser-alpha-repair-20261015/`:

- `production-cached*/build-provenance.json`: clean build-start identity,
  132 hashes, commands and products. Compiler trees are removed.
- `finalb-frame/` and `finalb-frame-summary.json`: the slower dynamic candidate's
  actual-frame proof. `candidate-build-provenance.json` binds its separate build;
  the original source-commit metadata identifies capture tools.
- `cached-finalb-worker.json` and `cached-finalb-summary.json`: passed all seven
  cached Final B frames, with exact laser/VM packets. Five outside-title beam
  masks are pixel-identical; two retain four old pixels, each at most 1/255.
  The repair resolves 403 of 407 earlier residual pixels. The same scheduler
  PID 653259 is resumed; `active` is false. Capture time is about 354 seconds.
- `cached-api-gates.json`: passed remaining x86_64 filtering and four i386
  API profiles. Probe executables and owned Wine prefixes are removed.
- `queued-cached-gates.json`: retired before any gate started, after the
  newer sincos candidate passed its seven-frame capture. The retirement
  report records the replacement worker and source checkpoint.
- `alpha-benchmark*/report.json`: generated probe, renderer/binary identities,
  compile commands, raw timings and checksums. The public rerun is in the
  isolated worktree's `build/cached-alpha-benchmark/`.

The following candidate continues the residual investigation. The earlier products and their retirement scopes remain recorded under
`build/laser-sampling-production-20261015/`. Keep the latest gate inputs frozen.

## Sincos precision candidate

The follow-up is isolated in `.analysis/portable-sincos-precision/`, branch
`fix/portable-sincos-precision`, source checkpoint `0c9f0ef4`. It changes only
the modern sincos macro to double evaluation followed by float stores. Its
only changed input among the 132 build inputs is `src/ZunMath.hpp`.

A synchronous paired capture of frames 11,490 and 17,460 repeats all four
residual pixels. Both retail playfields match the retained retail exactly;
both portable playfields match the ordinary cached candidate. Region input
comparisons find three differing quads. Quad 1,031 at frame 17,460 is laser
slot 84's body VM, with identical recorded VM fields. Its angle `0x3f879c1a`
exposes glibc cosf `0x3efa8c2a` versus the observed target-compatible and
80-digit-oracle result `0x3efa8c2b`. The corrected arithmetic restores all
observed XY words. The other two residuals occur on projected particle quads;
their projection inputs still need observation.

Eight golden sincos cases, the preceding-header failure control, i386
single-precision mode and the existing 10 vector contracts pass. The selected
82 AnmManager comparisons and the full 1,106-unit single-job cold comparison
pass against the canonical target. Reports are in
`build/laser-residual-pixels-20261015/`. VC7 object/PCH/PDB intermediates are
removed after comparison.

Fresh recorder products, both with verified ELF/global ownership, are under
`build/laser-sincos-repair-20261015/`:

- GCC 8 x86_64: `25af84840b5175da7718f640359c9f9220d8c1f5c68f6d686a512f1cc9d1c370`.
- GCC 12 i386: `8faef6f0234480746122fb9f4987ad906fddce516ddb61aa61d5cbf7959560a4`.

`production*/build-provenance.json` records the clean source commit, all
inputs, commands and product hashes. Their compiler trees are removed.
`finalb-worker.json` records a completed seven-frame x86_64 default-context
capture with exact laser/VM packets. The original scheduler is resumed and
`active` is false. The two laser-slot-84 pixels disappear; the only remaining
outside-title beam differences are frame 11,490 at `(307, 179)` and frame
17,460 at `(329, 90)`, each at most 1/255. This resolves 405 of the original
407 beam-mask differences in that region.

The complete playfield below the title band has 23 remaining changed pixels
across the seven frames, all at most 1/255. Only the two repaired pixels change
between candidates; no new image differences appear. `pixel-regression.json`
records both scopes. From the sincos branch, `summarize-laser-pixels.py
--full-playfield --top-pixels 32` reports both masks and all these residual
coordinates without changing the strict image comparison.

`api-gates.json` passes all eight current-source profiles: two architectures,
two GL contexts, and alpha-state/low-alpha filtering, each with 6,144 samples.
The sincos candidate's queued worker was retired before any scene/corpus
gate started. Its completed numerical and seven-frame evidence remains
specific to that source. The D3DX candidate below replaces it for full gates.
The retired alpha and sincos queues must remain stopped.

The regional comparator now supports `--frame`, conservative
`--selection intersecting-bounds` and `--check`. All captured rows and stages
are validated before local selection. The new packed-precision probe matches
1,572,864 RGB samples for each point/quarter-texel-linear profile. Reproduce
these methods from the alpha or sincos branch using the
[control guide](../tests/fixtures/render-controls/README.md#exhaustive-packed-color-precision).
For the scalar repair, run `python3 scripts/test-portable-sincos.py` from the
sincos branch, or use `--baseline-ref b4e70194 --expect-failure` for its recorded
Linux failure control. Both new probes use generated data and remove their
executables, raw buffers and owned temporary runtimes.

## Orbit particle rounding repair

The corrected retail-only observer completes both frames, and the same
scheduler PID 653259 is resumed. `confirmation-worker.json` is passed with
`active: false`. It identifies effect slots 349 and 462, both effect ID 13,
with `EffectOrbitUpdate @ 0x00426030`. All recorded effect fields agree except
position; both inputs reproduce the original projection locals and XY words.
The initial failed observation remains separate diagnostic evidence.

The geometry-only portable observation also completes and resumes the same
scheduler. `geometry-worker.json` and `geometry-revalidation.json` record exact
laser/VM packets and all 469/3,087 selected playfield quads. Its inferior
records `LP_NO_RAST=true`; this is numerical evidence alongside ordinary
rasterized images. Two observer contracts reproduce and detect the earlier
hardware-slot and temporary-`__file__` failures.

A pinned original-library probe reproduces both target orbit positions.
`quaternion-library-comparison.json` attests all 226 bytes of
`c_D3DXMatrixRotationQuaternion @ 0x004781CD`, including its relocated 2.0
constant. `transform-library-comparison.json` attests the complete 256-byte
SSE transform-coordinate section at runtime-observed `0x0048EDF0`.
The latter entry is not separately added to the current inventory; neither
comparison changes an acceptance ledger or aggregate count.

The follow-up is isolated in `.analysis/portable-quaternion-rounding/`, branch
`fix/portable-d3dx-math-rounding`. It changes only the modern D3DX adapter:
component doubling and separate diagonal subtraction for quaternion matrices;
paired vector accumulation, one RCPPS refinement and multiplication for
transform coordinates. The old direct division loses the original reciprocal
rounding even at W=1. A sine-only control leaves both position errors; the
two D3DX repairs restore all matrix/position words in both standalone samples.
No actual-game image repair is claimed before the fresh captures.

The public `scripts/test-portable-d3dx-rounding.py` passes eight attributed
API goldens. `--baseline-ref d1b5ffe4 --expect-failure` reproduces four old
failures. `--native --attest-target` passes all 264 cases, including 128
generated quaternions and 128 projective transforms, and repeats both
relocation-aware archive/target comparisons plus the target constant check.
Reports are in the new worktree's `build/d3dx-rounding-*/`; temporary probe
objects and executables are removed. Reference inputs and recording-credit
links are committed in `tests/fixtures/d3dx-rounding.json`.

Evidence and diagnostic prototypes remain under
`build/laser-projection-inputs-20261015/`. The latest source-pinned products and completed focused gates are recorded
below. Their scene/full-corpus worker is the sole active game queue.
The geometry-only observer still needs publication as a general capture
option; the public API probe and ordinary seven-frame capture commands are
already reproducible.

Source checkpoint `d64dafd8` is committed and pushed. Both fresh products pass
ELF/global ownership verification with 132 commit-attested inputs; only
`src/modern/linux/d3dx8_compat.cpp` changes from the sincos product:

- GCC 8 x86_64: `ef33728154500c7cf8ecfefa6abf32d2f4fd2bf7ab22c32ef4e0b86d5ac09dc2`.
- GCC 12 i386: `5b639a5a2112d1dd9b2c8786d1ab21fdb78346d4273295c08ba7bb252dbe9a55`.

The phase is `build/laser-d3dx-math-repair-20261015/`. `build-worker.json` is
passed and its compiler trees are removed. Both production compiler profiles
pass 264 original-library math cases and both archive/target attestations.
Renderer contracts pass in both GL contexts on both architectures, including
the i386 device-precision case. `api-gates.json` is passed; its worker and the
build worker are terminal with exit code 0.

`finalb-worker.json` is passed and inactive. All seven x86_64 default-context
frames have exact laser/VM packets and zero changed RGB pixels in the entire
playfield below the title band `(32, 64)`–`(416, 464)`. This resolves all 23
prior full-playfield residuals, including both remaining beam-mask pixels.
Title text and timing-dependent HUD content retain their separate scope.

`obsolete-retirement.json` records retirement of the original x86_64 worker
and scheduler at a completed recording boundary, plus the unstarted old i386
and sincos queues. The original corpus retains 65 completed comparisons;
its remaining captured case is not counted as a completed suite comparison.
No old candidate is marked as having passed a complete corpus.

`queued-worker-handle.json` identifies the sole active game queue, PID
1110151, process start 94543796, session 48287. It pins 302 inputs and runs
five scenes in both GL contexts on both architectures, then 108 recordings
and retained verification per architecture. Only the matching new seven-frame
capture is reused. `queued-gates.json` records each gate. Production source,
products, root capture tools and reference evidence stay frozen. Inspect
actual processes before starting another game. The full gates are in flight;
no complete-corpus claim is made for `d64dafd8` yet.

The first broad worker stopped after two completed scene groups because it
rewrote the already-pinned Final B build-provenance sidecar. No image/state
gate failed. The original sidecar bytes are restored and every previous pin
except the intentionally revised controller is verified. The resumed worker
preserves existing provenance, revalidates completed captures and continues
with Stage 4B. `queued-gates-first-attempt.json` preserves the earlier state;
`queued-gates-resumed.log` and the current handle identify the resumed run.

An additional reciprocal boundary probe passes both production architectures
for zero/signed/tiny W values. Its 15 cases include an original-library
initialization call: the first D3DX call uses its C fallback while filling the
shared fast table. Warming that table before vector cases matches the game's
observed active SSE dispatch. `reciprocal-boundaries.json` records all words
and commands; `check-reciprocal-boundaries.py` is still a private diagnostic.

## Reusable geometry capture and remaining survey

The current tool batch changes only unpinned capture/analysis scripts in the
D3DX worktree. Every live production/capture-tool/reference pin is unchanged.
`check-laser-visuals.py --watch-quads --geometry-only` now skips rasterization
and image readback; `--geometry-reference` automatically checks the same
binary's ordinary capture. `compare-sprite-geometry.py --check` independently
verifies identities, complete quad/frame coverage, raw laser/VM packets,
reference images and observed runtime scope. Thirteen regression tests and the
existing 42 laser tests pass; target-independent CI passes. Actual new-mode
validation is next, reusing the retained sincos ordinary quad captures.

The expanded x86_64 default survey has zero below-title playfield error for
all selected Stage 3 and Final A frames. Stage 4B frame 15,360 has 6,929
changed playfield pixels, maximum channel error 22/255, outside the beam
mask. Connected differences follow tall background strips. Its older retail
reference records no explicit CSMT setting; a fresh synchronous canonical
reference is needed before attributing a code defect. Other Stage 4B frames
have zero below-title error. Extra frame 24,900 retains six changed pixels
at most 1/255; the remaining Extra frames are exact below the title. Reports
are `build/math-default-*-pixels.json` in the D3DX worktree. The live broad
queue continues with the original pinned reference scope.

The geometry profile now imports the existing post-Present frame-counter
contract and tests it against the producer, including a missing-boundary
failure control. A retained ordinary capture passes preflight. Capture
metadata also records elapsed wall time for repeatable performance checks.
