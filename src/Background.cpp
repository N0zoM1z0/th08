#include "th_pch.h"

#include "AnmManager.hpp"
#include "Background.hpp"
#include "Supervisor.hpp"

namespace th08
{
ZunBool IsDisableResourceReload();
DIFFABLE_STATIC(Background, g_Background);
DIFFABLE_STATIC(ChainElem, g_BackgroundCalcChain);
DIFFABLE_STATIC(ChainElem, g_BackgroundDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_BackgroundDrawChainLowPrio);

// FUNCTION: th08 0x4071a0
Background::Background()
{
    memset(this, 0, sizeof(Background));
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[0]) = D3DXVECTOR3(0, 0, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[1]) = D3DXVECTOR3(0, 0, 0);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[2]) = D3DXVECTOR3(0, 1.0f, 0);
    this->unk6394.unk48 = 0.5235987901687622f;
    this->unk6264 = this->unk6394;
    this->unk62b0 = this->unk6394;
}

// STUB: th08 0x407400
ChainCallbackResult Background::OnUpdate(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x409200
ChainCallbackResult Background::OnDrawHighPrio(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x409640
ChainCallbackResult Background::OnDrawLowPrio(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x409850
ZunResult Background::AddedCallback(Background *background)
{
    return ZUN_ERROR;
}

// FUNCTION: th08 0x409b20
#pragma var_order(stageAnmSecondary, background)
ZunResult Background::RegisterChain(i32 param)
{
    Background *background = &g_Background;
    void *stageAnmSecondary;

    if (IsDisableResourceReload())
    {
        stageAnmSecondary = background->stageAnmSecondary;
    }

    memset(background, 0, sizeof(Background));

    if (IsDisableResourceReload())
    {
        background->stageAnmSecondary = stageAnmSecondary;
    }

    *(i32 *)((u8 *)background + 0x81C) = 0;
    *(i32 *)((u8 *)background + 0x820) = param;

    g_BackgroundCalcChain.SetCallback((ChainCallback)Background::OnUpdate);
    g_BackgroundCalcChain.addedCallback = (ChainLifetimeCallback)Background::AddedCallback;
    g_BackgroundCalcChain.deletedCallback = (ChainLifetimeCallback)Background::DeletedCallback;
    g_BackgroundCalcChain.arg = background;
    if (g_Chain.AddToCalcChain(&g_BackgroundCalcChain, 8) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_BackgroundDrawChainHighPrio.SetCallback((ChainCallback)Background::OnDrawHighPrio);
    g_BackgroundDrawChainHighPrio.arg = background;
    g_Chain.AddToDrawChain(&g_BackgroundDrawChainHighPrio, 6);

    g_BackgroundDrawChainLowPrio.SetCallback((ChainCallback)Background::OnDrawLowPrio);
    g_BackgroundDrawChainLowPrio.arg = background;
    g_Chain.AddToDrawChain(&g_BackgroundDrawChainLowPrio, 7);

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x40b900
ZunBool IsDisableResourceReload()
{
    return g_Supervisor.unk16c;
}

// FUNCTION: th08 0x409c20
ZunResult Background::DeletedCallback(Background *background)
{
    if (!IsDisableResourceReload())
    {
        g_AnmManager->ReleaseAnm(4);
    }
    if (background->stageAnm != NULL)
    {
        g_ZunMemory.Free(background->stageAnm);
        background->stageAnm = NULL;
    }
    if (!IsDisableResourceReload() && background->stageAnmSecondary != NULL)
    {
        g_ZunMemory.Free(background->stageAnmSecondary);
        background->stageAnmSecondary = NULL;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409ca0
void Background::CutChain()
{
    g_Chain.Cut(&g_BackgroundCalcChain);
    g_Chain.Cut(&g_BackgroundDrawChainHighPrio);
    g_Chain.Cut(&g_BackgroundDrawChainLowPrio);
}

// STUB: th08 0x409ce0
ZunResult Background::LoadStageData()
{
    return ZUN_ERROR;
}

}; // Namespace th08
