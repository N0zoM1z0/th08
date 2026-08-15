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


## Member-constructor layout can recover large ctor batches

Verified with `GuiImpl::GuiImpl` and `GuiMsgVm::GuiMsgVm`: when target code is a
sequence of `ZunTimer` / `AnmVm` / small aggregate constructor calls, model the
real object layout with typed members and arrays instead of writing placement
`new` calls.  VC7 emits `eh_vector_constructor_iterator` for typed arrays and
plain member constructor calls for single objects, matching the target exactly.

## Signedness controls conditional branch opcodes

Verified with `Gui::MsgWait`: a raw state field compared against zero needed an
unsigned `u32` load to emit target `jbe`; using `i32` emitted signed `jle` with
otherwise identical bytes.  For raw flag/counter fields, infer signedness from
the target conditional jump before choosing field type.


## Cast placement changes x87 code shape before `__ftol2`

Verified with `ScreenEffect::CalcFadeIn` at `0x0045B160`.  The target computes
`255.0f - scaledTimer` in x87 and only then converts through `__ftol2`:

```cpp
(i32)(255.0f - ((255.0f * (f32)timer) / duration))
```

Writing the algebraically equivalent `255 - (i32)scaledTimer` emits an integer
subtraction after `__ftol2` and misses both instruction shape and relocation
offsets.


## D3D8 API wrappers can be reconstructed from vtable order

Verified with `ScreenEffect::Clear` (`0x0045B020`) and
`ScreenEffect::SetViewport` (`0x0045B0E0`).  The target follows the standard
Direct3DDevice8 call order: `Clear`, `Present`, fallback `Reset`, and
`SetViewport`.  Once the SDK call sequence is identified, natural C++ method
calls can match the target exactly; the remaining work is usually relocation
manifest offsets/addends for `g_Supervisor` fields.


## MSVC conversion operator names in config

VC7 mangles conversion operators as `??B...`; the return type is encoded near the
end of the decorated name, not in the `??B` operator token itself.  The detour
name normalizer now maps verified common cases to stable config names:

- `??BZunTimer@@...QAEMXZ` -> `ZunTimer::operator_float`
- `??BZunTimer@@...QAEHXZ` -> `ZunTimer::operator_int`
- `??BFloat3@@...QAEPAMXZ` -> `Float3::operator_float_ptr`

This avoids treating conversion operators as unknown symbols during full detour
builds and keeps `implemented.csv` names stable.


## Complete small operator batches before resuming larger owners

When a larger function exposes compiler-emitted inline operators, promote the
whole nearby operator family when possible.  `ScreenEffect` exposed missing
`ZunTimer` conversion/comparison emissions, so the exact batch now records
`operator_float`, `operator_int`, `operator==`, `operator<=`, and `operator>=`
together instead of leaving one-off helpers scattered across future work.


## D3D8 screen-effect quads use triangle strips

Verified with `ScreenEffect::DrawSquare` and `ScreenEffect::DrawSquareShaded`.
The four `VertexDiffuseXyzrhw` corners are laid out top-left, top-right,
bottom-left, bottom-right, then drawn with `D3DPT_TRIANGLESTRIP`, not
`D3DPT_TRIANGLEFAN`.  Declare the local vertex array after any preceding
statements that target emits first, because VC7 constructs typed local arrays at
the declaration point.
