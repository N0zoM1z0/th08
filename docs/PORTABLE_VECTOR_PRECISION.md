# Portable vector precision

The complete replay gate found a score difference in `lunatic-1-a`, Magic Team
Lunatic Final A (`th8_ud2cd7.rpy`). At Stage 4A, internal stage index 3,
input frame 14,564, portable's internal score was 12 higher: 120 displayed
points. The other 24 recorded fields agreed.

## From angle to score

Both the compiled recorder and the independent GDB observer reproduced the
same first difference. A retail score-write trace and portable call trace
identified `Item::CollectPointSmall`: the original awarded 55,580 points;
portable awarded 55,700.

Item slot 2,052 came from `Enemy::DropItems`, enemy slot 5, at frame 14,463.
Its spawn X was `379.6240234375` in retail and `379.62384033203125` in portable.
At pickup, its Y was `166.0` in retail and `165.9999542236328` in portable.
The score formula truncates the height above the point-value line to an
integer, so this small position difference changed the award.

The current enemy incarnation first diverged at frame 14,030, after
`UpdateBouncingEnemyMotion`. Its velocity inputs and acceleration variable
agreed. The following angle calculation differed by one float ULP:

| Value | Retail | Earlier portable |
| --- | --- | --- |
| `VectorAngle` input Y | `-0.3900333344936371` | same |
| `VectorAngle` input X | `1.9535869359970093` | same |
| Angle result bits | `0xBE49C9CC` | `0xBE49C9CB` |

The modern C++ math header selected `std::atan2(float, float)`, which calls
`atan2f`. The target `VectorAngle @ 0x0040C7B0` calls the double CRT path through
`_CIatan2 @ 0x004A42FA`. The portable expression now promotes both arguments
to double before the call and rounds the result to float afterward.

Two related helpers also rounded trigonometric results early.
`Float3::FromAngleMagnitude @ 0x004286E0` and
`Float3::FromRotatedVec2 @ 0x00428700` execute `FSINCOS`, multiply, then store
float coordinates. Their modern paths now use double sine/cosine products
before the final float stores. Ten independent high-precision contracts catch
the earlier rounding in both helpers.

The observed retail x87 control word is `0x007F`: single precision,
round to nearest. The device-creation calls omit `FPU_PRESERVE`.
[Microsoft's floating-point mode description](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dcreate)
provides supporting API context; the runtime register observation identifies
the mode in this replay capture.

## Reproduce the checks

The vector contracts need a C++ compiler and no game assets. Add a Debug ELF
to call the real `VectorAngle` implementation through GDB before game startup:

```bash
python3 scripts/test-portable-math.py \
  --candidate build/portable-replay/th08-modern
```

The preceding source checkpoint reproduces both vector failures:

```bash
python3 scripts/test-portable-math.py \
  --baseline-ref 687c29e8 --expect-failures
```

An ELF built at that checkpoint also reproduces the wrong `VectorAngle` bit
when supplied with `--candidate` in the same command. Historical controls
must fail at the expected helper; compiler or debugger errors fail the check.

Use the [replay state procedure](PORTABLE_REPLAY_TESTING.md) for the complete
recording. For a shorter motion diagnostic against an existing retail reference:

```bash
python3 scripts/capture-portable-replay.py \
  --candidate build/portable-replay/th08-modern \
  --game-data-dir /path/to/original-game-data \
  --replay tests/fixtures/replays/th08-1.00d/accepted/th8_ud2cd7.rpy \
  --reference build/replay-cold-20261011/matrix/lunatic-1-a/reference \
  --backend gdb --start-stage 3 --max-calculations 14566 \
  --watch-enemy 5 --watch-item 2052 --watch-item-start 14460 \
  --output-dir build/portable-magic-motion
```

The native runner's `--watch-enemy` and `--watch-item` options read the same
pool slots during a selected `--detail-stage/--detail-start/--detail-end`
window. Compare `enemy-motion.jsonl` and `item-motion.jsonl` by stage and frame;
coordinates and angles are raw IEEE-754 bits. GDB snapshots preserve the
portable field layouts for further inspection.

## Verification

After the angle repair, all 14,463 retained enemy observations agree with
retail, and the 14,566-calculation Stage 4A prefix passes all 25 fields.
Item slot 2,052 reaches the original `166.0` pickup height. The asset-free
angle probe reproduces the wrong bit in the earlier ELF and passes in the
repaired build. The complete portable corpus gate is recorded separately.

Local diagnosis and regression evidence is in `build/portable-state-20261012/`.
The native comparison branch retains its preceding expressions and ABI.
