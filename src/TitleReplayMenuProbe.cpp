#include "th_pch.h"

#include "AsciiManager.hpp"
#include "GameManager.hpp"
#include "ScoreDat.hpp"
#include "ResultScreen.hpp"
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

inline Float3::Float3(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

#define TITLE_SPRITE_KEYCONFIG_SLOWSHOT_START 75
#define TITLE_SPRITE_KEYCONFIG_SLOWSHOT_END 76

#define TITLE_SPRITE_CHARACTER_START 111
#define TITLE_SPRITE_CHARACTER_END 130

extern i32 g_TitleCharacterSpriteIndices[SHOT_ALL][4];
extern const char *g_FullWidthDigits[];
extern char g_FullWidthNumberBuffer[64];



enum
{
    TITLE_MENU_ITEM_START_START = 0,
    TITLE_MENU_ITEM_START_EXTRA_START = 1,
    TITLE_MENU_ITEM_START_SPELL_PRACTICE = 2,
    TITLE_MENU_ITEM_START_PRACTICE_START = 3,
    TITLE_MENU_ITEM_START_REPLAY = 4,
    TITLE_MENU_ITEM_START_RESULT = 5,
    TITLE_MENU_ITEM_START_MUSIC_ROOM = 6,
    TITLE_MENU_ITEM_START_OPTION = 7,
    TITLE_MENU_ITEM_START_QUIT = 8,
    TITLE_MENU_ITEM_START_NUM_ITEMS
};


enum { TITLE_MENU_ITEM_OPTION_KEYCONFIG = 8 };

enum
{
    TITLE_MENU_ITEM_KEYCONFIG_SHOT = 0,
    TITLE_MENU_ITEM_KEYCONFIG_BOMB = 1,
    TITLE_MENU_ITEM_KEYCONFIG_SLOW = 2,
    TITLE_MENU_ITEM_KEYCONFIG_SKIP = 3,
    TITLE_MENU_ITEM_KEYCONFIG_PAUSE = 4,
    TITLE_MENU_ITEM_KEYCONFIG_UP = 5,
    TITLE_MENU_ITEM_KEYCONFIG_DOWN = 6,
    TITLE_MENU_ITEM_KEYCONFIG_LEFT = 7,
    TITLE_MENU_ITEM_KEYCONFIG_RIGHT = 8,
    TITLE_MENU_ITEM_KEYCONFIG_SHOTSLOW = 9,
    TITLE_MENU_ITEM_KEYCONFIG_RESET = 10,
    TITLE_MENU_ITEM_KEYCONFIG_QUIT = 11,
};

DIFFABLE_STATIC_ASSIGN(i16, g_LastKeyChanged) = 32;
DIFFABLE_STATIC_ASSIGN(const char *, g_KeyConfigHelpText[]) = {
    TH_TITLE_KEYCONFIG_HELPTEXT0, TH_TITLE_KEYCONFIG_HELPTEXT1, TH_TITLE_KEYCONFIG_HELPTEXT2,
    TH_TITLE_KEYCONFIG_HELPTEXT3, TH_TITLE_KEYCONFIG_HELPTEXT4, TH_TITLE_KEYCONFIG_HELPTEXT5,
    TH_TITLE_KEYCONFIG_HELPTEXT6, TH_TITLE_KEYCONFIG_HELPTEXT7, TH_TITLE_KEYCONFIG_HELPTEXT8,
    TH_TITLE_KEYCONFIG_HELPTEXT9, TH_TITLE_KEYCONFIG_HELPTEXT10, TH_TITLE_KEYCONFIG_HELPTEXT11,
};

DIFFABLE_STATIC_ASSIGN(const char *, g_DemoReplayFiles[]) = {
    "demo/demorpy0.rpy", "demo/demorpy1.rpy", "demo/demorpy2.rpy", "demo/demorpy3.rpy",
};

DIFFABLE_STATIC_ASSIGN(const char *, g_StartMenuHelpText[]) = {
    TH_TITLE_STARTMENU_HELPTEXT0, TH_TITLE_STARTMENU_HELPTEXT1, TH_TITLE_STARTMENU_HELPTEXT2,
    TH_TITLE_STARTMENU_HELPTEXT3, TH_TITLE_STARTMENU_HELPTEXT4, TH_TITLE_STARTMENU_HELPTEXT5,
    TH_TITLE_STARTMENU_HELPTEXT6, TH_TITLE_STARTMENU_HELPTEXT7, TH_TITLE_STARTMENU_HELPTEXT8,
};

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

    // This screen stores its replay-specific state in the shared state field.
    switch ((i32)this->currentScreenState)
    {
    case TITLE_REPLAY_MENU_ENUMERATE_REPLAYS:
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
            this->replayEnumerationResetState = 0;
        }

        if (this->stateTimer2 >= 8)
        {
            this->currentScreenState =
                static_cast<TitleCurrentScreenState>(TITLE_REPLAY_MENU_SELECT_REPLAY);
            this->stateTimer = 0;
        }
        break;
    case TITLE_REPLAY_MENU_SELECT_REPLAY:
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

            this->currentScreenState =
                static_cast<TitleCurrentScreenState>(TITLE_REPLAY_MENU_SELECT_STAGE);

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
            this->currentScreenState =
                static_cast<TitleCurrentScreenState>(TITLE_REPLAY_MENU_EXIT);
            this->stateTimer = 0;
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 16);
        }

        break;
    case TITLE_REPLAY_MENU_SELECT_STAGE:
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

            this->currentScreenState =
                static_cast<TitleCurrentScreenState>(TITLE_REPLAY_MENU_SELECT_PLAYBACK_MODE);
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
            this->currentScreenState =
                static_cast<TitleCurrentScreenState>(TITLE_REPLAY_MENU_SELECT_REPLAY);
            this->stateTimer2 = 0;
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 14);
            this->cursor = this->selectedReplay;
            break;
        }
        break;
    case TITLE_REPLAY_MENU_SELECT_PLAYBACK_MODE:
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
            this->currentScreenState =
                static_cast<TitleCurrentScreenState>(TITLE_REPLAY_MENU_SELECT_STAGE);
            this->stateTimer2 = 0;
            this->cursor = this->selectedReplayStage;

            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 15);

            this->vms[this->selectedReplay % TITLE_REPLAYS_PER_PAGE + 80].SetInterrupt(17);
            break;
        }
        break;
    case TITLE_REPLAY_MENU_EXIT:
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





ChainCallbackResult TitleScreen::OnUpdateStartMenu()
{
    i32 i;
    i32 fileSize;

    switch (this->currentScreenState)
    {
    case TitleCurrentScreenState_Init:
        if (this->stateTimer2 == 0)
        {
            if (this->previousScreen == TitleCurrentScreen_StartMenu &&
                g_Supervisor.wantedState2 != SupervisorState_ResultScreen)
            {
                g_Supervisor.PlayMusic(8, 0);
            }

            if (this->previousScreen == TitleCurrentScreen_StartMenu ||
                this->previousScreen == TitleCurrentScreen_DifficultySelect ||
                this->previousScreen == TitleCurrentScreen_Replay ||
                this->previousScreen == TitleCurrentScreen_DifficultySelectPractice ||
                this->previousScreen == TitleCurrentScreen_DifficultySelectExtra ||
                this->previousScreen == TitleCurrentScreen_CharacterSelectSpell ||
                this->previousScreen == TitleCurrentScreen_SpellStageSelect)
            {
                if (g_AnmManager->LoadSurface(0, "title/title00.png") != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
                }
            }

            if (this->vmCount == 0)
            {
                this->vmCount = 142;
                this->vms = new AnmVm[this->vmCount];
                this->titleAnm->ExecuteAnmIdxArray(this->vms, 0, this->vmCount);
            }

            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 2);

            if (g_GameManager.flags.isReplay)
            {
                this->ChangeCurrentScreen(TitleCurrentScreen_Replay);
                g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 13);
                this->currentHelpTextVm->SetInterrupt(2);
                g_GameManager.SetIsReplayWeird(FALSE);

                return CHAIN_CALLBACK_RESULT_CONTINUE;
            }

            if (this->practiceState != 0)
            {
                if (this->practiceState == 2)
                {
                    g_GameManager.flags.isSpellPractice = TRUE;
                }

                this->ChangeCurrentScreen(g_GameManager.flags.isSpellPractice
                                              ? TitleCurrentScreen_CharacterSelectSpell
                                              : TitleCurrentScreen_DifficultySelectPractice);

                g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 5);
                this->currentHelpTextVm->SetInterrupt(2);

                return CHAIN_CALLBACK_RESULT_CONTINUE;
            }

            /* Set each menu item's sprite. */
            for (i = 0; i < TITLE_MENU_ITEM_START_NUM_ITEMS; i++)
            {
                this->titleAnm->SetSprite(&this->vms[1 + i], this->vms[1 + i].baseSpriteIndex + 1);
            }

            /* Mark the selected menu item. */
            this->titleAnm->SetSprite(&this->vms[1 + this->cursor], this->vms[1 + this->cursor].baseSpriteIndex);

            /* Mark the "Spell Practice" button as grayed out. */
            if (!g_GameManager.IsSpellPracticeUnlocked())
            {
                this->vms[3].color1.d3dColor = 0xff404040;
            }

            /* Mark the "Extra Start" button as grayed out. */
            if (!g_GameManager.IsExtraUnlocked())
            {
                this->vms[2].color1.d3dColor = 0xff404040;
            }
        }

        if (this->stateTimer2 < ARRAY_SIZE(g_StartMenuHelpText))
        {
            g_AnmManager->DrawTextCentered(&this->helpTextVms[this->stateTimer2], 0xfff0e0, 0x300000,
                                           g_StartMenuHelpText[this->stateTimer2]);
            this->stateTimer2++;

            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }

        this->stateTimer2 = 0;
        this->stateTimer = 0;
        this->cursor2 = -1;
        this->currentScreenState = TitleCurrentScreenState_Ready;
        this->startMenuIdleFrames = 0;
    case TitleCurrentScreenState_Ready:
        i = this->MoveCursorVertical(9);
        if (i != 0)
        {
            /* ... Just why, ZUN */
        back:
            if (!g_GameManager.IsSpellPracticeUnlocked())
            {
                if (this->cursor == TITLE_MENU_ITEM_START_SPELL_PRACTICE)
                {
                    this->cursor += i;
                    goto back;
                }
            }

            if (!g_GameManager.IsExtraUnlocked())
            {
                if (this->cursor == TITLE_MENU_ITEM_START_EXTRA_START)
                {
                    this->cursor += i;
                    goto back;
                }
            }

            /* Set each menu item's sprite. */
            for (i = 0; i < TITLE_MENU_ITEM_START_NUM_ITEMS; i++)
            {
                this->titleAnm->SetSprite(&this->vms[1 + i], this->vms[1 + i].baseSpriteIndex + 1);
            }

            /* Mark the selected menu item. */
            this->titleAnm->SetSprite(&this->vms[1 + this->cursor], this->vms[1 + this->cursor].baseSpriteIndex);

            /* Mark the "Spell Practice" button as grayed out. */
            if (!g_GameManager.IsSpellPracticeUnlocked())
            {
                this->vms[3].color1.d3dColor = 0xff404040;
            }

            /* Mark the "Extra Start" button as grayed out. */
            if (!g_GameManager.IsExtraUnlocked())
            {
                this->vms[2].color1.d3dColor = 0xff404040;
            }
        }

        this->startMenuIdleFrames++;
        if (g_CurFrameInput != 0)
        {
            this->startMenuIdleFrames = 0;
        }

        if (this->startMenuIdleFrames > 1500)
        {
            g_GameManager.currentDemoReplay++;
            g_GameManager.currentDemoReplay %= ARRAY_SIZE_SIGNED(g_DemoReplayFiles);
            strcpy(g_GameManager.replayFilename, g_DemoReplayFiles[g_GameManager.currentDemoReplay]);

            this->currentReplay = (ReplayData *)FileSystem::OpenFile(g_GameManager.replayFilename, &fileSize, FALSE);
            this->currentReplay = ReplayManager::LoadReplayData(this->currentReplay, fileSize);

            if (this->currentReplay == NULL)
            {
                utils::GuiDebugPrint("error : Demo Play is not ready\r\n");
                this->startMenuIdleFrames = 0;
            }
            else
            {
                g_GameManager.SetIsReplayWeird(TRUE);
                g_GameManager.flags.isDemoMode = TRUE;
                g_GameManager.demoFrameCount = 0;
                g_GameManager.difficulty = this->currentReplay->difficulty;

                // Leftover from PCB
                g_GameManager.shotType = this->currentReplay->shotType / 2;
                g_GameManager.fullShotType = this->currentReplay->shotType % 2;
                g_GameManager.shotType = this->currentReplay->shotType;

                i = 0;

                while (this->currentReplay->header.stageReplayData[i] == NULL)
                {
                    i++;
                }

                g_GameManager.currentStage = i;

                g_ZunMemory.Free(this->currentReplay);
                this->currentReplay = NULL;

                g_Supervisor.curState = SupervisorState_GameManager;
                g_GameManager.replayMode = REPLAY_MODE_NORMAL;

                g_GameManager.flags.isSpellPractice = FALSE;

                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            }
        }

        if (this->cursor2 != this->cursor)
        {
            this->currentHelpTextVm = this->helpTextVms + this->cursor;
            this->currentHelpTextVm->SetInterrupt(1);
        }

        this->cursor2 = this->cursor;

        if (this->stateTimer2 < 10)
        {
            break;
        }

        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            g_SoundPlayer.ProcessQueues();

            switch (this->cursor)
            {
            case TITLE_MENU_ITEM_START_START:
                g_GameManager.flags.isPracticeMode = FALSE;
                g_GameManager.flags.isSpellPractice = FALSE;
                this->cursor = g_Supervisor.cfg.defaultDifficulty;
                if (this->cursor >= EXTRA)
                {
                    this->cursor = HARD;
                }

                this->ChangeCurrentScreen(TitleCurrentScreen_DifficultySelect);
                g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 5);
                this->currentHelpTextVm->SetInterrupt(2);

                return CHAIN_CALLBACK_RESULT_CONTINUE;
            case TITLE_MENU_ITEM_START_PRACTICE_START:
                g_GameManager.flags.isPracticeMode = TRUE;
                g_GameManager.flags.isSpellPractice = FALSE;

                this->cursor = g_Supervisor.cfg.defaultDifficulty;
                if (this->cursor >= EXTRA)
                {
                    this->cursor = HARD;
                }

                this->ChangeCurrentScreen(TitleCurrentScreen_DifficultySelectPractice);
                g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 5);

                this->currentHelpTextVm->SetInterrupt(2);

                return CHAIN_CALLBACK_RESULT_CONTINUE;
            case TITLE_MENU_ITEM_START_EXTRA_START:
                if (g_GameManager.IsExtraUnlocked())
                {
                    g_GameManager.flags.isPracticeMode = FALSE;
                    g_GameManager.flags.isSpellPractice = FALSE;

                    this->cursor = 0;
                    this->ChangeCurrentScreen(TitleCurrentScreen_DifficultySelectExtra);

                    g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 5);
                    this->currentHelpTextVm->SetInterrupt(2);

                    return CHAIN_CALLBACK_RESULT_CONTINUE;
                }
            case TITLE_MENU_ITEM_START_SPELL_PRACTICE:
                if (g_GameManager.IsSpellPracticeUnlocked())
                {
                    g_GameManager.flags.isPracticeMode = TRUE;
                    g_GameManager.flags.isSpellPractice = TRUE;

                    this->cursor = g_GameManager.shotType;
                    this->ChangeCurrentScreen(TitleCurrentScreen_SpellStageSelect);

                    g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 5);
                    this->currentHelpTextVm->SetInterrupt(2);

                    return CHAIN_CALLBACK_RESULT_CONTINUE;
                }
            case TITLE_MENU_ITEM_START_REPLAY:
                g_GameManager.flags.isPracticeMode = FALSE;
                g_GameManager.flags.isSpellPractice = FALSE;

                this->ChangeCurrentScreen(TitleCurrentScreen_Replay);

                g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 13);
                this->currentHelpTextVm->SetInterrupt(2);

                return CHAIN_CALLBACK_RESULT_CONTINUE;
            case TITLE_MENU_ITEM_START_MUSIC_ROOM:
                g_Supervisor.curState = SupervisorState_MusicRoom;
                this->currentHelpTextVm->SetInterrupt(2);
                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            case TITLE_MENU_ITEM_START_RESULT:
                g_Supervisor.curState = SupervisorState_ResultScreen;
                this->currentHelpTextVm->SetInterrupt(2);
                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            case TITLE_MENU_ITEM_START_OPTION:
                // Enter the options initializer immediately, then leave its
                // cursor on the first row for the next title update.
                this->currentScreenState = TitleCurrentScreenState_Init;
                this->cursor = 0;
                this->stateTimer2 = 0;
                this->stateTimer = 0;
                this->currentScreenState = TitleCurrentScreenState_Changing;
                this->stateTimer = 0;
                this->OnUpdateOptions();
                this->cursor = 0;
                break;
            case TITLE_MENU_ITEM_START_QUIT:
                this->currentScreenState = TitleCurrentScreenState_Exit;
                this->stateTimer = 0;
                g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 1);
                if (g_Supervisor.cfg.musicMode == MIDI)
                {
                    g_Supervisor.midiOutput->PlayFile(30);
                }
                break;
            }
        }

        if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
        {
            this->titleAnm->SetSprite(&this->vms[this->cursor + 1], this->vms[this->cursor + 1].baseSpriteIndex + 1);
            this->cursor = TITLE_MENU_ITEM_START_QUIT;
            this->titleAnm->SetSprite(&this->vms[this->cursor + 1], this->vms[this->cursor + 1].baseSpriteIndex);
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            g_SoundPlayer.ProcessQueues();
        }
        break;
    case TitleCurrentScreenState_Exit:
        if (stateTimer >= 60)
        {
            ZUN_DELETE2(this->vms);
            // Yes, this->vms is set to NULL twice.
            this->vms = NULL;

            this->vmCount = 0;
            this->stateTimer2 = 0;

            g_Supervisor.curState = SupervisorState_ExitGame;

            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case TitleCurrentScreenState_Changing:
        if (stateTimer >= 30)
        {
            this->ChangeCurrentScreen(TitleCurrentScreen_Option);
            this->cursor = 0;
            this->currentGameConfig = g_Supervisor.cfg;

            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }
        break;
    }

    this->idleFrames++;
    this->stateTimer++;
    this->stateTimer2++;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}



#pragma var_order(vmPair, i, keyToChange, controllerState)
ChainCallbackResult TitleScreen::OnUpdateKeyConfig()
{
    AnmVm *vmPair;
    i32 i;
    u8 *controllerState;
    i16 keyToChange;

    switch (this->currentScreenState)
    {
    case TitleCurrentScreenState_Init:
        if (this->stateTimer2 == 0)
        {
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 4);

            for (i = 0; i < 12; i++)
            {
                this->titleAnm->SetSprite(&this->vms[i + 45], this->vms[i + 45].baseSpriteIndex + 1);
            }

            this->titleAnm->SetSprite(&this->vms[this->cursor + 45], this->vms[this->cursor + 45].baseSpriteIndex);
            this->currentScreenState = TitleCurrentScreenState_Init;
            this->stateTimer = 0;

            this->controllerMapping = g_Supervisor.cfg.controllerMapping;

            g_Supervisor.cfg.controllerMapping.upButton = -1;
            g_Supervisor.cfg.controllerMapping.downButton = -1;

            /* Yes, ZUN really did write this. */
            vmPair = &this->vms[57];
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.shotButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.bombButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.focusButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.skipButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.menuButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.upButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.downButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.leftButton);
            vmPair += 2;
            this->SetKeyNumberSprite(vmPair, this->controllerMapping.rightButton);

            this->cursor2 = -1;
        }

        this->currentScreenState = TitleCurrentScreenState_Ready;

        for (i = 0; i < ARRAY_SIZE(g_KeyConfigHelpText); i++)
        {
            g_AnmManager->DrawTextCentered(&this->helpTextVms[i], 0xfff0e0, 0x300000, g_KeyConfigHelpText[i]);
        }

    case TitleCurrentScreenState_Ready:
        if (this->MoveCursorVertical(12) != 0)
        {
            for (i = 0; i < 12; i++)
            {
                this->titleAnm->SetSprite(&this->vms[i + 45], this->vms[i + 45].baseSpriteIndex + 1);
            }

            this->titleAnm->SetSprite(&this->vms[this->cursor + 45], this->vms[this->cursor + 45].baseSpriteIndex);
        }
        if (this->cursor2 != this->cursor)
        {
            this->currentHelpTextVm = &this->helpTextVms[this->cursor];
            this->currentHelpTextVm->SetInterrupt(1);
        }
        this->cursor2 = this->cursor;

        vmPair = &this->vms[57];
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.shotButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.bombButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.focusButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.skipButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.menuButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.upButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.downButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.leftButton);
        vmPair += 2;
        this->SetKeyNumberSprite(vmPair, this->controllerMapping.rightButton);

        for (i = TITLE_SPRITE_KEYCONFIG_SLOWSHOT_START; i <= TITLE_SPRITE_KEYCONFIG_SLOWSHOT_END; i++)
        {
            this->titleAnm->SetSprite(&this->vms[i], this->vms[i].baseSpriteIndex + 1);
        }

        i = TITLE_SPRITE_KEYCONFIG_SLOWSHOT_START + g_Supervisor.cfg.shotSlow;
        this->titleAnm->SetSprite(&this->vms[i], this->vms[i].baseSpriteIndex);

        controllerState = Controller::GetControllerState();

        for (keyToChange = 0; keyToChange < 32; keyToChange++)
        {
            if ((controllerState[keyToChange] & TH_BUTTON_RIGHT) != 0)
            {
                break;
            }
        }

        if (keyToChange < 32 && g_LastKeyChanged != keyToChange)
        {
            switch (this->cursor)
            {
            case TITLE_MENU_ITEM_KEYCONFIG_SHOT:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.shotButton, 1);
                this->controllerMapping.shotButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_BOMB:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.bombButton, 0);
                this->controllerMapping.bombButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_SLOW:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.focusButton, 1);
                this->controllerMapping.focusButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_PAUSE:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.menuButton, 0);
                this->controllerMapping.menuButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_UP:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.upButton, 0);
                this->controllerMapping.upButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_DOWN:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.downButton, 0);
                this->controllerMapping.downButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_LEFT:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.leftButton, 0);
                this->controllerMapping.leftButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_RIGHT:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.rightButton, 0);
                this->controllerMapping.rightButton = keyToChange;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_SKIP:
                this->SetKeyConfigKey(keyToChange, this->controllerMapping.skipButton, 0);
                this->controllerMapping.skipButton = keyToChange;
                break;
            default:
                goto out;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            g_SoundPlayer.ProcessQueues();
        }

    out:

        g_LastKeyChanged = keyToChange;

        if (WAS_PRESSED(TH_BUTTON_LEFT))
        {
            switch (this->cursor)
            {
            case TITLE_MENU_ITEM_KEYCONFIG_SHOTSLOW:
                g_Supervisor.cfg.shotSlow = 1 - g_Supervisor.cfg.shotSlow;
                break;
            }
        }

        if (WAS_PRESSED(TH_BUTTON_RIGHT))
        {
            switch (this->cursor)
            {
            case TITLE_MENU_ITEM_KEYCONFIG_SHOTSLOW:
                g_Supervisor.cfg.shotSlow = 1 - g_Supervisor.cfg.shotSlow;
                break;
            }
        }

        if (g_CurFrameInput != 0)
        {
            this->idleFrames = 0;
        }

        if (this->idleFrames >= 3600)
        {
            goto exit_keyconfig;
        }

        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            switch (this->cursor)
            {
            case TITLE_MENU_ITEM_KEYCONFIG_RESET:
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                g_SoundPlayer.ProcessQueues();

                this->controllerMapping = g_ControllerMapping;
                g_Supervisor.cfg.shotSlow = TRUE;
                break;
            case TITLE_MENU_ITEM_KEYCONFIG_QUIT:
            exit_keyconfig:
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
                g_SoundPlayer.ProcessQueues();

                this->ChangeCurrentScreen(TitleCurrentScreen_Option);

                g_Supervisor.cfg.controllerMapping = this->controllerMapping;
                this->cursor = TITLE_MENU_ITEM_OPTION_KEYCONFIG;
                return CHAIN_CALLBACK_RESULT_CONTINUE;
            }
        }

        break;
    }

    this->idleFrames++;
    this->stateTimer++;
    this->stateTimer2++;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}



#pragma var_order(spellCardNumber, i, oldCursor, spellCardNumber2, i2, oldPageIdx, spellCardNumber3)
ChainCallbackResult TitleScreen::OnUpdateSpellCardSelect()
{
    i32 spellCardNumber;
    i32 spellCardNumber3;
    i32 spellCardNumber2;
    i32 i;
    i32 i2;
    i32 oldCursor;
    i32 oldPageIdx;

    switch (this->currentScreenState)
    {
    case TitleCurrentScreenState_Init:
        if (this->stateTimer2 == 0)
        {
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 26);

            this->practiceState = 0;
            this->currentScreenState = TitleCurrentScreenState_Init;
            this->stateTimer = 0;

            g_GameManager.flags.isPracticeMode = TRUE;
            g_GameManager.flags.isSpellPractice = TRUE;

            this->currentNumberOfSpellCards = g_SpellcardCountPerStage[g_GameManager.currentStage];

            this->UnlockLastWordSpellCards();

            this->cursor = 0;
            for (i = 0; i < this->currentNumberOfSpellCards; i++)
            {
                if (g_SpellcardNumbersPerStage[g_GameManager.currentStage][i] == g_GameManager.currentSpellCardNumber)
                {
                    this->cursor = i;
                    break;
                }
            }

            this->currentPageSpellCardSelect = this->cursor / TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE;

            for (i = 0; i < TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE; i++)
            {
                if ((i + this->currentPageSpellCardSelect * TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE) >=
                    this->currentNumberOfSpellCards)
                {
                    break;
                }

                spellCardNumber = g_SpellcardNumbersPerStage[g_GameManager.currentStage]
                                                            [i + this->currentPageSpellCardSelect *
                                                                     TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE];

                InitializeTitleVmAndSetSprite(this->resultTextAnm, &this->spellCardNameVms[i], i + 2);
                this->spellCardNameVms[i].pos = Float3(0, 0, 0);
                this->spellCardNameVms[i].anchor = 3;
                /* Copy paste mistake? */
                this->spellCardNameVms[0].fontWidth = 15;
                this->spellCardNameVms[i].fontHeight = 15;

                if (g_GameManager.catkData[spellCardNumber].inGameHistory.attempts[SHOT_ALL] == 0 &&
                    g_GameManager.catkData[spellCardNumber].spellPracticeHistory.attempts[SHOT_ALL] == 0)
                {
                    if (Spellcard::GetDifficultyFromSpellCard(spellCardNumber) <= EXTRA ||
                        !g_GameManager.IsLastWordSpellCardAttempted(spellCardNumber))
                    {
                        g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i], COLOR_TEXT_WHITE, 0,
                                                   TH_TITLE_SPELLCARD_NOT_UNLOCKED);
                    }
                    else
                    {
                        g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i], COLOR_TEXT_WHITE, 0,
                                                   TH_TITLE_SPELLCARD_AVAILABLE);
                    }
                }
                else
                {
                    g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i], COLOR_TEXT_WHITE, 0,
                                               g_GameManager.catkData[spellCardNumber].spellName);
                }

                this->spellCardNameVms[i].color1.a = 255;
                this->spellCardNameVms[i].color1.r = 96;
                this->spellCardNameVms[i].color1.g = 96;
                this->spellCardNameVms[i].color1.b = 96;
            }

            i = TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE;
            InitializeTitleVmAndSetSprite(this->resultTextAnm, &this->spellCardNameVms[i], i + 2);
            this->spellCardNameVms[i].pos = Float3(0, 0, 0);
            this->spellCardNameVms[i].anchor = 3;
            this->spellCardNameVms[i].fontWidth = 15;
            this->spellCardNameVms[i].fontHeight = 15;

            g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i], COLOR_TEXT_WHITE, 0, TH_TITLE_SPELL_CARD_INFO);

            this->spellCardNameVms[i].color1.a = 255;
            this->spellCardNameVms[i].color1.r = 255;
            this->spellCardNameVms[i].color1.g = 255;
            this->spellCardNameVms[i].color1.b = 255;

            i = this->cursor - (this->currentPageSpellCardSelect * TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE);
            this->spellCardNameVms[i].color1.r = 255;
            this->spellCardNameVms[i].color1.g = 255;
            this->spellCardNameVms[i].color1.b = 255;

            for (i = 0; i < 7; i++)
            {
                InitializeTitleVmAndSetSprite(g_Supervisor.textAnm, &this->spellCardInfoVms[i], i + 21);

                if (i < 4)
                {
                    this->spellCardInfoVms[i].pos = Float3(64.0f, (i * 16) + 344.0f, 0.0f);
                }
                else
                {
                    this->spellCardInfoVms[i].pos = Float3(64.0f, (i * 16) + 344.0f + 8.0f, 0.0f);
                }

                this->spellCardInfoVms[i].anchor = 3;
                this->spellCardInfoVms[i].fontWidth = 15;
                this->spellCardInfoVms[i].fontHeight = 15;
                this->spellCardInfoVms[i].color1.d3dColor = COLOR_WHITE;
            }

            this->FormatSpellCardInfo();
            this->spellCardInfoRevealCountdown = 0;
        }

        if (this->stateTimer2 == 8)
        {
            this->currentScreenState = TitleCurrentScreenState_Ready;
        }
        break;
    case TitleCurrentScreenState_Ready:
        oldPageIdx = this->currentPageSpellCardSelect;
        oldCursor = this->cursor;

        if (this->currentNumberOfSpellCards > TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE)
        {
            if (WAS_PRESSED_SCROLLING(TH_BUTTON_LEFT))
            {
                this->cursor -= TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE;

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                g_SoundPlayer.ProcessQueues();

                if (this->cursor < 0)
                {
                    this->cursor = this->currentNumberOfSpellCards - 1;
                }
                if (this->cursor >= this->currentNumberOfSpellCards)
                {
                    this->cursor = 0;
                }
            }
            if (WAS_PRESSED_SCROLLING(TH_BUTTON_RIGHT))
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);

                if (this->currentNumberOfSpellCards - this->cursor <=
                    this->currentNumberOfSpellCards % TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE)
                {
                    this->cursor %= TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE;
                }
                else
                {
                    this->cursor += TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE;
                    if (this->cursor < 0)
                    {
                        this->cursor = currentNumberOfSpellCards - 1;
                    }
                    if (this->cursor >= this->currentNumberOfSpellCards)
                    {
                        this->cursor = currentNumberOfSpellCards - 1;
                    }
                }
            }
        }

        this->MoveCursorVertical(this->currentNumberOfSpellCards);
        this->currentPageSpellCardSelect = this->cursor / TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE;

        if (oldPageIdx != this->currentPageSpellCardSelect)
        {
            for (i2 = 0; i2 < TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE; i2++)
            {
                if ((i2 + this->currentPageSpellCardSelect * TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE) >=
                    this->currentNumberOfSpellCards)
                {
                    break;
                }

                InitializeTitleVmAndSetSprite(this->resultTextAnm, &this->spellCardNameVms[i2], i2 + 2);
                this->spellCardNameVms[i2].pos = Float3(0, 0, 0);
                this->spellCardNameVms[i2].anchor = 3;
                /* Similar copy paste mistake as before? */
                this->spellCardInfoVms[0].fontWidth = 15;
                this->spellCardNameVms[i2].fontHeight = 15;

                spellCardNumber2 = g_SpellcardNumbersPerStage[g_GameManager.currentStage]
                                                             [i2 + this->currentPageSpellCardSelect *
                                                                       TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE];

                /* Why does ZUN use this helper method here, and in the initialization , use direct access? */
                if (g_GameManager.HasSpellCardBeenEncountered(spellCardNumber2, SHOT_ALL))
                {
                    g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i2], COLOR_TEXT_WHITE, 0,
                                               g_GameManager.catkData[spellCardNumber2].spellName);
                }
                else
                {
                    if (Spellcard::GetDifficultyFromSpellCard(spellCardNumber2) <= EXTRA ||
                        !g_GameManager.IsLastWordSpellCardAttempted(spellCardNumber2))
                    {
                        g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i2], COLOR_TEXT_WHITE, 0,
                                                   TH_TITLE_SPELLCARD_NOT_UNLOCKED);
                    }
                    else
                    {
                        g_AnmManager->DrawTextLeft(&this->spellCardNameVms[i2], COLOR_TEXT_WHITE, 0,
                                                   TH_TITLE_SPELLCARD_AVAILABLE);
                    }
                }

                this->spellCardNameVms[i2].color1.a = 255;
                this->spellCardNameVms[i2].color1.r = 96;
                this->spellCardNameVms[i2].color1.g = 96;
                this->spellCardNameVms[i2].color1.b = 96;
            }
        }

        if (oldCursor != this->cursor)
        {
            for (i2 = 0; i2 < TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE; i2++)
            {
                this->spellCardNameVms[i2].color1.r = 96;
                this->spellCardNameVms[i2].color1.g = 96;
                this->spellCardNameVms[i2].color1.b = 96;
            }

            i2 = this->cursor - (this->currentPageSpellCardSelect * TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE);
            this->spellCardNameVms[i2].color1.r = 255;
            this->spellCardNameVms[i2].color1.g = 255;
            this->spellCardNameVms[i2].color1.b = 255;

            for (i2 = 0; i2 < 7; i2++)
            {
                this->spellCardInfoVms[i2].color1.a = 0;
            }

            this->spellCardInfoRevealCountdown = 21;
        }

        this->FormatSpellCardInfo();

        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            spellCardNumber3 = g_SpellcardNumbersPerStage[g_GameManager.currentStage][this->cursor];
            if (g_GameManager.catkData[spellCardNumber3].inGameHistory.attempts[SHOT_ALL] != 0 ||
                g_GameManager.catkData[spellCardNumber3].spellPracticeHistory.attempts[SHOT_ALL] != 0 ||
                (spellCardNumber3 >= SPELLCARD_LAST_WORD_START &&
                 g_GameManager.IsLastWordSpellCardAttempted(spellCardNumber3)))
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

                g_GameManager.flags.isSpellPractice = TRUE;
                g_GameManager.currentSpellCardNumber =
                    g_SpellcardNumbersPerStage[g_GameManager.currentStage][this->cursor];

                g_GameManager.difficulty = Spellcard::GetDifficultyFromSpellCard(g_GameManager.currentSpellCardNumber);

                if (g_GameManager.difficulty > EXTRA)
                {
                    /* Set the correct difficulty for each Last Word spell card. */
                    switch (g_GameManager.currentSpellCardNumber)
                    {
                    case SPELLCARD_LW_WRIGGLE:
                        g_GameManager.currentStage = STAGE1;
                        break;
                    case SPELLCARD_LW_MYSTIA:
                        g_GameManager.currentStage = STAGE2;
                        break;
                    case SPELLCARD_LW_KEINE:
                        g_GameManager.currentStage = STAGE3;
                        break;
                    case SPELLCARD_LW_REISEN:
                        g_GameManager.currentStage = STAGE5;
                        break;
                    case SPELLCARD_LW_EIRIN:
                        g_GameManager.currentStage = STAGE6A;
                        break;
                    case SPELLCARD_LW_KAGUYA:
                        g_GameManager.currentStage = STAGE6B;
                        break;
                    case SPELLCARD_LW_MOKOU:
                        g_GameManager.currentStage = EXTRASTAGE;
                        break;
                    case SPELLCARD_LW_TEWI:
                        g_GameManager.currentStage = STAGE5;
                        break;
                    case SPELLCARD_LW_KEINEEX:
                        g_GameManager.currentStage = EXTRASTAGE;
                        break;
                    case SPELLCARD_LW_REIMU:
                        g_GameManager.currentStage = STAGE4A;
                        break;
                    case SPELLCARD_LW_MARISA:
                        g_GameManager.currentStage = STAGE4B;
                        break;
                    default: /* ... everyone else */
                        g_GameManager.currentStage = STAGE4A;
                        break;
                    }

                    g_GameManager.difficulty = NORMAL;
                }

                g_Supervisor.curState = SupervisorState_GameManager;
                g_GameManager.SetIsReplayWeird(FALSE);

                g_Supervisor.StopAudio();

                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            }
            else
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_INVALID_ACTION, 0);
            }
        }

        if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
        {
            g_GameManager.currentSpellCardNumber = g_SpellcardNumbersPerStage[g_GameManager.currentStage][this->cursor];

            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->cursor = g_GameManager.currentStage;

            this->ChangeCurrentScreen(TitleCurrentScreen_SpellStageSelect);

            return CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN;
        }

        break;
    }

    this->idleFrames++;
    this->stateTimer++;
    this->stateTimer2++;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}


// Probe-only recovery for the claimed Title lane. The three title VM
// initializations use the target-shaped inline helper so VC7 retains the
// per-inline work slot observed in the retail 0x88-byte frame.
#pragma var_order(menuLength1, vmIdx1, i1, horizontalCursorMovement, oldCursorPos, menuLength2, vmIdx2, i2, oldScreen)
ChainCallbackResult TitleScreen::OnUpdateSpellStageSelect()
{
    i32 menuLength1;
    i32 menuLength2;
    i32 vmIdx1;
    i32 vmIdx2;
    i32 i1;
    i32 i2;
    i32 oldCursorPos;
    i32 horizontalCursorMovement;
    TitleCurrentScreen oldScreen;

    switch (this->currentScreenState)
    {
    case TitleCurrentScreenState_Init:
        if (stateTimer2 == 0)
        {
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 18);

            this->vms[139].SetInterrupt(27);
            this->vms[138].SetInterrupt(27);

            if (this->previousScreen != TitleCurrentScreen_SpellCardSelect)
            {
                if (g_AnmManager->LoadSurface(0, "title/select00.png") != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
                }
            }

            this->cursor = g_GameManager.shotType;

            menuLength1 = g_GameManager.IsExtraUnlockedWithAllTeams() ? 12 : 4;
            while (!g_GameManager.IsSpellPracticeUnlockedForCharacter(this->cursor))
            {
                this->cursor++;
                if (this->cursor >= menuLength1)
                {
                    this->cursor -= menuLength1;
                }
            }

            g_GameManager.shotType = this->cursor;

            this->percentageCapturedSpellPracticePerShot = 0.0f;
            this->percentageCapturedInGamePerShot = 0.0f;
            this->percentageCapturedSpellPractice = 0.0f;
            this->percentageCapturedInGame = 0.0f;

            this->cursor = g_GameManager.currentStage;

            for (vmIdx1 = TITLE_SPRITE_CHARACTER_START; vmIdx1 <= TITLE_SPRITE_CHARACTER_END; vmIdx1++)
            {
                this->vms[vmIdx1].flag1 = FALSE;
                this->vms[vmIdx1].SetInterrupt(8);
                for (i1 = 1; i1 < ARRAY_SIZE(g_TitleCharacterSpriteIndices[0]) - 1; i1++)
                {
                    if (g_TitleCharacterSpriteIndices[g_GameManager.shotType][i1] == vmIdx1)
                    {
                        this->vms[vmIdx1].flag1 = TRUE;
                        this->vms[vmIdx1].SetInterrupt(9);
                    }
                }
                if (g_TitleCharacterSpriteIndices[g_GameManager.shotType][i1] == vmIdx1)
                {
                    this->vms[vmIdx1].flag1 = TRUE;
                    this->vms[vmIdx1].SetInterrupt(23);
                }
            }

            InitializeTitleVmAndSetSprite(this->resultTextAnm, &this->spellCardNameVms[0], 2);
            this->spellCardNameVms[0].pos = Float3(0, 0, 0);
            this->spellCardNameVms[0].anchor = 3;
            this->spellCardNameVms[0].fontWidth = 15;
            this->spellCardNameVms[0].fontHeight = 15;
            this->spellCardNameVms[0].color1.a = 255;
            this->spellCardNameVms[0].color1.r = 255;
            this->spellCardNameVms[0].color1.g = 255;
            this->spellCardNameVms[0].color1.b = 255;
            g_AnmManager->DrawTextLeft(&this->spellCardNameVms[0], COLOR_TEXT_WHITE, 0, TH_TITLE_SPELL_STAGE_INFO);

            InitializeTitleVmAndSetSprite(this->resultTextAnm, &this->spellCardNameVms[1], 3);
            this->spellCardNameVms[1].pos = Float3(0, 0, 0);
            this->spellCardNameVms[1].anchor = 3;
            this->spellCardNameVms[1].fontWidth = 15;
            this->spellCardNameVms[1].fontHeight = 15;
            this->spellCardNameVms[1].color1.a = 255;
            this->spellCardNameVms[1].color1.r = 255;
            this->spellCardNameVms[1].color1.g = 255;
            this->spellCardNameVms[1].color1.b = 255;
            g_AnmManager->DrawTextLeft(&this->spellCardNameVms[1], COLOR_TEXT_WHITE, 0,
                                       TH_TITLE_SPELL_CAPTURE_PERCENTAGE);

            /* ZUN bug: possible copy paste mistake? */
            InitializeTitleVmAndSetSprite(this->titleAnm, &this->spellCardNameVms[2], 144);
            this->spellCardNameVms[1].anchor = 3;
            this->spellCardNameVms[1].color1.a = 255;
            this->spellCardNameVms[1].color1.r = 255;
            this->spellCardNameVms[1].color1.g = 255;
            this->spellCardNameVms[1].color1.b = 255;

            this->currentScreenState = TitleCurrentScreenState_Init;
            this->stateTimer = 0;

            g_GameManager.flags.isPracticeMode = TRUE;
            g_GameManager.flags.isSpellPractice = TRUE;
        }

        if (this->practiceState != 0)
        {
            this->cursor = 0;
            this->ChangeCurrentScreen(TitleCurrentScreen_SpellCardSelect);
            if (g_GameManager.currentSpellCardNumber >= SPELLCARD_LAST_WORD_START)
            {
                g_GameManager.currentStage = STAGE_LAST_WORD;
            }
            return CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN;
        }

        if (stateTimer2 == 8)
        {
            this->currentScreenState = TitleCurrentScreenState_Ready;
        }
        break;
    case TitleCurrentScreenState_Ready:
        this->MoveCursorVertical(10);
        oldCursorPos = this->cursor;
        this->cursor = g_GameManager.shotType;

        menuLength2 = g_GameManager.IsExtraUnlockedWithAllTeams() ? 12 : 4;

        horizontalCursorMovement = this->MoveCursorHorizontal(menuLength2);
        if (horizontalCursorMovement != 0)
        {
            while (!g_GameManager.IsSpellPracticeUnlockedForCharacter(this->cursor))
            {
                this->cursor += horizontalCursorMovement;
                if (this->cursor >= menuLength2)
                {
                    this->cursor -= menuLength2;
                }
                if (this->cursor < 0)
                {
                    this->cursor += menuLength2;
                }
            }

            g_GameManager.shotType = this->cursor;

            for (vmIdx2 = TITLE_SPRITE_CHARACTER_START; vmIdx2 <= TITLE_SPRITE_CHARACTER_END; vmIdx2++)
            {
                this->vms[vmIdx2].flag1 = FALSE;
                this->vms[vmIdx2].SetInterrupt(8);
                for (i2 = 1; i2 < ARRAY_SIZE(g_TitleCharacterSpriteIndices[0]) - 1; i2++)
                {
                    if (g_TitleCharacterSpriteIndices[this->cursor][i2] == vmIdx2)
                    {
                        this->vms[vmIdx2].flag1 = TRUE;
                        this->vms[vmIdx2].SetInterrupt(9);
                    }
                }
                if (g_TitleCharacterSpriteIndices[this->cursor][i2] == vmIdx2)
                {
                    this->vms[vmIdx2].flag1 = TRUE;
                    this->vms[vmIdx2].SetInterrupt(23);
                }
            }

            this->stateTimer2 = 0;
        }

        this->cursor = oldCursorPos;

        if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            g_GameManager.currentStage = this->cursor;

            // The target enqueues the select sound a second time and flushes
            // the queue before entering spell-card selection.
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            g_SoundPlayer.ProcessQueues();

            this->cursor = 0;

            this->ChangeCurrentScreen(TitleCurrentScreen_SpellCardSelect);

            return CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN;
        }

        if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            g_SoundPlayer.ProcessQueues();

            this->currentScreenState = TitleCurrentScreenState_Changing;
            this->stateTimer = 0;

            g_GameManager.currentStage = this->cursor;
            g_AnmManager->SetInterruptArray(this->vms, this->vmCount, 6);
            break;
        }
        break;
    case TitleCurrentScreenState_Changing:
        if (this->stateTimer >= 20)
        {
            oldScreen = this->currentScreen;
            this->ChangeCurrentScreen(TitleCurrentScreen_StartMenu);
            this->cursor = 2;
            g_GameManager.flags.isPracticeMode = FALSE;
            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }
        break;
    }

    this->idleFrames++;
    this->stateTimer++;
    this->stateTimer2++;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#include "TitleFullWidthDigits.inl"


// Claim-safe owner for the Title dispatcher; compare through its associated
// switch data rather than inflating authored coverage.
ChainCallbackResult TitleScreen::OnUpdate(TitleScreen *titleScreen)
{
    ChainCallbackResult result;

    if (titleScreen->state != TitleScreenState_Ready)
    {
        if (titleScreen->state == TitleScreenState_Close)
        {
            return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
        }
        else
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }
    }

    switch (titleScreen->currentScreen)
    {
    case TitleCurrentScreen_StartMenu:
        result = titleScreen->OnUpdateStartMenu();
        break;
    case TitleCurrentScreen_Replay:
        result = titleScreen->OnUpdateReplayMenu();
        break;
    case TitleCurrentScreen_Option:
        result = titleScreen->OnUpdateOptions();
        break;
    case TitleCurrentScreen_KeyConfig:
        result = titleScreen->OnUpdateKeyConfig();
        break;
    case TitleCurrentScreen_DifficultySelect:
    case TitleCurrentScreen_DifficultySelectPractice:
    case TitleCurrentScreen_DifficultySelectExtra:
        result = titleScreen->OnUpdateDifficultySelect();
        break;
    case TitleCurrentScreen_CharacterSelect:
    case TitleCurrentScreen_CharacterSelectPractice:
    case TitleCurrentScreen_CharacterSelectExtra:
    case TitleCurrentScreen_CharacterSelectSpell:
        result = titleScreen->OnUpdateCharacterSelect();
        break;
    case TitleCurrentScreen_PracticeStageSelect:
        result = titleScreen->OnUpdatePracticeStageSelect();
        break;
    case TitleCurrentScreen_SpellStageSelect:
        result = titleScreen->OnUpdateSpellStageSelect();
        break;
    case TitleCurrentScreen_SpellCardSelect:
        result = titleScreen->OnUpdateSpellCardSelect();
        break;
    }

    g_AnmManager->ExecuteScriptArray(titleScreen->vms, titleScreen->vmCount);
    if (titleScreen->currentHelpTextVm != NULL)
    {
        g_AnmManager->ExecuteScript(titleScreen->currentHelpTextVm);
    }

    return result;
}



#include "TitleUnlockLastWords.inl"


#include "TitleFormatSpellCardInfo.inl"




#include "TitleCompletionStatus.inl"

struct TitlePieVertexPosition : Float3
{
    TitlePieVertexPosition &operator=(const Float3 &other)
    {
        *reinterpret_cast<D3DVECTOR *>(this) = *reinterpret_cast<const D3DVECTOR *>(&other);
        return *this;
    }
};

struct TitlePieVertex
{
    TitlePieVertexPosition pos;
    f32 w;
    D3DCOLOR diffuse;
};
C_ASSERT(sizeof(TitlePieVertex) == 0x14);

#pragma var_order(center, vm, vertices, i, angle)
void DrawPieChart(Float3 *position, D3DCOLOR color, f32 fraction, f32 diameter)
{
    TitlePieVertex vertices[64];
    Float3 center;
    AnmVm vm;
    f32 angle;
    i32 i;

    vm.blendMode = AnmBlendMode_Normal;
    vm.color1.d3dColor = COLOR_WHITE;
    vm.zWriteDisabled = TRUE;
    vm.flag15 = FALSE;

    vertices[0].diffuse = color;
    vertices[0].pos = *position;
    vertices[0].w = 1.0f;
    angle = -(ZUN_PI / 2.0f);
    center.x = diameter / 2.0f;
    center.y = 0.0f;

    for (i = 1; i < 64; i++)
    {
        Rotate(&vertices[i].pos, &center, angle);
        vertices[i].pos.x += vertices[0].pos.x;
        vertices[i].pos.y += vertices[0].pos.y;
        vertices[i].pos.z = vertices[0].pos.z;
        vertices[i].diffuse = color;
        vertices[i].w = 1.0f;
        angle = AddNormalizeAngle(angle, (ZUN_PI / 31.0f) * fraction);
    }
    g_AnmManager->DrawTriangleFan(&vm, reinterpret_cast<VertexDiffuseXyzrhw *>(vertices), 64);
}

} // namespace th08
