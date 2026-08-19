# Build and exact matching

## Toolchain

The inherited build reproduces the upstream Visual Studio .NET 2002 (VC7) and
DirectX 8 environment, generates `build.ninja`, and invokes the Windows tools
through `scripts/th08run.bat`. On Linux/macOS the wrapper runs them through
Wine.

Host requirements are:

- Python 3.11 or newer for the reconstruction ledgers and analysis helpers
  (`tomllib` is used); the inherited compiler build itself remains compatible
  with Python 3.4;
- `msiextract` and Wine on Linux/macOS;
- `aria2c` optionally, for torrent-backed dependency acquisition;
- the initialized Detours Git submodule for optional DLL builds.

Create the environment on Linux/macOS with:

```bash
git submodule update --init --recursive
./scripts/create_th08_prefix
```

The helper downloads historical toolchain inputs into ignored local paths and
uses `~/.wineth08` as its default Wine prefix. Set `WINE` before invocation if
a compatible alternative runner is required. On Windows:

```text
python scripts/create_devenv.py scripts/dls scripts/prefix
```

Do not commit downloaded compilers, SDKs, prefixes, or original game files.

## Builds

The canonical normal build is:

```bash
python3 ./scripts/build.py
```

It regenerates `build.ninja` and builds `build/th08.exe`. Other inherited build
modes are selected explicitly:

```bash
python3 ./scripts/build.py --build-type bugfix
python3 ./scripts/build.py --build-type diffbuild
python3 ./scripts/build.py --build-type dllbuild
python3 ./scripts/build.py --build-type objdiffbuild
```

These modes serve different runtime and comparison purposes. Success in a
bugfix, DLL, or object build does not establish that the normal
executable matches the original.

## Target detection

Place the privately supplied exact target at `resources/th08.exe` and verify
its identity before comparison:

```bash
sha256sum resources/th08.exe
stat -c '%s' resources/th08.exe
reccmp-project detect --search-path resources/
```

The required SHA-256 is
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`
and the required size is `840704` bytes. `reccmp-project.yml` sets
`allow_hash_mismatch: false`; do not weaken that gate.

## Executable comparison

After the normal build, register the reconstructed executable from `build/`
and generate a report there:

```bash
cd build
reccmp-project detect --what recompiled
reccmp-reccmp --target th08 --html report.html
```

The report consumes the mappings in `config/reccmp-functions.csv`,
`config/reccmp-globals.csv`, `config/reccmp-floats.csv`, and
`config/reccmp-strings.csv`. A mapped symbol is not necessarily implemented or
exact. Preserve the report as local evidence under `build/`; publish numerical
progress only through a reproducible generation path.

The progress generator reports source presence and exact matching separately:

```bash
python3 scripts/progress.py --check
```

Exact figures count only accepted entries in `config/matches.csv`; source
presence remains a separate inventory. See `docs/PROGRESS.md` for the generated
interpretation and counts.

## Object comparison

`objdiff.json` maps reconstructed objects under `build/objdiff/reimpl/` to
original/delinked objects under `build/objdiff/orig/`. Build all reconstructed
comparison objects with:

```bash
python3 ./scripts/build.py --build-type objdiffbuild
```

The build wrapper can also request one object through `--object-name`. For a
configured function, build only its translation unit and run the strict COFF
comparator against the verified executable:

```bash
python3 scripts/build.py --build-type=objdiffbuild --object-name ItemManager.obj
python3 scripts/compare-function.py item-auto-collect --json
```

The comparator requires the symbol, target address, size, and every COFF
relocation to agree with `config/match-units.toml`. Missing or extra
relocations fail closed. It reports exactness only for that configured function
range; it does not imply an object- or executable-wide match.

VC7 may include compiler-owned switch tables in a function's COFF auxiliary
extent. Such units keep `size` as authored code coverage and use `compare_size`
for the complete code-plus-table range. Exact acceptance still compares every
associated byte and relocation; table bytes do not increase authored progress.

### VC7 source-shape notes

The following `/Od` behaviors are confirmed by small VC7 probes and by strict
TH08 matches; use them as diagnostics, not as permission to force bytes:

- `#pragma var_order(a, b, c)` assigns listed function-scope locals from the
  least-negative stack slot downward in list order. Nested block locals are not
  reliably controlled by a function-level list.
  On large `/Od` functions, getting this order right can change total function
  size substantially because locals inside `-0x80..-0x1` use short EBP
  displacements while deeper slots require long displacements on every access.
- A block-scope `#pragma var_order` can control locals declared in that block,
  including a direct-initialized class local that receives a hidden return
  buffer. Do not put the pragma directly after a label: VC7 can mis-handle name
  visibility there. Open a normal `{ ... }` block after the label first.
- Unlisted scoped locals can otherwise occupy earlier stack slots than expected.
  When a large function's frame size is right but every named local is shifted,
  identify the actual owners of the leading slots before changing semantics.
- Under `/Od`, lexical `case` body order affects emitted switch layout even when
  the numeric case values and jump table are unchanged. Likewise, ordinary
  `break` statements can compile directly to the switch merge while explicit
  `goto` statements may introduce short trampoline chains. Preserve the target's
  source-level control-flow shape where the bytes distinguish them.
- Placement construction and tiny wrapper classes are not neutral stack-layout
  tools here. In the tested VC7 `/EHsc` configuration they introduced extra
  constructor/placement-`new` machinery, so prefer ordinary source constructs
  and compiler-native temporaries.
- A user-defined empty default constructor can still materialize as a call under
  `/Od`. If the target instead shows a plain aggregate copy (for example, one
  base load followed by three dword moves), keep the public/ABI type intact and
  use a function-local trivial aggregate with the same fields for the temporary.
  This can express the observed copy semantics without constructor machinery;
  accept it only when the strict comparator proves the full function.
- Value context matters for x87 comparisons. A floating comparison used directly
  as a ternary condition can branch on the status word without materializing a
  source value, while `static_cast<ZunBool>(comparison)` makes VC7 `/Od` store a
  32-bit `0`/`1` temporary and test it before selecting the ternary arm. When the
  target contains that materialized boolean slot, reproduce the value conversion
  in C++ rather than inventing a named stack local.
- Boolean grouping can change temporary ownership even when the truth table is
  identical. Under VC7 `/Od`, `gate ? (a && b) : false` can materialize the inner
  conjunction and then copy it into a second ternary-result slot; flattening the
  same logic to `gate && a && b` removes that outer slot. Preserve the target's
  expression grouping when stack shape shows both temporaries.
- Ternary comparison direction and integer signedness can change the exact
  branchless select sequence even when the result is algebraically identical.
  For example, VC7 may emit `setl` plus a signed difference mask for
  `value >= limit ? negative_a : negative_b`, while the inverted `<` form or
  unsigned hex constants select `setge` and a different mask/add sequence.
  Preserve the target's comparison direction and signed value context before
  trying to tune registers around an equivalent expression.
- A one-bit bitfield assignment has its own read-modify-write shape: VC7 can
  evaluate and mask/shift the RHS first, then load the containing word, clear the
  destination bit, OR the shifted value into that word, and store it. An
  equivalent hand-written integer mask/OR expression can choose the opposite OR
  destination register. When the target shows the bitfield pattern, a typed
  bitfield view is a more faithful C++ expression than algebraic register tuning.
- Recovered typed fields can also fix evaluation order that raw byte-pointer
  arithmetic gets wrong. A raw `*(u32 *)(p + dst) = *(u32 *)(p + src)` may make
  VC7 prefetch the left-hand base before finishing the RHS; expressing the same
  operation as a real field assignment such as `vm->color1Initial = vm->color1`
  can restore the target's RHS-first register sequence without inventing a stack
  temporary. Prefer the existing ABI type when the offsets are already proven.
- Signedness of byte fields is visible in exact codegen: plain `char` may produce
  `movsx`, while the target's `movzx` is evidence for an unsigned value context.
  Cast the read to `u8` (or recover the field type) instead of masking the result
  after sign extension.
- Keep nested condition ownership intact when an `else if` belongs only to the
  outer condition. Flattening `if (gate) { if (a && b) body; } else if (c)` into
  `if (gate && a && b) body; else if (c)` changes behavior when `gate` is true
  but `a` or `b` is false, and VC7 can expose the mistake as a near conditional
  jump where the target has a short jump to an outer merge trampoline.
- For class-valued arithmetic, algebraic commutativity does not imply identical
  VC7 hidden-return-buffer codegen. In the matched spell-effect interpolation,
  `delta / scale + base` lets the division temporary stay in `this` for the final
  `operator+` while `base` is pre-pushed as the RHS; spelling the equivalent
  `base + delta / scale` changes temporary creation and call setup. Preserve the
  target's operand order even for mathematically commutative operators.
- An empty SDK/class default constructor can be target-visible under `/Od`. A
  plain declaration such as `D3DXVECTOR3 position;` emitted the target's call to
  the empty constructor, while copy-initializing the same local from an existing
  vector elided that call and emitted three dword copies. Distinguish declaration
  from copy initialization when the target shows constructor timing explicitly.

For a function whose authored body is followed by compiler-owned tables, first
prove the authored extent independently, then set `compare_size` to the COFF
auxiliary extent and replay every table relocation as well. Local `$L...` COFF
labels are normalized to `$L*` by the comparator because VC7 renumbers them when
earlier code in the translation unit changes; their relocation offsets and
resolved target addresses remain the evidence.

When a shared header change appears to break unrelated strict units after a fresh
rebuild, establish causality before editing manifests: save the current header,
restore the committed header only for the failing object's rebuild, rerun that
unit, then restore the current header and rebuild again. If the same size or
relocation failure persists under the committed header, treat it as baseline
staleness rather than attributing it to the new declaration. Only update a
compiler-local relocation name when the A/B build proves that the current header
caused the renumbering while machine code and resolved target addresses remain
unchanged.

For stack, register-home, direct-call, absolute-reference, and return-cleanup
facts, install Python Capstone and generate a read-only target packet:

```bash
python3 -m pip install capstone
python3 scripts/typed-re.py 0x004413E0 --compare --json \
  > build/typed-re-004413E0.json
```

There is no VC7 library scanner yet. Do not borrow TH07 archives or infer
library matches from names. A future scanner must start with SHA-pinned
TH08-specific archives, relocation policy, and canonical comparator replay.


- If a target stack layout matches a known ternary with compiler-generated boolean/result
  slots, do not promote those slots into named locals. In `Item::CollectPoint`, two
  explicit decompiler locals displaced `this`; restoring the same
  `static_cast<ZunBool>(comparison) ? a : b` shape already proven by
  `CollectPointSmall` recreated the anonymous `-0x10/-0x14` temporaries naturally.
- A side-effecting loop condition can explain a target loop head that calls a helper,
  tests state, branches directly to the exit, and jumps back to the helper after the
  body. `while ((UpdateThreshold(), value >= threshold))` matched that VC7 shape;
  spelling it as an infinite loop plus `if (...) break` introduced a two-byte branch
  trampoline even though the behavior was equivalent.
- When a switch-bearing function emits compiler-owned jump tables in the same COFF section,
  keep `size` equal to the authored function extent but set `compare_size` to the full
  source-emitted COFF section. This attests jump-table entries and padding without
  inflating authored-byte progress. `Spellcard::Init` is a concrete example: 0xCA5
  authored bytes plus 0x63 bytes of three VC7 switch tables compare as a 0xD08 unit.
- Do not merge branch-local resource checks merely because the failure action is identical.
  In `Spellcard::Init`, each `PreloadAnm` branch performs its own immediate null check
  and return path. Hoisting those checks to a common merge preserved successful-case
  semantics but changed branch topology and shortened the target by dozens of bytes.


- For a dense VC7 switch, compare the object jump-table entry targets with the target table before rewriting the whole dispatcher. If every later case start is displaced by the same constant, that constant is often exactly one missing case body. In `GuiImpl::RunMsg`, cases 8 through 0x16 were all shifted by `0xEC`; reconstructing the missing 236-byte case 7 made every one of the 23 case starts land on its canonical address and also caused VC7 to emit the target function-level `push esi` naturally.
- Do not replace a target's repeated calls with a loop merely because the operands form a contiguous array. `GuiImpl::RunMsg` executes its eight message VMs as eight lexical `ExecuteScript` calls; spelling them as `4 + 2 + 2` loops added exactly 26 bytes of loop machinery under `/Os`.
- Pointer-update spelling can determine VC7 register ownership. The adjacent-engine source shape `currentInstr = (Instr *)((i32)&currentInstr->args + currentInstr->argSize)` preserved the base pointer in EAX and loaded `argSize` through ECX, exactly matching TH08. Algebraically rewriting it as `currentInstr + argSize + 4` reversed those roles even though the computed pointer was identical.
- A later case body can change the prologue of the entire function. Before GUI opcode 7 was restored, `RunMsg` had the correct frame but no callee-saved ESI save; the real case's register pressure made VC7 emit `push esi`/`pop esi` automatically. Do not force such prologue bytes locally—restore the missing source body first.


- The type of an apparently neutral constant can control where VC7 converts an integer expression to floating point. In `Gui::DrawGameScene`, `GetPower() + 488 + 0.0f` emits the target `add eax, 488; fild; fadd 0.0`, while `GetPower() + 488.0f` converts `GetPower()` first and emits a different x87 sequence. Preserve integer subexpressions and even a trailing `+ 0.0f` when the target shows an integer ALU operation immediately before `fild`.


- VC7 can match a target stack slot only when the source also reuses one local across phases. In `Gui::FUN_0043741d`, the target slot at `[ebp-0x20]` first holds the boss-life count and later the capped spell timer. Keeping separate `bossLives` and `cappedSpellcardSecondsRemaining` locals enlarged the frame by four bytes; one reusable `bossValue` restored the target frame without padding.
- `#pragma var_order` is lexical-scope sensitive in this codebase. A function-level pragma did not order locals declared inside the boss-HUD block. Moving the pragma into that block, while keeping declarations at their target execution points, restored `dark=-0x8`, `bright=-0xc`, `rect=-0x1c..-0x10`, `bossValue=-0x20`, `segmentIndex=-0x24`, `segmentStop=-0x28`, `timerColor=-0x2c`, `segmentWidth=-0x30`, and `textPos=-0x3c..-0x34`.
- A constructor can be target-visible even when the initialized value is overwritten before its apparent first use. The TH08 boss-HUD code constructs `Float3 textPos(48.0f, 16.0f, 0.0f)` immediately before the first health-bar draw and later assigns `(384,16,0)`. Omitting or moving that seemingly dead construction changed both size and control-flow offsets.
- Prefer correcting typed field order over compensating in expressions. `Gui::bossLifeBarMaxSize` was actually at `+0x34` while the previous header named the `+0x30` float as the max value. Swapping the two field names removed repeated target/object displacement mismatches throughout the health-bar code and preserves the proven segment arrays at `+0x3c/+0x5c/+0x7c`.


- For VC7 x87 comparisons, `a > b` and `b < a` are not interchangeable source shapes. In `Gui::FUN_00435900`, the target gauge test `fld size; fcomp max; test ah,0x41; jne` came from `bossLifeBarSize > bossLifeBarMaxSize`; spelling it as `bossLifeBarMaxSize < bossLifeBarSize` emitted the opposite operand/test-mask sequence despite identical C++ semantics.
- Preserve the target's outer condition direction when it controls a large lexical branch. The clock-display tail matches as `if (timer >= 60) { if (current < target) animate; else timer++; } else { timer++; }`. Rewriting it as `if (timer < 60) ... else if (...)` selected a short conditional jump plus an extra branch and made the function two bytes short.
- Equivalent boolean regions can have very different floating-point branch layouts. The GUI portrait-alpha target is naturally `if (x >= 64 && y < 128) { fade down } else { fade up }`; the De Morgan form `x < 64 || y >= 128` reversed both x87 test masks. Prefer the target fallthrough region over a logically equivalent negated predicate.


- A local array of a type with a non-trivial default constructor can explain an otherwise mysterious `eh_vector_constructor_iterator` call. `GuiImpl::DrawDialogue` declares `VertexDiffuseXyzrhw vertices[4]`; leaving it as a real local array naturally emits target helper `0x406850` with element ctor `VertexDiffuseXyzrhw::VertexDiffuseXyzrhw @ 0x40B580`.
- Preserve aggregate-copy spelling when the target copies a constructor temporary with string instructions. In `GuiImpl::DrawDialogue`, the adjacent source shape `memcpy(&vertices[i].pos, &Float3(...), sizeof(Float3))` emits `Float3::Float3` followed by three `movsd`/`rep`-style dword copies exactly like TH08. Replacing that with an apparently cleaner typed assignment can change register ownership and copy lowering.


- Do not merge adjacent label/value text calls just because one formatted string could display the same text. `Gui::FUN_0043826b` has a standalone `AddFormatText("Night Bonus")`, advances Y by 16, then a second `AddFormatText("        %8d0", value)`. Combining them into `"Night Bonus = %8d0"` made the function exactly 34 bytes short and changed the static call-site layout.


- A compile-time string length can be the right way to prevent over-folding of a floating expression. In `Gui::FUN_00438a89`, `(384.0f - (f32)strlen("Spell Card Bonus!") * 14.0f) / 2.0f + 32.0f` makes VC7 fold the literal `strlen` to `17.0f` but still emit the target five-step x87 multiply/subtract/divide/add sequence. Replacing it with the seemingly equivalent literal `17.0f` let VC7 fold the entire x coordinate to one constant and made the function 24 bytes short.

## Acceptance rules

- Verify the target hash before every new comparison environment.
- Start at function/object scope and resolve calls, globals, strings, floats,
  imports, and relocation differences explicitly.
- Preserve VC7 calling conventions, structure layout, compiler flags, source
  order, and translation-unit effects.
- Do not use copied target byte arrays, naked assembly dumps, fake types,
  arbitrary padding, or behaviorally empty bodies to manufacture equality.
- A successful compile or link means only `compiles`. Claim `matching` only for
  the exact scope demonstrated by the accepted report and command.
- Do not infer a repository-wide percentage from source coverage, mapping rows,
  or adjacent-version similarity.
- Cache-vs-direct-object access can be target-visible under `/Os`. In the GUI message
  initializer, caching `&this->msgVm` in a local pointer moved `this` from `-0x10` to
  `-0x14` and shortened every later large-offset access. The target repeatedly forms
  `this + 0x21814`; a local typed overlay is still useful as a type, but do not store
  a pointer to it unless the target has that stack slot.
- Preserve block placement, not just condition truth. The target's route setup spells
  replay handling as `if (!IsReplay()) { clear-history tree } else { replay tree }`;
  moving the replay tree before the non-replay checks kept semantics but changed a
  large forward branch region.
- Recover table row types when address generation disagrees. A flat `u32[]` indexed
  by `shotType * 4` generated `shl 2` plus a scale-4 SIB; the target used `shl 4` and
  a plain base+index load. Typing the table as 16-byte rows (`u32 colors[4]`) restored
  the exact addressing mode and removed one byte from each of four loads.

- In a small VC7 `/Os` cleanup loop, caching a repeated indexed field address in an explicit pointer local can make the function *shorter* than the target. `Enemy::FUN_0042bc90` is 96 bytes only when the three `this + i * 4 + 0x3384` accesses remain lexically explicit; a `void **slot` cache compiled to 88 bytes. Treat repeated address formation as source-shape evidence rather than automatically introducing a convenience local.

- Effect-specific storage can overlap semantically even when a shared struct currently gives the slots one effect's names. `ScreenEffect::CalcShake` reads its interpolation endpoints from `+0x18/+0x1C`, while fade drawing also uses those slots for color-related state. Preserve the target-observed offsets until `RegisterChain` proves a universal field meaning; do not force a misleading shared field name into another effect mode.
- Splitting a floating expression across assignments can be necessary to preserve VC7 `/Od` x87 spills. In `ScreenEffect::CalcShake`, one combined interpolation expression emitted `fimul/fidiv/fiadd` in-register and was 12 bytes short. Reusing one `f32 shakeAmount` across multiply, divide, and add assignments recreated the target's three `fstp` spill points plus intervening `fild/fdivr/fadd` sequence exactly.

- A dense switch can reveal the original lexical case order through physical case-body placement even when the jump table is numerically indexed. `ScreenEffect::RegisterChain` has numeric cases 0..7, but the target body order is `0,1,2,4,3,5,6,7`; preserving that source order made VC7 emit the exact 0x274-byte body and 0x20-byte table.
- When a function's public struct fields are effect-specific aliases, the constructor/registration path can be stronger layout evidence than any single consumer. `ScreenEffect::RegisterChain` proves `+0x18/+0x1C/+0x20` are generic param3/param4/param5 storage selected by effect type; consumers should keep target-observed offsets until a universally valid field model is recovered.

- Sibling formatted-text helpers need not share buffer size. `AnmManager::DrawTextRight` uses a 128-byte local buffer, but `DrawTextCentered` is exact only with `char buf[72]`; that gives the target 0x70 frame and places `buf/fontWidth` at `-0x50/-0x54`. Infer local-array extent from frame/offset evidence, not neighboring source.
- For VC7 x87 range tests, explicit negation can be the exact source shape. In `Player::FUN_00449ff0`, nested `!(min > point)` and `!(max < point)` preserve the target operand order while selecting `test ah,0x41; je` and `test ah,0x05; jnp`; flattening to `min <= point && max >= point` or De Morgan forms changes the status masks. The same function also needs a shared branch-local `next` merge for circle/rotated/axis misses rather than replacing every miss with `continue`.

- In `BulletManager::OnDraw`, four expressions spelled as `cosine * length + position` each made VC7 spill the product before calling `Float3::operator float*`, adding exactly 6 bytes and one compiler temporary per expression. Reordering the same arithmetic to `position + cosine * length` removed all four spills at once, shrinking the function by exactly 24 bytes and restoring the target 0x2C frame. For class/accessor-valued operands, preserve target evaluation order even when scalar addition is commutative.
- A raw dword copy can be the correct source shape when the target prefetches the destination base before evaluating the RHS. In `BulletManager::OnDraw`, typed `laser->vm1.color1 = laser->vm0.color1` evaluated the RHS base first; spelling the observed `*(u32 *)(laser+0x494) = *(u32 *)(laser+0x1F0)` produced the target `dst-base / src-base / value / store` register order. Use this only when target offsets and widths are independently proven.
- `Bullet::DrawSingleBullet` confirms `AnmVm+0x1FC` is the signed `type` field, not `pendingInterrupt(+0x1FE)`, and its angle update is source-shaped as `ZUN_PI / 2 + bulletAngle`; reversing the add operands changes the x87 load/add sequence.

- A function's COFF auxiliary extent can contain multiple sparse-switch lookup schemes, not just a plain pointer table. `BulletManager::AddedCallback` has a 0x67E authored body followed by 0xFF bytes containing a six-entry jump table + 111-byte selector table and a three-entry jump table + 108-byte selector table. Keep authored `size` separate from `compare_size`, and attest both pointer entries and selector bytes without counting compiler tables as authored progress.
- Sparse switch decompilation can hide target-visible dead writes. In `BulletManager::AddedCallback`, script id 5 writes size class 3 and intentionally falls through into the script-106 body, which immediately overwrites it with class 4. Preserving that source fallthrough is required for exact code even though the first assignment is semantically dead. The target's physical case-body map was recovered from the jump-pointer and selector tables rather than trusting decompiler case grouping.
- `BulletManager::AddedCallback` also reinforces that a convenient row pointer is not neutral: caching `&bulletTypeSprites[i]` made the function 290 bytes short, while direct indexed expressions recreated the target's repeated `imul i,0xD44` address formation.

- VC7 can choose the opposite setcc for semantically identical ternaries depending on which comparison is written explicitly. In `Item::CollectTimeOrb`, `time >= threshold ? A : B` emitted `setl`, while the equivalent `time < threshold ? B : A` emitted the target `setge` and the exact `dec / and 0x107f / add 0xdfffef80` select sequence. When a branchless ternary is otherwise exact, invert both predicate and arms before trying register-level workarounds.

- AABB tests can be exact only when the boolean region owns the same shared false block as the target. `Player::CalcItemBoxCollision` matches as one OR-chain of failure predicates (`minX > maxX || maxX < minX || ...`) followed by `return 0; return 1;`. Nested negated `if` statements preserve finite-value semantics but change the final x87 branch/fallthrough and miss the target.
- `ItemManager::OnUpdate` shows that two visually identical vector resets may have different source shapes in adjacent states. Its state-2 reset uses a `Float3(0,0,0)` temporary, but state-3/state-5 death resets are three direct component stores. Writing all three as aggregate assignments created two extra 12-byte compiler temporaries and enlarged the frame from target `0x88` to `0xA0`.
- Large state-machine block order can be recovered from temporary offset drift. `ItemManager::OnUpdate` is exact only when autocollect is the positive body of a structured `if (state == AUTOCOLLECT || condition) { ... } else { normal fall }`; a semantically equivalent `normalFall` block followed by `goto autoCollect` displaced the homing block by 64 bytes even though the shared move label later realigned.
- A target function's auxiliary extent may be mistaken for authored code if a jump table follows the return immediately. `ItemManager::OnUpdate` has exactly `0x7C5` authored bytes and a 9-entry/36-byte compiler table, so its strict unit uses `size=1989` and `compare_size=2025`.
- Do not model two same-named folded float constants as one target plus a large addend merely because the COFF symbol text is identical. `ItemManager::OnUpdate` references a double-zero instance at `0x004B5B30`; replaying it as `__real@0000000000000000 @ 0x004B4C90 + 0xEA0` left only relocation-field bytes mismatched. Bind the manifest row to the exact target constant instance with addend zero.

- Tiny class helpers that already exist as natural C++ may still lack a standalone production owner. The Rng seed accessors were originally class-inline definitions; moving their unchanged bodies to `Global.cpp` produced the target standalone VC7 functions (`SetSeed`, `ResetGenerationCount`, `GetSeed`) without changing semantics. Treat such moves as shared-header changes: fresh-rebuild callers and rerun their strict units before accepting the new owner.
- Existing COMDAT-emitted helpers can sometimes be attested without any source change at all. `Float3::operator+=` and `AnmManager::ResetFrameDebugInfo` were already emitted by production objects and matched their target extents directly; source presence should still be separated from exact ledger acceptance until the canonical comparator is run.

### VC7 dispatcher locals and lexical branch placement

Two recent strict closures expose source-shape rules that are useful for large VC7 dispatchers:

- `GameManager::GetClockTimeIncrement` (0x43C35F) only reproduces its 0x38-byte frame when each of the six repeated stage cases has its own pair of integer locals. Calling equivalent getters or reusing one pair shrinks the frame and changes every case body. The target also requires explicit `if/else` returns; a semantically equivalent ternary lowers to `setl/inc` instead. The authored body is 0x134 bytes followed by a compiler-owned 0x20-byte jump table.
- In `EffectManager::OnUpdate` (0x427BF0), an equivalent `else if (mode != 0)` placed its body immediately after the test and produced a short branch, leaving the function at 0x309 bytes. Structuring the same logic as `if (mode == 0) { nested cases } else { nonzero body }` moves that body after the nested subtree, causing VC7 to emit the target long branch and the exact 0x30A-byte body. For near-exact large functions, inspect where physical branch bodies live before changing data or adding padding.

These are compiler-owned layout effects, not semantic differences. Prefer source restructuring and strict recompile over manual byte compensation.

### VC7 conditional-result temporaries and collision gate layout

The Player collision cluster around 0x44A230..0x44A930 adds two reusable `/Od` source-shape rules:

- `Player::FUN_0044a930` needs a nested conditional expression, `extreme ? 3 : (moderate ? 2 : 1)`. VC7 materializes the outer conditional result in a hidden stack slot and then copies it to the named local. Writing `(moderate != 0) + 1` is semantically identical but lowers through `neg/sbb/neg/add` and removes the target hidden result temporary. When a decompiler shows an anonymous value copied into a named local immediately after a conditional, preserve the conditional-expression ownership rather than algebraically simplifying it.
- `Player::CalcLaserHitbox` uses four nested negated overlap gates for its first rectangle test. A failure exits the nested region and naturally falls into the graze path; only four successful gates execute the explicit `goto` to the lethal path. Flattening the same test into a positive `&&` or a failure `||` changes the final x87 branch sense and introduces/removes compiler trampolines. The same `!(min > max)` / `!(max < min)` spelling also preserves the target `test ah,0x41` and `test ah,0x05` masks.

The laser helper also demonstrates that a seemingly interchangeable argument can be target-visible: the successful graze callback passes `&this->position`, not the incoming laser position. That changes the call setup by nine bytes even though both positions may be close in gameplay semantics.

### VC7 cold-path placement and arithmetic spelling

`Player::Die` (0x44AB40) shows two more source-shape details worth preserving in large control-flow functions:

- The target keeps the normal `bombs >= 1` deathbomb path lexically before the rare Miss path. This makes VC7 emit a six-byte forward `jl` into the cold tail block. Writing the equivalent `if (bombs < 1) { Miss } else { main }` places the cold block first and shrinks the branch to a short inverse jump, shifting the remainder of the function.
- Doubling the pre-death counter must be written as `counter += counter`; `counter *= 2` lowers to `shl` and removes the target's repeated receiver/load/add/store sequence under `/Od`.

When a near-exact function is short by only a handful of bytes, inspect cold-path lexical placement and apparently trivial arithmetic rewrites before adding locals or touching data layout.

### Scope-owned temporaries, probe aliases, and duplicated source bodies

The death-flow closures at 0x42ADB0, 0x42BEA0, 0x44C650, and 0x44CBA0 add several reusable VC7 rules:

- Constructor placement follows lexical scope under `/Od`. In `EnemyOverlay::FUN_0042adb0`, the first `Float3` must be declared only after the parent-chain test succeeds, while the attached-enemy `Float3` is declared only inside the attached tail path. Declaring both at function entry moves constructors and changes every later stack slot.
- When two independent block locals refuse to occupy the target slots, a trivial local aggregate can express the original ownership without padding. The exact ADB0 shape uses a two-int local struct so `itemCount` and the loop index occupy one contiguous eight-byte allocation while the separate `itemType` remains in the shallow slot. This is source structure, not manual stack padding.
- Algebraic equivalence is not instruction equivalence on x87. `(f32)itemCount * 2.0f` lowers to target `fild; fadd st,st`; adding two separately cast copies can lower to a longer integer-memory add. Likewise, random coordinate updates in `Enemy::FUN_0042bea0` must use compound `+=` so the lvalue pointer returned by `Float3::operator float*()` survives the RNG call in the target compiler temporary.
- Do not deduplicate repeated source bodies just because they are semantically identical. `Player::FUN_0044c650` contains two copies of the “consume all remaining Bombs” path under the forced/non-forced deathbomb branches. Combining them with `isForced || bombs < 2` removes 39 target-authored bytes.
- A probe alias is not automatically a production global. The analysis name `g_EclEnemyTableF54CC0` resolves to `g_EnemyManager + 0x9DCDA0` in the shipped image. Production code should reference the real `EnemyManager` storage and let the COFF relocation carry the field addend instead of creating a second global at the same address.
- Bitfield-to-bitfield assignment can be target-visible. `Spellcard::FUN_0044cba0` only reproduces VC7's redundant-looking mask sequence when bit 7 is assigned from bit 0 through a one-bit overlay; simplifying it to whole-word arithmetic changes register ownership and bytes.

If a header change is correct but VC7 reports a newly declared member as absent, verify the precompiled header timestamp. This repository's object-only path can reuse a stale `build/th_pch.pch`; forcing a PCH rebuild is preferable to changing valid declarations to satisfy stale compiler state.

### VC7 structure-copy order, signed bit extraction, and compiler-owned dispatch data

The BulletManager closure around 0x42A410..0x43216A adds several source-shape rules that recur in large `/Od` state machines:

- A fixed-size copy that ends in `rep movsd` does not imply `memcpy` source. `CopyBulletAnmVmCore` is exact only as a typed `AnmVm` structure assignment: VC7 then emits `ESI = src; ECX = 0xA9; EDI = dst; rep movsd`. The intrinsic `memcpy(dst, src, 0x2A4)` emits the same copied bytes but loads the count before `ESI`, missing eight authored bytes. Infer the source operation from register-evaluation order, not only the final string instruction.
- Masking a signed field does not automatically make its later right shift unsigned. In `Bullet::FUN_0042ffc0`, `(record->int0 & mask) >> n` produced `sar`; the target uses `shr`. Cast the stored signed dword to `u32` before mask/shift when the target extracts packed unsigned subfields.
- Avoid a result-valued floating ternary when the target writes each branch directly to its destination. The `0x400/0x800` transform case is exact as `if (value >= 0) field = value; else field = currentSpeed;`. A ternary introduced one hidden dword at `-0x22C`, enlarged the frame from `0x228` to `0x22C`, and added six bytes.
- One authored function can own more than one contiguous compiler-generated dispatch structure. `Bullet::FUN_0042ffc0` has a `0x81B` authored body followed by a 20-byte pointer table and a 64-byte selector map (`compare_size = 0x86F`). `BulletManager::FUN_0042f5f0` has a `0x85A` authored body plus a 36-byte nine-entry jump table, while `BulletManager::OnUpdate` has a `0xF16` authored body plus a 20-byte five-entry table. Count only the authored body in progress, but strict-compare every compiler-owned byte when the COFF auxiliary extent includes it.
- VC7 can renumber named compiler/local-label symbols when earlier functions in the same translation unit change even though the relocation offset and resolved target are unchanged. The strict comparator therefore normalizes only the trailing numeric suffix of labels shaped like `$name$12345` (and existing `$L...` labels), while still requiring the semantic label stem, relocation type/offset, resolved target, and raw bytes to match. Do not treat this normalization as permission to ignore relocation targets.

The same cluster reinforces that adjacent-version source is scaffolding only: TH06 helped identify the broad bullet/laser state machine, but TH08 moved transform behavior into a separate 18-record interpreter. The TH08 shipped image, target relocations, and fresh VC7 objects remained the acceptance evidence for every recovered branch and field.

### Fastcall parameter homes, slot reuse, and large Player state-machine layout

The Player closure around `0x44AEC0`, `0x44D650`, and `0x451640` adds several useful VC7 `/Od` rules:

- Do not promote obvious assignment temporaries to source locals merely because the decompiler names them. In `Player::AddedCallback`, the two floats at `-0x18/-0x1C` are compiler temps created while assigning `g_PlayerPlayfieldWidth / 2` and `g_ItemPlayfieldBottom - 64` through `Float3::operator float *()`. Declaring them explicitly pushed the fastcall `Player *` home from target `-0x14` to `-0x1C` and enlarged/shifted the entry sequence. Writing the expressions directly restored the target home slots automatically.
- One physical stack slot can represent source variables with different signedness in different lexical loops. `AddedCallback` reuses `-0x4`: the 384-entry `PlayerUnkStruct0x40` reset loop lowers as unsigned (`jae`), while the later 128-entry shot-slot loop lowers as signed (`jge`). Keeping one `u32` source slot and spelling the second condition as `(i32)i < 0x80` recovers the target without inventing another local.
- Equivalent flat indexing can change address-generation ownership. The option callback tables are exact as rows of four pointers indexed `table[route].callbacks[slot]`, which makes VC7 form `route << 4` first and then apply `slot * 4`. Flattening to `table[route * 4 + slot]` is semantically identical but changes the register/evaluation sequence.
- A large switch's lexical case order is observable independently of the numeric case values. Both movement-speed switches in `Player::FUN_0044aec0` are exact with physical case order `4, 3, 1, 2, 5, 7, 6, 8`; two compiler-owned eight-entry tables map numeric cases back to those blocks. Strict comparison uses `size = 0x12A1` and `compare_size = 0x12E1`, counting only authored bytes while still verifying both tables.
- When IDA pseudo-code and shipped control-flow disagree, trust the shipped image. The team-route animation block at `0x44AEC0` is one example: the physical route/odd-even script selection was recovered from target branches and strict comparison rather than the initial decompiler arm labels.

### Shipped-vs-analysis-database target safety

- Preserve shipped executable semantics when the analysis database contains a research patch. At `0x44D0F9`, the shipped v1.00d bytes are `push -1` before `GameManager::AddLives`; the IDA database had been intentionally patched to `push 0`. Reconstruction follows `resources/th08.exe`, not the patched pseudo-code.
### Canonical relocation owners and local-array extent

- Raw bytes are not enough to justify a field owner. `Supervisor::CalculateFps` initially matched with source expressions that referenced the wrong members while a generated relocation manifest compensated by shifting the global base. Always verify each recovered relocation base against `config/reccmp-globals.csv`; the exact owners here are `g_GameManager + 0x2D`, `g_Supervisor + 0x300`, `+0x178`, and `+0x33C`. Exact source should make both bytes and relocation ownership canonical.
- An IDA stack-gap inference can overestimate a local array. `Supervisor::CheckFps` looked like `float samples[31]` from the decompiled frame, but VC7 only reproduced the target `0xA4` frame and `[ebp+index*4-0x88]` addressing with `float samples[29]`. Preserve target-visible dead locals (the elapsed-seconds calculation), then use fresh compiler stack allocation as stronger evidence than decompiler array guesses.
- For `/Os` control flow with a shared success epilogue, lexical ownership matters. `CheckFps` matches as `if (!disableVsync) { diagnostics; if (average >= 65) { ... return -2; } } return 0;`; spelling the two success paths as separate early returns adds a short inverse branch plus a trampoline.
- Commutative integer addition can determine register ownership. `CalculateFps` requires `currentQpc >= lastQpc + (frequency >> 1)` in that operand order to emit `eax=frequency/2; ecx=last; add ecx,eax; cmp current,ecx; jb`. Equivalent orderings folded one operand into memory or reversed the compare.
### D3DX projection owners and x87 comparison width

- Call arity plus the shipped push sequence can recover imported D3DX helpers without guessing library bodies. In `AnmManager::FUN_00463d60`, `0x477178` is `D3DXVec3Project` (out, input, viewport, projection, view, world), `0x477612` is `D3DXMatrixMultiply`, and `0x477F42/0x477FC2/0x478043` are `D3DXMatrixRotationX/Y/Z`. Use the standard D3DX API so production source remains ordinary C++ and the import thunks stay compiler/linker-owned.
- A floating zero literal's type is target-visible. The three rotation gates in `FUN_00463d60` are exact only as `rotation.axis != 0.0` (double literal); `0.0f` emits single-precision x87 compares and leaves exactly one opcode byte wrong at each of the X/Y/Z gates.
- A contiguous 64-byte `rep movsd` target copy is stronger layout evidence than an old field guess. `AnmManager+0x1C24..+0x1C63` is a cached `D3DXMATRIX`, not `Float3` followed by padding. Recovering the aggregate type lets VC7 emit the target matrix copy naturally.
- Do not route new reconstruction through the repository's `sincos` macro when inline assembly is disallowed. VC7 `sin`/`cos` intrinsics do not fuse into `fsincos`; functions whose target specifically requires that instruction should remain unclaimed until a non-inline-assembly source mechanism is found.

### VC7 block-local slot order and exact float literals

- Two class-valued locals can have the correct constructor order but the wrong physical stack slots. `PlayerOptionHomingToPlayer` and `PlayerOptionHomingToTarget` both construct `target` then `delta`; the target stores them at `target=-0x18` and `delta=-0x0c`. Keeping lexical construction order while changing `#pragma var_order` from `(target, delta, ...)` to `(delta, target, ...)` fixed all 15 byte differences in each function without changing control flow.
- Do not treat decimal float literals as approximate documentation. `PlayerRoute2OptionRender` writes the exact single-precision value produced by `0.49f` (`0x3efae148`). Spelling it as `0.495f` produced the same instruction shape but a different three-byte tail in the immediate. Recover the source literal/constant that rounds to the shipped IEEE value, then let the compiler emit it.

### Constructor lifetime placement and recovering absolute-address owners

- `#pragma var_order` controls physical slots but does not move constructor calls. In `AnmManager::FUN_004639e0` / `FUN_004640e0`, declaring `D3DXMATRIX` and four `Float3` locals at function entry emitted their constructors before the target's `rotation -> fsincos` sequence even though every slot was correct. Keeping scalar declarations first, executing `fsincos`, and only then declaring the class-valued locals moved the five constructors to the exact target offsets without changing the 0xA0 frame.
- Before creating a new production global for a target absolute address, subtract nearby known object bases. The projection reference at `0x004EA3F4` is exactly `g_Background + 0x63C4`, i.e. `g_Background.unk6394.vectors[4]`. Referencing the real aggregate owner preserves the correct `g_Background` relocation plus field addend and avoids overlapping storage aliases.

### Byte-lane writes versus packed-color RMW

- A source write to one byte of a packed color is not necessarily equivalent to the target's dword operation. `EffectOrbitUpdate @ 0x426030` keeps the existing RGB low 24 bits, converts the computed alpha with `__ftol2`, shifts it by 24, ORs it into the preserved dword, and writes the full color back. Spelling the operation as `effectBytes[0x1f3] = alpha` removed the target's ESI save/mask/or sequence and made the function 29 bytes short. Express the observed whole-word read-modify-write in ordinary C++ when the target proves it; an extra `(u8)` cast before the shift also inserts a target-absent `movzx`.

### Compiler-owned RHS temporaries and shared return blocks

- In `Player::FUN_0044cbf0`, explicit `spawnX` / `spawnY` source locals forced VC7 to reserve their stack slots before the member-function `this` home. The shipped code instead comes naturally from direct assignments such as `position.operator float *()[0] = expression`: because the conversion call would clobber the x87 value, VC7 creates compiler-owned RHS temporaries at the point of use. This restored `value=-0x4`, `this=-0x8`, an earlier ternary temp at `-0xC`, and the two late RHS temps at `-0x10/-0x14` without padding.
- Repeated `return 0` statements are not source-shape neutral under VC7 `/Od`. Two explicit zero returns in `Player::FUN_0044cbf0` each emitted a local `xor eax,eax` before jumping to the epilogue. Spelling the same control flow as ordinary `if/else` arms let both paths jump to the single shared final zero-return block, removing exactly two bytes per path and matching the target extent.

### Large draw loops: aggregate copies, operand order, and x87 loop ownership

- `EnemyManager::OnDrawImpl @ 0x42e140` recovered its target `sub esp, 0xA4` frame without padding by keeping the observed lexical local order and letting six `Float3` return temporaries remain compiler-owned. Source-level locals for these temporaries would change the frame layout.
- Adjacent scalar member copies can be source aggregate copies. Saving/restoring `AnmVm::scale` as a `Float2` generated the target paired loads/stores and removed otherwise repeated owner-pointer reloads. Separate `.x` / `.y` assignments were semantically equivalent but byte-different.
- Integer multiplication operand order matters at `/Od`: `savedAlpha * k` produced the target `movzx eax,alpha; imul eax,[k]`, while `k * savedAlpha` forced an extra register load and was two bytes longer at each fade site.
- A threshold in a `for` condition is not byte-equivalent to a body-local `break`/`continue`. For the `-990.0f` trail sentinel, spelling `if (sample < -990.0f) break/continue;` restored the target x87 `test ah,5` plus short `jp`/`jmp` trampoline. Algebraically negated conditions changed parity branches and jump ownership.

### Large varargs report builders: loop ownership and allocation temporaries

- `ResultScreen::LogScoreDataToFile @ 0x454298` recovered a 0x488-byte VC7 frame by preserving the target lexical stack order, including a 222-entry spell-name pointer array and a 64-byte temporary name buffer. `#pragma var_order` is useful here only after the real source-visible locals are distinguished from compiler-owned homes.
- A semantically redundant allocation temporary can be target-visible at `/Od`. The target stores the 0x10004-byte block allocation first in `blockCursor`, then copies it into the persistent `block` local. Writing `block = Alloc(...)` directly made the function exactly 14 bytes short.
- `do { if (!node) break; ... } while (limit)` was not byte-equivalent to the target score-list traversal. The target comes from `while (node != NULL) { ...; if (entry >= 10) break; }`, producing a direct near `je` at loop entry instead of a short inverse branch plus a near trampoline. Restoring that ownership fixed the final two-byte extent/direction difference.
- CP932 target text can be reconstructed portably with the same `\xNN` byte-literal convention used by generated `i18n.hpp`; this avoids host/source-codepage dependence while still letting VC7 pool ordinary string literals and emit canonical relocations.

### Constructor lowering and optimized integer scaling

- An array of non-trivial elements is not source-shape equivalent to adjacent named fields under VC7. `Effect::Effect @ 0x4287e0` needs nine distinct `Float3` members: `Float3 vectors[9]` collapses the nine target-visible constructor calls into `eh_vector_constructor_iterator`, while nine adjacent fields emit the exact individual call chain before the trailing `ZunTimer` constructor.
- Optimization pragmas are part of the source-shape contract. `GameManager::ScaleIntBasedOnRank @ 0x421ba0` only emits the target signed divide-by-32 correction (`cdq; and; add; sar`) inside the surrounding `#pragma optimize("t", on)` region; `/Od` emits `idiv` instead.
- Operand order remains target-visible even after strength reduction: spelling the final expression as `quotient + upper` lets VC7 use `add eax,[upper]`, while `upper + quotient` introduces an extra register move and changes the extent.

### Constructor families can recover object and global ABI together

- `Enemy::Enemy @ 0x42a280` shows how constructor lowering exposes the real aggregate shape. One `AnmVm` followed by an `AnmVm[2]`, one 0x228 context followed by a 16-element context array, distinct `Float3` fields, two `BulletSpawnDescriptor` members, a 96-row 0x1c trail array, a 194-row 0x1c textured-vertex array, and trailing timers naturally reproduce the target's individual calls and `eh_vector_constructor_iterator` sites. Flattening these into byte blobs loses the constructor call graph.
- A compiler-generated implicit constructor can be target-visible even without an explicit source declaration. `VertexTex1DiffuseXyzrhw` has a compiler-owned constructor because its `Float3` member is non-trivial; the existing AnmManager exact manifest already proves both the 0x14 untextured and 0x1c textured vertex constructor symbols fold to target address 0x40b580. The Enemy vertex array therefore needs the real 0x1c textured type, not a stride shim.
- Reconstructing `EnemyManager` as one `Enemy` plus `Enemy[481]`, a 0x30 target-observed region, sixteen 0x10 timeline rows, and the manager timer restores the target global size `0x9DCF10`. This also makes the following chain globals land at the correct relative offsets in the COFF data section, providing an independent ABI check beyond function-byte equality.

### Manager constructor arrays can reveal sentinel ownership

- `EffectManager::EffectManager @ 0x428740` proves the manager contains `Effect effects[654]` beginning at `+0x1C`, followed by five distinct `Effect` sentinel members. VC7 therefore emits one `eh_vector_constructor_iterator` for 654 rows and five individual constructor calls; one 659-element array is byte-different.
- The layout reaches `+0x8B03C`; its 0x20-byte tail gives the same `0x8B05C` extent independently observed in `ResetEffects`, so constructor lowering and reset extent cross-check the ABI.

### Collision predicates: preserve branch ownership and x87 operand order

- `Player::FUN_00451670 @ 0x451670` shows that `!(a <= b && c <= d && ...)` is not a byte-equivalent replacement for the target's direct separating-axis `a > b || c > d || ...` test. VC7 changes the x87 status mask and parity-branch direction even though the predicates are mathematically equivalent. Preserve the target comparison orientation instead of normalizing boolean algebra.
- Two independent early exits should remain two source `if` statements when the target has two distinct short trampolines. Combining `if (!active) continue; if (frame % interval != 0) continue;` into one `if (!active || frame % interval != 0)` made this function exactly two bytes short; splitting them restored the target 0x66E extent.
- Rotated AABB and circle tests are sensitive to comparison operand order. Writing the target's bound on the left (`-halfWidth > projectedRight`, `halfWidth < projectedLeft`, `radius * radius < distanceSquared`) restored the target x87 load order and `test ah`/`jp`/`jnp` sequence. Reversing the comparison while preserving semantics produced a different instruction stream.

### Large aggregate constructors can correct earlier field aliases

- `Player::Player @ 0x449ca0` proves the complete constructor-visible Player aggregate: `AnmVm @ +0x10`, two leading `Float3`s, `Float3[16]`, ten adjacent `Float3`s, `PlayerOptionState[4] @ +0x40c`, `PlayerBombState @ +0xfdc`, two 192-row collision pools, `PlayerShot[128] @ +0xbe838`, three 0x10 timeline rows, two late `Float3`s, and six `ZunTimer`s. This layout naturally emits the target 0x1a6-byte constructor with 30 relocations/26 calls.
- Constructor evidence can invalidate an earlier standalone field name without changing the observed offset. The old `Player::stateColor @ +0x200` alias lies inside the proven `AnmVm @ +0x10`; its real owner is `mainVm.color1`. Likewise the old `Player::frameStop @ +0xfdc` alias is the first dword of the proven `PlayerBombState`. Move callers to the real aggregate owner rather than creating overlapping fields.
- Cross-TU regressions are especially important after such a header lift. Keep object size and offsets fixed with `offsetof` assertions, then fresh-check consumers; a nested member expression should still lower to the same base relocation plus addend when the ABI is truly unchanged.

### Player bomb color/fade helpers

- VC7 keeps `ZunColor` byte-channel arithmetic source-visible at `/Od`. In `FUN_0040bc60 @ 0x40BC60`, computing each RGB channel as `0x80 - (0x80 - source.channel) * timer / 60` preserves the target `movzx`/integer-division sequence; replacing it with packed-color arithmetic or a helper changes register ownership and code shape.
- `ZunTimer` comparison/operator spelling matters even when every expression is mathematically an integer comparison. The target bomb fade helpers use the overloaded timer operators for `<`, `>=`, subtraction, and integer conversion, which produces distinct target-visible calls to `0x4066A0`, `0x40B8E0`, and `0x40D3B0`. Caching `timer.current` as an `i32` removes those calls and cannot match.
- Two visually identical screen-fade callbacks can differ only in literal channel initialization. `FUN_00412300` and `FUN_00412FA0` share the same control-flow/relocation skeleton and 0xCF extent; preserving separate lexical bodies lets VC7 keep their only semantic difference (white versus red `ZunColor`) without introducing an abstraction call not present in the target.

### Player bomb callback family: local ownership and repeated draw lanes

- For `#pragma var_order`, the first listed scalar/pointer receives the shallowest EBP slot in these VC7 `/Od` callbacks. `FUN_0040C820` initially emitted `workItem=-4 / i=-8 / vm=-C`; changing the pragma from `(workItem, i, vm)` to `(vm, i, workItem)` restored the target `vm=-4 / i=-8 / workItem=-C` without changing behavior.
- Signedness of an otherwise identical loop index is target-visible. The 16-entry work-item renderer at `0x40C820` uses signed `i32` and `jge`, while the 128-entry sibling at `0x40D010` uses `u32` and `jae`; preserving only the bound but not the source type changes the branch opcode.
- An apparently unused local can be required source shape. `FUN_0040DEE0` stores `&player->bombState.workItems[0]` in a stack local that is never consumed by the later C++ logic; removing that assignment changes the 0x38-byte frame/slot layout. Keep target-visible `/Od` locals until strict comparison proves they are compiler-owned noise.
- Float algebra is not freely interchangeable under VC7 x87 lowering. In `FUN_0040E610`, `angleStep + angleStep` emits a 3-byte memory `fadd`, making the function one byte too large; `angleStep * 2.0f` reuses the loaded x87 value and emits the target 2-byte `fadd st,st`.
- Repeated Player bomb draw callbacks often differ only by loop count, mix color, or whether the VM position is `anchor` versus `anchor + pos2`. Preserve separate lexical functions instead of abstracting them behind a shared helper: the target callback table points directly at these bodies, and an abstraction call would be target-visible.

### Player bomb callback family: shared initializer ownership

- A two-scalar copy can be six bytes larger than the target even when both fields are adjacent. In `Player::FUN_0040BF00`, assigning the two scale components separately reloads the effect pointer twice; assigning `Float2` as one aggregate emits the target paired loads/stores and shrinks the function from 0x10E to the exact 0x108.
- Reusing a cached subobject pointer is not always source-equivalent. `FUN_0040E3B0` uses cached `bomb` for its first timer checks, but the later modulo-four condition reloads `player->bombState.timer` through the full `+0xFF4` displacement twice. Reusing `bomb->timer` made the function exactly six bytes short (two short-displacement instructions instead of two long-displacement instructions).
- Equivalent member addresses can still encode different ownership. The first five script setup calls in `FUN_0040E3B0` are emitted from `bomb + {0x204,0x4A8,...}`, not from cached `workItem + {0x1B8,0x45C,...}`. Preserve the source base object selected by the target even when the final address is identical.
- Target-visible unused locals can carry semantic reconstruction clues. The same callback stores `-PI` in a local that is not consumed in that body; retaining it is necessary for the target frame/slot layout and may reflect a shared source template with sibling bomb callbacks.

### Player bomb burst callbacks: constructor timing and float-constant precision

- A non-trivial local's declaration point is code-generation-visible even when its final stack slot is already correct. In `FUN_00410C40`, declaring each `Float3 velocity(...)` at the top of its branch emitted the constructor before the target's common-init/collision calls. Moving the declaration to the exact point where the target constructs the vector fixed the first-difference without changing the 0x5C frame.
- Block-local pointer declaration order can invert shallow/deep stack ownership around a non-trivial local. The 10/20/30-frame burst blocks require the spawned-effect pointer at the shallower slot and the collision pointer below it; matching that required preserving the target lexical declaration order rather than assigning both through a shared temporary.
- Float macro algebra can lose a target ULP at compile time. `ZUN_PI` is explicitly narrowed to `f32`, so `ZUN_PI * 5.0f / 8.0f` folds to `0x3FFB53D1`; the shipped target stores `0x3FFB53D2`, the correctly rounded high-precision 5π/8 value. Where the target proves the stored float, use a float literal that rounds to that exact mathematical value rather than forcing arithmetic through an already-narrowed macro.
- A parameter can be target-visible purely through calling convention. `EffectManager::FUN_004259e0` never reads its fifth source parameter, but retaining it is required for the target six-stack-argument `ret 0x18`; deleting the apparently unused parameter would change the ABI even if the body stayed equivalent.

### Effect-strip vertices and bomb effect callback families

- `while` and `for` are not interchangeable at VC7 `/Od` even when their update expressions are identical. `AnmManager::FUN_00464b00` was four bytes short as two `while` loops; spelling the two odd/even vertex walks as `for` loops restored one two-byte entry trampoline per loop. The loop preheader order is also visible: target emits `i = N; currentY = y; jmp condition`, not `currentY = y; for (i = N; ...)`.
- A block-scope `#pragma var_order(position, radius)` can place a later-declared class local and an earlier scalar into the target slots without moving the class constructor. `FUN_004114e0` and `FUN_004117b0` require `Float3 position` immediately after the radius calculation while physically assigning the three-vector above the scalar on the stack; function-scope ordering cannot express that shape.
- Copy-initialized trivial `Float3` locals can be target-visible without a default-constructor call. The 139-byte effect initializer wrappers copy `effect+0x2A4` and `effect+0x2B0` into two 12-byte locals; `#pragma var_order(velocity, position)` reverses their physical slots while preserving the target's lexical copy order.
- Do not cache repeated `effect+0x338` timer access merely to shorten source. In `FUN_004114e0`, a `ZunTimer *timer` cache made the function 18 bytes too short. The target reloads the full displacement for each comparison/conversion, and that repeated address formation is part of the source-shape evidence.
- A ternary feeding one helper call can intentionally create a compiler-owned float stack temp. `FUN_004117b0` needs `AddNormalizeAngle(angle, (index & 1) ? +delta : -delta)` so VC7 materializes the selected delta once and then issues one call; duplicating the call in `if/else` branches changes both frame ownership and control flow.

### Player bomb slash callback: field-width and branch-local state

- Target-observed member width can explain an entire extent mismatch. `FUN_00411B10` was 21 bytes short because seven `slot+0x38` stores were initially interpreted as byte `mode` writes at `+0x3D`; the target uses dword `collisionInterval` writes at `+0x38`. Each `mov byte` was three bytes shorter than the target `mov dword`, giving exactly `7 * 3 = 21` bytes.
- Mutually exclusive branch-local `Float3 position = player->position` declarations can intentionally occupy distinct stack slots under VC7 `/Od`. The 70/80/90/100-frame branches in `FUN_00411B10` naturally allocate four separate 12-byte locals, which in turn leaves the following six compiler-owned `Float3` expression temporaries at the target offsets.
- For chained `Float3` interpolation, preserve the exact operand direction. The outbound move is `(workItem->position - workItem->anchor) * t + workItem->anchor`; the return move is `(workItem->anchor - workItem->position) * t + workItem->position`. Algebraically rearranging either expression changes hidden return-buffer ownership and the call sequence of `operator-`, `operator*`, and `operator+`.

### Large Player orbit/slash callback templates

- x87 branch ownership must match the target arm direction, not merely the predicate. In `FUN_00413140`, spelling the outer radius test as `if (radius < 500) { update } else { deactivate }` emitted `test ah,5 / jp`; the target comes from `if (radius >= 500) { deactivate } else { update }`, which emits `test ah,1` and the target short branch.
- A `continue` can be target-visible even when falling through the end of an `if/else` reaches the same loop update. The radius-expired arm in `FUN_00413140` must explicitly `continue`; otherwise VC7 jumps to the lexical loop tail first, changing only the four-byte branch destination while keeping the function extent identical.
- Recovering a few fields can unlock multiple multi-kilobyte bodies. Naming `PlayerBombWorkItem::rotationStep @ +0x8` and `effect @ +0x16D8` let `FUN_00413140` and `FUN_00413990` share the target's 128-row orbit update source shape instead of relying on raw byte offsets.
- `ZunTimer::FUN_0040ebc0` is the periodic-event predicate `current != previous && current % interval == 0`. Keeping it as the original member call preserves the target call boundary in `FUN_00413990`; open-coding the modulo in each consumer would change both caller extent and timer access ownership.
- A `break` before a `for` update can intentionally preserve the current iterator pointer for code after the loop. `FUN_00413990` fills at most 16 free work items and then passes `workItem->anchor.x` to the positioned-sound call; rewriting this as a counter-controlled loop that advances the pointer before exit changes the target pointer value and the emitted control flow.
- Sibling callback bodies can legitimately duplicate several kilobytes. `FUN_00413140` and `FUN_00413990` share the same four 16-entry initialization lanes and 128-entry runtime lane, but the latter adds a periodic 16-slot burst. Keeping the repeated source lexical structure matches the target; factoring it into a helper would introduce calls absent from the shipped binary.

### Effect interpolation setters and compiler-owned scalar temporaries

- A target stack slot that receives a ternary integer only to feed `fild` need not correspond to a source local. In `AnmVm::FUN_0040EB50`, declaring an explicit `offset` forced the scalar into `[ebp-4]` and displaced the compiler-owned `this` home to `[ebp-8]`. Inlining `(timerChanged & 1) ? 8 : 0` into the float expression lets VC7 keep `this=[ebp-4]` and materialize the integer conversion temp at `[ebp-8]`, exactly matching the target.
- The explicit `& 1` before a boolean ternary is machine-visible. Without it VC7 emits `neg; sbb; and 8`; the target includes `and eax,1` first. Preserve low-bit masking when the target proves it even if the helper currently returns only zero/one.
- Small interpolation setter families are best recovered as real `AnmVm` members rather than open-coded raw writes in each consumer. `FUN_0040EC30/ECA0/ED50/EDA0` share the same two-timer-plus-mode-byte pattern but write different aggregate/color payloads; keeping four separate member bodies preserves their target call boundaries and argument cleanup (`ret 0x10`).
- RGB unpack order is target-visible. `FUN_0040ECA0` writes red, green, blue as independent byte stores extracted from the same dword, in the exact order target uses; replacing that with a packed color assignment would change both shifts and store order.

### Branch-local vector lifetime and indexed-owner recovery in Player bomb callbacks

- `FUN_0040C010` shows that mutually exclusive class-valued locals must retain branch-local lifetime under VC7 `/Od`. Hoisting the `<40` `previousPosition` and the later `targetPosition` to function scope enlarged the frame from target `0x5C` to `0x74` and inserted two entry constructors. Keeping them inside their respective branches restores the target slots (`previousPosition` at `-0x1C`, `targetPosition` at `-0x34`) and allows the compiler to reuse the frame.
- Equivalent pointer ownership remains codegen-visible. In the initialization loop, `bomb->workItems[i].rotation = angle` emits the target `imul i,0x16F0` indexed path; spelling the same store through cached `workItem->rotation` removes ten bytes. Preserve the target's source owner even when both expressions address the same member.
- A direct target `sqrtf` result store to `workItem + 0x0C` identified that location as persistent state (`PlayerBombWorkItem::speed`), not a disposable local. Promoting the field in the real row type fixed both later source shape and cross-callback ABI without changing `sizeof(PlayerBombWorkItem)`.
- Positive/negative branch ownership still matters after semantics are known: `if (!workItem->active) continue;` emits the target short inverse branch plus short loop jump, whereas wrapping the large body in `if (workItem->active)` forces a six-byte near branch. Likewise, target-equivalent finite comparisons (`tail.x > -100.0f` vs an inverted `<=` form, and `speed > 10 ? 10 : speed` vs the algebraically equivalent inverse ternary) produce different x87 masks/arms.

### Function-scope non-trivial locals and duplicated compound-assignment arms

- `FUN_0040C910` demonstrates that a class-valued local declared later in a function can still take a shallower physical stack slot than earlier scalar locals unless it is included in the function-level `#pragma var_order`. The first exact-sized build put `previousPosition` at `-0x0C` and shifted `i/bomb/workItem/angle` down by 12 bytes. Adding `previousPosition` after those four names restores target `i=-4`, `bomb=-8`, `workItem=-0x0C`, `angle=-0x10`, and `previousPosition=-0x1C` without moving its constructor call.
- Do not merge branch-owned compound assignments into a ternary merely because both arms update the same lvalue. In the same callback, target `if (i & 1) rotationStep += 1.2f; else rotationStep += 2.4f;` keeps two independent load/add/store sequences. `rotationStep += (i & 1) ? 1.2f : 2.4f` shares the destination update and makes VC7 `/Od` seven bytes short. Restoring the lexical arms changes the emitted extent from `0x6F7` to the exact `0x6FE`.
- A small target data table referenced only by one callback is still a real relocation owner. The seven packed Dream Seal colors at `0x004C6100` are represented as `g_PlayerDreamSealColors[7]` and recorded in `reccmp-globals.csv`; do not compensate its relocation by inventing a nearby spell-card-table addend.

### Sibling timer-state callbacks and block-local class ordering

- `FUN_0040D430` and `FUN_0040D970` share one source-shaped 60-frame quadratic interpolation: a branch-local `Float3 target(192,224,0)`, `interp = (float)timer / 60`, `interp *= interp`, then `(target - savedAnchor) * interp + savedAnchor`. Keeping the class local inside the `<60` branch naturally leaves the three `Float3` operator return buffers compiler-owned and produces the target `0x58`/`0x64` frames.
- Mutually exclusive event arms can intentionally allocate distinct otherwise-unused effect pointers. The 100/110/120/130 and 130/140/150/160 lanes store `SpawnEffect` returns to separate stack slots even though the values are never consumed. Preserve those branch-local declarations instead of deleting dead-looking locals or sharing one pointer across the event chain.
- When a block has both scalar pointers and a later-constructed class local, list all of them in the block `#pragma var_order` if target slots require it. In the 120-frame `FUN_0040D970` burst, `(effect, damageSlot, burstPosition)` yields `effect=-0x1C`, `damageSlot=-0x20`, `burstPosition=-0x2C..-0x24` while the `Float3` constructor still executes only after the first three effect spawns. Listing only the pointers made the later class local steal the shallow slots.
- `PlayerOptionState +0x2C8` is a target-observed state dword: both sibling callbacks set it to `1` at their terminal event before assigning the row timer to zero. It is named conservatively as `state2C8`; the exact offset/width is proven, while higher-level gameplay semantics remain unclaimed.

### Fixed-size zeroing and mixed block-local aggregate ordering

- `FUN_0040E780` distinguishes three explicit zero component assignments from `memset` even for a 12-byte `Float3`. VC7 `/Oi` lowers `memset(&v, 0, 12)` to `xor eax,eax` plus three stores, while the shipped target has three independent immediate-zero stores. Writing `v.x = v.y = v.z = 0.0f` as separate statements restores exactly ten authored bytes without padding.
- One block can require physical stack order different from declaration/constructor order across mixed aggregate types. The periodic effect block is exact with `#pragma var_order(effect, position1, position0, scale1, scale0)`: the pointer occupies `-0x10`, the second-declared `Float3` occupies `-0x1C`, the first-declared `Float3` occupies `-0x28`, then two `Float2`s occupy `-0x30/-0x38`, while VC7 still executes the two `Float3` constructors in lexical declaration order.
- The collision tail in the same function independently requires `#pragma var_order(slot, position)`, giving the scalar collision pointer `-0x3C` and the later class-valued position `-0x48`. Block-local ordering is preferable to moving declarations when constructor timing already matches the target.

### Symmetric multi-option bomb templates and lexical sibling differences

- `FUN_0040EE10` and `FUN_0040F570` are two 0x73D-byte four-option callbacks whose 0xE4 frames come naturally from four lexical `Float3` interpolation expressions, not a loop. Each expression owns three compiler return buffers; replacing the four source copies with an indexed loop would destroy both the frame and call-site layout.
- In the `<60` interpolation block, the source-visible scalar must own the shallow slot even though a non-trivial `Float3` is declared after it. `#pragma var_order(interp, position)` gives target `interp=-0x0C` and `position=-0x18..-0x10` while preserving the constructor after the timer calculation.
- Equal-size sibling functions can still differ in target-visible lexical placement. The normal Remilia Bomb fills all four option targets before creating its player-centered cancel circle; the Last Spell declares the second-phase `Float3`, creates the cancel circle, and only then copies the player position into that local. Preserve these separate bodies instead of factoring a shared helper.
- The two siblings also preserve gameplay constants directly in codegen: movement scale `2.0f` versus `3.0f`, periodic interpolation end-scale `64.0f` versus `128.0f`, and terminal events at frames `239` versus `279`. Treat a matched template as a source-shape scaffold, not permission to normalize sibling constants or statement order.

### Parent-vs-subobject ownership and event-loop branch topology

- `FUN_0040FCD0` shows that the same `PlayerBombState` data may deliberately use different source owners in adjacent phases. Initialization forms `workItem = bomb->workItems` with an 8-bit `+0x4C`, while the knife-spawn phase forms `workItem = player->bombState.workItems` with target 32-bit `+0x1028`. Its per-knife event test likewise reloads `player->bombState.timer` through `+0xFF4` instead of the cached `bomb + 0x18`. Preserve the target owner even when both expressions name the same address.
- A sparse event loop whose body is large is target-shaped as `if (!justReached) continue; if (active) return; body`. VC7 emits a short conditional plus short jump to the loop update. Wrapping the body in `if (justReached) { ... }` emits a six-byte near branch over the large body and changes the function extent.
- The knife movement phase is physically laid out as the movement path first and the pure-rotation path later. Writing `if (timer < 30 || timer >= 70) { movement } else { rotation-only }` reproduces target branch placement; the equivalent positive conjunction `30 <= timer && timer < 70` places the rotation body first and changes near/short branch ownership.
- Sibling knife callbacks preserve source-owner differences as well as constants. The normal Bomb hit effect intentionally reindexes `bomb->workItems[i].anchor`, producing an `imul i,0x16F0`, while the Last Spell uses cached `workItem->anchor`. Do not normalize equivalent member addresses across sibling bodies.

### Small effect easing callbacks: preserve staged x87 updates

- The Player-owned effect callbacks at `0x40E040`, `0x40E120`, `0x40E200`, and `0x40E2D0` share a source-shaped easing sequence `interp = 1 - timer/40; interp *= interp; interp = 1 - interp`. Keeping those assignments separate reproduces the target's `fst/fmul/fstp` sequence and the two-dword frame (`interp=-4`, fastcall receiver home `-8`).
- `FUN_00413070` extends the same rule to a quartic curve: after normalizing `(timer - 30) / 30`, two lexical `interp *= interp` statements intentionally produce `t^2` and then `t^4`, with a target-visible store between them. Replacing this with `powf`, `interp*interp*interp*interp`, or algebraic folding changes the x87 spill pattern.
- A valid shipped function boundary does not require IDA to have created a function object. The four callbacks at `0x40E040..0x40E2D0` each have independent prologue/return pairs and CC padding in the canonical executable even though the current IDA database fails to decompile `0x40E040`. Boundary acceptance comes from shipped bytes plus exact COFF comparison, not database function metadata alone.

### Player option-state ABI and compiler-owned switch temporaries

- The option-update family proves the `PlayerOptionState` tail layout: `state2C8 @ +0x2C8`, `substate2CC @ +0x2CC`, `optionIndex @ +0x2D0`, `orbitAngle @ +0x2D8`, `timer @ +0x2E0`, update callback `+0x2EC`, and render callback `+0x2F0`. `Player +0xE2ABC` is the current `Enemy*` option homing target; the already-exact homing helpers independently dereference its `Enemy +0x2D88` position and gate it with `timerE2AC4`.
- Do not introduce named locals for a switch expression merely because the decompiler names one. `FUN_0044E3A0` is exact only when both switches read `option->state2C8` / `option->substate2CC` directly. VC7 then materializes its own `-0x0C/-0x10` switch temporaries. Named `state` and `substate` locals caused a second copy of each, enlarged the frame from `0x10` to `0x18`, and made the authored body 12 bytes longer before other differences.
- A nested switch default that reaches the shared function epilogue should remain `break`, not `return 0`. In `FUN_0044E3A0`, `default: return 0` emitted a local `xor eax,eax` plus jump; `default: break` lets all paths share the final zero return and completes the exact 17-byte correction.
- `FUN_0044E3A0` has a 0x3B5 authored body followed by one 16-byte four-entry compiler jump table (`compare_size=0x3C5`). `FUN_0044EB70` has a 0x2DB authored body followed by two independent 16-byte tables (`compare_size=0x2FB`) because the original source contains two lexical `switch(optionIndex)` statements. Verify all table relocations but count only authored code.
- Fallthrough is target-visible in option state machines. `FUN_0044EA40` state 1 initializes script/state and intentionally falls through into state 2 positioning; `FUN_0044EB70` state 1 initializes its per-slot target/angle and falls through into the state 2 orbit update. Avoid duplicating the shared body or inserting a helper call.

### Effect trail geometry: transposed UV strips and constructor timing

- The effect-trail family at `0x4272e0..0x427b50` gives a consistent target-observed tail layout for the 0x360-byte `Effect`: `+0x314/+0x318/+0x320` feed the primary radius/angle/strip-width geometry, `+0x324` is the segment count, `+0x32c/+0x330/+0x334` select and parameterize ellipse/phase modes, `+0x34c` is the custom draw callback, `+0x356` is the geometry-dirty byte, and `+0x358` owns the allocated textured-vertex buffer. These names describe the proven trail mode; do not assume every effect type gives the overlapping storage the same semantics.
- `AnmManager::FUN_004649a0` and `FUN_00464b00` are source-shaped siblings. Both use two lexical odd/even `for` walks over 0x1c-byte textured vertices; `0x4649a0` decrements U while holding V at the sprite's start/end edges, whereas `0x464b00` decrements V while holding U at the two edges. Preserve the `for` spelling: in this VC7 `/Od` family, replacing the walks with equivalent `while` loops changes the entry trampoline bytes.
- `#pragma var_order` fixes physical slots but not the lifetime point of a non-trivial local. In `FUN_00427450`, the dead branch-local `Float3` belongs at `-0x4c`, but the target constructs it only after storing the two phase angles and computing the phase step. Declaring it with the scalar locals kept the same frame and slot yet moved the constructor call earlier; moving only the lexical declaration point removed the final 49 byte differences.

### Effect camera-relative initializers and canonical aggregate owners

- Target absolute references inside Effect callbacks should be reconciled against nearby aggregate bases before inventing globals. The camera vectors at `0x4EA3C4`, `0x4EA3D0`, and `0x4EA3E8` are exactly `g_Background + 0x6394/+0x63a0/+0x63b8`, i.e. `g_Background.unk6394.vectors[0/1/3]`. Expressing them through the real `Background` aggregate makes the COFF relocation target `g_Background` with the corresponding addend and reproduces the shipped bytes in the `0x426280`, `0x426720`, and `0x426e70` random initializers.
- The small timer-driven Effect updates at `0x426bb0`, `0x426c90`, and `0x4271a0` naturally produce their target 0x20/0x2c frames from class-valued arithmetic temporaries. Keep expressions such as `vector6 * alpha + vector5` and `vector6 * alpha * 128.0f + vector5` intact instead of naming the intermediate `Float3`s; explicit source temporaries move the compiler-owned hidden return buffers.
- `FUN_004270c0` confirms that a seemingly redundant trailing `+ 0.0f` can be target-visible even in random scalar setup: `GetRandomF32InRange(1.5f) + 0.0f` preserves the target float-zero relocation before `Float3::operator*=`. Do not remove neutral floating terms until strict comparison proves they were folded in the original source.

### Tiny accessors: avoid convenience pointer homes

- `FUN_0042bc50 @ 0x42bc50` is exact only when its three stores operate directly from the fastcall receiver. Caching `reinterpret_cast<u8 *>(self)` in a local enlarges the target 0x32-byte helper to 0x3a by adding an extra stack home. For tiny `/Od` accessors and bit-manipulation helpers, start from repeated direct receiver expressions and introduce a pointer local only if the shipped frame proves one exists.
- `EclManager::GetTimelineCount/GetTimeline @ 0x42dfb0/0x42dfd0` independently prove the ECL header word at `+0x06` is the timeline count and the relocated timeline pointer table begins at `+0x08`. Keep the underlying header layout stable for claimed ECL interpreter work; a semantic accessor can expose the proven meaning without forcing an immediate shared-field rename.

### Enemy contact and motion branch ownership

- `Enemy::FUN_0042c290 @ 0x42c290` uses one source-visible `Float3 collisionSize` plus two compiler-owned return buffers for `size / 0.7f` and `size / 1.5f`. The route/attachment gate is not byte-equivalent as one OR expression: the target calls `HasAttachedEnemy()` only for route ids 0/4 and uses a short `je` into the collision body followed by a near jump to the epilogue when an attached enemy exists. Preserve that nested early-return ownership.
- `EnemyManager::FUN_0042c3b0` preserves the arithmetic grouping `GetLives() * 4 * 60`, which VC7 lowers to `shl eax,2; imul eax,eax,0x3c`; replacing it with `* 240` changes the target instruction shape. Its dialog gate likewise matches as one outer `if (!IsDialogPresent())` block rather than an explicit early return.
- `Enemy::FUN_0042deb0` places the normal X integration block before the mirrored-X block: spell bit 18 is tested as `if (bit == 0) add; else subtract`. Reversing the lexical arms leaves semantics unchanged but swaps the target `jne` and the physical `fadd/fsubr` opcodes.

### Enemy phase transitions: shared return blocks and tiny-member TU ownership

- `Enemy::FUN_0042b930 @ 0x42b930` only reproduces its 0x31c extent when the timeout test owns the whole success body: `if (timer >= timeout) { ... return 1; } return 0;`. An equivalent explicit early `return 0` generated a local zero-return/trampoline and made the function one byte short. The shipped function uses one six-byte near `je` from the timer gate directly to the shared final zero-return block.
- A tiny member's translation-unit owner can be visible in its epilogue. `Gui::FUN_0042f340 @ 0x42f340` compiled in the `/Os` Gui TU as a 0x14-byte function ending in `leave; ret 4`; the shipped 0x16-byte body ends in `mov esp,ebp; pop ebp; ret 4`. Defining the same ordinary C++ member in the neighboring `/Od` EnemyManager TU naturally emitted the exact body. Use address neighborhood and compiler epilogue shape as TU evidence before trying padding or assembly.
- `g_EnemyManagerUpdatePlayerTimer @ 0x018B89EC` is a standard production `ZunTimer`. Phase-change code in `FUN_0042b490/0042b930` writes it directly when forcing the player transition; promote the storage from probe-only knowledge to the production EnemyManager owner and record the global in the canonical ledger rather than hard-coding the address.


### Replay callbacks: hidden new temporaries, legacy arithmetic, and caller-proven ABI

- `ReplayManager::AddedCallback @ 0x452830` shows that an explicit source local around `new T` can duplicate VC7's compiler-owned new-expression temporary. Writing `g_ZunMemory.AddToRegistry(new ReplayData, sizeof(ReplayData), "ReplayDataInf")` directly gives the target `0x24` frame; caching `new ReplayData` in a named local adds one `mov local,local`, expands the frame to `0x28`, and makes the function six bytes long. Preserve the direct allocation expression when the target already exhibits the hidden temporary.
- `ReplayManager::AddedCallbackDemo @ 0x452D60` preserves an adjacent-generation replay leftover rather than simplified arithmetic: the replay shot byte is assigned through `/ 1`, `% 1`, and then overwritten by the original byte. Under VC7 `/Od`, signed `% 1` deliberately emits the target `and 0x80000000 / dec / or -1 / inc` sequence. Algebraically removing the first two stores destroys target-visible source history. TH06/TH07 can identify this family, but TH08 bytes remain the acceptance authority.
- A caller can prove a wider ABI than the callee body alone. `GameManager::SetClockTime @ 0x453C60` stores only the low byte, so its 25-byte body is identical for a narrow or `i32` parameter. The exact replay caller at `0x452FDB` first sign-extends the stored `i8` clock byte with `movsx` and pushes the resulting `int`; declaring `SetClockTime(i32)` makes both caller and callee exact. Prefer caller+callee evidence over inferring parameter width from the callee's final store.
- The extended replay stream at `ReplayManager +0x78` is independently constrained as a six-byte record: `AddedCallback` aliases it to the input stream, writes words at `+0/+2/+4`, and playback advances it by six bytes. Keep this layout typed as a three-word record even when a remaining callback has a register-allocation mismatch. A one-byte difference caused solely by `and r32,imm32` versus accumulator-form `and eax,imm32` is not permission to add padding or assembly; preserve the semantic reconstruction and continue compiler-shape inversion.
- `Gui::FUN_00437dc7 @ 0x437DC7` proves `GuiMsgVm +0x1568` is a one-byte state consumed by replay frame throttling. Promote the byte inside the real `GuiMsgVm` aggregate instead of hard-coding `g_Gui.impl + 0x1568`; the tiny helper is exact in the `/Os` Gui TU and gives later replay code a canonical owner.

### Gui added-callback: preserve table rank, branch-local validation, and real receiver owners

- `Gui::ActualAddedCallback @ 0x4390EE` proves three resource tables as real relocation owners: the 12-entry loading portrait table at `0x004C72C4`, the 9-entry stage-text ANM table at `0x004C745C`, and the 9x12 message-path table at `0x004C74C0`. Preserve the message table as a true two-dimensional array. Flattening `[stage][shot]` into `stage * 12 + shot` changes VC7 `/Os` from the target `imul stage,0x30` plus scaled column index into an `imul stage,0x0c`/add sequence even though the selected pointer is identical.
- Do not hoist identical validation out of mutually exclusive resource-load branches when the target duplicates it. The stage-text normal and spell-practice paths each assign `stageTextAnm`, immediately test that branch's result for NULL, and return `ZUN_ERROR` locally. Sharing one NULL check after the two assignments made the function 17 bytes short; restoring the two lexical validation blocks reproduced the target branch extents.
- Receiver ownership is target-visible even for the same object. The common stage-background disable helper is called as `g_Gui.FUN_004390d6()`, which emits the target five-byte immediate ECX load; `this->FUN_004390d6()` uses the existing stack home and makes the caller two bytes short. Likewise the `vm34d4` setup belongs to `timesAnm @ Gui+0x14`, not `stageTextAnm @ +0x10`; the two exact call sites distinguish those semantic owners.
- Recover anonymous `AnmVm` state through already-exact accessors before naming it in a large loop. `AnmVm::GetIntVar` proves `counterVar0 @ +0x120`; using that field in the 14x12 `vm5728` grid reproduces target writes at `GuiImpl + 0x5848`. `intVar0 @ +0x100` is address-equivalent only to a different member and produces the wrong offset.
- In this `/Os` Gui function, `#pragma var_order(i, j, k)` yields the target physical homes `i=-0x4`, `j=-0x8`, `k=-0xC`. The grid parity is lexical bit arithmetic `((i + j) & 1) + 3`; `% 2` introduces signed division machinery and changes the body despite identical non-negative loop values.

### Enemy timeline/spawn: split non-trivial lifetime from scalar slots

- `EclTimeline::Run @ 0x0042A8A0` is a useful VC7 `/Od` example where constructor timing and physical local ownership must be solved separately. The five `D3DXVECTOR3` positions are branch-local non-trivial objects: declaring them at function scope hoists all five default constructors into the prologue, while the target constructs each vector only inside the opcode arm that uses it. Keep those objects branch-local even when related scalar locals need longer lifetimes.
- `#pragma var_order` is not a reliable way to place names whose lifetime exists only inside a case block. Natural block lifetime, nested scopes, and `for`-initializer locals controlled the target homes more reliably. The random-play arm needed the argument cursor outside a nested vector scope, which produced `args=-0x4C` and the vector at `-0x58` without padding.
- When two related branch locals need a fixed adjacent layout, a real POD work aggregate can be the source shape rather than two independent locals. The special-spawn arm only matched when `{ Enemy *spawned; i32 *args; }` was one 8-byte local aggregate, giving target homes `spawned=-0x2C`, `args=-0x28` while the vector remained branch-local.
- Lexically equivalent loop conditions can change the authored body boundary. Timeline opcode 14 needs `if (slot >= 0) continue; slot = value;`; rewriting it as `if (slot < 0) slot = value;` removes a short branch and makes the body two bytes short. In this function those two bytes are exactly what place the 17-entry switch table at target `0x0042AD60`.
- Treat a compiler switch table as associated data when the target authored function ends before it. `EclTimeline::Run` has a `0x4C0` authored body and an immediately following `0x44` table with 17 relocations. Canonical validation uses `size = 0x4C0` plus `compare_size = 0x504`: all table bytes/relocations are verified, but the table is excluded from authored-byte progress.

### Enemy spawn: typed aggregate copies and true bitfield RMW

- `EnemyManager::SpawnEnemy1/SpawnEnemy2 @ 0x0042A4E0/0x0042A680` prove that large `memcpy` calls can have the right semantics but the wrong VC7 register ownership. Assigning a typed aggregate of `sizeof(Enemy)` reproduces the target `mov edi(dst); mov esi(src); mov ecx,count; rep movsd`. The `0x78` ECL-context copy in SpawnEnemy2 is a second shape: typed aggregate assignment produces target `mov esi(src); mov edi(dst); add edi,0x810; mov ecx,0x1E; rep movsd`.
- Enemy `+0x3324` writes are genuine bitfield read/modify/write operations. Modeling the active bit and spawn-variant bit as one-bit fields reproduces target accumulator/register ownership; hand-written masks and ORs were address/semantic equivalent but non-exact.
- Do not force a reconstructed dependency into the production link before its owner closure is ready. Spawn/timeline are canonical in `build/probes/EnemyTimeline.obj` while `EclManager::RunEcl` still has probe-only dependencies. The normal executable remains link-clean; the exact probe holds the real source implementation, not a forwarding shim. Promote the dependency group together once the remaining ECL owners are reconstructed.

### Exact inventory: overloaded authored names are range identities

- `reccmp-functions.csv` can legitimately contain more than one authored function with the same logical name. TH08 currently has both `Float3::Float3 @ 0x00404720` and `Float3::Float3 @ 0x0040B460`. Exact evidence is an address/extent claim, not a globally unique-name claim. Progress/tracking code must therefore bind an exact row to the authored inventory by target address (and size), then validate the logical name at that address; a dictionary keyed only by `name` silently picks one overload and rejects the other.
- Small `__fastcall` ECL adapters are also source-shape sensitive to argument caching. `StartEnemySpell @ 0x00421280` has only the two incoming ECX/EDX homes. Caching `instruction` in a third local expands the frame from target 8 to 12 and adds exactly 7 bytes. Re-read the fastcall argument directly when the target repeatedly materializes the same parameter home.

### ECL tail motion/shoot helpers: lexical lifetime and guard-return topology

- `Enemy::FUN_00422c40` proves that two observable default constructors do not imply overlapping class-local lifetimes.  VC7 `/Od` assigns the target slots only when the ctor-only legacy `Float3` lives in its own completed block and the used polar-velocity `Float3` is declared afterward.  Hoisting both into one scope swaps their stack slots; wrapping them in a synthetic aggregate creates a new constructor and is not source-equivalent.  Prefer sequential non-overlapping lexical scopes over padding or fake fields when the target proves both ctor calls.
- When a conditional expression feeds a narrow destination, preserve the conditional's natural wider type until the final store.  In `EclRunHighProposal::DispatchShotInstruction`, casting both `ResolveInt` arms to `i16` made VC7 allocate word temporaries.  Leaving both arms as `i32` lets the compiler merge them in the target dword slots and only then emit the low-word descriptor store.
- Multiple guard conditions can have target-visible return sharing.  The shot helper is exact as one `if (rejectHuman || rejectYoukai) return;` followed by one independent radius guard.  Splitting the two flag rejects into separate returns duplicates a five-byte epilogue jump; spelling the inverse as one large positive `if` instead creates long conditional branches.  Preserve the original guard grouping when the target has one local reject trampoline.
- Float guard polarity is ABI-visible through x87 status masks.  `if (radius > 0.0f && distanceSquared < radius) return;` emits the target `test ah,0x41` / `test ah,0x05` pattern and also preserves unordered/NaN behavior.  Rewriting the condition as ordered `radius <= 0 || distance >= radius` changes the condition-code sequence even though normal finite inputs look equivalent.
- TH06/TH07 ECL sources are useful lexical ancestors, not byte evidence.  The TH06 movement-mode block and TH07 `SpawnBulletVariant` exposed the original field/update order for the TH08 helpers, while TH08 target disassembly remained authoritative for new flags, offsets, branch guards, and exact acceptance.

### ECL boundary/interpolation helpers: x87 operand ownership and loop-exit topology

- For VC7 `/Od` floating comparisons that involve a conversion/operator call, algebraically equivalent operand reversal is not codegen-equivalent.  `position.x < bound + margin` keeps the `Float3::operator float*()` call first and compares the returned member directly; `bound + margin > position.x` makes the compiler spill the x87 value to a temporary.  Preserve the target-observed lexical left/right ownership instead of normalizing inequalities.
- A typed member expression can matter independently of the comparison operator.  In the periodic-X move helper, `playerX < enemy->vector2d34.x` produced the target `fld player / fcomp enemy`; the same condition through a raw `u8* + 0x2D34` load was canonicalized to the opposite x87 owner.  Prefer proven struct members when their AST shape explains register/FPU ownership.
- For direct-vs-wrapped distance selection, preserve the subtraction and comparison order (`directDistance < wrappedDistance`).  Rewriting it as the equivalent `wrappedDistance <= directDistance` changes the x87 stack order and status-mask branch sequence even with identical finite semantics.
- Large/small branch polarity is byte-visible.  The TH08 boundary helpers require `if (duration <= 0) { immediate body } else { timed helper }`: this leaves the large immediate path as fallthrough and places the short timed call at the tail.  Reversing the condition changes short/near jump selection.
- Loop bodies with an install-and-exit path may require rejection-first spelling.  `InstallInterpolationSlot` matches only as `if (occupied && affected != raw) continue; ... install ...; break;`.  The positive form `if (empty || affected == raw) { ...; return; }` emits a long reject branch plus an explicit epilogue jump and grows the function by 9 bytes.
- The eight-entry interpolation callback table is target-owned data at `0x004C6C90`, immediately before `g_EclExInsn @ 0x004C6CB0`.  Keep these as distinct owners; the interpolation installer indexes the former by slot callback index.

### ECL compare/context/ANM helpers: switch label placement, repeated reads, and aggregate owners

- A switch's shared labels may need to live lexically inside the switch.  `CompareOperands` is exact only when the shared success block appears after the last case but before `default`, with `default` and the failure label co-located afterward.  Moving those labels outside the switch makes VC7 synthesize a two-byte default trampoline; turning default into a direct return creates a separate NULL-return block.  Source label placement can therefore change the switch bounds-check target even when all case bodies are identical.
- For a large repeated compare dispatcher, do not introduce `lhs`, `rhs`, or result locals unless the target proves them.  The 12 `CompareOperands` cases deliberately spell their operand ternaries independently; VC7 allocates 24 compiler-owned conditional temporaries and produces the target 0x6C frame.  Collapsing the logic into reusable source locals dramatically shrinks and changes the function.
- Preserve repeated operand resolution when the target repeats calls.  `ApplyInterpolationOperation` intentionally resolves operand 2 twice: once for `delta = op1-op2` and once for `delta*op3+op2`.  Caching the second value is semantically tempting but changes call count, temporary ownership, and exact codegen.
- Typed aggregate assignment can also recover `rep movsd` evaluation order when the source is a true global.  `CallSubOnEnemy`'s 0x20 parameter copy matches only when `0x004ECE20` is represented as the global aggregate `g_EclCallParameters`; casting the absolute address as a pointer makes VC7 load ESI before the destination, while the global aggregate form emits the target destination/count/source order.  Treat target-resident copy sources as semantic globals when relocation evidence supports it.
- The 0x228 `EnemyEclContext` save/restore paths should use typed aggregate assignment, not `memcpy` or assembly.  This naturally emits the target `rep movsd` and preserves the distinct source/destination evaluation order for call-stack push versus pop.
- In extra-ANM cleanup, distinguish adjacent narrow VM fields from actual offsets.  The negative sub-ANM path clears `AnmVm::scriptIndex @ +0x21A`, not `activeSpriteIndex @ +0x214`; the six-byte difference is visible as enemy `+0x4CA` versus `+0x4C4` despite the same VM base and stride.

### ECL child-spawn sentinels and bitfield setters

- Prefer a real typed owner even when the target materializes a large absolute address.  The child-spawn failure sentinel `0x00F4F8F0` is exactly `&g_EnemyManager.enemies[480]`, and the failure flag `0x00F54E18` is `g_EnemyManager + 0x9DCEF8`.  Expressing those through `g_EnemyManager` produces the correct DIR32 symbol plus large addend and avoids inventing standalone globals.
- Closely related child-spawn helpers may share the exact lexical skeleton.  TH08 standard and alternate child spawn differ only by `position += parent+0x2D88`; keeping the same guard, right-to-left argument expressions, and sentinel result preserves identical compiler-temp placement around `SpawnEnemy2`.
- If the target computes `(value & 1) << bit` first, then loads an existing flag word, masks it, and performs `or old,new`, use a genuine bitfield assignment before hand-writing an RMW expression.  The Spellcard ECL bit6/bit11 setters match exactly as bitfield stores; equivalent mask/or expressions changed the OR destination register even at identical size.
- Header edits can require an explicit PCH rebuild when the edited type is included indirectly by `th_pch.h`.  `Spellcard.hpp` is pulled into the PCH through `GameManager.hpp`; after adding member declarations, remove/rebuild the PCH artifact before interpreting "not a member" diagnostics as source errors.

### ECL direct-call closure: TU ownership and bullet/laser field recovery

- A four-byte near-match can be a translation-unit/profile error rather than an expression problem. `Gui::StartStageBackgroundSequence @ 0x00439007` emitted 77 bytes in the `/Od` ECL dependency probe but the exact 73-byte target as soon as the same semantic body moved to the real `/Os` `Gui.cpp` owner. The target then naturally reused EAX for `impl->vm2156c` and used the compact `leave` epilogue. Recover the original TU before tuning registers.
- `BulletManager::RemoveBulletsInRadius @ 0x00430D30` proves two source-visible ordering details: materialize `&g_BulletManager.bullets[0]` before declaring the non-trivial `Float3 delta`, and spell the distance rejection as `if (LengthSq(delta) > radiusSquared) continue;`. The latter produces the target short false-branch plus explicit loop jump and preserves unordered x87 behavior.
- `BulletManager::SpawnLaserPattern @ 0x00430F20` retains the TH06 laser-spawn skeleton but with TH08 layouts. The target proves `Laser` runtime fields at `+0x554..+0x599` and nine laser-specific `BulletSpawnDescriptor` fields at `+0x1D0..+0x1F0`; promoting those fields into the real aggregates lets VC7 emit the complete 639-byte body exactly with `#pragma var_order(i, laser, this)`.
- When auditing a large dispatcher closure, compare direct `REL32` destinations against the exact ledger after each dependency batch. Once the TH08 RunEcl authored helpers were recovered, its only non-exact direct callees were the math-library wrappers `fmodf`, `sinf`, `cosf`, and `sqrtf`; keep those out of authored reconstruction work until authored coverage reaches 100%.

### SDK/header-inline COMDAT ownership

When the target contains a standalone body for an SDK/header-inline helper, do not copy the SDK implementation into repository source just to manufacture a symbol.  Prefer a real reconstructed caller TU that naturally emits the COMDAT under the target VC7 flags, then compare that emitted body and every relocation normally.  `D3DXVec3Length`, `D3DXVec3Dot`, and `D3DXVec3Cross` at `0x0040B4C0/0x0040B540/0x0040B7F0` are the corpus example: `Background.cpp` already uses those SDK inline helpers, so the exact functions are reproducibly emitted without changing their SDK source.

### Non-trivial member arrays versus repeated members

For VC7 `/Od`, an array of non-trivial class members and several individually declared members are not constructor-codegen equivalent.  `Float3 vectors[6]` in `BackgroundUnkVectors` emitted the vector-constructor iterator `??_H`, while the TH08 target constructor at `0x004073B0` contains six direct `Float3::Float3()` calls at offsets `0x00..0x3C`.  When every observed use is a constant index, repeated direct ctor calls are strong evidence that the original layout used individual members.  Promote the fields individually, preserve their offsets, and regress every accepted consumer before accepting the ABI change.

### Effect callback temporaries and tracking expressions

The `0x004264F0/0x00426990/0x00426D70` effect callbacks share a useful VC7 `/Od` pattern.  Perform the persistent vector updates first, then declare the non-trivial `Float3 delta`; moving that declaration to function entry runs its constructor too early and expands/shifts the frame.  The exact local layout is reproduced by `#pragma var_order(delta, dot, effect)`.  Do not cache aliases for the tracked enemy, `AnmVm::pos2`, or a background color: repeated direct owner expressions preserve the target's rematerialization and hidden `Float3` return slots.  For x87 comparisons, lexical polarity remains significant: `if (z >= 0.0f) return 0; return 1;` produces the target `test ah,1 / jne` layout, while the negated equivalent reverses the physical return blocks.

### Gui/Enemy cleanup corpus: normalized switches, temporary addresses, and early exits

- Reconcile the numeric enum before tuning a dense switch. `Gui::FUN_00438046 @ 0x00438046` uses the zero-based TH08 `Stage` enum; the target normalizes with `dec currentStage`, so `STAGE1` is the out-of-range/default path and the table covers only `STAGE2..EXTRASTAGE`. The exact `/Os` layout also requires `default:` lexically before the first case: placing it last forces a near bounds branch and moves the eight-entry table by one byte. Validate the authored `0x205` body together with the associated `0x20` table via `compare_size = 0x225`, but count only the body as authored bytes.
- Old MSVC can expose a constructor return directly through an address-of temporary. `Enemy::FUN_0042bcf0 @ 0x0042BCF0` matches the target marker call as `&Float3(-999.0f, -999.0f, 0.0f)`: VC7 constructs the temporary in the branch-local stack slot and immediately pushes the constructor's returned EAX. Declaring a named local and then passing `&local` inserts a three-byte `lea` and is not exact. Use this only when the target proves the temporary lifetime/call pattern; never emulate it with padding or asm.
- Semantically equivalent body guarding can differ by exactly two bytes. `Enemy::FUN_0042b370` needs `if (damage == 0) return;` so VC7 emits `jne body; jmp epilogue`; wrapping the body in `if (damage != 0)` emits a direct `je epilogue` and shortens the function. Preserve the target's lexical early-exit topology.
- Prefer real aggregate/global owners for target absolute storage. The cleanup path promotes `0x018B89B4` to the production `g_EnemyTrackedEnemy` owner, while the existing ECL enemy table remains its shared production storage. Probe-only aliases may keep their provisional decorated names, but normal code should have one real storage owner rather than a forwarding or duplicate shim.
- A typed array/member access can encode useful relocation addends without inventing globals. Supervisor loading-VM cleanup references the three `pendingInterrupt` members through `g_SupervisorLoadingVms[]`, naturally producing one base symbol with the target addends. Keep the aggregate owner when the shipped code proves repeated fixed-stride members.

### ECL interpolation callbacks: named x87 homes and canonical COFF ownership

- A source local can exist only to force a debug-build x87 home while the value remains live in ST0. `InterpolateLinear @ 0x00421120` needs separate `start` and `end` locals with `#pragma var_order(end, start)`. VC7 emits `fst [end]` (not `fstp`) after the second `ResolveFloat`, then immediately computes `(end-start)*t+start` from the still-live x87 value. Inlining the second resolver into the expression removes the `end` home, shrinks the frame from `0x14` to `0x10`, and makes the function three bytes short.
- Do not weaken canonical comparison just because a TU-local `static` function lacks a standard COFF function-definition aux record. `GetAnmFormat @ 0x00465510` already emitted exact bytes, but its internal-linkage symbol could not be consumed by `compare-function.py`. Promoting the existing implementation to its namespace-level real owner preserved all 83 target bytes while giving the COFF symbol a reproducible function extent. Prefer a real source owner over special-casing the comparator.
- SDK inline COMDATs may have multiple natural callers. `D3DXVec3LengthSq @ 0x0040B500` is emitted byte-identically by both EffectManager and Background; one stable production caller object is sufficient as the canonical evidence owner, while the SDK implementation itself remains untouched.

### Compiler-generated auxless COMDATs: strict canonical ownership

VC7 does not attach a normal function-definition auxiliary record to several compiler-generated bodies, notably scalar/vector deleting destructors and `??_H` vector-constructor iterators. Their source ownership is nevertheless reproducible: the compiler emits an isolated `.text` COMDAT from a real class/destructor or array-member construction in a production TU.

`compare-function.py` therefore has an explicit per-unit `allow_auxless_comdat = true` mode. It is intentionally stricter than ordinary symbol lookup: the target symbol must be section-defined at offset zero, the section must be both code and COMDAT, exactly one external offset-zero function symbol may own the section, and the entire section size must equal the manifest comparison extent. Undefined references with the same decorated name are ignored when selecting the section-defined owner. Relocation multiset validation and replay remain unchanged. Units without the explicit flag still reject auxless symbols.

This corpus attests the natural VC7 emissions for ResultScreen/AnmManager/MidiOutput/ChainElem/Pbg/zwave deleting destructors, `PbgArchiveEntry`'s vector deleting destructor, `DummyMidiTimer`'s implicit destructor, and the `0x00406850` vector-constructor iterator without adding handwritten destructor shims. `AnmVmBase::AnmVmBase @ 0x004067C0` is different: declaring/defining the real empty base constructor gives VC7 a normal function aux record while preserving the exact member-construction body, so it stays on the normal comparator path.

### ECL EX callback lifetime and branch-owner patterns

- `FUN_00423A60 @ 0x00423A60` demonstrates that a plain local with a declaration initializer can intentionally execute before a later non-trivial local constructor. Writing `u8 *bullet = ...;` before declaring the `Float3` work local makes VC7 materialize the bullet-pool cursor before the `Float3` constructor; declaring `bullet` without an initializer and assigning it in the body lets VC7 hoist the `Float3` constructor ahead of that assignment even though the later code is semantically equivalent.
- For short-circuit conditions over already-spilled locals, preserve lexical operand order. The target zone transition is `currentZone == 0 || previousZone == 0`; reversing those operands keeps semantics but swaps the two stack-home compares and misses by two instruction bytes under VC7 `/Od`.
- Repeated ECL EX setup callbacks at `0x00423530`, `0x00423DB0`, and `0x00424170` deliberately repeat two effect spawns, one ANM script assignment, and a callback-global publish. Keep the duplicated lexical template instead of abstracting it away; all three naturally emit the same 0x68-byte shape with only constants changed.

### ECL EX barrier render aggregate and callback ABI

- The barrier render globals around `0x004E4B60` are one aggregate, not unrelated absolute variables: `mode @ +0x00`, `AnmVm vm0 @ +0x08`, and `AnmVm vm1 @ +0x2AC`. The `0x2A4` spacing exactly matches `sizeof(AnmVm)`. Naming the aggregate lets VC7 emit one base DIR32 relocation plus natural member addends for scale, color, rotation, and position while retaining the target bytes.
- `FUN_004235A0 @ 0x004235A0` is a true no-argument fastcall callback. Its target prologue has no ECX/EDX homes and no argument reads. Earlier setup helpers can still publish its address through a DIR32 relocation; correcting the declaration changes only the COFF relocation symbol spelling, not their target bytes.
- A real `VertexDiffuseXyzrhw vertices[10]` local is required for the barrier strip. Its non-trivial constructor naturally emits VC7's `eh_vector_constructor_iterator`; replacing it with raw storage/memset loses both the helper call and the exact 0xE8 frame shape.
- Preserve even apparently dead authored locals when target evidence demands them. `FUN_004235A0` stores `&barrierState.vm0` into a stack local that is never read afterward; retaining that unused pointer is necessary for the exact local layout and source chronology.
- For a large render callback, existing exact D3D idioms are reusable compiler fingerprints. The barrier function uses the same `SetTextureStageState` sequence as Gui's untextured diffuse strip, then restores ANM render state through `ClearVertexShader`, `ClearColorOp`, `ClearBlendMode`, and `ClearZWrite`.

### ECL EX bullet/collision callback source-shape patterns

- In the `0x00424730/0x00424820/0x00424910` collision trio, `#pragma var_order` fixes physical stack slots independently of constructor chronology. The target slots nearest EBP are `position`, outer size, inner size, origin, while the source must construct in the opposite semantic order `origin -> outer -> inner -> position`. Declaring the four real `Float3` objects in semantic order while using `#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)` reproduces both the constructor call sequence and the `0x38` frame exactly.
- A long positive body gate is not interchangeable with a reject-and-`continue` spelling under VC7 `/Od`. ECL EX callbacks `0x00424A20`, `0x00424C40`, and `0x00424E50` require `if (tag & mask) { ...large body... }`; spelling the equivalent `if (!(tag & mask)) continue;` changes operand ownership and produces a characteristic function extent that is two bytes shorter.
- Float wrapping in `FUN_004244F0` needs both the target comparison polarity and target x87 operand owner. The exact source is equivalent to `delta > 0 ? -2*pi + delta : 2*pi + delta`; `delta < 0`, or writing `delta - 2*pi`, preserves ordinary arithmetic semantics but changes the x87 status mask and/or `fld`/`fadd` ordering.
- `FUN_004250D0` retains a target-proven `Float3` local that is constructed and never used. Keep such dead non-trivial locals when their constructor call and frame slot are present in the target; removing them is not source cleanup during matching.
- The scripted slowdown callbacks at `0x004251B0` and `0x00425290` prove `g_EclGameTimeScaleFlags @ 0x017CE8FC` as a separate ECL time-scale state word. Keep it distinct from `g_EclGameTimeScale @ 0x017CE8E0` and preserve the bit-0x20 RMW path.

### AsciiManager aggregate ctor and associated switch data

- `AsciiManager::AsciiManager @ 0x00402000` is an empty source body whose 0x128-byte target is entirely the compiler-generated member-constructor sequence. Once the public `AsciiManager` member layout is correct, the empty constructor naturally reproduces 21 relocations including nine `AnmVm` constructions, the `AsciiManagerString` and popup vector-constructor iterators, Pause/Retry menu constructors, and the demo icon constructor. Treat a long empty aggregate constructor as a strong whole-layout attestation rather than filling its body with manual initialization.
- `AsciiManager::OnDrawLowPrioImpl @ 0x00402B20` has `0x6A2` authored function bytes, while VC7's COFF aux extent is `0x6B6` because a five-entry `0x14` switch jump table immediately follows the body. When instruction offsets already align and `aux_size - authored_size` equals the associated table extent, validate with `size = authored` and `compare_size = body + table`; do not rewrite correct source merely to force the aux extent down to the authored range.
- The boss-marker distance calculation is `fabsf(marker.pos.x - 32.0f - g_Player.position.x)`. Keeping the typed `g_Player.position.x` owner produces the target `g_Player + 0x2B4` DIR32 addend and fixes both semantics and exact codegen.

### Runtime math helper inventory classification

- Target helpers `_sinf @ 0x00409060`, `_cosf @ 0x00408D40`, `_sqrtf @ 0x0040B440`, `fabs @ 0x004031E0`, `fmodf @ 0x0041F090`, and `fsincos @ 0x00433880` are math-runtime/library entries, not authored game functions. Their imported reccmp names already identify the runtime role, and their target bodies are x87/CRT helper shapes rather than subsystem-owned C++ source. Classify them as `library` instead of manufacturing authored replacements (especially inline-asm x87 wrappers, which are forbidden by the project rules).
- Inventory correction is separate from exact acceptance: reclassification removes a proven library helper from the authored denominator; it does not claim that helper is reconstructed. After authored reaches 100%, these helpers belong to the explicit library-reconstruction lane.

### VC7 float math inline-wrapper classification

- VC7 `MATH.H` defines `acosf`, `atanf`, and `tanf` inline as float-returning wrappers around the corresponding double CRT functions. TH08 targets `0x00462210`, `0x00462230`, and `0x00462250` are the expected 0x15-byte wrapper shape (`fld` float argument, call CRT core, `fst` compiler temp, `ret 4`). Treat these as library/runtime inventory, like the already-classified `sinf/cosf/sqrtf/fmodf/fsincos`, rather than manufacturing game-authored replacements.


### Large `/Os` setup-thread source-shape recovery

- `GameManager::GameplaySetupThread @ 0x0043ABD7` shows that equal addresses are not enough under VC7 `/Os`: the target deliberately mixes a cached `gameManager` local with direct `g_GameManager` accesses. Preserve the lexical owner seen in the target instead of normalizing every access through one spelling.
- Keep anti-tamper refreshes inside their original branch arms. Hoisting six identical `UpdateAntiTamper` calls to shared tails shortened the target by dozens of bytes even though values were equivalent.
- Preserve apparently redundant helpers when target locals prove them. The stage-5 spell-practice arm calls `IsSpellNumberEqualTo(212)` and discards the result; its compiler-owned BOOL work slots are part of the target 0x60-byte frame.
- Preserve multidimensional table shape. `g_TimeRequirementParams[stage][difficulty]` naturally emits the target `stage << 4` plus `difficulty * 4` calculation; flattening to `stage * 4 + difficulty` changes codegen.
- `for` and explicit `while` are not interchangeable. The spell-practice BGM table target keeps `++i` at the body tail; a `for (...; ...; i++)` spelling introduced a 2-byte trampoline.
- The play-count storage acts as seven contiguous `PlstPlayCounts` records: six difficulty records followed by totals. A narrow typed overlay models that physical table and restores the target constant-index fastcall argument evaluation without one-past-array UB.
- For constructor/destructor-free POD owners, naming real allocation work pointers and using direct `operator new/delete` can expose the same machine semantics without VC7 adding a second hidden new/delete temp. In this setup thread, `oldCfg`, `oldGlobals`, `newCfg`, `newGlobals`, and the malloc/free pointer occupy target homes `-0x14..-0x24`.
- The final exact local order is `#pragma var_order(..., allocation, stageMode, configMode)`. `stageMode` uses lexical `if/else`, and `configMode` is the integer source of the `fild`; the 3423-byte body then replays exactly with 183 relocations and no inline asm or padding.

### Isolated exact promotion from a dirty production translation unit

- A function that is exact only in a dirty production translation unit must not be ledgered against that uncommitted object. Extract the minimal exact source body into a separate probe TU, preserve the production TU, and point the canonical match unit at the probe object. `TitleScreen::OnUpdateReplayMenu @ 0x0046E136` was promoted this way: the minimal probe needs only the function body, `g_StageNames`, `TITLE_MENU_ITEM_START_REPLAY`, and the small `InitializeTitleVmAndSetSprite` helper, yet still reproduces all 3671 bytes and 107 relocations.
- Keep the probe on the same compile rule as the owning production TU. The replay-menu probe uses `cc_TitleScreen` (`/Os /Oi- /Ob1`), so source-shape conclusions remain comparable to the active Title lane while the normal executable link continues to use the production object.
- Do not bundle nearby near-matches into the exact anchor. `TitleScreen::RegisterChain` and `DrawPieChart` were investigated in the same probe but deliberately removed from the stable ReplayMenu commit until they reach strict zero-diff.

### Title `/Os` switch tails and local-owner details

- `TitleScreen::OnUpdateStartMenu @ 0x004674E0` and `OnUpdateKeyConfig @ 0x00469636` both have a 9-entry / 0x24-byte switch table immediately after the authored body. VC7 COFF aux extents include the table, so a naive size check reports +0x24. Record `size` as the authored body and `compare_size = size + 0x24`; replay all nine table relocations without counting them as authored bytes.
- In StartMenu, two target tests read `g_GameManager.flags.isReplay` and `.isSpellPractice` directly. Replacing them with out-of-line `IsReplay()` / `IsSpellPractice()` calls shortens each site by one byte under `/Os`; preserve the lexical bitfield owner in source even though the helper is semantically equivalent.
- In KeyConfig the entire 2383-byte body was already instruction-for-instruction correct; the residual was only a pair of stack homes. The target local order is `vmPair=-0x4, i=-0x8, keyToChange=-0xC, controllerState=-0x10`, reproduced by `#pragma var_order(vmPair, i, keyToChange, controllerState)`.

### Title spell-card cursor comparisons and switch-tail validation

- `TitleScreen::OnUpdateSpellCardSelect @ 0x0046BBC0` carries an 11-entry / 0x2C-byte jump table after its 0xFCF authored body. Canonical validation uses `compare_size = 0xFFB`, so the table relocations replay without inflating authored-byte progress.
- Two cursor-wrap checks in the recovered source were semantically reversed, not merely codegen-equivalent. The target is `if (cursor >= currentNumberOfSpellCards)`, while the provisional source had `if (currentNumberOfSpellCards >= cursor)`. Under `/Os` both forms have equal length but swap the `cmp` memory owner; strict replay exposed exactly two 7-byte residual spans. Fix the semantics rather than trying to reshape the compare instruction.
- When a large function is equal in extent, control flow, and relocation count but has a handful of repeated compare-owner residuals, inspect whether the source relation itself is reversed. Do not assume every operand-owner mismatch is an algebraically equivalent spelling issue.
### Repeated inline helper work slots and frame expansion

- `TitleScreen::OnUpdateSpellStageSelect @ 0x0046B174` had the same 589 decoded instructions in the target and provisional object, yet the object was 363 bytes shorter. The real discriminator was the frame: target `0x88`, object `0x7C`. Three calls used `AnmLoaded::InitializeAndSetSprite` directly, while the target-shaped project helper `InitializeTitleVmAndSetSprite` carries one source-visible `inlineSlot`. Using that helper at all three call sites restores three 4-byte homes, moves `this` from `-0x78` to `-0x84`, changes the repeated deep-stack accesses from disp8 to disp32, and naturally reproduces all 2636 bytes. When instruction counts match but frame size differs by a small multiple of four, inspect repeated inline-helper locals before assuming missing business logic or adding padding.
- `ConvertToFullWidthDigits @ 0x0046D763` is a useful tiny fastcall fingerprint: after the algorithm and six relocations matched, the only 16 residual bytes were an `i`/`multiplier` stack-home swap. `#pragma var_order(i, multiplier)` restores target `i @ -0x4`, `multiplier @ -0x8`; do not rewrite the digit loop when the opcode topology already matches.
- `TitleScreen::OnUpdate @ 0x00467399` is another associated-tail case: authored coverage is `0x10D`, while VC7 emits an isolated `0x147` COMDAT whose final `0x3A` bytes are switch-associated data before the next mapped function. Keep `size = 0x10D` and `compare_size = 0x147`; the full region replays exactly with 25 relocations.


### Title Last Word unlock boolean/source-shape recovery

- `TitleScreen::UnlockLastWordSpellCards @ 0x0046CBBB` is a useful VC7 `/Os /Ob1` fingerprint: direct `if (A || B)` does **not** preserve the target BOOL work slot, while `((A || B) ? TRUE : FALSE)` lowers to the target `temp=0/1; cmp temp` sequence. A single `A > 0` normalized through the same ternary lowers to the target `xor/cmp/sbb/neg/test` form.
- Put the history query on a small inline **member** view of `Catk`, not a free inline `(Catk *, shot)` helper. The member form preserves the target `push SHOT_ALL / pop / shl` index and one BOOL compiler temp without adding a separate Catk-pointer argument temp.
- An inline unlock helper taking `i32 spellCardNumber` naturally prevents constant-folding of `unlockedLastWordSpellCards[spell-205] = spell`; VC7 emits the target dword spell-number work home, variable index, and low-byte store. A `do { ... } while (0)` macro is not equivalent here: VC7 kept a six-byte zero-loop tail at every call site.
- Keep the target-proven loop lifetimes. The exact source uses distinct `k/n/ii/jj/kk` indices for five later loops and `#pragma var_order(i, totalCaptures, extraClearCount2, extraClearCount3, k, lastSpellCaptures15, extraClearCount4, n, requiredNormalCaptures, ii, extraClearCount6, jj, extraStageClearCount, kk, lastSpellCaptures30)`. This produces the target 0xF8 frame and places the Normal-list Catk pointer at `-0x64`.
- Preserve genuine leftovers even when redundant: the target clears `requiredNormalCaptures` **twice** immediately before the Normal spell-list loop. The second four-byte clear is required for the canonical 0xBA8 body.


### Title spell-card info formatter source-shape

- `TitleScreen::FormatSpellCardInfo @ 0x0046D7F9` matches as a 0x148-frame `/Os /Ob1` function with source locals ordered by `#pragma var_order(spellCardNumber, i, totalAttempts, commentLine1, commentLine2)`. Long-lived `Catk&` or Last-Word-table references are not source locals in the target; spelling those accesses directly lets VC7 create only the target compiler pointer temps at `-0x114..-0x148`.
- Equivalent ternaries are not byte-equivalent. The target uses zero-first lexical forms such as `totalAttempts == 0 ? unknown : spellName` and `captures[SHOT_ALL] == 0 ? unknown : commentBuffer`; reversing the condition and arms flips the short-branch topology.
- The Last Word hint table begins at spell **204**, one slot before `SPELLCARD_LAST_WORD_START` (205). Its 0x30-byte record has two format pointers and two groups of five integer arguments. Recover the source index as `spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)`, not `spellCardNumber - SPELLCARD_LAST_WORD_START`.
- `spellCardInfoVms[0..6]` are the natural typed owners for target offsets `0x11F2C..0x12F04`; two 128-byte comment buffers and repeated direct `CatkHistory` reads reproduce the target vararg push order without overlays.


### Title completion badge: preserving an inlined GameManager member owner

- `TitleScreen::DrawCompletionStatusText @ 0x0047052D` proves that an out-of-line-looking helper can still have an **inline member** source shape at a caller. A free `u16` bit-test helper kept the correct mask arithmetic but folded `g_GameManager.clrdData` into one memory displacement; a probe-local GameManager member view reproduced the target `this` parameter, `character * 0x24`, explicit `add &clrdData[0]`, and then the difficulty-indexed word load.
- In the inlined stage-clear helper, keep source order `difficultyBits & ZUN_BIT(stage)`. VC7 evaluates the right operand first, so the target starts with `xor/inc/shl` for the stage mask before loading the clear-data word. Reversing the `&` operands flips evaluation order even though the value is identical.
- The completion-status third condition is target-proven as five independent OR arms: Easy, Normal, Hard, Lunatic clear, **or `cursor > 3`**. The earlier reconstructed `Lunatic && cursor > 3` precedence was semantically wrong.
- Reuse the low-level `InitializeTitleVmAndSetSprite` source shape instead of `AnmLoaded::InitializeAndSetSprite`; four lexical branches then naturally allocate the target VM/AnmLoaded work homes and produce the exact 0x38 frame.

### Aggregate exact state requires a cold-build replay

Scope: the complete authored exact ledger and every object named by
`config/match-units.toml`.

Observed: on 2026-08-19, replaying 1,105 historically accepted rows from a cold
VC7 build exposed 14 units that no longer reproduced. The old aggregate path
also omitted 23 configured reimplementation objects unless they were built by
hand. Focused comparisons had been run against objects and a PCH produced at
different repository states, so their individual historical results did not
establish a current aggregate result.

Inference: a focused `exact` remains evidence for that function and that object
state, but aggregate progress is invalidated by an untested shared header,
layout, compiler-flag, PCH, or object-graph change. A successful normal link
does not fill this evidence gap.

Working shape: `configure.py` derives the aggregate `objdiff` dependencies from
both `objdiff.json` and every object in `match-units.toml`.
`verify-exact-units.py --all` then regenerates that graph, asks Ninja to clean
its declared outputs, removes the explicitly listed VC7/linker side outputs
that Ninja cannot see, builds serially, and replays only rows accepted by
`matches.csv`.

Rejected alternative: reusing `build/`, relying on normal-build objects, or
running aggregate replay only after an agent manually notices missing objects.
Those paths are useful for diagnosis but cannot publish repository-wide exact
totals.

Reproduce:

```bash
python3 scripts/analysis/verify-exact-units.py --all --json \
  > build/accepted-unit-replay.json
```

Result: after removing the 14 unreproducible claims, the cold build and replay
passed 1,091 / 1,091 accepted units. `--reuse-build` is explicitly diagnostic.
Public GitHub Actions cannot perform this attestation because the target and
pinned local VC7 environment are private; it remains a required local gate.

Generalization limit: this establishes current function-level authored replay
only. It does not prove original object partition, linked-image layout,
target-linked libraries, resources, or complete-PE identity.

### Target-linked D3DX fast-table and CRT thunk boundary recovery

- The D3DX optimized dispatch tables at `0x004867B0` (SSE) and `0x004868D0`
  (SSE2) are stronger naming/provenance evidence than heuristic disassembly
  labels.  Their relocation order matches `objd/i386/ssefasttable.obj` and
  `objd/i386/ssefasttable2.obj` from the VC7 PlatformSDK prerelease
  `D3DX8.LIB` (SHA-256
  `0d4a2b642485dcaa7671926a9a1a545c656d5eb73f160fe971b3deebf0b516b5`).
  In particular, the table slots identify `0x0048D3D0` as SSE Vec3 normalize,
  `0x0048D4A0` as SSE plane normalize, `0x0048DA50` as SSE Vec4 normalize,
  `0x0048E680` as SSE quaternion normalize, `0x0048EFB0` as SSE2 Vec3
  normalize, and `0x0048F080` as SSE2 plane normalize.  Do not infer these
  identities from vector width alone; table relocation identity distinguishes
  the same-looking four-float normalize families.
- For these optimized D3DX COMDATs, compare the complete archive section after
  masking/replaying its COFF relocations.  The target non-relocation bytes,
  including post-`ret` alignment padding, exactly match the corresponding
  archive sections.  The accepted function-body extents stop at the compiler
  return (`0xC6`, `0xD8`, `0x9C`, `0x9C`, `0xC6`, `0xD8` respectively), while
  the archive sections continue to aligned `0xD0`, `0xE0`, `0xA0`, `0xA0`,
  `0xD0`, `0xE0`.  Keep body extent separate from archive-member padding; a
  next mapped address is not a boundary proof.
- The `+0x0E` labels inside the aligned SSE/SSE2 Vec3/plane normalize bodies are
  real archive-local secondary symbols (`...Normalize$$1`), not independent
  TH08 inventory functions.  They have no target xrefs in the attested IDA
  session and live inside the same archive COMDAT.  Preserve them as internal
  entry evidence rather than creating overlapping mapping rows.
- `operator delete @ 0x004A43CF` is the VC7 static-runtime `??3@YAXPAX@Z`
  thunk: the target is exactly one five-byte near jump to `_free @ 0x004A427B`.
  With the repository's `/MT` compiler profile, the relevant archive is
  `LIBCMT.LIB` (SHA-256
  `8815af7b9b6e0e28b77708ede25ab7ecfc4b05e1d8811f092c516cff5ce19d94`),
  member `build/intel/mt_obj/delete.obj`.  That member owns an isolated
  five-byte `.text` section with one `DISP32 _free` relocation.  Use the member
  section/relocation as the extent proof; the adjacent `operator new` address
  alone is only corroboration.


### Microsoft COFF archive member identity for library replay

- VC7 `.LIB` long-name tables use NUL-terminated member names in the archive
  observed here; do not assume GNU ar's `/\n` spelling.  More importantly, a
  Microsoft archive can contain repeated member path names.  A library comparator
  must therefore preserve all occurrences and disambiguate the configured member
  by its section-defined COFF symbol (or fail if more than one occurrence owns the
  symbol), rather than treating the member path as a unique key.
- Library relocation replay must resolve the object field addend plus a pinned
  target base.  Masking relocation bytes is only a diagnostic.  The initial D3DX
  family proves `.data1` bases `0x018DA000` (SSE) and `0x018DA220` (SSE2), plus
  `.data` bases `0x004C9FC0` (Vec4 normalize) and `0x004CA180` (quaternion
  normalize); `operator delete` proves a REL32 target of `_free @ 0x004A427B`.

### Strict bounded symbols inside a shared library COFF section

Some VC7 CRT archive members place more than one externally named function in a
single `.text` section.  Do not reject those functions merely because their
symbol is not at section offset zero, and do not solve the problem with an
arbitrary byte slice.  A library match unit may opt into a bounded shared-
section comparison only by pinning both `section_offset` and the complete
`section_size`.  The comparator then requires the configured symbol to begin at
that exact offset, requires a COFF function-definition auxiliary record whose
`total_size` equals `compare_size`, bounds every relocation relative to that
symbol, and rejects any subrange that extends beyond the pinned section.  Units
without those explicit fields retain the stricter whole-section ownership rule.

### Library archive decoration is naming evidence only after range replay

- VC7 CRT/C++ EH members can turn anonymous imported inventory rows into stable
  names, but do not rename a `FUN_*` row merely because an archive contains a
  plausible decorated symbol of the same size.  First pin the archive hash and
  member, require the COFF function-definition extent to match the target body,
  replay every DIR32/REL32 field, and obtain a zero-difference canonical target
  comparison.  The `trnsctrl.obj` family at `0x004A4419..0x004A4745` is the
  corpus example: eight helpers replay exactly before four anonymous rows are
  promoted to `_CreateFrameInfo`, `IsExceptionObjectToBeDestroyed`,
  `_CallCatchBlock2`, and `_CallSETranslator`.
- MSVC decoration can directly prove storage-level facts such as global
  `__cdecl` (`YA`), return class, and simple parameter lists.  It does not
  justify guessing complicated numbered type back-references.  When the
  decorated `_CallSETranslator` symbol proves `__cdecl` and `int` return but the
  back-referenced parameter sequence has not been independently decoded, keep
  those parameter slots conservative rather than manufacturing a typed ABI.

### Library extent repair can emerge from dependency replay

- A mapped library row can be undersized even when it does not overlap a later
  row. `_inconsistency @ 0x004AA9E7` was seeded as `0x26` bytes, so the
  conservative archive proposer initially rejected it. `LIBCMT hooks.obj`
  carries a function-definition aux extent and section size of `0x2D`; the
  canonical target has the corresponding cleanup path and final tail jump
  through `0x004AAA13`. Replaying all four relocations over the full 45-byte
  range produced zero differences. Repair the mapping from `0x26` to `0x2D`
  rather than weakening the proposer or truncating the archive function.
