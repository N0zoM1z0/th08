#pragma once

#include "diffbuild.hpp"
#include "Global.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include "ZunMath.hpp"

#include "ScoreDat.hpp"

#include <windows.h>

#define REPLAY_MAGIC "T8RP"
#define REPLAY_VERSION 6
#define REPLAY_OBFUSCATION_VALUE 0x3f000318

namespace th08
{

struct ChainElem;

struct StageReplayData
{
    u32 score;
    i32 pointItemsCollected;
    i32 graze;
    i32 pointItemExteds;
    i32 nextPointItemExtendThreshold;
    i32 pointItemValue;
    i16 youkaiGauge;
    u16 rngSeed;
    u8 power;
    u8 lives;
    u8 bombs;
    u8 rank;
    u8 character;
    u8 unk0x21;
    i8 clockTime;

    unknown_fields(0x23, 0x1d);
};

C_ASSERT(sizeof(StageReplayData) == 0x40);

struct ReplayDataHeader
{
    u32 magic;
    u16 version;
    u8 unk0x6;
    u8 unk0x7;

    unknown_fields(0x8, 0x4);

    i32 fileSize;
    i32 checksum;

    u8 unk0x14;
    u8 value1;
    u8 unk0x16;
    u8 unk0x17;

    i32 compressedSize;
    i32 decompressedSize;

    StageReplayData *stageReplayData[MAX_STAGES];
    StageReplayData *stageReplayData2[MAX_STAGES];
};

struct ReplayData
{
    ReplayDataHeader header;

    u8 unk0x68;
    u8 minorVersion;
    u8 shotType;
    u8 difficulty;

    char date[6];
    char playerName[8];

    u8 unk0x7a;
    u8 isPractice;
    i16 spellcardNumber;

    char spellcardName[48];

    u16 majorVersion;

    u32 spellcardScore;

    GameConfiguration gameConfiguration;

    unknown_fields(0xf0, 0x24);

    float slowDownRate2;

    float slowDownRate;
    u8 clearState;

    i32 unk0x120;
    i32 exeSize;
    i32 exeChecksum;
    char exeVersion[6];
};

C_ASSERT(sizeof(ReplayData) == 0x134);

struct ReplayInputSync
{
    u16 input;
    u16 unk2;
    u16 rngSeed;
};

C_ASSERT(sizeof(ReplayInputSync) == 0x6);

struct ReplayManager
{
    ReplayManager();

    i32 frameCounter;
    i32 inputDelay;
    ReplayData *replayData;
    u8 *replayFileData;
    i32 isDemo;
    const char *replayPath;
    Float3 unk18;
    Float3 unk24;
    Float3 unk30;
    Float3 unk3c;
    unknown_fields(0x48, 0x6);
    u16 unk4e;
    u8 *replayInputs;
    u8 *replayInputStageBookmarks[MAX_STAGES];
    ReplayInputSync *replayInputs2;
    unknown_fields(0x7c, 0x24);
    u8 *replayRngInputs;
    u8 *replayInputStageBookmarks2[MAX_STAGES];
    ChainElem *calcChain;
    unknown_fields(0xcc, 0x4);
    ChainElem *frameControlChain;
    ChainElem *rngSyncChain;
    u16 rngSeed;
    u16 flags;

    static ZunResult RegisterChain(i32 replayMode, const char *replayPath);
    static ChainCallbackResult OnUpdateLowPrio(ReplayManager *replayManager);
    static ChainCallbackResult OnUpdateHighPrio(ReplayManager *replayManager);
    static ChainCallbackResult OnUpdateHighPrioDemo(ReplayManager *replayManager);
    static ChainCallbackResult OnUpdateHighPrioDemo2(ReplayManager *replayManager);
    static ChainCallbackResult OnUpdateFrameControl(ReplayManager *replayManager);
    static ZunResult AddedCallback(ReplayManager *replayManager);
    static ZunResult AddedCallbackDemo(ReplayManager *replayManager);
    static ZunResult DeletedCallback(ReplayManager *replayManager);

    static void SaveReplay(const char *replayPath, const char *replayName);
    static ReplayData *LoadReplayData(void *replayData, int fileSize);
    static void StopRecording();

    i32 IsDemo();
};

C_ASSERT(sizeof(ReplayManager) == 0xdc);
C_ASSERT(offsetof(ReplayManager, replayData) == 0x8);
C_ASSERT(offsetof(ReplayManager, replayFileData) == 0xc);
C_ASSERT(offsetof(ReplayManager, replayPath) == 0x14);
C_ASSERT(offsetof(ReplayManager, unk18) == 0x18);
C_ASSERT(offsetof(ReplayManager, unk4e) == 0x4e);
C_ASSERT(offsetof(ReplayManager, replayInputs) == 0x50);
C_ASSERT(offsetof(ReplayManager, replayInputStageBookmarks) == 0x54);
C_ASSERT(offsetof(ReplayManager, replayInputs2) == 0x78);
C_ASSERT(offsetof(ReplayManager, replayRngInputs) == 0xa0);
C_ASSERT(offsetof(ReplayManager, replayInputStageBookmarks2) == 0xa4);
C_ASSERT(offsetof(ReplayManager, calcChain) == 0xc8);
C_ASSERT(offsetof(ReplayManager, frameControlChain) == 0xd0);
C_ASSERT(offsetof(ReplayManager, rngSyncChain) == 0xd4);
C_ASSERT(offsetof(ReplayManager, rngSeed) == 0xd8);
C_ASSERT(offsetof(ReplayManager, flags) == 0xda);

DIFFABLE_EXTERN(ReplayManager *, g_ReplayManager);

} // namespace th08
