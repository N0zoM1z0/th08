# Architecture and binary inventory

## Exact target

The only accepted target is the original Japanese TH08 v1.00d executable. Its
identity is recorded by the SHA-256 in `reccmp-project.yml`; every analysis or
comparison must resolve to these facts:

| Property | Value |
| --- | --- |
| Architecture | 32-bit x86 PE GUI executable |
| File size | `840,704` bytes |
| SHA-256 | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| Image base | `0x00400000` |
| Entry point | `0x004A619E` |
| `.text` virtual range | `0x00402000`–`0x004B3B77` (inclusive) |
| Toolchain family | Visual C++ .NET 2002 (VC7) |

Function starts and extents in Ghidra, IDA, and CSV exports are analysis
artifacts. Tail chunks, alignment, shared code, and missed instructions must be
reconciled against the exact target before they become comparison boundaries.

## Provenance

The repository is a history-preserving continuation of
[GensokyoClub/th08](https://github.com/GensokyoClub/th08). The upstream source,
configuration, build tools, and contributor commits form the initial baseline.
Continuation changes should be additive commits by their actual authors; do
not squash or re-author the imported history.

## Runtime subsystems

The current `src/` layout follows the upstream engine responsibilities:

- `Supervisor`, `main`, window/input, callback chains, timing, and globals;
- ANM loading/VM/rendering, ASCII text, GUI, and screen effects;
- background, enemy, bullet, item, player, spell-card, and game managers;
- title, music room, replay, score, ending, and result screens;
- sound/MIDI and the `zwave` implementation;
- PBG archive, file, memory, and LZSS support under `src/pbg/`.

These filenames are useful source-ownership hypotheses. Only target evidence
and linked-object comparison can establish original translation-unit
boundaries.

## Repository structure

- `src/`: reconstructed C++ and ABI-facing headers.
- `config/mapping.csv`: address/type mapping used by upstream analysis tools.
- `config/reccmp-*.csv`: function, global, float, string, and comparison maps.
- `config/implemented.csv`: symbols with authored source; inclusion is not
  itself an exact-match result.
- `config/match-units.toml` and `config/matches.csv`: strict function-level
  comparison definitions and accepted exact results.
- `config/claims.csv`: coordinator-owned parallel work claims.
- `scripts/`: environment acquisition, Ninja generation, target verification,
  focused matching, typed target facts, and progress helpers.
- `reccmp-project.yml`: exact target hash and reccmp data sources.
- `objdiff.json`: reconstructed/original COFF unit mapping.
- `3rdparty/`: pinned Detours and munit submodules.
- `resources/`: non-source inputs and progress artwork; the private target is
  expected here but must not be committed.
- `build/`: generated executables, objects, maps, and reports.

`scripts/progress.py` derives both source-presence and strict exact-match views
in `docs/PROGRESS.md` and its SVG. Source presence comes from
`config/implemented.csv`; exact coverage counts only accepted rows in
`config/matches.csv`. CI checks these generated files but cannot replay private
target comparisons.

## Evidence relationship to adjacent games

TH06 and TH07 share engine concepts, compiler idioms, and many subsystem names
with TH08. They can quickly suggest candidates for source migration, but TH08
changed gameplay systems, layouts, control flow, globals, and translation-unit
boundaries. Treat every migrated declaration or implementation as an
unverified hypothesis until TH08 1.00d disassembly and comparison confirm it.
