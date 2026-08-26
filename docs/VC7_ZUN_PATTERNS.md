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

Use `scripts/analysis/report-reconstruction-status.py` to select a current gap.
For a conservative review artifact from existing objects, use
`scripts/analysis/propose-exact-units.py`; every proposed function still needs a normal
`match-units.toml` entry and `compare-function.py` proof.


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
- **Legacy note — do not use inline assembly in current TH08 reconstruction work.** An older `Supervisor::OnDraw2` match used a tiny asm island for a constant-index compiler-shape artifact. Treat that only as historical evidence of the target instruction sequence; new work must recover a natural C/C++ source shape (casts, condition direction, temporaries, declarations, `#pragma var_order`, or a real helper boundary) or leave the function non-exact rather than adding asm.

- **Unsigned string lengths can need an explicit `f32` conversion before mixed x87 arithmetic.** Verified by `AnmManager::DrawTextRight`: `(f32)strlen(buf) * fontWidth * scale / 2.0f` makes VC7 materialize the unsigned length through the target 64-bit `fild` temporary, then use `fimul fontWidth` and a dword float divisor. Integer `strlen * fontWidth` first emits a one-byte-longer integer multiply path; `f64` changes the divisor to qword.
- **Equivalent ternaries can reverse the emitted branch body.** For target code that stores the fallback first and jumps over the positive case, spell the condition in that direction. `vm->fontWidth <= 0 ? 15 : vm->fontWidth` matches `jg positive; store 15; jmp join; positive: load field`, while the algebraically equivalent `vm->fontWidth > 0 ? vm->fontWidth : 15` lays out the opposite body first.
- **Lifecycle cleanup for-loop shape**: if target initializes locals then jumps to the loop condition before the first body execution, write the source as a `for` loop with increment expressions rather than a natural `while`. Verified in `EnemyManager::DeletedCallback`, where `for (; i < 0x1E0; ++i, enemy += 0x53D0)` matches the target `jmp cond; body; jmp increment` layout. Keep large locals such as `D3DXVECTOR3 markerPosition` declared at their target construction point, not at function entry.
- **Constructor body after member construction may intentionally zero the whole object.** Verified with `AnmManager::AnmManager`: VC7 emits member/vector constructors first, then ZUN calls `memset(this, 0, sizeof(T))`, then restores global quad defaults and sentinel fields. Preserve this apparently redundant order.
- **Stride-separated quad globals**: `AnmManager::SetupVertexBuffer` proved that adjacent quad buffers can share component names but not stride. `g_AnmManagerUntexturedQuadVertices` and `g_BackgroundQuadVertices` are both `VertexTex0Xyzrhw[4]` globals with 0x18 stride, while `AnmManager::untexturedVector` is copied to the D3D vertex buffer as 0x14-stride data. Type globals from relocation/addend spacing before reusing a seemingly similar vertex struct.
- **Shared pause/retry guards may cover multiple loops.** Verified in `AsciiManager::OnUpdate`: `GameManager.flags` bit 10 skips both score/player popup and time popup update loops. Keeping the second loop outside the guard gives the right logic for normal play but produces a short conditional jump instead of the target near jump.
- **Split repeated VM execution into an out-of-line helper when target does.** Verified in `AsciiManager::OnUpdate` / `FUN_00406fd0`: even though the header has an inline `UpdateVms()` helper, the target emits a separate helper that calls `g_AnmManager->ExecuteScript` for each VM. Keep the caller as a call to the helper and recover the helper separately.
- **Pool spawn helpers can avoid explicit return locals.** Verified in `EffectManager::SpawnEffect` and variants: writing the final return as a ternary expression (`i >= limit ? fallback : slot`) lets VC7 create the target return-temp stack slot naturally. An explicit `ret` local swaps that slot with the implicit `this` save and mismatches.
- **Temporary vector assignment can depend on constructor return reuse.** Verified in `EffectManager::SpawnEffect00425B70`: `dst = D3DXVECTOR3(0, 0, 0)` reuses the constructor's returned pointer in `eax` and matches target; declaring a named `zeroVector` and then assigning it is one byte longer.
- **Avoid default constructors when target only calls a conversion/helper on stack storage.** Verified in `BulletManager::RemoveAllBullets`: declaring `Float3 position;` emitted an unwanted default-ctor call. Using `f32 position[3]` and `reinterpret_cast<Float3 *>(position)->operator float *();` preserved the target stack storage and the lone `Float3::operator float*` call without inline asm.
- **Constructor member grouping matters**: do not convert consecutive individually constructed members into an array unless the target uses the vector-constructor iterator. Verified in `Background::Background`: the first three `AnmVm` members are separate constructor calls, while the `anmVmArray[0x20]`, `timers63f4[5]`, and `vectors6480[0x20]` members use vector-constructor iterator calls. `Spellcard::Spellcard` is a stronger example: two `ZunTimer` members followed by fourteen consecutive `AnmVm` objects are still fourteen distinct members because the target emits fourteen direct `AnmVm` constructor calls; declaring `AnmVm[14]` changes the ABI/code shape to the vector-constructor iterator.
- For target callback fields that are invoked as `mov ecx, owner; call [ecx+field]`, type the field as a no-argument function pointer and call `owner->callback()`; adding an explicit owner argument forces an extra register move.
- Target draw helper wrappers may be semantically void even if an earlier guess used `ZunResult`; match the target epilogue/register behavior, then update all relocation manifests to the new decorated symbol.
- Thin AnmManager draw wrappers can stay exact by calling unrecovered transform helpers as stubs; keep the gate sequence as `IsVisible`, raw dword flag bit 1, raw byte +0x1F3, then the transform helper and `DrawInner(vm, 0)`.
- Candidate scans can be polluted by stale probe objects. Rebuild the selected
  Ninja object from current source before running `scripts/analysis/propose-exact-units.py`.
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

- When a thiscall helper returns `ret 4` but a surrounding call needs one original stack argument to remain for the next call, spell the outer expression naturally so VC7 pushes the second argument first. `Bullet::FUN_004326e0` uses `AddNormalizeAngle(g_Player.AngleToPoint(pos), angle)` to preserve `angle` on the stack across the player-angle call.
- VC7 can materialize a hidden return-value `Float3` temporary without a named local when an expression such as `position += velocity * dt` is used; naming the temporary adds a `lea`/copy and breaks `Bullet::FUN_004322b0`.
- With `/Gr`, free functions taking only floats (for example `VectorAngle(f32 y, f32 x)`) still pass float arguments on the stack and return with `ret 8`; using `atan2` rather than `atan2f` emits the target x87 `__CIatan2` helper wrapper.
- Boundary-bounce helpers may need explicit `Float3::operator float *()` calls when the target fetches `pos.x`/`pos.y` through the conversion operator; `Bullet::FUN_00432830` also shows the flag only guards the high-Y bounce, not the low-Y bounce.
- VC7 will allocate the hidden `this` home before an implicit integer-to-float `fild` temporary when the integer expression is cast directly; `GameManager::SetBombCount` matches only as `(f32)(checksum + rng7[3])`, while a named `expectedValue` local moves `this` from `-4` to `-8`.
- Do not treat a donor-local optimizer workaround as ownership evidence.
  `AddToDeaths` and `AddToBombsUsed` were first made exact in the `/Os`
  GameManager donor with `#pragma optimize("t", on)`, but target neighbors and
  production calls later proved that their natural exact definitions belong in
  the `/Od` Player TU without that pragma. `ScaleFloatBasedOnRank` does retain a
  local `optimize("t")` in the `/Od` boss-marker target cluster, and still needs
  an explicit `(f32)this->rank` cast to emit `fild rank` + `fmulp` instead of
  `fimul`.
- VC7 PCH include order can become COMDAT link order. When multiple inline
  helper families are first emitted by the same consumer, compare the target
  family order before rearranging definitions; the TH08 early Ascii cluster
  requires `SoundPlayer.hpp` before `GameManager.hpp` so Sound wrappers precede
  the GameManager setters.
- Fastcall helpers with an unused `edx` home can require an explicit dummy second parameter; for `IncrementTruncate(u32 *value, i32 unused)`, use an unsigned pointer and do not force `optimize("t")` so VC7 emits the target `push ecx; push ecx`, unsigned `jae`, and `inc eax` store-back shape.
- Header inline predicates can be promoted to out-of-line definitions when the target has a standalone copy: verified `GameManager::IsStageClearedWithoutRetries` / `IsStageClearedWithRetries`; keep the same macro expression so VC7 emits `xor edx; inc edx; shl edx, cl` instead of a boolean normalize sequence.
- Fastcall helpers may need an unused register dummy parameter to home `ecx`; for top-tested table scans, write `i = 0; while (...) { ...; i++; }` rather than `for`, or VC7 emits an entry `jmp`. Verified by `FUN_00439916` and `FUN_00439961`.
- `CALLBACK`/`__stdcall` functions using global struct members can match natural COM calls with relocation addends; verified `Supervisor::EnumGameControllersCb` emits `g_Supervisor + 0x14` and `+0x0C` DIR32 addends for `controller` and `dInputIface` without raw absolute addresses.
- Player shot draw loops can match naturally with typed `PlayerShot *slot` plus `#pragma var_order(i, slot, this)`: use a `for (i = 0; i < ARRAY_SIZE_SIGNED(shots); i++, slot++)` shape for the entry `jmp` loop and direct named VM position/color fields; verified by `Player::DrawHitShots`.
- For Player shot loops that update `Float3::operator float *()` components, avoid explicit pointer locals; direct `operator float *()[0/1] += ...` lets VC7 allocate target expression temporaries beyond hidden `this`. Verified by `Player::UpdateShots`.
- Repeated direct `this->shotTimer` expressions avoid introducing a cached timer pointer local and preserve the single hidden-`this` stack slot in timer state machines; verified by `Player::UpdateShooting`. A named member is source-shape neutral here; the rejected shape is the extra pointer local, not the semantic field name.
- DirectInput callbacks may need an explicit unused `pvRef` local to home the second stdcall argument before initializing stack structs. Verified by `Supervisor::ControllerCallback`, where `LPVOID context = pvRef;` creates the target `-0x1c` slot before `DIPROPRANGE` fields.
- Player shot initialization helpers should update `Float3::operator float *()[0/1]` directly instead of naming `f32 *xPtr/yPtr`; explicit pointer locals shift fastcall `this`/`slot` homes, while direct expressions preserve target `this@-4`, `slot@-8` and synthesize component pointer temporaries. Verified by `Player::InitializeShot`.

- Player shot-table pointer indexing can require a ternary table index rather than boolean arithmetic: `table += ((flags & 2) ? 7 : 6)` keeps VC7 from folding the `+6` into `lea [base+index*8+0x30]` and instead emits the target `add ecx, 6; lea eax, [edx+ecx*8]`. Verified by `Player::SpawnShots`.

- Player high-priority draw callbacks that index a callback table should call the table entry as a single-argument `__fastcall` function; adding a dummy second argument zeroes `edx` and breaks the target where `edx` remains the table index. Verified by `Player::OnDrawHighPrio`.

- For gauge-interrupt position gates, explicit `goto` structure can be required to force repeated reads and x87 comparison masks. `Player::FUN_0044d420` matches with `x < 160.0f` setting interrupt 2, a second `GetGaugeInterrupt()` read, and `x > 160.0f` setting interrupt 3.
- For file-loaded tables with relative offsets, use an in-place relocation update (`*relativePtr += (u32)base`) rather than `base + *relativePtr`; verified by `Player::LoadShtFile`, where the latter makes VC7 commute the add and breaks the target reload order.
- When the SHT loader local is promoted from `u8 *entry` to `PlayerShotDescriptor *descriptor`, update its `#pragma var_order` spelling too.  VC7 silently ignores the stale name and swaps the `i`/descriptor stack homes even though both locals remain four bytes.
- For the PlayerShot VM setup, assign the named bitfield `vm.zWriteDisabled = 1`; assigning through the public 16-bit `vm.flags` field emits word loads/stores and grows `SpawnShots` by two bytes, while the bitfield preserves the target dword `OR 0x2000`.
- Preserve `PlayerShot::tintInExtremeYoukai` as signed `i8`.  Testing an unsigned byte emits `movzx`; the target draw loops use `movsx`, and signedness is the only byte difference once the three named color stores are ordered `r`, `g`, `b`.
- When assigning from a global pointer and the target evaluates the destination object before the global, prefer the named global (`g_PlayerPrimaryShtFile`) over a raw absolute address; verified by `Player::FUN_0044d180`, which needs `mov eax,this` before `mov ecx,[g_PlayerPrimaryShtFile]`.

- Fastcall Player SHT callbacks with two explicit locals can require `#pragma var_order(index, i, this, slot)` to keep the decoded SHT index at `-4`, loop index at `-8`, hidden `this` at `-0xC`, and slot at `-0x10`; verified by `Player::SpawnPersistentShot`.
- For homing-angle shot callbacks, write `AddNormalizeAngle(VectorAngle(y_delta, x_delta), descriptor_angle + ZUN_PI / 2.0f)` as one expression so VC7 preserves the pending second `AddNormalizeAngle` argument across the `VectorAngle` call; verified by `Player::SpawnShotAimedAtTrackedPoint`.


- RunEcl high-opcode helpers that target a known singleton should bind the call-site `this` to that singleton, not to the EclManager service adapter.  Use `reinterpret_cast<TargetApi *>(&g_Gui)` for boss-gauge GUI calls, `reinterpret_cast<TargetApi *>(&g_EffectManager)` for effect spawns, and `reinterpret_cast<TargetApi *>(&g_Spellcard)` for spellcard state calls when the target loads those absolute objects into `ecx`.  Verified in `EclManager::RunEcl` opcodes 140, 158, and 164.
- For RunEcl boolean writes to the current ECL context, an explicit `if/else` can preserve target branch/store structure where a ternary collapses the assignment into a larger expression.  Verified by opcode 120's object-active test writing `currentContext + 0x60`.
- For RunEcl bitfield replacement from a raw byte operand, put the shifted incoming bit first and the masked existing flags second: `((raw & 1) << 25) | (flags & mask)`.  This keeps the handler span aligned for opcode 145; recheck the emitted instruction stream because equivalent commuted forms can change register order even when size is unchanged.
- When a RunEcl handler advances a slot counter that the target rereads around a spawned object store, preserve the field rereads rather than using only the cached local.  Opcode 128 uses the cached slot for entry bookkeeping but rereads `enemy + 0x53C0` for the effect table store and increment; this affects later handler alignment and must be rechecked with `scripts/analysis/historical/runecl-score-shape.py` after each probe.
- RunEcl item-drop handlers should bind concrete singleton services when the target loads them directly: use `g_Rng.GetRandomF32()` for the `0x0164D520` RNG calls and `g_ItemManager.SpawnItem(...)` for the `0x01653648` item-manager calls instead of routing through the EclManager service adapter.  Verified as a source-shape improvement in opcodes 142 and 168: op142 falls to `+2` and op168 to `+7` in the handler crosswalk, while preserving natural C++ and no inline asm.
- For RunEcl opcode 128, expressing the operand tail as a real `operands` pointer and reloading the spawned effect through the slot table matches the target's data flow better than keeping only the spawn return local.  The useful shape is `u8 *operands = instruction->operands; ... table[enemy->slot] = SpawnEffect(...); ... table[enemy->slot] + 0x2EC = *(operands + 4);`; verify the local/copy form with `runecl-score-shape` because a too-direct aggregate assignment over-shrinks the handler.
- More RunEcl singleton call-site binding: when target loads a concrete manager into `ecx`, bind that owner even if the dispatcher still uses a `TargetApi` overlay for the callee type.  Verified shape improvements: opcode 124 uses `reinterpret_cast<TargetApi *>(&g_SoundPlayer)->PlayPositioned(..., raw_x_bits)` to preserve the dword pan push and reaches exact span; opcode 139 uses `g_EffectManager.SpawnEffect`; opcode 141 uses `g_ItemManager.SpawnItem`; opcode 142 uses an out-of-line `g_GameManager.GetPower()` local rather than direct `if (g_GameManager.GetPower() < ...)`, because the direct condition inlines and over-shrinks; opcode 184 uses `reinterpret_cast<TargetApi *>(&g_Spellcard)->Call0041F0E0` and reaches exact span.
- RunEcl float operand stores may need `ReadFloatRawArg`/`TH08_ECL_READ_F_RAWARG` when the target preserves operand bits through integer moves and raw dword pushes before `ResolveFloat`; ordinary `ReadFloat` can introduce `fld`/`fstp` push and raw-branch stores.  Verified as shape improvements for opcodes 74, 152, and 172; applying it wholesale to larger movement handlers can over-shrink them, so score each opcode in isolation.
- Avoid writing through an incidental dispatcher scratch local when the target only uses the resolver temporary.  `RunEcl` opcode 163 should assign `g_EclGlobal00F54CEC = TH08_ECL_READ_I(ctx, 0)` directly, and opcode 113 should test `TH08_ECL_READ_I(ctx, 0)` directly before rereading operand 0 in the nonnegative branch.
- RunEcl opcode 181 is a concrete singleton call sequence, not a generic service-adapter sequence: use `g_GameManager.GetClockTime()`, `g_SoundPlayer.PlaySoundByIdx(...)`, `g_GameManager.AddToClockTime(1)`, and `g_Gui.FUN_00439093/50()` to match the target owners.
- For small RunEcl float arithmetic and single-float target stores, prefer raw-argument float reads when the target pushes the operand dword directly into `ResolveFloat` and stores fallback raw operands with integer moves.  Verified improvements in low opcodes 9, 15, 16, 17, 18, 32, 33, 37, and 70, and high opcodes 161, 165, 167, and 171.  Large movement handlers remain sensitive: opcode 72 accepts the first float as raw-argument shape, while replacing every float operand over-shrinks the handler.
- Remove incidental scratch locals around direct field tests when the target compares memory directly.  RunEcl opcode group 96-104 should test `enemy+0x2DFC` directly instead of caching it in `lhsInt` before the early break.
- When a RunEcl call tail loads a concrete owner singleton even though the provisional overlay method lives on `TargetApi`, bind that owner locally.  Opcode 109 keeps the existing vector-add adapter for now, but finalizes through `reinterpret_cast<TargetApi *>(&g_BulletManager)->FinalizeVectorState(...)`, matching the target's `g_BulletManager` owner and reducing the handler span without destabilizing neighboring handlers.

- RunEcl player-angle calls should bind the concrete player singleton when the target loads `0x017D5EF8` into `ecx`.  Opcodes 68 and 118 improve by calling `EclOperands::g_TargetPlayer017D5EF8.AngleToPlayer(...)`; opcode 69 was probed separately and should stay on the existing service path because the direct owner form worsened its span.
- More RunEcl owner binding remains useful for non-player helpers: opcode 125 should call `InitializeEclContext` through the target EclManager object at `0x004ECCB8`, and opcode 161 should call `SetAngleFromPosition` through `g_BulletManager` when target loads those objects directly into `ecx`.
- RunEcl raw float fallback stores can interact across nearby movement handlers.  Opcode 65's second float operand should use `ReadFloatRawArg`, but it only improves the aggregate crosswalk once combined with the opcode 74 timer-chain shape; keep scoring the whole dispatcher after each single-handler probe.
- RunEcl timer reset stores can match the target by chaining the integer store into the `ZunTimer` assignment: `*timer = (enemy_timer = ReadInt(...))`.  This removes the incidental `lhsInt` in opcode 74, but the similar-looking opcode 72 helper should not be inlined because that overgrows the handler.
- RunEcl presentation predicates should reuse `TH08_ECL_PRESENTATION_WRITES_ALLOWED()` instead of spelling the first bit test as `!= 1`; the macro emits the target `test` branch form for opcode 130.  For opcode 131, use an unsigned byte read and `!= 0` on the shifted flag to get the target `movzx`/`test` shape.
- Size-only wins are not enough for RunEcl.  Direct D3DX vector expressions in opcodes 94/109 and an over-direct opcode 128 slot reload lowered selected positive spans, but they also over-shrank neighboring handlers or increased absolute delta, so keep the more conservative source shape until the adjacent stack layout is recovered.

- For RunEcl opcode 128, the direct slot-table reload form can be retained when the whole dispatcher still improves: write the spawned effect's vector store through `TH08_ECL_AT(...0x5360 + slot * 4) + 0x2EC` without an intermediate `effect` local.  Recheck it with nearby stack-lifetime changes; by itself this form can worsen absolute delta, but paired with opcode 135's pointer clear it reduced op128 to `+1` while improving the aggregate score.
- RunEcl opcode 135's child-context zero fill is sensitive to the source loop form.  `memset` and simple index loops miss the target shape; a pointer-range loop first improved the aggregate crosswalk when paired with opcode 128.  Concrete `g_ZunMemory` owner binding for this handler was probed and rejected because it over-shrank the handler in that stack layout.
- Rejected RunEcl probes from the same pass: op66/op69 direct movement-reset expansion overgrew into large positive spans; op79 boolean locals and if/else bit writes exploded the handler; op142/168 extra `Float3::operator float*` paths overgrew item-drop handlers; op94 `Vec3`/return-local rewrites and op140 float temporaries worsened absolute delta.
- RunEcl vector-expression handlers can become viable only after nearby stack lifetimes are rebalanced.  Opcode 109's `D3DXVECTOR3` local sum form (`sum = pos + offset; enemy->vec = sum`) was previously rejected because it worsened absolute delta, but with opcode 135's count-based clear and opcode 168's aggregate copy it reduced remaining positive span without raising the max positive delta.  Re-test this expression whenever adjacent stack locals move.
- For RunEcl opcode 135, the best current zero-fill shape is count-based pointer walking (`i32 clearCount = 0x92C; i32 *clear = ...; while (clearCount--) *clear++ = 0;`).  It is closer than both the index loop and the pointer/end loop in the current dispatcher layout.
- RunEcl opcode 168 should copy the enemy position into the local `Float3` as an aggregate (`position = *reinterpret_cast<Float3 *>(&enemy_pos)`) before applying random offsets.  This matches the target's single-source setup better than three independent field assignments and removes the remaining positive span for the handler in the current crosswalk.
- During RunEcl convergence, opcode 140 required only operand 3's fallback
  store to be explicit raw bits while operands 4 and 5 stayed on
  `TH08_ECL_READ_F_RAWARG`. Applying the same explicit branch to every vector
  component over-shrank the handler; the checked-in form is part of the final
  accepted RunEcl unit.

- In RunEcl's shared post-dispatch tail, source-level `continue` can be too compact even when semantically natural.  For the interpolator callback scan, spelling the live path as `if (entry->callback) { ... }` preserves a target-like near branch around the large body.  For the child-context scan, keep the slot-table test and later slot rereads lexical instead of caching `child` before an early `continue`; this retains real behavior while moving opcode 3/84/85's shared tail span much closer to target.  Do not generalize this to inert rereads or fake locals: the repeated expression must be the actual slot lookup used by each following store/load.
- RunEcl movement handlers 66 and 69 should only inline the timer-reset tail, not the whole movement-state update.  Keeping `SetMovementState1(enemy)` as the semantic helper while spelling the timer assignment as direct field/`ZunTimer` stores brought op66/op69 close to target; fully expanding the movement-state flags overgrows both handlers into large positive spans.
- RunEcl remote-register opcodes 86 and 87 benefit from reusing the dispatcher integer scratch for the remote Boss selector before indexing `g_EnemyManager.bosses`.  For opcode 87, keep the target-observed second selector resolve in the flagged `ResolveFloat` branch; caching the Enemy pointer instead compiles differently or overgrows nearby stack layout.
- RunEcl opcode 79's bit replacement is not uniform.  The positive bit writes for `0x10` and the secondary `0x40` bit should be written as boolean shifts (`(((lhsInt & mask) != 0) << shift)`), while applying that same simple boolean-shift form to the inverted low bits over-shrinks the handler.
- For RunEcl opcode group 96-104, give the memcpy path its own `break` before the shot-dispatch call.  This is a real control-flow distinction and prevents VC7 from collapsing both arms into a shorter shared jump that misses the target span.
- RunEcl opcode 79 can be made exact only when the first inverted bit write is both lexically local and still assigned back to the shared dispatcher scratch: `i32 flags = (lhsInt = ReadInt(...));` followed by `((1 - ((flags & 1) != 0)) << 6)`.  This form by itself perturbs nearby allocation, so keep it coupled with the opcode 135 stack layout that re-stabilizes op80/op81.
- RunEcl opcode 135's child-context allocation is layout-coupled with opcode 79.  In the current dispatcher shape, concrete `g_ZunMemory.Free/Alloc` ownership plus a pointer/end zero-fill loop (`for (clear, end; clear < end; ++clear) *clear = 0`) reaches exact span for op135 and preserves the op79/op80/op81 exact trio.  Earlier layouts rejected the same owner binding, so re-score the whole dispatcher before carrying this pattern forward.
- Rejected in the same RunEcl pass: shifting opcode 128's `operands` pointer to `operands + 4` reduced positive delta by one, but over-shrank op128 and worsened absolute delta relative to the retained op79/op135 combination; op140 raw-none lowered absolute delta but raised `max_positive_delta` to 4.

- For small RunEcl flag-bit replacement handlers, source operand order can decide whether VC7 computes the incoming bit first or loads/masks the destination flags first.  Opcodes 151, 155, and 156 match the target span when written as shifted raw-byte first, then ORed with the masked existing flags; the flags-first expression is one byte shorter for each handler.
- RunEcl opcode 128 is register-allocation sensitive: keep the effect spawn before introducing the operands local. Declaring the operands pointer after the SpawnEffect call preserves the target-sized handler; declaring it before the call leaves a one-byte positive span, while pre-shifting it to operands + 4 over-shrinks op128 and worsens the overall absolute score.
- RunEcl's late high-opcode layout can be improved without changing the global float-raw macro: expanding only the final two `TH08_ECL_READ_F_RAWARG` sites (case 166's cosine radius operand 3 and the integrated case 82 operand 0) to explicit raw-bit fallback removes opcode 176's +2 span while preserving op133/op139 exactness.  Expanding the global high-side macro also removes op176's positive span, but it introduces +1 spans in op133/op139 and worsens absolute score.
