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
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction);
void __fastcall ConfigureRelativeMotion(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction);
}

// Private resolver overlay already used by EclOperandsInt.cpp and
// EclOperandsFloat.cpp.  Reusing its IsYoukai symbol keeps the direct target
// call at 0x0040BC40 distinct from the still-incomplete public Player layout.
namespace EclOperands
{
struct TargetVector3;
struct TargetPlayerOverlay
{
    f32 AngleToPlayer(const TargetVector3 *position);
    i32 IsYoukai();
};
extern TargetPlayerOverlay g_TargetPlayer017D5EF8;
}

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
};

inline LowResult MakeLowResult(LowControl control,
                               EclRawInstruction *nextInstruction = 0)
{
    LowResult result;
    result.control = control;
    result.nextInstruction = nextInstruction;
    return result;
}

// Target pointer table at 0x00F54CC0, indexed by the ECL enemy selector.
extern EclOperands::EnemyOverlay *g_EclEnemyTableF54CC0[];

// Observed helper ABIs for opcodes 90..92.  Both constructors receive the
// parent in ECX and the current instruction in EDX; the list-tail lookup uses
// only ECX.  Names remain provisional until the owning Enemy layout lands.
EclOperands::EnemyOverlay *__fastcall FindLinkedChildTail0041EFC0(
    EclOperands::EnemyOverlay *parent);
EclOperands::EnemyOverlay *__fastcall SpawnChildStandard0041F110(
    EclOperands::EnemyOverlay *parent, EclRawInstruction *instruction);
EclOperands::EnemyOverlay *__fastcall SpawnChildAlternate0041F280(
    EclOperands::EnemyOverlay *parent, EclRawInstruction *instruction);
void __fastcall FUN_004224a0(u8 *enemy, void *instruction);

// The returned effect begins with an ANM VM.  Keep the call out-of-line: the
// target dispatch calls AnmVm::SetInterrupt at 0x00407120 rather than inlining
// the public header's convenience setter.
struct SpawnedEffectAnmVm
{
    void SetInterrupt(i16 interrupt);
};

// Observed first flag word at Enemy +0x3324.  Opcode 90..92 assign bit 11
// from Player::IsYoukai through VC7's bitfield-assignment path; opcodes 173
// and 183 likewise assign bits 30 and 31 as one-bit fields.  Preserving the
// bitfield is necessary both for semantics and for the call-result stack home.
struct LinkedChildFlags1
{
    u32 unknown00 : 2;
    u32 effectMirrored : 1;
    u32 op79Bit3 : 1;
    u32 op79Bit4 : 1;
    u32 unknown05 : 1;
    u32 op79Bit6 : 1;
    u32 op156Bit7 : 1;
    u32 linkedChild : 1;
    u32 inheritParentPosition : 1;
    u32 unknown0A : 1;
    u32 isYoukai : 1;
    u32 unknown0C_13 : 8;
    u32 op129Bits20_22 : 3;
    u32 unknown17_18 : 2;
    u32 op145Bit25 : 1;
    u32 op151Bit26 : 1;
    u32 op155Bit27 : 1;
    u32 op79Bit28 : 1;
    u32 unknown1D : 1;
    u32 pauseTimer : 1;
    u32 noDamageDuringStop : 1;
};
C_ASSERT(sizeof(LinkedChildFlags1) == 4);

struct Op79Flags2
{
    u32 unknown00 : 1;
    u32 op83Bit1 : 1;
    u32 unknown02_05 : 4;
    u32 op79Bit6 : 1;
    u32 unknown07 : 1;
    u32 op182Bit8 : 1;
    u32 unknown09_31 : 23;
};
C_ASSERT(sizeof(Op79Flags2) == 4);

void __fastcall ApplyInterpolationOperation(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction);
void __fastcall InstallInterpolationSlot(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction);
void __fastcall CallSubOnEnemy(EclOperands::EnemyOverlay *enemy,
                               EclRawInstruction *instruction, i32 rawSubId);
i32 __fastcall PopEclContext(EclOperands::EnemyOverlay *enemy,
                             EclRawInstruction *instruction);
void __fastcall SetPrimaryAnmScripts(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction,
    i32 script0, i32 script1, i32 script2, i32 script3, i32 script4,
    i32 script5);
void __fastcall SetExtraAnmScript(EclOperands::EnemyOverlay *enemy,
                                  EclRawInstruction *instruction);

// Provisional semantic name for target FUN_00422020.  Caller and callee both
// establish Enemy in ECX and the current ECL instruction in EDX.
void __fastcall BeginBoundaryAwareMove(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction);

// Adapter boundary for target helpers whose owned layouts live outside the
// RunEcl lane.  Implementations must preserve the stated target behavior.
struct Services
{
    f32 Sin(f32 angle);              // 0x00409060
    f32 Cos(f32 angle);              // 0x00408D40
    f32 Atan2(f32 y, f32 x);         // 0x0040C7B0
    f32 Sqrt(f32 value);              // 0x0040B440
    f32 NormalizeAngle(f32 angle, f32 base); // 0x0043EDB0

    // ZunTimer::operator=(i32), target 0x004065F0.
    void AssignTimer(void *timer, i32 value);

    // Opcodes 35 and 36 call target helpers 0x00421300 and 0x004213F0.  Both
    // helpers own all operand resolution; RunEcl only supplies Enemy and the
    // current instruction.
    void ApplyInterpolationOperation(EclOperands::EnemyOverlay *enemy,
                                             EclRawInstruction *instruction);
    void InstallInterpolationSlot(EclOperands::EnemyOverlay *enemy,
                                          EclRawInstruction *instruction);

    // Opcode 53 / 0x00421CB0.  Returns nonzero when the primary context was
    // restored, zero when a saved nested context was popped in place.
    i32 PopEclContext(EclOperands::EnemyOverlay *enemy,
                              const EclRawInstruction *instruction);

    void SetPrimaryAnmScript(EclOperands::EnemyOverlay *enemy, i32 script);
    // 0x00421DE0 stores the six low words at +3332, +3338, +333A, +3334,
    // +3336, +333C respectively, then writes 0xFF at +332E.
    void SetPrimaryAnmScripts(EclOperands::EnemyOverlay *enemy,
                                      i32 script0, i32 script1, i32 script2,
                                      i32 script3, i32 script4, i32 script5);
    void SetExtraAnmScript(EclOperands::EnemyOverlay *enemy,
                                   EclRawInstruction *instruction); // 0x00421E50
    void RefreshBaseVector(EclOperands::EnemyOverlay *enemy); // 0x0042C180

    // Opcode 64 / 0x00420F40: begin an interpolated move toward (x,y).
    // The helper snapshots target/current vectors, resets the movement timer,
    // installs (mode & 7), selects movement state 2, and honors mirror bit 18.
    void BeginPointMove(EclOperands::EnemyOverlay *enemy,
                                EclRawInstruction *instruction);

    // Opcode 67 is wholly owned by 0x00422020, including its operand reads,
    // boundary-aware angle, and movement-state selection.
    void BeginBoundaryAwareMove(EclOperands::EnemyOverlay *enemy,
                                        EclRawInstruction *instruction);
    f32 AngleToPlayer(const void *vector);                       // 0x0044C1B0

    // 0x00421BD0.  It advances targetEnemy's current instruction, snapshots
    // its context when required, and invokes CallEclSub with rawSubId.
    void CallSubOnEnemy(EclOperands::EnemyOverlay *targetEnemy,
                                i16 rawSubId);

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

// FUNCTION: th08 0x004215F0
// The twelve comparison opcodes share this target fastcall helper.  Keeping
// their resolver calls here is essential: RunEcl itself has one direct call
// per opcode and receives either the branch destination or NULL.
static EclRawInstruction *__fastcall CompareOperands(
    EclOperands::EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    bool takeBranch = false;
    const i32 operation = instruction->opcode - 40;
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

    EnemyEclContext *context =
        *reinterpret_cast<EnemyEclContext **>(Bytes(enemy) + 0x2CA0);
    context->time.current = RawInt(instruction, 2);
    return reinterpret_cast<EclRawInstruction *>(
        reinterpret_cast<u8 *>(instruction) + RawInt(instruction, 3));
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

} // namespace EclRunLowProposal
} // namespace th08

#endif // TH08_ECL_RUN_LOW_DECLARATIONS

#if !defined(TH08_ECL_RUN_DECLARATIONS_ONLY)

#ifdef TH08_ECL_RUN_LOW_BODY
#define TH08_ECL_RUN_LOW_YIELD_SELECT_I(controlValue) \
    TH08_ECL_RUN_LOW_YIELD_##controlValue
#define TH08_ECL_RUN_LOW_YIELD_SELECT(controlValue) \
    TH08_ECL_RUN_LOW_YIELD_SELECT_I(controlValue)
#define TH08_ECL_RUN_LOW_YIELD_LOW_RETURN_MINUS_ONE(instructionValue) \
    do { return ZUN_ERROR; } while (0)
#define TH08_ECL_RUN_LOW_YIELD_LOW_REDISPATCH(instructionValue) \
    do { instruction = (instructionValue); goto low_redispatch_instruction; } while (0)
#define TH08_ECL_RUN_LOW_YIELD_LOW_RESTART_RUN_LOOP(instructionValue) \
    do { goto restart_context; } while (0)
#define TH08_ECL_RUN_LOW_YIELD_LOW_SELECT_NEXT_CONTEXT(instructionValue) \
    do { goto low_select_next_context; } while (0)
#define TH08_ECL_RUN_LOW_YIELD_LOW_ADVANCE(instructionValue) \
    do { goto low_advance_instruction; } while (0)
#define TH08_ECL_RUN_LOW_YIELD_LOW_NOT_HANDLED(instructionValue) \
    do { goto low_advance_instruction; } while (0)
#define TH08_ECL_RUN_LOW_YIELD(controlValue, instructionValue) \
    TH08_ECL_RUN_LOW_YIELD_SELECT(controlValue)(instructionValue)

// The target RunEcl body performs these byte-overlay accesses directly.  VC7
// /Ob0 emits even __forceinline helpers as separate COMDAT calls, so the
// lexical fragment uses expression macros while the standalone proposal keeps
// the readable helper functions above.
#define Bytes(owner) (reinterpret_cast<u8 *>(owner))
#define U32At(owner, offset) (*reinterpret_cast<u32 *>(Bytes(owner) + (offset)))
#define I32At(owner, offset) (*reinterpret_cast<i32 *>(Bytes(owner) + (offset)))
#define I16At(owner, offset) (*reinterpret_cast<i16 *>(Bytes(owner) + (offset)))
#define F32At(owner, offset) (*reinterpret_cast<f32 *>(Bytes(owner) + (offset)))
#define PointerAt(owner, offset) (*reinterpret_cast<void **>(Bytes(owner) + (offset)))
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
#else

namespace th08
{
namespace EclRunLowProposal
{

#define TH08_ECL_RUN_LOW_YIELD(controlValue, instructionValue) \
    return MakeLowResult((controlValue), (instructionValue))

inline LowResult Dispatch(EclOperands::EnemyOverlay *enemy,
                          EnemyEclContext *context,
                          EclRawInstruction *instruction,
                          Services &services)
{
#endif

#ifndef TH08_ECL_RUN_LOW_BODY
    i32 lhsInt;
#endif
#pragma var_order(angle, magnitude, lhsFloat, rhsFloat)
    f32 lhsFloat;
    f32 rhsFloat;
    f32 angle;
    f32 magnitude;

#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
    switch (instruction->opcode)
    {
#endif
    case 1:
        return ZUN_ERROR;

    case 2:
        context->secondaryTime = ReadInt(enemy, instruction, 0);
        break;

    case 3:
        break; // dispatch-table entry is the ordinary advance path

    case 5:
        --*WriteInt(enemy, instruction, 2);
        if (ReadInt(enemy, instruction, 2) <= 0)
            goto low_advance_instruction;

    case 4:
        context->time.current = RawInt(instruction, 0);
        instruction = reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) + RawInt(instruction, 1));
        goto low_redispatch_instruction;

    case 6:
        *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1);
        break;
    case 7:
        *WriteFloat(enemy, instruction, 0) = ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;
    case 8:
        *WriteInt(enemy, instruction, 0) =
            (g_Rng.GetRandomU16() & 1U ? 1 : -1) * ReadInt(enemy, instruction, 1);
        break;
    case 9:
        *WriteFloat(enemy, instruction, 0) =
            (g_Rng.GetRandomU16() & 1U ? 1.0f : -1.0f) *
            ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;

    case 10: *WriteInt(enemy, instruction, 0) += ReadInt(enemy, instruction, 1); break;
    case 15: *WriteFloat(enemy, instruction, 0) += ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case 11: *WriteInt(enemy, instruction, 0) -= ReadInt(enemy, instruction, 1); break;
    case 16: *WriteFloat(enemy, instruction, 0) -= ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case 12: *WriteInt(enemy, instruction, 0) *= ReadInt(enemy, instruction, 1); break;
    case 17: *WriteFloat(enemy, instruction, 0) *= ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case 13: *WriteInt(enemy, instruction, 0) /= ReadInt(enemy, instruction, 1); break;
    case 18: *WriteFloat(enemy, instruction, 0) /= ((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))); break;
    case 14: *WriteInt(enemy, instruction, 0) %= ReadInt(enemy, instruction, 1); break;
    case 19:
        *WriteFloat(enemy, instruction, 0) =
            fmodf(((instruction->operandFlags & (1U << 0))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))),
                  ((instruction->operandFlags & (1U << 1))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))));
        break;

    case 20: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) + ReadInt(enemy, instruction, 2); break;
    case 25: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) +
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case 21: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) - ReadInt(enemy, instruction, 2); break;
    case 26: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) -
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case 22: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) * ReadInt(enemy, instruction, 2); break;
    case 27: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) *
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case 23: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) / ReadInt(enemy, instruction, 2); break;
    case 28: *WriteFloat(enemy, instruction, 0) =
        ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))) /
        ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))); break;
    case 24: *WriteInt(enemy, instruction, 0) = ReadInt(enemy, instruction, 1) % ReadInt(enemy, instruction, 2); break;
    case 29:
        *WriteFloat(enemy, instruction, 0) =
            fmodf(((instruction->operandFlags & (1U << 1))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 1))),
                  ((instruction->operandFlags & (1U << 2))
                       ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                       : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))));
        break;
    case 30: ++*WriteInt(enemy, instruction, 0); break;
    case 31: --*WriteInt(enemy, instruction, 0); break;
    case 32: *WriteFloat(enemy, instruction, 0) = sinf(((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))); break;
    case 33: *WriteFloat(enemy, instruction, 0) = cosf(((instruction->operandFlags & (1U << 1))
        ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
        : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))); break;
    case 34:
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

    case 37:
        *WriteFloat(enemy, instruction, 0) =
            AddNormalizeAngle(((instruction->operandFlags & (1U << 0))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))), 0.0f);
        break;

    case 35:
        ApplyInterpolationOperation(enemy, instruction);
        break;

    case 36:
        InstallInterpolationSlot(enemy, instruction);
        break;

    case 38:
        angle = AddNormalizeAngle(((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))), 0.0f);
        magnitude = ((instruction->operandFlags & (1U << 3)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        *WriteFloat(enemy, instruction, 0) = cosf(angle) * magnitude;
        *WriteFloat(enemy, instruction, 1) = sinf(angle) * magnitude;
        break;

    case 39:
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
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    {
        EclRawInstruction *branch = CompareOperands(enemy, instruction);
        if (branch)
        {
            instruction = branch;
            goto low_redispatch_instruction;
        }
        break;
    }

    case 52:
        CallSubOnEnemy(enemy, instruction, RawInt(instruction, 0));
        TH08_ECL_RUN_LOW_YIELD(LOW_RESTART_RUN_LOOP, 0);

    case 53:
        if (!PopEclContext(enemy, instruction))
            TH08_ECL_RUN_LOW_YIELD(LOW_RESTART_RUN_LOOP, 0);
        TH08_ECL_RUN_LOW_YIELD(LOW_SELECT_NEXT_CONTEXT, 0);

    case 54:
        (*reinterpret_cast<AnmLoaded **>(
            reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCEEC))
            ->SetAndExecuteScriptIdx(
            reinterpret_cast<AnmVm *>(Bytes(enemy) + 0xC),
            ReadInt(enemy, instruction, 0));
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 55:
        lhsInt = ReadInt(enemy, instruction, 0);
        SetPrimaryAnmScripts(enemy, instruction, lhsInt, lhsInt + 1,
                             lhsInt + 2, lhsInt + 3, lhsInt + 4,
                             lhsInt + 5);
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 56:
        SetPrimaryAnmScripts(enemy, instruction,
                             ReadInt(enemy, instruction, 0),
                             ReadInt(enemy, instruction, 1),
                             ReadInt(enemy, instruction, 2),
                             ReadInt(enemy, instruction, 3),
                             ReadInt(enemy, instruction, 4),
                             ReadInt(enemy, instruction, 5));
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 57:
        SetExtraAnmScript(enemy, instruction);
        U32At(enemy, 0x3328) &= ~4U;
        break;
    case 58:
        (*reinterpret_cast<AnmLoaded **>(
            reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCEF0))
            ->SetAndExecuteScriptIdx(
            reinterpret_cast<AnmVm *>(Bytes(enemy) + 0xC),
            ReadInt(enemy, instruction, 0));
        U32At(enemy, 0x3328) |= 4U;
        break;
    case 59:
        lhsInt = ReadInt(enemy, instruction, 0);
        SetPrimaryAnmScripts(enemy, instruction, lhsInt, lhsInt + 1, lhsInt + 2,
                             lhsInt + 3, lhsInt + 4, lhsInt + 5);
        U32At(enemy, 0x3328) |= 4U;
        break;
    case 60:
        SetPrimaryAnmScripts(enemy, instruction,
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
        SetExtraAnmScript(enemy, instruction);
        break;
    case 62:
        if (((U32At(enemy, 0x3328) >> 2) & 1U) == 0)
        {
            (*reinterpret_cast<AnmLoaded **>(
                reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCEEC))
                ->SetAndExecuteScriptIdx(
                reinterpret_cast<AnmVm *>(Bytes(enemy) + 0xC),
                I16At(enemy, 0x333C));
        }
        else
        {
            (*reinterpret_cast<AnmLoaded **>(
                reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCEF0))
                ->SetAndExecuteScriptIdx(
                reinterpret_cast<AnmVm *>(Bytes(enemy) + 0xC),
                I16At(enemy, 0x333C));
        }
        break;

    case 63:
        F32At(enemy, 0x2D34) = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        F32At(enemy, 0x2D38) = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        F32At(enemy, 0x2D3C) = 0.0f;
        reinterpret_cast<Enemy *>(enemy)->ClampPosition();
        break;
    case 64:
        EclHelpers::ConfigureRelativeMotion(enemy, instruction);
        break;
    case 65:
        F32At(enemy, 0x2D94) = AddNormalizeAngle(((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))), 0.0f);
        F32At(enemy, 0x2DA8) = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & ~0x3000U) | 0x1000U;
        I32At(enemy, 0x2DE8) = 0;
        *reinterpret_cast<ZunTimer *>(Bytes(enemy) + 0x2DDC) = 0;
        break;
    case 66:
        if (ReadInt(enemy, instruction, 0) <= 0)
        {
            F32At(enemy, 0x2D94) = AddNormalizeAngle(((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))), 0.0f);
            F32At(enemy, 0x2DA8) = ((instruction->operandFlags & (1U << 3)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
            U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & 0xFFFFCFFFU) | 0x1000U;
            I32At(enemy, 0x2DE8) = 0;
            *reinterpret_cast<ZunTimer *>(Bytes(enemy) + 0x2DDC) = 0;
        }
        else EclHelpers::ConfigurePolarMotion(enemy, instruction);
        break;
    case 67:
        BeginBoundaryAwareMove(enemy, instruction);
        break;
#ifdef TH08_ECL_RUN_LOW_BODY
    // Target physical order places opcode 178 between opcodes 67 and 68.
    case 178:
        FUN_004224a0(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx));
        break;
#endif
    case 68:
        F32At(enemy, 0x2D94) =
            AddNormalizeAngle(
                ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0))),
                g_Player.FUN_0044c1b0(
                    reinterpret_cast<Float3 *>(Bytes(enemy) + 0x2D34)));
        F32At(enemy, 0x2DA8) = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;
    case 69:
        if (ReadInt(enemy, instruction, 0) <= 0)
        {
            F32At(enemy, 0x2D94) =
                AddNormalizeAngle(
                    ((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2))),
                    g_Player.FUN_0044c1b0(
                        reinterpret_cast<Float3 *>(Bytes(enemy) + 0x2D34)));
            F32At(enemy, 0x2DA8) = ((instruction->operandFlags & (1U << 3)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
            U32At(enemy, 0x3324) =
                (U32At(enemy, 0x3324) & 0xFFFFCFFFU) | 0x1000U;
            // The target resolves operand 0 again before timer assignment.
            *reinterpret_cast<ZunTimer *>(Bytes(enemy) + 0x2DDC) =
                (I32At(enemy, 0x2DE8) = ReadInt(enemy, instruction, 0));
        }
        else
        {
            EclHelpers::ConfigurePolarMotion(enemy, instruction);
        }
        break;

    case 70:
        F32At(enemy, 0x2D98) = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & 0xFFFFCFFFU) | 0x1000U;
        break;
    case 71:
        F32At(enemy, 0x2DAC) = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        U32At(enemy, 0x3324) = (U32At(enemy, 0x3324) & 0xFFFFCFFFU) | 0x1000U;
        break;
    case 72:
        *reinterpret_cast<ZunTimer *>(Bytes(enemy) + 0x2DDC) =
            (I32At(enemy, 0x2DE8) = ReadInt(enemy, instruction, 0));
        F32At(enemy, 0x2DD0) = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        F32At(enemy, 0x2DD4) = ((instruction->operandFlags & (1U << 2))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        F32At(enemy, 0x2D9C) = ((instruction->operandFlags & (1U << 3))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        F32At(enemy, 0x2DA0) = ((instruction->operandFlags & (1U << 4))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 4)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 4)));
        F32At(enemy, 0x2DB0) = ((instruction->operandFlags & (1U << 5))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 5)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 5)));
        F32At(enemy, 0x2DB4) = ((instruction->operandFlags & (1U << 6))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 6)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 6)));
        U32At(enemy, 0x3324) |= 0x3000U;
        break;
    case 73:
        *reinterpret_cast<ZunTimer *>(Bytes(enemy) + 0x2DDC) =
            (I32At(enemy, 0x2DE8) = ReadInt(enemy, instruction, 0));
        *reinterpret_cast<D3DXVECTOR3 *>(
            reinterpret_cast<u8 *>(enemy) + 0x2DD0) =
            *reinterpret_cast<D3DXVECTOR3 *>(
                reinterpret_cast<u8 *>(enemy) + 0x2D34);
        F32At(enemy, 0x2D9C) = ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        F32At(enemy, 0x2DA0) = ((instruction->operandFlags & (1U << 2))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        F32At(enemy, 0x2DB0) = 0.0f;
        F32At(enemy, 0x2DB4) = ((instruction->operandFlags & (1U << 3))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        U32At(enemy, 0x3324) |= 0x3000U;
        break;
    case 74:
        *reinterpret_cast<ZunTimer *>(Bytes(enemy) + 0x2DDC) =
            (I32At(enemy, 0x2DE8) = ReadInt(enemy, instruction, 0));
        F32At(enemy, 0x2DA0) = ((instruction->operandFlags & (1U << 1)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        F32At(enemy, 0x2DB4) = ((instruction->operandFlags & (1U << 2)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        U32At(enemy, 0x3324) |= 0x3000U;
        break;
    case 75:
        F32At(enemy, 0x3340) = ((instruction->operandFlags & (1U << 0)) ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0))) : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        F32At(enemy, 0x3344) = ((instruction->operandFlags & (1U << 1))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        F32At(enemy, 0x3348) = ((instruction->operandFlags & (1U << 2))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 2)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 2)));
        F32At(enemy, 0x334C) = ((instruction->operandFlags & (1U << 3))
                ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 3)))
                : *reinterpret_cast<f32 *>(&RawInt(instruction, 3)));
        U32At(enemy, 0x3324) |= 0x80000U;
        break;
    case 76:
        U32At(enemy, 0x3324) &= ~0x80000U;
        break;
    case 77:
        F32At(enemy, 0x2D70) = ((instruction->operandFlags & (1U << 0))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        F32At(enemy, 0x2D74) = ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;
    case 78:
        F32At(enemy, 0x2D7C) = ((instruction->operandFlags & (1U << 0))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 0)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 0)));
        F32At(enemy, 0x2D80) = ((instruction->operandFlags & (1U << 1))
            ? enemy->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1)));
        break;

    case 79:
        lhsInt = ReadInt(enemy, instruction, 0);
        reinterpret_cast<LinkedChildFlags1 *>(Bytes(enemy) + 0x3324)->op79Bit6 = (lhsInt & 1) == 0;
        reinterpret_cast<LinkedChildFlags1 *>(Bytes(enemy) + 0x3324)->effectMirrored = (lhsInt & 2) == 0;
        reinterpret_cast<LinkedChildFlags1 *>(Bytes(enemy) + 0x3324)->op79Bit3 = (lhsInt & 4) == 0;
        reinterpret_cast<LinkedChildFlags1 *>(Bytes(enemy) + 0x3324)->op79Bit4 = (lhsInt & 8) != 0;
        reinterpret_cast<LinkedChildFlags1 *>(Bytes(enemy) + 0x3324)->op79Bit28 = (lhsInt & 0x10) != 0;
        reinterpret_cast<Op79Flags2 *>(Bytes(enemy) + 0x3328)->op79Bit6 = (lhsInt & 0x20) != 0;
        break;

    case 80:
        lhsInt = ReadInt(enemy, instruction, 0);
        if (lhsInt & 1) U32At(enemy, 0x3324) &= ~0x40U;
        if (lhsInt & 2)
        {
            U32At(enemy, 0x3324) &= ~0x4U;
            if (PointerAt(enemy, 0x53C8))
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(PointerAt(enemy, 0x53C8)) + 0x1F8) &= ~0x20000U;
        }
        if (lhsInt & 4) U32At(enemy, 0x3324) &= ~0x8U;
        if (lhsInt & 8) U32At(enemy, 0x3324) |= 0x10U;
        if (lhsInt & 0x10) U32At(enemy, 0x3324) |= 0x10000000U;
        if (lhsInt & 0x20) U32At(enemy, 0x3328) |= 0x40U;
        break;

    case 81:
        lhsInt = ReadInt(enemy, instruction, 0);
        if (lhsInt & 1) U32At(enemy, 0x3324) |= 0x40U;
        if (lhsInt & 2)
        {
            U32At(enemy, 0x3324) |= 0x4U;
            if (PointerAt(enemy, 0x53C8))
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(PointerAt(enemy, 0x53C8)) + 0x1F8) |= 0x20000U;
        }
        if (lhsInt & 4) U32At(enemy, 0x3324) |= 0x8U;
        if (lhsInt & 8) U32At(enemy, 0x3324) &= ~0x10U;
        if (lhsInt & 0x10) U32At(enemy, 0x3324) &= ~0x10000000U;
        if (lhsInt & 0x20) U32At(enemy, 0x3328) &= ~0x40U;
        break;

#if !defined(TH08_ECL_RUN_LOW_BODY)
    case 82:
        F32At(enemy, 0x3350) = ReadFloat(enemy, instruction, 0);
        F32At(enemy, 0x3350) *= F32At(enemy, 0x3350);
        break;
    case 83:
        U32At(enemy, 0x3328) =
            (U32At(enemy, 0x3328) & ~2U) | ((ReadInt(enemy, instruction, 0) & 1) << 1);
        break;
#endif

    case 84:
    case 85:
        break; // both target entries are 0x0041E7E4 (ordinary advance)

    case 86:
        *WriteInt(enemy, instruction, 0) =
            (instruction->operandFlags & 2U)
                ? EclOperands::ResolveInt(
                      g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 2)],
                      RawInt(instruction, 1))
                : RawInt(instruction, 1);
        break;

    case 87:
        if (g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 2)])
            *WriteFloat(enemy, instruction, 0) =
                (instruction->operandFlags & 2U)
                    ? g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 2)]->ResolveFloat(
                          *reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
                    : *reinterpret_cast<f32 *>(&RawInt(instruction, 1));
        break;

    case 88:
        lhsInt = ReadInt(enemy, instruction, 0);
        CallSubOnEnemy(
            g_EclEnemyTableF54CC0[lhsInt],
            *reinterpret_cast<EclRawInstruction **>(
                *reinterpret_cast<u8 **>(Bytes(g_EclEnemyTableF54CC0[lhsInt]) + 0x2CA0)),
            RawInt(instruction, 1));
        break;

    case 89:
        if (g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 0)])
        {
            // Target resolves operand 0 a second time before the store.
            I16At(g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 0)],
                  0x2D30) = static_cast<i16>(ReadInt(enemy, instruction, 1));
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
    case 90:
    {
        EclOperands::EnemyOverlay *tail = FindLinkedChildTail0041EFC0(enemy);
        EclOperands::EnemyOverlay *child =
            SpawnChildStandard0041F110(enemy, instruction);

        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_EnemyManager) +
                                    0x9DCEF8) == 0)
        {
            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                linkedChild = 1;
            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                isYoukai =
                g_Player.IsYoukai();
            *reinterpret_cast<u8 *>(Bytes(child) + 0x332F) =
                static_cast<u8>(((g_Player.IsYoukai()
                                  ? -1
                                  : 0) &
                                 -2) +
                                2);
            U32At(child, 0x3324) &= ~4U;

            if (PointerAt(child, 0x53C8) == 0)
            {
                PointerAt(child, 0x53C8) = g_EffectManager.SpawnEffect00425B70(
                    0x20,
                    reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D34),
                    1, -1);
                reinterpret_cast<AnmVmBase *>(
                    PointerAt(child, 0x53C8))
                    ->SetInterrupt(
                        g_Player.IsYoukai()
                            ? 2
                            : 1);
                reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))->flag17 =
                    reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) +
                                                          0x3324)->
                            effectMirrored != 0;
                if (U32At(child, 0x2E0C) & 1)
                {
                    reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))
                        ->angleVel.z =
                        -reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))
                             ->angleVel.z;
                }
            }

            PointerAt(child, 0x2DA4) = enemy;
            PointerAt(tail, 8) = child;
            PointerAt(child, 4) = tail;
            ++I32At(enemy, 0x3380);
        }

        g_SoundPlayer.PlaySoundPositionedByIdx(
            SOUND_FAMILIAR_SPAWN,
            *reinterpret_cast<f32 *>(Bytes(enemy) + 0x2D34));
        break;
    }
    case 91:
    {
        EclOperands::EnemyOverlay *tail = FindLinkedChildTail0041EFC0(enemy);
        EclOperands::EnemyOverlay *child =
            SpawnChildAlternate0041F280(enemy, instruction);

        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_EnemyManager) +
                                    0x9DCEF8) == 0)
        {
            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                linkedChild = 1;
            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                isYoukai =
                g_Player.IsYoukai();
            *reinterpret_cast<u8 *>(Bytes(child) + 0x332F) =
                static_cast<u8>(((g_Player.IsYoukai()
                                  ? -1
                                  : 0) &
                                 -2) +
                                2);
            U32At(child, 0x3324) &= ~4U;

            if (PointerAt(child, 0x53C8) == 0)
            {
                PointerAt(child, 0x53C8) = g_EffectManager.SpawnEffect00425B70(
                    0x20,
                    reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D34),
                    1, -1);
                reinterpret_cast<AnmVmBase *>(
                    PointerAt(child, 0x53C8))
                    ->SetInterrupt(
                        g_Player.IsYoukai()
                            ? 2
                            : 1);
                reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))->flag17 =
                    reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) +
                                                          0x3324)->
                            effectMirrored != 0;
                if (U32At(child, 0x2E0C) & 1)
                {
                    reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))
                        ->angleVel.z =
                        -reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))
                             ->angleVel.z;
                }
            }

            PointerAt(child, 0x2DA4) = enemy;
            PointerAt(tail, 8) = child;
            PointerAt(child, 4) = tail;
            ++I32At(enemy, 0x3380);
        }

        g_SoundPlayer.PlaySoundPositionedByIdx(
            SOUND_FAMILIAR_SPAWN,
            *reinterpret_cast<f32 *>(Bytes(enemy) + 0x2D34));
        break;
    }
    case 92:
    {
        EclOperands::EnemyOverlay *tail = FindLinkedChildTail0041EFC0(enemy);
        EclOperands::EnemyOverlay *child =
            SpawnChildStandard0041F110(enemy, instruction);

        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_EnemyManager) +
                                    0x9DCEF8) == 0)
        {
            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                linkedChild = 1;
            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                isYoukai =
                g_Player.IsYoukai();
            *reinterpret_cast<u8 *>(Bytes(child) + 0x332F) =
                static_cast<u8>(((g_Player.IsYoukai()
                                  ? -1
                                  : 0) &
                                 -2) +
                                2);

            *reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D40) =
                *reinterpret_cast<D3DXVECTOR3 *>(Bytes(enemy) + 0x2D34);
            *reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D88) =
                *reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D40) +
                *reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D34);
            U32At(child, 0x3324) &= ~4U;

            if (PointerAt(child, 0x53C8) == 0)
            {
                PointerAt(child, 0x53C8) = g_EffectManager.SpawnEffect00425B70(
                    0x20,
                    reinterpret_cast<D3DXVECTOR3 *>(Bytes(child) + 0x2D88),
                    1, -1);
                reinterpret_cast<AnmVmBase *>(
                    PointerAt(child, 0x53C8))
                    ->SetInterrupt(
                        g_Player.IsYoukai()
                            ? 2
                            : 1);
                reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))->flag17 =
                    reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) +
                                                          0x3324)->
                            effectMirrored != 0;
                if (U32At(child, 0x2E0C) & 1)
                {
                    reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))
                        ->angleVel.z =
                        -reinterpret_cast<AnmVm *>(PointerAt(child, 0x53C8))
                             ->angleVel.z;
                }
            }

            reinterpret_cast<LinkedChildFlags1 *>(Bytes(child) + 0x3324)->
                inheritParentPosition = 1;
            PointerAt(child, 0x2DA4) = enemy;
            PointerAt(tail, 8) = child;
            PointerAt(child, 4) = tail;
            ++I32At(enemy, 0x3380);
        }

        g_SoundPlayer.PlaySoundPositionedByIdx(
            SOUND_FAMILIAR_SPAWN,
            *reinterpret_cast<f32 *>(Bytes(enemy) + 0x2D34));
        break;
    }

#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
    default:
        TH08_ECL_RUN_LOW_YIELD(LOW_NOT_HANDLED, 0);
    }

    TH08_ECL_RUN_LOW_YIELD(LOW_ADVANCE, 0);
#endif

#ifdef TH08_ECL_RUN_LOW_BODY
#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
low_dispatch_complete: ;
#endif
#else
}

} // namespace EclRunLowProposal
} // namespace th08
#endif

#undef TH08_ECL_RUN_LOW_YIELD

#ifdef TH08_ECL_RUN_LOW_BODY
#undef TH08_ECL_RUN_LOW_YIELD_LOW_NOT_HANDLED
#undef TH08_ECL_RUN_LOW_YIELD_LOW_ADVANCE
#undef TH08_ECL_RUN_LOW_YIELD_LOW_SELECT_NEXT_CONTEXT
#undef TH08_ECL_RUN_LOW_YIELD_LOW_RESTART_RUN_LOOP
#undef TH08_ECL_RUN_LOW_YIELD_LOW_REDISPATCH
#undef TH08_ECL_RUN_LOW_YIELD_LOW_RETURN_MINUS_ONE
#undef TH08_ECL_RUN_LOW_YIELD_SELECT
#undef TH08_ECL_RUN_LOW_YIELD_SELECT_I
#undef WriteFloat
#undef WriteInt
#undef ReadFloat
#undef ReadInt
#undef RawFloat
#undef RawInt
#undef PointerAt
#undef F32At
#undef I16At
#undef I32At
#undef U32At
#undef Bytes
#endif

#endif // !TH08_ECL_RUN_DECLARATIONS_ONLY
