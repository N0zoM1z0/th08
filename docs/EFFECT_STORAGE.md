# Effect storage and callback roles

`EffectManager` owns every `Effect` returned by its spawn functions. Callers
borrow these pointers; the manager controls their lifetime. The pool contract
observed in [EffectManager.cpp](../src/EffectManager.cpp) is:

| Range | Role | Allocation behavior |
| --- | --- | --- |
| `effects[0x000..0x1ff]` | Primary circular pool | `SpawnEffect*` scans at most 0x200 entries using `nextEffectIndex`. |
| `effects[0x200..0x27f]` | Secondary linear pool | `SpawnEffectInSecondaryPool` scans 0x80 entries from the beginning. |
| `effects[0x280..0x28c]` | 13 fixed slots | Fixed-slot helpers index directly and do not bounds-check the caller's slot. |
| `effects[0x28d]` | Exhaustion sentinel | Returned by a primary/secondary scan that reaches its limit. |

An initializer may mark a selected entry inactive. Check its active state
separately from pointer validity. The fixed-slot API always returns the
selected manager entry. Callers should use the spawn contract and retain any
target-observed checks.

## Scratch-vector role matrix

The numbered vectors are storage slots shared by several callback protocols.
Their role is local to the template's initializer/update pair.

| Callback family | Observed vector roles |
| --- | --- |
| Velocity-aware spawners | `vector1` receives the caller-provided seed/velocity before initialization. |
| Random splash | `vector2` is per-frame motion; `vector3` is its delta/deceleration. |
| Boss-tracking camera particles | `vector4` is camera-space position; `vector2` is velocity; `vector3` is acceleration; `vector1` contributes caller-provided bias. |
| Directional offsets | `vector5` stores the origin and `vector6` the direction/offset used by the update. |
| Radial trails | `vector1.x/y/z` seed angle, radius, and thickness; `vector5` is the world anchor; `vector6` and `vector7` initialize basis directions. Later shape state has dedicated named fields. |
| Anchored radial trail | `vector5` is refreshed from `vm.pos` by `SyncAnchoredRadialTrail`. |

Give these roles names in callback-local variables or a local view verified
to preserve layout and code generation. The shared `vector1` through `vector7`
storage retains neutral names across callback families.

`EffectManager::scaleX`, `scaleY`, `scaleZ`, and `scaleW` are initialized to
`1.0f`, but no current production consumer establishes distinct semantic
roles, so they retain neutral names.
