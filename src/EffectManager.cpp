#include "th_pch.h"

#include "EclManager.hpp"
#include "AnmManager.hpp"
#include "ItemManager.hpp"
#include "ReplayManager.hpp"
#include "GameManager.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();

// FUNCTION: th08 0x428310
#pragma var_order(delta, point)
void __fastcall FUN_00428310(AnmVm *effect, D3DXVECTOR3 *base)
{
    D3DXVECTOR3 delta;
    D3DXVECTOR3 point;

    if (*(u8 *)0x164D0BA == 0 && *(u8 *)0x164D0BB == 0)
    {
        point = *base + *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(effect) + 0x244);
        delta = *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(effect) + 0x288) - point;
        if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x288) > -9999.0f)
        {
            delta.x += 32.0f;
            delta.y += 16.0f;
            delta.z = 0.0f;
            if (D3DXVec3LengthSq(&delta) < 25600.0f)
            {
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x238) += 0.0005000000237487257f;
                *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(effect) + 0x244) +=
                    delta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x238);
            }
        }

        delta = point - *reinterpret_cast<D3DXVECTOR3 *>(0x17D61AC);
        delta.x -= 32.0f;
        delta.y -= 16.0f;
        delta.z = 0.0f;
        if (D3DXVec3LengthSq(&delta) < 7744.0f)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(effect) + 0x244) += delta * 0.019999999552965164f;
        }
    }
    *base += *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(effect) + 0x244);
}

struct Effect
{
    AnmVm vm;
    Float3 vector0;
    Float3 vector1;
    Float3 vector2;
    Float3 vector3;
    Float3 vector4;
    Float3 vector5;
    Float3 vector6;
    Float3 vector7;
    Float3 vector8;
    u8 unknown310[0x28];
    ZunTimer timer;
    u8 unknown344[0x1c];

    Effect();
};
C_ASSERT(sizeof(Effect) == 0x360);

// FUNCTION: th08 0x4287e0
Effect::Effect()
{
}

DIFFABLE_STATIC(EffectManager, g_EffectManager);
DIFFABLE_STATIC(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EffectManagerDrawChain);

// Target 0x004E4B64 is owned by Gui.cpp but participates in effect-resource setup.
extern i32 g_GuiMessageStageMode;

// FUNCTION: th08 0x425410
void EffectManager::ResetEffects()
{
    memset(this, 0, 0x8B05C);
}

// FUNCTION: th08 0x427bf0
#pragma var_order(effect, i)
ChainCallbackResult EffectManager::OnUpdate(EffectManager *effectManager)
{
    u8 *effect = reinterpret_cast<u8 *>(effectManager) + 0x1C;
    i32 i;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectManager) + 0x8) = 0;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B03C) =
        reinterpret_cast<u8 *>(effectManager) + 0x89F5C;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B040) =
        reinterpret_cast<u8 *>(effectManager) + 0x8A2BC;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B044) =
        reinterpret_cast<u8 *>(effectManager) + 0x8A61C;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B048) =
        reinterpret_cast<u8 *>(effectManager) + 0x8A97C;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B04C) =
        reinterpret_cast<u8 *>(effectManager) + 0x8ACDC;

    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8A2B8) = NULL;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8A618) = NULL;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8A978) = NULL;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8ACD8) = NULL;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B038) = NULL;

    for (i = 0; i < 653; i++, effect += 0x360)
    {
        if (*reinterpret_cast<i8 *>(effect + 0x350) == 0)
        {
            if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
            {
                g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));
                *reinterpret_cast<void **>(effect + 0x358) = NULL;
            }
            continue;
        }

        (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectManager) + 0x8))++;
        if (((*reinterpret_cast<u32 *>(0x164D0B4) >> 10) & 1) == 0 ||
            *reinterpret_cast<i8 *>(effect + 0x357) != 0)
        {
            if (*reinterpret_cast<void **>(effect + 0x348) != NULL &&
                reinterpret_cast<i32 (__fastcall *)(void *)>(*reinterpret_cast<void **>(effect + 0x348))(effect) != 1)
            {
                *reinterpret_cast<i8 *>(effect + 0x350) = 0;
                continue;
            }
            if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(effect)))
            {
                *reinterpret_cast<i8 *>(effect + 0x350) = 0;
                continue;
            }
            (*reinterpret_cast<ZunTimer *>(effect + 0x338))++;
        }

        *reinterpret_cast<u8 **>(effect + 0x35C) = NULL;
        if (*reinterpret_cast<i8 *>(effect + 0x351) == 0x40)
            continue;

        if (*reinterpret_cast<i8 *>(effect + 0x354) == 1 || *reinterpret_cast<i8 *>(effect + 0x354) >= 3)
        {
            *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B040) + 0x35C) = effect;
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B040) = effect;
        }
        else if (*reinterpret_cast<i8 *>(effect + 0x354) == 0)
        {
            if (*reinterpret_cast<i8 *>(effect + 0x355) != 0)
            {
                *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B048) + 0x35C) = effect;
                *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B048) = effect;
            }
            else if (((*reinterpret_cast<u32 *>(effect + 0x1F8) >> 4) & 3) == 1)
            {
                *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B04C) + 0x35C) = effect;
                *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B04C) = effect;
            }
            else
            {
                *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B03C) + 0x35C) = effect;
                *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B03C) = effect;
            }
        }
        else
        {
            *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B044) + 0x35C) = effect;
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B044) = effect;
        }
    }

    if (++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectManager) + 0x8B050) % 300 == 100 &&
        g_GameManager.IsTampered())
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x427f00
#pragma var_order(effect)
ChainCallbackResult EffectManager::OnDraw(EffectManager *effectManager)
{
    u8 *effect;

    effect = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8A2B8);
    while (effect != NULL)
    {
        if (*reinterpret_cast<void **>(effect + 0x34C) != NULL)
        {
            reinterpret_cast<void (__fastcall *)(void *)>(*reinterpret_cast<void **>(effect + 0x34C))(effect);
        }
        else
        {
            *reinterpret_cast<Float3 *>(effect + 0x208) = *reinterpret_cast<Float3 *>(effect + 0x2A4);
            *reinterpret_cast<f32 *>(effect + 0x208) += g_ItemAnmManagerScreenShakeOffset.x;
            *reinterpret_cast<f32 *>(effect + 0x20C) += g_ItemAnmManagerScreenShakeOffset.y;
            *reinterpret_cast<f32 *>(effect + 0x210) = 0.07f;
            reinterpret_cast<Float3 *>(effect + 0x208)->operator+=(
                *reinterpret_cast<Float3 *>(effect + 0x288));
            g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(effect));
        }
        effect = *reinterpret_cast<u8 **>(effect + 0x35C);
    }

    effect = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8A978);
    while (effect != NULL)
    {
        *reinterpret_cast<Float3 *>(effect + 0x208) = *reinterpret_cast<Float3 *>(effect + 0x2A4);
        g_AnmManager->FUN_00463cf0(reinterpret_cast<AnmVm *>(effect));
        effect = *reinterpret_cast<u8 **>(effect + 0x35C);
    }

    effect = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(effectManager) + 0x8B038);
    while (effect != NULL)
    {
        if (*reinterpret_cast<void **>(effect + 0x34C) != NULL)
        {
            reinterpret_cast<void (__fastcall *)(void *)>(*reinterpret_cast<void **>(effect + 0x34C))(effect);
        }
        else
        {
            *reinterpret_cast<Float3 *>(effect + 0x208) = *reinterpret_cast<Float3 *>(effect + 0x2A4);
            *reinterpret_cast<f32 *>(effect + 0x208) += g_ItemAnmManagerScreenShakeOffset.x;
            *reinterpret_cast<f32 *>(effect + 0x20C) += g_ItemAnmManagerScreenShakeOffset.y;
            *reinterpret_cast<f32 *>(effect + 0x210) = 0.07f;
            reinterpret_cast<Float3 *>(effect + 0x208)->operator+=(
                *reinterpret_cast<Float3 *>(effect + 0x288));
            g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(effect));
        }
        effect = *reinterpret_cast<u8 **>(effect + 0x35C);
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x4284b0
ZunResult EffectManager::AddedCallback(EffectManager *effectManager)
{
    effectManager->ResetEffects();
    *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B054) = g_AnmManager->GetAnm(6);
    g_GuiMessageStageMode = 0;
    *reinterpret_cast<i32 *>(0x4E4B60) = 2;

    if (!IsDisableResourceReload())
    {
        if (!g_GameManager.IsSpellPractice() || g_GameManager.currentSpellCardNumber < 216)
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B058) =
                g_AnmManager->PreloadAnm(9, reinterpret_cast<const char **>(0x4C7480)[g_GameManager.currentStage]);
        }
        else
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B058) =
                g_AnmManager->PreloadAnm(
                    9, reinterpret_cast<const char **>(0x4C7144)[g_GameManager.currentSpellCardNumber]);
        }
        if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B058) == NULL)
            return ZUN_ERROR;
    }
    else
    {
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B058) = g_AnmManager->GetAnm(9);
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x428590
#pragma var_order(effect, i)
ZunResult EffectManager::DeletedCallback(EffectManager *effectManager)
{
    u8 *effect = reinterpret_cast<u8 *>(effectManager) + 0x1C;
    i32 i;
    for (i = 0; i < 653; i++, effect += 0x360)
    {
        if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
        {
            g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));
            *reinterpret_cast<void **>(effect + 0x358) = NULL;
        }
    }
    if (!IsDisableResourceReload())
        g_AnmManager->ReleaseAnm(9);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x428620
ZunResult EffectManager::RegisterChain()
{
    EffectManager *effectManager = &g_EffectManager;
    effectManager->ResetEffects();
    g_EffectManagerCalcChain.SetCallback((ChainCallback)EffectManager::OnUpdate);
    g_EffectManagerCalcChain.addedCallback = (ChainLifetimeCallback)EffectManager::AddedCallback;
    g_EffectManagerCalcChain.deletedCallback = (ChainLifetimeCallback)EffectManager::DeletedCallback;
    g_EffectManagerCalcChain.arg = effectManager;
    if (g_Chain.AddToCalcChain(&g_EffectManagerCalcChain, 13) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_EffectManagerDrawChain.SetCallback((ChainCallback)EffectManager::OnDraw);
    g_EffectManagerDrawChain.arg = effectManager;
    g_Chain.AddToDrawChain(&g_EffectManagerDrawChain, 12);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x428100
#pragma var_order(effect, this)
i32 EffectManager::DrawUnkTypeEffects()
{
    u8 *effect = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x8acd8);

    while (effect != NULL)
    {
        if (*reinterpret_cast<void **>(effect + 0x34c) != NULL)
        {
            reinterpret_cast<void (__fastcall *)(void *)>(*reinterpret_cast<void **>(effect + 0x34c))(effect);
        }
        else
        {
            *reinterpret_cast<Float3 *>(effect + 0x208) = *reinterpret_cast<Float3 *>(effect + 0x2a4);
            *reinterpret_cast<f32 *>(effect + 0x208) += g_ItemAnmManagerScreenShakeOffset.x;
            *reinterpret_cast<f32 *>(effect + 0x20c) += g_ItemAnmManagerScreenShakeOffset.y;
            reinterpret_cast<Float3 *>(effect + 0x208)->operator+=(
                *reinterpret_cast<Float3 *>(effect + 0x288));
            *reinterpret_cast<f32 *>(effect + 0x210) = 0.04f;
            g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(effect));
        }

        effect = *reinterpret_cast<u8 **>(effect + 0x35c);
    }

    return 1;
}

// FUNCTION: th08 0x4281e0
#pragma var_order(effect, i, this)
i32 EffectManager::FUN_004281e0()
{
    u8 *effect = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x8A618);
    i32 i = 0;

    if (*reinterpret_cast<u8 *>((u8 *)&g_Supervisor + 0x13C) == 0)
    {
        return 1;
    }

    while (effect != NULL)
    {
        i++;
        if (*reinterpret_cast<u8 *>((u8 *)&g_Supervisor + 0x13C) == 1 && (i & 1) != 0)
        {
            return 1;
        }

        *reinterpret_cast<Float3 *>(effect + 0x208) = *reinterpret_cast<Float3 *>(effect + 0x2A4);
        if (*reinterpret_cast<i8 *>(effect + 0x354) == 4)
        {
            g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(effect));
        }
        else if (*reinterpret_cast<i8 *>(effect + 0x354) == 1)
        {
            if (*reinterpret_cast<i8 *>(effect + 0x351) == 0x33 || *reinterpret_cast<i8 *>(effect + 0x351) == 0x3F)
            {
                g_AnmManager->DrawWithCallback(reinterpret_cast<AnmVm *>(effect), (void *)FUN_00428310);
            }
            else
            {
                g_AnmManager->FUN_00463cf0(reinterpret_cast<AnmVm *>(effect));
            }
        }
        else
        {
            g_AnmManager->FUN_00464070(reinterpret_cast<AnmVm *>(effect));
        }

        effect = *reinterpret_cast<u8 **>(effect + 0x35C);
    }
    return 1;
}

// FUNCTION: th08 0x4286b0
void EffectManager::CutChain()
{
    g_Chain.Cut(&g_EffectManagerCalcChain);
    g_Chain.Cut(&g_EffectManagerDrawChain);
}




extern f32 g_EclGameTimeScale;

// FUNCTION: th08 0x423d70
Float3 *Float3::operator*=(f32 scalar)
{
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    return this;
}

// FUNCTION: th08 0x425fa0
Float3 Float3::operator-() const
{
    return Float3(-this->x, -this->y, -this->z);
}

// FUNCTION: th08 0x425d70
i32 __fastcall EffectRandomSplashInit(AnmVm *effect)
{
    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc)->operator float *()[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc)->operator float *()[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc)->operator float *()[2] = 0.0f;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2c8) =
        -*reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc) / 19.0f;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc) *= g_EclGameTimeScale;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2c8) *= g_EclGameTimeScale;
    return 0;
}

// FUNCTION: th08 0x425ea0
i32 __fastcall EffectRandomSplashBigInit(AnmVm *effect)
{
    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc)->operator float *()[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc)->operator float *()[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc)->operator float *()[2] = 0.0f;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2c8) =
        -*reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc) / 20.0f;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc) *= g_EclGameTimeScale;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2c8) *= g_EclGameTimeScale;
    return 0;
}

// FUNCTION: th08 0x425e60
i32 __fastcall EffectRandomSplashUpdate(AnmVm *effect)
{
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2a4) +=
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc) +=
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2c8);
    return 1;
}

// FUNCTION: th08 0x425fe0
i32 __fastcall EffectOrbitInit(AnmVm *effect)
{
    *reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x354) = 2;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2ec) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2f0) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2f4) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314) = 0.0f;
    return 0;
}

// FUNCTION: th08 0x426030
#pragma var_order(posOffset, verticalAngle, localMatrix, horizontalAngle, normalizedPos, alpha, this)
i32 __fastcall EffectOrbitUpdate(AnmVm *effect)
{
    Float3 posOffset;
    f32 verticalAngle;
    Float3 normalizedPos;
    D3DXMATRIX localMatrix;
    f32 horizontalAngle;
    f32 alpha;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&normalizedPos), reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(effect) + 0x2ec));
    verticalAngle = sinf(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x318));
    horizontalAngle = cosf(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x318));
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x304) = normalizedPos.x * verticalAngle;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x308) = normalizedPos.y * verticalAngle;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x30c) = normalizedPos.z * verticalAngle;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x310) = horizontalAngle;
    D3DXMatrixRotationQuaternion(&localMatrix, reinterpret_cast<D3DXQUATERNION *>(reinterpret_cast<u8 *>(effect) + 0x304));
    posOffset.x = normalizedPos.y * 1.0f - normalizedPos.z * 0.0f;
    posOffset.y = normalizedPos.z * 0.0f - normalizedPos.x * 1.0f;
    posOffset.z = normalizedPos.x * 0.0f - normalizedPos.y * 0.0f;
    if (D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&posOffset)) < 0.00001f)
        normalizedPos = Float3(1.0f, 0.0f, 0.0f);
    else
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&posOffset), reinterpret_cast<D3DXVECTOR3 *>(&posOffset));
    posOffset *= *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x314);
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3 *>(&posOffset), reinterpret_cast<D3DXVECTOR3 *>(&posOffset), &localMatrix);
    posOffset.z *= 6.0f;
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2a4) = posOffset + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2e0);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2ac) = 0.0f;
    if (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x352) != 0)
    {
        ++*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x353);
        if (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x353) >= 16)
            return 0;
        alpha = 1.0f - (f32)*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x353) / 16.0f;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(effect) + 0x1f0) =
            (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(effect) + 0x1f0) & 0xffffff) |
            ((i32)(alpha * 255.0f) << 24);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x1c) = 2.0f - alpha;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x18) = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x1c);
    }
    return 1;
}

struct EffectTemplate
{
    i32 scriptIdx;
    i32 field348;
    i32 (__fastcall *callback)(AnmVm *effect);
};
DIFFABLE_STATIC_ARRAY(EffectTemplate, 20, g_EffectTemplates);

// FUNCTION: th08 0x425430
#pragma var_order(effect, i)
AnmVm *EffectManager::SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color)
{
    u8 *effect = reinterpret_cast<u8 *>(this) + (*reinterpret_cast<i32 *>(this) * 0x360) + 0x1C;
    i32 i;

    for (i = 0; i < 0x200; i++)
    {
        *reinterpret_cast<i32 *>(this) = *reinterpret_cast<i32 *>(this) + 1;
        if (*reinterpret_cast<i32 *>(this) >= 0x200)
        {
            *reinterpret_cast<i32 *>(this) = 0;
        }

        if (*reinterpret_cast<i8 *>(effect + 0x350) != 0)
        {
            if (*reinterpret_cast<i32 *>(this) == 0)
            {
                effect = reinterpret_cast<u8 *>(this) + 0x1C;
            }
            else
            {
                effect += 0x360;
            }
            continue;
        }

        if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
        {
            g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));
        }

        memset(effect, 0, 0x360);
        *reinterpret_cast<i8 *>(effect + 0x350) = 1;
        *reinterpret_cast<i8 *>(effect + 0x351) = id;
        *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2A4) = *position;
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0x8B054))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(effect), g_EffectTemplates[id].scriptIdx);
        *reinterpret_cast<u32 *>(effect + 0x1F8) |= 0x2000;
        *reinterpret_cast<i32 *>(effect + 0x1F0) = color;
        *reinterpret_cast<i32 *>(effect + 0x288) = 0;
        *reinterpret_cast<i32 *>(effect + 0x28C) = 0;
        *reinterpret_cast<i32 *>(effect + 0x290) = 0;
        *reinterpret_cast<i32 *>(effect + 0x348) = g_EffectTemplates[id].field348;

        if (g_EffectTemplates[id].callback != NULL)
        {
            if (g_EffectTemplates[id].callback(reinterpret_cast<AnmVm *>(effect)) != 0)
            {
                *reinterpret_cast<i8 *>(effect + 0x350) = 0;
            }
        }

        count--;
        if (count == 0)
        {
            break;
        }

        if (*reinterpret_cast<i32 *>(this) == 0)
        {
            effect = reinterpret_cast<u8 *>(this) + 0x1C;
        }
        else
        {
            effect += 0x360;
        }
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x400;
    return reinterpret_cast<AnmVm *>(i >= 0x200 ? reinterpret_cast<u8 *>(this) + 0x89BFC : effect);
}

// FUNCTION: th08 0x425650
#pragma var_order(effect, i)
AnmVm *EffectManager::SpawnEffectAngle(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 count, i32 color)
{
    u8 *effect = reinterpret_cast<u8 *>(this) + (*reinterpret_cast<i32 *>(this) * 0x360) + 0x1C;
    i32 i;

    for (i = 0; i < 0x200; i++)
    {
        *reinterpret_cast<i32 *>(this) = *reinterpret_cast<i32 *>(this) + 1;
        if (*reinterpret_cast<i32 *>(this) >= 0x200)
        {
            *reinterpret_cast<i32 *>(this) = 0;
        }

        if (*reinterpret_cast<i8 *>(effect + 0x350) != 0)
        {
            if (*reinterpret_cast<i32 *>(this) == 0)
            {
                effect = reinterpret_cast<u8 *>(this) + 0x1C;
            }
            else
            {
                effect += 0x360;
            }
            continue;
        }

        if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
        {
            g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));
        }

        memset(effect, 0, 0x360);
        *reinterpret_cast<i8 *>(effect + 0x350) = 1;
        *reinterpret_cast<i8 *>(effect + 0x351) = id;
        *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2A4) = *position;
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0x8B054))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(effect), g_EffectTemplates[id].scriptIdx);
        *reinterpret_cast<i32 *>(effect + 0x1F0) = color;
        *reinterpret_cast<i32 *>(effect + 0x288) = 0;
        *reinterpret_cast<i32 *>(effect + 0x28C) = 0;
        *reinterpret_cast<i32 *>(effect + 0x290) = 0;
        *reinterpret_cast<i32 *>(effect + 0x348) = g_EffectTemplates[id].field348;
        *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2B0) = *velocity;

        if (g_EffectTemplates[id].callback != NULL)
        {
            if (g_EffectTemplates[id].callback(reinterpret_cast<AnmVm *>(effect)) != 0)
            {
                *reinterpret_cast<i8 *>(effect + 0x350) = 0;
            }
        }

        count--;
        if (count == 0)
        {
            break;
        }

        if (*reinterpret_cast<i32 *>(this) == 0)
        {
            effect = reinterpret_cast<u8 *>(this) + 0x1C;
        }
        else
        {
            effect += 0x360;
        }
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x400;
    return reinterpret_cast<AnmVm *>(i >= 0x200 ? reinterpret_cast<u8 *>(this) + 0x89BFC : effect);
}

// FUNCTION: th08 0x425870
#pragma var_order(effect)
AnmVm *EffectManager::FUN_00425870(i32 id, D3DXVECTOR3 *position, i32 slotIndex, i32 unused, i32 color)
{
    u8 *effect = reinterpret_cast<u8 *>(this) + (slotIndex + 0x280) * 0x360 + 0x1C;

    if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
    {
        g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));
    }

    memset(effect, 0, 0x360);
    *reinterpret_cast<i32 *>(effect + 0x328) = slotIndex;
    *reinterpret_cast<i8 *>(effect + 0x350) = 1;
    *reinterpret_cast<i8 *>(effect + 0x351) = id;
    *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2A4) = *position;

    if (g_EffectTemplates[id].scriptIdx >= 0)
    {
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0x8B054))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(effect), g_EffectTemplates[id].scriptIdx);
    }

    *reinterpret_cast<u32 *>(effect + 0x1F8) |= 0x2000;
    *reinterpret_cast<i32 *>(effect + 0x1F0) = color;
    *reinterpret_cast<i32 *>(effect + 0x288) = 0;
    *reinterpret_cast<i32 *>(effect + 0x28C) = 0;
    *reinterpret_cast<i32 *>(effect + 0x290) = 0;
    *reinterpret_cast<i32 *>(effect + 0x348) = g_EffectTemplates[id].field348;

    if (g_EffectTemplates[id].callback != NULL &&
        g_EffectTemplates[id].callback(reinterpret_cast<AnmVm *>(effect)) != 0)
    {
        *reinterpret_cast<i8 *>(effect + 0x350) = 0;
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x400;
    return reinterpret_cast<AnmVm *>(effect);
}

// FUNCTION: th08 0x426d10
#pragma var_order(effect, i, delta)
void __fastcall FUN_00426d10(Float3 *delta)
{
    u8 *effect = reinterpret_cast<u8 *>(&g_EffectManager) + 0x1C;
    i32 i;

    for (i = 0; i < 0x200; i++, effect += 0x360)
    {
        if (*reinterpret_cast<i8 *>(effect + 0x351) == 0x33)
        {
            *reinterpret_cast<Float3 *>(effect + 0x2D4) += *delta;
        }
    }
}

// FUNCTION: th08 0x425b70
#pragma var_order(effect, i, zeroVector)
AnmVm *EffectManager::SpawnEffect00425B70(i32 id, D3DXVECTOR3 *position, i32 count, i32 color)
{
    u8 *effect = reinterpret_cast<u8 *>(this) + 0x6C01C;
    i32 i;

    for (i = 0; i < 0x80; i++, effect += 0x360)
    {
        if (*reinterpret_cast<i8 *>(effect + 0x350) != 0)
        {
            continue;
        }

        if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
        {
            g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));
        }
        *reinterpret_cast<void **>(effect + 0x358) = NULL;
        *reinterpret_cast<i32 *>(effect + 0x34C) = 0;
        *reinterpret_cast<i8 *>(effect + 0x354) = 0;
        *reinterpret_cast<i8 *>(effect + 0x350) = 1;
        *reinterpret_cast<i8 *>(effect + 0x351) = id;
        *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2A4) = *position;
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0x8B054))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(effect), g_EffectTemplates[id].scriptIdx);
        *reinterpret_cast<i32 *>(effect + 0x1F0) = color;
        *reinterpret_cast<i32 *>(effect + 0x288) = 0;
        *reinterpret_cast<i32 *>(effect + 0x28C) = 0;
        *reinterpret_cast<i32 *>(effect + 0x290) = 0;
        *reinterpret_cast<i32 *>(effect + 0x348) = g_EffectTemplates[id].field348;
        *reinterpret_cast<ZunTimer *>(effect + 0x338) = 0;
        *reinterpret_cast<i8 *>(effect + 0x352) = 0;
        *reinterpret_cast<i8 *>(effect + 0x353) = 0;
        *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2B0) = D3DXVECTOR3(0, 0, 0);

        if (g_EffectTemplates[id].callback != NULL)
        {
            if (g_EffectTemplates[id].callback(reinterpret_cast<AnmVm *>(effect)) != 0)
            {
                *reinterpret_cast<i8 *>(effect + 0x350) = 0;
            }
        }

        count--;
        if (count == 0)
        {
            break;
        }
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x400;
    return reinterpret_cast<AnmVm *>(i >= 0x80 ? reinterpret_cast<u8 *>(this) + 0x89BFC : effect);
}

} // namespace th08
