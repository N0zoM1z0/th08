#include "th_pch.h"

#include "EclManager.hpp"

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


// STUB: th08 0x425430
void EffectManager::SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color)
{
}

} // namespace th08
