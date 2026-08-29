# Native 64-bit Linux port

TH08 now has a native-layout 64-bit Linux build alongside the original
fixed-layout i386 port. Both products compile the reconstructed authored game
sources and use the repository SDL2/OpenGL backend. Neither product executes
or bundles the original `th08.exe`.

## Architecture status

| Architecture | Build verification | Runtime verification |
| --- | --- | --- |
| x86_64 | ELF64 little-endian PIE; native globals; no fixed-address TH08 symbols | Title, demo replay, and Stage 5 gameplay resource load under Xvfb/Mesa for 40 seconds without a fatal signal |
| AArch64 | Cross-built ELF64 little-endian PIE with only AArch64 libraries | QEMU user-mode reaches DAT/version/logo/loading initialization; a real AArch64 desktop gameplay run remains required |

The x86_64 result is a playable 64-bit port, not merely a successful link. The
AArch64 artifact is build- and loader-verified, but emulated software OpenGL is
too slow to substitute honestly for a gameplay test on AArch64 hardware.

## Runtime data

Supply a legally obtained original Japanese TH08 data directory containing:

- `th08.dat`
- `thbgm.dat`

Start a packaged build with:

```bash
./run-th08.sh "/path/to/original/TH08 directory"
```

On Debian/Ubuntu x86_64, packaged runtime dependencies are:

```bash
sudo apt-get install \
  libstdc++6 libgl1 libfontconfig1 \
  libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-ttf-2.0-0 \
  fonts-vlgothic
```

Install the corresponding native packages on an AArch64 distribution.

The selected directory must be writable. TH08 creates configuration, score,
backup, replay, screenshot, and diagnostic files there. The port does not need
the original executable at runtime.

## Build

On an x86_64 Debian/Ubuntu host, install the native development libraries:

```bash
sudo apt-get install \
  g++ cmake ninja-build pkg-config \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
  libfontconfig1-dev libgl-dev
```

Then build and verify x86_64:

```bash
scripts/build-portable-linux.sh x86_64
```

For an AArch64 cross-build on the same host:

```bash
sudo dpkg --add-architecture arm64
sudo apt-get update
sudo apt-get install \
  g++-aarch64-linux-gnu \
  libsdl2-dev:arm64 libsdl2-image-dev:arm64 libsdl2-ttf-dev:arm64 \
  libfontconfig1-dev:arm64 libgl-dev:arm64
scripts/build-portable-linux.sh aarch64
```

On a native AArch64 host, the same command uses the host compiler rather than
the cross toolchain. Outputs are:

- `build/portable-linux-x86_64/th08-modern`
- `build/portable-linux-aarch64/th08-modern`

Create redistributable source-built archives with:

```bash
scripts/package-portable-linux.sh \
  build/portable-linux-x86_64/th08-modern x86_64
scripts/package-portable-linux.sh \
  build/portable-linux-aarch64/th08-modern aarch64
```

## Verification

Static verification rejects the old fixed-address i386 ownership model and
checks the requested ELF machine:

```bash
scripts/verify-portable-linux.sh \
  build/portable-linux-x86_64/th08-modern x86_64
scripts/verify-portable-linux.sh \
  build/portable-linux-aarch64/th08-modern aarch64
```

On a graphical x86_64 host, the isolated gameplay smoke test uses symlinks to
the two original archives, writes everything else below `build/`, and requires
the game to progress from the title screen into the bundled Stage 5 demo:

```bash
scripts/smoke-test-portable-linux.sh \
  build/portable-linux-x86_64/th08-modern \
  "/path/to/original/TH08 directory" 40
```

The check fails on an early exit, `modern-crash.txt`, or failure to request the
title, replay, SHT, STD, ECL, and message resources used by that route.

## Why a separate native layout exists

The exact reconstruction remains a 32-bit VC7 product. The i386 Linux port also
uses 32-bit pointers and a linker script that preserves target virtual
addresses. A 64-bit PIE cannot safely inherit either assumption.

The 64-bit build therefore defines `TH08_PORTABLE_NATIVE_LAYOUT` and keeps two
kinds of structure distinct:

- serialized files retain their original 32-bit offsets and exact byte sizes;
- live runtime objects use native pointers and native alignment.

Loaders now resolve, without truncation, the pointer-like fields in stage
background (`.std`), ECL, player shot (`.sht`), message, and replay data. Replay
save/load keeps the version-6 wire format compatible while maintaining a
native pointer table outside the serialized `0x134`-byte record.

Three target symbols were alternate names for bytes inside other objects. The
fixed i386 linker script supplied that identity automatically. Native layout
expresses the same ownership in C++: ECL time-scale flags are the Supervisor
flag word, the spell background ANM is the EffectManager field, and the two GUI
screen-effect counters share storage.

The VC7 build and comparison path never defines the native-layout macro. These
port changes do not claim new exact matches and must continue to pass the
repository's target-pinned comparison checks independently.
