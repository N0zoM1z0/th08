# ANM resource namespaces

ANM uses separate namespaces for manager file slots, resource-local scripts
and sprites, and `AnmVm` array indices. Identify the owner and receiving API
before interpreting a number.

## Manager file slots

`AnmFileSlot` in [AnmManager.hpp](../src/AnmManager.hpp) names all 25 stable entries in
`AnmManager::anmFiles`: text, ASCII, loading/capture, stage background, player,
bullet/effect, common and stage enemies, stage effects, GUI resources, portrait
and face resources, title/result/music room, and ending. These values identify
the loaded `AnmLoaded` owner; scripts and sprites have their own local IDs.

## Local IDs

| Domain | Owner that gives the number meaning | Current source landmarks |
| --- | --- | --- |
| Player scripts | The shot-type player ANM loaded into `Player::anmFile` | Main player scripts 0–9 are named in `Player.hpp`. Option setup currently uses raw file-local scripts 18, 24, and 29 at the observed `Player.cpp` call sites. |
| Bullet/effect scripts | The bullet/effect or stage-effect `AnmLoaded` selected by the manager | `g_EffectTemplates` in `EffectManager.cpp` maps 66 effect IDs to file-local script IDs and callbacks. Effect ID and script ID are different namespaces. |
| Title sprites/VMs | `TitleScreen::titleAnm` plus the `TitleScreen::vms` array | Named ranges at the top of `TitleScreen.cpp` cover option controls, key configuration, characters, and difficulty. The same integer is meaningful only with the API receiving it. |
| GUI sprites | `frontAnm`, auxiliary GUI ANM, stage text ANM, or clock ANM | Follow the concrete `AnmLoaded` receiver in `Gui.cpp`/`AsciiManager*.cpp`; the native owner audit shows why a visually related GUI object is not sufficient evidence. |
| Enemy/stage scripts | Common enemy, stage enemy, background, or stage-effect owner | ECL operands and callbacks select the owner separately from the raw script value. Preserve that selection in names and documentation. |

Player option scripts retain their observed numeric IDs. A descriptive rename
needs the owning loaded file, TH08 call sites, animation role, and exact/portable
validation results; the shipped asset provides no source-level symbolic names.

## ANM opcode evidence queue

| Opcode | TH08 observation | Current decision |
| --- | --- | --- |
| 25 | Writes `AnmVm::type`; no independent consumer has yet closed its role. | Keep neutral. |
| 31 | Writes `flag15`; `SetRenderStateForVm3D` compares it with the manager camera mode and selects a background camera. | Behavior documented; defer a shared-layout rename until the full consumer set is audited. |
| 83 | Writes the value consumed only by `DrawPlayerBullet`, which dispatches the six named `AnmPlayerBulletDrawMode` values. | Named `AnmOpcode_SetPlayerBulletDrawMode` / `playerBulletDrawMode`; acceptance is recorded in the semantic batch log. |
| 88 | Copies `byteArgs[1]` to `flag17`; render paths choose `color2` instead of `color1` while set. | Behavior documented; keep the byte-width protocol and defer a shared-layout rename. |

TH06/TH07 opcode names provide corroboration. TH08 reads, writes, and exact
comparison establish the interpretation for this target.
