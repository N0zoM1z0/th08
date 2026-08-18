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

struct GuiMsgVm
{
    void *msgFile;
    i32 currentMsgIdx;
    i32 currentLineIdx;
    ZunTimer timer;
    i32 linesCount;
    i32 unused1c;
    AnmVm dialogueLines[4];
    AnmVm portraits[2];
    AnmVm portraitEffects[2];
    unknown_fields(0x1540, 0x28);
    u8 unk1568;
    unknown_fields(0x1569, 0xf);

    GuiMsgVm();
};
C_ASSERT(sizeof(GuiMsgVm) == 0x1578);

struct GuiImpl
{
    AnmVm vm0000[16];
    unknown_fields(0x2a40, 0x4);
    AnmVm vm2a44[4];
    AnmVm vm34d4;
    AnmVm vm3778;
    AnmVm vm3a1c;
    AnmVm vm3cc0;
    AnmVm vm3f64[8];
    AnmVm vm5484;
    AnmVm vm5728[0xA8];
    AnmVm vm212c8;
    AnmVm vm2156c;
    unknown_fields(0x21810, 0x4);
    GuiMsgVm msgVm;
    GuiFormattedText formatted0;
    GuiFormattedText formatted1;
    GuiFormattedText formatted2;
    unknown_fields(0x22dec, 0x28);
    AnmVm vm22e14;

    GuiImpl();
    void FUN_0043396d(i32 value);
    i32 RunMsg();
    ZunResult DrawDialogue();
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
    void FUN_0042f340(i32 value);
    static ChainCallbackResult OnUpdate(Gui *gui);
    static ChainCallbackResult OnDraw(Gui *gui);

    static ZunResult AddedCallback(Gui *gui);
    static ZunResult DeletedCallback(Gui *gui);

    static ZunResult RegisterChain();
    static void CutChain();

    ZunResult ActualAddedCallback();
    ZunResult LoadMsg(const char *path);
    i32 MsgWait();
    i32 IsDialogPresent();
    bool IsBossPresent();
    i32 FUN_00437dc7();
    void FreeMsgFile();
    void FUN_00437ddd(i32 value);
    void FUN_00437e5d(i32 value, i32 isShown);
    void gui_fun_00437edc(i32 value);
    void FUN_00422c20(i32 value);
    void FUN_004230c0(f32 value);
    void FUN_00423130(i32 value);
    void FUN_00439007();
    ZunResult FUN_00439050();
    ZunResult FUN_00439093();
    ZunResult FUN_004390d6();
    void FUN_004396b8();
    void FUN_00439810(i32 value);
    void DrawGameScene();
    void FUN_0043741d();
    void FUN_00435900();
    void FUN_0043826b();
    void FUN_00438a89();

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
    f32 bossLifeBarSize;
    f32 bossLifeBarMaxSize;
    unknown_fields(0x38, 0x4);
    f32 bossLifeBarSegmentStop[MAX_BOSS_LIFEBAR_SEGMENTS];
    f32 bossLifeBarSegmentStart[MAX_BOSS_LIFEBAR_SEGMENTS];
    i32 bossLifeBarSegmentColor[MAX_BOSS_LIFEBAR_SEGMENTS];
};
C_ASSERT(sizeof(Gui) == 0x9c);

DIFFABLE_EXTERN(Gui, g_Gui);

} /* namespace th08 */
