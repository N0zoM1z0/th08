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

If no exact response is ready, the complete current input mask remains held.
A late response is counted and discarded; it is never applied on a later
frame. Accept, receive, and publish operations are all non-blocking on the game
thread. `enemy_manager_frame` is not used as the input clock because it can
freeze while held input still moves the player. The exported ELF symbol
`th08_solver_input_epoch` brackets generation-safe `/proc/<pid>/mem` capture.

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
miss alone preserves the complete held hard-no-Bomb mask. A protocol violation
closes only that client and therefore returns input to neutral.

Bridge mode preserves lives by default for hit-counting diagnostics. Set
`TH08_SOLVER_PRESERVE_LIVES=0` only for an explicitly diagnostic run that must
reach the retail game-over/result-screen replay-save path. The request flags
attest the selected behavior; a preserved-life run is not itself an NMNB
completion claim.

## Version 2 wire format

Each packet is one complete little-endian `SOCK_SEQPACKET` record. Requests are
56 bytes:

| Offset | Type | Meaning |
| --- | --- | --- |
| `0` | `u32` | magic `0x51523854` (`T8RQ`) |
| `4` | `u16` | protocol version `2` |
| `6` | `u16` | request size `56` |
| `8` | `u64` | completed-update source input epoch |
| `16` | `u64` | exact target input epoch, always source + 1 |
| `24` | `u16` | input active during the completed source update |
| `26` | `u16` | preceding input |
| `28` | `u16` | post-update RNG seed |
| `30` | `u16` | reserved, zero |
| `32` | `u32` | flags: replay target stamped; lives preserved |
| `36` | `u32` | cumulative missed response deadlines, saturating |
| `40` | `u32` | cumulative discarded late responses, saturating |
| `44` | `u32` | cumulative dropped request publications, saturating |
| `48` | `u64` | `CLOCK_MONOTONIC` publication time in microseconds |

Responses are 32 bytes:

| Offset | Type | Meaning |
| --- | --- | --- |
| `0` | `u32` | magic `0x53523854` (`T8RS`) |
| `4` | `u16` | protocol version `2` |
| `6` | `u16` | response size `32` |
| `8` | `u64` | request source epoch |
| `16` | `u64` | exact target epoch, always source + 1 |
| `24` | `u16` | complete TH08 logical input mask |
| `26` | `u16` | reserved, zero |
| `28` | `u32` | reserved, zero |

Bomb (`0x0002`), unknown bits, up+down, left+right, wrong-sized packets,
truncated packets, stale targets, and unrequested future targets are rejected.
The runtime drains all available responses at an input boundary, discarding
late packets and applying only the exact target.

## Verification boundary

The native i386 build and fixed-layout verifier pass with protocol version 2,
and `nm` exposes the 32-bit input-epoch bracket. This proves compilation,
layout, and the static non-blocking design. It does not prove 60 Hz deadline
performance, route survival, replay determinism, or original-v1.00d playback.
Those require retained online telemetry and a Linux-generated `.rpy` replayed
by the original executable under Wine.
