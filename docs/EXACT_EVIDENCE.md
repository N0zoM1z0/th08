# Exact verification: evidence scopes and agent workflow

Use this workflow when accepting a function, changing referenced data, or
investigating a mismatch. The target remains the hash-attested Japanese TH08
1.00d. [Issue #38](https://github.com/N0zoM1z0/th08/issues/38) records the
independent oracle analysis and motivating missing replay-label check.

## Read the scopes before the verdict

`compare-function.py` returns schema version 2. Its existing `result` describes
the configured code and all performed literal, declared-data and link checks.
An `exact` result does not pass dependencies reported as uncovered.

| Field | What it proves |
| --- | --- |
| `evidence.code` | The full configured COFF function/`compare_size`, strict relocation set, addends and local destinations match canonical target code. |
| `evidence.data` | Compiler literal contents and explicitly contracted initialized data have been checked. Each referenced contract is checked in full, even when the function only addresses its start. |
| `evidence.linkage` | With a rebuilt PE and map, the selected linked function matches the compared COFF body with every relocation replayed at linked addresses. Declared data contracts are also checked in that PE. |

`data.status = partial` means some direct raw-backed data has no reviewed
contract. `uncovered` entries include source symbols, resolved target addresses,
instruction offsets, stable finding codes and next actions. Default JSON
samples at most 12 entries per evidence list; counts remain complete. Use
`--details` for every data observation. Ordinary instruction/relocation reports
retain their existing fields.

Runtime state in zero-fill regions and import/ABI addresses appear in
`out_of_scope`. They do not become initialized-data passes. This tool does not
prove constructors, complete mutable lifecycles, recursive pointer closure,
callee bodies, gameplay, pixels or audio. Those retain their own units and
runtime gates. Compiler string checks cover through the first narrow/wide NUL;
they do not attest data after embedded terminators.

Linked evidence starts as `not_checked`. An unresolved/ambiguous map symbol
leaves it `partial`; no instruction field is masked to obtain a linked pass.
Use a fresh normal link and its paired PE/map. A bugfix object paired with an
unrelated normal image is not a valid source comparison.

## Focused commands

Discover a unit without materializing the complete inventory:

```bash
python3 scripts/compare-function.py --list --filter ReplayManager --json --limit 10
```

JSON listing returns `total`, `offset` and `next_offset`. Continue with that
offset or narrow the filter. Configuration is not acceptance: `matches.csv`
remains the accepted ledger.

Build the comparison object, including its objdiff wrapper:

```bash
python3 scripts/build.py --build-type=objdiffbuild --object-name ReplayManager.obj
python3 scripts/compare-function.py replay-manager-save-replay --json
python3 scripts/compare-data.py --list --json
python3 scripts/compare-data.py replay-difficulty-labels --json
```

For `build/objdiff/reimpl/*.obj`, `--object-name` regenerates the wrapper.
Building only the underlying `build/*.obj` does not refresh it. Probe owners
can instead be passed as their exact positional build targets.

Build and check the final native image serially:

```bash
python3 scripts/build.py --build-type normal -j 1
python3 scripts/compare-function.py replay-manager-save-replay \
  --rebuild build/th08.exe --map build/th08.map \
  --require-data --require-linkage --json
python3 scripts/analysis/verify-windows-i386-runtime-data.py
```

The existing final-link owner/callee verifier remains required. Its reviewed
plain-data and replay-label checks use the same contracts as COFF comparison;
its other ownership/runtime checks retain their existing scope.

`compare-data.py --all --rebuild ... --map ...` checks both current COFF
initializers and linked data. Add `--linked-only` to validate a retained product
without attributing a newer object to it. `--object PATH` permits one isolated
contract experiment; it does not change canonical ownership or award credit.

## Results, exits and next actions

| Result | Meaning and action |
| --- | --- |
| `exact`, exit 0 | Configured code and performed checks pass. Read data/linkage scopes before making a broader claim. |
| `mismatch`, exit 1 | Code, a literal, a declared initializer or linked evidence differs. Inspect `findings` and repair only from target evidence. |
| `error`, exit 1 | A required input, manifest, symbol or boundary is invalid. Fix that input rather than relaxing the comparator. |
| `incomplete`, exit 2 | `--require-data` or `--require-linkage` requested evidence that is still partial/not checked. Review gaps or provide the correct PE/map. |

The data CLI uses schema version 1: `verified`, `valid` and `listed` exit 0;
`mismatch`/`error` exit 1. `--validate-only` proves schema validity only and
reports `target_status = not_checked`. Comparison/input errors after argument
parsing return a single JSON object; argparse usage errors retain stderr and
exit 2. Aggregate compiler output goes to stderr. `next_commands` contains argument
arrays, never executed shell snippets.

| Finding code | Interpretation |
| --- | --- |
| `INITIALIZED_DATA_UNCOVERED` | Review the initializer's target extent and COFF owner, then add an explicit contract. |
| `DATA_OWNER_EXTENT` | The required range exceeds storage or crosses another owner. Check for a truncated initializer; do not absorb adjacent data. |
| `DATA_RELOCATION_SET` / `DATA_RELOCATION_INVALID` | Pointer initializers are missing, extra, duplicated or of the wrong relocation type. |
| `DATA_POINTER_IDENTITY` | The pointer selects the wrong symbol/addend/linked owner, even if another address holds equal text. |
| `DATA_CONTENT_MISMATCH` / `LITERAL_CONTENT_MISMATCH` | Inspect the first differing offset and expected/actual hex; check native Shift-JIS encoding and format punctuation. |
| `DATA_POINTER_INVALID` | A linked pointer lacks raw-backed storage for the declared string. |
| `LINK_SYMBOL_UNRESOLVED` | The map lacks a unique matching symbol. Inspect ownership/signature; do not wildcard it. |
| `LINK_CODE_MISMATCH` | Linked instructions differ from the compared COFF body. |
| `CONTRACT_SCHEMA_INVALID` / `CONTRACT_TARGET_MISMATCH` | Fix the schema or re-inspect the canonical target. Source agreement alone is not target evidence. |

## Adding a reviewed data contract

`config/data-contracts.toml` is versioned and pinned to the canonical SHA-256.
Each entry declares its name, object/symbol, target address, explicit size and
evidence citation. Unknown fields, duplicate names, overlapping target ranges
and paths outside the repository are rejected.

- `kind = "bytes"` requires exactly `size` reviewed bytes and rejects pointer
  relocations inside the source range.
- `kind = "pointer_array"` requires one typed entry per four-byte slot.
  `cstring` records the canonical pointer, terminated bytes, COFF symbol and
  addend; `symbol` records pointer identity; `null` requires a relocation-free
  zero. COFF and PE checks preserve these identities.

Target sizes require reviewed evidence. Next-symbol distances are object-side
safety bounds, not a way to invent target table lengths. See
`replay-difficulty-labels` for a complete pointer array and
`gui-stage-clear-bonuses` for plain array data. Static validation also binds each
root to an existing canonical DIR32 symbol/address. Parsed-object and manifest
caches use content, so same-size/same-timestamp changes cannot reuse a good
observation.

## Validation and retained evidence

Public CI generates PE/COFF fixtures without a game executable, SDK or Wine.
Mutations include missing sixth slots/initializers, changed strings/scalars,
wrong pointer identities, duplicate relocations, incorrect linked instructions,
ambiguous/missing symbols and incomplete evidence. Existing callee, switch and
truncated-trace controls remain in CI.

After changing the oracle, cold-build and replay the accepted ledger:

```bash
python3 scripts/analysis/verify-exact-units.py --all --json \
  > build/accepted-unit-replay.json
python3 scripts/build.py --build-type normal --fresh -j 1
python3 scripts/analysis/verify-windows-i386-runtime-data.py
python3 scripts/ci.py
git diff --check
```

The aggregate JSON reports data coverage separately, with concrete units to
inspect next. Uncovered-reference counts select work; they are not progress
percentages or whole-program proof. Cold replay precedes complete links because
it cleans generated linker outputs.

The initial target-backed run exposed 11 previously code-exact units with
incorrect literal data. Repairs restore native encoding, registration labels,
HUD format strings, score-dump newlines/spacing, an external-texture error and a
spell-card aggregate caption. Japanese text now uses named i18n entries where
needed. Separate FPS-warning and texture-load/create messages retain their
different target bytes. Literal relocation-name changes are accepted only after
checking the new COFF bytes at the existing canonical target operand.
