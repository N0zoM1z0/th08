#include "th_pch.h"

#include "AsciiManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "EnemyManager.hpp"
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
    u8 serializedReserved08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 serializedReserved0C[4];
    union
    {
        i32 value;
        i8 byteValue;
    };
};
C_ASSERT(offsetof(EclExInstruction, value) == 0x10);

namespace EclExIns
{
void __fastcall ReisenFreezeBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall MokouResurrection(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SetScriptedUpdateFreeze(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
}

void __fastcall DrawBulletWarpBarrier();

#define ECL_EX_CONTEXT(enemy) (reinterpret_cast<Enemy *>(enemy)->activeEclContext)

// FUNCTION: th08 0x423390
void __fastcall ConfigureNightBlindness(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_AsciiManager.nightBlindnessAlpha = ECL_EX_CONTEXT(enemy)->intVariables[0];
    *reinterpret_cast<i32 *>(&g_AsciiManager.nightBlindnessRadius) =
        *reinterpret_cast<i32 *>(&ECL_EX_CONTEXT(enemy)->floatVariables[0]);
}

// FUNCTION: th08 0x4233d0
void __fastcall TriggerShortScreenPulse(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 60, 1, -1, 0, CHAIN_PRIO_DRAW_SCREENEFFECT);
}

// FUNCTION: th08 0x423400
void __fastcall UpdateBouncingEnemyMotion(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 changed;

    changed = 0;
    if (reinterpret_cast<Enemy *>(enemy)->position.x <= 0.0f ||
        reinterpret_cast<Enemy *>(enemy)->position.x >= 384.0f)
    {
        reinterpret_cast<Enemy *>(enemy)->velocity.x =
            -reinterpret_cast<Enemy *>(enemy)->velocity.x;
        changed = 1;
    }

    if (reinterpret_cast<Enemy *>(enemy)->velocity.y <
        ECL_EX_CONTEXT(enemy)->floatVariables[7])
    {
        reinterpret_cast<Enemy *>(enemy)->velocity.y +=
            ECL_EX_CONTEXT(enemy)->floatVariables[6];
        changed = 1;
    }

    if (reinterpret_cast<Enemy *>(enemy)->position.y < -64.0f)
    {
        reinterpret_cast<Enemy *>(enemy)->velocity.y =
            -reinterpret_cast<Enemy *>(enemy)->velocity.y;
        changed = 1;
    }
    else if (reinterpret_cast<Enemy *>(enemy)->position.y >= 480.0f)
    {
        reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_ALLOW_OFFSCREEN;
    }

    if (changed)
    {
        reinterpret_cast<Enemy *>(enemy)->movementAngle =
            VectorAngle(reinterpret_cast<Enemy *>(enemy)->velocity.y,
                        reinterpret_cast<Enemy *>(enemy)->velocity.x);
    }
}

// FUNCTION: th08 0x423530
void __fastcall StartNarrowBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    AnmVm *effect;
    effect = g_EffectManager.SpawnEffectInFixedSlot(56, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position), 9, 1, -1);
    effect = g_EffectManager.SpawnEffectInFixedSlot(56, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position), 10, 1, -1);
    g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 97);
    g_Background.spellBackgroundDrawCallback = &DrawBulletWarpBarrier;
}

// FUNCTION: th08 0x4235a0
#pragma var_order(effect9, savedColor, i, radius9, unusedSpellVm, effect10, radius10, vertices)
void __fastcall DrawBulletWarpBarrier()
{
    VertexDiffuseXyzrhw vertices[10];
    Effect *effect9;
    i32 savedColor;
    i32 i;
    f32 radius9;
    AnmVm *unusedSpellVm;
    Effect *effect10;
    f32 radius10;

    effect9 = reinterpret_cast<Effect *>(g_EffectManager.GetFixedSlotVm(9));
    effect10 = reinterpret_cast<Effect *>(g_EffectManager.GetFixedSlotVm(10));
    unusedSpellVm = &g_Background.spellVms[0];

    radius9 = effect9->vm.pos.x * 0.7071068286895752f;
    radius10 = effect10->vm.pos.x * 0.7071068286895752f;

    vertices[0].pos.x = 32.0f + effect9->position.x - radius9;
    vertices[0].pos.y = 16.0f + effect9->position.y - radius9;
    vertices[1].pos.x = 32.0f + effect9->position.x - radius10;
    vertices[1].pos.y = 16.0f + effect9->position.y - radius10;
    vertices[2].pos.x = 32.0f + effect9->position.x + radius9;
    vertices[2].pos.y = 16.0f + effect9->position.y - radius9;
    vertices[3].pos.x = 32.0f + effect9->position.x + radius10;
    vertices[3].pos.y = 16.0f + effect9->position.y - radius10;
    vertices[4].pos.x = 32.0f + effect9->position.x + radius9;
    vertices[4].pos.y = 16.0f + effect9->position.y + radius9;
    vertices[5].pos.x = 32.0f + effect9->position.x + radius10;
    vertices[5].pos.y = 16.0f + effect9->position.y + radius10;
    vertices[6].pos.x = 32.0f + effect9->position.x - radius9;
    vertices[6].pos.y = 16.0f + effect9->position.y + radius9;
    vertices[7].pos.x = 32.0f + effect9->position.x - radius10;
    vertices[7].pos.y = 16.0f + effect9->position.y + radius10;
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
    g_Background.spellVms[0].scale.x = -1.5f;
    g_Background.spellVms[0].scale.y = -1.75f;
    g_Background.spellVms[0].pos.z = 0.7f;
    g_Background.spellVms[0].pos.x = 416.0f;
    g_Background.spellVms[0].pos.y = 464.0f;
    savedColor = g_Background.spellVms[0].color1.d3dColor;
    g_Background.spellVms[0].color1.d3dColor = 0xffe0c0c0;
    g_AnmManager->Draw2D(&g_Background.spellVms[0]);
    g_Background.spellVms[0].scale.x = 1.5f;
    g_Background.spellVms[0].scale.y = 1.75f;
    g_Background.spellVms[0].pos.z = 0.5f;
    g_Background.spellVms[0].pos.x = 32.0f;
    g_Background.spellVms[0].pos.y = 16.0f;
    g_Background.spellVms[0].color1.d3dColor = savedColor;

    g_Background.spellVms[1].rotation.z *= -1.0f;
    g_Background.spellVms[1].pos.z = 0.6f;
    savedColor = g_Background.spellVms[1].color1.d3dColor;
    g_Background.spellVms[1].color1.d3dColor = 0xffe0c0c0;
    g_AnmManager->Draw2DAndFlush(&g_Background.spellVms[1]);
    g_Background.spellVms[1].rotation.z *= -1.0f;
    g_Background.spellVms[1].pos.z = 0.5f;
    g_Background.spellVms[1].color1.d3dColor = savedColor;
    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
}

// FUNCTION: th08 0x423a60
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall WarpBulletsAcrossNarrowBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
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
void __fastcall StartMediumBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    AnmVm *effect;
    effect = g_EffectManager.SpawnEffectInFixedSlot(65, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position), 9, 1, -1);
    effect = g_EffectManager.SpawnEffectInFixedSlot(65, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position), 10, 1, -1);
    g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 99);
    g_Background.spellBackgroundDrawCallback = &DrawBulletWarpBarrier;
}



// FUNCTION: th08 0x423e20
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall WarpBulletsAcrossMediumBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
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
void __fastcall StopBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    reinterpret_cast<Effect *>(g_EffectManager.GetFixedSlotVm(9))->active = 0;
    reinterpret_cast<Effect *>(g_EffectManager.GetFixedSlotVm(10))->active = 0;
    g_Background.spellVmCount = 2;
}

// FUNCTION: th08 0x424170
void __fastcall StartWideBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    AnmVm *effect;
    effect = g_EffectManager.SpawnEffectInFixedSlot(58, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position), 9, 1, -1);
    effect = g_EffectManager.SpawnEffectInFixedSlot(58, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position), 10, 1, -1);
    g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 101);
    g_Background.spellBackgroundDrawCallback = &DrawBulletWarpBarrier;
}


// FUNCTION: th08 0x4241e0
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall WarpBulletsAcrossWideBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
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
void __fastcall SynchronizeOrbitingChildFormation(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 count;
    i32 groupId;
    EclOperands::EnemyOverlay *firstChild;
    f32 delta;
    f32 targetAngle;
    EclOperands::EnemyOverlay *cursor;

    groupId = ECL_EX_CONTEXT(enemy)->extraIntVariables[2];
    cursor = reinterpret_cast<EclOperands::EnemyOverlay *>(
        reinterpret_cast<Enemy *>(enemy)->parentEnemy);
    if (cursor == NULL)
        return;

    count = 0;
    while (reinterpret_cast<Enemy *>(cursor)->nextInAttachmentChain != NULL)
    {
        cursor = reinterpret_cast<EclOperands::EnemyOverlay *>(
            reinterpret_cast<Enemy *>(cursor)->nextInAttachmentChain);
        if (ECL_EX_CONTEXT(cursor)->extraIntVariables[2] == groupId)
        {
            ECL_EX_CONTEXT(cursor)->extraIntVariables[1] = count;
            if (count == 0)
                firstChild = cursor;
            ++count;
        }
    }

    ECL_EX_CONTEXT(enemy)->intVariables[5] = 0;
    if (ECL_EX_CONTEXT(enemy)->intVariables[6] != count)
    {
        if (ECL_EX_CONTEXT(enemy)->intVariables[6] != 0)
        {
            ECL_EX_CONTEXT(enemy)->intVariables[5] = 1;
        }
        ECL_EX_CONTEXT(enemy)->intVariables[6] = count;
    }

    groupId = ECL_EX_CONTEXT(enemy)->extraIntVariables[1];
    ++ECL_EX_CONTEXT(enemy)->intVariables[7];
    if (groupId != 0)
    {
        targetAngle = AddNormalizeAngle(
            reinterpret_cast<Enemy *>(firstChild)->orbitAngle,
            static_cast<f32>(groupId) * 6.2831854820251465f / static_cast<f32>(count));
        if (ECL_EX_CONTEXT(firstChild)->intVariables[7] !=
            ECL_EX_CONTEXT(enemy)->intVariables[7])
        {
            targetAngle = AddNormalizeAngle(
                targetAngle, reinterpret_cast<Enemy *>(firstChild)->orbitAngularVelocity);
        }

        delta = AddNormalizeAngle(reinterpret_cast<Enemy *>(enemy)->orbitAngle,
                                  reinterpret_cast<Enemy *>(enemy)->orbitAngularVelocity);
        delta = targetAngle - delta;
        if (fabsf(delta) > ZUN_PI)
        {
            delta = delta > 0.0f ? -6.2831854820251465f + delta : 6.2831854820251465f + delta;
        }
        delta *= 0.02f;
        reinterpret_cast<Enemy *>(enemy)->orbitAngle =
            AddNormalizeAngle(reinterpret_cast<Enemy *>(enemy)->orbitAngle, delta);
    }
}





// FUNCTION: th08 0x4246e0
void __fastcall TriggerScreenPulseAndShake(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(
        SCREEN_EFFECT_ARCADE_PULSE, 30, 5, 0x40ffffff, 0, CHAIN_PRIO_DRAW_SCREENEFFECT);
    ScreenEffect::RegisterChain(
        SCREEN_EFFECT_SHAKE_ENVELOPE, 4, 120, 190, 60, CHAIN_PRIO_DRAW_SCREENEFFECT);
}

// FUNCTION: th08 0x424a00
void __fastcall SetScreenEffectCounter(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_ScreenEffectCounter = instruction->value;
}

// FUNCTION: th08 0x424730
#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)
void __fastcall UpdateNarrowRotatingLaserHitbox(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    Float3 origin(
        reinterpret_cast<Enemy *>(enemy)->worldPosition.x -
            ECL_EX_CONTEXT(enemy)->floatVariables[0],
        reinterpret_cast<Enemy *>(enemy)->worldPosition.y -
            ECL_EX_CONTEXT(enemy)->floatVariables[1],
        0.0f);
    Float3 outerSize(590.0f, 160.0f, 0.0f);
    Float3 innerSize(590.0f, 128.0f, 0.0f);
    Float3 position(outerSize.x / 2.0f + origin.x, origin.y, 0.0f);

    if (reinterpret_cast<Enemy *>(enemy)->bossTimer.IsPeriodic(12))
    {
        g_Player.CalcLaserHitbox(&position, &innerSize, &origin,
                                 reinterpret_cast<Enemy *>(enemy)->vm.rotation.z, 1);
    }
    g_Player.CalcLaserHitbox(&position, &outerSize, &origin,
                             reinterpret_cast<Enemy *>(enemy)->vm.rotation.z, 0);
}



// FUNCTION: th08 0x424820
#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)
void __fastcall UpdateMediumRotatingLaserHitbox(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    Float3 origin(
        reinterpret_cast<Enemy *>(enemy)->position.x -
            ECL_EX_CONTEXT(enemy)->floatVariables[0],
        reinterpret_cast<Enemy *>(enemy)->position.y -
            ECL_EX_CONTEXT(enemy)->floatVariables[1],
        0.0f);
    Float3 outerSize(590.0f, 240.0f, 0.0f);
    Float3 innerSize(590.0f, 192.0f, 0.0f);
    Float3 position(outerSize.x / 2.0f + origin.x, origin.y, 0.0f);

    if (reinterpret_cast<Enemy *>(enemy)->bossTimer.IsPeriodic(12))
    {
        g_Player.CalcLaserHitbox(&position, &innerSize, &origin,
                                 reinterpret_cast<Enemy *>(enemy)->vm.rotation.z, 1);
    }
    g_Player.CalcLaserHitbox(&position, &outerSize, &origin,
                             reinterpret_cast<Enemy *>(enemy)->vm.rotation.z, 0);
}


// FUNCTION: th08 0x424910
#pragma var_order(position, outerSize, innerSize, origin, enemy, instruction)
void __fastcall UpdateWideRotatingLaserHitbox(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    Float3 origin(
        reinterpret_cast<Enemy *>(enemy)->worldPosition.x -
            ECL_EX_CONTEXT(enemy)->floatVariables[0],
        reinterpret_cast<Enemy *>(enemy)->worldPosition.y -
            ECL_EX_CONTEXT(enemy)->floatVariables[1],
        0.0f);
    Float3 outerSize(590.0f, 288.0f, 0.0f);
    Float3 innerSize(590.0f, 224.0f, 0.0f);
    Float3 position(outerSize.x / 2.0f + origin.x, origin.y, 0.0f);

    if (reinterpret_cast<Enemy *>(enemy)->bossTimer.IsPeriodic(12))
    {
        g_Player.CalcLaserHitbox(&position, &innerSize, &origin,
                                 reinterpret_cast<Enemy *>(enemy)->vm.rotation.z, 1);
    }
    g_Player.CalcLaserHitbox(&position, &outerSize, &origin,
                             reinterpret_cast<Enemy *>(enemy)->vm.rotation.z, 0);
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
             *reinterpret_cast<u32 *>(&ECL_EX_CONTEXT(enemy)->intVariables[0])) != 0)
        {
        if (bullet->sprites.bulletVm.type == 1)
        {
            bullet->sprites.bulletVm.type = 0;
            bullet->sprites.bulletVm.blendMode = AnmBlendMode_Additive;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex + 16);
            bullet->collisionDisabled = 1;
            bullet->velocity.FromAngleMagnitude(
                ECL_EX_CONTEXT(enemy)->floatVariables[0],
                g_Supervisor.framerateMultiplier *
                    ECL_EX_CONTEXT(enemy)->floatVariables[1]);
        }
        else
        {
            bullet->sprites.bulletVm.type = 1;
            bullet->sprites.bulletVm.blendMode = AnmBlendMode_Normal;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex - 16);
            bullet->collisionDisabled = 0;
            bullet->velocity.FromAngleMagnitude(
                bullet->angle,
                g_Supervisor.framerateMultiplier * bullet->speed);
        }
        }
    }

    if (ECL_EX_CONTEXT(enemy)->intVariables[1] == 0)
    {
        setCursor = enemy;
        while (reinterpret_cast<Enemy *>(setCursor)->nextInAttachmentChain != NULL)
        {
            setCursor = reinterpret_cast<EclOperands::EnemyOverlay *>(
                reinterpret_cast<Enemy *>(setCursor)->nextInAttachmentChain);
            reinterpret_cast<Enemy *>(setCursor)->flags2 |= ENEMY_FLAG2_FORCE_PAUSE;
        }
        g_Background.spellVms[0].SetInterrupt(2);
        g_Background.spellVms[1].SetInterrupt(2);
    }
    else
    {
        clearCursor = enemy;
        while (reinterpret_cast<Enemy *>(clearCursor)->nextInAttachmentChain != NULL)
        {
            clearCursor = reinterpret_cast<EclOperands::EnemyOverlay *>(
                reinterpret_cast<Enemy *>(clearCursor)->nextInAttachmentChain);
            reinterpret_cast<Enemy *>(clearCursor)->flags2 &= ~ENEMY_FLAG2_FORCE_PAUSE;
        }
        g_Background.spellVms[0].SetInterrupt(1);
        g_Background.spellVms[1].SetInterrupt(1);
    }
}

// FUNCTION: th08 0x424c40
#pragma var_order(i, bullet, enemy, instruction)
void __fastcall AdvanceReisenBulletPhase(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags &
             *reinterpret_cast<u32 *>(&ECL_EX_CONTEXT(enemy)->intVariables[0])) != 0)
        {
        if (bullet->sprites.bulletVm.type == 1)
        {
            bullet->sprites.bulletVm.type = 0;
            bullet->sprites.bulletVm.blendMode = AnmBlendMode_Additive;
            bullet->sprites.bulletVm.color1.a = 0;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex + 16);
            bullet->collisionDisabled = 1;
            bullet->velocity.FromAngleMagnitude(
                bullet->angle,
                g_Supervisor.framerateMultiplier *
                    ECL_EX_CONTEXT(enemy)->floatVariables[1]);
        }
        else if (bullet->sprites.bulletVm.type == 0)
        {
            bullet->sprites.bulletVm.type = 2;
            bullet->sprites.bulletVm.color1.a = 0;
            bullet->sprites.bulletVm.StartColor1AlphaInterpolation(15, AnmInterpMode_Linear, 0, 255);
        }
        else
        {
            bullet->sprites.bulletVm.type = 1;
            bullet->sprites.bulletVm.blendMode = AnmBlendMode_Normal;
            g_BulletManager.bulletAnm->SetSprite(
                &bullet->sprites.bulletVm,
                bullet->sprites.bulletVm.activeSpriteIndex - 16);
            bullet->collisionDisabled = 0;
            bullet->velocity.FromAngleMagnitude(
                bullet->angle,
                g_Supervisor.framerateMultiplier * bullet->speed);
        }
        }
    }
}


// FUNCTION: th08 0x424e00
void __fastcall ApplyRedBackgroundTint(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_Background.AccumulateTint(0xffc03030U);
}

// FUNCTION: th08 0x424e20
void __fastcall TriggerScreenShake(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(
        SCREEN_EFFECT_SHAKE_ENVELOPE, 16, 20, 20, 20, CHAIN_PRIO_DRAW_SCREENEFFECT);
}



// FUNCTION: th08 0x424e50
#pragma var_order(i, bullet, child, delta, enemy, instruction)
void __fastcall TriggerChildrenNearMarkedBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];
    EclOperands::EnemyOverlay *child;
    Float3 delta;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags & BULLET_TRANSFORM_ECL_EX_TRIGGER_MARKER) != 0)
        {
        child = reinterpret_cast<EclOperands::EnemyOverlay *>(
            reinterpret_cast<Enemy *>(enemy)->nextInAttachmentChain);
        while (child != NULL)
        {
            if (ECL_EX_CONTEXT(child)->extraIntVariables[2] == 0)
            {
                delta = bullet->position - reinterpret_cast<Enemy *>(child)->position;
                if (D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&delta)) < 4096.0f)
                {
                    ECL_EX_CONTEXT(child)->extraIntVariables[2] = 60;
                    ECL_EX_CONTEXT(child)->intVariables[7] =
                        ECL_EX_CONTEXT(enemy)->intVariables[7];
                }
            }
            child = reinterpret_cast<EclOperands::EnemyOverlay *>(
                reinterpret_cast<Enemy *>(child)->nextInAttachmentChain);
        }
        }
    }
}

// FUNCTION: th08 0x424f60
void __fastcall TriggerLongScreenPulse(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ScreenEffect::RegisterChain(
        SCREEN_EFFECT_ARCADE_PULSE, 180, 1, -1, 0, CHAIN_PRIO_DRAW_SCREENEFFECT);
}

// FUNCTION: th08 0x424f90
#pragma var_order(value, scale, enemy, instruction)
void __fastcall SetFrameRateDivisor(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 value;
    f32 scale;

    value = instruction->value;
    scale = 1.0f / static_cast<f32>(value);
    g_Supervisor.framerateMultiplier = scale;
}

// FUNCTION: th08 0x424fc0
void __fastcall PublishCurrentSpellCardNumber(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ECL_EX_CONTEXT(enemy)->intVariables[0] =
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
void __fastcall HideSpellCardPresentation(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_Spellcard.HideEnemySpellPresentation();
}

// FUNCTION: th08 0x425040
void __fastcall PublishCapturedSpellCardCount(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    ECL_EX_CONTEXT(enemy)->intVariables[0] =
        g_GameManager.globals->spellcardsCaptured;
}

// FUNCTION: th08 0x425070
void __fastcall EclExIns::SetScriptedUpdateFreeze(
    EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    g_GameManager.scriptedUpdateFreeze =
        instruction->byteValue;
    if (g_GameManager.scriptedUpdateFreeze)
    {
        g_Background.spellVms[0].SetInterrupt(2);
        g_Background.spellVms[1].SetInterrupt(2);
    }
    else
    {
        g_Background.spellVms[0].SetInterrupt(1);
        g_Background.spellVms[1].SetInterrupt(1);
    }
}


// FUNCTION: th08 0x4250d0
#pragma var_order(i, bullet, unusedVector, enemy, instruction)
void __fastcall SpawnEnemiesFromMarkedBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet = &g_BulletManager.bullets[0];
    Float3 unusedVector;

    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        if ((bullet->transformFlags & BULLET_TRANSFORM_ECL_EX_TRIGGER_MARKER) != 0)
        {
            ECL_EX_CONTEXT(enemy)->floatVariables[0] = bullet->angle;
            g_EnemyManager.SpawnEnemy2(
                ECL_EX_CONTEXT(enemy)->extraIntVariables[2],
                reinterpret_cast<D3DXVECTOR3 *>(&bullet->position), 800, -2, 10,
                ECL_EX_CONTEXT(enemy)->intVariables);
            bullet->transformFlags &= ~BULLET_TRANSFORM_ECL_EX_TRIGGER_MARKER;
        }
    }
}

// FUNCTION: th08 0x4251b0
#pragma var_order(i, bullet, enemy, instruction)
void __fastcall EnterScaledBulletTime(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    Bullet *bullet;

    g_Supervisor.framerateMultiplier =
        1.0f / static_cast<f32>(instruction->value);
    g_Background.spellVms[0].SetInterrupt(2);
    g_Background.spellVms[1].SetInterrupt(2);

    bullet = &g_BulletManager.bullets[0];
    for (i = 0; i < 0x600; ++i, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            continue;
        bullet->velocity *= g_Supervisor.framerateMultiplier;
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
void __fastcall ExitScaledBulletTime(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    i32 i;
    f32 scale;
    Bullet *bullet = &g_BulletManager.bullets[0];

    scale = 1.0f / g_Supervisor.framerateMultiplier;
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

    g_Supervisor.framerateMultiplier =
        1.0f / static_cast<f32>(instruction->value);
    if (g_Supervisor.framerateMultiplier < 1.0f)
        g_EclGameTimeScaleFlags |= 0x20U;
    g_Supervisor.framerateMultiplier = 1.0f;
    g_Background.spellVms[0].SetInterrupt(1);
    g_Background.spellVms[1].SetInterrupt(1);
}


// FUNCTION: th08 0x425390
void __fastcall SpawnBombOrExtendItem(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    if (g_Player.bombState.isInUse != 0)
        g_ItemManager.SpawnItem(&reinterpret_cast<Enemy *>(enemy)->position, ITEM_BOMB,
                                ITEM_STATE_DEFAULT);
    else
        g_ItemManager.SpawnItem(&reinterpret_cast<Enemy *>(enemy)->position, ITEM_EXTEND,
                                ITEM_STATE_DEFAULT);
}

#undef ECL_EX_CONTEXT

} // namespace th08
