# Portable draw-input comparisons

Use this diagnostic when two portable images differ at the same presented
replay frame. It records inputs to the Linux D3D8 adapter before OpenGL draws:
vertices, FVF, primitive layout, viewport, matrices, render/texture states and
texture-content hashes. CPU floating-point registers are recorded separately.
All vertex and matrix words retain their IEEE-754 bits.

## Capture and compare

Use Debug products and the same recording, stage, profile and font environment.
For example, capture the selected Border Team Lunatic Final B frame:

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate /path/to/x86_64/th08-modern --game-data-dir /path/to/game-data \
  --fixture lunatic-0-b --stage 4 --frames 15360 --watch-draws \
  --output-dir build/draw-x64
python3 scripts/analysis/check-laser-visuals.py \
  --candidate /path/to/i386/th08-modern --game-data-dir /path/to/game-data \
  --fixture lunatic-0-b --stage 4 --frames 15360 --watch-draws \
  --output-dir build/draw-i386
python3 scripts/analysis/compare-portable-draws.py \
  --reference build/draw-x64 --candidate build/draw-i386 \
  --output build/draw-input-comparison.json
```

The observer arms at `AnmManager::ClearVertexBuffer`, before the selected draw
chain, and stops after Present. Textures are hashed on first use and after dirty
updates. Captures allow at most 10,000 calls per frame, 30,000 vertices per call,
and 16 MiB per texture read. The comparator checks frame/asset/profile identities
and word layouts. Its report counts every difference while retaining at most
1,000 detailed entries per frame. A different result exits with status 1.

Submission order matters. Different late HUD batches can shift call indices;
inspect the earlier scene calls independently before interpreting later changes.

## Stage 4B frame 15,360, October 14, 2026

The tested x86_64 and i386 products attest identical reconstruction and adapter
source bytes. Their executable hashes are respectively
`62386854ab8e486a8ad742c97f27b7f2063d39e008aefc4af8600dd083b9aecc` and
`a82e7155eb7c71e5db645df9294cd5316e895df5f35dcbce696946ba4ba9de27`.

The captures contain 30 and 26 calls. In background call 2, all positions, UV,
matrices, viewport, texture bytes and render/texture states agree. The only
differences are 195 diffuse RGB words whose alpha bytes are zero. Later HUD
batches also differ, so this observation covers the background call rather than
the whole stream.

`Background::RenderObjects @ 0x0040A1B0`, mapped size `0x128A`, has two far-fog
branches that write alpha only. Canonical 1.00d disassembly at `0x0040AD9C`
zeros `[ebp-0xF9]` and copies it to `[ebp-0x115]`; `0x0040B123` zeros
`[ebp-0xC1]` and copies it to `[ebp-0xDD]`. Those paths leave the RGB bytes
unchanged. The reconstructed billboard-segment branch does the same. Its local
vertex colors retain stack contents when this branch does not assign RGB.

A render-data control borrowed only those zero-alpha RGB words from the x86_64
capture. Every other input had to match, and the original bytes were restored
after the one modified call. The ordinary paired playfield differed at 7,588
pixels, with mean RGB error 0.074071. The control's entire `(32,16)`–`(416,464)`
playfield is pixel-identical to x86_64. This establishes that the retained RGB
values affect interpolation along the background columns even though their
vertex alpha is zero.

The observation is inherited from the target's alpha-only path. Reconstruction
source and target bytes remain unchanged. This is separate from the repaired
packed-color and raster-edge defects; it supports no new exact-match claim.
Local evidence is under `build/draw-state-20261014/`, including the target-safe
disassembly, both ordinary captures, input comparison and color control.

## Reproduce the color control

```bash
python3 scripts/analysis/check-laser-visuals.py \
  --candidate /path/to/i386/th08-modern --game-data-dir /path/to/game-data \
  --fixture lunatic-0-b --stage 4 --frames 15360 --watch-draws \
  --draw-colors-from build/draw-x64 --draw-color-index 2 \
  --output-dir build/draw-i386-color-control
```

This explicitly experimental capture replaces render-buffer RGB for one call.
The guard accepts only matching 28-byte XYZRHW/diffuse/TEX1 layouts, identical
nonvertex inputs and position/UV words, and equal zero alpha. It rejects opaque
color changes. The report records the reference hash, changed-word count and
modification/restoration counts. Ordinary image and draw gates reject these
controlled captures; compare their images separately to assess the hypothesis.
