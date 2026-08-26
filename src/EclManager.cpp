#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "EnemyManager.hpp"
#include "Gui.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"

namespace th08
{



EclManager::EclManager()
{
}

EclTimelineState::EclTimelineState()
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

namespace EclOperands
{

// FUNCTION: th08 0x0041F000
i32 TargetEnemyHelpersOverlay::HasParentChain()
{
    return reinterpret_cast<Enemy *>(this)->parentEnemy == 0 &&
           *(void **)((u8 *)this + 8) != 0;
}

} // namespace EclOperands

// FUNCTION: th08 0x0041F040
void Spellcard::SetStoredVector(f32 x, f32 y, f32 z)
{
    *(f32 *)(this->spellEffect + 0x2A4) = x;
    *(f32 *)(this->spellEffect + 0x2A8) = y;
    *(f32 *)(this->spellEffect + 0x2AC) = z;
}

struct SpellcardEclFlagBits
{
    u32 lowBits : 6;
    u32 bit6 : 1;
    u32 bits7To10 : 4;
    u32 bit11 : 1;
    u32 highBits : 20;
};
C_ASSERT(sizeof(SpellcardEclFlagBits) == 4);

// FUNCTION: th08 0x0041F0B0
void Spellcard::FUN_0041f0b0(i32 value)
{
    reinterpret_cast<SpellcardEclFlagBits *>(&this->flags)->bit6 = value;
}

// FUNCTION: th08 0x0041F0E0
void Spellcard::FUN_0041f0e0(i32 value)
{
    reinterpret_cast<SpellcardEclFlagBits *>(&this->flags)->bit11 = value;
}

namespace EclOperands
{

// FUNCTION: th08 0x0041FD20
i32 TargetEnemyHelpersOverlay::HasAttachedEnemy()
{
    return reinterpret_cast<Enemy *>(this)->parentEnemy != 0;
}

// FUNCTION: th08 0x0041FD40
i32 TargetEnemyHelpersOverlay::CountParentChain()
{
    TargetEnemyHelpersOverlay *cursor = this;
    i32 count = 0;
    if (this->HasParentChain())
    {
        while (*(void **)((u8 *)cursor + 8) != 0)
        {
            cursor = *(TargetEnemyHelpersOverlay **)((u8 *)cursor + 8);
            count++;
        }
    }
    return count;
}

} // namespace EclOperands

// FUNCTION: th08 0x0041FD90
i32 Spellcard::GetActiveState()
{
    return this->IsActive() && ((this->flags >> 2) & 1);
}

// FUNCTION: th08 0x0041FDD0
i32 Spellcard::GetTimerFrames()
{
    return *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x108);
}

// FUNCTION: th08 0x41fdf0
void ZunTimer::operator+=(int value)
{
    this->Increment(value);
}

// FUNCTION: th08 0x422c20
bool Gui::SetBossPresent(bool value)
{
    return this->bossPresent = value;
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

// FUNCTION: th08 0x0042DFF0
i32 Spellcard::FUN_0042DFF0()
{
    return (this->flags >> 7) & 1;
}

} // namespace th08
