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

} // namespace th08
