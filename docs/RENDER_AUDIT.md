# Replay-driven render audit

The render audit is a deterministic regression oracle for modern TH08 ports.
It answers whether an enemy draw has valid source data and whether that draw
actually changes the expected framebuffer region. It does not try to decide
visual parity from a screenshot or replace a final graphical play-test.

The checker and CSV contract are platform-neutral. The first backend adapter is
implemented by the SDL/OpenGL Linux port; a future Windows, macOS, or web
backend can emit the same schema and reuse the checker and recorded scenarios.

## Run a recorded stage

Build the native Linux product, then supply an original data directory and a
replay recorded by the reconstructed game:

```bash
scripts/build-portable-linux.sh x86_64
scripts/audit-render-replay-linux.sh \
  build/portable-linux-x86_64/th08-modern \
  "/path/to/original/TH08 directory" \
  "/path/to/th8_01.rpy" \
  3 180
```

The stage index is zero-based and the final argument is the maximum wall-clock
capture window in seconds. The launcher creates an isolated directory under
`build/`, links only the two original DAT archives, copies the replay, and uses
Xvfb plus llvmpipe. Replay time advances without the normal 60 Hz wall-clock
wait, so a headless audit can cover several gameplay minutes quickly. Set
`TH08_RENDER_AUDIT_FAST=0` when reproducing a timing-sensitive observation. A
clean game exit is accepted; crashes, debugger-visible fatal signals, too few
samples, and hard checker failures are not.

The latest portable outputs are:

- `build/render-audit-last.csv`
- `build/render-audit-last-files.txt`
- `build/render-audit-last-runtime.log`

Set `TH08_KEEP_RENDER_AUDIT_DIR=1` to retain the isolated runtime directory.
Normal gameplay pays no probe or readback cost because the feature is disabled
unless `TH08_RENDER_AUDIT=1` is present. `TH08_LINUX_RENDER_AUDIT=1` remains a
compatibility alias for early local scripts.

The test launcher uses GDB to isolate two test-only actions from the product.
It skips a cosmetic enemy-name texture copy when starting directly in a later
stage; that original routine reads outside the stage-local sprite allocation
and otherwise depends on prior heap history. By default GDB also returns
immediately from negative `GameManager::AddLives` calls so a replay captured
during no-life-decrement testing reaches its later scenes. Set
`TH08_RENDER_AUDIT_KEEP_LIVES=0` to retain normal life rules. Neither action
changes the executable, packaged launcher, CI artifact, or release behavior.

## Two independent observations

Each sampled primary enemy or boss VM produces two observations.

1. The semantic observation validates `loadedSprite`, its texture, UV region,
   geometry, visibility, and draw result. The backend reads the CPU-side
   texture pixels in the selected UV rectangle.
2. The pixel observation flushes pending vertices, reads the clipped viewport
   rectangle, performs the real draw, flushes it, and reads the same rectangle
   again. The report records how many pixels and RGB values changed.

The source statistics are calculated twice: once from the raw texture and once
after the VM-selected color (`color1` or `color2`) and the active global ANM
mix color. This distinction matters during Bombs, damage flashes, dialogue,
stage tinting, and fades. A colorful source intentionally rendered with a very
dark tint is not automatically a backend color-loss failure.

Bosses are sampled every 15 frames. Other enemies are distributed over a
60-frame period using their pool index so the audit observes formations without
stalling every draw on `glReadPixels`.

## CSV schema version 1

Every row has `schema_version=1`. The fields form four groups:

| Group | Representative fields | Purpose |
| --- | --- | --- |
| Identity | `stage`, `frame`, `enemy_index`, `boss`, `draw_group` | Reproduce the scene and identify the owner |
| ANM state | `anm_file`, `script`, `sprite`, `loaded_anm`, `flag17`, `color1`, `color2`, `render_color` | Distinguish lookup, script, pointer, and tint failures |
| Geometry | `x`, `y`, `anchor`, `rotation`, `screen_x`, `screen_y`, `u0`–`v1` | Preserve anchor, shake-adjusted bounds, and scrolled UV evidence |
| Source | `source_visible`, `source_colorful`, `expected_visible`, `expected_colorful` | Prove that the selected UV region contains drawable data |
| Framebuffer | `draw_result`, `probe_pixels`, `changed_pixels`, `changed_chromatic`, `absolute_rgb_difference` | Prove that the queued draw reaches pixels |

Colors are D3D `AARRGGBB` values. Geometry and UV fields preserve enough state
to correlate a report with a screenshot or debugger watch without dumping game
assets.

## Status and acceptance policy

The checker treats these as immediate structural failures:

- `missing-sprite`
- `missing-texture`
- `invalid-geometry`
- `source-unavailable`
- `empty-source`

`not-queued`, `no-pixel-delta`, `color-loss-suspect`, and
`white-output-suspect` are suspects. A single sample can be caused by depth,
overlap, or a state boundary, so normal mode prints it for investigation.
Three or more zero-delta samples for the same stage/ANM/script/sprite become a
failure when they outnumber successful samples by at least three to one. Use
`--strict-suspects` when establishing a golden scene that should have no
transient suspects. Unknown schema versions, columns, or status labels fail
closed instead of silently weakening the policy.

`offscreen`, `partially-offscreen`, `unprobed`, `vm-transparent-output`, and
`vm-tinted-output` are diagnostic classifications, not rendering failures.
Keeping them explicit prevents a clipped spawn or intentional script tint from
weakening the real failure categories.

The checker contract has target-independent tests:

```bash
python3 scripts/test-render-audit.py
```

They cover success, structural failure, repeated no-op draws, transient
recovery, strict suspects, and required-stage enforcement. `scripts/ci.py`
runs the contract tests without requiring copyrighted data.

### Compare two ports

Once two backends have audited the same replay/stage window, compare their
reports directly:

```bash
python3 scripts/compare-render-audits.py \
  build/audit-i386.csv \
  build/audit-x86_64.csv
```

Rows align by stage, frame, and enemy-pool index. The comparator requires at
least 95% sample overlap by default, then checks ANM identity, VM colors,
geometry/UVs, raw and modulated source statistics, draw result, status class,
and whether a fully probed draw changed pixels. Exact framebuffer pixel counts
are intentionally not required across APIs because rasterization and filtering
can differ at edges. Use `--min-overlap`, `--float-tolerance`, and
`--max-differences` only with a documented platform reason.

This A/B path is stronger than treating one backend as visually self-consistent:
it can expose a native-layout field error that still points to a valid but wrong
sprite, color, or UV region.

Replay arithmetic can legitimately diverge between floating-point ABIs before
the requested scene. Treat a low overlap or broad geometry mismatch as a real
diagnostic result, not a reason to lower thresholds until the reports pass.
For a bounded compiler/runtime fix, a before/after report from the same
architecture is the strongest alignment; use the other architecture to show
that the failure class is absent there.

## Porting the oracle to another backend

A backend implementation needs only a small adapter around the shared CSV
contract:

1. expose texture-region RGBA data for a loaded ANM texture;
2. flush pending game draws before each read;
3. read an RGBA rectangle before and after the selected draw;
4. clip probes to the active D3D viewport;
5. emit schema version 1 with the same statuses.

The existing `TH08_AUTOPLAY_REPLAY` and `TH08_AUTOPLAY_STAGE` bootstrap is
backend-independent under `TH08_MODERN_PORT`; it starts a replay stage through
the normal ReplayManager path without menu automation. Backend-specific launch
scripts should keep data and output isolated in the same way as the Linux
launcher.

For native D3D8, use a lockable/readback render-target copy rather than changing
game drawing code. For a Metal, Vulkan, or web backend, stage the small probe
rectangle to CPU-visible memory after the relevant command submission. Preserve
the before/draw/after ordering even when the API is asynchronous.

## Limits

The current hook covers primary enemy and boss VMs. It does not yet cover
background layers, bullets, laser geometry, player/bomb effects, secondary
enemy VMs, dialogue composition, or HUD elements. Framebuffer deltas prove that
a draw reached pixels; they do not prove the exact blend equation, filtering,
z-order, or artistic appearance. Add focused probes for those owners instead
of broad full-frame image thresholds, and retain at least one real graphical
full-route test for release validation.
