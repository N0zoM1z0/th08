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

namespace th08
{
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

struct SpawnPacket
{
    i32 values[7];
};

struct Interpolator
{
    void *callback;       // +0x00, target calls with Enemy in ECX
    u8 timer[0x0C];       // +0x04
    i32 duration;         // +0x10
    i32 unknown14;        // +0x14
    i32 easing;           // +0x18, accepted values 1..6
    i32 unknown1C;
    i32 unknown20;
    i32 unknown24;
    i32 unknown28;
    f32 affectedVariable; // +0x2C; 10042..10044 mean position motion
};

enum DispatchResult
{
    DISPATCH_ADVANCE,
    DISPATCH_ENTER_SUBROUTINE,
};

// The coordinator should bind these operations to the indicated target
// functions/globals.  Their descriptive names are provisional.
struct TargetApi
{
    virtual i32 ResolveInt(u8 *enemy, i32 raw) = 0;                         // 0x0041F420
    virtual f32 ResolveFloat(u8 *enemy, f32 raw) = 0;                       // 0x00420120
    virtual i32 *ResolveIntLValue(u8 *enemy, i32 *raw, u16 flags, i32 idx) = 0; // 0x0041FE10
    virtual f32 *ResolveFloatLValue(u8 *enemy, f32 *raw, u16 flags, i32 idx) = 0; // 0x00420950

    virtual void TransformSpawnVector(Vec3 *value, const Vec3 *origin) = 0; // 0x00410A70
    virtual void *SpawnFromPacket(i32 type, const Vec3 *position, i32 a,
                                  i32 b, i32 c, i32 *contextInts) = 0;       // 0x0042A680
    virtual void ClearOrLimitBullets(i32 count, i32 mode) = 0;              // 0x0042EFB0
    virtual void DispatchShotInstruction(u8 *enemy, RawInstruction *insn) = 0; // 0x00422720
    virtual i32 RandomInt(i32 upper, i32 lower) = 0;                        // 0x00421BA0
    virtual i32 ConvertTime(i32 value) = 0;                                // 0x00406EF0
    virtual f32 RandomFloat() = 0;                                         // 0x0043ED50
    virtual f32 RandomFloatInRange(f32 range) = 0;                          // 0x0040D390
    virtual f32 AddNormalizeAngle(f32 a, f32 b) = 0;                        // 0x0043EDB0
    virtual f32 AngleToPlayer(const Vec3 *position) = 0;                    // 0x0044C1B0
    virtual f32 VectorAngle(f32 y, f32 x) = 0;                              // 0x0040C7B0
    virtual f32 Sin(f32 angle) = 0;                                        // 0x00409060
    virtual f32 Cos(f32 angle) = 0;                                        // 0x00408D40

    virtual void AddVectors(Vec3 *out, const Vec3 *left,
                            const Vec3 *right) = 0;                          // 0x00409080
    virtual void FinalizeVectorState(void *state) = 0;                      // 0x00430E10
    virtual void Call00415C60() = 0;
    virtual void *CreateModeObject(void *state) = 0;                         // 0x00430F20
    virtual void Call00421280(u8 *enemy) = 0;
    virtual void Call004212E0(u8 *enemy) = 0;
    virtual void Call004224A0(u8 *enemy) = 0;
    virtual void Call00423130(i32 value) = 0;
    virtual void Call0041FDF0(u8 *enemy, i32 value) = 0;
    virtual void Call0041F0B0(i32 value) = 0;
    virtual void Call0041F040(f32 a, f32 b, f32 c) = 0;
    virtual void Call0041F0E0(i32 value) = 0;

    virtual void SetTimer(void *timer, i32 value) = 0;                      // 0x004065F0
    virtual void ResetTimer(void *timer, i32 value) = 0;                    // 0x00406640
    virtual i32 TimerDone(void *timer, i32 duration) = 0;                   // 0x0040B8E0
    virtual f32 TimerValue(void *timer) = 0;                                // 0x0040B8C0
    virtual void *Allocate(i32 size, const char *tag) = 0;                  // 0x0040B880
    virtual void Free(void *allocation) = 0;                               // 0x0040B8A0
    virtual void InitializeEclContext(void *context, u16 subId) = 0;        // 0x00418450

    virtual void PlayPositioned(i32 id, i32 xBits) = 0;                     // 0x0045D660
    virtual void *SpawnEffect00425430(i32 id, const Vec3 *position,
                                      i32 count, i32 color) = 0;
    virtual void *SpawnEffect00425B70(i32 id, const Vec3 *position,
                                      i32 count, i32 color) = 0;
    virtual void *SpawnEffectWithVector(i32 id, const Vec3 *position,
                                        const Vec3 *vector, i32 count,
                                        i32 color) = 0;                     // 0x00425650
    virtual void SpawnItem(const Vec3 *position, i32 type, i32 arg) = 0;    // 0x004400A0
    virtual void ConfigureBoss(void *enemySubobject, void *state,
                               i32 ratioTimesTwo) = 0;                        // 0x004649A0

    virtual void SetBossPresence(i32 value) = 0;                            // 0x00422C20
    virtual void SetBossUiState(i32 slot, i32 state) = 0;                   // 0x00422BB0
    virtual void UnregisterBoss(u8 *enemy) = 0;                             // 0x0042A820
    virtual void SetBossHealth(f32 value) = 0;                              // 0x004230C0
    virtual void SetBossMarker(i32 slot, const Vec3 *position) = 0;         // 0x00422BE0
    virtual void SetBossGaugeSlot(i32 slot, f32 a, f32 b) = 0;              // 0x004230E0
    virtual void SetBossGaugeValue(i32 slot, i32 value) = 0;                // 0x00423110
    virtual i32 IsYoukai() = 0;                                            // 0x0040BC40
    virtual void SelectPlayerMode(i32 value) = 0;                           // 0x00407120

    virtual void SetAngleFromPosition(Vec3 *position, f32 angle) = 0;       // 0x00430D30
    virtual void Call00430830(i32 value) = 0;
    virtual void Call00439007() = 0;
    virtual void Call004390D6() = 0;
    virtual void Call00439093() = 0;
    virtual void Call00439050() = 0;
    virtual i8 GetGameState() = 0;                                         // 0x00406DD0
    virtual void SetGameState(i32 value) = 0;                               // 0x00406DF0
    virtual void PlaySound(i32 id, i32 arg) = 0;                            // 0x0045D550

    virtual void CallFunctionTable(i32 index) = 0;                          // table 0x004C6CB0
    virtual void *FunctionTableEntry(i32 index) = 0;
    virtual bool PresentationWritesAllowed() = 0;                           // bits in 0x0164D0B4
    virtual void SetBossSlot(i32 index, u8 *enemy) = 0;

    virtual f32 PlayerX() = 0;                                             // 0x017D61AC
    virtual i32 PlayerItemCount() = 0;                                     // 0x00422480 result
    virtual i32 &Global004EA290() = 0;
    virtual i32 &Global004ECCA8() = 0;
    virtual i32 &Global00F54CEC() = 0;
    virtual i32 &Global00F54E2C() = 0;
    virtual i32 &Global0164D30C() = 0;
    virtual u32 &GameFlags0164D0B4() = 0;
    virtual i32 GameStage0164D2CC() = 0;
    virtual i32 GameStateIndex0164D0B8() = 0;

    virtual void RunContextCallback(void *callback, u8 *enemy,
                                    void *callbackArg) = 0;
    virtual void RunInterpolatorCallback(void *callback, u8 *enemy,
                                         Interpolator *entry, f32 value) = 0;
    virtual void ResetEnemyAfterRun(u8 *enemy) = 0;                         // 0x00422C40
    virtual void FinalizeEnemyAfterRun(u8 *enemy) = 0;                      // 0x00423150
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

static void CopyDwords(void *destination, const void *source, i32 count)
{
    i32 *out = (i32 *)destination;
    const i32 *in = (const i32 *)source;
    for (i32 i = 0; i < count; ++i)
        out[i] = in[i];
}

static void SpawnFromInstruction(Context &ctx, bool transformPosition)
{
    SpawnPacket packet;
    CopyDwords(packet.values, ctx.instruction->operands, 7);

    Vec3 position;
    position.x = ctx.ReadF(1);
    position.y = ctx.ReadF(2);
    position.z = ctx.ReadF(3);
    if (transformPosition)
        ctx.api->TransformSpawnVector(&position, &ctx.At<Vec3>(0x2D34));

    ctx.api->SpawnFromPacket(packet.values[0], &position, ctx.ReadI(4), ctx.ReadI(5),
                             ctx.ReadI(6), (i32 *)(ctx.CurrentEclContext() + 0x18));
}

static void SpawnRandomizedItems(Context &ctx, bool selectFirstType)
{
    i32 count = ctx.ReadI(0);
    for (i32 i = 0; i < count; ++i)
    {
        Vec3 position = ctx.At<Vec3>(0x2D34);
        position.x += ctx.api->RandomFloat() * 128.0f - 64.0f;
        position.y += ctx.api->RandomFloat() * 128.0f - 64.0f;

        i32 type = 1;
        if (selectFirstType && ctx.api->PlayerItemCount() < 0x80)
            type = i == 0 ? 2 : 0;
        ctx.api->SpawnItem(&position, type, 0);
    }
}

static DispatchResult DispatchOpcode93To184(Context &ctx)
{
    u8 *object;
    i32 index;
    i32 value;

    switch (ctx.instruction->opcode)
    {
    case 93:
        if (ctx.At<i32>(0x2DFC) > 0)
            SpawnFromInstruction(ctx, false);
        break;
    case 94:
        if (ctx.At<i32>(0x2DFC) > 0)
            SpawnFromInstruction(ctx, true);
        break;
    case 95:
        ctx.api->ClearOrLimitBullets(8000, 0);
        break;

    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
        if (ctx.At<i32>(0x2DFC) > 0)
        {
            if (ctx.At<u32>(0x3324) & 0x00020000)
                CopyDwords(ctx.enemy + 0x3034, ctx.instruction, 11);
            else
                ctx.api->DispatchShotInstruction(ctx.enemy, ctx.instruction);
        }
        break;

    case 105:
        ctx.At<i32>(0x3060) = ctx.ReadI(0);
        if (ctx.At<i32>(0x3060) != 0)
        {
            value = ctx.At<i32>(0x3060);
            ctx.At<i32>(0x3060) += ctx.api->RandomInt(value / 5, -value / 5);
            ctx.api->SetTimer(ctx.enemy + 0x3064, 0);
        }
        break;
    case 106:
        ctx.At<i32>(0x3060) = ctx.ReadI(0);
        if (ctx.At<i32>(0x3060) != 0)
        {
            value = ctx.At<i32>(0x3060);
            ctx.At<i32>(0x3060) += ctx.api->RandomInt(value / 5, -value / 5);
            ctx.api->SetTimer(ctx.enemy + 0x3064,
                              ctx.api->ConvertTime(ctx.At<i32>(0x3060)));
        }
        break;
    case 107: ctx.At<u32>(0x3324) |= 0x00020000; break;
    case 108: ctx.At<u32>(0x3324) &= ~0x00020000; break;

    case 109:
    {
        Vec3 transformed;
        ctx.api->AddVectors(&transformed, &ctx.At<Vec3>(0x2D34), &ctx.At<Vec3>(0x2DB8));
        ctx.At<Vec3>(0x2E28) = transformed;
        ctx.api->FinalizeVectorState(ctx.enemy + 0x2E24);
        break;
    }
    case 110:
        ctx.At<f32>(0x2DB8) = ctx.ReadF(0);
        ctx.At<f32>(0x2DBC) = ctx.ReadF(1);
        ctx.At<i32>(0x2DC0) = 0;
        break;
    case 111:
    {
        f32 *entry = (f32 *)(ctx.enemy + 0x2E44 + ctx.ReadI(0) * 0x18);
        entry[4] = (f32)ctx.ReadI(1);
        entry[5] = (f32)ctx.ReadI(2);
        entry[2] = (f32)ctx.ReadI(3);
        entry[3] = (f32)ctx.ReadI(4);
        entry[0] = ctx.ReadF(5);
        entry[1] = ctx.ReadF(6);
        break;
    }
    case 112: ctx.api->Call00415C60(); break;

    case 113:
        value = ctx.ReadI(0);
        if (value < 0)
            ctx.At<u32>(0x3020) &= ~0x200;
        else
        {
            ctx.At<i32>(0x3024) = value;
            ctx.At<u32>(0x3020) |= 0x200;
        }
        ctx.At<i32>(0x3028) = ctx.ReadI(1);
        break;

    case 114:
    case 115:
    {
        u8 *state = ctx.enemy + 0x3070;
        Vec3 transformed;
        ctx.api->AddVectors(&transformed, &ctx.At<Vec3>(0x2D88), &ctx.At<Vec3>(0x2DB8));
        *(u16 *)(state + 0x00) = ctx.RawU16(0);
        *(u16 *)(state + 0x02) = (u16)ctx.ReadI16(2, 1);
        *(Vec3 *)(state + 0x04) = transformed;
        *(f32 *)(state + 0x10) = ctx.ReadF(1);
        *(f32 *)(state + 0x18) = ctx.ReadF(2);
        *(f32 *)(state + 0x1D0) = ctx.ReadF(3);
        *(f32 *)(state + 0x1D4) = ctx.ReadF(4);
        *(f32 *)(state + 0x1D8) = ctx.ReadF(5);
        *(f32 *)(state + 0x1DC) = ctx.ReadF(6);
        *(i32 *)(state + 0x1E0) = ctx.ReadI(7);
        *(i32 *)(state + 0x1E4) = ctx.ReadI(8);
        *(i32 *)(state + 0x1E8) = ctx.ReadI(9);
        *(i32 *)(state + 0x1EC) = ctx.RawI(10);
        *(i32 *)(state + 0x1F0) = ctx.RawI(11);
        *(i32 *)(state + 0x1FC) = ctx.RawI(12);
        *(u16 *)(state + 0x1F8) = ctx.instruction->opcode == 115 ? 0 : 1;
        ctx.At<void *>(0x3280 + ctx.At<i32>(0x3300) * 4) = ctx.api->CreateModeObject(state);
        break;
    }
    case 116: ctx.At<i32>(0x3300) = ctx.ReadI(0); break;
    case 117:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
            *(f32 *)(object + 0x554) = ctx.api->AddNormalizeAngle(*(f32 *)(object + 0x554), ctx.ReadF(1));
        break;
    case 118:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
            *(f32 *)(object + 0x554) = ctx.api->AngleToPlayer((Vec3 *)(object + 0x548)) + ctx.ReadF(1);
        break;
    case 119:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
        {
            *(f32 *)(object + 0x548) = ctx.At<f32>(0x2D88) + ctx.ReadF(1);
            *(f32 *)(object + 0x54C) = ctx.At<f32>(0x2D8C) + ctx.ReadF(2);
            *(f32 *)(object + 0x550) = ctx.At<f32>(0x2D90) + ctx.ReadF(3);
        }
        break;
    case 120:
        object = ctx.Object(ctx.ReadI(0));
        *(i32 *)(ctx.CurrentEclContext() + 0x60) = object && *(i32 *)(object + 0x584) ? 1 : 0;
        break;
    case 121:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object && *(i32 *)(object + 0x584) && *(u8 *)(object + 0x598) < 2)
        {
            *(u8 *)(object + 0x598) = 2;
            ctx.api->SetTimer(object + 0x588, 0);
            *(i32 *)(object + 0x564) = *(i32 *)(object + 0x568);
        }
        break;
    case 122: ctx.api->Call00421280(ctx.enemy); break;
    case 123: ctx.api->Call004212E0(ctx.enemy); break;
    case 124: ctx.api->PlayPositioned(ctx.ReadI(0), ctx.At<i32>(0x2D34)); break;
    case 125:
        ctx.At<i16>(0x2D30) = (i16)ctx.ReadI(0);
        return DISPATCH_ENTER_SUBROUTINE;
    case 126:
        ctx.At<i16>(0x2CF0 + ctx.ReadI(1) * 2) = (i16)ctx.ReadI(0);
        break;

    case 127:
        index = ctx.ReadI(0);
        if (index < 0)
        {
            u8 bossSlot = ctx.At<u8>(0x3313);
            if (bossSlot < 4)
                ctx.api->SetBossPresence(0);
            ctx.api->SetBossSlot(bossSlot, 0);
            ctx.At<u32>(0x3324) &= ~2U;
            ctx.api->SetBossUiState(bossSlot, 2);
            ctx.api->UnregisterBoss(ctx.enemy);
            Vec3 offscreen = {-999.0f, -999.0f, 0.0f};
            ctx.api->SetBossMarker(bossSlot, &offscreen);
        }
        else
        {
            ctx.api->SetBossSlot(index, ctx.enemy);
            if (index == 0)
            {
                ctx.api->SetBossPresence(1);
                ctx.api->SetBossHealth(1.0f);
            }
            ctx.At<u32>(0x3324) |= 2;
            ctx.At<u8>(0x3313) = (u8)index;
            ctx.api->SetBossUiState(ctx.At<u8>(0x3313), 1);
            ctx.At<i32>(0x3350) = 0;
        }
        break;

    case 128:
    {
        i32 slot = ctx.At<i32>(0x53C0);
        u8 *effect = (u8 *)ctx.api->SpawnEffect00425430(13, &ctx.At<Vec3>(0x2D34), 1,
                                                        0xFF6060D0);
        ctx.At<u8 *>(0x5360 + slot * 4) = effect;
        *(i32 *)(effect + 0x2EC) = ctx.RawI(1);
        *(i32 *)(effect + 0x2F0) = ctx.RawI(2);
        *(i32 *)(effect + 0x2F4) = ctx.RawI(3);
        ctx.At<i32>(0x53C4) = ctx.RawI(4);
        ctx.At<i32>(0x53C0) = slot + 1;
        break;
    }
    case 129:
        if (ctx.api->PresentationWritesAllowed())
            ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0xFF8FFFFF) | ((ctx.RawByte(0) & 7) << 20);
        break;
    case 130:
        if (ctx.api->PresentationWritesAllowed())
            ctx.At<u16>(0x2CEE) = ctx.RawU16(0);
        break;
    case 131:
        value = ctx.ReadI(0);
        ctx.At<i32>(0x2E00) = value;
        ctx.At<i32>(0x2DFC) = value;
        ctx.At<i32>(0x2E04) = value;
        if (ctx.At<i8>(0x3313) == 0 && (ctx.At<u32>(0x3324) & 2))
            for (i32 i = 0; i < 8; ++i)
                ctx.api->SetBossGaugeSlot(i, 0.0f, 0.0f);
        break;
    case 132: ctx.api->SetTimer(ctx.enemy + 0x2E14, ctx.ReadI(0)); break;
    case 133:
        index = ctx.ReadI(0);
        ctx.At<i32>(0x3358 + index * 4) = ctx.ReadI(1);
        if (ctx.api->PresentationWritesAllowed())
            ctx.At<i32>(0x3368 + index * 4) = ctx.ReadI(2);
        break;
    case 134:
        ctx.At<i32>(0x3378) = ctx.ReadI(0);
        if (ctx.api->PresentationWritesAllowed())
            ctx.At<i32>(0x337C) = ctx.ReadI(1);
        ctx.api->SetTimer(ctx.enemy + 0x2E14, 0);
        break;

    case 135:
    {
        index = ctx.ReadI(0);
        u8 *oldContext = ctx.At<u8 *>(0x3384 + index * 4);
        if (oldContext)
            ctx.api->Free(oldContext);
        ctx.At<u8 *>(0x3384 + index * 4) = 0;

        i32 subId = ctx.ReadI(1);
        if (subId >= 0)
        {
            u8 *child = (u8 *)ctx.api->Allocate(0x24B0, "ECLInt");
            ctx.At<u8 *>(0x3384 + index * 4) = child;
            if (child)
            {
                for (i32 i = 0; i < 0x92C; ++i)
                    ((i32 *)child)[i] = 0;
                *(i32 *)child = subId;
                ctx.api->InitializeEclContext(child + 8, *(u16 *)child);
                CopyDwords(child + 0x20, ctx.CurrentEclContext() + 0x18, 0x1E);
            }
        }
        break;
    }
    case 136: ctx.api->CallFunctionTable(ctx.ReadI(0)); break;
    case 137:
        index = ctx.ReadI(0);
        if (index < 0)
            *(void **)(ctx.CurrentEclContext() + 0x10) = 0;
        else
        {
            *(void **)(ctx.CurrentEclContext() + 0x10) = ctx.api->FunctionTableEntry(index);
            *(RawInstruction **)(ctx.CurrentEclContext() + 0x14) = ctx.instruction;
        }
        break;
    case 138:
        ctx.At<u8>(0x3310) = ctx.RawByte(0);
        ctx.At<u8>(0x3311) = ctx.RawByte(1);
        ctx.At<u8>(0x3312) = ctx.RawByte(2);
        break;
    case 139:
        ctx.api->SpawnEffect00425430(ctx.ReadI(0), &ctx.At<Vec3>(0x2D34),
                                     ctx.ReadI(1), *ctx.WriteI(2));
        break;
    case 140:
    {
        Vec3 vector;
        vector.x = ctx.ReadF(3);
        vector.y = ctx.ReadF(4);
        vector.z = ctx.ReadF(5);
        ctx.api->SpawnEffectWithVector(ctx.ReadI(0), &ctx.At<Vec3>(0x2D34), &vector,
                                       ctx.ReadI(1), *ctx.WriteI(2));
        break;
    }
    case 141: ctx.api->SpawnItem(&ctx.At<Vec3>(0x2D34), ctx.ReadI(0), 0); break;
    case 142: SpawnRandomizedItems(ctx, true); break;
    case 143: ctx.At<i32>(0x3304) = ctx.ReadI(0); break;
    case 144:
        ctx.At<i32>(0x3308) = ctx.ReadI(0);
        ctx.At<i32>(0x330C) = ctx.ReadI(1);
        break;
    case 145:
        ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0xFDFFFFFF) | ((ctx.RawByte(0) & 1) << 25);
        break;
    case 146: ctx.api->Call0041FDF0(ctx.enemy, ctx.ReadI(0)); break;
    case 147: ctx.api->Global004EA290() = ctx.ReadI(0); break;
    case 148:
        ctx.api->Call00423130(ctx.ReadI(0));
        ctx.api->Global0164D30C() += 0x708;
        break;
    case 149: ctx.At<u16>(0x20A) = (u16)ctx.ReadI(0); break;
    case 150:
        ctx.At<u16>(0x4AE + ctx.RawI(0) * 0x2A4) = ctx.RawU16(4);
        break;
    case 151:
        ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0xFBFFFFFF) | ((ctx.RawByte(0) & 1) << 26);
        break;
    case 152:
        ctx.At<f32>(0x2DEC) = ctx.ReadF(0);
        ctx.At<f32>(0x2DF0) = ctx.ReadF(1);
        ctx.At<u16>(0x2DF4) = (u16)ctx.ReadI(2);
        ctx.At<u16>(0x2DF6) = (u16)ctx.ReadI(3);
        ctx.At<u16>(0x2DF8) = (u16)ctx.ReadI(4);
        ctx.At<u16>(0x2DFA) = (u16)ctx.ReadI(5);
        break;
    case 153:
        ctx.At<i32>(0x337C) = (i32)ctx.At<i16>(0x2CEE);
        ctx.api->SetTimer(ctx.enemy + 0x2E14, 0);
        break;
    case 154:
        for (i32 i = 0; i < 0x20; ++i)
            ctx.At<void *>(0x3280 + i * 4) = 0;
        break;
    case 155:
        ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0xF7FFFFFF) | ((ctx.RawByte(0) & 1) << 27);
        ctx.api->Global004ECCA8() = 0x05F5E0F6;
        break;
    case 156:
        ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0xFFFFFF7F) | ((ctx.RawByte(0) & 1) << 7);
        ctx.At<u8>(0x332F) = 2;
        break;
    case 157:
        ctx.At<u8>(0x534C) = ctx.RawByte(0);
        ctx.At<u16>(0x534E) = (u16)ctx.ReadI(1);
        ctx.At<u16>(0x5350) = (u16)ctx.ReadI(2);
        ctx.At<u16>(0x5352) = (u16)ctx.ReadI(3);
        if (ctx.At<u8>(0x534C) & 8)
            ctx.api->ConfigureBoss(ctx.enemy + 0x0C, ctx.enemy + 0x3E14,
                                   (ctx.At<i16>(0x534E) / ctx.At<i16>(0x5352)) << 1);
        break;
    case 158:
        index = ctx.ReadI(0);
        ctx.api->SetBossGaugeSlot(index, (f32)ctx.ReadI(1) / (f32)ctx.At<i32>(0x2E00),
                                  (f32)ctx.ReadI(2) / (f32)ctx.At<i32>(0x2E00));
        ctx.api->SetBossGaugeValue(index, ctx.ReadI(3));
        break;
    case 159: ctx.At<u8>(0x332F) = (u8)ctx.ReadI(0); break;
    case 160: ctx.api->SetTimer(ctx.enemy + 0x5354, ctx.ReadI(0)); break;
    case 161: ctx.api->SetAngleFromPosition(&ctx.At<Vec3>(0x2D88), ctx.ReadF(0)); break;
    case 162: ctx.api->Call00430830(4); break;
    case 163: ctx.api->Global00F54CEC() = ctx.ReadI(0); break;
    case 164:
        value = ctx.ReadI(0);
        ctx.api->Call0041F0B0(value);
        if (value == 0)
            ctx.api->Call0041F040(ctx.ReadF(1), ctx.ReadF(2), ctx.ReadF(3));
        break;
    case 165: ctx.At<f32>(0x14) = ctx.ReadF(0); break;
    case 166:
    {
        f32 angle = ctx.ReadF(2);
        f32 magnitude = ctx.ReadF(3);
        *ctx.WriteF(1) = ctx.api->Sin(angle) * magnitude;
        *ctx.WriteF(0) = ctx.api->Cos(angle) * magnitude;
        break;
    }
    case 167:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
            *(f32 *)(object + 0x554) = ctx.ReadF(1);
        break;
    case 168: SpawnRandomizedItems(ctx, false); break;
    case 169:
        if ((ctx.api->PlayerX() < ctx.At<f32>(0x2D34) && 96.0f < ctx.At<f32>(0x2D34)) ||
            288.0f < ctx.At<f32>(0x2D34))
            *ctx.WriteF(0) = ctx.api->AddNormalizeAngle(
                ctx.api->RandomFloatInRange(1.5707964f) + 2.3561945f, 0.0f);
        else
            *ctx.WriteF(0) = ctx.api->RandomFloatInRange(1.5707964f) - 0.78539819f;
        break;
    case 170:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
            *(u8 *)(object + 0x599) = (u8)ctx.ReadI(1);
        break;
    case 171:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
            *(f32 *)(object + 0x560) = ctx.ReadF(1);
        break;
    case 172:
        index = ctx.ReadI(0);
        object = ctx.Object(index);
        if (object)
        {
            *(f32 *)(object + 0x558) = ctx.ReadF(1);
            *(f32 *)(object + 0x55C) = ctx.ReadF(2);
        }
        break;
    case 173:
        ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0xBFFFFFFF) | ((ctx.ReadI(0) & 1) << 30);
        break;
    case 174:
    {
        u8 *oldEffect = ctx.At<u8 *>(0x53C8);
        if (oldEffect)
            *(u8 *)(oldEffect + 0x350) = 0;
        u8 *effect = (u8 *)ctx.api->SpawnEffect00425B70(ctx.ReadI(0) + 0x20,
                                                        &ctx.At<Vec3>(0x2D88), 1, -1);
        ctx.At<u8 *>(0x53C8) = effect;
        ctx.api->SelectPlayerMode(ctx.api->IsYoukai() ? 2 : 1);
        if (ctx.At<u32>(0x2E0C) & 1)
            *(f32 *)(effect + 0x14) = -*(f32 *)(effect + 0x14);
        break;
    }
    case 175: ctx.api->Global00F54E2C() = ctx.ReadI(0); break;
    case 176:
    {
        u32 base = ctx.api->GameFlags0164D0B4() & 0xFFFFDE7F;
        u32 flags = base | 0x80;
        if (!(ctx.api->GameFlags0164D0B4() & 0x4000))
        {
            if (ctx.api->GameStage0164D2CC() == 6 || ctx.api->GameStage0164D2CC() == 7)
                flags = base | 0x2080;
        }
        else if ((ctx.api->GameStateIndex0164D0B8() > 0x8E && ctx.api->GameStateIndex0164D0B8() < 0x93) ||
                 (ctx.api->GameStateIndex0164D0B8() > 0xAA && ctx.api->GameStateIndex0164D0B8() < 0xBF))
            flags = base | 0x2080;
        ctx.api->GameFlags0164D0B4() = flags;
        ctx.At<u32>(0x3324) |= 0x40000000;
        break;
    }
    case 177: ctx.At<i32>(0x2E04) = ctx.ReadI(0); break;
    case 178: ctx.api->Call004224A0(ctx.enemy); break;
    case 179: ctx.api->Call00439007(); break;
    case 180: ctx.api->Call004390D6(); break;
    case 181:
        if (ctx.api->GetGameState() < 12)
        {
            ctx.api->PlaySound(0x2D, 0);
            ctx.api->SetGameState(1);
            if (ctx.api->GetGameState() == 12)
                ctx.api->Call00439093();
            else
                ctx.api->Call00439050();
        }
        break;
    case 182:
        ctx.At<u32>(0x3328) = (ctx.At<u32>(0x3328) & 0xFFFFFEFF) | ((ctx.ReadI(0) & 1) << 8);
        break;
    case 183:
        ctx.At<u32>(0x3324) = (ctx.At<u32>(0x3324) & 0x7FFFFFFF) | ((u32)ctx.ReadI(0) << 31);
        break;
    case 184: ctx.api->Call0041F0E0(ctx.ReadI(0)); break;
    }

    return DISPATCH_ADVANCE;
}

// Target 0x0041C88A.  The caller must continue dispatching rather than run the
// post-dispatch frame tail when this helper returns.
static void EnterSubroutine(Context &ctx)
{
    *(RawInstruction **)ctx.CurrentEclContext() =
        (RawInstruction *)((u8 *)ctx.instruction + ctx.instruction->nextOffset);

    if (!(ctx.At<u32>(0x3324) & 0x04000000))
    {
        u8 *destination = ctx.At<u8 *>(0x2CA4) + ctx.At<i16>(0x2CEA) * 0x228;
        CopyDwords(destination, ctx.enemy + 0x7F8, 0x8A);
    }

    u16 subId = ctx.At<u16>(0x2CF0 + ctx.At<i16>(0x2D30) * 2);
    ctx.api->InitializeEclContext(ctx.enemy + 0x7F8, subId);
    if (ctx.At<i16>(0x2CEA) < 15)
        ++ctx.At<i16>(0x2CEA);
    ctx.At<i16>(0x2D30) = -1;
}

// Target 0x0041E7E4: all ordinary cases and skipped instructions advance by
// the signed 16-bit nextOffset before re-entering the dispatch loop.
static void AdvanceInstruction(Context &ctx)
{
    ctx.instruction = (RawInstruction *)((u8 *)ctx.instruction + ctx.instruction->nextOffset);
}

static f32 ApplyEasing(f32 value, i32 easing)
{
    f32 inverse;
    switch (easing)
    {
    case 1: return value * value;
    case 2: return value * value * value;
    case 3: return value * value * value * value;
    case 4:
        inverse = 1.0f - value;
        return 1.0f - inverse * inverse;
    case 5:
        inverse = 1.0f - value;
        return 1.0f - inverse * inverse * inverse;
    case 6:
        inverse = 1.0f - value;
        return 1.0f - inverse * inverse * inverse * inverse;
    default: return value;
    }
}

// Target 0x0041E7F8..0x0041ECBD.  Returns false when another allocated child
// ECL context was selected and the caller must resume the dispatch loop.
static bool RunCommonTail(Context &ctx)
{
    if (ctx.At<i32>(0x2DFC) > 0)
    {
        bool restorePosition = false;
        Vec3 savedPosition = ctx.At<Vec3>(0x2D34);
        u8 *eclContext = ctx.CurrentEclContext();

        void *contextCallback = *(void **)(eclContext + 0x10);
        if (contextCallback)
            ctx.api->RunContextCallback(contextCallback, ctx.enemy, *(void **)(eclContext + 0x14));

        Interpolator *entry = (Interpolator *)(eclContext + 0x9C);
        for (i32 i = 0; i < 8; ++i, ++entry)
        {
            if (!entry->callback)
                continue;

            ctx.api->ResetTimer(entry->timer, 0);
            if (ctx.api->TimerDone(entry->timer, entry->duration))
                ctx.api->SetTimer(entry->timer, entry->duration);

            f32 progress = ApplyEasing(ctx.api->TimerValue(entry->timer) / entry->duration,
                                       entry->easing);
            ctx.api->RunInterpolatorCallback(entry->callback, ctx.enemy, entry, progress);
            if (ctx.api->TimerDone(entry->timer, entry->duration))
                entry->callback = 0;

            if (entry->affectedVariable == 10042.0f || entry->affectedVariable == 10043.0f ||
                entry->affectedVariable == 10044.0f)
                restorePosition = true;
        }

        if (restorePosition)
        {
            ctx.At<f32>(0x2D4C) = ctx.At<f32>(0x2D34) - savedPosition.x;
            ctx.At<f32>(0x2D50) = ctx.At<f32>(0x2D38) - savedPosition.y;
            ctx.At<f32>(0x2D94) = ctx.api->VectorAngle(ctx.At<f32>(0x2D50), ctx.At<f32>(0x2D4C));
            ctx.At<Vec3>(0x2D34) = savedPosition;
        }
    }

    if (ctx.activeChildContext == -1)
        ctx.At<i16>(0x2CE8) = ctx.At<i16>(0x2CEA);
    else
        *(i16 *)(ctx.At<u8 *>(0x3384 + ctx.activeChildContext * 4) + 6) = ctx.At<i16>(0x2CEA);

    *(RawInstruction **)ctx.CurrentEclContext() = ctx.instruction;
    ctx.api->ResetTimer(ctx.CurrentEclContext() + 4, 0);

    for (i32 next = ctx.activeChildContext + 1; next < 4; ++next)
    {
        u8 *child = ctx.At<u8 *>(0x3384 + next * 4);
        if (!child)
            continue;

        ctx.At<u8 *>(0x2CA4) = child + 0x230;
        ctx.At<u8 *>(0x2CA0) = child + 8;
        ctx.instruction = *(RawInstruction **)(child + 8);
        *(i32 *)(child + 0x228) = next + 1;
        ctx.At<i16>(0x2CEA) = *(i16 *)(child + 6);
        ctx.activeChildContext = next;
        return false;
    }

    ctx.At<u8 *>(0x2CA4) = ctx.enemy + 0xA20;
    ctx.At<u8 *>(0x2CA0) = ctx.enemy + 0x7F8;
    ctx.api->ResetEnemyAfterRun(ctx.enemy);
    ctx.api->FinalizeEnemyAfterRun(ctx.enemy);
    return true;
}

} // namespace EclRunHighProposal
} // namespace th08
