#include "th_pch.h"

#include "EclManager.hpp"
#include "AnmManager.hpp"
#include "ReplayManager.hpp"

namespace th08
{

DIFFABLE_STATIC(EffectManager, g_EffectManager);
DIFFABLE_STATIC(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EffectManagerDrawChain);

// FUNCTION: th08 0x4286b0
void EffectManager::CutChain()
{
    g_Chain.Cut(&g_EffectManagerCalcChain);
    g_Chain.Cut(&g_EffectManagerDrawChain);
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
