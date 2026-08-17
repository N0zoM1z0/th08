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

// FUNCTION: th08 0x45b1e0
void ScreenEffect::DrawSquare(ZunRect *rectDimensions, D3DCOLOR color)
{
    g_AnmManager->FlushVertexBuffer();

    VertexDiffuseXyzrhw vertices[4];

    vertices[0].pos = Float3(rectDimensions->left, rectDimensions->top, 0.0f);
    vertices[1].pos = Float3(rectDimensions->right, rectDimensions->top, 0.0f);
    vertices[2].pos = Float3(rectDimensions->left, rectDimensions->bottom, 0.0f);
    vertices[3].pos = Float3(rectDimensions->right, rectDimensions->bottom, 0.0f);

    vertices[3].w = 1.0f;
    vertices[2].w = vertices[3].w;
    vertices[1].w = vertices[2].w;
    vertices[0].w = vertices[1].w;

    vertices[3].diffuse = color;
    vertices[2].diffuse = vertices[3].diffuse;
    vertices[1].diffuse = vertices[2].diffuse;
    vertices[0].diffuse = vertices[1].diffuse;

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrhw));

    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

// FUNCTION: th08 0x45b490
void ScreenEffect::DrawSquareShaded(ZunRect *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft,
                                    D3DCOLOR bottomRight)
{
    g_AnmManager->FlushVertexBuffer();

    VertexDiffuseXyzrhw vertices[4];

    vertices[0].pos = Float3(rect->left, rect->top, 0.0f);
    vertices[1].pos = Float3(rect->right, rect->top, 0.0f);
    vertices[2].pos = Float3(rect->left, rect->bottom, 0.0f);
    vertices[3].pos = Float3(rect->right, rect->bottom, 0.0f);

    vertices[3].w = 1.0f;
    vertices[2].w = vertices[3].w;
    vertices[1].w = vertices[2].w;
    vertices[0].w = vertices[1].w;

    vertices[0].diffuse = topLeft;
    vertices[1].diffuse = topRight;
    vertices[2].diffuse = bottomLeft;
    vertices[3].diffuse = bottomRight;

    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrhw));

    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();

    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
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


// FUNCTION: th08 0x45b800
ChainCallbackResult ScreenEffect::CalcPartialFadeOut(ScreenEffect *screenEffect)
{
    if (screenEffect->unk24 == 0)
    {
        if (screenEffect->duration != 0 && screenEffect->timer <= screenEffect->duration)
        {
            screenEffect->arcadeFadeAlpha = (i32)(((f32)screenEffect->timer * 128.0f) / screenEffect->duration);
        }
    }
    else
    {
        if (screenEffect->timer <= 8)
        {
            screenEffect->arcadeFadeAlpha = 128 - (i32)(((f32)screenEffect->timer * 128.0f) / 8.0f);
        }
        else
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
    }

    screenEffect->timer++;
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


// FUNCTION: th08 0x45bbf0
ChainCallbackResult ScreenEffect::DrawPartialFade(ScreenEffect *screenEffect)
{
    ZunRect rect;

    rect.left = 0.0f;
    rect.top = 0.0f;
    rect.right = 640.0f;
    rect.bottom = 480.0f;
    ScreenEffect::DrawSquare(&rect, (screenEffect->arcadeFadeAlpha << 24) | screenEffect->arcadeFadeColor);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x45bd70
ChainCallbackResult ScreenEffect::FUN_0045bd70(ScreenEffect *screenEffect)
{
    ZunRect rect;

    rect.left = 32.0f;
    rect.top = 16.0f;
    rect.right = 416.0f;
    rect.bottom = 464.0f;
    ScreenEffect::DrawSquare(
        &rect, (screenEffect->arcadeFadeAlpha << 24) |
                       (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(screenEffect) + 0x1C) & 0xFFFFFF));
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x45bdc0
ChainCallbackResult ScreenEffect::CalcShake(ScreenEffect *screenEffect)
{
    f32 shakeAmount;

    if ((*reinterpret_cast<u32 *>(0x164D0B4) >> 10) & 1)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_GameManager.unk2C != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_ScreenEffectCounter != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    screenEffect->timer++;
    if (screenEffect->timer >= screenEffect->duration)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    shakeAmount = (f32)screenEffect->timer *
                  (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(screenEffect) + 0x1c) -
                   *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(screenEffect) + 0x18));
    shakeAmount = shakeAmount / screenEffect->duration;
    shakeAmount = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(screenEffect) + 0x18) + shakeAmount;

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_AnmManager->screenShakeOffset.x = 0.0f;
        break;
    case 1:
        g_AnmManager->screenShakeOffset.x = shakeAmount;
        break;
    case 2:
        g_AnmManager->screenShakeOffset.x = -shakeAmount;
        break;
    }

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_AnmManager->screenShakeOffset.y = 0.0f;
        break;
    case 1:
        g_AnmManager->screenShakeOffset.y = shakeAmount;
        break;
    case 2:
        g_AnmManager->screenShakeOffset.y = -shakeAmount;
        break;
    }

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
