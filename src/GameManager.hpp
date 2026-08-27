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
    enum
    {
        PLAYER_DEATH_DISSOLVE_SHIFT = 7,
        PLAYER_DEATH_DISSOLVE_MASK = 3,
    };

    u32 isPracticeMode : 1;
    u32 isDemoMode : 1;
    u32 replayInputEnabled : 1;
    u32 isReplay : 1;
    u32 gameCleared : 1;
    // Observed as a two-bit state at bits 5..6 by GameManager::OnDraw.
    u32 stageTransitionState : 2;
    // Nonzero selects the alternate player-death dissolve path.
    u32 playerDeathDissolveMode : 2;
    u32 stageClearSequenceActive : 1;
    u32 deathbombFreezeActive : 1;
    u32 finalStageRoute : 2;
    u32 suppressPlayerShots : 1;
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

enum GameplaySetupState
{
    GAMEPLAY_SETUP_COMPLETE,
    GAMEPLAY_SETUP_IN_PROGRESS,
    GAMEPLAY_SETUP_FAILED,
};

enum StageStartupMode
{
    STAGE_STARTUP_NONE,
    STAGE_STARTUP_PLAY_MUSIC,
    STAGE_STARTUP_WITHOUT_MUSIC,
};

struct SpellcardMusicEntry
{
    i32 spellcardNumber;
    i32 songNumber;
    const char *songPath;
    i32 songNameSpriteIdx;
    ZunBool musicPausesInSpellPractice;
};
C_ASSERT(sizeof(SpellcardMusicEntry) == 0x14);

DIFFABLE_EXTERN_ARRAY(SpellcardMusicEntry, 19, g_SpellcardMusicInfo);

struct GameManager
{
    GameManager();

    static ZunBool ShouldPauseMusicInSpellPractice(i32 spellcardNumber);
    static i32 GetSongNameSpriteIdx(i32 spellcardNumber);

    ZunBool IsWithinPlayfield(f32 x, f32 y, f32 width, f32 height);
    i32 CalcAntiTamperChecksum();
    static i32 CalcChecksum(u8 *address, i32 size);
    void CollectExtend();

    static ChainCallbackResult OnUpdate(GameManager *gameManager);
    static ChainCallbackResult OnDraw(GameManager *gameManager);

    static ZunResult RegisterChain();

    static ZunResult AddedCallback(GameManager *gameManager);
    static void __fastcall GameplaySetupThread(void *unused);

    void InitRankParams();

    static void InitializeAntiTamper();

    i32 GetTimeOrbs();
    i32 GetLastSpellTimeOrbThreshold();
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

    void RandomizeAntiTamper();

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

    ZunBool IsSpellPractice()
    {
        return this->flags.isSpellPractice;
    }
    ZunBool IsReplay()
    {
        return this->flags.isReplay;
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

    ZunBool IsSoloHuman()
    {
        return this->shotType >= 4 && (this->shotType & 1) == 0;
    }
    ZunBool IsSoloYoukai()
    {
        return this->shotType >= 4 && (this->shotType & 1) != 0;
    }
    i32 GetLives()
    {
        return this->globals->livesRemaining;
    }

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
    void SetClockTime(i32 value);

    ZunBool IsStageClearedWithRetries(i32 stage, i32 character, i32 difficulty);
    ZunBool IsStageClearedWithoutRetries(i32 stage, i32 character, i32 difficulty);
    inline ZunBool StageClearedWithoutRetries(
        i32 stage, i32 character, i32 difficulty)
    {
        return this->clrdData[character]
                   .difficultiesClearedWithoutRetries[difficulty] & ZUN_BIT(stage);
    }
    inline ZunBool StageClearedWithRetries(
        i32 stage, i32 character, i32 difficulty)
    {
        return this->clrdData[character]
                   .difficultiesClearedWithRetries[difficulty] & ZUN_BIT(stage);
    }

    ZunBool IsExtraUnlockedForCharacter(i32 character);
    ZunBool IsExtraUnlocked();
    ZunBool IsSpellPracticeUnlockedForCharacter(i32 character);
    ZunBool IsSpellPracticeUnlocked();

    ZunBool IsExtraUnlockedWithAllTeams();

    ZunBool HasSpellCardBeenEncountered(i32 spellCardNumber, i32 shotType)
    {
        Catk *catk = &this->catkData[spellCardNumber];

        return catk->inGameHistory.attempts[shotType] > 0 || catk->spellPracticeHistory.attempts[shotType] != 0;
    }

    ZunBool IsLastWordSpellCardAttempted(i32 spellCardNumber);

    i32 GetPower()
    {
        return this->globals->playerPower;
    }

    i32 GetClockTimeIncrement();
    void AdvanceToNextStage();
    void SetLives(i32 lives)
    {
        this->globals->livesRemaining = (f32)lives;
        this->UpdateAntiTamper();
    }
    void SetBombCount(i32 bombs)
    {
        this->globals->bombsRemaining = (f32)bombs;
        this->globals->antiTamperValue = this->globals->rng1[2];
        this->globals->antiTamperChecksum = this->CalcAntiTamperChecksum();
        this->antiTamperExpectedValue =
            (f32)(this->globals->antiTamperChecksum + this->globals->rng7[3]);
    }
    void SetPower(i32 power)
    {
        this->globals->playerPower = (f32)power;
        this->UpdateAntiTamper();
    }
    void AddScore(i32 score);
    void AddTimeOrbs(i32 amount);
    void AddToDeaths(i32 amount);
    void AddToBombsUsed(i32 amount);

    void AddLives(int lives);

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

    ZunBool ShouldSkipCurrentFrame()
    {
        return this->skipCurrentFrame;
    }

    i32 antiTamperHeapJitterAllocation;
    GameConfiguration *cfg;
    ZunGlobals *globals;
    Flsp flsp;
    i8 scriptedUpdateFreeze;
    i8 skipCurrentFrame;
    /* 2 bytes pad */
    i32 difficulty;
    i32 difficultyMask;
    u32 gameplaySetupState;
    i32 gameplaySetupWaitFrames;
    Catk catkData[SPELLCARD_COUNT_SPELLCARDS];
    Catk catkData2[SPELLCARD_COUNT_SPELLCARDS];
    Clrd clrdData[SHOT_ALL + 1];
    Pscr pscrData[SHOT_ALL];
    Plst plst;
    Hscr hscr;
    i32 playtimeFrames;
    i32 replayPauseRecorded;
    i32 nextSupervisorState;
    i32 humanityRateNumerator;
    i32 humanityRateDenominator;
    u8 character;
    u8 shotType;
    u8 fullShotType;
    u8 characterListIndex;
    GameManagerFlags flags;
    i16 currentSpellCardNumber;
    u8 isInGameMenu;
    u8 showRetryMenu;
    u8 currentDemoReplay;
    u8 replayMode;

    i32 demoFrameCount;
    char replayFilename[512];
    u16 stageRngSeed;
    u32 gameplayFrameCounter;
    i32 currentStage;
    i32 stageAtStart;
    u32 unconsumedDword3DDCC;
    u16 currentStageClearFlag;
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

    i32 stagePlayTimeAll;
    u32 frameSkipCounter;
    u32 unconsumedDword3DE0C;
    u32 runActiveFrames;
    u32 stageActiveFrames;
    u32 runExtremeYoukaiFrames;
    u32 runExtremeHumanFrames;
    u32 stageExtremeYoukaiFrames;
    u32 stageExtremeHumanFrames;
    u32 stageStartupMode;

    i32 rank;
    i32 maxRank;
    i32 minRank;
    i32 subRank;
};

C_ASSERT(sizeof(GameManager) == 0x3de3c);
C_ASSERT(offsetof(GameManager, antiTamperHeapJitterAllocation) == 0x0);
C_ASSERT(offsetof(GameManager, scriptedUpdateFreeze) == 0x2c);
C_ASSERT(offsetof(GameManager, catkData) == 0x40);
C_ASSERT(offsetof(GameManager, plst) == 0x3D804);
C_ASSERT(offsetof(GameManager, character) == 0x3DBA8);
C_ASSERT(offsetof(GameManager, shotType) == 0x3DBA9);
C_ASSERT(offsetof(GameManager, flags) == 0x3DBAC);
C_ASSERT(offsetof(GameManager, currentSpellCardNumber) == 0x3DBB0);
C_ASSERT(offsetof(GameManager, isInGameMenu) == 0x3DBB2);
C_ASSERT(offsetof(GameManager, showRetryMenu) == 0x3DBB3);
C_ASSERT(offsetof(GameManager, currentDemoReplay) == 0x3DBB4);
C_ASSERT(offsetof(GameManager, replayMode) == 0x3DBB5);
C_ASSERT(offsetof(GameManager, demoFrameCount) == 0x3DBB8);
C_ASSERT(offsetof(GameManager, replayFilename) == 0x3DBBC);
C_ASSERT(offsetof(GameManager, stageRngSeed) == 0x3DDBC);
C_ASSERT(offsetof(GameManager, gameplayFrameCounter) == 0x3DDC0);
C_ASSERT(offsetof(GameManager, currentStage) == 0x3DDC4);
C_ASSERT(offsetof(GameManager, stageAtStart) == 0x3DDC8);
C_ASSERT(offsetof(GameManager, unconsumedDword3DDCC) == 0x3DDCC);
C_ASSERT(offsetof(GameManager, currentStageClearFlag) == 0x3DDD0);
C_ASSERT(offsetof(GameManager, arcadeRegionTopLeftPos) == 0x3DDD4);
C_ASSERT(offsetof(GameManager, arcadeRegionSize) == 0x3DDDC);
C_ASSERT(offsetof(GameManager, playerMovementTopLeftPos) == 0x3DDE4);
C_ASSERT(offsetof(GameManager, playerMovementAreaSize) == 0x3DDEC);
C_ASSERT(offsetof(GameManager, stagePlayTimeAll) == 0x3DE04);
C_ASSERT(offsetof(GameManager, frameSkipCounter) == 0x3DE08);
C_ASSERT(offsetof(GameManager, unconsumedDword3DE0C) == 0x3DE0C);
C_ASSERT(offsetof(GameManager, runActiveFrames) == 0x3DE10);
C_ASSERT(offsetof(GameManager, stageActiveFrames) == 0x3DE14);
C_ASSERT(offsetof(GameManager, runExtremeYoukaiFrames) == 0x3DE18);
C_ASSERT(offsetof(GameManager, runExtremeHumanFrames) == 0x3DE1C);
C_ASSERT(offsetof(GameManager, stageExtremeYoukaiFrames) == 0x3DE20);
C_ASSERT(offsetof(GameManager, stageExtremeHumanFrames) == 0x3DE24);
C_ASSERT(offsetof(GameManager, stageStartupMode) == 0x3DE28);

DIFFABLE_EXTERN(GameManager, g_GameManager);
}; // Namespace th08
