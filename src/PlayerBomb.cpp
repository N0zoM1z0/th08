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

// Repository production TU for the target-contiguous Player bomb/shot callback
// family at 0x0040BC20..0x004142C0. The filename describes the recovered
// cluster; it does not claim the original source filename.
namespace th08
{
DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);
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

// FUNCTION: th08 0x40c010
#pragma var_order(i, bomb, workItem, angle)
void __fastcall FUN_0040c010(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x97\xEC\x95\x84\x81\x75\x96\xB2\x91\x7A\x96\xAD\x8E\xEC\x81\x76",
                     200, 260, 0);
        g_EffectManager.SpawnEffect(12, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xFF4040FF);
        angle = -ZUN_PI;
        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[i].vms[0], 19);
            bomb->workItems[i].rotation = angle;
            angle += ZUN_PI / 8.0f;
            workItem->anchor = player->position;
            workItem->points[0] = workItem->anchor;
            workItem->rotationStep = 0.0f;
            workItem->active = 1;
            workItem->cancelSlot = player->FUN_0044df00(&player->position, 96.0f, 0.0f, 200, 6);
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 64.0f, 0.0f, 5, 200);
            workItem->damageSlot->collisionInterval = 2;
            workItem->damageSlot->hitCap = 200;
            workItem->damageSlot->mode = 1;
        }
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
    }

    if (bomb->timer < 40)
    {
        Float3 previousPosition;
        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->rotation = AddNormalizeAngle(
                workItem->rotation, (i & 1) ? 0.052359879016876221f : -0.052359879016876221f);
            previousPosition = workItem->anchor;
            workItem->anchor.x = cosf(workItem->rotation) * workItem->rotationStep + workItem->points[0].x;
            workItem->anchor.y = sinf(workItem->rotation) * workItem->rotationStep + workItem->points[0].y;
            workItem->rotationStep += 3.2f;
            workItem->position = workItem->anchor - previousPosition;
        }
    }
    else
    {
#pragma var_order(yDelta, xDelta, speed, targetPosition, slot)
        Float3 targetPosition;
        f32 speed;
        f32 xDelta;
        f32 yDelta;
        PlayerUnkStruct0x40 *slot;
        if (bomb->timer.FUN_0040e350(40))
        {
            workItem = bomb->workItems;
            for (i = 0; i < 16; i++, workItem++)
            {
                workItem->speed = sqrtf(workItem->position.x * workItem->position.x +
                                        workItem->position.y * workItem->position.y);
                workItem->rotation = VectorAngle(workItem->position.x, workItem->position.y);
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(workItem) + 4) = 0;
                workItem->rotationStep = 8.0f;
            }
        }

        i = 0;
        workItem = bomb->workItems;
        for (; i < 16; i++, workItem++)
        {
            if (!workItem->active)
                continue;

            if (workItem->active == 1 && bomb->timer.FUN_0040d3d0())
            {
                if (player->tailPosition0.x > -100.0f)
                    targetPosition = player->tailPosition0;
                else
                    targetPosition = player->position;

                    xDelta = targetPosition.operator float *()[0] - workItem->anchor.x;
                    yDelta = targetPosition.operator float *()[1] - workItem->anchor.y;
                    speed = sqrtf(xDelta * xDelta + yDelta * yDelta) / (workItem->rotationStep / 8.0f);
                    if (speed < 1.0f)
                        speed = 1.0f;

                    xDelta = xDelta / speed + workItem->position.x;
                    yDelta = yDelta / speed + workItem->position.y;
                    speed = sqrtf(xDelta * xDelta + yDelta * yDelta);
                    workItem->rotationStep = speed > 10.0f ? 10.0f : speed;
                    if (workItem->rotationStep < 1.0f)
                        workItem->rotationStep = 1.0f;
                    workItem->position.x = xDelta * workItem->rotationStep / speed;
                    workItem->position.y = yDelta * workItem->rotationStep / speed;

                    player->FUN_0044df00(&workItem->anchor, 128.0f, 0.0f, 0, 6);
                    if (workItem->damageSlot->hitAccumulator >= workItem->damageSlot->hitCap ||
                        bomb->timer >= bomb->duration - 30)
                    {
                        workItem->cancelSlot->active = 0;
                        workItem->damageSlot->active = 0;
                        player->FUN_0044df00(&player->position, 64.0f, 4.266666889190674f, 30, 6);
                        slot = player->FUN_0044e040(&workItem->anchor, 64.0f, 12.800000190734863f, 500, 12);
                        slot->collisionInterval = 4;
                        slot->hitCap = 0;
                        g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor), 8, -1);
                        workItem->active = 2;
                        workItem->vms[0].pendingInterrupt = 1;
                        workItem->position / 8.0f;
                        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->anchor.x);
                        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
                    }
            }

            workItem->anchor.x += g_EclGameTimeScale * workItem->position.x;
            workItem->anchor.y += g_EclGameTimeScale * workItem->position.y;
        }
    }

    i = 0;
    workItem = bomb->workItems;
    for (; i < 16; i++, workItem++)
    {
        if (!workItem->active)
            continue;
        if (workItem->active == 1)
        {
            workItem->cancelSlot->center.x = workItem->anchor.x;
            workItem->cancelSlot->center.y = workItem->anchor.y;
            workItem->damageSlot->center.x = workItem->anchor.x;
            workItem->damageSlot->center.y = workItem->anchor.y;
        }
        else if (workItem->active && bomb->timer.FUN_0040d3d0())
        {
            if (++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(workItem) + 4) >= 30)
                workItem->active = 0;
        }
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
    }
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


DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 7, g_PlayerDreamSealColors) = {
    0x8FFFFFFF, 0x8F0000FF, 0x8FFF00FF, 0x8FFF0000, 0x8FFFFF00, 0x8F00FF00, 0x8F00FFFF,
};

// FUNCTION: th08 0x40c910
#pragma var_order(i, bomb, workItem, angle, previousPosition)
void __fastcall FUN_0040c910(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x90\x5F\x97\xEC\x81\x75\x96\xB2\x91\x7A\x95\x95\x88\xF3\x81\x40\x8F\x75\x81\x76",
                     200, 260, 1);
        g_EffectManager.SpawnEffect(12, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xFF4040FF);
        angle = -ZUN_PI;
        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[i].vms[0], 19);
            bomb->workItems[i].rotation = angle;
            angle += ZUN_PI / 8.0f;
            workItem->anchor = player->position;
            workItem->points[0] = workItem->anchor;
            workItem->rotationStep = 0.0f;
            workItem->active = 1;
            workItem->cancelSlot = player->FUN_0044df00(&player->position, 96.0f, 0.0f, 200, 6);
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 64.0f, 0.0f, 5, 200);
            workItem->damageSlot->collisionInterval = 2;
            workItem->damageSlot->hitCap = 200;
            workItem->damageSlot->mode = 1;
        }
        bomb->secondaryWorkIndex = 0;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
    }

    Float3 previousPosition;
    workItem = bomb->workItems;
    for (i = 0; i < 16; i++, workItem++)
    {
        if (workItem->active == 1)
        {
            workItem->rotation = AddNormalizeAngle(
                workItem->rotation, (i & 1) ? 0.052359879016876221f : -0.052359879016876221f);
            previousPosition = workItem->anchor;
            workItem->anchor.x = cosf(workItem->rotation) * workItem->rotationStep + workItem->points[0].x;
            workItem->anchor.y = sinf(workItem->rotation) * workItem->rotationStep + workItem->points[0].y;
            if (bomb->timer < 40)
            {
                if (i & 1)
                    workItem->rotationStep += 1.2000000476837158f;
                else
                    workItem->rotationStep += 2.4000000953674316f;
            }
            workItem->position = workItem->anchor - previousPosition;

            if (bomb->timer >= bomb->duration - 40 - i)
            {
                workItem->cancelSlot->active = 0;
                workItem->damageSlot->active = 0;
                player->FUN_0044df00(&player->position, 64.0f, 4.266666889190674f, 30, 6);
                PlayerUnkStruct0x40 *slot =
                    player->FUN_0044e040(&workItem->anchor, 64.0f, 8.533333778381348f, 25, 15);
                slot->collisionInterval = 5;
                slot->hitCap = 50;
                g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor), 8, -1);
                workItem->active = 2;
                workItem->vms[0].pendingInterrupt = 1;
                workItem->position / 8.0f;
                g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->anchor.x);
                ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
            }

            workItem->cancelSlot->center.x = workItem->anchor.x;
            workItem->cancelSlot->center.y = workItem->anchor.y;
            workItem->damageSlot->center.x = workItem->anchor.x;
            workItem->damageSlot->center.y = workItem->anchor.y;
        }
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
    }

    if (bomb->timer >= 40)
    {
#pragma var_order(color, spawnPosition, slot)
        Float3 spawnPosition;
        u32 color;
        PlayerUnkStruct0x40 *slot;
        if (bomb->timer % 20 == 0)
        {
            workItem = &bomb->workItems[bomb->secondaryWorkIndex + 16];
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 20);
            bomb->secondaryWorkIndex = 1;
            workItem->active = 1;
            color = g_PlayerDreamSealColors[(u32)((i32)bomb->timer / 20) % 7];
            if (player->tailPosition0.x > -100.0f)
                spawnPosition = player->tailPosition0;
            else
            {
                spawnPosition.x = g_Rng.GetRandomF32InRange(320.0f) + 32.0f;
                spawnPosition.y = g_Rng.GetRandomF32InRange(384.0f) + 32.0f;
                spawnPosition.z = 0.0f;
            }
            workItem->anchor = spawnPosition;
            g_EffectManager.SpawnEffect(49, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor), 1, color);
            g_EffectManager.SpawnEffect(55, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor), 1, color);
            workItem->cancelSlot = player->FUN_0044df00(&spawnPosition, 64.0f, 4.266666889190674f, 30, 6);
            workItem->damageSlot = player->FUN_0044e040(&spawnPosition, 64.0f, 8.533333778381348f, 400, 15);
            slot = workItem->damageSlot;
            slot->collisionInterval = 2;
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->anchor.x);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, color, 0, 21);
        }
    }

    workItem = &bomb->workItems[16];
    for (i = 16; i < ARRAY_SIZE(bomb->workItems); i++, workItem++)
    {
        if (!workItem->active)
            continue;
        workItem->cancelSlot->center.x = workItem->anchor.x;
        workItem->cancelSlot->center.y = workItem->anchor.y;
        workItem->damageSlot->center.x = workItem->anchor.x;
        workItem->damageSlot->center.y = workItem->anchor.y;
        if (g_AnmManager->ExecuteScript(&workItem->vms[0]))
            workItem->active = 0;
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
// FUNCTION: th08 0x40d430
#pragma var_order(bomb, workItem)
void __fastcall FUN_0040d430(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 1,
                     "\x96\x82\x95\x84\x81\x75\x83\x41\x81\x5B\x83\x65\x83\x42\x83\x74\x83\x8B\x83\x54\x83\x4E\x83\x8A\x83\x74\x83\x40\x83\x43\x83\x58\x81\x76",
                     210, 250, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->anchor = player->optionStates[0].position;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
    }

    if (bomb->timer < 60)
    {
        Float3 target(192.0f, 224.0f, 0.0f);
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        player->optionStates[0].position =
            (target - workItem->anchor) * interp + workItem->anchor;
        player->optionStates[0].vm.rotation.z += -0.31415927410125732f;
        player->FUN_0044df00(&player->optionStates[0].position, 32.0f, 0.0f, 0, 6);
        player->FUN_0044e040(&player->optionStates[0].position, 32.0f, 0.0f, 40, 0);
    }
    else
    {
        player->optionStates[0].vm.rotation.z = 0.0f;
        player->optionStates[0].position.x = 192.0f;
        player->optionStates[0].position.y = 224.0f;
        if (bomb->timer >= 150)
            player->optionStates[0].vm.color1.a = 0;

        if (bomb->timer.FUN_0040e350(60))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.FUN_0040e350(64))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.FUN_0040e350(68))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -20304);
        }
        else if (bomb->timer.FUN_0040e350(72))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.FUN_0040e350(76))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -49088);
        }
        else if (bomb->timer.FUN_0040e350(90))
        {
#pragma var_order(effect, damageSlot)
            AnmVm *effect;
            PlayerUnkStruct0x40 *damageSlot;
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(15), 0);
            effect = g_EffectManager.SpawnEffect(42, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(43, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(44, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            player->FUN_0044df00(&player->optionStates[0].position, 1.0f, 5.0f, 110, 6);
            damageSlot = player->FUN_0044e040(&player->optionStates[0].position, 1.0f, 5.0f, 70, 110);
            damageSlot->collisionInterval = 5;
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 24, 8, 0, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x8fffffff, 0, 21);
        }
        else if (bomb->timer.FUN_0040e350(100))
        {
            AnmVm *effect100 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.FUN_0040e350(110))
        {
            AnmVm *effect110 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.FUN_0040e350(120))
        {
            AnmVm *effect120 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.FUN_0040e350(130))
        {
            AnmVm *effect130 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -65536);
        }
        else if (bomb->timer.FUN_0040e350(150))
        {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, 0x8fffffff, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 24, 8, 0, 0, 21);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(25), 0);
        }
        else if (bomb->timer.FUN_0040e350(209))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40d950
void __fastcall FUN_0040d950(Player *player)
{
    FUN_0040bc60(player, 0x80404040);
}

// FUNCTION: th08 0x40d970
#pragma var_order(bomb, workItem)
void __fastcall FUN_0040d970(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 1,
                     "\x96\x82\x91\x80\x81\x75\x83\x8A\x83\x5E\x81\x5B\x83\x93\x83\x43\x83\x69\x83\x6A\x83\x81\x83\x67\x83\x6C\x83\x58\x81\x76",
                     230, 280, 1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->anchor = player->position;
    }

    if (bomb->timer < 60)
    {
        Float3 target(192.0f, 224.0f, 0.0f);
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        player->optionStates[0].position =
            (target - workItem->anchor) * interp + workItem->anchor;
        player->optionStates[0].vm.rotation.z += -0.31415927410125732f;
        player->FUN_0044df00(&player->optionStates[0].position, 32.0f, 0.0f, 0, 6);
        player->FUN_0044e040(&player->optionStates[0].position, 32.0f, 0.0f, 40, 0);
    }
    else
    {
        player->optionStates[0].vm.rotation.z = 0.0f;
        player->optionStates[0].position.x = 192.0f;
        player->optionStates[0].position.y = 224.0f;
        if (bomb->timer >= 128)
            player->optionStates[0].vm.color1.a = 0;

        if (bomb->timer.FUN_0040e350(60))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.FUN_0040e350(64))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.FUN_0040e350(68))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -20304);
        }
        else if (bomb->timer.FUN_0040e350(72))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.FUN_0040e350(76))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -49088);
        }
        else if (bomb->timer.FUN_0040e350(120))
        {
#pragma var_order(effect, damageSlot, burstPosition)
            AnmVm *effect;
            PlayerUnkStruct0x40 *damageSlot;
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(15), 0);
            effect = g_EffectManager.SpawnEffect(42, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(43, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(44, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            Float3 burstPosition(64.0f, 96.0f, 0.0f);
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xff0000f0);
            burstPosition.y = 352.0f;
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xfff00000);
            burstPosition.x = 320.0f;
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xff00f000);
            burstPosition.y = 96.0f;
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xff00f0f0);
            player->FUN_0044df00(&player->optionStates[0].position, 1.0f, 5.0f, 110, 6);
            damageSlot = player->FUN_0044e040(&player->optionStates[0].position, 1.0f, 5.0f, 70, 110);
            damageSlot->collisionInterval = 5;
        }
        else if (bomb->timer.FUN_0040e350(130))
        {
            AnmVm *effect130 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.FUN_0040e350(140))
        {
            AnmVm *effect140 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.FUN_0040e350(150))
        {
            AnmVm *effect150 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.FUN_0040e350(160))
        {
            AnmVm *effect160 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -65536);
        }
        else if (bomb->timer.FUN_0040e350(180))
        {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 8, 1, -1, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 24, 8, 0, 0, 21);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(25), 0);
        }
        else if (bomb->timer.FUN_0040e350(229))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
        }
    }
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

// FUNCTION: th08 0x40e040
i32 __fastcall FUN_0040e040(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 256.0f * interp;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 64;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 5.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = 0.0f;
    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) < 40)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 8.0f;
    }
    else
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = 64.0f * interp;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) += 2.0f;
    }
    return 1;
}

// FUNCTION: th08 0x40e120
i32 __fastcall FUN_0040e120(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 256.0f * interp;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 48;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = 128.0f * interp;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = ZUN_PI / 4.0f;
    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) < 40)
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 8.0f;
    else
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) += 1.5f;
    return 1;
}

// FUNCTION: th08 0x40e200
i32 __fastcall FUN_0040e200(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 256.0f * interp;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 48;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = 128.0f * interp;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = -ZUN_PI / 4.0f;
    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) < 40)
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 8.0f;
    else
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) += 1.5f;
    return 1;
}

// FUNCTION: th08 0x40e2d0
i32 __fastcall FUN_0040e2d0(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 192.0f * interp;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 8;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 8.0f;
    return 1;
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

// FUNCTION: th08 0x40e780
#pragma var_order(bomb, workItem, angle)
void __fastcall FUN_0040e780(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x96\x82\x96\x43\x81\x75\x83\x74\x83\x40\x83\x43\x83\x69\x83\x8B\x83\x58\x83\x70\x81\x5B\x83\x4E\x81\x76",
                     350, 380, 1);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(19), 0);
        workItem->anchor = player->position;
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[0], 35);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[1], 36);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[2], 37);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[3], 38);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[4], 39);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK7, 16, 120, 60, 120, 21);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.2f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.2f;
        bomb->secondaryWorkIndex = 0;
    }

    if (bomb->timer.FUN_0040ebc0(10))
    {
#pragma var_order(effect, position1, position0, scale1, scale0)
        AnmVm *effect = g_EffectManager.FUN_00425870(
            53, reinterpret_cast<D3DXVECTOR3 *>(&player->position), bomb->secondaryWorkIndex % 4 + 4, 1, -1);
        if (bomb->secondaryWorkIndex & 1)
            g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 92);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 32;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 0;

        Float3 position0;
        Float3 position1;
        position0.x = 0.0f;
        position0.y = 0.0f;
        position0.z = 0.0f;
        position1.x = 128.0f;
        position1.y = 0.0f;
        position1.z = 0.0f;
        effect->FUN_0040ec30(30, 4, &position0, &position1);

        Float2 scale0;
        Float2 scale1;
        scale0.x = 32.0f;
        scale0.y = 0.0f;
        scale1.x = 64.0f;
        scale1.y = 0.0f;
        effect->FUN_0040eda0(30, 1, &scale0, &scale1);
        effect->FUN_0040ed50(30, 3, 255, 0);
        effect->FUN_0040eca0(30, 0, -1, 0xffff0000);
        g_AnmManager->ExecuteScript(effect);
        bomb->secondaryWorkIndex++;
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(17), player->position.x);
    }

    if (player->bombState.timer.FUN_0040d3d0() && ((i32)player->bombState.timer % 4) != 0)
    {
#pragma var_order(slot, position)
        Float3 position;
        PlayerUnkStruct0x40 *slot;
        position = player->position;
        position.x = 192.0f;
        position.y /= 2.0f;
        player->FUN_0044de60(&position, 384.0f, position.y * 2.0f, 6, 0);

        position = player->position;
        position.y /= 2.0f;
        slot = player->FUN_0044dfa0(&position, 128.0f, position.y * 2.0f, 12, 0);
        slot->mode = 1;
        position.x = 192.0f;
        slot = player->FUN_0044dfa0(&position, 384.0f, position.y * 2.0f, 7, 0);
        slot->mode = 1;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 5);
}

// FUNCTION: th08 0x40ee10
#pragma var_order(bomb, workItem)
void __fastcall FUN_0040ee10(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 1,
                     "\x8D\x67\x95\x84\x81\x75\x95\x73\x96\xE9\x8F\xE9\x83\x8C\x83\x62\x83\x68\x81\x76",
                     240, 290, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem[0].anchor = player->optionStates[0].target;
        workItem[1].anchor = player->optionStates[1].target;
        workItem[2].anchor = player->optionStates[2].target;
        workItem[3].anchor = player->optionStates[3].target;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        bomb->secondaryWorkIndex = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.0f;
    }

    if (bomb->timer < 60)
    {
#pragma var_order(interp, position)
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        Float3 position;
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target =
            (position - workItem[0].anchor) * interp + workItem[0].anchor;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target =
            (position - workItem[1].anchor) * interp + workItem[1].anchor;
        position.y += 64.0f;
        player->optionStates[2].target =
            (position - workItem[2].anchor) * interp + workItem[2].anchor;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target =
            (position - workItem[3].anchor) * interp + workItem[3].anchor;
    }
    else
    {
        if (bomb->timer.FUN_0040e350(60))
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 2.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 2.0f;
            player->optionStates[0].vm.SetInterrupt(2);
            player->optionStates[1].vm.SetInterrupt(2);
            player->optionStates[2].vm.SetInterrupt(2);
            player->optionStates[3].vm.SetInterrupt(2);
        }

        Float3 position;
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target = position;
        position.y += 64.0f;
        player->optionStates[2].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target = position;

        workItem->cancelSlot = player->FUN_0044df00(&player->position, 96.0f, 0.0f, 0, 6);

        if (bomb->timer.FUN_0040ebc0(10))
        {
#pragma var_order(effect, position1, position0, scale1, scale0)
            AnmVm *effect = g_EffectManager.FUN_00425870(
                53, reinterpret_cast<D3DXVECTOR3 *>(&player->position), bomb->secondaryWorkIndex % 4 + 4, 1, -1);
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 32;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 4.0f;

            Float3 position0;
            Float3 position1;
            position0.x = 0.0f;
            position0.y = 0.0f;
            position0.z = 0.0f;
            position1.x = 192.0f;
            position1.y = g_Rng.GetRandomF32InRange(128.0f);
            position1.z = 0.0f;
            effect->FUN_0040ec30(30, 4, &position0, &position1);

            Float2 scale0;
            Float2 scale1;
            scale0.x = 64.0f;
            scale0.y = 0.0f;
            scale1.x = 64.0f;
            scale1.y = 0.0f;
            effect->FUN_0040eda0(30, 1, &scale0, &scale1);
            effect->FUN_0040ed50(30, 3, 255, 0);
            effect->FUN_0040eca0(30, 0, -1, 0xffff0000);
            bomb->secondaryWorkIndex++;
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(17), player->position.x);
            g_AnmManager->ExecuteScript(effect);
        }

        if (player->timerE2AC4 >= 5)
        {
            workItem->cancelSlot = player->FUN_0044de60(&player->position, 96.0f, 800.0f, 6, 0);
            workItem->cancelSlot = player->FUN_0044de60(&player->position, 800.0f, 96.0f, 6, 0);
        }

        if (bomb->timer.FUN_0040e350(239))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
            player->optionStates[1].state2C8 = 1;
            player->optionStates[1].timer = 0;
            player->optionStates[2].state2C8 = 1;
            player->optionStates[2].timer = 0;
            player->optionStates[3].state2C8 = 1;
            player->optionStates[3].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40f550
void __fastcall FUN_0040f550(Player *player)
{
    FUN_0040bc60(player, 0x80d02020);
}

// FUNCTION: th08 0x40f570
#pragma var_order(bomb, workItem)
void __fastcall FUN_0040f570(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 1,
                     "\x8D\x67\x96\x82\x81\x75\x83\x58\x83\x4A\x81\x5B\x83\x8C\x83\x62\x83\x67\x83\x66\x83\x72\x83\x8B\x81\x76",
                     280, 320, 1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem[0].anchor = player->optionStates[0].target;
        workItem[1].anchor = player->optionStates[1].target;
        workItem[2].anchor = player->optionStates[2].target;
        workItem[3].anchor = player->optionStates[3].target;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        bomb->secondaryWorkIndex = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.0f;
    }

    if (bomb->timer < 60)
    {
#pragma var_order(interp, position)
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        Float3 position;
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target =
            (position - workItem[0].anchor) * interp + workItem[0].anchor;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target =
            (position - workItem[1].anchor) * interp + workItem[1].anchor;
        position.y += 64.0f;
        player->optionStates[2].target =
            (position - workItem[2].anchor) * interp + workItem[2].anchor;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target =
            (position - workItem[3].anchor) * interp + workItem[3].anchor;
    }
    else
    {
        if (bomb->timer.FUN_0040e350(60))
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 3.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 3.0f;
            player->optionStates[0].vm.SetInterrupt(2);
            player->optionStates[1].vm.SetInterrupt(2);
            player->optionStates[2].vm.SetInterrupt(2);
            player->optionStates[3].vm.SetInterrupt(2);
        }

        Float3 position;
        workItem->cancelSlot = player->FUN_0044df00(&player->position, 96.0f, 0.0f, 0, 6);
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target = position;
        position.y += 64.0f;
        player->optionStates[2].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target = position;

        if (bomb->timer.FUN_0040ebc0(10))
        {
#pragma var_order(effect, position1, position0, scale1, scale0)
            AnmVm *effect = g_EffectManager.FUN_00425870(
                53, reinterpret_cast<D3DXVECTOR3 *>(&player->position), bomb->secondaryWorkIndex % 4 + 4, 1, -1);
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 32;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 4.0f;

            Float3 position0;
            Float3 position1;
            position0.x = 0.0f;
            position0.y = 0.0f;
            position0.z = 0.0f;
            position1.x = 192.0f;
            position1.y = g_Rng.GetRandomF32InRange(128.0f);
            position1.z = 0.0f;
            effect->FUN_0040ec30(30, 4, &position0, &position1);

            Float2 scale0;
            Float2 scale1;
            scale0.x = 64.0f;
            scale0.y = 0.0f;
            scale1.x = 128.0f;
            scale1.y = 0.0f;
            effect->FUN_0040eda0(30, 1, &scale0, &scale1);
            effect->FUN_0040ed50(30, 3, 255, 0);
            effect->FUN_0040eca0(30, 0, -1, 0xffff0000);
            bomb->secondaryWorkIndex++;
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(17), player->position.x);
            g_AnmManager->ExecuteScript(effect);
        }

        if (player->timerE2AC4 >= 5)
        {
            workItem->cancelSlot = player->FUN_0044de60(&player->position, 96.0f, 800.0f, 6, 0);
            workItem->cancelSlot = player->FUN_0044de60(&player->position, 800.0f, 96.0f, 6, 0);
        }

        if (bomb->timer.FUN_0040e350(279))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
            player->optionStates[1].state2C8 = 1;
            player->optionStates[1].timer = 0;
            player->optionStates[2].state2C8 = 1;
            player->optionStates[2].timer = 0;
            player->optionStates[3].state2C8 = 1;
            player->optionStates[3].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40fcb0
void __fastcall FUN_0040fcb0(Player *player)
{
    FUN_0040bc60(player, 0x80f00000);
}

// FUNCTION: th08 0x40fcd0
#pragma var_order(i, bomb, workItem, vm, angle)
void __fastcall FUN_0040fcd0(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    AnmVm *vm;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x8C\xB6\x95\x84\x81\x75\x8E\x45\x90\x6C\x83\x68\x81\x5B\x83\x8B\x81\x76",
                     250, 290, 0);
        workItem = bomb->workItems;
        for (i = 0; i < 96; i++, workItem++)
            workItem->active = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.5f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.5f;
        bomb->workItems[0].effect =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
    }

    if (bomb->timer >= 0 && bomb->timer <= 60)
    {
        reinterpret_cast<Effect *>(bomb->workItems[0].effect)->vector0 = player->position;
    }

    if (bomb->timer >= 20 && bomb->timer < 116)
    {
        workItem = player->bombState.workItems;
        for (i = 0; i < 96; i++, workItem++)
        {
            if (!player->bombState.timer.FUN_0040e350(2 * (i % 48) + 20))
                continue;
            if (workItem->active)
                return;
            workItem->active = 1;
                vm = &workItem->vms[0];
                player->anmFile->ExecuteAnmIdx(vm, 22);
                angle = (f32)i * ZUN_2PI / 96.0f - ZUN_PI;
                workItem->rotation = angle;
                workItem->speed = g_Rng.GetRandomF32InRange(1.0f) + 0.5f;
                workItem->rotationStep = g_Rng.GetRandomF32InRange(0.1f) + 0.03f;
                workItem->velocity.x = g_Rng.GetRandomU16InRange(1)
                    ? 0.15707963705062866f : -0.15707963705062866f;
                workItem->position.x = cosf(workItem->rotation) * 24.0f;
                workItem->position.y = sinf(workItem->rotation) * 24.0f;
                workItem->anchor = player->position + workItem->position;
                workItem->timer = 0;
                workItem->position.z = 0.0f;
                workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 32.0f, 0.0f, 500, 6);
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 32.0f, 0.0f, 20, 500);
        }
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 120, 4, 1, 0, 21);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (!workItem->active)
            continue;

        if ((i32)workItem->timer < 30 || (i32)workItem->timer >= 70)
        {
            if (workItem->timer.FUN_0040e350(70))
            {
                if (player->tailPosition0.x > -100.0f)
                {
                    workItem->rotation = AddNormalizeAngle(
                        VectorAngle(player->tailPosition0.y - workItem->anchor.y,
                                    player->tailPosition0.x - workItem->anchor.x),
                        0.0f);
                }
                workItem->speed = 14.0f;
            }
            workItem->speed += workItem->rotationStep;
            workItem->position.x = cosf(workItem->rotation) * workItem->speed;
            workItem->position.y = sinf(workItem->rotation) * workItem->speed;
        }
        else
        {
            workItem->rotation = AddNormalizeAngle(workItem->rotation, workItem->velocity.x);
            workItem->position.x = 0.0f;
            workItem->position.y = 0.0f;
        }

        if (workItem->damageSlot != NULL)
        {
            workItem->damageSlot->center.x = workItem->anchor.x;
            workItem->damageSlot->center.y = workItem->anchor.y;
            workItem->cancelSlot->center.x = workItem->anchor.x;
            workItem->cancelSlot->center.y = workItem->anchor.y;
            if (workItem->timer >= 120)
            {
                workItem->damageSlot->active = 0;
                workItem->cancelSlot->active = 0;
                workItem->cancelSlot = NULL;
                workItem->damageSlot = NULL;
            }
            else if (workItem->damageSlot->hitAccumulator > 0)
            {
                player->anmFile->ExecuteAnmIdx(&workItem->vms[0], 23);
                g_EffectManager.SpawnEffect(
                    0, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[i].anchor), 1, 0xffff80ff);
                workItem->damageSlot->active = 0;
                workItem->cancelSlot->active = 0;
                workItem->cancelSlot = NULL;
                workItem->damageSlot = NULL;
                g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(43), workItem->anchor.x);
            }
        }

        workItem->anchor += workItem->position;
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
        workItem->timer++;
    }
}

// FUNCTION: th08 0x4103f0
#pragma var_order(i, bomb, workItem, vm, angle)
void __fastcall FUN_004103f0(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    AnmVm *vm;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.FUN_0040d3d0() && bomb->timer == 0)
    {
        FUN_0040be30(player, 0,
                     "\x8C\xB6\x91\x92\x81\x75\x96\xE9\x96\xB6\x82\xCC\x8C\xB6\x89\x65\x8E\x45\x90\x6C\x8B\x53\x81\x76",
                     320, 350, 1);
        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
            workItem->active = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 0.5f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 0.5f;
        bomb->workItems[0].effect =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 50, 4, 1, 0, 21);
    }

    if (bomb->timer >= 0 && bomb->timer <= 60)
    {
        reinterpret_cast<Effect *>(bomb->workItems[0].effect)->vector0 = player->position;
    }

    if (bomb->timer >= 20 && bomb->timer < 148)
    {
        workItem = player->bombState.workItems;
        for (i = 0; i < 128; i++, workItem++)
        {
            if (!player->bombState.timer.FUN_0040e350(2 * (i % 64) + 20))
                continue;
            if (workItem->active)
                return;
            workItem->active = 1;
            vm = &workItem->vms[0];
            player->anmFile->ExecuteAnmIdx(vm, 20);
            angle = AddNormalizeAngle((f32)i * ZUN_2PI / 64.0f - ZUN_PI, 0.0f);
            workItem->rotation = angle;
            workItem->speed = g_Rng.GetRandomF32InRange(1.0f) + 0.5f;
            workItem->rotationStep = g_Rng.GetRandomF32InRange(0.1f) + 0.03f;
            workItem->velocity.x = g_Rng.GetRandomU16InRange(1)
                ? 0.15707963705062866f : -0.15707963705062866f;
            workItem->position.x = cosf(workItem->rotation) * 24.0f;
            workItem->position.y = sinf(workItem->rotation) * 24.0f;
            workItem->anchor = player->position + workItem->position;
            workItem->timer = 0;
            workItem->position.z = 0.0f;
            workItem->cancelSlot = player->FUN_0044df00(&workItem->anchor, 32.0f, 0.0f, 500, 6);
            workItem->damageSlot = player->FUN_0044e040(&workItem->anchor, 32.0f, 0.0f, 30, 500);
        }
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (!workItem->active)
            continue;

        if ((i32)workItem->timer < 30 || (i32)workItem->timer >= 70)
        {
            if (workItem->timer.FUN_0040e350(70))
            {
                if (player->tailPosition0.x > -100.0f)
                {
                    workItem->rotation = AddNormalizeAngle(
                        VectorAngle(player->tailPosition0.y - workItem->anchor.y,
                                    player->tailPosition0.x - workItem->anchor.x),
                        0.0f);
                }
                workItem->speed = 14.0f;
                g_EffectManager.SpawnEffect(
                    46, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor), 1, -1);
            }
            workItem->speed += workItem->rotationStep;
            workItem->position.x = cosf(workItem->rotation) * workItem->speed;
            workItem->position.y = sinf(workItem->rotation) * workItem->speed;
        }
        else
        {
            workItem->rotation = AddNormalizeAngle(workItem->rotation, workItem->velocity.x);
            workItem->position.x = 0.0f;
            workItem->position.y = 0.0f;
        }

        if (workItem->damageSlot != NULL)
        {
            workItem->damageSlot->center.x = workItem->anchor.x;
            workItem->damageSlot->center.y = workItem->anchor.y;
            workItem->cancelSlot->center.x = workItem->anchor.x;
            workItem->cancelSlot->center.y = workItem->anchor.y;
            if (workItem->timer >= 120)
            {
                workItem->damageSlot->active = 0;
                workItem->cancelSlot->active = 0;
                workItem->cancelSlot = NULL;
                workItem->damageSlot = NULL;
            }
            else if (workItem->damageSlot->hitAccumulator > 0)
            {
                if (i % 3 == 0)
                    ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 2, 1, 0x208080ff, 0, 21);
                g_EffectManager.SpawnEffect(
                    0, reinterpret_cast<D3DXVECTOR3 *>(&workItem->anchor), 1, 0xffff80ff);
                player->anmFile->ExecuteAnmIdx(&workItem->vms[0], 21);
                workItem->damageSlot->active = 0;
                workItem->cancelSlot->active = 0;
                workItem->cancelSlot = NULL;
                workItem->damageSlot = NULL;
                g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(43), workItem->anchor.x);
            }
        }

        workItem->anchor += workItem->position;
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
        workItem->timer++;
    }
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

// FUNCTION: th08 0x410bb0
i32 __fastcall FUN_00410bb0(AnmVm *effect)
{
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) += 8.0f;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 12;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 32.0f;
    return 1;
}

// FUNCTION: th08 0x413070
i32 __fastcall FUN_00413070(AnmVm *effect)
{
    f32 interp;
    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) < 30)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 192.0f;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 48;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 3.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = 0.0001f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = ZUN_PI / 2.0f;
    }
    else
    {
        interp = ((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effect) + 0x338) - 30.0f) / 30.0f;
        interp *= interp;
        interp *= interp;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = 192.0f * interp + 0.0001f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = 80.0f * interp + 3.0f;
    }
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    return 1;
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

} // namespace th08
