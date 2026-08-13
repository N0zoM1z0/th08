#pragma once

#include "Global.hpp"
#include "Supervisor.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <d3dx8.h>

namespace th08
{

enum ScreenEffectType
{
    SCREEN_EFFECT_FULL_FADE_IN,
    SCREEN_EFFECT_SHAKE,
    SCREEN_EFFECT_ARCADE_FADE_OUT,
    SCREEN_EFFECT_UNK3,
    SCREEN_EFFECT_FULL_FADE_OUT,
    SCREEN_EFFECT_UNK5,
    SCREEN_EFFECT_UNK6,
    SCREEN_EFFECT_UNK7,
};

struct ScreenEffect
{
    static void Clear(D3DCOLOR color);
    static void SetViewport(D3DCOLOR clearColor);

    static ChainCallbackResult CalcFadeIn(ScreenEffect *screenEffect);
    static void DrawSquare(ZunRect *rectDimensions, D3DCOLOR color);
    static void DrawSquareShaded(ZunRect *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft,
                                 D3DCOLOR bottomRight);
    static ChainCallbackResult CalcFadeOut(ScreenEffect *screenEffect);

    static ScreenEffect *RegisterChain(ScreenEffectType effect, i32 ticks, i32 param_3, i32 param_4, i32 param_5,
                                       i32 param_6);

    static ChainCallbackResult DrawFullFade(ScreenEffect *screenEffect);

    static ChainCallbackResult DrawArcadeFade(ScreenEffect *screenEffect);
    static ChainCallbackResult CalcShake(ScreenEffect *screenEffect);

    static ZunResult AddedCallback(ScreenEffect *screenEffect);
    static ZunResult DeletedCallback(ScreenEffect *screenEffect);

    unknown_fields(0x0, 0x4);
    ChainElem *calcChainElement;
    ChainElem *drawChainElement;
    unknown_fields(0xc, 0x4);
    i32 arcadeFadeAlpha;
    unknown_fields(0x14, 0x4);
    D3DCOLOR arcadeFadeColor;
    unknown_fields(0x1c, 0xc);
    ZunTimer timer;
};

C_ASSERT(sizeof(ScreenEffect) == 0x34);

DIFFABLE_EXTERN(i32, g_ScreenEffectCounter);

}; // Namespace th08
