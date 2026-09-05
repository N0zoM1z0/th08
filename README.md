# 東方永夜抄 ～ Imperishable Night

<p align="center">
  <img
    src="resources/title-screen.png"
    width="640"
    alt="Original Japanese TH08 1.00d title screen">
</p>

<p align="center">
  <img src="resources/progress.svg" alt="TH08 exact-source and playable-platform progress">
</p>

> [!IMPORTANT]
> 🌙 The authored reconstruction is complete, and the Linux port is playable.
> Download [TH08 Reconstruction v0.2.0 — Native Linux 64-bit](https://github.com/N0zoM1z0/th08/releases/latest);
> active ELF64 source lives on
> [`port/portable-64bit`](https://github.com/N0zoM1z0/th08/tree/port/portable-64bit).
> Windows and macOS ports remain in progress.

## TL;DR

| I want to... | Start here |
| --- | --- |
| Check reconstruction progress | [Repository status](#repository-status) |
| Understand how AI agents work on the project | [AI agent workflow](#ai-agent-workflow) |
| Read our accuracy and readability philosophy | [What we mean by semantic reconstruction](#what-we-mean-by-semantic-reconstruction) |
| Contribute | [Contributing](#contributing) |
| Play or build a port | [Platform guides](#platform-guides) |
| Reproduce the exact comparison | [Exact reconstruction](#exact-reconstruction) |
| Browse the technical documentation | [Project map](#project-map) |
| Review upstream history and attribution | [Credits and provenance](#credits-and-provenance) |

## Repository status

This repository reconstructs the original Japanese
`東方永夜抄 ～ Imperishable Night` version 1.00d executable. Every one of the
1,107 authored functions now has source. Strict comparison currently accepts
1,106 of them, covering 459,396 of 459,757 authored bytes.

| Area | Status | Current position |
| --- | --- | --- |
| Authored source | **Complete** | 1,107 / 1,107 functions are present in source |
| Strict authored comparison | **99.92% by bytes** | 1,106 / 1,107 functions are accepted as exact |
| Whole executable | **In progress** | PE layout, linked runtime/library code, and one authored near match remain |
| Web | **Playable** | Public WebAssembly/WebGL 2 build |
| Linux | **Playable** | Native i386; x86_64/AArch64 work on `port/portable-64bit` |
| Windows | **In progress** | Native startup stabilization and redistributable packaging are underway |
| macOS | **In progress** | Native backend and packaging are planned |

Exact reconstruction and playable ports are separate milestones. The progress
bar counts authored bytes accepted by strict comparison; the platform cards
show where the reconstructed source is currently playable.

The remaining exact-reconstruction work is the last authored near match,
whole-image layout, and the compiler/runtime and D3DX code linked into the
original game. The repository ledgers are the canonical source for live
counts.

## AI agent workflow

All new engineering in this continuation—reverse engineering, source matching,
semantic recovery, tooling, documentation, and porting—is carried out by AI
coding agents. The human maintainer sets the direction, decides what is
published or merged, and supplies the legally obtained target and game data.
The imported GensokyoClub history retains its original authorship and
contribution record.

Our premise in 2026 is that frontier coding agents can sustain native-code
reconstruction when they work with durable project memory, bounded tasks,
strong tools, and fast empirical feedback. Each agent contribution begins as a
testable hypothesis. The verified target and toolchain provide the verdict.

The most important design rule is simple: **the repository is the project's
shared memory.** Personal memory and chat sessions are temporary workspaces.
Durable knowledge, experience, and lessons belong in forms that the next
contributor can find, review, rerun, and improve: source, ledgers, focused
evidence notes, scripts, tests, guards, and reusable skills.

```mermaid
flowchart LR
    H["Human<br/>scope & release"]:::human --> A["Fresh AI agent<br/>cold start"]:::agent
    K[("Repository memory<br/>rules · ledgers · handoff<br/>skills · evidence · guards")]:::memory --> A
    A --> E["Bounded task<br/>+ TH08 evidence"]:::evidence
    E --> I["Natural C++<br/>ABI / VC7 shape intact"]:::work
    I --> F{"Focused VC7<br/>exact?"}:::gate
    F -->|Mismatch| D["Diagnose<br/>and refine"]:::reject --> I
    F -->|Exact| O["Required aggregate<br/>+ portable Oracles"]:::oracle
    O --> G{"All gates<br/>pass?"}:::gate
    G -->|Refine| D
    G -->|Pass| R["Promote evidence, unknowns,<br/>guards & lessons into repo"]:::memory
    R --> Q["Commit & push<br/>auditable checkpoint"]:::done
    R -.->|reusable knowledge| K

    classDef human fill:#fff1c2,stroke:#b7791f,color:#3b2f0b,stroke-width:2px;
    classDef agent fill:#ede9fe,stroke:#7c3aed,color:#2e1065,stroke-width:2px;
    classDef memory fill:#dbeafe,stroke:#2563eb,color:#172554,stroke-width:2px;
    classDef evidence fill:#cffafe,stroke:#0891b2,color:#083344,stroke-width:2px;
    classDef work fill:#fef3c7,stroke:#d97706,color:#451a03,stroke-width:2px;
    classDef oracle fill:#dcfce7,stroke:#16a34a,color:#052e16,stroke-width:2px;
    classDef gate fill:#f3f4f6,stroke:#4b5563,color:#111827,stroke-width:2px;
    classDef reject fill:#fee2e2,stroke:#dc2626,color:#450a0a,stroke-width:2px;
    classDef done fill:#ccfbf1,stroke:#0f766e,color:#042f2e,stroke-width:2px;
```

The “Oracle” in that diagram is a stack of reproducible checks. We pin the
exact Japanese 1.00d executable by size and SHA-256, compare the smallest
affected VC7 function or object, and verify relocations alongside instruction
bytes. A shared change then triggers a clean, single-job rebuild of every
configured comparison object and a replay of the whole accepted ledger.
Normal VC7 linking, modern Linux builds, fixed-layout checks, available runtime
tests, and repository CI cover different classes of regression. “Exact” is a
recorded, comparator-backed repository state.

Repository memory is part of the working architecture. Each durable result has
a canonical home:

- [AGENTS.md](AGENTS.md) holds the target, ABI, safety, and acceptance rules.
- The CSV/TOML ledgers and status scripts hold live mappings and accepted
  results; prose provides context for these canonical records.
- [The current handoff](docs/RE_HANDOFF.md) says what is complete, what is
  blocked, and what should happen next.
- [Task-specific skills](.agents/skills/) and
  [the knowledge map](docs/KNOWLEDGE_BASE.md) preserve tool recipes, VC7 source
  patterns, evidence boundaries, and lessons from failed experiments.
- Focused evidence documents explain why a name, layout, function boundary, or
  compiler shape was accepted, while CI guards completed surfaces against
  regression.

That structure makes agents interchangeable while keeping writes controlled.
A fresh agent can verify the target, read the tracked state, run the live
reports, and resume from a clean checkout with the repository as its complete
starting context. Reconstruction writes and Wine/VC7 matching remain
single-writer and serial, keeping edits, object freshness, and shared toolchain
state deterministic while making handoffs inexpensive.

The architecture treats every model inference as falsifiable: tasks stay
small, failed experiments feed the knowledge base, uncertainty remains
explicit, and each checkpoint carries the evidence needed to reproduce it.
That is what AI reconstruction means in this project.

## What we mean by semantic reconstruction

Matching the executable establishes the first requirement. Semantic
reconstruction then recovers the game concepts hidden behind object offsets,
anonymous fields, bare masks, and numbered interpreter cases and puts those
meanings back into the C++.

Accuracy comes first. We add a type or name only when TH08 itself supports it
through reads, writes, callers, or state transitions. TH06, TH07, and the
inherited upstream names are useful corroboration; the Japanese TH08 1.00d
target has final authority. Uncertain meanings remain explicitly documented as
unknowns.

Equivalent-looking C++ expressions can produce different VC7 code. Under
`/Ob0`, even a small helper or a reordered `switch` can change the output. The
accepted formulation preserves the target-shaped expression or case order
whenever exact emission depends on it.

Every semantic batch is checked in both directions. The VC7 comparison makes
sure accepted target bytes stay exact; the modern builds make sure the same
source still works as portable C++. Shared changes are rebuilt on Linux and
checked against the fixed-layout verifier, with relevant runtime tests used
when they are available.

To measure the semantic pass, we audited the repository and compared the same
kinds of interpreter code with
[GensokyoClub/th06](https://github.com/GensokyoClub/th06), our adjacent-engine
reference:

| Comparable protocol surface | This TH08 reconstruction | GensokyoClub/th06 reference |
| --- | ---: | ---: |
| ECL operand selectors | **101 / 101 named** | 25 named `EclVarId` values |
| Stage/background stream opcodes | **35 / 35 named** | 6 named `StageOpcode` values |
| ECL timeline opcodes | **17 / 17 named** | 11 numeric `case` labels remain |

On these measured surfaces, TH08 now **outperforms the TH06 reference in
readability coverage**. TH08 also has names for all 184 primary
ECL opcodes, the complete interpolation and camera-mode domains, replay event
bits, stable sound and resource protocols, and the small UI/gameplay state
machines found during the audit. CI keeps these finished surfaces on named
dispatch.

The comparison gives us a concrete readability benchmark. The audit also
classifies the remaining literals by evidence. Its 74 numeric `case` labels are
option-array indices, damage or life quantities, or per-file animation IDs
whose visual meaning remains ambiguous. Accuracy first means maximizing
readability within the available evidence.

The final pass cold-built all 75 configured comparison objects and reproduced
all **1,106 / 1,106 accepted exact functions**. The normal VC7 image linked,
and the full Linux i386 build and fixed-layout check passed. The [semantic
reconstruction record](docs/SEMANTIC_RECONSTRUCTION.md) has the full evidence
trail, the exact-safe source-shape rules, the unknowns we kept, and the results
for each batch.

## Contributing

Contributions are welcome. We are especially interested in:

- evidence-backed exact reconstruction and whole-image layout work;
- reliable native Windows startup and replacement of the D3DX debug dependency
  with a redistributable component;
- a native macOS window, input, audio, renderer, and packaging backend;
- Linux renderer fixes, MIDI support, and testing on additional hardware;
- browser correctness, performance, and compatibility work in
  [N0zoM1z0/th08-web](https://github.com/N0zoM1z0/th08-web).

Before changing reconstruction state, read [AGENTS.md](AGENTS.md),
[the reverse-engineering workflow](docs/RE_WORKFLOW.md), and
[the current handoff](docs/RE_HANDOFF.md). Exact-match contributions must be
supported by reproducible comparison against the specified target. Keep the
original executable, DAT archives, extracted retail assets, private analysis
databases, and credentials outside the repository.

## Platform guides

The ports compile the reconstructed game code for modern systems. Players
provide the original game data from a legally obtained copy of TH08.

### Web

**Status: Playable**

<p align="center">
  <a href="https://th08-web.pages.dev/">
    <img
      src="https://raw.githubusercontent.com/N0zoM1z0/th08-web/main/resources/th08-web-social-preview.jpg"
      width="800"
      alt="TH08 Web source-built browser port and Imperishable Night title screen">
  </a>
</p>

[Play in the browser](https://th08-web.pages.dev/) ·
[source and documentation](https://github.com/N0zoM1z0/th08-web) ·
[latest release](https://github.com/N0zoM1z0/th08-web/releases/latest) ·
[engineering the Web port](https://github.com/N0zoM1z0/th08-web/blob/main/docs/WEB_PORTING.md)

TH08 Web compiles the reconstructed C++ directly to WebAssembly and runs it in
a browser worker. It uses WebGL 2, Web Audio, browser-local files, and
IndexedDB-backed saves.

Select `th08.dat` and `thbgm.dat` from a legal TH08 installation in the
launcher. `th08.dat` remains in volatile session memory; `thbgm.dat` is
range-read from its browser `File` object. Both files stay on the player's
machine and outside persistent browser storage. Chrome has the best observed
frame pacing; Firefox is also supported and is usually slower.

### Linux

**Status: Playable**

- [Download the latest native Linux release](https://github.com/N0zoM1z0/th08/releases/latest)
- [Download, installation, and player guide](docs/PLAY_LINUX.md)
- [Native Linux porting architecture and validation](docs/LINUX_PORTING.md)
- [Native 64-bit branch, build, and validation](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_64BIT.md)
- [Portable Linux build workflow](.github/workflows/portable-linux.yml)

On Debian or Ubuntu, build and run against the original game-data directory:

```bash
scripts/setup-modern-linux.sh "/path/to/the/original/TH08 directory"
```

For later runs, use the incremental launcher:

```bash
scripts/play-modern-linux.sh "/path/to/the/original/TH08 directory"
```

The latest release includes x86_64, i386, and experimental AArch64 portable
packages. Extract the package for your architecture and pass the original data
directory:

```bash
./run-th08.sh "/path/to/the/original/TH08 directory"
```

The native i386 ELF has been tested under WSLg and in a Kali Linux x86-64
virtual machine. It reads `th08.dat` and `thbgm.dat` directly and runs
independently of the original `th08.exe`. Settings, scores, replays, and
backups stay in the selected data directory.

The native-layout x86_64 PIE is the recommended Linux package. Its source is on
[`port/portable-64bit`](https://github.com/N0zoM1z0/th08/tree/port/portable-64bit).
It has been played through a Lunatic Stage 1–6A route, including the ending,
results, and return to title, plus Stage 4A/6B Practice runs under WSLg. The
AArch64 build and loader have been verified, but it still needs a gameplay run
on real hardware.

<p align="center">
  <img
    src="https://raw.githubusercontent.com/N0zoM1z0/th08/port/portable-64bit/resources/portable64-kaguya-lunatic.png"
    width="800"
    alt="Native x86_64 TH08 running Kaguya's Lunatic Princess spell under WSLg">
</p>

> Maintainer bias, openly declared: Kaguya is my favorite, and
> **竹取飛翔 ～ Lunatic Princess** is my favorite track. XD

<p align="center">
  <img
    src="resources/kali-linux-port.gif"
    width="800"
    alt="TH08 native Linux reconstruction starting and running on Kali Linux">
</p>

The portable window uses the project-owned
[`resources/modern-icon.png`](resources/modern-icon.png). On software-rendered
systems, a fresh configuration's fullscreen FPS/vsync calibration can be slow;
reusing an existing `th08.cfg` is optional.

#### Earlier Linux renderer regression

An early Linux build sometimes tiled a dynamic text texture across the outer
frame and HUD during the Stage 4-to-5 transition, most visibly as repeated
`Yakumo Yukari` text. It also had missing enemy/boss art and incomplete effects.
The native-layout and renderer fixes produced clean final x86_64 full-route and
Practice runs. We keep the screenshot as a useful regression sample; reports
from additional drivers and desktops are welcome.

<p align="center">
  <img
    src="resources/linux-stage5-texture-tiling.png"
    width="640"
    alt="Historical Linux Stage 5 dynamic text texture tiling regression">
</p>

### Windows

**Status: In progress**

See the [native Windows guide](docs/PLAY_WINDOWS.md) for the current build and
release requirements. The source produces a 32-bit MinGW bring-up executable.
Current work focuses on reliable native startup and replacing the DirectX SDK
debug DLL with redistributable components before publishing a supported
Windows release.

The goal is a self-contained native build that accepts any legal TH08 data
directory and ships with redistributable components.

### macOS

**Status: In progress**

See the [native macOS guide](docs/PLAY_MACOS.md) for the current plan. Native
window, input, audio, rendering, packaging, and real-hardware validation are
the remaining milestones.

## Exact reconstruction

The exact target is one binary: the original Japanese TH08 version 1.00d. A
localized, patched, trial, or earlier executable is a different target.

This repository is a history-preserving continuation of
[GensokyoClub/th08](https://github.com/GensokyoClub/th08). Its complete Git
history was imported rather than squashed, preserving the original authorship
and contribution record.

### Target executable

Supply your own original executable as `resources/th08.exe`:

| Property | Required value |
| --- | --- |
| Version | Original Japanese 1.00d |
| Size | `840,704` bytes |
| SHA-256 | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| PE image base | `0x00400000` |
| Entry point | `0x004A619E` |

The executable and game data remain copyrighted assets supplied privately by
each contributor. Verify the private target before analysis or comparison:

```bash
python3 scripts/verify-target.py
```

### Build and compare

Initialize the third-party submodules, then create the Visual Studio .NET
2002/DirectX 8 environment. On Linux or macOS:

```bash
git submodule update --init --recursive
./scripts/create_th08_prefix
python3 ./scripts/build.py
```

The prefix helper uses Wine by default. Set `WINE` before invoking it when a
different compatible runner is required. On Windows, use the setup script
directly:

```text
python scripts/create_devenv.py scripts/dls scripts/prefix
python scripts/build.py
```

See [Build and exact matching](docs/BUILD_MATCHING.md) for dependencies,
build modes, reccmp, objdiff, and acceptance rules.

### Analysis and live progress

IDA MCP follows whichever database is active in the GUI, so TH08 analysis
begins with [the documented database attestation](docs/IDA_MCP.md).
Target-safe headless tools and the repository's target-pinned analysis scripts
cover other analysis sessions.

Read current figures directly from the ledgers:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
```

Exact-match status comes from an accepted, reproducible comparison against the
verified target. Source mappings, generated progress artwork, successful
builds, and `config/implemented.csv` serve their own tracking and build roles.
Generated source-presence and strict-match figures are recorded in
[docs/PROGRESS.md](docs/PROGRESS.md).

## Project map

- [TH08 Web browser port and engineering documentation](https://github.com/N0zoM1z0/th08-web)
- [Linux download, installation, and play guide](docs/PLAY_LINUX.md)
- [Native Windows user guide and status](docs/PLAY_WINDOWS.md)
- [Native macOS user guide and status](docs/PLAY_MACOS.md)
- [Architecture and binary inventory](docs/ARCHITECTURE.md)
- [Reverse-engineering workflow](docs/RE_WORKFLOW.md)
- [Semantic reconstruction and two-oracle acceptance](docs/SEMANTIC_RECONSTRUCTION.md)
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

This continuation exists because of the reconstruction and tooling work by the
contributors to [GensokyoClub/th08](https://github.com/GensokyoClub/th08).
Their commits retain their original author/committer metadata. The upstream
project also credits @EstexNT for porting its `var_order` pragma to MSVC7.

The [N0zoM1z0/th07 reconstruction](https://github.com/N0zoM1z0/th07) supplies
this repository's workflow, structure, target gates, matching, and
documentation model. [GensokyoClub/th06](https://github.com/GensokyoClub/th06)
provides adjacent-engine corroboration, while TH08 target evidence retains
final authority.

## License

Repository code and documentation are provided under the included MIT License.
Rights to the original game, executable, and game data remain with their
respective owners.
