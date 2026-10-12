# Portable replay state testing

Use this suite to compare Linux gameplay with the original Japanese TH08
1.00d. It runs the [credited replay corpus](../tests/fixtures/replays/th08-1.00d/README.md)
serially and checks the same 25 fields used by the
[native replay gate](REPLAY_TESTING.md): scores, resources, player position,
RNG state, replay input, and stage counters. Floating-point fields use exact
IEEE-754 bits.

## Run the corpus

Install the [Linux build dependencies](PORTABLE_64BIT.md) and `xvfb`, `xauth`,
and `gdb`. Keep your original `th08.dat` and `thbgm.dat` in a separate data
directory. The runner creates isolated saves and settings under `build/`.
SDL's dummy audio driver keeps playback muted after the replay restores its
recorded configuration.

Build the optional recorder:

```bash
cmake -S . -B build/portable-replay -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DTH08_ENABLE_REPLAY_TRACE=ON
cmake --build build/portable-replay --parallel 1
scripts/verify-portable-linux.sh build/portable-replay/th08-modern x86_64
```

Run every accepted recording:

```bash
python3 scripts/test-portable-replay-suite.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --output-dir build/portable-replay-suite
```

The runner captures missing retail references using the canonical executable
in `resources/th08.exe`. Each retail capture must reproduce its published
calculation count and trace fingerprint. Reuse an existing native suite with
`--reference-dir build/replay-cold-20261011/matrix`, or point this option to
your own completed suite. Reference metadata must identify the canonical
executable, replay, DAT/BGM, schema, configuration, and playback mode.

Use `--cases easy-0-b,easy-5-b,extra-2-extra` for a smaller selection.
`--keep-going` records each failure and continues the remaining cases.
Repeat the command to resume: completed captures are revalidated, and failed
attempts remain under their case directory. A changed executable, observer,
asset set, schema, or playback profile requires a fresh output directory.

## Read the result

`suite.json` records each case. A complete pass requires:

- Every calculation agrees on all 25 fields.
- Stages appear in the recorded order, with continuous calculation counters.
- Each stage consumes its recorded inputs, allowing the documented trailer.
- Each stage ends at the recording's score.
- Playback reaches the natural replay deletion boundary and exhausts the
  complete retail trace.
- The trace matches the published fingerprint in
  [config/replay-fixtures.json](../config/replay-fixtures.json).

The first difference produces `first-difference.json` with the field names,
raw values, stage, input frame, and preceding calculations. The recorder stops
the isolated test process there. `rows.jsonl.gz`, completion metadata, and
compressed logs remain for inspection. Equal trace files share storage after
their decompressed bytes have been verified; per-product metadata remains
separate. Temporary binary traces, saves, data links, and owned Wine prefixes
are removed.

## Check the observer independently

The recorder is a Linux test build option, off by default. Its hooks read
state at `GameManager::OnUpdate` entry and
`ReplayManager::ControlPlaybackFrameAdvance` entry; they consume no RNG values.
The stage RNG origin follows the native schema. The test selects the public
fast-forward replay mode and preserves normal life rules and boss-name copies.
At natural deletion it closes the trace, then returns through normal game
shutdown on the next render iteration. Ordinary gameplay builds exclude these
hooks.

The GDB backend uses debug types to locate portable fields. It works with
ordinary Debug builds and leaves the compiled recorder disabled. For an
independent check against a complete retail capture:

```bash
python3 scripts/capture-portable-replay.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --replay tests/fixtures/replays/th08-1.00d/accepted/th8_ud2cdc.rpy \
  --reference build/replay-cold-20261011/matrix/easy-0-b/reference \
  --backend gdb --output-dir build/portable-gdb-check
```

Add `--paced --max-calculations 1000` to check an ordinary-paced prefix with
rasterization. A diagnostic prefix is recorded as incomplete and cannot count
as a full replay pass. The accelerated profile retains draw callbacks and
uses Mesa's rasterization switch; every accepted state trace still has to match
the retail fingerprint.

## Coverage

The repaired x86_64 recorder passes six complete recordings over **667,371
calculations**, with all 25 fields, recorded stage-end scores, and natural replay
completion equal to retail. These cover Border Team Easy/Lunatic Final B,
Solo Yukari Easy Final B, Magic Team Lunatic Final A, Ghost Team Lunatic Final B,
and Scarlet Team Extra: all nine internal stage indexes. The batch is in
`build/portable-state-20261012/matrix-fixed/`.

The initial GDB/recorder comparison agrees over 1,000 calculations, and the
repaired Stage 4A GDB diagnostic agrees over 14,566 calculations. The full
108-recording portable batch is the next gate. The completed native
108-recording result remains a separate product checkpoint.

The first complete batch exposed a modern math-overload difference in Magic
Team Stage 4A. [The vector precision investigation](PORTABLE_VECTOR_PRECISION.md)
records the score, item, and enemy motion evidence and the repaired diagnostic
window.

State parity covers the selected recordings and fields. Use the
[rendering gate](PORTABLE_RENDERING.md) for frame comparisons, HOME screenshots,
and renderer contracts. Hardware gameplay on AArch64 remains a separate check.
