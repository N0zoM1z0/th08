# Laser visual regression

Compare the original Japanese TH08 1.00d and Linux portable images at selected
laser frames. The [replay results](REPLAY_RESULTS.md) establish gameplay trace
equality; this check adds laser-pool state and images from the presented frame.

## Current result, October 15, 2026

All eight selected x86_64 frames pass the original 1.0 beam-region mean RGB
error limit, with identical raw laser/VM packets. The seven baseline failures
led to repairs in packed-color conversion, GDI font-height mapping, and
horizontal edge coverage. The [edge regression](#horizontal-edge-regression-october-14-2026)
records the edge repair; the [font controls](#font-rasterization-controls-october-14-2026)
record the latest tested x86_64 product.

The [surface-filter repair](#production-surface-filter-repair) now passes 640
pinned-D3DX8 API cases and reduces the matched-font Stage 3 title error. Small
font and synthetic texture-sampling differences remain measured below.
The [i386 refresh](#i386-refresh-october-14-2026) passes the same selected frames
in both context profiles. The [Final A investigation](#final-a-observation-october-14-2026)
finds a split spell-background script field; its owner repair passes the five
added x86_64 image cases with identical laser/VM packets. The surface-filter
product also passes all 108 recordings and retained-evidence verification.
The added [Final B samples](#final-b-precision-observation-october-14-2026)
exposed a one-ULP laser-origin difference at frame 8,400. The ECL sine/cosine
repair now passes all seven raw packets and 390 paired spawn-input records.
Its complete 108-recording replay refresh and retained-evidence pass succeed.
The new i386 product passes its API,
renderer and Final B checks. Its full 108-recording gate and separate
retained-evidence pass now succeed. Its scene refresh also passes all five
registered cases in default and OpenGL 2.1 contexts, with 24 selected frames
per context and exact raw laser/VM packets.
The [Extra case](#extra-laser-observation-october-15-2026) now passes four paired
images with exact raw packets. The dated sections preserve each repair's evidence.
The [October 15 sampling candidate](#production-candidate-october-15-2026)
passes its API checks and all 48 selected game images across both context
profiles. Its partial corpus is retained after retirement in favor of the
newer D3DX candidate.
The [sincos follow-up](#sincos-candidate-seven-frame-verification) repairs
laser rotation rounding. The subsequent
[D3DX repair](#orbit-particles-d3dx-rounding) resolves all 23 remaining pixels
below the Final B title band: every RGB pixel in that region matches retail
at all seven selected frames. Both production architectures pass 264
original-library arithmetic cases. Their full scene and replay gates remain
in flight; the completed 108-recording results above describe earlier products.

### Status of the seven initial failures

The seven failures belong to the October 13 baseline. A retained-image
revalidation against the original 1.0 limit gives the following results for
the surface-filter-repaired x86_64 recorder, SHA-256
`7eb846a665867ed9e8a1c19484d2c0fe1ee586954f0ecca3b1b756223e0e0853`.
All raw laser and VM packets still agree exactly with retail.

| Stage | Frame | Baseline mean RGB error | Surface-filter checkpoint error | Checkpoint result |
| --- | ---: | ---: | ---: | --- |
| 3 | 12,900 | 2.831371 | 0.591680 | Pass |
| 3 | 14,130 | 2.609911 | 0.263236 | Pass |
| 3 | 14,310 | 1.324700 | 0.003113 | Pass |
| 3 | 14,490 | 2.085793 | 0.544644 | Pass |
| 4B | 13,140 | 3.084492 | 0.004278 | Pass |
| 4B | 13,260 | 2.222010 | 0.001907 | Pass |
| 4B | 18,210 | 1.657255 | 0.021046 | Pass |

Stage 4B frame 15,360 is the eighth sample; it passed both builds at 0.003865.
The original Stage 4B failures came from packed-color conversion. Stage 3
also exposed font metrics/rasterization and the horizontal beam edge.
Image results throughout this report use Pass/Fail for the beam-region
mean RGB error limit of 1.0; exact raw-state comparisons are reported separately.
Passing the image limit allows small pixel differences. The table uses the
portable default font. With the same Droid font as retail, Stage 3's four
errors are 0.003292, 0.017577, 0.003113 and 0.018465. The
[surface-filter repair](#production-surface-filter-repair) records the
before/after title measurements. The early failures have identified causes
and passing regressions; remaining font and sampling residuals have separate
controls below. Reports are under `build/surface-filter-repair-20261014/`.

## Capture a recorded case

Build the x86_64 Debug product, supply the original executable and archives, and
run from the repository root:

```bash
scripts/build-portable-linux.sh x86_64
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case stage3-keine --wine-csmt 0 --retail \
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

## Run all registered scene cases

This Bash loop captures each retail case once and checks both portable context
profiles against it. It runs one game at a time, stops at the first failure,
and writes a pixel-region summary for each pair. Use a fresh output root.
The five cases cover 24 selected frames per portable profile. They complement
the [full gameplay replay corpus](PORTABLE_REPLAY_TESTING.md); selected image
checks and gameplay-state checks have different coverage.

```bash
set -e
th08_candidate=build/portable-linux-x86_64/th08-modern
th08_game_data="/path/to/original/TH08 directory"
th08_scene_root=build/laser-scene-matrix
test ! -e "$th08_scene_root"
for scene in stage3-keine stage4b-marisa finala-eirin finalb-kaguya extra-mokou; do
  python3 scripts/analysis/check-laser-visuals.py \
    --candidate "$th08_candidate" --game-data-dir "$th08_game_data" \
    --case "$scene" --wine-csmt 0 --retail --output-dir "$th08_scene_root/default/$scene"
  python3 scripts/analysis/check-laser-visuals.py \
    --candidate "$th08_candidate" --game-data-dir "$th08_game_data" \
    --case "$scene" --legacy-gl --output-dir "$th08_scene_root/legacy/$scene"
  python3 scripts/analysis/compare-laser-visuals.py \
    --reference "$th08_scene_root/default/$scene/retail" \
    --candidate "$th08_scene_root/legacy/$scene" \
    --output "$th08_scene_root/legacy/$scene/comparison.json"
  for profile in default legacy; do
    python3 scripts/analysis/summarize-laser-pixels.py \
      --reference "$th08_scene_root/default/$scene/retail" \
      --candidate "$th08_scene_root/$profile/$scene" \
      --output "$th08_scene_root/$profile/$scene/pixel-summary.json"
  done
done
```

Completed captures can be rechecked with the comparator and summary commands
without launching a game. Preserve their metadata and original hashes when
moving evidence; keep generated captures under `build/`.

## Stage 3 observation, October 13, 2026

All captured laser fields agree exactly in the four paired frames. The active
slot counts are 50, 20, 50, and 50; body and cap VM packets also agree. Retail
and portable `lasers.json` share SHA-256
`be8ce47843f9e3d55141c4de2279e2d80beb63393cf16b8c5fa30a14ad74c8d9`,
retained in the case manifest. A separate direct-stage portable diagnostic
matches the published retail reference for 14,500 calculations on all 25
fields. Its first 14,490 rows also match the fresh ordinary-paced retail
capture.

| Frame | Beam-geometry pixels | Beam-region mean RGB error | Image result |
| ---: | ---: | ---: | --- |
| 12,900 | 10,024 | 2.831371 | Fail |
| 14,130 | 3,148 | 2.609911 | Fail |
| 14,310 | 50,966 | 1.324700 | Fail |
| 14,490 | 46,628 | 2.085793 | Fail |

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

| Frame | Active slots | Beam-geometry pixels | Beam-region mean RGB error | Image result |
| ---: | ---: | ---: | ---: | --- |
| 13,140 | 5 | 935 | 3.084492 | Fail |
| 13,260 | 10 | 3,671 | 2.222010 | Fail |
| 15,360 | 10 | 1,035 | 0.003865 | Pass |
| 18,210 | 4 | 10,200 | 1.657255 | Fail |

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

| Stage | Frame | Baseline beam error | Repaired beam error | Repaired image result |
| --- | ---: | ---: | ---: | --- |
| 3 | 12,900 | 2.831371 | 1.484770 | Fail |
| 3 | 14,130 | 2.609911 | 1.244176 | Fail |
| 3 | 14,310 | 1.324700 | 0.293679 | Pass |
| 3 | 14,490 | 2.085793 | 1.292878 | Fail |
| 4B | 13,140 | 3.084492 | 0.004278 | Pass |
| 4B | 13,260 | 2.222010 | 0.001907 | Pass |
| 4B | 15,360 | 0.003865 | 0.003865 | Pass |
| 4B | 18,210 | 1.657255 | 0.021046 | Pass |

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
`(201, 236)`–`(247, 244)`. Its outside-header mean error is 0.650546. The
[subsequent vertex/origin investigation](#horizontal-edge-regression-october-14-2026)
localizes and repairs this edge residual.

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
49-pixel horizontal edge is investigated and repaired below; text rasterization
also has remaining differences. The asset-free pixel probe captures 27 text
images at seven positive and two negative heights, including two recorded
Stage 3 titles. At height 28, the first Japanese probe text (index 1) changes from
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

### Font rasterization controls, October 14, 2026

An offline SDL_ttf control reproduces all 27 retained production font images
pixel for pixel before varying hinting, kerning, or bold. It uses the same
Droid Sans Fallback file, font-height conversion, text and placement as the
paired Wine probe. Image and font hashes are checked before rendering.

| Control | Mean RGB error across 27 images | Improved / worsened images |
| --- | ---: | ---: |
| Production light hinting and bold | 2.812630 | Baseline |
| Normal hinting and bold | 2.737256 | 25 / 1 |
| Normal hinting, kerning disabled | 2.737256 | 25 / 1 |
| Light hinting, bold disabled | 3.316637 | 7 / 20 |

Normal hinting improves the average by 2.7%; the title remains too wide.
Kerning changes none of these pixels. At GDI height 28, probe text 1
spans 242 pixels in the production probe and 230 in Wine. Its left
edges differ by one pixel; the right edges differ by 11. The production image
has 207 nonzero gray levels, while Wine has 15.

The [SDL_ttf 2.22.0 source](https://github.com/libsdl-org/SDL_ttf/blob/release-2.22.0/SDL_ttf.c)
adds `ppem / 10` pixels to each synthetic-bold advance and expands the bitmap
horizontally. At 21 ppem this adds two pixels.
[Wine 9.0](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/win32u/freetype.c)
adds one pixel to the synthetic-bold advance and emboldens the outline with
strength `ppem * 64 / 24`. It also supplies a 16-step grayscale bitmap for
`GGO_GRAY4_BITMAP`. These adapter differences identify the next controls:
glyph advance, outline emboldening, and grayscale transfer. The measured
hinting change alone leaves most of the residual error.

Reproduce the controls after the paired font pixel probe above:

```bash
python3 scripts/analysis/analyze-font-rasterization.py \
  --capture build/font-pixels \
  --output-dir build/font-rasterization-controls
```

The analyzer needs Python Pillow and FontTools, plus the SDL2 and SDL2_ttf
shared libraries. It runs without a compiler, display server, Wine process or
game assets. Its baseline must reproduce all 27 production images before
the controls are accepted. Bounds use exclusive right and bottom coordinates;
the original C++ probe records inclusive endpoints. The report retains library
version, input hashes and all control images. Local evidence is under
`build/font-residual-audit-20261014/repro-hinting/`; no gameplay or renderer
source changes accompany this measurement.

The direct FreeType outline control adds the measured Wine advance,
emboldening and grayscale rules. Using system FreeType 2.13.2, it reproduces
26 of 27 Wine images pixel for pixel. All 27 visible-pixel masks agree. The
remaining image is the height-28 probe text 1: at `(208, 22)`, the control draws
gray 217 and Wine draws 208. Its mean error is 0.000122; the other 26 images
have zero error. The API comparison below localizes the last difference.
The control's raw coverage is 166, which quantizes to level 11. Wine's gray
208 is level 10, corresponding to raw coverage 151–165 under the same rule.
The i386 API control below reads coverage 165 at the corresponding glyph pixel.

The control rounds the original glyph advance upward after adding one pixel,
emboldens the outline by `ppem * 64 / 24`, and converts coverage with
`coverage * 17 / 256`. The Wine DIB painter skips levels zero and one and
uses its documented 17-entry intensity ramp. Source references are
[FreeType glyph handling](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/win32u/freetype.c),
[the DIB intensity ramp](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/win32u/dibdrv/graphics.c)
and [DIB glyph painting](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/win32u/dibdrv/primitives.c).
The analyzer retains glyph indices and advances, exact differing-pixel counts,
bounds, and maximum channel errors. The English label in this same-font probe
renders missing-glyph boxes in both backends; the two Japanese titles contain
real glyphs.

Install `freetype-py==2.5.1` in the Python environment to enable this control:

```bash
python3 scripts/analysis/analyze-font-rasterization.py \
  --capture build/font-pixels --outline-control \
  --output-dir build/font-outline-controls
```

These measurements cover white text on a black 32-bit bitmap at nine font
heights. General color blending, overlapping glyphs, and actual game images
are the next adapter checks. Local evidence is under
`build/font-residual-audit-20261014/repro-outline-final-v2/`. The controls leave
production rendering unchanged.

### FreeType architecture control, October 14, 2026

The last font pixel is reproduced independently through system FreeType APIs.
Both distro libraries are version `2.13.2+dfsg-1ubuntu0.2`. With the same font,
glyph U+90F7 (index 24,207), 21 ppem and bold strength 56 in 26.6 units,
the i386 and x86_64 captures have identical outline points, tags and contours
before and after emboldening. Their raw grayscale bitmaps differ at four
pixels, each by one coverage unit:

| Glyph-local pixel | i386 coverage | x86_64 coverage |
| --- | ---: | ---: |
| `(7, 13)` | 52 | 53 |
| `(6, 14)` | 165 | 166 |
| `(5, 15)` | 252 | 253 |
| `(5, 16)` | 148 | 149 |

Only `(6,14)` crosses a `coverage * 17 / 256` boundary: level 10 versus 11.
The Wine intensity ramp maps these to 208 and 217, exactly the retained
image difference at `(208,22)`. This control localizes that residual to
rasterization in the two FreeType builds, after equal outline generation.
The 27-image control records 26 exact images. The production repair below
carries the measured GDI rules into the adapter and checks game scenes.

Reproduce on Linux x86_64 with GDB, binutils, `libc6:i386`,
`libfreetype6:i386`, and Python `freetype-py`:

```bash
python3 scripts/analysis/probe-freetype-architecture.py \
  --font /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf \
  --output-dir build/freetype-architecture
```

The i386 observer uses a short-lived system libc process to call public
FreeType APIs. It runs without Wine, a compiler or game assets. The report
pins both FreeType files, libc, font, observer and analyzer hashes, and retains
full outlines and coverage bytes. Coordinates are relative to the glyph bitmap.
Local evidence is under `build/font-residual-audit-20261014/repro-architecture-final/`.

### Production font rendering repair, October 14, 2026

Scalable Linux fonts now use FreeType outline rendering with GDI's glyph
advances, synthetic bold strength and 17-level intensity ranges. Font selection
and the existing Windows-metric height conversion are retained. Bitmap fonts
keep the SDL_ttf path. Native VC7 source and layouts are unchanged.

Fresh paired API captures reproduce 26 of 27 Wine text images exactly; the
remaining pixel is the independently measured FreeType coverage difference
above. A second probe exercises colored backgrounds, black and colored text,
negative coordinates, overlapping draws, and unused bitmap bits in RGB555 and
32-bit DIBs. All 96 regular-weight cases are byte-identical. Of 96 semibold
cases, 11 are byte-identical; the other 85 differ only at measured coverage
locations and within bounds derived from both directions of the intensity ramp.
Unused bits must still agree, and every other pixel is compared exactly.

The first RGB555 repetition exposed 2,712 additional pixel differences. GDI
quantizes both the pen color and destination to the DIB format before blending;
five-bit channels expand by bit replication. Restoring those steps removes
all additional differences. The pre-repair `380dc74b` controls retain 17,682
unexplained semibold pixels and 20,639 regular pixels. This probe measures API
behavior independently of the renderer's image threshold.

The new x86_64 Debug recorder is
`4cce0fe8ee633fa2b37713214793fef078395edbb303f52c0cdd98e4bc914cc6`.
Its four default-font Stage 3 images and four Stage 4B images pass the unchanged
1.0 beam-region limit with the pinned retail laser/VM hashes. Matched-Droid
Stage 3 images also pass:

| Stage 3 frame | Default VL Gothic | Matched Droid |
| ---: | ---: | ---: |
| 12,900 | 0.694699 | 0.350891 |
| 14,130 | 0.317662 | 0.184668 |
| 14,310 | 0.003113 | 0.003113 |
| 14,490 | 0.711447 | 0.320516 |

Values are beam-region mean RGB errors. The matched-font game captures retain
image residuals that need further localization. Stage 4B errors remain
0.004278, 0.001907, 0.003865 and 0.021046.
The complete `lunatic-0-b` recording passes 113,221 calculations on all 25
fields, recorded end scores and natural exit; retained-evidence verification
also passes. All six renderer contract groups pass. The complete corpus,
i386 product and later-boss images remain separate refreshes for this binary.

Reproduce the blending gate after the architecture control above:

```bash
python3 scripts/analysis/probe-font-blending.py \
  --font /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf \
  --coverage-reference build/freetype-architecture \
  --baseline-ref 380dc74b --output-dir build/font-blending
python3 scripts/analysis/probe-freetype-architecture.py \
  --font /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf \
  --strength 0 --output-dir build/freetype-regular
python3 scripts/analysis/probe-font-blending.py \
  --font /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf \
  --weight regular --coverage-reference build/freetype-regular \
  --baseline-ref 380dc74b --output-dir build/font-blending-regular
```

The blending probe needs MinGW, the Linux build dependencies, Wine32 and Xvfb.
Both captures run muted and serially. It removes temporary executables and
its owned Wine prefix, compresses raw bitmaps, and records all pixel differences
and source hashes. Evidence is under `build/font-production-20261014/`.

### Remaining title texture scaling

The matched-font residual covers more than the beam mask alone. At frame
12,900, the complete header has 1,081 differing pixels and mean RGB error
1.501320. Within the beam mask, 99.1% of the remaining error lies in the header;
outside it the mean is 0.003603. The title's nine distinct glyphs were tested
through both system FreeType builds. Eight have identical outlines and coverage;
U+90F7 retains only the four one-unit coverage differences documented above.

Source inspection identifies the next scaling gate. The game text helper
copies a double-size bitmap into a half-size texture with filter `4`.
The canonical caller pushes `4` at `0x0043FC9E` and calls the linked surface
copy at `0x0043FCB5`.
Portable `D3DXLoadSurfaceFromSurface` discards the filter argument, and
`CopyRgbaToSurface` chooses pixels with integer nearest-neighbor coordinates.
The earlier 144-copy and exhaustive-copy probes test equal dimensions; they
provide no scaled-filter evidence.

Canonical target instructions at `D3DXLoadSurfaceFromSurface @ 0x0047ABAB`
check equal rectangle dimensions at `0x0047AC97` and `0x0047ACA4`. The fallback
forwards the filter at `0x0047AD19` to `FUN_0047a586`, which forwards it at
`0x0047A615` to `FUN_00480f33`; that dispatcher masks and validates filter
values 1–5. These are target observations, not a recovered filtering algorithm.
The configured VC7 prerelease D3DX8 archive was re-attested as SHA-256
`0d4a2b642485dcaa7671926a9a1a545c656d5eb73f160fe971b3deebf0b516b5`.
The [Microsoft filter reference](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dx-filter)
describes distinct point, linear, triangle and box filters; it supplies adjacent
API terminology rather than TH08 D3DX8 pixel evidence.

An independent API control now confirms the portable filter defect. It builds
with VC7 and links the hash-pinned prerelease D3DX8 archive explicitly, then
runs the same C++ probe against the portable backend. It tests A1R5G5B5 and
A8R8G8B8 surfaces, four patterns, equal-size copies, half-size copies,
double-size copies, unequal ratios, and filters 1–5: 160 cases. Two independent
runs produce identical reference and portable pixel records.

All 40 equal-size copies and all 24 scaled point-filter cases match. Across
the complete probe, 72/160 cases match. The D3DX8 output varies with filter in
24 of 32 pattern/size/format groups; portable varies in none. All eight
half-size filter-4 cases differ, changing 101 of 128 output pixel words.
For a black/white checkerboard, the A8R8G8B8 reference produces `0x80808080`,
while portable produces `0x00000000`. The ignored argument therefore has a
measured effect on scaled surfaces.

These are pinned-archive API observations. The wrapper sections at
`0x0047ABAB` and `0x0047A586` have matching extents and no differences outside
relocation fields; this partial comparison adds no accepted exact units.
The API control runs outside the retail game. The production repair below
applies these rules and measures their contribution in ordinary game images.
The original 1.0 game-image limit remains unchanged.

Reproduce the current failure control with the configured private VC7 toolchain
and D3DX8 archive, Wine32, Xvfb, and the Linux backend build dependencies:

```bash
python3 scripts/analysis/probe-surface-scaling.py \
  --baseline-ref b5608f80 --expect-filter-failure \
  --output-dir build/surface-scaling-control
```

The runner checks the target and archive hashes, records source, probe,
compiler, executable and linker-map hashes, and removes its owned Wine prefix
and compiled products. It retains pixel records and compressed logs under
`build/`. After a filter repair, `--check` requires all 160 API cases to agree.
Current API evidence is under `build/surface-scaling-20261014/repro-final/`.
Earlier title evidence is under
`build/font-production-20261014/title-glyph-controls/` and
`surface-filter-observation.json` in the same phase directory.

#### Production surface-filter repair

The Linux D3DX adapter now honors none, point, linear, triangle and box filters.
Triangle filtering integrates each source texel's footprint in destination
space. Linear and triangle edges wrap by default and follow the measured edge
flags. The half-size box path averages packed channel values; cross-format
copies use normalized colors. Default filtering applies the canonical 4x4
rounding matrix. File and texture loaders forward their filter argument too.

Read-only hardware observations inside the pinned-archive probe confirm the
single-precision color buffers and the encoders' temporary truncation mode.
The adapter restores the caller's rounding mode after encoding. The 640-case
extension covers both pixel formats in each conversion direction, four
patterns, five size/rectangle selections, filters 1–5, default filtering and
edge flags. Fresh system-compiler and GCC 8 checks match every output word and
preserve rounding state in every case. The immutable `b5608f80` backend still
reproduces the original 72/160 result and ignored-filter failure control.
The existing 297 color/blend samples, 144 raw copies, 196,608 exhaustive copies
and six renderer groups pass.

Reproduce the repair check:

```bash
python3 scripts/analysis/probe-surface-scaling.py --extended --check \
  --compiler g++-8 --output-dir build/surface-filter-check
```

Add `--observe-codecs` to retain source and encoded color buffers and x87
control words. The observer uses hardware breakpoints, attests the mapped
instructions, and enforces an 85-second deadline and 32 MiB color-data limit.
`--compiler` and `--pkg-config` also accept bounded architecture wrappers.

The fresh x86_64 Debug recorder is SHA-256
`7eb846a665867ed9e8a1c19484d2c0fe1ee586954f0ecca3b1b756223e0e0853`,
built with GCC 8.4.0 and one compiler job. Build identity and evidence are under
`build/surface-filter-repair-20261014/`. With the same Droid font as retail,
all four Stage 3 images pass and retain exact laser/VM packets. The complete
header rectangle `(32,16)`–`(416,64)` gives:

| Frame | Previous differing pixels | Repaired differing pixels | Previous mean RGB error | Repaired mean RGB error |
| ---: | ---: | ---: | ---: | ---: |
| 12,900 | 1,081 | 3 | 1.501320 | 0.000326 |
| 14,130 | 787 | 2 | 0.953197 | 0.000217 |
| 14,310 | 807 | 13 | 0.987431 | 0.001863 |
| 14,490 | 1,924 | 1,140 | 1.168529 | 0.034216 |

The comparator now reports header pixel counts, means and maximum channel
differences alongside the unchanged beam-region gate. Run it against the
previous and repaired captures to reproduce this table. The before/after
figure is `stage3-title-before-after.png` in the phase directory. Stage 4B's
four beam errors remain 0.004278, 0.001907, 0.003865 and 0.021046, with exact
laser/VM packets. Default-font Stage 3 also passes all four original limits,
with errors 0.591680, 0.263236, 0.003113 and 0.544644. Final A's five frames
pass at 0.015086, 0.151220, 0.076219, 0.812111 and 0.003248, with the
same pinned retail laser/VM packet. Four beam results are unchanged; frame
15,090 improves from 0.862803 to 0.812111. These thirteen default-font frames
and four matched-font frames are ordinary captures. The complete `lunatic-0-b`
and `lunatic-1-a` recordings pass 232,906 calculations on all 25 fields, end
scores and natural exits; retained-evidence verification passes both. The
108-recording refresh and retained-evidence verification both pass: 12,311,460
calculations agree on all 25 fields, stage-end scores, fingerprints and natural
exits. Evidence is under `x64-suite/` in the phase directory.
i386 refresh remains to be done. The compiled source is pinned to
checkpoint `183cd755`; the build provenance verifies all compiled input hashes
against its LF-normalized Git blobs.

## Horizontal-edge regression, October 14, 2026

The selected Stage 3 frame has 16 thin horizontal sprite quads crossing the
edge region `(201, 236)`–`(248, 245)`. All 448 vertex words agree between
retail and portable: position, reciprocal W, diffuse color, and UV. The
complete observed submission counts are 316 and 490; this equality claim
covers the selected 16 quads, not the entire sprite stream.

An independent D3D8 API probe draws those inputs with solid and synthetic
striped materials, separately and together, through two viewports. The
baseline differs in all 68 cropped images. Its solid rectangles omit row 236
and include row 244, while Wine D3D8 includes the top row and excludes the
bottom row. This localizes the edge to rasterization rather than the observed
input coordinates. A private viewport-only experiment leaves all 34 solid
cases different; an upper-left clip-origin experiment matches all 34.

At that checkpoint the Linux renderer selected the upper-left clip origin when
`GL_ARB_clip_control` was available. Its projection retained the existing image
orientation. Presentation and CPU backbuffer uploads use their lower-left
blit convention explicitly. The first repair kept the older path for contexts
without clip control; the [legacy gate below](#legacy-opengl-coverage-october-14-2026)
covers its follow-up repair. See the
[Direct3D filling convention](https://learn.microsoft.com/en-us/windows/win32/direct3d9/rasterization-rules)
for terminology and [ARB_clip_control](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_clip_control.txt)
for the OpenGL origin mapping. The measured oracle is Wine D3D8.

| Stage 3 frame | Before origin repair | After origin repair | Image result |
| ---: | ---: | ---: | --- |
| 12,900 | 1.377827 | 0.786645 | Pass |
| 14,130 | 0.636065 | 0.324227 | Pass |
| 14,310 | 0.292456 | 0.003113 | Pass |
| 14,490 | 0.970204 | 0.766607 | Pass |

All four Stage 4B errors remain unchanged and pass. The selected eight-frame
gate therefore passes with the original 1.0 limit and exact raw laser/VM
packets. At frame 12,900 the beam-mask mean outside the header falls from
0.650546 to 0.003603; its 49 pixels above 32 disappear. The independent probe
passes 53 of 68 exact image comparisons, including every solid case. Fifteen
striped-material images retain sampling differences; title font rasterization
also remains distinct. These are separate from the repaired coverage defect.

Capture the observed vertices without rewriting original gameplay memory:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 2 --frames 12900 --watch-quads --wine-csmt 0 --retail \
  --output-dir build/laser-quads
python3 scripts/analysis/compare-sprite-quads.py \
  --reference build/laser-quads/retail --candidate build/laser-quads \
  --output build/laser-quads/quad-comparison.json
python3 scripts/analysis/probe-subpixel-quads.py \
  --capture build/laser-quads --baseline-ref 8702c8ef --check-coverage \
  --output-dir build/subpixel-api
```

The retail hardware breakpoint at `AddSpriteToDrawBuffer @ 0x00462F10`
attests its instructions and reads the four 28-byte vertices. It is armed only
for selected frames; the portable observer uses Debug types and the same
word layout. Region selection retains every thin horizontal quad crossing
the ROI in submission order. Missing coverage and malformed/nonfinite vertex
data fail validation. Font and omission observers run in separate captures.

The asset-free renderer gate now checks top/bottom and left/right pixel-center
coverage, an offset viewport, and preservation through Present:

```bash
python3 scripts/test-portable-renderer.py --cases edges
python3 scripts/test-portable-renderer.py --cases edges \
  --baseline-ref 8702c8ef --expect-failures
```

The pre-repair renderer fails the named top-edge assertion. Local evidence is
under `build/laser-vertices-20261014/`: `stage3-12900-paired/` contains the
observed inputs; `repro-subpixel-api/` contains synthetic images and the
old-source control. Repaired game frames are in `stage3-origin/` and
`stage4b-origin/`. The tested x86_64 recorder has SHA-256
`f0138506b35e9849a2877ee6a8ebe6df66c933534b1375b40c35944af2d653d6`,
retained with source/build identities in `production-origin/`; its compiler
tree was removed. The complete `lunatic-0-b` recording passes 113,221
calculations and all 25 fields with a natural replay exit. Retained-evidence
verification and target-independent CI also pass. The refreshed whole corpus
and i386 images remain separate gates.
The renderer's five contract groups and all 297 texture-color samples and
196,608 surface copies pass after the origin change.

### Remaining sampling and title error

Rechecking the retained game frames places 99.6%, 94.6%, and 98.0% of the
remaining Stage 3 beam-mask error inside the title band at frames 12,900,
14,130, and 14,490. The band is `(32, 16)`–`(416, 64)`, with exclusive right
and bottom bounds. Outside it, the four frame means are 0.003603, 0.018719,
0.002726, and 0.017620. One pixel exceeds mean RGB error 32, at `(35, 197)`
in frame 14,130. Stage 4B's four beam masks avoid the title band; all their
means remain below 0.022. These are spatial measurements; the font API and
original-game text captures above supply the font evidence.

All 15 remaining striped-material probe differences lie on the first covered
row, at game Y=236. A private experiment maps OpenGL to the actual D3D
viewport as well as using the upper-left clip origin. It retains all 34 solid
passes and 15 textured differences, so viewport mapping alone does not resolve
the sampling discrepancy.

The reusable synthetic control snaps only X/Y to a 1/256-pixel grid in both
APIs. It changes 109 words by at most 0.0001220703125 pixels and preserves
depth, reciprocal W, color, and UV. With these inputs, all 34 solid images and
all 17 full-viewport textured images match exactly. Nine offset-viewport
textured images still differ on the first row. Wine's textured output changes
between the two viewports in exactly those nine images; portable's does not.
Applying the private viewport mapping to this control leaves the same result.
This narrows the remaining discrepancy to boundary-sensitive texture sampling;
the interpolation precision responsible for it still needs measurement.

Reproduce the grid control separately from the unmodified-input probe:

```bash
python3 scripts/analysis/probe-subpixel-quads.py \
  --capture build/laser-quads --position-grid 256 --check-coverage \
  --output-dir build/subpixel-grid-control
```

The report identifies the observed and transformed input hashes, changed-word
count, maximum coordinate change, and compiler/Wine versions. The control
leaves game source and captured vertices unchanged. Current region measurements
and the private viewport experiments are under `build/laser-sampling-20261014/`;
`repro-grid-control/` contains the reproducible Wine/portable control images;
`repro-observed-inputs/` repeats the unmodified 53/68 result with the same runner.

### U-gradient control, October 15, 2026

A fresh probe on the merged renderer reproduces 53/68 pixel-identical API
images. All 34 solid cases agree; the 15 textured differences remain confined
to the first covered row, game Y=236. The observed input SHA-256 is
`bf4d64e15be8d087ceed0d6f090d0892bdfbcfec7ad6083d4d379fc92dddc983`.

Setting every vertex U to `33/256` produces 68/68 pixel-identical images.
This synthetic control changes 32 U words while preserving positions, depth,
RHW, diffuse color and V. The result localizes the difference to behavior
that depends on the U gradient. It leaves the observed gameplay inputs and
ordinary image gate unchanged.

```bash
python3 scripts/analysis/probe-subpixel-quads.py \
  --capture build/laser-quads --constant-u 0.12890625 --check-coverage \
  --output-dir build/subpixel-constant-u
```

Two private viewport controls move the half-pixel correction into a floating
viewport, then also use the actual D3D viewport dimensions. They pass 53/68
and 54/68 images, respectively, with all solid cases still agreeing. Their
partial results leave the textured discrepancy unresolved. The
[control patches and reproduction commands](../tests/fixtures/render-controls/README.md)
keep these experiments separate from production code. `--renderer-control`
copies the experimental source into the output directory and labels its
results separately; it also checks solid coverage for that control.

Evidence is under `build/laser-sampling-refresh-20261015/`, in `observed/`,
`constant-u/`, `viewport-control/` and `actual-viewport-control/`. Each probe
removes its owned Wine prefix and compiled executables. The next measurement
was the actual reference shader and projection arithmetic for these inputs,
recorded below.

### Measured matrix and triangle convention, October 15, 2026

The opt-in shader trace confirms a projection-matrix multiply followed by
division by RHW. A GDB hardware-breakpoint observer captures all 68 projection
calls on Wine's render thread, verifies the mapped entry against the pinned
DLL, and records two distinct matrices. Observed reference images remain
pixel-identical to the earlier unobserved capture, 68/68.

The nonzero XY entries below are raw IEEE-754 words. Both matrices have Z
scale and translation zero because the probe disables depth; their W entry
is `3f800000`.

| D3D viewport | X scale | Y scale | X translation | Y translation |
| --- | --- | --- | --- | --- |
| `(0, 0, 640, 480)` | `3b4ccccd` | `3b888889` | `bf7f999a` | `bf7f7777` |
| `(32, 16, 384, 448)` | `3baaaaab` | `3b924925` | `bf950000` | `bf88db6e` |

These values place the half-pixel correction in the projection translation,
consistent with Wine's offscreen path without clip control. The earlier
floating-viewport controls exercised a different path. A new control uses
the observed projection expressions and actual viewport dimensions:

| Control | Default context: identical images |
| --- | --- |
| Production | 53/68 |
| First-vertex convention alone | 53/68; all images unchanged from production |
| Measured projection and offscreen orientation | 60/68 |
| Same, plus first-vertex convention | 68/68 |

The combined control also passes 68/68 under forced OpenGL 2.1; production
passes 52/68 there. All solid-material images agree in both contexts. Its
vertices remain the unmodified observed words. Replacing the vertex shader,
using an explicit XY texture lookup, and adding position invariance each
leave the intermediate 60/68 result unchanged. Selecting
`GL_FIRST_VERTEX_CONVENTION` removes the remaining differences in this Mesa
environment. Wine 9.0 selects that convention in
[context initialization](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/context_gl.c#L2109).
The measurements identify two interacting API-state differences; they do not
yet identify the driver's individual interpolation instructions.

Reproduce the result with the committed
[control patches and observer commands](../tests/fixtures/render-controls/README.md#measured-projection-and-first-vertex-controls).
Evidence is under `build/laser-sampling-refresh-20261015/`, in
`measured-fragment-control/` (shader and matrix traces),
`measured-projection-control/`, `first-vertex-only-control/`,
`first-vertex-control/` and `first-vertex-legacy-control/`.
Each report pins inputs and source hashes. The observer pins its Wine DLL,
entry bytes and matrix words; it changes no target instructions.

The controls above leave production source unchanged and cover RHW=1 quads
with depth disabled and two synthetic materials. The production candidate
below extends the check to other rendering behavior and game frames.

### Production candidate, October 15, 2026

Source checkpoint `2fd6a51b` moves the transformed half-pixel correction into
the viewport projection and selects the first-vertex convention where the
OpenGL extension is supported. Both context profiles use the same offscreen
row orientation. CPU uploads, readback, clears and presentation follow it.
The production backend passes 68/68 observed-input API images in each context.

The candidate also preserves reciprocal-depth texture interpolation and checks
depth across vertex formats. Its new `depthrange` contract catches an old
ordering defect: an XYZRHW quad at Z=0.30 incorrectly covered an XYZ quad at
Z=0.25 when the viewport range was `(0.2, 0.8)`. The repaired paths agree on
that range. The contract also checks nonunit RHW, return to XYZ and disabled
depth with out-of-range Z. It passes the candidate and reproduces the old
failure at `d44868af`. Default and OpenGL 2.1 checks pass all seven groups
across locks, perspective, clipping, depth range, fog, edges and orientation;
the [renderer guide](PORTABLE_RENDERING.md#pixel-contracts-without-game-assets)
provides commands, including the failure control. Repository CI passes.

The fresh GCC 8 x86_64 Debug product is
`dcf14c0c8014073a15e9c1df7fcba45ccf5a6e9f1963cc2ff2e17c60fc4ed611`.
Its provenance pins all 132 build inputs and the clean source checkpoint.
ELF64/native-global ownership verification passes. The compiler tree and
completed contract executables have been removed.

Its first four actual Stage 3 frames pass the original image gate, with exact
laser/VM packets. Outside the title band, the remaining changed laser-mask
pixels have maximum channel error 4/255:

| Frame | Beam mean RGB error | Changed mask pixels outside title | Maximum channel error outside title |
| ---: | ---: | ---: | ---: |
| 12,900 | 0.590982 | 23 | 4 |
| 14,130 | 0.245659 | 0 | 0 |
| 14,310 | 0.000687 | 38 | 4 |
| 14,490 | 0.527065 | 38 | 4 |

The title band retains larger differences. A laser geometry mask includes
other layers rendered at the same pixels, so these region counts describe
image locations rather than an isolated laser layer. Reproduce the counts
with the summary tool; it validates the paired capture and checks that the
regions reconstruct the strict mask's pixel count and total error:

```bash
python3 scripts/analysis/summarize-laser-pixels.py \
  --reference build/laser-retail \
  --candidate build/laser-portable \
  --top-pixels 8 \
  --output build/laser-pixel-summary.json
```

`--top-pixels` retains up to 32 largest differences per region, including
coordinates and both RGB values. Ties prefer earlier image coordinates.
The default, zero, keeps aggregate measurements only.

The same product also passes all seven Final B images with exact laser/VM
packets and a separate retained comparison. Every frame's beam mean improves
over product `e0a57a43…`:

| Frame | Beam mean RGB error | Changed mask pixels outside title | Maximum channel error outside title |
| ---: | ---: | ---: | ---: |
| 8,400 | 0.002173 | 68 | 6 |
| 11,160 | 0.003768 | 19 | 4 |
| 11,490 | 0.005141 | 184 | 6 |
| 12,240 | 0.001058 | 9 | 4 |
| 14,280 | 0.023551 | 92 | 16 |
| 16,770 | 0.142039 | 25 | 4 |
| 17,460 | 0.000296 | 10 | 4 |

At frame 16,770, the outside-title mean is 0.000455; the larger total includes
title-band differences. Frame 14,280 retains the largest outside-title channel
error in this set and remains a useful localization target. The region script
reproduces these counts from `scene-default-finalb-kaguya/` and the pinned
retail capture. `finalb-pixel-summary.json` retains the strict comparison and
region measurements.

The coordinate export locates that maximum at `(168, 310)`: retail RGB
`(30, 2, 2)`, candidate `(46, 12, 12)`. Laser slot 7's geometry covers the
pixel; the screenshot also contains other rendered layers. The focused draw,
beam-omission and alpha-test controls below localize this residual.
The 92 changed pixels outside the title band have channel-error magnitudes
from 1 to 16, with only one at 16. `finalb-pixel-locations-v2.json` records the
largest examples; each exported RGB value was checked against its PNG.

A focused ordinary capture records 39 portable draw calls. Its complete
playfield and laser/VM packet match the earlier frame exactly; full-image
differences are confined to the FPS display at `(513, 464)`–`(580, 480)`.
The observer therefore preserves the image region being investigated.
A second capture omits 18 beam submissions and preserves the raw laser/VM
packet. Of the 92 changed pixels outside the title, 50 remain unchanged by
beam omission; 42 change, including 37 by at least eight channel values.
At `(168, 310)`, beam omission changes the candidate RGB from `(46, 12, 12)`
to `(41, 15, 14)`. These controls measure portable beam influence while
retaining the other rendered layers. Evidence is in `focused-finalb-draws/`,
`focused-finalb-omitted/` and `finalb-14280-attribution.json`.

Those draws use linear texture filtering. A new asset-free API probe checks
A1R5G5B5, R5G6B5, A4R4G4B4 and A8R8G8B8 with point/linear filtering,
four subpixel phases, blending disabled, ordinary alpha blending and additive
blending. Each case draws once or four times and reads 32 pixels: 192 cases,
6,144 pixel samples. All 6,144 agree with Wine D3D8 in the default context.
A private control preserving packed GPU texture formats also agrees on all
6,144 samples. This experiment gives no pixel improvement from changing the
texture format; production upload code remains unchanged.

```bash
python3 scripts/analysis/probe-texture-filtering.py \
  --check --output-dir build/texture-filtering-check
```

`report.json` pins source, compiler, Wine and binary identities and retains
every mismatched sample. `--renderer-control` labels a private source copy
separately; `--compiler`, `--pkg-config` and `--legacy-gl` select other test
environments. The recorded default run is under `texture-filtering-default/`.
Temporary Wine prefixes and probe executables are removed. The paired sprite
vertex observations below use `--watch-quads --retail`.

The first fresh paired quad capture retains equal laser/VM packets, but the
default-threading retail image differs from the earlier retail playfield.
Its beam mean is 15.124527, while the portable playfield remains identical
to its ordinary capture. Retain this failed observation under
`paired-finalb-quads/`. The post-Present screenshot can stop Wine's rendering
worker before its queued presentation completes, as the
[Final A scheduling control](#native-windows-image-capture)
demonstrated earlier. The `csmt=0` retail quad capture repeats frame 14,280.
Paired and native image capture now default to `csmt=0`, including direct
`capture_retail()` calls. The CLI accepts `--retail --wine-csmt 0` and keeps
explicit `--wine-csmt 1` for scheduling controls. Its former validation accepted
the option only with `--native-map`, rejecting the documented paired command.
The routing contracts cover both modes and default/explicit threading; the
preceding CLI fails those contracts. The raw packet and image checks remain
separate.

The synchronous capture has now completed. Its entire retail playfield is
pixel-identical to the retained ordinary retail frame, and its laser/VM
packet agrees. The beam mean returns to 0.023551. Both threading profiles
record the same 15 selected quads whose bounds cover `(168, 310)`–`(169, 311)`;
all 420 vertex words match portable exactly. Total submitted-quad counts
are 791 retail and 970 portable, so this comparison uses the explicitly
selected region rather than pairing the complete lists by ordinal.
The focused result is in `finalb-14280-quad-inputs-csmt0.json`; the failed
default-profile observation remains separate.

### Alpha-test residual at frame 14,280

The captured low-alpha draw enables `ALPHATESTENABLE`, reference 4 and
`GREATEREQUAL`, with diffuse alpha 40/255. The original 6,144-sample filtering
probe disabled alpha testing. Its new `--alpha-test` profile exercises this
state with the same formats, filters, blending modes and subpixel phases:

| Profile | Production pixels equal to Wine | Floating-point comparison control |
| --- | ---: | ---: |
| Default OpenGL | 6,128 / 6,144 | 6,144 / 6,144 |
| Forced OpenGL 2.1 | 6,128 / 6,144 | 6,144 / 6,144 |

All 16 production mismatches draw pixels that Wine discards. The maximum
channel error is 35 with blending disabled and 4 after four blended draws.
The alpha-disabled profile still passes all 6,144 samples. A second private
control rounds alpha to eight bits inside the same fragment shader; it
reproduces all production mismatches and pixel values exactly. This isolates
the comparison's rounding from the choice of fixed-function or shader texture
processing. See the [control and reproduction commands](../tests/fixtures/render-controls/README.md#floating-point-alpha-test-control).

For example, the A1R5G5B5 linear-filter case at phase 0, sample 8 interpolates
alpha to approximately 3.645833 in 0–255 units. Direct comparison rejects it
against 4; rounding first accepts it. Wine 9's
[shader alpha test](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/glsl_shader.c#L7508)
compares floating-point values. The
[OpenGL fixed-function specification, section 4.1.4](https://registry.khronos.org/OpenGL/specs/gl/glspec14.pdf)
describes rounding for the alpha test. The observed results apply to the
recorded Wine/Mesa environment.

An offline archive inventory identifies the two relevant texture hashes as
`etama.anm` entries 0 and 5, both A4R4G4B4. Their original archive payloads
match the portable CPU texture bytes exactly. This is source-derived archive
analysis, rather than a measurement of retail GPU texture allocation.
The archive SHA-256 is
`9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`.

At `(168, 310)`, captured draw 22 has five covering triangles (zero-based
indices 131, 133, 135, 137 and 139). A CPU interpolation estimate gives alpha
3.8125–3.874016 in 0–255 units: all five lie below 4 and round to 4.
A bounded GPU replay uses those captured triangle words and the original
texture over a solid `(30, 2, 2)` background:

| Regional replay | Resulting RGB at `(168, 310)` |
| --- | --- |
| Wine D3D8 | `(30, 2, 2)` |
| Production portable | `(46, 12, 12)` |
| Floating-point alpha comparison | `(30, 2, 2)` |
| Shader comparison with eight-bit rounding | `(46, 12, 12)` |

These values reproduce the actual screenshot's maximum residual exactly.
The regional experiment isolates those five triangles over the retail final
pixel; it does not replay the complete frame or establish that every residual
has the same cause. The complete-frame fix still needs a renderer supporting
all used material and alpha-test states, followed by fresh image gates.
Production renderer inputs remain frozen for the running corpus gates.

Evidence is under `build/laser-sampling-production-20261015/`:
`texture-alpha-enabled-default/`, `texture-alpha-float-control/`,
`texture-alpha-float-control-legacy/`, `texture-alpha-rounded-control/`,
`finalb-14280-point-alpha-estimate.json` and `point-alpha-replay/report.json`.
The regional report pins the captured texture, generated probe, compiler
commands, binaries and renderer source hashes. Original texture bytes were
removed after execution; no original assets are committed. The asset-free
profile and diagnostic patch are retained in the repository.

### Alpha-pipeline candidate, October 15, 2026

Implementation checkpoint `1514eaed` adds a device-owned fragment program for
alpha-tested draws. Fixed-function vertex processing continues to supply
positions, colors, texture coordinates and fog distance. Fragment processing
handles the used texture/diffuse/texture-factor materials, all eight comparison
functions and linear fog. The program is unbound after each draw and released
before its owning context is destroyed.

The GCC 8 x86_64 Debug recorder is
`4ea6bafc9570de7f8673b671c70ef14482eb2addbee237a5a732b766fb255b1c`.
The GCC 12 i386 recorder is
`e8891b50c043f27e3741b479d81e04c634b9cf8d895e3ee4de1f5fd97ad25aab`.
Both products pin the same 132 build inputs and pass their ELF/global-owner
checks. The alpha-state matrix and low-alpha filtering profile each match all
6,144 Wine samples on both architectures, in default and forced GL 2.1
contexts. The x86_64 renderer's seven existing contract groups also pass.

A fresh x86_64 Final B frame 14,280 now has zero RGB error over all 12,059
retail beam-mask pixels. All 92 prior changed mask pixels disappear, and the
maximum residual point is `(30, 2, 2)` on both products. The entire playfield
below the title band `(32, 64)`–`(416, 464)` is pixel-identical. All raw laser
and VM words agree. Title text and the HUD retain separate image differences.

Build provenance and capture evidence are under
`build/laser-alpha-repair-20261015/`: `production*/build-provenance.json`,
`finalb-frame/`, and `finalb-frame-summary.json`. The capture's added
`candidate-build-provenance.json` binds the executable to its separate source
worktree; the original metadata source commit identifies the capture tools.
Compiler trees are removed, retaining about 10 MiB for the two products.

Reproduce the actual frame after building the candidate from that branch:

```bash
cmake -S . -B build/alpha-candidate -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DTH08_ENABLE_REPLAY_TRACE=ON
cmake --build build/alpha-candidate --parallel 1
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/alpha-candidate/th08-modern \
  --game-data-dir build/modern-runtime \
  --fixture lunatic-0-b --stage 7 --frames 14280 \
  --retail --wine-csmt 0 --output-dir build/alpha-finalb-paired
python3 scripts/analysis/summarize-laser-pixels.py \
  --reference build/alpha-finalb-paired/retail \
  --candidate build/alpha-finalb-paired --top-pixels 8 \
  --output build/alpha-finalb-summary.json
```

Acceptance still requires the other selected frames and full replay gates.
The fresh capture took about 658 seconds, versus about 248 seconds for the
previous same-frame draw observation. A warmed, interleaved three-round benchmark confirms the dynamic pipeline's
cost. It draws 20 frames with 12 full-playfield low-alpha layers per frame,
using the same inputs in all variants. Median times are 0.343635 seconds for
the fixed-function baseline, 0.348062 for the narrow floating-point diagnostic
and 0.775951 for this general shader. The two floating-point variants have
identical full-frame checksums. The baseline checksum differs because it
retains the known alpha-test behavior. Probe source, compile commands, hashes
and all timings are in `alpha-benchmark/report.json`. A cached shader variant
is being checked before the next production candidate.

### Cached candidate: seven-frame Final B verification

Source checkpoint `80d7635d` replaces the dynamic material branches with
context-owned cached shader variants. Alpha reference and texture-factor
values remain uniforms. Its fresh products are:

- GCC 8 x86_64: `e41f8717fc26dd04373696f6e720f850137204cce4af5befe0c224b5b79fb5fb`.
- GCC 12 i386: `b09cb926b396d4c8346337a0b5b11024a9496de8a3bdbfafa8f8db25f8ffb948`.

Both pass ELF/global-owner checks, and their manifests pin the same 132 build
inputs. The alpha-state matrix and low-alpha filtering profile each match
6,144 Wine samples on both architectures in both GL contexts. The x86_64
alpha-enabled observed-quad probe also matches all 68 images in GL 2.1.

Three warmed benchmark rounds have median times 0.360763 seconds cached,
0.371129 fixed-function and 0.327325 narrow float control. A fresh run through
the public benchmark reports 0.440766 cached and 0.437427 fixed-function.
Candidate/reference checksums agree in both runs. The shader cache removes
the dynamic pipeline's measured 2.2x synthetic cost. Reproduce with
`benchmark-alpha-pipeline.py --baseline-ref 575c2368`; the
[control guide](../tests/fixtures/render-controls/README.md#cached-alpha-pipeline)
describes the workload and optional independent reference.

The fresh x86_64 default-context gate passes all seven Final B frames and exact
laser/VM packets. Outside the title band, the beam-mask results are:

| Frame | Earlier changed pixels | Cached changed pixels | Cached maximum channel error |
| ---: | ---: | ---: | ---: |
| 8,400 | 68 | 0 | 0 |
| 11,160 | 19 | 0 | 0 |
| 11,490 | 184 | 1 | 1 |
| 12,240 | 9 | 0 | 0 |
| 14,280 | 92 | 0 | 0 |
| 16,770 | 25 | 0 | 0 |
| 17,460 | 10 | 3 | 1 |

The four remaining pixels are unchanged from the earlier candidate: frame
11,490 at `(307, 179)`; frame 17,460 at `(176, 293)`, `(223, 265)` and
`(329, 90)`. Their maximum channel error is 1/255. Thus 403 of the previous
407 outside-title residual pixels are resolved, with no new changed pixels
in that region. Their remaining cause still needs a bounded measurement.
Frame 16,770's total beam mean, 0.141590, comes from the title band; its
outside-title beam error is zero. This separates text-layer differences from
the repaired low-alpha rendering without changing the ordinary image gate.

The seven-frame capture takes about 354 seconds and resumes the same original
full-corpus scheduler afterwards. Its evidence is under
`build/laser-alpha-repair-20261015/cached-finalb/` and
`cached-finalb-summary.json`; all temporary runtimes are removed. Its broad queued worker was retired before
starting any gate when the newer sincos candidate passed the same capture.
The replacement queue tests the newer products; the seven-frame result above
remains specific to the cached checkpoint.

### Four residual pixels: vertex-input localization

A new paired `--watch-quads` capture selects frames 11,490 and 17,460 with
synchronous Wine rendering. Both retail playfields are pixel-identical to
the retained references, and both observed portable playfields match the
ordinary cached-candidate capture. The four residual RGB values repeat
exactly. The capture finishes and restores the original corpus scheduler;
its owned Wine prefix and temporary runtime are removed.

The conservative local comparisons find three differing quads:

| Frame | Pixel | Selected quads | Differing quad index | Differing coordinate words |
| ---: | --- | ---: | ---: | ---: |
| 11,490 | `(307, 179)` | 5 | 71 | 8 |
| 17,460 | `(176, 293)` | 29 | 1,031 | 3 |
| 17,460 | `(223, 265)` | 49 | 1,031 | 3 |
| 17,460 | `(329, 90)` | 8 | 27 | 8 |

Indices are zero-based submission indices, paired only within the stated
region and frame. The two middle points share the same differing quad.
Other selected words, including diffuse colors, Z/RHW and UVs, agree.
These observed coordinate differences precede GPU filtering and blending.

Quad 1,031 maps to laser slot 84's body VM. All recorded VM fields agree,
including position, rotation, scale and sprite size. Its rotation is binary32
`0x3f879c1a` (1.0594513416290283). The target `Draw2D @ 0x00463210` executes
`FSINCOS` at `0x0046328D`, then stores each result as float. The portable
macro calls `sinf`/`cosf`. For this input, glibc `cosf` returns `0x3efa8c2a`;
80-digit trigonometry rounded to binary32 gives `0x3efa8c2b`. The latter,
computed by double `cos` then converted to float, restores all eight observed
XY coordinate words through the same float arithmetic. The matching sine is
`0x3f5f411a`. The standalone golden-case test fails with the preceding
header and passes the double-then-float repair. The following fresh game
capture verifies the corresponding pixel change.

The other two quads have projected Z 0.9993928670883179. Their nearly square
edges measure about 24.000003 versus 24.000731 pixels at frame 11,490, and
23.999266 versus 24.000000 at frame 17,460. The centers and angles remain
close. These measurements identify a projected-geometry rounding lead;
the actual projection inputs and intermediate results remain to be captured.

An independent asset-free precision probe also checks all 65,536 A4R4G4B4
source words with four diffuse colors, three blend modes and one/four draws.
Point and quarter-texel linear profiles each match all 1,572,864 RGB samples
against Wine D3D8. This excludes discrepancies in those controlled sampling
conditions. The actual sprite transforms retain their separate evidence.
Use [the packed-precision and regional commands](../tests/fixtures/render-controls/README.md#exhaustive-packed-color-precision)
to reproduce the probes and local comparisons.

Artifacts are under `build/laser-residual-pixels-20261015/` and the isolated
worktree's `build/residual-paired-quads/`. Regional JSON reports preserve the
full paired image/state comparison, selected frame, ROI, original vertex
words and differences. `--frame` still validates all quad rows and stages;
`--check` returns failure for the observed mismatches. The new synthetic
probe removes its raw sample buffers and executables after recording hashes.

Reproduce the synchronized paired observation and local vertex comparison:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 7 --frames 14280 \
  --watch-quads --wine-csmt 0 --retail --output-dir build/finalb-quads
python3 scripts/analysis/compare-sprite-quads.py \
  --reference build/finalb-quads/retail --candidate build/finalb-quads \
  --roi 168,310,169,311 --selection covering-bounds \
  --output build/finalb-quads/vertex-comparison.json
```

### Retained sampling-candidate checks

At checkpoint `2fd6a51b`, against the synchronized frame-14,280 reference,
the beam-omission control is detected:
the beam mean rises from 0.023551 to 41.223733. Existing error localization
assigns 2.2% of the residual RGB error to cap regions, 54.1% to pixels changed
by beam omission and 43.7% to the remainder. These are spatial/control
partitions, with the meaning documented in `analyze-laser-errors.py`.
`finalb-14280-error-partition.json` and
`finalb-14280-omission-revalidation.json` retain those checks. The remaining
small error at that checkpoint has aligned local vertex inputs. The alpha-test
investigation above later attributes and repairs this frame's residual.

All four Stage 4B default-context image/state checks and retained comparisons
also pass. Their beam means are 0.004278, 0.000999, 0.003865 and 0.020752 at
frames 13,140, 13,260, 15,360 and 18,210. The changed laser-mask pixel counts
are 3, 3, 1 and 140; maximum channel errors are 3, 3, 4 and 4/255. The masks
avoid the title band. `stage4b-pixel-locations.json` preserves the summaries.

Evidence is under `build/laser-sampling-production-20261015/`. A serial worker
is repeating all five scene cases in both context profiles, then the complete
108-recording corpus and retained-evidence checks. Its state is in
`serial-gates.json`; a failed comparison stops the queue. These pending gates
are required before promoting this candidate. All five scene cases pass in
both contexts, including separate retained comparisons: 48 images and exact
laser/VM packets. The maximum beam means per case, equal across profiles, are:

| Scene | Frames per profile | Maximum beam mean RGB error |
| --- | ---: | ---: |
| Stage 3 | 4 | 0.590982 |
| Stage 4B | 4 | 0.020752 |
| Final A | 5 | 0.811188 |
| Final B | 7 | 0.142039 |
| Extra | 4 | 0.003049 |

This sampling product's full-corpus attempt was retired at a recording
boundary after 65 completed comparisons. Its queued i386 and subsequent
sincos workers were retired before their scene/corpus gates started. The
completed scene and diagnostic evidence above retains its product-specific
scope. The latest D3DX candidate now runs the complete gates below.
Native products and the earlier published corpus results retain their
recorded scope.

The refreshed GCC 12 i386 product is
`240b3f9a4bae462a73129fe0d6b10b65ad44fe9a036889694e422b71cd42ca98`.
It passes ELF32/fixed-layout verification. All 132 compiled inputs match
source checkpoint `bd1f826f` and the x86_64 build inputs. The build used one
CPU, 768 MiB, no swap and one job. Both context profiles pass eight
renderer/FPU groups and 68/68 API images. The probe's compiler/package
wrapper support is exercised by these runs; reports pin wrapper hashes,
compiler versions and compile commands. All 640 extended surface API cases
also agree with the pinned D3DX8 reference. Evidence is under `production-i386/`, `i386-contracts-*/`,
`i386-probe-*/` and `i386-preflight.json` in this phase directory. The compiler
tree and completed contract executables are removed.

### Sincos candidate: seven-frame verification

Checkpoint `0c9f0ef4` evaluates the modern sincos macro in double precision,
then stores binary32 results, following the observed target rounding for
these angles. All four callers draw ANM geometry. The native VC7 macro is
unchanged. Eight golden inputs and a preceding-header failure control pass,
as do the i386 single-precision test and existing vector contracts. A cold
single-job comparison retains all 1,106 accepted authored exact matches.

Fresh source-pinned products:

- GCC 8 x86_64: `25af84840b5175da7718f640359c9f9220d8c1f5c68f6d686a512f1cc9d1c370`.
- GCC 12 i386: `8faef6f0234480746122fb9f4987ad906fddce516ddb61aa61d5cbf7959560a4`.

The x86_64 default-context capture passes all seven Final B image limits with
exact laser/VM packets. Both laser-slot-84 differences disappear. The table
separates beam-mask measurements from the complete playfield below the title
band, rectangle `(32, 64)`–`(416, 464)`:

| Frame | Changed beam-mask pixels below title | Changed complete-playfield pixels below title |
| ---: | ---: | ---: |
| 8,400 | 0 | 3 |
| 11,160 | 0 | 4 |
| 11,490 | 1 | 6 |
| 12,240 | 0 | 4 |
| 14,280 | 0 | 0 |
| 16,770 | 0 | 0 |
| 17,460 | 1 | 6 |
| Total across frames | 2 | 23 |

All residual channel errors in this table are at most 1/255. The two beam
pixels are `(307, 179)` at frame 11,490 and `(329, 90)` at frame 17,460.
Compared with the cached candidate, only the two repaired pixels change in
the complete playfield; no new differences appear. This resolves 405 of the
original 407 outside-title beam-mask differences. Title text and the HUD have
separate measurements; the table excludes them.

Reports are under `build/laser-sincos-repair-20261015/`: `finalb/`,
`finalb-summary.json`, and `pixel-regression.json`. The compiler trees are
removed. All eight alpha/filter API profiles pass at this checkpoint
(two architectures and two contexts, 6,144 samples per profile). Its queued
scene/corpus worker was superseded before starting those gates by the D3DX
candidate below.

Reproduce the actual seven-frame observation and both pixel scopes:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 7 \
  --frames 8400,11160,11490,12240,14280,16770,17460 \
  --retail --wine-csmt 0 --output-dir build/sincos-finalb
python3 scripts/analysis/summarize-laser-pixels.py \
  --reference build/sincos-finalb/retail --candidate build/sincos-finalb \
  --full-playfield --top-pixels 32 --output build/sincos-finalb-pixels.json
python3 scripts/test-portable-sincos.py
python3 scripts/test-portable-sincos.py --baseline-ref b4e70194 --expect-failure
```

The summary verifies both regional totals against the strict comparison and
retains residual coordinates, RGB values, input hashes and analyzer identity.
The two projected-particle vertex differences remain the next investigation;
actual projection inputs and intermediates are still needed to explain them.

### Fast geometry observations

Use `--watch-quads --geometry-only` to read submitted sprite inputs with
rasterization disabled. This mode records vertices and laser/VM packets,
without framebuffer readback or placeholder images. It keeps at most 16
selected frames per run. Ordinary image comparisons require rasterized captures.

For a calibration, first retain an ordinary `--watch-quads` capture. Use the
same executable, recording, stage and frames for the fast capture:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 7 --frames 11490,17460 \
  --watch-quads --output-dir build/orbit-rasterized
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 7 --frames 11490,17460 \
  --watch-quads --geometry-only --geometry-reference build/orbit-rasterized \
  --output-dir build/orbit-geometry
python3 scripts/analysis/compare-sprite-geometry.py \
  --reference build/orbit-rasterized --candidate build/orbit-geometry \
  --check --output build/orbit-geometry/revalidation.json
```

The capture's `geometry-comparison.json` checks every sprite word in the
playfield, all requested frames, raw laser/VM packets and capture identities.
The standalone command repeats that check from retained files. Reports retain
input hashes and bounded mismatch examples. `geometry-observation.json`
records the inferior's rasterization environment and frame coverage. Timing
dependent HUD digits outside the playfield are excluded from vertex pairing.
Fresh game captures and strict image comparisons provide the final pixel gate.

The published mode passes an actual two-frame calibration against the retained
sincos product: all 469 and 3,087 playfield quads and raw laser/VM packets
agree. The capture takes 54.21 seconds and produces no frame images. Evidence
is in the D3DX worktree's `build/geometry-mode-smoke-20261015/`; elapsed time,
product and input hashes are recorded with the result. Thirteen regression
tests cover mismatches, incomplete frames, invalid profiles and CLI guards.

### Orbit particles: D3DX rounding

Completed paired observations at frames 11,490 and 17,460 identify effect
slots 349 and 462, both effect ID 13 with `EffectOrbitUpdate @ 0x00426030`.
All 41 position/scratch words and eight state bytes agree except the final
position coordinates. Quaternion orientation, radius, angle, camera and
projection inputs agree. Re-evaluating the recorded native inputs reproduces
all target projection locals and submitted XY words. The discrepancy begins
before projection.

A geometry-only portable capture repeats every selected playfield quad word:
469 quads at frame 11,490 and 3,087 at frame 17,460, plus exact raw laser/VM
packets. The inferior records `LP_NO_RAST=true`. Its scope is numerical
observation; ordinary rasterized captures provide the image evidence.

Two original-library arithmetic differences are reproduced without game
assets:

- `c_D3DXMatrixRotationQuaternion @ 0x004781CD` doubles quaternion components
  before multiplication and subtracts diagonal terms separately. Its complete
  226-byte archive member matches the target after its constant relocation;
  the target literal at `0x004B42EC` is binary32 2.0.
- The observed `D3DXVec3TransformCoord` dispatch points to `0x0048EDF0`.
  Its SSE section matches all 256 bytes, including the alternate entry and
  alignment tail. It adds `(X product + Y product) + (Z product + translation)`,
  refines `RCPPS` once, then multiplies by the W reciprocal. This rounding
  differs from direct division even for affine transforms with W=1.

The reference archive is the SHA-pinned prerelease `D3DX8.LIB` in
`config/library-provenance.toml`. Both comparisons are bounded diagnostic
proofs; the library inventory and acceptance ledgers are unchanged. The
modern adapter now follows these operations. Its standalone orbit evaluator
reproduces both original positions and all matrix words. A sine-only control
keeps the old position differences and is insufficient for these samples.

The committed [golden inputs](../tests/fixtures/d3dx-rounding.json) contain two
observed orbit quaternions, four observed projection inputs/results and two
unit-W reciprocal cases. The fixture distinguishes observed target locals
from original-library matrix outputs, identifies the reference CPU, and links
the recording credits. `RCPPS` approximation is CPU-dependent; the optional
native oracle runs on the same host as the portable probe.

```bash
python3 scripts/test-portable-d3dx-rounding.py \
  --output-dir build/d3dx-rounding-goldens
python3 scripts/test-portable-d3dx-rounding.py \
  --baseline-ref d1b5ffe4 --expect-failure \
  --output-dir build/d3dx-rounding-baseline
python3 scripts/test-portable-d3dx-rounding.py --native --attest-target \
  --output-dir build/d3dx-rounding-native
```

The default check passes all eight goldens; the historical adapter fails both
quaternion and both unit-W cases. The native run additionally compares 128
generated quaternions and 128 generated projective transforms: all 264 cases
match every output word. It uses VC7, the hash-checked archive and single
precision, records commands and hashes, and removes temporary executables and
objects. `--attest-target` also repeats both archive/target section comparisons
and checks the quaternion constant; it requires the canonical executable. Native mode requires the existing local VC7 environment; the golden
check requires only a Linux compiler and SDL development headers.
The first quaternion case initializes the original library's shared dispatch
table before vector cases. Use the same warm-up when probing isolated
transforms to reproduce the game's active SSE path.

Source checkpoint `d64dafd8` passes clean x86_64/i386 builds and ELF/global
ownership verification. Both production compiler profiles pass all 264 math
cases and renderer contracts in both GL contexts. Compiler trees and completed
probe executables are removed.

The fresh x86_64 default-context game capture also passes all seven Final B
frames with exact laser/VM packets. Every RGB pixel below the title band,
rectangle `(32, 64)`–`(416, 464)`, now matches retail in each frame. The changed
pixel counts are 0 at frames 8,400, 11,160, 11,490, 12,240, 14,280, 16,770
and 17,460. This resolves all 23 previous full-playfield residuals, including
both remaining beam-mask pixels. Title text and timing-dependent HUD content
have separate measurements.

Reports and source-pinned product manifests are under
`build/laser-d3dx-math-repair-20261015/`; `finalb-summary.json` reports both
pixel scopes. The sole active serial queue tests five scenes in both contexts
and architectures, followed by 108 recordings and retained verification per
architecture. Earlier incomplete candidate queues were retired at a recording
boundary, preserving their completed evidence. These full gates remain in
flight. Diagnostic inputs and prototypes are under
`build/laser-projection-inputs-20261015/`.

### Wider playfield survey

The same x86_64 product has zero changed RGB pixels below the title band in
all selected Stage 3 and Final A frames. Stage 4B has a background residual
outside the laser masks at frame 15,360. A fresh canonical capture with
`--wine-csmt 0` retains 5,951 changed pixels, maximum channel error 22/255;
the other three Stage 4B playfields match exactly. The older asynchronous
reference gave 6,929 changed pixels at that frame and differs from the new
retail image itself. Keep the two reference captures separate.

Extra frame 24,900 has six changed pixels below the title band, each at most
1/255; its other three frames match. Default and OpenGL 2.1 portable
playfields match each other at every selected Stage 4B and Extra frame.
These measurements extend the beam-mask gate to other visible layers.
The Stage 4B background cause remains under investigation.

The synchronous reference and comparison are in the D3DX worktree's
`build/stage4b-retail-csmt0-20261015/` and
`build/stage4b-synchronous-reference-summary.json`. The public pixel summary
command above reproduces both scopes from retained captures.

### Perspective arithmetic and billboard inputs

The follow-up is isolated on `fix/portable-perspective-rounding`. Generated
camera inputs expose a separate D3DX arithmetic difference. All 128 LookAt
cases agree with the original library. Perspective matrices disagree in
81/128 cases on x86_64 and 93/128 on i386, affecting X/Y scales and depth
translation. The target `D3DXMatrixPerspectiveFovLH @ 0x004784DD` stores
FSINCOS outputs, divides cosine by sine, and multiplies the rounded depth
scale by the near plane. The adapter previously used `1/tan` and a differently
ordered depth expression. The corrected adapter follows the target operations.

All 148 bytes of the pinned archive member match the target after one constant
relocation; the referenced binary32 0.5 at `0x004B847C` also matches. This is a
diagnostic comparison, with unchanged inventory and acceptance ledgers.
The regression fixture adds three perspective counterexamples and two LookAt
controls. The current default check passes 13 goldens; both production compiler
profiles pass all 525 native-oracle cases and three archive/target attestations.
The recorded pre-repair control fails exactly the three perspective goldens:

```bash
python3 scripts/test-portable-d3dx-rounding.py \
  --baseline-ref 4590feb3 --expect-perspective-failure \
  --output-dir build/perspective-baseline
python3 scripts/test-portable-d3dx-rounding.py --native --attest-target \
  --output-dir build/perspective-native
```

Actual Stage 4B image verification is still required for this source. The
preceding product's paired frame 15,360 captures complete with exact laser/VM
packets, but the added Draw3D observer records zero calls on both sides.
Its coverage assertion fails after capture; the images and ordinary quad
observations remain complete. The visible strips use projected billboard
segments instead. Five mixed-alpha segment quads have different RGB values
at their zero-alpha ends. Target branches at `0x0040AD9C` and `0x0040B123`
write only alpha, leaving those RGB bytes from the stack. These observations
identify a possible source of capture-to-capture background variation; a
controlled image comparison is still needed to measure its contribution.

Private API controls, canonical disassembly, archive attestation and the
paired-capture worker records are under
`build/stage4b-background-inputs-20261015/`. The ordinary paired images and
quads are in the D3DX worktree's `build/stage4b-background-paired-20261015/`.

The perspective source checkpoint `e09ae250` has fresh GCC 8 x86_64 and GCC 12
i386 recorder products. Both pass ELF/global ownership checks with 132
commit-attested inputs; the adapter is their only changed production input.
The x86_64 executable SHA-256 is
`2389e152dcb0a2046db194751afca7a1a5a3c479b9ad93976bca1e094c804a70`,
and i386 is
`f3143dc1ae7a947a5bfa994a21e22b9f51d72a4b921135ad0dd2507140dc83ee`.
Compiler intermediates are removed. The source-bound 525-case API checks and
both renderer context profiles pass on each architecture. Ordinary Stage 4B
captures and the separate endpoint-RGB diagnostic are in flight under
`build/laser-perspective-repair-20261015/`.

## Legacy OpenGL coverage, October 14, 2026

Disabling `GL_ARB_clip_control` reproduces the pre-repair top-edge failure at
source checkpoint `da521dcf`. That repair stores the logical top at the
lowest FBO row in this mode. Readback, CPU writes, viewport clears, framebuffer
audit regions, and presentation use the same mapping. At that checkpoint the
clip-control path kept its existing orientation.

Both default and forced OpenGL 2.1 contexts pass six asset-free renderer
contract groups, including asymmetric clears and actual Xvfb window pixels.
The legacy observed-quad API probe matches all 34 solid-material images and
52/68 total images; its old-source control differs in all 68. Sixteen textured
images retain sampling differences.

Fresh legacy game captures pass all eight original 1.0 image limits. Their
laser/VM packet hashes remain the published Stage 3 and Stage 4B hashes.

| Stage | Frames | Beam-region mean RGB errors |
| --- | --- | --- |
| 3 | 12,900; 14,130; 14,310; 14,490 | 0.786679; 0.323592; 0.003074; 0.763161 |
| 4B | 13,140; 13,260; 15,360; 18,210 | 0.004278; 0.001998; 0.003865; 0.021176 |

Frame 12,900's outside-header mean is 0.003639, with no pixels above mean RGB
error 32. The context observed through GDB is `2.1 Mesa 25.2.8-0ubuntu0.24.04.2`,
with a null clip-control function pointer. The tested x86_64 Debug recorder has
SHA-256 `62386854ab8e486a8ad742c97f27b7f2063d39e008aefc4af8600dd083b9aecc`.
Its build identity, captures, and API controls are under
`build/laser-legacy-20261014/`.
The same binary's complete `lunatic-0-b` recording passes 113,221 calculations
on all 25 fields with a natural replay exit. Retained-evidence verification
passes. Forced-profile checks also match all 297 color samples and preserve all
144 D3DX copies and 196,608 exhaustive copies. These adapter changes leave the
native VC7 source and accepted-unit ledgers unchanged.

Repeat the game capture and independent API control:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case stage3-keine --legacy-gl --retail \
  --output-dir build/laser-stage3-legacy
python3 scripts/analysis/probe-subpixel-quads.py \
  --capture build/laser-quads --legacy-gl --baseline-ref da521dcf --check-coverage \
  --output-dir build/subpixel-legacy-api
```

`--legacy-gl` changes the portable context only. Game captures record the
observed version and function pointer; retained comparisons reject missing or
incompatible legacy-context evidence. Renderer contract reports retain source
and binary hashes, compiler identity, exact assertions, and process results.
The Linux CI workflow runs both context profiles and uploads those reports.

## i386 refresh, October 14, 2026

The revised i386 Debug recorder was built with Debian GCC 12.2.0 in the
repository's [32-bit container](../containers/linux32.Dockerfile). Its SHA-256
is `a82e7155eb7c71e5db645df9294cd5316e895df5f35dcbce696946ba4ba9de27`.
The fixed target-owned layout check passes. Both context profiles pass the six
renderer groups and the extra x87 precision/`FPU_PRESERVE` group on the host's
Mesa 25.2.8 runtime.

All eight selected frames pass the original image limit in each profile:
16 captures, with identical retail laser/VM packet hashes.

| Profile | Stage | Beam-region mean RGB errors, in manifest frame order |
| --- | --- | --- |
| Clip control | 3 | 0.786645; 0.324227; 0.003113; 0.766607 |
| Clip control | 4B | 0.004278; 0.001907; 0.003865; 0.021046 |
| OpenGL 2.1 | 3 | 0.786679; 0.323592; 0.003074; 0.763161 |
| OpenGL 2.1 | 4B | 0.004278; 0.001998; 0.003865; 0.021176 |

Seven playfields per profile are pixel-identical to the retained x86_64
captures. Stage 4B frame 15,360 has small background-shading differences:
cross-product playfield means are 0.073440 with clip control and 0.074393 in
the legacy profile. The first comparison changes 7,587 pixels, with a maximum
channel difference of 9 and no pixel above mean RGB error 32. Difference
images place these changes on the background's thin columns. The
[draw-input investigation](PORTABLE_DRAW_INPUTS.md) subsequently localizes the
difference to RGB retained by the target's zero-alpha far-fog vertices, using
same-source products and an isolated color substitution. The earlier
default-profile x86_64 reference precedes the legacy source repair.

Evidence is under `build/laser-i386-20261014/`: the four named stage/profile
directories retain images, raw state and comparisons; `renderer-default-final/`
and `renderer-legacy-final/` retain the seven-group API results. Build provenance records
the container image, actual compiler, ELF `.comment`, configuration and source
hashes. Capturing on i386 also exposed a GDB call with an unknown `free` return
type; the observer now declares its void return explicitly. This changes the
capture tool only.

The same i386 binary's complete `lunatic-0-b` recording passes 113,221
calculations on all 25 fields with a natural replay exit and the published
trace fingerprint. Retained-evidence verification passes. The full corpus for
the revised binary remains a separate gate.

Use the same capture commands with the i386 recorder. Add `--legacy-gl` and a
fresh output directory for the second profile. See the
[i386 renderer command](PORTABLE_RENDERING.md#pixel-contracts-without-game-assets)
for architecture-specific compiler and package-metadata selection.

### i386 source refresh after the shared repairs

The new GCC 12 i386 recorder is SHA-256
`bd8b148c0ac6a478391ee4c1fea7334ac1f473466418131715cda4e1a900ce40`, with
build-input hashes pinned to source checkpoint `ed8ff8df`. It includes the
script-owner, GDI font, surface-filter and standalone ECL sine/cosine repairs.
ELF32 and fixed-layout ownership checks pass. The extended surface probe
passes all 640 pinned-D3DX8 API cases, and seven default renderer/FPU contract
groups pass. All seven Final B image/raw-state checks and all 390 paired spawn
inputs also pass. Beam errors are identical to the repaired x86_64 product at
0.003476–0.144060. Its full 108-recording replay gate and separate
retained-evidence pass now succeed: 12,311,460 calculations agree on all
25 fields, stage-end scores, published fingerprints and natural exits.
Temporary runtimes are removed, and all equal traces share reference storage.
The completed scene refresh is recorded below. Evidence is under
`build/laser-i386-refresh-20261014/`; the owned compiler tree has been removed.

This build runs CMake, package discovery and Ninja together in the pinned
container. Host CMake with only a compiler wrapper first failed to access its
ABI probe source, then failed to locate container Fontconfig libraries. Both
failed configuration logs remain retained. The
[container reproduction commands](PORTABLE_REPLAY_TESTING.md) use the committed
image recipe and a single compiler job with a 768 MiB limit.

### Refreshed i386 OpenGL 2.1 check, October 15, 2026

The same `bd8b148c…` product also passes all seven renderer/FPU contract groups
under forced OpenGL 2.1 with clip-control unavailable. Its seven Final B raw
laser/VM packets and all 390 paired spawn inputs agree exactly with retail.
Every beam image passes the unchanged 1.0 limit:

| Frame | Default-context mean RGB error | OpenGL 2.1 mean RGB error |
| ---: | ---: | ---: |
| 8,400 | 0.003476 | 0.003444 |
| 11,160 | 0.011708 | 0.011483 |
| 11,490 | 0.008568 | 0.008650 |
| 12,240 | 0.004209 | 0.004393 |
| 14,280 | 0.023993 | 0.023883 |
| 16,770 | 0.144060 | 0.144252 |
| 17,460 | 0.004788 | 0.004975 |

The new `finalb-kaguya` manifest pins the original seven-frame state file.
Retained-evidence verification passes that pin for x86_64, i386 default and
i386 legacy captures. Reports are under `renderer-legacy/` and
`finalb-legacy/` in `build/laser-i386-refresh-20261014/`. Both selected rendering
contexts now have actual Final B images; other scenes and complete gameplay
replays retain their own gates.

Reproduce either portable context with `--case finalb-kaguya --retail`.
Add `--legacy-gl` for the OpenGL 2.1 capture and use a separate output directory.
The same case supports `--watch-laser-spawns 8000:8400` for input regression.

### Completed i386 scene refresh, October 15, 2026

The same `bd8b148c…` recorder passes fresh Stage 3, Stage 4B, Final A and
Extra captures in both contexts. Every raw laser/VM packet agrees with the
pinned retail reference. A second comparison of each retained capture also
passes. The table reports the largest beam-region mean RGB error in each
case; the pass limit remains 1.0.

| Case | Frames per context | Default maximum error | OpenGL 2.1 maximum error |
| --- | ---: | ---: | ---: |
| Stage 3, Keine | 4 | 0.591680 | 0.591713 |
| Stage 4B, Marisa | 4 | 0.021046 | 0.021176 |
| Final A, Eirin | 5 | 0.812111 | 0.811995 |
| Extra, Mokou | 4 | 0.004902 | 0.005123 |

Together with the seven Final B frames above, these checks cover 24 selected
frames per context. Stage 3 uses the default portable font; its title-band
differences retain the scope measured earlier. Passing image thresholds
supports these selected scenes, while exact raw-state comparisons establish
agreement in the captured laser/VM fields.

Evidence is under `build/laser-i386-refresh-20261014/scene-default-*/` and
`scene-legacy-*/`, with `comparison.json` and `revalidation.json` in each
capture directory. All 34 pinned queue inputs retain their original hashes;
all temporary game runtimes are removed. The full 108-recording gameplay
gate and its independent retained-evidence pass are recorded in
[the replay results](REPLAY_RESULTS.md).

Use the registered cases to reproduce the images with a fresh i386 Debug
recorder and original game data:

```bash
for case in stage3-keine stage4b-marisa finala-eirin extra-mokou finalb-kaguya; do
  python3 scripts/analysis/check-laser-visuals.py \
    --candidate build/i386-replay/th08-modern \
    --game-data-dir "/path/to/original/TH08 directory" \
    --case "$case" --retail --output-dir "build/i386-default-$case"
  python3 scripts/analysis/check-laser-visuals.py \
    --candidate build/i386-replay/th08-modern \
    --game-data-dir "/path/to/original/TH08 directory" \
    --case "$case" --retail --legacy-gl --output-dir "build/i386-legacy-$case"
done
```

## Final A observation, October 14, 2026

Case `finala-eirin` uses `lunatic-1-a`, Magic Team, Lunatic, Final A,
credited to Asahi in the replay corpus. Its five paired frames cover a dense
stationary attack, startup, active beams, despawn and a later mixed-phase attack.
Retail and portable laser/VM packets are identical, with SHA-256
`8b072685ec6cfa446961167be9a9608eb2ad102dad0da1c7171c2849f0032a28`.
The portable executable is the current x86_64 recorder `62386854…` identified
in the legacy-profile section above. These captures use its default context.

| Frame | Active slots | Beam-geometry pixels | Beam-region mean RGB error | Image result |
| ---: | ---: | ---: | ---: | --- |
| 9,570 | 55 | 10,716 | 17.589088 | Fail |
| 14,490 | 4 | 1,435 | 20.909872 | Fail |
| 14,550 | 4 | 6,372 | 14.604154 | Fail |
| 15,090 | 4 | 5,780 | 15.289389 | Fail |
| 22,590 | 10 | 10,160 | 12.120407 | Fail |

Paired images show a large spell-background texture difference, while the
visible beams and bullets occupy corresponding positions. Full-playfield mean
errors range from 30.788425 to 40.457833. Beam masks also contain the background
visible through transparent laser pixels, so these failures require background
and laser contributions to be measured separately. Texture selection, UV/state
inputs remain investigation questions; the matched laser
packets alone do not establish the rendering cause. At frame 9,570, a fresh
ordinary-paced portable capture has the same laser packet and pixel-identical
playfield as the accelerated capture. Playback pacing is excluded for that
frame. Its draw-input capture records 30 calls, including two large rotated
background quads; comparison with retail texture and quad inputs is still needed.

Reproduce the failing selection with the unchanged 1.0 image limit:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case finala-eirin --retail --output-dir build/laser-finala
```

The retained paired evidence is under
`build/laser-stage4a-20261014/finala-paired/`. The ordinary-paced image and draw
inputs are in `finala-paced-draws/`, with `finala-pacing-comparison.json` in the
parent directory. The manifest pins all five frames and their retail state hash,
including the failing image cases.

### Spell-background script owner repair

Canonical `Background @ 0x004E4030 + spellVmScriptBase @ 0xB34` is
`0x004E4B64`. `GuiImpl::StartMessage @ 0x0043396D` writes 2 there for Final A,
Final B and Extra; `EffectManager::LoadEffectResources @ 0x004284B0` resets it
to 0. Spell-practice setup also selects 2 for the later boss paths.
`Spellcard::StartSpell @ 0x004152A0` reads the same word to choose the ANM scripts.
The target write at `0x004339E6` and read at `0x004154DD` attest this connection.

Source instead gave the writers standalone `g_GuiMessageStageMode` storage.
Both x86_64 portable and the retained native link split it from the Background field.
Portable therefore selected scripts 0/1 after the dialogue selected 2/3 in
retail. At frame 9,570, the first two retail background quads are axis-aligned;
the old portable quads rotate. Their positions and UVs differ in 22 words.
The repaired portable submissions match all words of those two retail quads.

All writers now use `g_Background.spellVmScriptBase`. The standalone global
and its fixed-layout alias are removed. Seven VC7 relocation entries now name
`g_Background @ 0x004E4030` with addend `0xB34`; all three affected functions
have zero non-relocation differences. The cold replay of all 1,106 accepted
units passes. Fresh normal and bugfix links pass the expanded runtime-owner
verifier, which checks all seven writes and the spell-start read and rejects
the pre-repair native product.

The initial repaired x86_64 product `c421597e…` passes all five original limits:

| Frame | Baseline beam-region error | Repaired beam-region error |
| ---: | ---: | ---: |
| 9,570 | 17.589088 | 0.015086 |
| 14,490 | 20.909872 | 0.151220 |
| 14,550 | 14.604154 | 0.076219 |
| 15,090 | 15.289389 | 0.862803 |
| 22,590 | 12.120407 | 0.003248 |

Its laser/VM packet retains the pinned retail hash. Evidence is under
`build/laser-background-owner-20261014/finala-repaired/`; build identities are
in `production-portable-bridge/`. The final shared-owner x86_64 product has
SHA-256 `34bcdd33b2d31a52e418e9f806314fb00c68ac2fd5cea2bad8259f6a5de9fab9`.
Its allocated ELF sections equal the initial repaired product, with debug
sections and the GNU build ID excluded. All five fresh captures in `finala-final/`
pass the same image limits and have pixel-identical playfields to the initial
repair captures. The final product's complete `lunatic-1-a` recording passes
119,685 calculations on all 25 fields, all recorded end scores and natural exit;
retained-evidence verification passes too. Its suite is in `x64-suite/`.
Remaining checks cover Final B, Extra, refreshed i386, native images and the
full corpus for the revised products. The original eight Stage 3/4B results
remain scoped to their earlier binaries.

Repeat the background vertex diagnosis using a single selected frame:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-1-a --stage 6 --frames 9570 --watch-quads --wine-csmt 0 --retail \
  --output-dir build/finala-quads
python3 scripts/analysis/compare-sprite-quads.py \
  --reference build/finala-quads/retail --candidate build/finala-quads \
  --selection covering-bounds --roi 32,16,415,463 \
  --output build/finala-quads/background-quads.json
```

The selector compares all submissions whose bounds contain the stated region,
in submission order. The interior pixel coordinates include the target's
half-pixel-adjusted background edges. The ordinary image mask and threshold
stay the same. The older i386 product already aliases both names at
`0x004E4B64` through its fixed-layout linker script; refreshed i386 testing
checks the explicit owner after retiring that alias. Native link reproduction uses
`python3 scripts/analysis/verify-windows-i386-runtime-data.py` after each fresh
normal/bugfix build; the [Windows guide](WINDOWS_I386_RUNTIME.md) gives the order.

A separate Stage 4A survey of this recording observes 22,947 presented gameplay
counter samples at interval 1, with no active laser slots. This describes the
observed presentations rather than all numerical counter values. The later
Final A interval-30 survey observes 321 samples with active lasers among 877
samples and records its counter gaps. Both complete within their frame limits;
their reports are in `scout-every-frame/` and `finala-scout/` under the same
evidence directory. The earlier Stage 4A report predates gap accounting.

## Native Windows image capture

Use the fresh VC7 `bugfix` executable and its matching linker map. The runner
derives the laser-pool base and hardware-breakpoint addresses from that map;
it attests the mapped instructions against the supplied executable before
reading state. The canonical Japanese 1.00d target remains the retail reference.

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/th08.exe --native-map build/th08.map \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case finala-eirin --retail --output-dir build/native-finala
```

Native captures use ordinary pacing, normal life rules, muted audio and
read-only gameplay observations. `--watch-quads` also records sprite vertices.
Linux draw/font probes, scout mode and beam-omission controls remain portable
features. The comparator validates native map identity, completed frame coverage
and observer provenance before applying the same 1.0 beam-region image limit.

The first native Final A capture, executable `1b8e217a…`, matches all five
retail laser/VM packets and the 22,590-calculation direct-stage prefix on all
25 fields. Four complete playfields are pixel-identical to retail. Frame
15,090 retains a large image difference, with beam-region mean 51.444637 and
playfield mean 19.197831; its displayed score differs while its recorded score
agrees. Evidence is under `build/laser-native-20261014/finala-repaired-v2/`.
The default native image gate therefore remains four of five.

Wine 9.0 enables its multithreaded command stream by default and reads the
`csmt` override from `WINE_D3D_CONFIG`; see the
[pinned Wine source](https://raw.githubusercontent.com/wine-mirror/wine/wine-9.0/dlls/wined3d/wined3d_main.c).
A paired `--wine-csmt 0` capture tests whether stopping the rendering worker
at the post-Present breakpoint leaves an older window image. This is a backend
scheduling control, recorded separately from the default profile. Native
comparisons require the same CSMT setting in both captures.

The paired `csmt=0` test recovers a pixel-identical full playfield at frame
15,090, zero beam-region error, identical laser/VM packets and all 25 fields
over its 15,090 calculations. The native image also equals the retained default
retail image at that frame. A live process check observes `WINE_D3D_CONFIG=csmt=0`
and three threads. Changing only command-stream scheduling resolves the image
failure while the gameplay records remain equal, locating this failure in
the Wine/debugger capture path. The default failure remains retained.
Control evidence is in
`build/laser-native-20261014/frame15090-csmt0-paired/`; the whole five-frame
selection under this uniform profile remains a follow-up gate.

The repaired native product also passes the complete `lunatic-1-a` recording:
119,685 calculations on all 25 fields, all recorded stage-end scores and natural
exit. This full-run gate reuses the validated retail reference at clock rate
128 with rasterization disabled; images above use clock rate 1 with rasterization.
Evidence is under `build/laser-native-20261014/full-state128/`.

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/th08.exe --native-map build/th08.map \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-1-a --stage 6 --frames 15090 --wine-csmt 0 --retail \
  --output-dir build/native-finala-csmt0
```

### Native main backport image gate

The main backport checkpoint `5dfd5197` has fresh VC7 normal and bugfix links,
a cold 1,106-unit exact gate, and passing linked owner/data/callee guards.
The bugfix executable is SHA-256
`9e699dd2c596acb090a5d44e9aa32b6c0bb19f0d5e11fe6a0babb0eb1469fd9b`.
Fresh paired `finala-eirin` captures with `--wine-csmt 0`, clock rate 1 and
rasterization pass all five frames: 9,570, 14,490, 14,550, 15,090 and 22,590.
Every raw laser/VM packet agrees, every beam-region mean RGB error is zero,
and all five complete playfields are pixel-identical to retail. The paired
22,590-calculation diagnostic prefix agrees on all 25 fields.

Products and linker maps are under
`build/laser-native-20261014/main-production/`; paired images, state, traces
and comparisons are under `main-finala-paired/` in that phase directory.
The capture tools are pinned separately from compiled source checkpoint
`5dfd5197`. Both owned Wine prefixes and temporary runtimes have been removed;
the retained paired evidence occupies about 5 MiB. The new main product's
complete native replay gate and separate retained-evidence verification now
pass all 108 recordings and 12,311,460 calculations, including all 25 fields,
stage-end scores, published fingerprints and completion boundaries. Evidence
is under `build/laser-native-20261014/main-full-state128/`; temporary runtimes
and the owned Wine prefix are removed. Earlier passes retain their product
identities.

Reproduce with a fresh bugfix link and matching map by adding
`--wine-csmt 0` to the native image command above.

## Final B precision observation, October 14, 2026

The Final B survey selected frames 8,400, 11,160, 11,490, 12,240, 14,280,
16,770 and 17,460 from `lunatic-0-b`. The preceding surface-filter product's
paired captures pass the original image limit in all seven frames, with beam
errors 0.003476–0.144060. Its strict packet gate fails at 8,400: slots 12 and 19
each differ by one ULP in their
origin Y coordinate, with corresponding start-cap Y differences. The other
six frames have identical laser/VM packets. The failed comparison is retained
under `finalb-paired/` in the phase directory; thresholds remain unchanged.

`SpawnLaserPattern @ 0x00430F20` copies the descriptor position. The ECL path
constructs that descriptor from enemy world position and shoot offset.
Canonical polar handlers at `0x004195B5` (opcode 38) and `0x0041E156` (opcode
166) call the float trigonometric wrappers, multiply their wider register
results, then store floats. Portable currently rounds through `sinf`/`cosf`
first. This repeats the wrapper distinction documented in
[polar motion precision](PORTABLE_POLAR_MOTION.md#target-evidence-and-repair).
A private counterfactual kept double-precision trigonometric results through
the radius multiplication in both handlers. Its fresh seven-frame capture
retains the same four differing fields at 8,400; the other six packets agree.
That experiment does not explain this failure, and its source changes were
reverted. The patch, native RunEcl comparison and failed capture remain under
`build/ecl-polar-laser-20261014/`.

### Spawn-input evidence and repair

A fresh paired capture records 390 calls to `SpawnLaserPattern` in frames
8,000–8,400. Enemy world positions and input angles agree. Eight calls have
one-ULP differences in shoot-offset components; cached ECL variables extend
the differences to eleven records. The first differing offset is at 8,042.
Slots 12 and 19's observed origins are created at 8,242, already carrying the
same differences seen at 8,400. Their start-cap differences follow those origins.

Subroutine 52's observed prefix uses ECL cosine opcode 33 at `+0x194` and
sine opcode 32 at `+0x1BC`, with float multiplications by 32 immediately
after each. The canonical handlers at `0x004193AF` and `0x00419347` call
`cosf @ 0x00408D40` and `sinf @ 0x00409060`, then store floats. Those wrappers
call the double CRT routines. For all eight differing offsets, double
trigonometry followed by a float conversion agrees with retail; the host's
float-returning libm calls reproduce the earlier portable words.

The portable opcode branches now call `sin(double)` and `cos(double)` and
convert their results to float before writing the ECL variable. The native
expressions remain unchanged. A fresh build and comparison of the affected
RunEcl unit is exact; this is a single-unit check. The initial GCC 8 x86_64 Debug
recorder is SHA-256
`ee78ef7efa7f9e79bd89f1b82320d266f8d63725eefd0298a4a3b2330b4aca10`.
Its 390 spawn-input records all agree with retail. All seven Final B laser/VM
packets now agree, and the beam image errors remain 0.003476–0.144060. The
final product, built from checkpoint `897decb2`, repeats all 390 input records
and seven image/packet checks successfully. Its SHA-256 is
`e0a57a43520524713b70dbaa38329155fd6ab4a09006869b4f09db8dbcc27649`,
with provenance under `production-final/`. All 132 recorded build-input hashes
agree with that checkpoint after LF normalization. Its 108-recording refresh
and separate retained-evidence pass both succeed under `x64-suite/`:
12,311,460 calculations agree on all 25 fields, stage-end scores, fingerprints
and natural exits. The equal traces share validated reference storage, and
all temporary runtimes are removed.
Evidence is under `build/laser-spawn-origin-20261014/`. Both owned compiler
trees and the superseded initial repair executable are removed; its metadata
and comparison reports remain.

Reproduce the paired input and frame checks with a newly built recorder:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 7 --frames 8400 \
  --watch-laser-spawns 8000:8400 --retail \
  --output-dir build/finalb-spawn-check
```

The command requires both comparisons to pass. `laser-spawns.json` stores
exact float words, owner slots, context state and call order. The native
observer uses attested hardware breakpoints and reserves their limited slots
for this diagnostic. Each capture holds at most 4,096 records. `firstFreeSlot`
records the first free pool slot before spawn-suppression checks. Portable
additionally saves bounded ECL subroutine prefixes under
the private build directory. Recompute the input comparison without launching
the game:

```bash
python3 scripts/analysis/compare-laser-spawns.py \
  --reference build/finalb-spawn-check/retail \
  --candidate build/finalb-spawn-check \
  --output build/finalb-spawn-check/spawn-revalidation.json
```

The 25-field gameplay pass remains scoped to its recorded fields. These spawn
and laser-pool packets provide the additional state evidence for this repair.

## Extra laser observation, October 15, 2026

The Border Team Extra recording `extra-0-extra` adds the Mokou case
`extra-mokou`. Fresh canonical-retail and repaired x86_64 captures observe
one laser in startup, active and despawning states:

| Frame | Phase | Beam pixels | Mean RGB error | Raw laser/VM packets |
| ---: | --- | ---: | ---: | --- |
| 24,900 | Startup | 5,138 | 0.003698 | Exact |
| 24,960 | Active | 6,120 | 0.004902 | Exact |
| 25,320 | Despawn | 2,943 | 0.001472 | Exact |
| 25,800 | Later active | 6,051 | 0.004848 | Exact |

All four pass the unchanged 1.0 beam-region image limit. The retail state file
is pinned in the committed case manifest to SHA-256
`53ceaed3e599c88c709c6275325c2169dc165133985dc6b27e8991a9f938386e`.
Retained-evidence comparison also passes the new case hash, coverage and phase
checks. The tested x86_64 recorder remains `e0a57a43…`, source `897decb2`.
Evidence is under `build/laser-extra-20261014/paired/`; the owned temporary
runtime and Wine prefix are removed, leaving about 4.2 MiB of evidence.

Reproduce from the credited recording and privately supplied original assets:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --case extra-mokou --retail --output-dir build/laser-extra
```

This adds image evidence for the selected Extra scene. Other shots and
unobserved laser paths retain their own scope.

## Read and verify the evidence

`comparison.json` records the executable and replay identities, raw laser-state
differences, image hashes, beam-geometry pixel counts, and mean RGB errors.
Images and state packets remain in the output directory; retail evidence is
under `retail/`. Recompute the comparison:

With `--case`, the copied case manifest also pins the retail laser-state file
hash, selected stage/frames and expected phases. The comparator checks all
three before accepting matching images. The three original case hashes have
been revalidated against their retained retail packets. New exploratory
captures can become published cases after their canonical retail observation
is complete.

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
rasterization disabled. It reads the pool at `LinuxDevice::Present` entry,
after the draw chain. `--sample-interval 1` checks each distinct presented
gameplay counter; smaller intervals can discover shorter events.
`scout-summary.json` records the interval, boundary, sampled range, counter
gaps, serialized record size and stop reason. Detailed snapshots have a
32 MiB serialized limit; reaching it fails the capture. `lasers.json` retains
samples containing active slots. Counter gaps remain explicit, so an empty
survey describes observed presented frames rather than every calculation.
Choose frames with substantial on-screen geometry, then capture those exact frames
with rasterization and retail comparison. Sampling can miss shorter events.
New cases should identify the boss, phase, credited fixture, stage, and frame,
and retain paired state and image evidence before entering the manifest.

### Additional discovery paths

The default scout samples `BulletManager::lasers`. Three other target helpers,
`0x00424730`, `0x00424820` and `0x00424910`, use enemy position and VM rotation
when calling `Player::CalcLaserHitbox @ 0x0044A6A0`. Track their invocation
ranges separately:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --fixture lunatic-0-b --stage 5 --scout \
  --sample-interval 30 --watch-rotating-hitboxes \
  --output-dir build/rotating-hitbox-scout
```

The optional observer records helper counts and sampled enemy/VM words under
`rotating-hitboxes.json`, with a shared 32 MiB record budget. Its boundary is
helper entry, distinct from the pool scout's Present boundary. Three completed
surveys on the filter-repaired recorder found no helper invocations: Stage 5
with a 20,606 frame limit, Final B and Extra with 30,000 frame limits. These
surveys validate breakpoint setup; a positive invocation is still needed to
exercise the enemy/VM sampling branch. A zero pool count describes that pool's
sampled frames, while helper and unobserved-frame coverage remain separate.

Evidence is under `build/surface-filter-repair-20261014/` in
`stage5-hitbox-scout/`, `finalb-hitbox-scout/` and `extra-hitbox-scout/`.
