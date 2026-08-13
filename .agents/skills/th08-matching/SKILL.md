---
name: th08-matching
description: Compile TH08 functions with the repository VC7 build and compare configured COFF functions against the hash-attested 1.00d target using config/match-units.toml and scripts/compare-function.py. Use for focused code-generation tuning, relocation diagnosis, or exact-match verification.
---

# TH08 exact matching

Use with `$th08-re` only after the target boundary, semantics, and ABI are
supported by evidence.

## Canonical loop

1. Verify the target and list configured units:

   ```bash
   python3 scripts/verify-target.py
   python3 scripts/compare-function.py --list
   ```

2. Read the selected `[[units]]` entry in `config/match-units.toml`. Confirm
   its target address/size, decorated COFF symbol, object path, and every
   relocation against `config/mapping.csv`, `config/reccmp-*.csv`, and target
   disassembly.
   If an original delinked object is required and Ghidra is configured, the
   coordinator may generate it through the verified disposable import:

   ```bash
   python3 scripts/export_ghidra_objs.py --import-csv
   ```
3. Build the entry's object basename. For an object recorded as
   `build/ScreenEffect.obj`, run:

   ```bash
   python3 scripts/build.py --build-type=objdiffbuild --object-name ScreenEffect.obj
   ```

4. Compare the configured function:

   ```bash
   python3 scripts/compare-function.py UNIT --json
   ```

5. Treat `exact`, `mismatch`, and `error` literally. Classify a mismatch before
   editing source: function boundary, ABI/layout, compiler flags, decorated
   symbol, REL32/DIR32 mapping, source/control-flow order, or translation-unit
   effect.
6. Change one source-plausible expression or declaration, rebuild the same
   object, and rerun the same unit.

## Relocation integrity

Add a `[[units.relocations]]` entry only after proving its byte offset, COFF
symbol, relocation kind, exact TH08 target address, and addend behavior. The
comparator rejects actual relocations that differ from the manifest and rejects
the wrong target hash. Never wildcard an unresolved relocation or borrow a
TH06/TH07 address.

## Acceptance

An `exact` JSON result proves only the configured function bytes after the
declared relocations. It does not prove the original object partition, adjacent
functions, data layout, or a repository-wide percentage. Only the coordinator
updates shared mapping/match manifests or publishes matching status.

Do not force bytes with naked assembly, copied code arrays, arbitrary padding,
fake types, ABI lies, empty behavior, or target patches. Serialize Wine/VC7
builds when parallel agents are active. End with:

```bash
python3 scripts/validate-tracking.py
python3 scripts/progress.py --check
git diff --check
```
