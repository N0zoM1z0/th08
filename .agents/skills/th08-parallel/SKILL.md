---
name: th08-parallel
description: Coordinate two or more non-overlapping TH08 reconstruction workers while protecting the shared target, claims, mapping, ABI, match units, compiler, Git history, and generated progress. Use when delegating parallel TH08 analysis, migration, implementation, or matching lanes.
---

# TH08 parallel reconstruction

Use with `$th08-re`. Keep worker scopes address-bounded and file-exclusive.

## Coordinator ownership

The coordinator alone owns:

- `config/claims.csv`, canonical `config/mapping.csv` names and types, and
  `config/implemented.csv` integration;
- shared headers/layouts, globals, `config/match-units.toml`, and relocation
  entries;
- IDA/Ghidra database writes, typed-RE automation/rules, generated progress,
  commits, and pushes;
- final VC7 builds, strict comparisons, and acceptance claims.

Workers never commit or push. They edit only explicitly assigned source files
or return source/shared-state proposals.

## Dispatch

Before dispatch:

1. Run `python3 scripts/verify-target.py` and
   `python3 scripts/validate-tracking.py`.
2. Fail closed on the current TH07 IDA session. Prepare evidence with verified
   target `objdump` or the repository's verified disposable TH08 headless
   Ghidra import.
3. Reconcile the exact function boundary and check `config/claims.csv`.
4. Add one canonical claim row with the assigned address, owner, UTC start,
   branch, and bounded notes. Do not invent a claim-management command; this
   repository currently validates the CSV directly.
5. Assign non-overlapping addresses and exclusive writable files. State any
   neighboring ranges and shared files that are explicitly excluded.
6. Create or approve the canonical `config/match-units.toml` entry before
   asking a worker to tune code generation.

## Worker brief

Include:

- target address/size and excluded neighbors;
- exclusive writable files and analysis-only references;
- exact target evidence already captured or safe read-only commands;
- unit name, object basename, and comparator command when matching is allowed;
- prohibition on IDA writes, claims, mappings, shared ABI, match manifests,
  generated progress, commits, and pushes;
- required Observed/Inferred/Unknown handoff and first mismatch/blocker.

Workers may inspect `_references/th06` and the TH07 tree read-only. Searches
must exclude `_references`, `build`, downloaded tools, and Wine prefixes unless
those paths are the explicit target; never follow a Wine `dosdevices/z:`
symlink.

## Integrate

Serialize focused VC7 builds and comparisons through the coordinator. Review
every diff, rerun the canonical unit with `$th08-matching`, apply approved
mapping/implemented/layout changes, release the claim, regenerate progress,
and run:

```bash
python3 scripts/validate-tracking.py
python3 scripts/progress.py --check
git diff --check
```
