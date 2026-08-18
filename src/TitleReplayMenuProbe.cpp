#include "th_pch.h"

#include "AsciiManager.hpp"
#include "GameManager.hpp"
#include "ScoreDat.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Spellcard.hpp"
#include "TitleScreen.hpp"
#include "ZunMath.hpp"
#include "i18n.hpp"

#include <direct.h>
#include <stdio.h>

namespace th08
{

enum { TITLE_MENU_ITEM_START_REPLAY = 4 };

DIFFABLE_STATIC_ASSIGN(const char *, g_StageNames[]) = {
    "Stage1 ", "Stage2 ", "Stage3 ", "Stage4A", "Stage4B", "Stage5 ", "Stage6A", "Stage6B", "StageEX",
};

static inline void InitializeTitleVmAndSetSprite(AnmLoaded *anm, AnmVm *vm, i32 sprite)
{
    i32 inlineSlot;
    vm->Initialize();
    vm->anmFile = anm;
    anm->SetSprite(vm, sprite);
}

// Probe-only owner copied from the active Title lane; production TitleScreen.cpp
// remains untouched. Canonical acceptance still requires target replay here.
#pragma var_order(i, firstFile, replayCount, fileSize, replayData, path, findData, fileSize2)
ChainCallbackResult TitleScreen::OnUpdateReplayMenu()
{
    i32 i;
    i32 replayCount;
    i32 fileSize;
    i32 fileSize2;
    HANDLE firstFile;
    WIN32_FIND_DATAA findData;
    ReplayData *replayData;
    char path[64];

    // Yeah, the enum doesn't work well here so, it's cast into an int
    switch ((i32)this->currentScreenState)
    {
    case 0:
        if (this->stateTimer2 == 0)
        {
            if (this->previousScreen != TitleCurrentScreen_Replay)
            {
                if (g_AnmManager->LoadSurface(0, "title/select00.png") != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
                }
            }

            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 14);

            this->cursor = 0;
            this->currentScreenState = TitleCurrentScreenState_Init;
            this->stateTimer = 0;
            this->currentHelpTextVm = NULL;

            replayCount = 0;

            for (i = 0; i < 15; i++)
            {
                sprintf(path, "./replay/th8_%.2d.rpy", i + 1);

                replayData = (ReplayData *)FileSystem::OpenFile(path, &fileSize, TRUE);
                if (replayData == NULL)
                {
                    continue;
                }

                replayData = ReplayManager::LoadReplayData(replayData, fileSize);
                if (replayData != NULL)
                {
                    this->replays[replayCount] = *replayData;

                    strcpy(this->replayFilePaths[replayCount], path);
                    sprintf(this->replayNumbers[replayCount], "No.%.2d", i + 1);

                    replayCount++;

                    g_ZunMemory.Free(replayData);
                }
            }

            _mkdir("./replay");
            _chdir("./replay");

            firstFile = FindFirstFileA("th8_ud????.rpy", &findData);
            if (firstFile != INVALID_HANDLE_VALUE)
            {
                for (i = 0; i < 45; i++)
                {
                    replayData = (ReplayData *)FileSystem::OpenFile(findData.cFileName, &fileSize, TRUE);
                    if (replayData == NULL)
                    {
                        continue;
                    }

                    replayData = ReplayManager::LoadReplayData(replayData, fileSize);
                    if (replayData != NULL)
                    {
                        this->replays[replayCount] = *replayData;

                        sprintf(this->replayFilePaths[replayCount], "./replay/%s", findData.cFileName);
                        sprintf(this->replayNumbers[replayCount], "User ");

                        g_ZunMemory.Free(replayData);

                        replayCount++;
                    }

                    if (!FindNextFileA(firstFile, &findData))
                    {
                        break;
                    }
                }
            }

            /* ZUN bug: this should be in the above `if` block, but this
             * doesn't do anything really if it's invalid.
             */
            FindClose(firstFile);
            _chdir("../");
            this->replayCount = replayCount;
            this->unk0xc284 = 0;
        }

        if (this->stateTimer2 >= 8)
        {
            this->currentScreenState = (TitleCurrentScreenState)1;
            this->stateTimer = 0;
        }
        break;
    case 1:
        this->MoveCursorVertical(this->replayCount);
        if (this->replayCount > TITLE_REPLAYS_PER_PAGE)
        {
            if (WAS_PRESSED_SCROLLING(TH_BUTTON_LEFT))
            {
                this->cursor -= TITLE_REPLAYS_PER_PAGE;
                if (this->cursor < 0)
                {
                    this->cursor += this->replayCount;
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
            }
            if (WAS_PRESSED_SCROLLING(TH_BUTTON_RIGHT))
            {
                this->cursor += TITLE_REPLAYS_PER_PAGE;
                if (this->cursor >= this->replayCount)
                {
                    this->cursor -= this->replayCount;
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
            }
        }

        this->selectedReplay = this->cursor;

        if (this->stateTimer < 10)
        {
            break;
        }

        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            if (this->replayCount == 0)
            {
                break;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            this->currentScreenState = (TitleCurrentScreenState)2;

            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 15);
            this->vms[this->selectedReplay % TITLE_REPLAYS_PER_PAGE + 80].SetInterrupt(17);

            this->currentReplay =
                (ReplayData *)FileSystem::OpenFile(this->replayFilePaths[this->selectedReplay], &fileSize2, TRUE);
            this->currentReplay = ReplayManager::LoadReplayData(this->currentReplay, fileSize2);

            for (i = 0; i < MAX_STAGES; i++)
            {
                if (this->currentReplay->header.stageReplayData[i] != NULL)
                {
                    this->currentReplay->header.stageReplayData[i] =
                        (StageReplayData *)((u32)this->currentReplay +
                                            (u32)this->currentReplay->header.stageReplayData[i]);
                }
            }

            this->cursor = 0;

            while (this->replays[this->selectedReplay].header.stageReplayData[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor > EXTRASTAGE)
                {
                    g_GameErrorContext.Fatal(TH_ERR_REPLAY_CORRUPTED);
                    g_Supervisor.curState = SupervisorState_ExitGame;

                    return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
                }
            }

            InitializeTitleVmAndSetSprite(this->resultTextAnm, &this->spellCardNameVms[0], i + 2);

            this->spellCardNameVms[0].pos = Float3(0.0, 0.0, 0.0);
            this->spellCardNameVms[0].anchor = 3;
            this->spellCardNameVms[0].fontWidth = 15;
            this->spellCardNameVms[0].fontHeight = 15;

            g_AnmManager->DrawTextLeft(&this->spellCardNameVms[0], COLOR_TEXT_WHITE, 0,
                                       this->replays[this->selectedReplay].spellcardName);

            this->spellCardNameVms[0].color1.a = 255;
            this->spellCardNameVms[0].color1.r = 255;
            this->spellCardNameVms[0].color1.g = 255;
            this->spellCardNameVms[0].color1.b = 255;
            break;
        }

        if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->currentScreenState = (TitleCurrentScreenState)4;
            this->stateTimer = 0;
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 16);
        }

        break;
    case 2:
        i = this->MoveCursorVertical(9);
        if (i < 0)
        {
            while (this->replays[this->selectedReplay].header.stageReplayData[this->cursor] == NULL)
            {
                this->cursor--;
                if (this->cursor < 0)
                {
                    this->cursor = MAX_STAGES;
                }
            }
        }
        else if (i > 0)
        {
            while (this->replays[this->selectedReplay].header.stageReplayData[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor >= ARRAY_SIZE(g_StageNames))
                {
                    this->cursor = 0;
                }
            }
        }

        this->selectedReplayStage = this->cursor;

        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 19);
            this->vms[this->selectedReplay % TITLE_REPLAYS_PER_PAGE + 80].SetInterrupt(17);

            this->currentScreenState = (TitleCurrentScreenState)3;
            this->cursor = 0;

            this->vms[108].pendingInterrupt = 21;
            this->vms[109].pendingInterrupt = 21;
            if (this->currentReplay->spellcardNumber < 0)
            {
                this->vms[110].pendingInterrupt = 21;
            }
            else
            {
                this->vms[110].color1.a = 0;
            }
            this->vms[this->cursor + 108].pendingInterrupt = 20;
            break;
        }

        if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
        {
            g_ZunMemory.Free(this->currentReplay);
            this->currentReplay = NULL;
            this->currentScreenState = (TitleCurrentScreenState)1;
            this->stateTimer2 = 0;
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 14);
            this->cursor = this->selectedReplay;
            break;
        }
        break;
    case 3:
        i = this->MoveCursorVertical((this->currentReplay->spellcardNumber < 0) ? 3 : 2);
        if (i != 0)
        {
            this->vms[108].pendingInterrupt = 21;
            this->vms[109].pendingInterrupt = 21;
            if (this->currentReplay->spellcardNumber < 0)
            {
                this->vms[110].pendingInterrupt = 21;
            }
            else
            {
                this->vms[110].color1.a = 0;
            }
            this->vms[this->cursor + 108].pendingInterrupt = 20;
        }
        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            g_GameManager.SetIsReplayWeird(TRUE);

            strcpy(g_GameManager.replayFilename, this->replayFilePaths[this->selectedReplay]);

            g_GameManager.difficulty = this->currentReplay->difficulty;
            g_GameManager.shotType = this->currentReplay->shotType;
            // Leftover from PCB
            g_GameManager.shotType = this->currentReplay->shotType;
            g_GameManager.flags.isSpellPractice = (this->currentReplay->spellcardNumber >= 0);
            g_GameManager.currentSpellCardNumber = this->currentReplay->spellcardNumber;

            g_ZunMemory.Free(this->currentReplay);
            this->currentReplay = NULL;

            g_GameManager.currentStage = this->selectedReplayStage;
            g_Supervisor.curState = SupervisorState_GameManager;
            g_GameManager.replayMode = this->cursor;

            g_Supervisor.StopAudio();

            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }

        if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
        {
            this->currentScreenState = (TitleCurrentScreenState)2;
            this->stateTimer2 = 0;
            this->cursor = this->selectedReplayStage;

            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 15);

            this->vms[this->selectedReplay % TITLE_REPLAYS_PER_PAGE + 80].SetInterrupt(17);
            break;
        }
        break;
    case 4:
        if (this->stateTimer >= 30)
        {
            this->ChangeCurrentScreen(TitleCurrentScreen_StartMenu);
            this->cursor = TITLE_MENU_ITEM_START_REPLAY;

            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }
        break;
    }

    this->idleFrames++;
    this->stateTimer++;
    this->stateTimer2++;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}



} // namespace th08
