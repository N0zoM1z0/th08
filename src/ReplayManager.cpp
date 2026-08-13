#include "th_pch.h"

#include "Global.hpp"
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

} // namespace th08
