#pragma once

#include "SpellCard.hpp"
#include "Supervisor.hpp"
#include "utils.hpp"
#include <windows.h>

#define SCORE_DAT_VERSION 1

#define SCORE_DAT_XOR_VALUE 0x59
#define SCORE_DAT_XOR_VALUE_INCREMENT 0x79
#define SCORE_DAT_CHUNK_SIZE 0x100
#define SCORE_DAT_MAX_BYTES 0xc00

#define TH8K_MAGIC MAKE_FOURCC('T', 'H', '8', 'K')

#define VRSM_MAGIC MAKE_FOURCC('V', 'R', 'S', 'M')
#define VRSM_VERSION 1

#define CATK_MAGIC MAKE_FOURCC('C', 'A', 'T', 'K')
#define CATK_VERSION 3

#define CLRD_MAGIC MAKE_FOURCC('C', 'L', 'R', 'D')
#define CLRD_VERSION 4

#define PSCR_MAGIC MAKE_FOURCC('P', 'S', 'C', 'R')
#define PSCR_VERSION 2

#define PLST_MAGIC MAKE_FOURCC('P', 'L', 'S', 'T')
#define PLST_VERSION 2

#define LSNM_MAGIC MAKE_FOURCC('L', 'S', 'N', 'M')
#define LSNM_VERSION 1

#define FLSP_MAGIC MAKE_FOURCC('F', 'L', 'S', 'P')
#define FLSP_VERSION 1

#define HSCR_MAGIC MAKE_FOURCC('H', 'S', 'C', 'R')
#define HSCR_VERSION 4

namespace th08
{

enum Difficulty
{
    EASY,
    NORMAL,
    HARD,
    LUNATIC,
    EXTRA,
    MAX_DIFFICULTIES
};

enum ShotType
{
    SHOT_REIMU_YUKARI,
    SHOT_MARISA_ALICE,
    SHOT_SAKUYA_REMILIA,
    SHOT_YOUMU_YUYUKO,
    SHOT_REIMU,
    SHOT_YUKARI,
    SHOT_MARISA,
    SHOT_ALICE,
    SHOT_SAKUYA,
    SHOT_REMILIA,
    SHOT_YOUMU,
    SHOT_YUYUKO,
    SHOT_ALL = 12,
};

enum Stage
{
    STAGE1,
    STAGE2,
    STAGE3,
    STAGE4A,
    STAGE4B,
    STAGE5,
    STAGE6A,
    STAGE6B,
    EXTRASTAGE,
    MAX_STAGES,
    STAGE_LAST_WORD = MAX_STAGES,
    MAX_STAGES_AND_LAST_WORD
};

struct Th8k
{
    u32 magic;
    u16 th8kLen;
    u16 unkLen;
    u8 version;
    u8 unk_9;
};
C_ASSERT(sizeof(Th8k) == 0xC);

struct PlstPlayCounts
{
    void __fastcall IncrementTotalAttempts(u32 limit);

    u32 attemptsTotal;
    i32 attemptsPerCharacter[SHOT_ALL];
    unknown_fields(0x34, 0x4);
    i32 clears;
    i32 continues;
    i32 practices;
};
C_ASSERT(sizeof(PlstPlayCounts) == 0x44);

struct PlstPlayCountsLegacy
{
    u32 attemptsTotal;
    u32 attemptsPerCharacter[SHOT_ALL];
    unknown_fields(0x34, 0x4);
    u32 clears;
    u32 continues;
    u32 practices;
};
C_ASSERT(sizeof(PlstPlayCountsLegacy) == 0x44);

struct Plst
{
    Th8k base;
    u32 totalHours;
    u32 totalMinutes;
    u32 totalSeconds;
    u32 totalMilliseconds;
    u32 gameHours;
    u32 gameMinutes;
    u32 gameSeconds;
    u32 gameMilliseconds;
    union
    {
        struct
        {
            PlstPlayCounts playDataByDifficulty[MAX_DIFFICULTIES + 1];
            PlstPlayCounts playDataTotals;
        };
        PlstPlayCountsLegacy playData[MAX_DIFFICULTIES + 2];
    };
    i8 bgmUnlocked[32];
};

C_ASSERT(sizeof(Plst) == 0x228);

struct Flsp
{
    Th8k base;
    BYTE unlockedLastWordSpellCards[SPELLCARD_COUNT_LAST_WORD_SPELLCARDS];
};

C_ASSERT(sizeof(Flsp) == 0x20);

struct CatkHistory
{
    i32 maxBonus[SHOT_ALL + 1];
    u32 attempts[SHOT_ALL + 1];
    u32 captures[SHOT_ALL + 1];
};

struct Catk
{
    Th8k base;
    u16 spellcardNumber;
    u8 unk0xe;
    u8 difficulty;

    char spellName[48];
    char spellOwnerName[48];
    char spellCommentLine1[64];
    char spellCommentLine2[64];
    CatkHistory inGameHistory;
    CatkHistory spellPracticeHistory;
    i32 unk0x228;

    ZunBool WasAttemptedWithShot(i32 shotType);
};

C_ASSERT(sizeof(Catk) == 0x22c);

struct Clrd
{
    Th8k base;
    u16 difficultiesClearedWithoutRetries[5];
    u16 difficultiesClearedWithRetries[5];
    bool unk_20;
    u8 shotNumber;
};

C_ASSERT(sizeof(Clrd) == 0x24);

struct Pscr
{
    Th8k base;

    i32 attempts[MAX_STAGES][MAX_DIFFICULTIES];
    i32 highScores[MAX_STAGES][MAX_DIFFICULTIES];
    u8 shotNumber;
    u8 unk0x175;
};

C_ASSERT(sizeof(Pscr) == 0x178);

struct Hscr
{
    Th8k base;
    u32 score;
    f32 lagPercentage;
    u8 character;
    u8 difficulty;
    u8 stage;
    char name[9];
    char date[6];
    i8 numRetries;
    u8 unk0x27;
    GameConfiguration cfg;
    i32 playtimeFrames;
    i32 numPointItemsCollected;
    i32 unk_6c;
    i32 numDeaths;
    i32 numBombsUsed;
    i32 numLastSpells;
    i32 numPauses;
    i32 numTimeOrbsCollected;
    i32 humanityRate;
    u8 spellCounters[SPELLCARD_COUNT_SPELLCARDS];
    u8 unk0x166;
    u8 unk0x167;
};

C_ASSERT(sizeof(Hscr) == 0x168);

struct Lsnm
{
    Th8k base;
    char name[9];
};

C_ASSERT(sizeof(Lsnm) == 0x18);

struct Vrsm
{
    Th8k base;
    char version[8];
    u32 exeSize;
    u32 exeChecksum;
};

struct ScoreListNode
{
    ScoreListNode *prev;
    ScoreListNode *next;
    Hscr *data;

    ScoreListNode()
    {
        this->prev = NULL;
        this->next = NULL;
        this->data = NULL;
    }
};

C_ASSERT(sizeof(ScoreListNode) == 0xc);

struct ScoreDat
{
    static i32 LinkScore(ScoreListNode *prevNode, Hscr *newScore);
    static void FreeAllScores(ScoreListNode *score);
    static ScoreDat *OpenScore(const char *filename);
    static u32 GetHighScore(ScoreDat *score, ScoreListNode *node, u32 character, u32 difficulty, u8 *continuesUsed);
    static i32 ParseCATK(ScoreDat *score, Catk *outCatk);
    static i32 ParseLSNM(ScoreDat *score, Lsnm *outLsnm);
    static i32 ParseFLSP(ScoreDat *score, Flsp *outFlsp);
    static i32 ParseCLRD(ScoreDat *score, Clrd *outClrd);
    static i32 ParsePSCR(ScoreDat *score, Pscr *outPscr);
    static i32 ParsePLST(ScoreDat *score, Plst *outPlst);
    static void ReleaseScore(ScoreDat *score);

    u8 unk0x0;
    u8 rngValue1;
    u16 checksum;
    u16 version;
    u8 rngValue2;
    u32 headerSize;
    ScoreListNode *scores;
    u32 decompressedFileSize;
    u32 decompressedFileSizeMinusHeader;
    u32 compressedFileSize;
};

C_ASSERT(sizeof(ScoreDat) == 0x1c);

} /* namespace th08 */
