#include "th_pch.h"

#include "BulletManager.hpp"
#include "AsciiManager.hpp"
#include "Background.hpp"
#include "Gui.hpp"
#include "EnemyManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "ReplayManager.hpp"
#include "Player.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"

#include <stdio.h>

namespace th08
{

DIFFABLE_STATIC(Gui, g_Gui);
DIFFABLE_STATIC(ChainElem, g_GuiCalcChain);
DIFFABLE_STATIC(ChainElem, g_GuiDrawChain);
DIFFABLE_STATIC(i32, g_GuiFullPowerModeFrames);
DIFFABLE_STATIC(i32, g_GuiMessageStageMode);
DIFFABLE_STATIC(u16, g_GuiMessageInputCurrent);
DIFFABLE_STATIC(u16, g_GuiMessageInputPrevious);
DIFFABLE_STATIC(i32, g_GuiMessageScreenEffectDuration);
DIFFABLE_STATIC_ARRAY(i32 *, MAX_STAGES, g_GuiStageScoreTables);
struct GuiMessageTextColorSet
{
    u32 colors[4];
};
DIFFABLE_STATIC_ARRAY(GuiMessageTextColorSet, SHOT_ALL, g_GuiMessageTextColors);

struct GuiStageMusicContextSet
{
    i32 values[3];
};
DIFFABLE_STATIC_ARRAY(GuiStageMusicContextSet, MAX_STAGES, g_GuiStageMusicContexts);
DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 4, g_GuiBossTimerColors) = {0x00a0d0ff, 0x00a080ff, 0x00e080c0, 0x00ff4040};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 2, g_GuiTimePeriodLabels) = {"AM", "PM"};

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_GuiLoadingAnmPaths) = {
    "loading00.anm", "loading01.anm", "loading02.anm", "loading03.anm", "loading00h.anm", "loading00a.anm",
    "loading01h.anm", "loading01a.anm", "loading02h.anm", "loading02a.anm", "loading03h.anm", "loading03a.anm",
};

typedef const char *GuiMessagePathRow[SHOT_ALL];
DIFFABLE_STATIC_ARRAY_ASSIGN(GuiMessagePathRow, MAX_STAGES, g_GuiMessagePaths) = {
    {"msg1a.dat", "msg1b.dat", "msg1c.dat", "msg1d.dat", "msg1a.dat", "msg1a.dat", "msg1b.dat", "msg1b.dat", "msg1c.dat", "msg1c.dat", "msg1d.dat", "msg1d.dat"},
    {"msg2a.dat", "msg2b.dat", "msg2c.dat", "msg2d.dat", "msg2a.dat", "msg2a.dat", "msg2b.dat", "msg2b.dat", "msg2c.dat", "msg2c.dat", "msg2d.dat", "msg2d.dat"},
    {"msg3a.dat", "msg3b.dat", "msg3c.dat", "msg3d.dat", "msg3a.dat", "msg3a.dat", "msg3b.dat", "msg3b.dat", "msg3c.dat", "msg3c.dat", "msg3d.dat", "msg3d.dat"},
    {"msg4dm.dat", "msg4ab.dat", "msg4ac.dat", "msg4dm.dat", "msg4dm.dat", "msg4dm.dat", "msg4ab.dat", "msg4ab.dat", "msg4ac.dat", "msg4ac.dat", "msg4dm.dat", "msg4dm.dat"},
    {"msg4ba.dat", "msg4dm.dat", "msg4dm.dat", "msg4bd.dat", "msg4ba.dat", "msg4ba.dat", "msg4dm.dat", "msg4dm.dat", "msg4dm.dat", "msg4dm.dat", "msg4bd.dat", "msg4bd.dat"},
    {"msg5a.dat", "msg5b.dat", "msg5c.dat", "msg5d.dat", "msg5a.dat", "msg5a.dat", "msg5b.dat", "msg5b.dat", "msg5c.dat", "msg5c.dat", "msg5d.dat", "msg5d.dat"},
    {"msg6a.dat", "msg6b.dat", "msg6c.dat", "msg6d.dat", "msg6a.dat", "msg6a.dat", "msg6b.dat", "msg6b.dat", "msg6c.dat", "msg6c.dat", "msg6d.dat", "msg6d.dat"},
    {"msg7a.dat", "msg7b.dat", "msg7c.dat", "msg7d.dat", "msg7a.dat", "msg7a.dat", "msg7b.dat", "msg7b.dat", "msg7c.dat", "msg7c.dat", "msg7d.dat", "msg7d.dat"},
    {"msg8a.dat", "msg8b.dat", "msg8c.dat", "msg8d.dat", "msg8a.dat", "msg8a.dat", "msg8b.dat", "msg8b.dat", "msg8c.dat", "msg8c.dat", "msg8d.dat", "msg8d.dat"},
};



i32 FUN_00439916(i32 unused);
i32 FUN_00439961(i32 unused);

struct GuiStageMusicDataOverlay
{
    unknown_fields(0x0, 0x290);
    char songPaths[4][128];
};

struct GuiRawMessageInstruction
{
    u16 time;
    u8 opcode;
    u8 argSize;
    u8 args[1];
};

struct GuiMessagePortraitArgs
{
    i32 portraitIndex;
    i32 scriptOrSprite;
};

struct GuiMessagePortraitShortArgs
{
    i16 portraitIndex;
    i16 scriptOrSprite;
};

struct GuiMessageScriptsArgs
{
    i32 portraitIndex;
    i32 scripts[4];
};

struct GuiMessageTextArgs
{
    i16 colorIndex;
    i16 lineIndex;
    char text[1];
};

struct GuiMessageStateOverlay
{
    void *msgFile;                  // +0x000
    u8 *currentInstr;               // +0x004
    i32 currentMsgIdx;              // +0x008
    ZunTimer timer;                 // +0x00C
    i32 framesElapsedDuringPause;   // +0x018
    i32 waitThreshold;              // +0x01C
    AnmVm portraits[4];             // +0x020
    AnmVm dialogueLines[2];         // +0xAB0
    AnmVm extraVms[2];              // +0xFF8
    u32 textColors[4];              // +0x1540
    u32 shadowColors[4];            // +0x1550
    i32 fontSize;                   // +0x1560
    u32 ignoreWaitCounter;          // +0x1564
    u8 dialogueSkippable;           // +0x1568
    u8 currentSide;                 // +0x1569
    u8 textPending;                 // +0x156A
    u8 currentLine;                 // +0x156B
    u8 currentPortrait;             // +0x156C
    u8 messageFlag;                 // +0x156D
    u8 routeChoice;                 // +0x156E
    u8 choiceSelection;             // +0x156F
    i32 resultState;                // +0x1570
    i32 unknown1574;                // +0x1574
};
C_ASSERT(sizeof(GuiMessageStateOverlay) == 0x1578);
C_ASSERT(offsetof(GuiMessageStateOverlay, portraits) == 0x20);
C_ASSERT(offsetof(GuiMessageStateOverlay, dialogueLines) == 0xAB0);
C_ASSERT(offsetof(GuiMessageStateOverlay, extraVms) == 0xFF8);
C_ASSERT(offsetof(GuiMessageStateOverlay, textColors) == 0x1540);

struct GuiStageResultUpdateOverlay
{
    i32 baseScore;                 // +0x00 / impl +0x22DEC
    i32 power;                     // +0x04
    i32 pointItemsCollectedInStage;// +0x08
    i32 grazeInStage;              // +0x0C
    i32 timeOrbs;                  // +0x10
    i32 clockIncrement;            // +0x14
    i32 clockDisplayStart;         // +0x18
    i32 clockDisplayTarget;        // +0x1C
    i32 clockDisplayCurrent;       // +0x20
    i32 clockDisplayTimer;         // +0x24
};
C_ASSERT(sizeof(GuiStageResultUpdateOverlay) == 0x28);


void __fastcall FUN_004353ec(char *out, const char *encoded);
void __fastcall FUN_00437f5c(i32 spriteIdx);
i32 FUN_00438fe9();
i32 FUN_00438ff3();
i32 FUN_00438ffd();

// FUNCTION: th08 0x4338ca
ChainCallbackResult Gui::OnUpdate(Gui *gui)
{
    if (g_EclScriptedGlobalUpdateFreeze)
        return CHAIN_CALLBACK_RESULT_CONTINUE;

    gui->FUN_00435900();
    gui->impl->RunMsg();
    if ((g_CurFrameInput & TH_BUTTON_SKIP) && g_GuiMessageScreenEffectDuration < 8)
        g_GuiMessageScreenEffectDuration = 8;
    gui->unk_0++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x433927
ChainCallbackResult Gui::OnDraw(Gui *gui)
{
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(gui->impl) + 0x22d84) != 0)
        gui->FUN_0043826b();
    gui->impl->DrawDialogue();
    gui->FUN_0043741d();
    gui->DrawGameScene();
    gui->FUN_00438a89();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x43396d
void GuiImpl::FUN_0043396d(i32 value)
{
    void *msgFile;

    utils::GuiDebugPrint("msg start %d\n\r", value);
    msgFile = reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->msgFile;
    memset(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm), 0, 0x1570);
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->msgFile = msgFile;

    if (value == 0)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE5:
            FUN_00437f5c(22);
            break;
        case STAGE6A:
            g_GuiMessageStageMode = 2;
            break;
        case STAGE6B:
        {
            AnmLoaded *tmp = g_Spellcard.enemyFaceAnm0;
            g_Spellcard.enemyFaceAnm0 = g_Spellcard.enemyFaceAnm1;
            g_Spellcard.enemyFaceAnm1 = tmp;
            g_GuiMessageStageMode = 2;
            FUN_00437f5c(24);
            break;
        }
        case EXTRASTAGE:
        {
            AnmLoaded *tmp = g_Spellcard.enemyFaceAnm0;
            g_Spellcard.enemyFaceAnm0 = g_Spellcard.enemyFaceAnm1;
            g_Spellcard.enemyFaceAnm1 = tmp;
            g_GuiMessageStageMode = 2;
            FUN_00437f5c(25);
            break;
        }
        default:
            break;
        }
    }
    else if (value == 10)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE5:
            if (g_GameManager.globals->numRetries > 0)
            {
                value = 1;
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
            }
            else if (!g_GameManager.IsReplay())
            {
                if (g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, EASY) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, NORMAL) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, HARD) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, LUNATIC) ||
                    g_GameManager.shotType > SHOT_YOUMU_YUYUKO)
                {
                    value = 3;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else if (g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, EASY) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, NORMAL) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, HARD) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, LUNATIC))
                {
                    value = 2;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else
                {
                    value = 1;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
                }
            }
            else
            {
                if ((i8)g_ReplayManager->replayData->clearState == 2)
                {
                    value = 3;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else if ((i8)g_ReplayManager->replayData->clearState == 1)
                {
                    value = 2;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else
                {
                    value = 1;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
                }
            }
            g_GameManager.flags.isGoingToFinalB = reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice;
            break;
        default:
            break;
        }
    }
    else if (value >= 6)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE6B:
            if ((i8)g_GameManager.GetClockTime() >= 12)
            {
                value = 5;
            }
            break;
        default:
            break;
        }
    }

    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentMsgIdx = value;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr = reinterpret_cast<u8 **>(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->msgFile)[value + 1];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0].scriptIndex = -1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1].scriptIndex = -1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->messageFlag = 1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->fontSize = 15;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[0] = g_GuiMessageTextColors[g_GameManager.shotType].colors[0];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[1] = g_GuiMessageTextColors[g_GameManager.shotType].colors[1];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[2] = g_GuiMessageTextColors[g_GameManager.shotType].colors[2];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[3] = g_GuiMessageTextColors[g_GameManager.shotType].colors[3];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[0] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[1] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[2] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[3] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueSkippable = 1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->waitThreshold = 6;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait = 0xff;

    g_BulletManager.bulletmanager_fun_00415c60();
    g_EnemyManager.KillAllNonBossEnemies(0, 0);
    g_ItemManager.AutoCollectAllItems();
}

// FUNCTION: th08 0x433db3
#pragma var_order(args, j, portraitArgs, k, portraitSpriteArgs, text3, text16, text19, text20, i)
i32 GuiImpl::RunMsg()
{
    GuiMessagePortraitArgs *args;
    u32 j;
    GuiMessagePortraitArgs *portraitArgs;
    u32 k;
    GuiMessagePortraitArgs *portraitSpriteArgs;
    char text3[64];
    char text19[64];
    char text20[64];
    char text16[64];
    u32 i;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentMsgIdx < 0)
        return -1;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->ignoreWaitCounter > 0)
        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->ignoreWaitCounter--;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueSkippable &&
        (g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
    {
        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer =
            reinterpret_cast<GuiRawMessageInstruction *>(
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->time;
    }

    if (g_Player.playerState != PLAYER_STATE_DYING)
        g_ItemManager.AutoCollectAllItems();

    while (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer >= (i32)
           reinterpret_cast<GuiRawMessageInstruction *>(
               reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->time)
    {
        switch (reinterpret_cast<GuiRawMessageInstruction *>(
                   reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->opcode)
        {
        case 0:
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentMsgIdx = -1;
            return -1;

        case 0xF:
            portraitArgs = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait !=
                reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->portraitIndex)
            {
                for (j = 0; j < 4; j++)
                {
                    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait == j)
                    {
                        if ((reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait / 2) !=
                            (reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->portraitIndex / 2))
                            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[j].pendingInterrupt = 6;
                        else
                            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[j].pendingInterrupt = 4;
                    }
                    else
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[j].pendingInterrupt = 4;
                }
            }
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                ->portraits[reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->portraitIndex].pendingInterrupt = 3;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait =
                reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->portraitIndex;
            if (reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[0] >= 0)
                g_Spellcard.playerFaceAnm0->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0],
                    reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[0]);
            if (reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[1] >= 0)
                g_Spellcard.playerFaceAnm1->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1],
                    reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[1]);
            if (reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[2] >= 0)
                g_Spellcard.enemyFaceAnm0->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2],
                    reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[2]);
            if (reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[3] >= 0)
                g_Spellcard.enemyFaceAnm1->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3],
                    reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->scripts[3]);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide =
                reinterpret_cast<GuiMessageScriptsArgs *>(portraitArgs)->portraitIndex;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
            break;

        case 0x11:
            portraitSpriteArgs = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait !=
                portraitSpriteArgs->portraitIndex)
            {
                for (k = 0; k < 4; k++)
                {
                    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait == k)
                    {
                        if ((reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait / 2) !=
                            (portraitSpriteArgs->portraitIndex / 2))
                            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[k].pendingInterrupt = 6;
                        else
                            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[k].pendingInterrupt = 4;
                    }
                    else
                    {
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[k].pendingInterrupt = 4;
                    }
                }
            }
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                ->portraits[portraitSpriteArgs->portraitIndex].pendingInterrupt = 3;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait =
                portraitSpriteArgs->portraitIndex;
            if (portraitSpriteArgs->scriptOrSprite >= 0)
            {
                switch (portraitSpriteArgs->portraitIndex)
                {
                case 0:
                    g_Spellcard.playerFaceAnm0->SetSprite(
                        &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0],
                        portraitSpriteArgs->scriptOrSprite);
                    break;
                case 1:
                    g_Spellcard.playerFaceAnm1->SetSprite(
                        &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1],
                        portraitSpriteArgs->scriptOrSprite);
                    break;
                case 2:
                    g_Spellcard.enemyFaceAnm0->SetSprite(
                        &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2],
                        portraitSpriteArgs->scriptOrSprite);
                    break;
                case 3:
                    g_Spellcard.enemyFaceAnm1->SetSprite(
                        &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3],
                        portraitSpriteArgs->scriptOrSprite);
                    break;
                }
            }
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide =
                portraitSpriteArgs->portraitIndex;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
            break;

        case 1:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            switch (reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex)
            {
            case 0:
                g_Spellcard.playerFaceAnm0->SetAndExecuteScriptIdx(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            case 1:
                g_Spellcard.playerFaceAnm1->SetAndExecuteScriptIdx(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            case 2:
                g_Spellcard.enemyFaceAnm0->SetAndExecuteScriptIdx(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            case 3:
                g_Spellcard.enemyFaceAnm1->SetAndExecuteScriptIdx(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            }
            if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                    ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]
                    .loadedSprite->widthPx > 128.0f)
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]) + 0x288) = -112.0f;
            else
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]) + 0x288) = 0.0f;
            break;

        case 2:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            switch (reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex)
            {
            case 0:
                g_Spellcard.playerFaceAnm0->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            case 1:
                g_Spellcard.playerFaceAnm1->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            case 2:
                g_Spellcard.enemyFaceAnm0->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            case 3:
                g_Spellcard.enemyFaceAnm1->SetSprite(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3],
                    reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->scriptOrSprite);
                break;
            }
            if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                    ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]
                    .loadedSprite->widthPx > 256.0f)
            {
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]) + 0x288) = -208.0f;
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]) + 0x28C) = -50.0f;
            }
            else if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]
                         .loadedSprite->widthPx > 128.0f)
            {
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]) + 0x288) = -80.0f;
            }
            else
            {
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(
                    &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                         ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex]) + 0x288) = 0.0f;
            }
            break;

        case 3:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            if (reinterpret_cast<GuiMessageTextArgs *>(args)->lineIndex == 0 &&
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1].scriptIndex >= 0)
            {
                g_AnmManager->DrawTextLeft(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1],
                                           reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[reinterpret_cast<GuiMessageTextArgs *>(args)->colorIndex],
                                           reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[reinterpret_cast<GuiMessageTextArgs *>(args)->colorIndex], " ");
            }
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(
                &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[reinterpret_cast<GuiMessageTextArgs *>(args)->lineIndex], reinterpret_cast<GuiMessageTextArgs *>(args)->lineIndex);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                ->dialogueLines[reinterpret_cast<GuiMessageTextArgs *>(args)->lineIndex].fontWidth =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                    ->dialogueLines[reinterpret_cast<GuiMessageTextArgs *>(args)->lineIndex].fontHeight =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->fontSize;
            FUN_004353ec(text3, reinterpret_cast<GuiMessageTextArgs *>(args)->text);
            g_AnmManager->DrawTextLeft(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[reinterpret_cast<GuiMessageTextArgs *>(args)->lineIndex],
                                       reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[reinterpret_cast<GuiMessageTextArgs *>(args)->colorIndex],
                                       reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[reinterpret_cast<GuiMessageTextArgs *>(args)->colorIndex], text3);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause = 0;
            break;

        case 0x10:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending)
            {
                if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1].scriptIndex >= 0)
                {
                    g_AnmManager->DrawTextLeft(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1],
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide],
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide], " ");
                }
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine = 0;
            }
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(
                &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                ->dialogueLines[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine].fontWidth =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                    ->dialogueLines[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine].fontHeight =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->fontSize;
            FUN_004353ec(text16, reinterpret_cast<const char *>(args));
            g_AnmManager->DrawTextLeft(
                &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide], text16);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause = 0;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 0;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine++;
            break;

        case 0x13:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0], 0);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0].fontWidth =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0].fontHeight =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->fontSize;
            FUN_004353ec(text19, reinterpret_cast<const char *>(args));
            g_AnmManager->DrawTextLeft(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[0],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[0], text19);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause = 0;
            break;

        case 0x14:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1], 1);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1].fontWidth =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1].fontHeight =
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->fontSize;
            FUN_004353ec(text20, reinterpret_cast<const char *>(args));
            g_AnmManager->DrawTextLeft(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[0],
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[0], text20);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause = 0;
            break;

        case 0x15:
            if ((g_GuiMessageInputCurrent & TH_BUTTON_UP) &&
                (g_GuiMessageInputCurrent & TH_BUTTON_UP) != (g_GuiMessageInputPrevious & TH_BUTTON_UP))
            {
                if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice == 1)
                    g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
            }
            if ((g_GuiMessageInputCurrent & TH_BUTTON_DOWN) &&
                (g_GuiMessageInputCurrent & TH_BUTTON_DOWN) != (g_GuiMessageInputPrevious & TH_BUTTON_DOWN))
            {
                if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice == 0)
                    g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
            }
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x224B4 +
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice * sizeof(AnmVm)) = -1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x224B4 +
                (1 - reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice) * sizeof(AnmVm)) =
                0xE0606060;
            if (!((g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) &&
                  (g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) !=
                      (g_GuiMessageInputPrevious & TH_BUTTON_SHOOT)) ||
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause < 60)
            {
                if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause >=
                    *reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args))
                {
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->waitThreshold = 30;
                    break;
                }
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause++;
                goto run_scripts;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            break;
        case 0x16:
            g_GameManager.flags.isGoingToFinalB = reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice;
            g_Gui.FUN_00439810(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice + 1);
            continue;
        case 4:
            if (!reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueSkippable ||
                !(g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
            {
                if (!(g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) ||
                    (g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) ==
                        (g_GuiMessageInputPrevious & TH_BUTTON_SHOOT) ||
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause <
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->waitThreshold)
                {
                    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause >=
                        *reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args))
                    {
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->waitThreshold = 30;
                        break;
                    }
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause++;
                    goto run_scripts;
                }
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->waitThreshold = 8;
            }
            break;

        case 5:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)
                ->portraits[reinterpret_cast<GuiMessagePortraitShortArgs *>(args)->portraitIndex].pendingInterrupt =
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(args) + 2);
            break;

        case 6:
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->ignoreWaitCounter++;
            break;

        case 7:
            if (*reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args) < 0)
            {
                g_Supervisor.StopAudio();
            }
            else
            {
                g_Gui.stageTextAnm->SetAndExecuteScriptIdx(&this->vm2a44[3], 3);
                g_Gui.stageTextAnm->SetSprite(
                    &this->vm2a44[3],
                    *reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args) + 3);
                if (g_Supervisor.PlayMusic(
                        *reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args),
                        reinterpret_cast<char *>(g_GuiStageMusicContexts[g_GameManager.currentStage]
                                                    .values[*reinterpret_cast<i32 *>(
                                                        reinterpret_cast<GuiRawMessageInstruction *>(
                                                            reinterpret_cast<GuiMessageStateOverlay *>(
                                                                &this->msgVm)->currentInstr)->args)])))
                {
                    g_Supervisor.PlayAudio(
                        reinterpret_cast<GuiStageMusicDataOverlay *>(g_Background.stageData)
                            ->songPaths[*reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args)],
                        g_GuiStageMusicContexts[g_GameManager.currentStage]
                            .values[*reinterpret_cast<i32 *>(reinterpret_cast<GuiRawMessageInstruction *>(
                                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args)]);
                }
            }
            break;

        case 8:
            args = reinterpret_cast<GuiMessagePortraitArgs *>(
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args);
            g_Spellcard.enemyFaceAnm0->SetAndExecuteScriptIdx(
                &reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->extraVms[0], 1);
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->framesElapsedDuringPause = 0;
            break;

        case 9:
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22DF0) = g_GameManager.GetPower();
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22DF4) = g_GameManager.globals->pointItemsCollectedInStage;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22DFC) = g_GameManager.GetTimeOrbs();
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22DF8) = g_GameManager.globals->grazeInStage;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E04) =
                (i8)g_GameManager.GetClockTime() * 30 + 0x294;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E00) = g_GameManager.GetClockTimeIncrement();
            g_GameManager.AddToClockTime(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E00));
            *reinterpret_cast<i32 **>(reinterpret_cast<u8 *>(this) + 0x22DEC) =
                g_GuiStageScoreTables[g_GameManager.currentStage];
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E08) =
                (i8)g_GameManager.GetClockTime() * 30 + 0x294;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E0C) =
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E04);
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E10) &= 0;
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->resultState = 1;
            g_GameManager.flags.unk9 = 1;

            if (g_GameManager.currentStage != STAGE6A && g_GameManager.currentStage != STAGE6B &&
                g_GameManager.currentStage != EXTRASTAGE)
            {
                g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(
                    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x212C8), 3);
                g_AsciiManager.asciiAnm->SetSprite(
                    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x212C8),
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x22E00) + 0x80);
            }
            else
            {
                reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x212C8)->currentInstruction = NULL;
            }
            reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x212C8)->SetInterrupt(1);

            if (g_GameManager.currentStage != STAGE6A && g_GameManager.currentStage != STAGE6B &&
                g_GameManager.currentStage != EXTRASTAGE)
            {
                (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(&g_Gui) + 0x18))
                    ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3778), 0);
                g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(
                    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3CC0), 1);
                g_AnmManager->SetTextureCaptureParams(
                    3, 0x20, 0x10, 0x180, 0x1C0,
                    (u32)(i32)reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3CC0)->loadedSprite->startPixelInclusive.x,
                    (u32)(i32)reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3CC0)->loadedSprite->startPixelInclusive.y,
                    (u32)(i32)reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3CC0)->loadedSprite->widthPx,
                    (u32)(i32)reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3CC0)->loadedSprite->heightPx);

                for (i = 0; i < 8; i++)
                {
                    g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(
                        reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x3F64 + i * sizeof(AnmVm)), 2);
                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x4084 + i * sizeof(AnmVm)) = i * 4 + 3;
                    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4157 + i * sizeof(AnmVm)) = 64 - i * 2;
                }
            }
            else
            {
                g_GameManager.globals->pointItemExtendsSoFar = -1;
            }

            if (g_GameManager.currentStage != STAGE6B && g_GameManager.currentStage != STAGE6A &&
                g_GameManager.currentStage != EXTRASTAGE && g_GameManager.GetBombsRemaining() < 3 &&
                (g_GameManager.shotType == SHOT_YOUMU_YUYUKO || g_GameManager.shotType == SHOT_YOUMU ||
                 g_GameManager.shotType == SHOT_YUYUKO))
            {
                g_GameManager.AddToBombCount(1);
                g_SoundPlayer.PlaySoundByIdx((SoundIdx)0x23, 0);
                g_Gui.flags.bombDisplayUpdateFrames = 2;
            }
            break;
        case 0xA:
            goto run_scripts;
        case 0xC:
            g_Supervisor.FadeOutMusic(4.0f);
            break;
        case 0xE:
            ScreenEffect::RegisterChain((ScreenEffectType)4, 442, 0xffffff, 0, 0, 21);
            g_GuiMessageScreenEffectDuration = 442;
            break;
        case 0xB:
            if (g_GameManager.currentStage == STAGE6A || g_GameManager.currentStage == STAGE6B ||
                g_GameManager.currentStage == EXTRASTAGE)
                g_GameManager.flags.unk5_6 = 2;
            goto run_scripts;
        case 0xD:
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueSkippable =
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args[0];
            break;
        case 0x12:
            reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->messageFlag =
                reinterpret_cast<GuiRawMessageInstruction *>(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args[0];
            break;

        }

        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr =
            reinterpret_cast<u8 *>(
                reinterpret_cast<i32>(&reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->args) +
                reinterpret_cast<GuiRawMessageInstruction *>(
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr)->argSize);
    }

    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer++;

run_scripts:
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->extraVms[0]);
    g_AnmManager->ExecuteScript(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->extraVms[1]);

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer < 60 &&
        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueSkippable &&
        (g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer = 60;

    return 0;
}

// FUNCTION: th08 0x4353ec
#pragma var_order(i, decoded)
void __fastcall FUN_004353ec(char *out, const char *encoded)
{
    char decoded;
    i32 i = 0;
    do
    {
        decoded = *encoded ^ 0x77;
        out[i] = decoded;
        i++;
        encoded++;
    } while (decoded != '\0');
}

// FUNCTION: th08 0x43542b
#pragma var_order(dialogueBoxHeight, vertices)
ZunResult GuiImpl::DrawDialogue()
{
    f32 dialogueBoxHeight;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentMsgIdx < 0)
        return ZUN_ERROR;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer < 60)
        dialogueBoxHeight = static_cast<f32>(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->timer) *
                            48.0f / 60.0f;
    else
        dialogueBoxHeight = 48.0f;

    VertexDiffuseXyzrhw vertices[4];
    memcpy(&vertices[0].pos, &Float3(g_GameManager.arcadeRegionTopLeftPos.x + 16.0f, 384.0f, 0.0f), sizeof(Float3));
    memcpy(&vertices[1].pos,
           &Float3(g_GameManager.arcadeRegionTopLeftPos.x + 384.0f - 16.0f, 384.0f, 0.0f), sizeof(Float3));
    memcpy(&vertices[2].pos,
           &Float3(g_GameManager.arcadeRegionTopLeftPos.x + 16.0f, 384.0f + dialogueBoxHeight, 0.0f), sizeof(Float3));
    memcpy(&vertices[3].pos,
           &Float3(g_GameManager.arcadeRegionTopLeftPos.x + 384.0f - 16.0f, 384.0f + dialogueBoxHeight, 0.0f),
           sizeof(Float3));

    vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
    vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0].pos.z >=
        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1].pos.z)
    {
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0]);
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1]);
    }
    else
    {
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[1]);
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[0]);
    }

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2].pos.z >=
        reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3].pos.z)
    {
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2]);
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3]);
    }
    else
    {
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[3]);
        g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->portraits[2]);
    }

    g_AnmManager->FlushVertexBuffer();

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->messageFlag)
    {
        if (!g_Supervisor.IsColorCompositingDisabled())
        {
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        }
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        if (!g_Supervisor.IsDepthTestDisabled())
            g_Supervisor.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrhw));
        g_AnmManager->ClearVertexShader();
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

    g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0]);
    g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1]);
    g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->extraVms[0]);
    g_AnmManager->DrawNoRotation(&reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->extraVms[1]);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x43587e
i32 Gui::MsgWait()
{
    if (this->impl == NULL)
        return 0;
    if (*(u32 *)((u8 *)this->impl + 0x22D78) > 0)
        return 0;
    return *(i32 *)((u8 *)this->impl + 0x2181C) >= 0;
}

// FUNCTION: th08 0x4358bb
i32 Gui::IsDialogPresent()
{
    if (this->impl == NULL)
        return 0;
    return *(i32 *)((u8 *)this->impl + 0x2181C) >= 0 || *(i32 *)((u8 *)this->impl + 0x2181C) == -2;
}

// FUNCTION: th08 0x435900
#pragma var_order(i, remaining, j, score, k)
void Gui::FUN_00435900()
{
    i32 i;
    i32 remaining;
    i32 j;
    i32 score;
    i32 k;

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->currentMsgIdx < 0)
    {
        if (this->bossPresent)
        {
            if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) == 0)
            {
                this->impl->vm0000[12].SetInterrupt(1);
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) = 1;
                this->bossUIOpacity = 0;
            }
            else
            {
                if (this->impl->vm0000[12].FUN_004396f8())
                    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) = 2;
                if (this->bossUIOpacity < 0xfc)
                    this->bossUIOpacity += 4;
                else
                    this->bossUIOpacity = 0xff;
            }
        }
        else if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) != 0)
        {
            if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) <= 2)
            {
                this->impl->vm0000[12].SetInterrupt(2);
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) = 3;
            }
            if (this->bossUIOpacity > 0)
                this->bossUIOpacity -= 4;
            else
                this->bossUIOpacity = 0;
            if (this->impl->vm0000[12].FUN_004396f8())
            {
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) = 0;
                this->bossLifeBarMaxSize = 0.0f;
                this->bossUIOpacity = 0;
            }
        }

        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) >= 2)
        {
            if (this->bossLifeBarSize > this->bossLifeBarMaxSize)
            {
                this->bossLifeBarMaxSize += 0.01f;
                if (this->bossLifeBarSize < this->bossLifeBarMaxSize)
                    this->bossLifeBarMaxSize = this->bossLifeBarSize;
            }
            else if (this->bossLifeBarSize < this->bossLifeBarMaxSize)
            {
                this->bossLifeBarMaxSize -= 0.02f;
                if (this->bossLifeBarSize > this->bossLifeBarMaxSize)
                    this->bossLifeBarMaxSize = this->bossLifeBarSize;
            }
        }
    }

    g_AnmManager->ExecuteScriptArray(this->impl->vm0000, 16);
    g_AnmManager->ExecuteScriptArray(this->impl->vm2a44, 4);
    if (!g_GameManager.flags.isSpellPractice && this->impl->vm2a44[0].color1.a)
        g_AnmManager->ExecuteScriptArray(&this->impl->vm34d4, 1);

    g_AnmManager->ExecuteScript(&this->impl->vm212c8);
    g_AnmManager->ExecuteScript(&this->impl->vm2156c);

    if (this->impl->vm2156c.color1.a)
    {
        if (EclOperands::g_TargetPlayerPosition017D61AC.x >= 64.0f &&
            EclOperands::g_TargetPlayerPosition017D61AC.y < 128.0f)
        {
            if (this->impl->vm2156c.color1.a > 0x40)
                this->impl->vm2156c.color1.a -= 4;
        }
        else if (this->impl->vm2156c.color1.a < 0xff)
        {
            if (this->impl->vm2156c.color1.a <= 0xfb)
                this->impl->vm2156c.color1.a += 4;
            else
                this->impl->vm2156c.color1.a = 0xff;
        }
    }

    g_AnmManager->ExecuteScript(&this->impl->vm5484);
    g_AnmManager->ExecuteScript(&this->impl->vm22e14);

    if (this->impl->vm3778.activeSpriteIndex >= 0)
    {
        if (g_AnmManager->ExecuteScript(&this->impl->vm3778))
            this->impl->vm3778.activeSpriteIndex = -1;
        if (g_AnmManager->ExecuteScript(&this->impl->vm3cc0))
            this->impl->vm3cc0.activeSpriteIndex = -1;
        for (i = 0; i < ARRAY_SIZE(this->impl->vm3f64); i++)
            g_AnmManager->ExecuteScript(&this->impl->vm3f64[i]);
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this->impl) + 0x21810) != 0)
    {
        remaining = 0xa8;
        for (j = 0; j < 0xa8; j++)
        {
            if (g_AnmManager->ExecuteScript(&this->impl->vm5728[j]))
                remaining--;
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this->impl) + 0x21810) = remaining;
    }

    if (this->impl->formatted0.isShown)
    {
        if (this->impl->formatted0.timer < 30)
            this->impl->formatted0.position.x =
                static_cast<f32>(this->impl->formatted0.timer) * -312.0f / 30.0f + 416.0f;
        else
            this->impl->formatted0.position.x = 104.0f;
        if (this->impl->formatted0.timer >= 250)
            this->impl->formatted0.isShown = 0;
        this->impl->formatted0.timer++;
    }

    if (this->impl->formatted1.isShown)
    {
        if (this->impl->formatted1.timer < 30)
            this->impl->formatted1.position.x =
                static_cast<f32>(this->impl->formatted1.timer) * -312.0f / 30.0f + 416.0f;
        else
            this->impl->formatted1.position.x = 104.0f;
        if (this->impl->formatted1.timer >= 180)
            this->impl->formatted1.isShown = 0;
        this->impl->formatted1.timer++;
    }

    if (this->impl->formatted2.isShown)
    {
        if (this->impl->formatted2.timer >= 280)
            this->impl->formatted2.isShown = 0;
        this->impl->formatted2.timer++;
    }

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->resultState == 1)
    {
        score = 0;
        score += reinterpret_cast<GuiStageResultUpdateOverlay *>(
                     reinterpret_cast<u8 *>(this->impl) + 0x22dec)->baseScore;
        score += reinterpret_cast<GuiStageResultUpdateOverlay *>(
                     reinterpret_cast<u8 *>(this->impl) + 0x22dec)->grazeInStage * 50;
        score += reinterpret_cast<GuiStageResultUpdateOverlay *>(
                     reinterpret_cast<u8 *>(this->impl) + 0x22dec)->pointItemsCollectedInStage * 5000;
        score += reinterpret_cast<GuiStageResultUpdateOverlay *>(
                     reinterpret_cast<u8 *>(this->impl) + 0x22dec)->timeOrbs * 100;

        if (g_GameManager.currentStage >= STAGE6A && !g_GameManager.IsPracticeMode())
        {
            score += 2500000 * g_GameManager.GetLives();
            score += 500000 * g_GameManager.GetBombsRemaining();
        }
        if (g_GameManager.currentStage == STAGE6B)
            score += 2000000 * (12 - static_cast<i8>(g_GameManager.GetClockTime()));

        switch (g_GameManager.difficulty)
        {
        case EASY:
            score /= 2;
            break;
        case HARD:
            score = score * 12 / 10;
            break;
        case LUNATIC:
            score = score * 15 / 10;
            break;
        case EXTRA:
            score *= 2;
            break;
        default:
            break;
        }

        switch (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(g_GameManager.cfg) + 0x1c))
        {
        case 3:
            score = score * 5 / 10;
            break;
        case 4:
            score = score * 2 / 10;
            break;
        case 5:
            score /= 10;
            break;
        case 6:
            score /= 20;
            break;
        default:
            break;
        }

        reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->unknown1574 = score;
        for (k = 0; k < 10; k++)
            g_GameManager.AddScore(score);
        reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->resultState++;
    }

    if (g_GameManager.currentStage < STAGE6A &&
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent != 0 &&
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent >=
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTarget &&
        g_GameManager.flags.unk5_6 == 0)
    {
        g_GameManager.flags.unk5_6 = 2;
    }

    if (reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent != 0 &&
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent !=
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTarget)
    {
        if (reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTimer >= 60)
        {
            if (reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent <
                reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTarget)
            {
                reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent++;
                if ((g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) || (g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
                {
                    reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent += 3;
                }
                if (reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent >
                    reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTarget)
                {
                    reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent =
                        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTarget;
                }
            }
            else
            {
                reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTimer++;
            }
        }
        else
        {
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayTimer++;
        }
    }
}

// FUNCTION: th08 0x43625d
#pragma var_order(yPos, xPos, idx, vm)
void Gui::DrawGameScene()
{
    AnmVm *vm;
    i32 idx;
    f32 xPos;
    f32 yPos;

    g_AnmManager->FlushVertexBuffer();
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    if (!g_Supervisor.IsMinimumGraphicsMode())
    {
        vm = &this->impl->vm0000[15];
        xPos = 480.0f;
        vm->pos = Float3(xPos, 40.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        vm->pos = Float3(xPos, 56.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        if (this->flags.lifeDisplayUpdateFrames)
        {
            vm->pos = Float3(xPos, 88.0f, 0.48f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.bombDisplayUpdateFrames)
        {
            vm->pos = Float3(xPos, 104.0f, 0.48f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.powerDisplayUpdateFrames)
        {
            vm->pos = Float3(xPos, 136.0f, 0.48f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.grazeDisplayUpdateFrames)
        {
            vm->pos = Float3(xPos, 152.0f, 0.48f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.pointDisplayUpdateFrames)
        {
            vm->pos = Float3(xPos, 168.0f, 0.48f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.timeDisplayUpdateFrames)
        {
            vm->pos = Float3(xPos, 184.0f, 0.48f);
            g_AnmManager->DrawNoRotation(vm);
        }
        vm->pos = Float3(512.0f, 464.0f, 0.48f);
        g_AnmManager->DrawNoRotation(vm);
    }

    vm = &this->impl->vm0000[13];
    if (g_Supervisor.IsHUDRedrawEnabled() || vm->currentInstruction != NULL || g_GuiFullPowerModeFrames != 0)
    {
        for (yPos = 0.0f; yPos < 464.0f; yPos += 32.0f)
        {
            vm->pos = Float3(0.0f, yPos, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        for (xPos = 416.0f; xPos < 624.0f; xPos += 32.0f)
        {
            for (yPos = 16.0f; yPos < 464.0f; yPos += 32.0f)
            {
                vm->pos = Float3(xPos, yPos, 0.49f);
                g_AnmManager->DrawNoRotation(vm);
            }
        }
        vm = &this->impl->vm0000[14];
        for (xPos = 0.0f; xPos < 624.0f; xPos += 128.0f)
        {
            vm->pos = Float3(xPos, 0.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
            vm->pos = Float3(xPos, 464.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[0]);
        g_AnmManager->Draw2D(&this->impl->vm0000[1]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[2]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[3]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[4]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[5]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[6]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[7]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[8]);
        g_AnmManager->DrawNoRotation(&this->impl->vm0000[9]);
        g_AnmManager->DrawNoRotation(&this->impl->vm22e14);
        this->flags.lifeDisplayUpdateFrames = 2;
        this->flags.bombDisplayUpdateFrames = 2;
        this->flags.grazeDisplayUpdateFrames = 2;
        this->flags.pointDisplayUpdateFrames = 2;
        this->flags.powerDisplayUpdateFrames = 2;
        this->flags.timeDisplayUpdateFrames = 2;
    }

    if (this->flags.lifeDisplayUpdateFrames)
    {
        vm = &this->impl->vm0000[10];
        for (idx = 0, xPos = 488.0f; idx < g_GameManager.GetLives(); idx++, xPos += 16.0f)
        {
            vm->pos = Float3(xPos, 88.0f, 0.46f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if (this->flags.bombDisplayUpdateFrames)
    {
        vm = &this->impl->vm0000[11];
        for (idx = 0, xPos = 488.0f; idx < g_GameManager.GetBombsRemaining(); idx++, xPos += 16.0f)
        {
            vm->pos = Float3(xPos, 104.0f, 0.46f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if ((this->flags.bombDisplayUpdateFrames || this->flags.lifeDisplayUpdateFrames) &&
        (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) == 1) && g_Spellcard.IsActive())
    {
        g_AnmManager->DrawNoRotation(&this->impl->vm5484);
    }

    vm = &this->impl->vm0000[14];
    for (xPos = 32.0f; xPos < 368.0f; xPos += 128.0f)
    {
        vm->pos = Float3(xPos, 464.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
    }

    {
        Float3 elemPos(488.0f, 56.0f, 0.0f);
        g_AsciiManager.AddFormatText(&elemPos, "%.9d", g_GameManager.globals->displayScore);
        elemPos.x += 117.0f;
        g_AsciiManager.AddFormatText(&elemPos, "%1d",
                                     g_GameManager.globals->numRetries > 9 ? 9 : g_GameManager.globals->numRetries);
        g_AsciiManager.SetScale(1.0f, 1.0f);

        elemPos = Float3(488.0f, 40.0f, 0.0f);
        g_AsciiManager.AddFormatText(&elemPos, "%.9d", g_GameManager.globals->displayedHighScore);
        elemPos.x += 117.0f;
        g_AsciiManager.AddFormatText(
            &elemPos, "%1d", g_GameManager.globals->continuesUsedInHighScore > 9
                                 ? 9
                                 : g_GameManager.globals->continuesUsedInHighScore);
        g_AsciiManager.SetScale(1.0f, 1.0f);

        if (this->flags.grazeDisplayUpdateFrames || g_Supervisor.IsMinimumGraphicsMode())
        {
            elemPos = Float3(488.0f, 152.0f, 0.0f);
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.globals->graze);
        }
        if (this->flags.pointDisplayUpdateFrames || g_Supervisor.IsMinimumGraphicsMode())
        {
            elemPos = Float3(488.0f, 168.0f, 0.0f);
            elemPos.x += g_AsciiManager.AddFormatText2(&elemPos, "%d", g_GameManager.globals->pointItemsCollected) * 13;
            g_AsciiManager.SetScale(0.5f, 1.0f);
            g_AsciiManager.AddFormatText(&elemPos, "/");
            g_AsciiManager.SetScale(1.0f, 1.0f);
            elemPos.x += 6.0f;
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.globals->nextPointItemExtendThreshold);
        }
        if (this->flags.timeDisplayUpdateFrames || g_Supervisor.IsMinimumGraphicsMode())
        {
            if (g_GameManager.GetTimeOrbs() >= g_GameManager.GetLastSpellTimeOrbThreshold())
                g_AsciiManager.SetColor(0xfffff0c0);
            elemPos = Float3(488.0f, 184.0f, 0.0f);
            elemPos.x += g_AsciiManager.AddFormatText2(&elemPos, "%d", g_GameManager.GetTimeOrbs()) * 13;
            g_AsciiManager.SetScale(0.5f, 1.0f);
            g_AsciiManager.AddFormatText(&elemPos, "/");
            g_AsciiManager.SetScale(1.0f, 1.0f);
            elemPos.x += 6.0f;
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.GetLastSpellTimeOrbThreshold());
            g_AsciiManager.SetColor(0xffffffff);
        }
    }

    g_AnmManager->FlushVertexBuffer();
    if (this->flags.powerDisplayUpdateFrames || g_Supervisor.IsMinimumGraphicsMode())
    {
        VertexDiffuseXyzrhw vertices[4];
        if (g_GameManager.GetPower() > 0)
        {
            vertices[0].pos = Float3(488.0f, 136.0f, 0.1f);
            vertices[1].pos = Float3(g_GameManager.GetPower() + 488 + 0.0f, 136.0f, 0.1f);
            vertices[2].pos = Float3(488.0f, 152.0f, 0.1f);
            vertices[3].pos = Float3(g_GameManager.GetPower() + 488 + 0.0f, 152.0f, 0.1f);
            vertices[0].diffuse = vertices[2].diffuse = 0xe0e0e0ff;
            vertices[1].diffuse = vertices[3].diffuse = 0x80e0e0ff;
            vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;

            if (!g_Supervisor.IsColorCompositingDisabled())
            {
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            }
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            if (!g_Supervisor.IsDepthTestDisabled())
                g_Supervisor.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
            g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrhw));
            g_AnmManager->ClearVertexShader();
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
        if (g_GameManager.GetPower() < 128)
        {
            g_AsciiManager.AddFormatText(&Float3(488.0f, 136.0f, 0.0f), "%d", g_GameManager.GetPower());
        }
        else
        {
            g_AsciiManager.AddFormatText(&Float3(488.0f, 136.0f, 0.0f), "MAX");
        }
    }

    if (this->flags.lifeDisplayUpdateFrames)
        this->flags.lifeDisplayUpdateFrames--;
    if (this->flags.powerDisplayUpdateFrames)
        this->flags.powerDisplayUpdateFrames--;
    if (this->flags.bombDisplayUpdateFrames)
        this->flags.bombDisplayUpdateFrames--;
    if (this->flags.grazeDisplayUpdateFrames)
        this->flags.grazeDisplayUpdateFrames--;
    if (this->flags.pointDisplayUpdateFrames)
        this->flags.pointDisplayUpdateFrames--;
    if (this->flags.timeDisplayUpdateFrames)
        this->flags.timeDisplayUpdateFrames--;
}

// FUNCTION: th08 0x43741d
void Gui::FUN_0043741d()
{
    i32 i;

    for (i = 0; i < 4; i++)
        g_AnmManager->Draw2D(&this->impl->vm2a44[i]);
    g_AnmManager->Draw2D(&this->impl->vm34d4);
    g_AnmManager->Draw2D(&this->impl->vm2156c);

    if (this->impl->vm3778.activeSpriteIndex >= 0)
    {
        g_AnmManager->DrawNoRotation(&this->impl->vm3778);
        g_AnmManager->FUN_00464070(&this->impl->vm3cc0);
        for (i = 0; i < ARRAY_SIZE(this->impl->vm3f64); i++)
            g_AnmManager->FUN_00464070(&this->impl->vm3f64[i]);
        if (this->impl->vm3a1c.activeSpriteIndex >= 0)
        {
            this->impl->vm3a1c.pos = Float3(304.0f, 448.0f, 0.0f);
            g_AnmManager->DrawNoRotation(&this->impl->vm3a1c);
        }
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this->impl) + 0x21810) != 0)
    {
        for (i = 0; i < 0xa8; i++)
        {
            g_AnmManager->FUN_00464070(&this->impl->vm5728[i]);
            g_AnmManager->ClearSprite();
        }
    }

    if (reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->currentMsgIdx < 0 &&
        (this->bossPresent + *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40)) > 0)
    {
#pragma var_order(bossColorDark, bossColor, rect, bossValue, segmentIndex, segmentStop, bossTimerColor, segmentWidth, textPos)
        ZunRect rect;
        D3DCOLOR bossColor;
        D3DCOLOR bossColorDark;
        i32 bossValue;
        i32 segmentIndex;

        rect.left = 64.0f;
        rect.top = 19.0f;
        rect.right = this->bossLifeBarMaxSize * 320.0f + 64.0f;
        rect.bottom = 23.0f;
        bossColor = (this->bossUIOpacity << 24) | 0x00ffffff;
        bossColorDark = (this->bossUIOpacity << 24) | 0x00202060;
        Float3 textPos(48.0f, 16.0f, 0.0f);
        ScreenEffect::DrawSquareShaded(&rect, bossColor, bossColor, bossColorDark, bossColorDark);

        f32 segmentStop;
        for (segmentIndex = 0; segmentIndex < MAX_BOSS_LIFEBAR_SEGMENTS; segmentIndex++)
        {
            if (this->bossLifeBarSegmentStop[segmentIndex] == 0.0f)
                continue;
            if (this->bossLifeBarSegmentStart[segmentIndex] >= this->bossLifeBarMaxSize)
                continue;

            segmentStop = this->bossLifeBarSegmentStop[segmentIndex];
            if (this->bossLifeBarMaxSize < segmentStop)
                segmentStop = this->bossLifeBarMaxSize;

            rect.left = this->bossLifeBarSegmentStart[segmentIndex] * 320.0f + 64.0f;
            rect.top = 19.0f;
            rect.right = segmentStop * 320.0f + 64.0f;
            rect.bottom = 23.0f;
            bossColor = (this->bossUIOpacity << 24) | (this->bossLifeBarSegmentColor[segmentIndex] & 0x00ffffff);
            bossColorDark = (this->bossUIOpacity << 24) |
                            ((this->bossLifeBarSegmentColor[segmentIndex] >> 2) & 0x003f3f3f);
            ScreenEffect::DrawSquareShaded(&rect, bossColor, bossColor, bossColorDark, bossColorDark);
        }

        g_AnmManager->DrawNoRotation(&this->impl->vm0000[12]);

        i32 segmentWidth;
        {
            rect.left = 33.0f;
            rect.top = 19.0f;
            rect.right = rect.left + 3.0f;
            rect.bottom = rect.top + 4.0f;
            bossValue = this->eclSetLives;
            segmentWidth = this->eclSetLives <= 5 ? 2 : 1;
            for (segmentIndex = 0; segmentIndex < bossValue; segmentIndex++)
            {
                rect.left = segmentIndex * 26.0f / bossValue + 35.0f;
                rect.right = (segmentIndex + 1) * 26.0f / bossValue + 35.0f - segmentWidth;
                bossColor = (this->bossUIOpacity << 24) | (0x00ffffff - segmentIndex * 0xff / 9);
                bossColorDark = (this->bossUIOpacity << 24) | 0x00202020;
                ScreenEffect::DrawSquareShaded(&rect, bossColor, bossColor, bossColorDark, bossColorDark);
            }
        }

        i32 bossTimerColor;
        {
            textPos = Float3(384.0f, 16.0f, 0.0f);
            if (this->spellcardSecondsRemaining >= 20)
                bossTimerColor = g_GuiBossTimerColors[0];
            else if (this->spellcardSecondsRemaining >= 10)
                bossTimerColor = g_GuiBossTimerColors[1];
            else if (this->spellcardSecondsRemaining >= 5)
                bossTimerColor = g_GuiBossTimerColors[2];
            else
                bossTimerColor = g_GuiBossTimerColors[3];

            g_AsciiManager.SetColor((this->bossUIOpacity << 24) | bossTimerColor);
            bossValue = this->spellcardSecondsRemaining > 99 ? 99 : this->spellcardSecondsRemaining;
            if (this->previousSpellcardSecondsRemaining != this->spellcardSecondsRemaining)
            {
                if (bossValue < 3)
                    g_SoundPlayer.PlaySoundByIdx((SoundIdx)0x26, 0);
                else if (bossValue < 10)
                    g_SoundPlayer.PlaySoundByIdx((SoundIdx)0x1d, 0);
            }
            g_AsciiManager.AddFormatText(&textPos, "%.2d", bossValue);
            g_AsciiManager.SetColor(0xffffffff);
            this->previousSpellcardSecondsRemaining = this->spellcardSecondsRemaining;

            if (!g_GameManager.isInGameMenu && !g_GameManager.showRetryMenu && !g_GameManager.flags.unk10 &&
                g_EnemyManager.bosses[0] != NULL)
            {
                textPos = Float3(2.0f, 29.0f, 0.0f);
                g_AsciiManager.SetScale(1.0f, 1.0f);
                g_AsciiManager.CreateFamiliarPopup(
                    &textPos,
                    reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(
                        g_EnemyManager.bosses[0])->CountParentChain(),
                    g_EnemyManager.bosses[0]->linkedChildCount,
                    0xfff0f00f);
            }
        }
    }

    g_AnmManager->DrawNoRotation(&this->impl->vm212c8);
}

// FUNCTION: th08 0x437a2f
ZunResult Gui::AddedCallback(Gui *gui)
{
    return gui->ActualAddedCallback();
}

// FUNCTION: th08 0x437a40
ZunResult Gui::DeletedCallback(Gui *gui)
{
    if (!FUN_00438ffd())
    {
        g_AnmManager->ReleaseAnm(13);
    }

    gui->FreeMsgFile();

    if (FUN_00438ff3())
    {
        g_AnmManager->ReleaseAnm(10);
        g_AnmManager->ReleaseAnm(12);
        g_AnmManager->ReleaseAnm(11);
        g_AnmManager->ReleaseAnm(14);
        ZUN_DELETE(gui->impl);
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x437ad0
ZunResult Gui::RegisterChain()
{
    Gui *gui = &g_Gui;

    if (FUN_00438fe9())
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = ZUN_NEW(GuiImpl, "GUI");
    }

    g_GuiCalcChain.SetCallback((ChainCallback)Gui::OnUpdate);
    g_GuiCalcChain.addedCallback = (ChainLifetimeCallback)Gui::AddedCallback;
    g_GuiCalcChain.deletedCallback = (ChainLifetimeCallback)Gui::DeletedCallback;
    g_GuiCalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcChain, 15) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_GuiDrawChain.SetCallback((ChainCallback)Gui::OnDraw);
    g_GuiDrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawChain, 17);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x437bc4
GuiImpl::GuiImpl()
{
}

// FUNCTION: th08 0x437ce2
GuiMsgVm::GuiMsgVm()
{
}

// FUNCTION: th08 0x437d45
GuiFormattedText::GuiFormattedText()
{
}

// FUNCTION: th08 0x437d64
void Gui::CutChain()
{
    g_Chain.Cut(&g_GuiCalcChain);
    g_Chain.Cut(&g_GuiDrawChain);
}

// FUNCTION: th08 0x437d87
i32 Gui::FUN_00437d87()
{
    return this->impl->vm3778.activeSpriteIndex >= 0 &&
           this->impl->vm3778.FUN_004396f8();
}

// FUNCTION: th08 0x437dc7
i32 Gui::FUN_00437dc7()
{
    return this->impl->msgVm.unk1568;
}

// FUNCTION: th08 0x437ddd
void Gui::FUN_00437ddd(i32 value)
{
    this->impl->formatted0.position = Float3(416.0f, 48.0f, 0.0f);
    this->impl->formatted0.isShown = 1;
    this->impl->formatted0.timer = 0;
    this->impl->formatted0.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437e5d
void Gui::FUN_00437e5d(i32 value, i32 isShown)
{
    this->impl->formatted1.position = Float3(416.0f, 168.0f, 0.0f);
    this->impl->formatted1.isShown = isShown;
    this->impl->formatted1.timer = 0;
    this->impl->formatted1.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437edc
void Gui::gui_fun_00437edc(i32 value)
{
    this->impl->formatted2.position = Float3(224.0f, 16.0f, 0.0f);
    this->impl->formatted2.isShown = 1;
    this->impl->formatted2.timer = 0;
    this->impl->formatted2.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437f5c
#pragma var_order(srcRect, destRect)
void __fastcall FUN_00437f5c(i32 spriteIdx)
{
    RECT destRect;
    RECT srcRect;

    destRect.left = (i32)g_Gui.stageTextAnm->GetSprite(10)->startPixelInclusive.x;
    destRect.top = (i32)g_Gui.stageTextAnm->GetSprite(10)->startPixelInclusive.y;
    destRect.right = (i32)g_Gui.stageTextAnm->GetSprite(10)->endPixelInclusive.x;
    destRect.bottom = (i32)g_Gui.stageTextAnm->GetSprite(10)->endPixelInclusive.y;

    srcRect.left = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->startPixelInclusive.x;
    srcRect.top = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->startPixelInclusive.y;
    srcRect.right = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->endPixelInclusive.x;
    srcRect.bottom = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->endPixelInclusive.y;

    g_AnmManager->CopyTextureRect(10, 0, 10, 1, &destRect, &srcRect);
}

// FUNCTION: th08 0x438046
void FUN_00438046()
{
    switch (g_GameManager.currentStage)
    {
    default:
        FUN_00437f5c(16);
        break;
    case STAGE2:
        FUN_00437f5c(17);
        break;
    case STAGE3:
        FUN_00437f5c(18);
        break;
    case STAGE4A:
        if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberEqualTo(214))
        {
            FUN_00437f5c(19);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(216))
        {
            FUN_00437f5c(26);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(217))
        {
            FUN_00437f5c(27);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(218))
        {
            FUN_00437f5c(28);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(219))
        {
            FUN_00437f5c(29);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(220))
        {
            FUN_00437f5c(30);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(221))
        {
            FUN_00437f5c(31);
        }
        break;
    case STAGE4B:
        FUN_00437f5c(20);
        break;
    case STAGE5:
        if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberEqualTo(212))
            FUN_00437f5c(21);
        else
            FUN_00437f5c(22);
        break;
    case STAGE6A:
        FUN_00437f5c(23);
        break;
    case STAGE6B:
        if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberInRange(147, 150))
            FUN_00437f5c(23);
        else
            FUN_00437f5c(24);
        break;
    case EXTRASTAGE:
        if (!g_GameManager.IsSpellPractice() ||
            g_GameManager.IsSpellNumberInRange(191, 193) ||
            g_GameManager.IsSpellNumberEqualTo(213))
            FUN_00437f5c(32);
        else
            FUN_00437f5c(25);
        break;
    }
}

// FUNCTION: th08 0x43826b
void Gui::FUN_0043826b()
{
    Float3 stringPos(120.0f, 96.0f, 0.0f);

    g_AsciiManager.SetColor(0xffffff40);
    if (g_GameManager.currentStage < STAGE6A)
    {
        g_AsciiManager.AddFormatText(&stringPos, "Stage Clear");
    }
    else
    {
        if (g_GameManager.currentStage >= STAGE6B)
            stringPos.y -= 16.0f;
        g_AsciiManager.AddFormatText(&stringPos, "All Clear!");
    }

    stringPos.y += 32.0f;
    g_AsciiManager.SetColor(0xffffffff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Clear = %8d0",
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->baseScore);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffe0e0ff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Point = %8d0",
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                ->pointItemsCollectedInStage * 5000);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffd0d0ff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Graze = %8d0",
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->grazeInStage * 50);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffd0d0ff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Time  = %8d0",
        reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->timeOrbs * 100);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffd0d0ff);
    stringPos.y += 16.0f;
    g_AsciiManager.AddFormatText(&stringPos, "over-80%% = %3d.%.2d%%",
                                 100 * (i32)g_GameManager.stageExtremeHumanFrames / (i32)g_GameManager.stageActiveFrames,
                                 10000 * (i32)g_GameManager.stageExtremeHumanFrames / (i32)g_GameManager.stageActiveFrames % 100);
    stringPos.y += 16.0f;
    g_AsciiManager.AddFormatText(&stringPos, "over 80%% = %3d.%.2d%%",
                                 100 * (i32)g_GameManager.stageExtremeYoukaiFrames / (i32)g_GameManager.stageActiveFrames,
                                 10000 * (i32)g_GameManager.stageExtremeYoukaiFrames / (i32)g_GameManager.stageActiveFrames % 100);

    if (g_GameManager.currentStage >= STAGE6A && !g_GameManager.IsPracticeMode() && !g_GameManager.IsReplayPractice())
    {
        stringPos.y += 16.0f;
        g_AsciiManager.SetColor(0xffffff80);
        g_AsciiManager.AddFormatText(&stringPos, "Player = %8d0", g_GameManager.GetLives() * 2500000);
        stringPos.y += 16.0f;
        g_AsciiManager.SetColor(0xffffff80);
        g_AsciiManager.AddFormatText(&stringPos, "Bomb = %8d0", g_GameManager.GetBombsRemaining() * 500000);

        if (g_GameManager.currentStage == STAGE6B && !g_GameManager.IsPracticeMode() && !g_GameManager.IsReplayPractice())
        {
            stringPos.y += 16.0f;
            g_AsciiManager.SetColor(0xffffff80);
            g_AsciiManager.AddFormatText(
                &stringPos, "Last Time = %2d:%.2d",
                reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                        ->clockDisplayTarget / 60 % 12,
                reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                        ->clockDisplayTarget % 60);
            stringPos.y += 16.0f;
            g_AsciiManager.AddFormatText(&stringPos, "Night Bonus");
            stringPos.y += 16.0f;
            g_AsciiManager.AddFormatText(&stringPos, "        %8d0",
                                         (12 - (i8)g_GameManager.GetClockTime()) * 2000000);
        }
    }

    stringPos.y += 32.0f;
    switch (g_GameManager.difficulty)
    {
    case EASY:
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddFormatText(&stringPos, "Rank Easy    (0.5)");
        break;
    case NORMAL:
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddFormatText(&stringPos, "Rank Normal  (1.0)");
        break;
    case HARD:
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddFormatText(&stringPos, "Rank Hard    (1.2)");
        break;
    case LUNATIC:
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddFormatText(&stringPos, "Rank Lunatic (1.5)");
        break;
    case EXTRA:
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddFormatText(&stringPos, "Rank Extra   (2.0)");
        break;
    case 5:
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddFormatText(&stringPos, "Rank Phantasm(2.0)");
        break;
    default:
        break;
    }

    if (g_GameManager.difficulty < EXTRA && !g_GameManager.flags.isPracticeMode)
    {
        stringPos.y += 16.0f;
        switch (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(g_GameManager.cfg) + 0x1c))
        {
        case 3:
            g_AsciiManager.SetColor(0xffff8080);
            g_AsciiManager.AddFormatText(&stringPos, "Slowdown Penalty 50%%");
            stringPos.y += 16.0f;
            break;
        case 4:
            g_AsciiManager.SetColor(0xffff8080);
            g_AsciiManager.AddFormatText(&stringPos, "Slowdown Penalty 80%%");
            stringPos.y += 16.0f;
            break;
        case 5:
            g_AsciiManager.SetColor(0xffff8080);
            g_AsciiManager.AddFormatText(&stringPos, "Slowdown Penalty 90%%");
            stringPos.y += 16.0f;
            break;
        case 6:
            g_AsciiManager.SetColor(0xffff8080);
            g_AsciiManager.AddFormatText(&stringPos, "Slowdown Penalty 95%%");
            stringPos.y += 16.0f;
            break;
        default:
            break;
        }
    }

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffffffff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Total        %8d0",
        reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->unknown1574);
    g_AsciiManager.SetColor(0xffffffff);

    if (g_GameManager.currentStage <= STAGE5)
    {
        stringPos.y += 40.0f;
        stringPos.x = 120.0f;
        g_AsciiManager.SetColor(0xffdfdfdf);
        g_AsciiManager.AddFormatText(
            &stringPos, "%s%2d:%.2d",
            g_GuiTimePeriodLabels[(reinterpret_cast<GuiStageResultUpdateOverlay *>(
                reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayStart / 60) < 12],
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                    ->clockDisplayStart / 60 % 12,
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                    ->clockDisplayStart % 60);
        stringPos.x += 99.0f;
        g_AsciiManager.SetColor(0xffafafaf);
        g_AsciiManager.AddFormatText(&stringPos, ">>");
        stringPos.x += 34.0f;
        g_AsciiManager.SetColor(0xffff8f8f);
        g_AsciiManager.AddFormatText(
            &stringPos, "%s%2d:%.2d",
            g_GuiTimePeriodLabels[(reinterpret_cast<GuiStageResultUpdateOverlay *>(
                reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent / 60) < 12],
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                    ->clockDisplayCurrent / 60 % 12,
            reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)
                    ->clockDisplayCurrent % 60);
        g_AsciiManager.SetColor(0xffffffff);
    }
}

// FUNCTION: th08 0x438a89
void Gui::FUN_00438a89()
{
    char bonusText[32];

    g_AsciiManager.SetIsGuiMode(1);

    if (this->impl->formatted0.isShown)
    {
        g_AsciiManager.SetColor(0xffffff80);
        g_AsciiManager.AddFormatText(&this->impl->formatted0.position, " BONUS %8d", this->impl->formatted0.value);
        g_AsciiManager.SetColor(0xffffffff);
    }

    switch (this->impl->formatted1.isShown)
    {
    case 1:
        g_AsciiManager.SetColor(0xffc0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->formatted1.position, "Full Power Mode!");
        g_AsciiManager.SetColor(0xffffffff);
        break;
    case 2:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->formatted1.position, "Supernatural Border!!");
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    case 3:
        g_AsciiManager.SetColor(0xffc0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->formatted1.position, "CherryPoint Max!");
        g_AsciiManager.SetColor(0xffffffff);
        break;
    case 4:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->formatted1.position, "Border Bonus %7d", this->impl->formatted1.value);
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    case 5:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->formatted1.position, "Spell Bonus Failed");
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    case 6:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->formatted1.position, "Last Spell Failed");
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    default:
        break;
    }

    if (this->impl->formatted2.isShown)
    {
        g_AsciiManager.SetColor(0xffff0000);
        this->impl->formatted2.position.x = (384.0f - (f32)strlen("Spell Card Bonus!") * 14.0f) / 2.0f + 32.0f;
        this->impl->formatted2.position.y = 80.0f;
        g_AsciiManager.AddFormatText(&this->impl->formatted2.position, "Spell Card Bonus!");
        this->impl->formatted2.position.y += 16.0f;
        sprintf(bonusText, "+%d", this->impl->formatted2.value);
        this->impl->formatted2.position.x = (384.0f - strlen(bonusText) * 28.0f) / 2.0f + 32.0f;
        g_AsciiManager.SetScale(2.0f, 2.0f);
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddString(&this->impl->formatted2.position, bonusText);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetColor(0xffffffff);
    }

    g_AsciiManager.SetIsGuiMode(0);
}


// FUNCTION: th08 0x438f58
ZunResult Gui::FUN_00438f58()
{
    g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(&this->impl->vm3cc0, 1);
    return g_AnmManager->SetTextureCaptureParams(
        3, 32, 16, 384, 448,
        (i32)this->impl->vm3cc0.loadedSprite->startPixelInclusive.x,
        (i32)this->impl->vm3cc0.loadedSprite->startPixelInclusive.y,
        (i32)this->impl->vm3cc0.loadedSprite->widthPx,
        (i32)this->impl->vm3cc0.loadedSprite->heightPx);
}

// FUNCTION: th08 0x438fe9
i32 FUN_00438fe9()
{
    return g_Supervisor.unk164;
}


// FUNCTION: th08 0x438ff3
i32 FUN_00438ff3()
{
    return g_Supervisor.unk168;
}

// FUNCTION: th08 0x438ffd
i32 FUN_00438ffd()
{
    return g_Supervisor.unk16c;
}


// FUNCTION: th08 0x439007
i32 Gui::StartStageBackgroundSequence()
{
    this->timesAnm->ExecuteAnmIdx(&this->impl->vm2156c, 2);
    this->timesAnm->SetSprite(&this->impl->vm2156c, static_cast<i8>(g_GameManager.GetClockTime()));
    return 0;
}

// FUNCTION: th08 0x439050
ZunResult Gui::FUN_00439050()
{
    this->timesAnm->SetSprite(
        reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C),
        static_cast<i8>(g_GameManager.GetClockTime()));
    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C)->SetInterrupt(1);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x439093
ZunResult Gui::FUN_00439093()
{
    this->timesAnm->SetSprite(
        reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C),
        static_cast<i8>(g_GameManager.GetClockTime()));
    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C)->SetInterrupt(2);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4390d6
ZunResult Gui::FUN_004390d6()
{
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2175f) = 0;
    return ZUN_SUCCESS;
}

#pragma var_order(i, j, k)
// FUNCTION: th08 0x4390ee
ZunResult Gui::ActualAddedCallback()
{
    i32 i;
    i32 j;
    u32 k;

    if (FUN_00438fe9())
    {
        memset(this->impl, 0, sizeof(GuiImpl));

        this->frontAnm = g_AnmManager->PreloadAnm(10, "front.anm");
        if (this->frontAnm == NULL)
            return ZUN_ERROR;

        this->FUN_004396b8();

        this->timesAnm = g_AnmManager->PreloadAnm(14, "times.anm");
        if (this->timesAnm == NULL)
            return ZUN_ERROR;

        this->loadingPortraitAnm = g_AnmManager->PreloadAnm(12, g_GuiLoadingAnmPaths[g_GameManager.shotType]);
        if (this->loadingPortraitAnm == NULL)
            return ZUN_ERROR;

        g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->impl->vm5484, 26);
        g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->impl->vm22e14, 25);
        if (g_GameManager.IsSpellPractice() && g_GameManager.currentSpellCardNumber >= 205)
            g_AsciiManager.asciiAnm->SetSprite(&this->impl->vm22e14, 288);
        else
            g_AsciiManager.asciiAnm->SetSprite(&this->impl->vm22e14, g_GameManager.difficulty + 283);
    }
    else
    {
        this->FUN_004396b8();
        g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(&this->impl->vm3cc0, 1);
        this->impl->vm3cc0.pendingInterrupt = 1;

        for (i = 0; i < 14; i++)
        {
            for (j = 0; j < 12; j++)
            {
                g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(&this->impl->vm5728[i * 12 + j], ((i + j) & 1) + 3);
                this->impl->vm5728[i * 12 + j].counterVar0 = i + j * 2;
                this->impl->vm5728[i * 12 + j].pos.x = j * 32.0f - 0.5f + 16.0f;
                this->impl->vm5728[i * 12 + j].pos.y = i * 32.0f - 0.5f + 16.0f;
                this->impl->vm5728[i * 12 + j].pos.z = 0.0f;
                this->impl->vm5728[i * 12 + j].uvScrollPos.x = j * 32.0f / 512.0f;
                this->impl->vm5728[i * 12 + j].uvScrollPos.y = i * 32.0f / 512.0f;
            }
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this->impl) + 0x21810) = 168;
    }

    g_Gui.FUN_004390d6();
    this->timesAnm->ExecuteAnmIdx(&this->impl->vm34d4, 0);
    this->timesAnm->SetSprite(&this->impl->vm34d4, static_cast<i8>(g_GameManager.GetClockTime()));

    if (!g_GameManager.IsSpellPractice() &&
        this->LoadMsg(g_GuiMessagePaths[g_GameManager.currentStage][g_GameManager.shotType]) != ZUN_SUCCESS)
        return ZUN_ERROR;

    if (!FUN_00438ffd())
    {
        if (!g_GameManager.flags.isSpellPractice || g_GameManager.currentSpellCardNumber < 205)
        {
            this->stageTextAnm = g_AnmManager->PreloadAnm(13, g_GuiStageTextAnmPaths[g_GameManager.currentStage]);
            if (this->stageTextAnm == NULL)
                return ZUN_ERROR;
        }
        else
        {
            this->stageTextAnm = g_AnmManager->PreloadAnm(13, g_GuiStageTextAnmPaths[MAX_STAGES - 1]);
            if (this->stageTextAnm == NULL)
                return ZUN_ERROR;
        }
    }

    if (FUN_00438fe9())
    {
        for (k = 0; k < 16; k++)
            this->frontAnm->SetAndExecuteScriptIdx(&this->impl->vm0000[k], k);
    }

    this->unk_0 = 0;
    this->bossPresent = false;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->impl) + 0x2a40) = 0;
    this->bossLifeBarSize = 0.0f;
    this->bossLifeBarMaxSize = 0.0f;

    if (!g_GameManager.flags.isSpellPractice)
    {
        this->stageTextAnm->ExecuteAnmIdxArray(&this->impl->vm2a44[0], 0, 4);
    }
    else if (!FUN_00438ffd() || FUN_00439916(g_GameManager.currentSpellCardNumber))
    {
        this->stageTextAnm->ExecuteAnmIdxArray(&this->impl->vm2a44[0], 3, 1);
        this->stageTextAnm->SetSprite(&this->impl->vm2a44[0], FUN_00439961(g_GameManager.currentSpellCardNumber) + 3);
    }

    reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->currentMsgIdx = -1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->impl->msgVm)->resultState = 0;
    this->impl->formatted0.isShown = 0;
    this->impl->formatted1.isShown = 0;
    this->impl->formatted2.isShown = 0;

    this->flags.lifeDisplayUpdateFrames = 2;
    this->flags.bombDisplayUpdateFrames = 2;
    this->flags.grazeDisplayUpdateFrames = 2;
    this->flags.pointDisplayUpdateFrames = 2;
    this->flags.powerDisplayUpdateFrames = 2;
    this->flags.timeDisplayUpdateFrames = 2;

    g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->impl->vm212c8, 3);
    g_GuiMessageScreenEffectDuration = 16;
    reinterpret_cast<GuiStageResultUpdateOverlay *>(reinterpret_cast<u8 *>(this->impl) + 0x22dec)->clockDisplayCurrent = 0;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4396b8
void Gui::FUN_004396b8()
{
    this->impl->vm3778.activeSpriteIndex = -1;
    this->impl->vm3a1c.activeSpriteIndex = -1;
    this->impl->vm3cc0.activeSpriteIndex = -1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this->impl) + 0x21810) = 0;
}

// FUNCTION: th08 0x4396f8
u32 AnmVm::FUN_004396f8()
{
    return (*(u32 *)((u8 *)this + 0x1F8) >> 14) & 1;
}

// FUNCTION: th08 0x439710
ZunResult Gui::LoadMsg(const char *path)
{
    this->FreeMsgFile();
    this->impl->msgVm.msgFile = FileSystem::OpenFile(path, NULL, 0);
    if (this->impl->msgVm.msgFile == NULL)
    {
        g_GameErrorContext.Log("\x65\x72\x72\x6f\x72\x20\x3a\x20\x83\x81\x83\x62\x83\x5a\x81\x5b\x83\x57\x83\x74\x83\x40\x83\x43\x83\x8b\x20\x25\x73\x20\x82\xaa\x93\xc7\x82\xdd\x8d\x9e\x82\xdf\x82\xdc\x82\xb9\x82\xf1\x82\xc5\x82\xb5\x82\xbd\x0d\x0a", path);
        return ZUN_ERROR;
    }
    *(i32 *)((u8 *)this->impl + 0x2181C) = -1;
    *(i32 *)((u8 *)this->impl + 0x21818) = 0;
    for (i32 i = 0; i < *(i32 *)this->impl->msgVm.msgFile; ++i)
    {
        ((i32 *)this->impl->msgVm.msgFile)[i + 1] += (i32)this->impl->msgVm.msgFile;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4397d5
void Gui::FreeMsgFile(void)
{
    if (this->impl->msgVm.msgFile != NULL)
    {
        ZUN_FREE(this->impl->msgVm.msgFile);
    }
}

// FUNCTION: th08 0x439810
void Gui::FUN_00439810(i32 value)
{
    this->impl->FUN_0043396d(value);
}

} /* namespace th08 */
