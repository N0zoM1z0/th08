# Player matching notes

Reusable VC7 source-shape evidence from the TH08 Player option/shot callback family.

- Reusing the shared `PlayerOptionState` state-1/state-2/state-3 lexical skeleton is more reliable than reconstructing each option callback independently.
- Under `/Od`, do not introduce a C++ reference alias when the target repeatedly reloads a scalar member. In `FUN_00450320`, `f32 &speed` added a 4-byte local home; repeating the `slot+0x44C` member expression restored the target frame.
- Aggregate assignment and component assignment are not interchangeable. `FUN_00450AD0` needs three separate `vm.pos.x/y/z = vectors[i].x/y/z` stores so VC7 recomputes `i*0xC` three times; aggregate copy shortened the body by 34 bytes.
- Preserve branch-local lifetime for non-trivial `Float3` work. `FUN_00450EE0` constructs its effect position only inside the every-eighth-frame branch; function-scope declaration hoisted the constructor and changed 52 bytes.
- The target sometimes writes `vm.pendingInterrupt = 1` directly instead of calling `SetInterrupt(1)`. `FUN_004505D0` and `FUN_00450840` require the direct field owner.
- Avoid chained assignments when target store order matters. `FUN_00450C50` needs `hitboxSize.x = value; hitboxSize.y = value;`; `x = y = value` reverses the stores under VC7.
- Keep associated tables out of authored progress: `FUN_0044F5E0` is 0x32A authored / 0x34E compared, and `FUN_00450C50` is 0x263 authored / 0x287 compared.
- The SHT callback ABI is consistently `__fastcall(Player *player, PlayerShot *slot, ...)`: ECX owns Player, EDX owns the shot, and extra SHT arguments are stack parameters. A claim-safe free-function probe reproduces this without editing the active Player TU.
