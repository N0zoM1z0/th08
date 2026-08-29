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
    u8 serializedReserved23;
    u8 inputStream[0x1c];
};

TH08_FILE_ASSERT(sizeof(StageReplayData) == 0x40);
TH08_FILE_ASSERT(offsetof(StageReplayData, spellcardsCaptured) == 0x21);
TH08_FILE_ASSERT(offsetof(StageReplayData, clockTime) == 0x22);
TH08_FILE_ASSERT(offsetof(StageReplayData, serializedReserved23) == 0x23);
TH08_FILE_ASSERT(offsetof(StageReplayData, inputStream) == 0x24);

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

#ifdef TH08_PORTABLE_NATIVE_LAYOUT
    u32 stageReplayDataOffsets[MAX_STAGES];
    u32 stageFpsDataOffsets[MAX_STAGES];
#else
    StageReplayData *stageReplayData[MAX_STAGES];
    u8 *stageFpsData[MAX_STAGES];
#endif
};
TH08_FILE_ASSERT(sizeof(ReplayDataHeader) == 0x68);
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, usesExtendedInputRecords) == 0x6);
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, hasUserDataSection) == 0x7);
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, obfuscationKey) == 0x15);
#ifdef TH08_PORTABLE_NATIVE_LAYOUT
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, stageReplayDataOffsets) == 0x20);
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, stageFpsDataOffsets) == 0x44);
#else
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, stageReplayData) == 0x20);
TH08_FILE_ASSERT(offsetof(ReplayDataHeader, stageFpsData) == 0x44);
#endif

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

#ifdef TH08_PORTABLE_NATIVE_LAYOUT
    StageReplayData *runtimeStageReplayData[MAX_STAGES];
    u8 *runtimeStageFpsData[MAX_STAGES];
#endif
};

#ifdef TH08_PORTABLE_NATIVE_LAYOUT
#define TH08_REPLAY_HEADER_SIZE 0x68
#define TH08_REPLAY_DATA_SIZE 0x134
#define TH08_REPLAY_STAGE_DATA(replay, stage) ((replay)->runtimeStageReplayData[(stage)])
#define TH08_REPLAY_FPS_DATA(replay, stage) ((replay)->runtimeStageFpsData[(stage)])
#else
#define TH08_REPLAY_HEADER_SIZE sizeof(ReplayDataHeader)
#define TH08_REPLAY_DATA_SIZE sizeof(ReplayData)
#define TH08_REPLAY_STAGE_DATA(replay, stage) replay->header.stageReplayData[stage]
#define TH08_REPLAY_FPS_DATA(replay, stage) replay->header.stageFpsData[stage]
#endif

TH08_FILE_ASSERT(offsetof(ReplayData, randomPayloadByte) == 0x68);
TH08_FILE_ASSERT(offsetof(ReplayData, reservedF0) == 0xF0);
TH08_FILE_ASSERT(offsetof(ReplayData, unconsumedConstant30) == 0x120);
#ifdef TH08_PORTABLE_NATIVE_LAYOUT
TH08_FILE_ASSERT(offsetof(ReplayData, runtimeStageReplayData) >= 0x134);
#else
TH08_FILE_ASSERT(sizeof(ReplayData) == 0x134);
#endif

struct ReplayInputSync
{
    u16 input;
    u16 eventFlags;
    u16 rngSeed;
};

TH08_FILE_ASSERT(sizeof(ReplayInputSync) == 0x6);
TH08_FILE_ASSERT(offsetof(ReplayInputSync, eventFlags) == 0x2);
TH08_FILE_ASSERT(offsetof(ReplayInputSync, rngSeed) == 0x4);

struct ReplayManager
{
    ReplayManager();

    i32 frameCounter;
    i32 inputDelay;
    ReplayData *replayData;
    u8 *replayFileData;
    i32 isDemo;
    const char *replayPath;
    Float3 unconsumedVector18;
    Float3 unconsumedVector24;
    Float3 unconsumedVector30;
    Float3 unconsumedVector3C;
    u8 unconsumedBytes48[0x6];
    u16 stageResetWord;
    u8 *replayInputCursor;
    u8 *replayInputEnds[MAX_STAGES];
    ReplayInputSync *extendedInputCursor;
    u8 unconsumedBytes7C[0x24];
    u8 *replayFpsSampleCursor;
    u8 *replayFpsSampleEnds[MAX_STAGES];
    ChainElem *calcChain;
    u8 unconsumedBytesCC[0x4];
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
    static ReplayData *LoadReplayData(ReplayData *replayData, int fileSize);
    static void StopRecording();

    i32 IsDemo();
};

C_ASSERT(sizeof(ReplayManager) == 0xdc);
C_ASSERT(offsetof(ReplayManager, replayData) == 0x8);
C_ASSERT(offsetof(ReplayManager, replayFileData) == 0xc);
C_ASSERT(offsetof(ReplayManager, replayPath) == 0x14);
C_ASSERT(offsetof(ReplayManager, unconsumedVector18) == 0x18);
C_ASSERT(offsetof(ReplayManager, unconsumedVector24) == 0x24);
C_ASSERT(offsetof(ReplayManager, unconsumedVector30) == 0x30);
C_ASSERT(offsetof(ReplayManager, unconsumedVector3C) == 0x3c);
C_ASSERT(offsetof(ReplayManager, unconsumedBytes48) == 0x48);
C_ASSERT(offsetof(ReplayManager, stageResetWord) == 0x4e);
C_ASSERT(offsetof(ReplayManager, replayInputCursor) == 0x50);
C_ASSERT(offsetof(ReplayManager, replayInputEnds) == 0x54);
C_ASSERT(offsetof(ReplayManager, extendedInputCursor) == 0x78);
C_ASSERT(offsetof(ReplayManager, unconsumedBytes7C) == 0x7c);
C_ASSERT(offsetof(ReplayManager, replayFpsSampleCursor) == 0xa0);
C_ASSERT(offsetof(ReplayManager, replayFpsSampleEnds) == 0xa4);
C_ASSERT(offsetof(ReplayManager, calcChain) == 0xc8);
C_ASSERT(offsetof(ReplayManager, unconsumedBytesCC) == 0xcc);
C_ASSERT(offsetof(ReplayManager, playbackFrameControlChain) == 0xd0);
C_ASSERT(offsetof(ReplayManager, frameSyncChain) == 0xd4);
C_ASSERT(offsetof(ReplayManager, frameRngSeed) == 0xd8);
C_ASSERT(offsetof(ReplayManager, frameEventFlags) == 0xda);

DIFFABLE_EXTERN(ReplayManager *, g_ReplayManager);

} // namespace th08
