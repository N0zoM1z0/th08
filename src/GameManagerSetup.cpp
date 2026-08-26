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
extern i32 FUN_0043bbe1();
extern i32 g_GuiMessageStageMode;
extern i32 g_TimeRequirementParams[][4];

#define GM_BYTE(gm, off) (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(gm) + (off)))
#define GM_WORD(gm, off) (*reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(gm) + (off)))
#define GM_I32(gm, off) (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(gm) + (off)))
#define GM_U32(gm, off) (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(gm) + (off)))
#define CFG_BYTE(gm, off) (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>((gm)->cfg) + (off)))
#define GLOBAL_BYTE(gm, off) (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>((gm)->globals) + (off)))
#define GLOBAL_WORD(gm, off) (*reinterpret_cast<u16 *>(reinterpret_cast<u8 *>((gm)->globals) + (off)))
#define GLOBAL_I32(gm, off) (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>((gm)->globals) + (off)))
#define GLOBAL_U32(gm, off) (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>((gm)->globals) + (off)))
#define GLOBAL_F32(gm, off) (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>((gm)->globals) + (off)))

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
    gameManager->unk3c = 0;
    g_Supervisor.systemTime = timeGetTime();

    gameManager->unk3DDD0 = static_cast<u16>(1 << gameManager->currentStage);
    gameManager->currentStage2 = gameManager->currentStage;
    if (gameManager->difficulty < 4)
        gameManager->difficultyMask = 1 << gameManager->difficulty;
    else
        gameManager->difficultyMask = 0xf;

    gameManager->unk3dbaa = gameManager->shotType + gameManager->fullShotType;
    g_Supervisor.framerateMultiplier = 1.0f;
    GM_U32(gameManager, 0x3DBAC) &= ~0x400U;

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
        GLOBAL_WORD(gameManager, 0x22) = 0;
        if (g_GameManager.currentStage == 8)
            stageMode = 6;
        else
            stageMode = 0;
        GLOBAL_BYTE(gameManager, 0x28) = static_cast<u8>(stageMode);
        if (g_GameManager.difficulty >= 4)
            CFG_BYTE(gameManager, 0x1c) = 2;
        if (g_GameManager.flags.isPracticeMode)
            CFG_BYTE(gameManager, 0x1c) = 8;

        if (Player::RegisterChain(0))
        {
            if (g_Supervisor.subthreadCloseRequestActive)
                goto thread_done;
            g_GameErrorContext.Log(reinterpret_cast<char *>(0x4B5930));
            goto setup_error;
        }

        if (!g_GameManager.flags.isReplay)
        {
            configMode = CFG_BYTE(gameManager, 0x1c);
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_GameManager.globals) + 0x74) =
                static_cast<f32>(configMode);
            g_GameManager.UpdateAntiTamper();
            g_GameManager.SetBombCount(static_cast<i32>(
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_Player.primaryShtFile) + 4)));
        }

        gameManager->InitArcadeRegionParams();
        GLOBAL_F32(gameManager, 0x98) = 0.0f;
        gameManager->UpdateAntiTamper();
        gameManager->unk3de04 = 0;
        gameManager->unk3DBA4 = 0;
        gameManager->unk3DBA0 = 0;
        GLOBAL_I32(gameManager, 0x00) = 0;
        GLOBAL_I32(gameManager, 0x08) = 0;
        GLOBAL_I32(gameManager, 0x10) = 0;
        GLOBAL_I32(gameManager, 0x14) = 100000;
        GLOBAL_BYTE(gameManager, 0x29) = 0;
        GLOBAL_I32(gameManager, 0x0c) = 0;
        GLOBAL_I32(gameManager, 0x30) = 0;

        if (gameManager->difficulty >= 4 || gameManager->flags.isPracticeMode ||
            gameManager->flags.isSpellPractice)
            CFG_BYTE(gameManager, 0x25) = 0;

        switch (g_GameManager.difficulty)
        {
        case 0:
            GLOBAL_I32(gameManager, 0x24) = 60000;
            break;
        case 1:
            GLOBAL_I32(gameManager, 0x24) = 100000;
            break;
        case 2:
            GLOBAL_I32(gameManager, 0x24) = 200000;
            break;
        case 3:
            GLOBAL_I32(gameManager, 0x24) = 300000;
            break;
        case 4:
            GLOBAL_I32(gameManager, 0x24) = 300000;
            break;
        }
        GLOBAL_I32(gameManager, 0x34) = 0;
        ItemManager::UpdatePointItemExtendThreshold();
        if (FUN_0043bbe1())
            goto setup_error;

        gameManager->InitRankParams();
        GLOBAL_F32(gameManager, 0x64) = 0.0f;
        GLOBAL_F32(gameManager, 0x68) = 0.0f;
        gameManager->UpdateAntiTamper();
        GLOBAL_F32(gameManager, 0x84) = 0.0f;
        GLOBAL_F32(gameManager, 0x88) = 0.0f;
        gameManager->UpdateAntiTamper();
        GLOBAL_I32(gameManager, 0x1c) = 0;
        gameManager->runActiveFrames = 0;
        gameManager->runExtremeYoukaiFrames = 0;
        gameManager->runExtremeHumanFrames = 0;

        if (!g_GameManager.flags.isReplay && !g_GameManager.flags.isSpellPractice)
        {
            if (!CFG_BYTE(gameManager, 0x25))
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
            CFG_BYTE(gameManager, 0x25) = 0;
        }
    }
    else
    {
        GLOBAL_I32(gameManager, 0x00) = GLOBAL_I32(gameManager, 0x08);
        GLOBAL_I32(gameManager, 0x10) = 0;
        GLOBAL_F32(gameManager, 0x68) = 0.0f;
        gameManager->UpdateAntiTamper();
        GLOBAL_F32(gameManager, 0x88) = 0.0f;
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
    GLOBAL_I32(gameManager, 0x2c) = 0;
    GLOBAL_I32(gameManager, 0x04) = 0;
    gameManager->isInGameMenu = 0;
    GM_U32(gameManager, 0x3DBAC) &= ~0x180U;
    GM_U32(gameManager, 0x3DBAC) &= ~0x2000U;
    gameManager->stageActiveFrames = 0;
    gameManager->stageExtremeYoukaiFrames = 0;
    gameManager->stageExtremeHumanFrames = 0;
    GLOBAL_WORD(gameManager, 0x20) = GLOBAL_WORD(gameManager, 0x22);
    GLOBAL_I32(gameManager, 0x3c) = 0;
    GLOBAL_I32(gameManager, 0x44) = 0;
    if (!g_GameManager.flags.isSpellPractice)
        GLOBAL_I32(gameManager, 0x40) = g_TimeRequirementParams[gameManager->currentStage][g_GameManager.difficulty];
    else
        GLOBAL_I32(gameManager, 0x40) = 0;

    if (gameManager->flags.isPracticeMode)
    {
        if (!gameManager->flags.isSpellPractice)
        {
            switch (gameManager->currentStage)
            {
            case 0:
                GLOBAL_F32(gameManager, 0x98) = 0.0f;
                gameManager->UpdateAntiTamper();
                break;
            case 1:
                GLOBAL_F32(gameManager, 0x98) = 112.0f;
                gameManager->UpdateAntiTamper();
                break;
            default:
                GLOBAL_F32(gameManager, 0x98) = 128.0f;
                gameManager->UpdateAntiTamper();
                break;
            }
        }
        else
        {
            if (gameManager->currentSpellCardNumber <= 1)
            {
                GLOBAL_F32(gameManager, 0x98) = 30.0f;
                gameManager->UpdateAntiTamper();
            }
            else if (gameManager->currentSpellCardNumber <= 12)
            {
                GLOBAL_F32(gameManager, 0x98) = 80.0f;
                gameManager->UpdateAntiTamper();
            }
            else
            {
                GLOBAL_F32(gameManager, 0x98) = 128.0f;
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
    *reinterpret_cast<u16 *>(&gameManager->unk3ddbc) = *reinterpret_cast<u16 *>(&g_Rng);

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
    GM_U32(gameManager, 0x3DBAC) |= 4U;
    if (g_Supervisor.unk16c && g_GameManager.flags.isSpellPractice &&
        !FUN_00439916(g_GameManager.currentSpellCardNumber))
        gameManager->unk3de28 = 2;
    else
        gameManager->unk3de28 = 1;

    if (g_Supervisor.curState != 3)
    {
        g_Supervisor.lagNumerator = 0.0f;
        g_Supervisor.lagDenominator = 0.0f;
    }

    gameManager->unk2C = 0;
    GLOBAL_I32(gameManager, 0x08) = 0;
    GM_U32(gameManager, 0x3DBAC) &= ~0x10U;
    g_AsciiManager.Reset();
    g_AsciiManager.InitializeVms();
    g_GameManager.unk2D = 0;
    g_EclCallbackPublishedEnemyField24 = 0;
    Supervisor::CalculateFps(0);

    if (g_GameManager.flags.isReplay)
    {
        while (gameManager->unk3c < 80)
            Sleep(17);
    }
    else
    {
        while (gameManager->unk3c < 30)
            Sleep(17);
    }

    g_Supervisor.FUN_00448972();
    while (gameManager->flags.unk5_6 != 0)
        Sleep(17);

    g_GameManager.unk38 = 0;
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.unk290 = FALSE;
    g_Supervisor.unk174 = 60;
    GM_U32(gameManager, 0x3DBAC) &= ~0x200U;
    g_Supervisor.unk16c = 0;
    g_ScreenEffectCounter = 2;
    goto thread_done;

setup_error:
    g_GameManager.unk38 = 2;
    g_Supervisor.FUN_00448972();
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.unk290 = FALSE;
    g_Supervisor.unk16c = 0;
    g_ScreenEffectCounter = 2;

thread_done:
    (void)unused;
}

#undef GLOBAL_F32
#undef GLOBAL_U32
#undef GLOBAL_I32
#undef GLOBAL_WORD
#undef GLOBAL_BYTE
#undef CFG_BYTE
#undef GM_U32
#undef GM_I32
#undef GM_WORD
#undef GM_BYTE

} // namespace th08
