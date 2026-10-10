# Reverse-engineering workflow

Use this page for evidence ranking, bounded reconstruction, and acceptance.
For orientation, see the [project guide](PROJECT_GUIDE.md); for session-start
commands, see [TOOLS.md](TOOLS.md#start-every-writable-session). Current work
is selected through the [handoff](RE_HANDOFF.md) and live ledgers.

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
   Every 32-bit or 64-bit `__real@...` relocation is decoded from its COFF
   symbol automatically. Static validation requires a `DIR32` relocation with
   zero addend; target-backed validation compares the decoded little-endian
   value with the addressed PE data. Record `data_hex` for a
   source-significant or corrected literal so that the reviewed bytes remain
   explicit in the match-unit row. A relocated instruction field matching by
   itself does not attest the value stored at the destination.
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

Imported library/runtime functions may legitimately contain compiler-generated
SEH cleanup funclets whose independent starts lie inside the parent's contiguous
mapping extent.  Record only proven adjacent parent/child overlaps in
`config/mapping-overlaps.csv` with `kind=nested-funclet`.  The tracking validator
requires every exception to describe a current overlap and reports any
unclassified overlap separately.  Do not shrink a parent through live control
flow or delete a funclet row merely to eliminate an overlap warning.

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

Record `matching` from the configured target comparator and generate
published percentages from current reproducible results. Mappings,
source-selection rows, and builds each retain their separate tracking roles.

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

Authored source is present for the complete authored inventory. The current
ledger contains 1,106 accepted exact units and one authored near match,
`ReplayManager::PlaybackExtendedInputAndFps @ 0x004526C0`, outside the accepted
ledger. A configured unit is not by itself an accepted result; live counts
come from `scripts/analysis/report-reconstruction-status.py` and
`config/matches.csv`, not this prose.

The semantic source-recovery phase formerly carried on
`semantic/typed-reconstruction` is complete and was merged into `main` through
PR #4; that remote branch has been retired. Start any newly bounded semantic
maintenance from current `main` on a fresh short-lived branch and follow
`docs/SEMANTIC_RECONSTRUCTION.md`. Existing accepted VC7 units are the binary
oracle; the modern Windows/Linux source products are the portability and
behavior oracle. Candidate counts from
`scripts/analysis/report-semantic-debt.py` select work but are not a progress
percentage or proof that every match should be rewritten.

Whole-executable layout reconstruction remains a valid deferred lane. Authored
source is complete, all library inventory rows have bounded extents, exact
archives are hash-pinned, and a reviewed subset of library members has a
separate accepted ledger. Broad library/runtime scanning remains paused: a
normal executable already links, so additional library work must be requested
by a concrete whole-image difference rather than by inventory percentage alone.

Native Windows i386 compile/link/play validation completed its prerequisite
whole-program pass on 2026-09-10. The reproducible order, final artifacts, and
runtime evidence are retained in `docs/WINDOWS_I386_RUNTIME.md`; the discovered
defects and dispositions remain in `docs/RUNTIME_ISSUES.md`. This closes the
gate that had blocked further modern-port stabilization. A modern MinGW/Linux
success or compatibility initializer still cannot substitute for this oracle,
and a future shared owner, ABI, TU, layout, PCH, compiler-profile, or production
link-graph change must rerun the native gate before claiming it remains valid.
When a new native run exposes a defect, repair one target-supported production
family and record the observation; do not transplant a port-only patch into the
native image.

New work should now select a fresh bounded lane: the modern redistributable
Windows package, the sole authored near match, or a whole-image difference that
identifies a concrete layout/library dependency. Do not continue broad native
runtime auditing without a new reproduction or a shared-source change that
invalidates the completed checkpoint.

For one semantic batch:

1. select one structure/field family and record the target users, offsets,
   widths, evidence classes, affected exact units, and portable surface;
2. add or preserve focused `sizeof`/`offsetof` assertions, then replace only
   the layout-shaped expressions supported by that evidence;
3. replay all affected accepted VC7 units and compile/link the modern target;
4. after a shared layout, PCH, inline, owner, or fixed-address change, run the
   cold aggregate VC7 gate and the applicable Linux layout/runtime checks;
5. record the accepted batch in `docs/SEMANTIC_HISTORY.md`, without
   changing authored/exact totals for a naming-only improvement.

For the deferred whole-executable lane, start from a single-job cold normal
build:

1. Run:

   ```bash
   python3 scripts/build.py --fresh
   python3 scripts/compare-whole-image.py --json \
     > build/whole-image-report.json
   ```

   The comparator verifies the canonical target, then separates PE
   headers/directories, section bytes, imports, resources, debug data, and
   accepted-unit linker-map address summaries. Add `--include-anchor-details`
   only for a bounded per-function layout investigation. The report is a
   diagnostic, not an exact ledger, and a successful link is not whole-image
   exactness.
2. Repair one evidence-backed link contract at a time: entry point and linker
   flags, section layout, resource tree/metadata, import set and descriptor
   order, object/archive order, globals, or static initialization. Rebuild cold
   after changing the link graph or a build-internal generator.
3. Recover emission ownership before tuning padding or global order. Use
   accepted function addresses as link-layout anchors and rank candidates with
   `scripts/analysis/report-tu-partition-candidates.py`. Inspect one production
   object at a time:

   - **Target neighborhoods:** order resets and large drift jumps suggest TU
     partitions. Inspect mapped non-anchor neighbors to establish boundaries.
     Object-list permutations alone cannot repair code interleaved inside an
     object.
   - **Distant clusters:** move real definitions to a same-profile production
     TU when target neighbors establish a separate owner. Update match-unit
     and namespace-map ownership and replay donor and recipient together.
   - **Consumer COMDATs:** inspect defined and undefined production symbols for
     plausible header helpers. A target-neighbor consumer may emit the exact
     copy under a different profile from the nominal class TU. Restore the
     header body, clean the PCH, and compare the consumer copy and affected
     production/probe callers.
   - **Body visibility:** if a shared header body changes an accepted caller,
     keep its declaration in the header and place the exact definition in the
     target-neighbor consumer. Preserve inline visibility when a target caller
     contains the body without a call relocation.
   - **Header order:** alternating wrappers and callees can indicate paired
     consumer-emitted COMDATs. Verify the outer caller and both helper layers.
     When several families first emit from one PCH consumer, test header include
     order and compare the complete family sequence after a clean PCH build.
   - **Explicit blocks:** a target-contiguous block can contain several classes.
     Preserve its local sequence when neighbors, profile, and exact replay
     agree. Making a deferred accessor explicit also requires a bounded caller
     set and relocations proving that the call boundary remains intact.
   - **Production use:** require a production caller before assigning a helper
     an owner. Restore lexical order within a continuous target neighborhood;
     create a separate TU when the target partition supports it.

   Moving a caller or helper may remove an implicit COMDAT from the donor.
   Inspect defined symbols in every affected object and update its canonical
   owner when target neighbors and natural emission agree. Cold-link and measure
   the new runs/drift, then run the aggregate replay.
4. Return to one CRT/D3DX/compiler-runtime member only when the whole-image
   report or a bounded link-provenance trace identifies that member as an
   import, extent, relocation, or layout dependency. For rebuild-only imports,
   run `python3 scripts/analysis/report-import-provenance.py`; it hash-checks
   configured archives and distinguishes current-map members from other archive
   candidates. Pin archive/member identity and use the separate library
   acceptance path:
   `config/library-provenance.toml`, `config/library-match-units.toml`,
   `scripts/compare-library.py`, and `config/library-matches.csv`. Public CI
   validates the schema without private archives; local attestation adds
   `python3 scripts/validate-library.py --require-archives`. Never put library
   claims in authored `implemented.csv` or `matches.csv`.

`3rdparty/Detours` supports the optional reconstruction DLL and is not code from
the original target. Do not spend target-matching effort on that submodule.

Before changing aggregate exact totals, run:

```bash
python3 scripts/analysis/verify-exact-units.py --all --json \
  > build/accepted-unit-replay.json
```

`--all` regenerates the objdiff graph, cleans Ninja outputs and known VC7/linker
side outputs, builds all configured objects serially, and then replays the
accepted ledger. A run with
`--reuse-build`, a normal link, or a focused object comparison cannot attest
repository-wide exact totals.

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

## Source-recovery examples

These examples come from completed reconstruction batches. Use the ledgers for
current acceptance and the [compiler-pattern corpus](BUILD_MATCHING.md#compiler-pattern-corpus)
for the detailed evidence.

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

When a newly reconstructed production function uses a target global that previously existed only as an `extern` in a probe-only lane, first provide a production owner so the normal link is testable, then revisit that provisional identity when the enclosing aggregate is recovered.  The early ECL name `g_EclEnemyTableF54CC0` was useful for bringing `Gui::FUN_0043741d` into the production link, but later whole-owner evidence proved `0x00F54CC0 == g_EnemyManager + 0x9DCDA0`.  Its apparent indices crossed Boss pointers and unrelated integer fields, so the alias was retired in favor of typed `EnemyManager` members and manager-base relocations.  Strict object matching proves emitted addresses, not that a provisional global boundary is semantically final.

The GUI updater at `0x435900` was accepted as a strict 2,397-byte match. Its proven state model links the boss-gauge fade state (`impl+0x2a40`), VM update batches, three `GuiFormattedText` timers, stage-result score calculation, and the animated clock-result tail. Reuse the local `GuiStageResultUpdateOverlay` at `impl+0x22dec` instead of rediscovering those ten dwords in later GUI work; do not cache a pointer to the overlay unless the target does so.

`GuiImpl::DrawDialogue @ 0x43542B` was accepted as a strict 1,107-byte match. For this family of GUI drawing routines, the TH06 source is especially useful for local declaration order and D3D state restoration, but TH08-specific portrait ordering must still be read from the target. The exact TH08 routine keeps two explicit if/else z-order pairs for four portrait VMs, so preserve lexical duplicate draw calls rather than abstracting them into a sort/loop.

`Gui::FUN_0043826b @ 0x43826B` was accepted as a strict 1,982-byte result-overlay draw. It independently reuses the `GuiStageResultUpdateOverlay` fields proven by the 0x435900 updater, so those offsets now have both update-side and draw-side evidence. For result screens, preserve each lexical `AsciiManager::AddFormatText` call and its Y advance; visually equivalent string consolidation is not compiler-equivalent.

`Gui::FUN_00438A89` was accepted as a strict 1,231-byte formatted-popup draw. The TH06 `Gui::OnDraw` popup block is a useful source-shape ancestor: TH08 keeps the same spell-card bonus centering logic, while expanding the full-power popup into a six-state switch. When an ancestor uses `strlen` of a string literal in a centering formula, preserve that call spelling instead of substituting the known length; the old compiler's partial constant folding can be target-visible.
