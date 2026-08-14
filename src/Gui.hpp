#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"
#include "utils.hpp"

namespace th08
{

struct GuiFormattedText
{
    GuiFormattedText();

    Float3 position;
    u32 value;
    i32 isShown;
    ZunTimer timer;
};
C_ASSERT(sizeof(GuiFormattedText) == 0x20);

struct GuiImpl
{
    unknown_fields(0x0, 0x21814);
    void *msgFile;
    unknown_fields(0x21818, 0x18a0);
};
C_ASSERT(sizeof(GuiImpl) == 0x230b8);

struct GuiFlags
{
    u32 lifeDisplayUpdateFrames : 2;
    u32 bombDisplayUpdateFrames : 2;
    u32 powerDisplayUpdateFrames : 2;
    u32 grazeDisplayUpdateFrames : 2;
    u32 pointDisplayUpdateFrames : 2;
    u32 timeDisplayUpdateFrames : 2;
};

#define MAX_BOSS_LIFEBAR_SEGMENTS 8

struct Gui
{
    static ChainCallbackResult OnUpdate(Gui *gui);
    static ChainCallbackResult OnDraw(Gui *gui);

    static ZunResult AddedCallback(Gui *gui);
    static ZunResult DeletedCallback(Gui *gui);

    static ZunResult RegisterChain();
    static void CutChain();

    ZunResult ActualAddedCallback();
    ZunResult LoadMsg(const char *path);
    void FreeMsgFile();
    void FUN_00422c20(i32 value);
    void FUN_004230c0(f32 value);
    void FUN_00423130(i32 value);
    void FUN_00439007();
    void FUN_004390d6();

    u32 unk_0;
    GuiFlags flags;
    GuiImpl *impl;
    AnmLoaded *frontAnm;
    AnmLoaded *stageTextAnm;
    AnmLoaded *timesAnm;
    AnmLoaded *loadingPortraitAnm;
    u32 bossUIOpacity;
    i32 eclSetLives;
    i32 spellcardSecondsRemaining;
    i32 previousSpellcardSecondsRemaining;
    bool bossPresent;
    f32 bossLifeBarMaxSize;
    f32 bossLifeBarSize;
    unknown_fields(0x38, 0x4);
    f32 bossLifeBarSegmentStop[MAX_BOSS_LIFEBAR_SEGMENTS];
    f32 bossLifeBarSegmentStart[MAX_BOSS_LIFEBAR_SEGMENTS];
    i32 bossLifeBarSegmentColor[MAX_BOSS_LIFEBAR_SEGMENTS];
};
C_ASSERT(sizeof(Gui) == 0x9c);

DIFFABLE_EXTERN(Gui, g_Gui);

} /* namespace th08 */
