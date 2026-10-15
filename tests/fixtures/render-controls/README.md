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
