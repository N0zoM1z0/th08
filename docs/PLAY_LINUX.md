# Download, install, and play on Linux

The native Linux reconstruction is the current playable release target. The
release lane includes the established 32-bit x86 product and a native-layout
x86_64 product; an AArch64 build is available for hardware validation. Wine,
Docker, and the original `th08.exe` are not runtime requirements.

## What you need

- an x86 or x86-64 Linux desktop with working OpenGL and audio;
- the runtime packages listed below;
- a legally obtained original Japanese TH08 1.00d data directory containing:
  - `th08.dat`
  - `thbgm.dat`

Those two DAT archives are the only copyrighted game files required by the
port. They are not included in the release. The selected data directory must
be writable because TH08 creates `th08.cfg`, `score.dat`, replays, screenshots,
backups, and diagnostic logs there.

## 1. Download the portable release

Open the [TH08 releases page](https://github.com/N0zoM1z0/th08/releases) and
download both Linux assets from the newest Linux Preview release:

- `th08-modern-linux-i386.tar.gz`
- `th08-modern-linux-i386.tar.gz.sha256`

The GitHub Actions artifact is useful for development snapshots, but the
Release assets are the stable download entry point for players.

Native x86_64 and AArch64 snapshots currently live on
[`port/portable-64bit`](https://github.com/N0zoM1z0/th08/tree/port/portable-64bit).
Open that branch's
[Portable Linux workflow](https://github.com/N0zoM1z0/th08/actions/workflows/portable-linux.yml?query=branch%3Aport%2Fportable-64bit),
select a successful run, and download the artifact matching your architecture.

## 2. Install the 32-bit runtime libraries

On Debian, Ubuntu, Kali Linux, or a compatible derivative running on x86-64:

```bash
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install \
  libstdc++6:i386 libgl1:i386 libfontconfig1:i386 \
  libsdl2-2.0-0:i386 libsdl2-image-2.0-0:i386 libsdl2-ttf-2.0-0:i386 \
  fonts-vlgothic
```

Other distributions need equivalent i386 packages for the C++ runtime,
OpenGL, Fontconfig, SDL2, SDL2_image, and SDL2_ttf, plus a Japanese font. The
launcher itself never invokes `sudo`.

## 3. Verify and extract the package

Run these commands in the directory containing the two downloaded files:

```bash
sha256sum -c th08-modern-linux-i386.tar.gz.sha256
tar -xzf th08-modern-linux-i386.tar.gz
cd th08-modern-linux-i386
```

Do not continue if the checksum command reports a mismatch.

## 4. Start TH08

Pass the directory containing your original DAT archives:

```bash
./run-th08.sh "/path/to/original/TH08 directory"
```

For example, if the extracted package is a child of the data directory:

```text
TH08/
├── th08.dat
├── thbgm.dat
└── th08-modern-linux-i386/
```

start it from inside the package with:

```bash
./run-th08.sh ..
```

The path is resolved at launch and is never hard-coded into the executable.
Spaces and non-ASCII characters are supported when the path is quoted.

## First launch

With only the two DAT files present, the game creates its configuration, score,
backup directory, and logs automatically. An existing `th08.cfg` is optional.
On a low-resource VM without 3D acceleration, the first fullscreen FPS/vsync
calibration can be slow and may temporarily look stalled.

The empty-backup startup path is covered by a regression test: the Linux Win32
compatibility layer now rejects an invalid file-search handle harmlessly,
creates the first score backup, and continues into the title and stage assets.

## Build and run from a source checkout

On Debian or Ubuntu, the one-command developer path installs missing build
dependencies, builds the native i386 executable, and launches it:

```bash
scripts/setup-modern-linux.sh "/path/to/original/TH08 directory"
```

After the initial setup, use the incremental path:

```bash
scripts/play-modern-linux.sh "/path/to/original/TH08 directory"
```

Those commands retain the fixed-layout i386 product. For native ELF64 PIE,
build and verify with:

```bash
scripts/build-portable-linux.sh x86_64
scripts/build-portable-linux.sh aarch64
```

The AArch64 command cross-builds on x86_64 when the documented arm64
development packages are installed. See [Native 64-bit Linux
port](PORTABLE_64BIT.md) for package, launch, and smoke-test commands.

Docker is available only as an optional reproducible build environment; it is
not required to run the resulting game. See [Native Linux playable
reconstruction](LINUX_PORTING.md) for backend architecture, build isolation,
validation coverage, and porting lessons.

## Troubleshooting

Run the launcher from a terminal so early errors remain visible:

```bash
./run-th08.sh "/path/to/original/TH08 directory"
echo $?
```

Check the executable and unresolved runtime libraries with:

```bash
file ./th08-modern
ldd ./th08-modern | grep "not found"
```

Runtime diagnostics are written inside the selected data directory:

- `log.txt`: normal game log;
- `modern-files.txt`: files requested during the current run;
- `modern-crash.txt`: symbolizable fatal-signal report;
- `backup/modern-crash.txt`: possible early-startup report while score backups
  are being rotated.

If the program exits unexpectedly, preserve those files and report the exit
code, distribution, desktop session, GPU/OpenGL environment, and the exact
launch command. Do not upload the DAT archives, original executable, score
files, or replays unless they are specifically needed and safe to share.

## Current limitations

- MIDI output is a compatibility stub; the normal WAV BGM path is validated.
- Keyboard input is validated; controller compatibility is not yet a release
  target.
- AArch64 has build and emulated-loader coverage; gameplay validation on a real
  AArch64 desktop remains pending.
- Stage 4-to-5 dynamic-text tiling and missing enemy/boss art were observed in
  earlier bring-up builds, but were not reproduced in the final x86_64
  full-route and Practice passes. Preserve diagnostics if either regression
  returns on another driver or desktop.
