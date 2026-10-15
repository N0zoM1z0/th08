# Diagnostic viewport controls

These patches isolate viewport arithmetic in the subpixel API probe. They
apply to the Linux renderer at `02ff72f2` and are experimental inputs, kept
separate from the production renderer. The unmodified input uses the vertex
words captured from TH08 Stage 3 frame 12,900.

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
