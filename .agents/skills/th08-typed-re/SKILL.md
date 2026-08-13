---
name: th08-typed-re
description: Generate and interpret target-pinned TH08 instruction and ABI fact packets with scripts/typed-re.py. Use for stack layout, register homes, access widths, direct calls, return cleanup, or VC7 source-shape diagnosis before strict comparison.
---

# TH08 typed reconstruction

Use the helper as a read-only fact extractor below the semantic decompiler
layer. It consumes only the hash-attested TH08 image and repository ledgers;
it never edits source, analysis databases, or tracking state.

## Generate a packet

1. Follow `$th08-re` preflight and ownership rules.
2. Run:

   ```bash
   python3 scripts/typed-re.py ADDRESS --compare --json \
     > build/typed-re-ADDRESS.json
   ```

3. Read `exact_observations` first: target bytes, stack accesses, register
   homes, saved registers, direct calls, absolute operands, and return cleanup.
4. Treat `inferences.compiler_recommendations` only as source-shaping probes.
5. Accept matching only through the canonical `comparison.report.result` from
   `scripts/compare-function.py`.

If no match unit owns the address, the packet remains useful for ABI recovery,
but comparison state is `not_configured`; ask the coordinator to define and
review the unit before claiming exactness.

## Shape source

- Prefer a type or lifetime change that explains several observed instructions.
- Preserve signed and unsigned narrow types when `movsx` or `movzx` requires
  them.
- Diagnose frame differences through declaration order and real lifetimes;
  never add anonymous filler, inert locals, fake behavior, or ABI lies.
- For a large dispatcher, derive switch-wide scratch locals from stable target
  EBP homes across several handlers. Reuse a real outer scalar/index home when
  the target does, and keep a table lookup direct when the target has no stable
  pointer home. Probe one slot at a time so movement of later homes remains
  attributable.
- If a target branches once around an entire large dispatch but VC7 emits a
  short inverse branch followed by a near jump, test the positive condition
  with the whole dispatch lexically nested inside it. Equivalent early exits
  and gotos can produce a different branch shape.
- Under the repository's VC7 `#pragma var_order` wrapper, local identifiers
  can affect allocation. After a stack layout is proven, treat a rename as a
  code-generation change and recompare it; prefer a semantic comment over a
  readability-only rename while tuning exact stack homes.
- When a target resolves one value into a stack home and then uses that same
  value both to write a field and to call a setter, first test the natural
  chained assignment `setter = (field = ReadValue(...))`. It preserves the
  C++ value flow while allowing VC7 to retain one temporary; a separately
  named local can move fastcall parameter homes. Verify the whole function,
  since this is a source-shaping hypothesis rather than a byte-forcing device.
- If an inlined fixed-size structure-tail `memcpy` has the correct semantics,
  size, and `rep movsd` but schedules its count and source setup differently,
  probe the typed address of the first copied field instead of byte-pointer
  arithmetic from the parent object. This is a source-shape hypothesis, not a
  shortcut around full relocation replay; `ReplayManager::SaveReplay` at
  `0x004531F0` is the exact corpus example.
- An ECX home proves only that incoming ECX was saved to a stack slot. Decide
  whether it is a C++ receiver or fastcall argument zero from the decorated
  symbol, ledger convention, and call sites; never create inert code merely to
  reproduce the home.
- Keep target facts separate from field names, calling-convention guesses,
  compiler recommendations, and TH06/TH07 hypotheses.
- Do not name an absolute memory operand as a standalone global from read-side
  evidence alone.  Search write, construction, and destruction xrefs for a
  known global owner.  If the target address is `global + member_offset`, keep
  that ownership in source so VC7 emits a `DIR32` relocation to the base symbol
  with the real addend; a zero-addend alias records the wrong layout even when
  it resolves to the same runtime address.  RunEcl's enemy ANM pointers at
  `0x00F54E0C` and `0x00F54E10` are the corpus example: both are members of
  `g_EnemyManager`, not independent globals.
- Resolve comparison destinations by each CSV header's named `address` column;
  the function/global ledgers place it differently from float/string ledgers.
  Keep attested IAT slots, import thunks, and other non-inventory symbols in
  `config/reccmp-relocations.csv`. Never add a fake function or global merely
  to make relocation replay pass. A destination being ledgered only removes an
  evidence gap: require exact isolated extent and full zero-difference replay
  before adding a match. The zwave and AnmManager unblocked batches are the
  corpus example (15 functions, 5,259 authored bytes, 146 relocations).
- Test every source-shaping change through `$th08-matching`.
- If target code ends before the next mapped function but the COFF auxiliary
  size continues through switch tables, compare the complete associated extent
  with `compare_size` while keeping `size` at the authored code extent. Report
  and count authored coverage from `size` only. Normalize compiler-local table
  symbols by relocation offset and resolved target, and replay every entry. A
  size error is a boundary question before it is a source-shaping instruction.
  The four exact AnmVm accessors at `0x0045E650..0x0045E953` are the compact
  corpus: 615 authored bytes plus 136 associated table bytes, all 751 compared.
- Large dispatchers may own several adjacent compiler tables. Prove each table
  boundary from code-local pointers and the next trusted function start, keep
  all table-entry relocations in the canonical manifest, and require exact
  relocation replay over the full COFF auxiliary extent. `AnmManager::ExecuteScript`
  is the corpus example: `0x366D` code plus `0x1A0` bytes of 91+6+7 entries.
- Do not infer a local vector type solely from matching three `f32` fields.
  An observed default-constructor call is a type/translation-unit fact: test
  the candidate type in the affected handler and retain it only if the span
  crosswalk improves. In particular, TH08 `RunEcl` opcode 140's
  `0x0040B460` construction is not reproduced by substituting SDK
  `D3DXVECTOR3` or the project `Float3`: both probes grew the handler by 11
  bytes because they moved the vector home from the target's `EBP-0x60` to
  the object's `EBP-0x90`. Reconcile the dispatcher stack layout first.
- When a dispatcher’s COFF extent grows or shrinks, make a read-only span
  crosswalk before changing source: resolve every target jump-table slot and
  every COFF table `DIR32` relocation to its handler start, deduplicate and
  sort starts in physical order, then compare adjacent target and object
  handler spans. This attributes a size delta to a bounded handler (including
  shared/default handlers) without treating Ghidra/IDA extents as compiler
  boundaries. It is a diagnostic fact map, not a matching claim; retain full
  relocation replay and canonical comparison as the acceptance gate.
- When a target passes a raw float operand with `mov`/`push` but VC7 emits an
  `fld`/`fstp` argument shuffle, changing only a byte-tail reinterpret cast
  into an overlay union can leave the COFF completely unchanged. Reject that
  no-op probe; investigate the resolver call expression or ABI instead. TH08
  `RunEcl` opcodes 34 and 39 are the corpus case.

## Improve the model

The stable CLIs are `scripts/typed-re.py` and
`scripts/scan-vc7-library.py`; implementation modules live under
`scripts/typed_re/`. Only the coordinator may change this automation, its
rules, regressions, or skill.

For a new extractor or rule, fail closed on target identity, PE mapping,
ledger extent, and incomplete decoding; keep deterministic target observations
out of `inferences`; add a target-pinned assertion to `--check`; then run:

```bash
python3 -m compileall -q scripts/typed_re scripts/typed-re.py \
  scripts/scan-vc7-library.py
python3 scripts/typed-re.py --check
```

The VC7 library scanner currently returns `unsupported` until SHA-pinned TH08
archives, a relocation allowlist, and comparator replay are configured. Do not
substitute TH07 archives or rules.

## Handoff

Report the address and size, exact observations used, recommendation tested,
canonical comparison unit and result, and any helper/rule changes.
