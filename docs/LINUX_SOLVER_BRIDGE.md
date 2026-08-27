# Linux online solver bridge

This derived Linux integration lane contains an opt-in, hard-no-Bomb input
bridge for the TH08 solver. It is separate from the exact VC7 reconstruction
claim and is inactive unless `TH08_SOLVER_SOCKET` names a Unix-domain socket.
Normal interactive Linux execution continues to read SDL keyboard state.

The active protocol is strictly online. The game never waits for the solver,
and solver time is never removed from QPC, `timeGetTime`, rendering, or logical
60 Hz admission. The old version-1 lockstep/time-exclusion experiment is
retired and has no route-action authority.

## Physical-frame boundary

`Supervisor::OnUpdate` samples DirectInput near the start of the calc chain.
After the complete calc chain succeeds, `GameWindow::Render` publishes one
post-update source epoch, then continues through draw and `Present`. The solver
may compute during that remaining wall-clock window. At the next DirectInput
sample, the runtime consumes only a response naming that exact target epoch.

If no exact response is ready, the runtime repeats the complete current input
mask only when an earlier response installed a finite continuation lease and
the current epoch, runtime context, held mask, and source generation still
match that lease. Otherwise it selects neutral Shot+Focus and records an
uncertified fallback. A late response is counted and discarded; it is never
applied on a later frame. Accept, receive, and publish operations are all
non-blocking on the game thread. `enemy_manager_frame` is not used as the input
clock because it can freeze while input still moves the player. The exported ELF symbol
`th08_solver_input_epoch` remains the final delivery-deadline check; gameplay
sensing no longer brackets a collection of live `/proc/<pid>/mem` reads with
that symbol.

At the post-update seam the runtime scans only in-process active flags, packs
the active bullet, laser, enemy, and item records plus the exact player,
timeline, ECL, SHT, auxiliary-context, and indexed-enemy address ranges into
one of two runtime-owned slots, and publishes a generation certificate. A published slot
is never overwritten until the solver releases that exact `(slot,
generation)`. If neither slot is free or the bounded 32 MiB slot is
insufficient, the game drops the snapshot and still publishes the input
deadline notification. It never waits.

The solver copies the packed slot once, queues its release, and services both
the immediate shield and background future/global graph through a local
address-space reader over those immutable bytes. The immediate shield decodes
typed active records directly with original slot indices. Full legacy pools
are reconstructed locally with zero-filled inactive records only for existing
background decoders. Snapshot packing and copying are ordinary wall-clock
work; no time is removed from the game clock.

The bridge supplies only keyboard state. Shared authored source still owns
current/last input, held-key repeat, callback order, gameplay state, RNG, and
replay recording. Bridge mode stamps `g_Supervisor.exeSize=840704` and
`exeChecksum=2724749753` before replay creation so a saved replay identifies
its intended Japanese v1.00d playback target. That is replay metadata, not an
ELF identity claim.

## Socket lifecycle

Set an absolute, task-owned socket path before launch:

```bash
TH08_SOLVER_SOCKET=/tmp/owned-run-directory/bridge.sock \
  build/modern-linux/th08-modern --data-dir /path/to/data
```

The runtime creates a non-blocking Unix `SOCK_SEQPACKET` listener. It never
deletes an existing path and does not unlink the path on exit; the launcher
owns cleanup of its exact run directory. Before a client connects, after a
disconnect, or after bridge failure, the runtime continues with neutral
Shot+Focus rather than SDL input or a stale direction. A connected deadline
miss also returns to neutral unless a versioned finite continuation lease
remains applicable. A protocol violation closes only that client and therefore
returns input to neutral.

Bridge mode preserves lives by default for hit-counting diagnostics. Set
`TH08_SOLVER_PRESERVE_LIVES=0` only for an explicitly diagnostic run that must
reach the retail game-over/result-screen replay-save path. The request flags
attest the selected behavior; a preserved-life run is not itself an NMNB
completion claim.

## Version 4 wire format

Each packet is one complete little-endian `SOCK_SEQPACKET` record. Requests are
104 bytes:

| Offset | Type | Meaning |
| --- | --- | --- |
| `0` | `u32` | magic `0x51523854` (`T8RQ`) |
| `4` | `u16` | protocol version `4` |
| `6` | `u16` | request size `104` |
| `8` | `u64` | completed-update source input epoch |
| `16` | `u64` | exact target input epoch, always source + 1 |
| `24` | `u16` | input active during the completed source update |
| `26` | `u16` | preceding input |
| `28` | `u16` | post-update RNG seed |
| `30` | `u16` | immutable slot index `0..1`, or `0xffff` |
| `32` | `u32` | flags: replay target stamped; lives preserved; immutable snapshot present |
| `36` | `u32` | cumulative missed response deadlines, saturating |
| `40` | `u32` | cumulative discarded late responses, saturating |
| `44` | `u32` | cumulative dropped request publications, saturating |
| `48` | `u64` | `CLOCK_MONOTONIC` publication time in microseconds |
| `56` | `u64` | immutable snapshot generation, or zero |
| `64` | `u32` | runtime slot address, or zero |
| `68` | `u32` | packed snapshot byte size, or zero |
| `72` | `u32` | packed range-entry count, or zero |
| `76` | `u32` | cumulative dropped snapshots, saturating |
| `80` | `u32` | most recent runtime snapshot-pack time in microseconds, saturating |
| `84` | `u32` | cumulative certified finite fallbacks, saturating |
| `88` | `u32` | cumulative neutral uncertified fallbacks, saturating |
| `92` | `u32` | current consecutive fallback count, saturating |
| `96` | `u32` | maximum consecutive fallback count, saturating |
| `100` | `u32` | cumulative rejected or context-revoked leases, saturating |

Responses are 40 bytes:

| Offset | Type | Meaning |
| --- | --- | --- |
| `0` | `u32` | magic `0x53523854` (`T8RS`) |
| `4` | `u16` | protocol version `4` |
| `6` | `u16` | response size `40` |
| `8` | `u64` | request source epoch |
| `16` | `u64` | exact target epoch, always source + 1 |
| `24` | `u16` | complete TH08 logical input mask |
| `26` | `u16` | continuation frames, `0..8` |
| `28` | `u64` | exact source snapshot generation, or zero without a lease |
| `36` | `u32` | reserved, zero |

Bomb (`0x0002`), unknown bits, up+down, left+right, wrong-sized packets,
truncated packets, stale targets, unrequested future targets, and unleased
snapshot generations are rejected. A continuation lease is installed only
while its exact runtime-owned source slot is still leased. At use time the
runtime additionally checks the input epoch and mask, gameplay/skip gates,
manager and update clocks, stage/spell identity, player phase, Bomb-active
state, dialogue/freeze gates, and time-scale bits. The runtime drains all
available responses at an input boundary, discarding late packets and applying
only the exact target.

Snapshot releases are separate 24-byte `SOCK_SEQPACKET` records. They contain
magic `0x4c523854` (`T8RL`), version/size, the exact `u64` generation, a `u16`
slot index, and zeroed reserved fields. A release that does not own the exact
live lease closes the client. The solver sends an action response before any
queued releases so lease traffic cannot take priority over the input deadline.

Each packed slot begins with an 80-byte `T8SN` version-1 header followed by
16-byte range descriptors `(source address, size, data offset, kind)`. The
header binds generation, source input epoch, manager frame, FRScreen update
serial, total size, entry count, typed active-record counts, and complete
publication status. The request repeats generation, size, and entry count;
the solver rejects any mismatch before decoding.

## Verification boundary

The native i386 build and fixed-layout verifier pass with protocol version 4,
and `nm` exposes the 32-bit input-epoch deadline check. This proves
compilation, layout, and the static lease/drop design. Focused process tests
prove parser bounds, sparse local reconstruction, action-before-release wire
ordering, and background scale binding. It does not prove that a client-issued
lease has sufficient local or global action authority. The current solver
route therefore withholds continuation leases until its repeated action has a
same-version second-layer global predecessor. These checks do not prove 60 Hz
deadline performance, route survival, replay determinism, or original-v1.00d playback.
Those require retained online telemetry and a Linux-generated `.rpy` replayed
by the original executable under Wine.
