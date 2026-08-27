#pragma once
#include "Global.hpp"
#include "AnmManager.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <windows.h>

namespace th08
{
struct EclExBarrierRenderState;
struct Effect;
struct RawStageInstr;
struct RawStageObject;
struct RawStageObjectInstance;

struct RawStageHeader
{
    i16 objectCount;
    i16 quadCount;
    i32 objectInstancesOffset;
    i32 scriptOffset;
    i32 unconsumedDword0C;
    char stageName[128];
    char songNames[4][128];
    char songPaths[4][128];
};
C_ASSERT(sizeof(RawStageHeader) == 0x490);
C_ASSERT(offsetof(RawStageHeader, unconsumedDword0C) == 0x0C);
C_ASSERT(offsetof(RawStageHeader, songNames) == 0x90);
C_ASSERT(offsetof(RawStageHeader, songPaths) == 0x290);

struct BackgroundCamera
{
    BackgroundCamera();

    Float3 position;
    Float3 lookAtOffset;
    Float3 up;
    Float3 forward;
    Float3 right;
    Float3 positionOffset;
    f32 fieldOfView;
};
C_ASSERT(sizeof(BackgroundCamera) == 0x4c);

struct BackgroundFog
{
    f32 nearPlane;
    f32 farPlane;
    ZunColor color;
};
C_ASSERT(sizeof(BackgroundFog) == 0xc);

enum SpellBackgroundState
{
    SPELL_BACKGROUND_INACTIVE,
    SPELL_BACKGROUND_FADING_IN,
    SPELL_BACKGROUND_ACTIVE,
};

struct Background
{
    Background();

    static ChainCallbackResult OnUpdate(Background *background);
    static ChainCallbackResult OnDrawHighPrio(Background *background);
    static ChainCallbackResult OnDrawLowPrio(Background *background);
    static ZunResult AddedCallback(Background *background);
    static ZunResult RegisterChain(i32 stageIndex);
    static ZunResult DeletedCallback(Background *background);
    static void CutChain();
    ZunResult LoadStageData(const char *path);
    ZunResult RenderObjects(i32 mode);

    void SetCamera1();
    void SetCamera2();
    void __fastcall InterpolateCameraVector(i32 index, Float3 *out, const Float3 *start,
                                            const Float3 *end, const Float3 *startTangent,
                                            const Float3 *endTangent);
    void AccumulateTint(D3DCOLOR color);
    u32 UpdateStageObjectVms();
    void StartSpellBackground();
    void StopSpellBackground();

    EclExBarrierRenderState &EclExBarrierState()
    {
        return *reinterpret_cast<EclExBarrierRenderState *>(&this->spellVmCount);
    }

    void *&EclExUpdateCallback()
    {
        return *reinterpret_cast<void **>(&this->spellBackgroundDrawCallback);
    }

    AnmVm *stageObjectVms;
    AnmVm stageVm0;
    AnmVm stageVm1;
    AnmVm stageVm2;
    AnmLoaded *stageAnmFile;
    RawStageHeader *stageData;
    i32 stageQuadCount;
    i32 stageObjectCount;
    RawStageObject **stageObjects;
    RawStageObjectInstance *stageObjectInstances;
    RawStageInstr *stageScript;
    ZunTimer stageScriptTimer;
    i32 stageScriptInstructionIndex;
    i32 frameCounter;
    i32 registeredStage;
    Float3 stagePosition;
    D3DCOLOR clearColor;
    u8 stageTextUsesYoukaiMode;
    ZunTimer stageTextTimer;
    AnmVm stageTextVm;
    Effect *stageEffect;
    BackgroundFog skyFog;
    BackgroundFog skyFogInterpInitial;
    BackgroundFog skyFogInterpFinal;
    i32 skyFogInterpDuration;
    ZunTimer skyFogInterpTimer;
    u8 skyFogNeedsSetup;
    i32 spellBackgroundState;
    i32 spellBackgroundTimer;
    i32 clearPending;
    i32 spellVmCount;
    i32 spellVmScriptBase;
    AnmVm spellVms[0x20];
    AnmVm spellAuxVm;
    void (*spellBackgroundDrawCallback)();
    i32 pendingStageScriptLabel;
    BackgroundCamera cameraTarget;
    BackgroundCamera cameraInterpolationStart;
    BackgroundCamera cameraInterpolationTangentEnd;
    BackgroundCamera cameraInterpolationTangentStart;
    BackgroundCamera cameraCurrent;
    i32 cameraInterpolationDuration[5];
    ZunTimer cameraInterpolationTimers[5];
    i32 cameraInterpolationModes[5];
    Float3 stagePositionTarget;
    i32 stagePositionEndFrame;
    Float3 stagePositionInitial;
    i32 stagePositionStartFrame;
    u8 compensateCameraJump;
    ZunColor tint;
    i32 retainTint;
    f32 cullingDistanceSq;
    u8 cameraMotionMode;
    i32 specialEffectPointCount;
    i32 collectSpecialEffectPoints;
    Float3 specialEffectPoints[0x20];
};
C_ASSERT(sizeof(Background) == 0x6600);
C_ASSERT(offsetof(Background, stageData) == 0x7F4);
C_ASSERT(offsetof(Background, stageScriptInstructionIndex) == 0x818);
C_ASSERT(offsetof(Background, stagePosition) == 0x824);
C_ASSERT(offsetof(Background, clearColor) == 0x830);
C_ASSERT(offsetof(Background, stageTextUsesYoukaiMode) == 0x834);
C_ASSERT(offsetof(Background, stageTextTimer) == 0x838);
C_ASSERT(offsetof(Background, stageEffect) == 0xAE8);
C_ASSERT(offsetof(Background, skyFog) == 0xAEC);
C_ASSERT(offsetof(Background, spellBackgroundState) == 0xB24);
C_ASSERT(offsetof(Background, spellVmCount) == 0xB30);
C_ASSERT(offsetof(Background, spellVms) == 0xB38);
C_ASSERT(offsetof(Background, spellBackgroundDrawCallback) == 0x625C);
C_ASSERT(offsetof(Background, pendingStageScriptLabel) == 0x6260);
C_ASSERT(offsetof(Background, cameraTarget) == 0x6264);
C_ASSERT(offsetof(Background, cameraCurrent) == 0x6394);
C_ASSERT(offsetof(Background, cameraInterpolationDuration) == 0x63E0);
C_ASSERT(offsetof(Background, stagePositionTarget) == 0x6444);
C_ASSERT(offsetof(Background, tint) == 0x6468);
C_ASSERT(offsetof(Background, cullingDistanceSq) == 0x6470);
C_ASSERT(offsetof(Background, specialEffectPointCount) == 0x6478);
C_ASSERT(offsetof(Background, specialEffectPoints) == 0x6480);

DIFFABLE_EXTERN(Background, g_Background);
DIFFABLE_EXTERN_ARRAY(const char *, 9, g_StageEnemyAnms);
DIFFABLE_EXTERN_ARRAY(const char *, 17, g_SpellEnemyAnms);
DIFFABLE_EXTERN_ARRAY(const char *, 9, g_StageEclFiles);
DIFFABLE_EXTERN_ARRAY(const char *, 9, g_StageSpellEclFiles);
DIFFABLE_EXTERN_ARRAY(const char *, 17, g_SpellEclFiles);
DIFFABLE_EXTERN_ARRAY(const char *, 9, g_GuiStageTextAnmPaths);
DIFFABLE_EXTERN_ARRAY(const char *, 15, g_EffectAnms);
}; // Namespace th08
