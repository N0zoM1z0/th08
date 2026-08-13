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
- Under the repository's VC7 `#pragma var_order` wrapper, local identifiers
  can affect allocation. After a stack layout is proven, treat a rename as a
  code-generation change and recompare it; prefer a semantic comment over a
  readability-only rename while tuning exact stack homes.
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
- Test every source-shaping change through `$th08-matching`.
- If target code ends before the next mapped function but the COFF auxiliary
  size continues through switch tables, compare the complete associated extent
  while reporting authored coverage from the code extent only. A size error is
  a boundary question before it is a source-shaping instruction.
- Large dispatchers may own several adjacent compiler tables. Prove each table
  boundary from code-local pointers and the next trusted function start, keep
  all table-entry relocations in the canonical manifest, and require exact
  relocation replay over the full COFF auxiliary extent. `AnmManager::ExecuteScript`
  is the corpus example: `0x366D` code plus `0x1A0` bytes of 91+6+7 entries.

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
