#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "AnmManager.hpp"
#include "Background.hpp"
#include "ItemManager.hpp"
#include "ReplayManager.hpp"
#include "GameManager.hpp"
#include "EnemyManager.hpp"

namespace th08
{

#ifdef TH08_MODERN_PORT
#define g_EclExBarrierRenderState (g_Background.EclExBarrierState())
#endif

ZunBool IsDisableResourceReload();

void __fastcall FUN_00428310(AnmVm *effect, D3DXVECTOR3 *base);
i32 __fastcall FUN_00428720(Effect *effect);
i32 __fastcall FUN_00427450(Effect *effect);


























DIFFABLE_STATIC(EffectManager, g_EffectManager);
DIFFABLE_STATIC(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EffectManagerDrawChain);

// Target 0x004E4B64 is owned by Gui.cpp but participates in effect-resource setup.
extern i32 g_GuiMessageStageMode;














extern f32 g_EclGameTimeScale;








struct EffectTemplate
{
    i32 scriptIdx;
    i32 field348;
    i32 (__fastcall *callback)(AnmVm *effect);
};
DIFFABLE_STATIC_ARRAY(EffectTemplate, 20, g_EffectTemplates);

// FUNCTION: th08 0x423d70
Float3 *Float3::operator*=(f32 scalar)
{
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    return this;
}

// FUNCTION: th08 0x4253e0
AnmVm *EffectManager::FUN_004253e0(i32 index)
{
    return &this->effects[index + 0x280].vm;
}

// FUNCTION: th08 0x425410
void EffectManager::ResetEffects()
{
    memset(this, 0, 0x8B05C);
}

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

// FUNCTION: th08 0x4259e0
#pragma var_order(effect)
AnmVm *EffectManager::FUN_004259e0(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 slotIndex,
                                   i32 unused, i32 color)
{
    u8 *effect = reinterpret_cast<u8 *>(this) + (slotIndex + 0x280) * 0x360 + 0x1C;

    if (*reinterpret_cast<void **>(effect + 0x358) != NULL)
        g_ZunMemory.Free(*reinterpret_cast<void **>(effect + 0x358));

    memset(effect, 0, 0x360);
    *reinterpret_cast<i32 *>(effect + 0x328) = slotIndex;
    *reinterpret_cast<D3DXVECTOR3 *>(effect + 0x2B0) = *velocity;
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

// FUNCTION: th08 0x425e60
i32 __fastcall EffectRandomSplashUpdate(AnmVm *effect)
{
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2a4) +=
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2bc) +=
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(effect) + 0x2c8);
    return 1;
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

// FUNCTION: th08 0x425fa0
Float3 Float3::operator-() const
{
    return Float3(-this->x, -this->y, -this->z);
}

// FUNCTION: th08 0x425fe0
i32 __fastcall EffectOrbitInit(AnmVm *effect)
{
    *reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x354) = 2;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2ec) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2f0) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x2f4) = 0.0f;
    reinterpret_cast<Effect *>(effect)->radius = 0.0f;
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
    verticalAngle = sinf(reinterpret_cast<Effect *>(effect)->angle);
    horizontalAngle = cosf(reinterpret_cast<Effect *>(effect)->angle);
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
    posOffset *= reinterpret_cast<Effect *>(effect)->radius;
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

// FUNCTION: th08 0x426280
#pragma var_order(backgroundOffset, effect)
i32 __fastcall FUN_00426280(Effect *effect)
{
    Float3 backgroundOffset;

    backgroundOffset = -g_Background.unk6394.vector1;
    effect->vector4 = g_Background.unk6394.vector1 + g_Background.unk6394.vector0;
    effect->vector4.x += g_Rng.GetRandomF32SignedInRange(60.0f) + backgroundOffset.x / 2.0f;
    effect->vector4.y += g_Rng.GetRandomF32SignedInRange(100.0f) - 50.0f + backgroundOffset.y / 2.0f;
    effect->vector4.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + backgroundOffset.z / 2.0f;

    effect->vector2.x = g_Rng.GetRandomF32SignedInRange(0.001f) + effect->vector1.x;
    effect->vector2.y = g_Rng.GetRandomF32SignedInRange(0.03f) + effect->vector1.y;
    effect->vector2.z = -g_Rng.GetRandomF32InRange(0.1f) - 0.3f + effect->vector1.z;
    effect->vector3.x = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.y = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.z = -0.0003f;
    effect->vector2 = effect->vector2 * g_EclGameTimeScale;
    effect->vector3 = effect->vector3 * g_EclGameTimeScale;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x354) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x288) = -9999.0f;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x238) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x244) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x248) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x24c) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x250) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x254) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x258) = 0;
    return 0;
}

// FUNCTION: th08 0x4264f0
#pragma var_order(delta, dot, effect)
i32 __fastcall FUN_004264f0(Effect *effect)
{
    f32 dot;

    effect->vector2 += effect->vector3;
    effect->vector4 += effect->vector2;
    effect->vector0 = effect->vector4;

    Float3 delta;
    delta = effect->vector0 - g_Background.unk6394.vector0;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&delta), reinterpret_cast<D3DXVECTOR3 *>(&delta));
    dot = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&g_Background.unk6394.vector3),
                      reinterpret_cast<D3DXVECTOR3 *>(&delta));
    if (dot < 0.94f)
        return 0;

    if (g_EnemyManager.HasBoss())
    {
        if (((g_EnemyManager.bosses[0]->flags1 >>
              ENEMY_FLAG_DAMAGEABLE_SHIFT) & 1) != 0)
        {
            if (effect->vm.pos2.x <= -9999.0f)
            {
                effect->vm.pos2 = g_EnemyManager.bosses[0]->position;
            }
            else
            {
                effect->vm.pos2 =
                    (g_EnemyManager.bosses[0]->position -
                     effect->vm.pos2) * 0.1f + effect->vm.pos2;
            }
        }
    }

    *reinterpret_cast<u32 *>(&effect->vm.flags) |= 0x20000;
    effect->vm.color2.r = ((u32)effect->vm.color1.r *
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(&g_Background) + 0xa34)->r) >> 8;
    effect->vm.color2.g = ((u32)effect->vm.color1.g *
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(&g_Background) + 0xa34)->g) >> 8;
    effect->vm.color2.b = ((u32)effect->vm.color1.b *
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(&g_Background) + 0xa34)->b) >> 8;
    effect->vm.color2.a = ((u32)effect->vm.color1.a *
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(&g_Background) + 0xa34)->a) >> 8;
    return 1;
}

// FUNCTION: th08 0x426720
#pragma var_order(backgroundOffset, effect)
i32 __fastcall FUN_00426720(Effect *effect)
{
    Float3 backgroundOffset;

    backgroundOffset = -g_Background.unk6394.vector1;
    effect->vector4 = g_Background.unk6394.vector1 + g_Background.unk6394.vector0;
    effect->vector4.x += g_Rng.GetRandomF32SignedInRange(60.0f) + backgroundOffset.x / 2.0f;
    effect->vector4.y += g_Rng.GetRandomF32SignedInRange(200.0f) - 200.0f + backgroundOffset.y / 2.0f;
    effect->vector4.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + backgroundOffset.z / 2.0f;

    effect->vector2.x = g_Rng.GetRandomF32SignedInRange(0.001f) + effect->vector1.x;
    effect->vector2.y = g_Rng.GetRandomF32SignedInRange(0.03f) + 0.4f;
    effect->vector2.z = -g_Rng.GetRandomF32InRange(0.1f) - 0.3f + effect->vector1.z;
    effect->vector3.x = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.y = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.z = -0.0003f;
    effect->vector2 = effect->vector2 * g_EclGameTimeScale;
    effect->vector3 = effect->vector3 * g_EclGameTimeScale;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x354) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x288) = -9999.0f;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x238) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x244) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x248) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x24c) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x250) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x254) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x258) = 0;
    return 0;
}

// FUNCTION: th08 0x426990
#pragma var_order(delta, dot, effect)
i32 __fastcall FUN_00426990(Effect *effect)
{
    f32 dot;

    effect->vector2 += effect->vector3;
    effect->vector4 += effect->vector2;
    effect->vector0 = effect->vector4;

    Float3 delta;
    delta = effect->vector0 - g_Background.unk6394.vector0;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&delta), reinterpret_cast<D3DXVECTOR3 *>(&delta));
    dot = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&g_Background.unk6394.vector3),
                      reinterpret_cast<D3DXVECTOR3 *>(&delta));
    if (dot < 0.94f)
        return 0;

    if (g_EnemyManager.bosses[0] != NULL)
    {
        if (((g_EnemyManager.bosses[0]->flags1 >>
              ENEMY_FLAG_DAMAGEABLE_SHIFT) & 1) != 0)
        {
            if (effect->vm.pos2.x <= -9999.0f)
            {
                effect->vm.pos2 = g_EnemyManager.bosses[0]->position;
            }
            else
            {
                effect->vm.pos2 =
                    (g_EnemyManager.bosses[0]->position -
                     effect->vm.pos2) * 0.1f + effect->vm.pos2;
            }
        }
    }
    return 1;
}

// FUNCTION: th08 0x426b20
#pragma var_order(angle, effect)
i32 __fastcall FUN_00426b20(Effect *effect)
{
    f32 angle;

    effect->vector5 = effect->vector0;
    effect->vector5.z = 0.0f;
    angle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
    effect->vector6.x = cosf(angle);
    effect->vector6.y = sinf(angle);
    effect->vector6.z = 0.0f;
    return 0;
}

// FUNCTION: th08 0x426bb0
#pragma var_order(alpha, effect)
i32 __fastcall FUN_00426bb0(Effect *effect)
{
    f32 alpha;

    alpha = 256.0f - (f32)effect->timer * 256.0f / 60.0f;
    effect->vector0 = effect->vector6 * alpha + effect->vector5;
    effect->vector0.z = 0.0f;
    return 1;
}

// FUNCTION: th08 0x426c40
i32 __fastcall FUN_00426c40(Effect *effect)
{
    if (FUN_00428720(effect))
        return 0;

    reinterpret_cast<EclOperands::Vector3 &>(effect->vector0) =
        EclOperands::g_TargetPlayerPosition017D61AC;
    return 1;
}

// FUNCTION: th08 0x426c90
#pragma var_order(alpha, effect)
i32 __fastcall FUN_00426c90(Effect *effect)
{
    f32 alpha;

    alpha = 256.0f - (f32)effect->timer * 256.0f / 240.0f;
    effect->vector0 = effect->vector6 * alpha + effect->vector5;
    return 1;
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

// FUNCTION: th08 0x426d70
#pragma var_order(delta, dot, effect)
i32 __fastcall FUN_00426d70(Effect *effect)
{
    f32 dot;

    effect->vector2 += effect->vector3;
    effect->vector4 += effect->vector2;
    effect->vector0 = effect->vector4;

    Float3 delta;
    delta = effect->vector0 - g_Background.unk6394.vector0;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&delta), reinterpret_cast<D3DXVECTOR3 *>(&delta));
    dot = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&g_Background.unk6394.vector3),
                      reinterpret_cast<D3DXVECTOR3 *>(&delta));
    if (dot < 0.94f)
        return 0;

    effect->vm.SetZRotation(AddNormalizeAngle(effect->vm.rotation.z, effect->vm.rotation.x));
    if (effect->vector0.z >= 0.0f)
        return 0;
    return 1;
}

// FUNCTION: th08 0x426e70
#pragma var_order(backgroundOffset, effect)
i32 __fastcall FUN_00426e70(Effect *effect)
{
    Float3 backgroundOffset;

    backgroundOffset = -g_Background.unk6394.vector1;
    effect->vector4 = g_Background.unk6394.vector1 + g_Background.unk6394.vector0;
    effect->vector4.x += g_Rng.GetRandomF32InRange(120.0f) - 60.0f + backgroundOffset.x / 2.0f;
    effect->vector4.y += g_Rng.GetRandomF32InRange(200.0f) - 100.0f + backgroundOffset.y / 2.0f;
    effect->vector4.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + backgroundOffset.z / 2.0f;

    effect->vector2.x = g_Rng.GetRandomF32InRange(0.06f) - 0.03f + effect->vector1.x;
    effect->vector2.y = g_Rng.GetRandomF32InRange(0.06f) - 0.03f + effect->vector1.y;
    effect->vector2.z = g_Rng.GetRandomF32InRange(0.1f) + 0.03f + effect->vector1.z;
    effect->vector3.x = g_Rng.GetRandomF32InRange(0.0002f) - 0.0001f;
    effect->vector3.y = g_Rng.GetRandomF32InRange(0.0002f) - 0.0001f;
    effect->vector2 = effect->vector2 * g_EclGameTimeScale;
    effect->vector3 = effect->vector3 * g_EclGameTimeScale;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x354) = 1;
    effect->vm.rotation.z = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
    effect->vm.rotation.x = g_Rng.GetRandomF32InRange(0.03141592815518379f) - 0.015707964077591896f;
    return 0;
}

// FUNCTION: th08 0x4270c0
#pragma var_order(angle, effect)
i32 __fastcall FUN_004270c0(Effect *effect)
{
    f32 angle;

    if (effect->vector1.x > -990.0)
        angle = AddNormalizeAngle(effect->vector1.x, 0.0f);
    else
        angle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;

    effect->vector5 = effect->vector0;
    effect->vector5.z = 0.0f;
    effect->vector6.x = cosf(angle);
    effect->vector6.y = sinf(angle);
    effect->vector6.z = 0.0f;
    effect->vector6 *= g_Rng.GetRandomF32InRange(1.5f) + 0.0f;
    return 0;
}

// FUNCTION: th08 0x4271a0
#pragma var_order(alpha, effect)
i32 __fastcall FUN_004271a0(Effect *effect)
{
    f32 alpha;

    alpha = (f32)effect->timer / 90.0f;
    alpha = 1.0f - (1.0f - alpha) * (1.0f - alpha);
    effect->vector0 = effect->vector6 * alpha * 128.0f + effect->vector5;
    effect->vector0.z = 0.0f;
    return 1;
}

// FUNCTION: th08 0x427250
i32 __fastcall FUN_00427250(Effect *effect)
{
    return 1;
}

// FUNCTION: th08 0x427260
#pragma var_order(offset, effect)
i32 __fastcall FUN_00427260(Effect *effect)
{
    Float3 offset;

    offset.FromAngleMagnitude(effect->vector1.x, 256.0f);
    effect->vector0.x += offset.x;
    effect->vector0.y += offset.y;
    effect->vm.rotation.z = AddNormalizeAngle(effect->vector1.x, ZUN_PI / 2.0f);
    return 0;
}

// FUNCTION: th08 0x4272e0
i32 __fastcall FUN_004272e0(Effect *effect)
{
    *reinterpret_cast<VertexTex1DiffuseXyzrhw **>(reinterpret_cast<u8 *>(effect) + 0x358) =
        static_cast<VertexTex1DiffuseXyzrhw *>(g_ZunMemory.Alloc(0x1c38, "Effect"));
    if (*reinterpret_cast<VertexTex1DiffuseXyzrhw **>(reinterpret_cast<u8 *>(effect) + 0x358) == NULL)
        return -1;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 3;
    effect->vector5 = effect->vector0;
    effect->vector6.x = 0.0f;
    effect->vector6.y = 0.0f;
    effect->vector6.z = 1.0f;
    effect->vector7.x = 0.0f;
    effect->vector7.y = -1.0f;
    effect->vector7.z = 0.0f;
    effect->angle = effect->vector1.x;
    effect->radius = effect->vector1.y;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = effect->vector1.z;

    g_AnmManager->FUN_004649a0(
        reinterpret_cast<AnmVm *>(effect),
        *reinterpret_cast<VertexTex1DiffuseXyzrhw **>(reinterpret_cast<u8 *>(effect) + 0x358),
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) * 2);
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<void **>(reinterpret_cast<u8 *>(effect) + 0x34c) = reinterpret_cast<void *>(&FUN_00427450);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = 0.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) = 0.0f;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 24;
    return 0;
}

// FUNCTION: th08 0x427450
#pragma var_order(i, innerRadius, vertex, angleStep, radius)
i32 __fastcall FUN_00427450(Effect *effect)
{
    i32 i;
    f32 innerRadius;
    VertexTex1DiffuseXyzrhw *vertex;
    f32 angleStep;
    f32 radius;

    if (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(effect) + 0x356))
    {
        angleStep = ZUN_2PI / *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324);
        radius = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) /
                 sinf((ZUN_PI - angleStep) / 2.0f);
        vertex = *reinterpret_cast<VertexTex1DiffuseXyzrhw **>(reinterpret_cast<u8 *>(effect) + 0x358);
        g_AnmManager->FUN_00464b00(
            reinterpret_cast<AnmVm *>(effect),
            *reinterpret_cast<VertexTex1DiffuseXyzrhw **>(reinterpret_cast<u8 *>(effect) + 0x358),
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) * 2 + 2);

        if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) == 0.0f)
        {
            f32 angle;
            angle = effect->angle;
            innerRadius = effect->radius - radius;
            radius += effect->radius;
            for (i = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) + 1; i > 0; --i)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;

                vertex->pos.z = 0.0f;
                vertex->pos.FromAngleMagnitude(angle, radius);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex++;

                vertex->pos.z = 0.0f;
                vertex->pos.FromAngleMagnitude(angle, innerRadius);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex++;

                angle += angleStep;
            }
        }
        else if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) == 0.0f)
        {
#pragma var_order(innerEllipseRadius, outerEllipseRadius, angle, point)
            f32 angle = 0.0f;
            Float3 point;
            f32 outerEllipseRadius;
            f32 innerEllipseRadius;

            outerEllipseRadius = radius + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c);
            innerEllipseRadius = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) - radius;
            innerRadius = effect->radius - radius;
            radius += effect->radius;

            for (i = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) + 1; i > 0; --i)
            {
                point.FromRotatedVec2(angle, radius, outerEllipseRadius);
                Rotate(&vertex->pos, &point, *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330));
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                point.FromRotatedVec2(angle, innerRadius, innerEllipseRadius);
                Rotate(&vertex->pos, &point, *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330));
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                angle += angleStep;
            }
        }
        else
        {
#pragma var_order(secondAngleStep, secondAngle, radialOffset, angle, unused)
            f32 secondAngle;
            f32 angle;
            f32 secondAngleStep;
            f32 radialOffset;

            secondAngle = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330);
            angle = effect->angle;
            secondAngleStep = ZUN_2PI * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) /
                              *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324);
            Float3 unused;
            innerRadius = effect->radius - radius;
            radius += effect->radius;

            for (i = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) + 1; i > 0; --i)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;
                if (secondAngle >= ZUN_PI)
                    secondAngle -= ZUN_2PI;

                radialOffset = cosf(secondAngle) *
                               *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c);
                vertex->pos.FromAngleMagnitude(angle, radius + radialOffset);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                vertex->pos.FromAngleMagnitude(angle, innerRadius + radialOffset);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                angle += angleStep;
                secondAngle += secondAngleStep;
            }
        }
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 0;
    }

    g_AnmManager->DrawVertices(
        reinterpret_cast<AnmVm *>(effect),
        *reinterpret_cast<VertexTex1DiffuseXyzrhw **>(reinterpret_cast<u8 *>(effect) + 0x358),
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) * 2 + 2);
    return 1;
}

// FUNCTION: th08 0x427970
i32 __fastcall FUN_00427970(Effect *effect)
{
    FUN_004272e0(effect);
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x355) = 1;
    return 0;
}

// FUNCTION: th08 0x427990
i32 __fastcall FUN_00427990(Effect *effect)
{
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    return 1;
}

// FUNCTION: th08 0x4279d0
i32 __fastcall FUN_004279d0(Effect *effect)
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) =
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x100);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) =
        (f32)*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x104);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = effect->vm.pos.y;
    effect->angle = effect->vm.rotation.z;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = effect->vm.rotation.y;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    return 1;
}

// FUNCTION: th08 0x427a60
i32 __fastcall FUN_00427a60(Effect *effect)
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) = 32;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = effect->vm.pos.y;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    if (effect->timer >= 120)
        return 0;
    return 1;
}

// FUNCTION: th08 0x427ae0
i32 __fastcall FUN_00427ae0(Effect *effect)
{
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x32c) = effect->vm.pos.y;
    effect->angle = effect->vm.rotation.z;
    if (effect->vm.color1.a == 0)
        return 0;
    return 1;
}

// FUNCTION: th08 0x427b50
i32 __fastcall FUN_00427b50(Effect *effect)
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x324) =
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x100);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x334) =
        (f32)*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effect) + 0x104);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x320) = effect->vm.scale.x;
    effect->radius =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x114);
    effect->angle = effect->vm.rotation.z;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effect) + 0x330) = effect->vm.rotation.y;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effect) + 0x356) = 1;
    effect->vector5 = effect->vm.pos;
    return 1;
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
        if (!g_GameManager.flags.unk10 ||
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
            *reinterpret_cast<f32 *>(effect + 0x208) += g_GameManager.arcadeRegionTopLeftPos.x;
            *reinterpret_cast<f32 *>(effect + 0x20C) += g_GameManager.arcadeRegionTopLeftPos.y;
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
            *reinterpret_cast<f32 *>(effect + 0x208) += g_GameManager.arcadeRegionTopLeftPos.x;
            *reinterpret_cast<f32 *>(effect + 0x20C) += g_GameManager.arcadeRegionTopLeftPos.y;
            *reinterpret_cast<f32 *>(effect + 0x210) = 0.07f;
            reinterpret_cast<Float3 *>(effect + 0x208)->operator+=(
                *reinterpret_cast<Float3 *>(effect + 0x288));
            g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(effect));
        }
        effect = *reinterpret_cast<u8 **>(effect + 0x35C);
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
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
            *reinterpret_cast<f32 *>(effect + 0x208) += g_GameManager.arcadeRegionTopLeftPos.x;
            *reinterpret_cast<f32 *>(effect + 0x20c) += g_GameManager.arcadeRegionTopLeftPos.y;
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

// FUNCTION: th08 0x428310
#pragma var_order(delta, point)
void __fastcall FUN_00428310(AnmVm *effect, D3DXVECTOR3 *base)
{
    D3DXVECTOR3 delta;
    D3DXVECTOR3 point;

    if (!g_GameManager.isInGameMenu && !g_GameManager.showRetryMenu)
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

// FUNCTION: th08 0x4284b0
ZunResult EffectManager::AddedCallback(EffectManager *effectManager)
{
    effectManager->ResetEffects();
    *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B054) = g_AnmManager->GetAnm(6);
    g_GuiMessageStageMode = 0;
    g_EclExBarrierRenderState.mode = 2;

    if (!IsDisableResourceReload())
    {
        if (!g_GameManager.IsSpellPractice() || g_GameManager.currentSpellCardNumber < 216)
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B058) =
                g_AnmManager->PreloadAnm(9, g_EffectAnms[g_GameManager.currentStage]);
        }
        else
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(effectManager) + 0x8B058) =
                g_AnmManager->PreloadAnm(
                    9, g_EffectAnms[g_GameManager.currentSpellCardNumber - 216 + 9]);
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

// FUNCTION: th08 0x4286b0
void EffectManager::CutChain()
{
    g_Chain.Cut(&g_EffectManagerCalcChain);
    g_Chain.Cut(&g_EffectManagerDrawChain);
}

// FUNCTION: th08 0x428720
i32 __fastcall FUN_00428720(Effect *effect)
{
    return effect->vm.currentInstruction == NULL;
}

// FUNCTION: th08 0x428740
EffectManager::EffectManager()
{
    this->ResetEffects();
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->scaleZ = 1.0f;
    this->scaleW = 1.0f;
}

// FUNCTION: th08 0x4287e0
Effect::Effect()
{
}


#ifdef TH08_MODERN_PORT
#undef g_EclExBarrierRenderState
#endif

} // namespace th08
