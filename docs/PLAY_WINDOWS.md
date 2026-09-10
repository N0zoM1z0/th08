# Native Windows guide

## Status: in progress

There is no supported native Windows release asset yet. Do not download the
Linux package for Windows, and do not treat the current development executable
as a redistributable build.

The intended Windows product will run directly on Windows without Wine and
will accept an arbitrary original-data directory containing `th08.dat` and
`thbgm.dat`. Current source can produce a 32-bit MinGW bring-up executable, but
that modern executable's startup/package validation and redistributable D3DX
replacement remain incomplete.

The separate pinned-VC7 Windows i386 compile/link/play prerequisite is
complete. It is a developer validation artifact, not the redistributable
Windows product. Developers who need to reproduce that gate should follow
[Native Windows i386 reconstruction runtime](WINDOWS_I386_RUNTIME.md).

Developers can follow the modern build state in [Playable reconstruction
ports](PORTING.md#native-windows). A public Windows download and player-facing
installation commands will be added here only after the modern executable and
package have passed real Windows testing.

Release requirements include:

- reliable native window creation and gameplay on supported Windows hosts;
- no dependency on the non-redistributable DirectX SDK debug DLL;
- a portable package with a data-directory launcher;
- end-to-end stage, dialogue, audio, input, and ending validation.
