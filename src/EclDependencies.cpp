#include "th_pch.h"

#include "Gui.hpp"
#include "Spellcard.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "EclOperands.hpp"
#include "EclManager.hpp"

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

#undef DEP_READ_FLOAT
#undef DEP_READ_INT
#undef DEP_BYTES

} // namespace EclRunLowProposal

namespace EclRunHighProposal
{
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
