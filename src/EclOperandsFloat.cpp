#include "EclOperands.hpp"

namespace th08
{
namespace EclOperands
{

// These private overlays expose only the data and calls observed in the
// hash-attested TH08 1.00d target.  Their owning lanes can replace the
// address-based names once the complete Enemy/Player/Spellcard layouts land.
struct TargetVector3
{
    f32 x;
    f32 y;
    f32 z;

    TargetVector3 operator-(const TargetVector3 &other) const;
    f32 Length() const;
};

struct TargetRngOverlay
{
    u32 GetRandomU32();
    f32 GetRandomF32();
    f32 GetRandomF32Signed();

    f32 GetRandomF32InRange(f32 range)
    {
        return GetRandomF32() * range;
    }
};

struct TargetPlayerOverlay
{
    f32 AngleToPlayer(const TargetVector3 *position);
    i32 IsYoukai();
};

struct TargetEnemyHelperOverlay
{
    u8 bytes[1];

    i32 FUN_0041F000();
    i32 FUN_0041FD20();
    i32 FUN_0041FD40();
};

struct TargetSpellcardOverlay
{
    i32 FUN_004178A0();
    i32 FUN_0041FD90();
    i32 FUN_00405260();
};

static __forceinline u8 *IndirectFields(EnemyOverlay *enemy)
{
    return *(u8 **)(enemy->bytes + 0x2CA0);
}

static __forceinline i32 *IndirectIntField(EnemyOverlay *enemy, i32 offset)
{
    return (i32 *)(IndirectFields(enemy) + offset);
}

static __forceinline f32 *IndirectFloatField(EnemyOverlay *enemy, i32 offset)
{
    return (f32 *)(IndirectFields(enemy) + offset);
}

static __forceinline i32 *IntField(EnemyOverlay *enemy, i32 offset)
{
    return (i32 *)(enemy->bytes + offset);
}

static __forceinline f32 *FloatField(EnemyOverlay *enemy, i32 offset)
{
    return (f32 *)(enemy->bytes + offset);
}

static __forceinline TargetVector3 *VectorField(EnemyOverlay *enemy, i32 offset)
{
    return (TargetVector3 *)(enemy->bytes + offset);
}

// Target globals not yet represented by an owner-lane type are deliberately
// named by address.  The addresses below are direct operands in 0x00420120.
extern TargetRngOverlay g_TargetRng0164D520;
extern i32 g_TargetInt0160F538;
extern i32 g_TargetInt0164D334;
extern u8 g_TargetByte0164D0B1;
extern i32 g_TargetInt004ECE20;
extern i32 g_TargetInt004ECE24;
extern i32 g_TargetInt004ECE28;
extern i32 g_TargetInt004ECE2C;
extern f32 g_TargetFloat004ECE30;
extern f32 g_TargetFloat004ECE34;
extern f32 g_TargetFloat004ECE38;
extern f32 g_TargetFloat004ECE3C;
extern TargetVector3 g_TargetVector017D61AC;
extern TargetPlayerOverlay g_TargetPlayer017D5EF8;
extern TargetSpellcardOverlay g_TargetSpellcard004EA670;

// Observed: TH08 1.00d 0x00420120 is a thiscall float resolver.  It converts
// the incoming float to i32, dispatches all IDs 0x2710..0x2773, returns in
// ST(0), and returns the raw operand for out-of-range IDs and 0x2772.
f32 EnemyOverlay::ResolveFloat(f32 operand)
{
    TargetEnemyHelperOverlay *helperEnemy = (TargetEnemyHelperOverlay *)this;

    switch ((i32)operand)
    {
    case 0x2710: return (f32)*IndirectIntField(this, 0x18);
    case 0x2711: return (f32)*IndirectIntField(this, 0x1C);
    case 0x2712: return (f32)*IndirectIntField(this, 0x20);
    case 0x2713: return (f32)*IndirectIntField(this, 0x24);
    case 0x2714: return (f32)*IndirectIntField(this, 0x28);
    case 0x2715: return (f32)*IndirectIntField(this, 0x2C);
    case 0x2716: return (f32)*IndirectIntField(this, 0x30);
    case 0x2717: return (f32)*IndirectIntField(this, 0x34);
    case 0x2718: return (f32)*IntField(this, 0x2CA8);
    case 0x2719: return (f32)*IntField(this, 0x2CAC);
    case 0x271A: return (f32)*IntField(this, 0x2CB0);
    case 0x271B: return (f32)*IntField(this, 0x2CB4);
    case 0x271C: return (f32)*IntField(this, 0x2CB8);
    case 0x271D: return (f32)*IntField(this, 0x2CBC);
    case 0x271E: return (f32)*IntField(this, 0x2CC0);
    case 0x271F: return (f32)*IntField(this, 0x2CC4);

    case 0x2720: return *IndirectFloatField(this, 0x38);
    case 0x2721: return *IndirectFloatField(this, 0x3C);
    case 0x2722: return *IndirectFloatField(this, 0x40);
    case 0x2723: return *IndirectFloatField(this, 0x44);
    case 0x2724: return *IndirectFloatField(this, 0x48);
    case 0x2725: return *IndirectFloatField(this, 0x4C);
    case 0x2726: return *IndirectFloatField(this, 0x50);
    case 0x2727: return *IndirectFloatField(this, 0x54);
    case 0x2728: return *FloatField(this, 0x2CC8);
    case 0x2729: return *FloatField(this, 0x2CCC);
    case 0x272A: return *FloatField(this, 0x2CD0);
    case 0x272B: return *FloatField(this, 0x2CD4);
    case 0x272C: return *FloatField(this, 0x2CD8);
    case 0x272D: return *FloatField(this, 0x2CDC);
    case 0x272E: return *FloatField(this, 0x2CE0);
    case 0x272F: return *FloatField(this, 0x2CE4);

    case 0x2730: return (f32)(g_TargetRng0164D520.GetRandomU32() & 0x7FFFFFFF);
    case 0x2731: return g_TargetRng0164D520.GetRandomF32();
    case 0x2732: return (f32)(i32)g_TargetRng0164D520.GetRandomU32();
    case 0x2733: return g_TargetRng0164D520.GetRandomF32Signed();
    case 0x2734: return (f32)*IndirectIntField(this, 0x58);
    case 0x2735: return (f32)*IndirectIntField(this, 0x5C);
    case 0x2736: return (f32)*IndirectIntField(this, 0x60);
    case 0x2737: return (f32)*IndirectIntField(this, 0x64);
    case 0x2738: return (f32)g_TargetInt0160F538;
    case 0x2739: return (f32)g_TargetInt0164D334;
    case 0x273A: return *FloatField(this, 0x2D88);
    case 0x273B: return *FloatField(this, 0x2D8C);
    case 0x273C: return *FloatField(this, 0x2D90);
    case 0x273D: return g_TargetVector017D61AC.x;
    case 0x273E: return g_TargetVector017D61AC.y;
    case 0x273F: return g_TargetVector017D61AC.z;

    case 0x2740: return g_TargetPlayer017D5EF8.AngleToPlayer(VectorField(this, 0x2D88));
    case 0x2741: return (f32)*IntField(this, 0x2E1C);
    case 0x2742: return (g_TargetVector017D61AC - *VectorField(this, 0x2D88)).Length();
    case 0x2743: return (f32)*IntField(this, 0x2DFC);
    case 0x2744: return (f32)g_TargetByte0164D0B1;
    case 0x2745: return (f32)*IndirectIntField(this, 0x70);
    case 0x2746: return (f32)*IndirectIntField(this, 0x74);
    case 0x2747: return (f32)*IndirectIntField(this, 0x78);
    case 0x2748: return (f32)*IndirectIntField(this, 0x7C);
    case 0x2749: return *IndirectFloatField(this, 0x80);
    case 0x274A: return *IndirectFloatField(this, 0x84);
    case 0x274B: return *IndirectFloatField(this, 0x88);
    case 0x274C: return *IndirectFloatField(this, 0x8C);
    case 0x274D: return (f32)g_TargetInt004ECE20;
    case 0x274E: return (f32)g_TargetInt004ECE24;
    case 0x274F: return (f32)g_TargetInt004ECE28;
    case 0x2750: return (f32)g_TargetInt004ECE2C;

    case 0x2751: return g_TargetFloat004ECE30;
    case 0x2752: return g_TargetFloat004ECE34;
    case 0x2753: return g_TargetFloat004ECE38;
    case 0x2754: return g_TargetFloat004ECE3C;
    case 0x2755: return *FloatField(this, 0x2D94);
    case 0x2756: return *FloatField(this, 0x2D98);
    case 0x2757: return *FloatField(this, 0x2DA8);
    case 0x2758: return *FloatField(this, 0x2DAC);
    case 0x2759: return *FloatField(this, 0x2DB0);
    case 0x275A: return *FloatField(this, 0x2DD0);
    case 0x275B: return *FloatField(this, 0x2DD4);
    case 0x275C: return *FloatField(this, 0x2DD8);
    case 0x275D: return *FloatField(this, 0x2D9C);
    case 0x275E: return *FloatField(this, 0x2DA0);
    case 0x275F: return *FloatField(this, 0x2DC4);
    case 0x2760: return *FloatField(this, 0x2DC8);
    case 0x2761: return *FloatField(this, 0x2DCC);
    case 0x2762: return g_TargetRng0164D520.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    case 0x2763: return (f32)*IntField(this, 0x3354);
    case 0x2764: return (f32)*(u8 *)(bytes + 0x3313);
    case 0x2765: return *FloatField(this, 0x2D64);
    case 0x2766: return *FloatField(this, 0x2D68);
    case 0x2767: return *FloatField(this, 0x2D6C);
    case 0x2768: return (f32)*IntField(this, 0x3358);
    case 0x2769: return (f32)*IntField(this, 0x335C);
    case 0x276A: return (f32)*IntField(this, 0x3360);
    case 0x276B: return (f32)*IntField(this, 0x3364);
    case 0x276C: return (f32)*IntField(this, 0x3304);
    case 0x276D: return (f32)*IntField(this, 0x2E08);
    case 0x276E: return *IndirectFloatField(this, 0x68);
    case 0x276F: return *IndirectFloatField(this, 0x6C);

    case 0x2770:
        if (helperEnemy->FUN_0041F000())
            return (f32)helperEnemy->FUN_0041FD40();
        if (helperEnemy->FUN_0041FD20())
            return (f32)((TargetEnemyHelperOverlay *)*(void **)(bytes + 0x2DA4))->FUN_0041FD40();
        return 0.0f;

    case 0x2771: return (f32)g_TargetPlayer017D5EF8.IsYoukai();
    case 0x2773:
        return (f32)(g_TargetSpellcard004EA670.FUN_004178A0()
                         ? g_TargetSpellcard004EA670.FUN_0041FD90()
                         : g_TargetSpellcard004EA670.FUN_00405260());

    case 0x2772:
    default: return operand;
    }
}

// Observed: TH08 1.00d 0x00420950 receives Enemy in ECX and operand in EDX.
// A clear operand flag returns the raw pointer; enabled operands dispatch the
// writable subset selected by the target's 80-byte sparse switch table.
f32 *__fastcall ResolveFloatLValue(EnemyOverlay *enemy, f32 *operand, u16 flags, i32 flagIndex)
{
    if (flagIndex >= 0 && !(flags & (1 << flagIndex)))
        return operand;

    switch ((i32)*operand)
    {
    case 0x2720: return IndirectFloatField(enemy, 0x38);
    case 0x2721: return IndirectFloatField(enemy, 0x3C);
    case 0x2722: return IndirectFloatField(enemy, 0x40);
    case 0x2723: return IndirectFloatField(enemy, 0x44);
    case 0x2724: return IndirectFloatField(enemy, 0x48);
    case 0x2725: return IndirectFloatField(enemy, 0x4C);
    case 0x2726: return IndirectFloatField(enemy, 0x50);
    case 0x2727: return IndirectFloatField(enemy, 0x54);
    case 0x2728: return FloatField(enemy, 0x2CC8);
    case 0x2729: return FloatField(enemy, 0x2CCC);
    case 0x272A: return FloatField(enemy, 0x2CD0);
    case 0x272B: return FloatField(enemy, 0x2CD4);
    case 0x272C: return FloatField(enemy, 0x2CD8);
    case 0x272D: return FloatField(enemy, 0x2CDC);
    case 0x272E: return FloatField(enemy, 0x2CE0);
    case 0x272F: return FloatField(enemy, 0x2CE4);

    case 0x273A: return FloatField(enemy, 0x2D34);
    case 0x273B: return FloatField(enemy, 0x2D38);
    case 0x273C: return FloatField(enemy, 0x2D3C);
    case 0x273D: return &g_TargetVector017D61AC.x;
    case 0x273E: return &g_TargetVector017D61AC.y;
    case 0x273F: return &g_TargetVector017D61AC.z;

    case 0x2749: return IndirectFloatField(enemy, 0x80);
    case 0x274A: return IndirectFloatField(enemy, 0x84);
    case 0x274B: return IndirectFloatField(enemy, 0x88);
    case 0x274C: return IndirectFloatField(enemy, 0x8C);
    case 0x2751: return &g_TargetFloat004ECE30;
    case 0x2752: return &g_TargetFloat004ECE34;
    case 0x2753: return &g_TargetFloat004ECE38;
    case 0x2754: return &g_TargetFloat004ECE3C;

    case 0x2755: return FloatField(enemy, 0x2D94);
    case 0x2756: return FloatField(enemy, 0x2D98);
    case 0x2757: return FloatField(enemy, 0x2DA8);
    case 0x2758: return FloatField(enemy, 0x2DAC);
    case 0x2759: return FloatField(enemy, 0x2DB0);
    case 0x275A: return FloatField(enemy, 0x2DD0);
    case 0x275B: return FloatField(enemy, 0x2DD4);
    case 0x275C: return FloatField(enemy, 0x2DD8);
    case 0x275D: return FloatField(enemy, 0x2D9C);
    case 0x275E: return FloatField(enemy, 0x2DA0);
    case 0x275F: return FloatField(enemy, 0x2DC4);
    case 0x2760: return FloatField(enemy, 0x2DC8);
    case 0x2761: return FloatField(enemy, 0x2DCC);
    case 0x276E: return IndirectFloatField(enemy, 0x68);
    case 0x276F: return IndirectFloatField(enemy, 0x6C);

    default: return operand;
    }
}

} // namespace EclOperands
} // namespace th08
