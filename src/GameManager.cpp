#include "th_pch.h"

#include "GameManager.hpp"
#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EnemyManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"

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

DIFFABLE_STATIC(GameManager, g_GameManager);
DIFFABLE_STATIC(ChainElem, g_GameManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_GameManagerDrawChain);


// FUNCTION: th08 0x439916
i32 FUN_00439916(i32 unused)
{
    i32 i;

    i = 0;
    while (*reinterpret_cast<i32 *>(0x4C7670 + i * 0x14) >= 0)
    {
        if (static_cast<i32>(*reinterpret_cast<i16 *>(0x164D0B8)) <=
            *reinterpret_cast<i32 *>(0x4C7670 + i * 0x14))
        {
            return *reinterpret_cast<i32 *>(0x4C7680 + i * 0x14);
        }
        i++;
    }
    return 0;
}

// FUNCTION: th08 0x439961
i32 FUN_00439961(i32 unused)
{
    i32 i;

    i = 0;
    while (*reinterpret_cast<i32 *>(0x4C7670 + i * 0x14) >= 0)
    {
        if (static_cast<i32>(*reinterpret_cast<i16 *>(0x164D0B8)) <=
            *reinterpret_cast<i32 *>(0x4C7670 + i * 0x14))
        {
            return *reinterpret_cast<i32 *>(0x4C767C + i * 0x14);
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

#pragma optimize("t", on)
// FUNCTION: th08 0x421ba0
i32 GameManager::ScaleIntBasedOnRank(i32 upper, i32 lower)
{
    return this->rank * (lower - upper) / 32 + upper;
}

f32 GameManager::ScaleFloatBasedOnRank(f32 upper, f32 lower)
{
    return upper + ((f32)this->rank * (lower - upper)) / 32.0f;
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::SetLives(i32 lives)
{
    this->globals->livesRemaining = (f32)lives;
    this->UpdateAntiTamper();
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::SetBombCount(i32 bombs)
{
    this->globals->bombsRemaining = (f32)bombs;
    this->globals->antiTamperValue = this->globals->rng1[2];
    this->globals->antiTamperChecksum = this->CalcAntiTamperChecksum();
    this->antiTamperExpectedValue = (f32)(this->globals->antiTamperChecksum + this->globals->rng7[3]);
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::SetPower(i32 power)
{
    this->globals->playerPower = (f32)power;
    this->UpdateAntiTamper();
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::AddScore(i32 score)
{
    this->globals->score += score / 10;
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::AddToDeaths(i32 amount)
{
    if (this->IsTampered())
    {
        CRASH_GAME();
    }
    this->globals->deaths += (f32)amount;
    this->globals->deathInStage += (f32)amount;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3DA9C) += 1;
    this->UpdateAntiTamper();
}
#pragma optimize("", on)

#pragma optimize("t", on)
void GameManager::AddToBombsUsed(i32 amount)
{
    if (this->IsTampered())
    {
        CRASH_GAME();
    }
    this->globals->bombsUsed += (f32)amount;
    this->globals->bombsUsedInStage += (f32)amount;
    this->UpdateAntiTamper();
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


// FUNCTION: th08 0x43be09
void __fastcall IncrementTruncate(u32 *value, i32 unused)
{
    if (*value < 999999u)
    {
        (*value)++;
    }
}

#pragma optimize("t", on)
i32 GameManager::GetTimeOrbs()
{
    return this->globals->currentTimeOrbs;
}

i32 GameManager::GetLastSpellTimeOrbThreshold()
{
    return this->globals->lastSpellTimeOrbThreshold;
}

i32 GameManager::GetLives()
{
    return this->globals->livesRemaining;
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
    if (this->GetLives() < 8)
    {
        this->AddLives(1);
        g_SoundPlayer.PlaySoundByIdx(SOUND_1UP, 0);
        this->IncreaseSubrank(200);
        g_Gui.flags.lifeDisplayUpdateFrames = 2;
    }
    else if (this->GetBombsRemaining() < 8)
    {
        this->AddToBombCount(1);
        g_SoundPlayer.PlaySoundByIdx(SOUND_1UP, 0);
        this->IncreaseSubrank(200);
        g_Gui.flags.bombDisplayUpdateFrames = 2;
    }
}

// FUNCTION: th08 0x43b936
void GameManager::InitRankParams()
{
    this->rank = g_RankParams[g_GameManager.difficulty].rank;
    this->minRank = g_RankParams[g_GameManager.difficulty].minRank;
    this->maxRank = g_RankParams[g_GameManager.difficulty].maxRank;
}

// STUB: th08 0x439bc7
ChainCallbackResult GameManager::OnUpdate(GameManager *gameManager)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

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
    if (gameManager->flags.unk5_6 == 1)
    {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }
    if (gameManager->unk38)
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
    mgr->unk3ddc0 = 0;
    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChain, 2))
    {
        return ZUN_ERROR;
    }

    g_GameManagerDrawChain.callback = (ChainCallback)GameManager::OnDraw;
    g_GameManagerDrawChain.addedCallback = NULL;
    g_GameManagerDrawChain.deletedCallback = NULL;
    g_GameManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChain, 5);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x43aaf4
ZunResult GameManager::AddedCallback(GameManager *gameManager)
{
    if (g_Supervisor.curState != SupervisorState_GameManagerReInit &&
        g_Supervisor.curState != SupervisorState_SpellcardPracticeRestart &&
        g_Supervisor.curState != SupervisorState_GameManagerNextStageWeird)
    {
        g_Supervisor.unk164 = TRUE;
    }
    else
    {
        g_Supervisor.unk164 = FALSE;
    }
    g_GameManager.unk38 = 1;

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

    if (gameManager->flags.unk5_6 >= 2)
    {
        gameManager->flags.unk5_6 = 1;
    }
    g_Supervisor.ThreadStart((LPTHREAD_START_ROUTINE)GameManager::GameplaySetupThread, NULL);
    return ZUN_SUCCESS;
}

// STUB: th08 0x43abd7
void GameManager::GameplaySetupThread()
{
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

// FUNCTION: th08 0x43be2c
ZunResult GameManager::DeletedCallback(GameManager *gameManager)
{
    g_ScreenEffectCounter = 1;
    g_EclCallbackPublishedEnemyField24 = 0;

    if (g_Supervisor.curState != SupervisorState_GameManagerReInit &&
        g_Supervisor.curState != SupervisorState_SpellcardPracticeRestart &&
        g_Supervisor.curState != SupervisorState_GameManagerNextStageWeird)
    {
        g_Supervisor.unk168 = TRUE;
    }
    else
    {
        g_Supervisor.unk168 = FALSE;
    }

    if (!g_GameManager.flags.isSpellPractice || g_Supervisor.unk168)
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
    gameManager->flags.unk2 = FALSE;
    g_AsciiManager.Reset();
    g_GameManager.unk2D = FALSE;
    g_GameManager.unk3ddc0 = 0;
    return ZUN_SUCCESS;
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
    if (g_Player.frameStop && !forceUpdate)
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

// FUNCTION: th08 0x42f230
#pragma optimize("t", on)
ZunBool GameManager::IsSoloHuman()
{
    return this->shotType >= 4 && (this->shotType & 1) == 0;
}

// FUNCTION: th08 0x42f270
ZunBool GameManager::IsSoloYoukai()
{
    return this->shotType >= 4 && (this->shotType & 1) != 0;
}
#pragma optimize("", on)

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
        this->currentStage = g_GameManager.flags.isGoingToFinalB ? STAGE6B : STAGE6A;
        break;
    case STAGE6A:
        this->currentStage = STAGE6B; // Was Kaguya meant to be a TLB at one point???
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
