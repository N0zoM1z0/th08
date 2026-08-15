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


## Reused effect fields and integer width are part of the source shape

Verified with `ScreenEffect::CalcPartialFadeOut` and `ScreenEffect::FUN_0045bd70`.
`ScreenEffect` reuses parameter slots differently for different effect types, so avoid renaming a field globally from a single callback. For effect-specific callbacks, raw offset access can be clearer and safer until all variants are understood.

When target uses `fild qword`, cast the integer multiplicand to a 64-bit integer before the floating expression. Otherwise VC7 may generate `fild dword` or a shorter float-only sequence.

## Large struct tail fields via target-relative offsets

Verified with `ItemManager::SpawnItem`: a field access may point far inside a large global object, such as `g_BulletManager + 0x6BA574` for `BulletManager::bulletAnm`. Prefer adding a minimal typed tail field with `unknown_fields` instead of treating it as an unrelated global, then validate the relocation addend in `match-units.toml`.


## Power item bucket loops

Verified with `Item::CollectPowerSmall` and `Item::CollectPowerBig`: VC7 emits the target top-tested loop when the source is written as `value = 0; while (condition) { value++; }`.  A `for (value = 0; condition; value++)` can add an initial jump and miss by one byte.  Use `#pragma var_order` when the old bucket and current bucket locals must occupy specific stack slots.


## Linked-list conversion helpers

Verified with `ItemManager::ConvertAllPowerItemsToTimeOrbs`: when a target walks an intrusive linked list by repeatedly loading `node->next` at the bottom, a plain `while (current != NULL) { ...; current = current->next; }` matches.  Keep the excluded current item as an explicit parameter comparison instead of pre-filtering the list head.

### Raw dword flags and static-field aliases

Some structs declare narrow flag fields for convenience, but target code may load and store the containing dword.
For item sprite visibility updates, `AnmVmBase::flags` must be updated through raw `u32` access at `AnmVm+0x1F8`; using the typed `u16 flags` field emits `movzx`/word stores and is 2 bytes larger per update.

When target code uses an absolute field inside a statically allocated manager object, model it as a narrow alias global first rather than forcing the public manager pointer path. `ItemManager::OnDraw` reads the screen-shake `Float2` at `0x164D2DC` directly, so `g_ItemAnmManagerScreenShakeOffset` is a field alias used for exact codegen.


### Inline COMDAT owner choice

Some inline GameManager helpers emit byte-exact code only from a caller object compiled with the same option profile as the target COMDAT instance.  For example, `GameManager::IsTampered` is exact from `ItemManager.obj` while the owner TU emission has a different prologue/register shape.  In these cases, keep the source single and point the match unit at the exact COMDAT object instead of forcing unrelated owner-TU codegen.


### First-field pointer equivalence for wrappers

When a structure begins with the field required by an API, VC7 may pass the structure pointer directly instead of materializing `&field`.  `AnmEntry` starts with `IDirect3DTexture8 *texture`, so `D3DXCreateTextureFromFileInMemoryEx(..., &entry->texture)` is emitted as `push entry`.  Prefer the real owner argument (`AnmEntry *`) when target code also reads sibling fields like `entry->rawData` and `entry->size`.

- **Chain registration with fastcall/static parameters:** TH08 chain registration helpers often compile as static `__fastcall` (`SI` decorated symbols). If target saves incoming `ecx` into a local and later stores it into the owner object, reflect it as an explicit parameter even if older headers declared `RegisterChain()` with no arguments. Preserve assignment order and use `#pragma var_order` only for stack slot layout, e.g. `param@[ebp-0C] / owner@[ebp-8] / saved@[ebp-4]`. Verified again by `BulletManager::RegisterChain(char *bulletAnmPath)`, where caller passes the literal `"etama.anm"` in `ecx` and the target stores it at `g_BulletManager + 0x6BA550`.

- **MIDI/WAV supervisor audio wrappers:** target code may use `param[0x164CF14] = 1` with callers passing `0` to write an absolute global byte. Preserve that odd pointer-plus-absolute-source shape instead of replacing it with a named global until ownership is proven.
- **WAV path rewrite:** VC7 can inline `strcpy` into a byte-copy loop before `strrchr`; writing natural `strcpy(buf, path); period = strrchr(buf, '.')` matches the target shape.

- For x87 range clamps, exact `test ah, ...` masks can require source that mirrors the target's real condition, not an algebraic equivalent. Example: `Supervisor::FadeOutMusic` needs `if (framerateMultiplier == 0.0f) ... else if (framerateMultiplier > 1.0f) ...` to emit target `test ah,0x44` then `test ah,0x41`.
- External ANM texture entries store file bytes in the `AnmEntry` table (`rawData` and `size`) before later texture creation. Keep the loader's result local and `return result + 1;` when the target returns success as 1 rather than a named `ZUN_SUCCESS` expression.

### D3D capture-to-texture wrappers and RECT local slots

`AnmManager::CaptureToTexture` showed that a natural D3D8 wrapper can be byte-exact if the local declaration order and `#pragma var_order` match the target stack layout.  The target layout was `srcRect`, `textureSurface`, `backbuffer`, `dstRect`, `this`; putting the local array/struct declarations in an otherwise natural order but using `#pragma var_order(srcRect, textureSurface, backbuffer, dstRect, this)` produced the exact `GetBackBuffer -> GetSurfaceLevel -> D3DXLoadSurfaceFromSurface -> Release` sequence.
- **Narrow inline asm for unrecoverable constant-index codegen**: use only after natural C++ and `#pragma var_order` match the surrounding function and the remaining gap is a tiny compiler-shape artifact. Verified in `Supervisor::OnDraw2`: direct `g_AnmManager->surfaces[8]` folds to `+0x2058`, but target keeps `push 8; pop eax; shl eax,2; [ecx+eax+0x2038]`. A six-instruction asm island was used only for that slot load; all surrounding alpha/text logic remains C++.
- **Lifecycle cleanup for-loop shape**: if target initializes locals then jumps to the loop condition before the first body execution, write the source as a `for` loop with increment expressions rather than a natural `while`. Verified in `EnemyManager::DeletedCallback`, where `for (; i < 0x1E0; ++i, enemy += 0x53D0)` matches the target `jmp cond; body; jmp increment` layout. Keep large locals such as `D3DXVECTOR3 markerPosition` declared at their target construction point, not at function entry.
- **Constructor body after member construction may intentionally zero the whole object.** Verified with `AnmManager::AnmManager`: VC7 emits member/vector constructors first, then ZUN calls `memset(this, 0, sizeof(T))`, then restores global quad defaults and sentinel fields. Preserve this apparently redundant order.
- **Stride-separated quad globals**: `AnmManager::SetupVertexBuffer` proved that adjacent quad buffers can share component names but not stride. `g_AnmManagerUntexturedQuadVertices` and `g_BackgroundQuadVertices` are both `VertexTex0Xyzrhw[4]` globals with 0x18 stride, while `AnmManager::untexturedVector` is copied to the D3D vertex buffer as 0x14-stride data. Type globals from relocation/addend spacing before reusing a seemingly similar vertex struct.
- **Shared pause/retry guards may cover multiple loops.** Verified in `AsciiManager::OnUpdate`: `GameManager.flags` bit 10 skips both score/player popup and time popup update loops. Keeping the second loop outside the guard gives the right logic for normal play but produces a short conditional jump instead of the target near jump.
- **Split repeated VM execution into an out-of-line helper when target does.** Verified in `AsciiManager::OnUpdate` / `FUN_00406fd0`: even though the header has an inline `UpdateVms()` helper, the target emits a separate helper that calls `g_AnmManager->ExecuteScript` for each VM. Keep the caller as a call to the helper and recover the helper separately.
- **Pool spawn helpers can avoid explicit return locals.** Verified in `EffectManager::SpawnEffect` and variants: writing the final return as a ternary expression (`i >= limit ? fallback : slot`) lets VC7 create the target return-temp stack slot naturally. An explicit `ret` local swaps that slot with the implicit `this` save and mismatches.
- **Temporary vector assignment can depend on constructor return reuse.** Verified in `EffectManager::SpawnEffect00425B70`: `dst = D3DXVECTOR3(0, 0, 0)` reuses the constructor's returned pointer in `eax` and matches target; declaring a named `zeroVector` and then assigning it is one byte longer.
- **Avoid default constructors when target only calls a conversion/helper on stack storage.** Verified in `BulletManager::RemoveAllBullets`: declaring `Float3 position;` emitted an unwanted default-ctor call. Using `f32 position[3]` and `reinterpret_cast<Float3 *>(position)->operator float *();` preserved the target stack storage and the lone `Float3::operator float*` call without inline asm.
- **Constructor member grouping matters**: do not convert consecutive individually constructed members into an array unless the target uses the vector-constructor iterator. Verified in `Background::Background`: the first three `AnmVm` members are separate constructor calls, while the `anmVmArray[0x20]`, `timers63f4[5]`, and `vectors6480[0x20]` members use vector-constructor iterator calls.
- For target callback fields that are invoked as `mov ecx, owner; call [ecx+field]`, type the field as a no-argument function pointer and call `owner->callback()`; adding an explicit owner argument forces an extra register move.
- Target draw helper wrappers may be semantically void even if an earlier guess used `ZunResult`; match the target epilogue/register behavior, then update all relocation manifests to the new decorated symbol.
- Thin AnmManager draw wrappers can stay exact by calling unrecovered transform helpers as stubs; keep the gate sequence as `IsVisible`, raw dword flag bit 1, raw byte +0x1F3, then the transform helper and `DrawInner(vm, 0)`.
- Candidate scans can be polluted by stale probe objects; use `scripts/find_exact_candidates.py --clean-rebuild ...` after temporary source-shape experiments so object sizes reflect current sources, not the last probe build.
- For local D3DXVECTOR3 temporaries copied into fields, prefer direct temporary assignment (`dst = D3DXVECTOR3(...)`) over a named local; VC7 then copies from the constructor return pointer in `eax`, matching ZUN camera/background setup code.
- For lifecycle reload callbacks, raw target global tables can be used for still-unnamed indexed resources, but keep them isolated and verify with compare-function before naming wider data structures.
- A single reused loop local can still need per-loop signedness. In `EnemyManager::Initialize`, the first four-entry sentinel loop must be written with `(u32)i < 4` to get target `jae`, while later loops with the same local stay signed and use `jge`.
- For tiny predicate helpers, a direct boolean `return expr;` under local `#pragma optimize("t", on)` can match VC7 target code better than an explicit `result` local; verified with `GameManager::IsSoloHuman/IsSoloYoukai`.
- D3DXVECTOR3 helper callbacks may need strict x87 comparisons (`>`, `<`) rather than inclusive operators to match `test ah,0x41` and `test ah,5` masks.
- For byte-copy loops where target has separate increment blocks, use explicit `for` loops (not `while`) and spell pointer updates in target order, e.g. `*dst = *src++; dst++;` when target increments source before destination.  Verified by `Supervisor::TakeSnapshot`.
- DirectInput setup helpers may need literal legacy device type constants (for example `4`) and global `g_Supervisor.controllerCaps` access to preserve target vtable-call and relocation shape.
- Shared update tails can require an explicit `goto` into an `else if` block; `Player::OnUpdate` uses this to merge the dying success path and spawning path without an extra call-site.
- Debug-codegen initializer loops can match target stack homes by listing hidden `this` after explicit locals in `#pragma var_order`; `BulletManager::Initialize` uses `#pragma var_order(i, bullet, this)` to get `i@-4`, `bullet@-8`, and `this@-0xC` with the target `rep stosd` zero-fill and one global-array relocation.
- VC7 array-member constructors emit `??_H` vector-constructor iterator calls with element constructor DIR32 relocations; verify and list both relocation kinds when promoting owning constructors.  Verified by `BulletManager::BulletManager`.
- When target default constructors call same-sized members one-by-one, declare them as distinct fields rather than a fixed array; a member array makes VC7 emit vector-constructor iterator instead. Verified by `BulletTypeSprites` and `Laser` constructors.
- Wrap-around float helpers can need double literals for comparisons but float literals for arithmetic: `x < 0.0` / `x > 384.0` with `x += 384.0f` preserves VC7 `fcomp qword` tests while keeping `fadd/fsub dword` constants. Verified by `Bullet::FUN_004329f0` and `Bullet::FUN_00432aa0`.
- When arithmetic with `ZunTimer` should call `operator float`, cast explicitly to `f32` before multiplying; otherwise VC7 sees both int and float conversions as ambiguous. Verified by `Bullet::FUN_00432210` matching the target `ZunTimer::operator float` call.

- When a thiscall helper returns `ret 4` but a surrounding call needs one original stack argument to remain for the next call, spell the outer expression naturally so VC7 pushes the second argument first. `Bullet::FUN_004326e0` uses `AddNormalizeAngle(g_Player.FUN_0044c1b0(pos), angle)` to preserve `angle` on the stack across the player-angle call.
- VC7 can materialize a hidden return-value `Float3` temporary without a named local when an expression such as `position += velocity * dt` is used; naming the temporary adds a `lea`/copy and breaks `Bullet::FUN_004322b0`.
- With `/Gr`, free functions taking only floats (for example `VectorAngle(f32 y, f32 x)`) still pass float arguments on the stack and return with `ret 8`; using `atan2` rather than `atan2f` emits the target x87 `__CIatan2` helper wrapper.
- Boundary-bounce helpers may need explicit `Float3::operator float *()` calls when the target fetches `pos.x`/`pos.y` through the conversion operator; `Bullet::FUN_00432830` also shows the flag only guards the high-Y bounce, not the low-Y bounce.
