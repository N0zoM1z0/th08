# Architecture and binary inventory

This reference records target identity, repository structure, and validation
order. For runtime relationships and terminology, start with the
[project and engine guide](PROJECT_GUIDE.md). The
[documentation index](README.md) provides human and agent reading routes.

## Exact target

The target is the original Japanese TH08 v1.00d executable, identified by the
SHA-256 in [reccmp-project.yml](../reccmp-project.yml):

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

## Build and runtime order

The pinned-VC7 Windows i386 production image is the first whole-program runtime
oracle for the reconstructed source. It must compile every production
translation unit, link a real PE32 executable without unresolved-symbol forcing,
and survive native Windows playtesting before a modern compiler/backend is used
as a replacement runtime.

This order exposes defects that platform adaptations can hide. A port may
introduce compatibility startup code, duplicate/fixed-layout storage bridges,
different static initialization, or compiler-specific ABI adapters. Those can
hide a missing source owner, translation-unit boundary, link dependency, or
lifetime defect. Portability builds provide a second independent check after
native validation. See the [runtime procedure](WINDOWS_I386_RUNTIME.md),
[owner audit](OWNER_AUDIT.md), and [runtime issues](RUNTIME_ISSUES.md).

The first native Windows i386 prerequisite pass completed on 2026-09-10. It
found and repaired target-data initialization, aggregate ownership, final-link
callee identity, callback-table ownership, and runtime lifetime failures that
had survived function-level comparison and modern-port testing. This completion
unblocks port stabilization. Redistributable packaging is a separate milestone.
Re-run the documented native gate after any shared
owner, layout, translation-unit, PCH, compiler-profile, or production link-graph
change that could invalidate the checkpoint.

## Provenance

The repository continues
[GensokyoClub/th08](https://github.com/GensokyoClub/th08). The upstream source,
configuration, build tools, and contributor commits form the initial baseline.
Keep the imported history intact and record continuation changes as new
commits by their actual authors.

## Runtime subsystems

The current `src/` layout follows the upstream engine responsibilities:

- `Supervisor`, `main`, window/input, callback chains, timing, and globals;
- ANM loading/VM/rendering, ASCII text, GUI, and screen effects;
- background, enemy, bullet, item, player, spell-card, and game managers;
- title, music room, replay, score, ending, and result screens;
- sound/MIDI and the `zwave` implementation;
- PBG archive, file, memory, and LZSS support under `src/pbg/`.

Read the [frame walkthrough](PROJECT_GUIDE.md#a-frame-through-the-engine) for
how these responsibilities interact, then use the
[semantic index](SEMANTIC_INDEX.md#subsystems) for declarations and evidence.

These filenames are useful source-ownership hypotheses. Only target evidence
and linked-object comparison can establish original translation-unit
boundaries.

## Repository structure

- `src/`: reconstructed C++ and ABI-facing headers.
- `config/mapping.csv`: address/type mapping used by upstream analysis tools.
- `config/reccmp-*.csv`: function, global, float, string, and comparison maps.
  `reccmp-relocations.csv` is a separate allowlist for attested IAT slots,
  import thunks, and data-symbol relocations.
- `config/implemented.csv`: source-selection ledger.
- `config/match-units.toml` and `config/matches.csv`: strict function-level
  comparison definitions and accepted exact results.
- `config/library-provenance.toml`, `config/library-match-units.toml`, and
  `config/library-matches.csv`: SHA-pinned target-linked archive provenance,
  library-specific comparison definitions, and accepted library exact results.
  Library progress is tracked separately from authored functions.
- `config/mapping-overlaps.csv`: explicit target-proven nested-funclet overlap
  exceptions; stale or unclassified overlap state is rejected/reported by the
  tracking validator.
- `config/claims.csv`: retired claim schema, kept header-only for compatibility
  with earlier history.
- `scripts/`: environment acquisition, Ninja generation, target verification,
  focused matching, typed target facts, and progress helpers. Reusable read-only
  investigations live under `scripts/analysis/`; completed phase-specific
  reproducers live under `scripts/analysis/historical/`.
- `reccmp-project.yml`: exact target hash and reccmp data sources.
- `objdiff.json`: reconstructed/original COFF unit mapping.
- `3rdparty/`: the pinned Detours submodule used only by the optional DLL build.
- `resources/`: non-source inputs and progress artwork; the private target is
  expected here but must not be committed.
- `build/`: generated executables, objects, maps, and reports.
- `.analysis/`: ignored, disposable scratch evidence for the active investigation.
  Promote conclusions to tracked documents or ledgers before handoff.

`scripts/progress.py` derives source-presence and strict exact-match views in
`docs/PROGRESS.md`. Its SVG uses authored exact bytes for the progress bar and
adds explicitly separate playable-platform delivery cards. Source presence
comes from `config/implemented.csv`; exact coverage counts only accepted rows
in `config/matches.csv`. CI checks these generated files but cannot replay
private target comparisons.

The `library` rows in `config/reccmp-functions.csv` describe code linked into
the original TH08 image, principally VC7 CRT/runtime and D3DX bodies.
`3rdparty/Detours` instead supports this repository's optional DLL build.

## Evidence relationship to adjacent games

The [N0zoM1z0/th07 reconstruction](https://github.com/N0zoM1z0/th07) supplies
this repository's workflow and structure. TH07 and TH06 share engine concepts,
compiler idioms, and many subsystem names with TH08, so their source can
quickly suggest candidates for migration, but TH08
changed gameplay systems, layouts, control flow, globals, and translation-unit
boundaries. Treat every migrated declaration or implementation as an
unverified hypothesis until TH08 1.00d disassembly and comparison confirm it.
