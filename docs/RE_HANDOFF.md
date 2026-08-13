# TH08 reconstruction handoff — 2026-08-13

## State

- `main` is pushed through `d621819`. Preserve the user's unrelated untracked `.factory/` directory.
- Canonical target: Japanese TH08 1.00d, `resources/th08.exe`, SHA-256 `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`, 840,704 bytes.
- Exact progress: **132,473 / 459,958 authored bytes (28.80%)**. Source-present coverage is **160,166 / 459,958 (34.82%)** and is not exact coverage.
- Active claim: `0x004184B0`, `EclManager::RunEcl`. Keep changes to `src/EclRun.cpp` and private ECL `.inl` files unless the coordinator widens the claim.

## This batch

| Commit | Result |
| --- | --- |
| `01df70b` | Documents accepted mapped-image attestation when an IDB/loader hash differs from the canonical PE hash. |
| `c3fa0d5` | Adds the raw-float `mov/push` versus VC7 `fld/fstp` no-op probe to typed-re. |
| `9e1000c` | Adds the per-handler span-crosswalk acceptance rule to typed-re. |
| `d621819` | Reorders real RunEcl tail declarations to target-observed initialization order: `restorePosition`, `entry`, `savedPosition`. |

The read-only IDA session matched canonical bytes at `0x00402000`, `0x004184B0`, `0x0041E40F`, and `0x004A619E`. Its reported `ec101…928b` hash is acceptable only under the mapped-image procedure in `docs/IDA_MCP.md`. IDA is semantic/navigation evidence only; no target bytes were patched.

## RunEcl facts

- Target `0x004184B0`: 26,638 code bytes, 184 jump-table slots, 162 unique handlers; table at `0x0041ECBE`.
- Source audit is complete: opcodes 1–92 and 93–184 are present. Target direct calls: 463. Resolver calls: `ResolveInt=145`, `ResolveIntLValue=18`, `ResolveFloat=103`, `ResolveFloatLValue=24` (290 total).
- Current COFF code is `0x6843` and function size `0x6B3B`; the match manifest requires `0x680E`. `compare-function.py ecl-manager-run-ecl` must therefore remain a size-boundary error, not a matching claim.
- `scripts/crosswalk-ecl-dispatch.py --object build/probes/EclRun.obj` is the bounded diagnostic. Baseline physical handler delta: +53 bytes; largest spans: opcode 140 (+14), 75 (+13), 34 (+12), 39 (+12), 89 (+12), 166 (+12).

### Stack/lifetime evidence

Target tail begins at `0x0041E7F8` and uses `EBP-0xEC` for `restorePosition`, `-0xF0` for `entry`, `-0xFC..-0xF4` for saved position, `-0xE4` for loop index, and `-0xE8` for progress. The current object has the same shape but these homes are 0x30 deeper (`-0x11C`, `-0x120`, `-0x118..-0x110`, `-0x124`, `-0x128`). This is an earlier dispatcher scratch/lifetime discrepancy. Do not use inert locals, padding, assembly, or broad `#pragma var_order` to force it; attribute it to real handler locals first.

## Rejected probes

1. Replacing the ECL byte operand tail with a union or `i32[1]` did not alter RunEcl COFF.
2. Expanding opcode 39 resolver branches shortened the object by 42 bytes and changed the physical delta to +11, but disagreed with target `0x0041969A`, which has four independent resolved-value homes followed by two subtractions. Reverted.
3. `D3DXVECTOR3` or `Float3` for opcode 140 grew its span by 11 bytes and moved its local home away from the target.
4. `ResolveFloat(f32)` is exact at `0x00420120`; do not change its ABI. The caller-side `fld/fstp` shape remains open.

## Skills and next lane

Read `.agents/skills/th08-typed-re/SKILL.md` before another ABI/source-shape probe. This batch added the two reusable rules above: a smaller whole object is not evidence without a handler crosswalk, and operand-tail type rewrites can be COFF no-ops. Also follow `th08-re` and `th08-matching` for preflight and exact acceptance.

Use headless, low-load commands:

```bash
nice -n 15 python3 scripts/build.py -j1 build/probes/EclRun.obj
python3 scripts/crosswalk-ecl-dispatch.py --object build/probes/EclRun.obj
python3 scripts/audit-ecl-dispatch.py
python3 scripts/typed-re.py 0x4184b0 --compare --json > .analysis/typed-re-runecl.json
python3 scripts/typed-re.py --check
python3 scripts/progress.py --check
git diff --check
```

Keep experiments reversible under `.analysis/` and rebuild after every revert. Never use native/inline/naked assembly, machine-code arrays, fake behavior, arbitrary padding, or byte patching.

Next, isolate the real high-opcode locals that create the 0x30 tail shift (`SpawnPacket`, `Vec3`, and resolver temporaries). Compare one handler at a time; do not broadly rewrite the dispatcher. Only a manifest-size-correct full comparison may promote RunEcl's 26,638 bytes to exact coverage.
