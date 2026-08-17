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
2. Select a small function or contiguous address range and ensure no other
   worker owns the same source/global/type surface.
3. Locate it in `config/mapping.csv` and `config/reccmp-functions.csv`.
4. Inspect target disassembly, callers, callees, strings, globals, imports,
   nearby functions, and exception/control-flow edges.
5. Reconcile the analyzed function extent with actual target instructions.
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

## High-leverage lanes

Prefer dependency work that turns a giant dispatcher into bounded units:

- ECL: establish `EclManager` lifecycle and the target-observed `0x228` ECL
  context, then reconstruct the four integer/float rvalue/lvalue resolvers at
  `0x0041F420`, `0x0041FE10`, `0x00420120`, and `0x00420950`. Together they
  cover 4,830 target bytes and 290 of `RunEcl`'s 463 direct-call sites. Port
  the TH07 private-overlay method, not its offsets or opcode numbers.
- GUI: recover constructor-proven `GuiImpl`/`GuiMsgVm` layout before the hard
  bodies. `Gui::DrawGameScene` is the first large target because restoring its
  real callers naturally emits four `/Os` Supervisor graphics predicates;
  defer the 23-case `RunMsg` until its raw instruction and trailing state are
  named.
- Large dispatchers: use the jump-table/call-multiset audit in `$th08-re` to
  establish source presence, while retaining strict comparator-only exactness.

The ECL dependency-first pass now has exact lifecycle units, complete four
resolver tables, source coverage for all 184 `RunEcl` opcodes, and a typed
`EnemyManager::OnUpdate` overlay. Re-audit the dispatcher with
`python3 scripts/audit-ecl-dispatch.py`. Remaining work is service/global
binding and compiler-shape convergence; source-present status does not imply
that the giant dispatcher or update loop is byte-matching.

## Parallel handoff

A bounded handoff contains:

- target version, address range, and source files;
- exact observations and separately labeled cross-version hypotheses;
- compiler/build mode and the full comparison command;
- comparison result or concrete blocker;
- mapping/global/header changes that need coordinator review.

Workers should avoid shared headers and mapping files unless explicitly
assigned. The coordinator reruns comparisons and owns progress publication.
