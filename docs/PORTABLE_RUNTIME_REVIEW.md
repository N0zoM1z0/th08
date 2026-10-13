# Portable runtime review

The initial 2026-10-12 review confirmed a HOME screenshot defect, extended the
background perspective defect to Stage 1, Stage 3, and Final B, and isolated a
fullscreen mismatch under Xvfb. All three now have production repairs. See
[PORTABLE_RENDERING.md](PORTABLE_RENDERING.md) for regression commands and
results; the following sections preserve observations from the preceding
x86_64 Linux renderer with llvmpipe software rendering.

Complete replay-state testing subsequently found an `atan2` overload difference
that changed a Magic Team Stage 4A item award. The
[vector precision investigation](PORTABLE_VECTOR_PRECISION.md) records the
first differing calculation, the enemy/item trajectory, and the repair.

## HOME screenshots

Pressing HOME during replay saved an old loading image. The next presented frame
was overwritten by that same image. This reproduced in both windowed and
fullscreen configurations at Stage 1 active frame 600 of `lunatic-0-b`.

`GameWindow::Present` invokes `Supervisor::TakeSnapshot`, which locks the
backbuffer with flags `0`. The preceding `LinuxSurface::LockRect` refreshed CPU
pixels only for `D3DLOCK_READONLY`, so this call read an earlier upload's cache.
`UnlockRect` marked those pixels dirty. On the next presentation,
`FlushBackbuffer` uploaded the old image over the newly rendered frame.

The saved BMP and the overwritten frame each match the old CPU cache pixel for
pixel. The saved screenshot differs from the requested game frame at 307,022
pixels in the windowed run. The window itself matches the rendered framebuffer
exactly before HOME is pressed.

Reproduce with a native-layout x86_64 Debug executable, the original game data,
GDB, Xvfb, xdotool, ImageMagick, and Python Pillow:

```bash
python3 scripts/analysis/check-portable-display.py \
  --portable build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --output-dir build/portable-display-check --verify
```

The tool starts the committed, credited recording with normal life rules and
muted audio. It sends a physical HOME key through XTest, records the actual
screenshot, and captures the surrounding frames. OpenGL readback leaves the CPU
cache untouched, preserving the failure being measured. PNGs, window geometry,
SDL dimensions, state packets, hashes, and compressed logs remain in the output;
temporary runtimes are removed. `--modes windowed` selects the windowed check.

The repaired lock/cache lifecycle refreshes all backbuffer reads and commits
writable unlocks immediately. Pixel contracts cover screenshot reads, read-only
locks, partial writes, subsequent draws, and viewport-independent uploads.

## Display under Xvfb

Without a window manager, the preceding fullscreen configuration reported a
1024×768 SDL drawable while X exposed a 640×480 window. Presentation scaled the
framebuffer to the reported larger dimensions, and the X window showed only
part of it, losing the right HUD and part of the lower playfield. Creating the
fullscreen window at desktop dimensions resolves that mismatch.

Explicit windowed mode reports 640×480 in both APIs and produces zero differing
pixels between framebuffer and visible-window captures. The same-frame replay
tool now writes a windowed, muted configuration so its window geometry agrees
with the native observer. Desktop fullscreen behavior with a window manager
remains a separate check; the captured mismatch establishes the Xvfb case.

## Additional stages

The [background study](PORTABLE_BACKGROUND.md) now accepts `--fixture`, `--stage`,
and `--frame`. For example, compare Final B at frame 1,800:

```bash
python3 scripts/analysis/check-stage5-background.py \
  --portable build/portable-linux-x86_64/th08-modern \
  --game-data-dir "/path/to/original/TH08 directory" \
  --fixture lunatic-0-b --stage 7 --frame 1800 \
  --profiles fast --retail \
  --output-dir build/final-b-frame-check
```

The executable in `resources/th08.exe` must be the hash-attested Japanese 1.00d
target. Retail observes calculation state and captures the visible window using
hardware breakpoints. The Final-stage observer disables completion polling when
arming the bounded screenshot, keeping its observers within four x86 hardware
breakpoint slots.

The same loss of clip `W` distorts backgrounds in all three additional samples.
Retaining `W` in the isolated renderer restores their perspective. Camera/script
state agrees between the production renderer and its variant; retail matrix
differences remain within small floating-point rounding differences.

| Scene | Internal stage index | Active frame | Pre-fix mean RGB error | XYZ-only probe mean RGB error |
| --- | ---: | ---: | ---: | ---: |
| Stage 1 | 0 | 600 | 2.695101 | 0.542699 |
| Stage 3 | 2 | 600 | 7.406285 | 2.134186 |
| Final B | 7 | 1,800 | 17.214737 | 1.000223 |

Errors use the playfield rectangle `(32, 16)`–`(416, 464)` on the 0–255 channel
scale. Residual differences need focused sampling, fog, and 2D rendering checks.
These point captures establish the shared perspective defect; the clip-W variant
was an isolated experiment; the production repair also handles pixel centers and
XYZRHW depth weights.

The frame tool now keeps the normal boss-name texture copy. Stage 5 also reaches
frame 6,166 with that path enabled, reproducing the same isolated color warning
without a crash. Earlier render windows skipped the copy. The explicit
`--skip-enemy-name-copy` option preserves that diagnostic setting and records it
in metadata.

## Evidence and coverage

Local evidence is in `build/portable-runtime-audit-20261012/`. The ordinary
portable executable has SHA-256
`cc5e11da46bbdcb237d1b9218992432787aa085db78253112084f01bbcee5d07`.
The `lunatic-0-b` fixture is pinned and credited in the
[replay corpus](../tests/fixtures/replays/th08-1.00d/README.md).

Subsequent checks passed all 108 recordings on both Linux x86_64 and i386,
with all 25 fields equal over 12,311,460 calculations per product. The
[validation results](REPLAY_RESULTS.md) summarize those complete gates.
The repaired renderer's six-window gate and 90-second Stage 5 sanitizer audit
are recorded in [PORTABLE_RENDERING.md](PORTABLE_RENDERING.md). AArch64 hardware
gameplay and audio output remain separate checks; these investigations ran muted.
