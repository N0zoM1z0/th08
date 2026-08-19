# Reverse-engineering workflow

## Sources of truth

Use evidence in this order:

1. The exact TH08 1.00d executable identified in `docs/ARCHITECTURE.md`.
2. Target disassembly, bytes, imports, relocations, xrefs, and runtime behavior.
3. Exact compiler/reccmp/objdiff output produced in this repository.
4. The inherited GensokyoClub TH08 source and mapping files.
5. TH06, TH07, decompiler output, and other adjacent-version references.

Lower-ranked evidence helps form hypotheses but cannot override target bytes.
Keep the target observation and the inference that follows from it separately
recorded in notes and handoffs.

## Bounded reconstruction loop

1. Verify the executable hash and size.
2. Run `scripts/analysis/report-reconstruction-status.py` and select one small
   function, one contiguous address range, or one inventory family.
3. Locate it in `config/mapping.csv` and `config/reccmp-functions.csv`.
4. Inspect target disassembly, callers, callees, strings, globals, imports,
   nearby functions, and exception/control-flow edges.
5. Reconcile the analyzed function extent with actual target instructions. If
   inventory rows split a range that has one target prologue, one compiler return,
   and one matching VC7 COFF auxiliary extent spanning the combined bytes, fix the
   inventory boundary before recording exactness; never accept only the false
   prefix as a function.
6. Use TH06, TH07, or the inherited TH08 source to test names, ABI, layout, and
   behavior; record disagreements rather than silently choosing one version.
7. Implement the smallest coherent change while preserving the VC7 x86 ABI.
8. Build the smallest affected target or object, then run the exact comparator.
9. Update mapping/progress inputs only to the level proven by the report.
10. Run `git diff --check` and hand off commands, results, and uncertainty.

After intentional mapping, implementation, or exact-match ledger changes,
regenerate both progress views with `python3 scripts/progress.py`. Source
presence remains inventory reporting; an exact row may be added only after the
strict function comparator succeeds against the verified target.

Relocation destinations use schema-aware ledgers. Function/global CSVs place
`address` in their second column, while float/string CSVs place it first; read
the header instead of assuming a column number. Use
`config/reccmp-relocations.csv` for hash-attested IAT slots, import thunks, or
other non-inventory relocation destinations. Do not add those addresses to
`mapping.csv` or `reccmp-functions.csv` merely to make a comparison pass.

## Current analysis backends

IDA MCP is usable only for an active TH08 database that passes the session
attestation in `docs/IDA_MCP.md`. The canonical file hash belongs to
`resources/th08.exe`; where an IDB reports a different loader/container hash,
use the documented multi-point mapped-byte comparison before accepting IDA as
semantic evidence. Until that gate passes, use headless tools only:

- `objdump` or `llvm-objdump` for PE headers, sections, imports, and disassembly;
- a headless local Ghidra project imported from the verified `resources/th08.exe`;
- `scripts/typed-re.py` for hash-attested instruction and ABI facts when
  Capstone is installed;
- `config/mapping.csv` and `config/reccmp-*.csv` as inherited upstream leads;
- [N0zoM1z0/th07](https://github.com/N0zoM1z0/th07) source and TH06 source or
  binaries for explicitly labeled cross-version clues.

See `docs/IDA_MCP.md` before changing analysis databases or running inherited
Ghidra export helpers.

## Acceptance language

Use precise terms in reviews and commits:

| Term | Meaning |
| --- | --- |
| `mapped` | an address/name association exists; behavior may be incomplete |
| `implemented` | source is selected by the build; exactness is not implied |
| `compiles` | the relevant build target succeeds |
| `matching` | the accepted comparator proves exact target code/data for the stated scope |
| `blocked` | a concrete missing input, boundary, mapping, or tool is named |

Source resemblance, a successful link, a Ghidra function name, or inclusion in
`config/implemented.csv` is not sufficient for `matching`. Do not publish a
matching percentage unless it is generated from a current reproducible report.

## Adjacent-version acceleration

Apply the workflow and structure from
[N0zoM1z0/th07](https://github.com/N0zoM1z0/th07), compare the inherited TH08
source first, and use TH06 only as adjacent-engine corroboration for matching
subsystem names, constants, ABI shapes, and compiler idioms. Migrate
small units and verify each against TH08. Prefer instruction-level evidence
over semantic resemblance, and re-check all absolute addresses and structure
offsets. Do not bulk-copy a module and mark it reconstructed.

When a large function repeatedly touches anonymous structure offsets, first look
for tiny target helpers that own the same fields. Reconstruct and strictly match
those helpers in isolation, then promote the proven offsets to named fields and
rerun every accepted unit for that class. This dependency-first field recovery
keeps semantic naming evidence separate from guesses made inside a large boss.

Probe objects are proof scaffolding, not production linkage. If a helper that was
first reconstructed in `build/probes/*.obj` becomes a dependency of linked game
code, move its real definition into a production-linked translation unit and
point the strict unit at that production object. Re-run both the helper and its
callers; do not satisfy the linker with an unverified duplicate shim. Compiler-
local relocation labels may be renumbered when shared declarations change, but
any manifest update must preserve the same relocation offset/type/target and the
full canonical byte comparison.

## Current phase selection

Authored source is present for the complete authored inventory and all but two
authored functions have accepted exact units. Those two near matches are
deliberately deferred in `docs/RE_HANDOFF.md`; do not restart blind expression
matrices for them.

The next primary lane is target-linked library/runtime recovery:

1. Reconcile library names and boundaries against target disassembly, beginning
   with the seven library rows that lack `mapping.csv` sizes and the
   imported overlapping ranges reported by `validate-tracking.py`.
2. Identify the exact VC7/CRT/D3DX archive provenance and pin archive hashes
   before writing a scanner. TH07 archives are not substitutes.
3. Design a separate library acceptance ledger and progress view before
   claiming library matches. Do not insert library rows into authored
   `implemented.csv`/`matches.csv` merely to reuse their percentage.
4. After library/object coverage is reproducible, compare link layout, globals,
   imports, PE metadata, resources, and remaining data as the whole-executable
   lane. A successful normal link alone is not whole-image exactness.

`3rdparty/Detours` supports the optional reconstruction DLL and is not code from
the original target. Do not spend target-matching effort on that submodule.

## Single-session checkpoint

A bounded checkpoint contains:

- target version, address range, and source files;
- exact observations and separately labeled cross-version hypotheses;
- compiler/build mode and the full comparison command;
- comparison result or concrete blocker;
- mapping/global/header changes and every accepted unit that must be rerun.

Keep `config/claims.csv` header-only. Before yielding a browser session, either
restore experiments or commit a coherent batch, refresh `docs/RE_HANDOFF.md` if
the phase changed, and leave exact commands/results. Do not depend on chat
history or `.analysis/` filenames as the only explanation of current state.
Route reusable conclusions through `docs/KNOWLEDGE_BASE.md` and remove scratch
that has been superseded by tracked evidence.

For large switch interpreters, treat the jump table as a structural checksum before
fine byte matching. Resolve each COFF local-label relocation and compare the ordered
case entry addresses with the target. A uniform displacement across all later cases
localizes the missing or oversized lexical case without requiring a full-function
decompiler diff. Once the case starts align, compare the shared merge/tail separately;
this split reduced `GuiImpl::RunMsg` from a 5.6 KB problem to one missing 236-byte
case and a 26-byte loop-vs-explicit-call tail mismatch.

When using an adjacent-game source as a reconstruction scaffold, first match the TH08
basic-block order and constants rather than copying the whole ancestor function. The
TH06 HUD draw routine supplied the local declaration order and several source-shaped
expressions, but TH08 reordered the minimum-graphics/background work and added the
time-orb row. In `Gui::DrawGameScene`, the TH08-first ordering yielded an exact 0x1C4
frame on the first complete pass; the remaining 10-byte size error came from only two
ancestor-style integer-plus-zero-float expressions.


When a newly reconstructed production function uses a target global that previously existed only as an `extern` in a probe-only ECL lane, promote the storage to a production-linked TU before accepting the function. `Gui::FUN_0043741d` calls through `g_EclEnemyTableF54CC0`; the target span to the next known global proves 92 pointer entries, so its storage now lives in `EclGlobals.cpp` while probes and GUI share the `EclOperands.hpp` declaration. This is the same dependency-first rule used for probe-only Enemy helpers: strict object matching is not enough if the normal link would otherwise have no owner for the symbol.


The GUI updater at `0x435900` is now a strict 2,397-byte match. Its proven state model links the boss-gauge fade state (`impl+0x2a40`), VM update batches, three `GuiFormattedText` timers, stage-result score calculation, and the animated clock-result tail. Reuse the local `GuiStageResultUpdateOverlay` at `impl+0x22dec` instead of rediscovering those ten dwords in later GUI work; do not cache a pointer to the overlay unless the target does so.


`GuiImpl::DrawDialogue @ 0x43542B` is now a strict 1,107-byte match. For this family of GUI drawing routines, the TH06 source is especially useful for local declaration order and D3D state restoration, but TH08-specific portrait ordering must still be read from the target. The exact TH08 routine keeps two explicit if/else z-order pairs for four portrait VMs, so preserve lexical duplicate draw calls rather than abstracting them into a sort/loop.


`Gui::FUN_0043826b @ 0x43826B` is now a strict 1,982-byte result-overlay draw. It independently reuses the `GuiStageResultUpdateOverlay` fields proven by the 0x435900 updater, so those offsets now have both update-side and draw-side evidence. For result screens, preserve each lexical `AsciiManager::AddFormatText` call and its Y advance; visually equivalent string consolidation is not compiler-equivalent.


`Gui::FUN_00438A89` is now a strict 1,231-byte formatted-popup draw. The TH06 `Gui::OnDraw` popup block is a useful source-shape ancestor: TH08 keeps the same spell-card bonus centering logic, while expanding the full-power popup into a six-state switch. When an ancestor uses `strlen` of a string literal in a centering formula, preserve that call spelling instead of substituting the known length; the old compiler's partial constant folding can be target-visible.
