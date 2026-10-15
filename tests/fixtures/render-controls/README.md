# Diagnostic rendering controls

These controls isolate viewport arithmetic, interpolation and texture storage.
Each section records its source checkpoint. The patches are experimental
inputs, kept separate from the production renderer. The subpixel probe uses
vertex words captured from TH08 Stage 3 frame 12,900.

The first two patches apply to the Linux renderer at `02ff72f2`.
The first patch moves the half-pixel adjustment from transformed vertices to
a floating viewport. The second also uses the D3D viewport dimensions. Both
retain the production legacy path. On the recorded Wine 9.0/Mesa environment,
they pass 53/68 and 54/68 complete image comparisons respectively; all 34
solid-material images agree. Neither resolves the textured cases.

With a completed `--watch-quads` capture at `build/laser-quads`, prepare a
normalized private source copy and apply each patch to a separate output:

```bash
mkdir -p build/viewport-controls
git show 02ff72f2:src/modern/linux/d3d8_compat.cpp \
  | sed 's/\r$//' > build/viewport-controls/renderer.cpp
for control in viewport-center viewport-center-and-size; do
  patch --batch --output "build/viewport-controls/$control.cpp" \
    build/viewport-controls/renderer.cpp \
    "tests/fixtures/render-controls/$control.patch"
  python3 scripts/analysis/probe-subpixel-quads.py \
    --capture build/laser-quads --check-coverage \
    --renderer-control "build/viewport-controls/$control.cpp" \
    --output-dir "build/viewport-controls/$control"
done
```

The runner captures the D3D8 reference, production renderer and named control
serially. Each report identifies the input words, source files, compiler
versions and image hashes. `--check-coverage` checks the production and control
solid cases. An optional pre-repair `--baseline-ref` remains a failure control.
Owned Wine prefixes and compiled probe executables are removed automatically.

The Wine 9.0 [viewport setup](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/state.c#L3896)
and [projection setup](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/utils.c#L5653)
informed these controls. Their observed pixels determine the results above;
Wine source alone does not establish a TH08 reconstruction match.

## Measured projection and first-vertex controls

The October 15 projection observer records 68 calls and two distinct matrices.
Their half-pixel correction is in the projection translation. This matches
Wine's offscreen path without clip control, rather than the floating-viewport
path tested above. `measured-projection.patch` uses those matrix expressions
and the actual D3D viewport. It passes 60/68 images. Applying
`first-vertex.patch` on top sets `GL_FIRST_VERTEX_CONVENTION` and passes 68/68
in both the default and forced OpenGL 2.1 contexts. All solid cases agree.

`first-vertex-only.patch` applies directly to the original renderer. It leaves
all 68 production images unchanged, at 53/68 reference matches. Both factors
are needed for the complete result in this probe. Wine 9.0 also selects the
first-vertex convention in its
[context initialization](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/context_gl.c#L2109).

```bash
patch --batch --output build/viewport-controls/measured-projection.cpp \
  build/viewport-controls/renderer.cpp \
  tests/fixtures/render-controls/measured-projection.patch
patch --batch --output build/viewport-controls/first-vertex.cpp \
  build/viewport-controls/measured-projection.cpp \
  tests/fixtures/render-controls/first-vertex.patch
python3 scripts/analysis/probe-subpixel-quads.py \
  --capture build/laser-quads --check-coverage \
  --trace-wine-shaders --trace-wine-projection \
  --renderer-control build/viewport-controls/first-vertex.cpp \
  --output-dir build/subpixel-measured-first-vertex
```

Repeat with `--legacy-gl` and a fresh output directory for OpenGL 2.1.
The GDB observer requires the installed Wine i386 DLL with SHA-256
`cd50dcd7ac2355f8ce6b429eaaf1e828ac1df656cc9846cce381122c41917ab4`.
It checks mapped entry bytes and uses hardware breakpoints across the Wine
render thread. `native/projection.json` preserves matrix words, floats and
DLL identity; `report.json` pins the observer and observation hashes. Shader
logs are compressed, with a 32 MiB file limit. Ordinary probe runs need no GDB.

These controls cover transformed quads with RHW=1, disabled depth and the two
synthetic materials. A production change also needs depth, other vertex
formats, render-target orientation and real-game regression checks. The
controls establish the local pixel result; broader renderer behavior remains
to be verified.

## Packed GPU texture control

`packed-textures.patch` applies to `2fd6a51b`. It selects `GL_RGB5_A1`,
`GL_RGB565` and `GL_RGBA4` for their corresponding packed D3D texture formats,
using the existing decoded upload bytes. The default-context filtering probe
passes 6,144/6,144 pixels with both production and this control. It covers four
formats, point/linear filters, subpixel phases, three blend modes and repeated
draws. The control gives no improvement in this experiment.

```bash
mkdir -p build/packed-texture-control
git show 2fd6a51b:src/modern/linux/d3d8_compat.cpp \
  | sed 's/\r$//' > build/packed-texture-control/renderer.cpp
patch --batch --output build/packed-texture-control/packed.cpp \
  build/packed-texture-control/renderer.cpp \
  tests/fixtures/render-controls/packed-textures.patch
python3 scripts/analysis/probe-texture-filtering.py --check \
  --renderer-control build/packed-texture-control/packed.cpp \
  --output-dir build/packed-texture-control/probe
```

This tests a GPU storage hypothesis for the small Final B residual. The
[Wine 9.0 format table](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/utils.c#L1574)
supplies supporting API evidence; it is independent of TH08 function matching.

## Floating-point alpha-test control

`float-alpha-test.patch` applies to renderer checkpoint `2fd6a51b`. It retains
fixed-function vertex processing and uses a small fragment shader for textured
MODULATE draws with alpha testing enabled. It compares the interpolated alpha
directly against `ALPHAREF / 255`, without rounding it to eight bits first.
The control supports the probe's single material and `GREATEREQUAL` comparison;
it aborts for fog, missing textures, other operations or other comparisons.
It is a diagnostic source copy, with one context per process.

```bash
mkdir -p build/alpha-controls
git show 2fd6a51b:src/modern/linux/d3d8_compat.cpp > build/alpha-controls/renderer.cpp
patch --batch --output build/alpha-controls/float-alpha.cpp \
  build/alpha-controls/renderer.cpp \
  tests/fixtures/render-controls/float-alpha-test.patch
python3 scripts/analysis/probe-texture-filtering.py \
  --alpha-test --renderer-control build/alpha-controls/float-alpha.cpp \
  --output-dir build/alpha-test-control
```

The October 15 run matches 6,128/6,144 production pixels and 6,144/6,144 control
pixels against Wine 9 D3D8. Both default and forced OpenGL 2.1 contexts give
these counts. Repeat with `--legacy-gl` and a fresh output directory. With
`--alpha-test`, `--check` deliberately fails for the current production
renderer; the report still retains both variants. Without `--alpha-test`,
the original 6,144-sample suite passes.

The profile uses alpha reference 4, `GREATEREQUAL` and diffuse alpha 40/255,
matching the low-alpha Final B draw. Wine's
[fragment shader generation](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/glsl_shader.c#L7508)
compares floating-point alpha; its
[reference conversion](https://github.com/wine-mirror/wine/blob/wine-9.0/dlls/wined3d/wined3d_private.h#L2915)
uses the same reference divided by 255. The
[OpenGL 1.4 specification, section 4.1.4](https://registry.khronos.org/OpenGL/specs/gl/glspec14.pdf)
describes rounding for the fixed-function alpha test. These sources explain
the control; observed API results establish the local discrepancy.

## Alpha-state matrix

The filtering runner also provides `--alpha-states`. This asset-free profile
checks all eight comparison functions against references 0, 4, 128 and 255.
Its six materials cover texture/diffuse modulation, texture-factor modulation,
untextured diffuse, independent color/alpha selection, SELECTARG2, and alpha
replication/complement. Each runs with transformed vertices or with XYZ
vertices and linear fog: 384 cases, 16 pixels each, 6,144 samples.

```bash
python3 scripts/analysis/probe-texture-filtering.py \
  --alpha-states --output-dir build/alpha-state-matrix
```

The current renderer matches 4,988/6,144 samples in the default context.
Most mismatches belong to SELECTARG2 and argument modifiers, which its
fixed-function material mapping does not implement. The profile also exposes
alpha comparison differences in ordinary texture/diffuse and texture-factor
materials. A private context-owned fragment-pipeline candidate matches all
6,144 samples; it remains separate from the frozen replay-gate products.
The candidate includes the required color/alpha sources, all comparison
functions and linear fog. Whole-game image gates still determine acceptance.

## Cached alpha pipeline

The production candidate on `fix/portable-alpha-test` specializes each used
material, alpha comparison and fog mode into a context-owned cached shader.
The reference and texture-factor values remain uniforms, updated on every
draw. Its default and GL 2.1 alpha-state matrices match all 6,144 Wine samples.
The observed Stage 3 quad probe also passes 68/68 images with alpha testing
enabled in the GL 2.1 context:

```bash
python3 scripts/analysis/probe-subpixel-quads.py \
  --capture build/laser-quads --alpha-test --legacy-gl --check-coverage \
  --output-dir build/alpha-quads-check
```

`--alpha-test` enables `GREATEREQUAL` with reference 4 in both APIs. The report
records that state separately from the older alpha-disabled observations.

Compare warmed rendering cost against the preceding fixed-function renderer:

```bash
python3 scripts/analysis/benchmark-alpha-pipeline.py \
  --baseline-ref 575c2368 --output-dir build/alpha-pipeline-benchmark
```

The benchmark interleaves three rounds per variant, after three warmup frames.
Each timed round draws 20 frames of 12 playfield-sized low-alpha layers.
`report.json` retains source/binary hashes, compiler commands, checksums and
all timings. Its clock covers drawing and GPU completion, excluding compilation
and warmup. `--reference-control` can add an independently generated source
copy, such as the narrow float-alpha patch above; the candidate must then
produce the same image checksum. Owned executables are removed after testing.
The fixed-function baseline keeps the known alpha-test discrepancy, so its
checksum differs. Timings describe this synthetic software-rendering workload;
actual scene gates remain required.

## Exhaustive packed-color precision

`probe-packed-precision.py` generates a 256x256 A4R4G4B4 texture containing all
65,536 source words. Four diffuse colors, three blend modes and one/four draws
give 24 cases and 1,572,864 RGB samples. Point sampling uses texel centers;
linear sampling shifts the quad by a quarter pixel. Both profiles match Wine
D3D8 in the recorded default-context investigation.

```bash
python3 scripts/analysis/probe-packed-precision.py \
  --filter point --check --output-dir build/packed-precision-point
python3 scripts/analysis/probe-packed-precision.py \
  --filter linear --check --output-dir build/packed-precision-linear
```

The report retains every case's mismatch count, up to 16 examples, source and
binary hashes, commands and complete sample hashes. Sample buffers stream in
256 KiB blocks; raw buffers, executables and the owned Wine prefix are removed
after comparison. `--renderer-control`, `--compiler`, `--pkg-config` and
`--legacy-gl` select other controlled variants. These tests cover the specified
colors, filtering offsets and blend states; actual sprite transforms remain a
separate measurement.

## Localizing one frame in a paired capture

`compare-sprite-quads.py --frame` validates the complete paired image/state and
quad coverage before selecting one frame. `--selection intersecting-bounds`
selects all sprite bounds touching the region, including narrow quads ending
inside it. It is a conservative spatial selection, rather than a rasterization
or visibility test. `--check` fails when any selected vertex word differs.

```bash
python3 scripts/analysis/compare-sprite-quads.py \
  --reference build/paired-quads/retail --candidate build/paired-quads \
  --frame 11490 --roi 306,178,308,180 \
  --selection intersecting-bounds --check \
  --output build/quad-pixel-inputs.json
```

The full capture stays in the report, alongside the selected frame and region.
A missing requested frame, malformed unselected quad or mismatched stage fails
validation. Differences outside the selected region remain outside this local
input comparison's claim.
