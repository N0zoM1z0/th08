#pragma once

#include "ScoreDat.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <stddef.h>
#include <windows.h>

#define EXTRA_UNLOCKED_FLAG ZUN_BIT(14)
#define SPELL_PRACTICE_UNLOCKED_FLAG ZUN_BIT(15)

#define IS_STAGE_CLEARED(difficulty, stage) (difficulty & ZUN_BIT(stage))

#define ANTITAMPER_RNG_RANGE 100000
#define ANTITAMPER_RNG_ADD 6543

namespace th08
{

void __fastcall IncrementTruncate(u32 *value, i32 unused);

struct GameManagerFlags
{
    u32 isPracticeMode : 1;
    u32 isDemoMode : 1;
    u32 unk2 : 1;
    u32 isReplay : 1;
    u32 unk4 : 1;
    // Observed as a two-bit state at bits 5..6 by GameManager::OnDraw.
    u32 unk5_6 : 2;
    u32 unk7 : 1;
    u32 unk8 : 1;
    u32 unk9 : 1;
    u32 unk10 : 1;
    u32 isGoingToFinalB : 2; // why 2 bits?
    u32 unk13 : 1;
    u32 isSpellPractice : 1;

    u32 isExtraUnlocked : 1;
    u32 isSpellPracticeUnlocked : 1;
    u32 isExtraUnlockedWithAllTeams : 1;
};

enum
{
    REPLAY_MODE_NORMAL,
    REPLAY_MODE_SLOWDOWN,
    REPLAY_MODE_BOSS,
};

struct GameManager
{
    GameManager();

    ZunBool IsWithinPlayfield(f32 x, f32 y, f32 width, f32 height);
    i32 CalcAntiTamperChecksum();
    static i32 CalcChecksum(u8 *address, i32 size);
    void CollectExtend();

    static ChainCallbackResult OnUpdate(GameManager *gameManager);
    static ChainCallbackResult OnDraw(GameManager *gameManager);

    static ZunResult RegisterChain();

    static ZunResult AddedCallback(GameManager *gameManager);
    static void GameplaySetupThread();

    void InitRankParams();

    static void InitializeAntiTamper();

    i32 GetTimeOrbs();
    i32 GetLastSpellTimeOrbThreshold();
    i32 GetLives();
    i32 GetBombsRemaining();
    i32 GetDeaths();
    i32 GetBombsUsed();

    void UpdateAntiTamper()
    {
        this->globals->rng1[2] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng7[3] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->antiTamperValue = this->globals->rng1[2];
        this->globals->antiTamperChecksum = CalcAntiTamperChecksum();
        this->antiTamperExpectedValue = this->globals->antiTamperChecksum + this->globals->rng7[3];
    }

    void RandomizeAntiTamper()
    {
        this->globals->rng1[0] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng1[1] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng1[2] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng1[3] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng1[4] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng4[0] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng4[1] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
        this->globals->rng4[2] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    }

    ZunBool IsTampered()
    {
        // There is zero chance ZUN actually used intptr_t here, but the codegen matches and not making
        // assumptions about pointer size is always nice
        return this->globals->antiTamperValue !=
                   this->globals->rng1[2] + this->globals->rng8[2] * ((intptr_t)&this->globals->antiTamperValue -
                                                                      (intptr_t)&this->globals->rng1 + 500) ||
               this->globals->antiTamperChecksum + this->globals->rng7[3] != (i32)this->antiTamperExpectedValue;
    }

    static ZunResult DeletedCallback(GameManager *gameManager);

    static void CutChain();

    void IncreaseSubrank(int amount);
    void DecreaseSubrank(int amount);
    i32 ScaleIntBasedOnRank(i32 upper, i32 lower);
    f32 ScaleFloatBasedOnRank(f32 upper, f32 lower);
    void AddToYoukaiGauge(i32 amount, i32 forceUpdate);
    void SetYoukaiGauge(u16 value);

    ZunBool IsPhantasmUnlocked();
    ZunBool IsReplayPractice();

    /* I know it's dumb but this is the only way to get it matching */
    void SetIsReplayWeird(ZunBool value)
    {
        ZunBool res = value;

        this->flags.isReplay = res;
    }

    ZunBool IsPracticeMode()
    {
        return this->flags.isPracticeMode;
    }

    ZunBool IsReplay()
    {
        return this->flags.isReplay;
    }
    ZunBool IsSpellPractice()
    {
        return this->flags.isSpellPractice;
    }
    ZunBool IsSpellNumberEqualTo(i32 spellCardNumber)
    {
        return this->flags.isSpellPractice ? this->currentSpellCardNumber == spellCardNumber : false;
    }
    ZunBool IsSpellNumberInRange(i32 firstSpell, i32 lastSpell)
    {
        return this->flags.isSpellPractice
                   ? this->currentSpellCardNumber >= firstSpell && this->currentSpellCardNumber <= lastSpell
                   : false;
    }
    ZunBool IsDemoMode()
    {
        return this->flags.isDemoMode;
    }

    ZunBool IsSoloHuman();
    ZunBool IsSoloYoukai();

    i32 GetYoukaiGauge()
    {
        return this->globals->youkaiGauge;
    }

    ZunBool GaugeIsExtremelyHuman()
    {
        return this->globals->youkaiGauge <= this->youkaiGaugeHumanEffectsThreshold;
    }
    ZunBool GaugeIsModeratelyHuman()
    {
        return this->globals->youkaiGauge <= this->youkaiGaugeHumanTintThreshold;
    }
    ZunBool GaugeIsExtremelyYoukai()
    {
        return this->globals->youkaiGauge >= this->youkaiGaugeYoukaiEffectsThreshold;
    }
    ZunBool GaugeIsModeratelyYoukai()
    {
        return this->globals->youkaiGauge >= this->youkaiGaugeYoukaiTintThreshold;
    }

    u8 GetClockTime()
    {
        return this->globals->clockTime;
    }
    void AddToClockTime(i8 value)
    {
        this->globals->clockTime += value;
    }
    void SetClockTime(i32 value)
    {
        this->globals->clockTime = value;
    }

    ZunBool IsStageClearedWithRetries(i32 stage, i32 character, i32 difficulty);
    ZunBool IsStageClearedWithoutRetries(i32 stage, i32 character, i32 difficulty);

    ZunBool IsExtraUnlockedForCharacter(i32 character);
    ZunBool IsExtraUnlocked();
    ZunBool IsSpellPracticeUnlockedForCharacter(i32 character);
    ZunBool IsSpellPracticeUnlocked();

    ZunBool IsExtraUnlockedWithAllTeams()
    {
        return this->IsExtraUnlockedForCharacter(SHOT_REIMU_YUKARI) &&
               this->IsExtraUnlockedForCharacter(SHOT_MARISA_ALICE) &&
               this->IsExtraUnlockedForCharacter(SHOT_SAKUYA_REMILIA) &&
               this->IsExtraUnlockedForCharacter(SHOT_YOUMU_YUYUKO);
    }

    ZunBool HasSpellCardBeenEncountered(i32 spellCardNumber, i32 shotType)
    {
        Catk *catk = &this->catkData[spellCardNumber];

        return catk->inGameHistory.attempts[shotType] > 0 || catk->spellPracticeHistory.attempts[shotType] != 0;
    }

    ZunBool IsLastWordSpellCardAttempted(i32 spellCardNumber)
    {
        return spellCardNumber < SPELLCARD_LAST_WORD_START &&
                   (this->catkData[spellCardNumber].inGameHistory.attempts[SHOT_ALL] != 0 ||
                    this->catkData[spellCardNumber].spellPracticeHistory.attempts[SHOT_ALL] != 0) ||
               this->flsp.unlockedLastWordSpellCards[spellCardNumber - SPELLCARD_LAST_WORD_START] == spellCardNumber;
    }

    i32 GetPower()
    {
        return this->globals->playerPower;
    }

    i32 GetClockTimeIncrement();
    void AdvanceToNextStage();
    void SetLives(i32 lives);
    void SetBombCount(i32 bombs);
    void SetPower(i32 power);
    void AddScore(i32 score);
    void AddTimeOrbs(i32 amount);
    void AddToDeaths(i32 amount);
    void AddToBombsUsed(i32 amount);

    void AddLives(int lives)
    {
        if (this->IsTampered())
        {
            CRASH_GAME();
        }
        this->globals->livesRemaining += lives;
        this->UpdateAntiTamper();
    }

    void AddPower(int power)
    {
        if (this->IsTampered())
        {
            CRASH_GAME();
        }
        this->globals->playerPower += power;
        this->UpdateAntiTamper();
    }

    void AddToBombCount(int amount)
    {
        if (this->IsTampered())
        {
            CRASH_GAME();
        }
        this->globals->bombsRemaining += amount;
        this->UpdateAntiTamper();
    }

    void InitArcadeRegionParams();

    ZunBool IsUnknown()
    {
        return this->unk2D;
    }

    i32 unk0x0;
    GameConfiguration *cfg;
    ZunGlobals *globals;
    Flsp flsp;
    i8 unk2C;
    i8 unk2D;
    /* 2 bytes pad */
    i32 difficulty;
    i32 difficultyMask;
    u32 unk38;
    i32 unk3c;
    Catk catkData[SPELLCARD_COUNT_SPELLCARDS];
    Catk catkData2[SPELLCARD_COUNT_SPELLCARDS];
    Clrd clrdData[SHOT_ALL + 1];
    Pscr pscrData[SHOT_ALL];
    Plst plst;
    Hscr hscr;
    i32 unk3DB94;
    i32 unk3DB98;
    i32 unk3DB9C;
    i32 unk3DBA0;
    i32 unk3DBA4;
    u8 character;
    u8 shotType;
    u8 fullShotType;
    u8 unk3dbaa;
    GameManagerFlags flags;
    i16 currentSpellCardNumber;
    u8 isInGameMenu;
    u8 showRetryMenu;
    u8 currentDemoReplay;
    u8 replayMode;
    u8 unk3DBB6;
    u8 unk3DBB7;

    i32 demoFrameCount;
    char replayFilename[512];
    u32 unk3ddbc;
    u32 unk3ddc0;
    i32 currentStage;
    i32 currentStage2;
    u32 unk3ddcc;
    u16 unk3DDD0;
    u16 unk3DDD2;
    Float2 arcadeRegionTopLeftPos;
    Float2 arcadeRegionSize;
    Float2 playerMovementTopLeftPos;
    Float2 playerMovementAreaSize;
    f32 antiTamperExpectedValue;
    i16 youkaiGaugeHumanLimit;
    i16 youkaiGaugeYoukaiLimit;
    i16 youkaiGaugeHumanEffectsThreshold;
    i16 youkaiGaugeYoukaiEffectsThreshold;
    i16 youkaiGaugeHumanTintThreshold;
    i16 youkaiGaugeYoukaiTintThreshold;

    i32 unk3de04;
    u32 unk3de08;
    u32 unk3de0c;
    u32 unk3de10;
    u32 unk3de14;
    u32 unk3de18;
    u32 unk3de1c;
    u32 unk3de20;
    u32 unk3de24;
    u32 unk3de28;

    i32 rank;
    i32 maxRank;
    i32 minRank;
    i32 subRank;
};

C_ASSERT(sizeof(GameManager) == 0x3de3c);

DIFFABLE_EXTERN(GameManager, g_GameManager);
}; // Namespace th08
