#include "th_pch.h"

#include "Global.hpp"
#include "ReplayManager.hpp"

#include "pbg/Lzss.hpp"

#include <stddef.h>

namespace th08
{

DIFFABLE_STATIC(ReplayManager *, g_ReplayManager);

// STUB: th08 0x4531f0
void ReplayManager::SaveReplay(const char *replayPath, const char *replayName)
{
}

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
