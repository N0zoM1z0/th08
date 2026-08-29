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
struct Effect;
struct EclTimelineInstruction;

typedef i32 (__fastcall *EffectUpdateCallback)(Effect *effect);
typedef i32 (__fastcall *EffectInitializeCallback)(Effect *effect);
typedef i32 (__fastcall *EffectDrawCallback)(Effect *effect);

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
    u32 unconsumedDword31C;
    f32 shapeThickness;
    i32 vertexSegmentCount;
    i32 slotIndex;
    f32 secondaryRadius;
    f32 secondaryAngle;
    f32 radialWaveCount;
    ZunTimer timer;
    u32 unconsumedDword344;
    EffectUpdateCallback updateCallback;
    EffectDrawCallback drawCallback;
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
C_ASSERT(offsetof(Effect, unconsumedDword31C) == 0x31c);
C_ASSERT(offsetof(Effect, timer) == 0x338);
C_ASSERT(offsetof(Effect, unconsumedDword344) == 0x344);
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
    i32 unconsumedDword04;
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
    static ZunResult LoadEffectResources(EffectManager *effectManager);
    static ZunResult ReleaseEffectResources(EffectManager *effectManager);
    static ZunResult RegisterChain();
    static void CutChain();
    Effect *SpawnEffectInSecondaryPool(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    Effect *SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    Effect *SpawnEffectWithVelocity(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 count, i32 color);
    Effect *GetFixedSlotEffect(i32 index);
    Effect *SpawnEffectInFixedSlot(i32 id, D3DXVECTOR3 *position, i32 slotIndex, i32 unused, i32 color);
    Effect *SpawnEffectInFixedSlotWithVelocity(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 slotIndex, i32 unused, i32 color);
    i32 DrawBulletLayerEffects();
    i32 DrawBackgroundEffects();
};
C_ASSERT(sizeof(EffectManager) == 0x8b05c);
C_ASSERT(offsetof(EffectManager, unconsumedDword04) == 0x4);
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
void __fastcall ShiftStageEffectOrigins(Float3 *delta);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerDrawChain);

DIFFABLE_EXTERN(u32, g_EclGameTimeScaleFlags);
struct EclRawInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 serializedReserved08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 operands[1];
};

struct EclExInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 serializedReserved08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 serializedReserved0C[4];
    union
    {
        i32 value;
        i8 byteValue;
    };
};
C_ASSERT(offsetof(EclExInstruction, value) == 0x10);

typedef void (__fastcall *EclExInstructionCallback)(
    Enemy *enemy, EclExInstruction *instruction);
extern EclExInstructionCallback g_EclExInsn[32];

// Only the target-observed fixed header is named here. Both pointer tables are
// stored as file-relative offsets and are rebased in place by Load.
struct EclRawHeader
{
    u32 version;
    i16 subCount;
    i16 timelineCount;
    u32 timelineOffsets[16];
    u32 subOffsets[1];
};
typedef char EclRawHeaderTimelineCountOffsetCheck[offsetof(EclRawHeader, timelineCount) == 0x6 ? 1 : -1];
typedef char EclRawHeaderSubTableOffsetCheck[offsetof(EclRawHeader, subOffsets) == 0x48 ? 1 : -1];

struct EnemyEclInterpolationSlot;
typedef void (__fastcall *EnemyEclInterpolatorCallback)(
    Enemy *enemy, EnemyEclInterpolationSlot *slot, f32 progress);

struct EnemyEclInterpolationSlot
{
    EnemyEclInterpolationSlot();

    EnemyEclInterpolatorCallback callback;
    ZunTimer timer;
    i32 duration;
    i32 callbackIndex;
    i32 easing;
    f32 parameters[4];
    f32 affectedVariable;
};
C_ASSERT(sizeof(EnemyEclInterpolationSlot) == 0x30);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, duration) == 0x10);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, callbackIndex) == 0x14);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, parameters) == 0x1c);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, affectedVariable) == 0x2c);

// Target-observed TH08 per-enemy ECL interpreter context. The constructor-
// bearing Enemy storage type retains its target symbol separately, while this
// ABI view names the state consumed by EclManager and the exact interpreter.
struct EnemyEclContext
{
    EnemyEclContext();

    EclRawInstruction *currentInstr;
    ZunTimer time;
    EclExInstructionCallback perFrameCallback;
    EclExInstruction *perFrameInstruction;
    i32 intVariables[8];
    f32 floatVariables[8];
    i32 extraIntVariables[4];
    f32 extraFloatVariables[2];
    i32 callParameterInts[4];
    f32 callParameterFloats[4];
    ZunTimer secondaryTime;
    EnemyEclInterpolationSlot interpolationSlots[8];
    i32 unconsumedDword21C;
    i32 childContextSlot;
    i16 subId;
    u8 contextTailAlignment226[2];
};
#ifndef TH08_PORTABLE_NATIVE_LAYOUT
typedef char EnemyEclContextCallbackOffsetCheck[offsetof(EnemyEclContext, perFrameCallback) == 0x10 ? 1 : -1];
typedef char EnemyEclContextIntVariablesOffsetCheck[offsetof(EnemyEclContext, intVariables) == 0x18 ? 1 : -1];
typedef char EnemyEclContextFloatVariablesOffsetCheck[offsetof(EnemyEclContext, floatVariables) == 0x38 ? 1 : -1];
typedef char EnemyEclContextExtraIntVariablesOffsetCheck[offsetof(EnemyEclContext, extraIntVariables) == 0x58 ? 1 : -1];
typedef char EnemyEclContextExtraFloatVariablesOffsetCheck[offsetof(EnemyEclContext, extraFloatVariables) == 0x68 ? 1 : -1];
typedef char EnemyEclContextCallParametersOffsetCheck[offsetof(EnemyEclContext, callParameterInts) == 0x70 ? 1 : -1];
typedef char EnemyEclContextSecondaryTimerOffsetCheck[offsetof(EnemyEclContext, secondaryTime) == 0x90 ? 1 : -1];
typedef char EnemyEclContextInterpolationSlotsOffsetCheck[offsetof(EnemyEclContext, interpolationSlots) == 0x9c ? 1 : -1];
typedef char EnemyEclContextUnconsumedOffsetCheck[offsetof(EnemyEclContext, unconsumedDword21C) == 0x21c ? 1 : -1];
typedef char EnemyEclContextChildSlotOffsetCheck[offsetof(EnemyEclContext, childContextSlot) == 0x220 ? 1 : -1];
typedef char EnemyEclContextSubIdOffsetCheck[offsetof(EnemyEclContext, subId) == 0x224 ? 1 : -1];
typedef char EnemyEclContextTailAlignmentOffsetCheck[offsetof(EnemyEclContext, contextTailAlignment226) == 0x226 ? 1 : -1];
typedef char EnemyEclContextSizeCheck[sizeof(EnemyEclContext) == 0x228 ? 1 : -1];
#endif

// Raw-allocated by ECL opcode 135. The target clears the complete block,
// installs one child context at +0x8, and uses the 16 following contexts as
// that child's call stack.
struct EnemyChildEclBlock
{
    i32 subId;
    u16 unconsumedWord04;
    i16 callStackDepth;
    EnemyEclContext eclContext;
    EnemyEclContext callStack[16];
};
C_ASSERT(sizeof(EnemyChildEclBlock) == 0x24b0);
C_ASSERT(offsetof(EnemyChildEclBlock, subId) == 0x0);
C_ASSERT(offsetof(EnemyChildEclBlock, unconsumedWord04) == 0x4);
C_ASSERT(offsetof(EnemyChildEclBlock, callStackDepth) == 0x6);
C_ASSERT(offsetof(EnemyChildEclBlock, eclContext) == 0x8);
C_ASSERT(offsetof(EnemyChildEclBlock, callStack) == 0x230);

struct EclTimelineState
{
    EclTimelineState();

    u8 unconsumedBytes000[0x100];
    D3DXVECTOR3 vectors[8];
};
C_ASSERT(sizeof(EclTimelineState) == 0x160);
C_ASSERT(offsetof(EclTimelineState, unconsumedBytes000) == 0x0);
C_ASSERT(offsetof(EclTimelineState, vectors) == 0x100);

struct EclManager
{
    EclManager();
    ZunResult Load(char *path);
    void Unload();
    ZunResult CallEclSub(EnemyEclContext *context, i16 subId);
    ZunResult RunEcl(Enemy *enemy);
    i32 GetTimelineCount();
    EclTimelineInstruction *GetTimeline(i32 index);

    EclRawHeader *eclFile;             // +0x000
    u32 *subTable;                     // +0x004
    EclTimelineState timelineState;    // +0x008
};
C_ASSERT(sizeof(EclManager) == 0x168);
C_ASSERT(offsetof(EclManager, timelineState) == 0x8);

DIFFABLE_EXTERN(EclManager, g_EclManager);

} // namespace th08
