# 東方永夜抄 ～ Imperishable Night

<h3 align="center">🌙「永夜已明」Authored reconstruction complete — Linux playable ✓ · Windows/macOS in progress · Web edition is the next spell card 💫</h3>

<p align="center">
  <img
    src="resources/title-screen.png"
    width="640"
    alt="Original Japanese TH08 1.00d title screen">
</p>

<p align="center">
  <img src="resources/progress.svg" alt="TH08 exact-source and playable-platform progress">
</p>

## Platform guides

| Platform | Guide | Status |
| --- | --- | --- |
| Linux | **[Download, install, and play](docs/PLAY_LINUX.md)** | **Playable** |
| Windows | [Native Windows guide](docs/PLAY_WINDOWS.md) | In progress |
| macOS | [Native macOS guide](docs/PLAY_MACOS.md) | In progress |

The Web edition is planned as a future playable target after the native
desktop ports. It is not available yet.

This project reconstructs the source code of the original Japanese
`東方永夜抄 ～ Imperishable Night` version 1.00d executable. All 1,107 authored
functions are now present in source, and 1,105 are accepted as byte-exact by
reproducible comparison. The authored-source recovery milestone is complete;
current work focuses on whole-image reconstruction, compiler/runtime libraries,
and playable Windows, Linux, and macOS products.

The repository continues the work of
[GensokyoClub/th08](https://github.com/GensokyoClub/th08). Its complete Git
history was imported rather than squashed, preserving the authorship and
contribution record of the original project. New infrastructure and
reconstruction work build on that baseline.

The project remains active reconstruction and platform-engineering work.
Existing source, symbol mappings, or generated progress artwork must not be
interpreted as a new exact matching percentage without a reproducible report
against the target binary. Current source-presence inventory is generated in
[docs/PROGRESS.md](docs/PROGRESS.md) and is deliberately labeled separately
from strict exact-match coverage.

## Target executable

Supply your own original executable as `resources/th08.exe`:

| Property | Required value |
| --- | --- |
| Version | Original Japanese 1.00d |
| Size | `840,704` bytes |
| SHA-256 | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| PE image base | `0x00400000` |
| Entry point | `0x004A619E` |

Localized or patched executables are different binaries and are intentionally
out of scope. The executable and game data are copyrighted assets and are not
included.

```bash
python3 scripts/verify-target.py
```

## Build

Initialize the third-party submodules, then create the upstream Visual Studio
.NET 2002/DirectX 8 environment. On Linux or macOS:

```bash
git submodule update --init --recursive
./scripts/create_th08_prefix
python3 ./scripts/build.py
```

The prefix helper uses Wine by default; set `WINE` before invoking it when a
different compatible runner is required. On Windows, use the upstream setup
script directly:

```text
python scripts/create_devenv.py scripts/dls scripts/prefix
python scripts/build.py
```

See [Build and exact matching](docs/BUILD_MATCHING.md) for dependency,
build-mode, reccmp, and objdiff details.

### Playable modern ports

An independent CMake target compiles the production-authored sources for
modern hosts. It does not replace or make an exactness claim about the VC7
build.

| Platform | Status | Delivery |
| --- | --- | --- |
| Linux i386 | **Done** | Local one-command build/run and CI portable archive |
| Windows x86 | **In progress** | Native startup and redistributable packaging are not complete |
| macOS | **In progress** | Native backend and packaging remain to be implemented |

For build dependencies, runtime asset expectations, `--data-dir`, and the
remaining platform sequence, see
[Playable reconstruction ports](docs/PORTING.md).

For a source checkout on Debian or Ubuntu, the Linux quick start installs
missing i386 dependencies, builds, and runs using only the original data
directory:

```bash
scripts/setup-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The [Portable Linux build workflow](.github/workflows/portable-linux.yml)
also publishes `th08-modern-linux-i386.tar.gz` as a downloadable Actions
artifact. Extract it and pass only the original game-data directory:

```bash
./run-th08.sh "/path/to/the/original/TH08 directory"
```

Neither path embeds the original executable or DAT archives.

The native i386 build has been exercised under both WSLg and a Kali Linux
x86-64 virtual machine. The Kali recording below was made in a low-memory VM
without 3D acceleration; its slow first 45 seconds are shown at 8x speed, while
the remainder plays at the recorded speed.

<p align="center">
  <img
    src="resources/kali-linux-port.gif"
    width="800"
    alt="TH08 native Linux reconstruction starting and running on Kali Linux">
</p>

Only `th08.dat` and `thbgm.dat` are runtime data requirements. The Linux port
does not open or execute the original `th08.exe`. A clean two-DAT directory
previously exposed a Linux compatibility bug during the first score-backup
rotation: Win32 rejects an invalid search handle harmlessly, while the Linux
backend tried to delete it and crashed. The corrected backend now creates the
backup and continues into the title assets with an initially empty `backup/`
directory. On a software-rendered VM, a fresh configuration's fullscreen
FPS/vsync calibration can still be slow; reusing an existing `th08.cfg` is an
optional startup convenience, not a data requirement.

The portable Linux window uses the project-owned
[`resources/modern-icon.png`](resources/modern-icon.png), derived from the
Touhou Lab artwork supplied for this reconstruction. It is not an icon
extracted from the original executable.

#### Known Linux issue

- During the Stage 4-to-5 transition, a dynamic text texture can still tile
  across the outer frame and HUD (most visibly as repeated `Yakumo Yukari`
  text). This is a known renderer/texture-state bug in the Linux port, not a
  damaged DAT archive; gameplay testing can continue past it.

<p align="center">
  <img
    src="resources/linux-stage5-texture-tiling.png"
    width="640"
    alt="Known Linux Stage 5 dynamic text texture tiling bug">
</p>

## Analysis status

IDA MCP follows whichever database is active in the GUI; it has no reliable
program selector. Use it for TH08 only after the active database passes the
attestation in [IDA and analysis safety](docs/IDA_MCP.md). Otherwise use
target-side `objdump`/`llvm-objdump`, the verified disposable Ghidra import, and
the target-pinned repository tools.

To see the live authored and library inventory instead of relying on prose:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
```

## Project map

- [Linux download, installation, and play guide](docs/PLAY_LINUX.md)
- [Native Windows user guide and status](docs/PLAY_WINDOWS.md)
- [Native macOS user guide and status](docs/PLAY_MACOS.md)
- [Architecture and binary inventory](docs/ARCHITECTURE.md)
- [Reverse-engineering workflow](docs/RE_WORKFLOW.md)
- [IDA and analysis safety](docs/IDA_MCP.md)
- [Build and exact matching](docs/BUILD_MATCHING.md)
- [Playable reconstruction ports](docs/PORTING.md)
- [Native Linux playable reconstruction](docs/LINUX_PORTING.md)
- [Tool selection and command recipes](docs/TOOLS.md)
- [Reusable knowledge map and contribution policy](docs/KNOWLEDGE_BASE.md)
- [Current handoff and next milestones](docs/RE_HANDOFF.md)
- [Generated reconstruction progress](docs/PROGRESS.md)
- [Agent operating rules](AGENTS.md)

## Credits and provenance

This continuation exists because of the reconstruction and tooling work by
the contributors to [GensokyoClub/th08](https://github.com/GensokyoClub/th08).
Their commits retain their original author/committer metadata in this
repository. The upstream project also credits @EstexNT for porting its
`var_order` pragma to MSVC7.

The [N0zoM1z0/th07 reconstruction](https://github.com/N0zoM1z0/th07) supplies
this repository's workflow, structure, target gates, matching, and
documentation model. [GensokyoClub/th06](https://github.com/GensokyoClub/th06)
is adjacent-engine corroboration only; neither reference overrides TH08 target
evidence.

## License

Repository code and documentation are provided under the included MIT License.
This does not grant rights to the original game, executable, or game data.
