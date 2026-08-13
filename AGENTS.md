# TH08 reconstruction agent rules

This repository reconstructs one exact binary: the original Japanese TH08
version 1.00d executable identified below and in `reccmp-project.yml`. Do not
analyze or substitute a localized, patched, trial, or earlier executable.

## Before changing reconstruction state

1. Read `docs/ARCHITECTURE.md`, `docs/RE_WORKFLOW.md`, and the relevant source.
2. Verify `resources/th08.exe`: SHA-256
   `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`
   and size `840704` bytes.
3. Confirm the address and symbol in `config/mapping.csv` and the appropriate
   `config/reccmp-*.csv` files before editing.
4. Inspect target instructions with a target-safe backend. Reconcile function
   boundaries against disassembly; Ghidra/IDA function extents are evidence,
   not accepted compiler boundaries.
5. Build and compare the smallest affected source/object before broad edits.

## Evidence and status rules

- Keep target observations, upstream reconstruction claims, adjacent-version
  inferences, and unknowns distinct.
- [GensokyoClub/th08](https://github.com/GensokyoClub/th08) is the imported
  reconstruction baseline. Its complete Git history and original contributor
  attribution must remain intact.
- The [N0zoM1z0/th07 reconstruction](https://github.com/N0zoM1z0/th07)
  supplies this repository's workflow and structure. TH07 source and TH06 are
  supporting evidence only: confirm names, ABI, field offsets, side effects,
  and boundaries against TH08 1.00d.
- Never mechanically paste decompiler output as source.
- `config/implemented.csv` selects upstream implementations; it is not proof of
  an exact match. Likewise, an entry in `config/mapping.csv` or
  `config/reccmp-functions.csv` is a mapping, not a matching claim.
- Only a reproducible reccmp/objdiff result against the exact target may support
  an exact-match claim. Do not invent or manually transcribe progress numbers.

## Analysis database safety

IDA MCP is attached to whichever file is open in the GUI and has no reliable
program selector. The currently attached IDA database is TH07, not TH08. Treat
all TH08 IDA queries as blocked until the GUI is switched to the exact TH08
1.00d target and all metadata gates in `docs/IDA_MCP.md` pass.

Until then, use read-only `objdump`/`llvm-objdump`, a correctly imported local
Ghidra project, and the upstream mappings. Do not write TH08 names or types into
the TH07 database. Never patch target bytes.

## ABI and implementation

- Preserve the Visual C++ .NET 2002 x86 ABI: calling conventions, field widths,
  class layout, vtable order, exception behavior, static initialization, and
  translation-unit boundaries.
- Prefer source expressions that plausibly produced the target. Do not use
  copied machine-code arrays, native assembly, inline assembly, naked-assembly
  byte dumps, fake returns, empty behavior, ABI lies, or arbitrary padding to
  force a comparison. Exact matching must come from natural VC7 emission of
  evidence-backed C/C++ behavior.
- Keep generated files under `build/` and private analysis under `.analysis/`.
- Do not commit original executables, game archives, IDA/Ghidra databases,
  downloaded toolchains, generated reports, credentials, or private keys.

## Parallel reconstruction

- Give workers non-overlapping modules or address ranges.
- The coordinator owns shared ABI/type decisions, mapping CSV changes, progress
  assets, commits, and pushes.
- Workers report target range, evidence, files changed, exact build/comparison
  commands, result, and remaining uncertainty.
- Shared headers, class layouts, compiler flags, object partitioning, and global
  mappings require coordinator review before integration.

## Handoff

Run the relevant normal/object build and comparison, followed by
`git diff --check`. Report the target addresses, evidence class, exact result,
and any unverified assumptions. If the exact executable or analysis backend is
unavailable, say so and stop short of a matching claim.
