#include "th_pch.h"

#include "EclManager.hpp"

namespace th08
{

EclTimelineState::EclTimelineState()
{
}

EclManager::EclManager()
{
}

ZunResult EclManager::Load(char *path)
{
    i32 index;

    this->eclFile = (EclRawHeader *)FileSystem::OpenFile(path, NULL, FALSE);
    if (this->eclFile == NULL)
    {
        g_GameErrorContext.Log(
            "\x93\x47\x83\x66\x81\x5b\x83\x5e\x82\xcc\x93\xc7\x82\xdd\x8d\x9e"
            "\x82\xdd\x82\xc9\x8e\xb8\x94\x73\x82\xb5\x82\xdc\x82\xb5\x82\xbd\x81\x41"
            "\x83\x66\x81\x5b\x83\x5e\x82\xaa\x89\xf3\x82\xea\x82\xc4\x82\xe9\x82\xa9"
            "\x8e\xb8\x82\xed\x82\xea\x82\xc4\x82\xa2\x82\xdc\x82\xb7\r\n");
        return ZUN_ERROR;
    }

    if (this->eclFile->version != 0x800)
    {
        g_GameErrorContext.Log(
            "\x93\x47\x83\x66\x81\x5b\x83\x5e\x82\xcc\x83\x6f\x81\x5b\x83\x57\x83\x87"
            "\x83\x93\x82\xaa\x88\xe1\x82\xa2\x82\xdc\x82\xb7\r\n");
        return ZUN_ERROR;
    }

    for (index = 0; index < 16; index++)
    {
        this->eclFile->timelineOffsets[index] += (u32)this->eclFile;
    }

    this->subTable = this->eclFile->subOffsets;
    for (index = 0; index < this->eclFile->subCount; index++)
    {
        this->subTable[index] += (u32)this->eclFile;
    }

    return ZUN_SUCCESS;
}

void EclManager::Unload()
{
    if (this->eclFile != NULL)
    {
        g_ZunMemory.Free(this->eclFile);
    }
    this->eclFile = NULL;
}

ZunResult EclManager::CallEclSub(EnemyEclContext *context, i16 subId)
{
    if (subId < 0)
    {
        return ZUN_SUCCESS;
    }

    context->currentInstr = (EclRawInstruction *)this->subTable[subId];
    context->time = 0;
    context->secondaryTime = 0;
    context->subId = subId;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42dfb0
i32 EclManager::GetTimelineCount()
{
    return this->eclFile->unknown06;
}

// FUNCTION: th08 0x42dfd0
u32 EclManager::GetTimeline(i32 index)
{
    return this->eclFile->timelineOffsets[index];
}

} // namespace th08
