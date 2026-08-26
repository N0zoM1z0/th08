---
name: th08-semantic
description: Replace raw TH08 object offsets, anonymous fields, and absolute field views with evidence-backed C++ types and names while preserving accepted VC7 bytes and playable modern-port behavior. Use for semantic cleanup of already-authored source; do not use for new function recovery or target-linked library work.
---

# TH08 semantic reconstruction

Work only from the verified Japanese TH08 1.00d target and the current authored
source.  Read `AGENTS.md`, `docs/ARCHITECTURE.md`, `docs/RE_WORKFLOW.md`,
`docs/RE_HANDOFF.md`, and `docs/SEMANTIC_RECONSTRUCTION.md` before editing.

## Preflight

Run the target/tracking preflight and keep `config/claims.csv` header-only.  Use
`scripts/analysis/report-semantic-debt.py` as a candidate router, never as a
completion metric.  Select one structure and one coherent field family.

Confirm every touched function in `config/mapping.csv`,
`config/reccmp-functions.csv`, and `config/match-units.toml`.  Establish the
current accepted-unit baseline for every affected object before changing a
shared declaration.

## Recover meaning

Record target facts, corroboration, inference, and unknowns separately.  Inspect
all reads and writes, widths, signedness, bit operations, callers, strings, and
canonical object ownership.  TH06/TH07 and upstream names are corroboration
only when TH08 evidence agrees.  IDA evidence remains blocked until the active
database passes `docs/IDA_MCP.md`.

Prefer real fields, aggregates, enums, bitfields, and member/index expressions.
Preserve opaque storage when only its extent is known.  Add focused `C_ASSERT`
checks for relied-on sizes and offsets, remembering that layout assertions do
not prove names.

Do not create a new global for a view inside an existing aggregate, change
field width/signedness, hide uncertainty behind a union/accessor, or combine a
typed cleanup with unrelated control-flow refactoring.  Serialization,
instruction decoding, and platform ABI glue may be correctly byte-oriented.

## Validate and record

Use `$th08-matching` for the smallest VC7 object build and replay every accepted
unit in affected objects.  A shared header, PCH, inline-body, layout, or object-
owner change requires cold aggregate replay.

Compile and link the modern target for shared-source changes.  Run the Linux
layout verifier and a relevant state-transition smoke when the batch changes
global identity, initialization, callback state, persistence, rendering, or
fixed-address ownership.  VC7 exactness and Linux behavior are complementary;
neither proves the English field name without the evidence record.

Append an accepted batch to `docs/SEMANTIC_RECONSTRUCTION.md` with addresses,
evidence classes, layout assertions, exact replay results, portable result, and
retained unknowns.  Update `docs/RE_HANDOFF.md` only for a phase/focus/blocker
change.  End with tracking validation, `scripts/ci.py`, and
`git diff --check`.
