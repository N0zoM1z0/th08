---
name: th08-re
description: Reconstruct bounded functions from the original Japanese TH08 1.00d executable using hash-attested target evidence, upstream GensokyoClub source, and explicitly labeled TH06/TH07 corroboration. Use for TH08 disassembly, ABI recovery, function naming, layout recovery, source migration, or implementation work.
---

# TH08 reconstruction

Work only against the target in `config/target.toml`: Japanese 1.00d, SHA-256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`,
image base `0x00400000`, entry `0x004A619E`, and `.text`
`0x00402000`–`0x004B3B77`.

## Preflight

1. Read `AGENTS.md`, `docs/ARCHITECTURE.md`, `docs/RE_WORKFLOW.md`, the
   relevant `config/mapping.csv` and `config/reccmp-functions.csv` rows, and
   `config/claims.csv`.
2. Run:

   ```bash
   python3 scripts/verify-target.py
   python3 scripts/validate-tracking.py
   ```

3. Fail closed on IDA. The current IDA MCP session is TH07, not TH08. Do not
   query or mutate it for TH08 work. Use verified-target `objdump` or the
   dedicated headless Ghidra import described in `docs/IDA_MCP.md`.
4. Ensure the coordinator assigned the address and exclusive writable files.
   Reconcile mapping/Ghidra boundaries against complete target control flow.

## Recover and implement

1. Record facts as **Observed**, **Inferred**, or **Unknown**. Target bytes,
   imports, relocations, and directly decoded accesses are observations.
2. Inspect the full function plus callers, callees, adjacent code, strings,
   globals, access widths, stack cleanup, and saved registers.
   Prioritize coherent high-ROI clusters in one translation unit: a sub-0x20
   getter or predicate may unlock a nearby callback's `REL32`, and a neighboring
   `CutChain` often shares the same object profile and chain globals. Investigate
   getter, callback, then `CutChain`; adjacency is a prioritization signal, not
   proof of boundaries, ownership, or semantics.
3. Use the inherited TH08 source as a hypothesis when it is stubbed or
   incomplete. Use TH06 and TH07 only as semantic or compiler corroboration.
4. Recover calling convention, return behavior, field offsets, side effects,
   and exceptional paths before shaping source.
   For a large switch dispatcher that is not exact yet, audit source presence
   with three independent surfaces: enumerate every primary and nested jump-
   table slot including shared/default handlers; compare the target direct-call
   destination multiset with the built function's `REL32` multiset; and
   reconcile returns, fallthroughs, field offsets, and access widths. This can
   justify `implemented.csv`, never `matches.csv`.
5. Implement the smallest coherent function in its existing module. Do not
   manufacture behavior, paste decompiler output, patch target bytes, or use
   assembly/byte arrays/padding to force code shape.
   If a semantically exact leaf differs only by compiler profile or epilogue,
   inspect its address cluster and callers before reshaping the body: a member
   defined for one subsystem may have been emitted in another subsystem's
   translation unit. Preserve the class owner while testing the target-proven
   TU/profile, as with `GameManager::SetYoukaiGauge` in `Player.obj`.
6. Ask the coordinator to review shared layouts, canonical mapping names,
   `config/implemented.csv`, and `config/match-units.toml` changes.
7. Use `$th08-matching` for the focused VC7 build and strict comparison.

When a coordinator needs Ghidra-backed original objects and Ghidra is
configured, use the target-verifying disposable import path:

```bash
python3 scripts/export_ghidra_objs.py --import-csv
```

Do not substitute an existing or remote database whose input identity is not
attested.

`config/implemented.csv` means source-present only. Never promote it or a
mapping row into an exact-match claim without the canonical comparator result.

## Handoff

Report address and size, observations, inferences, unknowns, files changed,
target inspection commands, focused unit/result, and remaining ABI or layout
risk. Run `python3 scripts/validate-tracking.py` and `git diff --check` before
handoff. Do not commit binaries, toolchains, databases, reports, or credentials.
