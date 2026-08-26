#include "th_pch.h"

#include "GameManager.hpp"
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
#include "ScreenEffect.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"

#include <stdlib.h>
#include <mmsystem.h>

namespace th08
{

extern i32 FUN_00439916(i32 unused);
extern i32 InitializeScoreData();
extern i32 g_GuiMessageStageMode;
extern i32 g_TimeRequirementParams[][4];

#define GM_FLAGS_WORD(gm) (*reinterpret_cast<u32 *>(&(gm)->flags))

struct SetupPlayCountTable
{
    u8 prefix[0x2C];
    PlstPlayCounts counts[MAX_DIFFICULTIES + 2];
};
C_ASSERT(offsetof(SetupPlayCountTable, counts) == 0x2C);

// Target-facing probe owner for GameManager::GameplaySetupThread @ 0x0043ABD7.
// The retail body has one unused fastcall ECX home even though CreateThread's
// LPTHREAD_START_ROUTINE argument is ignored.  Keeping the dummy parameter in
// this probe reconstructs that emitted home without changing the claimed
// GameManager.cpp callback surface.
#pragma var_order(gameManager, size, replaySeed, i, oldCfg, oldGlobals, newCfg, newGlobals, allocation, stageMode, configMode)
void __fastcall GameplaySetupThread0043ABD7(void *unused)
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

    if (g_Supervisor.unk164 || gameManager->flags.isSpellPractice ||
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
        gameManager->unk0x0 = reinterpret_cast<i32>(malloc(size));
        newCfg = static_cast<GameConfiguration *>(operator new(sizeof(GameConfiguration)));
        gameManager->cfg = newCfg;
        newGlobals = static_cast<ZunGlobals *>(operator new(sizeof(ZunGlobals)));
        gameManager->globals = newGlobals;
        GameManager::InitializeAntiTamper();
        *gameManager->cfg = g_Supervisor.cfg;
        allocation = reinterpret_cast<void *>(gameManager->unk0x0);
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
            g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B5930));
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
        gameManager->humanityRateDenominator = 0;
        gameManager->humanityRateNumerator = 0;
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
                if (g_Supervisor.curState == 10)
                {
                    IncrementTruncate(reinterpret_cast<u32 *>(&g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].unk0x34), 999999);
                    IncrementTruncate(reinterpret_cast<u32 *>(&reinterpret_cast<SetupPlayCountTable *>(&g_GameManager.plst)->counts[MAX_DIFFICULTIES + 1].unk0x34), 999999);
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
            g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B5930));
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
        g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B5904));
        goto setup_error;
    }
    if (BulletManager::RegisterChain(reinterpret_cast<char *>(0x4B4CA0)))
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B58DC));
        goto setup_error;
    }
    if (EnemyManager::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B58B8));
        goto setup_error;
    }
    if (EffectManager::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B588C));
        goto setup_error;
    }
    if (Gui::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B5864));
        goto setup_error;
    }
    if (Spellcard::RegisterChain())
    {
        if (g_Supervisor.subthreadCloseRequestActive)
            goto thread_done;
        g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B5834));
        goto setup_error;
    }

    if (!g_GameManager.flags.isReplay)
        ReplayManager::RegisterChain(0, reinterpret_cast<const char *>(0x4B5820));

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

    if (!g_Supervisor.unk16c)
    {
        if (g_GameManager.flags.isSpellPractice)
        {
            i = 0;
            while (*reinterpret_cast<i32 *>(0x4C7670 + i * 0x14) >= 0)
            {
                if (g_GameManager.currentSpellCardNumber <= *reinterpret_cast<i32 *>(0x4C7670 + i * 0x14))
                {
                    g_Supervisor.LoadMusic(0, *reinterpret_cast<char **>(0x4C7678 + i * 0x14));
                    break;
                }
                ++i;
            }
        }
        else
        {
            g_Supervisor.LoadMusic(0, reinterpret_cast<char *>(*reinterpret_cast<u8 **>(0x4E4824) + 0x290));
            if (*reinterpret_cast<i8 *>(*reinterpret_cast<u8 **>(0x4E4824) + 0x310) != 0x20)
                g_Supervisor.LoadMusic(1, reinterpret_cast<char *>(*reinterpret_cast<u8 **>(0x4E4824) + 0x310));
            if (*reinterpret_cast<i8 *>(*reinterpret_cast<u8 **>(0x4E4824) + 0x390) != 0x20)
                g_Supervisor.LoadMusic(2, reinterpret_cast<char *>(*reinterpret_cast<u8 **>(0x4E4824) + 0x390));
        }
    }

    gameManager->showRetryMenu = 0;
    GM_FLAGS_WORD(gameManager) |= 4U;
    if (g_Supervisor.unk16c && g_GameManager.flags.isSpellPractice &&
        !FUN_00439916(g_GameManager.currentSpellCardNumber))
        gameManager->stageStartupMode = STAGE_STARTUP_WITHOUT_MUSIC;
    else
        gameManager->stageStartupMode = STAGE_STARTUP_PLAY_MUSIC;

    if (g_Supervisor.curState != 3)
    {
        g_Supervisor.lagNumerator = 0.0f;
        g_Supervisor.lagDenominator = 0.0f;
    }

    gameManager->unk2C = 0;
    gameManager->globals->score = 0;
    GM_FLAGS_WORD(gameManager) &= ~0x10U;
    g_AsciiManager.Reset();
    g_AsciiManager.InitializeVms();
    g_GameManager.skipCurrentFrame = 0;
    g_EclCallbackPublishedEnemyField24 = 0;
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

    g_Supervisor.FUN_00448972();
    while (gameManager->flags.stageTransitionState != 0)
        Sleep(17);

    g_GameManager.gameplaySetupState = GAMEPLAY_SETUP_COMPLETE;
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.unk290 = FALSE;
    g_Supervisor.unk174 = 60;
    GM_FLAGS_WORD(gameManager) &= ~0x200U;
    g_Supervisor.unk16c = 0;
    g_ScreenEffectCounter = 2;
    goto thread_done;

setup_error:
    g_GameManager.gameplaySetupState = GAMEPLAY_SETUP_FAILED;
    g_Supervisor.FUN_00448972();
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.unk290 = FALSE;
    g_Supervisor.unk16c = 0;
    g_ScreenEffectCounter = 2;

thread_done:
    (void)unused;
}

#undef GM_FLAGS_WORD

} // namespace th08
