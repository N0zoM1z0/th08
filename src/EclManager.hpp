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
    Float3 position;
    Float3 vector1;
    Float3 vector2;
    Float3 vector3;
    Float3 vector4;
    Float3 vector5;
    Float3 vector6;
    Float3 vector7;
    Float3 orientationAxis;
    f32 orientationW;
    f32 radius;
    f32 angle;
    unknown_fields(0x31c, 4);
    f32 shapeThickness;
    i32 vertexSegmentCount;
    i32 slotIndex;
    f32 secondaryRadius;
    f32 secondaryAngle;
    f32 radialWaveCount;
    ZunTimer timer;
    unknown_fields(0x344, 4);
    void *updateCallback;
    void *drawCallback;
    i8 active;
    i8 effectId;
    i8 releaseRequested;
    i8 releaseTimer;
    i8 drawGroup;
    i8 alternateDrawGroup;
    i8 verticesDirty;
    i8 updateDuringFreeze;
    VertexTex1DiffuseXyzrhw *vertices;
    Effect *nextInDrawGroup;

    Effect();
};
C_ASSERT(sizeof(Effect) == 0x360);
C_ASSERT(offsetof(Effect, position) == 0x2a4);
C_ASSERT(offsetof(Effect, vector5) == 0x2e0);
C_ASSERT(offsetof(Effect, vector6) == 0x2ec);
C_ASSERT(offsetof(Effect, orientationAxis) == 0x304);
C_ASSERT(offsetof(Effect, orientationW) == 0x310);
C_ASSERT(offsetof(Effect, shapeThickness) == 0x320);
C_ASSERT(offsetof(Effect, vertexSegmentCount) == 0x324);
C_ASSERT(offsetof(Effect, slotIndex) == 0x328);
C_ASSERT(offsetof(Effect, secondaryRadius) == 0x32c);
C_ASSERT(offsetof(Effect, secondaryAngle) == 0x330);
C_ASSERT(offsetof(Effect, radialWaveCount) == 0x334);
C_ASSERT(offsetof(Effect, radius) == 0x314);
C_ASSERT(offsetof(Effect, angle) == 0x318);
C_ASSERT(offsetof(Effect, timer) == 0x338);
C_ASSERT(offsetof(Effect, updateCallback) == 0x348);
C_ASSERT(offsetof(Effect, drawCallback) == 0x34c);
C_ASSERT(offsetof(Effect, active) == 0x350);
C_ASSERT(offsetof(Effect, effectId) == 0x351);
C_ASSERT(offsetof(Effect, releaseRequested) == 0x352);
C_ASSERT(offsetof(Effect, releaseTimer) == 0x353);
C_ASSERT(offsetof(Effect, drawGroup) == 0x354);
C_ASSERT(offsetof(Effect, alternateDrawGroup) == 0x355);
C_ASSERT(offsetof(Effect, verticesDirty) == 0x356);
C_ASSERT(offsetof(Effect, updateDuringFreeze) == 0x357);
C_ASSERT(offsetof(Effect, vertices) == 0x358);
C_ASSERT(offsetof(Effect, nextInDrawGroup) == 0x35c);

struct EffectManager
{
    EffectManager();

    i32 nextEffectIndex;
    i32 unknown4;
    i32 activeCount;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 scaleW;
    Effect effects[654];
    Effect drawGroupSentinel0;
    Effect drawGroupSentinel1;
    Effect drawGroupSentinel2;
    Effect drawGroupSentinel3;
    Effect drawGroupSentinel4;
    Effect *drawGroupTails[5];
    i32 tamperCheckCounter;
    AnmLoaded *effectAnm;
    AnmLoaded *stageEffectAnm;

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
C_ASSERT(offsetof(EffectManager, effects) == 0x1c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel0) == 0x89f5c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel1) == 0x8a2bc);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel2) == 0x8a61c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel3) == 0x8a97c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel4) == 0x8acdc);
C_ASSERT(offsetof(EffectManager, drawGroupTails) == 0x8b03c);
C_ASSERT(offsetof(EffectManager, tamperCheckCounter) == 0x8b050);
C_ASSERT(offsetof(EffectManager, effectAnm) == 0x8b054);
C_ASSERT(offsetof(EffectManager, stageEffectAnm) == 0x8b058);
extern EffectManager g_EffectManager;
void __fastcall FUN_00426d10(Float3 *delta);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerDrawChain);

#ifndef TH08_MODERN_PORT
DIFFABLE_EXTERN(void *, g_EclExUpdateCallback);
#endif
struct EclExBarrierRenderState
{
    i32 mode;
    i32 unknown4;
    AnmVm vm0;
    AnmVm vm1;
};
C_ASSERT(offsetof(EclExBarrierRenderState, vm0) == 0x8);
C_ASSERT(offsetof(EclExBarrierRenderState, vm1) == 0x2ac);
#ifndef TH08_MODERN_PORT
DIFFABLE_EXTERN(EclExBarrierRenderState, g_EclExBarrierRenderState);
#endif
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

typedef void (__fastcall *EnemyEclContextCallback)(Enemy *enemy, void *argument);

struct EnemyEclInterpolationSlot
{
    void *callback;
    ZunTimer timer;
    i32 duration;
    i32 callbackIndex;
    i32 easing;
    f32 parameters[4];
    f32 affectedVariable;
};
C_ASSERT(sizeof(EnemyEclInterpolationSlot) == 0x30);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, duration) == 0x10);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, affectedVariable) == 0x2c);

// Target-observed TH08 per-enemy ECL interpreter context. The constructor-
// bearing Enemy storage type retains its target symbol separately, while this
// ABI view names the state consumed by EclManager and the exact interpreter.
struct EnemyEclContext
{
    EclRawInstruction *currentInstr;
    ZunTimer time;
    EnemyEclContextCallback callback;
    void *callbackArgument;
    i32 intVariables[8];
    f32 floatVariables[8];
    i32 extraIntVariables[4];
    f32 extraFloatVariables[2];
    i32 callParameterInts[4];
    f32 callParameterFloats[4];
    ZunTimer secondaryTime;
    EnemyEclInterpolationSlot interpolationSlots[8];
    i32 unknown21c;
    i32 childContextSlot;
    i16 subId;
    u8 unknown226[2];
};
typedef char EnemyEclContextCallbackOffsetCheck[offsetof(EnemyEclContext, callback) == 0x10 ? 1 : -1];
typedef char EnemyEclContextIntVariablesOffsetCheck[offsetof(EnemyEclContext, intVariables) == 0x18 ? 1 : -1];
typedef char EnemyEclContextFloatVariablesOffsetCheck[offsetof(EnemyEclContext, floatVariables) == 0x38 ? 1 : -1];
typedef char EnemyEclContextExtraIntVariablesOffsetCheck[offsetof(EnemyEclContext, extraIntVariables) == 0x58 ? 1 : -1];
typedef char EnemyEclContextExtraFloatVariablesOffsetCheck[offsetof(EnemyEclContext, extraFloatVariables) == 0x68 ? 1 : -1];
typedef char EnemyEclContextCallParametersOffsetCheck[offsetof(EnemyEclContext, callParameterInts) == 0x70 ? 1 : -1];
typedef char EnemyEclContextSecondaryTimerOffsetCheck[offsetof(EnemyEclContext, secondaryTime) == 0x90 ? 1 : -1];
typedef char EnemyEclContextInterpolationSlotsOffsetCheck[offsetof(EnemyEclContext, interpolationSlots) == 0x9c ? 1 : -1];
typedef char EnemyEclContextChildSlotOffsetCheck[offsetof(EnemyEclContext, childContextSlot) == 0x220 ? 1 : -1];
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
