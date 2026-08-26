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
DIFFABLE_STATIC_ARRAY(i32, MAX_STAGES, g_GuiStageClearBonuses);
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

void __fastcall DecryptGuiMessageText(char *out, const char *encoded);
i32 IsInitialStageLoad();
i32 ReleaseResourcesOnRestart();
i32 KeepStageResources();

// FUNCTION: th08 0x4338ca
ChainCallbackResult Gui::OnUpdate(Gui *gui)
{
    if (g_EclScriptedGlobalUpdateFreeze)
        return CHAIN_CALLBACK_RESULT_CONTINUE;

    gui->UpdateStageElements();
    gui->impl->RunMsg();
    if ((g_CurFrameInput & TH_BUTTON_SKIP) && g_GuiMessageScreenEffectDuration < 8)
        g_GuiMessageScreenEffectDuration = 8;
    gui->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x433927
ChainCallbackResult Gui::OnDraw(Gui *gui)
{
    if (gui->impl->stageClearScreenState != 0)
        gui->DrawStageClearScreen();
    gui->impl->DrawDialogue();
    gui->DrawStageElements();
    gui->DrawGameScene();
    gui->DrawAsciiText();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x43396d
void GuiImpl::StartMessage(i32 messageIndex)
{
    GuiMessageFile *msgFile;

    utils::GuiDebugPrint("msg start %d\n\r", messageIndex);
    msgFile = this->message.msgFile;
    memset(&this->message, 0, 0x1570);
    this->message.msgFile = msgFile;

    if (messageIndex == 0)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE5:
            Gui::CopyEnemyNameTexture(22);
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
            Gui::CopyEnemyNameTexture(24);
            break;
        }
        case EXTRASTAGE:
        {
            AnmLoaded *tmp = g_Spellcard.enemyFaceAnm0;
            g_Spellcard.enemyFaceAnm0 = g_Spellcard.enemyFaceAnm1;
            g_Spellcard.enemyFaceAnm1 = tmp;
            g_GuiMessageStageMode = 2;
            Gui::CopyEnemyNameTexture(25);
            break;
        }
        default:
            break;
        }
    }
    else if (messageIndex == 10)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE5:
            if (g_GameManager.globals->numRetries > 0)
            {
                messageIndex = 1;
                this->message.selectedOption = 0;
            }
            else if (!g_GameManager.IsReplay())
            {
                if (g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, EASY) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, NORMAL) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, HARD) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, LUNATIC) ||
                    g_GameManager.shotType > SHOT_YOUMU_YUYUKO)
                {
                    messageIndex = 3;
                    this->message.selectedOption = 1;
                }
                else if (g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, EASY) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, NORMAL) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, HARD) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, LUNATIC))
                {
                    messageIndex = 2;
                    this->message.selectedOption = 1;
                }
                else
                {
                    messageIndex = 1;
                    this->message.selectedOption = 0;
                }
            }
            else
            {
                if ((i8)g_ReplayManager->replayData->clearState == 2)
                {
                    messageIndex = 3;
                    this->message.selectedOption = 1;
                }
                else if ((i8)g_ReplayManager->replayData->clearState == 1)
                {
                    messageIndex = 2;
                    this->message.selectedOption = 1;
                }
                else
                {
                    messageIndex = 1;
                    this->message.selectedOption = 0;
                }
            }
            g_GameManager.flags.finalStageRoute = this->message.selectedOption;
            break;
        default:
            break;
        }
    }
    else if (messageIndex >= 6)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE6B:
            if ((i8)g_GameManager.GetClockTime() >= 12)
            {
                messageIndex = 5;
            }
            break;
        default:
            break;
        }
    }

    this->message.currentMsgIdx = messageIndex;
    this->message.currentInstr = this->message.msgFile->messages[messageIndex];
    this->message.dialogueLines[0].scriptIndex = -1;
    this->message.dialogueLines[1].scriptIndex = -1;
    this->message.textBoxVisible = 1;
    this->message.fontSize = 15;
    this->message.textColors[0] = g_GuiMessageTextColors[g_GameManager.shotType].colors[0];
    this->message.textColors[1] = g_GuiMessageTextColors[g_GameManager.shotType].colors[1];
    this->message.textColors[2] = g_GuiMessageTextColors[g_GameManager.shotType].colors[2];
    this->message.textColors[3] = g_GuiMessageTextColors[g_GameManager.shotType].colors[3];
    this->message.shadowColors[0] = 0;
    this->message.shadowColors[1] = 0;
    this->message.shadowColors[2] = 0;
    this->message.shadowColors[3] = 0;
    this->message.dialogueSkippable = 1;
    this->message.waitThreshold = 6;
    this->message.textColorIndex = 0;
    this->message.resetDialogueLines = 1;
    this->message.dialogueLineIndex = 0;
    this->message.currentPortraitIndex = 0xff;

    g_BulletManager.bulletmanager_fun_00415c60();
    g_EnemyManager.KillAllNonBossEnemies(0, 0);
    g_ItemManager.AutoCollectAllItems();
}

// FUNCTION: th08 0x433db3
#pragma var_order(args, j, portraitArgs, k, portraitSpriteArgs, text3, text16, text19, text20, i)
i32 GuiImpl::RunMsg()
{
    GuiMessageInstructionArgs *args;
    u32 j;
    GuiMessageConfigureAllPortraitsArgs *portraitArgs;
    u32 k;
    GuiMessageConfigurePortraitArgs *portraitSpriteArgs;
    char text3[64];
    char text19[64];
    char text20[64];
    char text16[64];
    u32 i;

    if (this->message.currentMsgIdx < 0)
        return -1;

    if (this->message.ignoreWaitCounter > 0)
        this->message.ignoreWaitCounter--;

    if (this->message.dialogueSkippable &&
        (g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
    {
        this->message.timer =
            this->message.currentInstr->time;
    }

    if (g_Player.playerState != PLAYER_STATE_DYING)
        g_ItemManager.AutoCollectAllItems();

    while (this->message.timer >= (i32)this->message.currentInstr->time)
    {
        switch (this->message.currentInstr->opcode)
        {
        case GUI_MSG_DELETE:
            this->message.currentMsgIdx = -1;
            return -1;

        case GUI_MSG_CONFIGURE_ALL_PORTRAITS:
            portraitArgs = &this->message.currentInstr->args.configureAllPortraits;
            if (this->message.currentPortraitIndex !=
                portraitArgs->portraitIndex)
            {
                for (j = 0; j < 4; j++)
                {
                    if (this->message.currentPortraitIndex == j)
                    {
                        if ((this->message.currentPortraitIndex / 2) !=
                            (portraitArgs->portraitIndex / 2))
                            this->message.portraits[j].pendingInterrupt = 6;
                        else
                            this->message.portraits[j].pendingInterrupt = 4;
                    }
                    else
                        this->message.portraits[j].pendingInterrupt = 4;
                }
            }
            this->message.portraits[portraitArgs->portraitIndex]
                .pendingInterrupt = 3;
            this->message.currentPortraitIndex =
                portraitArgs->portraitIndex;
            if (portraitArgs->spriteIndices[0] >= 0)
                g_Spellcard.playerFaceAnm0->SetSprite(
                    &this->message.portraits[0],
                    portraitArgs->spriteIndices[0]);
            if (portraitArgs->spriteIndices[1] >= 0)
                g_Spellcard.playerFaceAnm1->SetSprite(
                    &this->message.portraits[1],
                    portraitArgs->spriteIndices[1]);
            if (portraitArgs->spriteIndices[2] >= 0)
                g_Spellcard.enemyFaceAnm0->SetSprite(
                    &this->message.portraits[2],
                    portraitArgs->spriteIndices[2]);
            if (portraitArgs->spriteIndices[3] >= 0)
                g_Spellcard.enemyFaceAnm1->SetSprite(
                    &this->message.portraits[3],
                    portraitArgs->spriteIndices[3]);
            this->message.textColorIndex =
                portraitArgs->portraitIndex;
            this->message.resetDialogueLines = 1;
            break;

        case GUI_MSG_CONFIGURE_PORTRAIT:
            portraitSpriteArgs = &this->message.currentInstr->args.configurePortrait;
            if (this->message.currentPortraitIndex !=
                portraitSpriteArgs->portraitIndex)
            {
                for (k = 0; k < 4; k++)
                {
                    if (this->message.currentPortraitIndex == k)
                    {
                        if ((this->message.currentPortraitIndex / 2) !=
                            (portraitSpriteArgs->portraitIndex / 2))
                            this->message.portraits[k].pendingInterrupt = 6;
                        else
                            this->message.portraits[k].pendingInterrupt = 4;
                    }
                    else
                    {
                        this->message.portraits[k].pendingInterrupt = 4;
                    }
                }
            }
            this->message.portraits[portraitSpriteArgs->portraitIndex].pendingInterrupt = 3;
            this->message.currentPortraitIndex =
                portraitSpriteArgs->portraitIndex;
            if (portraitSpriteArgs->spriteIndex >= 0)
            {
                switch (portraitSpriteArgs->portraitIndex)
                {
                case 0:
                    g_Spellcard.playerFaceAnm0->SetSprite(
                        &this->message.portraits[0],
                        portraitSpriteArgs->spriteIndex);
                    break;
                case 1:
                    g_Spellcard.playerFaceAnm1->SetSprite(
                        &this->message.portraits[1],
                        portraitSpriteArgs->spriteIndex);
                    break;
                case 2:
                    g_Spellcard.enemyFaceAnm0->SetSprite(
                        &this->message.portraits[2],
                        portraitSpriteArgs->spriteIndex);
                    break;
                case 3:
                    g_Spellcard.enemyFaceAnm1->SetSprite(
                        &this->message.portraits[3],
                        portraitSpriteArgs->spriteIndex);
                    break;
                }
            }
            this->message.textColorIndex =
                portraitSpriteArgs->portraitIndex;
            this->message.resetDialogueLines = 1;
            break;

        case GUI_MSG_SET_PORTRAIT_ANM_SCRIPT:
            args = &this->message.currentInstr->args;
            switch (args->portraitAnmScript.portraitIndex)
            {
            case 0:
                g_Spellcard.playerFaceAnm0->SetAndExecuteScriptIdx(
                    &this->message.portraits[0],
                    args->portraitAnmScript.scriptIndex);
                break;
            case 1:
                g_Spellcard.playerFaceAnm1->SetAndExecuteScriptIdx(
                    &this->message.portraits[1],
                    args->portraitAnmScript.scriptIndex);
                break;
            case 2:
                g_Spellcard.enemyFaceAnm0->SetAndExecuteScriptIdx(
                    &this->message.portraits[2],
                    args->portraitAnmScript.scriptIndex);
                break;
            case 3:
                g_Spellcard.enemyFaceAnm1->SetAndExecuteScriptIdx(
                    &this->message.portraits[3],
                    args->portraitAnmScript.scriptIndex);
                break;
            }
            if (this->message.portraits[args->portraitAnmScript.portraitIndex]
                    .loadedSprite->widthPx > 128.0f)
                this->message.portraits[args->portraitAnmScript.portraitIndex]
                    .pos2.x = -112.0f;
            else
                this->message.portraits[args->portraitAnmScript.portraitIndex]
                    .pos2.x = 0.0f;
            break;

        case GUI_MSG_SET_PORTRAIT_SPRITE:
            args = &this->message.currentInstr->args;
            switch (args->portraitSprite.portraitIndex)
            {
            case 0:
                g_Spellcard.playerFaceAnm0->SetSprite(
                    &this->message.portraits[0],
                    args->portraitSprite.spriteIndex);
                break;
            case 1:
                g_Spellcard.playerFaceAnm1->SetSprite(
                    &this->message.portraits[1],
                    args->portraitSprite.spriteIndex);
                break;
            case 2:
                g_Spellcard.enemyFaceAnm0->SetSprite(
                    &this->message.portraits[2],
                    args->portraitSprite.spriteIndex);
                break;
            case 3:
                g_Spellcard.enemyFaceAnm1->SetSprite(
                    &this->message.portraits[3],
                    args->portraitSprite.spriteIndex);
                break;
            }
            if (this->message.portraits[args->portraitSprite.portraitIndex]
                    .loadedSprite->widthPx > 256.0f)
            {
                this->message.portraits[args->portraitSprite.portraitIndex]
                    .pos2.x = -208.0f;
                this->message.portraits[args->portraitSprite.portraitIndex]
                    .pos2.y = -50.0f;
            }
            else if (this->message.portraits[args->portraitSprite.portraitIndex]
                         .loadedSprite->widthPx > 128.0f)
            {
                this->message.portraits[args->portraitSprite.portraitIndex]
                    .pos2.x = -80.0f;
            }
            else
            {
                this->message.portraits[args->portraitSprite.portraitIndex]
                    .pos2.x = 0.0f;
            }
            break;

        case GUI_MSG_SHOW_DIALOGUE_TEXT:
            args = &this->message.currentInstr->args;
            if (args->dialogueText.lineIndex == 0 &&
                this->message.dialogueLines[1].scriptIndex >= 0)
            {
                g_AnmManager->DrawTextLeft(&this->message.dialogueLines[1],
                                           this->message.textColors[args->dialogueText.colorIndex],
                                           this->message.shadowColors[args->dialogueText.colorIndex], " ");
            }
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(
                &this->message.dialogueLines[args->dialogueText.lineIndex], args->dialogueText.lineIndex);
            this->message.dialogueLines[args->dialogueText.lineIndex].fontWidth =
                this->message.dialogueLines[args->dialogueText.lineIndex].fontHeight =
                this->message.fontSize;
            DecryptGuiMessageText(text3, args->dialogueText.encryptedText);
            g_AnmManager->DrawTextLeft(&this->message.dialogueLines[args->dialogueText.lineIndex],
                                       this->message.textColors[args->dialogueText.colorIndex],
                                       this->message.shadowColors[args->dialogueText.colorIndex], text3);
            this->message.framesElapsedDuringPause = 0;
            break;

        case GUI_MSG_SHOW_SPEAKER_TEXT:
            args = &this->message.currentInstr->args;
            if (this->message.resetDialogueLines)
            {
                if (this->message.dialogueLines[1].scriptIndex >= 0)
                {
                    g_AnmManager->DrawTextLeft(&this->message.dialogueLines[1],
                        this->message.textColors[this->message.textColorIndex],
                        this->message.shadowColors[this->message.textColorIndex], " ");
                }
                this->message.dialogueLineIndex = 0;
            }
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(
                &this->message.dialogueLines[this->message.dialogueLineIndex],
                this->message.dialogueLineIndex);
            this->message.dialogueLines[this->message.dialogueLineIndex].fontWidth =
                this->message.dialogueLines[this->message.dialogueLineIndex].fontHeight =
                this->message.fontSize;
            DecryptGuiMessageText(text16, args->plainText.encryptedText);
            g_AnmManager->DrawTextLeft(
                &this->message.dialogueLines[this->message.dialogueLineIndex],
                this->message.textColors[this->message.textColorIndex],
                this->message.shadowColors[this->message.textColorIndex], text16);
            this->message.framesElapsedDuringPause = 0;
            this->message.resetDialogueLines = 0;
            this->message.dialogueLineIndex++;
            break;

        case GUI_MSG_SHOW_TOP_TEXT:
            args = &this->message.currentInstr->args;
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->message.dialogueLines[0], 0);
            this->message.dialogueLines[0].fontWidth =
                this->message.dialogueLines[0].fontHeight =
                this->message.fontSize;
            DecryptGuiMessageText(text19, args->plainText.encryptedText);
            g_AnmManager->DrawTextLeft(&this->message.dialogueLines[0],
                this->message.textColors[0],
                this->message.shadowColors[0], text19);
            this->message.framesElapsedDuringPause = 0;
            break;

        case GUI_MSG_SHOW_BOTTOM_TEXT:
            args = &this->message.currentInstr->args;
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->message.dialogueLines[1], 1);
            this->message.dialogueLines[1].fontWidth =
                this->message.dialogueLines[1].fontHeight =
                this->message.fontSize;
            DecryptGuiMessageText(text20, args->plainText.encryptedText);
            g_AnmManager->DrawTextLeft(&this->message.dialogueLines[1],
                this->message.textColors[0],
                this->message.shadowColors[0], text20);
            this->message.framesElapsedDuringPause = 0;
            break;

        case GUI_MSG_SHOW_SELECTION:
            if ((g_GuiMessageInputCurrent & TH_BUTTON_UP) &&
                (g_GuiMessageInputCurrent & TH_BUTTON_UP) != (g_GuiMessageInputPrevious & TH_BUTTON_UP))
            {
                if (this->message.selectedOption == 1)
                    g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                this->message.selectedOption = 0;
            }
            if ((g_GuiMessageInputCurrent & TH_BUTTON_DOWN) &&
                (g_GuiMessageInputCurrent & TH_BUTTON_DOWN) != (g_GuiMessageInputPrevious & TH_BUTTON_DOWN))
            {
                if (this->message.selectedOption == 0)
                    g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                this->message.selectedOption = 1;
            }
            this->message.dialogueLines[this->message.selectedOption].color1.d3dColor = -1;
            this->message.dialogueLines[1 - this->message.selectedOption].color1.d3dColor = 0xE0606060;
            if (!((g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) &&
                  (g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) !=
                      (g_GuiMessageInputPrevious & TH_BUTTON_SHOOT)) ||
                this->message.framesElapsedDuringPause < 60)
            {
                if (this->message.framesElapsedDuringPause >=
                    this->message.currentInstr->args.wait.frames)
                {
                    this->message.resetDialogueLines = 1;
                    this->message.waitThreshold = 30;
                    break;
                }
                this->message.framesElapsedDuringPause++;
                goto run_scripts;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            break;
        case GUI_MSG_READ_SELECTED_MESSAGE:
            g_GameManager.flags.finalStageRoute = this->message.selectedOption;
            g_Gui.MsgRead(this->message.selectedOption + 1);
            continue;
        case GUI_MSG_WAIT:
            if (!this->message.dialogueSkippable ||
                !(g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
            {
                if (!(g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) ||
                    (g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) ==
                        (g_GuiMessageInputPrevious & TH_BUTTON_SHOOT) ||
                    this->message.framesElapsedDuringPause <
                        this->message.waitThreshold)
                {
                    if (this->message.framesElapsedDuringPause >=
                        this->message.currentInstr->args.wait.frames)
                    {
                        this->message.resetDialogueLines = 1;
                        this->message.waitThreshold = 30;
                        break;
                    }
                    this->message.framesElapsedDuringPause++;
                    goto run_scripts;
                }
                this->message.resetDialogueLines = 1;
                this->message.waitThreshold = 8;
            }
            break;

        case GUI_MSG_INTERRUPT_PORTRAIT_ANM:
            args = &this->message.currentInstr->args;
            this->message.portraits[args->portraitInterrupt.portraitIndex]
                .pendingInterrupt = args->portraitInterrupt.interrupt;
            break;

        case GUI_MSG_RESUME_ECL:
            this->message.ignoreWaitCounter++;
            break;

        case GUI_MSG_SET_MUSIC:
            if (this->message.currentInstr->args.music.musicIndex < 0)
            {
                g_Supervisor.StopAudio();
            }
            else
            {
                g_Gui.stageTextAnm->SetAndExecuteScriptIdx(&this->stageTextVms[3], 3);
                g_Gui.stageTextAnm->SetSprite(
                    &this->stageTextVms[3],
                    this->message.currentInstr->args.music.musicIndex + 3);
                if (g_Supervisor.PlayMusic(
                        this->message.currentInstr->args.music.musicIndex,
                        reinterpret_cast<char *>(g_GuiStageMusicContexts[g_GameManager.currentStage]
                                                    .values[this->message.currentInstr->args.music.musicIndex])))
                {
                    g_Supervisor.PlayAudio(
                        reinterpret_cast<GuiStageMusicDataOverlay *>(g_Background.stageData)
                            ->songPaths[this->message.currentInstr->args.music.musicIndex],
                        g_GuiStageMusicContexts[g_GameManager.currentStage]
                            .values[this->message.currentInstr->args.music.musicIndex]);
                }
            }
            break;

        case GUI_MSG_SHOW_INTRO_TEXT:
            args = &this->message.currentInstr->args;
            g_Spellcard.enemyFaceAnm0->SetAndExecuteScriptIdx(
                &this->message.introLines[0], 1);
            this->message.framesElapsedDuringPause = 0;
            break;

        case GUI_MSG_SHOW_STAGE_RESULTS:
            this->stageClear.power = g_GameManager.GetPower();
            this->stageClear.pointItemsCollected = g_GameManager.globals->pointItemsCollectedInStage;
            this->stageClear.timeOrbs = g_GameManager.GetTimeOrbs();
            this->stageClear.graze = g_GameManager.globals->grazeInStage;
            this->stageClear.clockDisplayStart =
                (i8)g_GameManager.GetClockTime() * 30 + 0x294;
            this->stageClear.clockIncrement = g_GameManager.GetClockTimeIncrement();
            g_GameManager.AddToClockTime(this->stageClear.clockIncrement);
            this->stageClear.stageBonus = g_GuiStageClearBonuses[g_GameManager.currentStage];
            this->stageClear.clockDisplayTarget =
                (i8)g_GameManager.GetClockTime() * 30 + 0x294;
            this->stageClear.clockDisplayCurrent = this->stageClear.clockDisplayStart;
            this->stageClear.clockDisplayTimer &= 0;
            this->stageClearScreenState = 1;
            g_GameManager.flags.unk9 = 1;

            if (g_GameManager.currentStage != STAGE6A && g_GameManager.currentStage != STAGE6B &&
                g_GameManager.currentStage != EXTRASTAGE)
            {
                g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(
                    &this->stageRankVm, 3);
                g_AsciiManager.asciiAnm->SetSprite(
                    &this->stageRankVm,
                    this->stageClear.clockIncrement + 0x80);
            }
            else
            {
                this->stageRankVm.currentInstruction = NULL;
            }
            this->stageRankVm.SetInterrupt(1);

            if (g_GameManager.currentStage != STAGE6A && g_GameManager.currentStage != STAGE6B &&
                g_GameManager.currentStage != EXTRASTAGE)
            {
                g_Gui.loadingPortraitAnm->SetAndExecuteScriptIdx(&this->loadingPortraitVm, 0);
                g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(
                    &this->arcadeCaptureVm, 1);
                g_AnmManager->SetTextureCaptureParams(
                    3, 0x20, 0x10, 0x180, 0x1C0,
                    (u32)(i32)this->arcadeCaptureVm.loadedSprite->startPixelInclusive.x,
                    (u32)(i32)this->arcadeCaptureVm.loadedSprite->startPixelInclusive.y,
                    (u32)(i32)this->arcadeCaptureVm.loadedSprite->widthPx,
                    (u32)(i32)this->arcadeCaptureVm.loadedSprite->heightPx);

                for (i = 0; i < 8; i++)
                {
                    g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(
                        &this->arcadeBlurVms[i], 2);
                    this->arcadeBlurVms[i].counterVar0 = i * 4 + 3;
                    this->arcadeBlurVms[i].color1.a = 64 - i * 2;
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
        case GUI_MSG_HALT:
            goto run_scripts;
        case GUI_MSG_FADE_OUT_MUSIC:
            g_Supervisor.FadeOutMusic(4.0f);
            break;
        case GUI_MSG_FADE_SCREEN:
            ScreenEffect::RegisterChain((ScreenEffectType)4, 442, 0xffffff, 0, 0, 21);
            g_GuiMessageScreenEffectDuration = 442;
            break;
        case GUI_MSG_END_STAGE:
            if (g_GameManager.currentStage == STAGE6A || g_GameManager.currentStage == STAGE6B ||
                g_GameManager.currentStage == EXTRASTAGE)
                g_GameManager.flags.stageTransitionState = 2;
            goto run_scripts;
        case GUI_MSG_SET_DIALOGUE_SKIPPABLE:
            this->message.dialogueSkippable =
                this->message.currentInstr->args.toggle.enabled;
            break;
        case GUI_MSG_SET_TEXT_BOX_VISIBLE:
            this->message.textBoxVisible =
                this->message.currentInstr->args.toggle.enabled;
            break;

        }

        this->message.currentInstr =
            reinterpret_cast<GuiMessageInstruction *>(
                reinterpret_cast<i32>(&this->message.currentInstr->args) +
                this->message.currentInstr->instructionSize);
    }

    this->message.timer++;

run_scripts:
    g_AnmManager->ExecuteScript(&this->message.portraits[0]);
    g_AnmManager->ExecuteScript(&this->message.portraits[1]);
    g_AnmManager->ExecuteScript(&this->message.portraits[2]);
    g_AnmManager->ExecuteScript(&this->message.portraits[3]);
    g_AnmManager->ExecuteScript(&this->message.dialogueLines[0]);
    g_AnmManager->ExecuteScript(&this->message.dialogueLines[1]);
    g_AnmManager->ExecuteScript(&this->message.introLines[0]);
    g_AnmManager->ExecuteScript(&this->message.introLines[1]);

    if (this->message.timer < 60 &&
        this->message.dialogueSkippable &&
        (g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
        this->message.timer = 60;

    return 0;
}

// FUNCTION: th08 0x4353ec
#pragma var_order(i, decoded)
void __fastcall DecryptGuiMessageText(char *out, const char *encoded)
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

    if (this->message.currentMsgIdx < 0)
        return ZUN_ERROR;

    if (this->message.timer < 60)
        dialogueBoxHeight = static_cast<f32>(this->message.timer) *
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

    if (this->message.portraits[0].pos.z >=
        this->message.portraits[1].pos.z)
    {
        g_AnmManager->DrawNoRotation(&this->message.portraits[0]);
        g_AnmManager->DrawNoRotation(&this->message.portraits[1]);
    }
    else
    {
        g_AnmManager->DrawNoRotation(&this->message.portraits[1]);
        g_AnmManager->DrawNoRotation(&this->message.portraits[0]);
    }

    if (this->message.portraits[2].pos.z >=
        this->message.portraits[3].pos.z)
    {
        g_AnmManager->DrawNoRotation(&this->message.portraits[2]);
        g_AnmManager->DrawNoRotation(&this->message.portraits[3]);
    }
    else
    {
        g_AnmManager->DrawNoRotation(&this->message.portraits[3]);
        g_AnmManager->DrawNoRotation(&this->message.portraits[2]);
    }

    g_AnmManager->FlushVertexBuffer();

    if (this->message.textBoxVisible)
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

    g_AnmManager->DrawNoRotation(&this->message.dialogueLines[0]);
    g_AnmManager->DrawNoRotation(&this->message.dialogueLines[1]);
    g_AnmManager->DrawNoRotation(&this->message.introLines[0]);
    g_AnmManager->DrawNoRotation(&this->message.introLines[1]);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x43587e
i32 Gui::MsgWait()
{
    if (this->impl == NULL)
        return 0;
    if (this->impl->message.ignoreWaitCounter > 0)
        return 0;
    return this->impl->message.currentMsgIdx >= 0;
}

// FUNCTION: th08 0x4358bb
i32 Gui::IsDialoguePresent()
{
    if (this->impl == NULL)
        return 0;
    return this->impl->message.currentMsgIdx >= 0 || this->impl->message.currentMsgIdx == -2;
}

// FUNCTION: th08 0x435900
#pragma var_order(i, remaining, j, score, k)
void Gui::UpdateStageElements()
{
    i32 i;
    i32 remaining;
    i32 j;
    i32 score;
    i32 k;

    if (this->impl->message.currentMsgIdx < 0)
    {
        if (this->bossPresent)
        {
            if (this->impl->bossLifeBarState == 0)
            {
                this->impl->frontVms[12].SetInterrupt(1);
                this->impl->bossLifeBarState = 1;
                this->bossUIOpacity = 0;
            }
            else
            {
                if (this->impl->frontVms[12].FUN_004396f8())
                    this->impl->bossLifeBarState = 2;
                if (this->bossUIOpacity < 0xfc)
                    this->bossUIOpacity += 4;
                else
                    this->bossUIOpacity = 0xff;
            }
        }
        else if (this->impl->bossLifeBarState != 0)
        {
            if (this->impl->bossLifeBarState <= 2)
            {
                this->impl->frontVms[12].SetInterrupt(2);
                this->impl->bossLifeBarState = 3;
            }
            if (this->bossUIOpacity > 0)
                this->bossUIOpacity -= 4;
            else
                this->bossUIOpacity = 0;
            if (this->impl->frontVms[12].FUN_004396f8())
            {
                this->impl->bossLifeBarState = 0;
                this->bossLifeBarDisplayedSize = 0.0f;
                this->bossUIOpacity = 0;
            }
        }

        if (this->impl->bossLifeBarState >= 2)
        {
            if (this->bossLifeBarTargetSize > this->bossLifeBarDisplayedSize)
            {
                this->bossLifeBarDisplayedSize += 0.01f;
                if (this->bossLifeBarTargetSize < this->bossLifeBarDisplayedSize)
                    this->bossLifeBarDisplayedSize = this->bossLifeBarTargetSize;
            }
            else if (this->bossLifeBarTargetSize < this->bossLifeBarDisplayedSize)
            {
                this->bossLifeBarDisplayedSize -= 0.02f;
                if (this->bossLifeBarTargetSize > this->bossLifeBarDisplayedSize)
                    this->bossLifeBarDisplayedSize = this->bossLifeBarTargetSize;
            }
        }
    }

    g_AnmManager->ExecuteScriptArray(this->impl->frontVms, 16);
    g_AnmManager->ExecuteScriptArray(this->impl->stageTextVms, 4);
    if (!g_GameManager.flags.isSpellPractice && this->impl->stageTextVms[0].color1.a)
        g_AnmManager->ExecuteScriptArray(&this->impl->clockIntroVm, 1);

    g_AnmManager->ExecuteScript(&this->impl->stageRankVm);
    g_AnmManager->ExecuteScript(&this->impl->clockTimeVm);

    if (this->impl->clockTimeVm.color1.a)
    {
        if (EclOperands::g_TargetPlayerPosition017D61AC.x >= 64.0f &&
            EclOperands::g_TargetPlayerPosition017D61AC.y < 128.0f)
        {
            if (this->impl->clockTimeVm.color1.a > 0x40)
                this->impl->clockTimeVm.color1.a -= 4;
        }
        else if (this->impl->clockTimeVm.color1.a < 0xff)
        {
            if (this->impl->clockTimeVm.color1.a <= 0xfb)
                this->impl->clockTimeVm.color1.a += 4;
            else
                this->impl->clockTimeVm.color1.a = 0xff;
        }
    }

    g_AnmManager->ExecuteScript(&this->impl->spellNullifyVm);
    g_AnmManager->ExecuteScript(&this->impl->difficultyVm);

    if (this->impl->loadingPortraitVm.activeSpriteIndex >= 0)
    {
        if (g_AnmManager->ExecuteScript(&this->impl->loadingPortraitVm))
            this->impl->loadingPortraitVm.activeSpriteIndex = -1;
        if (g_AnmManager->ExecuteScript(&this->impl->arcadeCaptureVm))
            this->impl->arcadeCaptureVm.activeSpriteIndex = -1;
        for (i = 0; i < ARRAY_SIZE(this->impl->arcadeBlurVms); i++)
            g_AnmManager->ExecuteScript(&this->impl->arcadeBlurVms[i]);
    }

    if (this->impl->stageTransitionActiveVmCount != 0)
    {
        remaining = 0xa8;
        for (j = 0; j < 0xa8; j++)
        {
            if (g_AnmManager->ExecuteScript(&this->impl->stageTransitionVms[j]))
                remaining--;
        }
        this->impl->stageTransitionActiveVmCount = remaining;
    }

    if (this->impl->bonusPopup.displayMode)
    {
        if (this->impl->bonusPopup.timer < 30)
            this->impl->bonusPopup.position.x =
                static_cast<f32>(this->impl->bonusPopup.timer) * -312.0f / 30.0f + 416.0f;
        else
            this->impl->bonusPopup.position.x = 104.0f;
        if (this->impl->bonusPopup.timer >= 250)
            this->impl->bonusPopup.displayMode = 0;
        this->impl->bonusPopup.timer++;
    }

    if (this->impl->statusPopup.displayMode)
    {
        if (this->impl->statusPopup.timer < 30)
            this->impl->statusPopup.position.x =
                static_cast<f32>(this->impl->statusPopup.timer) * -312.0f / 30.0f + 416.0f;
        else
            this->impl->statusPopup.position.x = 104.0f;
        if (this->impl->statusPopup.timer >= 180)
            this->impl->statusPopup.displayMode = 0;
        this->impl->statusPopup.timer++;
    }

    if (this->impl->spellcardBonusPopup.displayMode)
    {
        if (this->impl->spellcardBonusPopup.timer >= 280)
            this->impl->spellcardBonusPopup.displayMode = 0;
        this->impl->spellcardBonusPopup.timer++;
    }

    if (this->impl->stageClearScreenState == 1)
    {
        score = 0;
        score += this->impl->stageClear.stageBonus;
        score += this->impl->stageClear.graze * 50;
        score += this->impl->stageClear.pointItemsCollected * 5000;
        score += this->impl->stageClear.timeOrbs * 100;

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

        switch (static_cast<i8>(g_GameManager.cfg->lifeCount))
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

        this->impl->stageClearBonusTotal = score;
        for (k = 0; k < 10; k++)
            g_GameManager.AddScore(score);
        this->impl->stageClearScreenState++;
    }

    if (g_GameManager.currentStage < STAGE6A &&
        this->impl->stageClear.clockDisplayCurrent != 0 &&
        this->impl->stageClear.clockDisplayCurrent >=
            this->impl->stageClear.clockDisplayTarget &&
        g_GameManager.flags.stageTransitionState == 0)
    {
        g_GameManager.flags.stageTransitionState = 2;
    }

    if (this->impl->stageClear.clockDisplayCurrent != 0 &&
        this->impl->stageClear.clockDisplayCurrent !=
            this->impl->stageClear.clockDisplayTarget)
    {
        if (this->impl->stageClear.clockDisplayTimer >= 60)
        {
            if (this->impl->stageClear.clockDisplayCurrent <
                this->impl->stageClear.clockDisplayTarget)
            {
                this->impl->stageClear.clockDisplayCurrent++;
                if ((g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) || (g_GuiMessageInputCurrent & TH_BUTTON_SKIP))
                {
                    this->impl->stageClear.clockDisplayCurrent += 3;
                }
                if (this->impl->stageClear.clockDisplayCurrent >
                    this->impl->stageClear.clockDisplayTarget)
                {
                    this->impl->stageClear.clockDisplayCurrent =
                        this->impl->stageClear.clockDisplayTarget;
                }
            }
            else
            {
                this->impl->stageClear.clockDisplayTimer++;
            }
        }
        else
        {
            this->impl->stageClear.clockDisplayTimer++;
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
        vm = &this->impl->frontVms[15];
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

    vm = &this->impl->frontVms[13];
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
        vm = &this->impl->frontVms[14];
        for (xPos = 0.0f; xPos < 624.0f; xPos += 128.0f)
        {
            vm->pos = Float3(xPos, 0.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
            vm->pos = Float3(xPos, 464.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[0]);
        g_AnmManager->Draw2D(&this->impl->frontVms[1]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[2]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[3]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[4]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[5]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[6]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[7]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[8]);
        g_AnmManager->DrawNoRotation(&this->impl->frontVms[9]);
        g_AnmManager->DrawNoRotation(&this->impl->difficultyVm);
        this->flags.lifeDisplayUpdateFrames = 2;
        this->flags.bombDisplayUpdateFrames = 2;
        this->flags.grazeDisplayUpdateFrames = 2;
        this->flags.pointDisplayUpdateFrames = 2;
        this->flags.powerDisplayUpdateFrames = 2;
        this->flags.timeDisplayUpdateFrames = 2;
    }

    if (this->flags.lifeDisplayUpdateFrames)
    {
        vm = &this->impl->frontVms[10];
        for (idx = 0, xPos = 488.0f; idx < g_GameManager.GetLives(); idx++, xPos += 16.0f)
        {
            vm->pos = Float3(xPos, 88.0f, 0.46f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if (this->flags.bombDisplayUpdateFrames)
    {
        vm = &this->impl->frontVms[11];
        for (idx = 0, xPos = 488.0f; idx < g_GameManager.GetBombsRemaining(); idx++, xPos += 16.0f)
        {
            vm->pos = Float3(xPos, 104.0f, 0.46f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if ((this->flags.bombDisplayUpdateFrames || this->flags.lifeDisplayUpdateFrames) &&
        (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) == 1) && g_Spellcard.IsActive())
    {
        g_AnmManager->DrawNoRotation(&this->impl->spellNullifyVm);
    }

    vm = &this->impl->frontVms[14];
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
void Gui::DrawStageElements()
{
    i32 i;

    for (i = 0; i < 4; i++)
        g_AnmManager->Draw2D(&this->impl->stageTextVms[i]);
    g_AnmManager->Draw2D(&this->impl->clockIntroVm);
    g_AnmManager->Draw2D(&this->impl->clockTimeVm);

    if (this->impl->loadingPortraitVm.activeSpriteIndex >= 0)
    {
        g_AnmManager->DrawNoRotation(&this->impl->loadingPortraitVm);
        g_AnmManager->FUN_00464070(&this->impl->arcadeCaptureVm);
        for (i = 0; i < ARRAY_SIZE(this->impl->arcadeBlurVms); i++)
            g_AnmManager->FUN_00464070(&this->impl->arcadeBlurVms[i]);
        if (this->impl->loadingOverlayVm.activeSpriteIndex >= 0)
        {
            this->impl->loadingOverlayVm.pos = Float3(304.0f, 448.0f, 0.0f);
            g_AnmManager->DrawNoRotation(&this->impl->loadingOverlayVm);
        }
    }

    if (this->impl->stageTransitionActiveVmCount != 0)
    {
        for (i = 0; i < 0xa8; i++)
        {
            g_AnmManager->FUN_00464070(&this->impl->stageTransitionVms[i]);
            g_AnmManager->ClearSprite();
        }
    }

    if (this->impl->message.currentMsgIdx < 0 &&
        (this->bossPresent + this->impl->bossLifeBarState) > 0)
    {
#pragma var_order(bossColorDark, bossColor, rect, bossValue, segmentIndex, segmentStop, bossTimerColor, segmentWidth, textPos)
        ZunRect rect;
        D3DCOLOR bossColor;
        D3DCOLOR bossColorDark;
        i32 bossValue;
        i32 segmentIndex;

        rect.left = 64.0f;
        rect.top = 19.0f;
        rect.right = this->bossLifeBarDisplayedSize * 320.0f + 64.0f;
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
            if (this->bossLifeBarSegmentStart[segmentIndex] >= this->bossLifeBarDisplayedSize)
                continue;

            segmentStop = this->bossLifeBarSegmentStop[segmentIndex];
            if (this->bossLifeBarDisplayedSize < segmentStop)
                segmentStop = this->bossLifeBarDisplayedSize;

            rect.left = this->bossLifeBarSegmentStart[segmentIndex] * 320.0f + 64.0f;
            rect.top = 19.0f;
            rect.right = segmentStop * 320.0f + 64.0f;
            rect.bottom = 23.0f;
            bossColor = (this->bossUIOpacity << 24) | (this->bossLifeBarSegmentColor[segmentIndex] & 0x00ffffff);
            bossColorDark = (this->bossUIOpacity << 24) |
                            ((this->bossLifeBarSegmentColor[segmentIndex] >> 2) & 0x003f3f3f);
            ScreenEffect::DrawSquareShaded(&rect, bossColor, bossColor, bossColorDark, bossColorDark);
        }

        g_AnmManager->DrawNoRotation(&this->impl->frontVms[12]);

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

            if (!g_GameManager.isInGameMenu && !g_GameManager.showRetryMenu && !g_GameManager.flags.deathbombFreezeActive &&
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

    g_AnmManager->DrawNoRotation(&this->impl->stageRankVm);
}

// FUNCTION: th08 0x437a2f
ZunResult Gui::AddedCallback(Gui *gui)
{
    return gui->ActualAddedCallback();
}

// FUNCTION: th08 0x437a40
ZunResult Gui::DeletedCallback(Gui *gui)
{
    if (!KeepStageResources())
    {
        g_AnmManager->ReleaseAnm(13);
    }

    gui->FreeMsgFile();

    if (ReleaseResourcesOnRestart())
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

    if (IsInitialStageLoad())
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
i32 Gui::IsStageFinished()
{
    return this->impl->loadingPortraitVm.activeSpriteIndex >= 0 &&
           this->impl->loadingPortraitVm.FUN_004396f8();
}

// FUNCTION: th08 0x437dc7
i32 Gui::IsDialogueSkippable()
{
    return this->impl->message.dialogueSkippable;
}

// FUNCTION: th08 0x437ddd
void Gui::ShowBonusScore(i32 value)
{
    this->impl->bonusPopup.position = Float3(416.0f, 48.0f, 0.0f);
    this->impl->bonusPopup.displayMode = 1;
    this->impl->bonusPopup.timer = 0;
    this->impl->bonusPopup.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437e5d
void Gui::ShowPopupText(i32 value, i32 displayMode)
{
    this->impl->statusPopup.position = Float3(416.0f, 168.0f, 0.0f);
    this->impl->statusPopup.displayMode = displayMode;
    this->impl->statusPopup.timer = 0;
    this->impl->statusPopup.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437edc
void Gui::ShowSpellcardBonus(i32 value)
{
    this->impl->spellcardBonusPopup.position = Float3(224.0f, 16.0f, 0.0f);
    this->impl->spellcardBonusPopup.displayMode = 1;
    this->impl->spellcardBonusPopup.timer = 0;
    this->impl->spellcardBonusPopup.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437f5c
#pragma var_order(srcRect, destRect)
void __fastcall Gui::CopyEnemyNameTexture(i32 spriteIdx)
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
void Gui::CopyCurrentStageEnemyNameTexture()
{
    switch (g_GameManager.currentStage)
    {
    default:
        CopyEnemyNameTexture(16);
        break;
    case STAGE2:
        CopyEnemyNameTexture(17);
        break;
    case STAGE3:
        CopyEnemyNameTexture(18);
        break;
    case STAGE4A:
        if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberEqualTo(214))
        {
            CopyEnemyNameTexture(19);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(216))
        {
            CopyEnemyNameTexture(26);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(217))
        {
            CopyEnemyNameTexture(27);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(218))
        {
            CopyEnemyNameTexture(28);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(219))
        {
            CopyEnemyNameTexture(29);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(220))
        {
            CopyEnemyNameTexture(30);
        }
        else if (g_GameManager.IsSpellNumberEqualTo(221))
        {
            CopyEnemyNameTexture(31);
        }
        break;
    case STAGE4B:
        CopyEnemyNameTexture(20);
        break;
    case STAGE5:
        if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberEqualTo(212))
            CopyEnemyNameTexture(21);
        else
            CopyEnemyNameTexture(22);
        break;
    case STAGE6A:
        CopyEnemyNameTexture(23);
        break;
    case STAGE6B:
        if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberInRange(147, 150))
            CopyEnemyNameTexture(23);
        else
            CopyEnemyNameTexture(24);
        break;
    case EXTRASTAGE:
        if (!g_GameManager.IsSpellPractice() ||
            g_GameManager.IsSpellNumberInRange(191, 193) ||
            g_GameManager.IsSpellNumberEqualTo(213))
            CopyEnemyNameTexture(32);
        else
            CopyEnemyNameTexture(25);
        break;
    }
}

// FUNCTION: th08 0x43826b
void Gui::DrawStageClearScreen()
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
        this->impl->stageClear.stageBonus);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffe0e0ff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Point = %8d0",
        this->impl->stageClear.pointItemsCollected * 5000);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffd0d0ff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Graze = %8d0",
        this->impl->stageClear.graze * 50);

    stringPos.y += 16.0f;
    g_AsciiManager.SetColor(0xffd0d0ff);
    g_AsciiManager.AddFormatText(
        &stringPos, "Time  = %8d0",
        this->impl->stageClear.timeOrbs * 100);

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
                this->impl->stageClear.clockDisplayTarget / 60 % 12,
                this->impl->stageClear.clockDisplayTarget % 60);
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
        switch (static_cast<i8>(g_GameManager.cfg->lifeCount))
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
        this->impl->stageClearBonusTotal);
    g_AsciiManager.SetColor(0xffffffff);

    if (g_GameManager.currentStage <= STAGE5)
    {
        stringPos.y += 40.0f;
        stringPos.x = 120.0f;
        g_AsciiManager.SetColor(0xffdfdfdf);
        g_AsciiManager.AddFormatText(
            &stringPos, "%s%2d:%.2d",
            g_GuiTimePeriodLabels[(this->impl->stageClear.clockDisplayStart / 60) < 12],
            this->impl->stageClear.clockDisplayStart / 60 % 12,
            this->impl->stageClear.clockDisplayStart % 60);
        stringPos.x += 99.0f;
        g_AsciiManager.SetColor(0xffafafaf);
        g_AsciiManager.AddFormatText(&stringPos, ">>");
        stringPos.x += 34.0f;
        g_AsciiManager.SetColor(0xffff8f8f);
        g_AsciiManager.AddFormatText(
            &stringPos, "%s%2d:%.2d",
            g_GuiTimePeriodLabels[(this->impl->stageClear.clockDisplayCurrent / 60) < 12],
            this->impl->stageClear.clockDisplayCurrent / 60 % 12,
            this->impl->stageClear.clockDisplayCurrent % 60);
        g_AsciiManager.SetColor(0xffffffff);
    }
}

// FUNCTION: th08 0x438a89
void Gui::DrawAsciiText()
{
    char bonusText[32];

    g_AsciiManager.SetIsGuiMode(1);

    if (this->impl->bonusPopup.displayMode)
    {
        g_AsciiManager.SetColor(0xffffff80);
        g_AsciiManager.AddFormatText(&this->impl->bonusPopup.position, " BONUS %8d", this->impl->bonusPopup.value);
        g_AsciiManager.SetColor(0xffffffff);
    }

    switch (this->impl->statusPopup.displayMode)
    {
    case 1:
        g_AsciiManager.SetColor(0xffc0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->statusPopup.position, "Full Power Mode!");
        g_AsciiManager.SetColor(0xffffffff);
        break;
    case 2:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->statusPopup.position, "Supernatural Border!!");
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    case 3:
        g_AsciiManager.SetColor(0xffc0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->statusPopup.position, "CherryPoint Max!");
        g_AsciiManager.SetColor(0xffffffff);
        break;
    case 4:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->statusPopup.position, "Border Bonus %7d", this->impl->statusPopup.value);
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    case 5:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->statusPopup.position, "Spell Bonus Failed");
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    case 6:
        g_AsciiManager.SetScale(0.9f, 1.0f);
        g_AsciiManager.SetSpaceWidth(11);
        g_AsciiManager.SetColor(0xffe0b0ff);
        g_AsciiManager.AddFormatText(&this->impl->statusPopup.position, "Last Spell Failed");
        g_AsciiManager.SetColor(0xffffffff);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetSpaceWidth(13);
        break;
    default:
        break;
    }

    if (this->impl->spellcardBonusPopup.displayMode)
    {
        g_AsciiManager.SetColor(0xffff0000);
        this->impl->spellcardBonusPopup.position.x = (384.0f - (f32)strlen("Spell Card Bonus!") * 14.0f) / 2.0f + 32.0f;
        this->impl->spellcardBonusPopup.position.y = 80.0f;
        g_AsciiManager.AddFormatText(&this->impl->spellcardBonusPopup.position, "Spell Card Bonus!");
        this->impl->spellcardBonusPopup.position.y += 16.0f;
        sprintf(bonusText, "+%d", this->impl->spellcardBonusPopup.value);
        this->impl->spellcardBonusPopup.position.x = (384.0f - strlen(bonusText) * 28.0f) / 2.0f + 32.0f;
        g_AsciiManager.SetScale(2.0f, 2.0f);
        g_AsciiManager.SetColor(0xffff8080);
        g_AsciiManager.AddString(&this->impl->spellcardBonusPopup.position, bonusText);
        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetColor(0xffffffff);
    }

    g_AsciiManager.SetIsGuiMode(0);
}


// FUNCTION: th08 0x438f58
ZunResult Gui::CaptureArcade()
{
    g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(&this->impl->arcadeCaptureVm, 1);
    return g_AnmManager->SetTextureCaptureParams(
        3, 32, 16, 384, 448,
        (i32)this->impl->arcadeCaptureVm.loadedSprite->startPixelInclusive.x,
        (i32)this->impl->arcadeCaptureVm.loadedSprite->startPixelInclusive.y,
        (i32)this->impl->arcadeCaptureVm.loadedSprite->widthPx,
        (i32)this->impl->arcadeCaptureVm.loadedSprite->heightPx);
}

// FUNCTION: th08 0x438fe9
i32 IsInitialStageLoad()
{
    return g_Supervisor.isInitialStageLoad;
}


// FUNCTION: th08 0x438ff3
i32 ReleaseResourcesOnRestart()
{
    return g_Supervisor.releaseResourcesOnRestart;
}

// FUNCTION: th08 0x438ffd
i32 KeepStageResources()
{
    return g_Supervisor.keepStageResources;
}


// FUNCTION: th08 0x439007
i32 Gui::StartStageBackgroundSequence()
{
    this->timesAnm->ExecuteAnmIdx(&this->impl->clockTimeVm, 2);
    this->timesAnm->SetSprite(&this->impl->clockTimeVm, static_cast<i8>(g_GameManager.GetClockTime()));
    return 0;
}

// FUNCTION: th08 0x439050
ZunResult Gui::FlashClockTimeSlow()
{
    this->timesAnm->SetSprite(
        &this->impl->clockTimeVm,
        static_cast<i8>(g_GameManager.GetClockTime()));
    this->impl->clockTimeVm.SetInterrupt(1);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x439093
ZunResult Gui::FlashClockTimeFast()
{
    this->timesAnm->SetSprite(
        &this->impl->clockTimeVm,
        static_cast<i8>(g_GameManager.GetClockTime()));
    this->impl->clockTimeVm.SetInterrupt(2);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4390d6
ZunResult Gui::HideClockTime()
{
    this->impl->clockTimeVm.color1.a = 0;
    return ZUN_SUCCESS;
}

#pragma var_order(i, j, k)
// FUNCTION: th08 0x4390ee
ZunResult Gui::ActualAddedCallback()
{
    i32 i;
    i32 j;
    u32 k;

    if (IsInitialStageLoad())
    {
        memset(this->impl, 0, sizeof(GuiImpl));

        this->frontAnm = g_AnmManager->PreloadAnm(10, "front.anm");
        if (this->frontAnm == NULL)
            return ZUN_ERROR;

        this->InitStageClearScreen();

        this->timesAnm = g_AnmManager->PreloadAnm(14, "times.anm");
        if (this->timesAnm == NULL)
            return ZUN_ERROR;

        this->loadingPortraitAnm = g_AnmManager->PreloadAnm(12, g_GuiLoadingAnmPaths[g_GameManager.shotType]);
        if (this->loadingPortraitAnm == NULL)
            return ZUN_ERROR;

        g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->impl->spellNullifyVm, 26);
        g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->impl->difficultyVm, 25);
        if (g_GameManager.IsSpellPractice() && g_GameManager.currentSpellCardNumber >= 205)
            g_AsciiManager.asciiAnm->SetSprite(&this->impl->difficultyVm, 288);
        else
            g_AsciiManager.asciiAnm->SetSprite(&this->impl->difficultyVm, g_GameManager.difficulty + 283);
    }
    else
    {
        this->InitStageClearScreen();
        g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(&this->impl->arcadeCaptureVm, 1);
        this->impl->arcadeCaptureVm.pendingInterrupt = 1;

        for (i = 0; i < 14; i++)
        {
            for (j = 0; j < 12; j++)
            {
                g_AsciiManager.captureAnm->SetAndExecuteScriptIdx(&this->impl->stageTransitionVms[i * 12 + j], ((i + j) & 1) + 3);
                this->impl->stageTransitionVms[i * 12 + j].counterVar0 = i + j * 2;
                this->impl->stageTransitionVms[i * 12 + j].pos.x = j * 32.0f - 0.5f + 16.0f;
                this->impl->stageTransitionVms[i * 12 + j].pos.y = i * 32.0f - 0.5f + 16.0f;
                this->impl->stageTransitionVms[i * 12 + j].pos.z = 0.0f;
                this->impl->stageTransitionVms[i * 12 + j].uvScrollPos.x = j * 32.0f / 512.0f;
                this->impl->stageTransitionVms[i * 12 + j].uvScrollPos.y = i * 32.0f / 512.0f;
            }
        }
        this->impl->stageTransitionActiveVmCount = 168;
    }

    g_Gui.HideClockTime();
    this->timesAnm->ExecuteAnmIdx(&this->impl->clockIntroVm, 0);
    this->timesAnm->SetSprite(&this->impl->clockIntroVm, static_cast<i8>(g_GameManager.GetClockTime()));

    if (!g_GameManager.IsSpellPractice() &&
        this->LoadMsg(g_GuiMessagePaths[g_GameManager.currentStage][g_GameManager.shotType]) != ZUN_SUCCESS)
        return ZUN_ERROR;

    if (!KeepStageResources())
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

    if (IsInitialStageLoad())
    {
        for (k = 0; k < 16; k++)
            this->frontAnm->SetAndExecuteScriptIdx(&this->impl->frontVms[k], k);
    }

    this->frameCounter = 0;
    this->bossPresent = false;
    this->impl->bossLifeBarState = 0;
    this->bossLifeBarTargetSize = 0.0f;
    this->bossLifeBarDisplayedSize = 0.0f;

    if (!g_GameManager.flags.isSpellPractice)
    {
        this->stageTextAnm->ExecuteAnmIdxArray(&this->impl->stageTextVms[0], 0, 4);
    }
    else if (!KeepStageResources() || FUN_00439916(g_GameManager.currentSpellCardNumber))
    {
        this->stageTextAnm->ExecuteAnmIdxArray(&this->impl->stageTextVms[0], 3, 1);
        this->stageTextAnm->SetSprite(&this->impl->stageTextVms[0], FUN_00439961(g_GameManager.currentSpellCardNumber) + 3);
    }

    this->impl->message.currentMsgIdx = -1;
    this->impl->stageClearScreenState = 0;
    this->impl->bonusPopup.displayMode = 0;
    this->impl->statusPopup.displayMode = 0;
    this->impl->spellcardBonusPopup.displayMode = 0;

    this->flags.lifeDisplayUpdateFrames = 2;
    this->flags.bombDisplayUpdateFrames = 2;
    this->flags.grazeDisplayUpdateFrames = 2;
    this->flags.pointDisplayUpdateFrames = 2;
    this->flags.powerDisplayUpdateFrames = 2;
    this->flags.timeDisplayUpdateFrames = 2;

    g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->impl->stageRankVm, 3);
    g_GuiMessageScreenEffectDuration = 16;
    this->impl->stageClear.clockDisplayCurrent = 0;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4396b8
void Gui::InitStageClearScreen()
{
    this->impl->loadingPortraitVm.activeSpriteIndex = -1;
    this->impl->loadingOverlayVm.activeSpriteIndex = -1;
    this->impl->arcadeCaptureVm.activeSpriteIndex = -1;
    this->impl->stageTransitionActiveVmCount = 0;
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
    this->impl->message.msgFile =
        reinterpret_cast<GuiMessageFile *>(FileSystem::OpenFile(path, NULL, 0));
    if (this->impl->message.msgFile == NULL)
    {
        g_GameErrorContext.Log("\x65\x72\x72\x6f\x72\x20\x3a\x20\x83\x81\x83\x62\x83\x5a\x81\x5b\x83\x57\x83\x74\x83\x40\x83\x43\x83\x8b\x20\x25\x73\x20\x82\xaa\x93\xc7\x82\xdd\x8d\x9e\x82\xdf\x82\xdc\x82\xb9\x82\xf1\x82\xc5\x82\xb5\x82\xbd\x0d\x0a", path);
        return ZUN_ERROR;
    }
    this->impl->message.currentMsgIdx = -1;
    this->impl->message.currentInstr = NULL;
    for (i32 i = 0; i < this->impl->message.msgFile->messageCount; ++i)
    {
        ((i32 *)this->impl->message.msgFile)[i + 1] += (i32)this->impl->message.msgFile;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4397d5
void Gui::FreeMsgFile(void)
{
    if (this->impl->message.msgFile != NULL)
    {
        ZUN_FREE(this->impl->message.msgFile);
    }
}

// FUNCTION: th08 0x439810
void Gui::MsgRead(i32 messageIndex)
{
    this->impl->StartMessage(messageIndex);
}

} /* namespace th08 */
