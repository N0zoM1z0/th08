# Current reconstruction handoff

This file records only the current durable state. Historical investigations
belong in focused notes such as `RUNECL_FUNCTION_EXACT_NOTES.md`; live counts
come from the ledgers, not this prose.

## Active semantic reconstruction branch

`semantic/typed-reconstruction` starts from `main@4cffb2a` and makes semantic
source recovery the active bounded lane.  The bootstrap scope changes no game
layout, function mapping, authored ledger, or accepted exact unit.  It adds the
evidence/acceptance guide in `docs/SEMANTIC_RECONSTRUCTION.md`, the
`$th08-semantic` workflow, and the read-only
`scripts/analysis/report-semantic-debt.py` candidate router.

The verified starting baseline is authored **1,107 / 1,107** and strict exact
**1,105 / 1,107**; `config/claims.csv` is header-only.  Work one structure and
one coherent field family at a time.  VC7 focused/aggregate replay remains the
exact-code oracle, while the modern build and relevant Linux layout/runtime
checks remain the portable behavior oracle.  Do not publish candidate counts as
semantic progress.

The expensive portable Linux package workflow is temporarily manual-only on
this branch to accelerate push/PR iteration.  Local Linux build, layout, and
runtime commands remain available and must still be used when a semantic batch
touches their acceptance surface.  Restore the workflow's automatic
`push`/`pull_request` triggers before merging this phase into a release lane.

The first accepted source batch is the `ScreenEffect +0x18/+0x1C/+0x20`
variant-parameter family.  A TU-local typed overlay records fade color, shake
amplitude, pulse repeat/color, and three-phase shake-envelope views while
preserving both the generic `RegisterChain` initialization ABI and the shared
header byte-for-byte.  Focused VC7 replay passed `ScreenEffect.obj` **16 / 16**
and its canonical objdiff path **5 / 5**; the normal VC7 image linked, the
complete i386 container build linked, and the fixed-layout verifier passed.

The required cold aggregate diagnostic initially exposed a reproducible
branch-baseline blocker at **1,017 / 1,105**.  The follow-up bounded maintenance
batch has now closed it.  It restored target-observed direct-global and source-
expression shapes where the portable-owner pass had changed VC7 extent/codegen,
introduced target/modern storage bridges where the two linkers intentionally
differ, and migrated only relocation entries whose masked function bytes were
otherwise identical.  All 79 final relocation-only failures had zero
non-relocation byte differences; all 338 changed/new relocation entries inferred
the existing target bases without conflict and had mapping/global-ledger
evidence.

The current attested result is a non-reuse cold build of all 75 configured
comparison objects followed by **1,105 / 1,105 exact** accepted units with no
failures.  A normal VC7 production link passes.  The complete i386 Linux
container build and fixed-layout verifier also pass.  This closes the aggregate
blocker without changing authored progress or weakening the comparator; see the
post-port reconciliation entry in `docs/SEMANTIC_RECONSTRUCTION.md` for the
evidence split and reusable compatibility pattern.

The next accepted source batch recovers the Player deathbomb/Bomb lifecycle.
It replaces the raw `Player + 0xFDC..+0x1014` active/type/duration/timer and
calculation/draw callback views, plus the `+0xE2A68/+0xE2A6C/+0xE2B28`
deathbomb window, Bomb-input lock, and effect pointer, with asserted typed
fields.  TH08 target users cover `Die`, the Bomb update/draw paths, the dying
transition, initialization, shot/collision gates, and `Spellcard::EndSpell`;
TH06 only corroborates the stable active/duration/timer/calc/draw roles.  The
individual callback-slot meanings and retained opaque fields remain unknown.
Focused replay passed **249 / 249**, a new non-reuse cold replay passed **1,105
/ 1,105**, the normal VC7 link passed, and the complete i386 Linux build/layout
verifier passed.  No match manifest, authored/exact ledger, global identity, or
runtime state operation changed.

Select the next independent field family with:

```bash
python3 scripts/analysis/report-semantic-debt.py
```

Then prefer a small exact-backed field family over the largest anonymous
manager.  Whole-executable TU/layout work below is deferred, not invalidated.

## Active playable-port branch

`port/modern-windows-linux` is the independent playable reconstruction lane.
It does not replace the VC7 exact build or change authored/library ledgers.

Playable-port state on 2026-08-24:

- CMake compiles and links the complete production-authored source set with the
  32-bit MinGW toolchain into `build/modern-windows/th08-modern.exe`;
- the output is an i386 Windows GUI PE and has no MinGW support-DLL dependency,
  but the user reports that the current native BAT/startup path does not launch
  a usable application. Prior smoke observations are insufficient to call the
  Windows product verified; Windows remains in progress;
- `--data-dir <directory>` is the intended Windows interface for a Unicode
  directory containing `th08.dat` and `thbgm.dat`, but it must be revalidated
  as part of the native startup fix;
- the playable lane restores target-proven global ownership for the active
  `GameManager` state and playfield bounds, uses relocatable function symbols
  for Player option/shot/bomb callback tables, and connects the complete
  `EnemyManager::OnUpdate` implementation to the calculation chain;
- the MinGW bring-up build temporarily uses the non-redistributable SDK
  `d3dx8d.dll`. A distributable Windows build must replace the remaining D3DX
  calls;
- CMake also builds `th08-modern` as a native i386 ELF using repository-owned
  SDL2/OpenGL compatibility backends. `scripts/build-modern-linux.sh` is the
  default host multilib entry; the i386 container is optional CI/build
  isolation rather than a runtime dependency;
- native i386 ELF execution under WSLg has completed a full Lunatic story
  endurance pass, with player/enemy/bullet simulation, score awards, Boss
  bars, Bomb animations, focus marker, Stage 2 fog, Japanese dialogue,
  keyboard input, sound effects, streamed WAV BGM, and the ending transition
  active. The endurance run used an external GDB command that suppressed life
  decrement only; that test aid is ignored and never shipped. No fatal signal
  occurred. A real Linux desktop with hardware OpenGL remains additional
  release coverage;
- `th08-layout.ld` now aliases target-owned manager, callback-chain, and known
  aggregate-field views to their original addresses. This prevents duplicate
  spell/ECL callbacks across reloads, which had manifested as negative spell
  bonuses, saturated scores, missing scripted visuals, and later-stage exits;
- the OpenGL backend uses a texture-backed FBO plus a stable dialogue-entry
  snapshot, explicit eye-space linear fog, and independent RGB/alpha
  texture-stage combiners. A Stage 4-to-5 transition can still tile the dynamic
  player-name texture across the frame/HUD; it is documented as a deferred
  Linux renderer issue rather than a release blocker. Fatal Linux signals
  write a symbolizable `modern-crash.txt`; MIDI remains a follow-up;
- `scripts/setup-modern-linux.sh <data-directory>` is the one-command
  Debian/Ubuntu setup/build/run entry, while `scripts/play-modern-linux.sh`
  performs the normal incremental build and launch. Both accept only the
  original data directory and require `th08.dat` plus `thbgm.dat`;
- `.github/workflows/portable-linux.yml` builds and verifies the i386 ELF and
  uploads a tar archive with a standalone data-directory launcher. No original
  executable or archives enter the artifact. The project-owned Touhou Lab icon
  is copied beside the ELF and loaded as the SDL window icon;
- after the endurance pass, a fresh normal container build was linked without
  GDB or runtime patches, passed the ELF/layout/dependency/package checks, and
  was user-smoke-tested successfully under WSLg. The CI portable package was
  subsequently user-tested in a Kali Linux x86-64 GUI VM with native filesystem
  data, low memory, and no 3D acceleration. Normal life decrement remains
  active in this build;
- the Kali artifact's clean-data-directory exit was reproduced locally with
  the exact CI package: status 139, fault address `0xffffffff`, and a stack from
  `FindClose` into `Supervisor::StartupThread`. The report was written under
  `backup/modern-crash.txt` because the authored score-rotation code had already
  changed directories. The Linux `FindClose` backend now returns `FALSE` for
  null, `INVALID_HANDLE_VALUE`, or wrong-kind handles instead of deleting an
  invalid pointer. A clean two-DAT regression with an empty backup directory
  stayed alive, created the first score backup, and continued through the title
  assets. `th08.exe` is not opened; an existing `th08.cfg` is only useful for
  avoiding slow fullscreen calibration on unaccelerated VMs.

Reproduce the build with:

```bash
cmake -S . -B build/modern-windows -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/modern-windows --parallel 1
```

Run the executable without copying or linking the original archives:

```text
th08-modern.exe --data-dir "D:\path\to\the\original\TH08\directory"
```

Build and run the native Linux target with:

```bash
scripts/setup-modern-linux.sh "/path/to/the/original/TH08 directory"
```

After first-time dependency setup, use
`scripts/play-modern-linux.sh "/path/to/the/original/TH08 directory"`.

The optional isolated build is `scripts/build-modern-linux-container.sh` and
outputs `build/modern-linux-container/th08-modern`.

See `docs/LINUX_PORTING.md` for the platform boundary, environment overrides,
validation matrix, and reusable lessons from the Linux reconstruction.

The selected directory becomes the runtime working directory, matching the
original game's relative-file behavior for configuration, score, replay, and
BGM files. Keep the executable, SDK DLL, generated test data, and runtime
screenshots under `build/` or outside the repository; never commit the original
archives.

## Current status

As cold-built and replayed on 2026-08-20 against the original Japanese TH08
1.00d target:

- authored source: **1,107 / 1,107 functions**, **459,757 / 459,757 bytes**;
- strict authored exact: **1,105 / 1,107 functions**, **459,115 / 459,757 bytes**;
- library inventory: **1,119 classified functions**; all **1,119 / 1,119** now have
  mapping sizes totaling **217,165 bytes**; the independent library exact ledger currently accepts **260 functions / 52,955 body bytes**;
- `config/claims.csv` is header-only;
- a cold normal VC7 build links `build/th08.exe` successfully;
- a cold objdiff build followed by full replay passes **1,105 / 1,105**
  accepted units;
- the completed production-layout passes cover the Ascii target clusters,
  Player bomb/main regions, misplaced `main.cpp` helpers, Anm render helpers,
  Global math helpers, SoundPlayer helpers/tail, and GameManager helpers/main
  region. `Player.obj`, `GameManager.obj`, `Global.obj`, and `SoundPlayer.obj`
  now each have zero target-order inversions. Canonical owners record the
  section-defined production copies rather than stale probe/objdiff donors;
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

Library acceptance now has **260 / 260 configured units** and **52,955 body bytes**
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

The 2026-08-20 baseline after restoring the target-proven ICF link contract is:

- file size: target **840,704**, rebuild **817,152** bytes, so the rebuild is
  **23,552 bytes (`0x5C00`) smaller**;
- entry point: target `0x004A619E`, rebuild `0x00498EAB`;
- `.text` virtual/raw: target `0xB1B78 / 0xB1C00`, rebuild
  `0xA8C4F / 0xA8E00`; the raw-code deficit is `0x8E00`;
- `.rdata` virtual/raw: target `0x11284 / 0x11400`, rebuild
  `0x14FDC / 0x15000`;
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
  members recorded in `KNOWLEDGE_BASE.md`. The configured prerelease
  `d3dx8tex.obj` is now directly attested by exact auxless-COMDAT units for
  `D3DXLoadSurfaceFromFileInMemory @ 0x0047AAF3` (0xB8, four REL32 relocations)
  and `D3DXCreateTextureFromFileInMemoryEx @ 0x0047B72E` (0x40, one REL32).
  Both have zero non-relocation differences. An isolated `mssdk/lib/d3dx8.lib`
  link kept all 17 extra imports, reduced located anchors to 930 with 86 missing,
  and shrank `.text` raw size to `0x96C00`, so do not replace the pinned
  prerelease archive. The live D3DX import issue is member retention/splitting
  under `/OPT:NOREF`, not a production source API mismatch;
- 952 accepted address anchors are found in the linker map and 66 accepted units
  lack a unique map candidate. Twelve located anchors are ICF-folded aliases;
  they remain exact evidence but are excluded from TU-layout ranking because a
  shared linked RVA does not identify the contributing object's placement.
  Thirty-five production objects have accepted anchors, 34 retain at least one
  non-folded layout anchor, and the only folded-only production object is
  `utils.obj`. Anchor-derived current-object order still differs from target.
  The largest genuine intra-object drift is now `BulletManager.obj` at
  **104,240 bytes**, followed by `EnemyManager.obj` at **89,792 bytes**;
  `AnmManager.obj` falls from the former 359,792-byte apparent span to only
  **160 bytes** once its folded vertex-constructor alias is excluded.

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
(**59 anchors / 154 inversions / 7 runs / 9,600 span**) but no exact-code
regression. The later natural emissions of
`Float3::Float3(float,float,float) @ 0x00404720` and eight Sound fade/pause
COMDATs all have target-neighbor evidence; the pairwise inversion count grows
with the number of anchors and is not directly comparable to the earlier
45-anchor measurement.

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

The sixth bounded pass repaired `SoundPlayer.obj`. Eight exact fade/pause
helpers appended to the Sound donor had target addresses
`0x00406AC0..0x00406C50`, directly between the already recovered Anm and
GameManager Ascii-emitted clusters. `AsciiManager.obj` was the only production
object with undefined references to the five outer SoundPlayer wrappers. Once
the wrapper bodies in `SoundPlayer.hpp` and their three nested stream-helper
bodies in `zwave.hpp` were restored inline, VC7 emitted the exact target order:
each SoundPlayer wrapper immediately followed by its CStreamingSound helper,
then `Pause` and `UnPause`. Focused Ascii/Sound replay passed **86 / 86** and
the required full cold replay passed **1,105 / 1,105**.

The remaining Sound tail at `0x0045E2D0..0x0045E3F0` was continuous but its
three deferred header accessors appeared after the explicit constructor/free/
fade functions. All accessor callers are in the same `/Od` Sound TU and retain
target-proven call boundaries. Making `CWaveFile::GetFormat`,
`CSoundManager::GetDirectSound`, and `CSound::GetWaveFile` explicit definitions
immediately before the constructor restored the complete target lexical order;
focused replay passed **25 / 25**, followed by another **1,105 / 1,105** cold
aggregate replay. `SoundPlayer.obj` improved from **181 inversions / 4 runs /
358,864 bytes of drift span** to **0 / 1 / 0**.

The seventh bounded pass repaired `GameManager.obj`, which initially had **37
anchors / 174 inversions / 9 runs / 291,648 bytes of drift span**. It required
several kinds of evidence-backed repair rather than one artificial source-file
split:

- `SetLives`, `UpdateAntiTamper`, `SetBombCount`, and `SetPower` now emit from
  the early Ascii consumer cluster. The target places Sound helper COMDATs
  before these GameManager COMDATs, so `th_pch.h` includes `SoundPlayer.hpp`
  before `GameManager.hpp`; PCH include order is part of the VC7 emission
  contract.
- `IsSoloHuman`, `IsSoloYoukai`, and `GetLives` now emit from their real
  `EnemyManager.obj` consumer at the target `0x0042F230..0x0042F2B0`
  neighborhood.
- the retained GameManager main definitions were reordered by target address.
  `AddLives @ 0x0043C641` is an explicit definition between the constructor and
  `InitArcadeRegionParams`, matching the continuous target sequence without
  changing its exact body.
- the Player-local sequence at `0x0044E140..0x0044E348` is now explicit in
  `Player.cpp`: `SetYoukaiGauge`, `RandomizeAntiTamper`, `AddToDeaths`, and
  `AddToBombsUsed`. This both restores the target order and replaces the stale
  objdiff-only `RandomizeAntiTamper` owner with the exact production copy.
- `ScaleFloatBasedOnRank @ 0x00422B80` is the first definition in the existing
  target-contiguous `AsciiManagerBossMarker.cpp` block, immediately before the
  two Ascii functions at `0x00422BB0..0x00422C13`.

Focused replay passed **65 / 65** Ascii units for the setter move, **44 / 44**
Enemy units for the solo move, **62 / 62** Player units for the Player-local
block, **3 / 3** Ascii boss-marker units for the final cluster, and **40 / 40**
units in the final GameManager donor. Shared-header checkpoints and the final
cold aggregate replay pass **1,105 / 1,105**. The final linked metrics are
`GameManager.obj` **30 / 0 / 1 / 3,970**, `Player.obj` **36 / 0 / 1 / 7,264**,
and `AsciiManagerBossMarker.obj` **3 / 0 / 1 / 0**.

The eighth bounded pass repaired `Gui.obj`. The target neighborhood from
`Gui::OnUpdate @ 0x004338CA` through `Gui::FUN_00439810 @ 0x00439810` is
substantially continuous, so the twelve linked-order runs were lexical
definition disorder rather than evidence for twelve original source files. The
40 explicit definitions in `Gui.cpp` now follow target address order; only the
file-local helpers needed by earlier callers received forward declarations.

One residual remained after that reorder: header-inline
`AnmVm::FUN_004396f8 @ 0x004396F8` emitted at its first Gui use, placing it far
earlier than the target. Production callers are confined to Gui, while the two
PlayerOption probe callers retain target-proven REL32 calls. Making the header
declaration-only and placing the unchanged body explicitly between
`Gui::FUN_004396b8 @ 0x004396B8` and `Gui::LoadMsg @ 0x00439710` preserves all
call boundaries and the established Gui production owner. Focused replay
passed **41 / 41** Gui units and **15 / 15** PlayerOption probe units. The final
cold aggregate replay passed **1,105 / 1,105**. `Gui.obj` improved from **28
anchors / 167 inversions / 12 runs / 23,133 drift span** to **28 / 0 / 1 /
101**.

The ninth bounded pass repaired the safe lexical part of `SpellCard.obj`. Its
explicit function definitions now follow target address order across the current
production TU. Focused replay passed **34 / 34** accepted units, and the
subsequent cold aggregate replay passed **1,105 / 1,105**. The linked object
improved from **31 anchors / 122 inversions / 7 runs / 167,120 span** to **31 /
18 / 2 / 151,552**.

The 18 residual inversions are intentionally retained. The continuous Spellcard
main range through `CutChain @ 0x004180F0` is followed in the target by the
consumer-emitted `ZunTimer::operator--(int)` and GameManager spell-number
helpers at `0x00418110..0x00418180`. Current source also contains explicit
Spellcard helpers whose target homes are far away (`0x0041F040..0x0041FDD0`,
`0x0042DFF0`, and later Player-region helpers). They have no bounded production
caller/owner evidence that justifies moving them into another TU, while the
existing consumer COMDATs are already exact. Do not change header visibility or
invent owners solely to erase this metric residual.

The tenth bounded pass repaired `BulletManager.obj`. The production source had
real definitions in a highly non-target lexical order spanning the early
`0x00415C60` clear wrapper, the `0x0042A410..0x00432F20` bullet body, and the
`0x00433820..0x004338C0` lifecycle/resource tail. Sorting the existing
definitions by mapped target address required only forward declarations for
`CopyBulletAnmVmCore` and `SelectBulletSprite`; no behavior or owner changed.
Focused replay passed **38 / 38**, and the subsequent cold aggregate replay
passed **1,105 / 1,105**. Linked metrics improved from **25 anchors / 85
inversions / 4 runs / 121,968 span** to **25 / 0 / 1 / 104,240**. The large
remaining span is the real target distance between `0x00415C60` and the main
`0x0042F360+` block, not a target-order residual.

The eleventh bounded pass repaired `Background.obj`. Two tiny Background
methods were not Background-TU bodies at all in the shipped layout: the only
production callers are in `Spellcard.cpp`, and target addresses place
`background_fun_00415ce0 @ 0x00415CE0` between `StartSpell` and the cut-in
sequence, and `background_fun_00416ad0 @ 0x00416AD0` between `EndSpell` and
`spellcard_fun_00416af0`. Their unchanged definitions and canonical match-unit
owners now live in `SpellCard.obj`. Background donor/SpellCard recipient
focused replay passed **29 / 29** and **36 / 36** respectively.

The retained Background definitions were then repaired with targeted lexical
moves rather than a whole-file sort, preserving the interspersed file-scope
arrays/globals: constructor/OnUpdate/interpolation helpers, stage-data load/
update/render, and the late resource-reload predicate now follow target order.
`Background.obj` improved from **26 anchors / 61 inversions / 6 runs / 68,640
span** to **24 / 0 / 1 / 528**. The subsequent cold aggregate replay passed
**1,105 / 1,105**.

The twelfth bounded pass repaired `Midi.obj`. Its only target-order reset was
caused by two header-defined bodies whose correct owner was already Midi but
whose deferred emission positions were wrong. `MidiTimer::OnTimerElapsed @
0x0043EF40` is the empty virtual base callback and belongs before the main
`0x00443C60+` Midi implementation; `MidiOutput::Ntohl @ 0x004444E0` belongs
between `ParseFile` and `LoadFile`. Both caller sets are confined to Midi and
the target preserves real call/virtual boundaries, so the header declarations
are now bodyless and the unchanged definitions sit explicitly at their target
lexical positions in `Midi.cpp`. Focused replay passed **33 / 33**, and the
subsequent shared-header cold aggregate replay passed **1,105 / 1,105**.
`Midi.obj` improved from **33 anchors / 43 inversions / 2 runs / 25,888 span**
to **33 / 0 / 1 / 19,728**; the remaining span is the real target gap from the
base virtual callback to the main Midi TU.

The thirteenth bounded pass repaired `EnemyManager.obj`. Its five runs mixed
ordinary explicit-definition disorder with one target-contiguous cross-class
tail. Sorting only function bodies by mapped target address (leaving file-scope
statics in place) required one natural forward declaration for
`FUN_0042eb10`; focused replay remained **44 / 44** exact and reduced the
linked object to three residual inversions.

Those three inversions proved a real owner boundary. The target sequence is
`GameManager::IsSoloHuman/IsSoloYoukai/GetLives @ 0x0042F230..0x0042F2B0`,
then the recovered `AsciiManagerScale` block at `0x0042F2D0..0x0042F315`,
then `Gui::IsBossPresent @ 0x0042F320` and `Gui::FUN_0042f340 @ 0x0042F340`,
followed immediately by `BulletManager::Initialize @ 0x0042F360`. Both donor
and recipient use the same `/Od` PCH profile. Moving the unchanged Gui bodies
onto the end of `AsciiManagerScale.cpp` and canonicalizing their match-unit
owners produced an exact four-function `0x0042F2D0..0x0042F355` block. Donor
replay passed **42 / 42**, recipient replay **4 / 4**, and the cold aggregate
replay passed **1,105 / 1,105**. `EnemyManager.obj` improved from **18 anchors /
32 inversions / 5 runs / 111,744 span** to **17 / 0 / 1 / 89,616**;
`AsciiManagerScale.obj` is **4 / 0 / 1 / 0**.

The fourteenth bounded pass repaired `Supervisor.obj`. Three target-order
resets had two distinct causes. `SoundPlayer::UpdateFades @ 0x00447764` and
`GameManager::IsExtraUnlockedWithAllTeams @ 0x00447D04` already had the correct
Supervisor production owner, but header visibility made VC7 emit their exact
standalone bodies at earlier first-use positions. Their target locations are
respectively between `TakeSnapshot`/`LoadConfig` and `LoadConfig`/`LoadMusic`.
Both headers are now declaration-only and the unchanged bodies are explicit at
those target-local positions in `Supervisor.cpp`. The GameManager helper has
multiple Title callers, but their accepted units already contain real REL32
calls; focused replay passed **49 / 49** Supervisor units, **23 / 23** TitleScreen
units, and **11 / 11** TitleReplay probe units. The third reset was ordinary
lexical disorder: `UpdatePlayTime @ 0x004482A1` now precedes
`UpdateGameTime @ 0x00448418`. The cold aggregate replay passed **1,105 /
1,105**, and `Supervisor.obj` improved from **43 anchors / 29 inversions / 4
runs / 58,804 span** to **43 / 0 / 1 / 51,658**.

The fifteenth bounded pass repaired `ReplayManager.obj`. The object mixed plain
lexical disorder with two same-owner deferred header bodies. `LoadReplayData @
0x00451D90` is now the first Replay definition, the constructor moved from the
file head to its target slot after `DeletedCallback @ 0x00453080`, and the
existing `AppendFormat @ 0x00453B80` tail is followed by explicit
`GameManager::SetClockTime @ 0x00453C60` and `ReplayManager::IsDemo @
0x00453CC0`. The latter two were declaration-only header repairs; each has only
ReplayManager production callers. Focused replay passed **18 / 18**, the cold
aggregate replay passed **1,105 / 1,105**, and linked metrics improved from **15
anchors / 23 inversions / 4 runs / 12,352 span** to **15 / 0 / 1 / 192**.

The sixteenth bounded pass repaired `PbgFile.obj`. The sole target-order reset
was caused by the inline base-class `IPbgFile` constructor/destructor emitting
immediately after the derived `CPbgFile` constructor. The target instead keeps
the complete `CPbgFile` implementation through `GetFullFilePath @ 0x00473B40`,
then emits `IPbgFile::IPbgFile @ 0x00473C40` and `~IPbgFile @ 0x00473C60` before
the following PBG/LZSS code. Making those base bodies declaration-only in the
header and defining the unchanged bodies at the tail of `PbgFile.cpp` preserved
**15 / 15** focused units. The cold aggregate replay passed **1,105 / 1,105**,
and `PbgFile.obj` improved from **13 anchors / 20 inversions / 2 runs / 1,456
span** to **13 / 0 / 1 / 48**.

The seventeenth bounded pass repaired `EffectManager.obj`. This TU had 49
mapped function definitions in heavily scrambled lexical order while file-scope
statics and the `g_EffectTemplates` data definition were interspersed between
them. Rebuilding the source as the same declarations/data in their original
relative order followed by all real function bodies sorted by mapped target
address required only natural forward declarations for `FUN_00428310` and
`FUN_00428720`. No behavior or owner changed. Focused replay passed **52 / 52**
and the cold aggregate replay passed **1,105 / 1,105**. `EffectManager.obj`
improved from **8 anchors / 17 inversions / 4 runs / 23,424 span** to **8 / 0 /
1 / 304**.

The eighteenth bounded pass repaired `ResultScreen.obj` and the adjacent
ScoreDat owner boundary. `AnmManager::ReplaceSurface @ 0x0045A3FD` has only a
ResultScreen production caller, so its header body is now declaration-only and
the unchanged explicit definition follows `MoveCursorHorizontally @
0x0045A2F2` in `ResultScreen.cpp`. `ScoreListNode::ScoreListNode @ 0x0045A4DC`
required stronger evidence: a naive move into `/Od` `ScoreDat.cpp` produced a
0x2B-byte body instead of the accepted 0x20 bytes, but placing it immediately
after `Catk::WasAttemptedWithShot @ 0x0045A4A0` under that function's existing
`#pragma optimize("s", on)` reproduced the exact body. The canonical owner now
is `ScoreDat.obj`. Focused replay passed **33 / 33** ResultScreen and **14 / 14**
ScoreDat units; cold aggregate replay passed **1,105 / 1,105**. Linked metrics
are `ResultScreen.obj` **30 / 0 / 1 / 0** and `ScoreDat.obj` **13 / 0 / 1 / 9**.

The nineteenth bounded pass repaired `PbgArchive.obj`. Two same-owner header bodies were deferred at first use rather than their target tail positions: `CPbgFile::ReadInt @ 0x004751E0` and `PbgArchiveEntry::~PbgArchiveEntry @ 0x00475270`. Making both declaration-only and defining the unchanged bodies after `CopyFileName` / after `SeekPastString` respectively preserved **15 / 15** focused units. Cold aggregate replay passed **1,105 / 1,105**, and `PbgArchive.obj` improved from **14 anchors / 11 inversions / 3 runs / 2,032 span** to **14 / 0 / 1 / 32**.

The twentieth bounded pass repaired `TitleScreen.obj`. Two target-late helper
bodies were emitted at their first users because they were still header-inline:
`TitleScreen::SetKeyNumberSprite @ 0x00469FA9` and
`GameManager::IsLastWordSpellCardAttempted @ 0x0046FD5F`. Both are now
header declarations with unchanged explicit definitions after
`OnUpdateKeyConfig` and `DrawSpellStageSelect` respectively. Focused replay
passed **23 / 23** TitleScreen units and **11 / 11** TitleReplay probe units;
cold aggregate replay passed **1,105 / 1,105**. `TitleScreen.obj` improved from
**23 anchors / 4 inversions / 3 runs / 15,726 span** to **23 / 0 / 1 / 11,979**.

The twenty-second bounded pass repaired `MusicRoom.obj`. The first global
visibility probe for `Supervisor::IsMusicPreloadEnabled @ 0x00449C79` correctly
reproduced the standalone body but changed two accepted Supervisor callers by
1-2 bytes, so it was rejected. The accepted repair makes the header
**declaration-only only while compiling MusicRoom** via a TU-local preprocessor
gate; every other TU still sees the original inline body and therefore keeps its
caller codegen. The explicit MusicRoom-tail definition is itself `inline`, so it
remains a COMDAT and coexists with the other header copies while production
object order selects the target-local MusicRoom copy. Focused replay passed
MusicRoom **11 / 11**, Supervisor **49 / 49**, and SoundPlayer **25 / 25**; cold
aggregate replay passed **1,105 / 1,105**. `MusicRoom.obj` improved from **10
anchors / 7 inversions / 2 runs / 2,621 span** to **10 / 0 / 1 / 0**.

The twenty-first bounded pass repaired `ScreenEffect.obj`. Its only reset was ordinary lexical disorder: source emitted `DrawArcadeFade @ 0x0045BC40` before `DrawPartialFade @ 0x0045BBF0`, opposite the target. Swapping those complete bodies preserved **21 / 21** focused units; cold aggregate replay passed **1,105 / 1,105**. `ScreenEffect.obj` improved from **14 anchors / 1 inversion / 2 runs / 160 span** to **14 / 0 / 1 / 48**.

The twenty-third bounded pass repaired the production ECL owner lane that had
been split across `EnemyManager.obj`, `SpellCard.obj`, and `main.obj`. Target
mapping shows `EclManager::RunEcl @ 0x004184B0` followed by the ECL operand/run
neighborhood at `0x0041F000..0x0041FDF0`; all four donor/recipient TUs use the
same `/Od /Yu"th_pch.h"` profile. The unchanged `HasParentChain`,
`SetStoredVector`, spellcard ECL flag helpers, attached/parent-chain helpers,
`GetActiveState`, `GetTimerFrames`, and `ZunTimer::operator+=` bodies now live
in production `EclManager.cpp`, with `Spellcard::FUN_0042DFF0 @ 0x0042DFF0`
also following the EclManager timeline getters. Canonical match-unit owners were
updated with the bodies. Focused replay passed **18 / 18** EclManager, **39 /
39** EnemyManager, **30 / 30** SpellCard, and **30 / 30** main units. Cold
aggregate replay passed **1,105 / 1,105**. `SpellCard.obj` improved from **33 /
18 / 2 / 151,472** to **27 / 0 / 1 / 61,920**, and `main.obj` from **26 / 16 /
2 / 146,336** to **25 / 0 / 1 / 16**; recipient `EclManager.obj` is **10 / 0 /
1 / 60,848**. The large EclManager span is target distance, not an order reset.

The twenty-fourth bounded pass repaired `AnmManager.obj`. Its only reset was
`AnmManager::SpriteHasTexture @ 0x004622C0`, a PCH-defined inline body emitted
after `DrawTriangleFan @ 0x00464EC0` instead of between `SetInterruptArray @
0x00462270` and `ExecuteScriptArray @ 0x00462310`. A MusicRoom-style TU-local
header gate cannot work inside a `/Yu` consumer because `AnmManager.hpp` is
already fixed inside the PCH. The accepted PCH-safe shape therefore makes the
header declaration-only, provides the target-local `inline` COMDAT body in
`AnmManager.cpp`, and provides the same early inline definition in
`TitleScreen.cpp` so its optimized caller still compiles with body visibility.
Focused replay passed **83 / 83** AnmManager and **23 / 23** TitleScreen units;
cold aggregate replay passed **1,105 / 1,105**. `AnmManager.obj` improved from
**56 anchors / 19 inversions / 2 runs / 359,840 span** to **56 / 0 / 1 /
359,792**. The large span is the real distance from its early target COMDATs at
`0x00406700/0x0040B580` to the main `0x0045E430+` region, not an order reset.

The twenty-fifth bounded pass repaired the last remaining production layout,
`AsciiManager.obj`. Four independent fixes were required. First,
`AnmVmBase::Initialize @ 0x004068E0` became an `inline` COMDAT so it could join
the target helper emission queue. Second, `Float3::Float3(f32,f32,f32) @
0x00404720` moved from its header body to an unchanged ordinary definition
between `PauseMenu::OnUpdate` and `PauseMenu::OnDraw`, matching the target-local
slot. Third, only lexical order changed inside shared headers: `ZunTimer`
`operator=`/`SetCurrent` now precede `<`/`>`, and `GameManager::IsSpellPractice`
precedes `IsReplay`, matching the observed COMDAT order. Finally,
`GetGaugeInterrupt`, `ResetStrings`, and `SetSpaceWidth @ 0x00407140..7180`
were made declaration-only and their unchanged bodies/canonical owners moved to
the already recovered `/Od /Yu` `AsciiManagerGauge.obj` target cluster. The
relevant accepted callers contain real REL32 calls. Final focused replay passed
**62 / 62** AsciiManager, **5 / 5** AsciiManagerGauge, **41 / 41** Gui, and
**62 / 62** Player units. Moving the `Float3` constructor out of the shared
header exposed one final caller-visibility dependency under a full cold build:
Supervisor, TitleScreen, and the TitleReplay probe compiled larger until the
same unchanged constructor body was restored as an early TU-local `inline`
definition in those callers. This preserves their former inline compilation
context without changing the target-local Ascii owner. The final cold aggregate
replay then passed **1,105 / 1,105**, and a fresh normal link retained zero
inversions. `AsciiManager.obj` improved from **63 anchors / 86 inversions / 6
runs / 9,600 span** to **60 / 0 / 1 / 736**; `AsciiManagerGauge.obj` is **5 / 0 /
1 / 32**.

The twenty-sixth bounded pass restored the `Spellcard::StartSpell` service
cluster at `0x00415C60..0x00415CE0`. Two exact helper bodies had been placed in
their class implementation TUs during early local matching even though target
layout puts them immediately after `StartSpell`:
`BulletManager::bulletmanager_fun_00415c60 @ 0x00415C60` and
`Enemy::enemy_fun_00415c80 @ 0x00415C80`, followed by the already-correct
`Background::background_fun_00415ce0 @ 0x00415CE0` in `Spellcard.cpp`. Moving
the unchanged bodies into `Spellcard.cpp` and rebinding their canonical exact
owners preserved focused replay at **32 / 32** SpellCard, **37 / 37**
BulletManager, and **38 / 38** EnemyManager units. A fresh production link
reduced `BulletManager.obj` from **25 anchors / 104,240 span** to **24 / 176**
and `EnemyManager.obj` from **14 / 89,792** to **13 / 36,704**. The SpellCard
cluster from `StartSpell` through the cut-in helpers now has one uniform linked
drift; its object remains **29 anchors / 0 inversions / 1 run / 128 span**.

The twenty-seventh bounded pass closed the production-owner side of the main
ECL dispatcher dependency lane. `EclManager::RunEcl @ 0x004184B0` remains
**26,638 / 26,638 authored bytes** and **27,398 / 27,398 compared bytes**
strict exact while provisional service/global identities were rebound to their
natural production owners. The same pass canonicalized the exact integer and
float operand resolvers, restored `Enemy::ClampPosition @ 0x0042C180` to
`EnemyManager.cpp`, and promoted the target ECL call-parameter/interpolator/EX
dispatch data owners into `EclGlobals.cpp`. Focused replay passed **24 / 24**
EclDependencies, **33 / 33** EclExIns, **4 / 4** EnemyTimeline, **39 / 39**
EnemyManager, **32 / 32** SpellCard, and all four resolver units. A no-`/FORCE`
diagnostic link of production objects plus the exact ECL Run/Dependencies/
Helpers/Operands/Timeline/ExIns group now resolves completely; its only
remaining unresolved object is the still-probe-only `EnemyManagerUpdate.obj`,
with **77** provisional owner symbols. Treat that object as the next bounded
promotion blocker rather than reopening already-closed RunEcl/resolver owners.

The twenty-eighth bounded pass closed that final authored ECL owner blocker.
`EnemyManagerUpdate::OnUpdate @ 0x0042C660` remains **6,198 / 6,198 authored bytes**
and **6,214 / 6,214 compared bytes** strict exact while its 77 provisional
external identities were routed back to natural production owners. The pass
restored the real `g_EclManager @ 0x004ECCB8` data owner; its `0x168`-byte
extent ends exactly at `g_EclCallParameters @ 0x004ECE20`. It also rebound
GameManager/Player/subsystem singletons, ZunTimer calls, Enemy methods, the ECL
timeline lane, canonical `fabsf`, and the target-exact `0x0041FD20` /
`0x0042ADB0` helpers. The last data alias was proven to be
`g_EnemyManager.firstEnemy.bullet2e24`: `BulletSpawnDescriptor` is exactly
`0x210` bytes and its address is `0x00577F20 + 0x2E24 = 0x0057AD44`, matching
the target `memcpy` source. A no-`/FORCE` diagnostic link of production objects
plus EclRun, EclDependencies, EclHelpers, EclOperandsInt/Float, EnemyTimeline,
EclExIns, and EnemyManagerUpdate now exits successfully with **zero unresolved
externals**. Cold authored replay remains **1,105 / 1,105 exact**. The next
bounded step is production promotion and fresh whole-image measurement, not
further owner canonicalization.

The twenty-ninth bounded pass moved the closed ECL lane into the real production
link without changing its compiler contract: EclRun, EclDependencies, EclHelpers,
EclOperandsInt/Float, EnemyTimeline, EclExIns, and EnemyManagerUpdate now build as
`build/*.obj` with the same generic `/Oi /Gr` profile that produced their accepted
probe objects. All **69 / 69** promoted match units replay strict exact from the
production paths. Fresh whole-image routing then exposed three earlier probe lanes as
missing bodies inside existing target TUs rather than standalone objects.
`PauseMenu::OnDraw` / `RetryMenu::OnDraw` were restored to their real `void` ABI and
inserted into `AsciiManager.cpp`, which now replays **65 / 65 exact** and reduces its
former `0 / -304 / -752` drift staircase to only `0 / -80`-class alignment/COMDAT
noise. `GameManager::GameplaySetupThread @ 0x0043ABD7` and `FUN_0043BBE1` returned to
`GameManager.cpp`; the target `g_TimeRequirementParams @ 0x004C77F0` storage is a
semantic `9 x 4` i32 table, the setup member has the observed `__fastcall void(void *)`
ABI, and `GameManager.obj` now replays **42 / 42 exact** with the former 3,418-byte
drift jump removed. Fifteen Player option/shot functions were restored into the two
target gaps in `Player.cpp`; after preserving their VC7 `#pragma var_order` contracts,
`Player.obj` replays **77 / 77 exact** and the former `-63,824 -> -67,152 -> -70,960`
staircase collapses to one approximately constant drift. The resulting fresh linked
PBG lane is only **4,896..4,848 bytes early**, down from 16,672 bytes before these
owner restorations.

The promoted ECL code is target-backed but its object placement is not yet target-like.
The current normal image has `.text` raw size `0xBA800` versus target `0xB1C00`,
`.rdata` `0x15200` versus `0x11400`, `.data` `0x6400` versus `0x6E00`, and file-size
delta `+49,152` bytes. Do not interpret the `.text` overshoot as evidence that the
exact ECL bodies are spurious: their target anchors are valid, but the eight promoted
objects are currently appended after the main authored lane. In particular
`EclDependencies.obj` now reports **160 target-order inversions / 7 runs** and must be
partitioned/routed by target neighborhoods; `EclExIns.obj` has a small two-run kink.
The next bounded whole-image task is ECL object order/partition, followed by the small
remaining authored gap (PBG about 4.9 KB early), not further ECL owner canonicalization.

The production link contains **44 authored/PBG object files**. The fold-aware
whole-image report now includes the promoted ECL objects as production layout
evidence. Existing settled objects such as AsciiManager, GameManager, Player, and the
PBG lane retain one target-order run, while the newly promoted mixed ECL dependency TU
is intentionally the active partition candidate. `utils.obj` remains folded-only and
`EclGlobals.obj` remains data-only; neither should be used as TU-placement evidence. The final cold authored replay remains **1,105 /
1,105 exact**, `config/claims.csv` remains header-only, and the final
tracking/progress/CI/doc/whitespace gates pass. Do not infer whole-image byte
identity from this milestone: section drift, imports/resources, and non-layout
reconstruction work remain separate evidence classes. For future layout
changes, preserve the same acceptance rule: target-neighborhood evidence,
focused donor/recipient/caller replay, a cold normal linked-order measurement,
and a cold aggregate exact replay before committing.
Large intra-object drift means today's source combines or orders target TUs
differently, so permuting the existing object list alone cannot solve it. A
run reset is a routing clue, not automatically a TU boundary: the Player pass
showed that one investigation may require both a real split and target-order
restoration inside the retained TU. The 17
extra imports already have bounded contributing archive members; do not hide
them with `/OPT:REF`. A trial of that flag made the import set exact while
shrinking `.text` to `0x72B6F` and losing 212 located accepted anchors, so
`/OPT:NOREF` remains required by stronger whole-image evidence. ICF is a
separate target-proven contract: constructor-iterator sites across multiple
subsystems load the same `0x0040B580` body for four differently decorated
vertex constructors, and the linked image also contains the reviewed folded
sound-manager constructor at `0x004716E0`. Keep `/OPT:ICF` enabled. Return to
CRT/D3DX exact recovery only if a subsequent diff identifies a specific member
operation. Do not copy the target icon payload and do not claim whole-image
exactness from successful linking or same-size sections.

Use `docs/TOOLS.md` to choose commands. Keep `.analysis/` disposable and update
this file whenever the active milestone or a durable blocker changes.
