# Native replay comparison

Use the original Japanese TH08 1.00d executable as the runtime reference.
`test-native-replay.py` runs its bundled demos and the playable VC7 build in
sequence, records each calculation frame, and reports the first difference.
BGM and sound effects are off throughout. The run needs no menu input or
supervision.

`test-replay-suite.py` extends the comparison to standard replay files. The
[fixture manifest](../config/replay-fixtures.json) pins 108 public 1.00d
recordings: each of the 12 shot types on Easy, Normal, Hard, and Lunatic through
Final A and Final B, plus Extra. Stage 4A and 4B follow the recorded shot's
route. The manifest includes download URLs, recorder credits, hashes, and
stage input counts. Replay files remain under `build/`.
`playedAt` preserves the archive's Play Date; `submittedAt`, when present,
records its upload date.

## Full route suite

The suite needs `xdotool` for menu input. It creates an isolated runtime with
one replay file, opens the Replay menu, selects the first recorded stage, and
plays through the recorded route. Wine audio drivers are disabled; replay
bytes and their saved configuration stay intact.

```bash
python3 scripts/test-replay-suite.py \
  --game-data build/modern-runtime/th08.dat \
  --bgm-data build/modern-runtime/thbgm.dat \
  --output-dir build/replay-suite-check
```

The runner downloads and verifies each fixture, captures retail playback,
then compares the native build. Cases run serially. `suite.json` records each
result and its first difference. Run the same command again to resume with
the same build and settings. `--cases easy-0-b,extra-0-extra` selects individual
cases; the number is the shot index listed in the manifest. `--fetch-only`
checks the input files without launching Wine.

Inspect a running or completed batch without opening its capture logs:

```bash
python3 scripts/analysis/report-replay-suite.py --suite build/replay-suite-check
```

This reads the ledger and latest capture progress. To verify retained results,
resume the suite or use the expectation-recording command below.

For a new build, use a fresh output directory and
`--reference-dir build/replay-suite-check` to reuse validated retail captures.
The runner checks replay, executable, game-data, and pacing identities before
reuse. Completed traces and debugger logs are compressed; temporary runtimes
and Wine prefixes are removed after each capture.

For a long serial batch, `--wine-prefix build/replay-suite-wine` reuses a
prefix marked as owned by this runner. It stays under `build/` until removed
after the batch; an existing unmarked prefix is rejected. Captures still
remove their individual runtimes.

The optional `--clock-rate 128 --no-rasterization` profile scales Wine's raw
monotonic clock and uses Mesa llvmpipe's
[profiling switch](https://docs.mesa3d.org/envvars.html#envvar-LP_NO_RAST) to
skip pixel rasterization. Game calculation and draw callbacks still run.
The profile matched the ordinary retail trace over all 16,080 bundled-demo
frames. `--observer-backend ptrace` uses Linux hardware execute breakpoints
with the same observer as GDB; it requires a Linux x86-64 host. Keep ordinary
pacing and GDB available for profile validation and investigations.

External traces use schema version 4. They record every calculation, including
stage-clear calculations with frozen replay input. Both replay input and game
frame counters must remain continuous. Each stage must reach its recorded end
score and consume its input stream through the recording's trailer. Complete
retail captures leave two or three records at an intermediate stage and six
or seven at the final stage. The recording callbacks append stage-clear and
stop-recording inputs; playback establishes the exact frame count for each
fixture. Completion comes from the game's replay exit. The comparison requires
both traces to contain the same complete sequence of calculations.

The 25 compared fields include the demo trace's gameplay state, plus point
value, character, shot type, and difficulty. RNG generations are measured from
the first gameplay calculation of each stage; raw starting counters remain
in completion metadata. This keeps title/loading activity separate from
gameplay RNG consumption. Floating-point values retain exact bit comparison.

The candidate compares against the reference while running. At the first
difference it saves `first-difference.json`, recent frames, and a compressed
player-state snapshot, then stops. Use `--keep-going` to investigate other
cases in the same batch. A timeout or incomplete stage records a failure.
Rendering comparisons use rasterization and separate image evidence.

To investigate an external replay, pass `--detail-stage`, `--detail-start`,
and `--detail-end` to `capture-replay.py`. The window
records RNG callers and attached-effect releases, then saves object state.
Add `--watch-score` to collect score writes, their call stacks, and item spawns
instead. Both GDB and ptrace support these windows; ptrace reallocates the four
hardware slots as callbacks are enabled or disabled. Window and failure
snapshots include player, spellcard, bullet, enemy, item, and effect objects.
Diagnostic captures stop at the requested frame and remain incomplete;
only playback through the game's replay exit can pass the parity gate.

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

## Reproduce published results

The fixture manifest's `retailTrace` entries are the published expectations.
Each records the complete calculation count and a SHA-256 of the canonical
retail trace. A fixture without that entry is planned coverage. The suite
checks published expectations against both captures, so equality between two
new traces alone does not reproduce an earlier result.

After preparing the VC7 build and game data as above, run:

```bash
python3 scripts/test-replay-suite.py \
  --claims-only \
  --game-data build/modern-runtime/th08.dat \
  --bgm-data build/modern-runtime/thbgm.dat \
  --output-dir build/replay-published-check \
  --wine-prefix build/replay-published-wine \
  --observer-backend ptrace --clock-rate 128 --no-rasterization
```

This downloads the pinned recordings and captures both executables locally.
It needs no private reference trace. Omit `--claims-only` to run the full
108-case corpus. Use `--cases` to reproduce one case. A nonzero exit status
means a difference, an invalid capture, or a failed published expectation;
inspect `suite.json` and the case's logs. Resuming revalidates retained traces.

The trace fingerprint hashes the schema as compact JSON with sorted keys,
followed by a newline and every row serialized as little-endian unsigned
32-bit values in schema field order. JSON indentation and gzip compression
do not affect it. `comparison.json` prints both fingerprints.

The first full-route checkpoint used Ubuntu 24.04 on WSL2 x86-64, Wine 9.0,
Mesa llvmpipe 25.2.8, Python 3.13.5, and Xvfb 21.1.12. Compiler and library
pins are in the [native build procedure](WINDOWS_I386_RUNTIME.md).
All nine published cases passed at source checkpoint `25c101f9`, including a
rerun of the first five cases after the replay-input repair. Build the `bugfix`
profile from this branch or a later revision containing its repairs.
Capture metadata records executable, map, data and configuration hashes,
source revision, observer backend, clock rate, rasterization, and host/Wine
versions. Preserve `suite.json`, comparison files, metadata, completion files,
and compressed traces when sharing an independent result. Delete the owned
Wine prefix after the batch; game assets and replay files stay local.

To review a completed batch and publish additional expectations:

```bash
python3 scripts/analysis/record-replay-expectations.py \
  --suite build/replay-suite-check
# Add --write after reviewing the verified cases.
```

This rechecks retained traces, fixture identities, the canonical reference,
candidate executable and map, and the recorded calculation counts. It rejects
changes to existing expectations. Only the acceptance hashes and counts enter
the manifest; generated reports and recordings remain under `build/`.

Nine complete Easy Final B cases have published expectations:

| Shot | Stage 4 route | Calculations |
| --- | --- | ---: |
| Border Team | B | 107,296 |
| Magic Team | A | 126,803 |
| Scarlet Team | A | 98,098 |
| Ghost Team | B | 103,875 |
| Reimu | B | 118,416 |
| Yukari | B | 132,629 |
| Marisa | A | 122,814 |
| Alice | A | 109,495 |
| Sakuya | A | 134,146 |

All 25 fields agree over 1,053,572 calculations on that source checkpoint.
The current batch continues through the remaining corpus. Repository CI validates
the manifest's complete 108-case grid and published expectation format.

## Reference fixture failures

[replay-rejected-fixtures.json](../config/replay-rejected-fixtures.json) retains
recordings that failed the retail completeness check, with their download
URLs, hashes, and observed failures. For Sakuya's `th8_ud2b7c.rpy`, normal and
built-in fast-forward playback produce the same 59,706-calculation prefix,
ending in Stage 4A at frame 19,840. Ordinary pacing with rasterization also
reproduces that stage's 19,840 calculations. The retail desynchronization's
cause is unresolved. The active `easy-8-b` fixture is `th8_ud1051.rpy`, which
completes all six stages and recorded end scores in retail.

Fetch the retained fixture, then reproduce the Stage 4A failure with ordinary
pacing and rendering:

```bash
python3 scripts/test-replay-suite.py \
  --manifest config/replay-rejected-fixtures.json --fetch-only \
  --game-data build/modern-runtime/th08.dat \
  --bgm-data build/modern-runtime/thbgm.dat \
  --output-dir build/replay-rejected-inputs
python3 scripts/capture-replay.py \
  --target resources/th08.exe \
  --replay build/replay-fixtures/th8_ud2b7c.rpy \
  --start-stage 3 --playback-mode 0 --observer-backend ptrace \
  --game-data build/modern-runtime/th08.dat \
  --bgm-data build/modern-runtime/thbgm.dat \
  --output-dir build/replay-retail-failure
```

In the recorded Wine/Mesa environment, this exits nonzero with 3,772 input
records left and score 31,688,942 instead of the recorded 40,908,184.
The active matrix requires a complete retail reference for each coverage cell.

## Native checkpoint

The 2026-10-10 muted Wine/GDB comparison passed on VC7 build
`94b35a71...bcf58f`, using schema version 2:

| Fixture | Stage | Frames | Recorded state |
| --- | --- | ---: | --- |
| `demo/demorpy0.rpy` | 5 | 6,120 | All 22 fields equal |
| `demo/demorpy1.rpy` | 4A | 4,920 | All 22 fields equal |
| `demo/demorpy2.rpy` | 3 | 5,040 | All 22 fields equal |

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
