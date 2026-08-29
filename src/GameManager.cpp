#include "th_pch.h"

#include "GameManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EnemyManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "ScoreDat.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Spellcard.hpp"

namespace th08
{

struct RankParams
{
    i32 rank;
    i32 minRank;
    i32 maxRank;
};
C_ASSERT(sizeof(RankParams) == 0xc);

DIFFABLE_STATIC_ARRAY_ASSIGN(RankParams, 6, g_RankParams) = {
    {10, 8, 16},
    {10, 8, 16},
    {8, 8, 12},
    {8, 8, 12},
    {16, 15, 16},
    {16, 15, 16},
};

typedef i32 TimeRequirementRow[4];
DIFFABLE_STATIC_ARRAY_ASSIGN(TimeRequirementRow, MAX_STAGES, g_TimeRequirementParams) = {
    {2000, 2500, 2700, 3000},
    {6500, 7200, 7200, 7200},
    {7500, 8500, 8800, 8800},
    {9999, 9999, 9999, 9999},
    {7500, 8500, 8500, 8500},
    {9999, 9999, 9999, 9999},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
};

DIFFABLE_STATIC_ARRAY_ASSIGN(SpellcardMusicEntry, 19, g_SpellcardMusicInfo) = {
    {1, 1, "th08_00.mid", 0, FALSE},
    {12, 2, "th08_03.mid", 1, FALSE},
    {16, 3, "th08_04.mid", 0, FALSE},
    {31, 4, "th08_05.mid", 1, FALSE},
    {35, 5, "th08_06.mid", 0, FALSE},
    {53, 6, "th08_07.mid", 1, FALSE},
    {76, 8, "th08_09.mid", 1, FALSE},
    {99, 9, "th08_10.mid", 1, FALSE},
    {118, 11, "th08_12.mid", 1, FALSE},
    {122, 12, "th08_13.mid", 0, FALSE},
    {142, 13, "th08_14.mid", 1, FALSE},
    {146, 15, "th08_13b.mid", 2, TRUE},
    {150, 12, "th08_13.mid", 0, FALSE},
    {170, 14, "th08_15.mid", 1, FALSE},
    {190, 15, "th08_13b.mid", 2, TRUE},
    {193, 16, "th08_18.mid", 0, FALSE},
    {204, 17, "th08_19.mid", 1, FALSE},
    {222, 20, "th08_20.mid", 2, FALSE},
    {-1, 0, " ", 0, FALSE},
};

DIFFABLE_STATIC(GameManager, g_GameManager);
DIFFABLE_STATIC(ChainElem, g_GameManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_GameManagerDrawChain);

i32 InitializeScoreData();
extern i32 g_GuiMessageStageMode;

// FUNCTION: th08 0x439829
ZunBool GameManager::IsStageClearedWithoutRetries(i32 stage, i32 character, i32 difficulty)
{
    return IS_STAGE_CLEARED(this->clrdData[character].difficultiesClearedWithoutRetries[difficulty], stage);
}

// FUNCTION: th08 0x439856
ZunBool GameManager::IsStageClearedWithRetries(i32 stage, i32 character, i32 difficulty)
{
    return IS_STAGE_CLEARED(this->clrdData[character].difficultiesClearedWithRetries[difficulty], stage);
}

// FUNCTION: th08 0x439916
ZunBool GameManager::ShouldPauseMusicInSpellPractice(i32 unused)
{
    i32 i;

    i = 0;
    while (g_SpellcardMusicInfo[i].spellcardNumber >= 0)
    {
        if (static_cast<i32>(g_GameManager.currentSpellCardNumber) <=
            g_SpellcardMusicInfo[i].spellcardNumber)
        {
            return g_SpellcardMusicInfo[i].musicPausesInSpellPractice;
        }
        i++;
    }
    return 0;
}

// FUNCTION: th08 0x439961
i32 GameManager::GetSongNameSpriteIdx(i32 unused)
{
    i32 i;

    i = 0;
    while (g_SpellcardMusicInfo[i].spellcardNumber >= 0)
    {
        if (static_cast<i32>(g_GameManager.currentSpellCardNumber) <=
            g_SpellcardMusicInfo[i].spellcardNumber)
        {
            return g_SpellcardMusicInfo[i].songNameSpriteIdx;
        }
        i++;
    }
    return 0;
}

// FUNCTION: th08 0x4399ac
ZunBool GameManager::IsWithinPlayfield(f32 x, f32 y, f32 width, f32 height)
{
    if (width / 2.0f + x < 0.0f)
    {
        return FALSE;
    }
    if (x - width / 2.0f > 384.0f)
    {
        return FALSE;
    }
    if (height / 2.0f + y < 0.0f)
    {
        return FALSE;
    }
    if (y - height / 2.0f > 448.0f)
    {
        return FALSE;
    }

    return TRUE;
}

i32 GameManager::CalcAntiTamperChecksum()
{
    i32 sum;

    // There is zero chance ZUN actually used intptr_t here, but the codegen matches
    // and not making assumptions about pointer size is always nice
    sum = CalcChecksum((u8 *)&g_GameManager.globals->rng1,
                       (intptr_t)&globals->antiTamperValue - (intptr_t)&globals->rng1);
    sum += CalcChecksum((u8 *)&g_GameManager.globals->rng8, sizeof(g_GameManager.globals->rng8));
    sum += CalcChecksum((u8 *)g_GameManager.cfg, sizeof(GameConfiguration));
    sum += CalcChecksum((u8 *)&g_Supervisor.cfg, sizeof(GameConfiguration));
    sum += CalcChecksum((u8 *)&this->hscr, sizeof(Hscr));

    return sum;
}

i32 GameManager::CalcChecksum(u8 *address, i32 size)
{
    i32 sum;
    i32 i;

    for (sum = 0, i = 0; i < size; i++, address++)
    {
        sum += *address;
        g_GameManager.globals->antiTamperValue += g_GameManager.globals->rng8[2];
    }

    return sum;
}



#pragma optimize("t", on)
// FUNCTION: th08 0x421ba0
i32 GameManager::ScaleIntBasedOnRank(i32 upper, i32 lower)
{
    return this->rank * (lower - upper) / 32 + upper;
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::AddScore(i32 score)
{
    this->globals->score += score / 10;
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::AddTimeOrbs(i32 amount)
{
    if (amount >= 0 || this->globals->currentTimeOrbs >= -amount)
    {
        this->globals->currentTimeOrbs += amount;
        this->globals->totalTimeOrbs += amount;
        this->hscr.numTimeOrbsCollected += amount;
        this->UpdateAntiTamper();
        if (amount > 0)
        {
            amount = amount + (this->globals->totalTimeOrbs & 1);
            this->globals->pointItemValue += 10 * (amount / 2);
        }
    }
    else
    {
        this->globals->currentTimeOrbs = 0;
    }
}
#pragma optimize("", on)


#pragma optimize("t", on)
i32 GameManager::GetTimeOrbs()
{
    return this->globals->currentTimeOrbs;
}

i32 GameManager::GetLastSpellTimeOrbThreshold()
{
    return this->globals->lastSpellTimeOrbThreshold;
}

#pragma optimize("", on)

i32 GameManager::GetBombsRemaining()
{
    return this->globals->bombsRemaining;
}

#pragma optimize("t", on)
i32 GameManager::GetDeaths()
{
    return this->globals->deaths;
}

i32 GameManager::GetBombsUsed()
{
    return this->globals->bombsUsed;
}
#pragma optimize("", on)

void GameManager::CollectExtend()
{
    if ((i32)this->globals->livesRemaining < 8)
    {
        this->AddLives(1);
        g_SoundPlayer.PlaySoundByIdx(SOUND_1UP, 0);
        this->IncreaseSubrank(200);
        g_Gui.flags.lifeDisplayUpdateFrames = 2;
    }
    else if ((i32)this->globals->bombsRemaining < 8)
    {
        this->AddToBombCount(1);
        g_SoundPlayer.PlaySoundByIdx(SOUND_1UP, 0);
        this->IncreaseSubrank(200);
        g_Gui.flags.bombDisplayUpdateFrames = 2;
    }
}

// FUNCTION: th08 0x439bc7
#define GM_FLAGS_WORD(gm) (*reinterpret_cast<u32 *>(&(gm)->flags))

#pragma var_order(checksum, antiTamperIdx, value, stageIdx, stage, musicIdx, clockTime, anmManager)
ChainCallbackResult GameManager::OnUpdate(GameManager *gameManager)
{
    i32 checksum;
    AnmManager *anmManager;
    i8 clockTime;
    i32 musicIdx;
    i32 stage;
    i32 stageIdx;
    u32 value;
    u32 antiTamperIdx;

    g_GameManager.playtimeFrames++;

    if (gameManager->flags.stageTransitionState != 0)
    {
        if (gameManager->flags.stageTransitionState == 2)
        {
            GM_FLAGS_WORD(gameManager) |= 0x60;
            g_GameManager.gameplaySetupState = GAMEPLAY_SETUP_IN_PROGRESS;
            g_GameManager.nextSupervisorState = -1;

            if (!g_GameManager.flags.isReplay)
        {
            if (g_GameManager.globals->numRetries == 0)
            {
                g_GameManager.clrdData[g_GameManager.shotType].difficultiesClearedWithoutRetries[g_GameManager.difficulty] |=
                    g_GameManager.currentStageClearFlag;
                g_GameManager.clrdData[SHOT_ALL].difficultiesClearedWithoutRetries[g_GameManager.difficulty] |= g_GameManager.currentStageClearFlag;
            }
            g_GameManager.clrdData[g_GameManager.shotType].difficultiesClearedWithRetries[g_GameManager.difficulty] |=
                g_GameManager.currentStageClearFlag;
            g_GameManager.clrdData[SHOT_ALL].difficultiesClearedWithRetries[g_GameManager.difficulty] |= g_GameManager.currentStageClearFlag;
        }
        gameManager->globals->displayScore = gameManager->globals->score;

        if (gameManager->flags.isPracticeMode)
            {
                g_GameManager.globals->displayScore = g_GameManager.globals->score;
                g_GameManager.nextSupervisorState = 6;
                return CHAIN_CALLBACK_RESULT_BREAK;
            }

        if (g_GameManager.currentStage != STAGE6A && g_GameManager.currentStage != STAGE6B &&
            g_GameManager.currentStage != EXTRASTAGE)
        {
            if (g_GameManager.flags.isReplay)
            {
                stage = 0;
                for (stageIdx = g_GameManager.currentStage + 1; stageIdx < MAX_STAGES; stageIdx++)
                {
                    if (static_cast<ZunBool>(TH08_REPLAY_STAGE_DATA(g_ReplayManager->replayData, stageIdx) != NULL))
                    {
                        stage = stageIdx;
                        break;
                    }
                }
                if (stage == 0)
                    g_Supervisor.curState = SupervisorState_FinishReplay;
                else
                {
                    g_GameManager.currentStage = stage;
                    g_Supervisor.curState = SupervisorState_GameManagerReInit;
                }
            }
            else
            {
                clockTime = (i8)g_GameManager.globals->clockTime;
                if (clockTime >= 12)
                {
                    g_GameManager.flags.gameCleared = 0;
                    g_GameManager.nextSupervisorState = 9;
                    return CHAIN_CALLBACK_RESULT_BREAK;
                }
                g_GameManager.AdvanceToNextStage();
                g_Supervisor.curState = SupervisorState_GameManagerReInit;
            }
        }
        else if (g_GameManager.flags.isReplay)
        {
            g_GameManager.nextSupervisorState = 7;
        }
        else if (g_GameManager.difficulty >= 4)
        {
            if (g_GameManager.difficulty == 4)
            {
                g_GameManager.clrdData[g_GameManager.shotType].difficultiesClearedWithoutRetries[g_GameManager.difficulty] |= 0x8000;
                g_GameManager.clrdData[SHOT_ALL].difficultiesClearedWithRetries[g_GameManager.difficulty] |= 0x8000;
            }
            g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].clears++;
            g_GameManager.flags.gameCleared = 1;
            g_GameManager.globals->displayScore = g_GameManager.globals->score;
            g_GameManager.nextSupervisorState = 6;
            return CHAIN_CALLBACK_RESULT_BREAK;
        }
        else
        {
            g_GameManager.flags.gameCleared = 1;
            g_GameManager.nextSupervisorState = 9;
            return CHAIN_CALLBACK_RESULT_BREAK;
        }

            if (g_GameManager.nextSupervisorState < 0)
                g_Gui.CaptureArcade();
        }

        if ((((g_CurFrameInput & 0x1001) != 0) && ((g_CurFrameInput & 0x1001) != (g_LastFrameInput & 0x1001))) ||
            g_GameManager.flags.isReplay || g_GameManager.currentStage == STAGE6A ||
            g_GameManager.currentStage == STAGE6B || g_GameManager.currentStage == EXTRASTAGE)
        {
            GM_FLAGS_WORD(gameManager) &= ~0x60U;
            if (g_GameManager.nextSupervisorState >= 0)
                g_Supervisor.curState = g_GameManager.nextSupervisorState;
        }
    }

    if (gameManager->gameplaySetupState != 0)
    {
        if (gameManager->gameplaySetupState == GAMEPLAY_SETUP_FAILED)
            return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
        gameManager->gameplaySetupWaitFrames++;
        return CHAIN_CALLBACK_RESULT_BREAK;
    }

    if (gameManager->stageStartupMode != 0)
    {
        Gui::CopyCurrentStageEnemyNameTexture();
        g_AnmManager->ReleaseSurface(8);
        g_Supervisor.loadingVmsHaveBeenSetup = 0;
        if (gameManager->stageStartupMode == STAGE_STARTUP_PLAY_MUSIC)
        {
            if (!g_GameManager.flags.isSpellPractice)
            {
                g_Supervisor.PlayMusic(
                    0,
                    g_GuiStageMusicContexts[g_GameManager.currentStage].songNumbers[0]);
            }
            else
            {
                musicIdx = 0;
                while (g_SpellcardMusicInfo[musicIdx].spellcardNumber >= 0)
                {
                    if (g_GameManager.currentSpellCardNumber <= g_SpellcardMusicInfo[musicIdx].spellcardNumber)
                    {
                        g_Supervisor.PlayMusic(
                            0, g_SpellcardMusicInfo[musicIdx].songNumber);
                        break;
                    }
                    musicIdx++;
                }
            }
        }
        gameManager->stageStartupMode = STAGE_STARTUP_NONE;
    }

    if (!gameManager->showRetryMenu && !gameManager->isInGameMenu && !gameManager->flags.isDemoMode &&
        !gameManager->skipCurrentFrame && (g_CurFrameInput & TH_BUTTON_MENU) &&
        (g_CurFrameInput & TH_BUTTON_MENU) != (g_LastFrameInput & TH_BUTTON_MENU))
    {
        gameManager->isInGameMenu = 1;
        g_GameManager.arcadeRegionTopLeftPos.x = 32.0f;
        g_GameManager.arcadeRegionTopLeftPos.y = 16.0f;
        g_GameManager.arcadeRegionSize.x = 384.0f;
        g_GameManager.arcadeRegionSize.y = 448.0f;
        gameManager->replayPauseRecorded = 1;
        g_SoundPlayer.QueueCommand(SOUNDPLAYER_COMMAND_PAUSE, 0, "Pause");
        g_SoundPlayer.PlaySoundByIdx(SOUND_PAUSE, 0);
        g_Supervisor.UpdateGameTime();
        g_Rng.SaveSeed();
        gameManager->hscr.numPauses++;
        g_GameManager.UpdateAntiTamper();
        g_Rng.RestoreSavedSeed();
    }

    g_Supervisor.viewport.X = (i32)gameManager->arcadeRegionTopLeftPos.x;
    g_Supervisor.viewport.Y = (i32)gameManager->arcadeRegionTopLeftPos.y;
    g_Supervisor.viewport.Width = (i32)gameManager->arcadeRegionSize.x;
    g_Supervisor.viewport.Height = (i32)gameManager->arcadeRegionSize.y;
    g_Supervisor.viewport.MinZ = 0.0f;
    g_Supervisor.viewport.MaxZ = 1.0f;
    anmManager = g_AnmManager;
    anmManager->cameraMode |= AnmCameraMode_Unset;

    if (g_GameManager.flags.isReplay && g_GameManager.replayMode == 1 && !g_Gui.IsDialoguePresent())
    {
        gameManager->frameSkipCounter++;
        if ((g_Supervisor.recordedFps < 20 && gameManager->frameSkipCounter % 3 != 0) ||
            (g_Supervisor.recordedFps >= 20 && g_Supervisor.recordedFps < 30 &&
             gameManager->frameSkipCounter % 2 != 0) ||
            (g_Supervisor.recordedFps >= 30 && g_Supervisor.recordedFps < 40 &&
             gameManager->frameSkipCounter % 3 == 0) ||
            (g_Supervisor.recordedFps >= 40 && g_Supervisor.recordedFps < 50 &&
             gameManager->frameSkipCounter % 6 == 0))
            return CHAIN_CALLBACK_RESULT_BREAK;
    }

    if (gameManager->flags.isDemoMode)
    {
        if ((g_CurFrameInput & TH_BUTTON_ANY) != 0 &&
            (g_CurFrameInput & TH_BUTTON_ANY) != (g_LastFrameInput & TH_BUTTON_ANY))
            g_Supervisor.curState = SupervisorState_TitleScreen;
        gameManager->demoFrameCount++;
        if ((gameManager->currentDemoReplay == 0 && gameManager->demoFrameCount == 6000) ||
            (gameManager->currentDemoReplay == 1 && gameManager->demoFrameCount == 4800) ||
            (gameManager->currentDemoReplay == 2 && gameManager->demoFrameCount == 4920) ||
            (gameManager->currentDemoReplay == 3 && gameManager->demoFrameCount == 6900))
        {
            ScreenEffect::RegisterChain(
                SCREEN_EFFECT_ARCADE_FADE_OUT, 120, 0, 0, 0, CHAIN_PRIO_DRAW_SCREENEFFECT);
            g_Supervisor.FadeOutMusic(3.0f);
        }
        if ((gameManager->currentDemoReplay == 0 && gameManager->demoFrameCount >= 6120) ||
            (gameManager->currentDemoReplay == 1 && gameManager->demoFrameCount >= 4920) ||
            (gameManager->currentDemoReplay == 2 && gameManager->demoFrameCount >= 5040) ||
            (gameManager->currentDemoReplay == 3 && gameManager->demoFrameCount == 7020))
        {
            g_Supervisor.curState = SupervisorState_TitleScreen;
            return CHAIN_CALLBACK_RESULT_BREAK;
        }
    }

    g_GameManager.globals->antiTamperValue = g_GameManager.globals->rng1[2];
    checksum = gameManager->CalcAntiTamperChecksum();
    g_GameManager.antiTamperExpectedValue = (f32)checksum + g_GameManager.globals->rng7[3];

    for (antiTamperIdx = 0; antiTamperIdx < 7; ++antiTamperIdx)
        if (gameManager->globals->rng1[antiTamperIdx] < 6543 || gameManager->globals->rng1[antiTamperIdx] > 106543)
            g_GameManager.antiTamperExpectedValue = -9999.0f;
    for (antiTamperIdx = 0; antiTamperIdx < 2; ++antiTamperIdx)
        if (gameManager->globals->rng3[antiTamperIdx] < 6543.0f || gameManager->globals->rng3[antiTamperIdx] > 106543.0f)
            g_GameManager.antiTamperExpectedValue = -9999.0f;

    gameManager->flags.replayInputEnabled = !gameManager->showRetryMenu && !gameManager->isInGameMenu;

    for (antiTamperIdx = 0; antiTamperIdx < 2; ++antiTamperIdx)
        if (gameManager->globals->rng2[antiTamperIdx] < 6543.0f || gameManager->globals->rng2[antiTamperIdx] > 106543.0f)
            g_GameManager.antiTamperExpectedValue = -9999.0f;
    for (antiTamperIdx = 0; antiTamperIdx < 8; ++antiTamperIdx)
        if (gameManager->globals->rng7[antiTamperIdx] < 6543 || gameManager->globals->rng7[antiTamperIdx] > 106543)
            g_GameManager.antiTamperExpectedValue = -9999.0f;

    g_Supervisor.d3dDevice->Clear(
        0, NULL, D3DCLEAR_ZBUFFER, g_Background.skyFog.color.d3dColor, 1.0f, 0);

    if (gameManager->isInGameMenu == 1 || gameManager->isInGameMenu == 2 || gameManager->showRetryMenu)
        return CHAIN_CALLBACK_RESULT_BREAK;

    if (gameManager->globals->score >= 1000000000U)
        gameManager->globals->score = 999999999U;

    if (gameManager->globals->displayScore != gameManager->globals->score)
    {
        if (gameManager->globals->score < gameManager->globals->displayScore)
            gameManager->globals->score = gameManager->globals->displayScore;
        value = (gameManager->globals->score - gameManager->globals->displayScore) >> 5;
        if (value >= 578910)
            value = 578910;
        else if (value == 0)
            value = 1;
        if (gameManager->globals->scoreDisplayStep < value)
            gameManager->globals->scoreDisplayStep = value;
        if (gameManager->globals->displayScore + gameManager->globals->scoreDisplayStep > gameManager->globals->score)
            gameManager->globals->scoreDisplayStep = gameManager->globals->score - gameManager->globals->displayScore;
        gameManager->globals->displayScore += gameManager->globals->scoreDisplayStep;
        if (gameManager->globals->displayScore >= gameManager->globals->score)
        {
            gameManager->globals->scoreDisplayStep = 0;
            gameManager->globals->displayScore = gameManager->globals->score;
        }
        if (gameManager->globals->displayedHighScore < gameManager->globals->displayScore)
        {
            gameManager->globals->displayedHighScore = gameManager->globals->displayScore;
            gameManager->globals->continuesUsedInHighScore = gameManager->globals->numRetries;
        }
    }

    for (antiTamperIdx = 0; antiTamperIdx < 3; ++antiTamperIdx)
        if (gameManager->globals->rng4[antiTamperIdx] < 6543.0f || gameManager->globals->rng4[antiTamperIdx] > 106543.0f)
            g_GameManager.antiTamperExpectedValue = -9999.0f;
    for (antiTamperIdx = 0; antiTamperIdx < 2; ++antiTamperIdx)
        if (gameManager->globals->rng5[antiTamperIdx] < 6543.0f || gameManager->globals->rng5[antiTamperIdx] > 106543.0f)
            g_GameManager.antiTamperExpectedValue = -9999.0f;
    for (antiTamperIdx = 0; antiTamperIdx < 5; ++antiTamperIdx)
        if (gameManager->globals->rng8[antiTamperIdx] < 6543 || gameManager->globals->rng8[antiTamperIdx] > 106543)
            g_GameManager.antiTamperExpectedValue = -9999.0f;

    if (g_GameManager.cfg->slowMode)
    {
        g_GameManager.skipCurrentFrame = 0;
        gameManager->frameSkipCounter++;
        if ((g_BulletManager.activeBulletCount >= 320 && gameManager->frameSkipCounter % 3 == 0) ||
            (g_BulletManager.activeBulletCount < 320 && g_BulletManager.activeBulletCount >= 224 &&
             gameManager->frameSkipCounter % 4 == 0) ||
            (g_BulletManager.activeBulletCount < 224 && g_BulletManager.activeBulletCount >= 128 &&
             gameManager->frameSkipCounter % 5 == 0))
        {
            g_GameManager.skipCurrentFrame = 1;
            return CHAIN_CALLBACK_RESULT_BREAK;
        }
        if (g_BulletManager.activeBulletCount < 128)
            gameManager->frameSkipCounter = 0;
    }

    g_GameManager.IsTampered();
    gameManager->gameplayFrameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#undef GM_FLAGS_WORD

// FUNCTION: th08 0x43aa03
ChainCallbackResult GameManager::OnDraw(GameManager *gameManager)
{
    if (gameManager->isInGameMenu)
    {
        gameManager->isInGameMenu = 2;
    }
    if (g_Supervisor.curState != SupervisorState_GameManager)
    {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }
    if (gameManager->flags.stageTransitionState == 1)
    {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }
    if (gameManager->gameplaySetupState)
    {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x43aa5c
ZunResult GameManager::RegisterChain()
{
    GameManager *mgr = &g_GameManager;

    g_GameManagerCalcChain.callback = (ChainCallback)GameManager::OnUpdate;
    g_GameManagerCalcChain.addedCallback = NULL;
    g_GameManagerCalcChain.deletedCallback = NULL;
    g_GameManagerCalcChain.addedCallback = (ChainLifetimeCallback)GameManager::AddedCallback;
    g_GameManagerCalcChain.deletedCallback = (ChainLifetimeCallback)GameManager::DeletedCallback;
    g_GameManagerCalcChain.arg = mgr;
    mgr->gameplayFrameCounter = 0;
    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChain, CHAIN_PRIO_CALC_GAMEMANAGER))
    {
        return ZUN_ERROR;
    }

    g_GameManagerDrawChain.callback = (ChainCallback)GameManager::OnDraw;
    g_GameManagerDrawChain.addedCallback = NULL;
    g_GameManagerDrawChain.deletedCallback = NULL;
    g_GameManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChain, CHAIN_PRIO_DRAW_GAMEMANAGER);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x43aaf4
ZunResult GameManager::AddedCallback(GameManager *gameManager)
{
    if (g_Supervisor.curState != SupervisorState_GameManagerReInit &&
        g_Supervisor.curState != SupervisorState_SpellcardPracticeRestart &&
        g_Supervisor.curState != SupervisorState_GameManagerNextStageWeird)
    {
        g_Supervisor.isInitialStageLoad = TRUE;
    }
    else
    {
        g_Supervisor.isInitialStageLoad = FALSE;
    }
    g_GameManager.gameplaySetupState = GAMEPLAY_SETUP_IN_PROGRESS;

    if (g_Supervisor.wantedState2 == SupervisorState_TitleScreen)
    {
        Float3 position;

        position.x = 500.0f;
        position.y = 440.0f;
        position.z = 0.0f;
        g_Supervisor.SetupLoadingVmsAndInitCapture(&position);
        g_Supervisor.StartEffect(0);
    }
    else
    {
        Float3 position;

        position.x = 280.0f;
        position.y = 430.0f;
        position.z = 0.0f;
        g_Supervisor.SetupLoadingVmsAndInitCapture(&position);
    }

    if (gameManager->flags.stageTransitionState >= 2)
    {
        gameManager->flags.stageTransitionState = 1;
    }
    g_Supervisor.ThreadStart((LPTHREAD_START_ROUTINE)GameManager::GameplaySetupThread, NULL);
    return ZUN_SUCCESS;
}

#define GM_FLAGS_WORD(gm) (*reinterpret_cast<u32 *>(&(gm)->flags))

struct SetupPlayCountTable
{
    u8 prefix[0x2C];
    PlstPlayCounts counts[MAX_DIFFICULTIES + 2];
};
C_ASSERT(offsetof(SetupPlayCountTable, counts) == 0x2C);

// FUNCTION: th08 0x43abd7
#pragma var_order(gameManager, size, replaySeed, i, oldCfg, oldGlobals, newCfg, newGlobals, allocation, stageMode, configMode)
void __fastcall GameManager::GameplaySetupThread(void *unused)
{
    GameManager *gameManager;
    u32 size;
    u16 replaySeed;
    i32 i;
    GameConfiguration *oldCfg;
    ZunGlobals *oldGlobals;
    GameConfiguration *newCfg;
    ZunGlobals *newGlobals;
    void *allocation;
    i32 stageMode;
    i32 configMode;

    gameManager = &g_GameManager;
    gameManager->gameplaySetupWaitFrames = 0;
    g_Supervisor.systemTime = timeGetTime();

    gameManager->currentStageClearFlag = static_cast<u16>(1 << gameManager->currentStage);
    gameManager->stageAtStart = gameManager->currentStage;
    if (gameManager->difficulty < 4)
        gameManager->difficultyMask = 1 << gameManager->difficulty;
    else
        gameManager->difficultyMask = 0xf;

    gameManager->characterListIndex = gameManager->shotType + gameManager->fullShotType;
    g_Supervisor.framerateMultiplier = 1.0f;
    GM_FLAGS_WORD(gameManager) &= ~0x400U;

    if (g_Supervisor.isInitialStageLoad || gameManager->flags.isSpellPractice ||
        g_GameManager.flags.isPracticeMode || g_GameManager.difficulty >= 4)
    {
        if (gameManager->cfg)
        {
            oldCfg = gameManager->cfg;
            operator delete(oldCfg);
            gameManager->cfg = NULL;
        }
        if (gameManager->globals)
        {
            oldGlobals = gameManager->globals;
            operator delete(oldGlobals);
            gameManager->globals = NULL;
        }

        size = g_Rng.GetRandomU32InRange(0xffff) + 16;
#ifdef TH08_PORTABLE_NATIVE_LAYOUT
        gameManager->antiTamperHeapJitterAllocation = malloc(size);
#else
        gameManager->antiTamperHeapJitterAllocation = reinterpret_cast<i32>(malloc(size));
#endif
        newCfg = static_cast<GameConfiguration *>(operator new(sizeof(GameConfiguration)));
        gameManager->cfg = newCfg;
        newGlobals = static_cast<ZunGlobals *>(operator new(sizeof(ZunGlobals)));
        gameManager->globals = newGlobals;
        GameManager::InitializeAntiTamper();
        *gameManager->cfg = g_Supervisor.cfg;
        allocation = reinterpret_cast<void *>(gameManager->antiTamperHeapJitterAllocation);
        free(allocation);

        gameManager->character = 0;
        gameManager->globals->youkaiGauge = 0;
        if (g_GameManager.currentStage == 8)
            stageMode = 6;
        else
            stageMode = 0;
        gameManager->globals->clockTime = static_cast<u8>(stageMode);
        if (g_GameManager.difficulty >= 4)
            gameManager->cfg->lifeCount = 2;
        if (g_GameManager.flags.isPracticeMode)
            gameManager->cfg->lifeCount = 8;

        if (Player::RegisterChain(0))
        {
            if (g_Supervisor.subthreadCloseRequestActive)
                goto thread_done;
            g_GameErrorContext.Log("error: player initialization failed\n");
            goto setup_error;
        }

        if (!g_GameManager.flags.isReplay)
        {
            configMode = gameManager->cfg->lifeCount;
            g_GameManager.globals->livesRemaining = static_cast<f32>(configMode);
            g_GameManager.UpdateAntiTamper();
            g_GameManager.SetBombCount(static_cast<i32>(
                g_Player.primaryShtFile->initialBombCount));
        }

        gameManager->InitArcadeRegionParams();
        gameManager->globals->playerPower = 0.0f;
        gameManager->UpdateAntiTamper();
        gameManager->stagePlayTimeAll = 0;
        // The target deliberately uses the global owner here instead of the
        // cached local used by adjacent setup fields.
        g_GameManager.humanityRateDenominator = 0;
        g_GameManager.humanityRateNumerator = 0;
        gameManager->globals->displayScore = 0;
        gameManager->globals->score = 0;
        gameManager->globals->scoreDisplayStep = 0;
        gameManager->globals->displayedHighScore = 100000;
        gameManager->globals->numRetries = 0;
        gameManager->globals->graze = 0;
        gameManager->globals->pointItemsCollected = 0;

        if (gameManager->difficulty >= 4 || gameManager->flags.isPracticeMode ||
            gameManager->flags.isSpellPractice)
            gameManager->cfg->slowMode = 0;

        switch (g_GameManager.difficulty)
        {
        case 0:
            gameManager->globals->pointItemValue = 60000;
            break;
        case 1:
            gameManager->globals->pointItemValue = 100000;
            break;
        case 2:
            gameManager->globals->pointItemValue = 200000;
            break;
        case 3:
            gameManager->globals->pointItemValue = 300000;
            break;
        case 4:
            gameManager->globals->pointItemValue = 300000;
            break;
        }
        gameManager->globals->pointItemExtendsSoFar = 0;
        ItemManager::UpdatePointItemExtendThreshold();
        if (InitializeScoreData())
            goto setup_error;

        gameManager->InitRankParams();
        gameManager->globals->deaths = 0.0f;
        gameManager->globals->deathInStage = 0.0f;
        gameManager->UpdateAntiTamper();
        gameManager->globals->bombsUsed = 0.0f;
        gameManager->globals->bombsUsedInStage = 0.0f;
        gameManager->UpdateAntiTamper();
        gameManager->globals->spellcardsCaptured = 0;
        gameManager->runActiveFrames = 0;
        gameManager->runExtremeYoukaiFrames = 0;
        gameManager->runExtremeHumanFrames = 0;

        if (!g_GameManager.flags.isReplay && !g_GameManager.flags.isSpellPractice)
        {
            if (!gameManager->cfg->slowMode)
            {
                IncrementTruncate(&g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].attemptsTotal, 999999);
                IncrementTruncate(&reinterpret_cast<SetupPlayCountTable *>(&g_GameManager.plst)->counts[MAX_DIFFICULTIES + 1].attemptsTotal, 999999);
                IncrementTruncate(reinterpret_cast<u32 *>(&g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].attemptsPerCharacter[gameManager->shotType]), 999999);
                IncrementTruncate(reinterpret_cast<u32 *>(&g_GameManager.plst.playDataTotals.attemptsPerCharacter[gameManager->shotType]), 999999);
                if (g_Supervisor.curState == SupervisorState_GameManagerRestartFromBeginning)
                {
                    IncrementTruncate(&g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].restarts, 999999);
                    IncrementTruncate(&reinterpret_cast<SetupPlayCountTable *>(&g_GameManager.plst)->counts[MAX_DIFFICULTIES + 1].restarts, 999999);
                }
                if (g_GameManager.flags.isPracticeMode && !g_GameManager.flags.isSpellPractice)
                {
                    IncrementTruncate(reinterpret_cast<u32 *>(&g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].practices), 999999);
                    IncrementTruncate(reinterpret_cast<u32 *>(&reinterpret_cast<SetupPlayCountTable *>(&g_GameManager.plst)->counts[MAX_DIFFICULTIES + 1].practices), 999999);
                }
            }
        }
        else
        {
            gameManager->cfg->slowMode = 0;
        }
    }
    else
    {
        gameManager->globals->displayScore = gameManager->globals->score;
        gameManager->globals->scoreDisplayStep = 0;
        gameManager->globals->deathInStage = 0.0f;
        gameManager->UpdateAntiTamper();
        gameManager->globals->bombsUsedInStage = 0.0f;
        gameManager->UpdateAntiTamper();
        if (Player::RegisterChain(0))
        {
            if (g_Supervisor.subthreadCloseRequestActive)
                goto thread_done;
            g_GameErrorContext.Log("error: player initialization failed\n");
            goto setup_error;
        }
    }

    gameManager->subRank = 0;
    gameManager->globals->pointItemsCollectedInStage = 0;
    gameManager->globals->grazeInStage = 0;
    gameManager->isInGameMenu = 0;
    GM_FLAGS_WORD(gameManager) &= ~0x180U;
    GM_FLAGS_WORD(gameManager) &= ~0x2000U;
    gameManager->stageActiveFrames = 0;
    gameManager->stageExtremeYoukaiFrames = 0;
    gameManager->stageExtremeHumanFrames = 0;
    gameManager->globals->youkaiGaugeCopy = gameManager->globals->youkaiGauge;
    gameManager->globals->currentTimeOrbs = 0;
    gameManager->globals->totalTimeOrbs = 0;
    if (!g_GameManager.flags.isSpellPractice)
        gameManager->globals->lastSpellTimeOrbThreshold = g_TimeRequirementParams[gameManager->currentStage][g_GameManager.difficulty];
    else
        gameManager->globals->lastSpellTimeOrbThreshold = 0;

    if (gameManager->flags.isPracticeMode)
    {
        if (!gameManager->flags.isSpellPractice)
        {
            switch (gameManager->currentStage)
            {
            case 0:
                gameManager->globals->playerPower = 0.0f;
                gameManager->UpdateAntiTamper();
                break;
            case 1:
                gameManager->globals->playerPower = 112.0f;
                gameManager->UpdateAntiTamper();
                break;
            default:
                gameManager->globals->playerPower = 128.0f;
                gameManager->UpdateAntiTamper();
                break;
            }
        }
        else
        {
            if (gameManager->currentSpellCardNumber <= 1)
            {
                gameManager->globals->playerPower = 30.0f;
                gameManager->UpdateAntiTamper();
            }
            else if (gameManager->currentSpellCardNumber <= 12)
            {
                gameManager->globals->playerPower = 80.0f;
                gameManager->UpdateAntiTamper();
            }
            else
            {
                gameManager->globals->playerPower = 128.0f;
                gameManager->UpdateAntiTamper();
            }
        }
    }

    if (g_GameManager.flags.isReplay)
    {
        gameManager->InitRankParams();
        ReplayManager::RegisterChain(1, g_GameManager.replayFilename);
        replaySeed = *reinterpret_cast<u16 *>(&g_Rng);
        gameManager->UpdateAntiTamper();
        *reinterpret_cast<u16 *>(&g_Rng) = replaySeed;
    }
    gameManager->stageRngSeed = *reinterpret_cast<u16 *>(&g_Rng);

    if (Background::RegisterChain(gameManager->currentStage))
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log("error: background initialization failed\n");
        goto setup_error;
    }
    if (BulletManager::RegisterChain(const_cast<char *>("etama.anm")))
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log("error: bullet initialization failed\n");
        goto setup_error;
    }
    if (EnemyManager::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log("error: enemy initialization failed\n");
        goto setup_error;
    }
    if (EffectManager::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log("error: effect initialization failed\n");
        goto setup_error;
    }
    if (Gui::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log("error: 2D initialization failed\n");
        goto setup_error;
    }
    if (Spellcard::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log("error: spell card initialization failed\n");
        goto setup_error;
    }

    if (!g_GameManager.flags.isReplay)
        ReplayManager::RegisterChain(0, "replay/th8_00.rpy");

    if (g_GameManager.flags.isSpellPractice)
    {
        switch (g_GameManager.currentStage)
        {
        case 5:
            g_GameManager.IsSpellNumberEqualTo(212);
            break;
        case 6:
            if (!g_GameManager.IsSpellNumberInRange(119, 122))
                g_GuiMessageStageMode = 2;
            break;
        case 7:
            if (!g_GameManager.IsSpellNumberInRange(147, 150))
                g_GuiMessageStageMode = 2;
            break;
        case 8:
            if (!g_GameManager.IsSpellNumberInRange(191, 193) &&
                !g_GameManager.IsSpellNumberEqualTo(213))
                g_GuiMessageStageMode = 2;
            break;
        }
    }

    if (!g_Supervisor.keepStageResources)
    {
        if (g_GameManager.flags.isSpellPractice)
        {
            i = 0;
            while (g_SpellcardMusicInfo[i].spellcardNumber >= 0)
            {
                if (g_GameManager.currentSpellCardNumber <= g_SpellcardMusicInfo[i].spellcardNumber)
                {
                    g_Supervisor.LoadMusic(0, const_cast<char *>(g_SpellcardMusicInfo[i].songPath));
                    break;
                }
                ++i;
            }
        }
        else
        {
            g_Supervisor.LoadMusic(0, g_Background.stageData->songPaths[0]);
            if (g_Background.stageData->songPaths[1][0] != 0x20)
                g_Supervisor.LoadMusic(1, g_Background.stageData->songPaths[1]);
            if (g_Background.stageData->songPaths[2][0] != 0x20)
                g_Supervisor.LoadMusic(2, g_Background.stageData->songPaths[2]);
        }
    }

    gameManager->showRetryMenu = 0;
    GM_FLAGS_WORD(gameManager) |= 4U;
    if (g_Supervisor.keepStageResources && g_GameManager.flags.isSpellPractice &&
        !GameManager::ShouldPauseMusicInSpellPractice(g_GameManager.currentSpellCardNumber))
        gameManager->stageStartupMode = STAGE_STARTUP_WITHOUT_MUSIC;
    else
        gameManager->stageStartupMode = STAGE_STARTUP_PLAY_MUSIC;

    if (g_Supervisor.curState != SupervisorState_GameManagerReInit)
    {
        g_Supervisor.lagNumerator = 0.0f;
        g_Supervisor.lagDenominator = 0.0f;
    }

    gameManager->scriptedUpdateFreeze = 0;
    gameManager->globals->score = 0;
    GM_FLAGS_WORD(gameManager) &= ~0x10U;
    g_AsciiManager.Reset();
    g_AsciiManager.InitializeVms();
    g_GameManager.skipCurrentFrame = 0;
    g_AsciiManager.nightBlindnessAlpha = 0;
    Supervisor::CalculateFps(0);

    if (g_GameManager.flags.isReplay)
    {
        while (gameManager->gameplaySetupWaitFrames < 80)
            Sleep(17);
    }
    else
    {
        while (gameManager->gameplaySetupWaitFrames < 30)
            Sleep(17);
    }

    g_Supervisor.BeginLoadingCompletion();
    while (gameManager->flags.stageTransitionState != 0)
        Sleep(17);

    g_GameManager.gameplaySetupState = GAMEPLAY_SETUP_COMPLETE;
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.subthreadActive = FALSE;
    g_Supervisor.screenTransitionCountdown = 60;
    GM_FLAGS_WORD(gameManager) &= ~0x200U;
    g_Supervisor.keepStageResources = 0;
    g_ScreenEffectCounter = 2;
    goto thread_done;

setup_error:
    g_GameManager.gameplaySetupState = GAMEPLAY_SETUP_FAILED;
    g_Supervisor.BeginLoadingCompletion();
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.subthreadActive = FALSE;
    g_Supervisor.keepStageResources = 0;
    g_ScreenEffectCounter = 2;

thread_done:
    (void)unused;
}

#undef GM_FLAGS_WORD

// FUNCTION: th08 0x43b936
void GameManager::InitRankParams()
{
    this->rank = g_RankParams[g_GameManager.difficulty].rank;
    this->minRank = g_RankParams[g_GameManager.difficulty].minRank;
    this->maxRank = g_RankParams[g_GameManager.difficulty].maxRank;
}

#pragma var_order(sum, i)
void GameManager::InitializeAntiTamper()
{
    i32 sum;
    u32 i;

    g_GameManager.globals->rng6 = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng1); i++)
    {
        g_GameManager.globals->rng1[i] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng7); i++)
    {
        g_GameManager.globals->rng7[i] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng2); i++)
    {
        g_GameManager.globals->rng2[i] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng3); i++)
    {
        g_GameManager.globals->rng3[i] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng4); i++)
    {
        g_GameManager.globals->rng4[i] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng5); i++)
    {
        g_GameManager.globals->rng5[i] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    for (i = 0; i < ARRAY_SIZE(g_GameManager.globals->rng8); i++)
    {
        g_GameManager.globals->rng8[i] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }
    g_GameManager.globals->antiTamperValue = g_GameManager.globals->rng1[2];
    sum = g_GameManager.CalcAntiTamperChecksum();
    g_GameManager.globals->antiTamperChecksum = sum;
    g_GameManager.antiTamperExpectedValue = (f32)sum + (f32)g_GameManager.globals->rng7[3];
}

// FUNCTION: th08 0x43bbe1
#pragma var_order(catk, i, scoreDat, j)
i32 InitializeScoreData()
{
    Catk *catk;
    i32 i;
    ScoreDat *scoreDat;
    i32 j;

    catk = g_GameManager.catkData;
    ResultScreen::RegisterChain(RESULT_SCREEN_REGISTER_SAVE_DATA);
    memset(g_GameManager.catkData, 0, sizeof(g_GameManager.catkData));

    for (i = 0; i < SPELLCARD_COUNT_SPELLCARDS; ++i, ++catk)
    {
        catk->base.magic = CATK_MAGIC;
        catk->base.chapterSizeCopy = sizeof(Catk);
        catk->base.chapterSize = sizeof(Catk);
        catk->base.version = 3;
        catk->spellcardNumber = static_cast<u16>(i);
        for (j = 0; j < 7; ++j)
        {
            catk->inGameHistory.attempts[j] = 0;
            catk->inGameHistory.captures[j] = 0;
            catk->inGameHistory.maxBonus[j] = 0;
        }
    }

    scoreDat = ScoreDat::OpenScore("score.dat");
    if (scoreDat == NULL)
    {
        g_GameErrorContext.Log("error : スコアファイルの作成に失敗しました\r\n");
        return ZUN_ERROR;
    }

    g_GameManager.globals->displayedHighScore =
        ScoreDat::GetHighScore(scoreDat, NULL, g_GameManager.shotType, g_GameManager.difficulty,
                               &g_GameManager.globals->continuesUsedInHighScore);
    ScoreDat::ParseCATK(scoreDat, g_GameManager.catkData);
    ScoreDat::ParseCLRD(scoreDat, g_GameManager.clrdData);
    ScoreDat::ParsePSCR(scoreDat, g_GameManager.pscrData);

    if (g_GameManager.flags.isPracticeMode)
    {
        g_GameManager.globals->displayedHighScore =
            g_GameManager.pscrData[g_GameManager.shotType]
                .highScores[g_GameManager.currentStage][g_GameManager.difficulty];
        g_GameManager.pscrData[g_GameManager.shotType]
            .attempts[g_GameManager.currentStage][g_GameManager.difficulty]++;
        g_GameManager.pscrData[g_GameManager.shotType].shotNumber = 1;
    }

    ScoreDat::ReleaseScore(scoreDat);
    memcpy(g_GameManager.catkData2, g_GameManager.catkData, sizeof(g_GameManager.catkData));
    memset(&g_GameManager.hscr, 0, sizeof(g_GameManager.hscr));
    g_GameManager.hscr.character = g_GameManager.shotType;
    g_GameManager.hscr.difficulty = static_cast<u8>(g_GameManager.difficulty);
    g_GameManager.hscr.cfg = g_Supervisor.cfg;
    g_GameManager.playtimeFrames = 0;
    return ZUN_SUCCESS;
}


// FUNCTION: th08 0x43be09
void __fastcall IncrementTruncate(u32 *value, i32 unused)
{
    if (*value < 999999u)
    {
        (*value)++;
    }
}

// FUNCTION: th08 0x43be2c
ZunResult GameManager::DeletedCallback(GameManager *gameManager)
{
    g_ScreenEffectCounter = 1;
    g_AsciiManager.nightBlindnessAlpha = 0;

    if (g_Supervisor.curState != SupervisorState_GameManagerReInit &&
        g_Supervisor.curState != SupervisorState_SpellcardPracticeRestart &&
        g_Supervisor.curState != SupervisorState_GameManagerNextStageWeird)
    {
        g_Supervisor.releaseResourcesOnRestart = TRUE;
    }
    else
    {
        g_Supervisor.releaseResourcesOnRestart = FALSE;
    }

    if (!g_GameManager.flags.isSpellPractice || g_Supervisor.releaseResourcesOnRestart)
    {
        g_Supervisor.StopAudio();
        if (g_Supervisor.cfg.musicMode == MIDI && g_Supervisor.midiOutput != NULL)
        {
            MidiOutput *midiOutput = g_Supervisor.midiOutput;

            midiOutput->StopPlayback();
            midiOutput->ParseFile(30);
            midiOutput->Play();
        }
    }

    while (g_SoundPlayer.ProcessQueues())
    {
    }
    Spellcard::CutChain();
    Background::CutChain();
    BulletManager::CutChain();
    Player::CutChain();
    EnemyManager::CutChain();
    EffectManager::CutChain();
    Gui::CutChain();

    if (!g_GameManager.flags.isReplay)
    {
        ReplayManager::StopRecording();
    }
    if (!g_GameManager.flags.isReplay)
    {
        g_Supervisor.UpdateGameTime();
    }
    g_Supervisor.systemTime = 0;
    g_Supervisor.UpdatePlayTime();
    gameManager->flags.replayInputEnabled = FALSE;
    g_AsciiManager.Reset();
    g_GameManager.skipCurrentFrame = FALSE;
    g_GameManager.gameplayFrameCounter = 0;
    return ZUN_SUCCESS;
}

void GameManager::CutChain()
{
    g_Chain.Cut(&g_GameManagerCalcChain);
    g_Chain.Cut(&g_GameManagerDrawChain);
    if (g_GameManager.globals->score >= 1000000000)
    {
        g_GameManager.globals->score = 999999999;
    }
    g_GameManager.globals->displayScore = g_GameManager.globals->score;
    g_Supervisor.framerateMultiplier = 1.0f;
}

void GameManager::IncreaseSubrank(int amount)
{
    this->subRank += amount;
    while (this->subRank >= 100)
    {
        this->rank++;
        this->subRank -= 100;
    }
    if (this->rank > this->maxRank)
    {
        this->rank = this->maxRank;
    }
}

void GameManager::DecreaseSubrank(int amount)
{
    this->subRank -= amount;
    while (this->subRank < 0)
    {
        this->rank--;
        this->subRank += 100;
    }
    if (this->rank < this->minRank)
    {
        this->rank = this->minRank;
    }
}

// FUNCTION: th08 0x43c0bb
void GameManager::AddToYoukaiGauge(i32 amount, i32 forceUpdate)
{
    if (g_Player.bombState.isInUse && !forceUpdate)
        return;

    this->globals->youkaiGauge += amount;
    if (this->globals->youkaiGauge < this->youkaiGaugeHumanLimit)
        this->globals->youkaiGauge = this->youkaiGaugeHumanLimit;
    else if (this->globals->youkaiGauge > this->youkaiGaugeYoukaiLimit)
        this->globals->youkaiGauge = this->youkaiGaugeYoukaiLimit;

    this->globals->youkaiGaugeCopy = this->globals->youkaiGauge;
}

// FUNCTION: th08 0x43c15f
ZunBool GameManager::IsExtraUnlockedForCharacter(i32 character)
{
    return (character > SHOT_YOUMU_YUYUKO) ||
           (this->clrdData[character].difficultiesClearedWithoutRetries[EASY] & EXTRA_UNLOCKED_FLAG ||
            this->clrdData[character].difficultiesClearedWithoutRetries[NORMAL] & EXTRA_UNLOCKED_FLAG ||
            this->clrdData[character].difficultiesClearedWithoutRetries[HARD] & EXTRA_UNLOCKED_FLAG ||
            this->clrdData[character].difficultiesClearedWithoutRetries[LUNATIC] & EXTRA_UNLOCKED_FLAG);
}

// FUNCTION: th08 0x43c1e9
ZunBool GameManager::IsExtraUnlocked()
{
    return this->IsExtraUnlockedForCharacter(SHOT_REIMU_YUKARI) ||
           this->IsExtraUnlockedForCharacter(SHOT_MARISA_ALICE) ||
           this->IsExtraUnlockedForCharacter(SHOT_SAKUYA_REMILIA) ||
           this->IsExtraUnlockedForCharacter(SHOT_YOUMU_YUYUKO);
}

// FUNCTION: th08 0x43c23b
ZunBool GameManager::IsSpellPracticeUnlockedForCharacter(i32 character)
{
    return (character > SHOT_YOUMU_YUYUKO) ||
           (this->clrdData[character].difficultiesClearedWithRetries[EASY] & SPELL_PRACTICE_UNLOCKED_FLAG ||
            this->clrdData[character].difficultiesClearedWithRetries[NORMAL] & SPELL_PRACTICE_UNLOCKED_FLAG ||
            this->clrdData[character].difficultiesClearedWithRetries[HARD] & SPELL_PRACTICE_UNLOCKED_FLAG ||
            this->clrdData[character].difficultiesClearedWithRetries[LUNATIC] & SPELL_PRACTICE_UNLOCKED_FLAG);
}

// FUNCTION: th08 0x43c2c5
ZunBool GameManager::IsSpellPracticeUnlocked()
{
    return this->IsSpellPracticeUnlockedForCharacter(SHOT_REIMU_YUKARI) ||
           this->IsSpellPracticeUnlockedForCharacter(SHOT_MARISA_ALICE) ||
           this->IsSpellPracticeUnlockedForCharacter(SHOT_SAKUYA_REMILIA) ||
           this->IsSpellPracticeUnlockedForCharacter(SHOT_YOUMU_YUYUKO);
}


// Leftover from PCB.
// FUNCTION: th08 0x43c317
ZunBool GameManager::IsPhantasmUnlocked()
{
    return FALSE;
}

// FUNCTION: th08 0x43c322
ZunBool GameManager::IsReplayPractice()
{
    return this->flags.isReplay && g_ReplayManager->replayData->isPractice;
}

#pragma var_order(timeOrbs1, threshold1, timeOrbs2, threshold2, timeOrbs3, threshold3, timeOrbs4, threshold4, timeOrbs5, threshold5, timeOrbs6, threshold6)
i32 GameManager::GetClockTimeIncrement()
{
    i32 timeOrbs1;
    i32 threshold1;
    i32 timeOrbs2;
    i32 threshold2;
    i32 timeOrbs3;
    i32 threshold3;
    i32 timeOrbs4;
    i32 threshold4;
    i32 timeOrbs5;
    i32 threshold5;
    i32 timeOrbs6;
    i32 threshold6;

    // ZUN bloat: each stage keeps its own pair of temporaries.
    switch (g_GameManager.currentStage)
    {
    case STAGE1:
        timeOrbs1 = g_GameManager.globals->currentTimeOrbs;
        threshold1 = g_GameManager.globals->lastSpellTimeOrbThreshold;
        if (timeOrbs1 >= threshold1)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    case STAGE2:
        timeOrbs2 = g_GameManager.globals->currentTimeOrbs;
        threshold2 = g_GameManager.globals->lastSpellTimeOrbThreshold;
        if (timeOrbs2 >= threshold2)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    case STAGE3:
        timeOrbs3 = g_GameManager.globals->currentTimeOrbs;
        threshold3 = g_GameManager.globals->lastSpellTimeOrbThreshold;
        if (timeOrbs3 >= threshold3)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    case STAGE4A:
        timeOrbs4 = g_GameManager.globals->currentTimeOrbs;
        threshold4 = g_GameManager.globals->lastSpellTimeOrbThreshold;
        if (timeOrbs4 >= threshold4)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    case STAGE4B:
        timeOrbs5 = g_GameManager.globals->currentTimeOrbs;
        threshold5 = g_GameManager.globals->lastSpellTimeOrbThreshold;
        if (timeOrbs5 >= threshold5)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    case STAGE5:
        timeOrbs6 = g_GameManager.globals->currentTimeOrbs;
        threshold6 = g_GameManager.globals->lastSpellTimeOrbThreshold;
        if (timeOrbs6 >= threshold6)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    case STAGE6A:
        return 0;
    case STAGE6B:
        return 0;
    default:
        return 4;
    }
}

void GameManager::AdvanceToNextStage()
{
    switch (this->currentStage)
    {
    case STAGE1:
        this->currentStage = STAGE2;
        break;
    case STAGE2:
        this->currentStage = STAGE3;
        break;
    case STAGE3:
        switch (g_GameManager.shotType)
        {
        case SHOT_REIMU_YUKARI:
        case SHOT_REIMU:
        case SHOT_YUKARI:
            this->currentStage = STAGE4B;
            break;
        case SHOT_MARISA_ALICE:
        case SHOT_MARISA:
        case SHOT_ALICE:
            this->currentStage = STAGE4A;
            break;
        case SHOT_SAKUYA_REMILIA:
        case SHOT_SAKUYA:
        case SHOT_REMILIA:
            this->currentStage = STAGE4A;
            break;
        case SHOT_YOUMU_YUYUKO:
        case SHOT_YOUMU:
        case SHOT_YUYUKO:
            this->currentStage = STAGE4B;
            break;
        }
        break;
    case STAGE4A:
    case STAGE4B:
        this->currentStage = STAGE5;
        break;
    case STAGE5:
        this->currentStage = g_GameManager.flags.finalStageRoute ? STAGE6B : STAGE6A;
        break;
    case STAGE6A:
        // Advance from the first final-stage route to the alternate route.
        this->currentStage = STAGE6B;
        break;
    }
}

GameManager::GameManager()
{
    memset(this, 0, sizeof(GameManager));
    this->arcadeRegionTopLeftPos.x = 32.0f;
    this->arcadeRegionTopLeftPos.y = 16.0f;
    this->arcadeRegionSize.x = 384.0f;
    this->arcadeRegionSize.y = 448.0f;
    this->currentDemoReplay = 3;
}

void GameManager::AddLives(int lives)
{
    if (this->IsTampered())
    {
        CRASH_GAME();
    }
    this->globals->livesRemaining += lives;
    this->UpdateAntiTamper();
}

void GameManager::InitArcadeRegionParams()
{
    this->arcadeRegionTopLeftPos.x = 32.0f;
    this->arcadeRegionTopLeftPos.y = 16.0f;
    this->arcadeRegionSize.x = 384.0f;
    this->arcadeRegionSize.y = 448.0f;
    this->playerMovementTopLeftPos.x = 8.0f;
    this->playerMovementTopLeftPos.y = 16.0f;
    this->playerMovementAreaSize.x = 368.0f;
    this->playerMovementAreaSize.y = 416.0f;
}

}; // Namespace th08
