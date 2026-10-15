# Portable rendering regression checks

The Linux renderer preserves homogeneous clip `W` for XYZ vertices and the
reciprocal depth of XYZRHW vertices. Both paths account for Direct3D's integer
pixel centers. Backbuffer locks read current GPU pixels; writable unlocks commit
updates immediately, and read-only unlocks leave subsequent rendering intact.
The offscreen buffer stores its logical top at the lowest OpenGL row in both
context profiles. Transformed vertices apply the half-pixel correction in the
projection matrix and use the D3D viewport dimensions. Where supported, the
backend selects the first-vertex convention used by D3D.
Fullscreen windows start at desktop dimensions so Xvfb presentation uses the
actual drawable size.

## Run the regression suite

Build a native-layout x86_64 Debug executable, then run:

```bash
scripts/build-portable-linux.sh x86_64
python3 scripts/test-portable-rendering.py \
  --candidate build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --output-dir build/portable-rendering-check
```

The suite requires the canonical Japanese 1.00d executable in
`resources/th08.exe`, original data archives, GDB, Wine32, Xvfb, xdotool,
ImageMagick, Python Pillow, and the normal Linux build dependencies. Audio is
muted, life rules are unchanged, and boss-name copying follows the product path.

It runs independent pixel contracts, sends the actual HOME key in windowed and
fullscreen modes, and compares four credited replay frames with retail. Stage 5
also runs at ordinary and accelerated pacing, requiring equal camera packets
and playfield pixels. The selected-frame regression limits are a mean RGB error
below 1.0 on the 0–255 channel scale and camera/matrix differences at most 0.001.
Script/timer state must agree exactly. These limits catch the preceding
perspective defects while allowing the small retained rendering differences.

Every capture records executable, recording, archive, and observer hashes.
Images, state packets, compressed logs, and comparison summaries remain under
the requested fresh output directory. Owned game runtimes and Wine prefixes are
removed. The fixture is `lunatic-0-b`, pinned and credited in the
[replay corpus](../tests/fixtures/replays/th08-1.00d/README.md).

Use `--cases stage0-frame600,stage7-frame1800` to select frames. After separately
running the renderer and HOME checks, `--frames-only` selects frame comparisons.
`--reference-captures build/portable-rendering-check` reuses that run's retail
captures after validating target, recording, archive, stage, and frame metadata;
copied evidence receives per-file hashes.

## Pixel contracts without game assets

```bash
python3 scripts/test-portable-renderer.py
```

This compiles the production Linux backend and exercises rendered pixels through
its D3D8 API. Checks cover flag-0 reads, read-only locks, immediate partial writes,
subsequent draws, XYZ and XYZRHW texture interpolation, pixel centers, negative-W
clipping, depth testing, linear fog, and viewport-independent CPU uploads.
The seven groups also check a non-default depth range across XYZ/XYZRHW
transitions, nonunit RHW, disabled-depth clipping, top/left edge inclusion, bottom/right exclusion,
asymmetric viewport clears, framebuffer audit coordinates, and actual Xvfb
window pixels after presentation. The window check uses X11 readback and needs
the `x11` development package alongside SDL2 and OpenGL.
The x86_64 GitHub build runs these checks without copyrighted game data.

For an i386 host build, select the multilib package directory and a compiler
wrapper that supplies `-m32`. Include `fpu` to check the device's x87 precision
and `D3DCREATE_FPU_PRESERVE` behavior:

```bash
mkdir -p build
cat > build/renderer-cxx-i386 <<'SH'
#!/bin/sh
exec g++ -m32 "$@"
SH
chmod +x build/renderer-cxx-i386
PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig \
  python3 scripts/test-portable-renderer.py \
    --compiler build/renderer-cxx-i386 \
    --cases locks,perspective,clipping,depthrange,fog,edges,orientation,fpu \
    --output-dir build/renderer-i386
```

Add `--legacy-gl` with a separate output directory for the older context.
`--pkg-config` selects an alternative package-metadata command when the compiler
uses a container or sysroot. Keep the compiler and package metadata on the same
architecture. The test executable runs on the host; reports identify the actual
compiler and OpenGL context.

The observed-quad sampling probe accepts the same compiler and package
commands. With a completed Stage 3 `--watch-quads` capture, use the wrapper
above to exercise i386:

```bash
PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig \
  python3 scripts/analysis/probe-subpixel-quads.py \
    --capture build/laser-quads --compiler build/renderer-cxx-i386 \
    --check-coverage --output-dir build/subpixel-i386
```

For a container compiler, also pass its package wrapper with `--pkg-config`.
The report preserves both command-file hashes, the complete portable compile
commands and compiler identity. Add `--legacy-gl` to repeat the older profile.

Exercise the OpenGL 2.1 path without clip control:

```bash
python3 scripts/test-portable-renderer.py --legacy-gl
python3 scripts/test-portable-renderer.py --legacy-gl \
  --baseline-ref da521dcf --cases edges --expect-failures \
  --output-dir build/portable-renderer-legacy-baseline
```

The runner forces Mesa's 2.1 profile and disables `GL_ARB_clip_control`.
Each contract reports the actual context version and extension availability.
`report.json` retains source and binary hashes, compiler identity, and each
contract's output and exit status. Linux CI runs both profiles and uploads the
reports.
The second command reproduces the pre-repair top-edge failure. The repaired
renderer stores the logical top at the bottom of its OpenGL FBO, then
maps readback, CPU writes, clears, audit regions, and presentation consistently.

The viewport-depth regression also has a reproducible failure control:

```bash
python3 scripts/test-portable-renderer.py \
  --baseline-ref d44868af --cases depthrange --expect-failures \
  --output-dir build/portable-renderer-depth-baseline
```

The preceding renderer places a farther XYZRHW quad in front of an XYZ quad
when the viewport depth range is `(0.2, 0.8)`. The current contract checks that
ordering, a nearer quad, the return to XYZ, and an out-of-range Z with depth
disabled.

Confirm the tests detect the preceding defects:

```bash
python3 scripts/test-portable-renderer.py \
  --baseline-ref 9cf5eae8 --expect-failures \
  --output-dir build/portable-renderer-baseline
```

All four baseline cases must fail at their named assertions. Compiler errors,
timeouts, and unrelated failures fail the control run.

## 2026-10-12 verification

Fresh x86_64 and i386 builds passed their ownership/layout checks. Four pixel
contract groups pass with the repaired backend and fail with the preceding
renderer. Actual HOME screenshots match the presented framebuffer exactly in
both display modes. Windowed presentation has zero differing pixels; fullscreen
Xvfb reports and displays 1024×768, with mean RGB error 0.123708 against the
independently scaled framebuffer.

| Scene | Stage index | Active frame | Before | Fixed production |
| --- | ---: | ---: | ---: | ---: |
| Stage 1 | 0 | 600 | 2.695101 | 0.005193 |
| Stage 3 | 2 | 600 | 7.406285 | 0.004768 |
| Stage 5 | 5 | 6,166 | 14.188657 | 0.292928 |
| Final B | 7 | 1,800 | 17.214737 | 0.323372 |

Values are mean absolute RGB differences in the playfield rectangle
`(32, 16)`–`(416, 464)`, using llvmpipe software rendering. The repaired Stage 5
ordinary/accelerated captures have identical playfield pixels and camera packets.
Small pixel differences remain, including in text regions; their causes need
separate measurement. The initial Stage 5 comparison used the diagnostic
boss-name bypass; repaired captures keep the normal copy.

The [laser visual check](PORTABLE_LASER_VISUALS.md) adds four later Stage 3
frames and four Marisa Stage 4B frames with matching raw laser and VM state.
The October 13 baseline failed seven strict beam-region image checks. All eight
pass after the Linux color, font-height, and edge-coverage repairs, using the
original 1.0 mean RGB error limit. The page records each repair, remaining font
and synthetic sampling differences, and the missing-beam control procedure.

All six existing replay windows pass with normal life rules and boss-name copying,
retaining 8,810 enemy-render samples. The Stage 5 window records nine heuristic
color suspects; their status remains unconfirmed. The recorded samples and
same-frame images support further visual checks.

The fresh AddressSanitizer artifact passes a 60-second title/demo smoke and a
90-second direct Stage 5 replay audit with the normal boss-name copy, retaining
1,891 render samples without sanitizer errors. Leak detection is disabled for
these bounded runs. The launcher rejects sanitizer reports and nonzero GDB
inferior exits, including zero-prefixed exit codes.

Local evidence is in `build/portable-render-fix-20261012/`. The ordinary portable
executable SHA-256 is
`a58ae75e570ad7bb650206a768284592b49ef0249d5bd39248553d987f28dc81`.
The source changes are confined to the Linux platform backend; target-facing
reconstruction source, layouts, and mapping ledgers retain the preceding native
checkpoint.

The subsequent Linux x86_64 and i386 state suites each pass all 108 recordings
over 12,311,460 calculations; see the [complete results](REPLAY_RESULTS.md).
AArch64 hardware gameplay remains to be verified. The [background investigation](PORTABLE_BACKGROUND.md)
and [runtime review](PORTABLE_RUNTIME_REVIEW.md) preserve the original findings.
