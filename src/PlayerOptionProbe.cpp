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

// Claim-safe exact owners for the in-progress Player TU.
i32 __fastcall FUN_0044ee70(Player *player, PlayerOptionState *option)
{
    Float3 desired;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 24);
        option->state2C8 = 2;
        option->target = player->position;
        switch (option->optionIndex)
        {
        case 0:
            option->target.x -= 30.0f;
            option->target.y -= 16.0f;
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->target.x -= 10.0f;
            option->target.y -= 32.0f;
            option->orbitAngle = ZUN_PI;
            break;
        case 2:
            option->target.x += 10.0f;
            option->target.y -= 32.0f;
            option->orbitAngle = 0.0f;
            break;
        case 3:
            option->target.x += 30.0f;
            option->target.y -= 16.0f;
            option->orbitAngle = ZUN_PI;
            break;
        default:
            break;
        }
        // Fall through into the orbit update.
    case 2:
        if (option->timer > 12)
        {
            switch (option->optionIndex)
            {
            case 0:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.02617993950843811f);
                break;
            case 1:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.03490658476948738f);
                break;
            case 2:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.03490658476948738f);
                break;
            case 3:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.02617993950843811f);
                break;
            default:
                break;
            }
        }

        option->vm.color1.d3dColor = 0xFFFF8080;
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        if (player->optionModeFlag == 0)
        {
            desired = player->position;
            option->vm.color1.d3dColor = 0xFF80FFFF;
            switch (option->optionIndex)
            {
            case 0:
                desired.x -= 30.0f;
                desired.y -= 16.0f;
                break;
            case 1:
                desired.x -= 10.0f;
                desired.y -= 32.0f;
                break;
            case 2:
                desired.x += 10.0f;
                desired.y -= 32.0f;
                break;
            case 3:
                desired.x += 30.0f;
                desired.y -= 16.0f;
                break;
            default:
                break;
            }
            option->target = (desired - option->target) * 0.2f + option->target;
        }
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80602050);
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

#pragma var_order(angleDifference, targetAngle)
i32 __fastcall FUN_0044f2d0(Player *player, PlayerOptionState *option)
{
    f32 targetAngle;
    f32 angleDifference;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = player->vectors2CC[15];
        option->orbitAngle = 0.0f;
        option->facingAngle = -ZUN_PI / 2.0f;
        // Fall through into the normal update.
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->target = (player->vectors2CC[15] - option->target) * 0.05f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80405080);

        switch (player->movementDirection)
        {
        case 0:
            goto optionUpdateDone;
        case 1:
            targetAngle = ZUN_PI / 2.0f;
            break;
        case 2:
            targetAngle = -ZUN_PI / 2.0f;
            break;
        case 3:
            targetAngle = 0.0f;
            break;
        case 4:
            targetAngle = ZUN_PI;
            break;
        case 5:
            targetAngle = ZUN_PI / 4.0f;
            break;
        case 6:
            targetAngle = 3.0f * ZUN_PI / 4.0f;
            break;
        case 7:
            targetAngle = -ZUN_PI / 4.0f;
            break;
        case 8:
            targetAngle = -3.0f * ZUN_PI / 4.0f;
            break;
        default:
            break;
        }

        angleDifference = fabsf(option->facingAngle - targetAngle);
        if (angleDifference > ZUN_PI)
        {
            targetAngle += option->facingAngle > targetAngle ? ZUN_2PI : -ZUN_2PI;
            angleDifference = fabsf(option->facingAngle - targetAngle);
        }
        if (angleDifference > ZUN_PI / 2.0f)
        {
            option->facingAngle = targetAngle;
        }
        else
        {
            option->facingAngle = AddNormalizeAngle(
                (targetAngle - option->facingAngle) * 0.07f, option->facingAngle);
        }
optionUpdateDone:
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}


#pragma var_order(angleDifference, targetAngle)
i32 __fastcall FUN_0044f5e0(Player *player, PlayerOptionState *option)
{
    f32 targetAngle;
    f32 angleDifference;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = player->vectors2CC[15];
        option->orbitAngle = 0.0f;
        option->facingAngle = -ZUN_PI / 2.0f;
        // Fall through into the normal update.
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->target = (player->vectors2CC[15] - option->target) * 0.05f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        option->vm.color1.d3dColor = 0xFFFF8080;

        if (player->optionModeFlag == 0)
        {
            option->vm.color1.d3dColor = 0xFFFFFFFF;
            switch (player->movementDirection)
            {
            case 0:
                goto optionUpdateDone;
            case 1:
                targetAngle = ZUN_PI / 2.0f;
                break;
            case 2:
                targetAngle = -ZUN_PI / 2.0f;
                break;
            case 3:
                targetAngle = 0.0f;
                break;
            case 4:
                targetAngle = ZUN_PI;
                break;
            case 5:
                targetAngle = ZUN_PI / 4.0f;
                break;
            case 6:
                targetAngle = 3.0f * ZUN_PI / 4.0f;
                break;
            case 7:
                targetAngle = -ZUN_PI / 4.0f;
                break;
            case 8:
                targetAngle = -3.0f * ZUN_PI / 4.0f;
                break;
            default:
                break;
            }

            angleDifference = fabsf(option->facingAngle - targetAngle);
            if (angleDifference > ZUN_PI)
            {
                targetAngle += option->facingAngle > targetAngle ? ZUN_2PI : -ZUN_2PI;
                angleDifference = fabsf(option->facingAngle - targetAngle);
            }
            if (angleDifference > ZUN_PI / 2.0f)
            {
                option->facingAngle = targetAngle;
            }
            else
            {
                option->facingAngle = AddNormalizeAngle(
                    (targetAngle - option->facingAngle) * 0.07f, option->facingAngle);
            }

            g_EffectManager.SpawnEffect(
                47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80405080);
        }
        else
        {
            g_EffectManager.SpawnEffect(
                47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0xFFF05080);
        }
optionUpdateDone:
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

i32 __fastcall FUN_0044f930(Player *player, PlayerOptionState *option)
{
    Float3 base = player->position;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = base;
        switch (option->optionIndex)
        {
        case 0:
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->orbitAngle = -ZUN_PI;
            break;
        default:
            break;
        }
        // Fall through.
    case 2:
        switch (option->optionIndex)
        {
        case 0:
            base.x -= 32.0f;
            option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
            break;
        case 1:
            base.x += 32.0f;
            option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.052359879016876221f);
            break;
        default:
            break;
        }

        option->position.FromAngleMagnitude(option->orbitAngle, 6.0f);
        option->target = (base - option->target) * 0.09f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80602050);
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}


#pragma var_order(magnitude, angle)
i32 __fastcall FUN_00450080(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        angle = AddNormalizeAngle(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0xE2B0C),
            *reinterpret_cast<f32 *>(entry + 0x14) + ZUN_PI / 2.0f);
        magnitude = *reinterpret_cast<f32 *>(entry + 0x18);
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = angle;
        return 1;
    }
    return 0;
}

#pragma var_order(magnitude, angle)
i32 __fastcall FUN_00450110(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (player->bombState.frameStop == 0 &&
        value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        angle = AddNormalizeAngle(player->optionStates[2].facingAngle, *reinterpret_cast<f32 *>(entry + 0x14));
        magnitude = *reinterpret_cast<f32 *>(entry + 0x18);
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = angle;
        return 1;
    }
    return 0;
}

i32 __fastcall FUN_004501b0(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) =
            g_Rng.GetRandomF32() * ZUN_PI / 48.0f - ZUN_PI / 2.0f;
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450),
            *reinterpret_cast<f32 *>(entry + 0x18));
        return 1;
    }
    return 0;
}

#pragma var_order(magnitude, angle)
i32 __fastcall FUN_00450240(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        if (player->optionHomingTarget != NULL)
        {
            angle = AddNormalizeAngle(
                VectorAngle(player->optionHomingTarget->vector2d34.y - slot->position.y,
                            player->optionHomingTarget->vector2d34.x - slot->position.x),
                *reinterpret_cast<f32 *>(entry + 0x14) + ZUN_PI / 2.0f);
            magnitude = *reinterpret_cast<f32 *>(entry + 0x18) * 1.5f;
            reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = angle;
        }
        return 1;
    }
    return 0;
}


#pragma var_order(yDelta, xDelta, magnitude)
i32 __fastcall FUN_00450320(Player *player, PlayerShot *slot)
{
    f32 xDelta;
    f32 yDelta;
    f32 magnitude;
    if (slot->state == 1)
    {
        if (player->tailPosition0.x > -100.0f && (i32)slot->timer < 40 && slot->timer.FUN_0040d3d0())
        {
            xDelta = player->tailPosition0.x - slot->position.operator float *()[0];
            yDelta = player->tailPosition0.y - slot->position.operator float *()[1];
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta) / (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / 4.0f);
            if (magnitude < 1.0f) magnitude = 1.0f;
            xDelta = xDelta / magnitude + slot->velocity.x;
            yDelta = yDelta / magnitude + slot->velocity.y;
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta);
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) = magnitude > 10.0f ? 10.0f : magnitude;
            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) < 1.0f) *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) = 1.0f;
            slot->velocity.x = xDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
            slot->velocity.y = yDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
        }
        else if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) < 10.0f)
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) += 1.0f / 3.0f;
            xDelta = slot->velocity.x;
            yDelta = slot->velocity.y;
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta);
            slot->velocity.x = xDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
            slot->velocity.y = yDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
        }
    }
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = VectorAngle(slot->velocity.y, slot->velocity.x);
    return 0;
}


i32 __fastcall FUN_00450580(Player *player, PlayerShot *slot)
{
    if (slot->state == 1)
        slot->velocity.y -= g_Rng.GetRandomF32InRange(0.1f) + 0.27f;
    return 0;
}


i32 __fastcall FUN_004505d0(Player *player, PlayerShot *slot)
{
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction !=
        reinterpret_cast<EclTimelineInstruction *>(slot))
    {
        if (slot->vm.FUN_004396f8()) slot->vm.pendingInterrupt = 1;
    }
    if (g_Gui.IsDialogPresent() || player->bombState.frameStop != 0 || g_GameManager.flags.unk13)
    {
        if ((i32)player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer > 20)
            player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer = 20;
    }
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer <= 0)
    {
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer = 0;
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction = NULL;
        slot->state = 0;
        return 1;
    }
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer <= 70)
    {
        if (slot->vm.FUN_004396f8()) slot->vm.pendingInterrupt = 1;
    }
    slot->position.x += *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x444);
    slot->position.z = 0.44f;
    if (player->playerState == PLAYER_STATE_DYING) return 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x1C) = slot->position.y / 14.0f;
    slot->hitboxSize.y = slot->position.y;
    slot->position.y /= 2.0f;
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer < 100)
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer--;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xD0; slot->vm.color1.b = 0xB0;
    }
    else
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xFF; slot->vm.color1.b = 0xFF;
    }
    return 0;
}


#pragma var_order(damageSlot, i)
i32 __fastcall FUN_00450840(Player *player, PlayerShot *slot)
{
    PlayerUnkStruct0x40 *damageSlot;
    i32 i;
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction !=
            reinterpret_cast<EclTimelineInstruction *>(slot) ||
        g_Gui.IsDialogPresent() ||
        (i32)player->timerE2AC4 < 0 ||
        player->playerState == PLAYER_STATE_DYING ||
        player->bombState.frameStop != 0 ||
        g_GameManager.flags.unk13)
    {
        slot->vm.pendingInterrupt = 1;
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction = NULL;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(slot) + 0x474) = 0;
    }
    if (player->optionStates[0].state2C8 == 0)
    {
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction = NULL;
        return 1;
    }
    for (i = 0; i < *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x46A); i++)
    {
        if (slot->vectors[i * 2].x >= -900.0f)
        {
            damageSlot = player->FUN_0044dfa0(&slot->vectors[i * 2], 16.0f, 448.0f, 1, 0);
            reinterpret_cast<u8 *>(damageSlot)[0x3D] = 1;
        }
    }
    for (i = 31; i > 0; i--)
    {
        slot->vectors[i] = slot->vectors[i - 1];
        slot->vectors[i].y -= 1.0f;
    }
    slot->vectors[0] = slot->position;
    slot->position = player->optionStates[0].position;
    slot->position.z = 0.44f;
    slot->hitboxSize.y = 448.0f;
    slot->position.y -= 208.0f;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xD0; slot->vm.color1.b = 0xB0;
    }
    else
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xFF; slot->vm.color1.b = 0xFF;
    }
    return 0;
}


#pragma var_order(color, i, originalColor)
i32 __fastcall FUN_00450ad0(Player *player, PlayerShot *slot)
{
    i32 color;
    i32 i;
    i32 originalColor;

    color = slot->vm.color1.a;
    originalColor = color;
    color = color * 3 / 4;
    for (i = 0; i < *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x46A) * 2; i += 2)
    {
        if (slot->vectors[i].x == -999.0f)
            break;
        slot->vm.pos.x = slot->vectors[i].x;
        slot->vm.pos.y = slot->vectors[i].y;
        slot->vm.pos.z = slot->vectors[i].z;
        if (i != 0)
            slot->vm.color1.a = color - ((color / 2) * i) / *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x46A);
        slot->vm.pos.x += g_ItemAnmManagerScreenShakeOffset.x;
        slot->vm.pos.y += g_ItemAnmManagerScreenShakeOffset.y;
        if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            slot->vm.color1.r = 0xFF;
            slot->vm.color1.g = 0x40;
            slot->vm.color1.b = 0x40;
        }
        g_AnmManager->Draw2D(&slot->vm);
    }
    slot->vm.color1.a = originalColor;
    return 0;
}


i32 __fastcall FUN_00450c50(Player *player, PlayerShot *slot, Float3 *effectPosition)
{
    f32 angle;

    if (slot->state == 2)
    {
        if ((i32)slot->timer % 2 != 0)
            return 1;
        if (g_Spellcard.IsActive() && (i32)slot->timer % 4 != 0)
            return 1;
        slot->damage /= 3;
        if (slot->damage == 0)
            slot->damage = 1;
        slot->velocity.x *= 0.88f;
        slot->velocity.y *= 0.88f;
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(ZUN_PI / 2.0f) - 3.0f * ZUN_PI / 4.0f;
        switch (slot->vm.scriptIndex)
        {
        case 12: slot->hitboxSize.x = 48.0f; slot->hitboxSize.y = 48.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 14: slot->hitboxSize.x = 64.0f; slot->hitboxSize.y = 64.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 16: slot->hitboxSize.x = 80.0f; slot->hitboxSize.y = 80.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 18: slot->hitboxSize.x = 96.0f; slot->hitboxSize.y = 96.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 20: slot->hitboxSize.x = 128.0f; slot->hitboxSize.y = 128.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        default: break;
        }
    }
    if ((i32)slot->timer % 6 == 0)
        g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(effectPosition), 1, -1);
    return 0;
}


i32 __fastcall FUN_00450ee0(Player *player, PlayerShot *slot, Float3 *effectPosition)
{
    (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 0xE2A94))++;
    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 0xE2A94) % 8 == 0)
    {
        Float3 position;
        position = *effectPosition;
        position.x = slot->position.x;
        g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, -1);
    }
    return 0;
}

} // namespace th08
