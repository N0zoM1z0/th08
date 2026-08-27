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

enum ReplayManagerMode
{
    REPLAY_MANAGER_RECORD = 0,
    REPLAY_MANAGER_PLAYBACK = 1,
};

struct StageReplayData
{
    u32 score;
    i32 pointItemsCollected;
    i32 graze;
    i32 pointItemExtends;
    i32 nextPointItemExtendThreshold;
    i32 pointItemValue;
    i16 youkaiGauge;
    u16 rngSeed;
    u8 power;
    u8 lives;
    u8 bombs;
    u8 rank;
    u8 character;
    u8 spellcardsCaptured;
    i8 clockTime;
    u8 padding23;
    u8 inputStream[0x1c];
};

C_ASSERT(sizeof(StageReplayData) == 0x40);
C_ASSERT(offsetof(StageReplayData, spellcardsCaptured) == 0x21);
C_ASSERT(offsetof(StageReplayData, inputStream) == 0x24);

struct ReplayDataHeader
{
    u32 magic;
    u16 version;
    u8 usesExtendedInputRecords;
    u8 hasUserDataSection;

    u8 reserved08[4];

    i32 fileSize;
    i32 checksum;

    u8 randomHeaderByte;
    u8 obfuscationKey;
    u8 reserved16[2];

    i32 compressedSize;
    i32 decompressedSize;

    StageReplayData *stageReplayData[MAX_STAGES];
    u8 *stageFpsData[MAX_STAGES];
};
C_ASSERT(sizeof(ReplayDataHeader) == 0x68);
C_ASSERT(offsetof(ReplayDataHeader, usesExtendedInputRecords) == 0x6);
C_ASSERT(offsetof(ReplayDataHeader, hasUserDataSection) == 0x7);
C_ASSERT(offsetof(ReplayDataHeader, obfuscationKey) == 0x15);
C_ASSERT(offsetof(ReplayDataHeader, stageReplayData) == 0x20);
C_ASSERT(offsetof(ReplayDataHeader, stageFpsData) == 0x44);

struct ReplayData
{
    ReplayDataHeader header;

    u8 randomPayloadByte;
    u8 minorVersion;
    u8 shotType;
    u8 difficulty;

    char date[6];
    char playerName[8];

    u8 reserved7A;
    u8 isPractice;
    i16 spellcardNumber;

    char spellcardName[48];

    u16 majorVersion;

    u32 spellcardScore;

    GameConfiguration gameConfiguration;

    u8 reservedF0[0x24];

    float slowDownRate2;

    float slowDownRate;
    u8 clearState;

    i32 unconsumedConstant30;
    i32 exeSize;
    i32 exeChecksum;
    char exeVersion[6];
};

C_ASSERT(sizeof(ReplayData) == 0x134);
C_ASSERT(offsetof(ReplayData, randomPayloadByte) == 0x68);
C_ASSERT(offsetof(ReplayData, reservedF0) == 0xF0);
C_ASSERT(offsetof(ReplayData, unconsumedConstant30) == 0x120);

struct ReplayInputSync
{
    u16 input;
    u16 eventFlags;
    u16 rngSeed;
};

C_ASSERT(sizeof(ReplayInputSync) == 0x6);
C_ASSERT(offsetof(ReplayInputSync, eventFlags) == 0x2);
C_ASSERT(offsetof(ReplayInputSync, rngSeed) == 0x4);

struct ReplayManager
{
    ReplayManager();

    i32 frameCounter;
    i32 inputDelay;
    ReplayData *replayData;
    u8 *replayFileData;
    i32 isDemo;
    const char *replayPath;
    Float3 unknownVector18;
    Float3 unknownVector24;
    Float3 unknownVector30;
    Float3 unknownVector3C;
    unknown_fields(0x48, 0x6);
    u16 stageResetWord;
    u8 *replayInputCursor;
    u8 *replayInputEnds[MAX_STAGES];
    ReplayInputSync *extendedInputCursor;
    unknown_fields(0x7c, 0x24);
    u8 *replayFpsSampleCursor;
    u8 *replayFpsSampleEnds[MAX_STAGES];
    ChainElem *calcChain;
    unknown_fields(0xcc, 0x4);
    ChainElem *playbackFrameControlChain;
    ChainElem *frameSyncChain;
    u16 frameRngSeed;
    u16 frameEventFlags;

    static ZunResult RegisterChain(i32 replayMode, const char *replayPath);
    static ChainCallbackResult CaptureFrameSyncState(ReplayManager *replayManager);
    static ChainCallbackResult RecordInputAndFps(ReplayManager *replayManager);
    static ChainCallbackResult PlaybackInputAndFps(ReplayManager *replayManager);
    static ChainCallbackResult PlaybackExtendedInputAndFps(ReplayManager *replayManager);
    static ChainCallbackResult ControlPlaybackFrameAdvance(ReplayManager *replayManager);
    static ZunResult BeginRecordingStage(ReplayManager *replayManager);
    static ZunResult BeginPlaybackStage(ReplayManager *replayManager);
    static ZunResult DeleteReplayManager(ReplayManager *replayManager);

    static void SaveReplay(const char *replayPath, const char *replayName);
    static ReplayData *LoadReplayData(void *replayData, int fileSize);
    static void StopRecording();

    i32 IsDemo();
};

C_ASSERT(sizeof(ReplayManager) == 0xdc);
C_ASSERT(offsetof(ReplayManager, replayData) == 0x8);
C_ASSERT(offsetof(ReplayManager, replayFileData) == 0xc);
C_ASSERT(offsetof(ReplayManager, replayPath) == 0x14);
C_ASSERT(offsetof(ReplayManager, stageResetWord) == 0x4e);
C_ASSERT(offsetof(ReplayManager, replayInputCursor) == 0x50);
C_ASSERT(offsetof(ReplayManager, replayInputEnds) == 0x54);
C_ASSERT(offsetof(ReplayManager, extendedInputCursor) == 0x78);
C_ASSERT(offsetof(ReplayManager, replayFpsSampleCursor) == 0xa0);
C_ASSERT(offsetof(ReplayManager, replayFpsSampleEnds) == 0xa4);
C_ASSERT(offsetof(ReplayManager, calcChain) == 0xc8);
C_ASSERT(offsetof(ReplayManager, playbackFrameControlChain) == 0xd0);
C_ASSERT(offsetof(ReplayManager, frameSyncChain) == 0xd4);
C_ASSERT(offsetof(ReplayManager, frameRngSeed) == 0xd8);
C_ASSERT(offsetof(ReplayManager, frameEventFlags) == 0xda);

DIFFABLE_EXTERN(ReplayManager *, g_ReplayManager);

} // namespace th08
