# Replay validation results

The native VC7 build and the Linux x86_64 and i386 builds each reproduce the
original Japanese TH08 1.00d gameplay trace for **108 recordings and
12,311,460 calculations**. Every calculation agrees on all 25 recorded fields,
including the exact floating-point bits. Each recording reaches its recorded
stage-end scores and natural replay exit and matches the published retail
trace fingerprint.

These results were completed on October 11–13, 2026. The native repairs are
merged in [PR #26](https://github.com/N0zoM1z0/th08/pull/26); the portable repairs
and verification tools are merged in
[PR #28](https://github.com/N0zoM1z0/th08/pull/28).

## Verified products

| Product | Recordings | Equal calculations | Observation method |
| --- | ---: | ---: | --- |
| Windows i386, VC7 `bugfix`, run through Wine | 108 / 108 | 12,311,460 | External, target-byte-checked hardware breakpoints |
| Linux x86_64, Debug recorder | 108 / 108 | 12,311,460 | Compiled typed reads; focused independent GDB checks |
| Linux i386, Debug recorder | 108 / 108 | 12,311,460 | Compiled typed reads; focused independent GDB checks |

The native gate rebuilt its normal and bugfix images from a cold state and
captured all 108 retail/candidate pairs afresh. The portable suites launched a
new process for each recording and reused those validated retail references.
Separate retained-evidence passes recomputed all native comparisons and all
portable comparisons. The portable `--verify-only` passes also checked capture
identities, completion records, saved reports, and suite ledgers.

### Refresh after spell-background ownership repair

The native owner-repair checkpoint `096c5a87` passes the full 108-recording
corpus again: 12,311,460 calculations agree on all 25 fields, stage-end scores,
fingerprints and natural exits. Retained-evidence verification also passes.
Its fresh VC7 bugfix product is SHA-256
`1b8e217a7fe48b74304b1cbf01600385e78f13273e63a6b90bcc1c32436e45f4`.
This refresh uses new serial candidate captures against validated retail
references, clock rate 128, no rasterization and ptrace observations. Evidence
is under `build/laser-native-20261014/full-state128/`; owned runtimes and the
shared Wine prefix have been removed.

The main backport checkpoint `5dfd5197` also has fresh normal/bugfix links and
a passing cold 1,106-unit gate. Its new bugfix product
`9e699dd2c596acb090a5d44e9aa32b6c0bb19f0d5e11fe6a0babb0eb1469fd9b`
passes five fresh paired Final A images with `csmt=0`: complete playfields are
pixel-identical, raw laser/VM packets agree, and all 25 fields agree across
the 22,590-calculation diagnostic prefix. Its complete 108-recording replay
gate and separate retained-evidence pass now also succeed: 12,311,460
calculations agree on all 25 fields, stage-end scores, published fingerprints
and replay completion boundaries. The refresh uses serial candidate captures
against validated retail references, clock rate 128, no rasterization and the
ptrace observer. Evidence is under
`build/laser-native-20261014/main-full-state128/`; temporary runtimes and the
owned Wine prefix are removed.

The main backport is merged in
[PR #31](https://github.com/N0zoM1z0/th08/pull/31), checkpoint `08c7f00f`.

The [laser investigation](PORTABLE_LASER_VISUALS.md) records the visual
reproduction, canonical field ownership and image comparisons. Its latest
Linux surface-filter repair passes 640 pinned-D3DX8 API cases and seventeen
selected images. Its GCC 8 x86_64 product, SHA-256
`7eb846a665867ed9e8a1c19484d2c0fe1ee586954f0ecca3b1b756223e0e0853`,
passes all 108 recordings and 12,311,460 calculations with a separate
retained-evidence pass. This refresh uses fresh serial candidate processes,
validated retail references and the compiled recorder without rasterization.
Source checkpoint is `183cd755`; evidence is under
`build/surface-filter-repair-20261014/x64-suite/`. The added Final B image
samples exposed a one-ULP laser-origin difference at frame 8,400 in fields
outside the gameplay trace. The subsequent ECL sine/cosine repair passes all
seven image/packet checks and 390 paired spawn-input records; its full corpus
refresh is recorded below. The laser guide retains the failed and repaired
evidence.
Earlier portable full-corpus results remain tied to the products listed below.

The subsequent ECL-repaired x86_64 product, SHA-256
`e0a57a43520524713b70dbaa38329155fd6ab4a09006869b4f09db8dbcc27649`,
source `897decb2`, passes all 108 recordings and its separate retained-evidence
verification: 12,311,460 calculations agree on all 25 fields, recorded
stage-end scores, published fingerprints and natural exits. All temporary
runtimes are removed; the 108 equal traces share validated reference storage.
Evidence is under `build/laser-spawn-origin-20261014/x64-suite/` and
`x64-suite-revalidation.log`. This completed gate supersedes the earlier
73-recording snapshot.

The refreshed GCC 12 i386 product, SHA-256
`bd8b148c0ac6a478391ee4c1fea7334ac1f473466418131715cda4e1a900ce40`,
source `ed8ff8df`, also passes all 108 recordings and a separate
`--verify-only` pass: 12,311,460 calculations agree on all 25 fields,
recorded stage-end scores, published fingerprints and natural exits. The
runner launches fresh serial candidate processes against validated retail
references, with rasterization disabled. All temporary runtimes are removed;
all 108 equal traces share reference storage. Evidence is under
`build/laser-i386-refresh-20261014/full-suite/`, `full-suite-revalidation.log`
and `full-suite-audit.json`. Its refreshed scene checks follow the complete
gameplay gate and are recorded in the laser report.

## Coverage

The [committed corpus](../tests/fixtures/replays/th08-1.00d/README.md) contains
four teams and eight solo characters: Border, Magic, Scarlet, and Ghost Teams;
Reimu, Yukari, Marisa, Alice, Sakuya, Remilia, Youmu, and Yuyuko. Every selection
has an Easy, Normal, Hard, and Lunatic recording through each Final route,
plus an Extra recording.

| Difficulty | Final A recordings | Final A calculations | Final B recordings | Final B calculations |
| --- | ---: | ---: | ---: | ---: |
| Easy | 12 | 1,306,507 | 12 | 1,428,086 |
| Normal | 12 | 1,501,889 | 12 | 1,507,910 |
| Hard | 12 | 1,423,291 | 12 | 1,543,941 |
| Lunatic | 12 | 1,484,719 | 12 | 1,515,122 |

The 12 Extra recordings add 599,995 calculations. Together, the 96 story
recordings and 12 Extra recordings cover all nine internal stage indexes:
Stage 1, 2, 3, 4A, 4B, 5, Final A, Final B, and Extra. Stage 4 follows each
selection's recorded route.

[config/replay-fixtures.json](../config/replay-fixtures.json) supplies every
case's file hash, stage headers, calculation count, and retail fingerprint.
The [native guide's tables](REPLAY_TESTING.md#reproduce-published-results)
provide the per-selection calculation counts. Author credits and original
download links are retained in the
[credit index](../tests/fixtures/replays/th08-1.00d/CREDITS.md).

Three additional recordings failed completeness in retail under the recorded
Wine environment. They remain credited diagnostic fixtures in
[replay-rejected-fixtures.json](../config/replay-rejected-fixtures.json), with
the observed failures and replacement recordings documented in the
[reference failure record](REPLAY_TESTING.md#reference-fixture-failures).
The accepted grid uses recordings that complete in retail.

## What a pass checks

The observer records gameplay state at playback-control entry, after the
calculation callbacks. The
[25-field schema](../scripts/replay-external-schema.json) covers scores, graze,
deaths, lives, bombs, power, player position and state, RNG seed and generation,
replay input, point items, time orbs, gauge, clock, point value, character, shot,
difficulty, and calculation/stage/frame counters.

A complete pass requires equal raw values at every calculation, continuous
counters, stages in the recorded order, consumption of the recorded inputs
through the documented trailer, the recorded end score for every stage, and
the game's natural replay exit. Float fields use their IEEE-754 bits. The
comparison applies no tolerance and ignores no fields.

Both traces must also reproduce the fixture's published calculation count and
fingerprint. The fingerprint hashes the schema as sorted, compact JSON plus a
newline, then every row as little-endian uint32 values in schema order. JSON
spacing and gzip compression do not affect it.

The full suites run serially, muted, with accelerated clocks and rasterization
disabled. Calculation and draw callbacks still run. The native ordinary-paced,
rasterized demo gate separately passes all three bundled demos: 6,120 Stage 5
frames, 4,920 Stage 4A frames, and 5,040 Stage 3 frames, for 16,080 frames with
all 22 demo-schema fields equal. Its combined fingerprint is
`9481fa99b1a9d9bfeda397d3a5051ab0a9f8ad9818d2ed90e7c822d86e3ce9b0`.

## Bugs found and repaired

| Area | Cause and resulting behavior | Evidence |
| --- | --- | --- |
| Fantasy Seal trajectories | Restored the original sine/cosine call order and corrected relocation bindings | [RT-009](RUNTIME_ISSUES.md#rt-009) |
| Spellcard orbit lifetime | Death mode 2 now preserves the six attached orbits, restoring subsequent effect allocation and RNG consumption | [RT-010](RUNTIME_ISSUES.md#rt-010) |
| Stage transitions | Spellcard chain removal reads the live lifetime owner, removing the preceding stage's callback | [RT-011](RUNTIME_ISSUES.md#rt-011) |
| Laser cancellation | Implemented the empty trigonometric helper, restoring cancelled-laser item positions and scoring | [RT-012](RUNTIME_ISSUES.md#rt-012) |
| Solo Yukari input | Homing options read the replay-fed gameplay input | [RT-013](RUNTIME_ISSUES.md#rt-013) |
| Portable vector angles | Uses the original double `atan2` path and retains trigonometric precision through vector multiplication | [Vector precision](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_VECTOR_PRECISION.md) |
| Portable D3DX normalization | Reproduces the observed x86 SSE normalization order, restoring particle lifetimes and effect-pool allocation | [Normalization](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_NORMALIZATION.md) |
| Portable polar motion | Retains trigonometric precision until the target's float stores, restoring enemy trajectories and shot hits | [Polar motion](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_POLAR_MOTION.md) |
| Linux i386 device precision | Device creation selects the observed single-precision x87 mode and honors `FPU_PRESERVE` | [i386 precision](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_I386_FLOATING_POINT.md) |

The focused investigations retain old-build failures, target observations, and
independent diagnostics. After the shared gameplay changes, a cold VC7 build
passed all 1,106 accepted units; normal and bugfix final-link checks also passed.
That check preserves existing function-level exact results. Whole-executable
identity and the library helper's separate exact status remain tracked by the
reconstruction ledgers.

Replay-driven image checks also found portable perspective, pixel-center,
backbuffer-lock, screenshot, and fullscreen-size defects. The repaired renderer
passes four pixel-contract groups, actual HOME-key tests in both display modes,
four retail frame comparisons, and six bounded render windows with 8,810
samples. The selected-frame mean RGB errors are 0.005193 (Stage 1), 0.004768
(Stage 3), 0.292928 (Stage 5), and 0.323372 (Final B), on the 0–255 channel scale.
Stage 5 ordinary/accelerated playfields and camera packets agree exactly.
The [rendering report](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_RENDERING.md)
records the limits, negative controls, and sanitizer checks.

The [laser visual investigation](PORTABLE_LASER_VISUALS.md)
adds eight selected Stage 3 and Stage 4B boss frames and a missing-beam control.
All laser and VM packets agree exactly. The October 13 baseline passed one
strict beam-region image check and failed seven. After the Linux color, font
height, and edge-coverage repairs, all eight pass the unchanged 1.0 mean RGB
error limit. Font rasterization and synthetic texture-sampling differences
remain documented in that investigation. The revised x86_64 product also passes
the full corpus as recorded above. The original seven image failures are
resolved; the added Final B packet difference now has a verified ECL
sine/cosine repair and its own input regression.

## Reproduce the results

Use the [native replay procedure](REPLAY_TESTING.md#reproduce-published-results)
for VC7 builds and fresh retail/native captures. Use the
[portable replay procedure](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_REPLAY_TESTING.md)
on `port/portable-64bit` for the Linux recorder build, all 108 recordings, and
the `--verify-only` retained-evidence check. Its i386 build instructions select
the fixed-layout product; its x86_64 instructions select the native-layout
product.

Each full command works from the committed recordings and privately supplied
executable and archives. Missing retail references are captured locally. No
maintainer-local trace directory is required for a fresh run. A new build can
have a different executable hash; reproduction requires the same complete
trace fingerprints and counts, with the new artifact's identity recorded.

### Recorded artifact identities

The target is Japanese 1.00d, size 840,704 bytes. SHA-256 identities:

| Artifact | SHA-256 |
| --- | --- |
| Original `th08.exe` | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| `th08.dat` | `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb` |
| `thbgm.dat` | `2c2ef05f9ff6f43f752dae5da519a2477372c3f8875b7b44996a8f69fdad4d89` |
| Native VC7 candidate | `19b24c3a1d06de5414114ff5f224576f5047d9a0d9c38d7bf5c4ec3612d6723e` |
| Native linker map | `2289537ed1d5edd556a7afbd166eb36333c2851ba61e0cf82a59a09fef242ec7` |
| Linux x86_64 candidate | `d74a574ac983d2df7c7a52cd421a868a01b90c34471881802ed83cea9e5b802f` |
| Linux i386 candidate | `34a358542feb2188304ce91c7b02b60d2a463a167f74f8f698add8d96296b9af` |
| Portable compiled observer source | `6f63f7d77ba942d6dafd001823606f4ee7efde89a276238fa92da6ffee1b3500` |
| External replay schema file | `363345e2167257b5c04a52d35565aca2c4658432bfa1bf462f4c2c7b1586835b` |

The native cold captures identify clean source `c47cc97a`; its repairs were
already present at `25c101f9`. Both portable artifacts were built during work
on top of `92f9bbb5`, with a dirty source tree. Their provenance records pin
the relevant build, observer, and repaired source files by hash and attest
those files against committed checkpoint `af9cb1d3`, using the repository's
checkout line endings. The artifact hashes above identify the binaries actually
exercised.

### Local evidence and resources

These are maintainer-local generated directories; the procedures above regenerate
the evidence. Original game assets and generated reports are kept outside Git.

| Evidence | Location under `build/` |
| --- | --- |
| Fresh retail/native pairs and archived executable/map | `replay-cold-20261011/matrix/`, `replay-cold-20261011/candidate/` |
| Ordinary-paced native demos | `replay-cold-20261011/demos/` |
| Linux x86_64 full suite and binary provenance | `portable-state-20261012/matrix-final/`, `portable-state-20261012/final-x86_64-candidate/` |
| Linux i386 full suite and binary provenance | `portable-state-20261012/i386-matrix-fpu/`, `portable-state-20261012/fpu-i386-candidate/` |
| Comparator revalidation and memory measurements | `portable-state-20261012/streaming-comparison/` |
| Render repairs and subsequent frame regression | `portable-render-fix-20261012/`, `portable-state-20261012/polar-rendering-regression/` |

The portable runs used an x86_64 WSL2 Linux host, Xvfb, and Mesa llvmpipe.
The x86_64 candidate used Ubuntu GCC 8.4.0; the i386 candidate used Debian
GCC 12.2.0 in the pinned multilib container. Native captures used Wine 9.0.
The retained i386 provenance originally labeled its compiler as GCC 8.4.0.
Its build log and ELF `.comment` identify GCC 12.2.0; a local correction
addendum preserves the original record and both evidence hashes.
Per-capture metadata preserves host, backend, source, hashes, and playback
settings. Retain that metadata, `complete.json`, `comparison.json`, `suite.json`,
compressed rows, the candidate, and build provenance for an independent audit.

| Portable operation | Elapsed time on this host | Peak resident memory |
| --- | --- | ---: |
| x86_64 full suite | 4 h 14 min | 240 MiB |
| i386 serial suite, reusing one completed case | 5 h 21 min 59 s | 205 MiB |
| x86_64 retained-evidence verification | 10 min 51 s | 53 MiB |
| i386 retained-evidence verification | 11 min 5 s | 53 MiB |

Trace comparison streams rows. On the 126,803-calculation `easy-1-b` pair,
its peak memory fell from 227 MiB to 17.5 MiB, with identical comparison output.
Equal compressed traces share storage only after their decompressed bytes
have been checked. Temporary runtimes, owned Wine prefixes, and completed
compiler trees were removed; tested binaries and useful evidence were retained.

## Scope of the conclusion

This establishes gameplay-state equality for the 108 selected recordings and
25 fields. The native demo gate and portable image checks supply their own
bounded evidence for ordinary pacing and rendering. Small pixel differences
and nine unconfirmed Stage 5 color suspects remain documented in the rendering
report. Audio playback, AArch64 hardware gameplay, and Web behavior have
separate validation requirements. AArch64 currently has cross-build and
initialization evidence.

Practice, Spell Practice, user-selected paths, and input sequences beyond this
corpus can reveal additional bugs. Reports with the build, stage, route,
reproduction steps, replay, and screenshots help extend the evidence.
