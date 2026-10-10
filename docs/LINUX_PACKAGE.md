# TH08 modern Linux i386 package

This archive contains the native Linux i386 executable, launcher, and
project-owned window icon. Supply the game data from your own TH08 installation.

## Run

When this tarball and its `.sha256` file come from the GitHub Actions artifact,
verify it before extraction:

```bash
sha256sum -c th08-modern-linux-i386.tar.gz.sha256
tar -xzf th08-modern-linux-i386.tar.gz
cd th08-modern-linux-i386
```

Then pass the directory from your legally obtained original Japanese TH08
1.00d installation:

```bash
./run-th08.sh "/path/to/the/original/TH08 directory"
```

The directory must contain `th08.dat` and `thbgm.dat`. It may live anywhere;
neither the launcher nor the executable has a hard-coded data path. The
selected directory becomes the working directory, so configuration, score,
replay, screenshot, and crash-diagnostic files are read or written there.
The original `th08.exe` is not read or executed.

A directory containing only the two DAT files is sufficient. The game creates
`th08.cfg`, `score.dat`, and the score-backup directory on first launch.
On a VM without accelerated OpenGL, fullscreen startup and FPS/vsync calibration
may be slow. An existing `th08.cfg` can avoid that delay.

## Runtime requirements

The current executable is a dynamically linked 32-bit x86 ELF. On a 64-bit
Debian or Ubuntu installation, install its runtime libraries with:

```bash
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install \
  libstdc++6:i386 libgl1:i386 libfontconfig1:i386 \
  libsdl2-2.0-0:i386 libsdl2-image-2.0-0:i386 libsdl2-ttf-2.0-0:i386 \
  fonts-vlgothic
```

Equivalent i386 SDL2, SDL2_image, SDL2_ttf, Fontconfig, OpenGL, and C++ runtime
packages are required on other distributions. A graphical desktop and working
OpenGL/audio sessions are required. The executable runs directly on the host.

See `PORTING.md` in this archive for implementation details, known limitations,
and debugging notes.
