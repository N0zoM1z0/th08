# Portable replay state testing

Use this suite to compare Linux gameplay with the original Japanese TH08
1.00d. It runs the [credited replay corpus](../tests/fixtures/replays/th08-1.00d/README.md)
serially and checks the same 25 fields used by the
[native replay gate](REPLAY_TESTING.md): scores, resources, player position,
RNG state, replay input, and stage counters. Floating-point fields use exact
IEEE-754 bits.

The [validation results](REPLAY_RESULTS.md) record the complete native,
x86_64, and i386 passes, their tested artifacts, and coverage.

## Run the corpus

Install the [Linux build dependencies](PORTABLE_64BIT.md), Python 3.11 or later,
and `xvfb`, `xauth`, and `gdb`. Capturing retail references also needs the
[native capture dependencies](REPLAY_TESTING.md#run-the-comparison), including
Wine32 and `xdotool`. Keep your original `th08.dat` and `thbgm.dat` in a separate
data directory. The runner creates isolated saves and settings under `build/`.
SDL's dummy audio driver keeps playback muted after the replay restores its
recorded configuration.

On an x86_64 host, build the optional recorder:

```bash
cmake -S . -B build/portable-replay -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DTH08_ENABLE_REPLAY_TRACE=ON
cmake --build build/portable-replay --parallel 1
scripts/verify-portable-linux.sh build/portable-replay/th08-modern x86_64
```

For i386, install the [multilib development dependencies](PORTING.md#linux)
and select the 32-bit toolchain and package metadata:

```bash
PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig \
  cmake -S . -B build/i386-replay -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/linux-i386-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Debug -DTH08_ENABLE_REPLAY_TRACE=ON
cmake --build build/i386-replay --parallel 1
scripts/verify-modern-linux.sh build/i386-replay/th08-modern
```

Use that executable as `--candidate` in the same suite commands, with a separate
output directory for each architecture.

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

Revalidate a completed suite without launching the game or changing retained
evidence:

```bash
python3 scripts/test-portable-replay-suite.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --reference-dir build/replay-cold-20261011/matrix \
  --output-dir build/portable-replay-suite \
  --verify-only
```

This mode requires every capture and saved comparison to exist. It checks the
candidate, target, DAT/BGM, observer, schema, fixture, playback profile, natural
exit, all 25 trace fields, and the published calculation count and fingerprint.
It also recomputes each comparison and requires the result to match the saved
case report and suite ledger.

## Resource use and cleanup

Build with `--parallel 1` and run one suite at a time. The suite captures one
recording per process, limits Mesa worker threads, and gives each capture a
deadline (`--timeout`, 1,800 seconds by default). Finish the active suite before
starting another build. Prefix the suite command with `/usr/bin/time -v` to record
its elapsed time and maximum resident set size.

Trace validation and comparison stream the rows, retaining stage boundaries
and a five-row diagnostic window. On the 126,803-calculation `easy-1-b` pair,
the comparison process peaked at 17.5 MiB, down from 227 MiB with the buffered
comparator at `af9cb1d3`. Both produced identical reports. A regression test
compares 40,000 rows within a 64 MiB process address-space limit; this budget
applies to the comparator, while the game runs in its own process.

Measure a completed pair with:

```bash
/usr/bin/time -v python3 scripts/compare-replay-traces.py \
  build/replay-cold-20261011/matrix/easy-1-b/reference \
  build/portable-replay-suite/easy-1-b/candidate --allow-pacing-difference
```

To measure the preceding comparator, export it with
`git show af9cb1d3:scripts/compare-replay-traces.py > build/compare-replay-before.py`,
then repeat that command with `PYTHONPATH=scripts` and
`build/compare-replay-before.py` as the script path. Use the same retained pair
for both measurements.

Each completed capture compresses its logs and removes its temporary runtime.
The suite shares equal compressed traces with the validated retail reference,
so repeated passes retain metadata without duplicating those traces. Keep the
tested executable, build provenance, completed references, and comparison
results. Once build checks finish, copy the executable to a separate candidate
directory and remove that build's compiler tree. Check `free -h` and
`du -sh build/portable-replay*` between batches.

After an interrupted run, check the suite and capture processes before cleaning
any leftover `build/portable-state.*` directories. Resume with the same command:
the runner validates completed captures before continuing.

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

Use a prefix from the first recorded stage to diagnose a cross-stage failure.
Selecting a later stage restores its replay header; cumulative fields can
differ from a full-run reference. For example, starting Stage 5 directly
resets the death count in a recording that reaches it with two deaths.
`--max-calculations` preserves the preceding stages and their state.

## Coverage

The Linux x86_64 and i386 builds each pass the complete 108-recording corpus:
12,311,460 calculations, all 25 fields, recorded stage-end scores, natural exits,
and published fingerprints. Both retained-evidence passes also succeed.

[Replay validation results](REPLAY_RESULTS.md) records the full coverage grid,
artifact identities, native and portable repairs, image checks, resource
measurements, and evidence locations. The focused precision investigations
retain target observations, negative controls, and independent GDB diagnostics.
