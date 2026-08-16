// TH08 1.00d RunEcl high-opcode reconstruction proposal.
//
// Scope owned by this proposal:
//   - real ECL opcodes 93..184 (the target switch indexes opcode - 1), and
//   - the shared post-dispatch tail at 0x0041E7E4..0x0041ECBD.
//
// This file is deliberately not included by a translation unit yet.  It is a
// typed integration proposal for the coordinator-owned EclRun.cpp.  Names in
// TargetApi are semantic hypotheses; addresses, access widths, constants,
// switch destinations, and byte offsets are target observations.  In
// particular, .analysis/ecl-decompile.c labels these cases 0x5c..0xb7 because
// Ghidra describes the normalized selector.  The case labels below restore
// the actual target opcode numbers and therefore run from 93 through 184.
//
// Observed jump-table destinations (table 0x0041ECBE, indexed by opcode - 1):
//   93:41D6DE  94:41D8A3  95:41DA7D  96-104:41B4DC
//   105:41B70F 106:41B7B1 107:41B866 108:41B883
//   109:41B8A0 110:41B8F1 111:41B529 112:41DAFC
//   113:41DB0B 114-115:41B983 116:41BCD3 117:41BD17
//   118:41BE61 119:41BF17 120:41C0F9 121:41C17F
//   122:41CB06 123:41CB16 124:41C6FB 125:41C849
//   126:41C7CC 127:41C443 128:41C627 129:41C74B
//   130:41C796 131:41C941 132:41CB26 133:41CB70
//   134:41CCFC 135:41CDF3 136:41D4B8 137:41D500
//   138:41B6DD 139:41CF86 140:41D027 141:41D5F5
//   142:41D24A 143:41D183 144:41D1C7 145:41D48C
//   146:41D5A8 147:41D646 148:41D686 149:41DA93
//   150:41DAD9 151:41DBF9 152:41DC25 153:41DDAE
//   154:41C254 155:41DDD6 156:41DE0C 157:41DE42
//   158:41C9EF 159:41C6B7 160:41DF60 161:41DFAA
//   162:41DFFB 163:41C402 164:41E00C 165:41E114
//   166:41E156 167:41BDC9 168:41D37A 169:41E28F
//   170:41C062 171:41C284 172:41C31C 173:41E359
//   174:41E589 175:41E650 176:41E40F 177:41E690
//   178:419FF0 179:41E6D4 180:41E6E3 181:41E6F2
//   182:41E74B 183:41E3B4 184:41E7A3

#ifndef TH08_ECL_RUN_HIGH_DECLARATIONS
#define TH08_ECL_RUN_HIGH_DECLARATIONS

namespace th08
{
extern u32 g_EnemyManagerUpdateManagerFlags;
extern void *g_EclExInsn[];
extern i32 g_EclGlobal004EA290; // target 0x004EA290
extern i32 g_EclGlobal00F54E2C; // target 0x00F54E2C
extern i32 g_EclGlobal004ECCA8; // target 0x004ECCA8
extern i32 g_EclGlobal00F54CEC; // target 0x00F54CEC
void __fastcall StartEnemySpell(u8 *enemy, void *instruction);
void __fastcall EndEnemySpell(u8 *enemy, void *instruction);

namespace EclRunHighProposal
{

struct Vec3
{
    f32 x;
    f32 y;
    f32 z;
};

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

// Provisional semantic name for target FUN_00422720.  Both its caller and
// callee establish Enemy in ECX and the current ECL instruction in EDX.
void __fastcall DispatchShotInstruction(u8 *enemy,
                                        RawInstruction *instruction);

struct SpawnPacket
{
    i32 values[7];
};

struct SpawnPacketTyped
{
    i32 type;
    D3DXVECTOR3 position;
    i32 arg4;
    i32 arg5;
    i32 arg6;
};

struct TargetZunTimerOverlay
{
    ZunBool AddAssign0041FDF0(i32 value);
};

// RunEcl invokes the per-context callback with Enemy in ECX and its opaque
// context argument in EDX.  Interpolator callbacks receive Enemy in ECX and
// their progress value on the stack.
typedef void (__fastcall *EclContextCallback)(Enemy *enemy, void *argument);
typedef void (Enemy::*EclInterpolatorCallback)(f32 progress);

struct Interpolator
{
    EclInterpolatorCallback callback; // +0x00, target calls with Enemy in ECX
    ZunTimer timer;                 // +0x04
    i32 duration;         // +0x10
    i32 unknown14;        // +0x14
    i32 easing;           // +0x18, accepted values 1..6
    i32 unknown1C;
    i32 unknown20;
    i32 unknown24;
    i32 unknown28;
    f32 affectedVariable; // +0x2C; 10042..10044 mean position motion
};
typedef char InterpolatorSizeCheck[sizeof(Interpolator) == 0x30 ? 1 : -1];

enum DispatchResult
{
    DISPATCH_ADVANCE,
    DISPATCH_ENTER_SUBROUTINE,
};

// The coordinator should bind these operations to the indicated target
// functions/globals.  Their descriptive names are provisional.
struct TargetApi
{
    i32 ResolveInt(u8 *enemy, i32 raw);                         // 0x0041F420
    f32 ResolveFloat(u8 *enemy, f32 raw);                       // 0x00420120
    i32 *ResolveIntLValue(u8 *enemy, i32 *raw, u16 flags, i32 idx); // 0x0041FE10
    f32 *ResolveFloatLValue(u8 *enemy, f32 *raw, u16 flags, i32 idx); // 0x00420950

    void TransformSpawnVector(Vec3 *value, const Vec3 *origin); // 0x00410A70
    void *SpawnFromPacket(i32 type, const Vec3 *position, i32 a,
                                  i32 b, i32 c, i32 *contextInts);       // 0x0042A680
    void ClearOrLimitBullets(i32 count, i32 mode);              // 0x0042EFB0
    void DispatchShotInstruction(u8 *enemy, RawInstruction *insn); // 0x00422720
    i32 RandomInt(i32 upper, i32 lower);                        // 0x00421BA0
    i32 ConvertTime(i32 value);                                // 0x00406EF0
    f32 RandomFloat();                                         // 0x0043ED50
    f32 RandomFloatInRange(f32 range);                          // 0x0040D390
    f32 AddNormalizeAngle(f32 a, f32 b);                        // 0x0043EDB0
    f32 AngleToPlayer(const Vec3 *position);                    // 0x0044C1B0
    f32 VectorAngle(f32 y, f32 x);                              // 0x0040C7B0

    void AddVectors(Vec3 *out, const Vec3 *left,
                            const Vec3 *right);                          // 0x00409080
    void FinalizeVectorState(void *state);                      // 0x00430E10
    void Call00415C60();
    void *CreateModeObject(void *state);                         // 0x00430F20
    void Call00421280(u8 *enemy);
    void Call004212E0(u8 *enemy);
    void Call004224A0(u8 *enemy);
    void Call00423130(i32 value);
    void Call0041FDF0(u8 *enemy, i32 value);
    void Call0041F0B0(i32 value);
    void Call0041F040(f32 a, f32 b, f32 c);
    void Call0041F0E0(i32 value);

    void SetTimer(void *timer, i32 value);                      // 0x004065F0
    void ResetTimer(void *timer, i32 value);                    // 0x00406640
    i32 TimerDone(void *timer, i32 duration);                   // 0x0040B8E0
    f32 TimerValue(void *timer);                                // 0x0040B8C0
    void *Allocate(i32 size, const char *tag);                  // 0x0040B880
    void Free(void *allocation);                               // 0x0040B8A0
    void InitializeEclContext(void *context, u16 subId);        // 0x00418450

    void PlayPositioned(i32 id, i32 xBits);                     // 0x0045D660
    void *SpawnEffect00425430(i32 id, const Vec3 *position,
                                      i32 count, i32 color);
    void *SpawnEffect00425B70(i32 id, const Vec3 *position,
                                      i32 count, i32 color);
    void *SpawnEffectWithVector(i32 id, const Vec3 *position,
                                        const Vec3 *vector, i32 count,
                                        i32 color);                     // 0x00425650
    void SpawnItem(const Vec3 *position, i32 type, i32 arg);    // 0x004400A0
    void ConfigureBoss(void *enemySubobject, void *state,
                               i32 ratioTimesTwo);                        // 0x004649A0

    void SetBossPresence(i32 value);                            // 0x00422C20
    void SetBossUiState(i32 slot, i32 state);                   // 0x00422BB0
    void UnregisterBoss(u8 *enemy);                             // 0x0042A820
    void SetBossHealth(f32 value);                              // 0x004230C0
    void SetBossMarker(i32 slot, const Vec3 *position);         // 0x00422BE0
    void SetBossGaugeSlot(i32 slot, f32 a, f32 b);              // 0x004230E0
    void SetBossGaugeValue(i32 slot, i32 value);                // 0x00423110
    i32 IsYoukai();                                            // 0x0040BC40
    void SelectPlayerMode(i32 value);                           // 0x00407120

    void SetAngleFromPosition(Vec3 *position, f32 angle);       // 0x00430D30
    void Call00430830(i32 value);
    void Call00439007();
    void Call004390D6();
    void Call00439093();
    void Call00439050();
    i8 GetGameState();                                         // 0x00406DD0
    void SetGameState(i32 value);                               // 0x00406DF0
    void PlaySound(i32 id, i32 arg);                            // 0x0045D550

    void CallFunctionTable(i32 index);                          // table 0x004C6CB0
    void *FunctionTableEntry(i32 index);
    bool PresentationWritesAllowed();                           // bits in 0x0164D0B4
    void SetBossSlot(i32 index, u8 *enemy);

    f32 PlayerX();                                             // 0x017D61AC
    i32 PlayerItemCount();                                     // 0x00422480 result
    i32 &Global004EA290();
    i32 &Global004ECCA8();
    i32 &Global00F54CEC();
    i32 &Global00F54E2C();
    i32 &Global0164D30C();

};

struct Context
{
    u8 *enemy;
    RawInstruction *instruction;
    TargetApi *api;
    i32 activeChildContext;

    template <typename T> T &At(i32 offset)
    {
        return *(T *)(enemy + offset);
    }

    template <typename T> T &At(void *base, i32 offset)
    {
        return *(T *)((u8 *)base + offset);
    }

    i32 RawI(i32 index) const
    {
        return *(i32 *)(instruction->operands + index * 4);
    }

    f32 RawF(i32 index) const
    {
        return *(f32 *)(instruction->operands + index * 4);
    }

    u8 RawByte(i32 byteOffset) const
    {
        return instruction->operands[byteOffset];
    }

    u16 RawU16(i32 byteOffset) const
    {
        return *(u16 *)(instruction->operands + byteOffset);
    }

    i32 ReadI16(i32 byteOffset, i32 flagIndex)
    {
        i32 raw = (i32)*(i16 *)(instruction->operands + byteOffset);
        return (instruction->operandFlags & (1 << flagIndex)) ? api->ResolveInt(enemy, raw) : raw;
    }

    i32 ReadI(i32 index)
    {
        i32 raw = RawI(index);
        return (instruction->operandFlags & (1 << index)) ? api->ResolveInt(enemy, raw) : raw;
    }

    f32 ReadF(i32 index)
    {
        f32 raw = RawF(index);
        return (instruction->operandFlags & (1 << index)) ? api->ResolveFloat(enemy, raw) : raw;
    }

    i32 *WriteI(i32 index)
    {
        return api->ResolveIntLValue(enemy, (i32 *)(instruction->operands + index * 4),
                                     instruction->operandFlags, index);
    }

    f32 *WriteF(i32 index)
    {
        return api->ResolveFloatLValue(enemy, (f32 *)(instruction->operands + index * 4),
                                       instruction->operandFlags, index);
    }

    u8 *CurrentEclContext()
    {
        return At<u8 *>(0x2CA0);
    }

    u8 *Object(i32 index)
    {
        return At<u8 *>(0x3280 + index * 4);
    }
};

#ifndef TH08_ECL_CONTEXT_ENEMY
#define TH08_ECL_CONTEXT_ENEMY(context) ((context).enemy)
#define TH08_ECL_CONTEXT_INSTRUCTION(context) ((context).instruction)
#define TH08_ECL_CONTEXT_API(context) ((context).api)
#define TH08_ECL_CONTEXT_CHILD(context) ((context).activeChildContext)
#endif

// RunEcl was built with /Ob0, and its target body accesses these overlays
// directly.  Named expressions preserve the recovered widths while avoiding
// hundreds of non-target Context/accessor COMDAT calls.
#define TH08_ECL_AT(ctx, type, offset) \
    (*reinterpret_cast<type *>(TH08_ECL_CONTEXT_ENEMY(ctx) + (offset)))
#define TH08_ECL_RAW_I(ctx, index) \
    (*reinterpret_cast<i32 *>(TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (index) * 4))
#define TH08_ECL_RAW_F(ctx, index) \
    (*reinterpret_cast<f32 *>(TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (index) * 4))
#define TH08_ECL_RAW_BYTE(ctx, byteOffset) \
    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands[(byteOffset)])
#define TH08_ECL_RAW_U16(ctx, byteOffset) \
    (*reinterpret_cast<u16 *>(TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (byteOffset)))
#define TH08_ECL_READ_I16(ctx, byteOffset, flagIndex) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (flagIndex))) \
         ? EclOperands::ResolveInt( \
               reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)), \
               static_cast<i32>(*reinterpret_cast<i16 *>( \
                   TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (byteOffset)))) \
         : static_cast<i32>(*reinterpret_cast<i16 *>( \
               TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (byteOffset))))
#define TH08_ECL_READ_I(ctx, index) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (index))) \
         ? EclOperands::ResolveInt( \
               reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)), \
               TH08_ECL_RAW_I((ctx), (index))) \
         : TH08_ECL_RAW_I((ctx), (index)))
#define TH08_ECL_READ_F(ctx, index) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (index))) \
         ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat( \
               TH08_ECL_RAW_F((ctx), (index))) \
         : TH08_ECL_RAW_F((ctx), (index)))
#define TH08_ECL_READ_F_RAWARG(ctx, index) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (index))) \
         ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat( \
               *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I((ctx), (index)))) \
         : TH08_ECL_RAW_F((ctx), (index)))

#define TH08_ECL_WRITE_I(ctx, index) \
    EclOperands::ResolveIntLValue( \
        reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)), \
        &TH08_ECL_RAW_I((ctx), (index)), TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags, (index))
#define TH08_ECL_WRITE_F(ctx, index) \
    EclOperands::ResolveFloatLValue( \
        reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)), \
        &TH08_ECL_RAW_F((ctx), (index)), TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags, (index))
#define TH08_ECL_CURRENT_CONTEXT(ctx) TH08_ECL_AT((ctx), u8 *, 0x2CA0)
#define TH08_ECL_OBJECT(ctx, index) TH08_ECL_AT((ctx), u8 *, 0x3280 + (index) * 4)
#define TH08_ECL_PRESENTATION_WRITES_ALLOWED() \
    ((((g_EnemyManagerUpdateManagerFlags >> 14) & 1) == 0) || \
     (((g_EnemyManagerUpdateManagerFlags >> 7) & 3) == 0))

} // namespace EclRunHighProposal
} // namespace th08

#endif // TH08_ECL_RUN_HIGH_DECLARATIONS

#if !defined(TH08_ECL_RUN_DECLARATIONS_ONLY)

#ifdef TH08_ECL_RUN_HIGH_BODY
#define TH08_ECL_RUN_HIGH_YIELD(value) \
    do { goto high_dispatch_complete; } while (0)
#else

namespace th08
{
namespace EclRunHighProposal
{

#define TH08_ECL_RUN_HIGH_YIELD(value) return (value)

static DispatchResult DispatchOpcode93To184(Context &ctx)
{
#endif

#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
    i32 lhsInt;
#endif

#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
    switch (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->opcode)
    {
#endif
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
        if (TH08_ECL_AT(ctx, i32, 0x2DFC) <= 0)
            break;
        if (((TH08_ECL_AT(ctx, u32, 0x3324) >> 17) & 1) == 1)
        {
            memcpy(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x3034, TH08_ECL_CONTEXT_INSTRUCTION(ctx), 11 * sizeof(i32));
            break;
        }
        DispatchShotInstruction(TH08_ECL_CONTEXT_ENEMY(ctx),
                                TH08_ECL_CONTEXT_INSTRUCTION(ctx));
        break;

    case 111:
    {
        f32 *entry = (f32 *)(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x2E44 + TH08_ECL_READ_I(ctx, 0) * 0x18);
        entry[4] = (f32)TH08_ECL_READ_I(ctx, 1);
        entry[5] = (f32)TH08_ECL_READ_I(ctx, 2);
        entry[2] = (f32)TH08_ECL_READ_I(ctx, 3);
        entry[3] = (f32)TH08_ECL_READ_I(ctx, 4);
        entry[0] = TH08_ECL_READ_F_RAWARG(ctx, 5);
        entry[1] = TH08_ECL_READ_F_RAWARG(ctx, 6);
        break;
    }
    case 138:
        TH08_ECL_AT(ctx, u8, 0x3310) = TH08_ECL_RAW_BYTE(ctx, 0);
        TH08_ECL_AT(ctx, u8, 0x3311) = TH08_ECL_RAW_BYTE(ctx, 1);
        TH08_ECL_AT(ctx, u8, 0x3312) = TH08_ECL_RAW_BYTE(ctx, 2);
        break;

    case 105:
        TH08_ECL_AT(ctx, i32, 0x3060) = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_AT(ctx, i32, 0x3060) != 0)
        {
            lhsInt = TH08_ECL_AT(ctx, i32, 0x3060);
            TH08_ECL_AT(ctx, i32, 0x3060) += g_GameManager.ScaleIntBasedOnRank(lhsInt / 5, -lhsInt / 5);
            *reinterpret_cast<ZunTimer *>(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x3064) = 0;
        }
        break;
    case 106:
        TH08_ECL_AT(ctx, i32, 0x3060) = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_AT(ctx, i32, 0x3060) != 0)
        {
            lhsInt = TH08_ECL_AT(ctx, i32, 0x3060);
            TH08_ECL_AT(ctx, i32, 0x3060) += g_GameManager.ScaleIntBasedOnRank(lhsInt / 5, -lhsInt / 5);
            *reinterpret_cast<ZunTimer *>(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x3064) =
                g_Rng.GetRandomU32InRange(TH08_ECL_AT(ctx, i32, 0x3060));
        }
        break;
    case 107: TH08_ECL_AT(ctx, u32, 0x3324) |= 0x00020000; break;
    case 108: TH08_ECL_AT(ctx, u32, 0x3324) &= ~0x00020000; break;

    case 109:
    {
        D3DXVECTOR3 sum =
            *reinterpret_cast<D3DXVECTOR3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D34)) +
            *reinterpret_cast<D3DXVECTOR3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2DB8));
        TH08_ECL_AT(ctx, Vec3, 0x2E28) = *reinterpret_cast<Vec3 *>(&sum);
        reinterpret_cast<TargetApi *>(&g_BulletManager)->FinalizeVectorState(
            TH08_ECL_CONTEXT_ENEMY(ctx) + 0x2E24);
        break;
    }
    case 110:
        TH08_ECL_AT(ctx, f32, 0x2DB8) = TH08_ECL_READ_F_RAWARG(ctx, 0);
        TH08_ECL_AT(ctx, f32, 0x2DBC) = TH08_ECL_READ_F_RAWARG(ctx, 1);
        TH08_ECL_AT(ctx, i32, 0x2DC0) = 0;
        break;

    case 114:
    case 115:
    {
        u8 *state = TH08_ECL_CONTEXT_ENEMY(ctx) + 0x3070;
        Vec3 transformed;
        TH08_ECL_CONTEXT_API(ctx)->AddVectors(&transformed, &TH08_ECL_AT(ctx, Vec3, 0x2D88), &TH08_ECL_AT(ctx, Vec3, 0x2DB8));
        *(u16 *)(state + 0x00) = TH08_ECL_RAW_U16(ctx, 0);
        *(u16 *)(state + 0x02) = (u16)TH08_ECL_READ_I16(ctx, 2, 1);
        *(Vec3 *)(state + 0x04) = transformed;
        *(f32 *)(state + 0x10) = TH08_ECL_READ_F(ctx, 1);
        *(f32 *)(state + 0x18) = TH08_ECL_READ_F(ctx, 2);
        *(f32 *)(state + 0x1D0) = TH08_ECL_READ_F(ctx, 3);
        *(f32 *)(state + 0x1D4) = TH08_ECL_READ_F(ctx, 4);
        *(f32 *)(state + 0x1D8) = TH08_ECL_READ_F(ctx, 5);
        *(f32 *)(state + 0x1DC) = TH08_ECL_READ_F(ctx, 6);
        *(i32 *)(state + 0x1E0) = TH08_ECL_READ_I(ctx, 7);
        *(i32 *)(state + 0x1E4) = TH08_ECL_READ_I(ctx, 8);
        *(i32 *)(state + 0x1E8) = TH08_ECL_READ_I(ctx, 9);
        *(i32 *)(state + 0x1EC) = TH08_ECL_RAW_I(ctx, 10);
        *(i32 *)(state + 0x1F0) = TH08_ECL_RAW_I(ctx, 11);
        *(i32 *)(state + 0x1FC) = TH08_ECL_RAW_I(ctx, 12);
        *(u16 *)(state + 0x1F8) = TH08_ECL_CONTEXT_INSTRUCTION(ctx)->opcode == 115 ? 0 : 1;
        TH08_ECL_AT(ctx, void *, 0x3280 + TH08_ECL_AT(ctx, i32, 0x3300) * 4) = TH08_ECL_CONTEXT_API(ctx)->CreateModeObject(state);
        break;
    }
    case 116: TH08_ECL_AT(ctx, i32, 0x3300) = TH08_ECL_READ_I(ctx, 0); break;
    case 117:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x554) =
                AddNormalizeAngle(
                    *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x554),
                    TH08_ECL_READ_F_RAWARG(ctx, 1));
        break;
    case 167:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x554) =
                TH08_ECL_READ_F_RAWARG(ctx, 1);
        break;
    case 118:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x554) =
                EclOperands::g_TargetPlayer017D5EF8.AngleToPlayer(
                    reinterpret_cast<EclOperands::TargetVector3 *>(
                        TH08_ECL_OBJECT(ctx, lhsInt) + 0x548)) +
                TH08_ECL_READ_F_RAWARG(ctx, 1);
        break;
    case 119:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
        {
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x548) = TH08_ECL_AT(ctx, f32, 0x2D88) + TH08_ECL_READ_F_RAWARG(ctx, 1);
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x54C) = TH08_ECL_AT(ctx, f32, 0x2D8C) + TH08_ECL_READ_F_RAWARG(ctx, 2);
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x550) = TH08_ECL_AT(ctx, f32, 0x2D90) + TH08_ECL_READ_F_RAWARG(ctx, 3);
        }
        break;
    case 170:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
            *(u8 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x599) =
                (u8)TH08_ECL_READ_I(ctx, 1);
        break;
    case 120:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt) &&
            *(i32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x584))
            *(i32 *)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x60) = 1;
        else
            *(i32 *)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x60) = 0;
        break;
    case 121:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt) &&
            *(i32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x584) &&
            *(u8 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x598) < 2)
        {
            *(u8 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x598) = 2;
            *reinterpret_cast<ZunTimer *>(TH08_ECL_OBJECT(ctx, lhsInt) + 0x588) = 0;
            *(i32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x564) =
                *(i32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x568);
        }
        break;
    case 154:
        for (i32 i = 0; i < 0x20; ++i)
            TH08_ECL_AT(ctx, void *, 0x3280 + i * 4) = 0;
        break;
    case 171:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x560) =
                TH08_ECL_READ_F_RAWARG(ctx, 1);
        break;
    case 172:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_OBJECT(ctx, lhsInt))
        {
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x558) =
                TH08_ECL_READ_F_RAWARG(ctx, 1);
            *(f32 *)(TH08_ECL_OBJECT(ctx, lhsInt) + 0x55C) =
                TH08_ECL_READ_F_RAWARG(ctx, 2);
        }
        break;
    case 163: g_EclGlobal00F54CEC = TH08_ECL_READ_I(ctx, 0); break;
    case 127:
        if (TH08_ECL_READ_I(ctx, 0) < 0)
        {
            if (TH08_ECL_AT(ctx, u8, 0x3313) < 4)
                g_Gui.FUN_00422c20(0);
            EclRunLowProposal::g_EclEnemyTableF54CC0[
                TH08_ECL_AT(ctx, u8, 0x3313)] = 0;
            TH08_ECL_AT(ctx, u32, 0x3324) &= ~2U;
            g_AsciiManager.FUN_00422bb0(
                TH08_ECL_AT(ctx, u8, 0x3313), 2);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->FUN_0042a820();
            D3DXVECTOR3 offscreen(-999.0f, -999.0f, 0.0f);
            g_AsciiManager.SetBossMarkerPosition(
                TH08_ECL_AT(ctx, u8, 0x3313), &offscreen);
        }
        else
        {
            EclRunLowProposal::g_EclEnemyTableF54CC0[TH08_ECL_READ_I(ctx, 0)] =
                reinterpret_cast<EclOperands::EnemyOverlay *>(
                    TH08_ECL_CONTEXT_ENEMY(ctx));
            if (TH08_ECL_READ_I(ctx, 0) == 0)
            {
                g_Gui.FUN_00422c20(1);
                g_Gui.FUN_004230c0(1.0f);
            }
            TH08_ECL_AT(ctx, u32, 0x3324) |= 2;
            TH08_ECL_AT(ctx, u8, 0x3313) = (u8)TH08_ECL_READ_I(ctx, 0);
            g_AsciiManager.FUN_00422bb0(TH08_ECL_AT(ctx, u8, 0x3313), 1);
            TH08_ECL_AT(ctx, i32, 0x3350) = 0;
        }
        break;

    case 128:
    {
        TH08_ECL_AT(ctx, u8 *, 0x5360 + TH08_ECL_AT(ctx, i32, 0x53C0) * 4) =
            reinterpret_cast<u8 *>(g_EffectManager.SpawnEffect(
                13, reinterpret_cast<D3DXVECTOR3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D34)),
                1, 0xFF6060D0));
        u8 *operands = TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands;
        *reinterpret_cast<Vec3 *>(
            TH08_ECL_AT(ctx, u8 *, 0x5360 + TH08_ECL_AT(ctx, i32, 0x53C0) * 4) +
            0x2EC) = *reinterpret_cast<Vec3 *>(operands + 4);
        TH08_ECL_AT(ctx, i32, 0x53C4) = *reinterpret_cast<i32 *>(operands + 0x10);
        TH08_ECL_AT(ctx, i32, 0x53C0)++;
        break;
    }
    case 159: TH08_ECL_AT(ctx, u8, 0x332F) = (u8)TH08_ECL_READ_I(ctx, 0); break;
    case 124: reinterpret_cast<TargetApi *>(&g_SoundPlayer)->PlayPositioned(TH08_ECL_READ_I(ctx, 0), TH08_ECL_AT(ctx, i32, 0x2D34)); break;
    case 129:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
            TH08_ECL_AT(ctx, u32, 0x3324) = ((TH08_ECL_RAW_BYTE(ctx, 0) & 7) << 20) | (TH08_ECL_AT(ctx, u32, 0x3324) & 0xFF8FFFFF);
        break;
    case 130:
        if ((((g_EnemyManagerUpdateManagerFlags >> 14) & 1) != 1) ||
            (((g_EnemyManagerUpdateManagerFlags >> 7) & 3) == 0))
            TH08_ECL_AT(ctx, u16, 0x2CEE) = TH08_ECL_RAW_U16(ctx, 0);
        break;
    case 126:
        TH08_ECL_AT(ctx, i16, 0x2CF0 + TH08_ECL_READ_I(ctx, 1) * 2) = (i16)TH08_ECL_READ_I(ctx, 0);
        break;
    case 125:
        TH08_ECL_AT(ctx, i16, 0x2D30) = (i16)TH08_ECL_READ_I(ctx, 0);
#ifdef TH08_ECL_RUN_HIGH_BODY
enter_subroutine:
        // Target 0x0041C88A is shared by opcode 125 and the pending-subroutine
        // check at the top of RunEcl's dispatch loop.  Keeping it lexical at
        // this case preserves the target's handler ordering.
        *(RawInstruction **)TH08_ECL_CURRENT_CONTEXT(ctx) =
            (RawInstruction *)((u8 *)instruction + instruction->nextOffset);

        if (((TH08_ECL_AT(ctx, u32, 0x3324) >> 26) & 1) != 1)
        {
            memcpy(TH08_ECL_AT(ctx, u8 *, 0x2CA4) +
                       TH08_ECL_AT(ctx, i16, 0x2CEA) * 0x228,
                   TH08_ECL_CONTEXT_ENEMY(ctx) + 0x7F8,
                   0x8A * sizeof(i32));
        }

        reinterpret_cast<TargetApi *>(0x004ECCB8)->InitializeEclContext(
            TH08_ECL_CONTEXT_ENEMY(ctx) + 0x7F8,
            TH08_ECL_AT(ctx, u16, 0x2CF0 +
                TH08_ECL_AT(ctx, i16, 0x2D30) * 2));
        if (TH08_ECL_AT(ctx, i16, 0x2CEA) < 15)
            ++TH08_ECL_AT(ctx, i16, 0x2CEA);
        TH08_ECL_AT(ctx, i16, 0x2D30) = -1;
        goto restart_context;
#else
        TH08_ECL_RUN_HIGH_YIELD(DISPATCH_ENTER_SUBROUTINE);
#endif
    case 131:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        TH08_ECL_AT(ctx, i32, 0x2E00) = lhsInt;
        TH08_ECL_AT(ctx, i32, 0x2DFC) = lhsInt;
        TH08_ECL_AT(ctx, i32, 0x2E04) = lhsInt;
        if (TH08_ECL_AT(ctx, u8, 0x3313) == 0 && (((TH08_ECL_AT(ctx, u32, 0x3324) >> 1) & 1) != 0))
            for (i32 i = 0; i < 8; ++i)
                reinterpret_cast<TargetApi *>(&g_Gui)->SetBossGaugeSlot(i, 0.0f, 0.0f);
        break;
    case 158:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        reinterpret_cast<TargetApi *>(&g_Gui)->SetBossGaugeSlot(lhsInt, (f32)TH08_ECL_READ_I(ctx, 1) / (f32)TH08_ECL_AT(ctx, i32, 0x2E00),
                                                     (f32)TH08_ECL_READ_I(ctx, 2) / (f32)TH08_ECL_AT(ctx, i32, 0x2E00));
        reinterpret_cast<TargetApi *>(&g_Gui)->SetBossGaugeValue(lhsInt, TH08_ECL_READ_I(ctx, 3));
        break;
    case 122: StartEnemySpell(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx)); break;
    case 123: EndEnemySpell(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx)); break;
    case 132: *reinterpret_cast<ZunTimer *>(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x2E14) = TH08_ECL_READ_I(ctx, 0); break;
    case 133:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
        {
            TH08_ECL_AT(ctx, i32, 0x3358 + TH08_ECL_READ_I(ctx, 0) * 4) =
                TH08_ECL_READ_I(ctx, 1);
            TH08_ECL_AT(ctx, i32, 0x3368 + TH08_ECL_READ_I(ctx, 0) * 4) =
                TH08_ECL_READ_I(ctx, 2);
        }
        else
            TH08_ECL_AT(ctx, i32, 0x3358 + TH08_ECL_READ_I(ctx, 0) * 4) =
                TH08_ECL_READ_I(ctx, 1);
        break;
    case 134:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
        {
            TH08_ECL_AT(ctx, i32, 0x3378) = TH08_ECL_READ_I(ctx, 0);
            TH08_ECL_AT(ctx, i32, 0x337C) = TH08_ECL_READ_I(ctx, 1);
        }
        else
            TH08_ECL_AT(ctx, i32, 0x3378) = TH08_ECL_READ_I(ctx, 0);
        *reinterpret_cast<ZunTimer *>(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x2E14) = 0;
        break;

    case 135:
    {
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        u8 *oldContext = TH08_ECL_AT(ctx, u8 *, 0x3384 + lhsInt * 4);
        if (oldContext)
            g_ZunMemory.Free(oldContext);
        TH08_ECL_AT(ctx, u8 *, 0x3384 + lhsInt * 4) = 0;

        if (TH08_ECL_READ_I(ctx, 1) >= 0)
        {
            u8 *child = (u8 *)g_ZunMemory.Alloc(0x24B0, "ECLInt");
            TH08_ECL_AT(ctx, u8 *, 0x3384 + lhsInt * 4) = child;
            if (child)
            {
                for (i32 *clear = (i32 *)child, *end = (i32 *)child + 0x92C;
                     clear < end; ++clear)
                    *clear = 0;
                *(i32 *)child = TH08_ECL_READ_I(ctx, 1);
                TH08_ECL_CONTEXT_API(ctx)->InitializeEclContext(child + 8, *(u16 *)child);
                memcpy(child + 0x20, TH08_ECL_CURRENT_CONTEXT(ctx) + 0x18,
                       0x1E * sizeof(i32));
            }
        }
        break;
    }
    case 139:
        g_EffectManager.SpawnEffect(TH08_ECL_READ_I(ctx, 0), reinterpret_cast<D3DXVECTOR3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D34)),
                                     TH08_ECL_READ_I(ctx, 1), *TH08_ECL_WRITE_I(ctx, 2));
        break;
    case 140:
    {
        Float3 vector;
        if (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
            vector.x = reinterpret_cast<EclOperands::EnemyOverlay *>(
                TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                    *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        else
            *reinterpret_cast<i32 *>(&vector.x) = TH08_ECL_RAW_I(ctx, 3);
        if (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 4))
            vector.y = reinterpret_cast<EclOperands::EnemyOverlay *>(
                TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                    *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 4)));
        else
            *reinterpret_cast<i32 *>(&vector.y) = TH08_ECL_RAW_I(ctx, 4);
        vector.z = TH08_ECL_READ_F(ctx, 5);
        g_EffectManager.SpawnEffectAngle(TH08_ECL_READ_I(ctx, 0), reinterpret_cast<D3DXVECTOR3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D34)), reinterpret_cast<D3DXVECTOR3 *>(static_cast<f32 *>(vector)),
                                       TH08_ECL_READ_I(ctx, 1), *TH08_ECL_WRITE_I(ctx, 2));
        break;
    }
    case 143: TH08_ECL_AT(ctx, i32, 0x3304) = TH08_ECL_READ_I(ctx, 0); break;
    case 144:
        TH08_ECL_AT(ctx, i32, 0x3308) = TH08_ECL_READ_I(ctx, 0);
        TH08_ECL_AT(ctx, i32, 0x330C) = TH08_ECL_READ_I(ctx, 1);
        break;
    case 142:
    {
        i32 count = TH08_ECL_READ_I(ctx, 0);
        for (i32 i = 0; i < count; ++i)
        {
            Vec3 position = TH08_ECL_AT(ctx, Vec3, 0x2D34);
            position.x += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            position.y += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            i32 playerPower = g_GameManager.GetPower();
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&position),
                                    static_cast<ItemType>(playerPower < 0x80 ? ((i == 0) * 2) : 1), 0);
        }
        break;
    }
    case 168:
    {
        Float3 position;
        i32 count = TH08_ECL_READ_I(ctx, 0);
        for (i32 i = 0; i < count; ++i)
        {
            position = *reinterpret_cast<Float3 *>(
                &TH08_ECL_AT(ctx, Vec3, 0x2D34));
            position.x += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            ((f32 *)position)[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&position), static_cast<ItemType>(1), 0);
        }
        break;
    }
    case 145:
        TH08_ECL_AT(ctx, u32, 0x3324) = ((TH08_ECL_RAW_BYTE(ctx, 0) & 1) << 25) | (TH08_ECL_AT(ctx, u32, 0x3324) & 0xFDFFFFFF);
        break;
    case 136: reinterpret_cast<void (__fastcall *)(u8 *, RawInstruction *)>(g_EclExInsn[TH08_ECL_READ_I(ctx, 0)])(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx)); break;
    case 137:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            *(void **)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x10) = g_EclExInsn[TH08_ECL_READ_I(ctx, 0)];
            *(RawInstruction **)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x14) = TH08_ECL_CONTEXT_INSTRUCTION(ctx);
        }
        else
            *(void **)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x10) = 0;
        break;
    case 146:
        reinterpret_cast<TargetZunTimerOverlay *>(TH08_ECL_CURRENT_CONTEXT(ctx) + 4)->AddAssign0041FDF0(TH08_ECL_READ_I(ctx, 0));
        break;
    case 141: g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D34)), static_cast<ItemType>(TH08_ECL_READ_I(ctx, 0)), 0); break;
    case 147: g_EclGlobal004EA290 = TH08_ECL_READ_I(ctx, 0); break;
    case 148:
        g_Gui.FUN_00423130(TH08_ECL_READ_I(ctx, 0));
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3E04) += 0x708;
        break;
    case 93:
        if (TH08_ECL_AT(ctx, i32, 0x2DFC) > 0)
        {
            SpawnPacketTyped packet;
            memcpy(&packet, TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands, sizeof(packet));
            D3DXVECTOR3 position;
            position.x = TH08_ECL_READ_F_RAWARG(ctx, 1);
            position.y = TH08_ECL_READ_F_RAWARG(ctx, 2);
            position.z = TH08_ECL_READ_F_RAWARG(ctx, 3);
            void *spawned = g_EnemyManager.SpawnEnemy2(packet.type, &position,
                                      TH08_ECL_READ_I(ctx, 4), TH08_ECL_READ_I(ctx, 5),
                                      TH08_ECL_READ_I(ctx, 6),
                                      (i32 *)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x18));
            (void)spawned;
        }
        break;
    case 94:
        if (TH08_ECL_AT(ctx, i32, 0x2DFC) > 0)
        {
            SpawnPacketTyped packet;
            memcpy(&packet, TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands, sizeof(packet));
            D3DXVECTOR3 position;
            position.x = TH08_ECL_READ_F_RAWARG(ctx, 1);
            position.y = TH08_ECL_READ_F_RAWARG(ctx, 2);
            position.z = TH08_ECL_READ_F_RAWARG(ctx, 3);
            Float3 *positionResult = reinterpret_cast<Float3 *>(&position)->operator+=(
                *reinterpret_cast<Float3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D34)));
            g_EnemyManager.SpawnEnemy2(packet.type, reinterpret_cast<D3DXVECTOR3 *>(positionResult),
                                      TH08_ECL_READ_I(ctx, 4), TH08_ECL_READ_I(ctx, 5),
                                      TH08_ECL_READ_I(ctx, 6),
                                      (i32 *)(TH08_ECL_CURRENT_CONTEXT(ctx) + 0x18));
        }
        break;
    case 95:
        g_EnemyManager.FUN_0042efb0(8000, 0);
        break;
    case 149: TH08_ECL_AT(ctx, u16, 0x20A) = (u16)TH08_ECL_READ_I(ctx, 0); break;
    case 150:
        TH08_ECL_AT(ctx, u16, 0x4AE + TH08_ECL_RAW_I(ctx, 0) * 0x2A4) = TH08_ECL_RAW_U16(ctx, 4);
        break;
    case 112: g_BulletManager.bulletmanager_fun_00415c60(); break;

    case 113:
        if (TH08_ECL_READ_I(ctx, 0) < 0)
            TH08_ECL_AT(ctx, u32, 0x3020) &= ~0x200;
        else
        {
            TH08_ECL_AT(ctx, i32, 0x3024) = TH08_ECL_READ_I(ctx, 0);
            TH08_ECL_AT(ctx, u32, 0x3020) |= 0x200;
        }
        TH08_ECL_AT(ctx, i32, 0x3028) = TH08_ECL_READ_I(ctx, 1);
        break;
    case 151:
        TH08_ECL_AT(ctx, u32, 0x3324) =
            ((TH08_ECL_RAW_BYTE(ctx, 0) & 1) << 26) |
            (TH08_ECL_AT(ctx, u32, 0x3324) & 0xFBFFFFFF);
        break;
    case 152:
        TH08_ECL_AT(ctx, f32, 0x2DEC) = TH08_ECL_READ_F_RAWARG(ctx, 0);
        TH08_ECL_AT(ctx, f32, 0x2DF0) = TH08_ECL_READ_F_RAWARG(ctx, 1);
        TH08_ECL_AT(ctx, u16, 0x2DF4) = (u16)TH08_ECL_READ_I(ctx, 2);
        TH08_ECL_AT(ctx, u16, 0x2DF6) = (u16)TH08_ECL_READ_I(ctx, 3);
        TH08_ECL_AT(ctx, u16, 0x2DF8) = (u16)TH08_ECL_READ_I(ctx, 4);
        TH08_ECL_AT(ctx, u16, 0x2DFA) = (u16)TH08_ECL_READ_I(ctx, 5);
        break;
    case 153:
        TH08_ECL_AT(ctx, i32, 0x337C) = (i32)TH08_ECL_AT(ctx, i16, 0x2CEE);
        *reinterpret_cast<ZunTimer *>(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x2E14) = 0;
        break;
    case 155:
        TH08_ECL_AT(ctx, u32, 0x3324) =
            ((TH08_ECL_RAW_BYTE(ctx, 0) & 1) << 27) |
            (TH08_ECL_AT(ctx, u32, 0x3324) & 0xF7FFFFFF);
        g_EclGlobal004ECCA8 = 0x05F5E0F6;
        break;
    case 156:
        TH08_ECL_AT(ctx, u32, 0x3324) =
            ((TH08_ECL_RAW_BYTE(ctx, 0) & 1) << 7) |
            (TH08_ECL_AT(ctx, u32, 0x3324) & 0xFFFFFF7F);
        TH08_ECL_AT(ctx, u8, 0x332F) = 2;
        break;
    case 157:
        TH08_ECL_AT(ctx, u8, 0x534C) = TH08_ECL_RAW_BYTE(ctx, 0);
        TH08_ECL_AT(ctx, u16, 0x534E) = (u16)TH08_ECL_READ_I(ctx, 1);
        TH08_ECL_AT(ctx, u16, 0x5350) = (u16)TH08_ECL_READ_I(ctx, 2);
        TH08_ECL_AT(ctx, u16, 0x5352) = (u16)TH08_ECL_READ_I(ctx, 3);
        if (TH08_ECL_AT(ctx, u8, 0x534C) & 8)
            TH08_ECL_CONTEXT_API(ctx)->ConfigureBoss(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x0C, TH08_ECL_CONTEXT_ENEMY(ctx) + 0x3E14,
                                   (TH08_ECL_AT(ctx, i16, 0x534E) / TH08_ECL_AT(ctx, i16, 0x5352)) << 1);
        break;
    case 160: *reinterpret_cast<ZunTimer *>(TH08_ECL_CONTEXT_ENEMY(ctx) + 0x5354) = TH08_ECL_READ_I(ctx, 0); break;
    case 161:
        reinterpret_cast<TargetApi *>(&g_BulletManager)->SetAngleFromPosition(
            &TH08_ECL_AT(ctx, Vec3, 0x2D88), TH08_ECL_READ_F_RAWARG(ctx, 0));
        break;
    case 162: g_BulletManager.RemoveAllBullets(4); break;
    case 164:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        reinterpret_cast<TargetApi *>(&g_Spellcard)->Call0041F0B0(lhsInt);
        if (lhsInt == 0)
            reinterpret_cast<TargetApi *>(&g_Spellcard)->Call0041F040(TH08_ECL_READ_F_RAWARG(ctx, 1), TH08_ECL_READ_F_RAWARG(ctx, 2), TH08_ECL_READ_F_RAWARG(ctx, 3));
        break;
    case 165: TH08_ECL_AT(ctx, f32, 0x14) = TH08_ECL_READ_F_RAWARG(ctx, 0); break;
    case 166:
        *TH08_ECL_WRITE_F(ctx, 1) =
            sinf(TH08_ECL_READ_F_RAWARG(ctx, 2)) * TH08_ECL_READ_F_RAWARG(ctx, 3);
        *TH08_ECL_WRITE_F(ctx, 0) =
            cosf(TH08_ECL_READ_F_RAWARG(ctx, 2)) *
            ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                 ? reinterpret_cast<EclOperands::EnemyOverlay *>(
                       TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                       *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
                 : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        break;
    case 169:
        if (
            TH08_ECL_AT(ctx, f32, 0x2D34) >
                (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4)) &&
            96.0f < TH08_ECL_AT(ctx, f32, 0x2D34) ||
            288.0f < TH08_ECL_AT(ctx, f32, 0x2D34))
            *TH08_ECL_WRITE_F(ctx, 0) = AddNormalizeAngle(
                g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
        else
            *TH08_ECL_WRITE_F(ctx, 0) =
                g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
        break;
    case 173:
        TH08_ECL_AT(ctx, u32, 0x3324) = (TH08_ECL_AT(ctx, u32, 0x3324) & 0xBFFFFFFF) | ((TH08_ECL_READ_I(ctx, 0) & 1) << 30);
        break;
    case 183:
        TH08_ECL_AT(ctx, u32, 0x3324) = (TH08_ECL_AT(ctx, u32, 0x3324) & 0x7FFFFFFF) | ((u32)(TH08_ECL_READ_I(ctx, 0) & 1) << 31);
        break;
    case 176:
    {
        *reinterpret_cast<u32 *>(&g_GameManager.flags) =
            (*reinterpret_cast<u32 *>(&g_GameManager.flags) & ~0x180U) | 0x80U;
        *reinterpret_cast<u32 *>(&g_GameManager.flags) &= ~0x2000U;
        if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 14) & 1) == 0)
        {
            if (g_GameManager.currentStage == 6 || g_GameManager.currentStage == 7)
                *reinterpret_cast<u32 *>(&g_GameManager.flags) |= 0x2000U;
        }
        else if ((g_GameManager.currentSpellCardNumber >= 0x8F &&
                  g_GameManager.currentSpellCardNumber <= 0x92) ||
                 (g_GameManager.currentSpellCardNumber >= 0xAB &&
                  g_GameManager.currentSpellCardNumber <= 0xBE))
            *reinterpret_cast<u32 *>(&g_GameManager.flags) |= 0x2000U;
        TH08_ECL_AT(ctx, u32, 0x3324) |= 0x40000000;
        break;
    }
#ifdef TH08_ECL_RUN_HIGH_BODY
    // Opcodes 82 and 83 are emitted here in integrated RunEcl to reproduce
    // the target's late physical handler order.  Their standalone low-opcode
    // forms remain in EclRunLow.inl for source ownership and audit coverage.
    case 82:
        TH08_ECL_AT(ctx, f32, 0x3350) =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & 1U)
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(
                      TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)))
                : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0));
        TH08_ECL_AT(ctx, f32, 0x3350) *= TH08_ECL_AT(ctx, f32, 0x3350);
        break;
    case 83:
        TH08_ECL_AT(ctx, u32, 0x3328) =
            (TH08_ECL_AT(ctx, u32, 0x3328) & ~2U) |
            ((TH08_ECL_READ_I(ctx, 0) & 1) << 1);
        break;
#endif
    case 174:
    {
        if (TH08_ECL_AT(ctx, u8 *, 0x53C8))
            *(u8 *)(TH08_ECL_AT(ctx, u8 *, 0x53C8) + 0x350) = 0;
        TH08_ECL_AT(ctx, u8 *, 0x53C8) =
            reinterpret_cast<u8 *>(g_EffectManager.SpawnEffect00425B70(
                TH08_ECL_READ_I(ctx, 0) + 0x20,
                reinterpret_cast<D3DXVECTOR3 *>(&TH08_ECL_AT(ctx, Vec3, 0x2D88)),
                1, -1));
        reinterpret_cast<EclRunLowProposal::SpawnedEffectAnmVm *>(
            TH08_ECL_AT(ctx, u8 *, 0x53C8))->SetInterrupt(g_Player.IsYoukai() ? 2 : 1);
        if (TH08_ECL_AT(ctx, u32, 0x2E0C) & 1)
            *(f32 *)(TH08_ECL_AT(ctx, u8 *, 0x53C8) + 0x14) =
                -*(f32 *)(TH08_ECL_AT(ctx, u8 *, 0x53C8) + 0x14);
        break;
    }
    case 175: g_EclGlobal00F54E2C = TH08_ECL_READ_I(ctx, 0); break;
    case 177: TH08_ECL_AT(ctx, i32, 0x2E04) = TH08_ECL_READ_I(ctx, 0); break;
#if !defined(TH08_ECL_RUN_HIGH_BODY)
    case 178: TH08_ECL_CONTEXT_API(ctx)->Call004224A0(TH08_ECL_CONTEXT_ENEMY(ctx)); break;
#endif
    case 179: g_Gui.FUN_00439007(); break;
    case 180: g_Gui.FUN_004390d6(); break;
    case 181:
        if (g_GameManager.GetClockTime() < 12)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(0x2D), 0);
            g_GameManager.AddToClockTime(1);
            if (g_GameManager.GetClockTime() == 12)
                g_Gui.FUN_00439093();
            else
                g_Gui.FUN_00439050();
        }
        break;
    case 182:
        TH08_ECL_AT(ctx, u32, 0x3328) = (TH08_ECL_AT(ctx, u32, 0x3328) & 0xFFFFFEFF) | ((TH08_ECL_READ_I(ctx, 0) & 1) << 8);
        break;
    case 184: reinterpret_cast<TargetApi *>(&g_Spellcard)->Call0041F0E0(TH08_ECL_READ_I(ctx, 0)); break;
#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
    }

    TH08_ECL_RUN_HIGH_YIELD(DISPATCH_ADVANCE);
#endif

#ifdef TH08_ECL_RUN_HIGH_BODY
#if !defined(TH08_ECL_RUN_SHARED_SWITCH)
high_dispatch_complete: ;
#endif
#else
}

} // namespace EclRunHighProposal
} // namespace th08
#endif

#undef TH08_ECL_RUN_HIGH_YIELD
#endif // !TH08_ECL_RUN_DECLARATIONS_ONLY

#if !defined(TH08_ECL_RUN_HIGH_BODY)

namespace th08
{
namespace EclRunHighProposal
{

} // namespace EclRunHighProposal
} // namespace th08
#endif // !TH08_ECL_RUN_HIGH_BODY
