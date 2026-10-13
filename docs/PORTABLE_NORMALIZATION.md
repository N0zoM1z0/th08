# Portable D3DX normalization

The Border Team Normal Final B recording `th8_ud2cd5.rpy` exposed a portable
RNG difference at Stage 1 frame 2,516. Score, player position, resources,
and input still agreed; portable had consumed four fewer RNG values.

The first effect pool difference occurs at frame 496. Camera particle 25
expires in retail and survives one more frame in portable. Later allocations
use different slots. When the primary pool fills, a hit effect can consume
RNG in one product and fail to allocate in the other.

## Target evidence

The canonical Japanese 1.00d capture selects
`sse2_D3DXVec3Normalize @ 0x0048EFB0` through dispatch slot `0x004C872C`.
The linked library computes the squared length, uses `RSQRTSS`, and performs
one Newton refinement. Each multiply, add, and subtract rounds to float.
Its constants at `0x018DA220` are squared epsilon `2^-46`, `0.5`, and `3.0`.

`UpdateTintedBossTrackingCameraParticle @ 0x004264F0` rejects directions whose
dot product with the camera forward vector is below `0.94`. At frame 496,
retail and portable have identical particle positions, camera positions,
and forward vectors. The input delta bits are:

```text
c15436fb 43676bb8 42bccfe0
```

The normalized components below are runtime reads. The dot products are
calculated from those values using the target's float multiply/add order.

| Result | X | Y | Z | Computed dot |
| --- | --- | --- | --- | --- |
| Retail SSE2 | `bd590edc` | `3f6cb3cf` | `3ec11f05` | `3f70a3d6` |
| Earlier portable division | `bd590edc` | `3f6cb3d0` | `3ec11f06` | `3f70a3d7` |

The latter dot product equals the float representation of `0.94`, changing
the particle lifetime. The Linux x86 backend now uses SSE intrinsics for the
attested normalization order and preserves in-place output. This change is
confined to the Linux backend; the accepted VC7 comparison units remain valid.

## Reproduce the contracts

Build a Debug Linux candidate using [the replay procedure](PORTABLE_REPLAY_TESTING.md).
This test calls the actual compiled function before game startup, using
scratch globals. It needs GDB and no game archives:

```bash
python3 scripts/test-portable-normalization.py \
  --candidate build/portable-replay/th08-modern
```

It checks the retail sample, aliased output, the zero/nonzero epsilon boundary,
and null arguments. Use `--expect-failure` with a candidate built at `0a82c740`
to reproduce the earlier camera-particle result.

## Trace the effect pool

Both capture tools support `--watch-effect-pool`. They record the primary
pool's 512 active/ID pairs, allocation cursor, and active count in
`effect-pool.jsonl.gz`. For the portable diagnostic:

```bash
python3 scripts/capture-portable-replay.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --replay tests/fixtures/replays/th08-1.00d/accepted/th8_ud2cd5.rpy \
  --reference build/retail-suite/normal-0-b/reference \
  --backend gdb --max-calculations 2517 --watch-effect-pool \
  --output-dir build/portable-border-pool
```

Capture the matching retail prefix with `scripts/capture-replay.py`, the same
replay and archives, `--watch-effect-pool --detail-stage 0 --detail-start 2515
--detail-end 2517`. The [native procedure](REPLAY_TESTING.md) describes its
required arguments and accelerated profile. Compare the diagnostics:

```bash
python3 scripts/analysis/compare-effect-pools.py \
  --reference build/retail-border-pool \
  --candidate build/portable-border-pool
```

The comparator validates replay, target, archive, and playback-mode identity,
then reports the first allocation/occupancy difference. A prefix result covers
the shared diagnostic frames. The full state gate also checks replay end
scores, input consumption, natural exit, and published fingerprints.

For normalized vectors, both capture tools accept `--watch-normalization
--detail-stage 0 --detail-start 495 --detail-end 497`. The portable tool
requires `--backend gdb` and a diagnostic calculation limit. Portable RNG
callers use `--watch-rng` with the same window options; the native tool records
RNG callers by default in an external detail window.

## Checkpoint

The repaired x86_64 recorder passes the complete Border Team Normal Final B
recording: **100,979 calculations**, all 25 fields, stage-end scores, and natural
replay completion agree with retail. The subsequent
[polar motion repair](PORTABLE_POLAR_MOTION.md) resolves the Solo Youmu
Easy and Normal failures.

The ordinary GDB build passes 2,517 calculations. Its first 2,516 pool records
agree with retail, including every slot's active/ID pair and allocation cursor.

Evidence is under `build/portable-state-20261012/`, including the original
normalized vectors, pool traces, old-build controls, and `matrix-normalization/`.
The i386 build passes the same asset-free contracts and ownership checks.
At `92f9bbb5`, its Border Team recording advances past the repaired failure, then differs
at calculation 60,796, Stage 5 frame 3,789: internal score +4 and four extra
RNG generations. The subsequent
[i386 device precision repair](PORTABLE_I386_FLOATING_POINT.md) resolves it.
AArch64 hardware gameplay is pending; ARM retains scalar normalization.
