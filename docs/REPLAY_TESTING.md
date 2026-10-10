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

References from schema version 1 need a fresh capture: that observer missed
intermediate dialogue fast-forward frames. The runner validates a reused
reference before launching the candidate.

## Read the evidence

`candidate/comparison.json` contains the result, differing-frame counts, and
the first difference with nearby frames. Each capture records executable and
data hashes in `metadata.json`; candidate captures also record the linker-map
hash, source commit, and whether source/configuration changes were present.
`rows.jsonl` uses the field order in
[replay-schema.json](../scripts/replay-schema.json).

The observer reads memory at entry to `ControlPlaybackFrameAdvance`, after
the gameplay calculation callbacks. Dialogue fast-forward restarts the
calculation chain up to three times per rendered frame; this position records
each logical frame. At the terminal demo frame, `GameManager::OnUpdate`
breaks the chain before the playback-control callback. A second hardware
breakpoint after `RunCalcChain` records that exit state and detects completion.
The schema requires all 6,120, 4,920, and 5,040 frames respectively, including
those terminal records. Both locations are verified against the loaded
instruction bytes. Candidate addresses come from its linker map; gameplay
receives its input from the bundled replay.

The trace compares stage, replay and game frames, input, score, graze, deaths,
lives, bombs, power, player position/state, RNG seed/generation, point items,
time orbs, gauge, and clock. Input is the replay-fed value consumed by player
gameplay (`g_GuiMessageInputCurrent`). Floating-point values are compared by
their exact IEEE-754 bits. A death or score change is a failure only when the
retail trace differs. Demo 0 includes a death in the original game.

A passing result establishes equality for these fields over the selected
fixtures. Rendering, audio output, and gameplay routes beyond those fixtures
need their own evidence. Function-level exact comparisons remain a separate
gate.

## Native checkpoint

The 2026-10-10 muted Wine/GDB comparison passed on VC7 build
`94b35a71...bcf58f`, using schema version 2:

| Fixture | Stage | Frames | Recorded state |
| --- | --- | ---: | --- |
| `demo/demorpy0.rpy` | 5 | 6,120 | All 22 fields equal |
| `demo/demorpy1.rpy` | 3 | 4,920 | All 22 fields equal |
| `demo/demorpy2.rpy` | 2 | 5,040 | All 22 fields equal |

The local evidence is in `build/native-replay-parity-final/`; both captures
completed naturally. The source repairs also passed a single-job cold
comparison of all 1,106 accepted authored units and final-link verification.

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
regions, effects, point value, and FPU control for the selected window.
`score-events.jsonl` records `AddScore` arguments and return addresses. Resolve
retail callers through the target mappings and candidate callers through the
linker map. Allocation slots make pool entries comparable across the two
images.

`rng-events.jsonl` records RNG consumption and caller frames. When the call
comes from an ANM instruction, it also identifies the VM's object pool,
allocation slot, file, and script. This distinguishes an extra gameplay
decision from extra animation instances that consume the shared RNG.

To follow an effect across the whole replay, add `--watch-effect 13` for
spellcard orbits, or another ID from `EffectId`. `effects.jsonl` records only
the selected IDs, including inactive slots, animation flags, script position,
and release state, alongside boss attachment lists. `effect-events.jsonl`
records `ReleaseAttachedEffects` callers and the slots they release.
This capture needs no detail window and keeps long
lifetime investigations small.
Use separate captures for effect watching and detail windows, which together
would exceed x86's four hardware breakpoint slots.

Start with the earliest differing state, follow its producer back to target
instructions, and fix one bounded cause. Rebuild and compare the affected
object, verify the final playable link, then rerun the complete fixtures.
Keep the small summary and required trace; delete duplicate captures and
diagnostic windows after recording their conclusions.

This procedure found the Fantasy Seal sine/cosine binding error in
[RT-009](RUNTIME_ISSUES.md#rt-009) and the death-mode switch entry that
prematurely released spellcard orbits in [RT-010](RUNTIME_ISSUES.md#rt-010).
