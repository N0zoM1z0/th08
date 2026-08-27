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
    SCREEN_EFFECT_ARCADE_PULSE,
    SCREEN_EFFECT_FULL_FADE_OUT,
    SCREEN_EFFECT_FULL_FADE_HOLD,
    SCREEN_EFFECT_ARCADE_FADE_HOLD,
    SCREEN_EFFECT_SHAKE_ENVELOPE,
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
    static ChainCallbackResult CalcFadeHold(ScreenEffect *screenEffect);

    static ScreenEffect *RegisterChain(ScreenEffectType effect, i32 durationFrames, i32 primaryParameter,
                                       i32 secondaryParameter, i32 tertiaryParameter, i32 drawPriority);

    static ChainCallbackResult DrawFullFade(ScreenEffect *screenEffect);
    static ChainCallbackResult DrawPartialFade(ScreenEffect *screenEffect);

    static ChainCallbackResult DrawArcadeFade(ScreenEffect *screenEffect);
    static ChainCallbackResult CalcArcadePulse(ScreenEffect *screenEffect);
    static ChainCallbackResult DrawArcadePulse(ScreenEffect *screenEffect);
    static ChainCallbackResult CalcShake(ScreenEffect *screenEffect);
    static ChainCallbackResult CalcShakeEnvelope(ScreenEffect *screenEffect);

    static ZunResult InitializeTimer(ScreenEffect *screenEffect);
    static ZunResult DeleteScreenEffect(ScreenEffect *screenEffect);
    void BeginFadeRelease();

    ScreenEffectType type;
    ChainElem *calcChainElement;
    ChainElem *drawChainElement;
    i32 unconsumedDword0C;
    i32 overlayAlpha;
    i32 duration;
    D3DCOLOR rawParameter0;
    i32 rawParameter1;
    i32 rawParameter2;
    i32 fadeReleaseRequested;
    ZunTimer timer;
};

C_ASSERT(sizeof(ScreenEffect) == 0x34);
C_ASSERT(offsetof(ScreenEffect, type) == 0x0);
C_ASSERT(offsetof(ScreenEffect, unconsumedDword0C) == 0x0C);
C_ASSERT(offsetof(ScreenEffect, overlayAlpha) == 0x10);
C_ASSERT(offsetof(ScreenEffect, rawParameter0) == 0x18);
C_ASSERT(offsetof(ScreenEffect, fadeReleaseRequested) == 0x24);
C_ASSERT(offsetof(ScreenEffect, timer) == 0x28);

DIFFABLE_EXTERN(i32, g_ScreenEffectCounter);

}; // Namespace th08
