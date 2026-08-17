#pragma once

#include "Global.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"

#include <stddef.h>

namespace th08
{

struct Enemy;
struct AnmVm;

// Target-observed effect-manager entry points used by ECL and game teardown.
// The full manager layout remains outside the recovered public ABI.
struct EffectManager
{
    static void CutChain();
    AnmVm *SpawnEffect00425B70(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    AnmVm *SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    AnmVm *SpawnEffectAngle(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 count, i32 color);
    AnmVm *FUN_00425870(i32 id, D3DXVECTOR3 *position, i32 slotIndex, i32 unused, i32 color);
    i32 FUN_004281e0();
};
extern EffectManager g_EffectManager;
void __fastcall FUN_00426d10(Float3 *delta);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerDrawChain);

// A dword published by an ECL enemy callback from the target object's +0x18
// field.  Its consumer semantics are still unknown; this name records only
// the observed callback ownership and width.
DIFFABLE_EXTERN(i32, g_EclCallbackPublishedEnemyField24);

struct EclRawInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 unknown08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 operands[1];
};

// Only the target-observed fixed header is named here. Both pointer tables are
// stored as file-relative offsets and are rebased in place by Load.
struct EclRawHeader
{
    u32 version;
    i16 subCount;
    i16 unknown06;
    u32 timelineOffsets[16];
    u32 subOffsets[1];
};
typedef char EclRawHeaderSubTableOffsetCheck[offsetof(EclRawHeader, subOffsets) == 0x48 ? 1 : -1];

// Target-observed TH08 per-enemy ECL context. Unknown spans deliberately keep
// the public ABI narrower than the still-unrecovered interpreter state.
struct EnemyEclContext
{
    EclRawInstruction *currentInstr; // +0x000
    ZunTimer time;                   // +0x004
    u8 unknown10[0x80];
    ZunTimer secondaryTime; // +0x090
    u8 unknown9C[0x188];
    i16 subId; // +0x224
    u8 unknown226[2];
};
typedef char EnemyEclContextSecondaryTimerOffsetCheck[offsetof(EnemyEclContext, secondaryTime) == 0x90 ? 1 : -1];
typedef char EnemyEclContextSubIdOffsetCheck[offsetof(EnemyEclContext, subId) == 0x224 ? 1 : -1];
typedef char EnemyEclContextSizeCheck[sizeof(EnemyEclContext) == 0x228 ? 1 : -1];

struct EclTimelineState
{
    EclTimelineState();

    u8 unknown000[0x100];
    D3DXVECTOR3 vectors[8];
};

struct EclManager
{
    EclManager();
    ZunResult Load(char *path);
    void Unload();
    ZunResult CallEclSub(EnemyEclContext *context, i16 subId);
    ZunResult RunEcl(Enemy *enemy);

    EclRawHeader *eclFile;             // +0x000
    u32 *subTable;                     // +0x004
    EclTimelineState timelineState;    // +0x008
};

} // namespace th08
