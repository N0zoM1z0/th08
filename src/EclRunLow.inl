// TH08 1.00d RunEcl low-opcode body (opcodes 1..92).
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
// EclRun.cpp includes the declarations once, then includes this opcode body
// lexically inside EclManager::RunEcl.  That shared function scope is required
// for the target's handler order, labels, locals, and stack frame.
//
// Every flagged scalar operand in this switch is resolved through
// EclOperands.  Raw operands are used only where the target demonstrably does
// so: jump times/displacements, opcode 36's slot selector, opcode 52's sub-id,
// and opcode 88's remote sub-id.

#ifndef TH08_ECL_RUN_LOW_DECLARATIONS
#define TH08_ECL_RUN_LOW_DECLARATIONS

#include "AnmManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "EnemyManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"

#include <math.h>

namespace th08
{

namespace EclHelpers
{
void __fastcall ConfigurePolarMotion(
    Enemy *enemy, EclRawInstruction *instruction);
void __fastcall ConfigureRelativeMotion(
    Enemy *enemy, EclRawInstruction *instruction);
}

namespace EclRunLow
{

// Observed helper ABIs for opcodes 90..92. Both spawners receive the parent in
// ECX and the current instruction in EDX; the chain-tail lookup uses only ECX.
Enemy *__fastcall FindAttachmentChainTail(
    Enemy *parent);
Enemy *__fastcall SpawnChildAtScriptPosition(
    Enemy *parent, EclRawInstruction *instruction);
Enemy *__fastcall SpawnChildAtParentOffset(
    Enemy *parent, EclRawInstruction *instruction);
void __fastcall ApplyRandomBiasedMove(
    Enemy *enemy, EclRawInstruction *instruction);

// The returned effect begins with an ANM VM.  Keep the call out-of-line: the
// target dispatch calls AnmVm::SetInterrupt at 0x00407120 rather than inlining
// the public header's convenience setter.
struct SpawnedEffectAnmVm
{
    void SetInterrupt(i16 interrupt);
};

void __fastcall ApplyInterpolationOperation(
    Enemy *enemy, EclRawInstruction *instruction);
void __fastcall InstallInterpolationSlot(
    Enemy *enemy, EclRawInstruction *instruction);
void __fastcall CallSubOnEnemy(Enemy *enemy,
                               EclRawInstruction *instruction, i32 rawSubId);
i32 __fastcall PopEclContext(Enemy *enemy,
                             EclRawInstruction *instruction);
void __fastcall SetPrimaryAnmScripts(
    Enemy *enemy, EclRawInstruction *instruction,
    i32 script0, i32 script1, i32 script2, i32 script3, i32 script4,
    i32 script5);
void __fastcall SetExtraAnmScript(Enemy *enemy,
                                  EclRawInstruction *instruction);

// Target behavior at 0x00422020 establishes Enemy in ECX and the current ECL
// instruction in EDX for this boundary-aware movement helper.
void __fastcall BeginBoundaryAwareMove(
    Enemy *enemy, EclRawInstruction *instruction);

inline i32 &RawInt(EclRawInstruction *instruction, i32 index)
{
    return *reinterpret_cast<i32 *>(instruction->operands + index * 4);
}

inline f32 &RawFloat(EclRawInstruction *instruction, i32 index)
{
    return *reinterpret_cast<f32 *>(instruction->operands + index * 4);
}

inline i32 ReadInt(Enemy *enemy,
                   EclRawInstruction *instruction, i32 index)
{
    const i32 raw = RawInt(instruction, index);
    return (instruction->operandFlags & (1U << index))
               ? EclOperands::ResolveInt(enemy, raw)
               : raw;
}

inline f32 ReadFloat(Enemy *enemy,
                     EclRawInstruction *instruction, i32 index)
{
    const f32 raw = RawFloat(instruction, index);
    return (instruction->operandFlags & (1U << index))
               ? enemy->ResolveFloat(raw)
               : raw;
}

inline i32 *WriteInt(Enemy *enemy,
                     EclRawInstruction *instruction, i32 index)
{
    return EclOperands::ResolveIntLValue(enemy, &RawInt(instruction, index),
                                         instruction->operandFlags, index);
}

inline f32 *WriteFloat(Enemy *enemy,
                       EclRawInstruction *instruction, i32 index)
{
    return EclOperands::ResolveFloatLValue(enemy, &RawFloat(instruction, index),
                                           instruction->operandFlags, index);
}

// FUNCTION: th08 0x004215F0
// The twelve comparison opcodes share this target fastcall helper.  Keeping
// their resolver calls here is essential: RunEcl itself has one direct call
// per opcode and receives either the branch destination or NULL.
static EclRawInstruction *__fastcall CompareOperands(
    Enemy *enemy, EclRawInstruction *instruction)
{
    bool takeBranch = false;
    const i32 operation =
        instruction->opcode - ECL_OPCODE_JUMP_IF_INT_EQUAL;
    if (operation == 0)
        takeBranch = ReadInt(enemy, instruction, 0) == ReadInt(enemy, instruction, 1);
    else if (operation == 1)
        takeBranch = ReadFloat(enemy, instruction, 0) == ReadFloat(enemy, instruction, 1);
    else if (operation == 2)
        takeBranch = ReadInt(enemy, instruction, 0) != ReadInt(enemy, instruction, 1);
    else if (operation == 3)
        takeBranch = ReadFloat(enemy, instruction, 0) != ReadFloat(enemy, instruction, 1);
    else if (operation == 4)
        takeBranch = ReadInt(enemy, instruction, 0) < ReadInt(enemy, instruction, 1);
    else if (operation == 5)
        takeBranch = ReadFloat(enemy, instruction, 0) < ReadFloat(enemy, instruction, 1);
    else if (operation == 6)
        takeBranch = ReadInt(enemy, instruction, 0) <= ReadInt(enemy, instruction, 1);
    else if (operation == 7)
        takeBranch = ReadFloat(enemy, instruction, 0) <= ReadFloat(enemy, instruction, 1);
    else if (operation == 8)
        takeBranch = ReadInt(enemy, instruction, 0) > ReadInt(enemy, instruction, 1);
    else if (operation == 9)
        takeBranch = ReadFloat(enemy, instruction, 0) > ReadFloat(enemy, instruction, 1);
    else if (operation == 10)
        takeBranch = ReadInt(enemy, instruction, 0) >= ReadInt(enemy, instruction, 1);
    else if (operation == 11)
        takeBranch = ReadFloat(enemy, instruction, 0) >= ReadFloat(enemy, instruction, 1);

    if (!takeBranch)
        return NULL;

    EnemyEclContext *context = enemy->activeEclContext;
    context->time.current = RawInt(instruction, 2);
    return reinterpret_cast<EclRawInstruction *>(
        reinterpret_cast<u8 *>(instruction) + RawInt(instruction, 3));
}

} // namespace EclRunLow
} // namespace th08

#endif // TH08_ECL_RUN_LOW_DECLARATIONS

#if !defined(TH08_ECL_RUN_DECLARATIONS_ONLY)

#if !defined(TH08_ECL_RUN_LOW_BODY) || !defined(TH08_ECL_RUN_SHARED_SWITCH)
#error EclRunLow.inl opcode body must be included inside EclManager::RunEcl
#endif

#define TH08_ECL_RUN_LOW_YIELD_SELECT_I(controlValue) \
    TH08_ECL_RUN_LOW_YIELD_##controlValue
#define TH08_ECL_RUN_LOW_YIELD_SELECT(controlValue) \
    TH08_ECL_RUN_LOW_YIELD_SELECT_I(controlValue)
#define TH08_ECL_RUN_LOW_YIELD_LOW_RESTART_RUN_LOOP(instructionValue) \
    do { goto restart_context; } while (0)
#define TH08_ECL_RUN_LOW_YIELD_LOW_SELECT_NEXT_CONTEXT(instructionValue) \
    do { goto low_select_next_context; } while (0)
#define TH08_ECL_RUN_LOW_YIELD(controlValue, instructionValue) \
    TH08_ECL_RUN_LOW_YIELD_SELECT(controlValue)(instructionValue)

#define RawInt(insn, index) \
    (*reinterpret_cast<i32 *>((insn)->operands + (index) * 4))
#define RawFloat(insn, index) \
    (*reinterpret_cast<f32 *>((insn)->operands + (index) * 4))
#define ReadInt(owner, insn, index) \
    (((insn)->operandFlags & (1U << (index))) \
         ? EclOperands::ResolveInt((owner), RawInt((insn), (index))) \
         : RawInt((insn), (index)))
#define ReadFloat(owner, insn, index) \
    (((insn)->operandFlags & (1U << (index))) \
         ? (owner)->ResolveFloat(RawFloat((insn), (index))) \
         : RawFloat((insn), (index)))
#define ReadFloatRawArg(owner, insn, index) \
    (((insn)->operandFlags & (1U << (index))) \
         ? (owner)->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt((insn), (index)))) \
         : RawFloat((insn), (index)))

#define WriteInt(owner, insn, index) \
    EclOperands::ResolveIntLValue((owner), &RawInt((insn), (index)), \
                                  (insn)->operandFlags, (index))
#define WriteFloat(owner, insn, index) \
    EclOperands::ResolveFloatLValue((owner), &RawFloat((insn), (index)), \
                                    (insn)->operandFlags, (index))
#pragma var_order(angle, magnitude, lhsFloat, rhsFloat)
    f32 lhsFloat;
    f32 rhsFloat;
    f32 angle;
    f32 magnitude;

    case ECL_OPCODE_TERMINATE:
        return ZUN_ERROR;

    case ECL_OPCODE_SET_SECONDARY_TIME:
        context->secondaryTime = ReadInt(enemy, instruction, 0);
        break;

    case ECL_OPCODE_NOP:
        break; // dispatch-table entry is the ordinary advance path

    case ECL_OPCODE_JUMP_DEC:
        --*WriteInt(enemy, instruction, 2);
        if (ReadInt(enemy, instruction, 2) <= 0)
            goto low_advance_instruction;

    case ECL_OPCODE_JUMP:
        context->time.current = RawInt(instruction, 0);
        instruction = reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) + RawInt(instruction, 1));
        goto low_redispatch_instruction;

    case ECL_OPCODE_SET_INT:
        *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1);
        break;
    case ECL_OPCODE_SET_FLOAT:
        *WriteFloat(enemy, instruction, 0) = ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;
    case ECL_OPCODE_SET_INT_RANDOM_SIGN:
        *WriteInt(enemy, instruction, 0) =
            (g_Rng.GetRandomU16() & 1U ? 1 : -1) * ReadInt(enemy, instruction, 1);
        break;
    case ECL_OPCODE_SET_FLOAT_RANDOM_SIGN:
        *WriteFloat(enemy, instruction, 0) =
            (g_Rng.GetRandomU16() & 1U ? 1.0f : -1.0f) *
            ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;

    case ECL_OPCODE_INT_ADD_ASSIGN: *WriteInt(enemy, instruction, 0) += ReadInt(enemy, instruction, 1); break;
    case ECL_OPCODE_FLOAT_ADD_ASSIGN: *WriteFloat(enemy, instruction, 0) += ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case ECL_OPCODE_INT_SUBTRACT_ASSIGN: *WriteInt(enemy, instruction, 0) -= ReadInt(enemy, instruction, 1); break;
    case ECL_OPCODE_FLOAT_SUBTRACT_ASSIGN: *WriteFloat(enemy, instruction, 0) -= ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case ECL_OPCODE_INT_MULTIPLY_ASSIGN: *WriteInt(enemy, instruction, 0) *= ReadInt(enemy, instruction, 1); break;
    case ECL_OPCODE_FLOAT_MULTIPLY_ASSIGN: *WriteFloat(enemy, instruction, 0) *= ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case ECL_OPCODE_INT_DIVIDE_ASSIGN: *WriteInt(enemy, instruction, 0) /= ReadInt(enemy, instruction, 1); break;
    case ECL_OPCODE_FLOAT_DIVIDE_ASSIGN: *WriteFloat(enemy, instruction, 0) /= ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case ECL_OPCODE_INT_MODULO_ASSIGN: *WriteInt(enemy, instruction, 0) %= ReadInt(enemy, instruction, 1); break;
    case ECL_OPCODE_FLOAT_MODULO_ASSIGN:
        *WriteFloat(enemy, instruction, 0) =
            fmodf(((instruction->operandFlags & (1U << 0))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))),
                  ((instruction->operandFlags & (1U << 1))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))));
        break;

    case ECL_OPCODE_INT_ADD: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) + ReadInt(enemy, instruction, 2); break;
    case ECL_OPCODE_FLOAT_ADD: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) +
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case ECL_OPCODE_INT_SUBTRACT: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) - ReadInt(enemy, instruction, 2); break;
    case ECL_OPCODE_FLOAT_SUBTRACT: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) -
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case ECL_OPCODE_INT_MULTIPLY: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) * ReadInt(enemy, instruction, 2); break;
    case ECL_OPCODE_FLOAT_MULTIPLY: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) *
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case ECL_OPCODE_INT_DIVIDE: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) / ReadInt(enemy, instruction, 2); break;
    case ECL_OPCODE_FLOAT_DIVIDE: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) /
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case ECL_OPCODE_INT_MODULO: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) % ReadInt(enemy, instruction, 2); break;
    case ECL_OPCODE_FLOAT_MODULO:
        *WriteFloat(enemy, instruction, 0) =
            fmodf(((instruction->operandFlags & (1U << 1))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))),
                  ((instruction->operandFlags & (1U << 2))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))));
        break;
    case ECL_OPCODE_INT_INCREMENT: ++*WriteInt(enemy, instruction, 0); break;
    case ECL_OPCODE_INT_DECREMENT: --*WriteInt(enemy, instruction, 0); break;
    case ECL_OPCODE_FLOAT_SINE: *WriteFloat(enemy, instruction, 0) = sinf(((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))); break;
    case ECL_OPCODE_FLOAT_COSINE: *WriteFloat(enemy, instruction, 0) = cosf(((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))); break;
    case ECL_OPCODE_POINT_ANGLE:
        *WriteFloat(enemy, instruction, 0) = VectorAngle((((instruction->operandFlags & (1U << 4))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 4)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 4))) - ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)))), (((instruction->operandFlags & (1U << 3))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 3))) - ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))));
        break;

    case ECL_OPCODE_NORMALIZE_ANGLE:
        *WriteFloat(enemy, instruction, 0) =
            AddNormalizeAngle(((instruction->operandFlags & (1U << 0))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))), 0.0f);
        break;

    case ECL_OPCODE_INTERPOLATE_VALUE:
        ApplyInterpolationOperation(enemy, instruction);
        break;

    case ECL_OPCODE_INSTALL_INTERPOLATION:
        InstallInterpolationSlot(enemy, instruction);
        break;

    case ECL_OPCODE_POLAR_TO_CARTESIAN:
        angle = AddNormalizeAngle(((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))), 0.0f);
        magnitude = ((instruction->operandFlags & (1U << 3)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        *WriteFloat(enemy, instruction, 0) = cosf(angle) * magnitude;
        *WriteFloat(enemy, instruction, 1) = sinf(angle) * magnitude;
        break;

    case ECL_OPCODE_POINT_DISTANCE:
        lhsFloat = ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) - ((instruction->operandFlags & (1U << 3))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        rhsFloat = ((instruction->operandFlags & (1U << 2))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))) - ((instruction->operandFlags & (1U << 4))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 4)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 4)));
        *WriteFloat(enemy, instruction, 0) =
            sqrtf(lhsFloat * lhsFloat + rhsFloat * rhsFloat);
        break;

    // 0x004215F0 comparison order is ==, !=, <, <=, >, >=, with integer
    // and float variants interleaved.  Successful branches use raw operands
    // 2 and 3 for the replacement time and signed bytecode displacement.
    case ECL_OPCODE_JUMP_IF_INT_EQUAL:
    case ECL_OPCODE_JUMP_IF_FLOAT_EQUAL:
    case ECL_OPCODE_JUMP_IF_INT_NOT_EQUAL:
    case ECL_OPCODE_JUMP_IF_FLOAT_NOT_EQUAL:
    case ECL_OPCODE_JUMP_IF_INT_LESS:
    case ECL_OPCODE_JUMP_IF_FLOAT_LESS:
    case ECL_OPCODE_JUMP_IF_INT_LESS_EQUAL:
    case ECL_OPCODE_JUMP_IF_FLOAT_LESS_EQUAL:
    case ECL_OPCODE_JUMP_IF_INT_GREATER:
    case ECL_OPCODE_JUMP_IF_FLOAT_GREATER:
    case ECL_OPCODE_JUMP_IF_INT_GREATER_EQUAL:
    case ECL_OPCODE_JUMP_IF_FLOAT_GREATER_EQUAL:
    {
        EclRawInstruction *branch = CompareOperands(enemy, instruction);
        if (branch)
        {
            instruction = branch;
            goto low_redispatch_instruction;
        }
        break;
    }

    case ECL_OPCODE_CALL:
        CallSubOnEnemy(enemy, instruction, RawInt(instruction, 0));
        TH08_ECL_RUN_LOW_YIELD(LOW_RESTART_RUN_LOOP, 0);

    case ECL_OPCODE_RETURN:
        if (!PopEclContext(enemy, instruction))
            TH08_ECL_RUN_LOW_YIELD(LOW_RESTART_RUN_LOOP, 0);
        TH08_ECL_RUN_LOW_YIELD(LOW_SELECT_NEXT_CONTEXT, 0);

    case ECL_OPCODE_SET_MAIN_ANM:
        g_EnemyManager.enemyAnm
            ->SetAndExecuteScriptIdx(
            &enemy->vm,
            ReadInt(enemy, instruction, 0));
        enemy->flags2 &= ~ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_PRIMARY_ANM_SEQUENCE:
        lhsInt = ReadInt(enemy, instruction, 0);
        SetPrimaryAnmScripts(enemy, instruction, lhsInt, lhsInt + 1,
                             lhsInt + 2, lhsInt + 3, lhsInt + 4,
                             lhsInt + 5);
        enemy->flags2 &= ~ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_PRIMARY_ANM_SCRIPTS:
        SetPrimaryAnmScripts(enemy, instruction,
                             ReadInt(enemy, instruction, 0),
                             ReadInt(enemy, instruction, 1),
                             ReadInt(enemy, instruction, 2),
                             ReadInt(enemy, instruction, 3),
                             ReadInt(enemy, instruction, 4),
                             ReadInt(enemy, instruction, 5));
        enemy->flags2 &= ~ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_EXTRA_ANM_SCRIPT:
        SetExtraAnmScript(enemy, instruction);
        enemy->flags2 &= ~ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_MAIN_ANM_ALTERNATE:
        g_EnemyManager.alternateEnemyAnm
            ->SetAndExecuteScriptIdx(
            &enemy->vm,
            ReadInt(enemy, instruction, 0));
        enemy->flags2 |= ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_PRIMARY_ANM_SEQUENCE_ALTERNATE:
        lhsInt = ReadInt(enemy, instruction, 0);
        SetPrimaryAnmScripts(enemy, instruction, lhsInt, lhsInt + 1, lhsInt + 2,
                             lhsInt + 3, lhsInt + 4, lhsInt + 5);
        enemy->flags2 |= ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_PRIMARY_ANM_SCRIPTS_ALTERNATE:
        SetPrimaryAnmScripts(enemy, instruction,
                             ReadInt(enemy, instruction, 0),
                             ReadInt(enemy, instruction, 1),
                             ReadInt(enemy, instruction, 2),
                             ReadInt(enemy, instruction, 3),
                             ReadInt(enemy, instruction, 4),
                             ReadInt(enemy, instruction, 5));
        enemy->flags2 |= ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        break;
    case ECL_OPCODE_SET_EXTRA_ANM_SCRIPT_ALTERNATE:
        enemy->flags2 |= ENEMY_FLAG2_ALTERNATE_ANM_BANK;
        SetExtraAnmScript(enemy, instruction);
        break;
    case ECL_OPCODE_PLAY_SPECIAL_ANM:
        if (((enemy->flags2 >> ENEMY_FLAG2_ALTERNATE_ANM_BANK_SHIFT) & 1U) == 0)
        {
            g_EnemyManager.enemyAnm
                ->SetAndExecuteScriptIdx(
                &enemy->vm,
                enemy->anmScripts.special);
        }
        else
        {
            g_EnemyManager.alternateEnemyAnm
                ->SetAndExecuteScriptIdx(
                &enemy->vm,
                enemy->anmScripts.special);
        }
        break;

    case ECL_OPCODE_SET_POSITION:
        enemy->position.x = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        enemy->position.y = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        enemy->position.z = 0.0f;
        enemy->ClampPosition();
        break;
    case ECL_OPCODE_MOVE_TO:
        EclHelpers::ConfigureRelativeMotion(enemy, instruction);
        break;
    case ECL_OPCODE_SET_DIRECTION_AND_SPEED:
        enemy->movementAngle = AddNormalizeAngle(((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))), 0.0f);
        enemy->speed = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        enemy->flags1 = (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
                        ENEMY_FLAG_MOVEMENT_MODE_POLAR;
        enemy->movementDuration = 0;
        enemy->movementTimer = 0;
        break;
    case ECL_OPCODE_MOVE_IN_DIRECTION:
        if (ReadInt(enemy, instruction, 0) <= 0)
        {
            enemy->movementAngle = AddNormalizeAngle(((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))), 0.0f);
            enemy->speed = ((instruction->operandFlags & (1U << 3)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
            enemy->flags1 = (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
                            ENEMY_FLAG_MOVEMENT_MODE_POLAR;
            enemy->movementDuration = 0;
            enemy->movementTimer = 0;
        }
        else EclHelpers::ConfigurePolarMotion(enemy, instruction);
        break;
    case ECL_OPCODE_MOVE_RANDOM_IN_BOUNDS:
        BeginBoundaryAwareMove(enemy, instruction);
        break;
#ifdef TH08_ECL_RUN_LOW_BODY
    // Target physical order places opcode 178 between opcodes 67 and 68.
    case ECL_OPCODE_MOVE_RANDOM_BIASED:
        ApplyRandomBiasedMove(
            TH08_ECL_CONTEXT_ENEMY(ctx),
            reinterpret_cast<EclRawInstruction *>(TH08_ECL_CONTEXT_INSTRUCTION(ctx)));
        break;
#endif
    case ECL_OPCODE_SET_AIMED_DIRECTION_AND_SPEED:
        enemy->movementAngle =
            AddNormalizeAngle(
                ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))),
                g_Player.AngleToPoint(
                    &enemy->position));
        enemy->speed = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;
    case ECL_OPCODE_MOVE_IN_AIMED_DIRECTION:
        if (ReadInt(enemy, instruction, 0) <= 0)
        {
            enemy->movementAngle =
                AddNormalizeAngle(
                    ((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))),
                    g_Player.AngleToPoint(
                        &enemy->position));
            enemy->speed = ((instruction->operandFlags & (1U << 3)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
            enemy->flags1 =
                (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
                ENEMY_FLAG_MOVEMENT_MODE_POLAR;
            // The target resolves operand 0 again before timer assignment.
            enemy->movementTimer =
                (enemy->movementDuration = ReadInt(enemy, instruction, 0));
        }
        else
        {
            EclHelpers::ConfigurePolarMotion(enemy, instruction);
        }
        break;

    case ECL_OPCODE_SET_ANGULAR_VELOCITY:
        enemy->angularVelocity = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        enemy->flags1 = (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
                        ENEMY_FLAG_MOVEMENT_MODE_POLAR;
        break;
    case ECL_OPCODE_SET_ACCELERATION:
        enemy->acceleration = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        enemy->flags1 = (enemy->flags1 & ~ENEMY_FLAG_MOVEMENT_MODE_MASK) |
                        ENEMY_FLAG_MOVEMENT_MODE_POLAR;
        break;
    case ECL_OPCODE_ORBIT_AROUND_POINT:
        enemy->movementTimer =
            (enemy->movementDuration = ReadInt(enemy, instruction, 0));
        enemy->movementInterpolationOrigin.x = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        enemy->movementInterpolationOrigin.y = ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        enemy->orbitAngle = ((instruction->operandFlags & (1U << 3))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        enemy->orbitAngularVelocity = ((instruction->operandFlags & (1U << 4))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 4)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 4)));
        enemy->orbitRadius = ((instruction->operandFlags & (1U << 5))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 5)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 5)));
        enemy->radialVelocity = ((instruction->operandFlags & (1U << 6))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 6)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 6)));
        enemy->flags1 |= ENEMY_FLAG_MOVEMENT_MODE_ORBIT;
        break;
    case ECL_OPCODE_ORBIT_AROUND_CURRENT_POSITION:
        enemy->movementTimer =
            (enemy->movementDuration = ReadInt(enemy, instruction, 0));
        *D3DXVECTOR3_PTR(
            &enemy->movementInterpolationOrigin) =
            *D3DXVECTOR3_PTR(&enemy->position);
        enemy->orbitAngle = ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        enemy->orbitAngularVelocity = ((instruction->operandFlags & (1U << 2))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        enemy->orbitRadius = 0.0f;
        enemy->radialVelocity = ((instruction->operandFlags & (1U << 3))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        enemy->flags1 |= ENEMY_FLAG_MOVEMENT_MODE_ORBIT;
        break;
    case ECL_OPCODE_SET_ORBIT_VELOCITIES:
        enemy->movementTimer =
            (enemy->movementDuration = ReadInt(enemy, instruction, 0));
        enemy->orbitAngularVelocity = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        enemy->radialVelocity = ((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        enemy->flags1 |= ENEMY_FLAG_MOVEMENT_MODE_ORBIT;
        break;
    case ECL_OPCODE_SET_MOVEMENT_BOUNDS:
        enemy->movementBounds.lower.x = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        enemy->movementBounds.lower.y = ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        enemy->movementBounds.upper.x = ((instruction->operandFlags & (1U << 2))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        enemy->movementBounds.upper.y = ((instruction->operandFlags & (1U << 3))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        enemy->flags1 |= ENEMY_FLAG_CLAMP_POSITION;
        break;
    case ECL_OPCODE_DISABLE_MOVEMENT_BOUNDS:
        enemy->flags1 &= ~ENEMY_FLAG_CLAMP_POSITION;
        break;
    case ECL_OPCODE_SET_HITBOX:
        enemy->hitboxDimensions.x = ((instruction->operandFlags & (1U << 0))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        enemy->hitboxDimensions.y = ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;
    case ECL_OPCODE_SET_SECONDARY_HITBOX:
        enemy->secondaryHitboxDimensions.x = ((instruction->operandFlags & (1U << 0))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        enemy->secondaryHitboxDimensions.y = ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;

    case ECL_OPCODE_SET_INTERACTION_FLAGS:
        lhsInt = ReadInt(enemy, instruction, 0);
        reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->acceptsDamage =
            (lhsInt & ECL_INTERACTION_ACCEPTS_DAMAGE) == 0;
        reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->collision =
            (lhsInt & ECL_INTERACTION_COLLISION) == 0;
        reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->damageable =
            (lhsInt & ECL_INTERACTION_DAMAGEABLE) == 0;
        reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->noSprite =
            (lhsInt & ECL_INTERACTION_NO_SPRITE) != 0;
        reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->allowOffscreen =
            (lhsInt & ECL_INTERACTION_ALLOW_OFFSCREEN) != 0;
        reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->noDeath =
            (lhsInt & ECL_INTERACTION_NO_DEATH) != 0;
        break;

    case ECL_OPCODE_DISABLE_INTERACTION_FLAGS:
        lhsInt = ReadInt(enemy, instruction, 0);
        if (lhsInt & ECL_INTERACTION_ACCEPTS_DAMAGE)
            enemy->flags1 &= ~ENEMY_FLAG_ACCEPTS_DAMAGE;
        if (lhsInt & ECL_INTERACTION_COLLISION)
        {
            enemy->flags1 &= ~ENEMY_FLAG_COLLISION;
            if (enemy->alignmentEffect)
                enemy->alignmentEffect->vm.flag17 = 0;
        }
        if (lhsInt & ECL_INTERACTION_DAMAGEABLE)
            enemy->flags1 &= ~ENEMY_FLAG_DAMAGEABLE;
        if (lhsInt & ECL_INTERACTION_NO_SPRITE)
            enemy->flags1 |= ENEMY_FLAG_NO_SPRITE;
        if (lhsInt & ECL_INTERACTION_ALLOW_OFFSCREEN)
            enemy->flags1 |= ENEMY_FLAG_ALLOW_OFFSCREEN;
        if (lhsInt & ECL_INTERACTION_NO_DEATH)
            enemy->flags2 |= ENEMY_FLAG2_NO_DEATH;
        break;

    case ECL_OPCODE_ENABLE_INTERACTION_FLAGS:
        lhsInt = ReadInt(enemy, instruction, 0);
        if (lhsInt & ECL_INTERACTION_ACCEPTS_DAMAGE)
            enemy->flags1 |= ENEMY_FLAG_ACCEPTS_DAMAGE;
        if (lhsInt & ECL_INTERACTION_COLLISION)
        {
            enemy->flags1 |= ENEMY_FLAG_COLLISION;
            if (enemy->alignmentEffect)
                enemy->alignmentEffect->vm.flag17 = 1;
        }
        if (lhsInt & ECL_INTERACTION_DAMAGEABLE)
            enemy->flags1 |= ENEMY_FLAG_DAMAGEABLE;
        if (lhsInt & ECL_INTERACTION_NO_SPRITE)
            enemy->flags1 &= ~ENEMY_FLAG_NO_SPRITE;
        if (lhsInt & ECL_INTERACTION_ALLOW_OFFSCREEN)
            enemy->flags1 &= ~ENEMY_FLAG_ALLOW_OFFSCREEN;
        if (lhsInt & ECL_INTERACTION_NO_DEATH)
            enemy->flags2 &= ~ENEMY_FLAG2_NO_DEATH;
        break;

    case ECL_OPCODE_NOP_84:
    case ECL_OPCODE_NOP_85:
        break; // both target entries are 0x0041E7E4 (ordinary advance)

    case ECL_OPCODE_SET_REMOTE_INT:
        *WriteInt(enemy, instruction, 0) =
            (instruction->operandFlags & 2U)
                ? EclOperands::ResolveInt(
                      g_EnemyManager.bosses[ReadInt(enemy, instruction, 2)],
                      RawInt(instruction, 1))
                : RawInt(instruction, 1);
        break;

    case ECL_OPCODE_SET_REMOTE_FLOAT:
        if (g_EnemyManager.bosses[ReadInt(enemy, instruction, 2)])
            *WriteFloat(enemy, instruction, 0) =
                (instruction->operandFlags & 2U)
                    ? g_EnemyManager.bosses[ReadInt(enemy, instruction, 2)]->ResolveFloat(
                          *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                    : *reinterpret_cast<f32 *>(&RawInt(instruction, 1));
        break;

    case ECL_OPCODE_CALL_REMOTE:
        lhsInt = ReadInt(enemy, instruction, 0);
        CallSubOnEnemy(
            g_EnemyManager.bosses[lhsInt],
            g_EnemyManager.bosses[lhsInt]->activeEclContext->currentInstr,
            RawInt(instruction, 1));
        break;

    case ECL_OPCODE_SCHEDULE_REMOTE_SUBROUTINE:
        if (g_EnemyManager.bosses[ReadInt(enemy, instruction, 0)])
        {
            // Target resolves operand 0 a second time before the store.
            g_EnemyManager.bosses[ReadInt(enemy, instruction, 0)]->pendingEclSubroutineIndex =
                static_cast<i16>(ReadInt(enemy, instruction, 1));
        }
        break;

    // Target fact map for the linked-child cluster:
    //   90: 0x0041AF5B..0x0041B10A, standard constructor 0x0041F110
    //   91: 0x0041B10B..0x0041B2BA, alternate constructor 0x0041F280
    //   92: 0x0041B2BB..0x0041B4DB, standard constructor 0x0041F110
    // Each case calls 0x0041EFC0, its constructor, IsYoukai three times when
    // initialization is enabled, 0x00425B70 and 0x00407120 when no effect is
    // attached, and unconditionally plays sound 0x24 through 0x0045D660.
    // Case 92 additionally calls D3DXVECTOR3::operator+ at 0x00409080.
    case ECL_OPCODE_SPAWN_FAMILIAR_AT_POSITION:
    {
        Enemy *tail = FindAttachmentChainTail(enemy);
        Enemy *child =
            SpawnChildAtScriptPosition(enemy, instruction);

        if (!g_EnemyManager.lastSpawnFailed)
        {
            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                linkedChild = 1;
            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                youkaiAligned =
                g_Player.IsYoukai();
            child->drawGroup =
                static_cast<u8>(((g_Player.IsYoukai()
                                  ? -1
                                  : 0) &
                                 -2) +
                                2);
            child->flags1 &= ~ENEMY_FLAG_COLLISION;

            if (child->alignmentEffect == 0)
            {
                child->alignmentEffect =
                    g_EffectManager.SpawnEffectInSecondaryPool(
                    EFFECT_ALIGNMENT_BASE,
                    D3DXVECTOR3_PTR(
                        &child->position),
                    1, -1);
                child->alignmentEffect->vm.SetInterrupt(
                        g_Player.IsYoukai()
                            ? 2
                            : 1);
                child->alignmentEffect->vm.flag17 =
                    reinterpret_cast<EnemyFlag1Bits *>(
                        &child->flags1)->
                            collision != 0;
                if (child->enemyIndex & 1)
                {
                    reinterpret_cast<AnmVm *>(child->alignmentEffect)
                        ->angleVel.z =
                        -reinterpret_cast<AnmVm *>(child->alignmentEffect)
                             ->angleVel.z;
                }
            }

            child->parentEnemy =
                enemy;
            tail->nextInAttachmentChain =
                child;
            child->previousInAttachmentChain =
                tail;
            ++enemy->linkedChildCount;
        }

        g_SoundPlayer.PlaySoundPositionedByIdx(
            SOUND_FAMILIAR_SPAWN,
            enemy->position.x);
        break;
    }
    case ECL_OPCODE_SPAWN_FAMILIAR_AT_OFFSET:
    {
        Enemy *tail = FindAttachmentChainTail(enemy);
        Enemy *child =
            SpawnChildAtParentOffset(enemy, instruction);

        if (!g_EnemyManager.lastSpawnFailed)
        {
            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                linkedChild = 1;
            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                youkaiAligned =
                g_Player.IsYoukai();
            child->drawGroup =
                static_cast<u8>(((g_Player.IsYoukai()
                                  ? -1
                                  : 0) &
                                 -2) +
                                2);
            child->flags1 &= ~ENEMY_FLAG_COLLISION;

            if (child->alignmentEffect == 0)
            {
                child->alignmentEffect =
                    g_EffectManager.SpawnEffectInSecondaryPool(
                    EFFECT_ALIGNMENT_BASE,
                    D3DXVECTOR3_PTR(
                        &child->position),
                    1, -1);
                child->alignmentEffect->vm.SetInterrupt(
                        g_Player.IsYoukai()
                            ? 2
                            : 1);
                child->alignmentEffect->vm.flag17 =
                    reinterpret_cast<EnemyFlag1Bits *>(
                        &child->flags1)->
                            collision != 0;
                if (child->enemyIndex & 1)
                {
                    reinterpret_cast<AnmVm *>(child->alignmentEffect)
                        ->angleVel.z =
                        -reinterpret_cast<AnmVm *>(child->alignmentEffect)
                             ->angleVel.z;
                }
            }

            child->parentEnemy =
                enemy;
            tail->nextInAttachmentChain =
                child;
            child->previousInAttachmentChain =
                tail;
            ++enemy->linkedChildCount;
        }

        g_SoundPlayer.PlaySoundPositionedByIdx(
            SOUND_FAMILIAR_SPAWN,
            enemy->position.x);
        break;
    }
    case ECL_OPCODE_SPAWN_FAMILIAR_INHERITING_POSITION:
    {
        Enemy *tail = FindAttachmentChainTail(enemy);
        Enemy *child =
            SpawnChildAtScriptPosition(enemy, instruction);

        if (!g_EnemyManager.lastSpawnFailed)
        {
            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                linkedChild = 1;
            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                youkaiAligned =
                g_Player.IsYoukai();
            child->drawGroup =
                static_cast<u8>(((g_Player.IsYoukai()
                                  ? -1
                                  : 0) &
                                 -2) +
                                2);

            *D3DXVECTOR3_PTR(
                &child->positionOffset) =
                *D3DXVECTOR3_PTR(
                    &enemy->position);
            *D3DXVECTOR3_PTR(
                &child->worldPosition) =
                *D3DXVECTOR3_PTR(
                    &child->positionOffset) +
                *D3DXVECTOR3_PTR(
                    &child->position);
            child->flags1 &= ~ENEMY_FLAG_COLLISION;

            if (child->alignmentEffect == 0)
            {
                child->alignmentEffect =
                    g_EffectManager.SpawnEffectInSecondaryPool(
                    EFFECT_ALIGNMENT_BASE,
                    D3DXVECTOR3_PTR(
                        &child->worldPosition),
                    1, -1);
                child->alignmentEffect->vm.SetInterrupt(
                        g_Player.IsYoukai()
                            ? 2
                            : 1);
                child->alignmentEffect->vm.flag17 =
                    reinterpret_cast<EnemyFlag1Bits *>(
                        &child->flags1)->
                            collision != 0;
                if (child->enemyIndex & 1)
                {
                    reinterpret_cast<AnmVm *>(child->alignmentEffect)
                        ->angleVel.z =
                        -reinterpret_cast<AnmVm *>(child->alignmentEffect)
                             ->angleVel.z;
                }
            }

            reinterpret_cast<EnemyFlag1Bits *>(&child->flags1)->
                inheritParentPosition = 1;
            child->parentEnemy =
                enemy;
            tail->nextInAttachmentChain =
                child;
            child->previousInAttachmentChain =
                tail;
            ++enemy->linkedChildCount;
        }

        g_SoundPlayer.PlaySoundPositionedByIdx(
            SOUND_FAMILIAR_SPAWN,
            enemy->position.x);
        break;
    }

#undef TH08_ECL_RUN_LOW_YIELD

#undef TH08_ECL_RUN_LOW_YIELD_LOW_SELECT_NEXT_CONTEXT
#undef TH08_ECL_RUN_LOW_YIELD_LOW_RESTART_RUN_LOOP
#undef TH08_ECL_RUN_LOW_YIELD_SELECT
#undef TH08_ECL_RUN_LOW_YIELD_SELECT_I
#undef WriteFloat
#undef WriteInt
#undef ReadFloat
#undef ReadInt
#undef RawFloat
#undef RawInt

#endif // !TH08_ECL_RUN_DECLARATIONS_ONLY
