# Reusable knowledge map

Use this index to find prior evidence and record reusable lessons. For
orientation, use the
[documentation index](README.md) or [project guide](PROJECT_GUIDE.md).
Search the ledgers and focused documents before repeating target analysis or
compiler-shape probes. Live status is in [RE_HANDOFF.md](RE_HANDOFF.md) and the
ledgers.

## Where knowledge lives

| Need | Canonical location | What it contains |
| --- | --- | --- |
| Target identity, sections, inventory classes, repository layout | [ARCHITECTURE.md](ARCHITECTURE.md) | Stable project facts and evidence boundaries. |
| Current milestone, deferred work, next bounded lane | [RE_HANDOFF.md](RE_HANDOFF.md) | Short, replaceable current state only. |
| Production/probe/shared-include ownership and build selection | [SOURCE_MAP.md](SOURCE_MAP.md) | Current routing from a source family to its playable owner, VC7 probe, and validation entry point. |
| Current subsystem semantics | [SEMANTIC_INDEX.md](SEMANTIC_INDEX.md) | Short owner/evidence index; use the semantic policy and history documents for acceptance rules and chronological batch records. |
| Evidence ranking and reconstruction loop | [RE_WORKFLOW.md](RE_WORKFLOW.md) | Durable operating method and acceptance language. |
| Semantic field/type recovery and two-oracle acceptance | [SEMANTIC_RECONSTRUCTION.md](SEMANTIC_RECONSTRUCTION.md) | Evidence classes, bounded batch format, typed-layout rules, and VC7/portable validation matrix. |
| Semantic/readability method for later titles | [SEMANTIC_PLAYBOOK.md](SEMANTIC_PLAYBOOK.md) | Consumer closure, four ownership axes, naming stop rules, protocol audit, Oracle escalation, and the cross-title transfer contract. |
| Completed semantic batches | [SEMANTIC_HISTORY.md](SEMANTIC_HISTORY.md) | Chronological observed/corroborated/inferred evidence and Oracle results. |
| Command selection and scratch lifecycle | [TOOLS.md](TOOLS.md) | Public entry points, copyable recipes, and tool limits. |
| Exact authored totals | [PROGRESS.md](PROGRESS.md) and [config/matches.csv](../config/matches.csv) | Generated totals and accepted per-address evidence. |
| Target mappings and types | [config/mapping.csv](../config/mapping.csv), `config/reccmp-*.csv` | Imported/reconciled leads; mapping alone is not exactness. |
| Canonical exact replay | [config/match-units.toml](../config/match-units.toml) | COFF symbol, object, extent, and explicit relocations. |
| Aggregate cold-build attestation | `scripts/analysis/verify-exact-units.py --all` and [BUILD_MATCHING.md](BUILD_MATCHING.md) | Rebuilds every configured input before replay; the durable stale-object/PCH lesson is indexed under `cold-build`. |
| Whole-image observations and ownership cases | [WHOLE_IMAGE_RECONSTRUCTION.md](WHOLE_IMAGE_RECONSTRUCTION.md#reading-map) | Recorded link-contract observations and case studies; measurements retain their checkpoint scope. |
| Detailed VC7/comparator corpus | [BUILD_MATCHING.md](BUILD_MATCHING.md) | Address-backed build, relocation, boundary, ABI, and source-shape lessons. |
| Concise cross-subsystem compiler patterns | [VC7_ZUN_PATTERNS.md](VC7_ZUN_PATTERNS.md) | Reusable VC7/ZUN patterns with exact examples. |
| IDA/Ghidra trust boundary | [IDA_MCP.md](IDA_MCP.md) | Active-database attestation and safe fallback paths. |
| Long completed investigations | Focused `*_MATCHING.md` or `*_EXACT_NOTES.md` files | Chronological evidence and rejected probes for one bounded subsystem/function. |
| Active experiments | `.analysis/` | Disposable, untracked inputs and results. |

## Existing subject index

| Subject | Start here | Notes |
| --- | --- | --- |
| ECL/RunEcl dispatch and relocations | [RUNECL_FUNCTION_EXACT_NOTES.md](RUNECL_FUNCTION_EXACT_NOTES.md) | [Formal final result](RUNECL_FUNCTION_EXACT_NOTES.md#formal-relocation-manifest); earlier sections preserve intermediate investigation. Historical reproducers live under `scripts/analysis/historical/`. |
| Player callbacks, SHT ABI, option fields | [PLAYER_MATCHING.md](PLAYER_MATCHING.md), then search [BUILD_MATCHING.md](BUILD_MATCHING.md) for `Player::` | Compact ABI summary plus exact address-backed corpus entries. |
| Game manager state/setup/score | [Production ownership notes](GAME_MANAGER_MATCHING.md#production-ownership-and-order), [combined ownership case](WHOLE_IMAGE_RECONSTRUCTION.md#gamemanager-ownership-case) | Ownership summary and detailed source-shape cases. |
| Pause/retry stage menus | [STAGE_MENU_MATCHING.md](STAGE_MENU_MATCHING.md) | Known draw/update family and probe ownership. |
| GUI/title/replay source shapes | [Compiler-pattern corpus](BUILD_MATCHING.md#compiler-pattern-corpus), then the symbol or address | Includes switch-table extents, inline ownership, table dimensionality, and frame-shape cases. |
| Function boundaries, COFF aux extents, relocations | [Object comparison](BUILD_MATCHING.md#object-comparison) | Search for `compare_size`, `COMDAT`, `REL32`, or `DIR32`. |
| Relocated VC7 floating literals | [BUILD_MATCHING.md](BUILD_MATCHING.md), [scripts/match_literals.py](../scripts/match_literals.py) | Every `__real@...` symbol is decoded and target-checked; `data_hex` records reviewed high-risk values explicitly. |
| Generic VC7 declaration/branch/local patterns | [VC7_ZUN_PATTERNS.md](VC7_ZUN_PATTERNS.md) | Search this before creating expression or `#pragma var_order` matrices. |
| Target-linked CRT/D3DX work | [RE_HANDOFF.md](RE_HANDOFF.md), [th08-library skill](../.agents/skills/th08-library/SKILL.md), [config/library-provenance.toml](../config/library-provenance.toml), and [scripts/compare-library.py](../scripts/compare-library.py) | Paused library foundation: archive provenance, relocation-aware match units, and a separate accepted ledger. Resume only for a bounded whole-link dependency. |
| Whole-executable reconstruction | [scripts/compare-whole-image.py](../scripts/compare-whole-image.py), [RE_WORKFLOW.md](RE_WORKFLOW.md), then [RE_HANDOFF.md](RE_HANDOFF.md) | Cold-build PE diff, import/resource/debug contracts, section sizes, and accepted-address link anchors. |
| Translation-unit partition candidates | [scripts/analysis/report-tu-partition-candidates.py](../scripts/analysis/report-tu-partition-candidates.py) | Deterministic ranking by target-order inversions/drift jumps, plus bounded per-object anchor details. This is routing evidence, not a boundary claim. |
| Library candidate discovery | [scripts/analysis/propose-library-units.py](../scripts/analysis/propose-library-units.py) | Conservative review queue from one pinned archive; candidate status is not exact acceptance and must be promoted through an explicit unit plus `compare-library.py`. |
| Stale object/PCH exact-state failures | [Cold-build replay lesson](BUILD_MATCHING.md#aggregate-exact-state-requires-a-cold-build-replay) | Why focused historical successes cannot be promoted to a current aggregate without a cold full replay. |
| Raw offsets, anonymous fields, and semantic naming | [SEMANTIC_RECONSTRUCTION.md](SEMANTIC_RECONSTRUCTION.md), [SEMANTIC_PLAYBOOK.md](SEMANTIC_PLAYBOOK.md), [th08-semantic skill](../.agents/skills/th08-semantic/SKILL.md), then [scripts/analysis/report-semantic-debt.py](../scripts/analysis/report-semantic-debt.py) | Candidate scans are routing only. Accept one field family from target evidence plus applicable VC7 and portable oracle results. |
| ANM file/script/sprite namespaces | [ANM_RESOURCE_INDEX.md](ANM_RESOURCE_INDEX.md) | Keeps manager slots and resource-local IDs distinct and records the remaining opcode evidence queue. |
| Effect pool ownership and callback scratch roles | [EFFECT_STORAGE.md](EFFECT_STORAGE.md) | Borrowed-pointer/pool contract and callback-local vector-role matrix. |

Fast lookup recipes:

```bash
rg -n "0x004526C0|PlaybackExtendedInputAndFps" docs config src
rg -n "compare_size|COMDAT|DIR32|REL32" docs/BUILD_MATCHING.md
rg -n "pragma var_order|frame|stack home" docs/VC7_ZUN_PATTERNS.md
rg -n "cold-build|stale object|aggregate exact" docs/BUILD_MATCHING.md docs/RE_WORKFLOW.md
```

Use the target address to distinguish overloaded or provisional names.

## Whole-image reconstruction lessons

The detailed observations and case studies now live in
[WHOLE_IMAGE_RECONSTRUCTION.md](WHOLE_IMAGE_RECONSTRUCTION.md#reading-map).
Start with its reading map for linker contracts, translation-unit layout,
consumer-emitted helpers, and production-owner cases. The recorded
measurements retain their historical checkpoint scope.

For current whole-image work, use the [RE workflow](RE_WORKFLOW.md#current-phase-selection)
and [tool recipes](TOOLS.md#choose-the-command-by-question).

## Promote knowledge instead of accumulating scratch

Use this lifecycle for every nontrivial investigation:

1. Put temporary disassembly, matrices, logs, and objects under `.analysis/`
   with the target address or unit in the filename.
2. Once a conclusion survives the canonical comparison, encode exact evidence
   in the appropriate ledger first.
3. Record a broadly reusable compiler/build lesson in `BUILD_MATCHING.md`.
   Distill it into `VC7_ZUN_PATTERNS.md` only when it is useful across more than
   the original function or prevents a common wrong probe.
4. Use a focused note when the evidence chain or rejected probes are too long
   for the corpus. Put a completion/historical banner at the top when done.
5. Put only the live milestone, durable blocker, and immediate next command in
   `RE_HANDOFF.md`; replace obsolete state instead of appending chronology.
6. Add a row to this subject index when a new focused note becomes a durable
   entry point.
7. Delete the superseded `.analysis/` inputs and outputs after the tracked
   knowledge and reproducible commands are committed.

Keep rejected probes whose evidence and outcome help avoid repeating an
expensive investigation.

## Reusable lesson template

Use this compact structure in `BUILD_MATCHING.md`, `VC7_ZUN_PATTERNS.md`, or a
focused note:

```text
### Short pattern name
Scope: symbol @ address, compiler profile, object/unit
Observed: target instructions/layout/relocations (facts only)
Inference: source/ABI interpretation and confidence
Working shape: natural C/C++ form that reproduced the observation
Rejected alternative: only if plausible and materially different
Reproduce: exact build and comparator commands
Result: exact bytes/extent/relocations, or named remaining uncertainty
Generalization limit: where this lesson must not be copied blindly
```

An exact example should include the target address and canonical unit name.
Avoid phrases such as “current”, “final”, or “remaining” in durable corpus
entries unless they include a date/commit and are explicitly historical.

## What belongs where

- Put machine-checked exact state in CSV/TOML ledgers.
- Put stable project policy in `AGENTS.md` or `RE_WORKFLOW.md`.
- Put command routing in `TOOLS.md` and link to it from focused notes.
- Put current work in `RE_HANDOFF.md`.
- Keep executable/object/database output in ignored storage.
- Keep target observations separate from TH06/TH07 or upstream hypotheses.

## Knowledge review checklist

Before committing a knowledge update, check that:

- the target version/address and evidence class are explicit;
- the command still exists and `--help` explains its role;
- any exact claim names a reproducible comparator result;
- the lesson does not contradict the live ledgers or `RE_HANDOFF.md`;
- historical intermediate language is clearly labeled;
- another agent can find the conclusion by symbol, address, or subject index;
- replaced scratch and stale handoffs have been removed.

Run `python3 scripts/check-docs.py` and `python3 scripts/ci.py` after reorganizing
tracked knowledge.

### Library accepted rows and match units are one atomic claim

Commit library acceptance rows and their match units together. Each accepted
row in `config/library-matches.csv` names a unit in
`config/library-match-units.toml`, which records archive/member identity,
target extent, and relocations. `validate-library.py` rejects orphan rows.
If an interrupted commit separates the files, restore the reviewed unit and
rerun the comparator before updating progress.

### A zero-item conservative proposal queue is a routing result, not completion

When `propose-library-units.py --archive vc7-libcmt --min-size 1` returns zero
unique candidates, it means all candidates satisfying its strict function-aux,
size, supported-relocation, and non-relocation-byte gates have been configured.
The remaining CRT/runtime inventory needs a different investigation: boundary
repair, shared/local funclet evidence, COMDAT/alias analysis, or another archive
family. Preserve the proposer's strict gates when selecting that next step.
