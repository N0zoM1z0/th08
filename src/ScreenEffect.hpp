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
    ScreenEffect();

    static void Clear(D3DCOLOR color);
    static void SetViewport(D3DCOLOR clearColor);

    static ChainCallbackResult CalcFadeIn(ScreenEffect *screenEffect);
    static void DrawSquare(ZunRect *rectDimensions, D3DCOLOR color);
    static void DrawSquareShaded(ZunRect *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft,
                                 D3DCOLOR bottomRight);
    static ChainCallbackResult CalcFadeOut(ScreenEffect *screenEffect);
    static ChainCallbackResult CalcPartialFadeOut(ScreenEffect *screenEffect);

    static ScreenEffect *RegisterChain(ScreenEffectType effect, i32 ticks, i32 param_3, i32 param_4, i32 param_5,
                                       i32 param_6);

    static ChainCallbackResult DrawFullFade(ScreenEffect *screenEffect);
    static ChainCallbackResult DrawPartialFade(ScreenEffect *screenEffect);

    static ChainCallbackResult DrawArcadeFade(ScreenEffect *screenEffect);
    static ChainCallbackResult FUN_0045bc90(ScreenEffect *screenEffect);
    static ChainCallbackResult FUN_0045bd70(ScreenEffect *screenEffect);
    static ChainCallbackResult CalcShake(ScreenEffect *screenEffect);
    static ChainCallbackResult FUN_0045bf10(ScreenEffect *screenEffect);

    static ZunResult AddedCallback(ScreenEffect *screenEffect);
    static ZunResult DeletedCallback(ScreenEffect *screenEffect);

    i32 type;
    ChainElem *calcChainElement;
    ChainElem *drawChainElement;
    i32 unk0c;
    i32 arcadeFadeAlpha;
    i32 duration;
    D3DCOLOR arcadeFadeColor;
    i32 shakeStart;
    i32 shakeEnd;
    i32 unk24;
    ZunTimer timer;
};

C_ASSERT(sizeof(ScreenEffect) == 0x34);

DIFFABLE_EXTERN(i32, g_ScreenEffectCounter);

}; // Namespace th08
