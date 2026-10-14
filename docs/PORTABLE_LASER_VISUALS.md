# Laser visual regression

Compare the original Japanese TH08 1.00d and Linux portable images at selected
laser frames. The [replay results](REPLAY_RESULTS.md) establish gameplay trace
equality; this check adds laser-pool state and images from the presented frame.

## Current result, October 14, 2026

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
added x86_64 image cases with identical laser/VM packets. Other bosses and the full corpus for the revised
binaries remain separate checks. The dated sections below preserve the baseline
and each repair's evidence.

### Status of the seven initial failures

The seven failures belong to the October 13 baseline. A retained-image
revalidation against the original 1.0 limit gives the following results for
the font-repaired x86_64 recorder, SHA-256
`4cce0fe8ee633fa2b37713214793fef078395edbb303f52c0cdd98e4bc914cc6`.
All raw laser and VM packets still agree exactly with retail.

| Stage | Frame | Baseline mean RGB error | Current mean RGB error | Current result |
| --- | ---: | ---: | ---: | --- |
| 3 | 12,900 | 2.831371 | 0.694699 | Pass |
| 3 | 14,130 | 2.609911 | 0.317662 | Pass |
| 3 | 14,310 | 1.324700 | 0.003113 | Pass |
| 3 | 14,490 | 2.085793 | 0.711447 | Pass |
| 4B | 13,140 | 3.084492 | 0.004278 | Pass |
| 4B | 13,260 | 2.222010 | 0.001907 | Pass |
| 4B | 18,210 | 1.657255 | 0.021046 | Pass |

Stage 4B frame 15,360 is the eighth sample; it passed both builds at 0.003865.
The original Stage 4B failures came from packed-color conversion. Stage 3
also exposed font metrics/rasterization and the horizontal beam edge.
Passing the selected image limit allows small pixel differences. The remaining
title investigation now confirms an API filtering defect, described under
[title texture scaling](#remaining-title-texture-scaling); its contribution to
the actual game images still needs a repair-and-recapture comparison.
The fresh retained-image reports are under `build/surface-scaling-20261014/`.

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
108-recording refresh is active under `x64-suite/` in the phase directory.
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

The Linux renderer now selects the upper-left clip origin when
`GL_ARB_clip_control` is available. Its projection retains the existing image
orientation. Presentation and CPU backbuffer uploads use their lower-left
blit convention explicitly. The first repair kept the older path for contexts
without clip control; the [legacy gate below](#legacy-opengl-coverage-october-14-2026)
covers its follow-up repair. See the
[Direct3D filling convention](https://learn.microsoft.com/en-us/windows/win32/direct3d9/rasterization-rules)
for terminology and [ARB_clip_control](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_clip_control.txt)
for the OpenGL origin mapping. The measured oracle is Wine D3D8.

| Stage 3 frame | Before origin repair | After origin repair | Strict result |
| ---: | ---: | ---: | --- |
| 12,900 | 1.377827 | 0.786645 | Equal |
| 14,130 | 0.636065 | 0.324227 | Equal |
| 14,310 | 0.292456 | 0.003113 | Equal |
| 14,490 | 0.970204 | 0.766607 | Equal |

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
  --fixture lunatic-0-b --stage 2 --frames 12900 --watch-quads --retail \
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

## Legacy OpenGL coverage, October 14, 2026

Disabling `GL_ARB_clip_control` reproduces the pre-repair top-edge failure at
source checkpoint `da521dcf`. The renderer now stores the logical top at the
lowest FBO row in this mode. Readback, CPU writes, viewport clears, framebuffer
audit regions, and presentation use the same mapping. The clip-control path
keeps its existing orientation.

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
| 9,570 | 55 | 10,716 | 17.589088 | Different |
| 14,490 | 4 | 1,435 | 20.909872 | Different |
| 14,550 | 4 | 6,372 | 14.604154 | Different |
| 15,090 | 4 | 5,780 | 15.289389 | Different |
| 22,590 | 10 | 10,160 | 12.120407 | Different |

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
  --fixture lunatic-1-a --stage 6 --frames 9570 --watch-quads --retail \
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

## Final B precision observation, October 14, 2026

The Final B survey selected frames 8,400, 11,160, 11,490, 12,240, 14,280,
16,770 and 17,460 from `lunatic-0-b`. Fresh paired captures pass the original
image limit in all seven frames, with beam errors 0.003476–0.144060. The strict
packet gate fails at 8,400: slots 12 and 19 each differ by one ULP in their
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
Next trace the spawning operands and verify the precision repair against the
failed packet and fresh captures. The 25-field gameplay pass remains scoped
to its recorded fields.

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
