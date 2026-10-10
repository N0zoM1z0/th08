# Native replay comparison

Use the original Japanese TH08 1.00d executable as the runtime reference.
`test-native-replay.py` runs its bundled demos and the playable VC7 build in
sequence, records each calculation frame, and reports the first difference.
BGM and sound effects are off throughout. The run needs no menu input or
supervision.

## Run the comparison

Prepare the VC7 environment and your own game data using
[WINDOWS_I386_RUNTIME.md](WINDOWS_I386_RUNTIME.md). On Linux, the capture also
requires Wine with 32-bit support, GDB with Python support, and Xvfb.

```bash
python3 scripts/build.py --build-type bugfix --fresh -j1
python3 scripts/analysis/verify-windows-i386-runtime-data.py
python3 scripts/test-native-replay.py \
  --game-data build/modern-runtime/th08.dat \
  --bgm-data build/modern-runtime/thbgm.dat \
  --output-dir build/replay-check
```

Use a fresh output directory for each run. The default selects all three
bundled demos; `--demos 0` selects the Stage 5 Border Team Lunatic fixture.
The title screen starts each demo after its idle interval. Allow several
minutes per capture; the script prints progress and stops at its deadline.
`--display :88` selects an existing X display when needed.

Each capture uses its own Wine prefix, configuration, and data links. It
removes the prefix and copied runtime on exit, including failed runs. It
retains the trace and logs under `build/`; game data and personal saves stay
in their original locations. Run captures and VC7 builds sequentially.

To reuse a retail reference, pass `--reference build/replay-check/reference`
with a new output directory and the same `--demos` selection. The comparator
checks the fixture, game-data hash, muted configuration, schema, complete
demo sequence, and continuous frame coverage. Its exit status is zero only
when every recorded value agrees.

## Read the evidence

`candidate/comparison.json` contains the result, differing-frame counts, and
the first difference with nearby frames. Each capture records executable and
data hashes in `metadata.json`; candidate captures also record the linker-map
hash, source commit, and whether source/configuration changes were present.
`rows.jsonl` uses the field order in
[replay-schema.json](../scripts/replay-schema.json).

The observer reads memory at a hardware breakpoint immediately after
`Chain::RunCalcChain` returns from `GameWindow::Render`. It verifies the loaded
instruction bytes first. Candidate addresses come from the candidate's
linker map. Neither executable is patched, and gameplay receives its input
from the bundled replay.

The trace compares stage, replay and game frames, input, score, graze, deaths,
lives, bombs, power, player position/state, RNG seed/generation, point items,
time orbs, gauge, and clock. Floating-point values are compared by their exact
IEEE-754 bits. A death or score change is a failure only when the retail trace
differs. Demo 0 includes a death in the original game.

A passing result establishes equality for these fields over the selected
fixtures. Rendering, audio output, and gameplay routes beyond those fixtures
need their own evidence. Function-level exact comparisons remain a separate
gate.

## Investigate a difference

Capture a narrow frame window from both images:

```bash
python3 scripts/capture-replay.py \
  --target resources/th08.exe \
  --candidate build/th08.exe --map build/th08.map \
  --game-data build/modern-runtime/th08.dat \
  --bgm-data build/modern-runtime/thbgm.dat \
  --detail-start 5200 --detail-end 5270 \
  --output-dir build/replay-detail
```

Omit `--candidate` and `--map` for the retail capture. `items.jsonl` adds active
items, player shots, enemies, enemy bullets, bomb work items, collision
regions, point value, and FPU control for the selected window.
`score-events.jsonl` records `AddScore` arguments and return addresses. Resolve
retail callers through the target mappings and candidate callers through the
linker map. Allocation slots make pool entries comparable across the two
images.

Start with the earliest differing state, follow its producer back to target
instructions, and fix one bounded cause. Rebuild and compare the affected
object, verify the final playable link, then rerun the complete fixtures.
Keep the small summary and required trace; delete duplicate captures and
diagnostic windows after recording their conclusions.

The first use of this procedure found the Fantasy Seal bomb/deathbomb sine
and cosine binding error recorded as [RT-009](RUNTIME_ISSUES.md#rt-009).
