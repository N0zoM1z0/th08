# Current reconstruction handoff

This file records only the current durable state. Historical investigations
belong in focused notes such as `RUNECL_FUNCTION_EXACT_NOTES.md`; live counts
come from the ledgers, not this prose.

## Current status

As cold-built and replayed on 2026-08-19 against the original Japanese TH08
1.00d target:

- authored source: **1,107 / 1,107 functions**, **459,757 / 459,757 bytes**;
- strict authored exact: **1,091 / 1,107 functions**, **445,728 / 459,757 bytes**;
- library inventory: **1,112 classified functions**; 1,105 currently have
  mapping sizes totaling 215,533 bytes; no library exact ledger exists;
- `config/claims.csv` is header-only;
- a cold normal VC7 build links `build/th08.exe` successfully;
- a cold objdiff build followed by full replay passes **1,091 / 1,091**
  accepted units;
- repository-owned target-independent CI passes.

Regenerate or verify these values with:

```bash
python3 scripts/analysis/report-reconstruction-status.py --summary
python3 scripts/validate-tracking.py --require-target
python3 scripts/analysis/verify-exact-units.py --all --json \
  > build/accepted-unit-replay.json
python3 scripts/progress.py --check
python3 scripts/ci.py
```

`verify-exact-units.py --all` performs its own single-job cold build. Do not add
`--reuse-build` when attesting aggregate totals.

## Authored exact backlog

Two functions were already deferred and have never been accepted. Do not count
them as exact or restart broad brute-force matrices without a new target-backed
hypothesis.

- `ReplayManager::OnUpdateHighPrioDemo2 @ 0x004526C0`, 361 bytes: the natural
  source object is one byte longer and the residual is a register-allocation
  phase difference. Its adjacent callback family is exact.
- `TitleScreen::RegisterChain @ 0x0047146D`, 281 bytes: target and object have
  the same extent; five stack-displacement bytes differ because the target
  frame is `0x40` while the current natural object frame is `0x5C`.

The source for both is present and behaviorally reconstructed. Exact coverage
will increase only when the canonical comparator returns `exact` for a natural,
evidence-backed C++ form.

Fourteen additional rows were removed from `config/matches.csv` after the cold
aggregate replay showed that their historical build objects no longer
reproduced. Their configured units remain in `config/match-units.toml` as
diagnostic starting points; they are not accepted claims.

| Address | Unit | Cold-build result class |
| --- | --- | --- |
| `0x00402600` | `accepted-exact-00402600` | caller bytes/relocations differ |
| `0x00405420` | `ascii-manager-on-draw-high-prio-impl` | relocation symbol drift |
| `0x00406580` | `main-exact-00406580` | configured COMDAT absent from `main.obj` |
| `0x004069F0` | `manual-comdat-exact-004069f0` | configured COMDAT symbol drift |
| `0x0040EC00` | `anm-vm-set-z-rotation` | object `0x2D`, target `0x2B` |
| `0x00439B29` | `game-manager-roi-exact-00439b29` | object `0x8F`, target `0x9E` |
| `0x00448B16` | `music-room-roi-exact-00448b16` | caller bytes/relocations differ |
| `0x0044C390` | `player-onupdate` | relocation symbol drift |
| `0x0045964D` | `result-screen-roi-exact-0045964d` | caller bytes/relocations differ |
| `0x00465CC0` | `anm-manager-unblocked-exact-00465cc0` | relocation symbol drift |
| `0x0046F5CB` | `accepted-source-exact-0046f5cb` | object `0x7AC`, target `0x794` |
| `0x0046FFC0` | `accepted-source-exact-0046ffc0` | object `0x357`, target `0x34B` |
| `0x00470A6C` | `title-screen-roi-exact-00470a6c` | object `0x367`, target `0x369` |
| `0x00470E10` | `title-screen-roi-exact-00470e10` | caller bytes/relocations differ |

Recheck one without accepting it automatically:

```bash
python3 scripts/analysis/verify-exact-units.py \
  --unit UNIT --include-unaccepted --json
```

Recover these only as bounded future authored tasks. They do not block the
separate target-linked library inventory/provenance work below.

## Next milestone: target-linked libraries

The `library` inventory is code inside the original executable, not
`3rdparty/Detours`. Start with inventory integrity and provenance, not source
stubs or inline assembly.

Seven library rows do not yet have mapping extents:

```bash
python3 scripts/analysis/report-reconstruction-status.py \
  --category library --state missing-size --sort address
```

The current output is:

```text
0x0048D3D0 D3DXVec3Normalize_rsqrtss_0048d3d0
0x0048D4A0 D3DXVec4Normalize3_rsqrtss_0048d4a0
0x0048DA50 D3DXVec4Normalize_rsqrtss_0048da50
0x0048E680 D3DXVec4Normalize_rsqrtss_0048e680
0x0048EFB0 D3DXVec3Normalize_rsqrtss_48efb0
0x0048F080 D3DXVec4Normalize3_rsqrtss_0048f080
0x004A43CF operator delete
```

For the first bounded library batch:

1. verify target boundaries and callers for one coherent family;
2. determine the originating archive/member and record its SHA-256 provenance;
3. compare extracted VC7 COFF code with relocation-aware target bytes;
4. propose a library-specific ledger/progress schema before accepting results;
5. keep authored `implemented.csv`, `matches.csv`, and authored percentages
   unchanged.

There is intentionally no library scanner yet. Building one before archives,
hashes, boundary policy, and relocation replay are pinned would automate an
unverified assumption.

## Later whole-executable lane

After target-linked library objects are reproducible, audit linker order and
padding, globals and static initialization, imports, PE headers, resources, and
non-code data. The normal executable currently proves that the reconstructed
source links; it is not a claim that the complete PE matches.

Use `docs/TOOLS.md` to choose commands. Keep `.analysis/` disposable and update
this file whenever the active milestone or a durable blocker changes.
