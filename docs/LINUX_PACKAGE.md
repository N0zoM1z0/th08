# TH08 modern Linux i386 package

This archive contains the native Linux playable reconstruction. It does not
contain the original TH08 executable, DAT archives, music, or other copyrighted
game assets. `th08-modern.png` is the project-owned application icon used by
the SDL window; it is not extracted from the original game.

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

A directory containing only the two DAT files is sufficient and the port will
create `th08.cfg` and `score.dat`. On a VM without accelerated OpenGL, the
fresh configuration's fullscreen startup and first-run FPS/vsync calibration
may be very slow. Reusing an existing `th08.cfg`, or selecting the complete
original installation directory that contains it, avoids that misleading
first-start delay; the EXE's presence in that directory is incidental.

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
OpenGL/audio sessions are required; Docker is used only to make the CI build
reproducible and is not part of the runtime.

See `PORTING.md` in this archive for implementation details, known limitations,
and debugging notes.
