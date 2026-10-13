# Laser visual regression

Compare the original Japanese TH08 1.00d and Linux portable images at selected
laser frames. The [replay results](REPLAY_RESULTS.md) establish gameplay trace
equality; this check adds laser-pool state and images from the presented frame.

## Capture a recorded case

Build the x86_64 Debug product, supply the original executable and archives, and
run from the repository root:

```bash
scripts/build-portable-linux.sh x86_64
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case stage3-keine --retail \
  --output-dir build/laser-stage3
```

Dependencies are GDB, Wine32, Xvfb, xdotool, ImageMagick, and Python Pillow,
alongside the Linux build dependencies. `resources/th08.exe` must match the
canonical 1.00d hash. Recordings come from the
[credited corpus](../tests/fixtures/replays/th08-1.00d/CREDITS.md).

Keep the font environment with image evidence: portable uses `TH08_FONT` when
supplied, otherwise Fontconfig's Japanese monospace fallback; retail uses Wine
GDI font substitution. Glyph differences can affect beam-region errors where
text overlays a laser. The recorded host selects VL Gothic for portable;
the Stage 3 retail font trace selects Droid Sans Fallback. Portable captures
now record the selected font file and SHA-256. Use `--font` for a controlled
same-font comparison.

The runner captures portable first and retail second. Both run muted with
normal life rules and playback mode 0. Retail uses ordinary pacing; portable
uses the accelerated clock with rasterization enabled. Add `--paced` to use
ordinary pacing for portable too. Each process has a 900-second limit.
Owned game runtimes and Wine prefixes are removed; logs are compressed.

The [case manifest](../config/laser-visual-fixtures.json) selects Stage 3,
Border Team, Lunatic, Final B recording `lunatic-0-b`:

| Gameplay frame | Selected phase |
| ---: | --- |
| 12,900 | Moving beams |
| 14,130 | Stationary startup |
| 14,310 | Stationary active |
| 14,490 | Stationary despawn |

Phases can overlap across laser slots. The runner requires the selected state
to be present in each frame. It starts directly at the recorded stage.

## Stage 3 observation, October 13, 2026

All captured laser fields agree exactly in the four paired frames. The active
slot counts are 50, 20, 50, and 50; body and cap VM packets also agree. Retail
and portable `lasers.json` share SHA-256
`be8ce47843f9e3d55141c4de2279e2d80beb63393cf16b8c5fa30a14ad74c8d9`,
retained in the case manifest. A separate direct-stage portable diagnostic
matches the published retail reference for 14,500 calculations on all 25
fields. Its first 14,490 rows also match the fresh ordinary-paced retail
capture.

| Frame | Beam-geometry pixels | Beam-region mean RGB error | Strict pixel result |
| ---: | ---: | ---: | --- |
| 12,900 | 10,024 | 2.831371 | Different |
| 14,130 | 3,148 | 2.609911 | Different |
| 14,310 | 50,966 | 1.324700 | Different |
| 14,490 | 46,628 | 2.085793 | Different |

The strict 1.0 limit fails in all four frames. Initial image review shows the
corresponding beams; difference images highlight spell-title text and smaller
background and cap-region differences. Beam geometry overlaps those regions;
the [localization below](#error-localization) identifies the color-conversion
contribution and remaining title/edge pixels. These observations cover selected frames; later bosses,
other routes and difficulties, and Linux i386 images remain to be checked.

Local evidence is in `build/laser-visual-20261013/stage3-aligned/`. The existing
x86_64 Debug recorder artifact has SHA-256
`d74a574ac983d2df7c7a52cd421a868a01b90c34471881802ed83cea9e5b802f`;
recorder hooks were inactive for these GDB captures. The tested gameplay source
is the previously validated `af9cb1d3` checkpoint. These October 13 baseline
captures precede the color repair described below.

## Stage 4B observation, October 13, 2026

Case `stage4b-marisa` uses the same Border Team Lunatic recording. It captures
colored stationary startup, active and despawning beams, then moving beams in
a later Marisa attack. All four raw laser and VM packets match retail exactly.
Both `lasers.json` files have SHA-256
`adea4722edaf1f07fd87199e04c2bf4f32d9ac3b872d093973f959b1b470e503`.

| Frame | Active slots | Beam-geometry pixels | Beam-region mean RGB error | Strict pixel result |
| ---: | ---: | ---: | ---: | --- |
| 13,140 | 5 | 935 | 3.084492 | Different |
| 13,260 | 10 | 3,671 | 2.222010 | Different |
| 15,360 | 10 | 1,035 | 0.003865 | Equal |
| 18,210 | 4 | 10,200 | 1.657255 | Different |

Corresponding colored and moving beams are visible in the paired images.
The despawn frame passes the strict pixel limit; the
[color experiment](#error-localization) accounts for the other three failures.
Evidence is in
`build/laser-visual-20261013/stage4b-marisa/`. To reproduce this selection, use
`--case stage4b-marisa` with the capture command above.

The paired missing-beam control omits 10 on-screen quad submissions while
preserving the identical laser-state packet. Beam-region errors rise to
42.524421, 39.223917, 61.727536, and 63.994869; all four omission checks pass.
Control evidence is in `build/laser-visual-20261013/stage4b-control/`.

## Error localization

The seven baseline pixel failures contain two measured differences. The
Stage 3 title/header band accounts for 29.4%–64.6% of the original error.
Stage 4B's three failing beam masks avoid that band, and their pixels show a
small, consistent dark bias. Cap regions contribute 12.2%–33.0% of those
Stage 4B errors; visible beam regions and background inside the quads account
for the remainder.

The baseline Linux color codecs expand 5/6-bit channels with integer division
and pack them with another truncated division. The Wine D3D8 API probe expands those
channels by bit replication: five-bit value 4 becomes 33, while the production
decoder yields 32. The baseline D3DX surface-copy path also decodes through RGBA and
re-encodes pixels, allowing a same-format copy to lose channel bits.

A private experiment used bit replication for expansion and kept the high bits
when packing. It changed both Linux pixel converters in a separate build.
The standalone Wine/portable probe measures 297 samples: the baseline differs
on 15 of 32 A1R5G5B5 samples and 44 of 64 R5G6B5 samples; the repaired decoder
matches all 297. A4R4G4B4, 32-bit textures,
identical-format D3D8 copies, and nine untextured alpha-blend samples agree
in the baseline too. The separate baseline D3DX copy probe changes 92 of 144
raw pixel words during identical-format copies: 30 A1R5G5B5 samples and 62
R5G6B5 samples. The repaired codecs preserve all 144. This probe exercises
platform APIs; the paired game frames
below supply the TH08 evidence.

| Stage | Frame | Baseline beam error | Repaired beam error | Repaired pixel result |
| --- | ---: | ---: | ---: | --- |
| 3 | 12,900 | 2.831371 | 1.484770 | Different |
| 3 | 14,130 | 2.609911 | 1.244176 | Different |
| 3 | 14,310 | 1.324700 | 0.293679 | Equal |
| 3 | 14,490 | 2.085793 | 1.292878 | Different |
| 4B | 13,140 | 3.084492 | 0.004278 | Equal |
| 4B | 13,260 | 2.222010 | 0.001907 | Equal |
| 4B | 15,360 | 0.003865 | 0.003865 | Equal |
| 4B | 18,210 | 1.657255 | 0.021046 | Equal |

All eight private-build laser and VM packets retain their published retail
hashes. The unchanged strict gate passes five frames. Stage 4B's reduction
localizes its failures to color conversion; the corresponding beams remain
present in both builds.

After the color change, the header accounts for 98.5% and 98.8% of the remaining
error in Stage 3 frames 14,130 and 14,490. Their beam-mask errors outside the
header are 0.019959 and 0.017975. The title glyphs differ between Wine GDI and
SDL_ttf, with selection and metrics examined by the isolated probe below. Frame
12,900 has 60.0% of its remaining error in the header and a small horizontal
beam-edge residual: 49 pixels with mean RGB error above 32 within
`(201, 236)`–`(247, 244)`. Its outside-header mean error is 0.650546. Vertex
and sampling evidence is still needed to explain that residual.

The counterfactual evidence is retained under `build/laser-visual-20261013/`.
The following checks exercise the repair in repository source.

### Production color repair, October 14, 2026

Both Linux converters now expand packed channels by bit replication and pack
their high bits. A fresh Wine comparison matches all 297 color, D3D8-copy,
and blend samples. All 144 D3DX copy samples preserve their raw words. Exhaustive
copies preserve every one of the 65,536 words in each of A1R5G5B5, R5G6B5,
and A4R4G4B4, including alpha bits: 196,608 checks in total.

The pre-repair commit `2ddf6afd` supplies a failure control. It reproduces
59 color-sample differences and 92 lossy D3DX copy samples; exhaustive copies
alter 65,520 words in each of A1R5G5B5 and R5G6B5. A4R4G4B4 remains unchanged.
Renderer lock, perspective, clipping, and fog contracts also pass after the
repair. Fresh production Stage 4B captures pass all four strict image checks,
with errors 0.004278, 0.001907, 0.003865, and 0.021046. Raw laser and VM
packets retain the published retail hash. Stage 3 errors are 1.484770, 1.244176,
0.293679, and 1.292878: one passes and three retain the title/edge differences
localized above. All eight beam masks match the counterfactual RGB pixels,
preserving that error decomposition. The unchanged strict gate
passes five of eight frames. The complete `lunatic-0-b` recording, which
supplies both laser cases, also passes 113,221 calculations on all 25 gameplay
fields, with a natural replay exit. These adapter changes leave the native
VC7 implementation untouched. The earlier 108-recording gates apply to their
published binaries in [the replay results](REPLAY_RESULTS.md).

The tested x86_64 Debug recorder has SHA-256
`f7be8742dd9e7edd4d5a4a161e5b94fde0aba7fa238b49e639493838d9e18fd7`.
Its executable, source hashes, compiler identity, and build configuration are
retained in `build/laser-codec-repair-20261014/production-x86_64/`; completed
compiler outputs were removed. The images and comparisons are in
`stage3-keine/` and `stage4b-marisa/` under the same investigation directory.
The full-recording evidence is in `x86_64-suite/lunatic-0-b/`; use
`scripts/test-portable-replay-suite.py --cases lunatic-0-b` with the portable
replay guide's normal capture and retained-evidence commands.

The initial isolated font API probe uses the game's Japanese face name and semibold,
Shift-JIS, antialiased, fixed-pitch parameters. In a fresh Wine32 prefix it
selects Droid Sans Fallback; portable selects VL Gothic. At requested height
30, Wine reports ascent/descent 24/6 and a 151-pixel width for `Stage 3`;
direct SDL_ttf reports 26/4 and 126 pixels. This measures substitution and
point-size metrics independently of the GDI adapter. The initial probe source
and measurements are in `font-metrics/`;
the committed runner reproduces all six heights in `final-font-metrics/`
under the same investigation directory.

Repeat the isolated measurement with the committed probe:

```bash
python3 scripts/analysis/probe-font-metrics.py \
  --output-dir build/font-metrics
```

It needs SDL2_ttf, Fontconfig, MinGW's i686 compiler, Wine32, and Xvfb. It
records the portable font file and hash, API parameters, locale, source hashes,
and compiler/Wine versions. `TH08_FONT` selects the portable font as it does in the
game. The temporary Wine prefix and probe executable are removed after use.

Local evidence is in `build/laser-codec-repair-20261014/color-contracts/`,
`final-color-contracts/`, and `renderer-contracts/`. Run the asset-free
regression gate:

```bash
python3 scripts/analysis/probe-texture-colors.py --check \
  --output-dir build/texture-color-check
```

This uses the Wine-verified expected samples and runs the portable backend.
Add a fresh Wine reference and the old-source failure control with:

```bash
python3 scripts/analysis/probe-texture-colors.py --wine --check \
  --baseline-ref 2ddf6afd --expect-baseline-failure \
  --output-dir build/texture-color-paired
```

The Wine command additionally needs MinGW's i686 compiler and Wine32. The
Linux CI job runs the asset-free gate; `--check` returns a failure if a color
sample or any exhaustive copy differs.

### Reproduce the decomposition and color experiment

Given the normal and missing-beam captures above:

```bash
python3 scripts/analysis/analyze-laser-errors.py \
  --reference build/laser-stage3/retail \
  --normal build/laser-stage3 \
  --control build/laser-stage3-control \
  --output build/laser-stage3/error-regions.json
```

Regions partition the original beam mask once, in this order: header, cap
quads, pixels influenced by portable beam omission, and remainder. The header
rectangle is `(32, 16)`–`(416, 64)`; influence requires a maximum channel change
of at least 8. The tool retains every pixel's contribution and the original
strict result. This partition is a diagnostic over composite images.

The asset-free color experiment additionally needs MinGW's i686 compiler:

```bash
python3 scripts/analysis/probe-texture-colors.py \
  --wine --baseline-ref 2ddf6afd --counterfactual --build-game \
  --output-dir build/texture-color-experiment
```

The optional private game build uses `g++-8` by default; select another compiler
with `--game-compiler`. Builds and probes run serially. The command removes its
Wine prefix and game compiler tree, retaining source hashes, samples, reports,
and `th08-color-counterfactual`.

Capture `--case stage4b-marisa` with that executable using the existing laser
runner, then compare against a fresh normal capture's `retail/`. Use
`--case stage3-keine` for the Stage 3 comparison. The captures require the same
original archives as the normal procedure.

## Font-height regression, October 14, 2026

The Stage 3 original-game capture records 81 bold-text calls. Every text byte,
position, size, color, and outline parameter agrees with portable. The target
requests GDI height 28; Wine selects `DroidSansFallbackFull.ttf` and maps that
height to 21 ppem. The native observer reads the attested `CreateFontA` return
at `0x0043FA12` with a hardware breakpoint; gameplay memory and target bytes
remain unchanged. `--watch-text-fonts` writes `text-fonts.json` and retains
Wine font diagnostics in the compressed retail log.

The Linux adapter previously forwarded GDI height directly to SDL_ttf's
point-size argument. Positive GDI height includes internal leading; negative
height selects the em square. The repair uses the font's OpenType Windows
ascent/descent to select the em size and aligns the rendered baseline with
GDI's ascent. See [CreateFontA](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta)
and the [SDL_ttf point-size contract](https://www.libsdl.org/projects/docs/old/SDL_ttf/SDL_ttf_14.html).
Linux builds now link FreeType explicitly. Native VC7 source and build flags
are unchanged.

| Stage 3 frame | Color repair, default font | Height repair, default font | Height repair, matched Droid font |
| ---: | ---: | ---: | ---: |
| 12,900 | 1.484770 | 1.377827 | 1.281292 |
| 14,130 | 1.244176 | 0.636065 | 0.592546 |
| 14,310 | 0.293679 | 0.292456 | 0.292456 |
| 14,490 | 1.292878 | 0.970204 | 0.913421 |

Both repaired-font captures pass three of four strict checks and retain the
retail laser/VM packet hash. Frame 12,900 still exceeds 1.0. The previous
49-pixel horizontal edge needs vertex/sampling evidence; text rasterization
also has remaining differences. The asset-free pixel probe captures 27 text
images at seven positive and two negative heights, including two recorded
Stage 3 titles. At height 28, the sword title's bounds change from
`[6, 11, 319, 37]` to `[5, 8, 246, 28]`; Wine's bounds are
`[6, 8, 235, 27]`. Remaining widths and grayscale levels differ, so the probe
reports measurements rather than a pixel-equality pass.

To reproduce the actual-game parameter/font capture:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case stage3-keine --watch-text-fonts --retail \
  --font /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf \
  --output-dir build/laser-font-paired
```

The portable font override affects only portable. Check the original game's
font-selection log before interpreting this as a same-font comparison on
another host. Repeat the adapter pixel probe with the pre-repair failure
control:

```bash
python3 scripts/analysis/probe-font-pixels.py \
  --font /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf \
  --baseline-ref 5b07fd8a --output-dir build/font-pixels
```

The probe needs MinGW's i686 compiler, Wine32, Pillow, and the Linux build
libraries. It converts PPM evidence to verified lossless PNGs and removes the
temporary Wine prefix and probe executables. It records the actual Wine face,
font file hash, locale, compilers, adapter/source hashes, and image identities.

Local evidence is under `build/laser-font-20261014/`: the original-game font
trace is in `stage3-droid-paired/retail/`; repaired captures are in
`stage3-cell-height-default/` and `stage3-cell-height-droid/`. API images and
old-source controls are in `final-font-pixels/`. The tested x86_64 recorder
has SHA-256 `75f785111922b27bedb71a2d8142a23b71f6c98cd0ac854a47d5b978539fc566`,
retained with source hashes and build configuration in `cell-height-build/`.
The completed compiler tree was removed. Fresh default-font Stage 4B captures
pass all four checks with the color-repair errors unchanged. The default-font
image gate now passes seven of eight frames. The complete `lunatic-0-b`
recording passes 113,221 calculations on all 25 fields with a natural exit;
its evidence is in `x86_64-suite/`. These results cover the selected x86_64
product; refreshed i386 images and later-boss cases remain to be exercised.
Retained-evidence verification, four renderer contract groups, 297 color
samples, and all 196,608 surface copies also pass with the height repair.

## Read and verify the evidence

`comparison.json` records the executable and replay identities, raw laser-state
differences, image hashes, beam-geometry pixel counts, and mean RGB errors.
Images and state packets remain in the output directory; retail evidence is
under `retail/`. Recompute the comparison:

```bash
python3 scripts/analysis/compare-laser-visuals.py \
  --reference build/laser-stage3/retail \
  --candidate build/laser-stage3 \
  --output build/laser-stage3/rechecked.json
```

The check requires identical active slots and captured raw fields, including
IEEE-754 bits. It compares RGB pixels inside the retail beam quads, clipped to
the playfield, with a mean error limit of 1.0 on the 0–255 channel scale. A
reference frame needs at least 50 beam-geometry pixels. Full-playfield error
is reported separately; the pass limit applies to beam geometry. Cap VM state
is captured, while the pixel mask covers laser bodies.

Both observers use `GameManager.gameplayFrameCounter` after Present. The
separate `stageActiveFrames` statistic excludes dialogue and can identify a
different scene with the same number. An initial exploratory capture used
that statistic on portable and was discarded as a visual comparison.

Retail reads the laser pool at `g_BulletManager + 0x660938`, stride `0x59C`,
attested by `BulletManager::OnDraw @ 0x00432B50`. VM dimensions and rotation
follow `AnmManager::Draw2D @ 0x00463210`; signed scales determine anchor shifts.
`DrawInner @ 0x004628B0` selects the diffuse color through VM flag 17. The mask
excludes zero-alpha selections. The post-Present hardware breakpoint
at `0x00442082` checks its bytes against the canonical executable before use.
Retail gameplay memory and executable bytes remain unmodified. Portable
layout offsets come from the candidate's GDB types, supporting both Linux
pointer layouts. Metadata identifies the actual executable; its source
revision and build provenance should be retained alongside it.

## Check detection of missing beams

Run an isolated control using the same executable and recorded frames:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case stage3-keine \
  --suppress-laser-bodies \
  --output-dir build/laser-stage3-control
```

Compare it against the preceding normal capture:

```bash
python3 scripts/analysis/compare-laser-visuals.py \
  --reference build/laser-stage3/retail \
  --candidate build/laser-stage3-control \
  --normal-candidate build/laser-stage3 \
  --expect-missing \
  --output build/laser-stage3-control/comparison.json
```

The control
omits beam quad submissions through GDB while keeping laser calculations and
VM preparation. `control.json` counts suppressed submissions. With
`--expect-missing`, exit 0 requires `negativeControl.detected: true`: raw state
agrees, beam-region error rises by at least 10.0 and reaches at least five
times the normal capture's error in every frame. Normal capture differences
alone cannot pass this control. The ordinary strict pixel comparison remains
a separate result. Unrelated failures and a control that suppresses no
submissions fail validation.

The October 13 control suppresses 170 beam submissions across the four frames
and preserves the identical raw laser-state packet. Errors rise from
2.831371, 2.609911, 1.324700, and 2.085793 to 93.506351, 93.319568, 90.584128,
and 75.029646. All four omission checks pass. Evidence is retained under
`build/laser-visual-20261013/stage3-control/`; the normal strict pixel gate
continues to report the smaller differences listed above.

The target-independent checks exercise missing-beam detection and reject wrong
executable/archive identities, frame counters, incomplete or nonrasterized
captures, empty coverage, and absent on-screen geometry:

```bash
python3 scripts/test-laser-visuals.py
```

## Find additional laser scenes

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 4 --scout \
  --output-dir build/laser-scout-stage4b
```

Scout mode samples active laser state every 30 presented gameplay frames with
rasterization disabled. `scout-summary.json` records the sampled range and
stop reason; `lasers.json` retains samples containing active slots. Choose
frames with substantial on-screen geometry, then capture those exact frames
with rasterization and retail comparison. Sampling can miss shorter events.
New cases should identify the boss, phase, credited fixture, stage, and frame,
and retain paired state and image evidence before entering the manifest.
