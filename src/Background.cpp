#include "th_pch.h"

#include "AnmManager.hpp"
#include "Background.hpp"
#include "Gui.hpp"
#include "ScreenEffect.hpp"
#include "EclManager.hpp"
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


// FUNCTION: th08 0x409640
#pragma var_order(zValue, alpha, rect, i, background)
ChainCallbackResult Background::OnDrawLowPrio(Background *background)
{
    ZunRect rect;
    i32 i;
    i32 alpha;
    f32 zValue;

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) <= 1 && !g_Gui.IsDialogPresent())
    {
        background->RenderObjects(2);
        background->RenderObjects(3);
        if (!g_Supervisor.IsFogDisabled())
        {
            g_Supervisor.DisableFog();
        }
        g_EffectManager.FUN_004281e0();
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) == 1)
        {
            rect.left = 32.0f;
            rect.top = 16.0f;
            rect.right = 416.0f;
            rect.bottom = 464.0f;
            alpha = (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB28) * 255) / 60;
            g_AnmManager->FlushVertexBuffer();
            g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
            if (!g_Supervisor.IsFogDisabled())
            {
                g_Supervisor.SetRenderState(D3DRS_FOGENABLE, FALSE);
            }
            ScreenEffect::DrawSquare(&rect, alpha << 24);
        }
    }

    g_AnmManager->FlushVertexBuffer();
    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.DisableFog();
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) >= 1)
    {
        for (i = 0; i < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB30); i++)
        {
            g_AnmManager->Draw2DAndFlush(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(background) + 0xB38 + i * sizeof(AnmVm)));
        }
        if (background->onDrawLowPrioCallback != NULL)
        {
            background->onDrawLowPrioCallback();
        }
    }

    g_AnmManager->SetCameraMode(0);
    background->SetCamera1();
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    zValue = 1000.0f;
    g_Supervisor.SetRenderState(D3DRS_FOGSTART, *reinterpret_cast<u32 *>(&zValue));
    zValue = 2000.0f;
    g_Supervisor.SetRenderState(D3DRS_FOGEND, *reinterpret_cast<u32 *>(&zValue));
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x646C) == 0)
    {
        g_AnmManager->SetMixColorDefault();
    }
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x646C) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x647C) = 0;
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

// STUB: th08 0x40a1b0
void Background::RenderObjects(i32 mode)
{
}

// STUB: th08 0x409ce0
ZunResult Background::LoadStageData()
{
    return ZUN_ERROR;
}

}; // Namespace th08
