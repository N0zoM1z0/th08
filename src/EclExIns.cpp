#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ScreenEffect.hpp"
#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "ItemManager.hpp"
#include "Background.hpp"

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

namespace EclExIns
{
void __fastcall ReisenFreezeBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall MokouResurrection(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
}

DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);
extern i32 g_EclCallbackPublishedEnemyField56;

// The retail image gives these ECL extension views their own target symbols.
// The modern port stores the same bytes in Background, so keep one semantic
// spelling in the instruction bodies while preserving both storage models.
#ifdef TH08_MODERN_PORT
#define g_EclExUpdateCallback (g_Background.EclExUpdateCallback())
#define g_EclExBarrierRenderState (g_Background.EclExBarrierState())
#endif

void __fastcall FUN_004235a0();

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

// FUNCTION: th08 0x423400
void __fastcall FUN_00423400(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 changed;

    changed = 0;
    if (*reinterpret_cast<f32 *>(enemy->bytes + 0x2d34) <= 0.0f ||
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d34) >= 384.0f)
    {
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d4c) =
            -*reinterpret_cast<f32 *>(enemy->bytes + 0x2d4c);
        changed = 1;
    }

    if (*reinterpret_cast<f32 *>(enemy->bytes + 0x2d50) <
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
            *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x54))
    {
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d50) +=
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x50);
        changed = 1;
    }

    if (*reinterpret_cast<f32 *>(enemy->bytes + 0x2d38) < -64.0f)
    {
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d50) =
            -*reinterpret_cast<f32 *>(enemy->bytes + 0x2d50);
        changed = 1;
    }
    else if (*reinterpret_cast<f32 *>(enemy->bytes + 0x2d38) >= 480.0f)
    {
        *reinterpret_cast<u32 *>(enemy->bytes + 0x3324) &= ~0x10000000U;
    }

    if (changed)
    {
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d94) =
            VectorAngle(*reinterpret_cast<f32 *>(enemy->bytes + 0x2d50),
                        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d4c));
    }
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

// FUNCTION: th08 0x4235a0
#pragma var_order(effect9, savedColor, i, radius9, unusedVm, effect10, radius10, vertices)
void __fastcall FUN_004235a0()
{
    VertexDiffuseXyzrhw vertices[10];
    Effect *effect9;
    i32 savedColor;
    i32 i;
    f32 radius9;
    AnmVm *unusedVm;
    Effect *effect10;
    f32 radius10;

    effect9 = reinterpret_cast<Effect *>(g_EffectManager.FUN_004253e0(9));
    effect10 = reinterpret_cast<Effect *>(g_EffectManager.FUN_004253e0(10));
    unusedVm = &g_EclExBarrierRenderState.vm0;

    radius9 = effect9->vm.pos.x * 0.7071068286895752f;
    radius10 = effect10->vm.pos.x * 0.7071068286895752f;

    vertices[0].pos.x = 32.0f + effect9->vector0.x - radius9;
    vertices[0].pos.y = 16.0f + effect9->vector0.y - radius9;
    vertices[1].pos.x = 32.0f + effect9->vector0.x - radius10;
    vertices[1].pos.y = 16.0f + effect9->vector0.y - radius10;
    vertices[2].pos.x = 32.0f + effect9->vector0.x + radius9;
    vertices[2].pos.y = 16.0f + effect9->vector0.y - radius9;
    vertices[3].pos.x = 32.0f + effect9->vector0.x + radius10;
    vertices[3].pos.y = 16.0f + effect9->vector0.y - radius10;
    vertices[4].pos.x = 32.0f + effect9->vector0.x + radius9;
    vertices[4].pos.y = 16.0f + effect9->vector0.y + radius9;
    vertices[5].pos.x = 32.0f + effect9->vector0.x + radius10;
    vertices[5].pos.y = 16.0f + effect9->vector0.y + radius10;
    vertices[6].pos.x = 32.0f + effect9->vector0.x - radius9;
    vertices[6].pos.y = 16.0f + effect9->vector0.y + radius9;
    vertices[7].pos.x = 32.0f + effect9->vector0.x - radius10;
    vertices[7].pos.y = 16.0f + effect9->vector0.y + radius10;
    vertices[8].pos = vertices[0].pos;
    vertices[9].pos = vertices[1].pos;

    for (i = 0; i < 10; ++i)
    {
        vertices[i].pos.z = 0.8f;
        vertices[i].w = 1.0f;
        vertices[i].diffuse = 0xff000000;
    }

    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    g_Supervisor.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 8, vertices, sizeof(VertexDiffuseXyzrhw));
    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    g_EclExBarrierRenderState.vm0.scale.x = -1.5f;
    g_EclExBarrierRenderState.vm0.scale.y = -1.75f;
    g_EclExBarrierRenderState.vm0.pos.z = 0.7f;
    g_EclExBarrierRenderState.vm0.pos.x = 416.0f;
    g_EclExBarrierRenderState.vm0.pos.y = 464.0f;
    savedColor = g_EclExBarrierRenderState.vm0.color1.d3dColor;
    g_EclExBarrierRenderState.vm0.color1.d3dColor = 0xffe0c0c0;
    g_AnmManager->Draw2D(&g_EclExBarrierRenderState.vm0);
    g_EclExBarrierRenderState.vm0.scale.x = 1.5f;
    g_EclExBarrierRenderState.vm0.scale.y = 1.75f;
    g_EclExBarrierRenderState.vm0.pos.z = 0.5f;
    g_EclExBarrierRenderState.vm0.pos.x = 32.0f;
    g_EclExBarrierRenderState.vm0.pos.y = 16.0f;
    g_EclExBarrierRenderState.vm0.color1.d3dColor = savedColor;

    g_EclExBarrierRenderState.vm1.rotation.z *= -1.0f;
    g_EclExBarrierRenderState.vm1.pos.z = 0.6f;
    savedColor = g_EclExBarrierRenderState.vm1.color1.d3dColor;
    g_EclExBarrierRenderState.vm1.color1.d3dColor = 0xffe0c0c0;
    g_AnmManager->Draw2DAndFlush(&g_EclExBarrierRenderState.vm1);
    g_EclExBarrierRenderState.vm1.rotation.z *= -1.0f;
    g_EclExBarrierRenderState.vm1.pos.z = 0.5f;
    g_EclExBarrierRenderState.vm1.color1.d3dColor = savedColor;
    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
}

// FUNCTION: th08 0x423a60
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall FUN_00423a60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    i32 previousZone;
    Bullet *bullet = &g_BulletManager.bullets[0];
    i32 currentZone;
    Float3 previousPosition;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if (bullet->zoneTransitionCooldownFrames != 0)
        {
            --bullet->zoneTransitionCooldownFrames;
            continue;
        }

        previousPosition = bullet->position - bullet->velocity;

        if (bullet->position.x > 124.11774444580078f &&
            bullet->position.x < 259.88226318359375f &&
            bullet->position.y > 140.11773681640625f &&
            bullet->position.y < 275.88226318359375f)
            currentZone = 0;
        else if (bullet->position.x > 56.23548889160156f &&
                 bullet->position.x < 327.7645263671875f &&
                 bullet->position.y > 72.23548889160156f &&
                 bullet->position.y < 343.7645263671875f)
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
            bullet->zoneTransitionCooldownFrames = 2;
            bullet->velocity *= -1.0f;
            if (currentZone == 0 || previousZone == 0)
            {
                bullet->position.x =
                    (bullet->position.x - 192.0f) *
                        67.88225555419922f / 135.76451110839844f + 192.0f;
                bullet->position.y =
                    (bullet->position.y - 208.0f) *
                        67.88225555419922f / 135.76451110839844f + 208.0f;
            }
            else
            {
                bullet->position.x =
                    (bullet->position.x - 192.0f) *
                        135.76451110839844f / 67.88225555419922f + 192.0f;
                bullet->position.y =
                    (bullet->position.y - 208.0f) *
                        135.76451110839844f / 67.88225555419922f + 208.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, ZUN_PI);
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



// FUNCTION: th08 0x423e20
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall FUN_00423e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    i32 previousZone;
    Bullet *bullet = &g_BulletManager.bullets[0];
    i32 currentZone;
    Float3 previousPosition;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if (bullet->zoneTransitionCooldownFrames != 0)
        {
            --bullet->zoneTransitionCooldownFrames;
            continue;
        }

        previousPosition = bullet->position - bullet->velocity;

        if (bullet->position.x > 112.80403137207031f &&
            bullet->position.x < 271.1959533691406f &&
            bullet->position.y > 128.8040313720703f &&
            bullet->position.y < 287.1959533691406f)
            currentZone = 0;
        else if (bullet->position.x > 33.608070373535156f &&
                 bullet->position.x < 350.3919372558594f &&
                 bullet->position.y > 49.608070373535156f &&
                 bullet->position.y < 366.3919372558594f)
            currentZone = 1;
        else
            currentZone = 2;

        if (previousPosition.x > 112.80403137207031f &&
            previousPosition.x < 271.1959533691406f &&
            previousPosition.y > 128.8040313720703f &&
            previousPosition.y < 287.1959533691406f)
            previousZone = 0;
        else if (previousPosition.x > 33.608070373535156f &&
                 previousPosition.x < 350.3919372558594f &&
                 previousPosition.y > 49.608070373535156f &&
                 previousPosition.y < 366.3919372558594f)
            previousZone = 1;
        else
            previousZone = 2;

        if (currentZone != previousZone)
        {
            bullet->zoneTransitionCooldownFrames = 2;
            bullet->velocity *= -1.0f;
            if (currentZone == 0 || previousZone == 0)
            {
                bullet->position.x =
                    (bullet->position.x - 192.0f) *
                        158.39193725585938f / 79.19596862792969f + 192.0f;
                bullet->position.y =
                    (bullet->position.y - 208.0f) *
                        158.39193725585938f / 79.19596862792969f + 208.0f;
            }
            else
            {
                bullet->position.x =
                    (bullet->position.x - 192.0f) *
                        79.19596862792969f / 158.39193725585938f + 192.0f;
                bullet->position.y =
                    (bullet->position.y - 208.0f) *
                        79.19596862792969f / 158.39193725585938f + 208.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, ZUN_PI);
        }
    }
}

// FUNCTION: th08 0x424130
void __fastcall FUN_00424130(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(g_EffectManager.FUN_004253e0(9)) + 0x350) = 0;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(g_EffectManager.FUN_004253e0(10)) + 0x350) = 0;
    g_EclExBarrierRenderState.mode = 2;
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


// FUNCTION: th08 0x4241e0
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall FUN_004241e0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    i32 previousZone;
    Bullet *bullet = &g_BulletManager.bullets[0];
    i32 currentZone;
    Float3 previousPosition;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if (bullet->zoneTransitionCooldownFrames != 0)
        {
            --bullet->zoneTransitionCooldownFrames;
            continue;
        }

        previousPosition = bullet->position - bullet->velocity;

        if (bullet->position.x > 56.23548889160156f &&
            bullet->position.x < 327.7645263671875f &&
            bullet->position.y > 88.23548889160156f &&
            bullet->position.y < 359.7645263671875f)
            currentZone = 0;
        else if (bullet->position.x > -32.0f &&
                 bullet->position.x < 416.0f &&
                 bullet->position.y > 0.0f &&
                 bullet->position.y < 448.0f)
            currentZone = 1;
        else
            currentZone = 2;

        if (previousPosition.x > 56.23548889160156f &&
            previousPosition.x < 327.7645263671875f &&
            previousPosition.y > 88.23548889160156f &&
            previousPosition.y < 359.7645263671875f)
            previousZone = 0;
        else if (previousPosition.x > -31.100006103515625f &&
                 previousPosition.x < 416.0f &&
                 previousPosition.y > 0.0f &&
                 previousPosition.y < 448.0f)
            previousZone = 1;
        else
            previousZone = 2;

        if (currentZone != previousZone)
        {
            bullet->zoneTransitionCooldownFrames = 2;
            bullet->velocity *= -1.0f;
            if (currentZone == 0 || previousZone == 0)
            {
                bullet->position.x =
                    (bullet->position.x - 192.0f) *
                        224.0f / 135.76451110839844f + 192.0f;
                bullet->position.y =
                    (bullet->position.y - 224.0f) *
                        224.0f / 135.76451110839844f + 224.0f;
            }
            else
            {
                bullet->position.x =
                    (bullet->position.x - 192.0f) *
                        135.76451110839844f / 224.0f + 192.0f;
                bullet->position.y =
                    (bullet->position.y - 224.0f) *
                        135.76451110839844f / 224.0f + 224.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, ZUN_PI);
        }
    }
}




// FUNCTION: th08 0x4244f0
#pragma var_order(count, groupId, firstChild, delta, targetAngle, cursor, enemy, instruction)
void __fastcall FUN_004244f0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 count;
    i32 groupId;
    EclOperands::EnemyOverlay *firstChild;
    f32 delta;
    f32 targetAngle;
    EclOperands::EnemyOverlay *cursor;

    groupId = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x60);
    cursor = *reinterpret_cast<EclOperands::EnemyOverlay **>(enemy->bytes + 0x2da4);
    if (cursor == NULL)
        return;

    count = 0;
    while (*reinterpret_cast<EclOperands::EnemyOverlay **>(cursor->bytes + 0x8) != NULL)
    {
        cursor = *reinterpret_cast<EclOperands::EnemyOverlay **>(cursor->bytes + 0x8);
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(cursor->bytes + 0x2ca0)) + 0x60) == groupId)
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(cursor->bytes + 0x2ca0)) + 0x5c) = count;
            if (count == 0)
                firstChild = cursor;
            ++count;
        }
    }

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x2c) = 0;
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
            *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x30) != count)
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x30) != 0)
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x2c) = 1;
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
            *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x30) = count;
    }

    groupId = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x5c);
    ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x34);
    if (groupId != 0)
    {
        targetAngle = AddNormalizeAngle(
            *reinterpret_cast<f32 *>(firstChild->bytes + 0x2d9c),
            static_cast<f32>(groupId) * 6.2831854820251465f / static_cast<f32>(count));
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(firstChild->bytes + 0x2ca0)) + 0x34) !=
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x34))
        {
            targetAngle = AddNormalizeAngle(
                targetAngle, *reinterpret_cast<f32 *>(firstChild->bytes + 0x2da0));
        }

        delta = AddNormalizeAngle(*reinterpret_cast<f32 *>(enemy->bytes + 0x2d9c),
                                  *reinterpret_cast<f32 *>(enemy->bytes + 0x2da0));
        delta = targetAngle - delta;
        if (fabsf(delta) > ZUN_PI)
        {
            delta = delta > 0.0f ? -6.2831854820251465f + delta : 6.2831854820251465f + delta;
        }
        delta *= 0.02f;
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d9c) =
            AddNormalizeAngle(*reinterpret_cast<f32 *>(enemy->bytes + 0x2d9c), delta);
    }
}





// FUNCTION: th08 0x4246e0
void __fastcall FUN_004246e0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(static_cast<ScreenEffectType>(3), 30, 5, 0x40ffffff, 0, 21);
    ScreenEffect::RegisterChain(static_cast<ScreenEffectType>(7), 4, 120, 190, 60, 21);
}

// FUNCTION: th08 0x424a00
void __fastcall FUN_00424a00(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_ScreenEffectCounter = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(instruction) + 0x10);
}

// FUNCTION: th08 0x424730
#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)
void __fastcall FUN_00424730(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    Float3 origin(
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d88) -
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x38),
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d8c) -
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x3c),
        0.0f);
    Float3 outerSize(590.0f, 160.0f, 0.0f);
    Float3 innerSize(590.0f, 128.0f, 0.0f);
    Float3 position(outerSize.x / 2.0f + origin.x, origin.y, 0.0f);

    if (reinterpret_cast<ZunTimer *>(enemy->bytes + 0x2e14)->FUN_0040ebc0(12))
    {
        g_Player.CalcLaserHitbox(&position, &innerSize, &origin,
                                 *reinterpret_cast<f32 *>(enemy->bytes + 0x14), 1);
    }
    g_Player.CalcLaserHitbox(&position, &outerSize, &origin,
                             *reinterpret_cast<f32 *>(enemy->bytes + 0x14), 0);
}



// FUNCTION: th08 0x424820
#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)
void __fastcall FUN_00424820(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    Float3 origin(
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d34) -
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x38),
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d38) -
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x3c),
        0.0f);
    Float3 outerSize(590.0f, 240.0f, 0.0f);
    Float3 innerSize(590.0f, 192.0f, 0.0f);
    Float3 position(outerSize.x / 2.0f + origin.x, origin.y, 0.0f);

    if (reinterpret_cast<ZunTimer *>(enemy->bytes + 0x2e14)->FUN_0040ebc0(12))
    {
        g_Player.CalcLaserHitbox(&position, &innerSize, &origin,
                                 *reinterpret_cast<f32 *>(enemy->bytes + 0x14), 1);
    }
    g_Player.CalcLaserHitbox(&position, &outerSize, &origin,
                             *reinterpret_cast<f32 *>(enemy->bytes + 0x14), 0);
}


// FUNCTION: th08 0x424910
#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)
void __fastcall FUN_00424910(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    Float3 origin(
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d88) -
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x38),
        *reinterpret_cast<f32 *>(enemy->bytes + 0x2d8c) -
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x3c),
        0.0f);
    Float3 outerSize(590.0f, 288.0f, 0.0f);
    Float3 innerSize(590.0f, 224.0f, 0.0f);
    Float3 position(outerSize.x / 2.0f + origin.x, origin.y, 0.0f);

    if (reinterpret_cast<ZunTimer *>(enemy->bytes + 0x2e14)->FUN_0040ebc0(12))
    {
        g_Player.CalcLaserHitbox(&position, &innerSize, &origin,
                                 *reinterpret_cast<f32 *>(enemy->bytes + 0x14), 1);
    }
    g_Player.CalcLaserHitbox(&position, &outerSize, &origin,
                             *reinterpret_cast<f32 *>(enemy->bytes + 0x14), 0);
}

// FUNCTION: th08 0x424a20
#pragma var_order(i, bullet, setCursor, clearCursor, enemy, instruction)
void __fastcall EclExIns::ReisenFreezeBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];
    EclOperands::EnemyOverlay *setCursor;
    EclOperands::EnemyOverlay *clearCursor;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags &
             *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(
                 *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x18)) != 0)
        {
        if (bullet->sprites.bulletVm.type == 1)
        {
            bullet->sprites.bulletVm.type = 0;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f8) =
                (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f8) & 0xffffffcfU) | 0x10U;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex + 16);
            bullet->collisionDisabled = 1;
            bullet->velocity.FromAngleMagnitude(
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x38),
                g_EclGameTimeScale *
                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x3c));
        }
        else
        {
            bullet->sprites.bulletVm.type = 1;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f8) &= 0xffffffcfU;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex - 16);
            bullet->collisionDisabled = 0;
            bullet->velocity.FromAngleMagnitude(
                bullet->angle,
                g_EclGameTimeScale * bullet->speed);
        }
        }
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
            *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x1c) == 0)
    {
        setCursor = enemy;
        while (*reinterpret_cast<EclOperands::EnemyOverlay **>(setCursor->bytes + 0x8) != NULL)
        {
            setCursor = *reinterpret_cast<EclOperands::EnemyOverlay **>(setCursor->bytes + 0x8);
            *reinterpret_cast<u32 *>(setCursor->bytes + 0x3328) |= 0x80U;
        }
        g_EclExBarrierRenderState.vm0.SetInterrupt(2);
        g_EclExBarrierRenderState.vm1.SetInterrupt(2);
    }
    else
    {
        clearCursor = enemy;
        while (*reinterpret_cast<EclOperands::EnemyOverlay **>(clearCursor->bytes + 0x8) != NULL)
        {
            clearCursor = *reinterpret_cast<EclOperands::EnemyOverlay **>(clearCursor->bytes + 0x8);
            *reinterpret_cast<u32 *>(clearCursor->bytes + 0x3328) &= ~0x80U;
        }
        g_EclExBarrierRenderState.vm0.SetInterrupt(1);
        g_EclExBarrierRenderState.vm1.SetInterrupt(1);
    }
}

// FUNCTION: th08 0x424c40
#pragma var_order(i, bullet, enemy, instruction)
void __fastcall FUN_00424c40(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags &
             *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(
                 *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x18)) != 0)
        {
        if (bullet->sprites.bulletVm.type == 1)
        {
            bullet->sprites.bulletVm.type = 0;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f8) =
                (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f8) & 0xffffffcfU) | 0x10U;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f3) = 0;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex + 16);
            bullet->collisionDisabled = 1;
            bullet->velocity.FromAngleMagnitude(
                bullet->angle,
                g_EclGameTimeScale *
                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x3c));
        }
        else if (bullet->sprites.bulletVm.type == 0)
        {
            bullet->sprites.bulletVm.type = 2;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f3) = 0;
            bullet->sprites.bulletVm.FUN_0040ed50(15, 0, 0, 255);
        }
        else
        {
            bullet->sprites.bulletVm.type = 1;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bullet->sprites.bulletVm) + 0x1f8) &= 0xffffffcfU;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex - 16);
            bullet->collisionDisabled = 0;
            bullet->velocity.FromAngleMagnitude(
                bullet->angle,
                g_EclGameTimeScale * bullet->speed);
        }
        }
    }
}


// FUNCTION: th08 0x424e00
void __fastcall FUN_00424e00(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_Background.FUN_00409160(0xffc03030U);
}

// FUNCTION: th08 0x424e20
void __fastcall FUN_00424e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(static_cast<ScreenEffectType>(7), 16, 20, 20, 20, 21);
}



// FUNCTION: th08 0x424e50
#pragma var_order(i, bullet, child, delta, enemy, instruction)
void __fastcall FUN_00424e50(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];
    EclOperands::EnemyOverlay *child;
    Float3 delta;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags & 0x100000U) != 0)
        {
        child = *reinterpret_cast<EclOperands::EnemyOverlay **>(enemy->bytes + 0x8);
        while (child != NULL)
        {
            if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                    *reinterpret_cast<EnemyEclContext **>(child->bytes + 0x2ca0)) + 0x60) == 0)
            {
                delta = bullet->position -
                        *reinterpret_cast<Float3 *>(child->bytes + 0x2d34);
                if (D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&delta)) < 4096.0f)
                {
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                        *reinterpret_cast<EnemyEclContext **>(child->bytes + 0x2ca0)) + 0x60) = 60;
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                        *reinterpret_cast<EnemyEclContext **>(child->bytes + 0x2ca0)) + 0x34) =
                        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                            *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x34);
                }
            }
            child = *reinterpret_cast<EclOperands::EnemyOverlay **>(child->bytes + 0x8);
        }
        }
    }
}

// FUNCTION: th08 0x424f60
void __fastcall FUN_00424f60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(static_cast<ScreenEffectType>(3), 180, 1, -1, 0, 21);
}

// FUNCTION: th08 0x424f90
#pragma var_order(value, scale, enemy, instruction)
void __fastcall FUN_00424f90(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 value;
    f32 scale;

    value = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(instruction) + 0x10);
    scale = 1.0f / static_cast<f32>(value);
    g_EclGameTimeScale = scale;
}

// FUNCTION: th08 0x424fc0
void __fastcall FUN_00424fc0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x18) =
        static_cast<i32>(g_GameManager.currentSpellCardNumber);
}

// FUNCTION: th08 0x424ff0
void __fastcall EclExIns::MokouResurrection(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_Spellcard.CutInEnemyNoPortrait(
        "\x81\x75\x83\x8a\x83\x55\x83\x8c\x83\x4e\x83\x56\x83\x87\x83\x93\x81\x76",
        reinterpret_cast<i32>(enemy));
}

// FUNCTION: th08 0x425020
void __fastcall FUN_00425020(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_Spellcard.spellcard_fun_00416160();
}

// FUNCTION: th08 0x425040
void __fastcall FUN_00425040(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
        *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x18) =
        g_GameManager.globals->spellcardsCaptured;
}

// FUNCTION: th08 0x425070
void __fastcall FUN_00425070(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_EclScriptedGlobalUpdateFreeze =
        *reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(instruction) + 0x10);
    if (g_EclScriptedGlobalUpdateFreeze)
    {
        g_EclExBarrierRenderState.vm0.SetInterrupt(2);
        g_EclExBarrierRenderState.vm1.SetInterrupt(2);
    }
    else
    {
        g_EclExBarrierRenderState.vm0.SetInterrupt(1);
        g_EclExBarrierRenderState.vm1.SetInterrupt(1);
    }
}


// FUNCTION: th08 0x4250d0
#pragma var_order(i, bullet, unusedVector, enemy, instruction)
void __fastcall FUN_004250d0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];
    Float3 unusedVector;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags & 0x100000U) != 0)
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x38) =
                bullet->angle;
            g_EnemyManager.SpawnEnemy2(
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                    *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x60),
                reinterpret_cast<D3DXVECTOR3 *>(&bullet->position), 800, -2, 10,
                reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(
                    *reinterpret_cast<EnemyEclContext **>(enemy->bytes + 0x2ca0)) + 0x18));
            bullet->transformFlags &= ~0x100000U;
        }
    }
}

// FUNCTION: th08 0x4251b0
#pragma var_order(i, bullet, enemy, instruction)
void __fastcall FUN_004251b0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet;

    g_EclGameTimeScale =
        1.0f / static_cast<f32>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(instruction) + 0x10));
    g_EclExBarrierRenderState.vm0.SetInterrupt(2);
    g_EclExBarrierRenderState.vm1.SetInterrupt(2);

    bullet = &g_BulletManager.bullets[0];
    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        bullet->velocity *= g_EclGameTimeScale;
        bullet->sprites.bulletVm.baseSpriteIndex = bullet->sprites.bulletVm.activeSpriteIndex;
        if (bullet->sprites.bulletVm.activeSpriteIndex >= 96 &&
            bullet->sprites.bulletVm.activeSpriteIndex <= 111)
        {
            g_BulletManager.bulletAnm->SetSprite(&bullet->sprites.bulletVm, 111);
        }
    }
}

// FUNCTION: th08 0x425290
#pragma var_order(i, scale, bullet, enemy, instruction)
void __fastcall FUN_00425290(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    f32 scale;
    Bullet *bullet = &g_BulletManager.bullets[0];

    scale = 1.0f / g_EclGameTimeScale;
    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        bullet->velocity *= scale;
        if (bullet->sprites.bulletVm.activeSpriteIndex >= 96 &&
            bullet->sprites.bulletVm.activeSpriteIndex <= 111)
        {
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm, bullet->sprites.bulletVm.baseSpriteIndex);
        }
    }

    g_EclGameTimeScale =
        1.0f / static_cast<f32>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(instruction) + 0x10));
    if (g_EclGameTimeScale < 1.0f)
        g_EclGameTimeScaleFlags |= 0x20U;
    g_EclGameTimeScale = 1.0f;
    g_EclExBarrierRenderState.vm0.SetInterrupt(1);
    g_EclExBarrierRenderState.vm1.SetInterrupt(1);
}


// FUNCTION: th08 0x425390
void __fastcall FUN_00425390(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xfdc) != 0)
        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(enemy->bytes + 0x2d34), static_cast<ItemType>(3), 0);
    else
        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(enemy->bytes + 0x2d34), static_cast<ItemType>(5), 0);
}

} // namespace th08

#ifdef TH08_MODERN_PORT
#undef g_EclExUpdateCallback
#undef g_EclExBarrierRenderState
#endif
