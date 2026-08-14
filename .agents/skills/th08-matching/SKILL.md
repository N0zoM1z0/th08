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

   If VC7 emits the right implicit constructor bytes but omits a COFF function-
   definition auxiliary record, declare and define the natural empty constructor
   explicitly, rebuild, and compare again. Do not synthesize behavior; member
   construction must remain compiler-driven.

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

For a read-only search of already built, source-emitted leaves, use the
conservative discovery gate:

```bash
nice -n 15 python3 scripts/discover-exact-units.py --min-size 0x80
```

It emits review-only candidates only when the target-pinned isolated extent,
full relocation replay, direct-call multiset, named destination ledgers, and
unique object ownership agree. `--allow-unimplemented` may reveal source-built
functions missing from `implemented.csv`; it never updates tracking. Inspect
the source and target, add a reviewed unit, and obtain this skill's canonical
`compare-function.py` `exact` result before entering either ledger.

For a scanner batch, reject same-address multi-owner candidates and run the
canonical comparator once for every remaining unit before adding any
`implemented.csv` or `matches.csv` rows. Aggregate replay evidence is a
prioritization result, not a substitute for per-unit acceptance.

The scanner includes ordinary and `build/probes/` VC7 objects.  Use
`--allow-unnamed-mapping` only to diagnose a one-to-one exact body whose
target row is still `FUN_*`; it is review-only.  Reconcile the target name and
ABI in the ledgers before creating a canonical unit, rather than accepting a
placeholder name.

If a fully replayable leaf is blocked only by a source-emitted static global,
decode that relocation against the target and add its canonical global-ledger
row before acceptance.  Then add the function mapping/implementation record,
write the unit's explicit relocation, and require the canonical comparator to
replay it.  Do not treat a private static name or a successful object build as
an implicit target address.

For a VC7 switch function whose COFF auxiliary `total_size` includes compiler-
owned jump tables, keep `size` as the authored code coverage and set
`compare_size` to the complete code-plus-table extent. Prove the extent ends at
a target boundary, list every code and table relocation, and require the whole
`compare_size` range to match. Never count table bytes as authored progress.

For a nonmatching dispatcher, diagnose size movement with a read-only span
crosswalk: map each target table slot and each COFF table `DIR32` relocation to
handler starts, sort unique physical starts, and compare neighboring spans.
This exposes the responsible handler even when slots share a default body. It
is only a source-shaping aid; it does not establish a boundary or an exact
match without the canonical full-range comparison.

For `EclManager::RunEcl`, use the checked-in implementation rather than
rebuilding this bookkeeping ad hoc:

```bash
python3 scripts/crosswalk-ecl-dispatch.py --object build/probes/EclRun.obj --top 20
python3 scripts/ecl-shape-score.py --top 20
```

`crosswalk-ecl-dispatch.py` reports the physical handler movement.  The score
helper is a non-mutating guardrail for probes: it checks the required
code-plus-table extent, sums all positive/absolute handler deltas, and lists
focus opcodes.  Prefer changes that preserve the `0x6B06` associated extent and
improve the global score; do not commit a swap that merely moves bytes from one
handler to another unless a later bounded probe depends on that exact shape.
Neither script can be used to count unmatched bytes as authored.

When a dispatcher target loads ECX from an absolute address immediately before
a `__thiscall`, resolve that address through `config/reccmp-globals.csv` before
choosing the C++ owner.  Bind the observed global and an evidence-backed member
declaration rather than retaining a dispatcher facade or guessing an adjacent
subobject with pointer arithmetic.  Record a new global/function ledger name
only after target disassembly establishes its address, access width, and ABI;
cross-version class ownership is not sufficient evidence.

## Relocation integrity

Add a `[[units.relocations]]` entry only after proving its byte offset, COFF
symbol, relocation kind, exact TH08 target address, and addend behavior. The
comparator rejects actual relocations that differ from the manifest and rejects
the wrong target hash. Never wildcard an unresolved relocation or borrow a
TH06/TH07 address. A zero-addend `DIR32` function pointer accepts only the
canonical TH08 function start supported by the ledgers; an address merely
falling inside the image is not evidence for a function, global, or literal.
Compiler-local `$L` numeric suffixes may change when an earlier function in the
same translation unit changes; their offset, kind, addend, and resolved target
must still agree exactly.

## Translation-unit emission

Treat a missing header-inline COMDAT as a caller/TU question, not a license to
manufacture a symbol. Prefer restoring a target-proven caller so the configured
object emits the function naturally. Do not add dummy callers, artificial
function-pointer references, or `noinline` solely to satisfy the comparator;
defer exact acceptance when natural emission is not yet available.

Conversely, when a mapped member has an observed target body and target callers
use a direct `REL32` call to it, do not leave its recovered definition header-
inline. Move the same natural, side-effect-free body to the target-proven
translation unit, rebuild its owner object, and compare the independent body
before recording it. This restores real object partitioning and can unblock
caller relocations; it is not permission to add `noinline`, dummy references,
or a forwarding shim. The four `Supervisor` option predicates at
`0x00438A29..0x00438A71` are the corpus example.

After converting a header-inline or declaration-only member into a target-
observed direct callee, run the serial normal link in addition to the focused
object comparisons.  A focused caller object can exactly replay an unresolved
`REL32`, while the normal link correctly exposes a declaration with no emitted
definition. Recover and compare the real target body instead of adding a dummy
definition or a linker-only workaround. `Player::DeletedCallback` and
`AsciiManager::FUN_00422bb0` are the TH08 corpus example.

A mapping and an external target body establish ABI, not the original
translation-unit profile. Before moving an inline member, compare the natural
candidate object(s): a target-only `mov esp, ebp; pop ebp` epilogue or a
different receiver/member access sequence is evidence that the assumed TU or
layout is wrong. Stop short of a match and retain the existing source form
until that difference is explained; do not tune it with inert locals, padding,
or assembly. The `GameManager` query cluster at `0x00406C70..0x00406DE3` is
the corpus counterexample.

This VC7 build can retain obsolete inline definitions in `build/th_pch.pch`
after a header-only member becomes out-of-line. If a full link reports a
duplicate that is absent from the edited header, rebuild the generated PCH and
then the normal target before diagnosing source ownership. Keep this recovery
limited to reproducible `build/` artifacts; never delete source, target, or
analysis data to clear a compiler cache.

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
