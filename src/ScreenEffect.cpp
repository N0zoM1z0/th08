#include "th_pch.h"

#include "ScreenEffect.hpp"
#include "AnmManager.hpp"
#include "EclManager.hpp"
#include "GameManager.hpp"

namespace th08
{

DIFFABLE_STATIC(i32, g_ScreenEffectCounter);
DIFFABLE_STATIC(ScreenEffect, g_ScreenEffect);

namespace
{

struct ScreenEffectRawParameters
{
    i32 primary;
    i32 secondary;
    i32 tertiary;
};

struct ScreenEffectFadeParameters
{
    D3DCOLOR color;
};

struct ScreenEffectShakeParameters
{
    i32 initialAmplitude;
    i32 finalAmplitude;
};

struct ScreenEffectPulseParameters
{
    i32 repeatCount;
    D3DCOLOR color;
};

struct ScreenEffectShakeEnvelopeParameters
{
    i32 rampUpFrames;
    i32 holdFrames;
    i32 rampDownFrames;
};

union ScreenEffectParameters
{
    ScreenEffectRawParameters raw;
    ScreenEffectFadeParameters fade;
    ScreenEffectShakeParameters shake;
    ScreenEffectPulseParameters pulse;
    ScreenEffectShakeEnvelopeParameters shakeEnvelope;
};

C_ASSERT(sizeof(ScreenEffectParameters) == 0xc);
C_ASSERT(sizeof(ScreenEffectRawParameters) == 0xc);
C_ASSERT(offsetof(ScreenEffect, arcadeFadeColor) == 0x18);
C_ASSERT(offsetof(ScreenEffectShakeParameters, finalAmplitude) == 0x4);
C_ASSERT(offsetof(ScreenEffectPulseParameters, color) == 0x4);
C_ASSERT(offsetof(ScreenEffectShakeEnvelopeParameters, holdFrames) == 0x4);
C_ASSERT(offsetof(ScreenEffectShakeEnvelopeParameters, rampDownFrames) == 0x8);

#define SCREEN_EFFECT_PARAMETERS(screenEffect)                                                                         \
    (*reinterpret_cast<ScreenEffectParameters *>(&(screenEffect)->arcadeFadeColor))

}; // Anonymous namespace

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

// FUNCTION: th08 0x45b8b0
#pragma var_order(calcChain, drawChain, screenEffect)
ScreenEffect *ScreenEffect::RegisterChain(ScreenEffectType effect, i32 ticks, i32 primaryParameter,
                                          i32 secondaryParameter, i32 tertiaryParameter, i32 drawPriority)
{
    ChainElem *calcChain = NULL;
    ChainElem *drawChain = NULL;
    ScreenEffect *screenEffect = ZUN_NEW(ScreenEffect, "ScreenInf");

    if (screenEffect == NULL)
        return NULL;

    memset(screenEffect, 0, sizeof(ScreenEffect));

    switch (effect)
    {
    case SCREEN_EFFECT_FULL_FADE_IN:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcFadeIn);
        drawChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawFullFade);
        break;
    case SCREEN_EFFECT_SHAKE:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcShake);
        break;
    case SCREEN_EFFECT_ARCADE_FADE_OUT:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcFadeOut);
        drawChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawArcadeFade);
        break;
    case SCREEN_EFFECT_FULL_FADE_OUT:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcFadeOut);
        drawChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawFullFade);
        break;
    case SCREEN_EFFECT_UNK3:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::FUN_0045bc90);
        drawChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::FUN_0045bd70);
        break;
    case SCREEN_EFFECT_UNK5:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcPartialFadeOut);
        drawChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawPartialFade);
        break;
    case SCREEN_EFFECT_UNK6:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcPartialFadeOut);
        drawChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawArcadeFade);
        break;
    case SCREEN_EFFECT_UNK7:
        calcChain = g_Chain.CreateElem((ChainCallback)ScreenEffect::FUN_0045bf10);
        break;
    }

    calcChain->addedCallback = (ChainLifetimeCallback)ScreenEffect::AddedCallback;
    calcChain->deletedCallback = (ChainLifetimeCallback)ScreenEffect::DeletedCallback;
    calcChain->arg = screenEffect;

    screenEffect->type = effect;
    screenEffect->duration = ticks;
    SCREEN_EFFECT_PARAMETERS(screenEffect).raw.primary = primaryParameter;
    SCREEN_EFFECT_PARAMETERS(screenEffect).raw.secondary = secondaryParameter;
    SCREEN_EFFECT_PARAMETERS(screenEffect).raw.tertiary = tertiaryParameter;

    if (g_Chain.AddToCalcChain(calcChain, 3) != ZUN_SUCCESS)
        return NULL;

    if (drawChain != NULL)
    {
        drawChain->arg = screenEffect;
        g_Chain.AddToDrawChain(drawChain, drawPriority);
    }

    screenEffect->calcChainElement = calcChain;
    screenEffect->drawChainElement = drawChain;
    return screenEffect;
}

// FUNCTION: th08 0x45c160
void ScreenEffect::FUN_0045c160()
{
    this->unk24 = 1;
    this->timer = 0;
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

    ScreenEffect::DrawSquare(
        &rect, (screenEffect->arcadeFadeAlpha << 24) | SCREEN_EFFECT_PARAMETERS(screenEffect).fade.color);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ScreenEffect::DrawPartialFade(ScreenEffect *screenEffect)
{
    ZunRect rect;

    rect.left = 0.0f;
    rect.top = 0.0f;
    rect.right = 640.0f;
    rect.bottom = 480.0f;
    ScreenEffect::DrawSquare(
        &rect, (screenEffect->arcadeFadeAlpha << 24) | SCREEN_EFFECT_PARAMETERS(screenEffect).fade.color);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}


// FUNCTION: th08 0x45bbf0
ChainCallbackResult ScreenEffect::DrawArcadeFade(ScreenEffect *screenEffect)
{
    ZunRect rect;

    rect.left = 32.0f;
    rect.top = 16.0f;
    rect.right = 416.0f;
    rect.bottom = 464.0f;
    ScreenEffect::DrawSquare(
        &rect, (screenEffect->arcadeFadeAlpha << 24) | SCREEN_EFFECT_PARAMETERS(screenEffect).fade.color);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x45bc90
ChainCallbackResult ScreenEffect::FUN_0045bc90(ScreenEffect *screenEffect)
{
    u32 alpha = (SCREEN_EFFECT_PARAMETERS(screenEffect).pulse.color >> 24) & 0xff;

    if (g_ScreenEffectCounter != 0)
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;

    if (screenEffect->timer < screenEffect->duration)
    {
        screenEffect->arcadeFadeAlpha =
            alpha - (i32)((f32)screenEffect->timer * alpha / screenEffect->duration);
        if (screenEffect->arcadeFadeAlpha < 0)
            screenEffect->arcadeFadeAlpha = 0;
    }
    else
    {
        screenEffect->arcadeFadeAlpha = 0;
        SCREEN_EFFECT_PARAMETERS(screenEffect).pulse.repeatCount--;
        if (SCREEN_EFFECT_PARAMETERS(screenEffect).pulse.repeatCount <= 0)
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        screenEffect->timer = 0;
    }

    screenEffect->timer++;
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
    ScreenEffect::DrawSquare(&rect, (screenEffect->arcadeFadeAlpha << 24) |
                                       (SCREEN_EFFECT_PARAMETERS(screenEffect).pulse.color & 0xFFFFFF));
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x45bdc0
ChainCallbackResult ScreenEffect::CalcShake(ScreenEffect *screenEffect)
{
    f32 shakeAmount;

    if (g_GameManager.flags.deathbombFreezeActive)
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
                  (SCREEN_EFFECT_PARAMETERS(screenEffect).shake.finalAmplitude -
                   SCREEN_EFFECT_PARAMETERS(screenEffect).shake.initialAmplitude);
    shakeAmount = shakeAmount / screenEffect->duration;
    shakeAmount = SCREEN_EFFECT_PARAMETERS(screenEffect).shake.initialAmplitude + shakeAmount;

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

// FUNCTION: th08 0x45bf10
ChainCallbackResult ScreenEffect::FUN_0045bf10(ScreenEffect *screenEffect)
{
    f32 shakeAmount;

    if (g_GameManager.flags.deathbombFreezeActive)
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    if (g_EclScriptedGlobalUpdateFreeze)
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    if (*reinterpret_cast<i32 *>(0x164D2C8) <= 1)
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;

    screenEffect->timer++;
    if (screenEffect->timer < SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampUpFrames)
    {
        shakeAmount =
            (f32)screenEffect->timer / SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampUpFrames;
    }
    else if (screenEffect->timer <
             SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampUpFrames +
                 SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.holdFrames)
    {
        shakeAmount = 1.0f;
    }
    else
    {
        if (screenEffect->timer <
            SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampUpFrames +
                SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.holdFrames +
                SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampDownFrames)
        {
            shakeAmount =
                ((f32)(u32)(SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampUpFrames +
                            SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.holdFrames +
                            SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampDownFrames) -
                 (f32)screenEffect->timer) /
                (u32)SCREEN_EFFECT_PARAMETERS(screenEffect).shakeEnvelope.rampDownFrames;
        }
        else
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
    }

    shakeAmount = (f32)screenEffect->duration * shakeAmount;

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

#undef SCREEN_EFFECT_PARAMETERS

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
