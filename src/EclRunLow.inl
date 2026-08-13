// TH08 1.00d RunEcl low-opcode proposal (opcodes 1..92).
//
// Evidence boundary:
//   - dispatcher: 0x004184B0, jump at 0x00418623
//   - comparison helper: 0x004215F0
//   - target executable: resources/th08.exe (hash-attested by verify-target.py)
//   - .analysis/ecl-decompile.c is used only as a navigation aid
//
// The dispatch table indexes (opcode - 1).  Consequently Ghidra's `case 0`
// is target opcode 1.  Opcodes 3, 84, and 85 all land on the ordinary
// instruction-advance path in the target.
//
// This file is deliberately not wired into EclManager::RunEcl yet.  It is a
// self-contained integration proposal: the owner lane can provide a Services
// adapter for already-recovered engine helpers, then map the control result to
// its outer context loop.  The assumed control contract is:
//
//   LOW_ADVANCE             advance by instruction->nextOffset;
//   LOW_REDISPATCH          use nextInstruction immediately;
//   LOW_RESTART_RUN_LOOP    restart at the active ECL context;
//   LOW_SELECT_NEXT_CONTEXT resume the outer four-context scan;
//   LOW_RETURN_MINUS_ONE    return -1 from RunEcl;
//   LOW_NOT_HANDLED         opcode is outside 1..92.
//
// Every flagged scalar operand in this switch is resolved through
// EclOperands.  Raw operands are used only where the target demonstrably does
// so: jump times/displacements, opcode 36's slot selector, opcode 52's sub-id,
// and opcode 88's remote sub-id.

#pragma once

#include "EclManager.hpp"
#include "EclOperands.hpp"

namespace th08
{
namespace EclRunLowProposal
{

enum LowControl
{
    LOW_NOT_HANDLED,
    LOW_ADVANCE,
    LOW_REDISPATCH,
    LOW_RESTART_RUN_LOOP,
    LOW_SELECT_NEXT_CONTEXT,
    LOW_RETURN_MINUS_ONE
};

struct LowResult
{
    LowControl control;
    EclRawInstruction *nextInstruction;

    LowResult(LowControl control_, EclRawInstruction *nextInstruction_ = 0)
        : control(control_), nextInstruction(nextInstruction_)
    {
    }
};

enum ChildConstructor
{
    CHILD_STANDARD_41F110,
    CHILD_ALTERNATE_41F280
};

// Adapter boundary for target helpers whose owned layouts live outside the
// RunEcl lane.  Implementations must preserve the stated target behavior.
struct Services
{
    virtual ~Services() {}

    virtual u32 RandomU32() = 0; // target RNG call at 0x0043ECC0
    virtual f32 Remainder(f32 lhs, f32 rhs) = 0; // 0x0041F090
    virtual f32 Sin(f32 angle) = 0;              // 0x00409060
    virtual f32 Cos(f32 angle) = 0;              // 0x00408D40
    virtual f32 Atan2(f32 y, f32 x) = 0;         // 0x0040C7B0
    virtual f32 Sqrt(f32 value) = 0;              // 0x0040B440
    virtual f32 NormalizeAngle(f32 angle, f32 base) = 0; // 0x0043EDB0

    // ZunTimer::operator=(i32), target 0x004065F0.
    virtual void AssignTimer(void *timer, i32 value) = 0;

    // Opcode 36 / 0x004213F0.  selector is deliberately raw.  The target
    // installs one of eight 0x30-byte interpolation records at context+0x9C.
    virtual void InstallInterpolationSlot(EclOperands::EnemyOverlay *enemy,
                                          f32 selector,
                                          i32 duration,
                                          i32 callbackIndex,
                                          i32 easingMode,
                                          f32 value0,
                                          f32 value1,
                                          f32 value2,
                                          f32 value3) = 0;

    // Opcode 53 / 0x00421CB0.  Returns nonzero when the primary context was
    // restored, zero when a saved nested context was popped in place.
    virtual i32 PopEclContext(EclOperands::EnemyOverlay *enemy,
                              const EclRawInstruction *instruction) = 0;

    virtual void SetPrimaryAnmScript(EclOperands::EnemyOverlay *enemy, i32 script) = 0;
    // 0x00421DE0 stores the six low words at +3332, +3338, +333A, +3334,
    // +3336, +333C respectively, then writes 0xFF at +332E.
    virtual void SetPrimaryAnmScripts(EclOperands::EnemyOverlay *enemy,
                                      i32 script0, i32 script1, i32 script2,
                                      i32 script3, i32 script4, i32 script5) = 0;
    virtual void SetExtraAnmScript(EclOperands::EnemyOverlay *enemy,
                                   i32 slot, i32 script) = 0; // 0x00421E50
    virtual void RefreshBaseVector(EclOperands::EnemyOverlay *enemy) = 0; // 0x0042C180

    // Opcode 64 / 0x00420F40: begin an interpolated move toward (x,y).
    // The helper snapshots target/current vectors, resets the movement timer,
    // installs (mode & 7), selects movement state 2, and honors mirror bit 18.
    virtual void BeginPointMove(EclOperands::EnemyOverlay *enemy,
                                i32 duration, i32 mode, f32 x, f32 y) = 0;

    // First half of opcode 67 / 0x00422020.  This computes only the
    // boundary-aware random angle; the switch below owns every ECL operand
    // resolution and the state transition selected by operand 0.
    virtual f32 ChooseBoundaryAwareAngle(EclOperands::EnemyOverlay *enemy) = 0;
    virtual f32 AngleToPlayer(const void *vector) = 0;                       // 0x0044C1B0

    // Global active-enemy table at 0x00F54CC0.
    virtual EclOperands::EnemyOverlay *EnemyByIndex(i32 index) = 0;

    // 0x00421BD0.  It advances targetEnemy's current instruction, snapshots
    // its context when required, and invokes CallEclSub with rawSubId.
    virtual void CallSubOnEnemy(EclOperands::EnemyOverlay *targetEnemy,
                                i16 rawSubId) = 0;

    // Opcodes 90..92.  The implementation owns the 0x0041EFC0 linked-list
    // tail lookup, the selected constructor, disabled-spawn check, child flag
    // initialization, ANM creation, parent/child links, parent child-count,
    // and the unconditional sound request.  inheritParentPosition additionally
    // copies +0x2D34 to child +0x2D40, rebuilds child +0x2D88, and sets bit 9.
    virtual void SpawnLinkedChild(EclOperands::EnemyOverlay *parent,
                                  const EclRawInstruction *instruction,
                                  ChildConstructor constructor,
                                  i32 inheritParentPosition) = 0;
};

inline u8 *Bytes(EclOperands::EnemyOverlay *enemy)
{
    return reinterpret_cast<u8 *>(enemy);
}

inline u32 &U32At(EclOperands::EnemyOverlay *enemy, i32 offset)
{
    return *reinterpret_cast<u32 *>(Bytes(enemy) + offset);
}

inline i32 &I32At(EclOperands::EnemyOverlay *enemy, i32 offset)
{
    return *reinterpret_cast<i32 *>(Bytes(enemy) + offset);
}

inline i16 &I16At(EclOperands::EnemyOverlay *enemy, i32 offset)
{
    return *reinterpret_cast<i16 *>(Bytes(enemy) + offset);
}

inline f32 &F32At(EclOperands::EnemyOverlay *enemy, i32 offset)
{
    return *reinterpret_cast<f32 *>(Bytes(enemy) + offset);
}

inline void *&PointerAt(EclOperands::EnemyOverlay *enemy, i32 offset)
{
    return *reinterpret_cast<void **>(Bytes(enemy) + offset);
}

inline i32 &RawInt(EclRawInstruction *instruction, i32 index)
{
    return *reinterpret_cast<i32 *>(instruction->operands + index * 4);
}

inline f32 &RawFloat(EclRawInstruction *instruction, i32 index)
{
    return *reinterpret_cast<f32 *>(instruction->operands + index * 4);
}

inline i32 ReadInt(EclOperands::EnemyOverlay *enemy,
                   EclRawInstruction *instruction, i32 index)
{
    const i32 raw = RawInt(instruction, index);
    return (instruction->operandFlags & (1U << index))
               ? EclOperands::ResolveInt(enemy, raw)
               : raw;
}

inline f32 ReadFloat(EclOperands::EnemyOverlay *enemy,
                     EclRawInstruction *instruction, i32 index)
{
    const f32 raw = RawFloat(instruction, index);
    return (instruction->operandFlags & (1U << index))
               ? enemy->ResolveFloat(raw)
               : raw;
}

inline i32 *WriteInt(EclOperands::EnemyOverlay *enemy,
                     EclRawInstruction *instruction, i32 index)
{
    return EclOperands::ResolveIntLValue(enemy, &RawInt(instruction, index),
                                         instruction->operandFlags, index);
}

inline f32 *WriteFloat(EclOperands::EnemyOverlay *enemy,
                       EclRawInstruction *instruction, i32 index)
{
    return EclOperands::ResolveFloatLValue(enemy, &RawFloat(instruction, index),
                                           instruction->operandFlags, index);
}

inline LowResult Jump(EnemyEclContext *context, EclRawInstruction *instruction,
                      i32 timeOperand, i32 displacementOperand)
{
    context->time.current = RawInt(instruction, timeOperand);
    return LowResult(LOW_REDISPATCH,
                     reinterpret_cast<EclRawInstruction *>(
                         reinterpret_cast<u8 *>(instruction) +
                         RawInt(instruction, displacementOperand)));
}

inline LowResult ConditionalJump(EnemyEclContext *context,
                                 EclRawInstruction *instruction)
{
    return Jump(context, instruction, 2, 3);
}

inline void SetMovementState1(EclOperands::EnemyOverlay *enemy)
{
    U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & 0xFFFFCFFFU) | 0x1000U;
}

inline void ResetMovementTimer(EclOperands::EnemyOverlay *enemy,
                               Services &services, i32 duration)
{
    I32At(enemy, 0x2DE8) = duration;
    services.AssignTimer(Bytes(enemy) + 0x2DDC, duration);
}

inline void BeginTimedMoveAtAngle(EclOperands::EnemyOverlay *enemy,
                                  EclRawInstruction *instruction,
                                  Services &services,
                                  f32 angle,
                                  i32 speedOperand)
{
    // 0x00420D10 and 0x004222B0 deliberately resolve random-capable speed and
    // duration operands more than once.  Keep those calls separate.
    F32At(enemy, 0x2DC4) = services.Cos(angle) *
                           ReadFloat(enemy, instruction, speedOperand) *
                           ReadInt(enemy, instruction, 0);
    F32At(enemy, 0x2DC8) = services.Sin(angle) *
                           ReadFloat(enemy, instruction, speedOperand) *
                           ReadInt(enemy, instruction, 0);
    F32At(enemy, 0x2DCC) = 0.0f;
    F32At(enemy, 0x2DD0) = F32At(enemy, 0x2D88);
    F32At(enemy, 0x2DD4) = F32At(enemy, 0x2D8C);
    F32At(enemy, 0x2DD8) = F32At(enemy, 0x2D90);
    ResetMovementTimer(enemy, services, ReadInt(enemy, instruction, 0));

    const i32 mode = ReadInt(enemy, instruction, 1);
    U32At(enemy, 0x3324) =
        (U32At(enemy, 0x3324) & 0xFFFE3FFFU) | ((mode & 7) << 14);
    U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & 0xFFFFCFFFU) | 0x2000U;
    if (U32At(enemy, 0x3324) & 0x40000U)
        F32At(enemy, 0x2DC4) = -F32At(enemy, 0x2DC4);
}

inline void BeginTimedMove(EclOperands::EnemyOverlay *enemy,
                           EclRawInstruction *instruction,
                           Services &services)
{
    BeginTimedMoveAtAngle(enemy, instruction, services,
                          services.NormalizeAngle(ReadFloat(enemy, instruction, 2), 0.0f),
                          3);
}

inline LowResult Dispatch(EclOperands::EnemyOverlay *enemy,
                          EnemyEclContext *context,
                          EclRawInstruction *instruction,
                          Services &services)
{
    i32 lhsInt;
    i32 rhsInt;
    f32 lhsFloat;
    f32 rhsFloat;
    f32 angle;
    f32 magnitude;
    i32 mode;
    EclOperands::EnemyOverlay *targetEnemy;

    switch (instruction->opcode)
    {
    case 1:
        return LowResult(LOW_RETURN_MINUS_ONE);

    case 2:
        services.AssignTimer(&context->secondaryTime, ReadInt(enemy, instruction, 0));
        break;

    case 3:
        break; // dispatch-table entry is the ordinary advance path

    case 4:
        return Jump(context, instruction, 0, 1);

    case 5:
        --*WriteInt(enemy, instruction, 2);
        if (ReadInt(enemy, instruction, 2) > 0)
            return Jump(context, instruction, 0, 1);
        break;

    case 6:
        *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1);
        break;
    case 7:
        *WriteFloat(enemy, instruction, 0) = ReadFloat(enemy, instruction, 1);
        break;
    case 8:
        *WriteInt(enemy, instruction, 0) =
            (services.RandomU32() & 1U ? 1 : -1) * ReadInt(enemy, instruction, 1);
        break;
    case 9:
        *WriteFloat(enemy, instruction, 0) =
            (services.RandomU32() & 1U ? 1.0f : -1.0f) * ReadFloat(enemy, instruction, 1);
        break;

    case 10: *WriteInt(enemy, instruction, 0) += ReadInt(enemy, instruction, 1); break;
    case 11: *WriteInt(enemy, instruction, 0) -= ReadInt(enemy, instruction, 1); break;
    case 12: *WriteInt(enemy, instruction, 0) *= ReadInt(enemy, instruction, 1); break;
    case 13: *WriteInt(enemy, instruction, 0) /= ReadInt(enemy, instruction, 1); break;
    case 14: *WriteInt(enemy, instruction, 0) %= ReadInt(enemy, instruction, 1); break;
    case 15: *WriteFloat(enemy, instruction, 0) += ReadFloat(enemy, instruction, 1); break;
    case 16: *WriteFloat(enemy, instruction, 0) -= ReadFloat(enemy, instruction, 1); break;
    case 17: *WriteFloat(enemy, instruction, 0) *= ReadFloat(enemy, instruction, 1); break;
    case 18: *WriteFloat(enemy, instruction, 0) /= ReadFloat(enemy, instruction, 1); break;
    case 19:
        *WriteFloat(enemy, instruction, 0) =
            services.Remainder(ReadFloat(enemy, instruction, 0),
                               ReadFloat(enemy, instruction, 1));
        break;

    case 20: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) + ReadInt(enemy, instruction, 2); break;
    case 21: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) - ReadInt(enemy, instruction, 2); break;
    case 22: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) * ReadInt(enemy, instruction, 2); break;
    case 23: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) / ReadInt(enemy, instruction, 2); break;
    case 24: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) % ReadInt(enemy, instruction, 2); break;
    case 25: *WriteFloat(enemy, instruction, 0) = ReadFloat(enemy, instruction, 1) + ReadFloat(enemy, instruction, 2); break;
    case 26: *WriteFloat(enemy, instruction, 0) = ReadFloat(enemy, instruction, 1) - ReadFloat(enemy, instruction, 2); break;
    case 27: *WriteFloat(enemy, instruction, 0) = ReadFloat(enemy, instruction, 1) * ReadFloat(enemy, instruction, 2); break;
    case 28: *WriteFloat(enemy, instruction, 0) = ReadFloat(enemy, instruction, 1) / ReadFloat(enemy, instruction, 2); break;
    case 29:
        *WriteFloat(enemy, instruction, 0) =
            services.Remainder(ReadFloat(enemy, instruction, 1),
                               ReadFloat(enemy, instruction, 2));
        break;
    case 30: ++*WriteInt(enemy, instruction, 0); break;
    case 31: --*WriteInt(enemy, instruction, 0); break;
    case 32: *WriteFloat(enemy, instruction, 0) = services.Sin(ReadFloat(enemy, instruction, 1)); break;
    case 33: *WriteFloat(enemy, instruction, 0) = services.Cos(ReadFloat(enemy, instruction, 1)); break;
    case 34:
        *WriteFloat(enemy, instruction, 0) =
            services.Atan2(ReadFloat(enemy, instruction, 4) - ReadFloat(enemy, instruction, 2),
                           ReadFloat(enemy, instruction, 3) - ReadFloat(enemy, instruction, 1));
        break;

    case 35:
        lhsFloat = ReadFloat(enemy, instruction, 1);
        rhsFloat = ReadFloat(enemy, instruction, 2);
        magnitude = ReadFloat(enemy, instruction, 3);
        // The target resolves operand 2 a second time for the final add.
        *WriteFloat(enemy, instruction, 0) =
            (lhsFloat - rhsFloat) * magnitude + ReadFloat(enemy, instruction, 2);
        break;

    case 36:
        services.InstallInterpolationSlot(enemy, RawFloat(instruction, 0),
                                          ReadInt(enemy, instruction, 1),
                                          ReadInt(enemy, instruction, 2),
                                          ReadInt(enemy, instruction, 3),
                                          ReadFloat(enemy, instruction, 4),
                                          ReadFloat(enemy, instruction, 5),
                                          ReadFloat(enemy, instruction, 6),
                                          ReadFloat(enemy, instruction, 7));
        break;

    case 37:
        *WriteFloat(enemy, instruction, 0) =
            services.NormalizeAngle(ReadFloat(enemy, instruction, 0), 0.0f);
        break;

    case 38:
        angle = services.NormalizeAngle(ReadFloat(enemy, instruction, 2), 0.0f);
        magnitude = ReadFloat(enemy, instruction, 3);
        *WriteFloat(enemy, instruction, 0) = services.Cos(angle) * magnitude;
        *WriteFloat(enemy, instruction, 1) = services.Sin(angle) * magnitude;
        break;

    case 39:
        lhsFloat = ReadFloat(enemy, instruction, 1) - ReadFloat(enemy, instruction, 3);
        rhsFloat = ReadFloat(enemy, instruction, 2) - ReadFloat(enemy, instruction, 4);
        *WriteFloat(enemy, instruction, 0) =
            services.Sqrt(lhsFloat * lhsFloat + rhsFloat * rhsFloat);
        break;

    // 0x004215F0 comparison order is ==, !=, <, <=, >, >=, with integer
    // and float variants interleaved.  Successful branches use raw operands
    // 2 and 3 for the replacement time and signed bytecode displacement.
    case 40:
        lhsInt = ReadInt(enemy, instruction, 0); rhsInt = ReadInt(enemy, instruction, 1);
        if (lhsInt == rhsInt) return ConditionalJump(context, instruction); break;
    case 41:
        lhsFloat = ReadFloat(enemy, instruction, 0); rhsFloat = ReadFloat(enemy, instruction, 1);
        if (lhsFloat == rhsFloat) return ConditionalJump(context, instruction); break;
    case 42:
        lhsInt = ReadInt(enemy, instruction, 0); rhsInt = ReadInt(enemy, instruction, 1);
        if (lhsInt != rhsInt) return ConditionalJump(context, instruction); break;
    case 43:
        lhsFloat = ReadFloat(enemy, instruction, 0); rhsFloat = ReadFloat(enemy, instruction, 1);
        if (lhsFloat != rhsFloat) return ConditionalJump(context, instruction); break;
    case 44:
        lhsInt = ReadInt(enemy, instruction, 0); rhsInt = ReadInt(enemy, instruction, 1);
        if (lhsInt < rhsInt) return ConditionalJump(context, instruction); break;
    case 45:
        lhsFloat = ReadFloat(enemy, instruction, 0); rhsFloat = ReadFloat(enemy, instruction, 1);
        if (lhsFloat < rhsFloat) return ConditionalJump(context, instruction); break;
    case 46:
        lhsInt = ReadInt(enemy, instruction, 0); rhsInt = ReadInt(enemy, instruction, 1);
        if (lhsInt <= rhsInt) return ConditionalJump(context, instruction); break;
    case 47:
        lhsFloat = ReadFloat(enemy, instruction, 0); rhsFloat = ReadFloat(enemy, instruction, 1);
        if (lhsFloat <= rhsFloat) return ConditionalJump(context, instruction); break;
    case 48:
        lhsInt = ReadInt(enemy, instruction, 0); rhsInt = ReadInt(enemy, instruction, 1);
        if (lhsInt > rhsInt) return ConditionalJump(context, instruction); break;
    case 49:
        lhsFloat = ReadFloat(enemy, instruction, 0); rhsFloat = ReadFloat(enemy, instruction, 1);
        if (lhsFloat > rhsFloat) return ConditionalJump(context, instruction); break;
    case 50:
        lhsInt = ReadInt(enemy, instruction, 0); rhsInt = ReadInt(enemy, instruction, 1);
        if (lhsInt >= rhsInt) return ConditionalJump(context, instruction); break;
    case 51:
        lhsFloat = ReadFloat(enemy, instruction, 0); rhsFloat = ReadFloat(enemy, instruction, 1);
        if (lhsFloat >= rhsFloat) return ConditionalJump(context, instruction); break;

    case 52:
        services.CallSubOnEnemy(enemy, static_cast<i16>(RawInt(instruction, 0)));
        return LowResult(LOW_RESTART_RUN_LOOP);

    case 53:
        return LowResult(services.PopEclContext(enemy, instruction)
                             ? LOW_SELECT_NEXT_CONTEXT
                             : LOW_RESTART_RUN_LOOP);

    case 54:
        services.SetPrimaryAnmScript(enemy, ReadInt(enemy, instruction, 0));
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 55:
        lhsInt = ReadInt(enemy, instruction, 0);
        services.SetPrimaryAnmScripts(enemy, lhsInt, lhsInt + 1, lhsInt + 2,
                                     lhsInt + 3, lhsInt + 4, lhsInt + 5);
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 56:
        services.SetPrimaryAnmScripts(enemy,
                                     ReadInt(enemy, instruction, 0),
                                     ReadInt(enemy, instruction, 1),
                                     ReadInt(enemy, instruction, 2),
                                     ReadInt(enemy, instruction, 3),
                                     ReadInt(enemy, instruction, 4),
                                     ReadInt(enemy, instruction, 5));
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 57:
        services.SetExtraAnmScript(enemy, ReadInt(enemy, instruction, 0),
                                  ReadInt(enemy, instruction, 1));
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 58:
        services.SetPrimaryAnmScript(enemy, ReadInt(enemy, instruction, 0));
        U32At(enemy, 0x3328) |= 4U;
        break;
    case 59:
        lhsInt = ReadInt(enemy, instruction, 0);
        services.SetPrimaryAnmScripts(enemy, lhsInt, lhsInt + 1, lhsInt + 2,
                                     lhsInt + 3, lhsInt + 4, lhsInt + 5);
        U32At(enemy, 0x3328) |= 4U;
        break;
    case 60:
        services.SetPrimaryAnmScripts(enemy,
                                     ReadInt(enemy, instruction, 0),
                                     ReadInt(enemy, instruction, 1),
                                     ReadInt(enemy, instruction, 2),
                                     ReadInt(enemy, instruction, 3),
                                     ReadInt(enemy, instruction, 4),
                                     ReadInt(enemy, instruction, 5));
        U32At(enemy, 0x3328) |= 4U;
        break;
    case 61:
        U32At(enemy, 0x3328) |= 4U;
        services.SetExtraAnmScript(enemy, ReadInt(enemy, instruction, 0),
                                  ReadInt(enemy, instruction, 1));
        break;
    case 62:
        services.SetPrimaryAnmScript(enemy, I16At(enemy, 0x333C));
        break;

    case 63:
        F32At(enemy, 0x2D34) = ReadFloat(enemy, instruction, 0);
        F32At(enemy, 0x2D38) = ReadFloat(enemy, instruction, 1);
        F32At(enemy, 0x2D3C) = 0.0f;
        services.RefreshBaseVector(enemy);
        break;
    case 64:
        services.BeginPointMove(enemy,
                                ReadInt(enemy, instruction, 0),
                                ReadInt(enemy, instruction, 1),
                                ReadFloat(enemy, instruction, 2),
                                ReadFloat(enemy, instruction, 3));
        break;
    case 65:
        F32At(enemy, 0x2D94) = services.NormalizeAngle(ReadFloat(enemy, instruction, 0), 0.0f);
        F32At(enemy, 0x2DA8) = ReadFloat(enemy, instruction, 1);
        SetMovementState1(enemy);
        ResetMovementTimer(enemy, services, 0);
        break;
    case 66:
        mode = ReadInt(enemy, instruction, 0);
        if (mode < 1)
        {
            F32At(enemy, 0x2D94) = services.NormalizeAngle(ReadFloat(enemy, instruction, 2), 0.0f);
            F32At(enemy, 0x2DA8) = ReadFloat(enemy, instruction, 3);
            SetMovementState1(enemy);
            ResetMovementTimer(enemy, services, 0);
        }
        else
        {
            BeginTimedMove(enemy, instruction, services);
        }
        break;
    case 67:
        angle = services.ChooseBoundaryAwareAngle(enemy);
        mode = ReadInt(enemy, instruction, 0);
        if (mode <= 0)
        {
            F32At(enemy, 0x2D94) = angle;
            F32At(enemy, 0x2DA8) = ReadFloat(enemy, instruction, 2);
            SetMovementState1(enemy);
            ResetMovementTimer(enemy, services, 0);
        }
        else
        {
            BeginTimedMoveAtAngle(enemy, instruction, services, angle, 2);
        }
        break;
    case 68:
        F32At(enemy, 0x2D94) =
            services.NormalizeAngle(ReadFloat(enemy, instruction, 0),
                                    services.AngleToPlayer(Bytes(enemy) + 0x2D34));
        F32At(enemy, 0x2DA8) = ReadFloat(enemy, instruction, 1);
        break;
    case 69:
        mode = ReadInt(enemy, instruction, 0);
        if (mode < 1)
        {
            F32At(enemy, 0x2D94) =
                services.NormalizeAngle(ReadFloat(enemy, instruction, 2),
                                        services.AngleToPlayer(Bytes(enemy) + 0x2D34));
            F32At(enemy, 0x2DA8) = ReadFloat(enemy, instruction, 3);
            SetMovementState1(enemy);
            // The target resolves operand 0 again before timer assignment.
            ResetMovementTimer(enemy, services, ReadInt(enemy, instruction, 0));
        }
        else
        {
            BeginTimedMove(enemy, instruction, services);
        }
        break;

    case 70:
        F32At(enemy, 0x2D98) = ReadFloat(enemy, instruction, 0);
        SetMovementState1(enemy);
        break;
    case 71:
        F32At(enemy, 0x2DAC) = ReadFloat(enemy, instruction, 0);
        SetMovementState1(enemy);
        break;
    case 72:
        ResetMovementTimer(enemy, services, ReadInt(enemy, instruction, 0));
        F32At(enemy, 0x2DD0) = ReadFloat(enemy, instruction, 1);
        F32At(enemy, 0x2DD4) = ReadFloat(enemy, instruction, 2);
        F32At(enemy, 0x2D9C) = ReadFloat(enemy, instruction, 3);
        F32At(enemy, 0x2DA0) = ReadFloat(enemy, instruction, 4);
        F32At(enemy, 0x2DB0) = ReadFloat(enemy, instruction, 5);
        F32At(enemy, 0x2DB4) = ReadFloat(enemy, instruction, 6);
        U32At(enemy, 0x3324) |= 0x3000U;
        break;
    case 73:
        ResetMovementTimer(enemy, services, ReadInt(enemy, instruction, 0));
        F32At(enemy, 0x2DD0) = F32At(enemy, 0x2D34);
        F32At(enemy, 0x2DD4) = F32At(enemy, 0x2D38);
        F32At(enemy, 0x2DD8) = F32At(enemy, 0x2D3C);
        F32At(enemy, 0x2D9C) = ReadFloat(enemy, instruction, 1);
        F32At(enemy, 0x2DA0) = ReadFloat(enemy, instruction, 2);
        F32At(enemy, 0x2DB0) = 0.0f;
        F32At(enemy, 0x2DB4) = ReadFloat(enemy, instruction, 3);
        U32At(enemy, 0x3324) |= 0x3000U;
        break;
    case 74:
        ResetMovementTimer(enemy, services, ReadInt(enemy, instruction, 0));
        F32At(enemy, 0x2DA0) = ReadFloat(enemy, instruction, 1);
        F32At(enemy, 0x2DB4) = ReadFloat(enemy, instruction, 2);
        U32At(enemy, 0x3324) |= 0x3000U;
        break;
    case 75:
        F32At(enemy, 0x3340) = ReadFloat(enemy, instruction, 0);
        F32At(enemy, 0x3344) = ReadFloat(enemy, instruction, 1);
        F32At(enemy, 0x3348) = ReadFloat(enemy, instruction, 2);
        F32At(enemy, 0x334C) = ReadFloat(enemy, instruction, 3);
        U32At(enemy, 0x3324) |= 0x80000U;
        break;
    case 76:
        U32At(enemy, 0x3324) &= ~0x80000U;
        break;
    case 77:
        F32At(enemy, 0x2D70) = ReadFloat(enemy, instruction, 0);
        F32At(enemy, 0x2D74) = ReadFloat(enemy, instruction, 1);
        break;
    case 78:
        F32At(enemy, 0x2D7C) = ReadFloat(enemy, instruction, 0);
        F32At(enemy, 0x2D80) = ReadFloat(enemy, instruction, 1);
        break;

    case 79:
        mode = ReadInt(enemy, instruction, 0);
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & ~0x40U) | ((mode & 1) == 0 ? 0x40U : 0);
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & ~0x4U) | ((mode & 2) == 0 ? 0x4U : 0);
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & ~0x8U) | ((mode & 4) == 0 ? 0x8U : 0);
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & ~0x10U) | ((mode & 8) ? 0x10U : 0);
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & ~0x10000000U) | ((mode & 0x10) ? 0x10000000U : 0);
        U32At(enemy, 0x3328) = (U32At(enemy, 0x3328) & ~0x40U) | ((mode & 0x20) ? 0x40U : 0);
        break;

    case 80:
        mode = ReadInt(enemy, instruction, 0);
        if (mode & 1) U32At(enemy, 0x3324) &= ~0x40U;
        if (mode & 2)
        {
            U32At(enemy, 0x3324) &= ~0x4U;
            if (PointerAt(enemy, 0x53C8))
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(PointerAt(enemy, 0x53C8)) + 0x1F8) &= ~0x20000U;
        }
        if (mode & 4) U32At(enemy, 0x3324) &= ~0x8U;
        if (mode & 8) U32At(enemy, 0x3324) |= 0x10U;
        if (mode & 0x10) U32At(enemy, 0x3324) |= 0x10000000U;
        if (mode & 0x20) U32At(enemy, 0x3328) |= 0x40U;
        break;

    case 81:
        mode = ReadInt(enemy, instruction, 0);
        if (mode & 1) U32At(enemy, 0x3324) |= 0x40U;
        if (mode & 2)
        {
            U32At(enemy, 0x3324) |= 0x4U;
            if (PointerAt(enemy, 0x53C8))
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(PointerAt(enemy, 0x53C8)) + 0x1F8) |= 0x20000U;
        }
        if (mode & 4) U32At(enemy, 0x3324) |= 0x8U;
        if (mode & 8) U32At(enemy, 0x3324) &= ~0x10U;
        if (mode & 0x10) U32At(enemy, 0x3324) &= ~0x10000000U;
        if (mode & 0x20) U32At(enemy, 0x3328) &= ~0x40U;
        break;

    case 82:
        F32At(enemy, 0x3350) = ReadFloat(enemy, instruction, 0);
        F32At(enemy, 0x3350) *= F32At(enemy, 0x3350);
        break;
    case 83:
        U32At(enemy, 0x3328) =
            (U32At(enemy, 0x3328) & ~2U) | ((ReadInt(enemy, instruction, 0) & 1) << 1);
        break;

    case 84:
    case 85:
        break; // both target entries are 0x0041E7E4 (ordinary advance)

    case 86:
        if (instruction->operandFlags & 2U)
        {
            // Operand 2 selects the Enemy whose register namespace resolves
            // operand 1.  The destination remains in the current Enemy.
            targetEnemy = services.EnemyByIndex(ReadInt(enemy, instruction, 2));
            lhsInt = EclOperands::ResolveInt(targetEnemy, RawInt(instruction, 1));
        }
        else
        {
            lhsInt = RawInt(instruction, 1);
        }
        *WriteInt(enemy, instruction, 0) = lhsInt;
        break;

    case 87:
        targetEnemy = services.EnemyByIndex(ReadInt(enemy, instruction, 2));
        if (targetEnemy)
        {
            if (instruction->operandFlags & 2U)
            {
                // 0x0041ADC1 resolves operand 2 again before selecting the
                // foreign register namespace for operand 1.
                targetEnemy = services.EnemyByIndex(ReadInt(enemy, instruction, 2));
                lhsFloat = targetEnemy->ResolveFloat(RawFloat(instruction, 1));
            }
            else
            {
                lhsFloat = RawFloat(instruction, 1);
            }
            *WriteFloat(enemy, instruction, 0) = lhsFloat;
        }
        break;

    case 88:
        targetEnemy = services.EnemyByIndex(ReadInt(enemy, instruction, 0));
        services.CallSubOnEnemy(targetEnemy, static_cast<i16>(RawInt(instruction, 1)));
        break;

    case 89:
        targetEnemy = services.EnemyByIndex(ReadInt(enemy, instruction, 0));
        if (targetEnemy)
        {
            const i16 callbackSub = static_cast<i16>(ReadInt(enemy, instruction, 1));
            // Target resolves operand 0 a second time before the store.
            targetEnemy = services.EnemyByIndex(ReadInt(enemy, instruction, 0));
            I16At(targetEnemy, 0x2D30) = callbackSub;
        }
        break;

    case 90:
        services.SpawnLinkedChild(enemy, instruction, CHILD_STANDARD_41F110, 0);
        break;
    case 91:
        services.SpawnLinkedChild(enemy, instruction, CHILD_ALTERNATE_41F280, 0);
        break;
    case 92:
        services.SpawnLinkedChild(enemy, instruction, CHILD_STANDARD_41F110, 1);
        break;

    default:
        return LowResult(LOW_NOT_HANDLED);
    }

    return LowResult(LOW_ADVANCE);
}

} // namespace EclRunLowProposal
} // namespace th08
