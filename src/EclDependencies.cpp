#include "th_pch.h"

#include "Gui.hpp"
#include "Spellcard.hpp"
#include "EnemyManager.hpp"
#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "EclOperands.hpp"
#include "EclManager.hpp"
#include "utils.hpp"

#include <math.h>

namespace th08
{

struct EclSpellCardInstructionArgs
{
    u8 header[0xC];
    i16 enemyFace;
    u16 spellCardNumber;
    i32 bonus;
    u8 encodedName[0x30];
    u8 encodedOwner[0x30];
    char commentLine1[0x40];
    char commentLine2[0x40];
};

C_ASSERT(offsetof(EclSpellCardInstructionArgs, enemyFace) == 0x0C);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, spellCardNumber) == 0x0E);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, bonus) == 0x10);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, encodedName) == 0x14);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, encodedOwner) == 0x44);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, commentLine1) == 0x74);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, commentLine2) == 0xB4);

// FUNCTION: th08 0x421280
void __fastcall StartEnemySpell(u8 *enemy, void *instruction)
{
    g_Spellcard.StartSpell(static_cast<EclSpellCardInstructionArgs *>(instruction)->spellCardNumber,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->encodedName,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->enemyFace,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->bonus, enemy,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->encodedOwner,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->commentLine1,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->commentLine2);
}

// FUNCTION: th08 0x4212e0
void __fastcall EndEnemySpell(u8 *enemy, void *instruction)
{
    g_Spellcard.EndSpell();
}

// FUNCTION: th08 0x4230c0
void Gui::FUN_004230c0(f32 value)
{
    this->bossLifeBarSize = value;
}

// FUNCTION: th08 0x4230e0
void Gui::SetBossGaugeSlot(i32 index, f32 start, f32 stop)
{
    this->bossLifeBarSegmentStart[index] = start;
    this->bossLifeBarSegmentStop[index] = stop;
}

// FUNCTION: th08 0x423110
void Gui::SetBossGaugeValue(i32 index, i32 value)
{
    this->bossLifeBarSegmentColor[index] = value;
}

// FUNCTION: th08 0x423130
void Gui::FUN_00423130(i32 value)
{
    this->eclSetLives = value;
}



namespace EclRunLowProposal
{

#define DEP_BYTES(enemy) (reinterpret_cast<u8 *>(enemy))
#define DEP_READ_INT(enemy, instruction, index) \
    ((instruction)->operandFlags & (1U << (index)) \
         ? EclOperands::ResolveInt((enemy), *reinterpret_cast<i32 *>((instruction)->operands + (index) * 4)) \
         : *reinterpret_cast<i32 *>((instruction)->operands + (index) * 4))
#define DEP_READ_FLOAT(enemy, instruction, index) \
    ((instruction)->operandFlags & (1U << (index)) \
         ? (enemy)->ResolveFloat(*reinterpret_cast<f32 *>((instruction)->operands + (index) * 4)) \
         : *reinterpret_cast<f32 *>((instruction)->operands + (index) * 4))

struct TimedPolarFlags
{
    u32 lowBits : 12;
    u32 movementMode : 2;
    u32 easingMode : 3;
    u32 bit17 : 1;
    u32 mirrorX : 1;
    u32 highBits : 13;
};
C_ASSERT(sizeof(TimedPolarFlags) == 4);

// FUNCTION: th08 0x4222b0
void __fastcall StartTimedPolarDisplacement(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction, f32 angle)
{
    *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2dc4) =
        cosf(angle) * DEP_READ_FLOAT(enemy, instruction, 2) *
        DEP_READ_INT(enemy, instruction, 0);
    *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2dc8) =
        sinf(angle) * DEP_READ_FLOAT(enemy, instruction, 2) *
        DEP_READ_INT(enemy, instruction, 0);
    *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2dcc) = 0.0f;
    *reinterpret_cast<Float3 *>(DEP_BYTES(enemy) + 0x2dd0) =
        *reinterpret_cast<Float3 *>(DEP_BYTES(enemy) + 0x2d88);
    *reinterpret_cast<ZunTimer *>(DEP_BYTES(enemy) + 0x2ddc) =
        (*reinterpret_cast<i32 *>(DEP_BYTES(enemy) + 0x2de8) =
             DEP_READ_INT(enemy, instruction, 0));
    reinterpret_cast<TimedPolarFlags *>(DEP_BYTES(enemy) + 0x3324)->easingMode =
        DEP_READ_INT(enemy, instruction, 1);
    reinterpret_cast<TimedPolarFlags *>(DEP_BYTES(enemy) + 0x3324)->movementMode = 2;
}

// FUNCTION: th08 0x422020
void __fastcall BeginBoundaryAwareMove(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    f32 angle;

    if (EclOperands::g_TargetPlayerPosition017D61AC.x <
        *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2d34))
    {
        angle = AddNormalizeAngle(
            g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
    }

    if (reinterpret_cast<Float3 *>(DEP_BYTES(enemy) + 0x2d34)->operator float *()[0] <
        *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x3340) + 96.0f)
    {
        if (angle > 1.5707964f)
            angle = 3.1415927f - angle;
        else if (angle < -1.5707964f)
            angle = -3.1415927f - angle;
    }

    if (reinterpret_cast<Float3 *>(DEP_BYTES(enemy) + 0x2d34)->operator float *()[0] >
        *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x3348) - 96.0f)
    {
        if (angle < 1.5707964f && angle >= 0.0f)
            angle = 3.1415927f - *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2d94);
        else if (angle > -1.5707964f && angle <= 0.0f)
            angle = -3.1415927f - angle;
    }

    if (reinterpret_cast<Float3 *>(DEP_BYTES(enemy) + 0x2d34)->operator float *()[1] <
            *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x3344) + 48.0f &&
        angle < 0.0f)
    {
        angle = -angle;
    }

    if (reinterpret_cast<Float3 *>(DEP_BYTES(enemy) + 0x2d34)->operator float *()[1] >
            *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x334c) - 48.0f &&
        angle > 0.0f)
    {
        angle = -angle;
    }

    if (DEP_READ_INT(enemy, instruction, 0) <= 0)
    {
        *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2d94) = angle;
        *reinterpret_cast<f32 *>(DEP_BYTES(enemy) + 0x2da8) =
            DEP_READ_FLOAT(enemy, instruction, 2);
        *reinterpret_cast<u32 *>(DEP_BYTES(enemy) + 0x3324) =
            (*reinterpret_cast<u32 *>(DEP_BYTES(enemy) + 0x3324) & 0xffffcfffU) |
            0x1000U;
        *reinterpret_cast<i32 *>(DEP_BYTES(enemy) + 0x2de8) = 0;
        *reinterpret_cast<ZunTimer *>(DEP_BYTES(enemy) + 0x2ddc) = 0;
    }
    else
    {
        StartTimedPolarDisplacement(enemy, instruction, angle);
    }
}

// FUNCTION: th08 0x4224a0
void __fastcall FUN_004224a0(u8 *rawEnemy, void *rawInstruction)
{
    f32 wrappedPlayerX;
    f32 angle;

#define RM_ENEMY (reinterpret_cast<EclOperands::EnemyOverlay *>(rawEnemy))
#define RM_INSTRUCTION (reinterpret_cast<EclRawInstruction *>(rawInstruction))
#define RM_FLOAT(offset) (*reinterpret_cast<f32 *>(rawEnemy + (offset)))
#define RM_READ_INT(index) \
    ((RM_INSTRUCTION->operandFlags & (1U << (index))) \
         ? EclOperands::ResolveInt(RM_ENEMY, *reinterpret_cast<i32 *>(RM_INSTRUCTION->operands + (index) * 4)) \
         : *reinterpret_cast<i32 *>(RM_INSTRUCTION->operands + (index) * 4))
#define RM_READ_FLOAT(index) \
    ((RM_INSTRUCTION->operandFlags & (1U << (index))) \
         ? RM_ENEMY->ResolveFloat(*reinterpret_cast<f32 *>(RM_INSTRUCTION->operands + (index) * 4)) \
         : *reinterpret_cast<f32 *>(RM_INSTRUCTION->operands + (index) * 4))

    if (g_Rng.GetRandomU32InRange(4) != 0)
    {
        if (EclOperands::g_TargetPlayerPosition017D61AC.x < reinterpret_cast<Enemy *>(rawEnemy)->vector2d34.x)
        {
            wrappedPlayerX = EclOperands::g_TargetPlayerPosition017D61AC.x + 384.0f;
            if (reinterpret_cast<Enemy *>(rawEnemy)->vector2d34.x - EclOperands::g_TargetPlayerPosition017D61AC.x <
                wrappedPlayerX - reinterpret_cast<Enemy *>(rawEnemy)->vector2d34.x)
            {
                angle = AddNormalizeAngle(
                    g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                angle = AddNormalizeAngle(
                    g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f, 0.0f);
            }
        }
        else
        {
            wrappedPlayerX = EclOperands::g_TargetPlayerPosition017D61AC.x - 384.0f;
            if (EclOperands::g_TargetPlayerPosition017D61AC.x - reinterpret_cast<Enemy *>(rawEnemy)->vector2d34.x <
                reinterpret_cast<Enemy *>(rawEnemy)->vector2d34.x - wrappedPlayerX)
            {
                angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
            }
            else
            {
                angle = AddNormalizeAngle(
                    g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
        }
    }
    else
    {
        angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    }

    if (reinterpret_cast<Float3 *>(rawEnemy + 0x2d34)->operator float *()[1] <
            RM_FLOAT(0x3344) + 48.0f &&
        angle < 0.0f)
    {
        angle = -angle;
    }
    if (reinterpret_cast<Float3 *>(rawEnemy + 0x2d34)->operator float *()[1] >
            RM_FLOAT(0x334c) - 48.0f &&
        angle > 0.0f)
    {
        angle = -angle;
    }

    if (RM_READ_INT(0) <= 0)
    {
        RM_FLOAT(0x2d94) = angle;
        RM_FLOAT(0x2da8) = RM_READ_FLOAT(2);
        *reinterpret_cast<u32 *>(rawEnemy + 0x3324) =
            (*reinterpret_cast<u32 *>(rawEnemy + 0x3324) & 0xffffcfffU) | 0x1000U;
        *reinterpret_cast<i32 *>(rawEnemy + 0x2de8) = 0;
        *reinterpret_cast<ZunTimer *>(rawEnemy + 0x2ddc) = 0;
    }
    else
    {
        StartTimedPolarDisplacement(RM_ENEMY, RM_INSTRUCTION, angle);
    }

#undef RM_READ_FLOAT
#undef RM_READ_INT
#undef RM_FLOAT
#undef RM_INSTRUCTION
#undef RM_ENEMY
}



// FUNCTION: th08 0x421300
void __fastcall ApplyInterpolationOperation(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    f32 delta;

    delta = DEP_READ_FLOAT(enemy, instruction, 1) -
            DEP_READ_FLOAT(enemy, instruction, 2);
    *EclOperands::ResolveFloatLValue(
        enemy, reinterpret_cast<f32 *>(instruction->operands),
        instruction->operandFlags, 0) =
        delta * DEP_READ_FLOAT(enemy, instruction, 3) +
        DEP_READ_FLOAT(enemy, instruction, 2);
}

struct InterpolationSlot
{
    void *callback;
    ZunTimer timer;
    i32 duration;
    i32 callbackIndex;
    i32 easing;
    f32 parameter0;
    f32 parameter1;
    f32 parameter2;
    f32 parameter3;
    f32 affectedVariable;
};
C_ASSERT(sizeof(InterpolationSlot) == 0x30);

extern void *g_EclInterpolatorCallbacks[];

// FUNCTION: th08 0x4213f0
void __fastcall InstallInterpolationSlot(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    InterpolationSlot *slot;
    i32 i;

    slot = reinterpret_cast<InterpolationSlot *>(
        *reinterpret_cast<u8 **>(DEP_BYTES(enemy) + 0x2ca0) + 0x9c);
    for (i = 0; i < 8; i++, slot++)
    {
        if (slot->callback != NULL &&
            slot->affectedVariable != *reinterpret_cast<f32 *>(instruction->operands))
            continue;

        {
            slot->timer = 0;
            slot->affectedVariable = *reinterpret_cast<f32 *>(instruction->operands);
            slot->duration = DEP_READ_INT(enemy, instruction, 1);
            slot->callbackIndex = DEP_READ_INT(enemy, instruction, 2);
            slot->easing = DEP_READ_INT(enemy, instruction, 3);
            slot->callback = g_EclInterpolatorCallbacks[slot->callbackIndex];
            slot->parameter0 = DEP_READ_FLOAT(enemy, instruction, 4);
            slot->parameter1 = DEP_READ_FLOAT(enemy, instruction, 5);
            slot->parameter2 = DEP_READ_FLOAT(enemy, instruction, 6);
            slot->parameter3 = DEP_READ_FLOAT(enemy, instruction, 7);
            break;
        }
    }
}

// FUNCTION: th08 0x4215f0
EclRawInstruction *__fastcall CompareOperands(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    switch (instruction->opcode)
    {
    case 40:
        if (DEP_READ_INT(enemy, instruction, 0) == DEP_READ_INT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 41:
        if (DEP_READ_FLOAT(enemy, instruction, 0) == DEP_READ_FLOAT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 42:
        if (DEP_READ_INT(enemy, instruction, 0) != DEP_READ_INT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 43:
        if (DEP_READ_FLOAT(enemy, instruction, 0) != DEP_READ_FLOAT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 44:
        if (DEP_READ_INT(enemy, instruction, 0) < DEP_READ_INT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 45:
        if (DEP_READ_FLOAT(enemy, instruction, 0) < DEP_READ_FLOAT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 46:
        if (DEP_READ_INT(enemy, instruction, 0) <= DEP_READ_INT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 47:
        if (DEP_READ_FLOAT(enemy, instruction, 0) <= DEP_READ_FLOAT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 48:
        if (DEP_READ_INT(enemy, instruction, 0) > DEP_READ_INT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 49:
        if (DEP_READ_FLOAT(enemy, instruction, 0) > DEP_READ_FLOAT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 50:
        if (DEP_READ_INT(enemy, instruction, 0) >= DEP_READ_INT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;
    case 51:
        if (DEP_READ_FLOAT(enemy, instruction, 0) >= DEP_READ_FLOAT(enemy, instruction, 1))
            goto compare_success;
        goto compare_failure;

compare_success:
        *reinterpret_cast<i32 *>(
            *reinterpret_cast<u8 **>(DEP_BYTES(enemy) + 0x2ca0) + 0x0c) =
            *reinterpret_cast<i32 *>(instruction->operands + 8);
        return reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) +
            *reinterpret_cast<i32 *>(instruction->operands + 12));

    default:
compare_failure:
        return NULL;
    }
}


// FUNCTION: th08 0x421de0
void __fastcall SetPrimaryAnmScripts(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction,
    i32 script0, i32 script1, i32 script2, i32 script3, i32 script4, i32 script5)
{
    *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x3332) = static_cast<i16>(script0);
    *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x3338) = static_cast<i16>(script1);
    *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x333a) = static_cast<i16>(script2);
    *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x3334) = static_cast<i16>(script3);
    *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x3336) = static_cast<i16>(script4);
    *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x333c) = static_cast<i16>(script5);
    *reinterpret_cast<u8 *>(DEP_BYTES(enemy) + 0x332e) = 0xff;
}


struct EclCallParameterCopy
{
    u32 words[8];
};
C_ASSERT(sizeof(EclCallParameterCopy) == 0x20);
extern EclCallParameterCopy g_EclCallParameters;

// FUNCTION: th08 0x421bd0
void __fastcall CallSubOnEnemy(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction, i32 rawSubId)
{
    (*reinterpret_cast<EnemyEclContext **>(DEP_BYTES(enemy) + 0x2ca0))->currentInstr =
        reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);

    if (((*reinterpret_cast<u32 *>(DEP_BYTES(enemy) + 0x3324) >> 26) & 1) == 0)
    {
        *reinterpret_cast<EnemyEclContext *>(
            *reinterpret_cast<u8 **>(DEP_BYTES(enemy) + 0x2ca4) +
            static_cast<i16>(*reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea)) *
                sizeof(EnemyEclContext)) =
            **reinterpret_cast<EnemyEclContext **>(DEP_BYTES(enemy) + 0x2ca0);
    }

    reinterpret_cast<EclManager *>(0x004ECCB8)->CallEclSub(
        *reinterpret_cast<EnemyEclContext **>(DEP_BYTES(enemy) + 0x2ca0),
        static_cast<i16>(rawSubId));

    *reinterpret_cast<EclCallParameterCopy *>(
        reinterpret_cast<u8 *>(
            *reinterpret_cast<EnemyEclContext **>(DEP_BYTES(enemy) + 0x2ca0)) + 0x70) =
        g_EclCallParameters;

    if (((*reinterpret_cast<u32 *>(DEP_BYTES(enemy) + 0x3324) >> 26) & 1) == 0 &&
        *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea) < 15)
    {
        ++*reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea);
    }
}


// FUNCTION: th08 0x421cb0
int __fastcall PopEclContext(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    i32 contextIndex;

    if (((*reinterpret_cast<u32 *>(DEP_BYTES(enemy) + 0x3324) >> 26) & 1) != 0)
        utils::DebugPrint("error : no Stack Ret\r\n");

    --*reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea);
    if (*reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea) < 0)
    {
        contextIndex =
            *reinterpret_cast<i32 *>(
                reinterpret_cast<u8 *>(
                    *reinterpret_cast<EnemyEclContext **>(DEP_BYTES(enemy) + 0x2ca0)) + 0x220) -
            1;
        if (*reinterpret_cast<void **>(DEP_BYTES(enemy) + 0x3384 + contextIndex * 4) != NULL)
            g_ZunMemory.Free(
                *reinterpret_cast<void **>(DEP_BYTES(enemy) + 0x3384 + contextIndex * 4));
        *reinterpret_cast<void **>(DEP_BYTES(enemy) + 0x3384 + contextIndex * 4) = NULL;
        *reinterpret_cast<u8 **>(DEP_BYTES(enemy) + 0x2ca4) = DEP_BYTES(enemy) + 0x0a20;
        *reinterpret_cast<u8 **>(DEP_BYTES(enemy) + 0x2ca0) = DEP_BYTES(enemy) + 0x07f8;
        *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea) =
            *reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2ce8);
        return 1;
    }

    **reinterpret_cast<EnemyEclContext **>(DEP_BYTES(enemy) + 0x2ca0) =
        *reinterpret_cast<EnemyEclContext *>(
            *reinterpret_cast<u8 **>(DEP_BYTES(enemy) + 0x2ca4) +
            static_cast<i16>(*reinterpret_cast<i16 *>(DEP_BYTES(enemy) + 0x2cea)) *
                sizeof(EnemyEclContext));
    return 0;
}


// FUNCTION: th08 0x421e50
void __fastcall SetExtraAnmScript(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    if (DEP_READ_INT(enemy, instruction, 0) >= 2)
        utils::DebugPrint("error : sub anim overflow\r\n");

    if (DEP_READ_INT(enemy, instruction, 1) >= 0)
    {
        if (((*reinterpret_cast<u32 *>(DEP_BYTES(enemy) + 0x3328) >> 2) & 1) != 0)
        {
            (*reinterpret_cast<AnmLoaded **>(
                reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9dcef0))
                ->SetAndExecuteScriptIdx(
                    reinterpret_cast<AnmVm *>(
                        DEP_BYTES(enemy) + 0x2b0 +
                        DEP_READ_INT(enemy, instruction, 0) * sizeof(AnmVm)),
                    DEP_READ_INT(enemy, instruction, 1));
        }
        else
        {
            (*reinterpret_cast<AnmLoaded **>(
                reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9dceec))
                ->SetAndExecuteScriptIdx(
                    reinterpret_cast<AnmVm *>(
                        DEP_BYTES(enemy) + 0x2b0 +
                        DEP_READ_INT(enemy, instruction, 0) * sizeof(AnmVm)),
                    DEP_READ_INT(enemy, instruction, 1));
        }
    }
    else
    {
        reinterpret_cast<AnmVm *>(
            DEP_BYTES(enemy) + 0x2b0 +
            DEP_READ_INT(enemy, instruction, 0) * sizeof(AnmVm))
            ->scriptIndex = -1;
    }
}


// FUNCTION: th08 0x41efc0
EclOperands::EnemyOverlay *__fastcall FindLinkedChildTail0041EFC0(
    EclOperands::EnemyOverlay *parent)
{
    EclOperands::EnemyOverlay *cursor;

    cursor = parent;
    if (reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(parent)->HasParentChain())
    {
        while (*reinterpret_cast<EclOperands::EnemyOverlay **>(cursor->bytes + 8) != NULL)
            cursor = *reinterpret_cast<EclOperands::EnemyOverlay **>(cursor->bytes + 8);
    }
    return cursor;
}

// FUNCTION: th08 0x41f110
EclOperands::EnemyOverlay *__fastcall SpawnChildStandard0041F110(
    EclOperands::EnemyOverlay *parent, EclRawInstruction *instruction)
{
    EclOperands::EnemyOverlay *child;

    child = reinterpret_cast<EclOperands::EnemyOverlay *>(&g_EnemyManager.enemies[480]);
    if (*reinterpret_cast<i32 *>(parent->bytes + 0x2dfc) > 0 &&
        (((*reinterpret_cast<u32 *>(parent->bytes + 0x3324) >> 10) & 1) == 0))
    {
        Float3 position;
        position.x = DEP_READ_FLOAT(parent, instruction, 1);
        position.y = DEP_READ_FLOAT(parent, instruction, 2);
        position.z = 0.0f;
        child = reinterpret_cast<EclOperands::EnemyOverlay *>(
            g_EnemyManager.SpawnEnemy2(
                *reinterpret_cast<i32 *>(instruction->operands),
                reinterpret_cast<D3DXVECTOR3 *>(&position),
                DEP_READ_INT(parent, instruction, 3),
                DEP_READ_INT(parent, instruction, 4),
                DEP_READ_INT(parent, instruction, 5),
                reinterpret_cast<i32 *>(
                    *reinterpret_cast<u8 **>(parent->bytes + 0x2ca0) + 0x18)));
    }
    else
    {
        *reinterpret_cast<i32 *>(
            reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9dcef8) = 1;
    }
    return child;
}

// FUNCTION: th08 0x41f280
EclOperands::EnemyOverlay *__fastcall SpawnChildAlternate0041F280(
    EclOperands::EnemyOverlay *parent, EclRawInstruction *instruction)
{
    EclOperands::EnemyOverlay *child;

    child = reinterpret_cast<EclOperands::EnemyOverlay *>(&g_EnemyManager.enemies[480]);
    if (*reinterpret_cast<i32 *>(parent->bytes + 0x2dfc) > 0 &&
        (((*reinterpret_cast<u32 *>(parent->bytes + 0x3324) >> 10) & 1) == 0))
    {
        Float3 position;
        position.x = DEP_READ_FLOAT(parent, instruction, 1);
        position.y = DEP_READ_FLOAT(parent, instruction, 2);
        position.z = 0.0f;
        position += *reinterpret_cast<Float3 *>(parent->bytes + 0x2d88);
        child = reinterpret_cast<EclOperands::EnemyOverlay *>(
            g_EnemyManager.SpawnEnemy2(
                *reinterpret_cast<i32 *>(instruction->operands),
                reinterpret_cast<D3DXVECTOR3 *>(&position),
                DEP_READ_INT(parent, instruction, 3),
                DEP_READ_INT(parent, instruction, 4),
                DEP_READ_INT(parent, instruction, 5),
                reinterpret_cast<i32 *>(
                    *reinterpret_cast<u8 **>(parent->bytes + 0x2ca0) + 0x18)));
    }
    else
    {
        *reinterpret_cast<i32 *>(
            reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9dcef8) = 1;
    }
    return child;
}

#undef DEP_READ_FLOAT
#undef DEP_READ_INT
#undef DEP_BYTES

} // namespace EclRunLowProposal

namespace EclRunHighProposal
{

struct SpawnPacketTyped
{
    SpawnPacketTyped();

    i32 type;
    D3DXVECTOR3 position;
    i32 arg4;
    i32 arg5;
    i32 arg6;
};

// FUNCTION: th08 0x41f400
SpawnPacketTyped::SpawnPacketTyped() {}

struct RawInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 unknown08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 operands[1];
};

struct ShotArgs
{
    i16 bulletType;
    i16 color;
    i32 count1;
    i32 count2;
    f32 speed1;
    f32 speed2;
    f32 angle;
    f32 angleStep;
    u32 transformFlags;
};
C_ASSERT(sizeof(ShotArgs) == 0x20);

// FUNCTION: th08 0x422720
void __fastcall DispatchShotInstruction(u8 *enemy, RawInstruction *instruction)
{
    BulletSpawnDescriptor *descriptor;
    ShotArgs *args;
    i32 packed;

    args = reinterpret_cast<ShotArgs *>(instruction->operands);
    descriptor = reinterpret_cast<BulletSpawnDescriptor *>(enemy + 0x2e24);

    if (((args->transformFlags & 0x8000) != 0 &&
         ((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 11) & 1) == 0) ||
        ((args->transformFlags & 0x10000) != 0 &&
         ((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 11) & 1) != 0))
        return;
    if ((*reinterpret_cast<f32 *>(enemy + 0x3350) > 0.0f) &&
        (((*reinterpret_cast<f32 *>(enemy + 0x2d88) - EclOperands::g_TargetPlayerPosition017D61AC.x) *
             (*reinterpret_cast<f32 *>(enemy + 0x2d88) - EclOperands::g_TargetPlayerPosition017D61AC.x) +
         (*reinterpret_cast<f32 *>(enemy + 0x2d8c) - EclOperands::g_TargetPlayerPosition017D61AC.y) *
             (*reinterpret_cast<f32 *>(enemy + 0x2d8c) - EclOperands::g_TargetPlayerPosition017D61AC.y)) <
        *reinterpret_cast<f32 *>(enemy + 0x3350)))
        return;

            descriptor->position =
                *reinterpret_cast<Float3 *>(enemy + 0x2d88) +
                *reinterpret_cast<Float3 *>(enemy + 0x2db8);

            packed = args->bulletType;
            descriptor->bulletType = (instruction->operandFlags & 1)
                                         ? EclOperands::ResolveInt(
                                               reinterpret_cast<EclOperands::EnemyOverlay *>(enemy), packed)
                                         : packed;
            descriptor->aimMode = instruction->opcode - 0x60;
            descriptor->count1 = (instruction->operandFlags & 4)
                                     ? EclOperands::ResolveInt(
                                           reinterpret_cast<EclOperands::EnemyOverlay *>(enemy), args->count1)
                                     : args->count1;
            descriptor->count2 = (instruction->operandFlags & 8)
                                     ? EclOperands::ResolveInt(
                                           reinterpret_cast<EclOperands::EnemyOverlay *>(enemy), args->count2)
                                     : args->count2;
            descriptor->angle = (instruction->operandFlags & 0x40)
                                    ? reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->ResolveFloat(args->angle)
                                    : args->angle;
            descriptor->speed1 = (instruction->operandFlags & 0x10)
                                     ? reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->ResolveFloat(args->speed1)
                                     : args->speed1;
            descriptor->angleStep = (instruction->operandFlags & 0x80)
                                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->ResolveFloat(args->angleStep)
                                        : args->angleStep;
            descriptor->speed2 = (instruction->operandFlags & 0x20)
                                     ? reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->ResolveFloat(args->speed2)
                                     : args->speed2;

            if (!g_Spellcard.IsActive())
            {
                descriptor->count1 += g_GameManager.ScaleIntBasedOnRank(
                    *reinterpret_cast<i16 *>(enemy + 0x2df4),
                    *reinterpret_cast<i16 *>(enemy + 0x2df6));
                if (descriptor->count1 <= 0)
                    descriptor->count1 = 1;
                descriptor->count2 += g_GameManager.ScaleIntBasedOnRank(
                    *reinterpret_cast<i16 *>(enemy + 0x2df8),
                    *reinterpret_cast<i16 *>(enemy + 0x2dfa));
                if (descriptor->count2 <= 0)
                    descriptor->count2 = 1;

                if (descriptor->speed1 != 0.0f)
                {
                    descriptor->speed1 += g_GameManager.ScaleFloatBasedOnRank(
                        *reinterpret_cast<f32 *>(enemy + 0x2dec),
                        *reinterpret_cast<f32 *>(enemy + 0x2df0));
                    if (descriptor->speed1 < 0.3f)
                        descriptor->speed1 = 0.3f;
                }
                descriptor->speed2 += g_GameManager.ScaleFloatBasedOnRank(
                                          *reinterpret_cast<f32 *>(enemy + 0x2dec),
                                          *reinterpret_cast<f32 *>(enemy + 0x2df0)) /
                                      2.0f;
                if (descriptor->speed2 < 0.3f)
                    descriptor->speed2 = 0.3f;
            }

            descriptor->unknown1FA = 0;
            descriptor->transformFlags = args->transformFlags;
            packed = args->color;
            descriptor->color = (instruction->operandFlags & 2)
                                    ? EclOperands::ResolveInt(
                                          reinterpret_cast<EclOperands::EnemyOverlay *>(enemy), packed)
                                    : packed;
            g_BulletManager.FUN_00430e10(descriptor);
}
} // namespace EclRunHighProposal

// FUNCTION: th08 0x423150
void Enemy::FUN_00423150()
{
    i32 direction;
    AnmLoaded *anm;

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x2dfc) > 0)
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3060) > 0)
        {
            this->timer3064++;
            if (this->timer3064 >=
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3060))
            {
                EclRunHighProposal::DispatchShotInstruction(
                    reinterpret_cast<u8 *>(this),
                    reinterpret_cast<EclRunHighProposal::RawInstruction *>(
                        reinterpret_cast<u8 *>(this) + 0x3034));
                this->timer3064 = 0;
            }
        }

        if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x3338) >= 0)
        {
            direction = 0;
            if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 18) & 1) == 0)
            {
                if (this->vector2d4c.x < -0.01f)
                    direction = 1;
                else if (this->vector2d4c.x > 0.01f)
                    direction = 2;
            }
            else
            {
                if (this->vector2d4c.x < -0.01f)
                    direction = 2;
                else if (this->vector2d4c.x > 0.01f)
                    direction = 1;
            }

            if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x332e) != direction)
            {
                anm = (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) >> 2) & 1) != 0)
                          ? *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9dcef0)
                          : *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9dceec);

                switch (direction)
                {
                case 0:
                    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x332e) == 0xff)
                        anm->SetAndExecuteScriptIdx(
                            &this->vm, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x3332));
                    else if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x332e) == 1)
                        anm->SetAndExecuteScriptIdx(
                            &this->vm, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x3334));
                    else
                        anm->SetAndExecuteScriptIdx(
                            &this->vm, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x3336));
                    break;
                case 1:
                    anm->SetAndExecuteScriptIdx(
                        &this->vm, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x3338));
                    break;
                case 2:
                    anm->SetAndExecuteScriptIdx(
                        &this->vm, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x333a));
                    break;
                }
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x332e) = static_cast<u8>(direction);
            }
        }
    }
}

} // namespace th08
