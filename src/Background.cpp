#include "th_pch.h"

#include "AnmManager.hpp"
#include "Background.hpp"
#include "Gui.hpp"
#include "ScreenEffect.hpp"
#include "EclManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"

namespace th08
{
ZunBool IsDisableResourceReload();
f32 __stdcall CubicHermiteInterpolate(f32 value0, f32 value1, f32 value2, f32 value3, f32 time);
u8 MixColors(u8 color1, u8 color2);

struct RawStageQuadBasic
{
    i16 type;
    i16 byteSize;
    i16 anmScript;
    i16 vmIdx;
    D3DXVECTOR3 position;
    D3DXVECTOR2 size;
};
C_ASSERT(sizeof(RawStageQuadBasic) == 0x1c);

struct RawStageObject
{
    i16 id;
    i8 zLevel;
    i8 flags;
    D3DXVECTOR3 position;
    D3DXVECTOR3 size;
    RawStageQuadBasic firstQuad;
};
C_ASSERT(sizeof(RawStageObject) == 0x38);

struct RawStageObjectInstance
{
    i16 id;
    i16 serializedReserved02;
    Float3 position;
};
C_ASSERT(sizeof(RawStageObjectInstance) == 0x10);
C_ASSERT(offsetof(RawStageObjectInstance, serializedReserved02) == 0x2);
C_ASSERT(offsetof(RawStageObjectInstance, position) == 0x4);

struct RawStageInstr
{
    i32 frame;
    i16 opcode;
    i16 size;
    i32 args[3];
};
C_ASSERT(sizeof(RawStageInstr) == 0x14);

struct BackgroundAnmVmSnapshot
{
    u32 words[0xA9];
};
C_ASSERT(sizeof(BackgroundAnmVmSnapshot) == sizeof(AnmVm));

struct RawStageQuadType1
{
    i16 type;
    i16 byteSize;
    i16 anmScript;
    i16 vmIdx;
    Float3 position1;
    Float3 position2;
    f32 width;
};
C_ASSERT(sizeof(RawStageQuadType1) == 0x24);

struct BackgroundStageVertex
{
    BackgroundStageVertex()
    {
    }

    Float3 pos;
    f32 w;
    ZunColor diffuse;
    Float2 textureUV;
};
C_ASSERT(sizeof(BackgroundStageVertex) == 0x1c);
DIFFABLE_STATIC(Background, g_Background);

DIFFABLE_STATIC(ChainElem, g_BackgroundCalcChain);
DIFFABLE_STATIC(ChainElem, g_BackgroundDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_BackgroundDrawChainLowPrio);

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageAnmFiles) = {
    "stg1bg.anm", "stg2bg.anm", "stg3bg.anm", "stg4abg.anm", "stg4abg.anm",
    "stg5bg.anm", "stg6bg.anm", "stg7bg.anm", "stg8bg.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageStdFiles) = {
    "stage1.std", "stage2.std", "stage3.std", "stage4a.std", "stage4b.std",
    "stage5.std", "stage6.std", "stage7.std", "stage8.std"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageStdFilesSpell) = {
    "stage1_s.std", "stage2_s.std", "stage3_s.std", "stage4a_s.std", "stage4b_s.std",
    "stage5_s.std", "stage6_s.std", "stage7_s.std", "stage8_s.std"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageEnemyAnms) = {
    "stg1enm.anm", "stg2enm.anm", "stg3enm.anm", "stg4aenm.anm", "stg4benm.anm",
    "stg5enm.anm", "stg6enm.anm", "stg7enm.anm", "stg8enm.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 17, g_SpellEnemyAnms) = {
    "stg1enm.anm", "stg2enm.anm", "stg3enm.anm", "stg5enm.anm", "stg6enm.anm",
    "stg7enm.anm", "stg8enm.anm", "stg5enm.anm", "stg8enm.anm", "stg4aenm.anm",
    "stg4benm.anm", "stgenm_sk.anm", "stgenm_ym.anm", "stgenm_al.anm", "stgenm_rm.anm",
    "stgenm_yy.anm", "stgenm_yk.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageEclFiles) = {
    "ecldata1.ecl", "ecldata2.ecl", "ecldata3.ecl", "ecldata4a.ecl", "ecldata4b.ecl",
    "ecldata5.ecl", "ecldata6.ecl", "ecldata7.ecl", "ecldata8.ecl"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageSpellEclFiles) = {
    "ecldata1sp.ecl", "ecldata2sp.ecl", "ecldata3sp.ecl", "ecldata4asp.ecl", "ecldata4bsp.ecl",
    "ecldata5sp.ecl", "ecldata6sp.ecl", "ecldata7sp.ecl", "ecldata8sp.ecl"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 17, g_SpellEclFiles) = {
    "ecldata1sp.ecl", "ecldata2sp.ecl", "ecldata3sp.ecl", "ecldata5sp.ecl", "ecldata6sp.ecl",
    "ecldata7sp.ecl", "ecldata8sp.ecl", "ecldata5sp.ecl", "ecldata8sp.ecl", "ecldata4asp.ecl",
    "ecldata4bsp.ecl", "ecldata_sk.ecl", "ecldata_ym.ecl", "ecldata_al.ecl", "ecldata_rm.ecl",
    "ecldata_yy.ecl", "ecldata_yk.ecl"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_GuiStageTextAnmPaths) = {
    "stg1txt.anm", "stg2txt.anm", "stg3txt.anm", "stg4atxt.anm", "stg4btxt.anm", "stg5txt.anm",
    "stg6txt.anm", "stg7txt.anm", "stg8txt.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 15, g_EffectAnms) = {
    "eff01.anm", "eff02.anm", "eff03.anm", "eff04a.anm", "eff04b.anm",
    "eff05.anm", "eff06.anm", "eff07.anm", "eff08.anm", "eff09sk.anm",
    "eff09ym.anm", "eff09al.anm", "eff09rm.anm", "eff09yy.anm", "eff09yk.anm"};

// FUNCTION: th08 0x4071a0
Background::Background()
{
    memset(this, 0, sizeof(Background));
    *reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.position) = D3DXVECTOR3(0, 0, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.lookAtOffset) = D3DXVECTOR3(0, 0, 0);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.up) = D3DXVECTOR3(0, 1.0f, 0);
    this->cameraCurrent.fieldOfView = 0.5235987901687622f;
    this->cameraTarget = this->cameraCurrent;
    this->cameraInterpolationStart = this->cameraCurrent;
}

// FUNCTION: th08 0x4073b0
BackgroundCamera::BackgroundCamera()
{
}

// FUNCTION: th08 0x407400
#pragma var_order(curInsn, pos, spawnedStageEffect)
ChainCallbackResult Background::OnUpdate(Background *background)
{
    RawStageInstr *curInsn;
    D3DXVECTOR3 pos;
    AnmVm *spawnedStageEffect;

    if (background->stageData == NULL)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_GameManager.flags.deathbombFreezeActive)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (g_GameManager.currentStage == STAGE6B)
    {
        if (background->stageEffect == NULL)
        {
            Float3 zeroVector(0.0f, 0.0f, 0.0f);
            background->stageEffect = reinterpret_cast<Effect *>(
                g_EffectManager.SpawnEffectInFixedSlot(0x40, reinterpret_cast<D3DXVECTOR3 *>(&zeroVector), 0xC, 1, -1));
            spawnedStageEffect = &background->stageEffect->vm;
            background->stageAnmFile->SetAndExecuteScriptIdx(spawnedStageEffect, 11);
        }
        else if (background->pendingStageScriptLabel == 1)
        {
            AnmVm *stageEffect1 = &background->stageEffect->vm;
            background->stageAnmFile->SetAndExecuteScriptIdx(stageEffect1, 11);
        }
        else if (background->pendingStageScriptLabel == 2)
        {
            AnmVm *stageEffect2 = &background->stageEffect->vm;
            BackgroundAnmVmSnapshot savedStageVm2 = *reinterpret_cast<BackgroundAnmVmSnapshot *>(stageEffect2);
            background->stageAnmFile->SetAndExecuteScriptIdx(stageEffect2, 12);
            stageEffect2->SetInterrupt(2);
            stageEffect2->posFinal = reinterpret_cast<AnmVm *>(&savedStageVm2)->posFinal;
            stageEffect2->posInitial = reinterpret_cast<AnmVm *>(&savedStageVm2)->posInitial;
            stageEffect2->interpCurrentTimers[0] = reinterpret_cast<AnmVm *>(&savedStageVm2)->interpCurrentTimers[0];
            stageEffect2->interpEndTimers[0] = reinterpret_cast<AnmVm *>(&savedStageVm2)->interpEndTimers[0];
            stageEffect2->interpModes[0] = reinterpret_cast<AnmVm *>(&savedStageVm2)->interpModes[0];
            stageEffect2->color1 = reinterpret_cast<AnmVm *>(&savedStageVm2)->color1;
        }
        else if (background->pendingStageScriptLabel == 3)
        {
            AnmVm *stageEffect3 = &background->stageEffect->vm;
            BackgroundAnmVmSnapshot savedStageVm3 = *reinterpret_cast<BackgroundAnmVmSnapshot *>(stageEffect3);
            stageEffect3->SetInterrupt(3);
            stageEffect3->posFinal = reinterpret_cast<AnmVm *>(&savedStageVm3)->posFinal;
            stageEffect3->posInitial = reinterpret_cast<AnmVm *>(&savedStageVm3)->posInitial;
            stageEffect3->interpCurrentTimers[0] = reinterpret_cast<AnmVm *>(&savedStageVm3)->interpCurrentTimers[0];
            stageEffect3->interpEndTimers[0] = reinterpret_cast<AnmVm *>(&savedStageVm3)->interpEndTimers[0];
            stageEffect3->interpModes[0] = reinterpret_cast<AnmVm *>(&savedStageVm3)->interpModes[0];
            stageEffect3->color1 = reinterpret_cast<AnmVm *>(&savedStageVm3)->color1;
        }
        else if (background->pendingStageScriptLabel == 4)
        {
            AnmVm *stageEffect4 = &background->stageEffect->vm;
            BackgroundAnmVmSnapshot savedStageVm4 = *reinterpret_cast<BackgroundAnmVmSnapshot *>(stageEffect4);
            stageEffect4->SetInterrupt(4);
            stageEffect4->posFinal = reinterpret_cast<AnmVm *>(&savedStageVm4)->posFinal;
            stageEffect4->posInitial = reinterpret_cast<AnmVm *>(&savedStageVm4)->posInitial;
            stageEffect4->interpCurrentTimers[0] = reinterpret_cast<AnmVm *>(&savedStageVm4)->interpCurrentTimers[0];
            stageEffect4->interpEndTimers[0] = reinterpret_cast<AnmVm *>(&savedStageVm4)->interpEndTimers[0];
            stageEffect4->interpModes[0] = reinterpret_cast<AnmVm *>(&savedStageVm4)->interpModes[0];
            stageEffect4->color1 = reinterpret_cast<AnmVm *>(&savedStageVm4)->color1;
        }
    }

    if (background->pendingStageScriptLabel != 0)
    {
        i32 seekIndex = 0;
        curInsn = background->stageScript;
        background->stageScriptInstructionIndex = 0;
        while ((curInsn->opcode != 0x1F || background->pendingStageScriptLabel != curInsn->args[0]) && curInsn->frame != -1)
        {
            curInsn++;
            seekIndex++;
        }
        if (curInsn->frame != -1)
        {
            background->stageScriptInstructionIndex = seekIndex + 1;
            background->stageScriptTimer = curInsn->frame;
            background->pendingStageScriptLabel = 0;
        }
    }

read_instruction:
    curInsn = background->stageScript + background->stageScriptInstructionIndex;
    if (background->stageScriptTimer >= curInsn->frame)
    {
        if (curInsn->frame != -1)
        {
    switch (curInsn->opcode)
    {
    case 0:
        if (curInsn->frame == -1)
        {
            background->stagePositionInitial = *reinterpret_cast<Float3 *>(curInsn->args);
            background->stagePosition.x = background->stagePositionInitial.x;
            background->stagePosition.y = background->stagePositionInitial.y;
            background->stagePosition.z = background->stagePositionInitial.z;
        }
        else
        {
            pos = *reinterpret_cast<D3DXVECTOR3 *>(curInsn->args);
            background->stagePosition.x = pos.x;
            background->stagePosition.y = pos.y;
            background->stagePosition.z = pos.z;
            background->stagePositionInitial = *reinterpret_cast<Float3 *>(&pos);
            background->stagePositionStartFrame = curInsn->frame;
            curInsn++;
            background->stagePositionEndFrame = curInsn->frame;
            background->stagePositionTarget = *reinterpret_cast<Float3 *>(curInsn->args);
        }
        break;
    case 1:
        background->skyFog.color.d3dColor = curInsn->args[0];
        background->skyFog.nearPlane = *reinterpret_cast<f32 *>(&curInsn->args[1]);
        background->skyFog.farPlane = *reinterpret_cast<f32 *>(&curInsn->args[2]);
        background->skyFogInterpFinal = background->skyFog;
        break;
    case 2:
        background->skyFogInterpInitial = background->skyFog;
        background->skyFogInterpDuration = curInsn->args[0];
        background->skyFogInterpTimer = 0;
        break;
    case 5:
        if (background->compensateCameraJump)
        {
            Float3 cameraDelta = *reinterpret_cast<Float3 *>(curInsn->args) - background->cameraTarget.position;
            ShiftStageEffectOrigins(&cameraDelta);
            background->compensateCameraJump = 0;
        }
        background->cameraInterpolationStart.position = background->cameraTarget.position;
        background->cameraTarget.position = *reinterpret_cast<Float3 *>(curInsn->args);
        if (background->cameraInterpolationDuration[0] == 0)
            background->cameraCurrent.position = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 6:
        background->cameraInterpolationDuration[0] = curInsn->args[0];
        background->cameraInterpolationTimers[0] = 0;
        background->cameraInterpolationModes[0] = curInsn->args[1];
        break;
    case 7:
        background->cameraInterpolationStart.lookAtOffset = background->cameraTarget.lookAtOffset;
        background->cameraTarget.lookAtOffset = *reinterpret_cast<Float3 *>(curInsn->args);
        if (background->cameraInterpolationDuration[1] == 0)
            background->cameraCurrent.lookAtOffset = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 8:
        background->cameraInterpolationDuration[1] = curInsn->args[0];
        background->cameraInterpolationTimers[1] = 0;
        background->cameraInterpolationModes[1] = curInsn->args[1];
        break;
    case 9:
        background->cameraInterpolationStart.up = background->cameraTarget.up;
        background->cameraTarget.up = *reinterpret_cast<Float3 *>(curInsn->args);
        if (background->cameraInterpolationDuration[2] == 0)
            background->cameraCurrent.up = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 10:
        background->cameraInterpolationDuration[2] = curInsn->args[0];
        background->cameraInterpolationModes[2] = curInsn->args[1];
        background->cameraInterpolationTimers[2] = 0;
        break;
    case 11:
        background->cameraInterpolationStart.fieldOfView = background->cameraTarget.fieldOfView;
        background->cameraTarget.fieldOfView = *reinterpret_cast<f32 *>(&curInsn->args[0]);
        if (background->cameraInterpolationDuration[3] == 0)
            background->cameraCurrent.fieldOfView = *reinterpret_cast<f32 *>(&curInsn->args[0]);
        break;
    case 12:
        background->cameraInterpolationDuration[3] = curInsn->args[0];
        background->cameraInterpolationTimers[3] = 0;
        background->cameraInterpolationModes[3] = curInsn->args[1];
        break;
    case 13:
        background->clearColor = curInsn->args[0];
        break;
    case 3:
        if (background->pendingStageScriptLabel != 0)
        {
            background->pendingStageScriptLabel = 0;
            break;
        }
        goto instructions_done;
    case 4:
        background->stageScriptInstructionIndex = curInsn->args[0];
        background->stageScriptTimer = curInsn->args[1];
        background->cameraInterpolationDuration[0] = 0;
        background->compensateCameraJump = 1;
        goto read_instruction;
    case 14: background->cameraInterpolationStart.position = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 15: background->cameraTarget.position = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 16: background->cameraInterpolationTangentStart.position = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 17: background->cameraInterpolationTangentEnd.position = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 18:
        background->cameraInterpolationDuration[0] = curInsn->args[0];
        background->cameraInterpolationTimers[0] = 0;
        background->cameraInterpolationModes[0] = 7;
        break;
    case 19: background->cameraInterpolationStart.lookAtOffset = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 20: background->cameraTarget.lookAtOffset = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 21: background->cameraInterpolationTangentStart.lookAtOffset = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 22: background->cameraInterpolationTangentEnd.lookAtOffset = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 23:
        background->cameraInterpolationDuration[1] = curInsn->args[0];
        background->cameraInterpolationTimers[1] = 0;
        background->cameraInterpolationModes[1] = 7;
        break;
    case 24: background->cameraInterpolationStart.up = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 25: background->cameraTarget.up = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 26: background->cameraInterpolationTangentStart.up = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 27: background->cameraInterpolationTangentEnd.up = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 28:
        background->cameraInterpolationDuration[2] = curInsn->args[0];
        background->cameraInterpolationTimers[2] = 0;
        background->cameraInterpolationModes[2] = 7;
        break;
    case 29:
        if (curInsn->args[0] >= 0) background->stageAnmFile->ExecuteAnmIdx(&background->stageVm0, curInsn->args[0]);
        else background->stageVm0.activeSpriteIndex = -1;
        break;
    case 30:
        if (curInsn->args[0] >= 0) background->stageAnmFile->ExecuteAnmIdx(&background->stageVm1, curInsn->args[0]);
        else background->stageVm0.activeSpriteIndex = -1;
        break;
    case 33:
        background->cameraMotionMode = *reinterpret_cast<u8 *>(&curInsn->args[0]);
        background->cameraInterpolationDuration[4] = 0;
        background->cameraInterpolationTimers[4] = 0;
        background->cameraInterpolationModes[4] = 0;
        break;
    case 32:
        background->cameraCurrent.positionOffset = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 34:
        if (curInsn->args[0] >= 0) background->stageAnmFile->ExecuteAnmIdx(&background->stageVm2, curInsn->args[0]);
        else background->stageVm2.activeSpriteIndex = -1;
        break;
    case 31:
        break;
    default:
        break;
    }

    background->stageScriptInstructionIndex++;
    goto read_instruction;
        }
    }

instructions_done:
    {
#pragma var_order(interpolationIndex, interpolationDelta, interpolationTime, angle1, angle2, angle3, fogInterpRatio, i, j, spawnedEffect, k)
    i32 interpolationIndex;
    f32 interpolationDelta;
    f32 interpolationTime;
    f32 angle1;
    f32 angle2;
    f32 angle3;
    f32 fogInterpRatio;
    i32 i;
    i32 j;
    AnmVm *spawnedEffect;
    i32 k;

    interpolationIndex = 0;
    if (background->cameraInterpolationDuration[interpolationIndex] != 0)
        background->InterpolateCameraVector(interpolationIndex, &background->cameraCurrent.position, &background->cameraInterpolationStart.position,
                                 &background->cameraTarget.position, &background->cameraInterpolationTangentStart.position, &background->cameraInterpolationTangentEnd.position);
    interpolationIndex = 1;
    if (background->cameraInterpolationDuration[interpolationIndex] != 0)
        background->InterpolateCameraVector(interpolationIndex, &background->cameraCurrent.lookAtOffset, &background->cameraInterpolationStart.lookAtOffset,
                                 &background->cameraTarget.lookAtOffset, &background->cameraInterpolationTangentStart.lookAtOffset, &background->cameraInterpolationTangentEnd.lookAtOffset);
    interpolationIndex = 2;
    if (background->cameraInterpolationDuration[interpolationIndex] != 0)
        background->InterpolateCameraVector(interpolationIndex, &background->cameraCurrent.up, &background->cameraInterpolationStart.up,
                                 &background->cameraTarget.up, &background->cameraInterpolationTangentStart.up, &background->cameraInterpolationTangentEnd.up);
    interpolationIndex = 3;
    if (background->cameraInterpolationDuration[interpolationIndex] != 0)
    {
        if (background->cameraInterpolationTimers[interpolationIndex] < background->cameraInterpolationDuration[interpolationIndex])
        {
            background->cameraInterpolationTimers[interpolationIndex]++;
            interpolationTime = (f32)background->cameraInterpolationTimers[interpolationIndex] /
                                background->cameraInterpolationDuration[interpolationIndex];
        }
        else
        {
            background->cameraInterpolationTimers[interpolationIndex] = background->cameraInterpolationDuration[interpolationIndex];
            interpolationTime = 1.0f;
            background->cameraInterpolationDuration[interpolationIndex] = 0;
        }
        switch (background->cameraInterpolationModes[interpolationIndex])
        {
        case 1: interpolationTime = 1.0f - interpolationTime; interpolationTime = 1.0f - interpolationTime * interpolationTime; break;
        case 2: interpolationTime = 1.0f - interpolationTime; interpolationTime = 1.0f - interpolationTime * interpolationTime * interpolationTime; break;
        case 3: interpolationTime = 1.0f - interpolationTime; interpolationTime = 1.0f - interpolationTime * interpolationTime * interpolationTime * interpolationTime; break;
        case 4: interpolationTime = interpolationTime * interpolationTime; break;
        case 5: interpolationTime = interpolationTime * interpolationTime * interpolationTime; break;
        case 6: interpolationTime = interpolationTime * interpolationTime * interpolationTime * interpolationTime; break;
        }
        interpolationDelta = background->cameraTarget.fieldOfView - background->cameraInterpolationStart.fieldOfView;
        background->cameraCurrent.fieldOfView = interpolationDelta * interpolationTime + background->cameraInterpolationStart.fieldOfView;
    }

    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&background->cameraCurrent.forward),
                      reinterpret_cast<D3DXVECTOR3 *>(&background->cameraCurrent.lookAtOffset));

    if (background->cameraMotionMode != 0)
    {
        switch (background->cameraMotionMode)
        {
        case 1:
        {
            angle1 = (f32)background->cameraInterpolationTimers[4] * ZUN_PI * 2.0f / 480.0f - ZUN_PI;
            background->cameraCurrent.positionOffset.x = sinf(angle1) * 40.0f;
            background->cameraInterpolationTimers[4]++;
            if (background->cameraInterpolationTimers[4] >= 480) background->cameraInterpolationTimers[4] = 0;
            break;
        }
        case 2:
        {
            angle2 = (f32)background->cameraInterpolationTimers[4] * ZUN_PI * 2.0f / 480.0f - ZUN_PI;
            background->cameraCurrent.positionOffset.x = sinf(angle2) * 70.0f;
            background->cameraCurrent.up.x = -sinf(angle2) * 0.1f;
            background->cameraInterpolationTimers[4]++;
            if (background->cameraInterpolationTimers[4] >= 480) background->cameraInterpolationTimers[4] = 0;
            break;
        }
        case 3:
        {
            angle3 = (f32)background->cameraInterpolationTimers[4] * ZUN_PI * 2.0f / 4800.0f - ZUN_PI;
            background->cameraCurrent.up.x = sinf(angle3) * 1.0f;
            background->cameraCurrent.up.z = cosf(angle3) * 1.0f;
            background->cameraInterpolationTimers[4]++;
            if (background->cameraInterpolationTimers[4] >= 4800) background->cameraInterpolationTimers[4] = 0;
            break;
        }
        }
    }

    if (background->skyFogInterpDuration != 0)
    {
        background->skyFogInterpTimer++;
        fogInterpRatio =
            (f32)background->skyFogInterpTimer / background->skyFogInterpDuration;
        if (fogInterpRatio >= 1.0f) fogInterpRatio = 1.0f;
        for (i = 0; i < 4; i++)
        {
            reinterpret_cast<u8 *>(&background->skyFog.color)[i] =
                (u8)(((f32)reinterpret_cast<u8 *>(&background->skyFogInterpFinal.color)[i] -
                      (f32)reinterpret_cast<u8 *>(&background->skyFogInterpInitial.color)[i]) * fogInterpRatio +
                     (f32)reinterpret_cast<u8 *>(&background->skyFogInterpInitial.color)[i]);
        }
        background->skyFog.nearPlane =
            (background->skyFogInterpFinal.nearPlane - background->skyFogInterpInitial.nearPlane) *
                fogInterpRatio +
            background->skyFogInterpInitial.nearPlane;
        background->skyFog.farPlane =
            (background->skyFogInterpFinal.farPlane - background->skyFogInterpInitial.farPlane) * fogInterpRatio +
            background->skyFogInterpInitial.farPlane;
        if (background->skyFogInterpTimer >= background->skyFogInterpDuration) background->skyFogInterpDuration = 0;
    }

    if (curInsn->opcode != 3)
        background->stageScriptTimer++;
    background->UpdateStageObjectVms();

    if (background->spellBackgroundState >= SPELL_BACKGROUND_FADING_IN)
    {
        if (background->spellBackgroundTimer == 60) background->spellBackgroundState++;
        background->spellBackgroundTimer++;
        for (j = 0; j < background->spellVmCount; j++)
            g_AnmManager->ExecuteScript(&background->spellVms[j]);
    }
    if (background->stageVm0.activeSpriteIndex > 0) g_AnmManager->ExecuteScript(&background->stageVm0);
    if (background->stageVm1.activeSpriteIndex > 0) g_AnmManager->ExecuteScript(&background->stageVm1);
    if (background->stageVm2.activeSpriteIndex > 0)
    {
        g_AnmManager->ExecuteScript(&background->stageVm2);
        background->clearColor = background->stageVm2.color1.d3dColor;
    }

    if (background->frameCounter % 3 == 0 &&
        (background->frameCounter >= 700 || g_GameManager.IsSpellPractice()) &&
        background->spellBackgroundState < SPELL_BACKGROUND_ACTIVE)
    {
        for (k = 0; k < 12; k++)
        {
            spawnedEffect = g_EffectManager.SpawnEffect(62, reinterpret_cast<D3DXVECTOR3 *>(&background->specialEffectPoints[k]),
                                                        1, 0x20FFFFFF);
            reinterpret_cast<Effect *>(spawnedEffect)->drawGroup = 4;
        }
    }
    background->collectSpecialEffectPoints = 1;
    if (background->spellBackgroundState >= SPELL_BACKGROUND_ACTIVE)
        background->specialEffectPointCount = 0;

    background->frameCounter++;
    if (background->frameCounter % 500 == 250 && g_GameManager.IsTampered())
    {
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

}

// FUNCTION: th08 0x408d60
void __fastcall Background::InterpolateCameraVector(i32 index, Float3 *out, const Float3 *start, const Float3 *end,
                                         const Float3 *control2, const Float3 *control3)
{
    f32 time;

    if (this->cameraInterpolationTimers[index] < this->cameraInterpolationDuration[index])
    {
        this->cameraInterpolationTimers[index]++;
        time = (f32)this->cameraInterpolationTimers[index] / this->cameraInterpolationDuration[index];
    }
    else
    {
        this->cameraInterpolationTimers[index] = this->cameraInterpolationDuration[index];
        time = 1.0f;
        this->cameraInterpolationDuration[index] = 0;
    }

    switch (this->cameraInterpolationModes[index])
    {
    case 1:
        time = 1.0f - time;
        time = 1.0f - time * time;
        break;
    case 2:
        time = 1.0f - time;
        time = 1.0f - time * time * time;
        break;
    case 3:
        time = 1.0f - time;
        time = 1.0f - time * time * time * time;
        break;
    case 4:
        time = time * time;
        break;
    case 5:
        time = time * time * time;
        break;
    case 6:
        time = time * time * time * time;
        break;
    }

    if (this->cameraInterpolationModes[index] != 7)
    {
        *out = *end - *start;
        *out = (*out * time) + *start;
    }
    else
    {
        out->x = CubicHermiteInterpolate(start->x, end->x, control2->x, control3->x, time);
        out->y = CubicHermiteInterpolate(start->y, end->y, control2->y, control3->y, time);
        out->z = CubicHermiteInterpolate(start->z, end->z, control2->z, control3->z, time);
    }
}

// FUNCTION: th08 0x408fc0
#pragma var_order(weight3, weight1, weight2, weight0)
f32 __stdcall CubicHermiteInterpolate(f32 value0, f32 value1, f32 value2, f32 value3, f32 time)
{
    f32 weight0;
    f32 weight1;
    f32 weight2;
    f32 weight3;

    weight0 = (time - 1.0f) * (time - 1.0f) * (2.0f * time + 1.0f);
    weight1 = time * time * (3.0f - 2.0f * time);
    weight2 = (1.0f - time) * (1.0f - time) * time;
    weight3 = (time - 1.0f) * time * time;
    return weight0 * value0 + weight1 * value1 + weight2 * value2 + weight3 * value3;
}

// FUNCTION: th08 0x409080
Float3 Float3::operator+(const Float3 &other) const
{
    return Float3(this->x + other.x, this->y + other.y, this->z + other.z);
}

// FUNCTION: th08 0x4090d0
Float3 Float3::operator-(const Float3 &other) const
{
    return Float3(this->x - other.x, this->y - other.y, this->z - other.z);
}

// FUNCTION: th08 0x409120
Float3 Float3::operator*(f32 scalar) const
{
    return Float3(this->x * scalar, this->y * scalar, this->z * scalar);
}

// FUNCTION: th08 0x409160
#pragma var_order(color2, this)
void Background::AccumulateTint(D3DCOLOR color)
{
    ZunColor color2;

    if (this->tint.a == 0)
    {
        this->tint.d3dColor = color;
    }
    else
    {
        color2.d3dColor = color;
        this->tint.r = ((u32)color2.r + this->tint.r) >> 1;
        this->tint.g = ((u32)color2.g + this->tint.g) >> 1;
        this->tint.b = ((u32)color2.b + this->tint.b) >> 1;
        this->tint.a = ((u32)color2.a + this->tint.a) >> 1;
    }
}

// FUNCTION: th08 0x409200
#pragma var_order(i, viewport, effect, rect, fogColor, background)
ChainCallbackResult Background::OnDrawHighPrio(Background *background)
{
    i32 i;
    D3DVIEWPORT8 viewport;
    AnmVm *effect;
    ZunRect rect;
    ZunColor fogColor;

    background->specialEffectPointCount = 0;
    for (i = 0; i < 16; i++)
    {
        background->specialEffectPoints[i] = Float3(0.0f, 0.0f, 0.0f);
    }

    g_Supervisor.viewport.X = 32;
    g_Supervisor.viewport.Y = 16;
    g_Supervisor.viewport.Width = 384;
    g_Supervisor.viewport.Height = 448;

    g_AnmManager->ClearVertexBuffer();
    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();
    g_AnmManager->ResetFrameDebugInfo();
    g_AnmManager->ClearCameraSettings();
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.DisableFog();
    }
    g_AnmManager->FlushVertexBuffer();

    if (background->clearPending != 0)
    {
        viewport.X = 32;
        viewport.Y = 16;
        viewport.Width = 384;
        viewport.Height = 448;
        g_Supervisor.d3dDevice->SetViewport(&viewport);
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, COLOR_BLACK, 1.0f, 0);
        background->clearPending = 0;
    }
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    if (background->tint.a > 0)
    {
        g_AnmManager->SetMixColor(background->tint.d3dColor);
    }
    background->tint.a = 0;
    background->tint.r = 0x80;
    background->tint.g = 0x80;
    background->tint.b = 0x80;

    if (background->spellBackgroundState <= SPELL_BACKGROUND_FADING_IN &&
        !g_Gui.IsDialoguePresent())
    {
        if (background->stageVm0.activeSpriteIndex > 0)
        {
            g_AnmManager->Draw2DAndFlush(&background->stageVm0);
        }
        if (background->stageVm1.activeSpriteIndex > 0)
        {
            g_AnmManager->Draw2DAndFlush(&background->stageVm1);
        }
        if (background->stageEffect != NULL)
        {
            effect = &background->stageEffect->vm;
            (reinterpret_cast<void (__fastcall *)(AnmVm *)>(
                reinterpret_cast<Effect *>(effect)->drawCallback))(effect);
        }
    }

    if ((background->clearColor & COLOR_ALPHA_MASK) == COLOR_ALPHA_MASK)
    {
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                                      background->clearColor, 1.0f, 0);
    }
    else if (background->clearColor != 0)
    {
        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        ScreenEffect::DrawSquare(&rect, background->clearColor);
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, background->clearColor, 1.0f, 0);
    }
    else
    {
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, background->clearColor, 1.0f, 0);
    }

    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    if (!g_AnmManager->useMixColor)
    {
        g_Supervisor.SetRenderState(D3DRS_FOGCOLOR, background->skyFog.color.d3dColor);
    }
    else
    {
        fogColor.d3dColor = background->skyFog.color.d3dColor;
        fogColor.r = MixColors(fogColor.r, g_AnmManager->color.r);
        fogColor.g = MixColors(fogColor.g, g_AnmManager->color.g);
        fogColor.b = MixColors(fogColor.b, g_AnmManager->color.b);
        g_Supervisor.SetRenderState(D3DRS_FOGCOLOR, fogColor.d3dColor);
    }
    g_Supervisor.SetRenderState(D3DRS_FOGSTART,
                                *reinterpret_cast<u32 *>(&background->skyFog.nearPlane));
    g_Supervisor.SetRenderState(D3DRS_FOGEND,
                                *reinterpret_cast<u32 *>(&background->skyFog.farPlane));
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.EnableFog();
    }

    if (background->spellBackgroundState <= SPELL_BACKGROUND_FADING_IN &&
        !g_Gui.IsDialoguePresent())
    {
        background->RenderObjects(0);
        background->RenderObjects(1);
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}


// FUNCTION: th08 0x409640
#pragma var_order(zValue, alpha, rect, i, background)
ChainCallbackResult Background::OnDrawLowPrio(Background *background)
{
    ZunRect rect;
    i32 i;
    i32 alpha;
    f32 zValue;

    if (background->spellBackgroundState <= SPELL_BACKGROUND_FADING_IN &&
        !g_Gui.IsDialoguePresent())
    {
        background->RenderObjects(2);
        background->RenderObjects(3);
        if (!g_Supervisor.IsFogDisabled())
        {
            g_Supervisor.DisableFog();
        }
        g_EffectManager.DrawBackgroundEffects();
        if (background->spellBackgroundState == SPELL_BACKGROUND_FADING_IN)
        {
            rect.left = 32.0f;
            rect.top = 16.0f;
            rect.right = 416.0f;
            rect.bottom = 464.0f;
            alpha = (background->spellBackgroundTimer * 255) / 60;
            g_AnmManager->FlushVertexBuffer();
            g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
            if (!g_Supervisor.IsFogDisabled())
            {
                g_Supervisor.SetRenderState(D3DRS_FOGENABLE, FALSE);
            }
            ScreenEffect::DrawSquare(&rect, alpha << 24);
        }
    }

    g_AnmManager->FlushVertexBuffer();
    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.DisableFog();
    }

    if (background->spellBackgroundState >= SPELL_BACKGROUND_FADING_IN)
    {
        for (i = 0; i < background->spellVmCount; i++)
        {
            g_AnmManager->Draw2DAndFlush(&background->spellVms[i]);
        }
        if (background->spellBackgroundDrawCallback != NULL)
        {
            background->spellBackgroundDrawCallback();
        }
    }

    g_AnmManager->SetCameraMode(0);
    background->SetCamera1();
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    zValue = 1000.0f;
    g_Supervisor.SetRenderState(D3DRS_FOGSTART, *reinterpret_cast<u32 *>(&zValue));
    zValue = 2000.0f;
    g_Supervisor.SetRenderState(D3DRS_FOGEND, *reinterpret_cast<u32 *>(&zValue));
    if (background->retainTint == 0)
    {
        g_AnmManager->SetMixColorDefault();
    }
    background->retainTint = 0;
    background->collectSpecialEffectPoints = 0;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x409850
#pragma var_order(i, vector0, vector1, vector2, vector3, background)
ZunResult Background::AddedCallback(Background *background)
{
    i32 i;

    background->stageScriptTimer = 0;
    background->stageScriptInstructionIndex = 0;
    background->stagePosition.x = 0.0f;
    background->stagePosition.y = 0.0f;
    background->stagePosition.z = 0.0f;
    background->spellBackgroundState = SPELL_BACKGROUND_INACTIVE;
    background->skyFogInterpDuration = 0;

    if (!IsDisableResourceReload())
    {
        background->stageAnmFile = g_AnmManager->PreloadAnm(4, g_StageAnmFiles[g_GameManager.currentStage]);
        if (background->stageAnmFile == NULL)
        {
            return ZUN_ERROR;
        }
    }
    else
    {
        background->stageAnmFile = g_AnmManager->GetAnm(4);
    }

    if (!g_GameManager.IsSpellPractice())
    {
        if (background->LoadStageData(g_StageStdFiles[g_GameManager.currentStage]) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
    }
    else
    {
        if (background->LoadStageData(g_StageStdFilesSpell[g_GameManager.currentStage]) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
    }

    background->skyFog.color.d3dColor = 0xFF000000;
    background->skyFog.nearPlane = 200.0f;
    background->skyFog.farPlane = 500.0f;

    *reinterpret_cast<D3DXVECTOR3 *>(&background->cameraCurrent.position) = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->cameraCurrent.lookAtOffset) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->cameraCurrent.positionOffset) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->cameraCurrent.up) = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    background->cameraCurrent.fieldOfView = 0.5235987901687622f;
    background->cameraTarget = background->cameraCurrent;
    background->cameraInterpolationStart = background->cameraCurrent;

    background->cameraMotionMode = 0;
    for (i = 0; i < 4; i++)
    {
        background->cameraInterpolationDuration[i] = 0;
        background->cameraInterpolationTimers[i] = 0;
    }

    background->pendingStageScriptLabel = 0;
    background->cullingDistanceSq = 1322500.0f;
    if (g_GameManager.currentStage == 5)
    {
        background->cullingDistanceSq = 1822500.0f;
    }
    else if (g_GameManager.currentStage == 6 || g_GameManager.currentStage == 7)
    {
        background->cullingDistanceSq = 3240000.0f;
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409b20
#pragma var_order(stageData, background)
ZunResult Background::RegisterChain(i32 param)
{
    Background *background = &g_Background;
    RawStageHeader *stageData;

    if (IsDisableResourceReload())
    {
        stageData = background->stageData;
    }

    memset(background, 0, sizeof(Background));

    if (IsDisableResourceReload())
    {
        background->stageData = stageData;
    }

    background->frameCounter = 0;
    background->registeredStage = param;

    g_BackgroundCalcChain.SetCallback((ChainCallback)Background::OnUpdate);
    g_BackgroundCalcChain.addedCallback = (ChainLifetimeCallback)Background::AddedCallback;
    g_BackgroundCalcChain.deletedCallback = (ChainLifetimeCallback)Background::DeletedCallback;
    g_BackgroundCalcChain.arg = background;
    if (g_Chain.AddToCalcChain(&g_BackgroundCalcChain, CHAIN_PRIO_CALC_BACKGROUND) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_BackgroundDrawChainHighPrio.SetCallback((ChainCallback)Background::OnDrawHighPrio);
    g_BackgroundDrawChainHighPrio.arg = background;
    g_Chain.AddToDrawChain(&g_BackgroundDrawChainHighPrio, CHAIN_PRIO_DRAW_BACKGROUND_HIGH_PRIO);

    g_BackgroundDrawChainLowPrio.SetCallback((ChainCallback)Background::OnDrawLowPrio);
    g_BackgroundDrawChainLowPrio.arg = background;
    g_Chain.AddToDrawChain(&g_BackgroundDrawChainLowPrio, CHAIN_PRIO_DRAW_BACKGROUND_LOW_PRIO);

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409c20
ZunResult Background::DeletedCallback(Background *background)
{
    if (!IsDisableResourceReload())
    {
        g_AnmManager->ReleaseAnm(4);
    }
    if (background->stageObjectVms != NULL)
    {
        g_ZunMemory.Free(background->stageObjectVms);
        background->stageObjectVms = NULL;
    }
    if (!IsDisableResourceReload() && background->stageData != NULL)
    {
        g_ZunMemory.Free(background->stageData);
        background->stageData = NULL;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409ca0
void Background::CutChain()
{
    g_Chain.Cut(&g_BackgroundCalcChain);
    g_Chain.Cut(&g_BackgroundDrawChainHighPrio);
    g_Chain.Cut(&g_BackgroundDrawChainLowPrio);
}

// FUNCTION: th08 0x409ce0
#pragma var_order(vmIdx, i, curObj, curQuad, this)
ZunResult Background::LoadStageData(const char *path)
{
    RawStageObject *curObj;
    RawStageQuadBasic *curQuad;
    i32 i;
    i32 vmIdx;

    if (!IsDisableResourceReload())
    {
        this->stageData = reinterpret_cast<RawStageHeader *>(FileSystem::OpenFile(path, NULL, 0));
        if (this->stageData == NULL)
        {
            g_GameErrorContext.Log("ステージデータが見つかりません。データが壊れています\r\n");
            return ZUN_ERROR;
        }
    }

    this->stageObjectCount = this->stageData->objectCount;
    this->stageQuadCount = this->stageData->quadCount;
    this->stageObjectInstances = reinterpret_cast<RawStageObjectInstance *>(
        this->stageData->objectInstancesOffset + (i32)this->stageData);
    this->stageScript = reinterpret_cast<RawStageInstr *>(
        this->stageData->scriptOffset + (i32)this->stageData);
    this->stageObjects = reinterpret_cast<RawStageObject **>(
        (u8 *)this->stageData + sizeof(RawStageHeader));

    if (!IsDisableResourceReload())
    {
        for (i = 0; i < this->stageObjectCount; i++)
        {
            this->stageObjects[i] =
                (RawStageObject *)((i32)this->stageObjects[i] + (i32)this->stageData);
        }
    }

    this->stageObjectVms = reinterpret_cast<AnmVm *>(
        g_ZunMemory.Alloc(this->stageQuadCount * sizeof(AnmVm), "bgscroll"));
    for (i = 0, vmIdx = 0; i < this->stageObjectCount; i++)
    {
        curObj = this->stageObjects[i];
        curObj->flags = 1;
        curQuad = &curObj->firstQuad;
        while (curQuad->type >= 0)
        {
            this->stageAnmFile->ExecuteAnmIdx(&this->stageObjectVms[vmIdx], curQuad->anmScript);
            curQuad->vmIdx = vmIdx++;
            curQuad = (RawStageQuadBasic *)((u8 *)curQuad + curQuad->byteSize);
        }
    }

    switch (g_GameManager.currentStage)
    {
    case 2:
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->stageTextVm, 33);
        break;
    default:
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->stageTextVm, 33);
        break;
    }
    this->stageTextVm.SetInterrupt(2);
    this->stageTextUsesYoukaiMode = 0;
    this->stageTextTimer = 0;
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409f40
#pragma var_order(unusedQuad, activeVms, i, vm, curObj, curQuad, this)
u32 Background::UpdateStageObjectVms()
{
    RawStageQuadBasic *curQuad;
    RawStageObject *curObj;
    AnmVm *vm;
    i32 i;
    i32 activeVms;
    RawStageQuadBasic *unusedQuad;

    if (this->stageTextUsesYoukaiMode != 0)
    {
        if (g_Player.IsHuman())
        {
            this->stageTextUsesYoukaiMode = 0;
            this->stageTextTimer = 0;
            this->stageTextVm.SetInterrupt(2);
        }
    }
    else if (g_Player.IsYoukai())
    {
        this->stageTextUsesYoukaiMode = 1;
        this->stageTextTimer = 0;
        this->stageTextVm.SetInterrupt(1);
    }

    this->stageTextTimer++;
    g_AnmManager->ExecuteScript(&this->stageTextVm);

    for (i = 0; i < this->stageObjectCount; i++)
    {
        curObj = this->stageObjects[i];
        if ((curObj->flags & 1) != 0)
        {
            activeVms = 0;
            curQuad = &curObj->firstQuad;
            while (curQuad->type >= 0)
            {
                vm = &this->stageObjectVms[curQuad->vmIdx];
                switch (curQuad->type)
                {
                case 0:
                    g_AnmManager->ExecuteScript(vm);
                    break;
                case 1:
                    unusedQuad = curQuad;
                    g_AnmManager->ExecuteScript(vm);
                    break;
                }

                if (vm->currentInstruction != NULL)
                {
                    activeVms++;
                }
                curQuad = (RawStageQuadBasic *)((u8 *)curQuad + curQuad->byteSize);
            }

            if (vm->type == 1)
            {
                vm->flagsWord |= 0x20000;
                vm->color2.r = ((u32)vm->color1.r * this->stageTextVm.color1.r) >> 8;
                vm->color2.g = ((u32)vm->color1.g * this->stageTextVm.color1.g) >> 8;
                vm->color2.b = ((u32)vm->color1.b * this->stageTextVm.color1.b) >> 8;
                vm->color2.a = ((u32)vm->color1.a * this->stageTextVm.color1.a) >> 8;
            }

            if (activeVms == 0)
            {
                curObj->flags &= ~1;
            }
        }
    }
    return 0;
}

// FUNCTION: th08 0x40a1b0
#pragma var_order(objQuadType1, curQuadVm, instancesDrawn, instance, fogState, worldMatrix, obj, objectDistance, cameraVec, quadPos, projectDest, curQuad, didDraw, radius, projectSrc, quadWidth, originalColor, this)
ZunResult Background::RenderObjects(i32 mode)
{
    RawStageQuadType1 *objQuadType1;
    AnmVm *curQuadVm;
    i32 instancesDrawn;
    RawStageObjectInstance *instance;
    i32 fogState;
    RawStageObject *obj;
    f32 objectDistance;
    RawStageQuadBasic *curQuad;
    i32 didDraw;
    f32 radius;
    f32 quadWidth;
    ZunColor originalColor;

    instance = this->stageObjectInstances;
    instancesDrawn = 0;
    didDraw = 0;

    Float3 quadPos;
    Float3 cameraVec;
    Float3 projectDest;
    Float3 projectSrc(0.0f, 0.0f, 0.0f);
    D3DXMATRIX worldMatrix;

    fogState = 255;

    this->SetCamera2();
    g_AnmManager->SetCameraMode(1);
    D3DXMatrixIdentity(&worldMatrix);
    cameraVec = *reinterpret_cast<Float3 *>(&g_Supervisor.viewMatrix);
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&cameraVec),
                      reinterpret_cast<D3DXVECTOR3 *>(&cameraVec));

    while (instance->id >= 0)
    {
        obj = this->stageObjects[instance->id];
        if (obj->zLevel == mode)
        {
            curQuad = &obj->firstQuad;

            quadPos.x = obj->position.x + instance->position.x - this->stagePosition.x + obj->size.x / 2.0f;
            quadPos.y = obj->position.y + instance->position.y - this->stagePosition.y + obj->size.y / 2.0f;
            quadPos.z = obj->position.z + instance->position.z - this->stagePosition.z + obj->size.z / 2.0f;
            quadPos = quadPos - (this->cameraCurrent.position + this->cameraCurrent.positionOffset);

            if (this->cullingDistanceSq <
                D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&quadPos)))
            {
                goto skip;
            }

            objectDistance = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&quadPos),
                                         reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.forward));
            radius = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&obj->size)) / 2.0f + 960.0f;
            if ((objectDistance > radius) || (objectDistance < 80.0f))
            {
                goto skip;
            }

            obj->flags |= 2;
            didDraw = 1;
            while (curQuad->type >= 0)
            {
                        curQuadVm = &this->stageObjectVms[curQuad->vmIdx];
                        switch (curQuad->type)
                        {
                        case 0:
                            curQuadVm->pos.x = curQuadVm->pos2.x + curQuad->position.x + instance->position.x -
                                                 this->stagePosition.x;
                            curQuadVm->pos.y = curQuadVm->pos2.y + curQuad->position.y + instance->position.y -
                                                 this->stagePosition.y;
                            curQuadVm->pos.z = curQuadVm->pos2.z + curQuad->position.z + instance->position.z -
                                                 this->stagePosition.z;
                            if (curQuad->size.x != 0.0f)
                            {
                                curQuadVm->scale.x = curQuad->size.x / curQuadVm->loadedSprite->widthPx;
                            }
                            if (curQuad->size.y != 0.0f)
                            {
                                curQuadVm->scale.y = curQuad->size.y / curQuadVm->loadedSprite->heightPx;
                            }

                            if ((curQuadVm->type & 0xF) == 2)
                            {
                                worldMatrix._41 = curQuadVm->pos[0];
                                worldMatrix._42 = curQuadVm->pos[1];
                                worldMatrix._43 = curQuadVm->pos[2];
                                D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&quadPos),
                                                reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                                &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);

                                if (curQuad->size.x != 0.0f)
                                {
                                    quadWidth = curQuad->size.x;
                                }
                                else
                                {
                                    quadWidth = curQuadVm->loadedSprite->widthPx;
                                }

                                worldMatrix._41 = cameraVec.x * quadWidth * curQuadVm->scale.x + worldMatrix._41;
                                worldMatrix._42 = cameraVec.y * quadWidth * curQuadVm->scale.x + worldMatrix._42;
                                worldMatrix._43 = cameraVec.z * quadWidth * curQuadVm->scale.x + worldMatrix._43;
                                D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectDest),
                                                reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                                &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                                projectDest = projectDest - quadPos;
                                curQuadVm->scale.x =
                                    D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest)) / quadWidth;
                                curQuadVm->scale.y = curQuadVm->scale.x;
                                if (quadWidth < 0.0f)
                                {
                                    curQuadVm->scale.y = -curQuadVm->scale.y;
                                }

                                projectDest = curQuadVm->pos - (this->cameraCurrent.position + this->cameraCurrent.positionOffset);
                                quadWidth = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest));
                                originalColor = curQuadVm->color1;
                                if (this->skyFog.nearPlane < quadWidth)
                                {
                                    quadWidth = (this->skyFog.nearPlane -
                                                 quadWidth) /
                                                (this->skyFog.nearPlane -
                                                 this->skyFog.farPlane);
                                    if (quadWidth >= 1.0f)
                                    {
                                        break;
                                    }
                                    curQuadVm->color1.b = curQuadVm->color1.b - static_cast<u8>(
                                        (curQuadVm->color1.b - this->skyFog.color.b) *
                                        quadWidth);
                                    curQuadVm->color1.g = curQuadVm->color1.g - static_cast<u8>(
                                        (curQuadVm->color1.g - this->skyFog.color.g) *
                                        quadWidth);
                                    curQuadVm->color1.r = curQuadVm->color1.r - static_cast<u8>(
                                        (curQuadVm->color1.r - this->skyFog.color.r) *
                                        quadWidth);
                                    curQuadVm->color1.a =
                                        static_cast<u8>(curQuadVm->color1.a * (1.0f - quadWidth));
                                }

                                curQuadVm->pos = quadPos;
                                if ((curQuadVm->pos.z < 0.0f) || (curQuadVm->pos.z > 1.0f))
                                {
                                    goto restore_color;
                                }

                                if (fogState != 0)
                                {
                                    if (!g_Supervisor.IsFogDisabled())
                                    {
                                        g_Supervisor.DisableFog();
                                    }
                                    fogState = 0;
                                }
                                g_AnmManager->DrawNoRotationNoRound(curQuadVm);
                                if ((curQuadVm->type & 0xF0) == 0x10 &&
                                    this->skyFog.nearPlane > quadWidth &&
                                    this->collectSpecialEffectPoints != 0)
                                {
                                    this->specialEffectPoints[*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) +
                                                                              0x6478)] = quadPos;
                                    this->specialEffectPoints[*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) +
                                                                              0x6478)].z = 0.0f;
                                    (this->specialEffectPointCount)++;
                                }

                            restore_color:
                                curQuadVm->color1 = originalColor;
                            }
                            else
                            {
                                if (!g_Supervisor.IsFogDisabled() && fogState != 1)
                                {
                                    if (!g_Supervisor.IsFogDisabled())
                                    {
                                        g_Supervisor.EnableFog();
                                    }
                                    fogState = 1;
                                }
                                g_AnmManager->Draw3D(curQuadVm);
                            }
                            break;

                        case 1:
                        {
                            objQuadType1 = reinterpret_cast<RawStageQuadType1 *>(curQuad);
#pragma var_order(type1World, halfWidthSecond, type1Width, vertices, projectedSecond, halfWidthFirst)
                            Float3 type1World;
                            Float3 projectedSecond;
                            BackgroundStageVertex vertices[4];
                            f32 halfWidthFirst;
                            f32 halfWidthSecond;
                            f32 type1Width;

                            type1World.x = objQuadType1->position1.x + instance->position.x - this->stagePosition.x;
                            type1World.y = objQuadType1->position1.y + instance->position.y - this->stagePosition.y;
                            type1World.z = objQuadType1->position1.z + instance->position.z - this->stagePosition.z;
                            worldMatrix._41 = type1World.x;
                            worldMatrix._42 = type1World.y;
                            worldMatrix._43 = type1World.z;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&quadPos),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);

                            if (objQuadType1->width != 0.0f)
                            {
                                type1Width = objQuadType1->width;
                            }
                            else
                            {
                                type1Width = curQuadVm->loadedSprite->widthPx;
                            }
                            worldMatrix._41 = cameraVec.x * type1Width + worldMatrix._41;
                            worldMatrix._42 = cameraVec.y * type1Width + worldMatrix._42;
                            worldMatrix._43 = cameraVec.z * type1Width + worldMatrix._43;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectDest),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                            projectDest = projectDest - quadPos;
                            halfWidthFirst =
                                D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest)) / 2.0f;

                            projectDest = type1World - (this->cameraCurrent.position + this->cameraCurrent.positionOffset);
                            type1Width = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest));
                            if (this->skyFog.nearPlane < type1Width)
                            {
                                type1Width = (this->skyFog.nearPlane -
                                              type1Width) /
                                             (this->skyFog.nearPlane -
                                              this->skyFog.farPlane);
                                if (type1Width < 1.0f)
                                {
                                    vertices[1].diffuse.b = curQuadVm->color1.b - static_cast<u8>(
                                        (curQuadVm->color1.b - this->skyFog.color.b) *
                                        type1Width);
                                    vertices[0].diffuse.b = vertices[1].diffuse.b;
                                    vertices[1].diffuse.g = curQuadVm->color1.g - static_cast<u8>(
                                        (curQuadVm->color1.g - this->skyFog.color.g) *
                                        type1Width);
                                    vertices[0].diffuse.g = vertices[1].diffuse.g;
                                    vertices[1].diffuse.r = curQuadVm->color1.r - static_cast<u8>(
                                        (curQuadVm->color1.r - this->skyFog.color.r) *
                                        type1Width);
                                    vertices[0].diffuse.r = vertices[1].diffuse.r;
                                    vertices[1].diffuse.a =
                                        static_cast<u8>(curQuadVm->color1.a * (1.0f - type1Width));
                                    vertices[0].diffuse.a = vertices[1].diffuse.a;
                                }
                                else
                                {
                                    vertices[1].diffuse.a = 0;
                                    vertices[0].diffuse.a = vertices[1].diffuse.a;
                                }
                            }
                            else
                            {
                                vertices[1].diffuse.d3dColor = curQuadVm->color1.d3dColor;
                                vertices[0].diffuse.d3dColor = vertices[1].diffuse.d3dColor;
                            }

                            type1World.x = objQuadType1->position2.x + instance->position.x - this->stagePosition.x;
                            type1World.y = objQuadType1->position2.y + instance->position.y - this->stagePosition.y;
                            type1World.z = objQuadType1->position2.z + instance->position.z - this->stagePosition.z;
                            worldMatrix._41 = type1World.x;
                            worldMatrix._42 = type1World.y;
                            worldMatrix._43 = type1World.z;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectedSecond),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);

                            if (objQuadType1->width != 0.0f)
                            {
                                type1Width = objQuadType1->width;
                            }
                            else
                            {
                                type1Width = curQuadVm->loadedSprite->widthPx;
                            }
                            worldMatrix._41 = cameraVec.x * type1Width + worldMatrix._41;
                            worldMatrix._42 = cameraVec.y * type1Width + worldMatrix._42;
                            worldMatrix._43 = cameraVec.z * type1Width + worldMatrix._43;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectDest),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                            projectDest = projectDest - projectedSecond;
                            halfWidthSecond =
                                D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest)) / 2.0f;

                            projectDest = type1World - (this->cameraCurrent.position + this->cameraCurrent.positionOffset);
                            type1Width = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest));
                            if (this->skyFog.nearPlane < type1Width)
                            {
                                type1Width = (this->skyFog.nearPlane -
                                              type1Width) /
                                             (this->skyFog.nearPlane -
                                              this->skyFog.farPlane);
                                if (type1Width < 1.0f)
                                {
                                    vertices[3].diffuse.b = curQuadVm->color1.b - static_cast<u8>(
                                        (curQuadVm->color1.b - this->skyFog.color.b) *
                                        type1Width);
                                    vertices[2].diffuse.b = vertices[3].diffuse.b;
                                    vertices[3].diffuse.g = curQuadVm->color1.g - static_cast<u8>(
                                        (curQuadVm->color1.g - this->skyFog.color.g) *
                                        type1Width);
                                    vertices[2].diffuse.g = vertices[3].diffuse.g;
                                    vertices[3].diffuse.r = curQuadVm->color1.r - static_cast<u8>(
                                        (curQuadVm->color1.r - this->skyFog.color.r) *
                                        type1Width);
                                    vertices[2].diffuse.r = vertices[3].diffuse.r;
                                    vertices[3].diffuse.a =
                                        static_cast<u8>(curQuadVm->color1.a * (1.0f - type1Width));
                                    vertices[2].diffuse.a = vertices[3].diffuse.a;
                                }
                                else
                                {
                                    vertices[3].diffuse.a = 0;
                                    vertices[2].diffuse.a = vertices[3].diffuse.a;
                                }
                            }
                            else
                            {
                                vertices[3].diffuse.d3dColor = curQuadVm->color1.d3dColor;
                                vertices[2].diffuse.d3dColor = vertices[3].diffuse.d3dColor;
                            }

                            projectSrc = projectedSecond - quadPos;
                            type1Width = sqrtf(projectSrc.x * projectSrc.x + projectSrc.y * projectSrc.y);
                            if (type1Width < 0.00001f)
                            {
                                goto advance_quad;
                            }
                            projectSrc /= type1Width;

                            if ((quadPos.z < 0.0f) || (quadPos.z > 1.0f))
                            {
                                goto advance_quad;
                            }
                            if ((projectedSecond.z < 0.0f) || (projectedSecond.z > 1.0f))
                            {
                                goto advance_quad;
                            }

                            vertices[0].pos.x = projectSrc.y * halfWidthFirst + quadPos.x;
                            vertices[0].pos.y = quadPos.y - projectSrc.x * halfWidthFirst;
                            vertices[0].pos.z = quadPos.z;
                            vertices[1].pos.x = quadPos.x - projectSrc.y * halfWidthFirst;
                            vertices[1].pos.y = projectSrc.x * halfWidthFirst + quadPos.y;
                            vertices[1].pos.z = quadPos.z;
                            vertices[2].pos.x = projectSrc.y * halfWidthSecond + projectedSecond.x;
                            vertices[2].pos.y = projectedSecond.y - projectSrc.x * halfWidthSecond;
                            vertices[2].pos.z = projectedSecond.z;
                            vertices[3].pos.x = projectedSecond.x - projectSrc.y * halfWidthSecond;
                            vertices[3].pos.y = projectSrc.x * halfWidthSecond + projectedSecond.y;
                            vertices[3].pos.z = projectedSecond.z;

                            vertices[2].textureUV.x = curQuadVm->loadedSprite->uvStart.x +
                                                      curQuadVm->uvScrollPos.x;
                            vertices[0].textureUV.x = vertices[2].textureUV.x;
                            vertices[3].textureUV.x = curQuadVm->loadedSprite->uvEnd.x +
                                                      curQuadVm->uvScrollPos.x;
                            vertices[1].textureUV.x = vertices[3].textureUV.x;
                            vertices[1].textureUV.y = curQuadVm->loadedSprite->uvStart.y +
                                                      curQuadVm->uvScrollPos.y;
                            vertices[0].textureUV.y = vertices[1].textureUV.y;
                            vertices[3].textureUV.y = curQuadVm->loadedSprite->uvEnd.y +
                                                      curQuadVm->uvScrollPos.y;
                            vertices[2].textureUV.y = vertices[3].textureUV.y;
                            vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;

                            if (fogState != 0)
                            {
                                if (!g_Supervisor.IsFogDisabled())
                                {
                                    g_Supervisor.DisableFog();
                                }
                                fogState = 0;
                            }
                            g_AnmManager->QueueSpriteQuad(curQuadVm, reinterpret_cast<VertexTex1DiffuseXyzrhw *>(vertices));
                            break;
                        }
                        }
                    advance_quad:
                        curQuad = reinterpret_cast<RawStageQuadBasic *>(reinterpret_cast<u8 *>(curQuad) +
                                                                       curQuad->byteSize);
            }
            instancesDrawn++;
        }
    skip:
        instance++;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x40b470
#pragma var_order(inverse, this)
Float3 *Float3::operator/=(f32 scalar)
{
    f32 inverse;

    inverse = 1.0f / scalar;
    this->x *= inverse;
    this->y *= inverse;
    this->z *= inverse;
    return this;
}

// FUNCTION: th08 0x40b5a0
#pragma var_order(cameraDistance, viewportMiddleHeight, viewportMiddleWidth, aspectRatio, fov, this)
void Background::SetCamera1()
{
    f32 fov;
    f32 aspectRatio;
    f32 viewportMiddleWidth;
    f32 viewportMiddleHeight;
    f32 cameraDistance;

    viewportMiddleWidth = (f32)g_Supervisor.viewport.Width / 2.0f;
    viewportMiddleHeight = (f32)g_Supervisor.viewport.Height / 2.0f;
    aspectRatio = (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height;
    fov = ZUN_PI / 10.0f;
    cameraDistance = viewportMiddleHeight / (f32)tan(fov / 2.0f);

    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix,
                       &D3DXVECTOR3(viewportMiddleWidth, viewportMiddleHeight, cameraDistance),
                       &D3DXVECTOR3(viewportMiddleWidth, viewportMiddleHeight, 0.0f),
                       &D3DXVECTOR3(0.0f, -1.0f, 0.0f));
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 1.0f, 10000.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
}

// FUNCTION: th08 0x40b6d0
#pragma var_order(eyeVec, atVec, this)
void Background::SetCamera2()
{
    Float3 atVec = this->cameraCurrent.lookAtOffset + this->cameraCurrent.position;
    Float3 eyeVec = this->cameraCurrent.positionOffset + this->cameraCurrent.position;
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix, reinterpret_cast<D3DXVECTOR3 *>(&eyeVec),
                       reinterpret_cast<D3DXVECTOR3 *>(&atVec),
                       reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.up));
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, this->cameraCurrent.fieldOfView,
                               (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height, 30.0f, 1800.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
    D3DXVec3Cross(reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.right),
                  reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.lookAtOffset),
                  reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.up));
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.right),
                      reinterpret_cast<D3DXVECTOR3 *>(&this->cameraCurrent.right));
}

// FUNCTION: th08 0x40b900
ZunBool IsDisableResourceReload()
{
    return g_Supervisor.keepStageResources;
}

}; // Namespace th08
