# Verified VC7 / ZUN source-shape patterns

These notes collect source-shape patterns verified by `compare-function.py` against the hash-checked TH08 1.00d target. Treat them as starting points for probes, not as proof for a new function until the comparator is exact.

## Direct boolean return can create the target result local

Several target functions materialize a `this` local and a result local even when the readable C++ is a direct boolean expression. The direct expression is often closer than hand-written nested assignments.

Verified examples:

- `EclOperands::TargetEnemyHelpersOverlay::HasParentChain` at `0x0041F000`
- `Spellcard::GetActiveState` at `0x0041FD90`
- `Spellcard::FUN_00417860` at `0x00417860`

Shape:

```cpp
return condition_a && condition_b;
```

Avoid prematurely expanding this into nested `if` blocks unless the target uses those explicit branches.

## `__fastcall` static helpers for ECX-only scalar queries

Some ZUN helper functions are logically static queries but receive their sole scalar argument in `ecx` and return with a plain `ret`. A static `__fastcall` member can model this without inventing a fake object.

Verified example:

- `Spellcard::IsLastSpell(i32)` at `0x00414540`

Shape:

```cpp
static i32 __fastcall IsLastSpell(i32 value);
```

## Probe-only exact functions should not break detour generation

Some exact units are built only in `build/probes/*.obj` and are not linked into the main detour input objects. `generate_detours.py` now warns and skips implemented symbols absent from the current detour input instead of failing the full build.

Verified by full build after probe-only ECL helpers were marked exact.

## Candidate scanning workflow

Use `scripts/find_exact_candidates.py` to discover defined symbols that already match `mapping.csv` names but have no `matches.csv` entry. It is a triage tool; every selected function still needs a normal `match-units.toml` entry and `compare-function.py` proof.


## Optimize pragma for `leave` epilogues

Verified examples: `AsciiManager::SetIsGuiMode` (0x004398FF) and
`AnmVm::FUN_004396f8` (0x004396F8).  Some tiny state helpers only match the
target when compiled under `#pragma optimize("s", on)`, which makes VC7 emit
`leave` instead of `mov esp, ebp; pop ebp`.  Prefer proving this with a real
comparator before changing a function globally.

## Raw-width flag loads

Verified example: `AnmVm::FUN_004396f8`.  If target reads a flags word as a
32-bit dword (`mov eax, [this+off]; shr eax, imm`) but the public field is a
bitfield or narrower typed member, use an explicit raw-width access in the
small helper.  Typed access may emit `movzx`/`sar` and miss the target even when
the C++ value is equivalent.
