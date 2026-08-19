# Current reconstruction handoff

This file records only the current durable state. Historical investigations
belong in focused notes such as `RUNECL_FUNCTION_EXACT_NOTES.md`; live counts
come from the ledgers, not this prose.

## Current status

As cold-built and replayed on 2026-08-19 against the original Japanese TH08
1.00d target:

- authored source: **1,107 / 1,107 functions**, **459,757 / 459,757 bytes**;
- strict authored exact: **1,091 / 1,107 functions**, **445,728 / 459,757 bytes**;
- library inventory: **1,112 classified functions**; all **1,112 / 1,112** now have
  mapping sizes totaling **216,288 bytes**; the independent library exact ledger currently accepts **57 functions / 7,754 body bytes**;
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

The seven previously missing library extents are repaired.  The canonical
selector now reports no missing-size rows:

```bash
python3 scripts/analysis/report-reconstruction-status.py \
  --category library --state missing-size --sort address
```

The six D3DX rows were reconciled against the target CPU dispatch tables and
the SHA-pinned VC7 PlatformSDK prerelease `D3DX8.LIB`
(`0d4a2b642485dcaa7671926a9a1a545c656d5eb73f160fe971b3deebf0b516b5`).
Their archive identities are SSE/SSE2 Vec3, Plane, Vec4, and Quaternion
normalize members; target body extents stop before archive alignment padding.
`operator delete @ 0x004A43CF` is the five-byte VC7 `/MT` `LIBCMT.LIB`
`delete.obj` tail jump to `_free`.  These boundary/provenance facts now feed the separate accepted library ledger; they are not authored claims.

Inventory integrity is now reconciled.  Four imported extents were stale and
were shortened at target-proven return/next-function boundaries (`0x0049E3CD`,
`0x004A51FD`, `_strchr @ 0x004A5BE0`, and `0x004A6890`).  The remaining nine
overlaps are VC7 CRT nested SEH cleanup funclets and are explicitly represented
in `config/mapping-overlaps.csv`; `validate-tracking.py` rejects stale exception
rows and reports any new unclassified overlap.

Separate library acceptance infrastructure is now in place:

- `library-provenance.toml` pins archive identity and SHA-256;
- `library-match-units.toml` records target body/compare extents, archive member,
  COFF symbol, and every relocation target base;
- `compare-library.py` verifies archive hash/member identity and replays DIR32/REL32
  relocations before comparing the complete target range;
- `library-matches.csv` is the independent accepted ledger;
- `validate-library.py` is the public CI gate, with `--require-archives` for local
  hash attestation; `library-progress.py` generates `LIBRARY_PROGRESS.md`.

Library acceptance now has **57 / 57 configured units** and **7,754 body bytes**
after canonical zero-difference replay.  The accepted set contains six SSE/SSE2
D3DX normalize helpers, VC7 `/MT` `operator delete`, seven CRT string/memory
helpers (including strict shared-section `strchr`/`strcpy`/`strcat`), six
isolated compiler-runtime arithmetic helpers (`_ftol2`, `_chkstk`, `_aulldiv`,
`_allmul`, `_ftol`, `_aullshr`), and the `LIBCMT malloc.obj` allocation wrappers
`_nh_malloc @ 0x004A423D` / `malloc @ 0x004A4269` with explicit DIR32/REL32 replay, plus the complete currently mapped `heapinit.obj`/`sbheap.obj` core accepted so far: `_heap_init`, `__sbh_heap_init`, `__sbh_find_block`, `__sbh_free_block`, `__sbh_alloc_new_region`, `__sbh_alloc_new_group`, the 735-byte `__sbh_resize_block`, and the 764-byte `__sbh_alloc_block`.  The next accepted family also covers eight `trnsctrl.obj` C++ EH helpers, including `_UnwindNestedFrames`, `__CxxFrameHandler`, `_GetRangeOfTrysToCheck`, `_CreateFrameInfo`, `IsExceptionObjectToBeDestroyed`, `_FindAndUnlinkFrame`, `_CallCatchBlock2`, and `_CallSETranslator`.  Shared-section `exsup.obj` coverage now also accepts `_global_unwind2`, `_local_unwind2`, `_abnormal_termination`, `_NLG_Notify1`, `_NLG_Notify`, plus the isolated `__EH_prolog`.  The EH dependency chain now also accepts `_getptd`, `__InternalCxxFrameHandler`, `_CallSettingFrame@12`, and `_inconsistency`; the latter exposed and fixed a stale 0x26 mapping extent, with `hooks.obj` and target control flow proving the full 0x2D body.  `__FrameUnwindToState` is accepted with a 0xAD main-body extent and a 0xCE comparison extent so its compiler-owned cleanup alternate-entry/funclet bytes are replayed without double-counting the separately mapped `0x004AA267` funclet.  Auxless whole-section COMDAT support is now explicit/fail-closed, and `FindHandler @ 0x004AA72E` is accepted as a 0x1E2-byte static code COMDAT with a unique offset-zero function owner and 13 replayed REL32 relocations.  `tidtable.obj` thread-runtime coverage now also accepts `_mtterm @ 0x004AA049` and `_mtinit @ 0x004AA0D8` with their TLS/lock/calloc relocation graph replayed explicitly.  `mlock.obj` coverage now accepts `_mtinitlocks`, `_mtdeletelocks`, `_unlock`, `_mtinitlocknum`, and `_lock`, including the dynamic critical-section table and lazy lock initialization paths.  Continue pinning provenance/match units for
remaining VC7 CRT/runtime, standard-library, D3DX, and compiler-runtime inventory;
keep library progress separate from authored totals.

Keep authored `implemented.csv`, `matches.csv`, and authored percentages
unchanged.  There is intentionally no whole-library scanner until archive
identity, relocation policy, COMDAT/padding rules, and failure modes are encoded.

## Later whole-executable lane

After target-linked library objects are reproducible, audit linker order and
padding, globals and static initialization, imports, PE headers, resources, and
non-code data. The normal executable currently proves that the reconstructed
source links; it is not a claim that the complete PE matches.

Use `docs/TOOLS.md` to choose commands. Keep `.analysis/` disposable and update
this file whenever the active milestone or a durable blocker changes.
