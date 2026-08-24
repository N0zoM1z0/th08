#pragma once

#include "Global.hpp"
#include "AnmManager.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"

#include <stddef.h>

namespace th08
{

struct Enemy;
struct AnmVm;

struct Effect
{
    AnmVm vm;
    Float3 vector0;
    Float3 vector1;
    Float3 vector2;
    Float3 vector3;
    Float3 vector4;
    Float3 vector5;
    Float3 vector6;
    Float3 vector7;
    Float3 vector8;
    unknown_fields(0x310, 0x28);
    ZunTimer timer;
    unknown_fields(0x344, 0x1c);

    Effect();
};
C_ASSERT(sizeof(Effect) == 0x360);

struct EffectManager
{
    EffectManager();

    i32 unknown0;
    i32 unknown4;
    i32 activeCount;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 scaleW;
    Effect effects[654];
    Effect sentinel0;
    Effect sentinel1;
    Effect sentinel2;
    Effect sentinel3;
    Effect sentinel4;
    unknown_fields(0x8b03c, 0x20);

    void ResetEffects();
    static ChainCallbackResult OnUpdate(EffectManager *effectManager);
    static ChainCallbackResult OnDraw(EffectManager *effectManager);
    static ZunResult AddedCallback(EffectManager *effectManager);
    static ZunResult DeletedCallback(EffectManager *effectManager);
    static ZunResult RegisterChain();
    static void CutChain();
    AnmVm *SpawnEffect00425B70(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    AnmVm *SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    AnmVm *SpawnEffectAngle(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 count, i32 color);
    AnmVm *FUN_004253e0(i32 index);
    AnmVm *FUN_00425870(i32 id, D3DXVECTOR3 *position, i32 slotIndex, i32 unused, i32 color);
    AnmVm *FUN_004259e0(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 slotIndex, i32 unused, i32 color);
    i32 DrawUnkTypeEffects();
    i32 FUN_004281e0();
};
C_ASSERT(sizeof(EffectManager) == 0x8b05c);
extern EffectManager g_EffectManager;
void __fastcall FUN_00426d10(Float3 *delta);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerDrawChain);

// A dword published by an ECL enemy callback from the target object's +0x18
// field.  Its consumer semantics are still unknown; this name records only
// the observed callback ownership and width.
DIFFABLE_EXTERN(i32, g_EclCallbackPublishedEnemyField24);
DIFFABLE_EXTERN(i32, g_EclCallbackPublishedEnemyField56);
struct EclExBarrierRenderState
{
    i32 mode;
    i32 unknown4;
    AnmVm vm0;
    AnmVm vm1;
};
C_ASSERT(offsetof(EclExBarrierRenderState, vm0) == 0x8);
C_ASSERT(offsetof(EclExBarrierRenderState, vm1) == 0x2ac);
DIFFABLE_EXTERN(i8, g_EclScriptedGlobalUpdateFreeze);
DIFFABLE_EXTERN(f32, g_EclGameTimeScale);
DIFFABLE_EXTERN(u32, g_EclGameTimeScaleFlags);
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
    i32 GetTimelineCount();
    u32 GetTimeline(i32 index);

    EclRawHeader *eclFile;             // +0x000
    u32 *subTable;                     // +0x004
    EclTimelineState timelineState;    // +0x008
};
C_ASSERT(sizeof(EclManager) == 0x168);

DIFFABLE_EXTERN(EclManager, g_EclManager);

} // namespace th08
