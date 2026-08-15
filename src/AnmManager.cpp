#include "th_pch.h"

#include "AnmManager.hpp"
#include "TextHelper.hpp"
#include "ZunMath.hpp"
#include "i18n.hpp"
#include "utils.hpp"
#include <stdarg.h>
#include <stdio.h>

namespace th08
{

// FUNCTION: th08 0x40b580
VertexDiffuseXyzrhw::VertexDiffuseXyzrhw()
{
}


#pragma optimize("s", on)
// FUNCTION: th08 0x004396F8
u32 AnmVm::FUN_004396f8()
{
    return (*(u32 *)((u8 *)this + 0x1F8) >> 14) & 1;
}
#pragma optimize("", on)


// FUNCTION: th08 0x4068e0
void AnmVmBase::Initialize()
{
    memset(this, 0, sizeof(AnmVmBase));

    this->scale.x = 1.0f;
    this->scale.y = 1.0f;
    this->color1.d3dColor = COLOR_WHITE;
    D3DXMatrixIdentity(&this->matrix1);
    this->flags = 7;
    this->currentTimeInScript.Initialize();
}

// FUNCTION: th08 0x40ec00
void AnmVm::SetZRotation(f32 z)
{
    this->rotation.z = z;
    this->flags |= 4;
}

// FUNCTION: th08 0x406700
void AnmVm::ClearVisible()
{
    this->visible = false;
}

DIFFABLE_STATIC(AnmManager *, g_AnmManager);
DIFFABLE_STATIC_ARRAY(VertexTex1DiffuseXyzrhw, 4, g_QuadVertices);
DIFFABLE_STATIC_ARRAY(VertexTex0Xyzrhw, 4, g_AnmManagerUntexturedQuadVertices);
DIFFABLE_STATIC_ARRAY(VertexTex0Xyzrhw, 4, g_BackgroundQuadVertices);

D3DFORMAT g_TextureFormatD3D8Mapping[] = {D3DFMT_UNKNOWN, D3DFMT_A8R8G8B8, D3DFMT_A1R5G5B5,
                                          D3DFMT_R5G6B5,  D3DFMT_R8G8B8,   D3DFMT_A4R4G4B4};

u32 g_TextureFormatBytesPerPixel[] = {4, 4, 2, 2, 3, 2};

ZunResult AnmLoaded::SetSprite(AnmVm *vm, int spriteIdx)
{
    if (this->rawData == NULL || this->numberEntriesToBeLoaded != 0)
    {
        return ZUN_ERROR;
    }

    vm->anmFile = this;
    vm->activeSpriteIndex = spriteIdx;
    vm->loadedSprite = &this->sprites[spriteIdx];
    vm->spriteSize.x = vm->loadedSprite->widthPx;
    vm->spriteSize.y = vm->loadedSprite->heightPx;

    D3DXMatrixIdentity(&vm->matrix1);
    D3DXMatrixIdentity(&vm->matrix3);

    /* ZUN bloat: what does this do? */
    if (vm->loadedSprite->scaleFactor.x < 1.0f)
    {
        spriteIdx = 0;
    }

    vm->matrix1.m[0][0] = vm->spriteSize.x / 256.0f;
    vm->matrix1.m[1][1] = vm->spriteSize.y / 256.0f;

    vm->matrix3.m[0][0] = (vm->spriteSize.x / vm->loadedSprite->width) * vm->loadedSprite->scaleFactor.x;
    vm->matrix3.m[1][1] = (vm->spriteSize.y / vm->loadedSprite->height) * vm->loadedSprite->scaleFactor.y;

    vm->matrix2 = vm->matrix1;

    return ZUN_SUCCESS;
}

void AnmLoaded::SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginningOfScript)
{
    if (beginningOfScript == NULL || (this->numberEntriesToBeLoaded != 0))
    {
        memset(vm, 0, sizeof(AnmVm));
    }
    else
    {
        vm->Initialize();
        vm->anmFileIndex = this->anmIdx;
        vm->anmFile = this;
        vm->flip = 0;
        vm->beginningOfScript = beginningOfScript;
        vm->currentInstruction = vm->beginningOfScript;
        vm->currentTimeInScript = 0;
        vm->visible = FALSE;
        g_AnmManager->ExecuteScript(vm);
        g_AnmManager->unk0xc++;
    }
}

f32 AnmVm::GetFloatVar(f32 varId)
{
    switch ((int)varId)
    {
    case AnmVariable_I0:
        return this->intVar0;
    case AnmVariable_I1:
        return this->intVar1;
    case AnmVariable_I2:
        return this->intVar2;
    case AnmVariable_I3:
        return this->intVar3;
    case AnmVariable_F0:
        return this->floatVar0;
    case AnmVariable_F1:
        return this->floatVar1;
    case AnmVariable_F2:
        return this->floatVar2;
    case AnmVariable_F3:
        return this->floatVar3;
    case AnmVariable_IC0:
        return this->counterVar0;
    case AnmVariable_IC1:
        return this->counterVar1;
    default:
        return varId;
    }
}

i32 AnmVm::GetIntVar(i32 varId)
{
    switch (varId)
    {
    case AnmVariable_I0:
        return this->intVar0;
    case AnmVariable_I1:
        return this->intVar1;
    case AnmVariable_I2:
        return this->intVar2;
    case AnmVariable_I3:
        return this->intVar3;
    case AnmVariable_F0:
        return this->floatVar0;
    case AnmVariable_F1:
        return this->floatVar1;
    case AnmVariable_F2:
        return this->floatVar2;
    case AnmVariable_F3:
        return this->floatVar3;
    case AnmVariable_IC0:
        return this->counterVar0;
    case AnmVariable_IC1:
        return this->counterVar1;
    default:
        return varId;
    }
}

f32 *AnmVm::GetFloatVarPtr(f32 *varPtr, u16 varMask, u32 variableNumber)
{
    if ((varMask & (1 << variableNumber)) == 0)
    {
        return varPtr;
    }

    switch ((int)*varPtr)
    {
    case AnmVariable_F0:
        return &this->floatVar0;
    case AnmVariable_F1:
        return &this->floatVar1;
    case AnmVariable_F2:
        return &this->floatVar2;
    case AnmVariable_F3:
        return &this->floatVar3;
    }

    return varPtr;
}

i32 *AnmVm::GetIntVarPtr(i32 *varPtr, u16 varMask, u32 variableNumber)
{
    if ((varMask & (1 << variableNumber)) == 0)
    {
        return varPtr;
    }

    switch (*varPtr)
    {
    case AnmVariable_I0:
        return &this->intVar0;
    case AnmVariable_I1:
        return &this->intVar1;
    case AnmVariable_I2:
        return &this->intVar2;
    case AnmVariable_I3:
        return &this->intVar3;
    case AnmVariable_IC0:
        return &this->counterVar0;
    case AnmVariable_IC1:
        return &this->counterVar1;
    }

    return varPtr;
}

#pragma var_order(instruction, nextInstruction, i, interp)
ZunBool AnmManager::ExecuteScript(AnmVm *vm)
{
    AnmRawInstr *instruction;
    AnmRawInstr *nextInstruction;
    int i;
    float interp;

    if (vm->currentInstruction == NULL)
    {
        return TRUE;
    }

    if (vm->flag19 != 0)
    {
        return FALSE;
    }

    if (vm->pendingInterrupt != 0)
    {
        goto handleInterrupt;
    }

    while (instruction = vm->currentInstruction, instruction->time <= (int)vm->currentTimeInScript)
    {
#define GET_INT_VAR(argNumber)                                                                                         \
    ((instruction->varMask & (1 << argNumber)) ? vm->GetIntVar(instruction->intArgs[argNumber])                        \
                                               : instruction->intArgs[argNumber])
#define GET_FLOAT_VAR(argNumber)                                                                                       \
    ((instruction->varMask & (1 << argNumber)) ? vm->GetFloatVar(instruction->floatArgs[argNumber])                    \
                                               : instruction->floatArgs[argNumber])

#define GET_INT_VAR_PTR(idx) vm->GetIntVarPtr(&instruction->intArgs[idx], instruction->varMask, idx)

#define GET_FLOAT_VAR_PTR(idx) vm->GetFloatVarPtr(&instruction->floatArgs[idx], instruction->varMask, idx)

        switch (instruction->opcode)
        {
        case AnmOpcode_EndOfScript:
        case AnmOpcode_Delete:
            vm->visible = false;
        case AnmOpcode_Static:
            vm->currentInstruction = NULL;
            return TRUE;
        case AnmOpcode_Sprite:
            vm->visible = true;

            vm->anmFile->SetSprite(vm, GET_INT_VAR(0));
            vm->timeOfLastSpriteSet = (int)vm->currentTimeInScript;
            break;
        case AnmOpcode_Scale:
            vm->scale.x = GET_FLOAT_VAR(0);
            vm->scale.y = GET_FLOAT_VAR(1);

            vm->updateScale = true;
            break;
        case AnmOpcode_Alpha:
            vm->color1.a = GET_INT_VAR(0);
            break;
        case AnmOpcode_Color:
            vm->color1.r = GET_INT_VAR(0);
            vm->color1.g = GET_INT_VAR(1);
            vm->color1.b = GET_INT_VAR(2);
            break;
        case AnmOpcode_Alpha2:
            vm->color2.a = GET_INT_VAR(0);
            break;
        case AnmOpcode_Color2:
            vm->color2.r = GET_INT_VAR(0);
            vm->color2.g = GET_INT_VAR(1);
            vm->color2.b = GET_INT_VAR(2);
            break;
        case AnmOpcode_Jmp:
            vm->currentTimeInScript = instruction->intArgs[1];
            vm->currentInstruction = (AnmRawInstr *)(((u8 *)vm->beginningOfScript) + instruction->intArgs[0]);
            continue;
        case AnmOpcode_JmpDec:
            *GET_INT_VAR_PTR(0) -= 1;

            if (GET_INT_VAR(0) > 0)
            {
                vm->currentTimeInScript = instruction->intArgs[2];
                vm->currentInstruction = (AnmRawInstr *)(((u8 *)vm->beginningOfScript) + instruction->intArgs[1]);
                continue;
            }
            break;
        case AnmOpcode_FlipX:
            vm->flip ^= (1 << 0);
            vm->scale.x *= -1.0f;
            vm->updateScale = true;
            break;
        case AnmOpcode_PosMode:
            vm->usePosOffset = instruction->intArgs[0];
            break;
        case AnmOpcode_FlipY:
            vm->flip ^= (1 << 1);
            vm->scale.y *= -1.0f;
            vm->updateScale = true;
            break;
        case AnmOpcode_Rotate:
            vm->rotation.x = GET_FLOAT_VAR(0);
            vm->rotation.y = GET_FLOAT_VAR(1);
            vm->rotation.z = GET_FLOAT_VAR(2);

            vm->updateRotation = true;
            break;
        case AnmOpcode_AngularVelocity:
            vm->angleVel.x = GET_FLOAT_VAR(0);
            vm->angleVel.y = GET_FLOAT_VAR(1);
            vm->angleVel.z = GET_FLOAT_VAR(2);

            vm->updateRotation = true;
            break;
        case AnmOpcode_ScaleGrowth:
            vm->scaleGrowth.x = GET_FLOAT_VAR(0);
            vm->scaleGrowth.y = GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_ScaleTimeLinear:
            vm->interpCurrentTimers[AnmInterp_Scale] = 0;

            vm->interpEndTimers[AnmInterp_Scale] = GET_INT_VAR(2);

            vm->interpModes[AnmInterp_Scale] = AnmInterpMode_Linear;
            vm->scaleInitial = vm->scale;

            vm->scaleFinal.x = GET_FLOAT_VAR(0);
            vm->scaleFinal.y = GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_AlphaTimeLinear:
            vm->color1Initial.a = vm->color1.a;
            vm->color1Final.a = instruction->intArgs[0];

            vm->interpCurrentTimers[AnmInterp_Alpha1] = 0;
            vm->interpEndTimers[AnmInterp_Alpha1] = GET_INT_VAR(1);
            vm->interpModes[AnmInterp_Alpha1] = AnmInterpMode_Linear;
            break;
        case AnmOpcode_AdditiveBlendMode:
            vm->blendMode = instruction->intArgs[0] != 0;
            break;
        case AnmOpcode_BlendMode:
            vm->blendMode = instruction->intArgs[0];
            break;
        case AnmOpcode_Pos:
            if (!vm->usePosOffset)
            {
                vm->pos = Float3(GET_FLOAT_VAR(0), GET_FLOAT_VAR(1), GET_FLOAT_VAR(2));
            }
            else
            {
                vm->pos2 = Float3(GET_FLOAT_VAR(0), GET_FLOAT_VAR(1), GET_FLOAT_VAR(2));
            }
            break;
        case AnmOpcode_PosTimeDecel2:
            vm->interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOutQuartic;
            goto posTime;
        case AnmOpcode_PosTimeDecel:
            vm->interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOut;
            goto posTime;
        case AnmOpcode_PosTimeLinear:
            vm->interpModes[AnmInterp_Pos] = AnmInterpMode_Linear;
        posTime:
            if (!vm->usePosOffset)
            {
                vm->posInitial = vm->pos;
            }
            else
            {
                vm->posInitial = vm->pos2;
            }

            vm->posFinal = Float3(GET_FLOAT_VAR(0), GET_FLOAT_VAR(1), GET_FLOAT_VAR(2));

            vm->interpEndTimers[AnmInterp_Pos] = GET_INT_VAR(3);
            vm->interpCurrentTimers[AnmInterp_Pos] = 0;
            break;
        case AnmOpcode_Wait:
            if (vm->waitTimer == 0)
            {
                vm->waitTimer = GET_INT_VAR(0);
            }
            else
            {
                vm->waitTimer--;
            }

            if (vm->waitTimer <= 0)
            {
                vm->waitTimer = 0;
                break;
            }
            vm->currentTimeInScript--;
            goto stop;
        case AnmOpcode_StopHide:
            vm->visible = false;
        case AnmOpcode_Stop:
            if (vm->pendingInterrupt == 0)
            {
                vm->stopped = true;
                vm->currentTimeInScript--;
                goto stop;
            }

        handleInterrupt:
            nextInstruction = NULL;
            instruction = vm->beginningOfScript;
            while (
                !(instruction->opcode == AnmOpcode_InterruptLabel && vm->pendingInterrupt == instruction->intArgs[0]) &&
                instruction->opcode != AnmOpcode_EndOfScript)
            {
                if (instruction->opcode == AnmOpcode_InterruptLabel && instruction->intArgs[0] == -1)
                {
                    nextInstruction = instruction;
                }
                instruction = (AnmRawInstr *)((u8 *)instruction + instruction->instructionSize);
            }

            vm->pendingInterrupt = 0;
            vm->stopped = false;

            if (instruction->opcode != AnmOpcode_InterruptLabel)
            {
                if (nextInstruction == NULL)
                {
                    vm->currentTimeInScript--;
                    goto stop;
                }
                instruction = nextInstruction;
            }

            vm->interruptReturnTime = vm->currentTimeInScript;
            vm->interruptReturnInstruction = vm->currentInstruction;
            instruction = (AnmRawInstr *)((u8 *)instruction + instruction->instructionSize);
            vm->currentInstruction = instruction;
            vm->currentTimeInScript = vm->currentInstruction->time;
            vm->visible = true;
            continue;
        case AnmOpcode_ReturnFromInterrupt:
            vm->currentTimeInScript = vm->interruptReturnTime;
            vm->currentInstruction = vm->interruptReturnInstruction;
            continue;
        case AnmOpcode_Visible:
            vm->visible = instruction->intArgs[0];
            break;
        case AnmOpcode_AnchorTopLeft:
            vm->anchor = 3;
            break;
        case AnmOpcode_Ins25:
            vm->type = instruction->intArgs[0];
            break;
        case AnmOpcode_AddU:
            vm->uvScrollPos.x += GET_FLOAT_VAR(0);
            ;
            if (vm->uvScrollPos.x >= 1.0f)
            {
                vm->uvScrollPos.x -= 1.0f;
            }
            else
            {
                if (vm->uvScrollPos.x < 0.0f)
                {
                    vm->uvScrollPos.x += 1.0f;
                }
            }
            break;
        case AnmOpcode_AddV:
            vm->uvScrollPos.y += GET_FLOAT_VAR(0);
            if (vm->uvScrollPos.y >= 1.0f)
            {
                vm->uvScrollPos.y -= 1.0f;
            }
            else
            {
                if (vm->uvScrollPos.y < 0.0f)
                {
                    vm->uvScrollPos.y += 1.0f;
                }
            }
            break;
        case AnmOpcode_UScroll:
            vm->uvScrollVel.x = GET_FLOAT_VAR(0);
            break;
        case AnmOpcode_VScroll:
            vm->uvScrollVel.y = GET_FLOAT_VAR(0);
            break;
        case AnmOpcode_ZWriteDisable:
            vm->zWriteDisabled = instruction->intArgs[0];
            break;
        case AnmOpcode_Ins31:
            vm->flag15 = instruction->intArgs[0];
            break;
        case AnmOpcode_PosTime:
            vm->interpCurrentTimers[AnmInterp_Pos] = 0;
            vm->interpEndTimers[AnmInterp_Pos] = GET_INT_VAR(0);
            vm->interpModes[AnmInterp_Pos] = instruction->intArgs[1];

            if (!vm->usePosOffset)
            {
                vm->posInitial = vm->pos;
            }
            else
            {
                vm->posInitial = vm->pos2;
            }

            vm->posFinal.x = GET_FLOAT_VAR(2);
            vm->posFinal.y = GET_FLOAT_VAR(3);
            vm->posFinal.z = GET_FLOAT_VAR(4);
            break;
        case AnmOpcode_ColorTime:
            vm->interpCurrentTimers[AnmInterp_RGB1] = 0;

            vm->interpEndTimers[AnmInterp_RGB1] = GET_INT_VAR(0);

            vm->interpModes[AnmInterp_RGB1] = instruction->intArgs[1];
            vm->color1Initial.r = vm->color1.r;
            vm->color1Initial.g = vm->color1.g;
            vm->color1Initial.b = vm->color1.b;

            vm->color1Final.r = GET_INT_VAR(2);
            vm->color1Final.g = GET_INT_VAR(3);
            vm->color1Final.b = GET_INT_VAR(4);
            break;
        case AnmOpcode_AlphaTime:
            vm->interpCurrentTimers[AnmInterp_Alpha1] = 0;
            vm->interpEndTimers[AnmInterp_Alpha1] = GET_INT_VAR(0);
            vm->interpModes[AnmInterp_Alpha1] = instruction->intArgs[1];

            vm->color1Initial.a = vm->color1.a;
            vm->color1Final.a = GET_INT_VAR(2);
            break;
        case AnmOpcode_Color2Time:
            vm->interpCurrentTimers[AnmInterp_RGB2] = 0;

            vm->interpEndTimers[AnmInterp_RGB2] = GET_INT_VAR(0);

            vm->interpModes[AnmInterp_RGB2] = instruction->intArgs[1];
            vm->color2Initial.r = vm->color2.r;
            vm->color2Initial.g = vm->color2.g;
            vm->color2Initial.b = vm->color2.b;

            vm->color2Final.r = GET_INT_VAR(2);
            vm->color2Final.g = GET_INT_VAR(3);
            vm->color2Final.b = GET_INT_VAR(4);
            break;
        case AnmOpcode_Alpha2Time:
            vm->interpCurrentTimers[AnmInterp_Alpha2] = 0;
            vm->interpEndTimers[AnmInterp_Alpha2] = GET_INT_VAR(0);
            vm->interpModes[AnmInterp_Alpha2] = instruction->intArgs[1];

            vm->color2Initial.a = vm->color2.a;
            vm->color2Final.a = GET_INT_VAR(2);
            break;
        case AnmOpcode_RotateTime:
            vm->interpCurrentTimers[AnmInterp_Rotate] = 0;

            vm->interpEndTimers[AnmInterp_Rotate] = GET_INT_VAR(0);

            vm->interpModes[AnmInterp_Rotate] = instruction->intArgs[1];
            vm->rotateInitial = vm->rotation;

            vm->rotateFinal.x = GET_FLOAT_VAR(2);
            vm->rotateFinal.y = GET_FLOAT_VAR(3);
            vm->rotateFinal.z = GET_FLOAT_VAR(4);

            vm->updateRotation = true;
            break;
        case AnmOpcode_ScaleTime:
            vm->interpCurrentTimers[AnmInterp_Scale] = 0;
            vm->interpEndTimers[AnmInterp_Scale] = GET_INT_VAR(0);

            vm->interpModes[AnmInterp_Scale] = instruction->intArgs[1];
            vm->scaleInitial = vm->scale;

            vm->scaleFinal.x = GET_FLOAT_VAR(2);
            vm->scaleFinal.y = GET_FLOAT_VAR(3);
            vm->updateScale = true;
            break;
        case AnmOpcode_Ins83:
            vm->playerBulletHitAnimationType = instruction->intArgs[0];
            break;
        case AnmOpcode_ISet:
            *GET_INT_VAR_PTR(0) = GET_INT_VAR(1);
            break;
        case AnmOpcode_FSet:
            *GET_FLOAT_VAR_PTR(0) = GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_ISetAdd:
            *GET_INT_VAR_PTR(0) = GET_INT_VAR(1) + GET_INT_VAR(2);
            break;
        case AnmOpcode_FSetAdd:
            *GET_FLOAT_VAR_PTR(0) = GET_FLOAT_VAR(1) + GET_FLOAT_VAR(2);
            break;
        case AnmOpcode_ISetSub:
            *GET_INT_VAR_PTR(0) = GET_INT_VAR(1) - GET_INT_VAR(2);
            break;
        case AnmOpcode_FSetSub:
            *GET_FLOAT_VAR_PTR(0) = GET_FLOAT_VAR(1) - GET_FLOAT_VAR(2);
            break;
        case AnmOpcode_ISetMul:
            *GET_INT_VAR_PTR(0) = GET_INT_VAR(1) * GET_INT_VAR(2);
            break;
        case AnmOpcode_FSetMul:
            *GET_FLOAT_VAR_PTR(0) = GET_FLOAT_VAR(1) * GET_FLOAT_VAR(2);
            break;
        case AnmOpcode_ISetDiv:
            *GET_INT_VAR_PTR(0) = GET_INT_VAR(1) / GET_INT_VAR(2);
            break;
        case AnmOpcode_FSetDiv:
            *GET_FLOAT_VAR_PTR(0) = GET_FLOAT_VAR(1) / GET_FLOAT_VAR(2);
            break;
        case AnmOpcode_ISetMod:
            *GET_INT_VAR_PTR(0) = GET_INT_VAR(1) % GET_INT_VAR(2);
            break;
        case AnmOpcode_FSetMod:
            *GET_FLOAT_VAR_PTR(0) = fmodf(GET_FLOAT_VAR(1), GET_FLOAT_VAR(2));
            break;
        case AnmOpcode_IAdd:
            *GET_INT_VAR_PTR(0) += GET_INT_VAR(1);
            break;
        case AnmOpcode_FAdd:
            *GET_FLOAT_VAR_PTR(0) += GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_ISub:
            *GET_INT_VAR_PTR(0) -= GET_INT_VAR(1);
            break;
        case AnmOpcode_FSub:
            *GET_FLOAT_VAR_PTR(0) -= GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_IMul:
            *GET_INT_VAR_PTR(0) *= GET_INT_VAR(1);
            break;
        case AnmOpcode_FMul:
            *GET_FLOAT_VAR_PTR(0) *= GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_IDiv:
            *GET_INT_VAR_PTR(0) /= GET_INT_VAR(1);
            break;
        case AnmOpcode_FDiv:
            *GET_FLOAT_VAR_PTR(0) /= GET_FLOAT_VAR(1);
            break;
        case AnmOpcode_IMod:
            *GET_INT_VAR_PTR(0) %= GET_INT_VAR(1);
            break;
        case AnmOpcode_FMod:
            *GET_FLOAT_VAR_PTR(0) = fmodf(GET_FLOAT_VAR(0), GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_ISetRand:
            *GET_INT_VAR_PTR(0) = g_Rng.GetRandomU32InRange(GET_INT_VAR(1));
            break;
        case AnmOpcode_FSetRand:
            *GET_FLOAT_VAR_PTR(0) = g_Rng.GetRandomF32InRange(GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_FSin:
            *GET_FLOAT_VAR_PTR(0) = sinf(GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_FCos:
            *GET_FLOAT_VAR_PTR(0) = cosf(GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_FTan:
            *GET_FLOAT_VAR_PTR(0) = tanf(GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_FAcos:
            *GET_FLOAT_VAR_PTR(0) = acosf(GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_FAtan:
            *GET_FLOAT_VAR_PTR(0) = atanf(GET_FLOAT_VAR(1));
            break;
        case AnmOpcode_NormalizeAngle:
            *GET_FLOAT_VAR_PTR(0) = AddNormalizeAngle(GET_FLOAT_VAR(0), 0);
            break;
        case AnmOpcode_IJmpEq:
            if (GET_INT_VAR(0) == GET_INT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_FJmpEq:
            if (GET_FLOAT_VAR(0) == GET_FLOAT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_IJmpNeq:
            if (GET_INT_VAR(0) != GET_INT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_FJmpNeq:
            if (GET_FLOAT_VAR(0) != GET_FLOAT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_IJmpLess:
            if (GET_INT_VAR(0) < GET_INT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_FJmpLess:
            if (GET_FLOAT_VAR(0) < GET_FLOAT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_IJmpLessOrEq:
            if (GET_INT_VAR(0) <= GET_INT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_FJmpLessOrEq:
            if (GET_FLOAT_VAR(0) <= GET_FLOAT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_IJmpGreater:
            if (GET_INT_VAR(0) > GET_INT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_FJmpGreater:
            if (GET_FLOAT_VAR(0) > GET_FLOAT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_IJmpGreaterOrEq:
            if (GET_INT_VAR(0) >= GET_INT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_FJmpGreaterOrEq:
            if (GET_FLOAT_VAR(0) >= GET_FLOAT_VAR(1))
            {
                goto jump;
            }
            break;
        case AnmOpcode_Ins88:
            vm->flag17 = instruction->byteArgs[1];
            break;
        jump:
            vm->currentTimeInScript = instruction->intArgs[3];
            vm->currentInstruction = (AnmRawInstr *)(((u8 *)vm->beginningOfScript) + instruction->intArgs[2]);
            continue;
        default:
            break;
        }
#undef GET_FLOAT_VAR_PTR
#undef GET_INT_VAR_PTR
#undef GET_FLOAT_VAR
#undef GET_INT_VAR

        vm->currentInstruction = (AnmRawInstr *)((u8 *)instruction + instruction->instructionSize);
    }
stop:
    if (vm->angleVel.x != 0.0f)
    {
        vm->rotation.x = AddNormalizeAngle(vm->rotation.x, g_Supervisor.framerateMultiplier * vm->angleVel.x);
        vm->updateRotation = true;
    }

    if (vm->angleVel.y != 0.0f)
    {
        vm->rotation.y = AddNormalizeAngle(vm->rotation.y, g_Supervisor.framerateMultiplier * vm->angleVel.y);
        vm->updateRotation = true;
    }

    if (vm->angleVel.z != 0.0f)
    {
        vm->rotation.z = AddNormalizeAngle(vm->rotation.z, g_Supervisor.framerateMultiplier * vm->angleVel.z);
        vm->updateRotation = true;
    }

    for (i = 0; i < AnmInterp_Last; i++)
    {
        if (vm->interpEndTimers[i] > 0)
        {
            vm->interpCurrentTimers[i]++;
            if (vm->interpCurrentTimers[i] >= (int)vm->interpEndTimers[i])
            {
                interp = 1.0f;
                vm->interpEndTimers[i] = 0;
            }
            else
            {
                interp = (float)vm->interpCurrentTimers[i] / (float)vm->interpEndTimers[i];
            }

            switch (vm->interpModes[i])
            {
            case AnmInterpMode_EaseIn:
                interp = interp * interp;
                break;
            case AnmInterpMode_EaseInCubic:
                interp = interp * interp * interp;
                break;
            case AnmInterpMode_EaseInQuartic:
                interp = interp * interp;
                interp = interp * interp;
                break;
            case AnmInterpMode_EaseOut:
                interp = (1.0f - interp);
                interp *= interp;
                interp = (1.0f - interp);
                break;
            case AnmInterpMode_EaseOutCubic:
                interp = (1.0f - interp);
                interp = interp * interp * interp;
                interp = (1.0f - interp);
                break;
            case AnmInterpMode_EaseOutQuartic:
                interp = (1.0f - interp);
                interp = interp * interp;
                interp = interp * interp;
                interp = (1.0f - interp);
                break;
            }

            switch (i)
            {
            case AnmInterp_Pos:
                if (!vm->usePosOffset)
                {
                    vm->pos.x = interp * (vm->posFinal.x - vm->posInitial.x) + vm->posInitial.x;
                    vm->pos.y = interp * (vm->posFinal.y - vm->posInitial.y) + vm->posInitial.y;
                    vm->pos.z = interp * (vm->posFinal.z - vm->posInitial.z) + vm->posInitial.z;
                }
                else
                {
                    vm->pos2.x = interp * (vm->posFinal.x - vm->posInitial.x) + vm->posInitial.x;
                    vm->pos2.y = interp * (vm->posFinal.y - vm->posInitial.y) + vm->posInitial.y;
                    vm->pos2.z = interp * (vm->posFinal.z - vm->posInitial.z) + vm->posInitial.z;
                }
                break;
            case AnmInterp_RGB1:
                vm->color1.r = interp * ((float)vm->color1Final.r - vm->color1Initial.r) + vm->color1Initial.r;
                vm->color1.g = interp * ((float)vm->color1Final.g - vm->color1Initial.g) + vm->color1Initial.g;
                vm->color1.b = interp * ((float)vm->color1Final.b - vm->color1Initial.b) + vm->color1Initial.b;
                break;
            case AnmInterp_Alpha1:
                vm->color1.a = interp * ((float)vm->color1Final.a - vm->color1Initial.a) + vm->color1Initial.a;
                break;
            case AnmInterp_RGB2:
                vm->color2.r = interp * ((float)vm->color2Final.r - vm->color2Initial.r) + vm->color2Initial.r;
                vm->color2.g = interp * ((float)vm->color2Final.g - vm->color2Initial.g) + vm->color2Initial.g;
                vm->color2.b = interp * ((float)vm->color2Final.b - vm->color2Initial.b) + vm->color2Initial.b;
                break;
            case AnmInterp_Alpha2:
                vm->color2.a = interp * ((float)vm->color2Final.a - vm->color2Initial.a) + vm->color2Initial.a;
                break;
            case AnmInterp_Rotate:
                vm->rotation.x =
                    AddNormalizeAngle((vm->rotateFinal.x - vm->rotateInitial.x) * interp, vm->rotateInitial.x);
                vm->rotation.y =
                    AddNormalizeAngle((vm->rotateFinal.y - vm->rotateInitial.y) * interp, vm->rotateInitial.y);
                vm->rotation.z =
                    AddNormalizeAngle((vm->rotateFinal.z - vm->rotateInitial.z) * interp, vm->rotateInitial.z);
                vm->updateRotation = true;
                break;
            case AnmInterp_Scale:
                vm->scale.x = interp * (vm->scaleFinal.x - vm->scaleInitial.x) + vm->scaleInitial.x;
                vm->scale.y = interp * (vm->scaleFinal.y - vm->scaleInitial.y) + vm->scaleInitial.y;
                vm->updateScale = true;
                break;
            }
        }
    }

    if (vm->scaleGrowth.y != 0.0f)
    {
        vm->scale.y += g_Supervisor.framerateMultiplier * vm->scaleGrowth.y;
        vm->updateScale = true;
    }

    if (vm->scaleGrowth.x != 0.0f)
    {
        vm->scale.x += g_Supervisor.framerateMultiplier * vm->scaleGrowth.x;
        vm->updateScale = true;
        vm->updateRotation = true;
    }

    vm->uvScrollPos.x += vm->uvScrollVel.x;

    if (vm->uvScrollPos.x >= 1.0f)
    {
        vm->uvScrollPos.x -= 1.0f;
    }
    else
    {
        if (vm->uvScrollPos.x < 0.0f)
        {
            vm->uvScrollPos.x += 1.0f;
        }
    }

    vm->uvScrollPos.y += vm->uvScrollVel.y;
    if (vm->uvScrollPos.y >= 1.0f)
    {
        vm->uvScrollPos.y -= 1.0f;
    }
    else
    {
        if (vm->uvScrollPos.y < 0.0f)
        {
            vm->uvScrollPos.y += 1.0f;
        }
    }

    vm->currentTimeInScript++;
    this->scriptsExecutedThisFrame++;

    return FALSE;
}

void AnmManager::SetInterruptArray(AnmVm *vm, int count, i16 interrupt)
{
    while (count != 0)
    {
        if (g_AnmManager->SpriteHasTexture(vm))
        {
            vm->SetInterrupt(interrupt);
        }
        vm++;
        count--;
    }
}

void AnmManager::ExecuteScriptArray(AnmVm *sprite, int count)
{
    while (count != 0)
    {
        if (sprite->scriptIndex >= 0)
        {
            g_AnmManager->ExecuteScript(sprite);
        }
        sprite++;
        count--;
    }
}

void AnmLoaded::ExecuteAnmIdxArray(AnmVm *vm, i32 scriptIdx, i32 count)
{
    while (count != 0)
    {
        this->ExecuteAnmIdx(vm, scriptIdx);
        vm->baseSpriteIndex = vm->activeSpriteIndex;

        scriptIdx++;
        vm++;
        count--;
    }
}

u8 MixColors(u8 color1, u8 color2)
{
    u32 color = ((color1 * color2) / 128U);

    if (color >= 256)
    {
        color = 255;
    }

    return color;
}

void AnmManager::SetRenderStateForVm(AnmVm *vm)
{
    if (this->currentBlendMode != vm->blendMode)
    {
        this->FlushVertexBuffer();
        this->currentBlendMode = vm->blendMode;

        switch (this->currentBlendMode)
        {
        case AnmBlendMode_Normal:
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            break;
        case AnmBlendMode_Additive:
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        }
    }

    if (!g_Supervisor.IsDepthTestDisabled() && this->disableZWrite != vm->zWriteDisabled)
    {
        this->disableZWrite = vm->zWriteDisabled;
        if (!this->disableZWrite)
        {
            g_Supervisor.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_Supervisor.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }

    this->renderStateChangesThisFrame++;
}

static const f32 g_ZeroPointFive = 0.5;

#pragma var_order(triangleY1, triangleY2, triangleX2, triangleX1, color)
ZunResult AnmManager::DrawInner(AnmVm *vm, i32 flags)
{
    ZunColor color;
    float triangleX1, triangleX2, triangleY1, triangleY2;

    g_QuadVertices[0].pos.x += this->screenShakeOffset.x;
    g_QuadVertices[0].pos.y += this->screenShakeOffset.y;
    g_QuadVertices[1].pos.x += this->screenShakeOffset.x;
    g_QuadVertices[1].pos.y += this->screenShakeOffset.y;
    g_QuadVertices[2].pos.x += this->screenShakeOffset.x;
    g_QuadVertices[2].pos.y += this->screenShakeOffset.y;
    g_QuadVertices[3].pos.x += this->screenShakeOffset.x;
    g_QuadVertices[3].pos.y += this->screenShakeOffset.y;

    if (flags & 1)
    {
        /* same as in EoSD. */
        __asm
        {
            fld g_QuadVertices[0 * TYPE g_QuadVertices].pos.x
            frndint
            fsub g_ZeroPointFive
            fld g_QuadVertices[1 * TYPE g_QuadVertices].pos.x
            frndint
            fsub g_ZeroPointFive
            fld g_QuadVertices[0 * TYPE g_QuadVertices].pos.y
            frndint
            fsub g_ZeroPointFive
            fld g_QuadVertices[2 * TYPE g_QuadVertices].pos.y
            frndint
            fsub g_ZeroPointFive
            fst g_QuadVertices[2 * TYPE g_QuadVertices].pos.y
            fstp g_QuadVertices[3 * TYPE g_QuadVertices].pos.y
            fst g_QuadVertices[0 * TYPE g_QuadVertices].pos.y
            fstp g_QuadVertices[1 * TYPE g_QuadVertices].pos.y
            fst g_QuadVertices[1 * TYPE g_QuadVertices].pos.x
            fstp g_QuadVertices[3 * TYPE g_QuadVertices].pos.x
            fst g_QuadVertices[0 * TYPE g_QuadVertices].pos.x
            fstp g_QuadVertices[2 * TYPE g_QuadVertices].pos.x
        }
    }

    g_QuadVertices[0].textureUV.x = g_QuadVertices[2].textureUV.x = vm->loadedSprite->uvStart.x + vm->uvScrollPos.x;
    g_QuadVertices[1].textureUV.x = g_QuadVertices[3].textureUV.x = vm->loadedSprite->uvEnd.x + vm->uvScrollPos.x;
    g_QuadVertices[0].textureUV.y = g_QuadVertices[1].textureUV.y = vm->loadedSprite->uvStart.y + vm->uvScrollPos.y;
    g_QuadVertices[2].textureUV.y = g_QuadVertices[3].textureUV.y = vm->loadedSprite->uvEnd.y + vm->uvScrollPos.y;

    triangleX1 = max(g_QuadVertices[0].pos.x, g_QuadVertices[1].pos.x);
    triangleX1 = max(g_QuadVertices[2].pos.x, triangleX1);
    triangleX1 = max(g_QuadVertices[3].pos.x, triangleX1);

    triangleY1 = max(g_QuadVertices[0].pos.y, g_QuadVertices[1].pos.y);
    triangleY1 = max(g_QuadVertices[2].pos.y, triangleY1);
    triangleY1 = max(g_QuadVertices[3].pos.y, triangleY1);

    triangleX2 = min(g_QuadVertices[0].pos.x, g_QuadVertices[1].pos.x);
    triangleX2 = min(g_QuadVertices[2].pos.x, triangleX2);
    triangleX2 = min(g_QuadVertices[3].pos.x, triangleX2);

    triangleY2 = min(g_QuadVertices[0].pos.y, g_QuadVertices[1].pos.y);
    triangleY2 = min(g_QuadVertices[2].pos.y, triangleY2);
    triangleY2 = min(g_QuadVertices[3].pos.y, triangleY2);

    if (triangleX1 < g_Supervisor.viewport.X || triangleY1 < g_Supervisor.viewport.Y ||
        triangleX2 > (g_Supervisor.viewport.X + g_Supervisor.viewport.Width) ||
        triangleY2 > (g_Supervisor.viewport.Y + g_Supervisor.viewport.Height))
    {
        return ZUN_SUCCESS;
    }

    if (this->currentTexture != vm->loadedSprite->texture)
    {
        this->currentTexture = vm->loadedSprite->texture;
        this->FlushVertexBuffer();
        g_Supervisor.d3dDevice->SetTexture(0, this->currentTexture);
    }

    if (this->currentVertexShader != 1)
    {
        this->FlushVertexBuffer();
        this->currentVertexShader = 1;
    }

    if ((flags & 2) == 0)
    {
        color.d3dColor = vm->flag17 ? vm->color2.d3dColor : vm->color1.d3dColor;

        if (this->useMixColor)
        {
            color.r = MixColors(color.r, this->color.r);
            color.g = MixColors(color.g, this->color.g);
            color.b = MixColors(color.b, this->color.b);
            color.a = MixColors(color.a, this->color.a);
        }

        g_QuadVertices[0].diffuse = color.d3dColor;
        g_QuadVertices[1].diffuse = color.d3dColor;
        g_QuadVertices[2].diffuse = color.d3dColor;
        g_QuadVertices[3].diffuse = color.d3dColor;
    }

    this->SetRenderStateForVm(vm);
    this->AddSpriteToDrawBuffer(g_QuadVertices);

    return ZUN_SUCCESS;
}

void AnmManager::ClearVertexBuffer()
{
    this->spritesToDraw = 0;
    this->vertexBufferStartPtr = this->vertexBufferEndPtr = this->vertexBuffer;
}

void AnmManager::FlushVertexBuffer()
{
    if (this->spritesToDraw == 0)
    {
        return;
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, this->spritesToDraw * 2, this->vertexBufferStartPtr,
                                            sizeof(VertexTex1DiffuseXyzrhw));

    this->vertexBufferStartPtr = this->vertexBufferEndPtr;
    this->spritesToDraw = 0;
    this->flushesThisFrame++;
}

/* This function copies 4 vertices creating a quad into 6 vertices
 * (2 triangles) for rendering.
 */
ZunResult AnmManager::AddSpriteToDrawBuffer(VertexTex1DiffuseXyzrhw *vertices)
{
    this->vertexBufferEndPtr[0] = vertices[0];
    this->vertexBufferEndPtr[1] = vertices[1];
    this->vertexBufferEndPtr[2] = vertices[2];
    this->vertexBufferEndPtr[3] = vertices[1];
    this->vertexBufferEndPtr[4] = vertices[2];
    this->vertexBufferEndPtr[5] = vertices[3];

    this->vertexBufferEndPtr += 6;
    this->spritesToDraw++;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x00463470
u8 AnmManager::FUN_00463470(AnmVm *vm)
{
    return 0;
}

#pragma var_order(spriteHalfWidth, spriteHalfHeight)
ZunResult AnmManager::DrawNoRotation(AnmVm *vm)
{
    float spriteHalfWidth;
    float spriteHalfHeight;

    if (!vm->IsVisible())
    {
        return ZUN_ERROR;
    }

    if (!vm->flag1)
    {
        return ZUN_ERROR;
    }

    if (vm->color1.a == 0)
    {
        return ZUN_ERROR;
    }

    spriteHalfWidth = (vm->spriteSize.x * vm->scale.x) / 2.0f;
    spriteHalfHeight = (vm->spriteSize.y * vm->scale.y) / 2.0f;

    if ((vm->anchor & 1) == 0)
    {
        g_QuadVertices[0].pos.x = g_QuadVertices[2].pos.x = vm->pos.x - spriteHalfWidth;
        g_QuadVertices[1].pos.x = g_QuadVertices[3].pos.x = spriteHalfWidth + vm->pos.x;
    }
    else
    {
        g_QuadVertices[0].pos.x = g_QuadVertices[2].pos.x = vm->pos.x;
        g_QuadVertices[1].pos.x = g_QuadVertices[3].pos.x = spriteHalfWidth + vm->pos.x + spriteHalfWidth;
    }

    if ((vm->anchor & 2) == 0)
    {
        g_QuadVertices[0].pos.y = g_QuadVertices[1].pos.y = vm->pos.y - spriteHalfHeight;
        g_QuadVertices[2].pos.y = g_QuadVertices[3].pos.y = spriteHalfHeight + vm->pos.y;
    }
    else
    {
        g_QuadVertices[0].pos.y = g_QuadVertices[1].pos.y = vm->pos.y;
        g_QuadVertices[2].pos.y = g_QuadVertices[3].pos.y = spriteHalfHeight + vm->pos.y + spriteHalfHeight;
    }

    g_QuadVertices[0].pos.z = g_QuadVertices[1].pos.z = g_QuadVertices[2].pos.z = g_QuadVertices[3].pos.z = vm->pos.z;

    return this->DrawInner(vm, 1);
}

void AnmManager::TranslateRotation(VertexTex1DiffuseXyzrhw *vertex, float x, float y, float sine, float cosine,
                                   float xOffset, float yOffset)
{
    vertex->pos.x = x * cosine - y * sine + xOffset;
    vertex->pos.y = x * sine + y * cosine + yOffset;
}

#pragma var_order(sine, rotation, cosine, x, y, yOffset, xOffset)
ZunResult AnmManager::Draw2D(AnmVm *vm)
{
    float sine, cosine, rotation, xOffset, yOffset, x, y;

    if (vm->rotation.z == 0.0f)
    {
        return this->DrawNoRotation(vm);
    }

    if (!vm->IsVisible())
    {
        return ZUN_ERROR;
    }

    if (!vm->flag1)
    {
        return ZUN_ERROR;
    }

    if (vm->color1.a == 0)
    {
        return ZUN_ERROR;
    }

    rotation = vm->rotation.z;

    sincos(rotation, sine, cosine);

    xOffset = vm->pos.x;
    yOffset = vm->pos.y;

    x = (vm->spriteSize.x * vm->scale.x) / 2.0f;
    y = (vm->spriteSize.y * vm->scale.y) / 2.0f;

    this->TranslateRotation(&g_QuadVertices[0], -x, -y, sine, cosine, xOffset, yOffset);
    this->TranslateRotation(&g_QuadVertices[1], x, -y, sine, cosine, xOffset, yOffset);
    this->TranslateRotation(&g_QuadVertices[2], -x, y, sine, cosine, xOffset, yOffset);
    this->TranslateRotation(&g_QuadVertices[3], x, y, sine, cosine, xOffset, yOffset);

    g_QuadVertices[0].pos.z = g_QuadVertices[1].pos.z = g_QuadVertices[2].pos.z = g_QuadVertices[3].pos.z = vm->pos.z;

    if (vm->anchor & 1)
    {
        g_QuadVertices[0].pos.x += x;
        g_QuadVertices[1].pos.x += x;
        g_QuadVertices[2].pos.x += x;
        g_QuadVertices[3].pos.x += x;
    }

    if (vm->anchor & 2)
    {
        g_QuadVertices[0].pos.y += y;
        g_QuadVertices[1].pos.y += y;
        g_QuadVertices[2].pos.y += y;
        g_QuadVertices[3].pos.y += y;
    }

    return this->DrawInner(vm, 0);
}

/* This is identical to DrawNoRotation except for 0 being passed to DrawInner,
 * which doesn't round and subtract 0.5 from each vertex.
 */
#pragma var_order(spriteHalfWidth, spriteHalfHeight)
ZunResult AnmManager::DrawNoRotationNoRound(AnmVm *vm)
{
    float spriteHalfWidth;
    float spriteHalfHeight;

    if (!vm->IsVisible())
    {
        return ZUN_ERROR;
    }

    if (!vm->flag1)
    {
        return ZUN_ERROR;
    }

    if (vm->color1.a == 0)
    {
        return ZUN_ERROR;
    }

    spriteHalfWidth = (vm->spriteSize.x * vm->scale.x) / 2.0f;
    spriteHalfHeight = (vm->spriteSize.y * vm->scale.y) / 2.0f;

    if ((vm->anchor & 1) == 0)
    {
        g_QuadVertices[0].pos.x = g_QuadVertices[2].pos.x = vm->pos.x - spriteHalfWidth;
        g_QuadVertices[1].pos.x = g_QuadVertices[3].pos.x = spriteHalfWidth + vm->pos.x;
    }
    else
    {
        g_QuadVertices[0].pos.x = g_QuadVertices[2].pos.x = vm->pos.x;
        g_QuadVertices[1].pos.x = g_QuadVertices[3].pos.x = spriteHalfWidth + vm->pos.x + spriteHalfWidth;
    }

    if ((vm->anchor & 2) == 0)
    {
        g_QuadVertices[0].pos.y = g_QuadVertices[1].pos.y = vm->pos.y - spriteHalfHeight;
        g_QuadVertices[2].pos.y = g_QuadVertices[3].pos.y = spriteHalfHeight + vm->pos.y;
    }
    else
    {
        g_QuadVertices[0].pos.y = g_QuadVertices[1].pos.y = vm->pos.y;
        g_QuadVertices[2].pos.y = g_QuadVertices[3].pos.y = spriteHalfHeight + vm->pos.y + spriteHalfHeight;
    }

    g_QuadVertices[0].pos.z = g_QuadVertices[1].pos.z = g_QuadVertices[2].pos.z = g_QuadVertices[3].pos.z = vm->pos.z;

    return this->DrawInner(vm, 0);
}

ZunResult AnmManager::DrawTriangleFan(AnmVm *vm, VertexDiffuseXyzrhw *vertices, i32 vertexCount)
{
    if (this->spritesToDraw != 0)
    {
        this->FlushVertexBuffer();
    }

    if (this->currentVertexShader != 4)
    {
        g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        this->currentVertexShader = 4;
    }

    this->SetRenderStateForVm(vm);

    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

    g_Supervisor.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, vertexCount - 2, vertices, sizeof(VertexDiffuseXyzrhw));

    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();

    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    return ZUN_SUCCESS;
}


// STUB: th08 0x40baf0
ZunResult AnmManager::Draw2DAndFlush(AnmVm *vm)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x40ba90
void AnmManager::SetCameraMode(i32 mode)
{
}

// FUNCTION: th08 0x465070
AnmManager::AnmManager()
{
    memset((void *)this, 0, sizeof(AnmManager));

    g_AnmManagerUntexturedQuadVertices[0].w = g_AnmManagerUntexturedQuadVertices[1].w =
        g_AnmManagerUntexturedQuadVertices[2].w = g_AnmManagerUntexturedQuadVertices[3].w = 1.0f;
    g_AnmManagerUntexturedQuadVertices[0].textureUV.x = 0.0f;
    g_AnmManagerUntexturedQuadVertices[0].textureUV.y = 0.0f;
    g_AnmManagerUntexturedQuadVertices[1].textureUV.x = 1.0f;
    g_AnmManagerUntexturedQuadVertices[1].textureUV.y = 0.0f;
    g_AnmManagerUntexturedQuadVertices[2].textureUV.x = 0.0f;
    g_AnmManagerUntexturedQuadVertices[2].textureUV.y = 1.0f;
    g_AnmManagerUntexturedQuadVertices[3].textureUV.x = 1.0f;
    g_AnmManagerUntexturedQuadVertices[3].textureUV.y = 1.0f;

    g_QuadVertices[0].w = g_QuadVertices[1].w = g_QuadVertices[2].w = g_QuadVertices[3].w = 1.0f;
    g_QuadVertices[0].textureUV.x = 0.0f;
    g_QuadVertices[0].textureUV.y = 0.0f;
    g_QuadVertices[1].textureUV.x = 1.0f;
    g_QuadVertices[1].textureUV.y = 0.0f;
    g_QuadVertices[2].textureUV.x = 0.0f;
    g_QuadVertices[2].textureUV.y = 1.0f;
    g_QuadVertices[3].textureUV.x = 1.0f;
    g_QuadVertices[3].textureUV.y = 1.0f;

    this->quadVertexBuffer = NULL;
    this->currentTexture = NULL;
    this->currentBlendMode = 0;
    this->currentColorOp = 0;
    *(u32 *)((u8 *)this + 0x24B8) = 1;
    this->currentVertexShader = 0;
    this->cameraMode = (AnmCameraMode)0xff;
    this->disableZWrite = (AnmZWriteMode)0;
    this->captureAnmIdx = -1;
    this->captureSurfaceIdx = -1;
}

// FUNCTION: th08 0x465250
void AnmManager::SetupVertexBuffer()
{
    void *lockedVertexBuffer;

    this->untexturedVector[2].pos.x = -128.0f;
    this->untexturedVector[0].pos.x = -128.0f;
    this->untexturedVector[3].pos.x = 128.0f;
    this->untexturedVector[1].pos.x = 128.0f;
    this->untexturedVector[1].pos.y = -128.0f;
    this->untexturedVector[0].pos.y = -128.0f;
    this->untexturedVector[3].pos.y = 128.0f;
    this->untexturedVector[2].pos.y = 128.0f;
    this->untexturedVector[3].pos.z = 0.0f;
    this->untexturedVector[2].pos.z = 0.0f;
    this->untexturedVector[1].pos.z = 0.0f;
    this->untexturedVector[0].pos.z = 0.0f;
    this->untexturedVector[2].w = 0.0f;
    this->untexturedVector[0].w = 0.0f;
    this->untexturedVector[3].w = 1.0f;
    this->untexturedVector[1].w = 1.0f;
    *(u32 *)&this->untexturedVector[1].diffuse = 0;
    *(u32 *)&this->untexturedVector[0].diffuse = 0;
    *(u32 *)&this->untexturedVector[3].diffuse = 0x3f800000;
    *(u32 *)&this->untexturedVector[2].diffuse = 0x3f800000;

    g_BackgroundQuadVertices[0].pos = this->untexturedVector[0].pos;
    g_BackgroundQuadVertices[1].pos = this->untexturedVector[1].pos;
    g_BackgroundQuadVertices[2].pos = this->untexturedVector[2].pos;
    g_BackgroundQuadVertices[3].pos = this->untexturedVector[3].pos;
    *(u32 *)&g_BackgroundQuadVertices[0].textureUV.x = *(u32 *)&this->untexturedVector[0].w;
    *(u32 *)&g_BackgroundQuadVertices[0].textureUV.y = *(u32 *)&this->untexturedVector[0].diffuse;
    *(u32 *)&g_BackgroundQuadVertices[1].textureUV.x = *(u32 *)&this->untexturedVector[1].w;
    *(u32 *)&g_BackgroundQuadVertices[1].textureUV.y = *(u32 *)&this->untexturedVector[1].diffuse;
    *(u32 *)&g_BackgroundQuadVertices[2].textureUV.x = *(u32 *)&this->untexturedVector[2].w;
    *(u32 *)&g_BackgroundQuadVertices[2].textureUV.y = *(u32 *)&this->untexturedVector[2].diffuse;
    *(u32 *)&g_BackgroundQuadVertices[3].textureUV.x = *(u32 *)&this->untexturedVector[3].w;
    *(u32 *)&g_BackgroundQuadVertices[3].textureUV.y = *(u32 *)&this->untexturedVector[3].diffuse;

    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->CreateVertexBuffer(sizeof(this->untexturedVector), 0, D3DFVF_XYZ | D3DFVF_TEX1,
                                                   D3DPOOL_MANAGED, &this->quadVertexBuffer);
        this->quadVertexBuffer->Lock(0, 0, (BYTE **)&lockedVertexBuffer, 0);
        memcpy(lockedVertexBuffer, this->untexturedVector, sizeof(this->untexturedVector));
        this->quadVertexBuffer->Unlock();
        g_Supervisor.d3dDevice->SetStreamSource(0, g_AnmManager->quadVertexBuffer, sizeof(VertexDiffuseXyzrhw));
    }
}

static i32 GetAnmFormat(i32 format)
{
    if (g_Supervisor.Is16bitColorMode() != 0)
    {
        if ((g_TextureFormatD3D8Mapping[format] == D3DFMT_A8R8G8B8) ||
            (g_TextureFormatD3D8Mapping[format] == D3DFMT_UNKNOWN))
        {
            format = 5;
        }
        else if (g_TextureFormatD3D8Mapping[format] == D3DFMT_R8G8B8)
        {
            format = 3;
        }
    }

    return format;
}

// FUNCTION: th08 0x465570
ZunResult AnmManager::CreateTextureFromFile(AnmEntry *entry, i32 format, i32 colorKey)
{
    format = GetAnmFormat(format);
    if (D3DXCreateTextureFromFileInMemoryEx(g_Supervisor.d3dDevice, entry->rawData, entry->size, 0, 0, 0, 0,
                                            g_TextureFormatD3D8Mapping[format], D3DPOOL_MANAGED, 3, -1, colorKey,
                                            NULL, NULL, &entry->texture) != D3D_OK)
    {
        return ZUN_ERROR;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(surface, textureSurfaceLevel, header, lockedRect, currentY, textureSrc, textureDest)
ZunResult AnmManager::CreateTextureFromAnm(IDirect3DTexture8 **outTexture, AnmTextureHeader *textureHeader, i32 format)
{
    IDirect3DSurface8 *surface;
    IDirect3DSurface8 *textureSurfaceLevel;
    AnmTextureHeader *header;
    const void *textureSrc;
    void *textureDest;
    D3DLOCKED_RECT lockedRect;
    int currentY;

    surface = NULL;
    textureSurfaceLevel = NULL;
    format = GetAnmFormat(format);
    header = textureHeader;

    g_Supervisor.d3dDevice->CreateImageSurface(header->width, header->height,
                                               g_TextureFormatD3D8Mapping[header->format], &surface);

    surface->LockRect(&lockedRect, NULL, 0);

    for (currentY = 0; currentY < header->height; currentY++)
    {
        textureDest = (u8 *)lockedRect.pBits + currentY * lockedRect.Pitch;
        textureSrc = ((u8 *)textureHeader) + sizeof(AnmTextureHeader) +
                     (currentY * header->width * g_TextureFormatBytesPerPixel[header->format]);
        memcpy(textureDest, textureSrc, header->width * g_TextureFormatBytesPerPixel[header->format]);
    }

    surface->UnlockRect();

    if (D3DXCreateTexture(g_Supervisor.d3dDevice, header->width, header->height, 1, 0,
                          g_TextureFormatD3D8Mapping[format], D3DPOOL_MANAGED, outTexture) != D3D_OK)
    {
        goto err;
    }

    (*outTexture)->GetSurfaceLevel(0, &textureSurfaceLevel);

    if (D3DXLoadSurfaceFromSurface(textureSurfaceLevel, NULL, NULL, surface, NULL, NULL, 3, 0) != D3D_OK)
    {
        goto err;
    }

    if (surface != NULL)
    {
        surface->Release();
        surface = NULL;
    }
    if (textureSurfaceLevel != NULL)
    {
        textureSurfaceLevel->Release();
        textureSurfaceLevel = NULL;
    }

    return ZUN_SUCCESS;

err:
    if (surface != NULL)
    {
        surface->Release();
        surface = NULL;
    }
    if (textureSurfaceLevel != NULL)
    {
        textureSurfaceLevel->Release();
        textureSurfaceLevel = NULL;
    }

    return ZUN_ERROR;
}

ZunResult AnmManager::CreateEmptyTexture(IDirect3DTexture8 **outTexture, i32 width, i32 height, i32 format)
{
    D3DXCreateTexture(g_Supervisor.d3dDevice, width, height, 1, 0, g_TextureFormatD3D8Mapping[format], D3DPOOL_MANAGED,
                      outTexture);

    return ZUN_SUCCESS;
}

AnmLoaded *AnmManager::LoadAnm(i32 anmIdx, const char *filename)
{
    utils::DebugPrint("::loadAnim : %s\n", filename);
    AnmLoaded *anmLoaded = this->ReadAnmEntries(anmIdx, filename);
    if (anmLoaded != NULL)
    {
        anmLoaded->numberEntriesToBeLoaded = 1;

#ifdef FIX_REALLY_BAD_BUGS
        while (anmLoaded != NULL && anmLoaded->numberEntriesToBeLoaded != 0)
#else
        /* ZUN bug: no NULL check! */
        while (anmLoaded->numberEntriesToBeLoaded != 0)
#endif
        {
            anmLoaded = this->PostloadAnmEntry(anmLoaded);
        }
    }

    return anmLoaded;
}

#pragma var_order(curEntryNum, totalSprites, totalEntries, anmLoaded, entry, result, totalScripts, curEntry)
AnmLoaded *AnmManager::ReadAnmEntries(int anmIdx, const char *filename)
{
    i32 result;

    utils::DebugPrint("::preloadAnim : %s\n", filename);

    if (anmIdx >= 25)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_NO_TEXTURE_STORAGE);
        return NULL;
    }

    this->ReleaseAnm(anmIdx);

    AnmRawEntry *entry = (AnmRawEntry *)FileSystem::OpenFile(filename, NULL, 0);
    i32 totalEntries = 0;
    i32 totalScripts = 0;
    i32 totalSprites = 0;
    i32 curEntryNum = 0;

    AnmLoaded *anmLoaded = this->anmFiles + anmIdx;
    if (entry == NULL)
    {
        return NULL;
    }

    anmLoaded->anmIdx = anmIdx;
    anmLoaded->rawData = entry;
    AnmRawEntry *curEntry = entry;

    while (true)
    {
        totalEntries++;
        totalScripts += curEntry->numScripts;
        totalSprites += curEntry->numSprites;

        if (curEntry->nextOffset == 0)
        {
            break;
        }

        curEntry = (AnmRawEntry *)(((u8 *)curEntry) + curEntry->nextOffset);
    }

    anmLoaded->totalEntries = totalEntries;

    anmLoaded->textures = (AnmEntry *)g_ZunMemory.Alloc(totalEntries * sizeof(AnmEntry));
    memset(anmLoaded->textures, 0, sizeof(AnmEntry) * totalEntries);
    anmLoaded->sprites = (AnmLoadedSprite *)g_ZunMemory.Alloc(totalSprites * sizeof(AnmLoadedSprite));
    anmLoaded->scripts = (AnmRawInstr **)g_ZunMemory.Alloc(totalScripts * sizeof(void *));

    curEntry = entry;
    totalEntries = 0;
    totalSprites = 0;
    totalScripts = 0;

    while (true)
    {
        result = this->LoadExternalTextureData(anmLoaded, curEntryNum, &totalSprites, &totalScripts, curEntry);
        if (result < ZUN_SUCCESS)
        {
            return NULL;
        }

        curEntryNum++;

        if (curEntry->nextOffset == 0)
        {
            break;
        }

        curEntry = (AnmRawEntry *)(((u8 *)curEntry) + curEntry->nextOffset);
    }

    return anmLoaded;
}

AnmLoaded *AnmManager::PreloadAnm(i32 anmIdx, const char *filename)
{
    AnmLoaded *anmLoaded = this->ReadAnmEntries(anmIdx, filename);
    if (anmLoaded == NULL)
    {
        return NULL;
    }

    /* AnmManager::ServicePreloadedAnims, called on the main thread every
     * frame through Supervisor::OnUpdate will process one entry on each
     * loading file until loading is finished
     */
    anmLoaded->numberEntriesToBeLoaded = 1;
    while (anmLoaded->numberEntriesToBeLoaded != 0 && !g_Supervisor.subthreadCloseRequestActive)
    {
        Sleep(1);
    }
    utils::DebugPrint("::preloadAnimEnd : %s\n", filename);

    return g_Supervisor.subthreadCloseRequestActive ? NULL : anmLoaded;
}

// STUB: th08 0x465ac0
#pragma var_order(result, startOfEntry, path, fileSize, fileData)
i32 AnmManager::LoadExternalTextureData(AnmLoaded *anmLoaded, i32 entryNumber, i32 *sprites, i32 *scripts,
                                        AnmRawEntry *rawEntry)
{
    i32 result = 0;
    AnmRawEntry *startOfEntry;
    const char *path;
    i32 fileSize;
    u8 *fileData;

    if (rawEntry == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_ANIMATION_CORRUPTED);
        return ZUN_ERROR;
    }

    startOfEntry = rawEntry;
    if (startOfEntry->version != 3)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_ANIMATION_WRONG_VERSION);
        return ZUN_ERROR;
    }

    if (!startOfEntry->hasData)
    {
        path = (const char *)((u8 *)startOfEntry + startOfEntry->nameOffset);
        if (path[0] != '@')
        {
            fileData = FileSystem::OpenFile(path, &fileSize, TRUE);
            if (fileData == NULL)
            {
                g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_EXTERN_TEXTURE_CORRUPTED, path);
                return ZUN_ERROR;
            }
            anmLoaded->textures[entryNumber].size = fileSize;
            anmLoaded->textures[entryNumber].rawData = fileData;
        }
    }

    return result + 1;
}

#pragma var_order(currentEntryNumber, currentNumSprites, entryLoadNumber, data, result, currentNumScripts, rawEntry)
AnmLoaded *AnmManager::PostloadAnmEntry(AnmLoaded *anmLoaded)
{
    i32 result;

    utils::DebugPrint("::postloadAnim : %d, %d\n", anmLoaded->anmIdx, anmLoaded->numberEntriesToBeLoaded);

    AnmRawEntry *rawData = anmLoaded->rawData;

    i32 entryLoadNumber = 0;
    i32 currentNumScripts = 0;
    i32 currentNumSprites = 0;
    i32 currentEntryNumber = 0;

    /* ??? */
    anmLoaded->rawData = rawData;
    AnmRawEntry *rawEntry = rawData;

    while (true)
    {
        if (entryLoadNumber == anmLoaded->numberEntriesToBeLoaded - 1 &&
            (result = this->LoadTextureData(anmLoaded, currentEntryNumber, currentNumSprites, currentNumScripts,
                                            rawEntry)) < ZUN_SUCCESS)
        {
            anmLoaded->numberEntriesToBeLoaded = 0;
            return NULL;
        }

        currentNumSprites += rawEntry->numSprites;
        currentNumScripts += rawEntry->numScripts;
        currentEntryNumber++;

        if (rawEntry->nextOffset == 0)
        {
            break;
        }

        rawEntry = (AnmRawEntry *)(((u8 *)rawEntry) + rawEntry->nextOffset);
        entryLoadNumber++;

        if (entryLoadNumber == anmLoaded->numberEntriesToBeLoaded)
        {
            anmLoaded->numberEntriesToBeLoaded++;
            return anmLoaded;
        }
    }

    anmLoaded->numberEntriesToBeLoaded = 0;

    return anmLoaded;
}

#pragma var_order(result, startOfEntry, surfaceDesc, path, rawSprite, i, currentOffset, loadedSprite)
int AnmManager::LoadTextureData(AnmLoaded *anmLoaded, i32 entryNumber, i32 currentSpriteNumber, i32 currentScriptNumber,
                                AnmRawEntry *rawEntry)
{
    int result = 0;
    AnmLoadedSprite loadedSprite;
    int i;
    const char *path;

    if (rawEntry == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_ANIMATION_CORRUPTED);
        return ZUN_ERROR;
    }

    AnmRawEntry *startOfEntry = rawEntry;

    if (startOfEntry->version != 3)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_ANIMATION_WRONG_VERSION);
        return ZUN_ERROR;
    }

    if (!startOfEntry->hasData)
    {
        path = (const char *)(((u8 *)startOfEntry) + startOfEntry->nameOffset);

        if (path[0] == '@')
        {
            this->CreateEmptyTexture(&anmLoaded->textures[entryNumber].texture, startOfEntry->width,
                                     startOfEntry->height, startOfEntry->format);
        }
        else
        {
            if (this->CreateTextureFromFile(&anmLoaded->textures[entryNumber], startOfEntry->format,
                                            startOfEntry->colorKey) != ZUN_SUCCESS)
            {
                g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_EXTERN_TEXTURE_CORRUPTED, path);
                return ZUN_ERROR;
            }
        }
    }
    else
    {
        if (this->CreateTextureFromAnm(&anmLoaded->textures[entryNumber].texture,
                                       (AnmTextureHeader *)(((u8 *)startOfEntry) + startOfEntry->textureOffset),
                                       startOfEntry->format) != ZUN_SUCCESS)
        {
            g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED);
            return ZUN_ERROR;
        }
    }

    anmLoaded->textures[entryNumber].texture->SetPriority(startOfEntry->priority);
    anmLoaded->textures[entryNumber].texture->PreLoad();

    D3DSURFACE_DESC surfaceDesc;

    anmLoaded->textures[entryNumber].texture->GetLevelDesc(0, &surfaceDesc);

    u32 *currentOffset = (u32 *)((u8 *)startOfEntry + sizeof(AnmRawEntry));

    AnmRawSprite *rawSprite;

    for (i = 0; i < startOfEntry->numSprites; i++, currentOffset++)
    {
        rawSprite = (AnmRawSprite *)((u8 *)startOfEntry + *currentOffset);

        loadedSprite.anmIdx = anmLoaded->anmIdx;
        loadedSprite.texture = anmLoaded->textures[entryNumber].texture;
        loadedSprite.scaleFactor.x = surfaceDesc.Width / (float)startOfEntry->width;
        loadedSprite.scaleFactor.y = surfaceDesc.Height / (float)startOfEntry->height;

        loadedSprite.startPixelInclusive.x = rawSprite->x * loadedSprite.scaleFactor.x;
        loadedSprite.startPixelInclusive.y = rawSprite->y * loadedSprite.scaleFactor.y;
        loadedSprite.endPixelInclusive.x = (rawSprite->x + rawSprite->width) * loadedSprite.scaleFactor.x;
        loadedSprite.endPixelInclusive.y = (rawSprite->y + rawSprite->height) * loadedSprite.scaleFactor.y;
        loadedSprite.width = surfaceDesc.Width;
        loadedSprite.height = surfaceDesc.Height;

        anmLoaded->LoadSprite(currentSpriteNumber, &loadedSprite);

        currentSpriteNumber++;
    }

    for (i = 0; i < startOfEntry->numScripts; i++, currentOffset += 2)
    {
        anmLoaded->scripts[currentScriptNumber] = (AnmRawInstr *)(((u8 *)startOfEntry) + currentOffset[1]);
        currentScriptNumber++;
    }

    return result + 1;
}

ZunResult AnmManager::ServicePreloadedAnims()
{
    for (int i = 0; i < ARRAY_SIZE(this->anmFiles); i++)
    {
        if (this->anmFiles[i].numberEntriesToBeLoaded != 0 && this->PostloadAnmEntry(this->anmFiles + i) == NULL)
        {
            return ZUN_ERROR;
        }
    }

    return ZUN_SUCCESS;
}

void AnmManager::ReleaseAnm(i32 anmIdx)
{
    if (anmIdx < 0 || anmIdx >= ARRAY_SIZE(this->anmFiles))
    {
        return;
    }

    if (this->anmFiles[anmIdx].rawData != NULL)
    {
        for (int i = 0; i < this->anmFiles[anmIdx].totalEntries; i++)
        {
            this->ReleaseAnmEntry(&this->anmFiles[anmIdx].textures[i]);
        }

        g_ZunMemory.Free(this->anmFiles[anmIdx].textures);
        g_ZunMemory.Free(this->anmFiles[anmIdx].sprites);
        g_ZunMemory.Free(this->anmFiles[anmIdx].scripts);
        g_ZunMemory.Free(this->anmFiles[anmIdx].rawData);

        memset(&this->anmFiles[anmIdx], 0, sizeof(AnmLoaded));
    }
}

void AnmManager::ReleaseAnmEntry(AnmEntry *entry)
{
    if (entry->texture != NULL)
    {
        entry->texture->Release();
        entry->texture = NULL;
    }
    if (entry->rawData != NULL)
    {
        g_ZunMemory.Free(entry->rawData);
        /* there should be a entry->rawData = NULL */
    }
}

void AnmLoaded::LoadSprite(i32 spriteIdx, AnmLoadedSprite *loadedSprite)
{
    this->sprites[spriteIdx] = *loadedSprite;

    this->sprites[spriteIdx].uvStart.x =
        this->sprites[spriteIdx].startPixelInclusive.x / (this->sprites[spriteIdx].width);
    this->sprites[spriteIdx].uvEnd.x = this->sprites[spriteIdx].endPixelInclusive.x / (this->sprites[spriteIdx].width);
    this->sprites[spriteIdx].uvStart.y =
        this->sprites[spriteIdx].startPixelInclusive.y / (this->sprites[spriteIdx].height);
    this->sprites[spriteIdx].uvEnd.y = this->sprites[spriteIdx].endPixelInclusive.y / (this->sprites[spriteIdx].height);
    this->sprites[spriteIdx].widthPx =
        (this->sprites[spriteIdx].endPixelInclusive.x - this->sprites[spriteIdx].startPixelInclusive.x) /
        (loadedSprite->scaleFactor.x);
    this->sprites[spriteIdx].heightPx =
        (this->sprites[spriteIdx].endPixelInclusive.y - this->sprites[spriteIdx].startPixelInclusive.y) /
        (loadedSprite->scaleFactor.y);
}

void AnmManager::DrawTextInner(IDirect3DTexture8 *outTexture, i32 x, i32 y, i32 width, i32 height, i32 fontWidth,
                               i32 fontHeight, COLORREF textColor, COLORREF outlineColor, const char *buffer,
                               float scaleFactorX, float scaleFactorY)
{
    if (fontWidth <= 0)
    {
        fontWidth = 15;
    }

    if (fontHeight <= 0)
    {
        fontHeight = 15;
    }

    if (fontWidth > 8)
    {
        TextHelper::RenderTextToTextureBold(x, y, width, height, fontWidth * scaleFactorX, fontHeight * scaleFactorY,
                                            textColor, outlineColor, buffer, outTexture);
    }
    else
    {
        TextHelper::RenderTextToTexture(x, y, width, height, 8, 8, textColor, outlineColor, buffer, outTexture);
    }
}

#pragma var_order(buf, fontWidth)
void AnmManager::DrawTextLeft(AnmVm *vm, COLORREF textColor, COLORREF shadowColor, const char *fmt, ...)
{
    char buf[128];
    int fontWidth = vm->fontWidth;

    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    this->DrawTextInner(vm->loadedSprite->texture, vm->loadedSprite->startPixelInclusive.x,
                        vm->loadedSprite->startPixelInclusive.y, vm->loadedSprite->width, vm->loadedSprite->height,
                        fontWidth, vm->fontHeight, textColor, shadowColor, buf, vm->loadedSprite->scaleFactor.x,
                        vm->loadedSprite->scaleFactor.y);

    vm->visible = true;
}

// STUB: th08 0x466650
void AnmManager::DrawTextCentered(AnmVm *vm, COLORREF textColor, COLORREF shadowColor, const char *fmt, ...)
{
}

#pragma var_order(surface, fileSize, fileData)
ZunResult AnmManager::LoadSurface(i32 surfaceIdx, const char *filename)
{
    u8 *fileData;
    i32 fileSize;
    IDirect3DSurface8 *surface;

    if (this->surfaces[surfaceIdx] != NULL)
    {
        this->ReleaseSurface(surfaceIdx);
    }

    if (surfaceData[surfaceIdx] == NULL)
    {
        fileData = FileSystem::OpenFile(filename, &fileSize, 0);
        if (fileData == NULL)
        {
            g_GameErrorContext.Fatal(TH_ERR_CANNOT_BE_LOADED, filename);
            return ZUN_ERROR;
        }
    }
    else
    {
        fileData = this->surfaceData[surfaceIdx];
        fileSize = this->surfaceDataSizes[surfaceIdx];
        this->surfaceData[surfaceIdx] = NULL;
    }

    if (g_Supervisor.d3dDevice->CreateImageSurface(640, 1024, g_Supervisor.presentParameters.BackBufferFormat,
                                                   &surface) != D3D_OK)
    {
        return ZUN_ERROR;
    }

    if (D3DXLoadSurfaceFromFileInMemory(surface, NULL, NULL, fileData, fileSize, NULL, 1, 0,
                                        (D3DXIMAGE_INFO *)&surfaceInfo[surfaceIdx]) != D3D_OK)
    {
        goto err;
    }

    if (g_Supervisor.d3dDevice->CreateRenderTarget(this->surfaceInfo[surfaceIdx].Width, surfaceInfo[surfaceIdx].Height,
                                                   g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE,
                                                   1, &this->surfaces[surfaceIdx]) != D3D_OK)
    {
        if (g_Supervisor.d3dDevice->CreateImageSurface(
                this->surfaceInfo[surfaceIdx].Width, this->surfaceInfo[surfaceIdx].Height,
                g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[surfaceIdx]) != D3D_OK)
        {
            goto err;
        }
    }

    if (g_Supervisor.d3dDevice->CreateImageSurface(
            this->surfaceInfo[surfaceIdx].Width, this->surfaceInfo[surfaceIdx].Height,
            g_Supervisor.presentParameters.BackBufferFormat, &this->surfacesBis[surfaceIdx]) != D3D_OK)
    {
        goto err;
    }

    if (D3DXLoadSurfaceFromSurface(this->surfaces[surfaceIdx], NULL, NULL, surface, NULL, NULL, D3DX_FILTER_NONE, 0) !=
        D3D_OK)
    {
        goto err;
    }

    if (D3DXLoadSurfaceFromSurface(this->surfacesBis[surfaceIdx], NULL, NULL, surface, NULL, NULL, D3DX_FILTER_NONE,
                                   0) != D3D_OK)
    {
        goto err;
    }

    if (surface != NULL)
    {
        surface->Release();
        surface = NULL;
    }
    g_ZunMemory.Free(fileData);

    return ZUN_SUCCESS;
err:
    if (surface != NULL)
    {
        surface->Release();
        surface = NULL;
    }
    g_ZunMemory.Free(fileData);

    return ZUN_ERROR;
}

#pragma var_order(fileSize, fileData)
ZunResult AnmManager::PreloadSurface(i32 surfaceIdx, const char *path)
{
    u32 fileSize;
    u8 *fileData;

    if (this->surfaces[surfaceIdx] != NULL)
    {
        this->ReleaseSurface(surfaceIdx);
    }

    fileData = FileSystem::OpenFile(path, (i32 *)&fileSize, 0);
    if (fileData == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_CANNOT_BE_LOADED, path);
        return ZUN_ERROR;
    }

    this->surfaceData[surfaceIdx] = fileData;
    this->surfaceDataSizes[surfaceIdx] = fileSize;

    return ZUN_SUCCESS;
}

void AnmManager::ReleaseSurface(i32 surfaceIdx)
{
    if (this->surfaces[surfaceIdx] != NULL)
    {
        this->surfaces[surfaceIdx]->Release();
        this->surfaces[surfaceIdx] = NULL;
    }
    if (this->surfacesBis[surfaceIdx] != NULL)
    {
        this->surfacesBis[surfaceIdx]->Release();
        this->surfacesBis[surfaceIdx] = NULL;
    }
    if (this->surfaceData[surfaceIdx] != NULL)
    {
        g_ZunMemory.Free(this->surfaceData[surfaceIdx]);
    }
    this->surfaceData[surfaceIdx] = NULL;
}

/* completely identical to EoSD. */
void AnmManager::CopySurfaceToBackbuffer(i32 surfaceIdx, i32 left, i32 top, i32 x, i32 y)
{
    if (this->surfacesBis[surfaceIdx] == NULL)
    {
        return;
    }

    IDirect3DSurface8 *destSurface;
    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &destSurface) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surfaceIdx] == NULL)
    {
        if (g_Supervisor.d3dDevice->CreateRenderTarget(
                this->surfaceInfo[surfaceIdx].Width, this->surfaceInfo[surfaceIdx].Height,
                g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surfaceIdx]) != D3D_OK)
        {
            if (g_Supervisor.d3dDevice->CreateImageSurface(
                    this->surfaceInfo[surfaceIdx].Width, this->surfaceInfo[surfaceIdx].Height,
                    g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[surfaceIdx]) != D3D_OK)
            {
                destSurface->Release();
                return;
            }
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surfaceIdx], NULL, NULL, this->surfacesBis[surfaceIdx], NULL,
                                       NULL, D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            destSurface->Release();
            return;
        }
    }

    RECT sourceRect;
    POINT destPoint;
    sourceRect.left = left;
    sourceRect.top = top;
    sourceRect.right = this->surfaceInfo[surfaceIdx].Width;
    sourceRect.bottom = this->surfaceInfo[surfaceIdx].Height;
    destPoint.x = x;
    destPoint.y = y;
    g_Supervisor.d3dDevice->CopyRects(this->surfaces[surfaceIdx], &sourceRect, 1, destSurface, &destPoint);
    destSurface->Release();
}

void AnmManager::CopySurfaceToBackbuffer2(i32 surfaceIdx, i32 rectX, i32 rectY, i32 rectLeft, i32 rectTop, i32 width,
                                          i32 height)
{
    if (this->surfacesBis[surfaceIdx] == NULL)
    {
        return;
    }

    IDirect3DSurface8 *backbuffer;
    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer) != D3D_OK)
    {
        return;
    }

    if (this->surfaces[surfaceIdx] == NULL)
    {
        if (g_Supervisor.d3dDevice->CreateRenderTarget(
                this->surfaceInfo[surfaceIdx].Width, this->surfaceInfo[surfaceIdx].Height,
                g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surfaceIdx]) != D3D_OK)
        {
            if (g_Supervisor.d3dDevice->CreateImageSurface(
                    this->surfaceInfo[surfaceIdx].Width, this->surfaceInfo[surfaceIdx].Height,
                    g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[surfaceIdx]) != D3D_OK)
            {
                backbuffer->Release();
                return;
            }
        }

        if (D3DXLoadSurfaceFromSurface(this->surfaces[surfaceIdx], NULL, NULL, this->surfacesBis[surfaceIdx], NULL,
                                       NULL, D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backbuffer->Release();
            return;
        }
    }

    RECT rect;
    POINT point;
    rect.left = rectLeft;
    rect.top = rectTop;
    rect.right = rectLeft + width;
    rect.bottom = rectTop + height;
    point.x = rectX;
    point.y = rectY;
    g_Supervisor.d3dDevice->CopyRects(this->surfaces[surfaceIdx], &rect, 1, backbuffer, &point);
    backbuffer->Release();
}

// FUNCTION: th08 0x466f20
#pragma var_order(srcRect, textureSurface, backbuffer, dstRect, this)
void AnmManager::CaptureToTexture(i32 captureAnmIdx, i32 srcX, i32 srcY, i32 srcW, i32 srcH, i32 dstX, i32 dstY,
                                  i32 dstW, i32 dstH)
{
    IDirect3DSurface8 *backbuffer;
    IDirect3DSurface8 *textureSurface;
    RECT srcRect;
    RECT dstRect;

    if (this->anmFiles[captureAnmIdx].textures->texture == NULL)
    {
        return;
    }

    this->FlushVertexBuffer();

    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer) != D3D_OK)
    {
        return;
    }

    if (this->anmFiles[captureAnmIdx].textures->texture->GetSurfaceLevel(0, &textureSurface) != D3D_OK)
    {
        backbuffer->Release();
        return;
    }

    srcRect.left = srcX;
    srcRect.top = srcY;
    srcRect.right = srcX + srcW;
    srcRect.bottom = srcY + srcH;
    dstRect.left = dstX;
    dstRect.top = dstY;
    dstRect.right = dstX + dstW;
    dstRect.bottom = dstY + dstH;

    if (D3DXLoadSurfaceFromSurface(textureSurface, NULL, &dstRect, backbuffer, NULL, &srcRect, -1, 0) != D3D_OK)
    {
        textureSurface->Release();
        backbuffer->Release();
        return;
    }

    textureSurface->Release();
    backbuffer->Release();
}

#pragma var_order(srcRect, backbuffer, dstRect)
void AnmManager::CaptureToSurface(i32 captureSurfaceIdx, i32 srcX, i32 srcY, i32 srcW, i32 srcH, i32 dstX, i32 dstY,
                                  i32 dstW, i32 dstH)
{
    IDirect3DSurface8 *backbuffer;
    RECT srcRect;
    RECT dstRect;

    this->FlushVertexBuffer();

    if (this->surfaces[captureSurfaceIdx] != NULL)
    {
        this->ReleaseSurface(captureSurfaceIdx);
    }

    srcRect.left = srcX;
    srcRect.top = srcY;
    srcRect.right = srcX + srcW;
    srcRect.bottom = srcY + srcH;

    dstRect.left = dstX;
    dstRect.top = dstY;
    dstRect.right = dstX + dstW;
    dstRect.bottom = dstY + dstH;

    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer) != D3D_OK)
    {
        return;
    }

    this->surfaceInfo[captureSurfaceIdx].Width = dstW;
    this->surfaceInfo[captureSurfaceIdx].Height = dstH;

    if (g_Supervisor.d3dDevice->CreateRenderTarget(this->surfaceInfo[captureSurfaceIdx].Width,
                                                   this->surfaceInfo[captureSurfaceIdx].Height,
                                                   g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE,
                                                   1, &this->surfaces[captureSurfaceIdx]) != D3D_OK)
    {
        if (g_Supervisor.d3dDevice->CreateImageSurface(
                this->surfaceInfo[captureSurfaceIdx].Width, this->surfaceInfo[captureSurfaceIdx].Height,
                g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[captureSurfaceIdx]) != D3D_OK)
        {
            goto out;
        }
    }

    if (g_Supervisor.d3dDevice->CreateImageSurface(
            this->surfaceInfo[captureSurfaceIdx].Width, this->surfaceInfo[captureSurfaceIdx].Height,
            g_Supervisor.presentParameters.BackBufferFormat, &this->surfacesBis[captureSurfaceIdx]) != D3D_OK)
    {
        goto out;
    }

    if (D3DXLoadSurfaceFromSurface(this->surfaces[captureSurfaceIdx], NULL, &dstRect, backbuffer, NULL, &srcRect, -1,
                                   0) != D3D_OK)
    {
        goto out;
    }

    D3DXLoadSurfaceFromSurface(this->surfacesBis[captureSurfaceIdx], NULL, NULL, this->surfaces[captureSurfaceIdx],
                               NULL, NULL, -1, 0);

out:
    SAFE_RELEASE(backbuffer);
}

}; // Namespace th08
