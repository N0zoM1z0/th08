# Portable runtime review

The 2026-10-12 review confirms a HOME screenshot defect and extends the known
background perspective defect to Stage 1, Stage 3, and Final B. It also isolates
a fullscreen display mismatch in Xvfb without a window manager. These findings
use the current x86_64 Linux renderer with llvmpipe software rendering;
production C++ remains unchanged.

## HOME screenshots

Pressing HOME during replay saves an old loading image. The next presented frame
is overwritten by that same image. This reproduces in both windowed and
fullscreen configurations at Stage 1 active frame 600 of `lunatic-0-b`.

`GameWindow::Present` invokes `Supervisor::TakeSnapshot`, which locks the
backbuffer with flags `0`. `LinuxSurface::LockRect` refreshes its CPU pixels only
for `D3DLOCK_READONLY`, so this call reads the cache left by an earlier upload.
`UnlockRect` then marks those pixels dirty. On the next presentation,
`FlushBackbuffer` uploads the old image over the newly rendered frame.

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
  --output-dir build/portable-display-check
```

The tool starts the committed, credited recording with normal life rules and
muted audio. It sends a physical HOME key through XTest, records the actual
screenshot, and captures the surrounding frames. OpenGL readback leaves the CPU
cache untouched, preserving the failure being measured. PNGs, window geometry,
SDL dimensions, state packets, hashes, and compressed logs remain in the output;
temporary runtimes are removed. `--modes windowed` selects the windowed check.

The repair belongs in the portable surface lock/cache lifecycle. Regression
coverage should include screenshot reads, read-only locks, partial writes, and
backbuffer uploads used during loading and transitions.

## Display under Xvfb

Without a window manager, the fullscreen configuration reports a 1024×768 SDL
window and drawable, while X exposes a 640×480 window. Presentation scales the
640×480 framebuffer to the reported larger dimensions, and the X window shows
only part of it. The image loses the right HUD and part of the lower playfield.

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
  --profiles fast --retail --clip-w-probe \
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

| Scene | Internal stage index | Active frame | Current mean RGB error | Clip-W variant mean RGB error |
| --- | ---: | ---: | ---: | ---: |
| Stage 1 | 0 | 600 | 2.695101 | 0.542699 |
| Stage 3 | 2 | 600 | 7.406285 | 2.134186 |
| Final B | 7 | 1,800 | 17.214737 | 1.000223 |

Errors use the playfield rectangle `(32, 16)`–`(416, 464)` on the 0–255 channel
scale. Residual differences need focused sampling, fog, and 2D rendering checks.
These point captures establish the shared perspective defect; the clip-W variant
is still an experiment.

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

The existing six bounded render windows and 60-second sanitizer smoke remain
useful runtime coverage. The complete 108-recording, 25-field parity result is
for the VC7 product. Portable needs its own complete state-parity run, longer
sanitizer replay checks, and runtime testing on AArch64 hardware. Audio behavior
also needs a separate pass because these replay investigations run muted.
