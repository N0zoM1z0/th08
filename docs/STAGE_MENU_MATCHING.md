# Stage menu matching notes

- TH08 Pause/Retry menu code remains structurally close to TH06 `StageMenu`, but TH08 has a 10-sprite Pause menu and a 6-sprite Retry menu. TH06 is a useful lexical skeleton; TH08 target code must still arbitrate state IDs, script IDs, and gates.
- The background-capture gate is lexically `((g_EclGameTimeScaleFlags >> 1) & 1) != 0`. Simplifying it to `flags & 2` shortens both draw callbacks by two bytes (`shr ecx,1; and ecx,1` versus `and ecx,2`).
- Pause and Retry draw loops intentionally differ in counter signedness. Pause's 10-sprite loop uses an unsigned counter (`jae` at the bound), while Retry's 3/4-sprite loops use signed counters (`jge`).
- Pause draw copies `menuBackground` to a branch-local `AnmVm`, sets `zWriteDisabled`, then draws the local. Preserve the typed aggregate copy so VC7 emits the target `rep movsd` register ownership.
