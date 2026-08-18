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



// FUNCTION: th08 0x423e20
#pragma var_order(i, previousZone, bullet, currentZone, previousPosition, enemy, instruction)
void __fastcall FUN_00423e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
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

        if (*reinterpret_cast<f32 *>(bullet + 0xd44) > 112.80403137207031f &&
            *reinterpret_cast<f32 *>(bullet + 0xd44) < 271.1959533691406f &&
            *reinterpret_cast<f32 *>(bullet + 0xd48) > 128.8040313720703f &&
            *reinterpret_cast<f32 *>(bullet + 0xd48) < 287.1959533691406f)
            currentZone = 0;
        else if (*reinterpret_cast<f32 *>(bullet + 0xd44) > 33.608070373535156f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd44) < 350.3919372558594f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd48) > 49.608070373535156f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd48) < 366.3919372558594f)
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
            *reinterpret_cast<i32 *>(bullet + 0xdc4) = 2;
            *reinterpret_cast<Float3 *>(bullet + 0xd50) *= -1.0f;
            if (currentZone == 0 || previousZone == 0)
            {
                *reinterpret_cast<f32 *>(bullet + 0xd44) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd44) - 192.0f) *
                        158.39193725585938f / 79.19596862792969f + 192.0f;
                *reinterpret_cast<f32 *>(bullet + 0xd48) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd48) - 208.0f) *
                        158.39193725585938f / 79.19596862792969f + 208.0f;
            }
            else
            {
                *reinterpret_cast<f32 *>(bullet + 0xd44) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd44) - 192.0f) *
                        79.19596862792969f / 158.39193725585938f + 192.0f;
                *reinterpret_cast<f32 *>(bullet + 0xd48) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd48) - 208.0f) *
                        79.19596862792969f / 158.39193725585938f + 208.0f;
            }
            *reinterpret_cast<f32 *>(bullet + 0xd74) =
                AddNormalizeAngle(*reinterpret_cast<f32 *>(bullet + 0xd74), ZUN_PI);
        }
    }
}

// FUNCTION: th08 0x424130
void __fastcall FUN_00424130(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction)
{
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(g_EffectManager.FUN_004253e0(9)) + 0x350) = 0;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(g_EffectManager.FUN_004253e0(10)) + 0x350) = 0;
    *reinterpret_cast<i32 *>(0x4E4B60) = 2;
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

        if (*reinterpret_cast<f32 *>(bullet + 0xd44) > 56.23548889160156f &&
            *reinterpret_cast<f32 *>(bullet + 0xd44) < 327.7645263671875f &&
            *reinterpret_cast<f32 *>(bullet + 0xd48) > 88.23548889160156f &&
            *reinterpret_cast<f32 *>(bullet + 0xd48) < 359.7645263671875f)
            currentZone = 0;
        else if (*reinterpret_cast<f32 *>(bullet + 0xd44) > -32.0f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd44) < 416.0f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd48) > 0.0f &&
                 *reinterpret_cast<f32 *>(bullet + 0xd48) < 448.0f)
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
            *reinterpret_cast<i32 *>(bullet + 0xdc4) = 2;
            *reinterpret_cast<Float3 *>(bullet + 0xd50) *= -1.0f;
            if (currentZone == 0 || previousZone == 0)
            {
                *reinterpret_cast<f32 *>(bullet + 0xd44) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd44) - 192.0f) *
                        224.0f / 135.76451110839844f + 192.0f;
                *reinterpret_cast<f32 *>(bullet + 0xd48) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd48) - 224.0f) *
                        224.0f / 135.76451110839844f + 224.0f;
            }
            else
            {
                *reinterpret_cast<f32 *>(bullet + 0xd44) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd44) - 192.0f) *
                        135.76451110839844f / 224.0f + 192.0f;
                *reinterpret_cast<f32 *>(bullet + 0xd48) =
                    (*reinterpret_cast<f32 *>(bullet + 0xd48) - 224.0f) *
                        135.76451110839844f / 224.0f + 224.0f;
            }
            *reinterpret_cast<f32 *>(bullet + 0xd74) =
                AddNormalizeAngle(*reinterpret_cast<f32 *>(bullet + 0xd74), ZUN_PI);
        }
    }
}

} // namespace th08
