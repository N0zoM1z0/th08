# Native Windows guide

## Status: native windowed smoke passed; gameplay validation in progress

There is no supported native Windows release asset yet. Do not download the
Linux package for Windows, and do not treat the current development executable
as a redistributable build.

The intended Windows product runs directly on Windows without Wine and accepts
an arbitrary original-data directory containing `th08.dat` and `thbgm.dat`.
Current source produces a verified 32-bit MinGW bring-up executable. A native
Windows 11 windowed smoke now reaches the title resources and audio startup,
survives the bounded run, and exits normally. Manual gameplay coverage and the
redistributable D3DX replacement remain incomplete.

## Local build and isolated playtest directory

The development build still uses the non-redistributable DirectX 8 SDK debug
DLL. With the repository development prefix already installed, build and
validate the PE32 i386 executable with:

```bash
scripts/build-modern-windows.sh
```

Prepare an isolated directory for a real Windows playtest from WSL with:

```bash
scripts/deploy-modern-windows-playtest.sh \
  "/mnt/d/path/to/original-th08-directory" \
  "/mnt/d/path/to/th08-reconstruct"
```

The deployment copies the reconstructed executable, the local SDK debug DLL,
the two required DAT archives, an existing configuration and score file, and
an existing replay directory. It never copies the original executable and
does not modify the source installation. Existing mutable files in the
playtest directory are preserved on later deployments. Double-click
`run-th08-reconstruction.bat` in the resulting directory. The launcher passes
`--windowed`, so testing does not depend on a fullscreen-compatible display
path; the selected mode is also saved to the playtest copy of `th08.cfg` after
a normal exit.

`TH08_WINDOWS_BUILD_DIR` selects a different repository-relative CMake build
directory. Builds are serial by default because the reconstruction workflow
permits only one active compiler job; `TH08_WINDOWS_BUILD_JOBS` is available
for environments that explicitly allow a different value.

## Native startup smoke

From WSL on a Windows host, the bounded smoke helper starts only the deployed
reconstruction, requires a top-level window and continued process liveness,
requests a normal window close, checks exit code zero, and rejects a newly
written crash report:

```bash
powershell.exe -NoProfile -ExecutionPolicy Bypass \
  -File "$(wslpath -w "$PWD/scripts/smoke-modern-windows-host.ps1")" \
  -PlaytestDirectory 'D:\path\to\th08-reconstruct' \
  -Seconds 20
```

The 2026-09-10 checkpoint passed on a native Windows host in windowed mode.
`modern-files.txt` showed the expected configuration, score, version, title,
loading, sound-effect, text, capture, BGM format, and title-screen resource
requests. The DirectSound and keyboard initialization log completed, the
process survived the test interval, WM_CLOSE produced exit code zero, and no
`modern-crash.txt` was created. This is startup/title evidence, not a gameplay
or playable-status claim.

Developers can follow the platform boundary in [Playable reconstruction
ports](PORTING.md#native-windows). A public Windows download will be added only
after manual gameplay testing and removal of the SDK-only runtime dependency.

Release requirements include:

- broader native gameplay, transition, replay, and ending validation on
  supported Windows hosts;
- no dependency on the non-redistributable DirectX SDK debug DLL;
- a portable package with a data-directory launcher;
- end-to-end stage, dialogue, audio, input, and ending validation.
