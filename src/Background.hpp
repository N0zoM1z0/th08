#pragma once
#include "Global.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <windows.h>

namespace th08
{
struct Background
{
    Background();

    static ChainCallbackResult OnUpdate(Background *background);
    static ChainCallbackResult OnDrawHighPrio(Background *background);
    static ChainCallbackResult OnDrawLowPrio(Background *background);
    static ZunResult AddedCallback(Background *background);
    static ZunResult RegisterChain();
    static ZunResult DeletedCallback(Background *background);
    static void CutChain();
    ZunResult LoadStageData();

    void SetCamera1()
    {
    }

    void SetCamera2()
    {
    }

    void *stageAnm; // +0x000: target-owned allocation released at teardown
    unknown_fields(0x4, 0x7f0);
    void *stageAnmSecondary; // +0x7f4: release is gated by resource reload
    unknown_fields(0x7f8, 0x328);
    u8 skyFogNeedsSetup; // Leftover from earlier games. Never checked in IN
    unknown_fields(0xb21, 0x5adf);
};
C_ASSERT(sizeof(Background) == 0x6600);

DIFFABLE_EXTERN(Background, g_Background);
}; // Namespace th08
