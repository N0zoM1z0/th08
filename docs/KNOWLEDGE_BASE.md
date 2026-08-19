# Reusable knowledge map

This page routes accumulated TH08 knowledge. It is an index and contribution
policy, not another status snapshot. Search the ledgers and focused documents
before repeating target analysis or compiler-shape probes.

## Where knowledge lives

| Need | Canonical location | What it contains |
| --- | --- | --- |
| Target identity, sections, inventory classes, repository layout | [ARCHITECTURE.md](ARCHITECTURE.md) | Stable project facts and evidence boundaries. |
| Current milestone, deferred work, next bounded lane | [RE_HANDOFF.md](RE_HANDOFF.md) | Short, replaceable current state only. |
| Evidence ranking and reconstruction loop | [RE_WORKFLOW.md](RE_WORKFLOW.md) | Durable operating method and acceptance language. |
| Command selection and scratch lifecycle | [TOOLS.md](TOOLS.md) | Public entry points, copyable recipes, and tool limits. |
| Exact authored totals | [PROGRESS.md](PROGRESS.md) and `config/matches.csv` | Generated totals and accepted per-address evidence. |
| Target mappings and types | `config/mapping.csv`, `config/reccmp-*.csv` | Imported/reconciled leads; mapping alone is not exactness. |
| Canonical exact replay | `config/match-units.toml` | COFF symbol, object, extent, and explicit relocations. |
| Aggregate cold-build attestation | `scripts/analysis/verify-exact-units.py --all` and [BUILD_MATCHING.md](BUILD_MATCHING.md) | Rebuilds every configured input before replay; the durable stale-object/PCH lesson is indexed under `cold-build`. |
| Detailed VC7/comparator corpus | [BUILD_MATCHING.md](BUILD_MATCHING.md) | Address-backed build, relocation, boundary, ABI, and source-shape lessons. |
| Concise cross-subsystem compiler patterns | [VC7_ZUN_PATTERNS.md](VC7_ZUN_PATTERNS.md) | Reusable VC7/ZUN patterns with exact examples. |
| IDA/Ghidra trust boundary | [IDA_MCP.md](IDA_MCP.md) | Active-database attestation and safe fallback paths. |
| Long completed investigations | Focused `*_MATCHING.md` or `*_EXACT_NOTES.md` files | Chronological evidence and rejected probes for one bounded subsystem/function. |
| Active disposable experiments | `.analysis/` | Untracked scratch only; never current truth. |

## Existing subject index

| Subject | Start here | Notes |
| --- | --- | --- |
| ECL/RunEcl dispatch and relocations | [RUNECL_FUNCTION_EXACT_NOTES.md](RUNECL_FUNCTION_EXACT_NOTES.md) | Completed historical investigation; the final exact result is at the end. Historical reproducers live under `scripts/analysis/historical/`. |
| Player callbacks, SHT ABI, option fields | [PLAYER_MATCHING.md](PLAYER_MATCHING.md), then search `BUILD_MATCHING.md` for `Player::` | Compact ABI summary plus exact address-backed corpus entries. |
| Game manager state/setup/score | [GAME_MANAGER_MATCHING.md](GAME_MANAGER_MATCHING.md), then search `BUILD_MATCHING.md` for `GameManager::` | Focused notes are intentionally short; detailed shapes are in the corpus. |
| Pause/retry stage menus | [STAGE_MENU_MATCHING.md](STAGE_MENU_MATCHING.md) | Known draw/update family and probe ownership. |
| GUI/title/replay source shapes | Search `BUILD_MATCHING.md` for the symbol or address | Includes switch-table extents, inline ownership, table dimensionality, and frame-shape cases. |
| Function boundaries, COFF aux extents, relocations | `BUILD_MATCHING.md` | Search for `compare_size`, `COMDAT`, `REL32`, or `DIR32`. |
| Generic VC7 declaration/branch/local patterns | `VC7_ZUN_PATTERNS.md` | Search this before creating expression or `#pragma var_order` matrices. |
| Target-linked CRT/D3DX work | `RE_HANDOFF.md`, `$th08-library`, `config/library-provenance.toml`, and `scripts/compare-library.py` | Separate archive provenance, relocation-aware match units, accepted library ledger, and current library lane. |
| Library candidate discovery | `scripts/analysis/propose-library-units.py` | Conservative review queue from one pinned archive; candidate status is not exact acceptance and must be promoted through an explicit unit plus `compare-library.py`. |
| Stale object/PCH exact-state failures | Search `BUILD_MATCHING.md` for `cold-build` | Why focused historical successes cannot be promoted to a current aggregate without a cold full replay. |

Fast lookup recipes:

```bash
rg -n "0x004526C0|OnUpdateHighPrioDemo2" docs config src
rg -n "compare_size|COMDAT|DIR32|REL32" docs/BUILD_MATCHING.md
rg -n "pragma var_order|frame|stack home" docs/VC7_ZUN_PATTERNS.md
rg -n "cold-build|stale object|aggregate exact" docs/BUILD_MATCHING.md docs/RE_WORKFLOW.md
```

Use the target address when names are overloaded or provisional. A logical
name alone is not a unique exact identity.

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

Do not preserve every experiment. Keep a rejected probe only when its target
evidence and outcome will prevent a plausible, expensive repetition.

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

- Put machine-checked exact state in CSV/TOML ledgers, never only in prose.
- Put stable project policy in `AGENTS.md` or `RE_WORKFLOW.md`, not a focused
  function note.
- Put command routing in `TOOLS.md`; do not copy competing command lists into
  every note.
- Put current work in `RE_HANDOFF.md`; do not use completed notes as a queue.
- Put executable/object/database output in ignored storage, not Git.
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

`config/library-matches.csv` must never contain an accepted row whose `unit` is absent from `config/library-match-units.toml`.  The accepted row names the claim; the unit preserves archive/member identity, target extent, and every relocation needed to reproduce it.  `validate-library.py` intentionally fails on orphan accepted rows.  If an interrupted or split commit separates the two files, restore the reviewed unit schema and rerun the canonical comparator before treating progress as current; do not weaken the validator or keep a progress-only claim.
