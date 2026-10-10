# Native Windows guide

## Status: in progress

The Windows port is in development. Its intended release will run natively
and accept a data directory containing `th08.dat` and `thbgm.dat` from your
original game installation. The current 32-bit MinGW build still needs startup
validation, packaging, and a redistributable replacement for D3DX.

The separate pinned-VC7 Windows i386 compile/link/play prerequisite is
complete. That build is used for developer validation. To reproduce it, follow
[Native Windows i386 reconstruction runtime](WINDOWS_I386_RUNTIME.md).

Developers can follow the modern build state in [Playable reconstruction
ports](PORTING.md#native-windows). A public Windows download and player-facing
installation commands will be added after native testing and packaging are
complete.

Release requirements include:

- reliable native window creation and gameplay on supported Windows hosts;
- redistributable replacements for the DirectX SDK debug DLL;
- a portable package with a data-directory launcher;
- end-to-end stage, dialogue, audio, input, and ending validation.
