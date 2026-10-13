# Portable polar motion precision

Solo Youmu Easy Final B, `th8_ud2bff.rpy`, first differed at Stage 2 frame
6,825. Retail registered a player-shot hit that portable missed: enemy 1
took 31 damage, internal score increased by 6, and the hit effect consumed
four RNG values. The player and shot positions agreed at that boundary.

Enemy 1's trajectory first differed at frame 5,573. Its stored interpolation
displacement already differed by one float ULP in Y:

| Field | Retail | Earlier portable |
| --- | --- | --- |
| Displacement Y bits | `425788bd` | `425788bc` |
| Position Y at frame 5,573 | `81.0625` | `81.06249237060547` |
| Velocity Y | `1.5416717529296875` | `1.5416641235351562` |

The displacement changes the interpolated velocity and movement angle.
The position difference later moves the enemy across the shot's collision
boundary. The repaired motion trace agrees on all 14 recorded enemy fields
over 6,824 shared frames; the 25-field GDB replay prefix passes 6,826
calculations, including the original failure frame.

## Target evidence and repair

The target's `cosf @ 0x00408D40` and `sinf @ 0x00409060` wrappers load a float
argument, call the double CRT routine, and execute `FST` to a float local.
`FST` writes the rounded copy and leaves the wider value in `ST(0)` for the
caller. Modern float-returning math functions return the rounded value.

`ConfigurePolarMotion @ 0x00420D10` and
`StartTimedPolarDisplacement @ 0x004222B0` multiply that wider trigonometric
result by a float speed, then by an integer duration. The observed retail
x87 control word is `0x007F`: each arithmetic operation rounds to a 24-bit
mantissa. The portable branches now keep the trigonometric result as double,
round the speed product to float, and scale it by the integer duration.

The native branches retain their preceding source expressions. Fresh builds
of both affected VC7 objects pass all 26 accepted comparison units. The normal
production link and its owner/data/callee verifier also pass.

## Reproduce the trajectory

Build a Debug portable candidate using [the replay procedure](PORTABLE_REPLAY_TESTING.md).
Capture the repaired prefix:

```bash
python3 scripts/capture-portable-replay.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --replay tests/fixtures/replays/th08-1.00d/accepted/th8_ud2bff.rpy \
  --reference build/retail-suite/easy-11-b/reference \
  --backend gdb --max-calculations 17870 --watch-enemy 1 \
  --output-dir build/portable-youmu-motion
```

This starts at the first recorded stage and retains its cumulative state.
Calculation 17,870 reaches Stage 2 frame 6,826 in the full reference.
The recorded 6,826-calculation stage-only diagnostic used a separate retail
reference that also started at Stage 2.

For retail, use `scripts/capture-replay.py` with the same replay and archives,
`--detail-stage 1 --detail-start 1 --detail-end 6825
--watch-enemy 1`. The [native procedure](REPLAY_TESTING.md) describes the
remaining arguments. Both tools write `enemy-motion.jsonl` with the same
field names and exact float bits. Compare records by `(stage, frame)`.
A candidate built at `92f9bbb5` reproduces the first motion difference at
frame 5,573 and the later replay difference at frame 6,825.

To inspect a bounded RNG window, use `--watch-rng --detail-stage 1
--detail-start 6822 --detail-end 6826` with the portable GDB backend.
`--watch-score-stage 1 --watch-score-frame 6822` also records score callers.

## Full replay checkpoint

The repaired x86_64 recorder passes three complete recordings:

| Case | Calculations |
| --- | ---: |
| Solo Youmu Easy Final B | 127,605 |
| Border Team Normal Final B | 100,979 |
| Solo Youmu Normal Final B | 129,556 |
| Total | 358,140 |

All 25 fields, stage-end scores, natural exits, and published fingerprints
agree with retail. All four retail frame comparisons also pass after the
motion repair. After the separate
[i386 device precision repair](PORTABLE_I386_FLOATING_POINT.md), the final
x86_64 and i386 builds each pass the full 108-recording gate over 12,311,460
calculations. The [validation results](REPLAY_RESULTS.md) record the complete gates.
Evidence is in `build/portable-state-20261012/matrix-polar/`, the
`youmu-easy-*` diagnostics, and `polar-rendering-regression/`.
