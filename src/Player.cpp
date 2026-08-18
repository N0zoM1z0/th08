#include "th_pch.h"

#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "AnmManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"
#include "ReplayManager.hpp"
#include "EnemyManager.hpp"
#include "Spellcard.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ScreenEffect.hpp"
#include "utils.hpp"

namespace th08
{

DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);

// FUNCTION: th08 0x40bc60
#pragma var_order(sourceColor, mixedColor)
void __fastcall FUN_0040bc60(Player *player, D3DCOLOR color)
{
    ZunColor mixedColor;
    ZunColor sourceColor;
    sourceColor.d3dColor = color;

    if (player->bombState.timer < 60)
    {
        mixedColor.r = 0x80 - (0x80 - sourceColor.r) * (i32)player->bombState.timer / 60;
        mixedColor.g = 0x80 - (0x80 - sourceColor.g) * (i32)player->bombState.timer / 60;
        mixedColor.b = 0x80 - (0x80 - sourceColor.b) * (i32)player->bombState.timer / 60;
    }
    else if (player->bombState.timer >= player->bombState.duration - 60)
    {
        mixedColor.r = 0x80 - (0x80 - sourceColor.r) * (player->bombState.duration - (i32)player->bombState.timer) / 60;
        mixedColor.g = 0x80 - (0x80 - sourceColor.g) * (player->bombState.duration - (i32)player->bombState.timer) / 60;
        mixedColor.b = 0x80 - (0x80 - sourceColor.b) * (player->bombState.duration - (i32)player->bombState.timer) / 60;
    }
    else
    {
        mixedColor.d3dColor = sourceColor.d3dColor;
    }
    mixedColor.a = 0x80;
    g_AnmManager->SetMixColorDefault();
    g_Background.FUN_00409160(mixedColor.d3dColor);
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Background) + 0x646C) = 1;
}

// FUNCTION: th08 0x40eb50
i32 AnmVm::FUN_0040eb50()
{
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x356) = 1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x324) = 48;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x320) = 32.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x314) =
        64.0f + (f32)((reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x338)->FUN_0040d3d0() & 1) ? 8 : 0);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x318) = 0.0f;
    return 1;
}

// FUNCTION: th08 0x40ec30
void AnmVm::FUN_0040ec30(i32 duration, i32 mode, Float3 *value0, Float3 *value1)
{
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x50) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xA4) = duration;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0xF8) = mode;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x238) = *value0;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x244) = *value1;
}

// FUNCTION: th08 0x40eca0
void AnmVm::FUN_0040eca0(i32 duration, i32 mode, u32 color0, u32 color1)
{
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x5C) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xB0) = duration;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0xF9) = mode;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x27A) = (color0 >> 16) & 0xFF;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x279) = (color0 >> 8) & 0xFF;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x278) = color0 & 0xFF;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x27E) = (color1 >> 16) & 0xFF;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x27D) = (color1 >> 8) & 0xFF;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x27C) = color1 & 0xFF;
}

// FUNCTION: th08 0x40ed50
void AnmVm::FUN_0040ed50(i32 duration, i32 mode, i32 alpha0, i32 alpha1)
{
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x68) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xBC) = duration;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0xFA) = mode;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x27B) = alpha0;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x27F) = alpha1;
}

// FUNCTION: th08 0x40eda0
void AnmVm::FUN_0040eda0(i32 duration, i32 mode, Float2 *value0, Float2 *value1)
{
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x80) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xD4) = duration;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0xFC) = mode;
    *reinterpret_cast<Float2 *>(reinterpret_cast<u8 *>(this) + 0x268) = *value0;
    *reinterpret_cast<Float2 *>(reinterpret_cast<u8 *>(this) + 0x270) = *value1;
}

// FUNCTION: th08 0x40ebc0
ZunBool ZunTimer::FUN_0040ebc0(i32 interval)
{
    return this->current != this->previous && (this->current % interval) == 0;
}

// FUNCTION: th08 0x40e350
ZunBool ZunTimer::FUN_0040e350(i32 value)
{
    return this->current != this->previous && this->current == value;
}

// FUNCTION: th08 0x40bf00
void Player::FUN_0040bf00()
{
    AnmVm *effect;
    if (this->stateEffect != NULL)
        this->stateEffect->active = false;

    effect = g_EffectManager.FUN_00425870(23, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 0, 1, -1);
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x80) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0xD4) = this->timer;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0xFC) = 0;
    *reinterpret_cast<Float2 *>(reinterpret_cast<u8 *>(effect) + 0x268) = effect->scale;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x270) = 0.0625f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x274) = 0.0625f;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x100) = (i32)this->timer;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x14) *= -1.0f;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x1F2) = 0xFF;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x1F1) = 0x40;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x1F0) = 0x40;
    this->stateEffect = reinterpret_cast<PlayerStateEffect *>(effect);
}

// FUNCTION: th08 0x40be30
#pragma var_order(i, bomb, workItem)
void __fastcall FUN_0040be30(Player *player, i32 cutInType, const char *cutInText, i32 duration, i32 timer, i32 cutInArg)
{
    u32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    g_Spellcard.CutInPlayer(cutInType, cutInText, cutInArg);
    bomb->duration = duration;
    player->timer = timer;
    player->playerState = PLAYER_STATE_DEAD;
    player->FUN_0040bf00();
    i = 0;
    workItem = bomb->workItems;
    for (; i < 128; i++, workItem++)
        workItem->active = 0;
    g_ItemManager.AutoCollectAllItems();
    bomb->tailPosition = player->position;
}

// FUNCTION: th08 0x40c820
#pragma var_order(vm, i, workItem)
void __fastcall FUN_0040c820(Player *player)
{
    PlayerBombWorkItem *workItem;
    i32 i;
    AnmVm *vm;

    FUN_0040bc60(player, 0x80404040);
    i = 0;
    workItem = player->bombState.workItems;
    for (; i < 16; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;
        vm = &workItem->vms[0];
        vm->pos = workItem->anchor + vm->pos2;
        vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        vm->pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
    }
}

// FUNCTION: th08 0x40d010
#pragma var_order(vm, i, workItem)
void __fastcall FUN_0040d010(Player *player)
{
    PlayerBombWorkItem *workItem;
    u32 i;
    AnmVm *vm;

    FUN_0040bc60(player, 0x802020d0);
    i = 0;
    workItem = player->bombState.workItems;
    for (; i < 128; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;
        vm = &workItem->vms[0];
        vm->pos = workItem->anchor + vm->pos2;
        vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        vm->pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
    }
}

// FUNCTION: th08 0x40d100
#pragma var_order(bomb, effect, i)
void __fastcall FUN_0040d100(Player *player)
{
    PlayerBombState *bomb;
    AnmVm *effect;
    u32 i;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        if (!g_GameManager.IsSpellPractice())
            FUN_0040be30(player, -1,
                         "\x81\x75\x83\x66\x83\x42\x83\x5D\x83\x8B\x83\x75\x83\x58\x83\x79\x83\x8B\x81\x76",
                         120, 200, 0);
        else
            FUN_0040be30(player, -1,
                         "\x81\x75\x83\x66\x83\x42\x83\x5D\x83\x8B\x83\x75\x83\x58\x83\x79\x83\x8B\x81\x76",
                         40, 200, 0);

        g_EffectManager.SpawnEffect(12, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xff4040ff);
        effect = g_EffectManager.FUN_00425870(50, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 4, 1,
                                              0xff4040ff);
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x50) = 0;
        if (!g_GameManager.IsSpellPractice())
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0xA4) = 90;
        else
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0xA4) = 30;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0xF8) = 5;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x238) = 8.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x244) = 128.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x23C) = 64.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x248) = 0.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x208) = 8.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x20C) = 64.0f;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 64;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x318) = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 8.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 15.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 6.0f;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.0f;

        for (i = 0; i < 8; i++)
        {
            if (reinterpret_cast<Enemy **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0)[i] != NULL)
            {
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(
                    reinterpret_cast<Enemy **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0)[i]) + 0x3324) &=
                    ~0x40u;
            }
        }
    }
}

// FUNCTION: th08 0x40d310
#pragma var_order(bomb, colorValue)
void __fastcall FUN_0040d310(Player *player)
{
    PlayerBombState *bomb = &player->bombState;
    if (bomb->timer < 60)
    {
        FUN_0040bc60(player, 0x80404040);
    }
    else
    {
        i32 colorValue = ((i32)bomb->timer - 60) * 176 / 60 + 64;
        FUN_0040bc60(player, 0x80000000 | (colorValue << 16) | (colorValue << 8) | colorValue);
    }
}

// FUNCTION: th08 0x40d950
void __fastcall FUN_0040d950(Player *player)
{
    FUN_0040bc60(player, 0x80404040);
}

// FUNCTION: th08 0x40dee0
#pragma var_order(bomb, workItem, rect, fadeValue, color, color2, rect2)
void __fastcall FUN_0040dee0(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    ZunRect rect;
    i32 fadeValue;
    ZunColor color;
    ZunColor color2;
    ZunRect rect2;

    bomb = &player->bombState;
    workItem = player->bombState.workItems;
    if (bomb->timer < 90)
    {
        FUN_0040bc60(player, 0x802020d0);
        return;
    }
    if (bomb->timer <= 120)
    {
        fadeValue = 208 * ((i32)bomb->timer - 90) / 30;
        color.r = fadeValue / 5 + 0xd0;
        color.g = fadeValue + 0x20;
        color.b = fadeValue + 0x20;
        color.a = 0x80;
        FUN_0040bc60(player, color.d3dColor);

        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        color.a = 255 * ((i32)bomb->timer - 90) / 30;
        ScreenEffect::DrawSquare(&rect, color.d3dColor);
        return;
    }
    if (bomb->timer <= 220)
    {
        rect2.left = 32.0f;
        rect2.top = 16.0f;
        rect2.right = 416.0f;
        rect2.bottom = 464.0f;
        color2.r = 0xff;
        color2.g = 0xff;
        color2.b = 0xff;
        color2.a = 0x70;
        ScreenEffect::DrawSquare(&rect2, color2.d3dColor);
        return;
    }
    FUN_0040bc60(player, 0x802020d0);
}

// FUNCTION: th08 0x40e3b0
#pragma var_order(bomb, workItem, angle, slot, position)
void __fastcall FUN_0040e3b0(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;
    PlayerUnkStruct0x40 *slot;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x97\xF6\x95\x84\x81\x75\x83\x7D\x83\x58\x83\x5E\x81\x5B\x83\x58\x83\x70\x81\x5B\x83\x4E\x81\x76",
                     300, 350, 0);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(19), 0);
        workItem->anchor = player->position;
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[0], 30);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[1], 31);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[2], 32);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[3], 33);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[4], 34);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.2f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.2f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK7, 16, 120, 60, 120, 21);
    }

    if (player->bombState.timer.FUN_0040d3d0() && ((i32)player->bombState.timer % 4) != 0)
    {
        Float3 position;
        position = player->position;
        position.x = 192.0f;
        position.y /= 2.0f;
        player->FUN_0044de60(&position, 384.0f, position.y * 2.0f, 6, 0);

        position = player->position;
        position.y /= 2.0f;
        slot = player->FUN_0044dfa0(&position, 128.0f, position.y * 2.0f, 12, 0);
        slot->mode = 1;
        position.x = 192.0f;
        slot = player->FUN_0044dfa0(&position, 384.0f, position.y * 2.0f, 6, 0);
        slot->mode = 1;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 5);
}

// FUNCTION: th08 0x40e610
#pragma var_order(vm, i, angleStep, angle)
void __fastcall FUN_0040e610(Player *player)
{
    AnmVm *vm;
    i32 i;
    f32 angleStep;
    f32 angle;

    FUN_0040bc60(player, 0x80404040);
    angleStep = ZUN_PI / 15.0f;
    vm = &player->bombState.workItems[0].vms[0];
    for (i = 0; i < 5; i++, vm++)
    {
        angle = (f32)i * angleStep - ZUN_PI / 2.0f - angleStep * 2.0f;
        if (angle < -ZUN_PI)
            angle += ZUN_2PI;
        vm->pos = player->position;
        vm->pos.x += cosf(angle) * vm->loadedSprite->widthPx * vm->scale.x / 2.0f;
        vm->pos.y += sinf(angle) * vm->loadedSprite->widthPx * vm->scale.x / 2.0f;
        vm->SetZRotation(angle);
        vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        vm->pos.z = 0.0f;
        g_AnmManager->Draw2D(vm);
    }
}

// FUNCTION: th08 0x40f550
void __fastcall FUN_0040f550(Player *player)
{
    FUN_0040bc60(player, 0x80d02020);
}

// FUNCTION: th08 0x40fcb0
void __fastcall FUN_0040fcb0(Player *player)
{
    FUN_0040bc60(player, 0x80f00000);
}

// FUNCTION: th08 0x410300
#pragma var_order(i, workItem)
void __fastcall FUN_00410300(Player *player)
{
    PlayerBombWorkItem *workItem;
    i32 i;

    FUN_0040bc60(player, 0x80404040);
    workItem = player->bombState.workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;
        workItem->vms[0].SetZRotation(workItem->rotation);
        workItem->vms[0].pos = workItem->anchor;
        workItem->vms[0].pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        workItem->vms[0].pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x410ac0
#pragma var_order(i, workItem)
void __fastcall FUN_00410ac0(Player *player)
{
    PlayerBombWorkItem *workItem;
    i32 i;

    FUN_0040bc60(player, 0x80202080);
    workItem = player->bombState.workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;
        workItem->vms[0].SetZRotation(workItem->rotation);
        workItem->vms[0].pos = workItem->anchor;
        workItem->vms[0].pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        workItem->vms[0].pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x410c40
#pragma var_order(bomb, workItem, angle)
void __fastcall FUN_00410c40(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        PlayerUnkStruct0x40 *slot;

        FUN_0040be30(player, 1,
                     "\x8B\xAB\x95\x84\x81\x75\x8E\x6C\x8F\x64\x8C\x8B\x8A\x45\x81\x76",
                     150, 200, 0);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->anchor = player->position;
        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(ZUN_PI / 4.0f, 1.0f, 4.0f);
        g_EffectManager.FUN_004259e0(36, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor),
                                     reinterpret_cast<D3DXVECTOR3 *>(&velocity), 4, 1, -1);
    }

    if (bomb->timer.FUN_0040e350(10))
    {
        PlayerUnkStruct0x40 *slot;
        AnmVm *effect;

        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(ZUN_PI * 3.0f / 8.0f, 1.0f, 4.0f);
        effect = g_EffectManager.FUN_004259e0(36, reinterpret_cast<D3DXVECTOR3 *>(&player->position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 5, 1, -1);
        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 89);
        bomb->workItems[1].anchor = player->position;
    }

    if (bomb->timer.FUN_0040e350(20))
    {
        PlayerUnkStruct0x40 *slot;
        AnmVm *effect;

        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(ZUN_PI / 2.0f, 1.0f, 4.0f);
        effect = g_EffectManager.FUN_004259e0(36, reinterpret_cast<D3DXVECTOR3 *>(&player->position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 6, 1, -1);
        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 90);
        bomb->workItems[2].anchor = player->position;
    }

    if (bomb->timer.FUN_0040e350(30))
    {
        PlayerUnkStruct0x40 *slot;
        AnmVm *effect;

        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(1.9634954929351807f, 1.0f, 4.0f);
        effect = g_EffectManager.FUN_004259e0(36, reinterpret_cast<D3DXVECTOR3 *>(&player->position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 7, 1, -1);
        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 91);
        bomb->workItems[3].anchor = player->position;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 2);
}

// FUNCTION: th08 0x4114e0
#pragma var_order(interp, i, slot, angle)
i32 __fastcall FUN_004114e0(AnmVm *effect)
{
    f32 interp;
    i32 i;
    PlayerUnkStruct0x40 *slot;
    f32 angle;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) < 40)
    {
        interp = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) / 40.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) =
            88.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) * 80.0f / 40.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) =
            192.0f - 384.0f * interp * interp;
        --*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324);
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    }
    else
    {
        if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) == 40)
        {
#pragma var_order(position, radius)
            f32 radius;
            ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x8ff08080, 0, 21);
            angle = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x318) + ZUN_PI / 4.0f;
            radius = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) * 0.7071067094802856f;
            Float3 position;
            g_AnmManager->FUN_00464b00(
                effect,
                reinterpret_cast<VertexTex1DiffuseXyzrhw *>(*reinterpret_cast<void **>(reinterpret_cast<u8 *>(effect) + 0x358)),
                2 * *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) + 2);
            for (i = 0; i < 4; i++)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;
                position.FromAngleMagnitude(angle, radius);
                position += *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2E0);
                slot = g_Player.FUN_0044dfa0(
                    &position, radius * 8.0f,
                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) * 4.0f, 60, 70);
                slot->collisionInterval = 4;
                slot->angle = AddNormalizeAngle(angle, ZUN_PI / 2.0f);
                angle = slot->angle;
                slot = g_Player.FUN_0044de60(
                    &position, radius * 4.0f,
                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) * 4.0f, 6, 100);
                slot->angle = angle;
            }
        }
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    }
    return 1;
}

// FUNCTION: th08 0x4123d0
#pragma var_order(unused0, unused1, bomb, workItem, interp)
void __fastcall FUN_004123d0(Player *player)
{
    i32 unused0;
    i32 unused1;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 interp;

    bomb = &player->bombState;
    unused1 = 0;
    unused0 = 0;
    workItem = &bomb->workItems[0];

    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x90\x6C\x8B\x53\x81\x75\x96\xA2\x97\x88\x89\x69\x8D\x85\x8E\x61\x81\x76",
                     250, 300, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.0f;
        player->anmFile->SetAndExecuteScriptIdx(&player->mainVm, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        workItem->anchor = player->position;
        workItem->position = workItem->anchor;
        workItem->position.y = 416.0f;
    }

    if (bomb->timer < 40)
    {
        interp = (f32)bomb->timer / 40.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->position - workItem->anchor) * interp + workItem->anchor;
        return;
    }
    else if (bomb->timer.FUN_0040e350(40))
    {
        g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xffff8080);
        return;
    }
    else if (bomb->timer.FUN_0040e350(70))
    {
        player->position.y = 32.0f;
        Float3 position = player->position;
        position.y = 224.0f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xefffffff, 0, 21);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(42), 0);
        workItem->cancelSlot = player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 500, 0);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 60);
        workItem->damageSlot->collisionInterval = 5;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        return;
    }
    else if (bomb->timer.FUN_0040e350(80))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 32.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 64.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xcfffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(90))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 64.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 128.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xafffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(100))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 96.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 192.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x8fffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(110))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 128.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 60, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 256.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 60, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x6fffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(120))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 160.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 50, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 320.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 50, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x5fffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(130))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 192.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 40, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 384.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 40, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x5fffffff, 0, 21);
        workItem->position = player->position;
        return;
    }
    else if (bomb->timer >= 150 && bomb->timer < 180)
    {
        interp = ((f32)bomb->timer - 150.0f) / 30.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->anchor - workItem->position) * interp + workItem->position;
        return;
    }
    else if (bomb->timer.FUN_0040e350(180))
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 1.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 1.0f;
    }
}

// FUNCTION: th08 0x411b10
#pragma var_order(unused0, unused1, bomb, workItem, interp)
void __fastcall FUN_00411b10(Player *player)
{
    i32 unused0;
    i32 unused1;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 interp;

    bomb = &player->bombState;
    unused1 = 0;
    unused0 = 0;
    workItem = &bomb->workItems[0];

    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x90\x6C\x95\x84\x81\x75\x8C\xBB\x90\xA2\x8E\x61\x81\x76",
                     220, 270, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.0f;
        player->anmFile->SetAndExecuteScriptIdx(&player->mainVm, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        workItem->anchor = player->position;
        workItem->position = workItem->anchor;
        workItem->position.y = 416.0f;
    }

    if (bomb->timer < 40)
    {
        interp = (f32)bomb->timer / 40.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->position - workItem->anchor) * interp + workItem->anchor;
        return;
    }
    else if (bomb->timer.FUN_0040e350(40))
    {
        g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xff8080ff);
        return;
    }
    else if (bomb->timer.FUN_0040e350(70))
    {
        player->position.y = 32.0f;
        Float3 position = player->position;
        position.y = 224.0f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xefffffff, 0, 21);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(42), 0);
        workItem->cancelSlot = player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 300, 10);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 80, 60);
        workItem->damageSlot->collisionInterval = 5;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        return;
    }
    else if (bomb->timer.FUN_0040e350(80))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 32.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        position.x += 64.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xcfffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(90))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 64.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        position.x += 128.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xbfffffff, 0, 21);
        return;
    }
    else if (bomb->timer.FUN_0040e350(100))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 96.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageSlot->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        position.x += 192.0f;
        player->FUN_0044de60(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageSlot = player->FUN_0044dfa0(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageSlot->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x8fffffff, 0, 21);
        workItem->position = player->position;
        return;
    }
    else if (bomb->timer >= 120 && bomb->timer < 150)
    {
        interp = ((f32)bomb->timer - 120.0f) / 30.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->anchor - workItem->position) * interp + workItem->position;
        return;
    }
    else if (bomb->timer.FUN_0040e350(150))
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 1.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 1.0f;
    }
}

// FUNCTION: th08 0x4117b0
#pragma var_order(interp, radialBase, i, slot, angle)
i32 __fastcall FUN_004117b0(AnmVm *effect)
{
    f32 interp;
    f32 radialBase;
    i32 i;
    PlayerUnkStruct0x40 *slot;
    f32 angle;

    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x318) =
        AddNormalizeAngle(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x318),
                          ((*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x328) & 1) != 0)
                              ? 0.039269909f
                              : -0.039269909f);
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) < 50)
    {
        interp = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) / 50.0f;
        radialBase = (f32)(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x328) - 4) * 32.0f + 384.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) =
            88.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) * 80.0f / 50.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) =
            (f32)(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x328) - 4) * 32.0f + 192.0f -
            radialBase * interp * interp;
        --*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324);
    }
    else
    {
        if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) == 50)
        {
#pragma var_order(position, radius)
            f32 radius;
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x8f6060f0, 0, 21);
            angle = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x318) + ZUN_PI / 4.0f;
            radius = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) * 0.7071067094802856f;
            Float3 position;
            g_AnmManager->FUN_00464b00(
                effect,
                reinterpret_cast<VertexTex1DiffuseXyzrhw *>(*reinterpret_cast<void **>(reinterpret_cast<u8 *>(effect) + 0x358)),
                2 * *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) + 2);
            for (i = 0; i < 4; i++)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;
                position.FromAngleMagnitude(angle, radius);
                position += *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2E0);
                slot = g_Player.FUN_0044dfa0(
                    &position, radius * 8.0f,
                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) * 4.0f, 60, 100);
                slot->collisionInterval = 2;
                slot->angle = AddNormalizeAngle(angle, ZUN_PI / 2.0f);
                angle = slot->angle;
                slot = g_Player.FUN_0044de60(
                    &position, radius * 4.0f,
                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) * 4.0f, 6, 150);
                slot->angle = angle;
            }
        }
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    }
    return 1;
}

// FUNCTION: th08 0x411720
#pragma var_order(velocity, position)
i32 __fastcall FUN_00411720(AnmVm *effect)
{
    Float3 position = *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2A4);
    Float3 velocity = *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2B0);

    g_EffectManager.FUN_004259e0(35, reinterpret_cast<D3DXVECTOR3 *>(&position),
                                 reinterpret_cast<D3DXVECTOR3 *>(&velocity),
                                 *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x328), 1, -1);
    *reinterpret_cast<void **>(reinterpret_cast<u8 *>(effect) + 0x348) = reinterpret_cast<void *>(FUN_004114e0);
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 44;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 4.0f;
    return 0;
}

// FUNCTION: th08 0x411a80
#pragma var_order(velocity, position)
i32 __fastcall FUN_00411a80(AnmVm *effect)
{
    Float3 position = *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2A4);
    Float3 velocity = *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2B0);

    g_EffectManager.FUN_004259e0(35, reinterpret_cast<D3DXVECTOR3 *>(&position),
                                 reinterpret_cast<D3DXVECTOR3 *>(&velocity),
                                 *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x328), 1, -1);
    *reinterpret_cast<void **>(reinterpret_cast<u8 *>(effect) + 0x348) = reinterpret_cast<void *>(FUN_004117b0);
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 54;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 6.0f;
    return 0;
}

// FUNCTION: th08 0x410fe0
#pragma var_order(bomb, workItem, angle)
void __fastcall FUN_00410fe0(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        PlayerUnkStruct0x40 *slot;

        FUN_0040be30(player, 1,
                     "\x8B\xAB\x8A\x45\x81\x75\x89\x69\x96\xE9\x8E\x6C\x8F\x64\x8C\x8B\x8A\x45\x81\x76",
                     250, 300, 1);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->anchor = player->position;
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[0], 21);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[1], 22);
        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 100, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(ZUN_PI / 4.0f, 1.0f, 4.0f);
        g_EffectManager.FUN_004259e0(37, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor),
                                     reinterpret_cast<D3DXVECTOR3 *>(&velocity), 4, 1, -1);
    }

    if (bomb->timer.FUN_0040e350(10))
    {
        PlayerUnkStruct0x40 *slot;
        AnmVm *effect;

        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(ZUN_PI * 3.0f / 8.0f, 1.0f, 4.0f);
        effect = g_EffectManager.FUN_004259e0(37, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[0].anchor),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 5, 1, -1);
        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 93);
        bomb->workItems[1].anchor = player->position;
    }

    if (bomb->timer.FUN_0040e350(20))
    {
        PlayerUnkStruct0x40 *slot;
        AnmVm *effect;

        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 100, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(ZUN_PI / 2.0f, 1.0f, 4.0f);
        effect = g_EffectManager.FUN_004259e0(37, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[0].anchor),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 6, 1, -1);
        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 94);
        bomb->workItems[2].anchor = player->position;
    }

    if (bomb->timer.FUN_0040e350(30))
    {
        PlayerUnkStruct0x40 *slot;
        AnmVm *effect;

        player->FUN_0044df00(&player->position, 100.0f, 1.0f, 100, 6);
        slot = player->FUN_0044e040(&player->position, 100.0f, 1.0f, 70, 40);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(slot) + 0x38) = 5;
        Float3 velocity(1.9634954929351807f, 1.0f, 4.0f);
        effect = g_EffectManager.FUN_004259e0(37, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[0].anchor),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 7, 1, -1);
        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 95);
        bomb->workItems[3].anchor = player->position;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 2);
}

// FUNCTION: th08 0x4113a0
#pragma var_order(vm, workItem)
void __fastcall FUN_004113a0(Player *player)
{
    PlayerBombWorkItem *workItem;
    AnmVm *vm;

    workItem = player->bombState.workItems;
    FUN_0040bc60(player, 0x802020d0);
    vm = &workItem->vms[0];
    vm->pos = workItem->anchor + vm->pos2;
    vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
    vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
    vm->pos.z = 0.01f;
    g_AnmManager->Draw2D(vm);

    vm++;
    vm->pos = workItem->anchor + vm->pos2;
    vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
    vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
    vm->pos.z = 0.0f;
    g_AnmManager->Draw2D(vm);
}

// FUNCTION: th08 0x413140
#pragma var_order(i, bomb, workItem)
void __fastcall FUN_00413140(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 1,
                     "\x8E\x80\x95\x84\x81\x75\x83\x4D\x83\x83\x83\x58\x83\x67\x83\x8A\x83\x68\x83\x8A\x81\x5B\x83\x80\x81\x76",
                     300, 350, 0);

        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
            workItem->active = 0;

        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = 0.013089969754219055f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = -0.013089969754219055f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = 0.015707964077591896f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 1.5f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = -0.015707964077591896f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 1.5f;
        }

        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.8f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.8f;
        bomb->workItems[0].effect =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 120, 12, 0, 0, 21);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;

        workItem->points[1] = workItem->anchor;
        workItem->position.x += workItem->position.y;
        workItem->rotation = AddNormalizeAngle(workItem->rotation, workItem->rotationStep);
        workItem->anchor.FromAngleMagnitude(workItem->rotation, workItem->position.x);
        workItem->anchor += workItem->points[0];
        workItem->points[1] = workItem->anchor - workItem->points[1];

        if (workItem->position.x >= 500.0f)
        {
            workItem->active = 0;
            workItem->damageSlot->active = 0;
            workItem->cancelSlot->active = 0;
            continue;
        }
        else
        {
            if (workItem->damageSlot != NULL)
            {
                workItem->damageSlot->center.x = workItem->anchor.x;
                workItem->damageSlot->center.y = workItem->anchor.y;
                workItem->cancelSlot->center.x = workItem->anchor.x;
                workItem->cancelSlot->center.y = workItem->anchor.y;
            }

            if (g_AnmManager->ExecuteScript(&workItem->vms[0]))
            {
                workItem->active = 0;
                workItem->damageSlot->active = 0;
                workItem->cancelSlot->active = 0;
            }
        }
    }
}

// FUNCTION: th08 0x413990
#pragma var_order(i, bomb, workItem, spawned, signedScaled)
void __fastcall FUN_00413990(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    i32 spawned;
    f32 signedScaled;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 1,
                     "\x8E\x80\x92\xB1\x81\x75\x89\xD8\xE3\xEF\x82\xCC\x89\x69\x96\xB0\x81\x76",
                     300, 350, 1);

        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
            workItem->active = 0;

        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = 0.013089969754219055f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = -0.013089969754219055f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = 0.015707964077591896f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 1.5f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->rotation = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->rotationStep = -0.015707964077591896f;
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 24.0f, 0.0f, 50, 500);
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 24.0f, 0.0f, 500, 6);
            workItem->damageSlot->hitCap = 800;
            workItem->position.x = 0.0f;
            workItem->position.y = 1.5f;
        }

        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.8f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.8f;
        bomb->workItems[0].effect =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 60, 16, 0, 0, 21);
    }

    if (bomb->timer >= 60 && bomb->timer < 200 && bomb->timer.FUN_0040ebc0(20))
    {
        signedScaled = g_Rng.GetRandomF32SignedInRange(ZUN_PI);
        spawned = 0;
        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
        {
            if (workItem->active != 0)
                continue;

            workItem->active = 1;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 20);
            workItem->rotation = AddNormalizeAngle((f32)i * ZUN_PI / 8.0f, signedScaled);
            workItem->points[0] = player->position;
            workItem->anchor = workItem->points[0];
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 64.0f, 0.0f, 100, 500);
            workItem->damageSlot->hitCap = 1200;
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 64.0f, 0.0f, 500, 6);
            workItem->position.x = 0.0f;
            workItem->position.y = 8.0f;
            if (++spawned >= 16)
                break;
        }
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->anchor.x);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 30, 8, 0, 0, 21);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0xe0f0f0f0, 0, 21);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;

        workItem->points[1] = workItem->anchor;
        workItem->position.x += workItem->position.y;
        workItem->rotation = AddNormalizeAngle(workItem->rotation, workItem->rotationStep);
        workItem->anchor.FromAngleMagnitude(workItem->rotation, workItem->position.x);
        workItem->anchor += workItem->points[0];
        workItem->points[1] = workItem->anchor - workItem->points[1];

        if (workItem->position.x >= 500.0f)
        {
            workItem->active = 0;
            workItem->damageSlot->active = 0;
            workItem->cancelSlot->active = 0;
            continue;
        }

        if (workItem->damageSlot != NULL)
        {
            workItem->damageSlot->center.x = workItem->anchor.x;
            workItem->damageSlot->center.y = workItem->anchor.y;
            workItem->cancelSlot->center.x = workItem->anchor.x;
            workItem->cancelSlot->center.y = workItem->anchor.y;
        }

        if (g_AnmManager->ExecuteScript(&workItem->vms[0]))
        {
            workItem->active = 0;
            workItem->damageSlot->active = 0;
            workItem->cancelSlot->active = 0;
        }
    }
}

// FUNCTION: th08 0x413890
#pragma var_order(i, workItem)
void __fastcall FUN_00413890(Player *player)
{
    i32 i;
    PlayerBombWorkItem *workItem;

    FUN_0040bc60(player, 0x80404040);
    workItem = player->bombState.workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;
        workItem->vms[0].SetZRotation(VectorAngle(workItem->points[1].y, workItem->points[1].x));
        workItem->vms[0].pos = workItem->anchor;
        workItem->vms[0].pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        workItem->vms[0].pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x4142c0
#pragma var_order(i, workItem)
void __fastcall FUN_004142c0(Player *player)
{
    i32 i;
    PlayerBombWorkItem *workItem;

    FUN_0040bc60(player, 0x80802020);
    workItem = player->bombState.workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (workItem->active == 0)
            continue;
        workItem->vms[0].SetZRotation(VectorAngle(workItem->points[1].y, workItem->points[1].x));
        workItem->vms[0].pos = workItem->anchor;
        workItem->vms[0].pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        workItem->vms[0].pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x412300
#pragma var_order(bomb, rect, color)
void __fastcall FUN_00412300(Player *player)
{
    PlayerBombState *bomb = &player->bombState;
    FUN_0040bc60(player, 0x80404040);
    if (bomb->timer >= 70)
    {
        FUN_0040bc60(player, 0x80000030);
        ZunColor color;
        ZunRect rect;
        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        if (bomb->timer < 100)
        {
            color.a = 0xff;
        }
        else if (bomb->timer < 160)
        {
            color.a = 0xff - 0xff * ((i32)bomb->timer - 100) / 60;
        }
        else
        {
            return;
        }
        ScreenEffect::DrawSquare(&rect, color.d3dColor);
    }
}

// FUNCTION: th08 0x412fa0
#pragma var_order(bomb, rect, color)
void __fastcall FUN_00412fa0(Player *player)
{
    PlayerBombState *bomb = &player->bombState;
    FUN_0040bc60(player, 0x80404040);
    if (bomb->timer >= 70)
    {
        FUN_0040bc60(player, 0x80000030);
        ZunRect rect;
        ZunColor color;
        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        color.r = 0xff;
        color.g = 0x00;
        color.b = 0x00;
        if (bomb->timer < 100)
        {
            color.a = 0xff;
        }
        else if (bomb->timer < 160)
        {
            color.a = 0xff - 0xff * ((i32)bomb->timer - 100) / 60;
        }
        else
        {
            return;
        }
        ScreenEffect::DrawSquare(&rect, color.d3dColor);
    }
}

DIFFABLE_STATIC(Player, g_Player);
DIFFABLE_STATIC(ChainElem *, g_PlayerCalcChain);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainLowPrio);
// The callback releases and clears these two independently allocated SHT files.
DIFFABLE_STATIC(PlayerRawShtFile *, g_PlayerPrimaryShtFile);
DIFFABLE_STATIC(PlayerRawShtFile *, g_PlayerSecondaryShtFile);
DIFFABLE_STATIC(i32, g_PlayerNormalBombCount);
DIFFABLE_STATIC(i32, g_PlayerDeathbombCount);
DIFFABLE_STATIC(u8, g_PlayerRouteStateFlag);
DIFFABLE_STATIC(u8, g_PlayerNoLivesFlag);


DIFFABLE_STATIC(f32, g_PlayerPlayfieldWidth);
DIFFABLE_STATIC_ARRAY(i16, 6, g_PlayerGaugeBounds);

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_PlayerAnmFilenames) = {
    "player00.anm", "player01.anm", "player02.anm", "player03.anm",
    "player00.anm", "player00.anm", "player01.anm", "player01.anm",
    "player02.anm", "player02.anm", "player03.anm", "player03.anm",
};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_Player1ShtFiles) = {
    "ply00a.sht", "ply01a.sht", "ply02a.sht", "ply03a.sht",
    "ply00a.sht", "ply00as.sht", "ply01a.sht", "ply01as.sht",
    "ply02a.sht", "ply02as.sht", "ply03a.sht", "ply03as.sht",
};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_Player2ShtFile) = {
    "ply00as.sht", "ply01as.sht", "ply02as.sht", "ply03as.sht",
    "ply00a.sht", "ply00as.sht", "ply01a.sht", "ply01as.sht",
    "ply02a.sht", "ply02as.sht", "ply03a.sht", "ply03as.sht",
};

struct PlayerBombCallbacks
{
    u32 callbacks[5];
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerBombCallbacks, 24, g_PlayerBombCallbackTable) = {
    {{0x0040C010,0x00410C40,0x0040C910,0x00410FE0,0x0040D100}},
    {{0x0040C820,0x0040D950,0x0040D010,0x004113A0,0x0040D310}},
    {{0x0040E3B0,0x0040D430,0x0040E780,0x0040D970,0x0040D100}},
    {{0x0040E610,0x0040D950,0x0040E610,0x0040DEE0,0x0040D310}},
    {{0x0040FCD0,0x0040EE10,0x004103F0,0x0040F570,0x0040D100}},
    {{0x00410300,0x0040F550,0x00410AC0,0x0040FCB0,0x0040D310}},
    {{0x00411B10,0x00413140,0x004123D0,0x00413990,0x0040D100}},
    {{0x00412300,0x00413890,0x00412FA0,0x004142C0,0x0040D310}},
    {{0x0040C010,0x0040C010,0x0040C910,0x0040C910,0x0040D100}},
    {{0x0040C820,0x0040C820,0x0040D010,0x0040D010,0x0040D310}},
    {{0x00410C40,0x00410C40,0x00410FE0,0x00410FE0,0x0040D100}},
    {{0x0040D950,0x0040D950,0x004113A0,0x004113A0,0x0040D310}},
    {{0x0040E3B0,0x0040E3B0,0x0040E780,0x0040E780,0x0040D100}},
    {{0x0040E610,0x0040E610,0x0040E610,0x0040E610,0x0040D310}},
    {{0x0040D430,0x0040D430,0x0040D970,0x0040D970,0x0040D100}},
    {{0x0040D950,0x0040D950,0x0040DEE0,0x0040DEE0,0x0040D310}},
    {{0x0040FCD0,0x0040FCD0,0x004103F0,0x004103F0,0x0040D100}},
    {{0x00410300,0x00410300,0x00410AC0,0x00410AC0,0x0040D310}},
    {{0x0040EE10,0x0040EE10,0x0040F570,0x0040F570,0x0040D100}},
    {{0x0040F550,0x0040F550,0x0040FCB0,0x0040FCB0,0x0040D310}},
    {{0x00411B10,0x00411B10,0x004123D0,0x004123D0,0x0040D100}},
    {{0x00412300,0x00412300,0x00412FA0,0x00412FA0,0x0040D310}},
    {{0x00413140,0x00413140,0x00413990,0x00413990,0x0040D100}},
    {{0x00413890,0x00413890,0x004142C0,0x004142C0,0x0040D310}},
};
struct PlayerOptionCallbackRow
{
    void *callbacks[4];
};
DIFFABLE_STATIC_ARRAY(PlayerOptionCallbackRow, 12, g_PlayerOptionUpdateCallbacks);
DIFFABLE_STATIC_ARRAY(PlayerOptionCallbackRow, 12, g_PlayerOptionRenderCallbacks);

extern u16 g_GuiMessageInputCurrent;
extern u16 g_GuiMessageInputPrevious;

// FUNCTION: th08 0x449ca0
Player::Player()
{
}

// FUNCTION: th08 0x449e50
PlayerOptionState::PlayerOptionState() {}

// FUNCTION: th08 0x449f70
PlayerBombWorkItem::PlayerBombWorkItem() {}

// FUNCTION: th08 0x449ea0
PlayerBombState::PlayerBombState() {}

// FUNCTION: th08 0x449ef0
PlayerShot::PlayerShot() {}

// FUNCTION: th08 0x451ce0
void __fastcall PlayerBuildAabb(Float3 *topLeft, Float3 *bottomRight, const Float3 *center, const Float3 *size)
{
    topLeft->x = center->x - size->x * 0.5f;
    topLeft->y = center->y - size->y * 0.5f;
    bottomRight->x = center->x + size->x * 0.5f;
    bottomRight->y = center->y + size->y * 0.5f;
}

// FUNCTION: th08 0x451670
#pragma var_order(bullet, i, enemyBottomRight, savedRotation, bulletBottomRight, enemyTopLeft, damage, region, bulletTopLeft)
i32 Player::FUN_00451670(Float3 *enemyPosition, Float3 *enemySize, i32 *hitAccumulator, i32 *bombHit)
{
    Float3 enemyTopLeft;
    Float3 enemyBottomRight;
    Float3 bulletTopLeft;
    Float3 bulletBottomRight;
    i32 damage;
    i32 i;
    i32 savedRotation;
    PlayerUnkStruct0x40 *region;
    PlayerShot *bullet;

    damage = 0;
    if (!reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4)->FUN_0040d3d0())
        return 0;

    PlayerBuildAabb(&enemyTopLeft, &enemyBottomRight, enemyPosition, enemySize);
    bullet = reinterpret_cast<PlayerShot *>(reinterpret_cast<u8 *>(this) + 0xBE838);
    if (bombHit != NULL)
        *bombHit = 0;

    for (i = 0; i < 128; i++, bullet++)
    {
        if (bullet->state == 0 || (bullet->state != 1 && bullet->type != 3))
            continue;

        PlayerBuildAabb(&bulletTopLeft, &bulletBottomRight, &bullet->position, &bullet->hitboxSize);
        if (bulletTopLeft.y > enemyBottomRight.y || bulletTopLeft.x > enemyBottomRight.x ||
            bulletBottomRight.y < enemyTopLeft.y || bulletBottomRight.x < enemyTopLeft.x)
            continue;

        if ((bullet->type == 4 || bullet->type == 5) && (bullet->timer % 2) != 0)
            continue;
        if (bullet->collisionCallback != NULL && bullet->collisionCallback(this, bullet, enemyPosition))
            continue;

        if (this->bombState.frameStop == 0)
            damage += bullet->damage;
        else
            damage += bullet->damage / 5 ? bullet->damage / 5 : 1;

        while (*hitAccumulator >= g_Player.damageAccumulatorThreshold)
        {
            if (g_GameManager.GaugeIsExtremelyHuman())
            {
                if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(bullet->shtEntry) + 0x1E) < 0)
                    g_ItemManager.SpawnItem(&bullet->position, static_cast<ItemType>(7), 3);
            }
            *hitAccumulator -= g_Player.damageAccumulatorThreshold;
        }

        if (bullet->type != 4 && bullet->type != 5 && bullet->type != 6)
        {
            if (bullet->state == 1)
            {
                savedRotation = *reinterpret_cast<i32 *>(&bullet->vm.rotation.z);
                this->anmFile->SetAndExecuteScriptIdx(&bullet->vm, bullet->animationIndex + 11);
                *reinterpret_cast<i32 *>(&bullet->vm.rotation.z) = savedRotation;
                g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(&bullet->position), 1, -1);
                bullet->position.operator float *()[2] = 0.1f;
            }
            bullet->state = 2;
            if (bullet->type != 3)
            {
                bullet->velocity.x /= 8.0f;
                bullet->velocity.y /= 8.0f;
            }
        }
    }

    *hitAccumulator += damage > 50 ? 50 : damage;

    {
        region = this->playerSlotsB;
        for (i = 0; i < 192; i++, region++)
        {
            if (!region->active)
                continue;
            if ((region->lifetime % region->collisionInterval) != 0)
                continue;

            if (region->radius == 0.0f)
            {
                if (region->angle == 0.0f)
                {
                    if (region->center.x - region->size.x / 2.0f > enemyBottomRight.x ||
                        region->center.x + region->size.x / 2.0f < enemyTopLeft.x ||
                        region->center.y - region->size.y / 2.0f > enemyBottomRight.y ||
                        region->center.y + region->size.y / 2.0f < enemyTopLeft.y)
                        continue;
                }
                else
                {
                    bulletTopLeft.x = enemyPosition->x - region->center.x;
                    bulletTopLeft.y = enemyPosition->y - region->center.y;
                    Rotate(&bulletBottomRight, &bulletTopLeft, -region->angle);
                    if (-region->size.x / 2.0f > enemySize->x / 2.0f + bulletBottomRight.x ||
                        region->size.x / 2.0f < bulletBottomRight.x - enemySize->x / 2.0f ||
                        -region->size.y / 2.0f > enemySize->y / 2.0f + bulletBottomRight.y ||
                        region->size.y / 2.0f < bulletBottomRight.y - enemySize->y / 2.0f)
                        continue;
                }
            }
            else if (region->radius * region->radius <
                     (region->center.x - enemyPosition->x) * (region->center.x - enemyPosition->x) +
                         (region->center.y - enemyPosition->y) * (region->center.y - enemyPosition->y))
            {
                continue;
            }

            damage += region->damage;
            region->hitAccumulator += region->damage;
            if (region->hitCap > 0 && region->hitCap <= region->hitAccumulator)
            {
                region->damage = 0;
                damage -= region->hitAccumulator - region->hitCap;
            }

            if (region->mode == 0 && (++*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0xE2A94) % 4) == 0)
            {
                if (i < 192)
                    g_EffectManager.SpawnEffect(3, reinterpret_cast<D3DXVECTOR3 *>(enemyPosition), 1, -1);
                else
                    g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(enemyPosition), 1, -1);
            }
            if (this->bombState.frameStop != 0 && bombHit != NULL)
                *bombHit = 1;
        }
    }

    if (g_GameManager.GaugeIsExtremelyYoukai() && damage != 0)
        damage = damage * 106 / 100;
    return damage;
}

// FUNCTION: th08 0x44e0e0
ZunBool IsResourceReloadDisabled()
{
    return !IsResourceReloadEnabled();
}

/* The target emits these cross-subsystem definitions in the Player translation unit. */
// FUNCTION: th08 0x44e0f0
void AnmVmBase::SetBlendModeAdditive()
{
    this->blendMode = AnmBlendMode_Additive;
}

// FUNCTION: th08 0x44e120
void AnmVmBase::SetBlendModeNormal()
{
    this->blendMode = AnmBlendMode_Normal;
}

// FUNCTION: th08 0x44e140
void GameManager::SetYoukaiGauge(u16 value)
{
    this->globals->youkaiGauge = value;
}

// FUNCTION: th08 0x44e350
void PlayerUnkStruct0x40::Deactivate()
{
    this->active = false;
}

// FUNCTION: th08 0x44e370
void PlayerUnkStruct0x40::Reset()
{
    memset(this, 0, sizeof(*this));
    this->collisionInterval = 1;
}

// FUNCTION: th08 0x44e770
#pragma var_order(delta, target, player, option)
void __fastcall PlayerOptionHomingToPlayer(Player *player, u8 *option)
{
    Float3 target;
    Float3 delta;

    target = player->position;
    target.y -= 96.0f;
    if (target.y < 32.0f)
        target.y = 32.0f;

    delta = target - *reinterpret_cast<Float3 *>(option + 0x2A4);
    delta /= 16.0f;
    *reinterpret_cast<Float3 *>(option + 0x2BC) +=
        (delta - *reinterpret_cast<Float3 *>(option + 0x2BC)) * 0.2f;
    *reinterpret_cast<Float3 *>(option + 0x2A4) += *reinterpret_cast<Float3 *>(option + 0x2BC);

    if (fabsf(*reinterpret_cast<f32 *>(option + 0x2BC)) < 0.05f)
        *reinterpret_cast<f32 *>(option + 0x2BC) = 0.0f;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AC4) >= 0 &&
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2ABC) != NULL &&
        *reinterpret_cast<ZunTimer *>(option + 0x2E0) >= 10)
    {
        reinterpret_cast<AnmVmBase *>(option)->SetInterrupt(3);
        *reinterpret_cast<i32 *>(option + 0x2CC) = 3;
    }
    else
    {
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2ABC) = NULL;
    }
}

// FUNCTION: th08 0x44e8d0
#pragma var_order(delta, target, player, option)
void __fastcall PlayerOptionHomingToTarget(Player *player, u8 *option)
{
    Float3 target;
    Float3 delta;

    target = *reinterpret_cast<Float3 *>(
        reinterpret_cast<u8 *>(*reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2ABC)) + 0x2D88);
    target.y += 32.0f;
    if (target.y < 32.0f)
        target.y = 32.0f;

    delta = target - *reinterpret_cast<Float3 *>(option + 0x2A4);
    delta /= 16.0f;
    *reinterpret_cast<Float3 *>(option + 0x2BC) +=
        (delta - *reinterpret_cast<Float3 *>(option + 0x2BC)) * 0.2f;
    *reinterpret_cast<Float3 *>(option + 0x2A4) += *reinterpret_cast<Float3 *>(option + 0x2BC);

    if (fabsf(*reinterpret_cast<f32 *>(option + 0x2BC)) < 0.05f)
        *reinterpret_cast<f32 *>(option + 0x2BC) = 0.0f;
}

// FUNCTION: th08 0x44e9e0
i32 __fastcall PlayerRoute2OptionRender(Player *, u8 *option)
{
    reinterpret_cast<AnmVm *>(option)->pos.x =
        g_ItemAnmManagerScreenShakeOffset.x + *reinterpret_cast<f32 *>(option + 0x2A4);
    reinterpret_cast<AnmVm *>(option)->pos.y =
        g_ItemAnmManagerScreenShakeOffset.y + *reinterpret_cast<f32 *>(option + 0x2A8);
    reinterpret_cast<AnmVm *>(option)->pos.z = 0.49f;
    g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(option));
    return 0;
}

// FUNCTION: th08 0x40d3d0
ZunBool ZunTimer::FUN_0040d3d0()
{
    return this->current != this->previous;
}

// FUNCTION: th08 0x40d410
i32 ZunTimer::operator%(i32 value)
{
    return this->current % value;
}

// FUNCTION: th08 0x40bc20
i32 Player::IsHuman()
{
    return !this->isYoukai;
}

// FUNCTION: th08 0x40bc40
i32 Player::IsYoukai()
{
    return this->isYoukai;
}

// FUNCTION: th08 0x44c1b0
#pragma var_order(yDelta, xDelta, this)
f32 Player::FUN_0044c1b0(Float3 *position)
{
    f32 yDelta;
    f32 xDelta;

    xDelta = reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4)->operator float *()[0] - position->x;
    yDelta = reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4)->operator float *()[1] - position->y;

    if (yDelta == 0.0f && xDelta == 0.0f)
    {
        return ZUN_PI / 2.0f;
    }

    return VectorAngle(yDelta, xDelta);
}

// FUNCTION: th08 0x44a230
#pragma var_order(boundsMax, boundsMin)
i32 Player::FUN_0044a230(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A90) = 6;
    if (this->FUN_00449ff0(position, size))
        return 2;

    boundsMin.x = position->x - size->x / 2.0f;
    boundsMin.y = position->y - size->y / 2.0f;
    boundsMax.x = size->x / 2.0f + position->x;
    boundsMax.y = size->y / 2.0f + position->y;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x38C) > boundsMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x390) > boundsMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x398) < boundsMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x39C) < boundsMin.y)
        return 0;

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 1;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a360
#pragma var_order(boundsMax, boundsMin)
i32 Player::FUN_0044a360(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A90) = 6;
    boundsMin.x = position->x - size->x / 2.0f;
    boundsMin.y = position->y - size->y / 2.0f;
    boundsMax.x = size->x / 2.0f + position->x;
    boundsMax.y = size->y / 2.0f + position->y;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x38C) > boundsMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x390) > boundsMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x398) < boundsMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x39C) < boundsMin.y)
        return 0;

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 1;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a470
#pragma var_order(boundsMax, boundsMin)
i32 Player::FUN_0044a470(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A90) = 6;
    if (this->FUN_00449ff0(position, size))
        return 2;

    boundsMin.x = position->x - size->x / 2.0f - 20.0f;
    boundsMin.y = position->y - size->y / 2.0f - 20.0f;
    boundsMax.x = size->x / 2.0f + position->x + 20.0f;
    boundsMax.y = size->y / 2.0f + position->y + 20.0f;

    if (this->playerState == PLAYER_STATE_DYING || this->playerState == PLAYER_STATE_SPAWNING)
        return 0;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3A4) > boundsMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3B0) < boundsMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3A8) > boundsMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3B4) < boundsMin.y)
        return 0;

    this->FUN_0044a930(position, 0);
    return 1;
}

// FUNCTION: th08 0x44a6a0
#pragma var_order(playerMin, incomingMax, incomingMin, playerMax)
u32 Player::CalcLaserHitbox(Float3 *position, Float3 *size, Float3 *origin, f32 angle, i32 graze)
{
    Float3 incomingMin;
    Float3 incomingMax;
    Float3 playerMin;
    Float3 playerMax;

    incomingMin = this->position - *origin;
    Rotate(&incomingMax, &incomingMin, -angle);
    incomingMax.z = 0.0f;
    incomingMin = incomingMax + *origin;

    playerMin = incomingMin - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    playerMax = incomingMin + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    incomingMin = *position - *size / 2.0f;
    incomingMax = *position + *size / 2.0f;

    if (!(playerMin.x > incomingMax.x))
    {
        if (!(playerMax.x < incomingMin.x))
        {
            if (!(playerMin.y > incomingMax.y))
            {
                if (!(playerMax.y < incomingMin.y))
                    goto lethalPath;
            }
        }
    }

grazePath:
    {
        if (!graze)
            return 0;

        incomingMin.x -= 48.0f;
        incomingMin.y -= 48.0f;
        incomingMax.x += 48.0f;
        incomingMax.y += 48.0f;

        if (playerMin.x > incomingMax.x || playerMax.x < incomingMin.x ||
            playerMin.y > incomingMax.y || playerMax.y < incomingMin.y)
            return 0;

        if (this->playerState == PLAYER_STATE_DYING || this->playerState == PLAYER_STATE_SPAWNING)
            return 0;
        this->FUN_0044a930(&this->position, 1);
        return 2;
    }

lethalPath:
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 0;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a930
#pragma var_order(midpoint, gaugeGain, score)
void Player::FUN_0044a930(Float3 *position, i32 suppressExtraItems)
{
    Float3 midpoint;
    i32 gaugeGain;
    i32 score;

    if (g_Player.bombState.frameStop == 0)
    {
        gaugeGain = g_GameManager.GaugeIsExtremelyHuman()
                        ? 3
                        : (g_GameManager.GaugeIsModeratelyHuman() ? 2 : 1);

        if (g_GameManager.globals->grazeInStage < 99999)
            g_GameManager.globals->grazeInStage += gaugeGain;
        if (g_GameManager.globals->graze < 999999)
            g_GameManager.globals->graze += gaugeGain;
    }

    midpoint = (this->position + *position) / 2.0f;
    g_EffectManager.SpawnEffect(8, reinterpret_cast<D3DXVECTOR3 *>(&midpoint), 1, -1);
    g_GameManager.IncreaseSubrank(6);
    g_Gui.flags.grazeDisplayUpdateFrames = 2;
    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(30), position->x);

    score = g_GameManager.GaugeIsModeratelyYoukai() ? 4000 : 2000;
    g_GameManager.AddScore(score);
    if (this->IsYoukai())
        g_GameManager.AddToYoukaiGauge(100, 0);

    if (!g_GameManager.IsSoloHuman() || *reinterpret_cast<u8 *>(0x164D0B1) == 10)
    {
        if (g_EnemyManager.FUN_0042f1f0() && g_GameManager.GaugeIsExtremelyYoukai())
        {
            g_ItemManager.SpawnItem(position, ITEM_TIME2, 1);
            if (!suppressExtraItems && g_Spellcard.IsActive())
            {
                g_ItemManager.SpawnItem(position, ITEM_TIME2, 1);
                if (!g_GameManager.IsSoloYoukai())
                    g_ItemManager.SpawnItem(position, ITEM_TIME2, 1);
            }
        }
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x2000;
}

// FUNCTION: th08 0x44ab40
#pragma var_order(effectVm)
void Player::Die()
{
    AnmVm *effectVm;

    utils::DebugPrint("player DEAD");
    g_EclScriptedGlobalUpdateFreeze = 0;
    g_GameManager.UpdateAntiTamper();
    g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 16, -1);
    this->playerState = PLAYER_STATE_DYING;
    this->timer = 0;
    g_SoundPlayer.PlaySoundPositionedByIdx(SOUND_PICHUN, this->position.x);
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x200;

    if (((*reinterpret_cast<u32 *>(0x164D0B4) >> 7) & 3) != 0)
    {
        utils::DebugPrint(" desolve\n");
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) = 2;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x6) = 1;
    }
    else
    {
        g_GameManager.SetYoukaiGauge(0);
        if (g_GameManager.GetBombsRemaining() >= 1)
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) =
                g_GameManager.GetBombsRemaining() * 6;
            if (g_GameManager.GetTimeOrbs() >= g_GameManager.GetLastSpellTimeOrbThreshold())
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) += 7;
            if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) > 15)
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) = 15;

            if (g_Spellcard.IsActive())
            {
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) +=
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68);
                if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) > 30)
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) = 30;
            }

            if (*reinterpret_cast<u8 *>(0x164D0B1) == 0 || *reinterpret_cast<u8 *>(0x164D0B1) == 4 ||
                *reinterpret_cast<u8 *>(0x164D0B1) == 5)
            {
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) *= 9;
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) /= 5;
            }

            utils::DebugPrint(" preDeadCount %d\n",
                              *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68));
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x206) = 0xFF;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x205) = 0xFF;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x204) = 0xFF;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x207) =
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x203);
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x208) |= 0x20000;

            *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0xE2B28) =
                g_EffectManager.FUN_00425870(59, reinterpret_cast<D3DXVECTOR3 *>(&this->position),
                                              11, 1, 0xFFF0404F);
            effectVm = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0xE2B28);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effectVm) + 0x50) = 0;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effectVm) + 0xA4) =
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effectVm) + 0xF8) = 4;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x238) = 128.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x244) = 8.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x23C) = 32.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x248) = 0.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x208) = 128.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x20C) = 32.0f;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectVm) + 0x324) = 64;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectVm) + 0x318) = 0;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x314) = 128.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x320) = 15.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x334) = 6.0f;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effectVm) + 0x357) = 1;

            if (g_Spellcard.IsActive())
                *reinterpret_cast<u32 *>(0x164D0B4) |= 0x400;
        }
        else
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) = 2;
            utils::DebugPrint(" Miss\n");
        }
    }

    g_ItemManager.CancelAutoCollect();
}

// FUNCTION: th08 0x44a5a0
#pragma var_order(itemMax, itemMin)
u32 Player::CalcItemBoxCollision(Float3 *position, Float3 *size)
{
    Float3 itemMin;
    Float3 itemMax;

    if (this->playerState != 0 && this->playerState != 3 && this->playerState != 4)
        return 0;

    itemMin = *position - *size / 2.0f;
    itemMax = *position + *size / 2.0f;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3bc) > itemMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3c8) < itemMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3c0) > itemMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3cc) < itemMin.y)
        return 0;
    return 1;
}

// FUNCTION: th08 0x449ff0
#pragma var_order(halfSize, yDelta, xDelta, i, rotated, delta, slot, boundsMax)
i32 Player::FUN_00449ff0(Float3 *position, Float3 *position2)
{
    Float3 delta;
    Float3 rotated;
    Float3 halfSize;
    Float3 boundsMax;
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 i;
    f32 xDelta;
    f32 yDelta;

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->playerSlotsC); i++, slot++)
    {
        if (!slot->active)
            continue;

        if (slot->radius != 0.0)
        {
            xDelta = position->x - slot->center.x;
            yDelta = position->y - slot->center.y;
            if (xDelta * xDelta + yDelta * yDelta < slot->radius * slot->radius)
                goto hit;
            goto next;
        }

        if (slot->angle != 0.0f)
        {
            delta.x = position->x - slot->center.x;
            delta.y = position->y - slot->center.y;
            Rotate(&rotated, &delta, -slot->angle);
            halfSize.x = slot->size.x / 2.0f;
            halfSize.y = slot->size.y / 2.0f;
            if (-halfSize.x <= rotated.x && rotated.x <= halfSize.x && -halfSize.y <= rotated.y &&
                rotated.y <= halfSize.y)
                goto hit;
            goto next;
        }

        halfSize.x = slot->center.x - slot->size.x / 2.0f;
        halfSize.y = slot->center.y - slot->size.y / 2.0f;
        boundsMax.x = slot->size.x / 2.0f + slot->center.x;
        boundsMax.y = slot->size.y / 2.0f + slot->center.y;
        if (!(halfSize.x > position->x))
        {
            if (!(boundsMax.x < position->x))
            {
                if (!(halfSize.y > position->y))
                {
                    if (!(boundsMax.y < position->y))
                        goto hit;
                }
            }
        }
        goto next;

    next:
        ;
    }

    return 0;

hit:
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xe2a90) = slot->collisionValue;
    slot->hitAccumulator++;
    return 2;
}

#pragma var_order(primaryShtFile, player, secondaryShtFile)
// FUNCTION: th08 0x44c230
ZunResult Player::RegisterChain(u32 playerType)
{
    Player *player = &g_Player;
    PlayerRawShtFile *secondaryShtFile;
    PlayerRawShtFile *primaryShtFile;

    if (IsResourceReloadDisabled())
    {
        primaryShtFile = player->primaryShtFile;
        secondaryShtFile = player->secondaryShtFile;
    }

    memset(player, 0, sizeof(*player));

    if (IsResourceReloadDisabled())
    {
        player->primaryShtFile = primaryShtFile;
        player->secondaryShtFile = secondaryShtFile;
    }

    player->timer = 0;
    player->playerType = playerType;

    player->calcChain = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->calcChain->arg = player;
    player->calcChain->addedCallback = (ChainLifetimeCallback)Player::AddedCallback;
    player->calcChain->deletedCallback = (ChainLifetimeCallback)Player::DeletedCallback;
    if (g_Chain.AddToCalcChain(player->calcChain, 9))
        return ZUN_ERROR;

    player->drawChainHighPrio = g_Chain.CreateElem((ChainCallback)Player::OnDrawHighPrio);
    player->drawChainLowPrio = g_Chain.CreateElem((ChainCallback)Player::OnDrawLowPrio);
    player->drawChainHighPrio->arg = player;
    player->drawChainLowPrio->arg = player;
    g_Chain.AddToDrawChain(player->drawChainHighPrio, 9);
    g_Chain.AddToDrawChain(player->drawChainLowPrio, 10);

    return ZUN_SUCCESS;
}


// FUNCTION: th08 0x44c650
#pragma var_order(isForced, i)
void Player::FUN_0044c650()
{
    u32 i;
    i32 isForced;
    isForced = 0;
    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x6) != 0 &&
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) == 1)
    {
        isForced = 1;
        goto acceptBomb;
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A6C) != 0)
        --*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A6C);

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) != 0)
    {
        if (reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4)->FUN_0040d3d0())
            g_Gui.flags.pointDisplayUpdateFrames = 2;

        if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4) >= *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE4))
        {
            g_Spellcard.spellcard_fun_00416130();
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) = 0;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x408) = 1.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x404) = 1.0f;

            if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) == 4)
            {
                *reinterpret_cast<u32 *>(&g_GameManager.flags) &= 0xFFFFFE7Fu;
                for (i = 0; i < 8; i++)
                {
                    if (reinterpret_cast<EclOperands::EnemyOverlay **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0)[i] != NULL)
                    {
                        reinterpret_cast<EclOperands::EnemyOverlay **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0)[i]->FUN_0042adb0(0);
                        *reinterpret_cast<i32 *>(reinterpret_cast<EclOperands::EnemyOverlay **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0)[i]->bytes + 0x2DFC) = 0;
                        *reinterpret_cast<u32 *>(reinterpret_cast<EclOperands::EnemyOverlay **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0)[i]->bytes + 0x3324) &= 0xBFFFFFFFu;
                    }
                }
                ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 30, 1, -1, 0, 21);
            }
        }
        else
        {
            reinterpret_cast<void (__fastcall **)(Player *)>(reinterpret_cast<u8 *>(this) + 0x1000)
                [*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0)](this);
            (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4))++;
        }

        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) < 4)
        {
            if ((*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) & 1) != 0)
                g_GameManager.AddToYoukaiGauge(26000 / *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE4), 1);
            else
                g_GameManager.AddToYoukaiGauge(-26000 / *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE4), 1);
        }
        return;
    }

    if ((g_GuiMessageInputCurrent & 2) != 0 && !g_GameManager.IsTampered() && !g_Gui.IsDialogPresent() &&
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) != 0 &&
        g_GameManager.GetBombsRemaining() > 0 &&
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A6C) == 0)
    {
        if ((((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0) ||
            (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 14) & 1) != 0))
        {
            if ((g_GuiMessageInputCurrent & 2) != 0)
            {
                if ((g_GuiMessageInputCurrent & 2) != (g_GuiMessageInputPrevious & 2))
                    g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(41), 0);
            }
            goto done;
        }

acceptBomb:
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 1;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x6) = 0;
    if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0)
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) = 4;
    }
    else
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x208) &= 0xFFFDFFFFu;
        if (*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B28) != NULL)
        {
            *reinterpret_cast<u8 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B28) + 0x350) = 0;
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B28) = NULL;
        }
        *reinterpret_cast<u32 *>(&g_GameManager.flags) &= 0xFFFFFBFFu;
        g_AnmManager->SetMixColorDefault();

        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) =
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3);
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4))
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) =
                1 - *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0);

        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4))
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) += 2;
            if (isForced)
            {
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFEC) = g_GameManager.GetBombsRemaining();
                g_GameManager.SetBombCount(0);
            }
            else
            {
                if (g_GameManager.GetBombsRemaining() < 2)
                {
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFEC) = g_GameManager.GetBombsRemaining();
                    g_GameManager.SetBombCount(0);
                }
                else
                {
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFEC) = 2;
                    g_GameManager.AddToBombCount(-2);
                }
            }
            ++g_PlayerDeathbombCount;
        }
        else
        {
            ++g_PlayerNormalBombCount;
            g_GameManager.AddToBombCount(-1);
        }
        g_GameManager.AddToBombsUsed(1);
    }

    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4) = 0;
    g_Gui.flags.bombDisplayUpdateFrames = 2;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) = 1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A7C) = 1;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE4) = 999;

    {
        reinterpret_cast<void (__fastcall **)(Player *)>(reinterpret_cast<u8 *>(this) + 0x1000)
            [*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0)](this);
    }
    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4))++;
    g_GameManager.DecreaseSubrank(200);
    g_Spellcard.FUN_0044cba0();

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) += 6;
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) >
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x8))
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x8);
    }
        goto done;
    }
    else
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A7C) = 0;
    }

done:
    return;
}
// FUNCTION: th08 0x44cbf0
#pragma var_order(value, this)
i32 Player::FUN_0044cbf0()
{
    f32 value;

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) != 0)
    {
        g_GameManager.AddTimeOrbs(-15);
        --*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68);
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 4) = 1;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) == 0)
        {
            if (*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B28) != NULL)
            {
                *reinterpret_cast<u8 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B28) + 0x350) = 0;
                *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B28) = NULL;
            }
            g_EffectManager.FUN_00425870(12, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 3, 1, 0xFF4040FF);
            g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 16, -1);
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), this->position.x);
            *reinterpret_cast<u32 *>(&g_GameManager.flags) &= ~0x400u;
            g_AnmManager->SetMixColorDefault();
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x208) &= ~0x20000u;
            *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 4;
            g_PlayerRouteStateFlag = 0;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 4) = 0;
            g_Spellcard.FUN_0044d150();
            g_GameManager.AddToDeaths(1);
            g_Gui.flags.timeDisplayUpdateFrames = 2;
            g_GameManager.AddTimeOrbs(g_GameManager.globals->currentTimeOrbs > 5000
                                           ? -500
                                           : -g_GameManager.globals->currentTimeOrbs / 10);

            if (g_GameManager.GetLives() > 0)
            {
                if (g_GameManager.GetPower() <= 16)
                    g_GameManager.SetPower(0);
                else
                    g_GameManager.AddPower(-16);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_BIG, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                if (g_GameManager.GetBombsRemaining() > 0 &&
                    (g_TargetByte0164D0B1 == 2 || g_TargetByte0164D0B1 == 8 || g_TargetByte0164D0B1 == 9))
                    g_ItemManager.SpawnItem(&this->position, ITEM_BOMB, ITEM_STATE_UNK2);
                g_Gui.flags.powerDisplayUpdateFrames = 2;
                g_ItemManager.CancelAutoCollect();
            }
            else
            {
                g_GameManager.SetPower(0);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_Gui.flags.powerDisplayUpdateFrames = 2;
            }
            g_GameManager.DecreaseSubrank(1600);
        }
    }
    else
    {
        value = (f32)this->timer / 30.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 3.0f * value + 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 1.0f - 1.0f * value;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x200) =
        ((i32)(255.0f - (f32)this->timer * 255.0f / 30.0f) << 24) | 0xFFFFFF;
    reinterpret_cast<AnmVmBase *>(reinterpret_cast<u8 *>(this) + 0x10)->SetBlendModeAdditive();
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2AA0) = 0;

    if ((i32)this->timer >= 30)
    {
        this->playerState = PLAYER_STATE_SPAWNING;
        this->position.operator float *()[0] = g_PlayerPlayfieldWidth / 2.0f;
        this->position.operator float *()[1] = g_ItemPlayfieldBottom - 64.0f;
        this->position.operator float *()[2] = 0.2f;
        this->timer = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 3.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 3.0f;
        if ((g_TargetByte0164D0B1 < 4 && *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0) ||
            (g_TargetByte0164D0B1 & 1) == 0)
            (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 0);
        else
            (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 5);

        if (g_GameManager.GetLives() <= 0)
        {
            g_PlayerNoLivesFlag = 1;
        }
        else
        {
            g_GameManager.AddLives(-1);
            g_Gui.flags.lifeDisplayUpdateFrames = 2;
            g_GameManager.SetBombCount((i32)*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 4));
            g_Gui.flags.bombDisplayUpdateFrames = 2;
            return 1;
        }
    }
    }
    return 0;
}
// FUNCTION: th08 0x44d180
#pragma var_order(value, this)
void Player::FUN_0044d180()
{
    f32 value;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A70) = 60;
    value = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) / 60.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 2.0f * value + 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 1.0f - 1.0f * value;
    reinterpret_cast<AnmVmBase *>(reinterpret_cast<u8 *>(this) + 0x10)->SetBlendModeAdditive();
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x408) = 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x404) = 1.0f;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x200) =
        (((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) * 0xFF) / 30 << 24) | 0xFFFFFF;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) = 0;

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) >= 30)
    {
        *reinterpret_cast<i8 *>(this) = 3;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 1.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 1.0f;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x200) = 0xFFFFFFFF;
        reinterpret_cast<AnmVmBase *>(reinterpret_cast<u8 *>(this) + 0x10)->SetBlendModeNormal();
        if (((*reinterpret_cast<u32 *>(0x164D0B4) >> 14) & 1) == 0)
        {
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) = 240;
        }
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xE2A68) =
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x8);
    }
}
// FUNCTION: th08 0x44aec0
#pragma var_order(oldDirection, verticalSpeed, horizontalSpeed, focus, option, optionIndex, option2, optionExitIndex, route3Index, historyInitIndex, optionUpdateIndex, gaugeDelta, historyIndex, this)
i32 Player::FUN_0044aec0()
{
    typedef void (__fastcall *OptionUpdateCallback)(Player *, u8 *);

    i32 oldDirection;
    f32 verticalSpeed;
    f32 horizontalSpeed;
    i32 focus;
    u8 *option;
    u32 optionIndex;
    u8 *option2;
    u32 optionExitIndex;
    i32 route3Index;
    u32 historyInitIndex;
    u32 optionUpdateIndex;
    i32 gaugeDelta;
    i32 historyIndex;

    horizontalSpeed = 0.0f;
    verticalSpeed = 0.0f;
    oldDirection = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98);

    if ((g_GuiMessageInputCurrent & 0x50) == 0x50)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 5;
    else if ((g_GuiMessageInputCurrent & 0x60) == 0x60)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 7;
    else if ((g_GuiMessageInputCurrent & 0x90) == 0x90)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 6;
    else if ((g_GuiMessageInputCurrent & 0xA0) == 0xA0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 8;
    else if ((g_GuiMessageInputCurrent & 0x20) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 2;
    else if ((g_GuiMessageInputCurrent & 0x10) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 1;
    else if ((g_GuiMessageInputCurrent & 0x40) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 3;
    else if ((g_GuiMessageInputCurrent & 0x80) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 4;
    else
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 0;

    focus = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFDC)
                ? (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) & 1)
                : (g_GuiMessageInputCurrent & 4);

    if (focus)
    {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) != 1)
        {
            if (g_TargetByte0164D0B1 <= 3)
            {
                option = reinterpret_cast<u8 *>(this) + 0x40C;
                for (optionIndex = 0; optionIndex < 4; optionIndex++, option += 0x2F4)
                {
                    memset(option, 0, 0x2F4);
                    *reinterpret_cast<void **>(option + 0x2EC) =
                        reinterpret_cast<void *(*)[4]>(0x004C7D40)[g_TargetByte0164D0B1][optionIndex];
                    *reinterpret_cast<void **>(option + 0x2F0) =
                        reinterpret_cast<void *(*)[4]>(0x004C7E10)[g_TargetByte0164D0B1][optionIndex];
                    if (*reinterpret_cast<void **>(option + 0x2EC) != NULL)
                    {
                        *reinterpret_cast<i32 *>(option + 0x2C8) = 1;
                        *reinterpret_cast<ZunTimer *>(option + 0x2E0) = 0;
                        *reinterpret_cast<i32 *>(option + 0x2D0) = optionIndex;
                    }
                    else
                    {
                        *reinterpret_cast<i32 *>(option + 0x2C8) = 0;
                    }
                }
            }

            if (g_TargetByte0164D0B1 < 4)
            {
                (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                    ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 5);
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = 0.0f;
                if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 4)
                    g_EffectManager.SpawnEffect(29, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 1, 0x80FF8080);
            }
            if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xBE834) == NULL)
            {
                *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xBE834) =
                    g_EffectManager.FUN_00425870(22, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 2, 1, -1);
            }
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) = 0;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) = 0;
        }
        else
        {
            ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8);
        }
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 7)
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 1;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) = 1;
    }
    else
    {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) != 0)
        {
            option2 = reinterpret_cast<u8 *>(this) + 0x40C;
            if (g_TargetByte0164D0B1 < 3)
            {
                for (optionExitIndex = 0; optionExitIndex < 4; optionExitIndex++, option2 += 0x2F4)
                {
                    if (*reinterpret_cast<i32 *>(option2 + 0x2C8) != 0 &&
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) != 3)
                    {
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) = 3;
                        *reinterpret_cast<ZunTimer *>(option2 + 0x2E0) = 0;
                    }
                }
            }
            else if (g_TargetByte0164D0B1 == 3)
            {
                for (route3Index = 0; route3Index < 2; route3Index++, option2 += 0x2F4)
                {
                    if (*reinterpret_cast<i32 *>(option2 + 0x2C8) != 0 &&
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) != 3)
                    {
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) = 3;
                        *reinterpret_cast<ZunTimer *>(option2 + 0x2E0) = 0;
                    }
                }
                memset(option2, 0, 0x2F4);
                *reinterpret_cast<void **>(option2 + 0x2EC) = reinterpret_cast<void **>(0x004C7E00)[route3Index];
                *reinterpret_cast<void **>(option2 + 0x2F0) = reinterpret_cast<void **>(0x004C7ED0)[route3Index];
                *reinterpret_cast<i32 *>(option2 + 0x2C8) = 1;
                *reinterpret_cast<ZunTimer *>(option2 + 0x2E0) = 0;
                *reinterpret_cast<i32 *>(option2 + 0x2D0) = route3Index;
                for (historyInitIndex = 0; historyInitIndex < 16; ++historyInitIndex)
                    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC + historyInitIndex * 12) = this->position;
            }

            if (g_TargetByte0164D0B1 < 4)
            {
                (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                    ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 0);
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = 0.0f;
                if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 4)
                    g_EffectManager.SpawnEffect(28, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 1, 0x808080FF);
            }
            if (*reinterpret_cast<AnmVmBase **>(reinterpret_cast<u8 *>(this) + 0xBE834) != NULL)
                (*reinterpret_cast<AnmVmBase **>(reinterpret_cast<u8 *>(this) + 0xBE834))->SetInterrupt(1);
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xBE834) = NULL;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) = 0;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) = 0;
        }
        else
        {
            ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8);
        }
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 7)
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 0;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) = 0;
    }

    if (g_TargetByte0164D0B1 >= 4)
    {
        if ((g_TargetByte0164D0B1 & 1) != 0)
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 1;
        else
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 0;
    }

    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) != 0)
    {
        switch (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98))
        {
        case 4: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 3: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 1: verticalSpeed =   -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 2: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 5: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); verticalSpeed = horizontalSpeed; break;
        case 7: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); horizontalSpeed = -verticalSpeed; break;
        case 6: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); verticalSpeed = -horizontalSpeed; break;
        case 8: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); verticalSpeed = horizontalSpeed; break;
        default: break;
        }
    }
    else
    {
        switch (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98))
        {
        case 4: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 3: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 1: verticalSpeed =   -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 2: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 5: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); verticalSpeed = horizontalSpeed; break;
        case 7: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); horizontalSpeed = -verticalSpeed; break;
        case 6: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); verticalSpeed = -horizontalSpeed; break;
        case 8: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); verticalSpeed = horizontalSpeed; break;
        default: break;
        }
    }

    horizontalSpeed *= *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x404);
    verticalSpeed *= *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x408);

#define SET_PLAYER_SCRIPT(idx) ((*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), (idx)))
    if (g_TargetByte0164D0B1 < 4)
    {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(1);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(2);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(3);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(4);
        }
        else
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(6);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(7);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(8);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(9);
        }
    }
    else
    {
        if ((g_TargetByte0164D0B1 & 1) != 0)
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(6);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(7);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(8);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(9);
        }
        else
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(1);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(2);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(3);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(4);
        }
    }
#undef SET_PLAYER_SCRIPT

    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = horizontalSpeed;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AA0) = verticalSpeed;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3F8) = horizontalSpeed * g_EclGameTimeScale;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3FC) = verticalSpeed * g_EclGameTimeScale;
    this->position.operator float *()[0] += *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3F8);
    this->position.operator float *()[1] += *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3FC);

    if (this->position.operator float *()[0] < *reinterpret_cast<f32 *>(0x0164D2EC))
        this->position.operator float *()[0] = *reinterpret_cast<f32 *>(0x0164D2EC);
    else if (this->position.operator float *()[0] >
             *reinterpret_cast<f32 *>(0x0164D2EC) + *reinterpret_cast<f32 *>(0x0164D2F4))
    {
        this->position.operator float *()[0] =
            *reinterpret_cast<f32 *>(0x0164D2EC) + *reinterpret_cast<f32 *>(0x0164D2F4);
    }
    if (this->position.operator float *()[1] < *reinterpret_cast<f32 *>(0x0164D2F0))
        this->position.operator float *()[1] = *reinterpret_cast<f32 *>(0x0164D2F0);
    else if (this->position.operator float *()[1] >
             *reinterpret_cast<f32 *>(0x0164D2F0) + *reinterpret_cast<f32 *>(0x0164D2F8))
    {
        this->position.operator float *()[1] =
            *reinterpret_cast<f32 *>(0x0164D2F0) + *reinterpret_cast<f32 *>(0x0164D2F8);
    }

    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x38C) = this->position - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x398) = this->position + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3A4) = this->position - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3E0);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3B0) = this->position + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3E0);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3BC) = this->position - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3EC);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3C8) = this->position + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3EC);

    for (optionUpdateIndex = 0; optionUpdateIndex < 4; ++optionUpdateIndex)
    {
        if (*reinterpret_cast<OptionUpdateCallback *>(reinterpret_cast<u8 *>(this) + 0x6F8 + optionUpdateIndex * 0x2F4) != NULL)
        {
            (*reinterpret_cast<OptionUpdateCallback *>(reinterpret_cast<u8 *>(this) + 0x6F8 + optionUpdateIndex * 0x2F4))(
                this, reinterpret_cast<u8 *>(this) + 0x40C + optionUpdateIndex * 0x2F4);
            g_AnmManager->ExecuteScript(
                reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x40C + optionUpdateIndex * 0x2F4));
            (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x6EC + optionUpdateIndex * 0x2F4))++;
        }
    }

    if ((g_GuiMessageInputCurrent & 1) != 0 && !g_Gui.IsDialogPresent() && !g_GameManager.IsTampered())
        this->FUN_00451640();

    if (!g_Gui.IsDialogPresent() && *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 30 &&
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) == 0)
    {
        gaugeDelta = 0;
        if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) >= 0)
        {
            if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0) > 0)
                (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0))--;
            else
            {
                gaugeDelta = (i32)((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) > 300.0f
                                       ? 21.0f
                                       : (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) / 15.0f);
                if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
                    gaugeDelta = -gaugeDelta;
                g_GameManager.AddToYoukaiGauge((i32)((f32)gaugeDelta * g_EclGameTimeScale), 0);
                (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8))++;
            }
        }
        else
        {
            if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0) >= 4)
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) = 0;
            if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0) >= 30)
            {
                if (fabs((double)g_GameManager.GetYoukaiGauge()) <= 9.0)
                {
                    g_GameManager.SetYoukaiGauge(0);
                }
                else
                {
                    if (g_GameManager.GaugeIsExtremelyYoukai()) gaugeDelta = -5;
                    else if (g_GameManager.GaugeIsModeratelyYoukai()) gaugeDelta = -3;
                    else if (g_GameManager.GetYoukaiGauge() > 0) gaugeDelta = -2;
                    else if (!g_GameManager.GaugeIsModeratelyHuman()) gaugeDelta = 2;
                    else if (!g_GameManager.GaugeIsExtremelyHuman()) gaugeDelta = 3;
                    else gaugeDelta = 5;
                    g_GameManager.AddToYoukaiGauge((i32)((f32)gaugeDelta * g_EclGameTimeScale), 0);
                }
            }
            else
                (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0))++;
        }
    }

    if ((g_GameManager.GaugeIsExtremelyHuman() || g_GameManager.GaugeIsExtremelyYoukai()) &&
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) == NULL)
    {
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) =
            g_EffectManager.FUN_00425870(25, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 8, 1, -1);
    }
    if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) != NULL)
    {
        *reinterpret_cast<Float3 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B24) + 0x2A4) = this->position;
        if (!g_GameManager.GaugeIsExtremelyHuman() && !g_GameManager.GaugeIsExtremelyYoukai())
        {
            *reinterpret_cast<u8 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B24) + 0x350) = 0;
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) = NULL;
        }
    }

    if (verticalSpeed != 0.0f || horizontalSpeed != 0.0f)
    {
        for (historyIndex = 15; historyIndex > 0; --historyIndex)
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC + historyIndex * 12) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC + (historyIndex - 1) * 12);
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC) = this->position;
    }
    return 0;
}
// FUNCTION: th08 0x451150
#pragma var_order(i, slot, this)
void Player::FUN_00451150()
{
    u8 *slot;
    i32 i;

    if ((*reinterpret_cast<u32 *>(0x164D0B4) >> 10) & 1)
    {
        return;
    }

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) == 0)
        {
            continue;
        }

        if (*reinterpret_cast<u32 *>(slot + 0x474) != 0)
        {
            if (reinterpret_cast<i32 (__fastcall *)(Player *, u8 *)>(*reinterpret_cast<u32 *>(slot + 0x474))(this, slot) != 0)
            {
                *reinterpret_cast<i16 *>(slot + 0x462) = 0;
                continue;
            }
        }

        reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0] +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(slot + 0x43C);
        reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1] +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(slot + 0x440);

        if (*reinterpret_cast<i16 *>(slot + 0x464) != 4 && *reinterpret_cast<i16 *>(slot + 0x464) != 5)
        {
            if (!g_GameManager.IsWithinPlayfield(
                    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0],
                    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1],
                    *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(slot + 0x224) + 0x34),
                    *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(slot + 0x224) + 0x30)))
            {
                *reinterpret_cast<i16 *>(slot + 0x462) = 0;
            }
        }

        if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(slot)) != ZUN_SUCCESS)
        {
            *reinterpret_cast<i16 *>(slot + 0x462) = 0;
        }
        (*reinterpret_cast<ZunTimer *>(slot + 0x454))++;
    }
}
// FUNCTION: th08 0x4512f0
#pragma var_order(i, slot, this)
void Player::FUN_004512f0()
{
    u8 *slot;
    i32 i;

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 1)
        {
            continue;
        }
        if (*reinterpret_cast<i16 *>(slot + 0x1FC) != 0)
        {
            reinterpret_cast<AnmVm *>(slot)->SetZRotation(*reinterpret_cast<f32 *>(slot + 0x450));
        }
        *reinterpret_cast<f32 *>(slot + 0x208) = g_ItemAnmManagerScreenShakeOffset.x + *reinterpret_cast<f32 *>(slot + 0x2A4);
        *reinterpret_cast<f32 *>(slot + 0x20C) = g_ItemAnmManagerScreenShakeOffset.y + *reinterpret_cast<f32 *>(slot + 0x2A8);
        *reinterpret_cast<f32 *>(slot + 0x210) = 0.4f;
        if (*reinterpret_cast<i8 *>(slot + 0x470) != 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x1F2) = 0xff;
            *reinterpret_cast<u8 *>(slot + 0x1F1) = 0x40;
            *reinterpret_cast<u8 *>(slot + 0x1F0) = 0x40;
        }
        g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(slot));
        if (*reinterpret_cast<u32 *>(slot + 0x478) != 0)
        {
            reinterpret_cast<void (__fastcall *)(Player *, u8 *)>(*reinterpret_cast<u32 *>(slot + 0x478))(this, slot);
        }
    }
}

// FUNCTION: th08 0x451400
#pragma var_order(i, slot, this)
void Player::FUN_00451400()
{
    u8 *slot;
    i32 i;

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 2)
        {
            continue;
        }
        if (*reinterpret_cast<i16 *>(slot + 0x1FC) != 0)
        {
            reinterpret_cast<AnmVm *>(slot)->SetZRotation(*reinterpret_cast<f32 *>(slot + 0x450));
        }
        *reinterpret_cast<f32 *>(slot + 0x208) = g_ItemAnmManagerScreenShakeOffset.x + *reinterpret_cast<f32 *>(slot + 0x2A4);
        *reinterpret_cast<f32 *>(slot + 0x20C) = g_ItemAnmManagerScreenShakeOffset.y + *reinterpret_cast<f32 *>(slot + 0x2A8);
        *reinterpret_cast<f32 *>(slot + 0x210) = 0.2f;
        if (*reinterpret_cast<i8 *>(slot + 0x470) != 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x1F2) = 0xff;
            *reinterpret_cast<u8 *>(slot + 0x1F1) = 0x40;
            *reinterpret_cast<u8 *>(slot + 0x1F0) = 0x40;
        }
        g_AnmManager->DrawPlayerBullet(reinterpret_cast<AnmVm *>(slot));
    }
}
// FUNCTION: th08 0x451500
i32 Player::FUN_00451500()
{
    if (*reinterpret_cast<i32 *>(0x164D2C8) < 20)
    {
        return 0;
    }

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
    {
        return 0;
    }

    if (this->FUN_00451d50())
    {
        return 0;
    }

    if (reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4)->FUN_0040d3d0())
    {
        if (*reinterpret_cast<i32 *>(0x17D6ED4) == 0 ||
            (*reinterpret_cast<u8 *>(0x164D0B1) != 1 && *reinterpret_cast<u8 *>(0x164D0B1) != 7 &&
             *reinterpret_cast<u8 *>(0x164D0B1) != 6))
        {
            this->FUN_00450f60((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4));
        }
    }

    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4))++;

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) >= 20)
    {
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = -1;
    }

    if ((*reinterpret_cast<u16 *>(0x164D52C) & 1) != 0)
    {
        if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
        {
            if (!g_Gui.IsDialogPresent())
            {
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = 0;
            }
        }
    }

    if (*reinterpret_cast<i8 *>(this) == 2 || *reinterpret_cast<i8 *>(this) == 1)
    {
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = -1;
    }

    return 0;
}

// FUNCTION: th08 0x44fb70
void __fastcall Player::FUN_0044fb70(u8 *slot, u8 *entry)
{
    if (*reinterpret_cast<i16 *>(entry + 0x20) == 0)
    {
        *reinterpret_cast<Float3 *>(slot + 0x2A4) =
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4);
    }
    else
    {
        *reinterpret_cast<Float3 *>(slot + 0x2A4) = *reinterpret_cast<Float3 *>(
            reinterpret_cast<u8 *>(this) + ((*reinterpret_cast<i16 *>(entry + 0x20) - 1) * 0x2F4) + 0x6B0);
    }

    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0] += *reinterpret_cast<f32 *>(entry + 0x4);
    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1] += *reinterpret_cast<f32 *>(entry + 0x8);
    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[2] = 0.495f;

    *reinterpret_cast<u32 *>(slot + 0x430) = *reinterpret_cast<u32 *>(entry + 0x0C);
    *reinterpret_cast<u32 *>(slot + 0x434) = *reinterpret_cast<u32 *>(entry + 0x10);
    *reinterpret_cast<f32 *>(slot + 0x438) = 1.0f;
    *reinterpret_cast<u32 *>(slot + 0x450) = *reinterpret_cast<u32 *>(entry + 0x14);
    *reinterpret_cast<u32 *>(slot + 0x44C) = *reinterpret_cast<u32 *>(entry + 0x18);
    *reinterpret_cast<f32 *>(slot + 0x43C) = cosf(*reinterpret_cast<f32 *>(entry + 0x14)) * *reinterpret_cast<f32 *>(entry + 0x18);
    *reinterpret_cast<f32 *>(slot + 0x440) = sinf(*reinterpret_cast<f32 *>(entry + 0x14)) * *reinterpret_cast<f32 *>(entry + 0x18);

    *reinterpret_cast<ZunTimer *>(slot + 0x454) = 0;
    *reinterpret_cast<u8 *>(slot + 0x46C) = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3);
    *reinterpret_cast<i16 *>(slot + 0x464) = *reinterpret_cast<i16 *>(entry + 0x22);
    *reinterpret_cast<i16 *>(slot + 0x460) = *reinterpret_cast<i16 *>(entry + 0x1C);
    *reinterpret_cast<i16 *>(slot + 0x46E) = *reinterpret_cast<i16 *>(entry + 0x24);

    if (*reinterpret_cast<i16 *>(entry + 0x26) >= 0)
    {
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(*reinterpret_cast<i16 *>(entry + 0x26)),
                                               *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4));
    }

    reinterpret_cast<AnmLoaded *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xC))
        ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(slot), *reinterpret_cast<i16 *>(entry + 0x24) + 10);

    *reinterpret_cast<u8 *>(slot + 0x470) = 0;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        if (*reinterpret_cast<i16 *>(entry + 0x1E) > 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x470) = 1;
        }
    }
}

// FUNCTION: th08 0x44fd80
#pragma var_order(slot, this)
i32 __fastcall Player::FUN_0044fd80(u8 *slot, i32 value, u8 *entry)
{
    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        return 1;
    }

    return 0;
}


// FUNCTION: th08 0x44fdd0
#pragma var_order(slot, this)
i32 __fastcall Player::FUN_0044fdd0(u8 *slot, i32 value, u8 *entry)
{
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) == 0 &&
        value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x44fe20
#pragma var_order(index, i, this, slot)
i32 __fastcall Player::FUN_0044fe20(u8 *slot, i32 value, u8 *entry)
{
    i32 index;
    i32 i;

    index = *reinterpret_cast<i16 *>(entry + 2);
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) != 0)
    {
        return 0;
    }
    if ((*reinterpret_cast<u32 *>(0x164D0B4) >> 13) & 1)
    {
        return 0;
    }

    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) != 0)
    {
        if (*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A80 + index * 4) != entry)
        {
            *reinterpret_cast<i16 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) + 0x1FE) = 1;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) = 0;
        }
        return 0;
    }

    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2A38 + (index << 4)) = 999;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) = slot;
    *reinterpret_cast<i16 *>(slot + 0x466) = index;
    *reinterpret_cast<i16 *>(slot + 0x468) = *reinterpret_cast<i16 *>(entry + 0x20);
    *reinterpret_cast<u32 *>(slot + 0x444) = *reinterpret_cast<u32 *>(entry + 4);
    *reinterpret_cast<u32 *>(slot + 0x448) = *reinterpret_cast<u32 *>(entry + 8);
    *reinterpret_cast<i16 *>(slot + 0x46A) = *reinterpret_cast<i16 *>(entry);
    this->FUN_0044fb70(slot, entry);

    for (i = 31; i >= 0; i--)
    {
        *reinterpret_cast<u32 *>(slot + i * 0xC + 0x2B0) = 0xC479C000;
    }
    *reinterpret_cast<u32 *>(slot + 0x2A4) = 0xC479C000;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A80 + index * 4) = entry;

    return 1;
}

// FUNCTION: th08 0x44ffa0
#pragma var_order(magnitude, angle, this, slot)
i32 __fastcall Player::FUN_0044ffa0(u8 *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AB0) > -100.0f)
        {
            angle = AddNormalizeAngle(
                VectorAngle(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AB4) - *reinterpret_cast<f32 *>(slot + 0x2A8),
                            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AB0) - *reinterpret_cast<f32 *>(slot + 0x2A4)),
                *reinterpret_cast<f32 *>(entry + 0x14) + ZUN_PI / 2.0f);
            magnitude = *reinterpret_cast<f32 *>(entry + 0x18) * 1.5f;
            reinterpret_cast<Float3 *>(slot + 0x43C)->FromAngleMagnitude(angle, magnitude);
            *reinterpret_cast<u32 *>(slot + 0x450) = *reinterpret_cast<u32 *>(&angle);
        }
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x450f60
#pragma var_order(i, table, slot, result, entry, this, value)
void __fastcall Player::FUN_00450f60(i32 value)
{
    unsigned __int64 *table;
    u8 *entry;
    u8 *slot;
    i32 result;
    i32 i;

    table = (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
                ? reinterpret_cast<unsigned __int64 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A74) + 0x38)
                : reinterpret_cast<unsigned __int64 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A78) + 0x38);

    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) != 0 &&
        ((*reinterpret_cast<u8 *>(0x164D0B1) == 2 &&
          (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) & 1) != 0) ||
         *reinterpret_cast<u8 *>(0x164D0B1) == 9) &&
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4) >= 60)
    {
        table += ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) & 2) ? 7 : 6);
    }
    else
    {
        while (g_GameManager.GetPower() >= *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(table) + 4))
        {
            table++;
        }
    }

    entry = *reinterpret_cast<u8 **>(table);
    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 0)
        {
            continue;
        }

processEntry:
        if (*reinterpret_cast<u32 *>(entry + 0x28) != 0)
        {
            result = reinterpret_cast<i32 (__fastcall *)(Player *, u8 *, i32, u8 *)>(
                *reinterpret_cast<u32 *>(entry + 0x28))(this, slot, value, entry);
        }
        else
        {
            result = this->FUN_0044fd80(slot, value, entry);
        }

        if (result == 1)
        {
            *reinterpret_cast<u32 *>(slot + 0x1F8) |= 0x2000;
            *reinterpret_cast<i16 *>(slot + 0x462) = 1;
            *reinterpret_cast<u8 **>(slot + 0x480) = entry;
            *reinterpret_cast<u32 *>(slot + 0x474) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x2C);
            *reinterpret_cast<u32 *>(slot + 0x478) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x30);
            *reinterpret_cast<u32 *>(slot + 0x47C) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x34);
        }

        entry += 0x38;
        if (*reinterpret_cast<i16 *>(entry) < 0)
        {
            return;
        }
        if (result == 0)
        {
            goto processEntry;
        }
    }
}

// FUNCTION: th08 0x451640
void Player::FUN_00451640()
{
    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = 0;
}

// FUNCTION: th08 0x451d50
i32 Player::FUN_00451d50()
{
    return *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xfdc) != 0 &&
           *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xfe0) == 4;
}
// FUNCTION: th08 0x44d420
void Player::FUN_0044d420()
{
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xE2AA4) = Float3(-999.0f, -999.0f, 0.0f);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xE2AB0) = Float3(-999.0f, -999.0f, 0.0f);
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2AC0) = 0;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B8) >= 400.0f)
    {
        if (g_AsciiManager.GetGaugeInterrupt() != 2)
        {
            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4) < 160.0f)
            {
                g_AsciiManager.SetGaugeInterrupt(2);
                goto doneTop;
            }
        }

        if (g_AsciiManager.GetGaugeInterrupt() == 2)
        {
            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4) > 160.0f)
            {
                g_AsciiManager.SetGaugeInterrupt(3);
            }
        }

doneTop:
        return;
    }

    if (g_AsciiManager.GetGaugeInterrupt() == 2)
    {
        g_AsciiManager.SetGaugeInterrupt(3);
    }
}
// FUNCTION: th08 0x44c390
ChainCallbackResult Player::OnUpdate(Player *player)
{
    if (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x2C) != 0)
    {
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xBE834) != 0)
        {
            *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xBE834) + 0x1F8) |= 0x80000;
        }
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B24) != 0)
        {
            *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xE2B24) + 0x1F8) |= 0x80000;
        }
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xBE834) != 0)
    {
        *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xBE834) + 0x1F8) &= 0xfff7ffff;
    }
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B24) != 0)
    {
        *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xE2B24) + 0x1F8) &= 0xfff7ffff;
    }
    player->FUN_0044c5b0();
    player->FUN_0044c650();
    if (player->playerState == PLAYER_STATE_DYING)
    {
        if (player->FUN_0044cbf0() != 0)
        {
            goto updateD180;
        }
    }
    else if (player->playerState == PLAYER_STATE_SPAWNING)
    {
updateD180:
        player->FUN_0044d180();
    }
    player->FUN_0044d2c0();
    if (player->playerState != PLAYER_STATE_DYING && player->playerState != PLAYER_STATE_SPAWNING)
    {
        player->FUN_0044aec0();
    }
    g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10));
    player->FUN_00451150();
    player->FUN_00451500();
    player->FUN_0044d420();
    if (!g_Gui.IsDialogPresent())
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE10) += 1;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE14) += 1;
        if (g_GameManager.GaugeIsExtremelyHuman())
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE1C) += 1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE24) += 1;
            g_GameManager.AddScore(100);
        }
        else if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE18) += 1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE20) += 1;
            g_GameManager.AddScore(100);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(index, slot)
// FUNCTION: th08 0x44c5b0
void Player::FUN_0044c5b0()
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 384; index++, slot++)
    {
        if (slot->lifetime < 0)
            continue;

        slot->lifetime--;
        slot->radius += slot->radiusGrowth;
        slot->size.x += slot->sizeGrowth.x;
        slot->size.y += slot->sizeGrowth.y;

        if (slot->lifetime <= 0)
            slot->Deactivate();
    }
}

// FUNCTION: th08 0x44d530
#pragma var_order(i, this)
ChainCallbackResult Player::OnDrawHighPrio(Player *player)
{
    u32 i;

    player->FUN_004512f0();

    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xFDC) != 0)
    {
        reinterpret_cast<void (__fastcall *)(Player *)>(
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) +
                                      (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xFE0) * 4) +
                                      0x1014))(player);
    }

    if (*reinterpret_cast<u8 *>(0x164D0BB) == 0)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x218) =
            g_ItemAnmManagerScreenShakeOffset.x + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x2B4);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x21C) =
            g_ItemAnmManagerScreenShakeOffset.y + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x2B8);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x220) = 0.1f;
        g_AnmManager->DrawNoRotation(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10));
    }

    for (i = 0; i < 4; i++)
    {
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + i * 0x2F4 + 0x6FC) != 0)
        {
            reinterpret_cast<void (__fastcall *)(Player *, u8 *)>(
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + i * 0x2F4 + 0x6FC))(
                player, reinterpret_cast<u8 *>(player) + i * 0x2F4 + 0x40C);
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d630
ChainCallbackResult Player::OnDrawLowPrio(Player *player)
{
    player->FUN_00451400();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d2c0
void Player::FUN_0044d2c0()
{
    if (this->playerStateSlotCooldown != 0)
    {
        this->playerStateSlotCooldown--;
        this->FUN_0044de60(&this->position, 768.0f, 896.0f, -1, 0);
    }

    if (this->playerState == PLAYER_STATE_DEAD)
    {
        this->stateFlag = false;

        if (this->stateEffect != NULL)
            this->stateEffect->position = this->position;

        this->timer--;
        if ((i32)this->timer <= 0)
        {
            if (this->stateEffect != NULL)
            {
                this->stateEffect->active = false;
                this->stateEffect = NULL;
            }

            this->playerState = PLAYER_STATE_ALIVE;
            this->timer = 0;
            this->mainVm.color1.d3dColor = -1;
        }
        else if ((i32)this->timer % 8 < 2)
        {
            this->mainVm.color1.d3dColor = 0xfff02020;
        }
        else
        {
            this->mainVm.color1.d3dColor = -1;
        }
    }
    else
    {
        this->timer++;
    }
}

// FUNCTION: th08 0x44d650
#pragma var_order(i, shotSlot, option, m, player)
ZunResult Player::AddedCallback(Player *player)
{
    u32 i;
    u8 *shotSlot;
    u8 *option;
    u32 m;

    if (IsResourceReloadEnabled())
    {
        if (Player::LoadShtFile(&player->primaryShtFile, g_Player1ShtFiles[g_TargetByte0164D0B1]) != ZUN_SUCCESS)
            return ZUN_ERROR;
        if (Player::LoadShtFile(&player->secondaryShtFile, g_Player2ShtFile[g_TargetByte0164D0B1]) != ZUN_SUCCESS)
            return ZUN_ERROR;
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC) =
            g_AnmManager->PreloadAnm(5, g_PlayerAnmFilenames[g_TargetByte0164D0B1]);
        if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC) == NULL)
            return ZUN_ERROR;
    }
    else
    {
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC) = g_AnmManager->GetAnm(5);
    }

    if (g_TargetByte0164D0B1 < 4 || (g_TargetByte0164D0B1 & 1) == 0)
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10), 0);
    else
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10), 5);

    player->position.operator float *()[0] = g_PlayerPlayfieldWidth / 2.0f;
    player->position.operator float *()[1] = g_ItemPlayfieldBottom - 64.0f;
    player->position.operator float *()[2] = 0.49f;

    for (i = 0; i < 0x180; ++i)
        reinterpret_cast<PlayerUnkStruct0x40 *>(player->playerSlotsB)[i].Reset();

    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3D8) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0xC) / 2.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3D4) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3D8);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3DC) = 5.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E4) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x10) / 2.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E0) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E4);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E8) = 5.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3F0) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x18) / 2.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3EC) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3F0);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3F4) = 5.0f;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2A98) = 0;
    player->playerState = PLAYER_STATE_SPAWNING;
    player->timer = g_GameManager.IsSpellPractice() ? 10 : 120;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 2) = 1;

    shotSlot = reinterpret_cast<u8 *>(player) + 0xBE838;
    for (i = 0; (i32)i < 0x80; ++i, shotSlot += 0x484)
        *reinterpret_cast<i16 *>(shotSlot + 0x462) = 0;

    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AC4) = -1;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AD0) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AE8) = 0;

    *reinterpret_cast<PlayerBombCallbacks *>(reinterpret_cast<u8 *>(player) + 0x1000) =
        g_PlayerBombCallbackTable[g_TargetByte0164D0B1 * 2];
    *reinterpret_cast<PlayerBombCallbacks *>(reinterpret_cast<u8 *>(player) + 0x1014) =
        g_PlayerBombCallbackTable[g_TargetByte0164D0B1 * 2 + 1];

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xFDC) = 0;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B0C) = 0xBFC90FDB;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 1.0f;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2A68) =
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 8);

    if (IsResourceReloadEnabled())
        g_AsciiManager.SetGaugeInterrupt(1);
    g_AsciiManager.FUN_00422bb0(0, 2);
    g_AsciiManager.FUN_00422bb0(1, 2);
    g_AsciiManager.FUN_00422bb0(2, 2);

    g_PlayerGaugeBounds[0] = -10000;
    g_PlayerGaugeBounds[2] = -8000;
    g_PlayerGaugeBounds[4] = -2000;
    g_PlayerGaugeBounds[1] = 10000;
    g_PlayerGaugeBounds[3] = 8000;
    g_PlayerGaugeBounds[5] = 2000;
    if (g_TargetByte0164D0B1 == 3)
    {
        g_PlayerGaugeBounds[0] = -5000;
        g_PlayerGaugeBounds[2] = -3000;
        g_PlayerGaugeBounds[4] = -2000;
    }
    else if (g_TargetByte0164D0B1 == 10)
    {
        g_PlayerGaugeBounds[0] = -5000;
        g_PlayerGaugeBounds[2] = -3000;
        g_PlayerGaugeBounds[4] = -2000;
        g_PlayerGaugeBounds[1] = 5000;
        g_PlayerGaugeBounds[3] = 3000;
        g_PlayerGaugeBounds[5] = 2000;
    }
    else if (g_GameManager.IsSoloHuman())
    {
        g_PlayerGaugeBounds[1] = 2000;
        g_PlayerGaugeBounds[3] = 8000;
        g_PlayerGaugeBounds[5] = 2001;
    }
    else if (g_GameManager.IsSoloYoukai())
    {
        g_PlayerGaugeBounds[0] = -2000;
        g_PlayerGaugeBounds[2] = -8000;
        g_PlayerGaugeBounds[4] = -2001;
    }

    *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2B24) = NULL;
    for (i = 0; i < 16; ++i)
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(player) + 0x2CC + i * 12) = player->position;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 3) = 2;

    if (g_TargetByte0164D0B1 > 3)
    {
        option = reinterpret_cast<u8 *>(player) + 0x40C;
        for (m = 0; m < 4; ++m, option += 0x2F4)
        {
            memset(option, 0, 0x2F4);
            *reinterpret_cast<void **>(option + 0x2EC) =
                g_PlayerOptionUpdateCallbacks[g_TargetByte0164D0B1].callbacks[m];
            *reinterpret_cast<void **>(option + 0x2F0) =
                g_PlayerOptionRenderCallbacks[g_TargetByte0164D0B1].callbacks[m];
            if (*reinterpret_cast<void **>(option + 0x2EC) != NULL)
            {
                *reinterpret_cast<i32 *>(option + 0x2C8) = 1;
                *reinterpret_cast<ZunTimer *>(option + 0x2E0) = 0;
                *reinterpret_cast<i32 *>(option + 0x2D0) = m;
            }
            else
            {
                *reinterpret_cast<i32 *>(option + 0x2C8) = 0;
            }
        }
    }

    if (g_GameManager.IsSoloHuman())
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2B2C) = 27;
    else
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2B2C) = 40;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x2E10) =
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2B2C);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x44dc60
ZunResult Player::DeletedCallback(Player *player)
{
    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(5);
        g_AsciiManager.SetGaugeInterrupt(99);
        g_AsciiManager.FUN_00422bb0(0, 99);
        g_AsciiManager.FUN_00422bb0(1, 99);
        g_AsciiManager.FUN_00422bb0(2, 99);

        if (g_PlayerPrimaryShtFile != NULL)
        {
            g_ZunMemory.Free(g_PlayerPrimaryShtFile);
            g_PlayerPrimaryShtFile = NULL;
        }

        if (g_PlayerSecondaryShtFile != NULL)
        {
            g_ZunMemory.Free(g_PlayerSecondaryShtFile);
            g_PlayerSecondaryShtFile = NULL;
        }
    }

    return ZUN_SUCCESS;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44de60
PlayerUnkStruct0x40 *Player::FUN_0044de60(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->size.x = value1;
    slot->size.y = value2;
    slot->lifetime = value4;
    slot->collisionValue = value3;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44df00
PlayerUnkStruct0x40 *Player::FUN_0044df00(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->radius = value1;
    slot->radiusGrowth = value2;
    slot->lifetime = value3;
    slot->collisionValue = value4;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44dfa0
PlayerUnkStruct0x40 *Player::FUN_0044dfa0(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->size.x = value1;
    slot->size.y = value2;
    slot->lifetime = value4;
    slot->damage = value3;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44e040
PlayerUnkStruct0x40 *Player::FUN_0044e040(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->radius = value1;
    slot->radiusGrowth = value2;
    slot->lifetime = value4;
    slot->damage = value3;

    return slot;
}

void Player::CutChain()
{
    g_Chain.Cut(g_PlayerCalcChain);
    g_PlayerCalcChain = NULL;
    g_Chain.Cut(g_PlayerDrawChainHighPrio);
    g_PlayerDrawChainHighPrio = NULL;
    g_Chain.Cut(g_PlayerDrawChainLowPrio);
    g_PlayerDrawChainLowPrio = NULL;
}

// FUNCTION: th08 0x44dd70
#pragma var_order(i, entry, header, path)
ZunResult Player::LoadShtFile(PlayerRawShtFile **header, const char *path)
{
    i32 i;
    u8 *entry;

    *header = reinterpret_cast<PlayerRawShtFile *>(FileSystem::OpenFile(path, NULL, 0));
    if (*header == NULL)
    {
        return ZUN_ERROR;
    }

    for (i = 0; i < *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(*header) + 0x2); i++)
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(*header) + i * 8 + 0x38) +=
            reinterpret_cast<u32>(*header);
        entry = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(*header) + i * 8 + 0x38);

        while (*reinterpret_cast<i16 *>(entry) >= 0)
        {
            *reinterpret_cast<u32 *>(entry + 0x28) =
                *reinterpret_cast<u32 *>(0x4C7EE0 + *reinterpret_cast<u32 *>(entry + 0x28) * 4);
            *reinterpret_cast<u32 *>(entry + 0x2C) =
                *reinterpret_cast<u32 *>(0x4C7F04 + *reinterpret_cast<u32 *>(entry + 0x2C) * 4);
            *reinterpret_cast<u32 *>(entry + 0x30) =
                *reinterpret_cast<u32 *>(0x4C7F1C + *reinterpret_cast<u32 *>(entry + 0x30) * 4);
            *reinterpret_cast<u32 *>(entry + 0x34) =
                *reinterpret_cast<u32 *>(0x4C7F24 + *reinterpret_cast<u32 *>(entry + 0x34) * 4);
            entry += 0x38;
        }
    }

    return ZUN_SUCCESS;
}

} /* namespace th08 */
