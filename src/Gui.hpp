#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"
#include "utils.hpp"

namespace th08
{

#define GUI_TRANSITION_ROWS 14
#define GUI_TRANSITION_COLUMNS 12

enum
{
    GUI_STAGE_MUSIC_CONTEXT_COUNT = 9,
};

struct GuiStageMusicContextSet
{
    i32 songNumbers[3];
};
C_ASSERT(sizeof(GuiStageMusicContextSet) == 0xc);

DIFFABLE_EXTERN_ARRAY(
    GuiStageMusicContextSet, GUI_STAGE_MUSIC_CONTEXT_COUNT, g_GuiStageMusicContexts);

struct GuiFormattedText
{
    GuiFormattedText();

    Float3 position;
    u32 value;
    i32 displayMode;
    ZunTimer timer;
};
C_ASSERT(sizeof(GuiFormattedText) == 0x20);

enum GuiDisplayMode
{
    GUI_DISPLAY_HIDDEN = 0,
    GUI_DISPLAY_SHOWN = 1,
    GUI_DISPLAY_FULL_POWER = 1,
    GUI_DISPLAY_SUPERNATURAL_BORDER = 2,
    GUI_DISPLAY_TIME_ORB_MAX = 3,
    GUI_DISPLAY_BORDER_BONUS = 4,
    GUI_DISPLAY_SPELLCARD_BONUS_FAILED = 5,
    GUI_DISPLAY_LAST_SPELL_FAILED = 6,
};

enum GuiMessageOpcode
{
    GUI_MSG_DELETE = 0,
    GUI_MSG_SET_PORTRAIT_ANM_SCRIPT = 1,
    GUI_MSG_SET_PORTRAIT_SPRITE = 2,
    GUI_MSG_SHOW_DIALOGUE_TEXT = 3,
    GUI_MSG_WAIT = 4,
    GUI_MSG_INTERRUPT_PORTRAIT_ANM = 5,
    GUI_MSG_RESUME_ECL = 6,
    GUI_MSG_SET_MUSIC = 7,
    GUI_MSG_SHOW_INTRO_TEXT = 8,
    GUI_MSG_SHOW_STAGE_RESULTS = 9,
    GUI_MSG_HALT = 10,
    GUI_MSG_END_STAGE = 11,
    GUI_MSG_FADE_OUT_MUSIC = 12,
    GUI_MSG_SET_DIALOGUE_SKIPPABLE = 13,
    GUI_MSG_FADE_SCREEN = 14,
    GUI_MSG_CONFIGURE_ALL_PORTRAITS = 15,
    GUI_MSG_SHOW_SPEAKER_TEXT = 16,
    GUI_MSG_CONFIGURE_PORTRAIT = 17,
    GUI_MSG_SET_TEXT_BOX_VISIBLE = 18,
    GUI_MSG_SHOW_TOP_TEXT = 19,
    GUI_MSG_SHOW_BOTTOM_TEXT = 20,
    GUI_MSG_SHOW_SELECTION = 21,
    GUI_MSG_READ_SELECTED_MESSAGE = 22,
};

struct GuiMessageConfigureAllPortraitsArgs
{
    i32 portraitIndex;
    i32 spriteIndices[4];
};
C_ASSERT(sizeof(GuiMessageConfigureAllPortraitsArgs) == 0x14);

struct GuiMessageConfigurePortraitArgs
{
    i32 portraitIndex;
    i32 spriteIndex;
};
C_ASSERT(sizeof(GuiMessageConfigurePortraitArgs) == 0x8);

struct GuiMessagePortraitAnmScriptArgs
{
    i16 portraitIndex;
    i16 scriptIndex;
};
C_ASSERT(sizeof(GuiMessagePortraitAnmScriptArgs) == 0x4);

struct GuiMessagePortraitSpriteArgs
{
    i16 portraitIndex;
    i16 spriteIndex;
};
C_ASSERT(sizeof(GuiMessagePortraitSpriteArgs) == 0x4);

struct GuiMessageDialogueTextArgs
{
    i16 colorIndex;
    i16 lineIndex;
    char encryptedText[1];
};
C_ASSERT(offsetof(GuiMessageDialogueTextArgs, encryptedText) == 0x4);

struct GuiMessagePlainTextArgs
{
    char encryptedText[1];
};

struct GuiMessageWaitArgs
{
    i32 frames;
};

struct GuiMessagePortraitInterruptArgs
{
    i16 portraitIndex;
    u8 interrupt;
};
C_ASSERT(offsetof(GuiMessagePortraitInterruptArgs, interrupt) == 0x2);

struct GuiMessageMusicArgs
{
    i32 musicIndex;
};

struct GuiMessageByteToggleArgs
{
    u8 enabled;
};

union GuiMessageInstructionArgs
{
    GuiMessageConfigureAllPortraitsArgs configureAllPortraits;
    GuiMessageConfigurePortraitArgs configurePortrait;
    GuiMessagePortraitAnmScriptArgs portraitAnmScript;
    GuiMessagePortraitSpriteArgs portraitSprite;
    GuiMessageDialogueTextArgs dialogueText;
    GuiMessagePlainTextArgs plainText;
    GuiMessageWaitArgs wait;
    GuiMessagePortraitInterruptArgs portraitInterrupt;
    GuiMessageMusicArgs music;
    GuiMessageByteToggleArgs toggle;
    u8 raw[1];
};
C_ASSERT(sizeof(GuiMessageInstructionArgs) == 0x14);

// Message records are variable-sized. instructionSize is measured from the
// payload start, so the next record begins at &args + instructionSize.
struct GuiMessageInstruction
{
    u16 time;
    u8 opcode;
    u8 instructionSize;
    GuiMessageInstructionArgs args;
};
C_ASSERT(offsetof(GuiMessageInstruction, args) == 0x4);
C_ASSERT(sizeof(GuiMessageInstruction) == 0x18);

// The on-disk pointer table contains message-relative offsets. LoadMsg fixes
// every entry up in place before the interpreter consumes it.
struct GuiMessageFile
{
    i32 messageCount;
    GuiMessageInstruction *messages[1];
};
C_ASSERT(offsetof(GuiMessageFile, messages) == 0x4);

struct GuiMsgVm
{
    GuiMessageFile *msgFile;
    GuiMessageInstruction *currentInstr;
    i32 currentMsgIdx;
    ZunTimer timer;
    i32 framesElapsedDuringPause;
    i32 waitThreshold;
    AnmVm portraits[4];
    AnmVm dialogueLines[2];
    AnmVm introLines[2];
    u32 textColors[4];
    u32 shadowColors[4];
    i32 fontSize;
    u32 ignoreWaitCounter;
    u8 dialogueSkippable;
    u8 textColorIndex;
    u8 resetDialogueLines;
    u8 dialogueLineIndex;
    u8 currentPortraitIndex;
    u8 textBoxVisible;
    u8 selectedOption;
    u8 padding156f;

    GuiMsgVm();
};
C_ASSERT(sizeof(GuiMsgVm) == 0x1570);
C_ASSERT(offsetof(GuiMsgVm, portraits) == 0x20);
C_ASSERT(offsetof(GuiMsgVm, dialogueLines) == 0xAB0);
C_ASSERT(offsetof(GuiMsgVm, introLines) == 0xFF8);
C_ASSERT(offsetof(GuiMsgVm, textColors) == 0x1540);

struct GuiStageClearData
{
    i32 stageBonus;
    i32 power;
    i32 pointItemsCollected;
    i32 graze;
    i32 timeOrbs;
    i32 clockIncrement;
    i32 clockDisplayStart;
    i32 clockDisplayTarget;
    i32 clockDisplayCurrent;
    i32 clockDisplayTimer;
};
C_ASSERT(sizeof(GuiStageClearData) == 0x28);

struct GuiImpl
{
    AnmVm frontVms[16];
    u8 bossLifeBarState;
    u8 bossLifeBarPadding[3];
    AnmVm stageTextVms[4];
    AnmVm clockIntroVm;
    AnmVm loadingPortraitVm;
    AnmVm loadingOverlayVm;
    AnmVm arcadeCaptureVm;
    AnmVm arcadeBlurVms[8];
    AnmVm spellNullifyVm;
    AnmVm stageTransitionVms[GUI_TRANSITION_ROWS * GUI_TRANSITION_COLUMNS];
    AnmVm stageRankVm;
    AnmVm clockTimeVm;
    i32 stageTransitionActiveVmCount;
    GuiMsgVm message;
    i32 stageClearScreenState;
    i32 stageClearBonusTotal;
    GuiFormattedText bonusPopup;
    GuiFormattedText statusPopup;
    GuiFormattedText spellcardBonusPopup;
    GuiStageClearData stageClear;
    AnmVm difficultyVm;

    GuiImpl();
    void StartMessage(i32 messageIndex);
    i32 RunMsg();
    ZunResult DrawDialogue();
};
C_ASSERT(sizeof(GuiImpl) == 0x230b8);
C_ASSERT(offsetof(GuiImpl, bossLifeBarState) == 0x2A40);
C_ASSERT(offsetof(GuiImpl, stageTextVms) == 0x2A44);
C_ASSERT(offsetof(GuiImpl, clockIntroVm) == 0x34D4);
C_ASSERT(offsetof(GuiImpl, loadingPortraitVm) == 0x3778);
C_ASSERT(offsetof(GuiImpl, loadingOverlayVm) == 0x3A1C);
C_ASSERT(offsetof(GuiImpl, arcadeCaptureVm) == 0x3CC0);
C_ASSERT(offsetof(GuiImpl, arcadeBlurVms) == 0x3F64);
C_ASSERT(offsetof(GuiImpl, spellNullifyVm) == 0x5484);
C_ASSERT(offsetof(GuiImpl, stageTransitionVms) == 0x5728);
C_ASSERT(offsetof(GuiImpl, stageRankVm) == 0x212C8);
C_ASSERT(offsetof(GuiImpl, clockTimeVm) == 0x2156C);
C_ASSERT(offsetof(GuiImpl, stageTransitionActiveVmCount) == 0x21810);
C_ASSERT(offsetof(GuiImpl, message) == 0x21814);
C_ASSERT(offsetof(GuiImpl, stageClearScreenState) == 0x22D84);
C_ASSERT(offsetof(GuiImpl, stageClearBonusTotal) == 0x22D88);
C_ASSERT(offsetof(GuiImpl, bonusPopup) == 0x22D8C);
C_ASSERT(offsetof(GuiImpl, statusPopup) == 0x22DAC);
C_ASSERT(offsetof(GuiImpl, spellcardBonusPopup) == 0x22DCC);
C_ASSERT(offsetof(GuiImpl, stageClear) == 0x22DEC);
C_ASSERT(offsetof(GuiImpl, difficultyVm) == 0x22E14);

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
    void SetBossTimerSeconds(i32 seconds);
    static ChainCallbackResult OnUpdate(Gui *gui);
    static ChainCallbackResult OnDraw(Gui *gui);

    static ZunResult AddedCallback(Gui *gui);
    static ZunResult DeletedCallback(Gui *gui);

    static ZunResult RegisterChain();
    static void CutChain();

    ZunResult ActualAddedCallback();
    ZunResult LoadMsg(const char *path);
    i32 MsgWait();
    i32 IsDialoguePresent();
    bool IsBossPresent();
    bool SetBossPresent(bool value);
    i32 StartStageBackgroundSequence();
    i32 IsStageFinished();
    i32 IsDialogueSkippable();
    void FreeMsgFile();
    void ShowBonusScore(i32 score);
    void ShowPopupText(i32 value, i32 displayMode);
    void ShowSpellcardBonus(i32 bonus);
    void SetBossLifeBarTarget(f32 value);
    void SetBossGaugeSlot(i32 index, f32 start, f32 stop);
    void SetBossGaugeColor(i32 index, i32 color);
    void SetBossLifeMarkerCount(i32 count);
    ZunResult FlashClockTimeSlow();
    ZunResult FlashClockTimeFast();
    ZunResult HideClockTime();
    void InitStageClearScreen();
    void MsgRead(i32 messageIndex);
    void DrawGameScene();
    void DrawStageElements();
    void UpdateStageElements();
    void DrawStageClearScreen();
    void DrawAsciiText();
    ZunResult CaptureArcade();
    static void CopyEnemyNameTexture(i32 spriteIndex);
    static void CopyCurrentStageEnemyNameTexture();

    u32 frameCounter;
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
    f32 bossLifeBarTargetSize;
    f32 bossLifeBarDisplayedSize;
    unknown_fields(0x38, 0x4);
    f32 bossLifeBarSegmentStop[MAX_BOSS_LIFEBAR_SEGMENTS];
    f32 bossLifeBarSegmentStart[MAX_BOSS_LIFEBAR_SEGMENTS];
    i32 bossLifeBarSegmentColor[MAX_BOSS_LIFEBAR_SEGMENTS];
};
C_ASSERT(sizeof(Gui) == 0x9c);

DIFFABLE_EXTERN(Gui, g_Gui);
DIFFABLE_EXTERN(u16, g_GuiMessageInputCurrent);
DIFFABLE_EXTERN(u16, g_GuiMessageInputPrevious);

} /* namespace th08 */
