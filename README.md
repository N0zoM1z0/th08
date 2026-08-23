# 東方永夜抄 ～ Imperishable Night

<p align="center">
  <img
    src="resources/title-screen.png"
    width="640"
    alt="Original Japanese TH08 1.00d title screen">
</p>

<p align="center">
  <img src="resources/progress.svg" alt="TH08 exact source reconstruction progress">
</p>

This project aims to reconstruct the source code of the original Japanese
`東方永夜抄 ～ Imperishable Night` version 1.00d executable, with reproducible
binary comparison as the acceptance criterion.

The repository continues the work of
[GensokyoClub/th08](https://github.com/GensokyoClub/th08). Its complete Git
history was imported rather than squashed, preserving the authorship and
contribution record of the original project. New infrastructure and
reconstruction work build on that baseline.

The project remains active reverse-engineering work. Existing source, symbol
mappings, or generated progress artwork must not be interpreted as a new exact
matching percentage without a reproducible report against the target binary.
Current source-presence inventory is generated in
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

### Playable modern Windows build

An independent CMake target now compiles the production-authored sources as a
32-bit modern Windows executable. It does not replace or make an exactness
claim about the VC7 build. For the current MinGW/MSVC requirements, runtime
asset expectations, and the Windows-to-Linux/macOS port sequence, see
[Playable reconstruction ports](docs/PORTING.md).

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

- [Architecture and binary inventory](docs/ARCHITECTURE.md)
- [Reverse-engineering workflow](docs/RE_WORKFLOW.md)
- [IDA and analysis safety](docs/IDA_MCP.md)
- [Build and exact matching](docs/BUILD_MATCHING.md)
- [Playable reconstruction ports](docs/PORTING.md)
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
