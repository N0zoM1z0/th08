#include "th_pch.h"

#include "ScreenEffect.hpp"
#include "AnmManager.hpp"
#include "GameManager.hpp"

namespace th08
{

DIFFABLE_STATIC(i32, g_ScreenEffectCounter);
DIFFABLE_STATIC(ScreenEffect, g_ScreenEffect);

// FUNCTION: th08 0x45b000
ScreenEffect::ScreenEffect()
{
}

// FUNCTION: th08 0x45b020
void ScreenEffect::Clear(D3DCOLOR color)
{
    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
    }
    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
    }
}

// FUNCTION: th08 0x45b0e0
void ScreenEffect::SetViewport(D3DCOLOR clearColor)
{
    if (g_AnmManager != NULL)
    {
        g_AnmManager->FlushVertexBuffer();
    }

    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.viewport.MinZ = 0.0f;
    g_Supervisor.viewport.MaxZ = 1.0f;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    ScreenEffect::Clear(clearColor);
}

// FUNCTION: th08 0x45b160
ChainCallbackResult ScreenEffect::CalcFadeIn(ScreenEffect *screenEffect)
{
    if (screenEffect->duration != 0)
    {
        screenEffect->arcadeFadeAlpha =
            (i32)(255.0f - ((255.0f * (f32)screenEffect->timer) / screenEffect->duration));
        if (screenEffect->arcadeFadeAlpha < 0)
        {
            screenEffect->arcadeFadeAlpha = 0;
        }
    }

    if (screenEffect->timer >= screenEffect->duration)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    screenEffect->timer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x45b1e0
void ScreenEffect::DrawSquare(ZunRect *rectDimensions, D3DCOLOR color)
{
}

// STUB: th08 0x45b490
void ScreenEffect::DrawSquareShaded(ZunRect *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft,
                                    D3DCOLOR bottomRight)
{
}

// FUNCTION: th08 0x45b760
ChainCallbackResult ScreenEffect::CalcFadeOut(ScreenEffect *screenEffect)
{
    if (g_ScreenEffectCounter != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    if (screenEffect->duration != 0)
    {
        screenEffect->arcadeFadeAlpha = (i32)((255.0f * (f32)screenEffect->timer) / screenEffect->duration);
        if (screenEffect->arcadeFadeAlpha < 0)
        {
            screenEffect->arcadeFadeAlpha = 0;
        }
    }

    if (screenEffect->timer >= screenEffect->duration)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    if (g_GameManager.isInGameMenu == 0 && g_GameManager.showRetryMenu == 0)
    {
        screenEffect->timer++;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x45b8b0
ScreenEffect *ScreenEffect::RegisterChain(ScreenEffectType effect, i32 ticks, i32 param_3, i32 param_4, i32 param_5,
                                          i32 param_6)
{
    return NULL;
}

// FUNCTION: th08 0x45bb50
ChainCallbackResult ScreenEffect::DrawFullFade(ScreenEffect *screenEffect)
{
    ZunRect rect;

    rect.left = 0.0f;
    rect.top = 0.0f;
    rect.right = 640.0f;
    rect.bottom = 480.0f;

    g_AnmManager->FlushVertexBuffer();
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    ScreenEffect::DrawSquare(&rect, (screenEffect->arcadeFadeAlpha << 24) | screenEffect->arcadeFadeColor);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ScreenEffect::DrawArcadeFade(ScreenEffect *screenEffect)
{
    ZunRect rect;

    rect.left = 32.0f;
    rect.top = 16.0f;
    rect.right = 416.0f;
    rect.bottom = 464.0f;
    ScreenEffect::DrawSquare(&rect, (screenEffect->arcadeFadeAlpha << 24) | screenEffect->arcadeFadeColor);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x45bdc0
ChainCallbackResult ScreenEffect::CalcShake(ScreenEffect *screenEffect)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult ScreenEffect::AddedCallback(ScreenEffect *screenEffect)
{
    screenEffect->timer = 0;
    return ZUN_SUCCESS;
}

ZunResult ScreenEffect::DeletedCallback(ScreenEffect *screenEffect)
{
    screenEffect->calcChainElement->deletedCallback = NULL;
    g_Chain.Cut(screenEffect->drawChainElement);
    screenEffect->drawChainElement = NULL;
    ZUN_DELETE(screenEffect);

    return ZUN_SUCCESS;
}

} /* namespace th08 */
