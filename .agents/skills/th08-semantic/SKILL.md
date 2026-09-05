---
name: th08-semantic
description: Replace raw TH08 object offsets, anonymous fields, protocol numbers, and absolute field views with evidence-backed C++ types and names while preserving accepted VC7 bytes and playable modern-port behavior. Use for semantic cleanup of already-authored source; do not use for new function recovery or target-linked library work.
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

When one fixed storage range has several target-proven roles selected by a
discriminator, a translation-unit-local typed union overlay may be safer than
changing a widely included VC7 header.  Anchor the single cast boundary to a
real member with `offsetof`/`sizeof` assertions, keep generic initialization
explicit, and use role-specific members everywhere else.  This exception is
for evidenced variant storage, not a way to assign meaning to unknown bytes.

Do not create a new global for a view inside an existing aggregate, change
field width/signedness, hide uncertainty behind a union/accessor, or combine a
typed cleanup with unrelated control-flow refactoring.  Serialization,
instruction decoding, and platform ABI glue may be correctly byte-oriented.

## Close router-invisible protocol debt

The structural debt report does not find numeric interpreter cases, stable
resource/sound/effect IDs, repeated flag masks, or ABI-compatible vector casts.
Audit those surfaces explicitly after field/layout cleanup:

1. Inventory every serialized opcode value and every numeric dispatch site.
   Include comparisons, range arithmetic, and out-of-order handlers, not only
   `case` labels.  Give every observed value an explicit enum initializer.
   Preserve the handler's physical order because VC7 switch emission can depend
   on lexical placement.  Use a neutral protocol name when the handler proves
   storage or control flow but not a narrower gameplay term.
   Do not stop after the primary opcode table: inventory operand selectors,
   timeline opcodes, stage/background streams, curve/mode selectors, replay
   synchronization flags, and other sibling dispatch tables.  TH08's primary
   ECL opcodes were readable while all 101 operand selectors and both secondary
   script streams were still raw; the structural debt router did not report
   any of them.
2. Separate instruction selector bits from physical object flag bits.  Two
   masks with the same numeric value are not interchangeable namespaces.  Name
   shifts and whole-word masks when target code requires a shift/test or raw
   dword update; do not rewrite them as prettier bitfield access if that changes
   emission.
3. Name only stable numeric IDs supported by a dispatcher/table and production
   call roles.  Resource slots may be named from load/get/release ownership;
   sound/effect names need target call-site behavior, with adjacent games used
   only as corroboration.  Leave stage-script data and visually ambiguous
   entries numeric or neutrally named rather than guessing from one animation.
4. For representation-compatible views such as `Float3 *` and
   `D3DXVECTOR3 *`, prefer a macro whose expansion is the already accepted cast.
   A normal inline accessor can introduce a call under `/Ob0`.  Keep the cast at
   a named boundary and validate both VC7 and the modern compiler.

Once a protocol surface is complete, add a source guard that checks its exact
value set, rejects numeric dispatch labels/writers on that surface, and proves
that every enum member is referenced by the decoder.  Keep this scoped: array
indices, numeric quantities, and visually ambiguous per-file ANM script IDs are
not automatically protocols and should not receive invented names merely to
drive a literal count to zero.

For serialized operand families, introduce typed wire schemas only after the
opcode names are stable.  Pin size, offset, width, padding, and signedness with
`C_ASSERT`.  In a large `/Ob0` interpreter, a compile-time field-index macro is
often safer than adding an `args` pointer or an accessor: derive the existing
resolver's numeric index with `offsetof(Type, member) / 4`, but keep the exact
resolver and bit-cast expression tree intact.

Do not assume two equivalent-looking float reads emit identically.  In the TH08
RunEcl readability pass, replacing
`*reinterpret_cast<f32 *>(&raw_i)` with a generic raw-float macro shortened the
function by one byte (`0x6B06` to `0x6B05`).  Restoring that precise bit-cast
tree inside the typed field macro recovered exact output.  Enable one operand
family at a time, rebuild `EclRun.obj`, and compare the complete function before
moving to the next schema.

## Reconcile post-port aggregate drift

When a portable-owner change breaks a previously accepted cold aggregate,
classify every failure before editing source: function extent, non-relocation
bytes, or relocation metadata.  For relocation failures, mask the relocation
fields on both sides and require zero remaining byte differences before
migrating the manifest.  Infer each target symbol base independently from the
target field value and COFF addend, then reject the migration if it conflicts
with an existing accepted base or lacks mapping/global-ledger evidence.  Keep
intentional nonzero addends; do not normalize them merely because a modern
owner provides a more convenient spelling.

An unoptimized VC7 translation unit may need a direct logical target symbol
even when the portable build stores that state inside a canonical aggregate.
In that case, keep the direct target declaration/storage and use a TU-local
`TH08_MODERN_PORT` macro bridge so the function body has one semantic spelling.
Do not replace a target-observed direct access with a non-inlined accessor.

Likewise, target-adjacent globals may support one typed contiguous operation
without promising adjacency in the portable linker.  Use the target operation
only in the VC7 build and explicit per-object operations in the portable build.
Both forms must describe the same initialized state and pass their respective
oracles.

## Validate and record

Use `$th08-matching` for the smallest VC7 object build and replay every accepted
unit in affected objects.  A shared header, PCH, inline-body, layout, or object-
owner change requires cold aggregate replay.

If a cold aggregate replay contradicts the accepted ledger, record the exact
command and generated failure set as a branch-baseline blocker.  Do not publish
the ledger total as a fresh exact result, broaden the semantic batch to repair
unrelated units, or weaken the focused oracle that covers the edited object.

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
