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
void Gui::SetBossLifeBarTarget(f32 value)
{
    this->bossLifeBarTargetSize = value;
}

// FUNCTION: th08 0x4230e0
void Gui::SetBossGaugeSlot(i32 index, f32 start, f32 stop)
{
    this->bossLifeBarSegmentStart[index] = start;
    this->bossLifeBarSegmentStop[index] = stop;
}

// FUNCTION: th08 0x423110
void Gui::SetBossGaugeColor(i32 index, i32 color)
{
    this->bossLifeBarSegmentColor[index] = color;
}

// FUNCTION: th08 0x423130
void Gui::SetBossLifeMarkerCount(i32 count)
{
    this->eclSetLives = count;
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

// FUNCTION: th08 0x4222b0
void __fastcall StartTimedPolarDisplacement(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction, f32 angle)
{
    reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x =
        cosf(angle) * DEP_READ_FLOAT(enemy, instruction, 2) *
        DEP_READ_INT(enemy, instruction, 0);
    reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.y =
        sinf(angle) * DEP_READ_FLOAT(enemy, instruction, 2) *
        DEP_READ_INT(enemy, instruction, 0);
    reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.z = 0.0f;
    reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin =
        reinterpret_cast<Enemy *>(enemy)->worldPosition;
    reinterpret_cast<Enemy *>(enemy)->movementTimer =
        (reinterpret_cast<Enemy *>(enemy)->movementDuration =
             DEP_READ_INT(enemy, instruction, 0));
    reinterpret_cast<EnemyFlag1Bits *>(
        &reinterpret_cast<Enemy *>(enemy)->flags1)->movementEasing =
        DEP_READ_INT(enemy, instruction, 1);
    reinterpret_cast<EnemyFlag1Bits *>(
        &reinterpret_cast<Enemy *>(enemy)->flags1)->movementMode = 2;
}

// FUNCTION: th08 0x422020
void __fastcall BeginBoundaryAwareMove(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    f32 angle;

    if (EclOperands::g_TargetPlayerPosition017D61AC.x <
        reinterpret_cast<Enemy *>(enemy)->position.x)
    {
        angle = AddNormalizeAngle(
            g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
    }

    if (reinterpret_cast<Enemy *>(enemy)->position.operator float *()[0] <
        reinterpret_cast<Enemy *>(enemy)->movementBounds.lower.x + 96.0f)
    {
        if (angle > 1.5707964f)
            angle = 3.1415927f - angle;
        else if (angle < -1.5707964f)
            angle = -3.1415927f - angle;
    }

    if (reinterpret_cast<Enemy *>(enemy)->position.operator float *()[0] >
        reinterpret_cast<Enemy *>(enemy)->movementBounds.upper.x - 96.0f)
    {
        if (angle < 1.5707964f && angle >= 0.0f)
            angle = 3.1415927f - reinterpret_cast<Enemy *>(enemy)->movementAngle;
        else if (angle > -1.5707964f && angle <= 0.0f)
            angle = -3.1415927f - angle;
    }

    if (reinterpret_cast<Enemy *>(enemy)->position.operator float *()[1] <
            reinterpret_cast<Enemy *>(enemy)->movementBounds.lower.y + 48.0f &&
        angle < 0.0f)
    {
        angle = -angle;
    }

    if (reinterpret_cast<Enemy *>(enemy)->position.operator float *()[1] >
            reinterpret_cast<Enemy *>(enemy)->movementBounds.upper.y - 48.0f &&
        angle > 0.0f)
    {
        angle = -angle;
    }

    if (DEP_READ_INT(enemy, instruction, 0) <= 0)
    {
        reinterpret_cast<Enemy *>(enemy)->movementAngle = angle;
        reinterpret_cast<Enemy *>(enemy)->speed =
            DEP_READ_FLOAT(enemy, instruction, 2);
        reinterpret_cast<Enemy *>(enemy)->flags1 =
            (reinterpret_cast<Enemy *>(enemy)->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
            0x1000U;
        reinterpret_cast<Enemy *>(enemy)->movementDuration = 0;
        reinterpret_cast<Enemy *>(enemy)->movementTimer = 0;
    }
    else
    {
        StartTimedPolarDisplacement(enemy, instruction, angle);
    }
}

// FUNCTION: th08 0x4224a0
void __fastcall ApplyRandomBiasedMove(u8 *rawEnemy, void *rawInstruction)
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
        if (EclOperands::g_TargetPlayerPosition017D61AC.x < reinterpret_cast<Enemy *>(rawEnemy)->position.x)
        {
            wrappedPlayerX = EclOperands::g_TargetPlayerPosition017D61AC.x + 384.0f;
            if (reinterpret_cast<Enemy *>(rawEnemy)->position.x - EclOperands::g_TargetPlayerPosition017D61AC.x <
                wrappedPlayerX - reinterpret_cast<Enemy *>(rawEnemy)->position.x)
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
            if (EclOperands::g_TargetPlayerPosition017D61AC.x - reinterpret_cast<Enemy *>(rawEnemy)->position.x <
                reinterpret_cast<Enemy *>(rawEnemy)->position.x - wrappedPlayerX)
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

    if (reinterpret_cast<Enemy *>(rawEnemy)->position.operator float *()[1] <
            reinterpret_cast<Enemy *>(rawEnemy)->movementBounds.lower.y + 48.0f &&
        angle < 0.0f)
    {
        angle = -angle;
    }
    if (reinterpret_cast<Enemy *>(rawEnemy)->position.operator float *()[1] >
            reinterpret_cast<Enemy *>(rawEnemy)->movementBounds.upper.y - 48.0f &&
        angle > 0.0f)
    {
        angle = -angle;
    }

    if (RM_READ_INT(0) <= 0)
    {
        reinterpret_cast<Enemy *>(rawEnemy)->movementAngle = angle;
        reinterpret_cast<Enemy *>(rawEnemy)->speed = RM_READ_FLOAT(2);
        reinterpret_cast<Enemy *>(rawEnemy)->flags1 =
            (reinterpret_cast<Enemy *>(rawEnemy)->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
            0x1000U;
        reinterpret_cast<Enemy *>(rawEnemy)->movementDuration = 0;
        reinterpret_cast<Enemy *>(rawEnemy)->movementTimer = 0;
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

// FUNCTION: th08 0x421120
#pragma var_order(end, start)
void __fastcall InterpolateLinear(
    EclOperands::EnemyOverlay *enemy, InterpolationSlot *slot, f32 t)
{
    f32 start;
    f32 end;

    start = enemy->ResolveFloat(slot->parameter0);
    end = enemy->ResolveFloat(slot->parameter1);
    *EclOperands::ResolveFloatLValue(enemy, &slot->affectedVariable, 0, -1) =
        (end - start) * t + start;
}

// FUNCTION: th08 0x421180
#pragma var_order(weight3, parameter3, weight1, parameter2, parameter1, weight2, weight0, parameter0)
void __fastcall InterpolateHermite(
    EclOperands::EnemyOverlay *enemy, InterpolationSlot *slot, f32 t)
{
    f32 parameter0;
    f32 parameter1;
    f32 parameter2;
    f32 parameter3;
    f32 weight0;
    f32 weight1;
    f32 weight2;
    f32 weight3;

    parameter0 = enemy->ResolveFloat(slot->parameter0);
    parameter1 = enemy->ResolveFloat(slot->parameter1);
    parameter2 = enemy->ResolveFloat(slot->parameter2);
    parameter3 = enemy->ResolveFloat(slot->parameter3);

    weight0 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    weight1 = t * t * (3.0f - 2.0f * t);
    weight2 = (1.0f - t) * (1.0f - t) * t;
    weight3 = (t - 1.0f) * t * t;

    *EclOperands::ResolveFloatLValue(enemy, &slot->affectedVariable, 0, -1) =
        weight0 * parameter0 + weight1 * parameter1 +
        weight2 * parameter2 + weight3 * parameter3;
}


// FUNCTION: th08 0x4213f0
void __fastcall InstallInterpolationSlot(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    InterpolationSlot *slot;
    i32 i;

    slot = reinterpret_cast<InterpolationSlot *>(
        reinterpret_cast<Enemy *>(enemy)->activeEclContext->interpolationSlots);
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
        reinterpret_cast<Enemy *>(enemy)->activeEclContext->time.current =
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
    reinterpret_cast<Enemy *>(enemy)->anmScripts.idleInitial = static_cast<i16>(script0);
    reinterpret_cast<Enemy *>(enemy)->anmScripts.moveLeft = static_cast<i16>(script1);
    reinterpret_cast<Enemy *>(enemy)->anmScripts.moveRight = static_cast<i16>(script2);
    reinterpret_cast<Enemy *>(enemy)->anmScripts.idleFromLeft = static_cast<i16>(script3);
    reinterpret_cast<Enemy *>(enemy)->anmScripts.idleFromRight = static_cast<i16>(script4);
    reinterpret_cast<Enemy *>(enemy)->anmScripts.special = static_cast<i16>(script5);
    reinterpret_cast<Enemy *>(enemy)->anmDirection = 0xff;
}


C_ASSERT(sizeof(EclCallParameterCopy) == 0x20);

// FUNCTION: th08 0x421bd0
void __fastcall CallSubOnEnemy(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction, i32 rawSubId)
{
    reinterpret_cast<Enemy *>(enemy)->activeEclContext->currentInstr =
        reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);

    if (((reinterpret_cast<Enemy *>(enemy)->flags1 >>
          ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) == 0)
    {
        reinterpret_cast<Enemy *>(enemy)->activeEclCallStack[
            reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth] =
            *reinterpret_cast<Enemy *>(enemy)->activeEclContext;
    }

    g_EclManager.CallEclSub(
        reinterpret_cast<Enemy *>(enemy)->activeEclContext,
        static_cast<i16>(rawSubId));

    *reinterpret_cast<EclCallParameterCopy *>(
        &reinterpret_cast<Enemy *>(enemy)->activeEclContext->callParameterInts[0]) =
        g_EclCallParameters;

    if (((reinterpret_cast<Enemy *>(enemy)->flags1 >>
          ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) == 0 &&
        reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth < 15)
    {
        ++reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth;
    }
}


// FUNCTION: th08 0x421cb0
int __fastcall PopEclContext(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    i32 contextIndex;

    if (((reinterpret_cast<Enemy *>(enemy)->flags1 >>
          ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) != 0)
        utils::DebugPrint("error : no Stack Ret\r\n");

    --reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth;
    if (reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth < 0)
    {
        contextIndex = reinterpret_cast<Enemy *>(enemy)->activeEclContext->childContextSlot - 1;
        if (reinterpret_cast<Enemy *>(enemy)->childEclBlocks[contextIndex] != NULL)
            g_ZunMemory.Free(reinterpret_cast<Enemy *>(enemy)->childEclBlocks[contextIndex]);
        reinterpret_cast<Enemy *>(enemy)->childEclBlocks[contextIndex] = NULL;
        reinterpret_cast<Enemy *>(enemy)->activeEclCallStack =
            reinterpret_cast<EnemyEclContext *>(
                &reinterpret_cast<Enemy *>(enemy)->mainEclCallStackStorage[0]);
        reinterpret_cast<Enemy *>(enemy)->activeEclContext =
            reinterpret_cast<EnemyEclContext *>(
                &reinterpret_cast<Enemy *>(enemy)->mainEclContextStorage);
        reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth =
            reinterpret_cast<Enemy *>(enemy)->mainEclCallStackDepth;
        return 1;
    }

    *reinterpret_cast<Enemy *>(enemy)->activeEclContext =
        reinterpret_cast<Enemy *>(enemy)->activeEclCallStack[
            reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth];
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
        if (((reinterpret_cast<Enemy *>(enemy)->flags2 >>
              ENEMY_FLAG2_ALTERNATE_ANM_BANK_SHIFT) & 1) != 0)
        {
            g_EnemyManager.alternateEnemyAnm
                ->SetAndExecuteScriptIdx(
                    &reinterpret_cast<Enemy *>(enemy)
                         ->secondaryVms[DEP_READ_INT(enemy, instruction, 0)],
                    DEP_READ_INT(enemy, instruction, 1));
        }
        else
        {
            g_EnemyManager.enemyAnm
                ->SetAndExecuteScriptIdx(
                    &reinterpret_cast<Enemy *>(enemy)
                         ->secondaryVms[DEP_READ_INT(enemy, instruction, 0)],
                    DEP_READ_INT(enemy, instruction, 1));
        }
    }
    else
    {
        reinterpret_cast<Enemy *>(enemy)
            ->secondaryVms[DEP_READ_INT(enemy, instruction, 0)]
            .scriptIndex = -1;
    }
}


// FUNCTION: th08 0x41efc0
EclOperands::EnemyOverlay *__fastcall FindLinkedChildTail0041EFC0(
    EclOperands::EnemyOverlay *parent)
{
    EclOperands::EnemyOverlay *cursor;

    cursor = parent;
    if (reinterpret_cast<Enemy *>(parent)->HasParentChain())
    {
        while (reinterpret_cast<Enemy *>(cursor)->nextInAttachmentChain != NULL)
            cursor = reinterpret_cast<EclOperands::EnemyOverlay *>(
                reinterpret_cast<Enemy *>(cursor)->nextInAttachmentChain);
    }
    return cursor;
}

// FUNCTION: th08 0x41f110
EclOperands::EnemyOverlay *__fastcall SpawnChildStandard0041F110(
    EclOperands::EnemyOverlay *parent, EclRawInstruction *instruction)
{
    EclOperands::EnemyOverlay *child;

    child = reinterpret_cast<EclOperands::EnemyOverlay *>(&g_EnemyManager.enemies[480]);
    if (reinterpret_cast<Enemy *>(parent)->life > 0 &&
        (((reinterpret_cast<Enemy *>(parent)->flags1 >>
           ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS_SHIFT) & 1) == 0))
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
                reinterpret_cast<Enemy *>(parent)->activeEclContext->intVariables));
    }
    else
    {
        g_EnemyManager.lastSpawnFailed = 1;
    }
    return child;
}

// FUNCTION: th08 0x41f280
EclOperands::EnemyOverlay *__fastcall SpawnChildAlternate0041F280(
    EclOperands::EnemyOverlay *parent, EclRawInstruction *instruction)
{
    EclOperands::EnemyOverlay *child;

    child = reinterpret_cast<EclOperands::EnemyOverlay *>(&g_EnemyManager.enemies[480]);
    if (reinterpret_cast<Enemy *>(parent)->life > 0 &&
        (((reinterpret_cast<Enemy *>(parent)->flags1 >>
           ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS_SHIFT) & 1) == 0))
    {
        Float3 position;
        position.x = DEP_READ_FLOAT(parent, instruction, 1);
        position.y = DEP_READ_FLOAT(parent, instruction, 2);
        position.z = 0.0f;
        position += reinterpret_cast<Enemy *>(parent)->worldPosition;
        child = reinterpret_cast<EclOperands::EnemyOverlay *>(
            g_EnemyManager.SpawnEnemy2(
                *reinterpret_cast<i32 *>(instruction->operands),
                reinterpret_cast<D3DXVECTOR3 *>(&position),
                DEP_READ_INT(parent, instruction, 3),
                DEP_READ_INT(parent, instruction, 4),
                DEP_READ_INT(parent, instruction, 5),
                reinterpret_cast<Enemy *>(parent)->activeEclContext->intVariables));
    }
    else
    {
        g_EnemyManager.lastSpawnFailed = 1;
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
C_ASSERT(offsetof(ShotArgs, count1) == 0x4);
C_ASSERT(offsetof(ShotArgs, speed1) == 0xc);
C_ASSERT(offsetof(ShotArgs, angle) == 0x14);
C_ASSERT(offsetof(ShotArgs, transformFlags) == 0x1c);

// FUNCTION: th08 0x422720
void __fastcall DispatchShotInstruction(u8 *enemy, RawInstruction *instruction)
{
    BulletSpawnDescriptor *descriptor;
    ShotArgs *args;
    i32 packed;

    args = reinterpret_cast<ShotArgs *>(instruction->operands);
    descriptor = &reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor;

    if (((args->transformFlags & BULLET_TRANSFORM_ONLY_WHEN_PLAYER_YOUKAI) != 0 &&
         ((reinterpret_cast<Enemy *>(enemy)->flags1 >>
           ENEMY_FLAG_YOUKAI_ALIGNED_SHIFT) & 1) == 0) ||
        ((args->transformFlags & BULLET_TRANSFORM_ONLY_WHEN_PLAYER_HUMAN) != 0 &&
         ((reinterpret_cast<Enemy *>(enemy)->flags1 >>
           ENEMY_FLAG_YOUKAI_ALIGNED_SHIFT) & 1) != 0))
        return;
    if ((reinterpret_cast<Enemy *>(enemy)->minimumPlayerDistanceSquared > 0.0f) &&
        (((reinterpret_cast<Enemy *>(enemy)->worldPosition.x - EclOperands::g_TargetPlayerPosition017D61AC.x) *
             (reinterpret_cast<Enemy *>(enemy)->worldPosition.x - EclOperands::g_TargetPlayerPosition017D61AC.x) +
         (reinterpret_cast<Enemy *>(enemy)->worldPosition.y - EclOperands::g_TargetPlayerPosition017D61AC.y) *
             (reinterpret_cast<Enemy *>(enemy)->worldPosition.y - EclOperands::g_TargetPlayerPosition017D61AC.y)) <
        reinterpret_cast<Enemy *>(enemy)->minimumPlayerDistanceSquared))
        return;

            descriptor->position =
                reinterpret_cast<Enemy *>(enemy)->worldPosition +
                reinterpret_cast<Enemy *>(enemy)->shootOffset;

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
                    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.count1Low,
                    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.count1High);
                if (descriptor->count1 <= 0)
                    descriptor->count1 = 1;
                descriptor->count2 += g_GameManager.ScaleIntBasedOnRank(
                    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.count2Low,
                    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.count2High);
                if (descriptor->count2 <= 0)
                    descriptor->count2 = 1;

                if (descriptor->speed1 != 0.0f)
                {
                    descriptor->speed1 += g_GameManager.ScaleFloatBasedOnRank(
                        reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedLow,
                        reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedHigh);
                    if (descriptor->speed1 < 0.3f)
                        descriptor->speed1 = 0.3f;
                }
                descriptor->speed2 += g_GameManager.ScaleFloatBasedOnRank(
                                          reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedLow,
                                          reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedHigh) /
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
            g_BulletManager.SpawnBulletPattern(descriptor);
}
} // namespace EclRunHighProposal

// FUNCTION: th08 0x423150
void Enemy::UpdateShotAndAnm()
{
    i32 direction;
    AnmLoaded *anm;

    if (this->life > 0)
    {
        if (this->shootIntervalFrames > 0)
        {
            this->shootIntervalTimer++;
            if (this->shootIntervalTimer >= this->shootIntervalFrames)
            {
                EclRunHighProposal::DispatchShotInstruction(
                    reinterpret_cast<u8 *>(this),
                    reinterpret_cast<EclRunHighProposal::RawInstruction *>(
                        this->pendingShotInstruction));
                this->shootIntervalTimer = 0;
            }
        }

        if (this->anmScripts.moveLeft >= 0)
        {
            direction = 0;
            if (((this->flags1 >> ENEMY_FLAG_MIRROR_MOVEMENT_X_SHIFT) & 1) == 0)
            {
                if (this->velocity.x < -0.01f)
                    direction = 1;
                else if (this->velocity.x > 0.01f)
                    direction = 2;
            }
            else
            {
                if (this->velocity.x < -0.01f)
                    direction = 2;
                else if (this->velocity.x > 0.01f)
                    direction = 1;
            }

            if (this->anmDirection != direction)
            {
                anm = (((this->flags2 >> ENEMY_FLAG2_ALTERNATE_ANM_BANK_SHIFT) & 1) != 0)
                          ? g_EnemyManager.alternateEnemyAnm
                          : g_EnemyManager.enemyAnm;

                switch (direction)
                {
                case 0:
                    if (this->anmDirection == 0xff)
                        anm->SetAndExecuteScriptIdx(
                            &this->vm, this->anmScripts.idleInitial);
                    else if (this->anmDirection == 1)
                        anm->SetAndExecuteScriptIdx(
                            &this->vm, this->anmScripts.idleFromLeft);
                    else
                        anm->SetAndExecuteScriptIdx(
                            &this->vm, this->anmScripts.idleFromRight);
                    break;
                case 1:
                    anm->SetAndExecuteScriptIdx(
                        &this->vm, this->anmScripts.moveLeft);
                    break;
                case 2:
                    anm->SetAndExecuteScriptIdx(
                        &this->vm, this->anmScripts.moveRight);
                    break;
                }
                this->anmDirection = static_cast<u8>(direction);
            }
        }
    }
}

} // namespace th08
