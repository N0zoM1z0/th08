#include "th_pch.h"

#include "Gui.hpp"
#include "Spellcard.hpp"
#include "EnemyManager.hpp"
#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
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
void __fastcall StartEnemySpell(
    Enemy *enemy, EclSpellCardInstructionArgs *instruction)
{
    g_Spellcard.StartSpell(instruction->spellCardNumber,
                           instruction->encodedName,
                           instruction->enemyFace,
                           instruction->bonus, enemy,
                           instruction->encodedOwner,
                           instruction->commentLine1,
                           instruction->commentLine2);
}

// FUNCTION: th08 0x4212e0
void __fastcall EndEnemySpell(
    Enemy *enemy, EclSpellCardInstructionArgs *instruction)
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



namespace EclRunLow
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
    Enemy *enemy, EclRawInstruction *instruction, f32 angle)
{
    enemy->movementInterpolationDelta.x =
        cosf(angle) * DEP_READ_FLOAT(enemy, instruction, 2) *
        DEP_READ_INT(enemy, instruction, 0);
    enemy->movementInterpolationDelta.y =
        sinf(angle) * DEP_READ_FLOAT(enemy, instruction, 2) *
        DEP_READ_INT(enemy, instruction, 0);
    enemy->movementInterpolationDelta.z = 0.0f;
    enemy->movementInterpolationOrigin =
        enemy->worldPosition;
    enemy->movementTimer =
        (enemy->movementDuration =
             DEP_READ_INT(enemy, instruction, 0));
    reinterpret_cast<EnemyFlag1Bits *>(
        &enemy->flags1)->movementEasing =
        DEP_READ_INT(enemy, instruction, 1);
    reinterpret_cast<EnemyFlag1Bits *>(
        &enemy->flags1)->movementMode = 2;
}

// FUNCTION: th08 0x422020
void __fastcall BeginBoundaryAwareMove(
    Enemy *enemy, EclRawInstruction *instruction)
{
    f32 angle;

    if (g_Player.position.x <
        enemy->position.x)
    {
        angle = AddNormalizeAngle(
            g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
    }

    if (enemy->position.operator float *()[0] <
        enemy->movementBounds.lower.x + 96.0f)
    {
        if (angle > 1.5707964f)
            angle = 3.1415927f - angle;
        else if (angle < -1.5707964f)
            angle = -3.1415927f - angle;
    }

    if (enemy->position.operator float *()[0] >
        enemy->movementBounds.upper.x - 96.0f)
    {
        if (angle < 1.5707964f && angle >= 0.0f)
            angle = 3.1415927f - enemy->movementAngle;
        else if (angle > -1.5707964f && angle <= 0.0f)
            angle = -3.1415927f - angle;
    }

    if (enemy->position.operator float *()[1] <
            enemy->movementBounds.lower.y + 48.0f &&
        angle < 0.0f)
    {
        angle = -angle;
    }

    if (enemy->position.operator float *()[1] >
            enemy->movementBounds.upper.y - 48.0f &&
        angle > 0.0f)
    {
        angle = -angle;
    }

    if (DEP_READ_INT(enemy, instruction, 0) <= 0)
    {
        enemy->movementAngle = angle;
        enemy->speed =
            DEP_READ_FLOAT(enemy, instruction, 2);
        enemy->flags1 =
            (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
            0x1000U;
        enemy->movementDuration = 0;
        enemy->movementTimer = 0;
    }
    else
    {
        StartTimedPolarDisplacement(enemy, instruction, angle);
    }
}

// FUNCTION: th08 0x4224a0
void __fastcall ApplyRandomBiasedMove(
    Enemy *enemy, EclRawInstruction *instruction)
{
    f32 wrappedPlayerX;
    f32 angle;

#define RM_ENEMY (enemy)
#define RM_INSTRUCTION (instruction)
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
        if (g_Player.position.x < enemy->position.x)
        {
            wrappedPlayerX = g_Player.position.x + 384.0f;
            if (enemy->position.x - g_Player.position.x <
                wrappedPlayerX - enemy->position.x)
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
            wrappedPlayerX = g_Player.position.x - 384.0f;
            if (g_Player.position.x - enemy->position.x <
                enemy->position.x - wrappedPlayerX)
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

    if (enemy->position.operator float *()[1] <
            enemy->movementBounds.lower.y + 48.0f &&
        angle < 0.0f)
    {
        angle = -angle;
    }
    if (enemy->position.operator float *()[1] >
            enemy->movementBounds.upper.y - 48.0f &&
        angle > 0.0f)
    {
        angle = -angle;
    }

    if (RM_READ_INT(0) <= 0)
    {
        enemy->movementAngle = angle;
        enemy->speed = RM_READ_FLOAT(2);
        enemy->flags1 =
            (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
            0x1000U;
        enemy->movementDuration = 0;
        enemy->movementTimer = 0;
    }
    else
    {
        StartTimedPolarDisplacement(RM_ENEMY, RM_INSTRUCTION, angle);
    }

#undef RM_READ_FLOAT
#undef RM_READ_INT
#undef RM_INSTRUCTION
#undef RM_ENEMY
}



// FUNCTION: th08 0x421300
void __fastcall ApplyInterpolationOperation(
    Enemy *enemy, EclRawInstruction *instruction)
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

extern EnemyEclInterpolatorCallback g_EclInterpolatorCallbacks[];

// FUNCTION: th08 0x421120
#pragma var_order(end, start)
void __fastcall InterpolateLinear(
    Enemy *enemy, EnemyEclInterpolationSlot *slot, f32 t)
{
    f32 start;
    f32 end;

    start = enemy->ResolveFloat(
        slot->parameters[0]);
    end = enemy->ResolveFloat(
        slot->parameters[1]);
    *EclOperands::ResolveFloatLValue(
        enemy,
        &slot->affectedVariable, 0, -1) =
        (end - start) * t + start;
}

// FUNCTION: th08 0x421180
#pragma var_order(weight3, parameter3, weight1, parameter2, parameter1, weight2, weight0, parameter0)
void __fastcall InterpolateHermite(
    Enemy *enemy, EnemyEclInterpolationSlot *slot, f32 t)
{
    f32 parameter0;
    f32 parameter1;
    f32 parameter2;
    f32 parameter3;
    f32 weight0;
    f32 weight1;
    f32 weight2;
    f32 weight3;

    parameter0 = enemy->ResolveFloat(
        slot->parameters[0]);
    parameter1 = enemy->ResolveFloat(
        slot->parameters[1]);
    parameter2 = enemy->ResolveFloat(
        slot->parameters[2]);
    parameter3 = enemy->ResolveFloat(
        slot->parameters[3]);

    weight0 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    weight1 = t * t * (3.0f - 2.0f * t);
    weight2 = (1.0f - t) * (1.0f - t) * t;
    weight3 = (t - 1.0f) * t * t;

    *EclOperands::ResolveFloatLValue(
        enemy,
        &slot->affectedVariable, 0, -1) =
        weight0 * parameter0 + weight1 * parameter1 +
        weight2 * parameter2 + weight3 * parameter3;
}


// FUNCTION: th08 0x4213f0
void __fastcall InstallInterpolationSlot(
    Enemy *enemy, EclRawInstruction *instruction)
{
    EnemyEclInterpolationSlot *slot;
    i32 i;

    slot = enemy->activeEclContext->interpolationSlots;
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
            slot->parameters[0] = DEP_READ_FLOAT(enemy, instruction, 4);
            slot->parameters[1] = DEP_READ_FLOAT(enemy, instruction, 5);
            slot->parameters[2] = DEP_READ_FLOAT(enemy, instruction, 6);
            slot->parameters[3] = DEP_READ_FLOAT(enemy, instruction, 7);
            break;
        }
    }
}

// FUNCTION: th08 0x4215f0
EclRawInstruction *__fastcall CompareOperands(
    Enemy *enemy, EclRawInstruction *instruction)
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
        enemy->activeEclContext->time.current =
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
    Enemy *enemy, EclRawInstruction *instruction,
    i32 script0, i32 script1, i32 script2, i32 script3, i32 script4, i32 script5)
{
    enemy->anmScripts.idleInitial = static_cast<i16>(script0);
    enemy->anmScripts.moveLeft = static_cast<i16>(script1);
    enemy->anmScripts.moveRight = static_cast<i16>(script2);
    enemy->anmScripts.idleFromLeft = static_cast<i16>(script3);
    enemy->anmScripts.idleFromRight = static_cast<i16>(script4);
    enemy->anmScripts.special = static_cast<i16>(script5);
    enemy->anmDirection = 0xff;
}


C_ASSERT(sizeof(EclCallParameterCopy) == 0x20);

// FUNCTION: th08 0x421bd0
void __fastcall CallSubOnEnemy(
    Enemy *enemy, EclRawInstruction *instruction, i32 rawSubId)
{
    enemy->activeEclContext->currentInstr =
        reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);

    if (((enemy->flags1 >>
          ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) == 0)
    {
        enemy->activeEclCallStack[
            enemy->activeEclCallStackDepth] =
            *enemy->activeEclContext;
    }

    g_EclManager.CallEclSub(
        enemy->activeEclContext,
        static_cast<i16>(rawSubId));

    *reinterpret_cast<EclCallParameterCopy *>(
        &enemy->activeEclContext->callParameterInts[0]) =
        g_EclCallParameters;

    if (((enemy->flags1 >>
          ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) == 0 &&
        enemy->activeEclCallStackDepth < 15)
    {
        ++enemy->activeEclCallStackDepth;
    }
}


// FUNCTION: th08 0x421cb0
int __fastcall PopEclContext(
    Enemy *enemy, EclRawInstruction *instruction)
{
    i32 contextIndex;

    if (((enemy->flags1 >>
          ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) != 0)
        utils::DebugPrint("error : no Stack Ret\r\n");

    --enemy->activeEclCallStackDepth;
    if (enemy->activeEclCallStackDepth < 0)
    {
        contextIndex = enemy->activeEclContext->childContextSlot - 1;
        if (enemy->childEclBlocks[contextIndex] != NULL)
            g_ZunMemory.Free(enemy->childEclBlocks[contextIndex]);
        enemy->childEclBlocks[contextIndex] = NULL;
        enemy->activeEclCallStack =
            reinterpret_cast<EnemyEclContext *>(
                &enemy->mainEclCallStackStorage[0]);
        enemy->activeEclContext =
            reinterpret_cast<EnemyEclContext *>(
                &enemy->mainEclContextStorage);
        enemy->activeEclCallStackDepth =
            enemy->mainEclCallStackDepth;
        return 1;
    }

    *enemy->activeEclContext =
        enemy->activeEclCallStack[
            enemy->activeEclCallStackDepth];
    return 0;
}


// FUNCTION: th08 0x421e50
void __fastcall SetExtraAnmScript(
    Enemy *enemy, EclRawInstruction *instruction)
{
    if (DEP_READ_INT(enemy, instruction, 0) >= 2)
        utils::DebugPrint("error : sub anim overflow\r\n");

    if (DEP_READ_INT(enemy, instruction, 1) >= 0)
    {
        if (((enemy->flags2 >>
              ENEMY_FLAG2_ALTERNATE_ANM_BANK_SHIFT) & 1) != 0)
        {
            g_EnemyManager.alternateEnemyAnm
                ->SetAndExecuteScriptIdx(
                    &enemy
                         ->secondaryVms[DEP_READ_INT(enemy, instruction, 0)],
                    DEP_READ_INT(enemy, instruction, 1));
        }
        else
        {
            g_EnemyManager.enemyAnm
                ->SetAndExecuteScriptIdx(
                    &enemy
                         ->secondaryVms[DEP_READ_INT(enemy, instruction, 0)],
                    DEP_READ_INT(enemy, instruction, 1));
        }
    }
    else
    {
        enemy
            ->secondaryVms[DEP_READ_INT(enemy, instruction, 0)]
            .scriptIndex = -1;
    }
}


// FUNCTION: th08 0x41efc0
Enemy *__fastcall FindAttachmentChainTail(
    Enemy *parent)
{
    Enemy *cursor;

    cursor = parent;
    if (parent->HasParentChain())
    {
        while (cursor->nextInAttachmentChain != NULL)
            cursor = cursor->nextInAttachmentChain;
    }
    return cursor;
}

// FUNCTION: th08 0x41f110
Enemy *__fastcall SpawnChildAtScriptPosition(
    Enemy *parent, EclRawInstruction *instruction)
{
    Enemy *child;

    child = &g_EnemyManager.enemies[480];
    if (parent->life > 0 &&
        (((parent->flags1 >>
           ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS_SHIFT) & 1) == 0))
    {
        Float3 position;
        position.x = DEP_READ_FLOAT(parent, instruction, 1);
        position.y = DEP_READ_FLOAT(parent, instruction, 2);
        position.z = 0.0f;
        child = reinterpret_cast<Enemy *>(
            g_EnemyManager.SpawnEnemy2(
                *reinterpret_cast<i32 *>(instruction->operands),
                reinterpret_cast<D3DXVECTOR3 *>(&position),
                DEP_READ_INT(parent, instruction, 3),
                DEP_READ_INT(parent, instruction, 4),
                DEP_READ_INT(parent, instruction, 5),
                parent->activeEclContext->intVariables));
    }
    else
    {
        g_EnemyManager.lastSpawnFailed = 1;
    }
    return child;
}

// FUNCTION: th08 0x41f280
Enemy *__fastcall SpawnChildAtParentOffset(
    Enemy *parent, EclRawInstruction *instruction)
{
    Enemy *child;

    child = &g_EnemyManager.enemies[480];
    if (parent->life > 0 &&
        (((parent->flags1 >>
           ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS_SHIFT) & 1) == 0))
    {
        Float3 position;
        position.x = DEP_READ_FLOAT(parent, instruction, 1);
        position.y = DEP_READ_FLOAT(parent, instruction, 2);
        position.z = 0.0f;
        position += parent->worldPosition;
        child = reinterpret_cast<Enemy *>(
            g_EnemyManager.SpawnEnemy2(
                *reinterpret_cast<i32 *>(instruction->operands),
                reinterpret_cast<D3DXVECTOR3 *>(&position),
                DEP_READ_INT(parent, instruction, 3),
                DEP_READ_INT(parent, instruction, 4),
                DEP_READ_INT(parent, instruction, 5),
                parent->activeEclContext->intVariables));
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

} // namespace EclRunLow

namespace EclRunHigh
{

struct SpawnPacketTyped
{
    SpawnPacketTyped();

    i32 eclSubroutineId;
    D3DXVECTOR3 position;
    i32 life;
    i32 itemDropType;
    i32 score;
};

// FUNCTION: th08 0x41f400
SpawnPacketTyped::SpawnPacketTyped() {}

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
void __fastcall DispatchShotInstruction(
    Enemy *enemy, EclRawInstruction *instruction)
{
    BulletSpawnDescriptor *descriptor;
    ShotArgs *args;
    i32 packed;

    args = reinterpret_cast<ShotArgs *>(instruction->operands);
    descriptor = &enemy->bulletSpawnDescriptor;

    if (((args->transformFlags & BULLET_TRANSFORM_ONLY_WHEN_PLAYER_YOUKAI) != 0 &&
         ((enemy->flags1 >>
           ENEMY_FLAG_YOUKAI_ALIGNED_SHIFT) & 1) == 0) ||
        ((args->transformFlags & BULLET_TRANSFORM_ONLY_WHEN_PLAYER_HUMAN) != 0 &&
         ((enemy->flags1 >>
           ENEMY_FLAG_YOUKAI_ALIGNED_SHIFT) & 1) != 0))
        return;
    if ((enemy->minimumPlayerDistanceSquared > 0.0f) &&
        (((enemy->worldPosition.x - g_Player.position.x) *
             (enemy->worldPosition.x - g_Player.position.x) +
         (enemy->worldPosition.y - g_Player.position.y) *
             (enemy->worldPosition.y - g_Player.position.y)) <
        enemy->minimumPlayerDistanceSquared))
        return;

            descriptor->position =
                enemy->worldPosition +
                enemy->shootOffset;

            packed = args->bulletType;
            descriptor->bulletType = (instruction->operandFlags & 1)
                                         ? EclOperands::ResolveInt(
                                               enemy, packed)
                                         : packed;
            descriptor->aimMode = instruction->opcode - 0x60;
            descriptor->count1 = (instruction->operandFlags & 4)
                                     ? EclOperands::ResolveInt(
                                           enemy, args->count1)
                                     : args->count1;
            descriptor->count2 = (instruction->operandFlags & 8)
                                     ? EclOperands::ResolveInt(
                                           enemy, args->count2)
                                     : args->count2;
            descriptor->angle = (instruction->operandFlags & 0x40)
                                    ? enemy->ResolveFloat(args->angle)
                                    : args->angle;
            descriptor->speed1 = (instruction->operandFlags & 0x10)
                                     ? enemy->ResolveFloat(args->speed1)
                                     : args->speed1;
            descriptor->angleStep = (instruction->operandFlags & 0x80)
                                        ? enemy->ResolveFloat(args->angleStep)
                                        : args->angleStep;
            descriptor->speed2 = (instruction->operandFlags & 0x20)
                                     ? enemy->ResolveFloat(args->speed2)
                                     : args->speed2;

            if (!g_Spellcard.IsActive())
            {
                descriptor->count1 += g_GameManager.ScaleIntBasedOnRank(
                    enemy->bulletRankInfluence.count1Low,
                    enemy->bulletRankInfluence.count1High);
                if (descriptor->count1 <= 0)
                    descriptor->count1 = 1;
                descriptor->count2 += g_GameManager.ScaleIntBasedOnRank(
                    enemy->bulletRankInfluence.count2Low,
                    enemy->bulletRankInfluence.count2High);
                if (descriptor->count2 <= 0)
                    descriptor->count2 = 1;

                if (descriptor->speed1 != 0.0f)
                {
                    descriptor->speed1 += g_GameManager.ScaleFloatBasedOnRank(
                        enemy->bulletRankInfluence.speedLow,
                        enemy->bulletRankInfluence.speedHigh);
                    if (descriptor->speed1 < 0.3f)
                        descriptor->speed1 = 0.3f;
                }
                descriptor->speed2 += g_GameManager.ScaleFloatBasedOnRank(
                                          enemy->bulletRankInfluence.speedLow,
                                          enemy->bulletRankInfluence.speedHigh) /
                                      2.0f;
                if (descriptor->speed2 < 0.3f)
                    descriptor->speed2 = 0.3f;
            }

            descriptor->unconsumedWord1FA = 0;
            descriptor->transformFlags = args->transformFlags;
            packed = args->color;
            descriptor->color = (instruction->operandFlags & 2)
                                    ? EclOperands::ResolveInt(
                                          enemy, packed)
                                    : packed;
            g_BulletManager.SpawnBulletPattern(descriptor);
}
} // namespace EclRunHigh

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
                EclRunHigh::DispatchShotInstruction(
                    this,
                    reinterpret_cast<EclRawInstruction *>(
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
