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
retail's exact substituted face has not yet been measured.

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
background and cap-region differences. Beam geometry overlaps those regions,
so these measurements need further localization before attributing an error
to laser rendering. These observations cover selected frames; later bosses,
other routes and difficulties, and Linux i386 images remain to be checked.

Local evidence is in `build/laser-visual-20261013/stage3-aligned/`. The existing
x86_64 Debug recorder artifact has SHA-256
`d74a574ac983d2df7c7a52cd421a868a01b90c34471881802ed83cea9e5b802f`;
recorder hooks were inactive for these GDB captures. The tested gameplay source
is the previously validated `af9cb1d3` checkpoint. No gameplay or renderer source
changed in this investigation.

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
The despawn frame passes the strict pixel limit; the remaining three require
further localization. Evidence is in
`build/laser-visual-20261013/stage4b-marisa/`. To reproduce this selection, use
`--case stage4b-marisa` with the capture command above.

The paired missing-beam control omits 10 on-screen quad submissions while
preserving the identical laser-state packet. Beam-region errors rise to
42.524421, 39.223917, 61.727536, and 63.994869; all four omission checks pass.
Control evidence is in `build/laser-visual-20261013/stage4b-control/`.

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
