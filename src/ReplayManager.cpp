#include "th_pch.h"

#include "Global.hpp"
#include "EnemyManager.hpp"
#include "Gui.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "i18n.hpp"

#include "pbg/Lzss.hpp"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

namespace th08
{

DIFFABLE_STATIC(ReplayManager *, g_ReplayManager);

namespace
{
const char *g_ReplayDifficultyList[] = {"Easy", "Normal", "Hard", "Lunatic", "Extra"};

struct ReplayUserDataHeader
{
    u32 magic;
    i32 size;
    u8 unk0x8;
    u8 padding[3];
};

C_ASSERT(sizeof(ReplayUserDataHeader) == 0xc);

char *AppendFormat(char *buffer, const char *format, ...);
} // namespace

#pragma var_order(decodedReplay, i, replayData, obfuscateOffset, obfuscateCursor, checksum, checksumCursor)
ReplayData *ReplayManager::LoadReplayData(void *data, int fileSize)
{
    u8 *obfuscateCursor;
    u8 obfuscateOffset;
    u8 *checksumCursor;
    u32 checksum;
    i32 i;
    ReplayData *decodedReplay;
    ReplayData *replayData = (ReplayData *)data;

    if (replayData == NULL)
    {
        goto err1;
    }

    if (replayData->header.magic != *(u32 *)REPLAY_MAGIC)
    {
        goto err1;
    }

    if (replayData->header.version != REPLAY_VERSION)
    {
        goto err1;
    }

    obfuscateCursor = (u8 *)&replayData->header.compressedSize;
    obfuscateOffset = replayData->header.obfuscationKey;

    for (i = 0; i < replayData->header.fileSize - (i32)offsetof(ReplayDataHeader, compressedSize);
         i++, obfuscateCursor++)
    {
        *obfuscateCursor -= obfuscateOffset;
        obfuscateOffset += 7;
    }

    checksumCursor = &replayData->header.obfuscationKey;
    checksum = REPLAY_OBFUSCATION_VALUE;

    for (i = 0; i < replayData->header.fileSize - (i32)offsetof(ReplayDataHeader, obfuscationKey);
         i++, checksumCursor++)
    {
        checksum += *checksumCursor;
    }

    if (checksum != replayData->header.checksum)
    {
        goto err1;
    }

    decodedReplay = (ReplayData *)g_ZunMemory.Alloc(replayData->header.decompressedSize + sizeof(ReplayDataHeader) +
                                                    (fileSize - replayData->header.fileSize));

    memcpy(&decodedReplay->header, data, sizeof(ReplayDataHeader));

    Lzss::Decode((u8 *)replayData + sizeof(ReplayDataHeader), replayData->header.compressedSize,
                 (u8 *)decodedReplay + sizeof(ReplayDataHeader), replayData->header.decompressedSize);

    memcpy((u8 *)decodedReplay + sizeof(ReplayDataHeader) + replayData->header.decompressedSize,
           (u8 *)data + replayData->header.fileSize, fileSize - replayData->header.fileSize);

    replayData = decodedReplay;

    if (replayData->gameConfiguration.slowMode != 0)
    {
        goto err2;
    }

    if (g_Supervisor.CheckVersion(replayData->exeVersion, replayData->exeSize, replayData->exeChecksum) != ZUN_SUCCESS)
    {
        goto err2;
    }

    g_ZunMemory.Free(data);

    return decodedReplay;

err1:
    g_ZunMemory.Free(data);
    return NULL;

err2:
    g_ZunMemory.Free(data);
    g_ZunMemory.Free(decodedReplay);

    return NULL;
}

ZunResult ReplayManager::RegisterChain(i32 replayMode, const char *replayPath)
{
    ReplayManager *replayManager;

    g_GuiMessageInputPrevious = 0;
    g_GuiMessageInputCurrent = 0;

    if (g_ReplayManager == NULL)
    {
        replayManager = ZUN_NEW(ReplayManager, "ReplayInf");
        g_ReplayManager = replayManager;
        memset(replayManager, 0, sizeof(ReplayManager));
        replayManager->replayData = NULL;
        replayManager->isDemo = replayMode;
        replayManager->replayPath = replayPath;

        switch (replayMode)
        {
        case REPLAY_MANAGER_RECORD:
            replayManager->calcChain = g_Chain.CreateElem((ChainCallback)RecordInputAndFps);
            replayManager->calcChain->addedCallback = (ChainLifetimeCallback)BeginRecordingStage;
            replayManager->calcChain->deletedCallback = (ChainLifetimeCallback)DeleteReplayManager;
            replayManager->calcChain->arg = replayManager;

            if (g_Chain.AddToCalcChain(replayManager->calcChain, CHAIN_PRIO_CALC_REPLAYMANAGER_RECORD_HIGH_PRIO))
            {
                return ZUN_ERROR;
            }

            replayManager->playbackFrameControlChain = NULL;
            replayManager->frameSyncChain = g_Chain.CreateElem((ChainCallback)CaptureFrameSyncState);
            replayManager->frameSyncChain->arg = replayManager;
            g_Chain.AddToCalcChain(replayManager->frameSyncChain, CHAIN_PRIO_CALC_REPLAYMANAGER_LOW_PRIO);
            CaptureFrameSyncState(replayManager);
            break;

        case REPLAY_MANAGER_PLAYBACK:
            replayManager->calcChain = g_Chain.CreateElem((ChainCallback)PlaybackInputAndFps);
            replayManager->calcChain->addedCallback = (ChainLifetimeCallback)BeginPlaybackStage;
            replayManager->calcChain->deletedCallback = (ChainLifetimeCallback)DeleteReplayManager;
            replayManager->calcChain->arg = replayManager;

            if (g_Chain.AddToCalcChain(replayManager->calcChain, CHAIN_PRIO_CALC_REPLAYMANAGER_PLAYBACK_HIGH_PRIO))
            {
                return ZUN_ERROR;
            }

            if (replayManager->replayData->header.usesExtendedInputRecords != 0)
            {
                replayManager->calcChain->callback = (ChainCallback)PlaybackExtendedInputAndFps;
            }

            replayManager->playbackFrameControlChain =
                g_Chain.CreateElem((ChainCallback)ControlPlaybackFrameAdvance);
            replayManager->playbackFrameControlChain->arg = replayManager;
            g_Chain.AddToCalcChain(
                replayManager->playbackFrameControlChain, CHAIN_PRIO_CALC_REPLAYMANAGER_SKIP_FRAMES);

            replayManager->frameSyncChain = NULL;
            if (replayManager->replayData->header.usesExtendedInputRecords != 0)
            {
                replayManager->frameSyncChain = g_Chain.CreateElem((ChainCallback)CaptureFrameSyncState);
                replayManager->frameSyncChain->arg = replayManager;
                g_Chain.AddToCalcChain(replayManager->frameSyncChain, CHAIN_PRIO_CALC_REPLAYMANAGER_LOW_PRIO);
                CaptureFrameSyncState(replayManager);
            }
            break;
        }
    }
    else
    {
        switch (replayMode)
        {
        case REPLAY_MANAGER_RECORD:
            BeginRecordingStage(g_ReplayManager);
            break;
        case REPLAY_MANAGER_PLAYBACK:
            BeginPlaybackStage(g_ReplayManager);
            break;
        }
    }

    return ZUN_SUCCESS;
}

ChainCallbackResult ReplayManager::CaptureFrameSyncState(ReplayManager *replayManager)
{
    replayManager->frameEventFlags = 0;
    replayManager->frameRngSeed = g_Rng.GetSeed();
    g_Rng.ResetGenerationCount();

    if (g_GameManager.replayPauseRecorded != 0)
    {
        replayManager->frameEventFlags |= 0x100;
    }

    g_GameManager.replayPauseRecorded = 0;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(stage, input)
ChainCallbackResult ReplayManager::RecordInputAndFps(ReplayManager *replayManager)
{
    i32 stage;
    u16 input;

    if (!g_GameManager.flags.replayInputEnabled)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    g_GuiMessageInputPrevious = g_GuiMessageInputCurrent;
    g_GuiMessageInputCurrent = g_CurFrameInput;

    if (g_GameManager.cfg->slowMode != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_Supervisor.IsSpeedhackDetected())
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.stageClearSequenceActive)
    {
        if (replayManager->inputDelay >= 3)
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }

        replayManager->inputDelay++;
    }

    stage = g_GameManager.stageAtStart;
    input = g_CurFrameInput;
    g_GuiMessageInputCurrent = input;

    replayManager->replayInputCursor += sizeof(u16);
    replayManager->replayInputEnds[stage] = replayManager->replayInputCursor + sizeof(u16);
    *(u16 *)replayManager->replayInputCursor = input;

    if (replayManager->frameCounter % 30 == 0)
    {
        replayManager->replayFpsSampleCursor[0] =
            (u8)g_Supervisor.recordedFps | (g_Supervisor.recordingFpsWarning != 0 ? 0x80 : 0);
        replayManager->replayFpsSampleCursor[1] = (u8)g_Supervisor.recordedFps;
        replayManager->replayFpsSampleEnds[stage] = replayManager->replayFpsSampleCursor + 2;
        replayManager->replayFpsSampleCursor++;
    }

    replayManager->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::ControlPlaybackFrameAdvance(ReplayManager *replayManager)
{
    if (!g_GameManager.flags.replayInputEnabled)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.stageClearSequenceActive)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_Gui.IsDialoguePresent() && g_Gui.IsDialogueSkippable() && replayManager->frameCounter % 3 != 2)
    {
        return CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB;
    }

    if (!g_GameManager.flags.stageClearSequenceActive && g_GameManager.replayMode == 2 && !g_EnemyManager.HasBoss() &&
        replayManager->frameCounter % 5 != 4)
    {
        return CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::PlaybackInputAndFps(ReplayManager *replayManager)
{
    i32 unused;

    if (!g_GameManager.flags.replayInputEnabled)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.cfg->slowMode != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.stageClearSequenceActive)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    unused = 0;
    g_GuiMessageInputPrevious = g_GuiMessageInputCurrent;
    g_GuiMessageInputCurrent = *(u16 *)replayManager->replayInputCursor;
    replayManager->replayInputCursor += sizeof(u16);

    g_IsEighthFrameOfHeldInput = 0;
    if (g_GuiMessageInputPrevious == g_GuiMessageInputCurrent)
    {
        if (g_NumOfFramesInputsWereHeld >= 0x1e)
        {
            if (g_NumOfFramesInputsWereHeld % 8 == 0)
            {
                g_IsEighthFrameOfHeldInput = 1;
            }
            if (0x26 <= g_NumOfFramesInputsWereHeld)
            {
                g_NumOfFramesInputsWereHeld = 0x1e;
            }
        }

        g_NumOfFramesInputsWereHeld++;
    }
    else
    {
        g_NumOfFramesInputsWereHeld = 0;
    }

    if (replayManager->frameCounter % 30 == 0)
    {
        g_Supervisor.recordedFps = (i8)replayManager->replayFpsSampleCursor[1] & 0x7f;
        g_Supervisor.playbackFpsWarning = (i8)replayManager->replayFpsSampleCursor[1] >> 7;
        replayManager->replayFpsSampleCursor++;
    }

    replayManager->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::PlaybackExtendedInputAndFps(ReplayManager *replayManager)
{
    i32 unused;

    if (!g_GameManager.flags.replayInputEnabled)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.cfg->slowMode != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.stageClearSequenceActive)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    unused = 0;
    g_GuiMessageInputPrevious = g_GuiMessageInputCurrent;
    g_GuiMessageInputCurrent = replayManager->extendedInputCursor->input;
    replayManager->extendedInputCursor++;

    g_IsEighthFrameOfHeldInput = 0;
    if (g_GuiMessageInputPrevious == g_GuiMessageInputCurrent)
    {
        if (g_NumOfFramesInputsWereHeld >= 0x1e)
        {
            if (g_NumOfFramesInputsWereHeld % 8 == 0)
            {
                g_IsEighthFrameOfHeldInput = 1;
            }
            if (0x26 <= g_NumOfFramesInputsWereHeld)
            {
                g_NumOfFramesInputsWereHeld = 0x1e;
            }
        }

        g_NumOfFramesInputsWereHeld++;
    }
    else
    {
        g_NumOfFramesInputsWereHeld = 0;
    }

    if (replayManager->frameCounter % 30 == 0)
    {
        g_Supervisor.recordedFps = (i8)replayManager->replayFpsSampleCursor[1] & 0x7f;
        g_Supervisor.playbackFpsWarning = (i8)replayManager->replayFpsSampleCursor[1] >> 7;
        replayManager->replayFpsSampleCursor++;
    }

    replayManager->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(stageData, stage, stageFpsData, previousStage)
ZunResult ReplayManager::BeginRecordingStage(ReplayManager *replayManager)
{
    StageReplayData *previousStage;
    StageReplayData *stageFpsData;
    i32 stage;
    StageReplayData *stageData;

    replayManager->frameCounter = 0;
    replayManager->replayFileData = NULL;

    if (replayManager->replayData == NULL)
    {
        replayManager->replayData = (ReplayData *)g_ZunMemory.AddToRegistry(
            new ReplayData, sizeof(ReplayData), "ReplayDataInf");
        memset(replayManager->replayData, 0, sizeof(ReplayData));

        replayManager->replayData->header.magic = *(u32 *)REPLAY_MAGIC;
        replayManager->replayData->header.hasUserDataSection = 0;
        replayManager->replayData->shotType = g_GameManager.shotType;
        replayManager->replayData->header.version = REPLAY_VERSION;
        replayManager->replayData->header.usesExtendedInputRecords = 0;
        replayManager->replayData->majorVersion = 0x100;
        replayManager->replayData->minorVersion = 100;
        strcpy(replayManager->replayData->exeVersion, "0100d");
        replayManager->replayData->exeSize = g_Supervisor.exeSize;
        replayManager->replayData->exeChecksum = g_Supervisor.exeChecksum;
        replayManager->replayData->isPractice = (u8)g_GameManager.IsPracticeMode();
        replayManager->replayData->spellcardNumber =
            g_GameManager.flags.isSpellPractice ? g_GameManager.currentSpellCardNumber : -1;

        replayManager->replayData->clearState =
            g_GameManager.IsStageClearedWithoutRetries(7, g_GameManager.shotType, 0) ||
                    g_GameManager.IsStageClearedWithoutRetries(7, g_GameManager.shotType, 1) ||
                    g_GameManager.IsStageClearedWithoutRetries(7, g_GameManager.shotType, 2) ||
                    g_GameManager.IsStageClearedWithoutRetries(7, g_GameManager.shotType, 3) ||
                    g_GameManager.shotType > 3
                ? 2
                : (g_GameManager.IsStageClearedWithoutRetries(6, g_GameManager.shotType, 0) ||
                   g_GameManager.IsStageClearedWithoutRetries(6, g_GameManager.shotType, 1) ||
                   g_GameManager.IsStageClearedWithoutRetries(6, g_GameManager.shotType, 2) ||
                   g_GameManager.IsStageClearedWithoutRetries(6, g_GameManager.shotType, 3));

        replayManager->replayData->difficulty = (u8)g_GameManager.difficulty;
        memcpy(replayManager->replayData->playerName, "NO NAME", 4);
        replayManager->replayData->gameConfiguration = *g_GameManager.cfg;

        for (stage = 0; stage < MAX_STAGES; stage++)
        {
            replayManager->replayData->header.stageReplayData[stage] = NULL;
            replayManager->replayData->header.stageReplayFpsData[stage] = NULL;
        }
    }
    else
    {
        previousStage = NULL;
        for (stage = 0; stage < g_GameManager.currentStage; stage++)
        {
            if (replayManager->replayData->header.stageReplayData[stage] != NULL)
            {
                previousStage = replayManager->replayData->header.stageReplayData[stage];
            }
        }

        if (previousStage != NULL)
        {
            previousStage->score = g_GameManager.globals->score;
        }
    }

    stage = g_GameManager.currentStage;
    if (replayManager->replayData->header.stageReplayData[stage] != NULL)
    {
        g_ZunMemory.Free(replayManager->replayData->header.stageReplayData[stage]);
    }
    if (replayManager->replayData->header.stageReplayFpsData[stage] != NULL)
    {
        g_ZunMemory.Free(replayManager->replayData->header.stageReplayFpsData[stage]);
    }

    replayManager->replayData->header.stageReplayData[stage] =
        (StageReplayData *)g_ZunMemory.Alloc(0xd2f00, "rep data");
    replayManager->replayData->header.stageReplayFpsData[stage] =
        (StageReplayData *)g_ZunMemory.Alloc(0xd2f00, "rep data");

    stageData = replayManager->replayData->header.stageReplayData[stage];
    stageFpsData = replayManager->replayData->header.stageReplayFpsData[stage];

    stageData->graze = g_GameManager.globals->graze;
    stageData->bombs = (u8)g_GameManager.GetBombsRemaining();
    stageData->lives = (u8)g_GameManager.GetLives();
    stageData->power = (u8)g_GameManager.GetPower();
    stageData->rank = (u8)g_GameManager.rank;
    stageData->pointItemsCollected = g_GameManager.globals->pointItemsCollected;
    stageData->rngSeed = g_GameManager.stageRngSeed;
    stageData->character = g_GameManager.character;
    stageData->spellcardsCaptured = (u8)g_GameManager.globals->spellcardsCaptured;
    stageData->pointItemExtends = g_GameManager.globals->pointItemExtendsSoFar;
    stageData->nextPointItemExtendThreshold = g_GameManager.globals->nextPointItemExtendThreshold;
    stageData->youkaiGauge = g_GameManager.globals->youkaiGauge;
    stageData->clockTime = g_GameManager.GetClockTime();
    stageData->pointItemValue = g_GameManager.globals->pointItemValue;

    replayManager->replayInputCursor = stageData->inputData;
    replayManager->extendedInputCursor = reinterpret_cast<ReplayInputSync *>(replayManager->replayInputCursor);
    replayManager->replayFpsSampleCursor = reinterpret_cast<u8 *>(stageFpsData);
    *reinterpret_cast<u16 *>(replayManager->replayInputCursor) = 0;
    replayManager->extendedInputCursor->eventFlags = 0;
    replayManager->extendedInputCursor->rngSeed = g_Rng.GetSeed();
    replayManager->inputDelay = 0;
    replayManager->unk4e = 0;

    return ZUN_SUCCESS;
}

#pragma var_order(stageData, stage, fileSize, stageFpsData, previousStage)
ZunResult ReplayManager::BeginPlaybackStage(ReplayManager *replayManager)
{
    StageReplayData *previousStage;
    StageReplayData *stageFpsData;
    i32 fileSize;
    i32 stage;
    StageReplayData *stageData;

    replayManager->frameCounter = 0;

    if (replayManager->replayData == NULL)
    {
        replayManager->replayData = (ReplayData *)FileSystem::OpenFile(
            replayManager->replayPath, &fileSize, !g_GameManager.flags.isDemoMode);
        replayManager->replayData = LoadReplayData(replayManager->replayData, fileSize);
        if (replayManager->replayData == NULL)
        {
            return ZUN_ERROR;
        }

        replayManager->replayFileData = NULL;
        for (stage = 0; stage < MAX_STAGES; stage++)
        {
            if (replayManager->replayData->header.stageReplayData[stage] != NULL)
            {
                replayManager->replayData->header.stageReplayData[stage] =
                    (StageReplayData *)(reinterpret_cast<u8 *>(replayManager->replayData->header.stageReplayData[stage]) +
                                        reinterpret_cast<i32>(replayManager->replayData));
            }
            if (replayManager->replayData->header.stageReplayFpsData[stage] != NULL)
            {
                replayManager->replayData->header.stageReplayFpsData[stage] =
                    (StageReplayData *)(reinterpret_cast<u8 *>(replayManager->replayData->header.stageReplayFpsData[stage]) +
                                        reinterpret_cast<i32>(replayManager->replayData));
            }
        }
    }

    stage = g_GameManager.currentStage;
    if (replayManager->replayData->header.stageReplayData[stage] == NULL)
    {
        return ZUN_ERROR;
    }

    stageData = replayManager->replayData->header.stageReplayData[stage];
    stageFpsData = replayManager->replayData->header.stageReplayFpsData[stage];

    g_GameManager.shotType = replayManager->replayData->shotType / 1;
    g_GameManager.fullShotType = replayManager->replayData->shotType % 1;
    g_GameManager.shotType = replayManager->replayData->shotType;
    g_GameManager.difficulty = replayManager->replayData->difficulty;

    g_GameManager.globals->pointItemsCollected = stageData->pointItemsCollected;
    g_GameManager.rank = stageData->rank;
    g_GameManager.SetLives(stageData->lives);
    g_GameManager.SetBombCount(stageData->bombs);
    g_GameManager.SetPower(stageData->power);
    g_GameManager.globals->graze = stageData->graze;

    replayManager->replayInputCursor = stageData->inputData;
    replayManager->extendedInputCursor = reinterpret_cast<ReplayInputSync *>(replayManager->replayInputCursor);
    g_GameManager.character = stageData->character;
    g_GameManager.globals->pointItemValue = stageData->pointItemValue;
    *g_GameManager.cfg = replayManager->replayData->gameConfiguration;
    g_Rng.SetSeed(stageData->rngSeed);
    g_GameManager.globals->spellcardsCaptured = stageData->spellcardsCaptured;
    g_GameManager.globals->pointItemExtendsSoFar = stageData->pointItemExtends;
    g_GameManager.globals->nextPointItemExtendThreshold = stageData->nextPointItemExtendThreshold;
    g_GameManager.globals->youkaiGauge = stageData->youkaiGauge;
    g_GameManager.SetClockTime(stageData->clockTime);

    replayManager->replayFpsSampleCursor = reinterpret_cast<u8 *>(stageFpsData);
    replayManager->inputDelay = 0;

    previousStage = NULL;
    for (stage = 0; stage < g_GameManager.currentStage; stage++)
    {
        if (replayManager->replayData->header.stageReplayData[stage] != NULL)
        {
            previousStage = replayManager->replayData->header.stageReplayData[stage];
        }
    }

    if (previousStage != NULL)
    {
        g_GameManager.globals->score = previousStage->score;
        g_GameManager.globals->displayScore = g_GameManager.globals->score;
    }

    return ZUN_SUCCESS;
}

ZunResult ReplayManager::DeleteReplayManager(ReplayManager *replayManager)
{
    if (replayManager->playbackFrameControlChain != NULL)
    {
        g_Chain.Cut(replayManager->playbackFrameControlChain);
        replayManager->playbackFrameControlChain = NULL;
    }

    if (replayManager->frameSyncChain != NULL)
    {
        g_Chain.Cut(replayManager->frameSyncChain);
        replayManager->frameSyncChain = NULL;
    }

    g_ZunMemory.Free(g_ReplayManager->replayData);

    if (replayManager->replayFileData != NULL)
    {
        g_ZunMemory.Free(replayManager->replayFileData);
    }

    ZUN_DELETE(g_ReplayManager);
    g_ReplayManager = NULL;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x453160
ReplayManager::ReplayManager()
{
}

void ReplayManager::StopRecording()
{
    ReplayManager *mgr = g_ReplayManager;
    i32 stage;

    if (mgr != NULL)
    {
        mgr->replayInputCursor += sizeof(u16);
        *(u16 *)mgr->replayInputCursor = 0;
        stage = g_GameManager.currentStage;
        mgr->replayInputEnds[stage] = mgr->replayInputCursor + 6;
    }
}

#pragma var_order(i, mgr, infoCursor, bytesWritten, compressedData, slowDownRate, compressedSize, stageSize,          \
                  tempBuffer, replayCopy, infoBuffer, file, infoHeader, currentOffset, localTime, currentTime,      \
                  dateBuffer, checksum, checksumCursor, obfuscateOffset, obfuscateCursor)
void ReplayManager::SaveReplay(const char *replayPath, const char *replayName)
{
    ReplayManager *mgr;
    u8 *tempBuffer;
    ReplayData replayCopy;
    i32 currentOffset;
    i32 stageSize;
    float slowDownRate;
    ReplayUserDataHeader infoHeader;
    char infoBuffer[1024];
    char dateBuffer[252];
    char *infoCursor;
    time_t currentTime;
    tm *localTime;
    u8 *compressedData;
    i32 compressedSize;
    u8 *checksumCursor;
    u32 checksum;
    u8 *obfuscateCursor;
    u8 obfuscateOffset;
    HANDLE file;
    DWORD bytesWritten;
    i32 i;

    if (g_ReplayManager != NULL)
    {
        mgr = g_ReplayManager;

        if (!mgr->IsDemo())
        {
            if (!g_GameManager.IsPracticeMode() && g_GameManager.difficulty < LUNATIC + 1 &&
                memcmp(&g_Supervisor.cfg, &mgr->replayData->gameConfiguration, sizeof(GameConfiguration)) != 0)
            {
                goto release_stage_data;
            }

            if (mgr->replayData->gameConfiguration.slowMode != 0)
            {
                goto release_stage_data;
            }

            if (replayPath != NULL)
            {
                utils::DebugPrint("info : Replay File write %s\r\n", replayPath);

    tempBuffer = (u8 *)g_ZunMemory.Alloc(0x400000, "rep tmp");
    replayCopy = *mgr->replayData;

    ReplayManager::StopRecording();

    i = g_GameManager.stageAtStart;
    mgr->replayData->header.stageReplayData[i]->score = g_GameManager.globals->score;

    currentOffset = sizeof(ReplayDataHeader);
    currentOffset += sizeof(ReplayData) - sizeof(ReplayDataHeader);

    for (i = 0; i < MAX_STAGES; i++)
    {
        if (mgr->replayData->header.stageReplayData[i] != NULL)
        {
            stageSize = mgr->replayInputEnds[i] - (u8 *)mgr->replayData->header.stageReplayData[i];
            memcpy(tempBuffer + currentOffset - sizeof(ReplayDataHeader),
                   mgr->replayData->header.stageReplayData[i], stageSize);
            replayCopy.header.stageReplayData[i] = (StageReplayData *)currentOffset;
            currentOffset += stageSize;
        }
    }

    for (i = 0; i < MAX_STAGES; i++)
    {
        if (mgr->replayData->header.stageReplayFpsData[i] != NULL)
        {
            stageSize = mgr->replayFpsSampleEnds[i] - (u8 *)mgr->replayData->header.stageReplayFpsData[i];
            memcpy(tempBuffer + currentOffset - sizeof(ReplayDataHeader),
                   mgr->replayData->header.stageReplayFpsData[i], stageSize);
            replayCopy.header.stageReplayFpsData[i] = (StageReplayData *)currentOffset;
            currentOffset += stageSize;
        }
    }

    replayCopy.spellcardScore = g_GameManager.globals->displayScore;

    if (replayCopy.spellcardNumber >= 0)
    {
        memcpy(replayCopy.spellcardName, g_GameManager.catkData[replayCopy.spellcardNumber].spellName,
               sizeof(replayCopy.spellcardName));
    }

    slowDownRate = ((g_Supervisor.lagNumerator / g_Supervisor.lagDenominator) - 0.5f) * 2.0f;

    if (slowDownRate < 0.0f)
    {
        slowDownRate = 0.0f;
    }
    else if (slowDownRate >= 1.0f)
    {
        slowDownRate = 1.0f;
    }

    replayCopy.slowDownRate = (1.0f - slowDownRate) * 100.0f;

    infoHeader.magic = *(u32 *)"USER";
    infoHeader.unk0x8 = 0;
    memset(infoBuffer, 0, sizeof(infoBuffer));
    infoCursor = infoBuffer;

    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_PLAYER_NAME, replayName);

    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(dateBuffer, 20, "%Y/%m/%d %H:%M:%S", localTime);

    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_PLAY_TIME, dateBuffer);
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_CHARACTER, ResultScreen::GetCharacterName(g_GameManager.shotType));
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_SCORE, g_GameManager.globals->displayScore);
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_DIFFICULTY, g_ReplayDifficultyList[g_GameManager.difficulty]);

    if (replayCopy.spellcardNumber >= 0)
    {
        infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_SPELLCARD, replayCopy.spellcardNumber + 1,
                                  replayCopy.spellcardName);
    }
    else
    {
        infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_FINAL_STAGE,
                                  g_GameManager.flags.gameCleared ? "Clear" : ResultScreen::GetStageName(g_GameManager.stageAtStart));
    }

    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_DEATHS, g_GameManager.GetDeaths());
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_BOMBS, g_GameManager.GetBombsUsed());
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_SLOWDOWN, replayCopy.slowDownRate);

    g_GameManager.hscr.humanityRate = (i32)(((float)g_GameManager.humanityRateNumerator / g_GameManager.humanityRateDenominator) * 10.0f);
    infoCursor =
        AppendFormat(infoCursor, TH_REPLAY_INFO_HUMAN_RATE, (float)g_GameManager.hscr.humanityRate / 100.0f);
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_VERSION, 1, 0, 'd');

    infoHeader.size = strlen(infoBuffer) + sizeof(ReplayUserDataHeader);
    infoHeader.size += infoHeader.size & 1;

    replayCopy.header.hasUserDataSection = 1;
    strcpy(replayCopy.playerName, replayName);
    ResultScreen::FormatDate(replayCopy.date);

    replayCopy.header.obfuscationKey = (u8)(g_Rng.GetRandomU16InRange(0x80) + 0x40);
    replayCopy.randomPayloadByte = (u8)g_Rng.GetRandomU16InRange(0x100);
    replayCopy.header.randomHeaderByte = (u8)g_Rng.GetRandomU16InRange(0x100);
    replayCopy.slowDownRate2 = replayCopy.slowDownRate + 1.12f;
    replayCopy.unk0x120 = 30;

    memcpy(tempBuffer, &replayCopy.randomPayloadByte, sizeof(ReplayData) - sizeof(ReplayDataHeader));

    utils::DebugPrint("info : original size %d\r\n", currentOffset);

    replayCopy.header.decompressedSize = currentOffset - sizeof(ReplayDataHeader);
    compressedData = Lzss::Encode(tempBuffer, replayCopy.header.decompressedSize, &replayCopy.header.compressedSize);
    g_ZunMemory.Free(tempBuffer);
    compressedSize = replayCopy.header.compressedSize;

    checksumCursor = &replayCopy.header.obfuscationKey;
    checksum = REPLAY_OBFUSCATION_VALUE;

    for (i = 0; i < sizeof(ReplayDataHeader) - offsetof(ReplayDataHeader, obfuscationKey); i++, checksumCursor++)
    {
        checksum += *checksumCursor;
    }

    checksumCursor = compressedData;

    for (i = 0; i < compressedSize; i++, checksumCursor++)
    {
        checksum += *checksumCursor;
    }

    replayCopy.header.checksum = checksum;
    obfuscateCursor = (u8 *)&replayCopy.header.compressedSize;
    obfuscateOffset = replayCopy.header.obfuscationKey;

    for (i = 0; i < sizeof(ReplayDataHeader) - offsetof(ReplayDataHeader, compressedSize); i++, obfuscateCursor++)
    {
        *obfuscateCursor += obfuscateOffset;
        obfuscateOffset += 7;
    }

    obfuscateCursor = compressedData;

    for (i = 0; i < compressedSize; i++, obfuscateCursor++)
    {
        *obfuscateCursor += obfuscateOffset;
        obfuscateOffset += 7;
    }

    replayCopy.header.fileSize = compressedSize + sizeof(ReplayDataHeader);

    file = CreateFileA(replayPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
        goto release_stage_data;
    }

    {
        WriteFile(file, &replayCopy.header, sizeof(ReplayDataHeader), &bytesWritten, NULL);
        WriteFile(file, compressedData, compressedSize, &bytesWritten, NULL);
        WriteFile(file, &infoHeader, sizeof(infoHeader), &bytesWritten, NULL);
        WriteFile(file, infoBuffer, infoHeader.size - sizeof(infoHeader), &bytesWritten, NULL);
        CloseHandle(file);

        utils::DebugPrint("info : Size %d -> %d\r\n", currentOffset, compressedSize + sizeof(ReplayDataHeader));
        GlobalFree(compressedData);
    }
            }
release_stage_data:
    for (i = 0; i < MAX_STAGES; i++)
    {
        if (g_ReplayManager->replayData->header.stageReplayData[i] != NULL)
        {
            g_ZunMemory.Free(g_ReplayManager->replayData->header.stageReplayData[i]);
        }

        if (mgr->replayData->header.stageReplayFpsData[i] != NULL)
        {
            g_ZunMemory.Free(g_ReplayManager->replayData->header.stageReplayFpsData[i]);
        }
    }
        }

        g_Chain.Cut(g_ReplayManager->calcChain);
    }
}

namespace
{
char *AppendFormat(char *buffer, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    return buffer + strlen(buffer);
}
} // namespace

// FUNCTION: th08 0x453c60
void GameManager::SetClockTime(i32 value)
{
    this->globals->clockTime = value;
}

// FUNCTION: th08 0x453cc0
i32 ReplayManager::IsDemo()
{
    return this->isDemo;
}

} // namespace th08
