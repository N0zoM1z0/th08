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

For a function whose authored body is followed by compiler-owned tables, first
prove the authored extent independently, then set `compare_size` to the COFF
auxiliary extent and replay every table relocation as well. Local `$L...` COFF
labels are normalized to `$L*` by the comparator because VC7 renumbers them when
earlier code in the translation unit changes; their relocation offsets and
resolved target addresses remain the evidence.

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
