# Current reconstruction handoff

This file records only the current durable state. Historical investigations
belong in focused notes such as `RUNECL_FUNCTION_EXACT_NOTES.md`; live counts
come from the ledgers, not this prose.

## Current status

As cold-built and replayed on 2026-08-20 against the original Japanese TH08
1.00d target:

- authored source: **1,107 / 1,107 functions**, **459,757 / 459,757 bytes**;
- strict authored exact: **1,105 / 1,107 functions**, **459,115 / 459,757 bytes**;
- library inventory: **1,119 classified functions**; all **1,119 / 1,119** now have
  mapping sizes totaling **217,165 bytes**; the independent library exact ledger currently accepts **258 functions / 52,707 body bytes**;
- `config/claims.csv` is header-only;
- a cold normal VC7 build links `build/th08.exe` successfully;
- a cold objdiff build followed by full replay passes **1,105 / 1,105**
  accepted units;
- five production-object layout repairs are complete. Seven exact AsciiManager
  bodies now live in four target-cluster TUs, while the early Player bomb/shot
  callback family now lives in `PlayerBomb.cpp`; both remaining donor objects
  have zero target-order inversions. The contiguous main Player TU is also back
  in target function order. Fourteen shared helpers formerly appended to
  `main.cpp` now emit naturally from their target-neighbor consumer TUs. The
  retained AnmManager render cluster is also back in target function order.
  The continuous Global main region and its target-neighbor math helpers are
  now in target order as well;
- the whole-image comparator is in place. After the first evidence-backed link
  contract repairs, the rebuild has the target's DLL descriptor order, no debug
  directory, and the same two resource paths and resource section extent. The
  placeholder icon payload intentionally remains different;
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

Two authored functions remain unaccepted. Do not count them as exact or
restart broad brute-force matrices without a new target-backed hypothesis.

- `ReplayManager::OnUpdateHighPrioDemo2 @ 0x004526C0`, 361 bytes: the natural
  source object is one byte longer and the residual is a register-allocation
  phase difference. Its adjacent callback family is exact.  The ordinary
  pointer/cast/increment/local-order variants are exhausted and recorded in
  `BUILD_MATCHING.md`; resume only from a new allocator/TU hypothesis.
- `TitleScreen::RegisterChain @ 0x0047146D`, 281 bytes: target and object have
  the same extent; five stack-displacement bytes differ because the target
  frame is `0x40` while the current natural object frame is `0x5C`.  The
  constructor itself remains exact with frame `0x4C`; declaration order, PCH,
  inline-depth, default-init, factory, optimizer-preset, and exception-spec
  probes did not reproduce the target caller contract.

The source for both is present and behaviorally reconstructed. Exact coverage
will increase only when the canonical comparator returns `exact` for a natural,
evidence-backed C++ form.

The other fourteen cold regressions have been repaired and accepted. Four
TitleScreen functions required production-source restoration. Seven other
functions were already byte-exact but had stale relocation identities in
`match-units.toml`. `Supervisor::IsFogDisabled` required restoring its original
header-inline definition: VC7 then emits the target `0x1A` COMDAT in
`main.obj`, while the calling `AsciiManager` function remains exact. The
`AnmVm::SetZRotation` and `GameManager::CollectExtend` then required bounded
production-source owner/value-flow fixes. The 2026-08-20 single-job cold replay
verifies all **1,105 / 1,105** accepted units.
See `BUILD_MATCHING.md` before changing a shared inline/out-of-line contract.

## Library foundation (paused except for whole-link blockers)

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

Library acceptance now has **258 / 258 configured units** and **52,707 body bytes**
after canonical zero-difference replay.  The accepted set contains six SSE/SSE2
D3DX normalize helpers plus seven exact `x3d_quat.obj` quaternion helpers (including the 0xA59-byte `x3d_D3DXQuaternionSquadSetup` with 66 replayed relocations) and six exact `x3d_matx.obj` matrix helpers (including 0x1198/0x1154-byte transformation implementations), VC7 `/MT` `operator delete`, seven CRT string/memory
helpers (including strict shared-section `strchr`/`strcpy`/`strcat`), six
isolated compiler-runtime arithmetic helpers (`_ftol2`, `_chkstk`, `_aulldiv`,
`_allmul`, `_ftol`, `_aullshr`), and the `LIBCMT malloc.obj` allocation wrappers
`_nh_malloc @ 0x004A423D` / `malloc @ 0x004A4269` with explicit DIR32/REL32 replay, plus the complete currently mapped `heapinit.obj`/`sbheap.obj` core accepted so far: `_heap_init`, `__sbh_heap_init`, `__sbh_find_block`, `__sbh_free_block`, `__sbh_alloc_new_region`, `__sbh_alloc_new_group`, the 735-byte `__sbh_resize_block`, and the 764-byte `__sbh_alloc_block`.  The next accepted family also covers eight `trnsctrl.obj` C++ EH helpers, including `_UnwindNestedFrames`, `__CxxFrameHandler`, `_GetRangeOfTrysToCheck`, `_CreateFrameInfo`, `IsExceptionObjectToBeDestroyed`, `_FindAndUnlinkFrame`, `_CallCatchBlock2`, and `_CallSETranslator`.  Shared-section `exsup.obj` coverage now also accepts `_global_unwind2`, `_local_unwind2`, `_abnormal_termination`, `_NLG_Notify1`, `_NLG_Notify`, plus the isolated `__EH_prolog`.  The EH dependency chain now also accepts `_getptd`, `__InternalCxxFrameHandler`, `_CallSettingFrame@12`, and `_inconsistency`; the latter exposed and fixed a stale 0x26 mapping extent, with `hooks.obj` and target control flow proving the full 0x2D body.  `__FrameUnwindToState` is accepted with a 0xAD main-body extent and a 0xCE comparison extent so its compiler-owned cleanup alternate-entry/funclet bytes are replayed without double-counting the separately mapped `0x004AA267` funclet.  Auxless whole-section COMDAT support is now explicit/fail-closed, and `FindHandler @ 0x004AA72E` is accepted as a 0x1E2-byte static code COMDAT with a unique offset-zero function owner and 13 replayed REL32 relocations.  `tidtable.obj` thread-runtime coverage now also accepts `_mtterm @ 0x004AA049` and `_mtinit @ 0x004AA0D8` with their TLS/lock/calloc relocation graph replayed explicitly.  `mlock.obj` coverage now accepts `_mtinitlocks`, `_mtdeletelocks`, `_unlock`, `_mtinitlocknum`, and `_lock`, including the dynamic critical-section table and lazy lock initialization paths.  Its immediate dependencies now also accept `_errno`, `_amsg_exit`, and `__crtInitCritSecAndSpinCount`; the latter two required target-proven extent repairs from 0x22 to 0x25 and 0x67 to 0x8B respectively.  The message/critical-section leaf layer now additionally accepts `__crtInitCritSecNoSpinCount@8`, `_FF_MSGBANNER`, and `_NMSG_WRITE`; the 0x177-byte message writer replays 30 relocations including CRT string helpers, Win32 I/O, and the security-cookie check.  Its immediate code dependencies now also accept `strncpy`, `__crtMessageBoxA`, and the 14-byte `seccook.obj` security-check-cookie helper at `0x004AB43C`; that last row remains conservatively named `FUN_004ab43c` until a repository-wide fastcall-decoration naming rule is fixed.  `_report_failure` is now accepted with its full 0x31-byte auxless COMDAT extent, and `_calloc` with its full 0xBB-byte VC7 function-definition extent; `calloc` deliberately retains the legal nested cleanup-funclet overlap rather than truncating the parent epilogue.  `trnsctrl.obj` also exposed a missing `CatchGuardHandler @ 0x004A44A1` inventory row and an undersized `TranslatorGuardHandler @ 0x004A44C5`; both auxless COMDATs are now accepted at 0x24/0x71 bytes.  `frame.obj` coverage now also accepts `TypeMatch`, the repaired 0x45-byte `__DestructExceptionObject`, `CatchIt`, and `FindHandlerForForeignException`.  `_JumpToContinuation` now replays a 0x30-byte COFF section while counting its 0x2B main body, and the linked 0x004A4412 helper is accepted through the existing `CallMemberFunction1` row; archive Function0/1/2 symbols are identical 7-byte COMDAT aliases of that linked copy.  `CallCatchBlock` and `BuildCatchObject` are now accepted with 0x97/0x170 body extents while replaying their complete 0x1A7/0x17C auxless COMDAT sections, which include separately mapped EH funclets/tails.  Their direct support layer now also accepts `AdjustPointer`, `ValidateRead`, corrected `ValidateWrite`, `ValidateExecute`, and the repaired full 0x33D-byte `memmove`; `FUN_004AA427` is now accepted through the explicit tail-local-funclet schema, pinned to `$L19653` and its `CallCatchBlock` parent section rather than forced through the normal function schema.  The same schema now accepts `FUN_004AA267` via `$L19568` at the tail of `__FrameUnwindToState`; `terminate @ 0x004AA9B2` is also repaired from 0x2E to the full 0x35-byte `hooks.obj` function and accepted with two `__getptd` calls plus the final `_abort @ 0x004B05BD` tail jump.  The locale/thread pair now accepts `__updatetlocinfo_lk @ 0x004A9F4C`, `__updatetlocinfo @ 0x004AA00E` with a 0x32 body/0x3B comparison extent, and its 9-byte `FUN_004AA040` tail-local cleanup; `_abort @ 0x004B05BD` is also accepted as a 0x17 body with the final `int3` included only in its 0x18-byte comparison extent.  This chain now continues through `__freetlocinfo @ 0x004A9E82`; `_abort` dependencies `_exit @ 0x004A6B6A` and `_raise @ 0x004B17F2` are accepted as well, with `_raise` repaired from 0x164 to its full 0x179-byte parent extent while retaining the legal nested 0x004B192D cleanup funclet.  The locale teardown chain now also accepts `__free_lc_time @ 0x004AFA38`, `__free_lconv_num @ 0x004AFBC8`, and `__free_lconv_mon @ 0x004AFC27`, closing the direct heap-release dependencies of `__freetlocinfo`.  Signal/ctype coverage accepts static `siglookup @ 0x004B17C4` and the `isdigit/isxdigit/isspace` leaf family.  Do **not** accept `signal @ 0x004B196B`: `winsig.obj::_signal` has a 0x1A9-byte function-definition section, but replay at that target start has hundreds of non-relocation byte differences; the current row remains an unresolved inventory/provenance item rather than an exact claim.  Time/locale formatting coverage accepts `_Strftime_mt`, `_Strftime`, `strftime`, `localtime`, and `time`; the archive-proven VC7 symbol corrects the imported `_time32` mapping name to `_time`. Formatting/small-CRT coverage also accepts `vsprintf`, `sprintf`, `sscanf`, `atol`, `rand`, and `purecall` from their pinned `LIBCMT` members; the stdio wrappers replay their `__output`/`__flsbuf` or `strlen`/`__input` relocations explicitly rather than masking them. Time-runtime dependencies now also accept `__tzset`, `_isindst`, and `gmtime`; termination coverage accepts `_onexit_lk`, `_onexit`, its separately mapped 6-byte tail cleanup funclet, and `atexit`. The internal timezone lane now also accepts `_tzset_lk` and `_isindst_lk` as auxless static `tzset.obj` COMDATs, replaying 58 and 30 relocations respectively; `_isindst_lk` exposes `_cvtdate @ 0x004AADC8` as the next direct dependency. That dependency is now accepted as the full 0x1B8-byte static `_cvtdate` COMDAT; `_tzset_lk` also closes through exact `_getenv_lk @ 0x004B05D5`. The environment conversion chain now continues through exact `_mbsnbicoll @ 0x004B1FC8` and archive-proven `__wtomb_environ @ 0x004B2016`, each with four explicit relocations.  String/heap support now accepts the repaired 0x88-byte `strcmp` plus `_msize` as a 0x6A main body with a 0x76 comparison extent and a separately accepted 9-byte cleanup tail. Startup/termination coverage now also accepts `__crtExitProcess`, `_lockexit`, `_unlockexit`, `_cinit`, `doexit`, public `exit`, `_cexit`, and the previously missing `_c_exit @ 0x004A6B8A`; `__crtExitProcess` counts 0x2F body bytes while replaying the final `int3` in its 0x30-byte COFF extent. `initsect.obj` coverage now repairs `_RTC_Initialize @ 0x004ACBFE` from 0x3D to 0x44 and adds the previously missing `_RTC_Terminate @ 0x004ACC42`, both exact 0x44-byte SEH iterator functions. File-I/O wrapper coverage now repairs `_lseek`, `_write`, and `_read` from imported 0xA0 extents to their full 0xAB VC7 function-definition extents.  Each parent still contains a separately mapped nested cleanup funclet, but control flow resumes in the parent afterward; the overlap exceptions remain valid and the child does not define the parent boundary. Stdio support also accepts `write_char`, `write_multi_char`, `write_string`, `input::__inc`, and archive-proven `_ungetc_lk`; the latter replaces the imported `FID_conflict:_ungetc` label after exact `ungetc.obj` replay. File-handle support now also accepts `_lseek_lk`, `_write_lk`, `_read_lk`, `_lock_fhandle`, and `_unlock_fhandle`, closing the direct lock/worker dependencies of the repaired `_lseek/_write/_read` wrappers. Error/handle support now also accepts `_doserrno`, `_dosmaperr`, and `_get_osfhandle`, replacing the anonymous doserrno row with its archive-proven identity. Stdio/MBC conversion coverage now also accepts `_getbuf`, `_filbuf`, `wctomb`, `mbtowc`, `__wctomb_mt`, and `__mbtowc_mt`, closing the direct buffer and multibyte conversion dependencies of the accepted `output`/`input` code. File/stdio lock internals now also accept `_free_osfhnd`, `_lseeki64_lk`, `_flush`, `_fflush_lk`, `_fclose_lk`, and `_close_lk`, providing archive-proven worker identities for the remaining public wrapper lane. `_isatty` is also accepted from `isatty.obj`, closing the simple terminal-test branch used by formatted and file output paths. Public-wrapper coverage now adds the previously missing `fclose @ 0x004B2609` and repairs `_commit @ 0x004B265A` / `_close @ 0x004B2E0D` to their full 0xBC / 0x9B VC7 extents while preserving their nested cleanup overlaps. `fflush.obj` inventory repair now also adds static `flsall @ 0x004B1C69` (0xD5) and `_flushall @ 0x004B1D3E` (0x9), which exactly fill the unmapped gap after `_fflush_lk` and before `___ansicp`. X87 `common.obj` coverage now accepts nine bounded functions in one shared 0x18C code section, including the previously missing `_check_overflow_exit @ 0x004A7F35`; each match pins its section offset instead of pretending the archive member contains nine independent sections.  Continue pinning provenance/match units for
remaining VC7 CRT/runtime, standard-library, D3DX, and compiler-runtime inventory
only when a bounded whole-image difference identifies the member as a dependency.
Keep library progress separate from authored totals; do not resume broad archive
scanning merely to increase its percentage.

Keep authored `implemented.csv`, `matches.csv`, and authored percentages
unchanged.  There is intentionally no whole-library scanner until archive
identity, relocation policy, COMDAT/padding rules, and failure modes are encoded.

## Current milestone: whole-executable reconstruction

The normal executable links, so whole-image differences now select the work.
Run the canonical cold baseline with:

```bash
python3 scripts/build.py --fresh
python3 scripts/compare-whole-image.py --json \
  > build/whole-image-report.json
```

The 2026-08-20 baseline after fixing the first observable link contracts is:

- file size: target **840,704**, rebuild **823,296** bytes;
- entry point: target `0x004A619E`, rebuild `0x0049A43B`;
- `.text` virtual/raw: target `0xB1B78 / 0xB1C00`, rebuild
  `0xAA26F / 0xAA400`;
- `.rdata` virtual/raw: target `0x11284 / 0x11400`, rebuild
  `0x15164 / 0x15200`;
- `.data1` raw bytes and extent are exact; `.data` and the rest of the image are
  not exact;
- `.rsrc` virtual/raw extents are both `0xD60 / 0xE00`, and its two paths and
  group-icon leaf agree. The 3,240-byte icon leaf retains repository-owned
  placeholder artwork, so its payload is intentionally different;
- both images have no debug-directory entries;
- DLL descriptor order agrees exactly. The target imports 155 symbols and the
  rebuild imports 172: there are no target-only imports and 17 rebuild-only
  `KERNEL32` symbols. Nine are referenced by D3DX `cd3dxfile.obj` /
  `cd3dxresource.obj`; the remaining set is referenced by six linked VC7 CRT
  members recorded in `KNOWLEDGE_BASE.md`;
- 944 accepted address anchors are found in the linker map and 68 accepted units
  lack a unique map candidate. Anchor-derived current-object order differs from
  target order, and large drift spans occur within production objects.

The first bounded target translation-unit/object-partition recovery is now
complete. The old `AsciiManager.obj` placed seven exact functions totaling
`0x220` bytes between `InitializeVms` and `RegisterChain`, although the target
places them in four distant clusters: `0x00406FD0`, `0x00422BB0`,
`0x0042F2D0`, and `0x004398FF`. Their definitions now live in
`AsciiManagerGauge.cpp`, `AsciiManagerBossMarker.cpp`,
`AsciiManagerScale.cpp`, and `AsciiManagerGuiMode.cpp`, compiled under the same
`/Od` PCH profile. Canonical focused replay passed all **45 / 45** accepted
units across the donor and recipients; the subsequent full cold replay passed
**1,105 / 1,105**.

This split reduced `AsciiManager.obj` from **189 inversions / 3 target-order
runs / 225,802 bytes of drift span** to **0 inversions / 1 run / 2,800 bytes of
drift span**. Its first 21 anchors, through `PauseMenu::OnUpdate @ 0x004037B0`,
now have zero linked-address drift. The extracted clusters each have uniform
internal drift; their final placement depends on later splits in neighboring
GameManager, EnemyManager, and Gui ownership and is not yet exact.
The executable layout proves that these bodies cannot remain in the early
AsciiManager object; it does not yet prove that all four clusters correspond
one-for-one to standalone original source files.

The second bounded investigation separated a genuine Player TU boundary from
ordinary lexical disorder. The callback family at
`0x0040BC20..0x004142C0` was merged into the same source file as the main
Player region at `0x00449CA0..0x00451D50`, despite the large target gap and
many intervening subsystems. Its real definitions now live in
`PlayerBomb.cpp`, under the same `/Od` PCH profile, and all **56** accepted
early-range units have canonical ownership in `PlayerBomb.obj`. The main
target neighborhood is otherwise substantially continuous, so its remaining
seven runs were definition-order drift rather than evidence for six more
standalone TUs. `Player.cpp` is now ordered by its mapped target functions;
two forward declarations preserve natural C++ visibility without adding
definitions or shims.

Focused donor/recipient replay passed all **116 / 116** accepted units. The
subsequent full single-job cold replay passed **1,105 / 1,105**. The original
merged `Player.obj` had **286 inversions / 8 runs / 275,088 bytes of drift
span**. The repaired `Player.obj` has **0 / 1 / 7,760**, and
`PlayerBomb.obj` has **0 / 1 / 544**. The name `PlayerBomb.cpp` is a
repository description of the proven callback cluster, not a claim about the
original filename.

Continue target translation-unit recovery one production object at a time.
Generate and rank the detailed anchors rather than manually scanning the whole
report:

```bash
python3 scripts/compare-whole-image.py --json --include-anchor-details \
  > build/whole-image-anchors.json
python3 scripts/analysis/report-tu-partition-candidates.py \
  build/whole-image-anchors.json
```

The third bounded pass repaired a different ownership error rather than
inventing another source-file split. Fourteen small `GameManager`/`ZunTimer`
helpers had explicit definitions appended to `main.cpp`, even though target
neighborhoods and production undefined references place twelve with the early
`AsciiManager` consumer cluster, `ZunTimer::operator--(int) @ 0x00418110`
beside `SpellCard`, and `GameManager::SetClockTime @ 0x00453C60` beside replay
loading. Restoring their natural header-inline bodies makes VC7 emit exact
COMDATs from `AsciiManager.obj`, `SpellCard.obj`, and `ReplayManager.obj` under
the actual consumer profiles. Focused donor/recipient replay passed **133 /
133** accepted units, and the subsequent full single-job cold replay passed
**1,105 / 1,105**.

This reduced `main.obj` from **277 inversions / 5 runs / 316,960 bytes of drift
span** to **16 / 2 / 146,336**. All 25 anchors in the retained main region are
continuous; the remaining reset is solely the exact but currently uncalled
`ZunTimer::operator+= @ 0x0041FDF0`. It remains explicitly defined in
`main.cpp` until a target-backed production consumer or owner is recovered;
do not assign it to an unrelated ECL/SpellCard TU just to improve the metric.
The current Ascii helper cluster retains a bounded lexical/emission-order issue
(**50 inversions / 6 runs / 9,600 span**) but no exact-code regression. The
increase from the earlier measurement is the natural canonical Ascii emission
of `Float3::Float3(float,float,float) @ 0x00404720`, whose target address is
beside `PauseMenu::OnDraw`; it is not an exact-code regression.

The fourth bounded pass addressed `AnmManager.obj`. Seven exact helper bodies
were mixed into the donor despite target-neighbor production consumers:
`AnmVmBase::Initialize` and `SetTextureCaptureParams` route to the early
Ascii cluster, `SetZRotation` to `PlayerBomb`, `FUN_004396f8` to `Gui`, and
`SetCameraMode`/`Draw2DAndFlush`/`GetAnm` to `Background`. Six can remain
natural consumer-emitted header bodies. `Initialize` must instead be an
explicit `/Od` definition in `AsciiManager.cpp`: exposing its body globally
changed five accepted `/Os /Ob1` Title functions during cold replay. The
expanded seven-object focused replay passed **294 / 294** accepted units and
the final full cold replay passed **1,105 / 1,105**.

The retained `0x00462270..0x00464EC0` Anm render cluster was continuous, so its
definitions were reordered by target function address instead of split into
invented TUs. `SpriteHasTexture @ 0x004622C0` deliberately remains
header-inline: changing it to out-of-line made the Anm metric perfect but
broke exact `TitleScreen::OnDraw @ 0x0047087F`, whose target contains the
inlined body and no call relocation. The accepted result reduces
`AnmManager.obj` from **195 inversions / 10 runs / 400,208 bytes of drift
span** to **20 / 2 / 359,840**. All 20 residual inversions are that required
deferred COMDAT placement; the main/render definitions themselves are in
target order.

The fifth bounded pass repaired `Global.obj`. Six exact shared math bodies were
appended between a substantially continuous Global main region and its final
global helpers. Target neighborhoods plus production undefined references
place `Float3::operator+`, `operator-`, `operator*`, and `operator/=` in
`Background.cpp`, and `VectorAngle` plus `Float3::operator/` in
`PlayerBomb.cpp`. Their explicit definitions now sit at the corresponding
target-local lexical positions without making the bodies globally visible.
`Chain::RunDrawChain` and `Chain::ReleaseSingleChain` remain in the continuous
Global TU but are lexically ordered as in the target.

Moving the six callers also stopped `Global.obj` from implicitly emitting
`Float3::Float3(float,float,float) @ 0x00404720`. The target constructor is
adjacent to `PauseMenu::OnDraw`, and `AsciiManager.obj` naturally emits the
same exact COMDAT, so its canonical match-unit owner now records that
section-defined production copy. Focused donor/recipient replay passed **191 /
191**, and the subsequent full single-job cold replay passed **1,105 /
1,105**. `Global.obj` improved from **194 inversions / 4 runs / 220,816 bytes
of drift span** to **0 / 1 / 96**.

The current ranking now places `SoundPlayer.obj` first (**181 inversions / 4
runs / 358,864 span**), followed by `GameManager.obj` (**174 / 9 / 291,648**)
and `Gui.obj` (**167 / 12 / 23,133**); select only one and confirm the target
neighborhood before moving definitions.
Large intra-object drift means today's source combines or orders target TUs
differently, so permuting the existing object list alone cannot solve it. A
run reset is a routing clue, not automatically a TU boundary: the Player pass
showed that one investigation may require both a real split and target-order
restoration inside the retained TU. The 17
extra imports already have bounded contributing archive members; do not hide
them with `/OPT:REF`. A trial of that flag made the import set exact while
shrinking `.text` to `0x72B6F` and losing 212 located accepted anchors, so
`/OPT:NOREF` remains required by stronger whole-image evidence. Return to
CRT/D3DX exact recovery only if a subsequent diff identifies a specific member
operation. Do not copy the target icon payload and do not claim whole-image
exactness from successful linking or same-size sections.

Use `docs/TOOLS.md` to choose commands. Keep `.analysis/` disposable and update
this file whenever the active milestone or a durable blocker changes.
