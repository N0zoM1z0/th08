# Stage 5 background perspective

The portable renderer loses homogeneous clip `W` when submitting 3D vertices.
At Stage 5 frame 6,166, this distorts wall and floor textures and changes
clipping near the camera. An isolated renderer variant that retains `W`
restores the corridor perspective. The production renderer awaits a repair.

## Reproduce the investigation

Use a native-layout Linux x86_64 Debug build, the canonical Japanese 1.00d
executable in `resources/th08.exe`, and the original `th08.dat` and `thbgm.dat`.
The method requires GDB, Wine with 32-bit support, Xvfb, xdotool, ImageMagick,
and Python Pillow alongside the normal portable build dependencies.

```bash
scripts/build-portable-linux.sh x86_64
python3 scripts/analysis/check-stage5-background.py \
  --portable build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --retail --clip-w-probe \
  --output-dir build/stage5-background-check
```

The committed `lunatic-0-b` recording is `th8_ud2ce5.rpy`, credited to
てーる in the [corpus](../tests/fixtures/replays/th08-1.00d/README.md).
Its SHA-256 is
`004f58e1b95655664087d6613780f3e6e16e25d0c55e5c17b2e2291d5446ae6b`.
Every capture starts at Stage 5, follows normal life rules, disables audio,
and stops at active frame 6,166. Portable captures run serially at accelerated
and ordinary pacing; retail uses ordinary pacing. `--profiles fast` or
`--profiles paced` selects one portable profile. Each invocation needs a fresh
output directory.

The output contains complete 640×480 PNGs, camera/script state, view and
projection matrices, sampled 3D draw vertices, hashes, and `summary.json`.
The summary compares the playfield rectangle `(32, 16)`–`(416, 464)` and derives
clip `W` from the captured vertices and matrices. Logs and traces are compressed;
temporary runtimes, the owned Wine prefix, and experimental build intermediates
are removed. The probe executable and its source variant remain available.

Portable direct-stage capture skips the cosmetic boss-name texture copy through
the existing external GDB procedure. Both portable profiles and the probe use
the same setting. Retail gameplay memory and executable bytes remain read-only;
its observers use hardware breakpoints.

## Evidence

The 2026-10-12 checkpoint uses source `59e239b4` and portable executable SHA-256
`cc5e11da46bbdcb237d1b9218992432787aa085db78253112084f01bbcee5d07`.
Local evidence is in `build/background-perspective-20261012/`.

At frame 6,166:

- Portable accelerated and ordinary captures have identical camera packets and
  playfield pixels. Playback pacing does not explain this frame's difference.
- Retail and portable agree on background script instruction 38, frame counter
  6,166, script/interpolation timer bytes, stage position, camera inputs, and
  viewport `(32, 16, 384, 448)`. Derived camera vectors differ by at most
  `5.96e-8`; view/projection differences are at most `6.11e-5` and `1.91e-6`.
- Actual background draws use `D3DFVF_XYZ | D3DFVF_TEX1` (`0x102`). One quad's
  clip `W` ranges from about 674 to 1,058. Another crosses the camera plane,
  with two negative and two positive values.
- The probe retains the original camera packets. Playfield mean absolute RGB
  error against retail falls from **14.188657** to **2.187907** on the 0–255
  channel scale. The corridor geometry and texture perspective recover;
  sampling and other rendering differences remain visible in pixel comparison.

The earlier native VC7 comparison at this same frame has zero differing
playfield pixels against retail. Its fresh 6,166-calculation prefix agrees on
all 25 recorded fields. That evidence is in
`build/replay-stage5-screenshot-20261012/`; the broader native replay gate is
documented in [REPLAY_TESTING.md](REPLAY_TESTING.md).

## Cause

`LinuxDevice::TransformPosition` applies world, view, and projection matrices,
then divides the coordinates by clip `W` and returns screen coordinates.
`LinuxDevice::Draw` submits those coordinates with `glVertex3f` under an
orthographic projection. `glVertex3f` supplies `W = 1`, so OpenGL receives none
of the original vertices' differing depth weights.

OpenGL uses clip `W` for perspective-correct interpolation and clipping before
the perspective divide. Replacing every vertex's value with one makes texture
interpolation affine and changes triangles that cross the camera plane. See
OpenGL 2.1 sections 2.7, 2.12, and 3.5.1 in the
[Khronos specification](https://registry.khronos.org/OpenGL/specs/gl/glspec21.pdf).

The target camera path is `Background::SetCamera2 @ 0x0040B6D0`; its disassembly
confirms camera offset `0x6394`, viewport dimensions, and the D3DX matrix calls.
The retail packet is read after this function's matrix setup. Those observations
and the unchanged probe camera packets locate the defect in the portable vertex
submission path.

## Repair boundary

Keep homogeneous coordinates through viewport conversion and submit all four
components with `glVertex4f`, preserving the D3D depth-range conversion. The
probe implements this for XYZ vertices and retains current XYZRHW behavior.
Production work should cover XYZRHW reciprocal depth, 2D pixel alignment,
near-plane crossings, texture interpolation, and fog. Validate other stages
with the same camera/frame method and the existing portable replay windows.

This investigation establishes the cause of the large Stage 5 perspective
difference. Its single-frame probe records a substantial error reduction;
complete portable pixel parity remains a separate validation task.
