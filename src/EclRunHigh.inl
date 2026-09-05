// TH08 1.00d RunEcl high-opcode reconstruction.
//
// Scope owned by this file:
//   - real ECL opcodes 93..184 (the target switch indexes opcode - 1), and
//   - the shared post-dispatch tail at 0x0041E7E4..0x0041ECBD.
//
// EclRun.cpp includes the declarations once, then includes the body lexically
// inside EclManager::RunEcl so VC7 can reproduce the target's shared locals and
// control flow.
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
struct EclSpellCardInstructionArgs;
void __fastcall StartEnemySpell(
    Enemy *enemy, EclSpellCardInstructionArgs *instruction);
void __fastcall EndEnemySpell(
    Enemy *enemy, EclSpellCardInstructionArgs *instruction);

namespace EclRunHigh
{

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

struct BulletTransformInstructionArgs
{
    i32 transformIndex;
    i32 kind;
    i32 allowWhileActive;
    i32 intPayload0;
    i32 intPayload1;
    f32 floatPayload0;
    f32 floatPayload1;
};
C_ASSERT(sizeof(BulletTransformInstructionArgs) == 0x1c);
C_ASSERT(offsetof(BulletTransformInstructionArgs, kind) == 0x4);
C_ASSERT(offsetof(BulletTransformInstructionArgs, intPayload0) == 0xc);
C_ASSERT(offsetof(BulletTransformInstructionArgs, floatPayload0) == 0x14);

struct BulletRankInfluenceInstructionArgs
{
    f32 speedLow;
    f32 speedHigh;
    i32 count1Low;
    i32 count1High;
    i32 count2Low;
    i32 count2High;
};
C_ASSERT(sizeof(BulletRankInfluenceInstructionArgs) == 0x18);
C_ASSERT(offsetof(BulletRankInfluenceInstructionArgs, count1Low) == 0x8);

struct TrailInstructionArgs
{
    u8 flags;
    u8 serializedPadding[3];
    i32 historyLength;
    i32 collisionLength;
    i32 sampleStride;
};
C_ASSERT(sizeof(TrailInstructionArgs) == 0x10);
C_ASSERT(offsetof(TrailInstructionArgs, historyLength) == 0x4);
C_ASSERT(offsetof(TrailInstructionArgs, sampleStride) == 0xc);

// Target behavior at 0x00422720 establishes this as the Enemy bullet-spawn
// descriptor dispatcher.
void __fastcall DispatchShotInstruction(Enemy *enemy,
                                        EclRawInstruction *instruction);

struct SpawnPacketTyped
{
    SpawnPacketTyped();

    i32 eclSubroutineId;
    D3DXVECTOR3 position;
    i32 life;
    i32 itemDropType;
    i32 score;
};

// RunEcl was built with /Ob0, and its target body accesses these overlays
// directly.  Named expressions preserve the recovered widths while avoiding
// hundreds of non-target Context/accessor COMDAT calls.
#define TH08_ECL_AT(ctx, type, offset) \
    (*reinterpret_cast<type *>( \
        reinterpret_cast<u8 *>(TH08_ECL_CONTEXT_ENEMY(ctx)) + (offset)))
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
               TH08_ECL_CONTEXT_ENEMY(ctx), \
               static_cast<i32>(*reinterpret_cast<i16 *>( \
                   TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (byteOffset)))) \
         : static_cast<i32>(*reinterpret_cast<i16 *>( \
               TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands + (byteOffset))))
#define TH08_ECL_READ_I(ctx, index) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (index))) \
         ? EclOperands::ResolveInt( \
               TH08_ECL_CONTEXT_ENEMY(ctx), \
               TH08_ECL_RAW_I((ctx), (index))) \
         : TH08_ECL_RAW_I((ctx), (index)))
#define TH08_ECL_READ_F(ctx, index) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (index))) \
         ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat( \
               TH08_ECL_RAW_F((ctx), (index))) \
         : TH08_ECL_RAW_F((ctx), (index)))
#define TH08_ECL_READ_F_RAWARG(ctx, index) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << (index))) \
         ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat( \
               *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I((ctx), (index)))) \
         : TH08_ECL_RAW_F((ctx), (index)))

// Use a wire-schema member as a compile-time operand index while preserving
// the exact resolver expression that RunEcl's /Ob0 target body emitted.
#define TH08_ECL_FIELD_INDEX(type, member) (offsetof(type, member) / 4)
#define TH08_ECL_READ_I_FIELD(ctx, type, member) \
    TH08_ECL_READ_I((ctx), TH08_ECL_FIELD_INDEX(type, member))
#define TH08_ECL_READ_F_BITCAST_FIELD(ctx, type, member) \
    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & \
      (1U << TH08_ECL_FIELD_INDEX(type, member))) \
         ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat( \
               *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I( \
                   (ctx), TH08_ECL_FIELD_INDEX(type, member)))) \
         : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I( \
               (ctx), TH08_ECL_FIELD_INDEX(type, member))))

#define TH08_ECL_WRITE_I(ctx, index) \
    EclOperands::ResolveIntLValue( \
        TH08_ECL_CONTEXT_ENEMY(ctx), \
        &TH08_ECL_RAW_I((ctx), (index)), TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags, (index))
#define TH08_ECL_WRITE_F(ctx, index) \
    EclOperands::ResolveFloatLValue( \
        TH08_ECL_CONTEXT_ENEMY(ctx), \
        &TH08_ECL_RAW_F((ctx), (index)), TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags, (index))
#define TH08_ECL_LASER(ctx, index) \
    (TH08_ECL_CONTEXT_ENEMY(ctx)->laserSlots[(index)])
#define TH08_ECL_PRESENTATION_WRITES_ALLOWED() \
    (((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> \
        GameManagerFlags::SPELL_PRACTICE_SHIFT) & 1) == 0) || \
     ((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >> \
        GameManagerFlags::PLAYER_DEATH_DISSOLVE_SHIFT) & \
       GameManagerFlags::PLAYER_DEATH_DISSOLVE_MASK) == 0))

} // namespace EclRunHigh
} // namespace th08

#endif // TH08_ECL_RUN_HIGH_DECLARATIONS

#if !defined(TH08_ECL_RUN_DECLARATIONS_ONLY)

#if !defined(TH08_ECL_RUN_HIGH_BODY) || !defined(TH08_ECL_RUN_SHARED_SWITCH)
#error EclRunHigh.inl opcode body must be included inside EclManager::RunEcl
#endif

    case ECL_OPCODE_SHOOT_FAN_AIMED:
    case ECL_OPCODE_SHOOT_FAN:
    case ECL_OPCODE_SHOOT_CIRCLE_AIMED:
    case ECL_OPCODE_SHOOT_CIRCLE:
    case ECL_OPCODE_SHOOT_OFFSET_CIRCLE_AIMED:
    case ECL_OPCODE_SHOOT_OFFSET_CIRCLE:
    case ECL_OPCODE_SHOOT_RANDOM_ANGLE:
    case ECL_OPCODE_SHOOT_RANDOM_SPEED:
    case ECL_OPCODE_SHOOT_RANDOM:
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->life <= 0)
            break;
        if (((TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 >>
              ENEMY_FLAG_DEFER_BULLET_PATTERN_SHIFT) & 1) == 1)
        {
            memcpy(TH08_ECL_CONTEXT_ENEMY(ctx)->pendingShotInstruction,
                   TH08_ECL_CONTEXT_INSTRUCTION(ctx),
                   sizeof(TH08_ECL_CONTEXT_ENEMY(ctx)->pendingShotInstruction));
            break;
        }
        DispatchShotInstruction(
                                TH08_ECL_CONTEXT_ENEMY(ctx),
                                TH08_ECL_CONTEXT_INSTRUCTION(ctx));
        break;

    case ECL_OPCODE_SET_BULLET_TRANSFORM:
    {
        BulletTransformRecord *entry =
            &TH08_ECL_CONTEXT_ENEMY(ctx)
                 ->bulletSpawnDescriptor.transforms[TH08_ECL_READ_I_FIELD(
                     ctx, BulletTransformInstructionArgs, transformIndex)];
        entry->kind = TH08_ECL_READ_I_FIELD(
            ctx, BulletTransformInstructionArgs, kind);
        entry->allowWhileActive = TH08_ECL_READ_I_FIELD(
            ctx, BulletTransformInstructionArgs, allowWhileActive);
        entry->payload.raw.int0 = TH08_ECL_READ_I_FIELD(
            ctx, BulletTransformInstructionArgs, intPayload0);
        entry->payload.raw.int1 = TH08_ECL_READ_I_FIELD(
            ctx, BulletTransformInstructionArgs, intPayload1);
        entry->payload.raw.float0 = TH08_ECL_READ_F_BITCAST_FIELD(
            ctx, BulletTransformInstructionArgs, floatPayload0);
        entry->payload.raw.float1 = TH08_ECL_READ_F_BITCAST_FIELD(
            ctx, BulletTransformInstructionArgs, floatPayload1);
        break;
    }
    case ECL_OPCODE_SET_DEATH_ANM_SCRIPTS:
        TH08_ECL_CONTEXT_ENEMY(ctx)->deathAnm1 = TH08_ECL_RAW_BYTE(ctx, 0);
        TH08_ECL_CONTEXT_ENEMY(ctx)->deathAnm2 = TH08_ECL_RAW_BYTE(ctx, 1);
        TH08_ECL_CONTEXT_ENEMY(ctx)->deathAnm3 = TH08_ECL_RAW_BYTE(ctx, 2);
        break;

    case ECL_OPCODE_SET_SHOOT_INTERVAL:
        TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames != 0)
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames +=
                g_GameManager.ScaleIntBasedOnRank(
                    TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames / 5,
                    -TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames / 5);
            TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalTimer = 0;
        }
        break;
    case ECL_OPCODE_SET_SHOOT_INTERVAL_DELAYED:
        TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames != 0)
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames +=
                g_GameManager.ScaleIntBasedOnRank(
                    TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames / 5,
                    -TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames / 5);
            TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalTimer =
                g_Rng.GetRandomU32InRange(
                    TH08_ECL_CONTEXT_ENEMY(ctx)->shootIntervalFrames);
        }
        break;
    case ECL_OPCODE_ENABLE_DEFERRED_SHOOTING: TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 |= ENEMY_FLAG_DEFER_BULLET_PATTERN; break;
    case ECL_OPCODE_DISABLE_DEFERRED_SHOOTING: TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 &= ~ENEMY_FLAG_DEFER_BULLET_PATTERN; break;

    case ECL_OPCODE_SHOOT_NOW:
    {
        *D3DXVECTOR3_PTR(
            &TH08_ECL_CONTEXT_ENEMY(ctx)
                 ->bulletSpawnDescriptor.position) =
            *D3DXVECTOR3_PTR(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->position) +
            *D3DXVECTOR3_PTR(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->shootOffset);
        g_BulletManager.SpawnBulletPattern(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->bulletSpawnDescriptor);
        break;
    }
    case ECL_OPCODE_SET_SHOOT_OFFSET:
        TH08_ECL_CONTEXT_ENEMY(ctx)->shootOffset.x = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)));
        TH08_ECL_CONTEXT_ENEMY(ctx)->shootOffset.y = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        TH08_ECL_CONTEXT_ENEMY(ctx)->shootOffset.z = 0.0f;
        break;

    case ECL_OPCODE_CREATE_LASER:
    case ECL_OPCODE_CREATE_LASER_AIMED:
    {
#pragma var_order(descriptor, args)
        LaserSpawnArgs *args =
            reinterpret_cast<LaserSpawnArgs *>(TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands);
        BulletSpawnDescriptor *descriptor =
            &TH08_ECL_CONTEXT_ENEMY(ctx)->laserSpawnDescriptor;
        *D3DXVECTOR3_PTR(&descriptor->position) =
            *D3DXVECTOR3_PTR(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->worldPosition) +
            *D3DXVECTOR3_PTR(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->shootOffset);
        descriptor->bulletType = args->bulletType;
        descriptor->color = (u16)((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & 2U) ? EclOperands::ResolveInt(TH08_ECL_CONTEXT_ENEMY(ctx), static_cast<i32>(args->color)) : static_cast<i32>(args->color));
        descriptor->angle =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      args->angle)
                : args->angle;
        descriptor->speed1 =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      args->speed)
                : args->speed;
        descriptor->laserStartOffset =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 4))
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      args->startOffset)
                : args->startOffset;
        descriptor->laserEndOffset =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 5))
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      args->endOffset)
                : args->endOffset;
        descriptor->laserStartLength =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 6))
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      args->startLength)
                : args->startLength;
        descriptor->laserWidth =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 7))
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      args->width)
                : args->width;
        descriptor->laserStartTime =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 8))
                ? EclOperands::ResolveInt(
                      TH08_ECL_CONTEXT_ENEMY(ctx),
                      args->startTime)
                : args->startTime;
        descriptor->laserDuration =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 9))
                ? EclOperands::ResolveInt(
                      TH08_ECL_CONTEXT_ENEMY(ctx),
                      args->duration)
                : args->duration;
        descriptor->laserDespawnDuration =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 10))
                ? EclOperands::ResolveInt(
                      TH08_ECL_CONTEXT_ENEMY(ctx),
                      args->despawnDuration)
                : args->despawnDuration;
        descriptor->laserHitboxStartTime = args->hitboxStartTime;
        descriptor->laserHitboxEndDelay = args->hitboxEndDelay;
        descriptor->transformFlags = args->transformFlags;
        if (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->opcode ==
            ECL_OPCODE_CREATE_LASER_AIMED)
            descriptor->aimMode = BULLET_AIM_FAN_AIMED;
        else
            descriptor->aimMode = BULLET_AIM_FAN;
        TH08_ECL_CONTEXT_ENEMY(ctx)
            ->laserSlots[TH08_ECL_CONTEXT_ENEMY(ctx)->selectedLaserSlot] =
            g_BulletManager.SpawnLaserPattern(descriptor);
        break;
    }
    case ECL_OPCODE_SELECT_LASER_SLOT:
        TH08_ECL_CONTEXT_ENEMY(ctx)->selectedLaserSlot =
            TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_ROTATE_LASER:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->angle =
                AddNormalizeAngle(
                    TH08_ECL_LASER(ctx, lhsInt)->angle,
                    ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))));
        break;
    case ECL_OPCODE_SET_LASER_ANGLE:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->angle =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        break;
    case ECL_OPCODE_AIM_LASER_AT_PLAYER:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->angle =
                g_Player.AngleToPoint(
                    &TH08_ECL_LASER(ctx, lhsInt)->position) +
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        break;
    case ECL_OPCODE_SET_LASER_POSITION:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
        {
            TH08_ECL_LASER(ctx, lhsInt)->position.x = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
                    ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                          *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
                    : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) + TH08_ECL_CONTEXT_ENEMY(ctx)->worldPosition.x;
            TH08_ECL_LASER(ctx, lhsInt)->position.y = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                    ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                          *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))
                    : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) + TH08_ECL_CONTEXT_ENEMY(ctx)->worldPosition.y;
            TH08_ECL_LASER(ctx, lhsInt)->position.z = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                    ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                          *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
                    : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3))) + TH08_ECL_CONTEXT_ENEMY(ctx)->worldPosition.z;
        }
        break;
    case ECL_OPCODE_SET_LASER_START_CAP_HIDDEN:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->hideCapDuringStartup =
                (u8)TH08_ECL_READ_I(ctx, 1);
        break;
    case ECL_OPCODE_TEST_LASER_ACTIVE:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt) && TH08_ECL_LASER(ctx, lhsInt)->inUse)
            TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->
                extraIntVariables[2] = 1;
        else
            TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->
                extraIntVariables[2] = 0;
        break;
    case ECL_OPCODE_CANCEL_LASER:
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
    case ECL_OPCODE_CLEAR_LASER_SLOTS:
        for (i32 i = 0; i < 0x20; ++i)
            TH08_ECL_CONTEXT_ENEMY(ctx)->laserSlots[i] = 0;
        break;
    case ECL_OPCODE_SET_LASER_START_LENGTH:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
            TH08_ECL_LASER(ctx, lhsInt)->startLength =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
        break;
    case ECL_OPCODE_SET_LASER_OFFSETS:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_LASER(ctx, lhsInt))
        {
            TH08_ECL_LASER(ctx, lhsInt)->startOffset =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)));
            TH08_ECL_LASER(ctx, lhsInt)->endOffset =
                ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)));
        }
        break;
    case ECL_OPCODE_SET_MANAGER_PROTOCOL_VALUE: g_EnemyManager.opcode163Value = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_SET_BOSS:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            g_EnemyManager.bosses[TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_CONTEXT_ENEMY(ctx);
            if (TH08_ECL_READ_I(ctx, 0) == 0)
            {
                g_Gui.SetBossPresent(true);
                g_Gui.SetBossLifeBarTarget(1.0f);
            }
            TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 |= ENEMY_FLAG_BOSS;
            TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot = (u8)TH08_ECL_READ_I(ctx, 0);
            g_AsciiManager.SetBossMarkerInterrupt(
                TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot, 1);
            TH08_ECL_CONTEXT_ENEMY(ctx)->minimumPlayerDistanceSquared = 0.0f;
        }
        else
        {
            if (TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot < 4)
                g_Gui.SetBossPresent(false);
            g_EnemyManager.bosses[
                TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot] = 0;
            TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 &= ~ENEMY_FLAG_BOSS;
            g_AsciiManager.SetBossMarkerInterrupt(
                TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot, 2);
            TH08_ECL_CONTEXT_ENEMY(ctx)->ReleaseAttachedEffects();
            g_AsciiManager.SetBossMarkerPosition(
                TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot,
                &D3DXVECTOR3(-999.0f, -999.0f, 0.0f));
        }
        break;

    case ECL_OPCODE_ATTACH_SPELL_EFFECT:
    {
        u8 *operands = TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands;
        TH08_ECL_CONTEXT_ENEMY(ctx)->attachedEffects[
            TH08_ECL_CONTEXT_ENEMY(ctx)->attachedEffectCount] =
            g_EffectManager.SpawnEffect(
                EFFECT_SPELLCARD_ORBIT, D3DXVECTOR3_PTR(
                        &TH08_ECL_CONTEXT_ENEMY(ctx)->position),
                1, 0xFF6060D0);
        TH08_ECL_CONTEXT_ENEMY(ctx)->attachedEffects[
            TH08_ECL_CONTEXT_ENEMY(ctx)->attachedEffectCount]->vector6 =
            *FLOAT3_PTR(operands + 4);
        TH08_ECL_CONTEXT_ENEMY(ctx)->attachedEffectDistance =
            *reinterpret_cast<f32 *>(operands + 0x10);
        TH08_ECL_CONTEXT_ENEMY(ctx)->attachedEffectCount++;
        break;
    }
    case ECL_OPCODE_SET_DRAW_GROUP: TH08_ECL_CONTEXT_ENEMY(ctx)->drawGroup = (u8)TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_PLAY_POSITIONED_SOUND: g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(TH08_ECL_READ_I(ctx, 0)), TH08_ECL_CONTEXT_ENEMY(ctx)->position.x); break;
    case ECL_OPCODE_SET_DEATH_MODE:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
            reinterpret_cast<EnemyFlag1Bits *>(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->deathMode = TH08_ECL_RAW_BYTE(ctx, 0);
        break;
    case ECL_OPCODE_SET_DEATH_CALLBACK:
        if (((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >>
              GameManagerFlags::SPELL_PRACTICE_SHIFT) & 1) == 0) ||
            ((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >>
              GameManagerFlags::PLAYER_DEATH_DISSOLVE_SHIFT) &
             GameManagerFlags::PLAYER_DEATH_DISSOLVE_MASK) == 0))
            TH08_ECL_CONTEXT_ENEMY(ctx)->deathCallbackSubId =
                TH08_ECL_RAW_U16(ctx, 0);
        break;
    case ECL_OPCODE_SET_SUBROUTINE_SLOT:
        TH08_ECL_CONTEXT_ENEMY(ctx)->eclSubroutineIds[
            TH08_ECL_READ_I(ctx, 1)] = (i16)TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_CALL_SUBROUTINE_SLOT:
        TH08_ECL_CONTEXT_ENEMY(ctx)->pendingEclSubroutineIndex =
            (i16)TH08_ECL_READ_I(ctx, 0);
enter_subroutine:
        // Target 0x0041C88A is shared by opcode 125 and the pending-subroutine
        // check at the top of RunEcl's dispatch loop.  Keeping it lexical at
        // this case preserves the target's handler ordering.
        TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->currentInstr =
            reinterpret_cast<EclRawInstruction *>((u8 *)instruction + instruction->nextOffset);

        if (((TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 >>
              ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT) & 1) == 0)
        {
            memcpy(TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclCallStack +
                       TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclCallStackDepth,
                   &TH08_ECL_CONTEXT_ENEMY(ctx)->mainEclContextStorage,
                   0x8A * sizeof(i32));
        }

        g_EclManager.CallEclSub(
            reinterpret_cast<EnemyEclContext *>(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->mainEclContextStorage),
            TH08_ECL_CONTEXT_ENEMY(ctx)->eclSubroutineIds[
                TH08_ECL_CONTEXT_ENEMY(ctx)->pendingEclSubroutineIndex]);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclCallStackDepth < 15)
            ++TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclCallStackDepth;
        TH08_ECL_CONTEXT_ENEMY(ctx)->pendingEclSubroutineIndex = -1;
        goto restart_context;
    case ECL_OPCODE_SET_LIFE:
        TH08_ECL_CONTEXT_ENEMY(ctx)->phaseStartingLife =
            TH08_ECL_CONTEXT_ENEMY(ctx)->life =
            TH08_ECL_CONTEXT_ENEMY(ctx)->maxLife = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->bossSlot == 0 &&
            (((TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 >>
               ENEMY_FLAG_BOSS_SHIFT) & 1) != 0))
            for (i32 i = 0; i < 8; ++i)
                g_Gui.SetBossGaugeSlot(i, 0.0f, 0.0f);
        break;
    case ECL_OPCODE_SET_BOSS_GAUGE_SLOT:
    {
        i32 index = TH08_ECL_READ_I(ctx, 0);
        g_Gui.SetBossGaugeSlot(
            index,
            (f32)TH08_ECL_READ_I(ctx, 1) / (f32)TH08_ECL_CONTEXT_ENEMY(ctx)->maxLife,
            (f32)TH08_ECL_READ_I(ctx, 2) / (f32)TH08_ECL_CONTEXT_ENEMY(ctx)->maxLife);
        g_Gui.SetBossGaugeColor(
            index, TH08_ECL_READ_I(ctx, 3));
        break;
    }
    case ECL_OPCODE_START_SPELL: StartEnemySpell(
        TH08_ECL_CONTEXT_ENEMY(ctx),
        reinterpret_cast<EclSpellCardInstructionArgs *>(
            TH08_ECL_CONTEXT_INSTRUCTION(ctx))); break;
    case ECL_OPCODE_END_SPELL: EndEnemySpell(
        TH08_ECL_CONTEXT_ENEMY(ctx),
        reinterpret_cast<EclSpellCardInstructionArgs *>(
            TH08_ECL_CONTEXT_INSTRUCTION(ctx))); break;
    case ECL_OPCODE_SET_BOSS_TIMER: TH08_ECL_CONTEXT_ENEMY(ctx)->bossTimer = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_SET_LIFE_CALLBACK:
        if (TH08_ECL_PRESENTATION_WRITES_ALLOWED())
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)->lifeCallbackThresholds[
                TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_READ_I(ctx, 1);
            TH08_ECL_CONTEXT_ENEMY(ctx)->lifeCallbackSubIds[
                TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_READ_I(ctx, 2);
        }
        else
            TH08_ECL_CONTEXT_ENEMY(ctx)->lifeCallbackThresholds[
                TH08_ECL_READ_I(ctx, 0)] =
                TH08_ECL_READ_I(ctx, 1);
        break;
    case ECL_OPCODE_SET_TIMER_CALLBACK:
        if (((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >>
              GameManagerFlags::SPELL_PRACTICE_SHIFT) & 1) == 0) ||
            ((((*reinterpret_cast<u32 *>(&g_GameManager.flags)) >>
              GameManagerFlags::PLAYER_DEATH_DISSOLVE_SHIFT) &
             GameManagerFlags::PLAYER_DEATH_DISSOLVE_MASK) == 0))
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)->timerCallbackThresholdFrames =
                TH08_ECL_READ_I(ctx, 0);
            TH08_ECL_CONTEXT_ENEMY(ctx)->timerCallbackSubId =
                TH08_ECL_READ_I(ctx, 1);
        }
        else
            TH08_ECL_CONTEXT_ENEMY(ctx)->timerCallbackThresholdFrames =
                TH08_ECL_READ_I(ctx, 0);
        TH08_ECL_CONTEXT_ENEMY(ctx)->bossTimer = 0;
        break;

    case ECL_OPCODE_SET_CHILD_ECL:
    {
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt])
            g_ZunMemory.Free(
                TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt]);
        TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt] = 0;

        if (TH08_ECL_READ_I(ctx, 1) >= 0)
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt] =
                static_cast<EnemyChildEclBlock *>(
                    g_ZunMemory.Alloc(sizeof(EnemyChildEclBlock), "ECLInt"));
            if (TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt])
            {
                memset(
                    TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt],
                    0, sizeof(EnemyChildEclBlock));
                TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt]->subId =
                    TH08_ECL_READ_I(ctx, 1);
                g_EclManager.CallEclSub(
                    &(TH08_ECL_CONTEXT_ENEMY(ctx)
                          ->childEclBlocks[lhsInt]
                          ->eclContext),
                    TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt]->subId);
                memcpy(TH08_ECL_CONTEXT_ENEMY(ctx)->childEclBlocks[lhsInt]
                           ->eclContext.intVariables,
                       TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->intVariables,
                       offsetof(EnemyEclContext, secondaryTime) -
                           offsetof(EnemyEclContext, intVariables));
            }
        }
        break;
    }
    case ECL_OPCODE_SPAWN_EFFECT:
        g_EffectManager.SpawnEffect(TH08_ECL_READ_I(ctx, 0), D3DXVECTOR3_PTR(
                                         &TH08_ECL_CONTEXT_ENEMY(ctx)->position),
                                     TH08_ECL_READ_I(ctx, 1), *TH08_ECL_WRITE_I(ctx, 2));
        break;
    case ECL_OPCODE_SPAWN_EFFECT_WITH_VELOCITY:
    {
        Float3 vector;
        vector.x = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        vector.y = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 4))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 4)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 4)));
        vector.z = ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 5))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 5)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 5)));
        g_EffectManager.SpawnEffectWithVelocity(
            TH08_ECL_READ_I(ctx, 0),
            D3DXVECTOR3_PTR(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->position),
            D3DXVECTOR3_PTR(&vector),
            TH08_ECL_READ_I(ctx, 1), *TH08_ECL_WRITE_I(ctx, 2));
        break;
    }
    case ECL_OPCODE_SET_ITEM_DROP_TYPE: TH08_ECL_CONTEXT_ENEMY(ctx)->itemDropType = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_SET_ITEM_DROP_COUNTS:
        TH08_ECL_CONTEXT_ENEMY(ctx)->pointItemDropCount = TH08_ECL_READ_I(ctx, 0);
        TH08_ECL_CONTEXT_ENEMY(ctx)->powerOrPointItemDropCount = TH08_ECL_READ_I(ctx, 1);
        break;
    case ECL_OPCODE_DROP_POWER_OR_POINT_ITEMS:
    {
        struct Op142Locals
        {
            Float3 position;
            i32 i;
        } locals;
        i32 count = TH08_ECL_READ_I(ctx, 0);
        for (locals.i = 0; locals.i < count; ++locals.i)
        {
            locals.position = TH08_ECL_CONTEXT_ENEMY(ctx)->position;
            ((f32 *)locals.position)[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            ((f32 *)locals.position)[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            if (g_GameManager.GetPower() < 0x80)
                g_ItemManager.SpawnItem(&locals.position,
                                        locals.i != 0 ? ITEM_POWER_SMALL : ITEM_POWER_BIG,
                                        ITEM_STATE_DEFAULT);
            else
                g_ItemManager.SpawnItem(&locals.position, ITEM_POINT, ITEM_STATE_DEFAULT);
        }
        break;
    }
    case ECL_OPCODE_DROP_POINT_ITEMS:
    {
        struct Op168Locals
        {
            Float3 position;
            i32 i;
        } locals;
        i32 count = TH08_ECL_READ_I(ctx, 0);
        for (locals.i = 0; locals.i < count; ++locals.i)
        {
            locals.position = TH08_ECL_CONTEXT_ENEMY(ctx)->position;
            ((f32 *)locals.position)[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            ((f32 *)locals.position)[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(FLOAT3_PTR(&locals.position), ITEM_POINT,
                                    ITEM_STATE_DEFAULT);
        }
        break;
    }
    case ECL_OPCODE_SET_ANM_ROTATION_ENABLED:
        reinterpret_cast<EnemyFlag1Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->rotateAnmWithMovement = TH08_ECL_RAW_BYTE(ctx, 0);
        break;
    case ECL_OPCODE_CALL_EX_INSTRUCTION:
        g_EclExInsn[TH08_ECL_READ_I(ctx, 0)](
            TH08_ECL_CONTEXT_ENEMY(ctx),
            reinterpret_cast<EclExInstruction *>(
                TH08_ECL_CONTEXT_INSTRUCTION(ctx)));
        break;
    case ECL_OPCODE_SET_REPEATING_EX_INSTRUCTION:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->perFrameCallback =
                g_EclExInsn[TH08_ECL_READ_I(ctx, 0)];
            TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->perFrameInstruction =
                reinterpret_cast<EclExInstruction *>(
                    TH08_ECL_CONTEXT_INSTRUCTION(ctx));
        }
        else
            TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->perFrameCallback = 0;
        break;
    case ECL_OPCODE_ADD_TIME:
        TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->time +=
            TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_SPAWN_ITEM: g_ItemManager.SpawnItem(&TH08_ECL_CONTEXT_ENEMY(ctx)->position, static_cast<ItemType>(TH08_ECL_READ_I(ctx, 0)), ITEM_STATE_DEFAULT); break;
    case ECL_OPCODE_SET_BACKGROUND_SCRIPT_LABEL: g_Background.pendingStageScriptLabel = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_SET_BOSS_LIFE_MARKER_COUNT:
        g_Gui.SetBossLifeMarkerCount(TH08_ECL_READ_I(ctx, 0));
        g_GameManager.catkData[SPELLCARD_ST2_BOSS_3L]
            .inGameHistory.maxBonus[SHOT_MARISA_ALICE] += 0x708;
        break;
    case ECL_OPCODE_SPAWN_ENEMY_AT_POSITION:
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->life > 0)
        {
            SpawnPacketTyped packet93;
            Enemy *spawned93;
            {
                D3DXVECTOR3 position93;
                memcpy(&packet93, TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands, sizeof(packet93));
                position93.x =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
                        ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(packet93.position.x)
                        : packet93.position.x;
                position93.y =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                        ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(packet93.position.y)
                        : packet93.position.y;
                position93.z =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                        ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(packet93.position.z)
                        : packet93.position.z;
                spawned93 = g_EnemyManager.SpawnEnemy2(
                    packet93.eclSubroutineId, &position93,
                    TH08_ECL_READ_I(ctx, 4), TH08_ECL_READ_I(ctx, 5),
                    TH08_ECL_READ_I(ctx, 6),
                    TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->intVariables);
            }
            (void)spawned93;
        }
        break;
    case ECL_OPCODE_SPAWN_ENEMY_RELATIVE:
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->life > 0)
        {
            SpawnPacketTyped packet94;
            Enemy *spawned94;
            {
                D3DXVECTOR3 position94;
                memcpy(&packet94, TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operands, sizeof(packet94));
                position94.x =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
                        ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(packet94.position.x)
                        : packet94.position.x;
                position94.y =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
                        ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(packet94.position.y)
                        : packet94.position.y;
                position94.z =
                    (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                        ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(packet94.position.z)
                        : packet94.position.z;
                FLOAT3_PTR(&position94)->operator+=(
                    TH08_ECL_CONTEXT_ENEMY(ctx)->position);
                spawned94 = g_EnemyManager.SpawnEnemy2(
                    packet94.eclSubroutineId, &position94,
                    TH08_ECL_READ_I(ctx, 4), TH08_ECL_READ_I(ctx, 5),
                    TH08_ECL_READ_I(ctx, 6),
                    TH08_ECL_CONTEXT_ENEMY(ctx)->activeEclContext->intVariables);
            }
            (void)spawned94;
        }
        break;
    case ECL_OPCODE_KILL_ALL_NON_BOSS_ENEMIES:
        g_EnemyManager.KillAllNonBossEnemies(8000, 0);
        break;
    case ECL_OPCODE_INTERRUPT_MAIN_ANM:
        TH08_ECL_CONTEXT_ENEMY(ctx)->vm.pendingInterrupt =
            (i16)TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_INTERRUPT_SECONDARY_ANM:
        TH08_ECL_CONTEXT_ENEMY(ctx)
            ->secondaryVms[TH08_ECL_RAW_I(ctx, 0)].pendingInterrupt =
            (i16)TH08_ECL_RAW_U16(ctx, 4);
        break;
    case ECL_OPCODE_CLEAR_BULLETS_FOR_TRANSITION: g_BulletManager.ClearBulletsForTransition(); break;

    case ECL_OPCODE_SET_BULLET_SOUNDS:
        if (TH08_ECL_READ_I(ctx, 0) >= 0)
        {
            TH08_ECL_CONTEXT_ENEMY(ctx)
                ->bulletSpawnDescriptor.spawnSound = TH08_ECL_READ_I(ctx, 0);
            TH08_ECL_CONTEXT_ENEMY(ctx)
                ->bulletSpawnDescriptor.transformFlags |= BULLET_TRANSFORM_PLAY_SPAWN_SOUND;
        }
        else
            TH08_ECL_CONTEXT_ENEMY(ctx)
                ->bulletSpawnDescriptor.transformFlags &= ~BULLET_TRANSFORM_PLAY_SPAWN_SOUND;
        TH08_ECL_CONTEXT_ENEMY(ctx)
            ->bulletSpawnDescriptor.transformSound = TH08_ECL_READ_I(ctx, 1);
        break;
    case ECL_OPCODE_SET_CALL_STACK_DISABLED:
        reinterpret_cast<EnemyFlag1Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->disableEclCallStack =
            TH08_ECL_RAW_BYTE(ctx, 0);
        break;
    case ECL_OPCODE_SET_BULLET_RANK_INFLUENCE:
        TH08_ECL_CONTEXT_ENEMY(ctx)->bulletRankInfluence.speedLow =
            TH08_ECL_READ_F_BITCAST_FIELD(
                ctx, BulletRankInfluenceInstructionArgs, speedLow);
        TH08_ECL_CONTEXT_ENEMY(ctx)->bulletRankInfluence.speedHigh =
            TH08_ECL_READ_F_BITCAST_FIELD(
                ctx, BulletRankInfluenceInstructionArgs, speedHigh);
        TH08_ECL_CONTEXT_ENEMY(ctx)->bulletRankInfluence.count1Low =
            (i16)TH08_ECL_READ_I_FIELD(
                ctx, BulletRankInfluenceInstructionArgs, count1Low);
        TH08_ECL_CONTEXT_ENEMY(ctx)->bulletRankInfluence.count1High =
            (i16)TH08_ECL_READ_I_FIELD(
                ctx, BulletRankInfluenceInstructionArgs, count1High);
        TH08_ECL_CONTEXT_ENEMY(ctx)->bulletRankInfluence.count2Low =
            (i16)TH08_ECL_READ_I_FIELD(
                ctx, BulletRankInfluenceInstructionArgs, count2Low);
        TH08_ECL_CONTEXT_ENEMY(ctx)->bulletRankInfluence.count2High =
            (i16)TH08_ECL_READ_I_FIELD(
                ctx, BulletRankInfluenceInstructionArgs, count2High);
        break;
    case ECL_OPCODE_RESET_BOSS_TIMER_CALLBACK:
        TH08_ECL_CONTEXT_ENEMY(ctx)->timerCallbackSubId =
            (i32)TH08_ECL_CONTEXT_ENEMY(ctx)->deathCallbackSubId;
        TH08_ECL_CONTEXT_ENEMY(ctx)->bossTimer = 0;
        break;
    case ECL_OPCODE_SET_TIMEOUT_SPELL:
        reinterpret_cast<EnemyFlag1Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->timeoutSpell =
            TH08_ECL_RAW_BYTE(ctx, 0);
        g_Spellcard.scoreLimit = 99999990;
        break;
    case ECL_OPCODE_SET_SPECIAL_INTERACTION:
        reinterpret_cast<EnemyFlag1Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->specialInteraction =
            TH08_ECL_RAW_BYTE(ctx, 0);
        TH08_ECL_CONTEXT_ENEMY(ctx)->drawGroup = 2;
        break;
    case ECL_OPCODE_SET_TRAIL:
        TH08_ECL_CONTEXT_ENEMY(ctx)->trailFlags = TH08_ECL_RAW_BYTE(
            ctx, offsetof(TrailInstructionArgs, flags));
        TH08_ECL_CONTEXT_ENEMY(ctx)->trailHistoryLength =
            (u16)TH08_ECL_READ_I_FIELD(
                ctx, TrailInstructionArgs, historyLength);
        TH08_ECL_CONTEXT_ENEMY(ctx)->trailCollisionLength =
            (u16)TH08_ECL_READ_I_FIELD(
                ctx, TrailInstructionArgs, collisionLength);
        TH08_ECL_CONTEXT_ENEMY(ctx)->trailSampleStride =
            (u16)TH08_ECL_READ_I_FIELD(
                ctx, TrailInstructionArgs, sampleStride);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->trailFlags & ENEMY_TRAIL_RENDER_AS_STRIP)
            g_AnmManager->InitializeHorizontalTextureStrip(
                &TH08_ECL_CONTEXT_ENEMY(ctx)->vm,
                TH08_ECL_CONTEXT_ENEMY(ctx)->trailVertices,
                (TH08_ECL_CONTEXT_ENEMY(ctx)->trailHistoryLength /
                 TH08_ECL_CONTEXT_ENEMY(ctx)->trailSampleStride) << 1);
        break;
    case ECL_OPCODE_SET_DAMAGE_REDUCTION_TIMER: TH08_ECL_CONTEXT_ENEMY(ctx)->damageReductionTimer = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_REMOVE_BULLETS_IN_RADIUS:
        g_BulletManager.RemoveBulletsInRadius(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->worldPosition, ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))));
        break;
    case ECL_OPCODE_REMOVE_ALL_BULLETS: g_BulletManager.RemoveAllBullets(4); break;
    case ECL_OPCODE_SET_SPELL_EFFECT_TRACKING_DISABLED:
        lhsInt = TH08_ECL_READ_I(ctx, 0);
        g_Spellcard.SetEffectTrackingDisabled(lhsInt);
        if (lhsInt == 0)
            g_Spellcard.SetStoredVector(((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 1))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 1))), ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))), ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3))));
        break;
    case ECL_OPCODE_SET_MAIN_ANM_ROTATION:
        TH08_ECL_CONTEXT_ENEMY(ctx)->vm.rotation.z =
            ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 0)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)));
        break;
    case ECL_OPCODE_POLAR_TO_CARTESIAN_ALT:
        *TH08_ECL_WRITE_F(ctx, 1) =
            sinf(((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2)) ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2))) : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))) * ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        *TH08_ECL_WRITE_F(ctx, 0) =
            cosf(((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 2))
            ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))
            : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 2)))) *
            ((TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & (1U << 3))
                 ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                       *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)))
                 : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 3)));
        break;
    case ECL_OPCODE_RANDOM_HORIZONTAL_ANGLE:
        if (
            FLOAT3_PTR(&g_Player.position)->x <
                TH08_ECL_AT(ctx, Float3, TH08_ECL_ENEMY_POSITION_OFFSET).x &&
            96.0f < TH08_ECL_AT(ctx, f32, TH08_ECL_ENEMY_POSITION_OFFSET) ||
            288.0f < TH08_ECL_AT(ctx, f32, TH08_ECL_ENEMY_POSITION_OFFSET))
            *TH08_ECL_WRITE_F(ctx, 0) = AddNormalizeAngle(
                g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
        else
            *TH08_ECL_WRITE_F(ctx, 0) =
                g_Rng.GetRandomF32InRange(1.5707964f) - 0.78539819f;
        break;
    case ECL_OPCODE_SET_TIMER_PAUSED:
        reinterpret_cast<EnemyFlag1Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->pauseTimer = TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_SET_NO_DAMAGE_DURING_STOP:
        reinterpret_cast<EnemyFlag1Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags1)->noDamageDuringStop = TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_PREPARE_STAGE_TIME_STOP:
    {
        *reinterpret_cast<u32 *>(&g_GameManager.flags) =
            (*reinterpret_cast<u32 *>(&g_GameManager.flags) &
             ~GameManagerFlags::PLAYER_DEATH_DISSOLVE_WORD_MASK) |
            GameManagerFlags::PLAYER_DEATH_DISSOLVE_MODE_1;
        *reinterpret_cast<u32 *>(&g_GameManager.flags) &=
            ~GameManagerFlags::SUPPRESS_PLAYER_SHOTS_MASK;
        if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >>
              GameManagerFlags::SPELL_PRACTICE_SHIFT) & 1) == 0)
        {
            if (g_GameManager.currentStage == 6 || g_GameManager.currentStage == 7)
                *reinterpret_cast<u32 *>(&g_GameManager.flags) |=
                    GameManagerFlags::SUPPRESS_PLAYER_SHOTS_MASK;
        }
        else if ((g_GameManager.currentSpellCardNumber >= 0x8F &&
                  g_GameManager.currentSpellCardNumber <= 0x92) ||
                 (g_GameManager.currentSpellCardNumber >= 0xAB &&
                  g_GameManager.currentSpellCardNumber <= 0xBE))
            *reinterpret_cast<u32 *>(&g_GameManager.flags) |=
                GameManagerFlags::SUPPRESS_PLAYER_SHOTS_MASK;
        TH08_ECL_CONTEXT_ENEMY(ctx)->flags1 |= ENEMY_FLAG_PAUSE_TIMER;
        break;
    }
    // Opcodes 82 and 83 are emitted here in integrated RunEcl to reproduce
    // the target's late physical handler order.
    case ECL_OPCODE_SET_MINIMUM_PLAYER_DISTANCE:
        TH08_ECL_CONTEXT_ENEMY(ctx)->minimumPlayerDistanceSquared =
            (TH08_ECL_CONTEXT_INSTRUCTION(ctx)->operandFlags & 1U)
                ? TH08_ECL_CONTEXT_ENEMY(ctx)->ResolveFloat(
                      *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0)))
                : *reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(ctx, 0));
        TH08_ECL_CONTEXT_ENEMY(ctx)->minimumPlayerDistanceSquared *=
            TH08_ECL_CONTEXT_ENEMY(ctx)->minimumPlayerDistanceSquared;
        break;
    case ECL_OPCODE_SET_FORM_EFFECT_ENABLED:
        reinterpret_cast<EnemyFlag2Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags2)->formEffect =
            TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_REPLACE_ALIGNMENT_EFFECT:
    {
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->alignmentEffect)
            TH08_ECL_CONTEXT_ENEMY(ctx)->alignmentEffect->active = 0;
        TH08_ECL_CONTEXT_ENEMY(ctx)->alignmentEffect =
            g_EffectManager.SpawnEffectInSecondaryPool(
                TH08_ECL_READ_I(ctx, 0) + 0x20,
                D3DXVECTOR3_PTR(
                    &TH08_ECL_CONTEXT_ENEMY(ctx)->worldPosition),
                1, -1);
        TH08_ECL_CONTEXT_ENEMY(ctx)->alignmentEffect->vm.SetInterrupt(
            g_Player.IsYoukai() ? 2 : 1);
        if (TH08_ECL_CONTEXT_ENEMY(ctx)->enemyIndex & 1)
            TH08_ECL_CONTEXT_ENEMY(ctx)->alignmentEffect->vm.angleVel.z =
                -TH08_ECL_CONTEXT_ENEMY(ctx)->alignmentEffect->vm.angleVel.z;
        break;
    }
    case ECL_OPCODE_SET_TIMELINE_SPAWNS_SUPPRESSED: g_EnemyManager.suppressTimelineSpawns = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_SET_PHASE_STARTING_LIFE: TH08_ECL_CONTEXT_ENEMY(ctx)->phaseStartingLife = TH08_ECL_READ_I(ctx, 0); break;
    case ECL_OPCODE_START_STAGE_BACKGROUND_SEQUENCE: g_Gui.StartStageBackgroundSequence(); break;
    case ECL_OPCODE_HIDE_CLOCK_TIME: g_Gui.HideClockTime(); break;
    case ECL_OPCODE_INCREMENT_CLOCK_TIME:
        if (static_cast<i8>(g_GameManager.GetClockTime()) < 12)
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT_LOUD, 0);
            g_GameManager.AddToClockTime(1);
            if (static_cast<i8>(g_GameManager.GetClockTime()) == 12)
                g_Gui.FlashClockTimeFast();
            else
                g_Gui.FlashClockTimeSlow();
        }
        break;
    case ECL_OPCODE_SET_EXTRA_ANM_FIXED_OFFSET:
        reinterpret_cast<EnemyFlag2Bits *>(
            &TH08_ECL_CONTEXT_ENEMY(ctx)->flags2)->extraVmFixedOffset = TH08_ECL_READ_I(ctx, 0);
        break;
    case ECL_OPCODE_SET_BONUS_UPDATES_DISABLED: g_Spellcard.SetBonusUpdatesDisabled(TH08_ECL_READ_I(ctx, 0)); break;
#endif // !TH08_ECL_RUN_DECLARATIONS_ONLY
