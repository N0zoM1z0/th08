#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ScreenEffect.hpp"
#include "AnmManager.hpp"
#include "BulletManager.hpp"

namespace th08
{

struct EclExInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 unknown08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 operands[1];
};

DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);
extern i32 g_EclCallbackPublishedEnemyField56;
extern void *g_EclExUpdateCallback;

void __fastcall FUN_004235a0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);

// FUNCTION: th08 0x423390
void __fastcall FUN_00423390(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_EclCallbackPublishedEnemyField24 = *reinterpret_cast<i32 *>(
        reinterpret_cast<u8 *>(*reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x18);
    g_EclCallbackPublishedEnemyField56 = *reinterpret_cast<i32 *>(
        reinterpret_cast<u8 *>(*reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x38);
}

// FUNCTION: th08 0x4233d0
void __fastcall FUN_004233d0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 60, 1, -1, 0, 21);
}

// FUNCTION: th08 0x423530
void __fastcall FUN_00423530(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    AnmVm *effect;
    effect = g_EffectManager.FUN_00425870(56, reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2d34), 9, 1, -1);
    effect = g_EffectManager.FUN_00425870(56, reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2d34), 10, 1, -1);
    g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 97);
    g_EclExUpdateCallback = reinterpret_cast<void *>(&FUN_004235a0);
}

// FUNCTION: th08 0x423a60
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall FUN_00423a60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    i32 previousZone;
    u8 *bullet = reinterpret_cast<u8 *>(&g_BulletManager) + 0x1a880;
    i32 currentZone;
    Float3 previousPosition;

    for (i = 0; i < 0x600; ++i, bullet += 0x10b8)
    {
        if (*reinterpret_cast<u16 *>(bullet + 0xdb8) == 0)
            continue;
        if (*reinterpret_cast<i32 *>(bullet + 0xdc4) != 0)
        {
            --*reinterpret_cast<i32 *>(bullet + 0xdc4);
            continue;
        }

        previousPosition = *reinterpret_cast<Float3 *>(bullet + 0xd44) -
                           *reinterpret_cast<Float3 *>(bullet + 0xd50);

        if (*reinterpret_cast<f32 *>(bullet + 0xd44) > 124.11774444580078f &&
            *reinterpret_cast<f32 *>(bullet + 0xd44) < 259.88226318359375f &&
            *reinterpret_cast<f32 *>(bullet + 0xd48) > 140.11773681640625f &&
            *reinterpret_cast<f32 *>(bullet + 0xd48) < 275.88226318359375f)
            currentZone = 0;
        else if (*reinterpret_cast<f32 *>(bullet + 0xd44) > 56.23548889160156f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd44) < 327.7645263671875f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd48) > 72.23548889160156f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd48) < 343.7645263671875f)
            currentZone = 1;
        else
            currentZone = 2;

        if (previousPosition.x > 124.11774444580078f &&
            previousPosition.x < 259.88226318359375f &&
            previousPosition.y > 140.11773681640625f &&
            previousPosition.y < 275.88226318359375f)
            previousZone = 0;
        else if (previousPosition.x > 56.23548889160156f &&
                 previousPosition.x < 327.7645263671875f &&
                 previousPosition.y > 72.23548889160156f &&
                 previousPosition.y < 343.7645263671875f)
            previousZone = 1;
        else
            previousZone = 2;

        if (currentZone != previousZone)
        {
            *reinterpret_cast<i32 *>(bullet + 0xdc4) = 2;
            *reinterpret_cast<Float3 *>(bullet + 0xd50) *= -1.0f;
            if (currentZone == 0 || previousZone == 0)
            {
                *reinterpret_cast<f32 *>(bullet + 0xd44) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd44) - 192.0f) *
                        67.88225555419922f / 135.76451110839844f + 192.0f;
                *reinterpret_cast<f32 *>(bullet + 0xd48) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd48) - 208.0f) *
                        67.88225555419922f / 135.76451110839844f + 208.0f;
            }
            else
            {
                *reinterpret_cast<f32 *>(bullet + 0xd44) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd44) - 192.0f) *
                        135.76451110839844f / 67.88225555419922f + 192.0f;
                *reinterpret_cast<f32 *>(bullet + 0xd48) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd48) - 208.0f) *
                        135.76451110839844f / 67.88225555419922f + 208.0f;
            }
            *reinterpret_cast<f32 *>(bullet + 0xd74) =
                AddNormalizeAngle(*reinterpret_cast<f32 *>(bullet + 0xd74), ZUN_PI);
        }
    }
}

// FUNCTION: th08 0x423db0
void __fastcall FUN_00423db0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    AnmVm *effect;
    effect = g_EffectManager.FUN_00425870(65, reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2d34), 9, 1, -1);
    effect = g_EffectManager.FUN_00425870(65, reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2d34), 10, 1, -1);
    g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 99);
    g_EclExUpdateCallback = reinterpret_cast<void *>(&FUN_004235a0);
}

// FUNCTION: th08 0x424170
void __fastcall FUN_00424170(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    AnmVm *effect;
    effect = g_EffectManager.FUN_00425870(58, reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2d34), 9, 1, -1);
    effect = g_EffectManager.FUN_00425870(58, reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2d34), 10, 1, -1);
    g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(effect, 101);
    g_EclExUpdateCallback = reinterpret_cast<void *>(&FUN_004235a0);
}

} // namespace th08
