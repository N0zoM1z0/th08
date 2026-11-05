# Cirno-Labs reference audit

This audit on `audit/cirno-reference`, based on `bcddaa2c`, found and repaired
one initialized-data omission. It did not add authored exact credit. The
remaining replay callback has a new compiler explanation, but its missing
source construct is still unknown.

## Pinned inputs and scope

- Canonical Japanese TH08 1.00d: 840,704 bytes, SHA-256
  `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`.
- [Cirno-Labs/th08](https://github.com/Cirno-Labs/th08), commit
  `b8a68cfeca6d22e644ec3813d76ff5199e50a674`, cloned under the ignored
  `_reference/cirno-th08/` directory. Its recorded executable SHA-1,
  `2044fea52667be4f61bcd78efc8f7d63fbbc244f`, agrees with our canonical file.
- Its tools submodule is pinned to
  `edf50486ef7f1777f8c097155f7e79b16d1b8592`.
- The reference is an independent reconstruction. Its names and interpretations
  are hypotheses; our imported GensokyoClub history and attribution remain the
  source baseline. No reference function body was imported.

The broad pass crosswalked every function start and authored extent, replayed
our entire accepted ledger from a cold build, and screened initialized data
shared by the two object sets. Focused source review covered replay input and
serialization, ANM fields, ECL position access, Player option callbacks, ending
tables, and the sound command dispatcher. This is not an exhaustive semantic
proof of every field, initializer, or execution path.

## Function inventory and comparison contracts

Both inventories describe the same 1,116 starts and lengths in
`0x00402000..0x00475298` (end exclusive). There are no missing starts or extent
disagreements in this range. Our authored inventory contains 1,107 functions;
the other nine are intentionally classified as library:

| Address | Reference name |
| --- | --- |
| `0x004031E0` | `fabsf` |
| `0x00408D40` | `cosf` |
| `0x00409060` | `sinf` |
| `0x0040B440` | `sqrtf` |
| `0x0041F090` | `fmodf` |
| `0x00433880` | `SinCos` |
| `0x00462210` | `acosf` |
| `0x00462230` | `atanf` |
| `0x00462250` | `tanf` |

The reference README's 1,127 compared bodies also includes separately emitted
inline copies. These are inventory conventions, not additional missing
authored behavior in this repository.

The pinned reference comparator resolves named relocations and local labels.
For unnamed initialized data, it masks the instruction address and compares
the referenced data, masking pointers inside that data. Unresolved code or
uninitialized references can remain masked. It also stops a function at its
associated switch table. Our manifests compare 50 accepted bodies through
their associated tables using `compare_size`; their authored `size` remains
separate. The reference's match claim therefore cannot replace our strict
relocation and extent checks.

Reproduce the read-only inventory crosswalk with:

```bash
git clone https://github.com/Cirno-Labs/th08.git _reference/cirno-th08
git -C _reference/cirno-th08 checkout b8a68cfeca6d22e644ec3813d76ff5199e50a674
git -C _reference/cirno-th08 submodule update --init tools
python3 scripts/analysis/compare-reference-inventory.py --json \
  > build/cirno-audit/inventory.json
```

The helper checks the canonical target and the reference's recorded checksum,
records its revision and dirty paths, and compares by address rather than
assuming independently chosen names are equivalent. It does not award matches.

## Confirmed data repair: replay difficulty labels

**Observed:** `SaveReplay @ 0x004531F0`, size `0x98D`, indexes the pointer table
at `0x004C7F30`. Target bytes contain six pointers:

| Index | Target string address | Text |
| --- | --- | --- |
| 0 | `0x004B704C` | `Easy` |
| 1 | `0x004B7044` | `Normal` |
| 2 | `0x004B703C` | `Hard` |
| 3 | `0x004B7034` | `Lunatic` |
| 4 | `0x004B702C` | `Extra` |
| 5 | `0x004B7020` | `LastWord` |

Our `g_ReplayDifficultyList` had only the first five initializers. Before the
repair, the fresh normal link's sixth slot contained `0x00000001`. The new
runtime-data check rejects that artifact. The repair restores `LastWord` and
checks all six linked pointers and pointed-to strings in both native products.
It does not add a persisted sixth `Difficulty` enum value.

`SaveReplay` already matched because its relocation proved the table address,
not the entire table initializer. The focused comparison remains exact after
the repair. No natural gameplay path reaching difficulty index 5 was reproduced;
the demonstrated defect is missing target data, not an observed player crash.

## Remaining callback: register phase is explained, source is not

**Observed:** `PlaybackExtendedInputAndFps @ 0x004526C0` ends at `0x00452829`,
before alignment bytes. Its target body is `0x169` bytes. Our ordinary source
emits `0x16A`, with the previously recorded register phase difference after
advancing the six-byte input record. The `% 8` lowering then uses the longer
generic `and` instead of the accumulator encoding.

The reference's `PlayOnUpdateWithSeeds` inserts an empty `if (0) {}` between
reading the record's input and incrementing its cursor. Its introduction is
explicitly explained in reference commit
`55382bf6dd495b7e9e6c32b0a446b96303466072`.

Two isolated controls were built with our VC7 flags and compared through the
canonical comparator engine, with all 19 relocations required:

| Source control at that lexical point | Result |
| --- | --- |
| Production source, no added construct | `0x16A`, rejected by the `0x169` extent |
| Reference's empty constant-false conditional | All `0x169` bytes and 19 relocations exact |
| Standard `assert(extendedInputCursor != NULL)` with `<assert.h>` and `NDEBUG` | All `0x169` bytes and 19 relocations exact |

The assertion's qualified expression in the actual probe was
`assert(replayManager->extendedInputCursor != NULL)`. Both controls were removed.
The assertion illustrates a compiled-out expression; its position after the
dereference does not establish a useful original validation contract.

**Inferred:** a discarded source construct can explain the allocator phase;
the earlier pointer/cast/local-order matrix did not test this mechanism.
**Unknown:** whether the original contained an assertion, a disabled diagnostic
branch, or another discarded expression, and what it tested. Neither an empty
conditional nor an invented assertion is accepted as recovered behavior.

The function now has the configured diagnostic unit
`replay-manager-playback-extended-input-and-fps`, absent from `matches.csv`:

```bash
python3 scripts/build.py --build-type=objdiffbuild --object-name ReplayManager.obj
python3 scripts/compare-function.py replay-manager-playback-extended-input-and-fps --json
```

The production result is deliberately still an extent error. Do not shorten
the manifest, pad code, or promote either control into the accepted ledger.

## Initialized-data screening and readable source choices

All 32 reference production translation units compiled serially with our VC7
environment. Generated source copies were converted from UTF-8 to CP932 under
`build/cirno-audit/reference/`; reference source remained unchanged. Compilation
used the reference's `/Od /Gr /GX /Oi /Gy /GF`, per-file `// cflags:` overrides,
`TH08_EXE=1`, and an explicit `/I scripts/prefix/mssdk/include` before the
ordinary include paths. This selects DX8's 20-byte `D3DXIMAGE_INFO` rather than
the prerelease Platform SDK's incompatible definition. No reference executable
was linked or playtested.

Screening joined 82 initialized owners by canonical target address using
reference symbols and our relocation manifests. Scalar bytes, resolved pointer
destinations, and directly pointed-to string/float literals were checked against
the target. Inferred extents stopped at the next COFF data symbol. Ten extent
differences required source review; eight owner rows retained unresolved nested
or private pointer identities. This screening is diagnostic, not data exact
credit. It found no further confirmed omission after the replay repair.

Concrete dispositions:

| Family | Comparison and decision |
| --- | --- |
| Replay labels | Adopt the complete six-entry initializer, established directly from target bytes. |
| Texture format widths at `0x004B8498` | Keep our six entries `4,4,2,2,3,2`. The reference declares seven; its final `1869363770 == 0x6F6C3A3A` reproduces the first four bytes of the separately mapped string `::loadAnim : %s\n @ 0x004B84B0`. No reviewed consumer establishes a seventh format. Do not import that inferred table boundary. |
| Player option callbacks | Reference uses two `[13][4]` tables. We use 12 shot-type rows plus named `g_PlayerRoute3Exit*Callbacks` at `0x004C7E00` and `0x004C7ED0`. The special rows are present and preserve their target destinations. Keep the explicit role names; this is not missing data. |
| Ending paths | Reference uses three 12-entry arrays at `0x004C7048/78/A8`; our `g_EndingFiles` owns the same 36 pointers as three rows. The apparent reference-only starts are subarrays of our owner. Keep the typed grouping. |
| Loading/effect/music/digit tables | Reviewed declared counts agree: 12 loading paths, 15 effect paths, 19 music records, and 10 digit strings. Next-symbol extent differences include alignment, not additional entries. The crypt signature likewise has three used bytes plus a reference-side trailing zero. |
| ANM 25/31/88 | Reference names `autoRotate`, `cameraSetup`, and `useColor2` are useful search hypotheses. Opcode 25 has no reviewed independent consumer there; 31 and 88 agree with behavior already recorded in our ANM evidence queue. Keep neutral shared names until the complete consumer audit, preserving opcode 88's second-byte operand. |
| ECL 169 | Reference's direct `enemy->pos.x` makes the intended condition clearer. Our byte view derives from `offsetof(Enemy, position)` to preserve proven VC7 shape and native pointer growth. The reference supports the field meaning; it does not justify removing that code-generation constraint without a new strict comparison. |
| Sound commands | Reference's `SoundCommand` and named stages make the dispatcher easier to navigate. Our source already has a typed command/state model. Preserve target branch/fallthrough order instead of importing its independent labels wholesale. |

The useful general practice is to document each field's lifecycle and consumer
beside its declaration, and give protocol values semantic names once all uses
support them. Compact spelling, an independent descriptive name, or a matching
instruction address alone does not establish an original aggregate boundary.

## Validation and limits

- Focused `SaveReplay` comparison: exact. All 18 accepted units owned by
  `ReplayManager.obj`: exact after rebuilding the repaired source.
- `verify-exact-units.py --all --json`: cold build, 1,106 selected, 1,106 exact,
  no failures. Accepted totals are unchanged.
- Fresh normal link and bugfix link: pass the extended native owner/data/callee
  verifier, including all six replay labels. The pre-repair normal product
  fails the new check at index 5.
- Linux x86_64 compilation, repository CI, progress/tracking checks, and
  `git diff --check` are recorded with the branch checkpoint.

Generated evidence stays under `build/cirno-audit/`, including inventory,
isolated control objects/reports, data-screening observations, cold comparison,
build logs, and separate before-repair/normal/bugfix native products. This batch
does not extend any published gameplay replay, visual, audio, or platform gate.
Whole-executable layout and the discarded replay source construct remain open.
