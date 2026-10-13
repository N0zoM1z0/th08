# Portable i386 device precision

After the normalization and polar-motion repairs, Linux i386 still differed
in Border Team Normal Final B, `th8_ud2cd5.rpy`, at calculation 60,796:
Stage 5 frame 3,789. Internal score was four higher, and the hit effect
consumed four extra RNG values. The x86_64 candidate passed the recording.

The i386 runtime retained x87 control word `0x037F`, with a 64-bit mantissa.
Retail uses `0x007F`, with a 24-bit mantissa, nearest rounding, and masked
exceptions. Unlike x86_64's SSE arithmetic, i386's ordinary x87 expressions
depend on this precision setting.

## Device behavior

The Linux D3D8 adapter now selects single precision, nearest rounding, and
masked exceptions after successful i386 device creation. It honors
`D3DCREATE_FPU_PRESERVE`. The implementation uses the platform's floating-point
environment APIs and its declared x87 control field.

The TH08 control words above are runtime observations against the canonical
Japanese 1.00d executable. Microsoft's
[Direct3D 9 flag documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dcreate)
describes the same default precision policy; it supplies API context for the
observed D3D8 behavior.

## Reproduce the device contract

In an i386 build environment with Python 3.9 or later and the SDL/OpenGL
development packages, run:

```bash
python3 scripts/test-portable-renderer.py \
  --cases fpu --output-dir build/i386-device-precision
```

For a cross compiler, pass `--compiler` and select the i386 package metadata
with `PKG_CONFIG_LIBDIR`. The test needs no game archives. It starts the caller
in extended precision and downward rounding, then checks both the default
device behavior and explicit preservation. A 64-bit executable reports this
x87 contract as skipped.

Use `--baseline-ref 92f9bbb5 --expect-failures` in the same i386 environment to
reproduce the missing precision switch. The existing pixel contracts retain
their default selection: locks, perspective, clipping, and fog.

## Reproduce replay parity

Build the optional i386 recorder using [the replay procedure](PORTABLE_REPLAY_TESTING.md),
then run the complete recording:

```bash
python3 scripts/test-portable-replay-suite.py \
  --candidate build/i386-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --cases normal-0-b --output-dir build/i386-border-replay
```

For an independent GDB check through the original failure:

```bash
python3 scripts/capture-portable-replay.py \
  --candidate build/i386-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --replay tests/fixtures/replays/th08-1.00d/accepted/th8_ud2cd5.rpy \
  --reference build/retail-suite/normal-0-b/reference \
  --backend gdb --max-calculations 60797 \
  --output-dir build/i386-border-gdb
```

The full recorder passes **100,979 calculations**, all 25 fields, recorded
stage-end scores, natural exit, and the published fingerprint. GDB independently
passes 60,797 calculations and records control word `0x007F` in
`window-state.json.gz`.

Use the full prefix to retain earlier stages' state. A direct Stage 5 start
resets the death count, while this full-run reference reaches Stage 5 with
two deaths. Such a startup comparison describes a different replay scenario.

Evidence is in `build/portable-state-20261012/i386-matrix-fpu/`,
`i386-stage5-full-prefix/`, and `fpu-device-contracts/`. The subsequent full i386
gate passes all 108 recordings and 12,311,460 calculations, including a separate
retained-evidence verification. See the [complete results](REPLAY_RESULTS.md).
AArch64 hardware gameplay remains a separate check.
