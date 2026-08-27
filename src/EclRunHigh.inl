// TH08 1.00d RunEcl high-opcode reconstruction.
//
// Scope owned by this file:
//   - real ECL opcodes 93..184 (the target switch indexes opcode - 1), and
//   - the shared post-dispatch tail at 0x0041E7E4..0x0041ECBD.
//
// EclRun.cpp includes the declarations once, then includes the body lexically
// inside EclManager::RunEcl so VC7 can reproduce the target's shared locals and
// control flow.  A dormant standalone dispatcher remains useful as an analysis
// harness; TargetApi contains only the services that harness actually needs.
// Addresses, access widths, constants, switch destinations, and byte offsets
// are target observations.  In particular, .analysis/ecl-decompile.c labels
// these cases 0x5c..0xb7 because Ghidra describes the normalized selector.  The
// case labels below restore the actual target opcode numbers and therefore run
// from 93 through 184.
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
extern void *g_EclExInsn[];
extern i32 g_EclGlobal004ECCA8; // target 0x004ECCA8
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

struct LaserSpawnArgs
{
    u16 bulletType;
    i16 color;
    f32 angle;
    f32 speed;
    f32 startOffset;
    f32 endOffset;
    f32 startLength;
    f32 width;
    i32 startTime;
    i32 duration;
    i32 despawnDuration;
    i32 hitboxStartTime;
    i32 hitboxEndDelay;
    u32 transformFlags;
};
C_ASSERT(sizeof(LaserSpawnArgs) == 0x34);
C_ASSERT(offsetof(LaserSpawnArgs, angle) == 0x4);
C_ASSERT(offsetof(LaserSpawnArgs, speed) == 0x8);
C_ASSERT(offsetof(LaserSpawnArgs, startOffset) == 0xc);
C_ASSERT(offsetof(LaserSpawnArgs, startTime) == 0x1c);
C_ASSERT(offsetof(LaserSpawnArgs, hitboxStartTime) == 0x28);
C_ASSERT(offsetof(LaserSpawnArgs, transformFlags) == 0x30);

// Provisional semantic name for target FUN_00422720.  Both its caller and
// callee establish Enemy in ECX and the current ECL instruction in EDX.
void __fastcall DispatchShotInstruction(u8 *enemy,
                                        RawInstruction *instruction);

struct SpawnPacketTyped
{
    SpawnPacketTyped();

    i32 type;
    D3DXVECTOR3 position;
    i32 arg4;
    i32 arg5;
    i32 arg6;
};

// RunEcl invokes the per-context callback with Enemy in ECX and its opaque
// context argument in EDX.  Interpolator callbacks receive Enemy in ECX and
// their progress value on the stack.
typedef void (__fastcall *EclContextCallback)(Enemy *enemy, void *argument);
struct Interpolator;
typedef void (__fastcall *EclInterpolatorCallback)(Enemy *enemy, Interpolator *entry, f32 progress);

struct Interpolator
{
    EclInterpolatorCallback callback; // +0x00, target calls with Enemy in ECX and entry in EDX
    ZunTimer timer;                 // +0x04
    i32 duration;         // +0x10
    i32 callbackIndex;    // +0x14
    i32 easing;           // +0x18, accepted values 1..6
    f32 parameters[4];    // +0x1C
    f32 affectedVariable; // +0x2C; 10042..10044 mean position motion
};
typedef char InterpolatorSizeCheck[sizeof(Interpolator) == 0x30 ? 1 : -1];
typedef char InterpolatorCallbackIndexOffsetCheck[offsetof(Interpolator, callbackIndex) == 0x14 ? 1 : -1];
typedef char InterpolatorParametersOffsetCheck[offsetof(Interpolator, parameters) == 0x1c ? 1 : -1];

enum DispatchResult
{
    DISPATCH_ADVANCE,
    DISPATCH_ENTER_SUBROUTINE,
};

// Services used only by the dormant standalone dispatcher analysis harness.
// The production EclManager::RunEcl path binds directly to concrete owners.
struct TargetApi
{
    i32 ResolveInt(u8 *enemy, i32 raw);                         // 0x0041F420
    f32 ResolveFloat(u8 *enemy, f32 raw);                       // 0x00420120
    i32 *ResolveIntLValue(u8 *enemy, i32 *raw, u16 flags, i32 idx); // 0x0041FE10
    f32 *ResolveFloatLValue(u8 *enemy, f32 *raw, u16 flags, i32 idx); // 0x00420950
    void ApplyRandomBiasedMove(u8 *enemy);                     // 0x004224A0
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
        return reinterpret_cast<u8 *>(
            reinterpret_cast<Enemy *>(enemy)->activeEclContext);
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
// RunEcl opcode 169 needs the target's byte-view source shape for its chained
// x87 comparisons.  Keep the literal behind a semantic constant and pin it to
// the public Enemy member rather than repeating an anonymous object offset.
#define TH08_ECL_ENEMY_POSITION_OFFSET 0x2D34
C_ASSERT(TH08_ECL_ENEMY_POSITION_OFFSET == offsetof(Enemy, position));
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
#define TH08_ECL_LASER(ctx, index) \
    (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->laserSlots[(index)])
#define TH08_ECL_PRESENTATION_WRITES_ALLOWED() \
    (((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> 14) & 1) == 0) || \
     ((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> 7) & 3) == 0))

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
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->life <= 0)
            break;
        if (((reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 >>
              ENEMY_FLAG_DEFER_BULLET_PATTERN_SHIFT) & 1) == 1)
        {
            memcpy(reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->pendingShotInstruction,
                   TH08_ECL_CONTEXT_INSTRUCTION(ctx),
                   sizeof(reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->pendingShotInstruction));
            break;
        }
        DispatchShotInstruction(TH08_ECL_CONTEXT_ENEMY(ctx),
                                TH08_ECL_CONTEXT_INSTRUCTION(ctx));
        break;

    case 111:
    {
        BulletTransformRecord *entry =
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
                 ->bulletSpawnDescriptor.transforms[TH08_ECL_READ_I(ctx, 0)];
        entry->kind = TH08_ECL_READ_I(ctx, 1);
        entry->allowWhileActive = TH08_ECL_READ_I(ctx, 2);
        entry->int0 = TH08_ECL_READ_I(ctx, 3);
        entry->int1 = TH08_ECL_READ_I(ctx, 4);
        entry->float0 = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 5))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 5)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 5)));
        entry->float1 = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 6))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 6)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 6)));
        break;
    }
    case 138:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->deathAnm1 = TH08_ECL_RAW_BYTE(ctx, 0);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->deathAnm2 = TH08_ECL_RAW_BYTE(ctx, 1);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->deathAnm3 = TH08_ECL_RAW_BYTE(ctx, 2);
        break;

    case 105:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames = TH08_ECL_READ_I(ctx, 0);
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames != 0)
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames +=
                g_GameManager.ScaleIntBasedOnRank(
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames / 5,
                    -reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames / 5);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalTimer = 0;
        }
        break;
    case 106:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames = TH08_ECL_READ_I(ctx, 0);
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames != 0)
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames +=
                g_GameManager.ScaleIntBasedOnRank(
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames / 5,
                    -reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames / 5);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalTimer =
                g_Rng.GetRandomU32InRange(
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootIntervalFrames);
        }
        break;
    case 107: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 |= ENEMY_FLAG_DEFER_BULLET_PATTERN; break;
    case 108: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 &= ~ENEMY_FLAG_DEFER_BULLET_PATTERN; break;

    case 109:
    {
        *reinterpret_cast<D3DXVECTOR3 *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
                 ->bulletSpawnDescriptor.position) =
            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position) +
            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootOffset);
        g_BulletManager.SpawnBulletPattern(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletSpawnDescriptor);
        break;
    }
    case 110:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootOffset.x = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)));
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootOffset.y = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootOffset.z = 0.0f;
        break;

    case 114:
    case 115:
    {
#pragma var_order(descriptor, args)
        LaserSpawnArgs *args =
            reinterpret_cast<LaserSpawnArgs *>(TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands);
        BulletSpawnDescriptor *descriptor =
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->laserSpawnDescriptor;
        *reinterpret_cast<D3DXVECTOR3 *>(&descriptor->position) =
            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->worldPosition) +
            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->shootOffset);
        descriptor->bulletType = args->bulletType;
        descriptor->color = (u16)((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & 2U) ? EclOperands::ResolveInt(reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)), static_cast<i32>(args->color)) : static_cast<i32>(args->color));
        descriptor->angle =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      args->angle)
                : args->angle;
        descriptor->speed1 =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      args->speed)
                : args->speed;
        descriptor->laserStartOffset =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 4))
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      args->startOffset)
                : args->startOffset;
        descriptor->laserEndOffset =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 5))
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      args->endOffset)
                : args->endOffset;
        descriptor->laserStartLength =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 6))
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      args->startLength)
                : args->startLength;
        descriptor->laserWidth =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 7))
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      args->width)
                : args->width;
        descriptor->laserStartTime =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 8))
                ? EclOperands::ResolveInt(
                      reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)),
                      args->startTime)
                : args->startTime;
        descriptor->laserDuration =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 9))
                ? EclOperands::ResolveInt(
                      reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)),
                      args->duration)
                : args->duration;
        descriptor->laserDespawnDuration =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 10))
                ? EclOperands::ResolveInt(
                      reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx)),
                      args->despawnDuration)
                : args->despawnDuration;
        descriptor->laserHitboxStartTime = args->hitboxStartTime;
        descriptor->laserHitboxEndDelay = args->hitboxEndDelay;
        descriptor->transformFlags = args->transformFlags;
        if (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->opcode == 115)
            descriptor->aimMode = BULLET_AIM_FAN_AIMED;
        else
            descriptor->aimMode = BULLET_AIM_FAN;
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
            ->laserSlots[reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->selectedLaserSlot] =
            g_BulletManager.SpawnLaserPattern(descriptor);
        break;
    }
    case 116:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->selectedLaserSlot =
            TH08_ECL_READ_I(ctx, 0);
        break;
    case 117:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->angle =
                AddNormalizeAngle(
                    TH08_ECL_LASER(ctx, lhsInt)->angle,
                    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))));
        break;
    case 167:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->angle =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        break;
    case 118:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->angle =
                g_Player.AngleToPoint(
                    &TH08_ECL_LASER(ctx, lhsInt)->position) +
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        break;
    case 119:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
        {
            TH08_ECL_LASER(ctx, lhsInt)->position.x = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
                    ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                          *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
                    : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) + reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->worldPosition.x;
            TH08_ECL_LASER(ctx, lhsInt)->position.y = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                    ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                          *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))
                    : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) + reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->worldPosition.y;
            TH08_ECL_LASER(ctx, lhsInt)->position.z = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                    ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                          *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
                    : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3))) + reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->worldPosition.z;
        }
        break;
    case 170:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->hideCapDuringStartup =
                (u8)TH08_ECL_READ_I(ctx, 1);
        break;
    case 120:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt) && TH08_ECL_LASER(ctx, lhsInt)->inUse)
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->
                extraIntVariables[2] = 1;
        else
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->
                extraIntVariables[2] = 0;
        break;
    case 121:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt) && TH08_ECL_LASER(ctx, lhsInt)->inUse &&
            TH08_ECL_LASER(ctx, lhsInt)->state < LASER_STATE_DESPAWNING)
        {
            TH08_ECL_LASER(ctx, lhsInt)->state = LASER_STATE_DESPAWNING;
            TH08_ECL_LASER(ctx, lhsInt)->timer = 0;
            *reinterpret_cast<i32 *>(&TH08_ECL_LASER(ctx, lhsInt)->width) =
                *reinterpret_cast<i32 *>(&TH08_ECL_LASER(ctx, lhsInt)->currentWidth);
        }
        break;
    case 154:
        for (i32 i = 0; i < 0x20; ++i)
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->laserSlots[i] = 0;
        break;
    case 171:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->startLength =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        break;
    case 172:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
        {
            TH08_ECL_LASER(ctx, lhsInt)->startOffset =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
            TH08_ECL_LASER(ctx, lhsInt)->endOffset =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)));
        }
        break;
    case 163: g_EnemyManager.opcode163Value = TH08_ECL_READ_I(ctx, 0); break;
    case 127:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            g_EnemyManager.bosses[TH08_ECL_READ_I(ctx, 0)] =
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx));
            if (TH08_ECL_READ_I(ctx, 0) == 0)
            {
                g_Gui.SetBossPresent(true);
                g_Gui.SetBossLifeBarTarget(1.0f);
            }
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 |= ENEMY_FLAG_BOSS;
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot = (u8)TH08_ECL_READ_I(ctx, 0);
            g_AsciiManager.SetBossMarkerInterrupt(
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot, 1);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->minimumPlayerDistanceSquared = 0.0f;
        }
        else
        {
            if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot < 4)
                g_Gui.SetBossPresent(false);
            g_EnemyManager.bosses[
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot] = 0;
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 &= ~ENEMY_FLAG_BOSS;
            g_AsciiManager.SetBossMarkerInterrupt(
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot, 2);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ReleaseAttachedEffects();
            g_AsciiManager.SetBossMarkerPosition(
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot,
                &D3DXVECTOR3(-999.0f, -999.0f, 0.0f));
        }
        break;

    case 128:
    {
        u8 *operands = TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands;
        reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->attachedEffects[
            reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->attachedEffectCount] =
            reinterpret_cast<Effect *>(g_EffectManager.SpawnEffect(
                13, reinterpret_cast<D3DXVECTOR3 *>(
                        &reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position),
                1, 0xFF6060D0));
        reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->attachedEffects[
            reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->attachedEffectCount]->vector6 =
            *reinterpret_cast<Float3 *>(operands + 4);
        reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->attachedEffectDistance =
            *reinterpret_cast<f32 *>(operands + 0x10);
        reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->attachedEffectCount++;
        break;
    }
    case 159: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->drawGroup = (u8)TH08_ECL_READ_I(ctx, 0); break;
    case 124: g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(TH08_ECL_READ_I(ctx, 0)), reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position.x); break;
    case 129:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
            reinterpret_cast<EnemyFlag1Bits *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->deathMode = TH08_ECL_RAW_BYTE(ctx, 0);
        break;
    case 130:
        if (((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> 14) & 1) == 0) ||
            ((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> 7) & 3) == 0))
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->deathCallbackSubId =
                TH08_ECL_RAW_U16(ctx, 0);
        break;
    case 126:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->eclSubroutineIds[
            TH08_ECL_READ_I(ctx, 1)] = (i16)TH08_ECL_READ_I(ctx, 0);
        break;
    case 125:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->pendingEclSubroutineIndex =
            (i16)TH08_ECL_READ_I(ctx, 0);
#ifdef TH08_ECL_RUN_HIGH_BODY
enter_subroutine:
        // Target 0x0041C88A is shared by opcode 125 and the pending-subroutine
        // check at the top of RunEcl's dispatch loop.  Keeping it lexical at
        // this case preserves the target's handler ordering.
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->currentInstr =
            reinterpret_cast<EclRawInstruction *>((u8 *)instruction + instruction->nextOffset);

        if (((reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 >>
              ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) == 0)
        {
            memcpy(reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclCallStack +
                       reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclCallStackDepth,
                   &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->mainEclContextStorage,
                   0x8A * sizeof(i32));
        }

        g_EclManager.CallEclSub(
            reinterpret_cast<EnemyEclContext *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->mainEclContextStorage),
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->eclSubroutineIds[
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->pendingEclSubroutineIndex]);
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclCallStackDepth < 15)
            ++reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclCallStackDepth;
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->pendingEclSubroutineIndex = -1;
        goto restart_context;
#else
        TH08_ECL_RUN_HIGH_YIELD(DISPATCH_ENTER_SUBROUTINE);
#endif
    case 131:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->phaseStartingLife =
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->life =
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->maxLife = TH08_ECL_READ_I(ctx, 0);
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossSlot == 0 &&
            (((reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 >>
               ENEMY_FLAG_BOSS_SHIFT) & 1) != 0))
            for (i32 i = 0; i < 8; ++i)
                g_Gui.SetBossGaugeSlot(i, 0.0f, 0.0f);
        break;
    case 158:
    {
        i32 index = TH08_ECL_READ_I(ctx, 0);
        g_Gui.SetBossGaugeSlot(
            index,
            (f32)TH08_ECL_READ_I(ctx, 1) / (f32)reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->maxLife,
            (f32)TH08_ECL_READ_I(ctx, 2) / (f32)reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->maxLife);
        g_Gui.SetBossGaugeColor(
            index, TH08_ECL_READ_I(ctx, 3));
        break;
    }
    case 122: StartEnemySpell(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx)); break;
    case 123: EndEnemySpell(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx)); break;
    case 132: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossTimer = TH08_ECL_READ_I(ctx, 0); break;
    case 133:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->lifeCallbackThresholds[
                TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_READ_I(ctx, 1);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->lifeCallbackSubIds[
                TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_READ_I(ctx, 2);
        }
        else
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->lifeCallbackThresholds[
                TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_READ_I(ctx, 1);
        break;
    case 134:
        if (((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> 14) & 1) == 0) ||
            ((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> 7) & 3) == 0))
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->timerCallbackThresholdFrames =
                TH08_ECL_READ_I(ctx, 0);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->timerCallbackSubId =
                TH08_ECL_READ_I(ctx, 1);
        }
        else
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->timerCallbackThresholdFrames =
                TH08_ECL_READ_I(ctx, 0);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossTimer = 0;
        break;

    case 135:
    {
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt])
            g_ZunMemory.Free(
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt]);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt] = 0;

        if (TH08_ECL_READ_I(ctx, 1) >= 0)
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt] =
                (u8 *)g_ZunMemory.Alloc(0x24B0, "ECLInt");
            if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt])
            {
                memset(
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt],
                    0, 0x24B0);
                *(i32 *)reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt] =
                    TH08_ECL_READ_I(ctx, 1);
                g_EclManager.CallEclSub(
                    reinterpret_cast<EnemyEclContext *>(
                        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt] + 8),
                    *(u16 *)reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt]);
                memcpy(reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->childEclBlocks[lhsInt] + 0x20,
                       reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->intVariables,
                       0x1E * sizeof(i32));
            }
        }
        break;
    }
    case 139:
        g_EffectManager.SpawnEffect(TH08_ECL_READ_I(ctx, 0), reinterpret_cast<D3DXVECTOR3 *>(
                                         &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position),
                                     TH08_ECL_READ_I(ctx, 1), *TH08_ECL_WRITE_I(ctx, 2));
        break;
    case 140:
    {
        Float3 vector;
        vector.x = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        vector.y = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 4))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 4)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 4)));
        vector.z = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 5))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 5)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 5)));
        g_EffectManager.SpawnEffectWithVelocity(
            TH08_ECL_READ_I(ctx, 0),
            reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position),
            reinterpret_cast<D3DXVECTOR3 *>(&vector),
            TH08_ECL_READ_I(ctx, 1), *TH08_ECL_WRITE_I(ctx, 2));
        break;
    }
    case 143: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->itemDropType = TH08_ECL_READ_I(ctx, 0); break;
    case 144:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->pointItemDropCount = TH08_ECL_READ_I(ctx, 0);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->powerOrPointItemDropCount = TH08_ECL_READ_I(ctx, 1);
        break;
    case 142:
    {
        struct Op142Locals
        {
            Float3 position;
            i32 i;
        } locals;
        i32 count = TH08_ECL_READ_I(ctx, 0);
        for (locals.i = 0; locals.i < count; ++locals.i)
        {
            locals.position = reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position;
            ((f32 *)locals.position)[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            ((f32 *)locals.position)[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            if (g_GameManager.GetPower() < 0x80)
                g_ItemManager.SpawnItem(&locals.position,
                                        static_cast<ItemType>(locals.i != 0 ? 0 : 2), 0);
            else
                g_ItemManager.SpawnItem(&locals.position, static_cast<ItemType>(1), 0);
        }
        break;
    }
    case 168:
    {
        struct Op168Locals
        {
            Float3 position;
            i32 i;
        } locals;
        i32 count = TH08_ECL_READ_I(ctx, 0);
        for (locals.i = 0; locals.i < count; ++locals.i)
        {
            locals.position = reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position;
            ((f32 *)locals.position)[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            ((f32 *)locals.position)[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&locals.position), static_cast<ItemType>(1), 0);
        }
        break;
    }
    case 145:
        reinterpret_cast<EnemyFlag1Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->rotateAnmWithMovement = TH08_ECL_RAW_BYTE(ctx, 0);
        break;
    case 136: reinterpret_cast<void (__fastcall *)(u8 *, RawInstruction *)>(g_EclExInsn[TH08_ECL_READ_I(ctx, 0)])(TH08_ECL_CONTEXT_ENEMY(ctx), TH08_ECL_CONTEXT_INSTRUCTION(ctx)); break;
    case 137:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->callback =
                reinterpret_cast<EnemyEclContextCallback>(g_EclExInsn[TH08_ECL_READ_I(ctx, 0)]);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->callbackArgument =
                TH08_ECL_CONTEXT_INSTRUCTION(ctx);
        }
        else
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->callback = 0;
        break;
    case 146:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->time +=
            TH08_ECL_READ_I(ctx, 0);
        break;
    case 141: g_ItemManager.SpawnItem(&reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position, static_cast<ItemType>(TH08_ECL_READ_I(ctx, 0)), 0); break;
    case 147: g_Background.pendingStageScriptLabel = TH08_ECL_READ_I(ctx, 0); break;
    case 148:
        g_Gui.SetBossLifeMarkerCount(TH08_ECL_READ_I(ctx, 0));
        g_GameManager.catkData[SPELLCARD_ST2_BOSS_3L]
            .inGameHistory.maxBonus[SHOT_MARISA_ALICE] += 0x708;
        break;
    case 93:
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->life > 0)
        {
            SpawnPacketTyped packet93;
            void *spawned93;
            {
                D3DXVECTOR3 position93;
                memcpy(&packet93, TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands, sizeof(packet93));
                position93.x =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(packet93.position.x)
                        : packet93.position.x;
                position93.y =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(packet93.position.y)
                        : packet93.position.y;
                position93.z =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(packet93.position.z)
                        : packet93.position.z;
                spawned93 = g_EnemyManager.SpawnEnemy2(
                    packet93.type, &position93,
                    TH08_ECL_READ_I(ctx, 4), TH08_ECL_READ_I(ctx, 5),
                    TH08_ECL_READ_I(ctx, 6),
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->intVariables);
            }
            (void)spawned93;
        }
        break;
    case 94:
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->life > 0)
        {
            SpawnPacketTyped packet94;
            void *spawned94;
            {
                D3DXVECTOR3 position94;
                memcpy(&packet94, TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands, sizeof(packet94));
                position94.x =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(packet94.position.x)
                        : packet94.position.x;
                position94.y =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(packet94.position.y)
                        : packet94.position.y;
                position94.z =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                        ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(packet94.position.z)
                        : packet94.position.z;
                reinterpret_cast<Float3 *>(&position94)->operator+=(
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->position);
                spawned94 = g_EnemyManager.SpawnEnemy2(
                    packet94.type, &position94,
                    TH08_ECL_READ_I(ctx, 4), TH08_ECL_READ_I(ctx, 5),
                    TH08_ECL_READ_I(ctx, 6),
                    reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->activeEclContext->intVariables);
            }
            (void)spawned94;
        }
        break;
    case 95:
        g_EnemyManager.KillAllNonBossEnemies(8000, 0);
        break;
    case 149:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->vm.pendingInterrupt =
            (i16)TH08_ECL_READ_I(ctx, 0);
        break;
    case 150:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
            ->secondaryVms[TH08_ECL_RAW_I(ctx, 0)].pendingInterrupt =
            (i16)TH08_ECL_RAW_U16(ctx, 4);
        break;
    case 112: g_BulletManager.ClearBulletsForTransition(); break;

    case 113:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
                ->bulletSpawnDescriptor.spawnSound = TH08_ECL_READ_I(ctx, 0);
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
                ->bulletSpawnDescriptor.transformFlags |= BULLET_TRANSFORM_PLAY_SPAWN_SOUND;
        }
        else
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
                ->bulletSpawnDescriptor.transformFlags &= ~BULLET_TRANSFORM_PLAY_SPAWN_SOUND;
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))
            ->bulletSpawnDescriptor.transformSound = TH08_ECL_READ_I(ctx, 1);
        break;
    case 151:
        reinterpret_cast<EnemyFlag1Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->disableEclCallStack =
            TH08_ECL_RAW_BYTE(ctx, 0);
        break;
    case 152:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletRankInfluence.speedLow = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)));
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletRankInfluence.speedHigh = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                  *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletRankInfluence.count1Low = (i16)TH08_ECL_READ_I(ctx, 2);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletRankInfluence.count1High = (i16)TH08_ECL_READ_I(ctx, 3);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletRankInfluence.count2Low = (i16)TH08_ECL_READ_I(ctx, 4);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bulletRankInfluence.count2High = (i16)TH08_ECL_READ_I(ctx, 5);
        break;
    case 153:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->timerCallbackSubId =
            (i32)reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->deathCallbackSubId;
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->bossTimer = 0;
        break;
    case 155:
        reinterpret_cast<EnemyFlag1Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->timeoutSpell =
            TH08_ECL_RAW_BYTE(ctx, 0);
        g_EclGlobal004ECCA8 = 0x05F5E0F6;
        break;
    case 156:
        reinterpret_cast<EnemyFlag1Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->specialInteraction =
            TH08_ECL_RAW_BYTE(ctx, 0);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->drawGroup = 2;
        break;
    case 157:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailFlags = TH08_ECL_RAW_BYTE(ctx, 0);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailHistoryLength = (u16)TH08_ECL_READ_I(ctx, 1);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailCollisionLength = (u16)TH08_ECL_READ_I(ctx, 2);
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailSampleStride = (u16)TH08_ECL_READ_I(ctx, 3);
        if (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailFlags & ENEMY_TRAIL_RENDER_AS_STRIP)
            g_AnmManager->InitializeHorizontalTextureStrip(
                &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->vm,
                reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailVertices,
                (reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailHistoryLength /
                 reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->trailSampleStride) << 1);
        break;
    case 160: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->damageReductionTimer = TH08_ECL_READ_I(ctx, 0); break;
    case 161:
        g_BulletManager.RemoveBulletsInRadius(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->worldPosition, ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))));
        break;
    case 162: g_BulletManager.RemoveAllBullets(4); break;
    case 164:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        g_Spellcard.SetEffectTrackingDisabled(lhsInt);
        if (lhsInt == 0)
            g_Spellcard.SetStoredVector(((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))), ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))), ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3))));
        break;
    case 165:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->vm.rotation.z =
            ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)));
        break;
    case 166:
        *TH08_ECL_WRITE_F(ctx, 1) =
            sinf(((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2)) ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))) * ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        *TH08_ECL_WRITE_F(ctx, 0) =
            cosf(((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
            ? reinterpret_cast<EclOperands::EnemyOverlay *>(TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))) *
            ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                 ? reinterpret_cast<EclOperands::EnemyOverlay *>(
                       TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                       *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
                 : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        break;
    case 169:
        if (
            reinterpret_cast<Vec3 *>(&g_Player.position)->x <
                TH08_ECL_AT(ctx, Vec3, TH08_ECL_ENEMY_POSITION_OFFSET).x &&
            96.0f < TH08_ECL_AT(ctx, f32, TH08_ECL_ENEMY_POSITION_OFFSET) ||
            288.0f < TH08_ECL_AT(ctx, f32, TH08_ECL_ENEMY_POSITION_OFFSET))
            *TH08_ECL_WRITE_F(ctx, 0) = AddNormalizeAngle(
                g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
        else
            *TH08_ECL_WRITE_F(ctx, 0) =
                g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
        break;
    case 173:
        reinterpret_cast<EnemyFlag1Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->pauseTimer = TH08_ECL_READ_I(ctx, 0);
        break;
    case 183:
        reinterpret_cast<EnemyFlag1Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1)->noDamageDuringStop = TH08_ECL_READ_I(ctx, 0);
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
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags1 |= ENEMY_FLAG_PAUSE_TIMER;
        break;
    }
#ifdef TH08_ECL_RUN_HIGH_BODY
    // Opcodes 82 and 83 are emitted here in integrated RunEcl to reproduce
    // the target's late physical handler order.  Their standalone low-opcode
    // forms remain in EclRunLow.inl for source ownership and audit coverage.
    case 82:
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->minimumPlayerDistanceSquared =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & 1U)
                ? reinterpret_cast<EclOperands::EnemyOverlay *>(
                      TH08_ECL_CONTEXT_ENEMY(ctx))->ResolveFloat(
                      *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)))
                : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0));
        reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->minimumPlayerDistanceSquared *=
            reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->minimumPlayerDistanceSquared;
        break;
    case 83:
        reinterpret_cast<EnemyFlag2Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags2)->formEffect =
            TH08_ECL_READ_I(ctx, 0);
        break;
#endif
    case 174:
    {
        if (reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->alignmentEffect)
            reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->alignmentEffect->active = 0;
        reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->alignmentEffect =
            reinterpret_cast<Effect *>(g_EffectManager.SpawnEffectInSecondaryPool(
                TH08_ECL_READ_I(ctx, 0) + 0x20,
                reinterpret_cast<D3DXVECTOR3 *>(
                    &reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->worldPosition),
                1, -1));
        reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->alignmentEffect->vm.SetInterrupt(
            g_Player.IsYoukai() ? 2 : 1);
        if (reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->enemyIndex & 1)
            reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->alignmentEffect->vm.angleVel.z =
                -reinterpret_cast<th08::Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->alignmentEffect->vm.angleVel.z;
        break;
    }
    case 175: g_EnemyManager.suppressTimelineSpawns = TH08_ECL_READ_I(ctx, 0); break;
    case 177: reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->phaseStartingLife = TH08_ECL_READ_I(ctx, 0); break;
#if !defined(TH08_ECL_RUN_HIGH_BODY)
    case 178: TH08_ECL_CONTEXT_API(ctx)->ApplyRandomBiasedMove(TH08_ECL_CONTEXT_ENEMY(ctx)); break;
#endif
    case 179: g_Gui.StartStageBackgroundSequence(); break;
    case 180: g_Gui.HideClockTime(); break;
    case 181:
        if (static_cast<i8>(g_GameManager.GetClockTime()) < 12)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(0x2D), 0);
            g_GameManager.AddToClockTime(1);
            if (static_cast<i8>(g_GameManager.GetClockTime()) == 12)
                g_Gui.FlashClockTimeFast();
            else
                g_Gui.FlashClockTimeSlow();
        }
        break;
    case 182:
        reinterpret_cast<EnemyFlag2Bits *>(
            &reinterpret_cast<Enemy *>(TH08_ECL_CONTEXT_ENEMY(ctx))->flags2)->extraVmFixedOffset = TH08_ECL_READ_I(ctx, 0);
        break;
    case 184: g_Spellcard.SetBonusUpdatesDisabled(TH08_ECL_READ_I(ctx, 0)); break;
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
