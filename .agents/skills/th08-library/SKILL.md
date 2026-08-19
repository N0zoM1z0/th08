---
name: th08-library
description: Recover and attest target-linked TH08 VC7 CRT, compiler-runtime, and D3DX library functions after authored coverage. Use for library inventory boundaries, archive/member provenance, hash pinning, relocation-aware comparison design, or whole-link preparation; do not use for repository third-party submodules.
---

# TH08 target-linked library recovery

This skill covers `type=library` rows inside the original Japanese TH08 1.00d
image. It does not cover `3rdparty/Detours`, which only supports the optional
reconstruction DLL.

## Preflight

1. Read `AGENTS.md`, `docs/RE_HANDOFF.md`, `docs/RE_WORKFLOW.md`, and
   `docs/TOOLS.md`.
2. Run:

   ```bash
   python3 scripts/verify-target.py
   python3 scripts/validate-tracking.py --require-target
   python3 scripts/analysis/report-reconstruction-status.py \
     --category library --state missing-size --sort address
   ```

   The current first-lane result is seven rows. If that count changes, trust
   the command and current ledgers rather than copying an old prose list.
3. Select one coherent runtime/D3DX family. Reconcile every start and extent
   against target control flow before editing `mapping.csv`; imported Ghidra
   boundaries and the next CSV address are leads, not accepted sizes.

## Evidence gates

- Identify the exact originating toolchain/library version and archive member.
  Record SHA-256 for each archive used as evidence. A same-name TH07 archive or
  a modern SDK library is not a substitute.
- Keep target facts, archive/member facts, inferred symbol names, and unknown
  boundaries distinct.
- Compare code only after replaying COFF relocations and accounting for archive
  member padding, COMDATs, jump tables, thunks, and linker-owned transformations.
- Prefer compiler/library source or extracted COFF evidence. Never recreate CRT
  or D3DX bodies with inline assembly, byte arrays, empty stubs, or copied target
  bytes.

## Ledger boundary

No library exact ledger exists yet. `config/implemented.csv`,
`config/matches.csv`, and `docs/PROGRESS.md` currently describe authored work;
do not repurpose them for library percentages.

Before the first library acceptance, propose and review:

1. an archive-provenance manifest with hashes and member identities;
2. a library match-unit schema with target address, accepted extent, object or
   archive member, relocation replay, and evidence command;
3. validation that works without private target bytes in public CI and a local
   target-required acceptance command;
4. a separate library progress view that does not change authored totals.

There is intentionally no library scanner. Build one only after these inputs
and failure modes are pinned; fail closed on unknown archive hashes, ambiguous
members, boundary overlap, unsupported relocation, or target mismatch.

## Whole-executable handoff

Once library objects are reproducible, carry exact object order and linker
metadata into the whole-image lane: section layout, padding, globals/static
initialization, imports, PE headers, resources, and non-code data. A successful
normal link or a function-name match is not whole-executable exactness.

End each bounded batch with target address/range, boundary evidence,
archive/member/hash evidence, exact comparison result or named missing schema,
files changed, and remaining uncertainty. Keep `config/claims.csv` header-only,
run `python3 scripts/ci.py`, and update `docs/RE_HANDOFF.md` only when the
durable milestone changes.
