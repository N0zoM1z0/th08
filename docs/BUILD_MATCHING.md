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
- initialized Detours and munit Git submodules.

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
python3 ./scripts/build.py --build-type tests
python3 ./scripts/build.py --build-type dllbuild
python3 ./scripts/build.py --build-type objdiffbuild
```

These modes serve different runtime and comparison purposes. Success in a
bugfix, DLL, test, or object build does not establish that the normal
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

`scripts/scan-vc7-library.py` is intentionally disabled until TH08-specific,
SHA-pinned library archives and relocation policy exist. Unsupported use exits
nonzero instead of borrowing TH07 assumptions.


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
