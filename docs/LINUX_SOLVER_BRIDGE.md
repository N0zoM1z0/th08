# Linux solver lockstep bridge

This branch contains an experimental, Linux-only input synchronization bridge
for the TH08 autoplay solver. It is not part of the exact VC7 reconstruction
claim and is inactive unless `TH08_SOLVER_SOCKET` names a Unix-domain socket.
Normal interactive Linux execution continues to read SDL keyboard state.

## Boundary

The hook is in the Linux DirectInput backend. Each real call made by
`Controller::GetInput` produces one protocol epoch. The bridge supplies only
keyboard state; shared source still owns current/last input, held-key repeat,
callback ordering, gameplay state, RNG, and replay recording.

While waiting for a response, the bridge freezes the QPC and millisecond clock
values returned by the Linux compatibility layer. On response it subtracts the
real wait from future game-visible time. Ordinary rendering and 60 Hz admission
resume afterward. No headless or faster-than-real behavior is implemented.

Bridge mode stamps `g_Supervisor.exeSize=840704` and
`exeChecksum=2724749753` before replay creation so the replay identifies its
intended Japanese v1.00d playback target. This is replay compatibility
metadata, not a claim that the Linux ELF is the retail executable.

## Socket lifecycle

Set an absolute socket path before launch:

```bash
TH08_SOLVER_SOCKET=/tmp/owned-run-directory/bridge.sock \
  build/modern-linux/th08-modern --data-dir /path/to/data
```

Bridge mode preserves lives by default, matching the analysis patch used for
full-route hit counting. Set `TH08_SOLVER_PRESERVE_LIVES=0` only for diagnostic
runs that must reach the retail game-over/result-screen replay-save path. The
request flags attest which behavior is active; a no-preserve run is not an
NMNB result.

The runtime creates the socket and blocks at the first input sample until one
client connects. It never deletes an existing path and does not unlink the
socket on exit. The launcher must create and later remove its own exact run
directory. There is deliberately no response-duration timeout: solver compute
time may be arbitrarily long. EOF or a protocol violation permanently changes
the bridge to neutral input for the rest of that process; it never falls back
to SDL.

## Version 1 wire format

All integers are unsigned little-endian. Requests are 32 bytes:

| Offset | Type | Meaning |
| --- | --- | --- |
| `0` | `u32` | magic `0x51523854` (`T8RQ`) |
| `4` | `u16` | protocol version `1` |
| `6` | `u16` | request size `32` |
| `8` | `u64` | monotonically increasing input epoch |
| `16` | `u16` | current input before this sample |
| `18` | `u16` | previous input before this sample |
| `20` | `u16` | current RNG seed |
| `22` | `u16` | reserved, zero |
| `24` | `u32` | flags; bit 0 means replay target identity stamped, bit 1 means lives preserved |
| `28` | `u32` | cumulative solver-wait milliseconds, saturating |

Responses are 24 bytes:

| Offset | Type | Meaning |
| --- | --- | --- |
| `0` | `u32` | magic `0x53523854` (`T8RS`) |
| `4` | `u16` | protocol version `1` |
| `6` | `u16` | response size `24` |
| `8` | `u64` | exact request epoch |
| `16` | `u16` | complete TH08 logical input mask |
| `18` | `u16` | reserved, zero |
| `20` | `u32` | reserved, zero |

Bomb (`0x0002`), unknown bits, up+down, and left+right are rejected. Every
transaction is full-width and exact-epoch; partial I/O is completed internally.
The solver reads bulk state through a local process-memory adapter while the
runtime is blocked, keeping game layouts out of this protocol.

## Evidence boundary

The i386 build and fixed-layout verifier pass with the bridge compiled. A live
version-1 neutral-input handshake has completed, including a 2.2-second delayed
response and fail-closed peer disconnect. This proves the synchronization path,
not replay determinism. The next gates are cumulative clock telemetry, invalid
Bomb response, a Windows-origin replay differential, then a Linux-generated
replay played by the original executable.
