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

extern u16 g_GuiMessageInputCurrent;
extern u16 g_GuiMessageInputPrevious;

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
    obfuscateOffset = replayData->header.value1;

    for (i = 0; i < replayData->header.fileSize - (i32)offsetof(ReplayDataHeader, compressedSize);
         i++, obfuscateCursor++)
    {
        *obfuscateCursor -= obfuscateOffset;
        obfuscateOffset += 7;
    }

    checksumCursor = &replayData->header.value1;
    checksum = REPLAY_OBFUSCATION_VALUE;

    for (i = 0; i < replayData->header.fileSize - (i32)offsetof(ReplayDataHeader, value1); i++, checksumCursor++)
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
        case 0:
            replayManager->calcChain = g_Chain.CreateElem((ChainCallback)OnUpdateHighPrio);
            replayManager->calcChain->addedCallback = (ChainLifetimeCallback)AddedCallback;
            replayManager->calcChain->deletedCallback = (ChainLifetimeCallback)DeletedCallback;
            replayManager->calcChain->arg = replayManager;

            if (g_Chain.AddToCalcChain(replayManager->calcChain, 17))
            {
                return ZUN_ERROR;
            }

            replayManager->frameControlChain = NULL;
            replayManager->rngSyncChain = g_Chain.CreateElem((ChainCallback)OnUpdateLowPrio);
            replayManager->rngSyncChain->arg = replayManager;
            g_Chain.AddToCalcChain(replayManager->rngSyncChain, 7);
            OnUpdateLowPrio(replayManager);
            break;

        case 1:
            replayManager->calcChain = g_Chain.CreateElem((ChainCallback)OnUpdateHighPrioDemo);
            replayManager->calcChain->addedCallback = (ChainLifetimeCallback)AddedCallbackDemo;
            replayManager->calcChain->deletedCallback = (ChainLifetimeCallback)DeletedCallback;
            replayManager->calcChain->arg = replayManager;

            if (g_Chain.AddToCalcChain(replayManager->calcChain, 6))
            {
                return ZUN_ERROR;
            }

            if (replayManager->replayData->header.unk0x6 != 0)
            {
                replayManager->calcChain->callback = (ChainCallback)OnUpdateHighPrioDemo2;
            }

            replayManager->frameControlChain = g_Chain.CreateElem((ChainCallback)OnUpdateFrameControl);
            replayManager->frameControlChain->arg = replayManager;
            g_Chain.AddToCalcChain(replayManager->frameControlChain, 18);

            replayManager->rngSyncChain = NULL;
            if (replayManager->replayData->header.unk0x6 != 0)
            {
                replayManager->rngSyncChain = g_Chain.CreateElem((ChainCallback)OnUpdateLowPrio);
                replayManager->rngSyncChain->arg = replayManager;
                g_Chain.AddToCalcChain(replayManager->rngSyncChain, 7);
                OnUpdateLowPrio(replayManager);
            }
            break;
        }
    }
    else
    {
        switch (replayMode)
        {
        case 0:
            AddedCallback(g_ReplayManager);
            break;
        case 1:
            AddedCallbackDemo(g_ReplayManager);
            break;
        }
    }

    return ZUN_SUCCESS;
}

ChainCallbackResult ReplayManager::OnUpdateLowPrio(ReplayManager *replayManager)
{
    replayManager->flags = 0;
    replayManager->rngSeed = g_Rng.GetSeed();
    g_Rng.ResetGenerationCount();

    if (g_GameManager.unk3DB98 != 0)
    {
        replayManager->flags |= 0x100;
    }

    g_GameManager.unk3DB98 = 0;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(stage, input)
ChainCallbackResult ReplayManager::OnUpdateHighPrio(ReplayManager *replayManager)
{
    i32 stage;
    u16 input;

    if (!g_GameManager.flags.unk2)
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

    if (g_GameManager.flags.unk9)
    {
        if (replayManager->inputDelay >= 3)
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }

        replayManager->inputDelay++;
    }

    stage = g_GameManager.currentStage2;
    input = g_CurFrameInput;
    g_GuiMessageInputCurrent = input;

    replayManager->replayInputs += sizeof(u16);
    replayManager->replayInputStageBookmarks[stage] = replayManager->replayInputs + sizeof(u16);
    *(u16 *)replayManager->replayInputs = input;

    if (replayManager->frameCounter % 30 == 0)
    {
        replayManager->replayRngInputs[0] =
            (u8)g_Supervisor.unk198 | (g_Supervisor.unk0x338 != 0 ? 0x80 : 0);
        replayManager->replayRngInputs[1] = (u8)g_Supervisor.unk198;
        replayManager->replayInputStageBookmarks2[stage] = replayManager->replayRngInputs + 2;
        replayManager->replayRngInputs++;
    }

    replayManager->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::OnUpdateFrameControl(ReplayManager *replayManager)
{
    if (!g_GameManager.flags.unk2)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.unk9)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_Gui.IsDialogPresent() && g_Gui.FUN_00437dc7() && replayManager->frameCounter % 3 != 2)
    {
        return CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB;
    }

    if (!g_GameManager.flags.unk9 && g_GameManager.replayMode == 2 && !g_EnemyManager.FUN_0042f1f0() &&
        replayManager->frameCounter % 5 != 4)
    {
        return CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::OnUpdateHighPrioDemo(ReplayManager *replayManager)
{
    i32 unused;

    if (!g_GameManager.flags.unk2)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.cfg->slowMode != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.unk9)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    unused = 0;
    g_GuiMessageInputPrevious = g_GuiMessageInputCurrent;
    g_GuiMessageInputCurrent = *(u16 *)replayManager->replayInputs;
    replayManager->replayInputs += sizeof(u16);

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
        *reinterpret_cast<u16 *>(&g_Supervisor.unk198) = (i8)replayManager->replayRngInputs[1] & 0x7f;
        g_Supervisor.unk0x33c = (i8)replayManager->replayRngInputs[1] >> 7;
        replayManager->replayRngInputs++;
    }

    replayManager->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::OnUpdateHighPrioDemo2(ReplayManager *replayManager)
{
    i32 unused;

    if (!g_GameManager.flags.unk2)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.cfg->slowMode != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.flags.unk9)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    unused = 0;
    g_GuiMessageInputPrevious = g_GuiMessageInputCurrent;
    g_GuiMessageInputCurrent = *reinterpret_cast<u16 *>(replayManager->replayInputs2);
    replayManager->replayInputs2++;

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
        *reinterpret_cast<u16 *>(&g_Supervisor.unk198) = (i8)replayManager->replayRngInputs[1] & 0x7f;
        g_Supervisor.unk0x33c = (i8)replayManager->replayRngInputs[1] >> 7;
        replayManager->replayRngInputs++;
    }

    replayManager->frameCounter++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(stageData, stage, stageData2, previousStage)
ZunResult ReplayManager::AddedCallback(ReplayManager *replayManager)
{
    StageReplayData *previousStage;
    StageReplayData *stageData2;
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
        replayManager->replayData->header.unk0x7 = 0;
        replayManager->replayData->shotType = g_GameManager.shotType;
        replayManager->replayData->header.version = REPLAY_VERSION;
        replayManager->replayData->header.unk0x6 = 0;
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
            replayManager->replayData->header.stageReplayData2[stage] = NULL;
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
    if (replayManager->replayData->header.stageReplayData2[stage] != NULL)
    {
        g_ZunMemory.Free(replayManager->replayData->header.stageReplayData2[stage]);
    }

    replayManager->replayData->header.stageReplayData[stage] =
        (StageReplayData *)g_ZunMemory.Alloc(0xd2f00, "rep data");
    replayManager->replayData->header.stageReplayData2[stage] =
        (StageReplayData *)g_ZunMemory.Alloc(0xd2f00, "rep data");

    stageData = replayManager->replayData->header.stageReplayData[stage];
    stageData2 = replayManager->replayData->header.stageReplayData2[stage];

    stageData->graze = g_GameManager.globals->graze;
    stageData->bombs = (u8)g_GameManager.GetBombsRemaining();
    stageData->lives = (u8)g_GameManager.GetLives();
    stageData->power = (u8)g_GameManager.GetPower();
    stageData->rank = (u8)g_GameManager.rank;
    stageData->pointItemsCollected = g_GameManager.globals->pointItemsCollected;
    stageData->rngSeed = *reinterpret_cast<u16 *>(&g_GameManager.unk3ddbc);
    stageData->character = g_GameManager.character;
    stageData->unk0x21 = (u8)g_GameManager.globals->spellcardsCaptured;
    stageData->pointItemExteds = g_GameManager.globals->pointItemExtendsSoFar;
    stageData->nextPointItemExtendThreshold = g_GameManager.globals->nextPointItemExtendThreshold;
    stageData->youkaiGauge = g_GameManager.globals->youkaiGauge;
    stageData->clockTime = g_GameManager.GetClockTime();
    stageData->pointItemValue = g_GameManager.globals->pointItemValue;

    replayManager->replayInputs = reinterpret_cast<u8 *>(stageData) + 0x24;
    replayManager->replayInputs2 = reinterpret_cast<ReplayInputSync *>(replayManager->replayInputs);
    replayManager->replayRngInputs = reinterpret_cast<u8 *>(stageData2);
    *reinterpret_cast<u16 *>(replayManager->replayInputs) = 0;
    replayManager->replayInputs2->unk2 = 0;
    replayManager->replayInputs2->rngSeed = g_Rng.GetSeed();
    replayManager->inputDelay = 0;
    replayManager->unk4e = 0;

    return ZUN_SUCCESS;
}

#pragma var_order(stageData, stage, fileSize, stageData2, previousStage)
ZunResult ReplayManager::AddedCallbackDemo(ReplayManager *replayManager)
{
    StageReplayData *previousStage;
    StageReplayData *stageData2;
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
            if (replayManager->replayData->header.stageReplayData2[stage] != NULL)
            {
                replayManager->replayData->header.stageReplayData2[stage] =
                    (StageReplayData *)(reinterpret_cast<u8 *>(replayManager->replayData->header.stageReplayData2[stage]) +
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
    stageData2 = replayManager->replayData->header.stageReplayData2[stage];

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

    replayManager->replayInputs = reinterpret_cast<u8 *>(stageData) + 0x24;
    replayManager->replayInputs2 = reinterpret_cast<ReplayInputSync *>(replayManager->replayInputs);
    g_GameManager.character = stageData->character;
    g_GameManager.globals->pointItemValue = stageData->pointItemValue;
    *g_GameManager.cfg = replayManager->replayData->gameConfiguration;
    g_Rng.SetSeed(stageData->rngSeed);
    g_GameManager.globals->spellcardsCaptured = stageData->unk0x21;
    g_GameManager.globals->pointItemExtendsSoFar = stageData->pointItemExteds;
    g_GameManager.globals->nextPointItemExtendThreshold = stageData->nextPointItemExtendThreshold;
    g_GameManager.globals->youkaiGauge = stageData->youkaiGauge;
    g_GameManager.SetClockTime(stageData->clockTime);

    replayManager->replayRngInputs = reinterpret_cast<u8 *>(stageData2);
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

ZunResult ReplayManager::DeletedCallback(ReplayManager *replayManager)
{
    if (replayManager->frameControlChain != NULL)
    {
        g_Chain.Cut(replayManager->frameControlChain);
        replayManager->frameControlChain = NULL;
    }

    if (replayManager->rngSyncChain != NULL)
    {
        g_Chain.Cut(replayManager->rngSyncChain);
        replayManager->rngSyncChain = NULL;
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
        mgr->replayInputs += sizeof(u16);
        *(u16 *)mgr->replayInputs = 0;
        stage = g_GameManager.currentStage;
        mgr->replayInputStageBookmarks[stage] = mgr->replayInputs + 6;
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

    i = g_GameManager.currentStage2;
    mgr->replayData->header.stageReplayData[i]->score = g_GameManager.globals->score;

    currentOffset = sizeof(ReplayDataHeader);
    currentOffset += sizeof(ReplayData) - sizeof(ReplayDataHeader);

    for (i = 0; i < MAX_STAGES; i++)
    {
        if (mgr->replayData->header.stageReplayData[i] != NULL)
        {
            stageSize = mgr->replayInputStageBookmarks[i] - (u8 *)mgr->replayData->header.stageReplayData[i];
            memcpy(tempBuffer + currentOffset - sizeof(ReplayDataHeader),
                   mgr->replayData->header.stageReplayData[i], stageSize);
            replayCopy.header.stageReplayData[i] = (StageReplayData *)currentOffset;
            currentOffset += stageSize;
        }
    }

    for (i = 0; i < MAX_STAGES; i++)
    {
        if (mgr->replayData->header.stageReplayData2[i] != NULL)
        {
            stageSize = mgr->replayInputStageBookmarks2[i] - (u8 *)mgr->replayData->header.stageReplayData2[i];
            memcpy(tempBuffer + currentOffset - sizeof(ReplayDataHeader),
                   mgr->replayData->header.stageReplayData2[i], stageSize);
            replayCopy.header.stageReplayData2[i] = (StageReplayData *)currentOffset;
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
                                  g_GameManager.flags.unk4 ? "Clear" : ResultScreen::GetStageName(g_GameManager.currentStage2));
    }

    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_DEATHS, g_GameManager.GetDeaths());
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_BOMBS, g_GameManager.GetBombsUsed());
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_SLOWDOWN, replayCopy.slowDownRate);

    g_GameManager.hscr.humanityRate = (i32)(((float)g_GameManager.unk3DBA0 / g_GameManager.unk3DBA4) * 10.0f);
    infoCursor =
        AppendFormat(infoCursor, TH_REPLAY_INFO_HUMAN_RATE, (float)g_GameManager.hscr.humanityRate / 100.0f);
    infoCursor = AppendFormat(infoCursor, TH_REPLAY_INFO_VERSION, 1, 0, 'd');

    infoHeader.size = strlen(infoBuffer) + sizeof(ReplayUserDataHeader);
    infoHeader.size += infoHeader.size & 1;

    replayCopy.header.unk0x7 = 1;
    strcpy(replayCopy.playerName, replayName);
    ResultScreen::FormatDate(replayCopy.date);

    replayCopy.header.value1 = (u8)(g_Rng.GetRandomU16InRange(0x80) + 0x40);
    replayCopy.unk0x68 = (u8)g_Rng.GetRandomU16InRange(0x100);
    replayCopy.header.unk0x14 = (u8)g_Rng.GetRandomU16InRange(0x100);
    replayCopy.slowDownRate2 = replayCopy.slowDownRate + 1.12f;
    replayCopy.unk0x120 = 30;

    memcpy(tempBuffer, &replayCopy.unk0x68, sizeof(ReplayData) - sizeof(ReplayDataHeader));

    utils::DebugPrint("info : original size %d\r\n", currentOffset);

    replayCopy.header.decompressedSize = currentOffset - sizeof(ReplayDataHeader);
    compressedData = Lzss::Encode(tempBuffer, replayCopy.header.decompressedSize, &replayCopy.header.compressedSize);
    g_ZunMemory.Free(tempBuffer);
    compressedSize = replayCopy.header.compressedSize;

    checksumCursor = &replayCopy.header.value1;
    checksum = REPLAY_OBFUSCATION_VALUE;

    for (i = 0; i < sizeof(ReplayDataHeader) - offsetof(ReplayDataHeader, value1); i++, checksumCursor++)
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
    obfuscateOffset = replayCopy.header.value1;

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

        if (mgr->replayData->header.stageReplayData2[i] != NULL)
        {
            g_ZunMemory.Free(g_ReplayManager->replayData->header.stageReplayData2[i]);
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
