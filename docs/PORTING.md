# Playable reconstruction ports

The binary-exact VC7 build and the playable modern ports are separate build
products. The existing Ninja/VC7 path remains the evidence path for the
original Japanese 1.00d executable. CMake builds `th08-modern` from the same
authored game sources without making an exact-code claim.

## Platform status

| Platform | Status | Notes |
| --- | --- | --- |
| Linux i386 | **Done** | Source build, one-command setup/run, and CI artifact are available |
| Linux x86_64 | **Playable** | Native-layout ELF64 PIE completed a Stage 1–6A Lunatic story route; CI package is available |
| Linux AArch64 | **Validation** | ELF64 PIE cross-build and QEMU startup pass; real-hardware gameplay remains |
| Windows x86 | **In progress** | Current native build/launcher is not yet a reliable distributable product |
| macOS | **In progress** | Platform backend and packaging are pending |

The ports compile the same production-authored game sources. The exact VC7 and
Linux i386 products retain the original pointer width and target-owned address
model. The 64-bit Linux products select a separate native runtime layout while
keeping serialized game and replay formats at their original byte widths.

### Native Windows

The Windows bring-up target keeps the original Win32, Direct3D 8, DirectInput
8, DirectSound, and WinMM backends. Wine is not intended to be part of its
runtime path, but the current native build and launcher have not produced a
reliable user-facing Windows package. Treat this target as development-only.

After creating the repository's existing DirectX 8 development prefix, build
the Windows executable from Linux with:

```bash
cmake -S . -B build/modern-windows \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32-toolchain.cmake
cmake --build build/modern-windows --parallel
```

The MinGW bring-up build uses the SDK-only `d3dx8d.dll` because the retail
`d3dx8.lib` is a static MSVC C++ archive and cannot be linked into MinGW C++.
This debug DLL is copied only from the developer's local SDK into `build/`; it
is not a redistributable project artifact. Replacing the remaining D3DX calls
is required before a distributable MinGW build.

On Windows with MSVC, the intended build is to configure with a Win32 generator and point
`TH08_DX8_SDK_ROOT` at a DirectX 8 SDK containing `include/` and `lib/`. Run it
natively from PowerShell with an arbitrary original-data directory once the
remaining Windows work is complete:

```powershell
& '.\th08-modern.exe' --data-dir 'D:\path\to\the\original\TH08 directory'
```

The MinGW executable and `d3dx8d.dll` must currently be kept together. The
user-supplied data directory need not contain either reconstructed runtime
file. The SDK DLL dependency and reported native startup failure both block a
Windows release artifact.

### Native Linux

Linux uses repository-owned compatibility backends: SDL2 for the window,
keyboard, timing, images, and PCM audio, plus fixed-function OpenGL for the
Direct3D 8 drawing surface. It does not use Wine, the original executable, or
the legacy DirectX SDK. The i386 product uses a linker script to preserve
target-owned global addresses. The x86_64/AArch64 products use native pointers,
PIE, explicit runtime ownership for target aliases, and 32-bit wire offsets for
retail data formats.

The first-time Debian/Ubuntu path needs only the original game-data directory:

```bash
scripts/setup-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The script validates `th08.dat` and `thbgm.dat`, installs missing i386
dependencies, builds, and launches. After initial setup, the normal incremental
build-and-play command is:

```bash
scripts/play-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The output is `build/modern-linux/th08-modern`. The build script uses a
repository CMake toolchain that passes `-m32` and constrains pkg-config to the
i386 package directory; CMake rejects a non-32-bit result. Docker is not part
of this default build or runtime path.

Build the native-layout products independently:

```bash
scripts/build-portable-linux.sh x86_64
scripts/build-portable-linux.sh aarch64
```

See [the 64-bit guide](PORTABLE_64BIT.md) for cross dependencies, architecture
verification, packaging, and the isolated title-to-gameplay smoke test. The
[render-audit guide](RENDER_AUDIT.md) documents deterministic replay autoplay,
the versioned CSV contract, and the framebuffer-probe adapter for later ports.

For CI or a host whose package manager cannot provide a clean multilib
development environment, an optional i386 container can compile the same ELF:

```bash
scripts/build-modern-linux-container.sh
TH08_LINUX_BINARY=build/modern-linux-container/th08-modern \
  scripts/run-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The container only compiles. The resulting executable still runs directly on
the Linux host and needs the corresponding 32-bit runtime libraries.

Every push and pull request runs the same container build in
`.github/workflows/portable-linux.yml`, verifies that the output is an i386
fixed-address ELF with the required target-owned layout symbols, and uploads
`th08-modern-linux-i386.tar.gz`. The archive preserves executable permissions
and contains the ELF, a standalone `run-th08.sh`, and runtime documentation.
It contains no original game data.

The executable still needs the legally obtained TH08 game data in its working
directory. `--data-dir` changes to the selected directory before the original
relative-file logic runs. The original executable and archives are never
embedded or copied into the modern build.

Only `th08.dat` and `thbgm.dat` are required at runtime; an A/B launch verified
that the Linux executable does not read `th08.exe`. A fresh two-DAT directory
generates its own configuration, score, backup, and log files. The Linux
`FindClose` compatibility boundary explicitly rejects the invalid search handle
returned for an initially empty backup directory, matching Win32's harmless
failure instead of dereferencing the sentinel. On low-resource virtual machines
without accelerated OpenGL, first-run fullscreen FPS/vsync calibration can
still appear stalled; reusing `th08.cfg` is an optional performance convenience.

The Linux renderer keeps the D3D8 backbuffer in an OpenGL framebuffer object
and restores a clean scene snapshot while dialogue deliberately pauses
background redraw. Its fixed-function mapping handles eye-space linear fog and
independent RGB/alpha texture-stage combiners. SDL_ttf, Fontconfig, and CP932
conversion provide the GDI text path used by Japanese dialogue.

The currently validated Linux path reaches the title/menu, playable stages,
and a complete story/ending transition with the player, enemies, bullets,
backgrounds, HUD, Japanese dialogue text, keyboard input, sound effects, and
streamed WAV BGM active. It has been exercised under WSLg and from the portable
archive in a Kali Linux x86-64 GUI VM. A fatal Linux signal writes
`modern-crash.txt` next to the selected game data, with addresses that can be
resolved against the non-PIE debug executable. MIDI output remains a
compatibility follow-up; ordinary WAV-mode gameplay does not depend on it.

See [Native Linux playable reconstruction](LINUX_PORTING.md) for distribution
notes, development overrides, architecture boundaries, failure analysis, and
the reusable lessons from the bring-up.

## Remaining port sequence

1. Run the AArch64 artifact through demo gameplay on a real Linux AArch64
   desktop and add hardware results to the validation matrix.
2. Keep Linux i386/x86_64 regression-covered and close the optional
   MIDI/controller gaps without changing replay-visible simulation behavior.
3. Finish a redistributable Windows x86 backend/package and validate startup
   on a clean native Windows host.
4. Add and validate the macOS backend after the portable boundary is stable.
