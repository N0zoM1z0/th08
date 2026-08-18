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


## Replay/VC7 source-shape corpus

When matching `/Od` replay callbacks, check three high-value shapes before introducing extra locals. First, let `new T` remain directly inside registry/allocation calls when the target frame already contains VC7's hidden new-expression temporary; a redundant named allocation local can add exactly six bytes. Second, retain apparently pointless legacy division/modulo stores when target code contains their signed arithmetic lowering; adjacent Touhou sources may explain why the statements existed, but validate the exact constants against TH08. Third, infer narrow-looking setter ABIs from exact callers as well as the callee: a byte store in the callee does not prove a byte parameter if callers consistently sign-extend to `int`.

For a final one-byte register-owner mismatch, do not use padding, inline assembly, or fake types. Build a bounded expression/type matrix under the repository VC7 flags and compare instruction templates against already-exact corpus functions. Record eliminated hypotheses in `.analysis/` or `docs/BUILD_MATCHING.md`, keep the semantic implementation, and defer the match until natural C/C++ emission reaches zero differences.

## GUI table/branch source-shape corpus

For `/Os` GUI setup code, keep target-observed table dimensionality and branch-local validation. A real `paths[stage][shot]` table can be codegen-visible (`imul stage,row_size` plus scaled column), so do not flatten it merely because the address is equivalent. Likewise, two mutually exclusive resource-load arms may intentionally carry duplicate NULL checks; hoisting a shared check can shorten the function materially. When a helper or ANM member is addressable through both a receiver home and a canonical global/member owner, preserve the owner shown by target ECX/member offsets. Use already-exact accessors to name overlapping `AnmVm` fields before tuning a large grid loop.

## Branch-local ctor / aggregate-copy corpus

For VC7 `/Od` interpreters and spawn paths, separate non-trivial object lifetime from scalar-local placement. Keep branch-only vectors or timer-like objects in the lexical branch so their constructors are not hoisted. If adjacent scalar locals remain swapped despite declaration and `#pragma var_order` experiments, test whether the source shape is one meaningful POD work aggregate rather than adding padding or no-ops. Preserve explicit `continue`/fallthrough structure when it controls short-branch extents or the start of an associated jump table.

For large copies, compare target `rep movsd` register ownership before using `memcpy`. Typed aggregate assignment can intentionally schedule source/destination/count in a different order and is often the real source shape. Likewise, use semantic one-bit fields for target bitfield RMW sequences rather than algebraically equivalent mask/or expressions.

When an exact dependency still belongs to the reconstruction probe graph, keep one canonical probe implementation and leave normal production link inputs clean. Do not create duplicate forwarding shims merely to satisfy the linker; promote the owner and its dependency closure together.

## Overloads and tiny fastcall adapters

Treat authored exact identity as target address + extent, not logical name. Overloaded members may share the same reccmp/mapping name; tooling that collapses authored rows by name will mis-account exact evidence. For tiny `__fastcall` bridge functions, do not cache ECX/EDX arguments unless target stack accesses prove a third local. Repeated casts/field accesses from the original parameter can be the exact `/Od` source shape.

## ECL tail guard/lifetime source-shape corpus

For VC7 `/Od` ECL helpers, two class-local constructor calls can require sequential non-overlapping source scopes.  If the target constructs a dead legacy `Float3` in one slot and then a used `Float3` in another, test `{ Float3 legacy; } Float3 used;` before changing declarations globally.  A synthetic aggregate is not equivalent if it creates a compiler-generated constructor.

Keep conditional expressions wide until a target-proven narrow store.  If a `ResolveInt`/raw-value ternary feeds an `i16` descriptor member, casting both arms early can create word stack temporaries; leaving the ternary as `i32` lets VC7 materialize a dword conditional temp and then copy its low word, matching the TH08 shot descriptor path.

Guard-return grouping is code-generation-visible.  A single `if (rejectA || rejectB) return;` can make VC7 share one nearby reject trampoline, while two separate returns duplicate long epilogue jumps and the inverse positive `if` can produce long conditional branches.  For x87 range gates, preserve positive strict predicates such as `radius > 0.0f && distanceSquared < radius` when the target status masks prove them; algebraically equivalent `<=`/`>=` rewrites can change unordered behavior and branch opcodes.

## ECL x87 comparison / loop-exit corpus

When a TH08 `/Od` float comparison contains `Float3::operator float*()` or another call, keep the exact source-side operand ownership observed in the target.  `positionMember < bound` can avoid an x87 spill that appears for the algebraically equivalent `bound > positionMember`.  Also prefer a proven typed member over a raw offset when the target's `fld`/`fcomp` ownership depends on the member AST.

Do not normalize direct/wrapped distance tests: the target can require `directDistance < wrappedDistance` specifically.  Reversing it to an equivalent `wrapped <= direct` changes `fcompp` ownership and status masks.  Likewise preserve branch polarity when one side is much larger; `if (duration <= 0) immediate else timed` can be required solely to keep the large block as fallthrough and the short call at the tail.

For slot-scanning installers, try rejection-first control flow before adding temporaries: `if (reject) continue; install; break;` lets VC7 place the continue edge at the loop increment and the successful install fall through to the common epilogue.  A positive accept block with `return` may add both a long continue branch and an epilogue jump.  TH08 interpolation callbacks use a separate eight-pointer table at `0x004C6C90`; do not alias it to `g_EclExInsn @ 0x004C6CB0`.

## ECL switch/context/ANM source-shape corpus

Switch bounds checks can depend on where shared labels are written in source.  If VC7 inserts a tiny default trampoline, try placing the shared success code inside the switch after the final case and place `default:` on the shared failure label immediately afterward.  In TH08 `CompareOperands`, that lexical layout removed the only two-byte residual while preserving the 12-entry associated jump table.

For repeated operand dispatch, do not cache resolved values unless target call counts prove a source local.  Independent `?:` reads can be responsible for large compiler-owned temp frames.  `ApplyInterpolationOperation` also proves that an operand may intentionally be resolved twice.

When a `rep movsd` copy uses a target-resident static source, test a real extern aggregate owner instead of an absolute pointer cast.  The `g_EclCallParameters @ 0x004ECE20` aggregate changes VC7 evaluation from source-first to the target destination/count/source order.  Use typed `EnemyEclContext` aggregate assignment for 0x228 stack frames rather than `memcpy`/asm.

For raw VM-offset cleanup, verify the exact narrow field.  `enemy + index*0x2A4 + 0x4CA` is `AnmVm::scriptIndex` relative to VM base `+0x2B0`; `activeSpriteIndex` would be six bytes earlier and is not equivalent.

## Typed sentinel / bitfield / PCH corpus

When a target address falls inside a known large global object, express it through the owning typed object and index/field before creating a new global.  `0x00F4F8F0` is `&g_EnemyManager.enemies[480]`; VC7 emits the target base relocation plus a large addend automatically.  This is preferable to an absolute pointer or synthetic sentinel symbol.

For one-bit setters, a real bitfield assignment can be required for register ownership even when a manual mask/OR expression is mathematically identical.  If target code computes the incoming bit first and then uses the old flag word as the OR destination, test a bitfield member store.

If MSVC reports that a newly declared member does not exist, check whether that class definition is already cached in the PCH through an indirect include.  Force-rebuild `th_pch.pch` before treating the diagnostic as a declaration/source mismatch.

## TU-profile and subsystem-field closure corpus

Before tuning a small residual, verify that the function is compiled in the target-proven translation unit and optimization profile. `Gui::StartStageBackgroundSequence @ 0x00439007` is 77 bytes in the `/Od` ECL dependency probe but exactly 73 bytes in the real `/Os` Gui TU with the same behavior; the profile change naturally restores EAX reuse and the `leave` epilogue.

For a loop that owns a non-trivial vector local, constructor timing and physical stack order are independent. `BulletManager::RemoveBulletsInRadius` materializes the bullet-pool pointer before constructing `Float3 delta`, while `#pragma var_order(delta, i, bullet, this)` controls the final slots. A rejection-first `if (distanceSquared > radiusSquared) continue;` is also target-visible and should be preferred over an algebraically equivalent positive-body form when the target has a short conditional plus explicit loop jump.

When a cross-version aggregate lines up field-for-field with target accesses, promote the target-proven fields instead of maintaining raw offsets. TH08 `SpawnLaserPattern` confirms the laser runtime tail and the descriptor's nine `+0x1D0..+0x1F0` laser parameters; the resulting typed TH06-style source matches all 639 target bytes directly. Re-audit large-dispatcher direct-call closures after such promotions and separate remaining runtime/library wrappers from authored functions.

## SDK/header-inline emitted helpers

If a target function is an SDK/header-inline helper, keep the toolchain header as the source of truth and let a real reconstruction caller TU emit its COMDAT naturally. Do not paste the SDK body into `src/` or add a dummy caller. Pick a stable TU that already calls the helper, require a normal build plus strict per-function relocation replay, and only then record exactness. The TH08 `D3DXVec3Length/Dot/Cross` bodies are emitted naturally by `Background.obj` this way.

## Repeated non-trivial members

Do not treat `T members[N]` as codegen-equivalent to `T member0; ...; T memberN;` when `T` has a target-visible constructor. VC7 can lower the array through `??_H` but emit one direct constructor call per individual member. If target construction shows repeated direct calls and all consumers use constant indices, split the fields, keep the same offsets, and rerun every accepted unit that touches the aggregate before promoting the layout.

## Effect callback lifetime/rematerialization

For VC7 effect callbacks, place a non-trivial work vector at the lexical point where the target first calls its constructor; do not hoist it just because C++ permits it. Use direct owner expressions when the target repeatedly reloads an enemy/global/member instead of caching references. Hidden `Float3` return temporaries then tend to fall into the target slots naturally. Treat x87 comparison polarity and return-block order as source evidence: equivalent negation can keep semantics but swap `je/jne` and fail strict matching.

## Normalized switch / temporary-address corpus

For dense `/Os` switches, verify the enum's actual numeric base before changing source shape. A compiler normalization such as `dec` can mean the first logical enum value belongs to the default path. If the target bounds check is short and the table starts immediately after the authored body, test lexical `default` placement as well as case order; `Gui::FUN_00438046` requires default first and uses `size=body`, `compare_size=body+table`.

For VC7 `/Od` branch-local class temporaries, the target may consume the constructor's returned `this` directly. When a target shows `ctor; push eax` with no later address materialization, an old-MSVC address-of temporary expression such as `&Float3(...)` can be the real source shape. A named local followed by `&local` adds an observable `lea`. Accept the temporary form only under strict canonical replay and never replace it with asm or byte padding.

Treat early-return topology as codegen evidence. `if (value == 0) return; body;` and `if (value != 0) { body; }` are semantically equivalent at function end but can differ by a short conditional plus an explicit epilogue jump. Preserve the form demonstrated by the target.

## x87 named-home / static-owner corpus

When VC7 `/Od` target code stores an x87 result with `fst [local]` and immediately continues arithmetic without reloading it, test a real named source local rather than an artificial spill. A named value assigned from the call and used in the following expression can make VC7 preserve both the stack home and the live ST0 value; use `#pragma var_order` only to place proven locals.

Canonical evidence should come from a COFF function symbol with a reproducible extent. If an already-exact TU-local `static` helper has only a raw/internal aux record, prefer promoting that existing implementation to its real namespace-level owner when ownership is semantically valid and codegen remains exact. Do not add comparator exceptions merely to accept an unbounded symbol.

## Auxless compiler COMDAT acceptance

Some VC7 compiler-generated functions have no function-definition aux record even though their `.text` COMDAT is an isolated, reproducible body. Do not hand-write a fake deleting destructor merely to manufacture an aux record. Use `allow_auxless_comdat = true` only when the manifest intentionally owns that compiler body and the comparator can prove all of: code+COMDAT section, function symbol at offset zero, exactly one section-defined external function owner, and section length equal to the full comparison extent. Relocations must still match and replay normally. Undefined symbols of the same decorated name are references, not competing body owners.

For a real implicit class constructor, prefer a real explicit empty constructor if that preserves bytes: `AnmVmBase::AnmVmBase` demonstrates that member initialization remains compiler-generated while the explicit owner gains a standard COFF function extent. Keep ordinary functions off the auxless path whenever a natural source-level owner can provide normal canonical evidence.

## Declaration initialization versus constructor timing

When a target writes a scalar/pointer local before constructing a later non-trivial local, test a declaration initializer rather than a body assignment. VC7 may hoist non-trivial local construction ahead of ordinary body statements, while C++ declaration initialization preserves declaration sequencing. `ECL EX FUN_00423A60` requires the bullet cursor to be initialized at declaration before a later `Float3` constructor.

Short-circuit operand order is machine-code evidence even when the boolean expression is commutative. For `a == 0 || b == 0`, VC7 emits the first stack-home compare exactly in lexical order; do not reorder equivalent operands after the target has established their sequence.

## Aggregate ownership from repeated fixed-size global spacing

When several absolute globals differ by an already-attested type size, test an aggregate owner before introducing independent globals. The TH08 ECL EX barrier block has `AnmVm` instances at `0x004E4B68` and `0x004E4E0C`, exactly `0x2A4` bytes apart. Modeling `g_EclExBarrierRenderState @ 0x004E4B60` with `vm0 @ +8` and `vm1 @ +0x2AC` naturally reproduces all target member-address relocations.

Do not infer callback parameters from the functions that publish the callback address. A stored function pointer may target a no-argument callback even when the publisher itself is fastcall with ECX/EDX inputs. Use the callback target prologue and actual indirect call sites; update relocation symbol spelling when ABI recovery changes the source declaration.

Non-trivial local arrays are compiler structure, not just storage. If the target calls VC7's vector-constructor iterator with element size/count/ctor, express the local as a real C++ array of that element type and keep its lexical lifetime. Also retain target-proven dead locals instead of deleting them as cleanup.

## Stack order versus constructor order

`#pragma var_order` controls VC7 local stack homes but does not require declarations to follow that same order. For multiple non-trivial locals, recover two separate facts from the target: physical slots and constructor call chronology. The ECL EX laser-hitbox trio constructs `origin -> outer -> inner -> position` while the stack is ordered nearest-EBP as `position, outer, inner, origin`; a matching pragma plus semantic declaration order reproduces both facts.

## Positive body gates

When a target tests a mask and then skips a large region with one forward branch, write the source as a positive wrapper around the body. Replacing it with an equivalent reject-and-`continue` commonly generates `jcc + short jmp` and changes the function by two bytes. This pattern repeats in ECL EX bullet callbacks at `0x00424A20`, `0x00424C40`, and `0x00424E50`.

For floating ternaries, recover comparison polarity and expression operand ownership separately. `constant + local` can be required even when `local +/- constant` is algebraically identical, because VC7 chooses different x87 load/add/sub forms.

## Empty aggregate constructors as layout attestations

An empty user-written constructor can still compile to a large target when its class contains non-trivial members and arrays. If the target consists of the expected member ctor calls/vector-constructor iterators, prefer fixing the class layout and leaving the body empty. `AsciiManager::AsciiManager @ 0x00402000` becomes 0x128-byte exact solely from correct member ownership/order.

## Distinguish authored body size from COFF aux extent

A VC7 function symbol's aux `total_size` may include an immediately associated jump table. If the target's authored mapping stops at the first table byte, keep authored progress at that body size and set the canonical unit's `compare_size` through the table. `AsciiManager::OnDrawLowPrioImpl` is 0x6A2 authored bytes but 0x6B6 compared bytes because of its five-entry table.

## Correct runtime helpers out of authored inventory

Do not force standard math/runtime helpers into authored reconstruction merely because an imported seed marked them `function`. If the target name/provenance and body establish a CRT/x87 helper (`_sinf`, `_cosf`, `_sqrtf`, `fabs`, `fmodf`, `fsincos` in TH08), change the inventory type to `library`. This is a classification correction, not an exact-match claim; revisit the helper only after authored work is complete. Never use inline asm just to mimic such runtime stubs.

- VC7 `MATH.H` itself is valid provenance for float wrapper classification: inline `acosf/atanf/tanf` lower to tiny x87 wrappers around CRT double cores. If the target has that wrapper shape and the core is runtime-owned, classify the wrapper as library; do not write inline asm or count it as authored reconstruction.


- In large VC7 `/Os` setup functions, local-vs-global receiver ownership, helper placement, table dimensionality, and loop spelling are first-class exact-match evidence. Equivalent refactors can move dozens of bytes.
- If POD `new/delete/free` traffic has stable stack work homes, model the real allocation/deallocation pointers as semantic locals and control them with `#pragma var_order`; do not use padding or asm. Direct `operator new/delete` is acceptable only when the owner has no constructor/destructor work and full canonical replay proves equivalence.
- A target dword work value later consumed as one byte should not automatically be declared `u8`. Preserve the dword source shape when target stores prove it.
- When a same-type totals record physically follows an array, a narrow overlay for the contiguous table can recover the original constant-index AST without undefined one-past-array access.
