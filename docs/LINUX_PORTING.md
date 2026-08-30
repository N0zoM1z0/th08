# Native Linux playable reconstruction

This document records the operational path and the engineering lessons from
bringing the reconstructed TH08 sources up as a native Linux executable. The
Linux product is separate from the VC7 exact-comparison build: it reuses the
authored game sources, but its platform compatibility code and modern compiler
output do not make a binary-exact claim.

## One-command setup and play

Supply a legally obtained original TH08 directory containing at least
`th08.dat` and `thbgm.dat`:

```bash
scripts/setup-modern-linux.sh "/path/to/the/original/TH08 directory"
```

On Debian or Ubuntu, this command:

1. validates the selected data directory before changing the host;
2. enables the `i386` package architecture when necessary;
3. installs the 32-bit compiler, SDL2, SDL2_image, SDL2_ttf, Fontconfig,
   OpenGL, and a Japanese font when they are missing;
4. configures and incrementally builds `build/modern-linux/th08-modern`;
5. runs the native ELF with the selected directory as `--data-dir`.

The script uses `sudo` only for missing system packages. It never embeds,
copies, moves, or links the original archives. Once dependencies are present,
the shorter normal entry point is:

```bash
scripts/play-modern-linux.sh "/path/to/the/original/TH08 directory"
```

`play-modern-linux.sh` performs an incremental build before every launch. To
separate the steps for development or debugging, use:

```bash
scripts/build-modern-linux.sh
scripts/run-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The native-layout 64-bit products have a separate build entry point so the
fixed-layout i386 release remains regression-testable:

```bash
scripts/build-portable-linux.sh x86_64
scripts/build-portable-linux.sh aarch64
```

See [Native 64-bit Linux port](PORTABLE_64BIT.md) for dependency, packaging,
wire-format, and runtime-verification details.

The selected directory becomes the process working directory. The original
relative-file behavior therefore reads and writes configuration, score,
replay, screenshots, and diagnostic files there. The original executable is
not read or executed by the Linux port.

A runtime A/B test verifies that a directory containing only `th08.dat` and
`thbgm.dat` starts without `th08.exe`, then creates configuration, score,
backup, and log files. The first version of that test was stopped during slow
software-rendered calibration and therefore did not cover the subsequent
score-backup path. On Kali, the exact CI artifact later exited with status 139
immediately after its final `score.dat` access. Its crash report was easy to
miss because `StartupThread` had already changed into `backup/`; the report was
`backup/modern-crash.txt`, and its stack identified `FindClose` with fault
address `0xffffffff`.

The authored startup code follows the Win32 behavior of calling `FindClose`
even when `FindFirstFileA` found no matching backup. Windows returns failure for
`INVALID_HANDLE_VALUE`, but the initial Linux compatibility implementation
deleted that sentinel as a pointer. The backend now rejects null, invalid, and
wrong-kind find handles before deletion. A clean-directory regression creates
the first score backup and reaches the title assets without a crash. A fresh
fullscreen configuration can still make FPS/vsync calibration slow on a VM
without accelerated OpenGL; reusing `th08.cfg` is optional and unrelated to the
fixed exit.

## Downloadable CI package

`.github/workflows/portable-linux.yml` builds the same source in the
repository-owned i386 container on every push and pull request. Before upload,
the workflow verifies the ELF class, machine, fixed-address executable type,
and representative target-owned layout aliases. It then publishes a compressed
archive plus SHA-256 checksum as the `th08-modern-linux-i386` Actions artifact.
The Docker build context is allowlisted to the container recipe itself; local
targets, DAT files, analysis state, and the source tree are not sent into the
image build. Compilation receives the checked-out tree through a temporary
bind mount.

The archive contains `th08-modern`, a sibling-aware `run-th08.sh`, the
project-owned `th08-modern.png` application icon, focused runtime instructions,
and these porting notes. It intentionally contains no original game executable
or data. After installing the documented i386 runtime libraries, an artifact
user only needs:

```bash
./run-th08.sh "/path/to/the/original/TH08 directory"
```

The tar archive is the distribution boundary rather than a bare Actions file
so the executable bit and the expected launcher/binary layout survive download
and extraction. `scripts/package-modern-linux.sh` creates the same artifact
locally from any verified `th08-modern` ELF.

The workflow also builds x86_64 and AArch64 ELF64 PIE in an architecture
matrix. It rejects fixed-address TH08 globals in those binaries and publishes
`th08-portable-linux-x86_64` and `th08-portable-linux-aarch64` artifacts.

### Other distributions and isolated builds

Automatic package installation currently supports Debian and Ubuntu on x86 or
x86-64 hosts. Other distributions need equivalent i386 multilib development
packages for SDL2, SDL2_image, SDL2_ttf, Fontconfig, and OpenGL. The port is a
32-bit x86 program because reconstructed object layouts and target-owned data
addresses still depend on the original pointer width.

An optional container can isolate compilation:

```bash
scripts/build-modern-linux-container.sh
TH08_LINUX_BINARY=build/modern-linux-container/th08-modern \
  scripts/run-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The container is never the runtime. The resulting ELF still executes on the
host and needs 32-bit runtime libraries and a real GUI/audio session.

Useful development overrides are:

| Variable | Purpose |
| --- | --- |
| `TH08_LINUX_BUILD_DIR` | Repository-relative CMake build directory |
| `TH08_LINUX_BUILD_JOBS` | Parallel build job count; defaults to one |
| `TH08_LINUX_BINARY` | Executable selected by `run-modern-linux.sh` |
| `TH08_LINUX_PKG_CONFIG_LIBDIR` | i386-only pkg-config search path |
| `TH08_PORTABLE_BUILD_DIR` | Repository-relative x86_64/AArch64 build directory |
| `TH08_PORTABLE_BUILD_JOBS` | Native-layout parallel build count; defaults to one |
| `TH08_PORTABLE_PKG_CONFIG_LIBDIR` | Architecture-specific native-layout pkg-config path |
| `TH08_FONT` | Explicit readable Japanese font file |

## Port boundary

The authored gameplay, ECL, animation, archive, and manager implementations
remain shared with the exact reconstruction. Linux-specific behavior is kept
under `src/modern/linux/` and selected only by the independent CMake product:

- SDL2 supplies the window, keyboard input, timing, PCM audio boundary, and
  image decoding; the window icon is loaded from the project-owned PNG beside
  the ELF rather than copied from the original executable;
- fixed-function OpenGL implements the Direct3D 8 device surface;
- SDL_ttf, Fontconfig, and CP932-to-UTF-8 conversion implement the GDI text
  boundary used for Japanese dialogue and dynamic labels;
- a non-PIE i386 ELF plus `th08-layout.ld` preserves target-owned data
  addresses that reconstructed source still references directly;
- x86_64 and AArch64 are PIE and resolve serialized 32-bit offsets into native
  pointer tables while expressing overlapping target symbols as aggregate
  fields rather than absolute linker aliases.

This separation matters. A portable backend fix must not be expressed as an
`#ifdef` inside an already reconstructed gameplay function when it can be
implemented at the platform API or link boundary.

## Lessons from the reconstruction

### Preserve object identity, not just symbol values

The original image contains many symbols that are alternate views into a
larger aggregate. For example, a separately named calc-chain global can be the
same bytes as a manager field at a fixed offset. Letting a modern linker create
independent ELF BSS objects produces valid-looking pointers with the wrong
identity.

The failure mode is much wider than the incorrectly split symbol:

- old callbacks survive reloads and run more than once per frame;
- spell bonuses decay multiple times, become negative, wrap an unsigned score,
  and saturate the display;
- Boss bars, Bomb animations, spell transitions, and scripted enemies appear
  intermittently missing;
- later stages can stall or exit even though the immediate callback looks
  correct.

The Linux linker script therefore aliases every target-owned manager, chain,
and known field-view symbol to the original address. A useful audit is that no
such runtime global remains as an unintended ordinary ELF `B` symbol. When a
new alias is added, verify both the aggregate address and the field offset.

### Rebuild target-initialized data explicitly

Some exact source expressions refer to tables and strings that the original PE
loader mapped at fixed addresses. A modern ELF cannot obtain those bytes from
the copyrighted executable. The Linux runtime initializes the evidence-backed
tables and string data needed by authored code, while the linker reserves the
target data arena as `NOLOAD` so it does not inflate the executable.

Keep this initialization at the port boundary and document its provenance.
Do not replace it with gameplay defaults scattered through authored source.

### Backbuffer semantics are gameplay-visible

TH08 used Direct3D 8 copy-swap behavior and does not redraw every visual layer
on every frame. In particular, authored dialogue rendering deliberately skips
the 3D background. SDL/Wayland swapchain contents are not stable enough to
inherit this behavior accidentally.

The Linux renderer owns a texture-backed 640x480 framebuffer. At dialogue
entry it captures a clean scene snapshot and restores that same snapshot
before every dialogue frame. This preserves the paused background without
accumulating bullets, items, portraits, or text from later frames. A persistent
framebuffer alone is insufficient: it retains moving foreground layers too.

A common portability shortcut is to force a complete redraw on every frame.
TH08 instead needs the more specific snapshot because its authored dialogue
path intentionally omits background drawing.

### Fixed-function state must be translated component by component

Direct3D 8 has separate color and alpha texture-stage operations. Mapping both
to a single OpenGL `GL_MODULATE` works for ordinary sprites but fails when one
component selects the texture and the other selects diffuse or texture-factor
color. One observed Stage 5 transition consequently tiled a dynamic
"Yakumo Yukari" text texture over the playfield and HUD.

The backend now configures `GL_COMBINE_RGB` and `GL_COMBINE_ALPHA`
independently, including their `TEXTURE`, `DIFFUSE`, and `TFACTOR` sources.
Pure-color primitives disable texturing when neither component uses it. The
mapping follows the D3D8 component semantics rather than relying on the most
common sprite blend as a proxy for every texture stage.

### CPU projection still needs eye-space fog

The compatibility device projects D3D vertices to screen space on the CPU and
then submits them through an orthographic OpenGL projection. Default OpenGL
fog would therefore see the post-projection depth instead of TH08's authored
eye-space distance. That made Stage 2's darkness and other background fog
effects disappear.

The backend retains eye-space Z after world/view transformation and submits it
as an explicit fog coordinate. `D3DRS_FOGCOLOR`, `FOGSTART`, `FOGEND`, and
linear vertex-fog state map to the corresponding fixed-function OpenGL state.

### Dynamic text crosses several format boundaries

Dialogue and spell labels originate as CP932 text, render through a GDI-like
surface, and are copied into D3D textures. The Linux path must preserve all of
the following:

- CP932-to-UTF-8 conversion before SDL_ttf rendering;
- a Japanese font selected through Fontconfig or `TH08_FONT`;
- the original 16/32-bit surface channel and alpha layouts;
- destination rectangles and dirty/upload state when a texture is modified.

Seeing text on screen proves only the encoding/font half. Stage transitions,
Bomb names, spell names, and dialogue must also be tested because they mutate
different regions of shared texture atlases.

### Test state transitions, not only isolated stages

Title-screen and Stage 1 smoke tests missed the most important failures. A
portable validation run should cover at least:

1. a captured spell and score award;
2. a death, Bomb, focus marker, and Boss bar;
3. Stage 1 dialogue with a stable background;
4. Stage 2 fog/darkness;
5. a stage reload or retry;
6. the Stage 4 route split and a callback-heavy spell;
7. the Stage 4-to-5 resource transition;
8. a complete ending/result transition when available.

Non-PIE debug builds, frame pointers, `-rdynamic`, and `modern-crash.txt` make
Linux failures symbolizable without altering authored code. Runtime GDB
patches used for endurance testing are analysis aids only and are never part
of the shipped executable.

### Validated Linux environments

WSLg exercises the native ELF, SDL, OpenGL, audio, Unicode path, and 32-bit
runtime, so it is valuable for rapid iteration. Its window lifecycle, process
signaling, GPU translation, and host-filesystem behavior differ from a normal
Linux desktop. The WSLg endurance pass reached a complete Lunatic story clear
without a fatal signal; its external no-life-decrement GDB command was a test
aid only and is not part of the executable or portable archive.

The downloadable portable package was also user-tested successfully in a Kali
Linux x86-64 GUI virtual machine using native filesystem data. That VM had low
memory and no 3D acceleration, making initial software-rendered calibration
unusually slow, but the game started and ran. A physical Linux desktop with
hardware OpenGL remains useful final release coverage rather than a prerequisite
for calling the existing WSLg and Kali paths validated.

The native-layout x86_64 PIE completed a user-driven Sakuya/Remilia Lunatic
route under WSLg: Stages 1 through 6A, the ending, results, and return to title.
The Stage 4-to-5 transition that had failed in an earlier fixed-layout build
completed normally. Isolated Xvfb/Mesa smoke tests still cover the bundled
Stage 5 demo, while replay-driven render audits can start a selected recorded
stage and compare enemy ANM source regions with the pixels changed by each draw.
The AArch64 PIE has been cross-linked exclusively against arm64 libraries and
reaches DAT/version/logo/loading initialization under QEMU user-mode. Emulated
software OpenGL is too slow here to represent a real-hardware gameplay result.

### Native clears and render oracles

Serialized records keep target wire sizes, but bulk initialization of live
objects must use the selected runtime layout. A fixed 32-bit `memset` size can
compile and survive startup on x86_64 while silently leaving pointer-expanded
tail fields stale. The resulting failure may appear much later—for example, an
uncleared enemy timeline prevented natural stage transitions. Keep the target
constant in the exact/fixed-layout branch and use `sizeof` only for the native
layout.

Visual diagnostics should separate three questions instead of relying on a
screenshot alone:

1. Does the VM reference a loaded sprite and texture with valid geometry?
2. Does the source UV region contain visible pixels after the VM/global ANM
   color modulation is applied?
3. Does flushing that exact draw change the expected framebuffer region?

`TH08_RENDER_AUDIT=1` enables this sampled oracle, and
`scripts/audit-render-replay-linux.sh` combines it with a user replay and
software-rendered Xvfb session. The CSV preserves stage/frame, enemy and boss
identity, ANM script/sprite, both VM colors, selected render color, texture
statistics, and framebuffer deltas. This makes missing assets, stale pointers,
off-screen draws, scripted fades, and post-texture rendering failures distinct
enough to triage without watching an entire route.
See [Replay-driven render audit](RENDER_AUDIT.md) for the schema and the small
backend contract intended for reuse by later ports. Its cross-report comparator
aligns the same replay frames across architectures without requiring identical
edge rasterization.

Do not preserve a multi-field value by casting the address of one scalar local
to an aggregate pointer. VC7 `#pragma var_order` can make adjacent floats act
like a `Float2`, but a modern compiler may separate or reverse those locals.
`EnemyManager::OnDrawImpl` requires a real portable `Float2` when saving and
restoring the scale around trail drawing; otherwise x86_64 can restore NaN as
the Y scale and make a valid boss texture disappear. Keep the scalar source
shape only in the target comparison branch.

## Current limitations

- MIDI output is a compatibility stub; normal WAV-mode gameplay is the
  validated audio path.
- Controller mapping is not yet a compatibility target; keyboard input is.
- AArch64 gameplay has not yet been exercised on real hardware; current
  coverage is cross-build, ELF ownership checks, and QEMU loader/startup.
- Windows and macOS remain in progress; neither currently has a release-ready
  native package.
