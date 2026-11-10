# Reconstruction lessons from completed work

Use [RE_HANDOFF.md](RE_HANDOFF.md) for current work. Earlier session snapshots
are available in Git history; their generated files are not required inputs.

## Replay investigation handoff — 2026-10-13

The replay repair series established several reusable distinctions:

- Input equality alone misses RNG, lifetime and final-score defects. Compare
  the selected state fields at every calculation and check natural completion.
- Native VC7 `normal` code is the exact oracle; the `bugfix` product supplies
  runtime evidence. Portable results apply to the particular tested build.
- A retained trace can be rechecked, but it does not test a newly compiled
  product. Use a fresh output directory when executable or observer inputs change.
- Streaming trace comparison reduced memory without changing comparison output.

The recorded results and reproduction commands are in
[REPLAY_RESULTS.md](REPLAY_RESULTS.md) and [REPLAY_TESTING.md](REPLAY_TESTING.md).

## Laser validation checkpoints before the final platform refresh, October 14, 2026

The visual investigation separated gameplay-state equality from image equality.
The useful findings are the spell-background owner repair, ECL polar-motion
precision, font coverage/height, surface filtering and boundary sampling.
Their target observations, failed controls and remaining questions are in
[PORTABLE_LASER_VISUALS.md](PORTABLE_LASER_VISUALS.md).

The native-layout reset repair later showed why literal i386 byte counts must
not clear native-sized objects. Use actual member extents and evidence-backed
`offsetof` expressions while preserving VC7 emission; see
[PORTABLE_LAYOUT_AUDIT.md](PORTABLE_LAYOUT_AUDIT.md).

Exact reconstruction also exposed stale PCH/object state and checkout-dependent
VC7 anonymous-namespace names. Cold comparisons settle shared-input changes;
normalizing a compiler discriminator must still preserve unique symbol identity,
full extents and every relocation. The details are in
[BUILD_MATCHING.md](BUILD_MATCHING.md) and [RE_WORKFLOW.md](RE_WORKFLOW.md).
