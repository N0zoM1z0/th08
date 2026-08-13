#include "th_pch.h"

#include "ScreenEffect.hpp"

namespace th08
{

DIFFABLE_STATIC(i32, g_ScreenEffectCounter);
DIFFABLE_STATIC(ScreenEffect, g_ScreenEffect);

// STUB: th08 0x45b020
void ScreenEffect::Clear(D3DCOLOR color)
{
}

// STUB: th08 0x45b0e0
void ScreenEffect::SetViewport(D3DCOLOR clearColor)
{
}

// STUB: th08 0x45b160
ChainCallbackResult ScreenEffect::CalcFadeIn(ScreenEffect *screenEffect)
{
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

// STUB: th08 0x45b760
ChainCallbackResult ScreenEffect::CalcFadeOut(ScreenEffect *screenEffect)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x45b8b0
ScreenEffect *ScreenEffect::RegisterChain(ScreenEffectType effect, i32 ticks, i32 param_3, i32 param_4, i32 param_5,
                                          i32 param_6)
{
    return NULL;
}

// STUB: th08 0x45bb50
ChainCallbackResult ScreenEffect::DrawFullFade(ScreenEffect *screenEffect)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x45bc40
ChainCallbackResult ScreenEffect::DrawArcadeFade(ScreenEffect *screenEffect)
{
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
