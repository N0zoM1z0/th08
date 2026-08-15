#include "EclOperands.hpp"

namespace th08
{
namespace EclOperands
{

// These private overlays expose only behavior and offsets observed in the
// hash-attested TH08 1.00d target. Their owning subsystem lanes can replace
// the address-based names once the corresponding public layouts are proven.
struct Vector3
{
    f32 x;
    f32 y;
    f32 z;

    Vector3 operator-(const Vector3 &other) const;
    f32 Length() const;
};

struct TargetRngOverlay
{
    u32 RandomU32();
    f32 RandomF32();
    f32 RandomF32Signed();
};

struct TargetPlayerOverlay
{
    f32 AngleToPlayer(const Vector3 *position);
    i32 IsYoukai();
};

struct TargetEnemyHelpersOverlay
{
    i32 HasParentChain();
    i32 HasAttachedEnemy();
    i32 CountParentChain();
};

struct TargetGameManagerOverlay
{
    i32 GetTimeOrbs();
    i32 GetLastSpellTimeOrbThreshold();
};

struct TargetItemManagerOverlay
{
    i32 GetTimeOrbCount();
};

struct TargetSpellcardOverlay
{
    i32 GetDemoFlag();
    i32 IsActive();
    i32 GetActiveState();
    i32 GetInactiveState();
    i32 GetTimerFrames();
};

extern TargetRngOverlay g_TargetRng0164D520;
extern TargetPlayerOverlay g_TargetPlayer017D5EF8;
extern Vector3 g_TargetPlayerPosition017D61AC;
extern TargetGameManagerOverlay g_TargetGameManager0160F508;
extern TargetItemManagerOverlay g_TargetItemManager01653648;
extern TargetSpellcardOverlay g_TargetSpellcard004EA670;

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

// This target cluster was built with inlining disabled.  Helper functions,
// including __forceinline helpers, therefore become out-of-line COMDAT calls
// and change both owned function bodies.  The original functions access their
// private Enemy/ECL layouts directly; these expression macros retain that
// target-observed shape without introducing a public Enemy ABI.
#define INT_FIELD(offset) (*(i32 *)(enemy->bytes + (offset)))
#define FLOAT_FIELD(offset) (*(f32 *)(enemy->bytes + (offset)))
#define VECTOR_FIELD(offset) (*(Vector3 *)(enemy->bytes + (offset)))
#define ECL_CONTEXT (*(u8 **)(enemy->bytes + 0x2ca0))
#define CONTEXT_INT(offset) (*(i32 *)(ECL_CONTEXT + (offset)))
#define CONTEXT_FLOAT(offset) (*(f32 *)(ECL_CONTEXT + (offset)))
#define ENEMY_HELPERS ((TargetEnemyHelpersOverlay *)enemy)



// FUNCTION: th08 0x0041F000
i32 TargetEnemyHelpersOverlay::HasParentChain()
{
    return *(void **)((u8 *)this + 0x2DA4) == 0 &&
           *(void **)((u8 *)this + 8) != 0;
}

// FUNCTION: th08 0x0041FD20
i32 TargetEnemyHelpersOverlay::HasAttachedEnemy()
{
    return *(void **)((u8 *)this + 0x2DA4) != 0;
}

// FUNCTION: th08 0x0041FD40
i32 TargetEnemyHelpersOverlay::CountParentChain()
{
    TargetEnemyHelpersOverlay *cursor = this;
    i32 count = 0;
    if (this->HasParentChain())
    {
        while (*(void **)((u8 *)cursor + 8) != 0)
        {
            cursor = *(TargetEnemyHelpersOverlay **)((u8 *)cursor + 8);
            count++;
        }
    }
    return count;
}

// Observed: 0x0041F420 is __fastcall, with Enemy in ECX and the raw integer
// operand in EDX. The primary table covers 0x2710 through 0x2774 inclusive.
// Values 0x275f..0x2762 deliberately share the raw-operand default handler.
i32 __fastcall ResolveInt(EnemyOverlay *enemy, i32 operand)
{
    switch (operand)
    {
    case 0x2710: return CONTEXT_INT(0x18);
    case 0x2711: return CONTEXT_INT(0x1c);
    case 0x2712: return CONTEXT_INT(0x20);
    case 0x2713: return CONTEXT_INT(0x24);
    case 0x2714: return CONTEXT_INT(0x28);
    case 0x2715: return CONTEXT_INT(0x2c);
    case 0x2716: return CONTEXT_INT(0x30);
    case 0x2717: return CONTEXT_INT(0x34);
    case 0x2718: return INT_FIELD(0x2ca8);
    case 0x2719: return INT_FIELD(0x2cac);
    case 0x271a: return INT_FIELD(0x2cb0);
    case 0x271b: return INT_FIELD(0x2cb4);
    case 0x271c: return INT_FIELD(0x2cb8);
    case 0x271d: return INT_FIELD(0x2cbc);
    case 0x271e: return INT_FIELD(0x2cc0);
    case 0x271f: return INT_FIELD(0x2cc4);

    // Case-body order follows the target jump-table emission order rather
    // than numeric selector order.
    case 0x2745: return CONTEXT_INT(0x70);
    case 0x2746: return CONTEXT_INT(0x74);
    case 0x2747: return CONTEXT_INT(0x78);
    case 0x2748: return CONTEXT_INT(0x7c);
    case 0x2734: return CONTEXT_INT(0x58);
    case 0x2735: return CONTEXT_INT(0x5c);
    case 0x2736: return CONTEXT_INT(0x60);
    case 0x2737: return CONTEXT_INT(0x64);
    case 0x2730: return (i32)(g_TargetRng0164D520.RandomU32() & 0x7fffffff);
    case 0x2731: return (i32)g_TargetRng0164D520.RandomF32();
    case 0x2732: return (i32)g_TargetRng0164D520.RandomU32();
    case 0x2733: return (i32)g_TargetRng0164D520.RandomF32Signed();
    case 0x2738: return g_TargetInt0160F538;
    case 0x2739: return g_TargetInt0164D334;
    case 0x2741: return INT_FIELD(0x2e1c);
    case 0x2743: return INT_FIELD(0x2dfc);
    case 0x2744: return g_TargetByte0164D0B1;
    case 0x276e: return (i32)CONTEXT_FLOAT(0x68);
    case 0x276f: return (i32)CONTEXT_FLOAT(0x6c);

    case 0x2720: return (i32)CONTEXT_FLOAT(0x38);
    case 0x2721: return (i32)CONTEXT_FLOAT(0x3c);
    case 0x2722: return (i32)CONTEXT_FLOAT(0x40);
    case 0x2723: return (i32)CONTEXT_FLOAT(0x44);
    case 0x2724: return (i32)CONTEXT_FLOAT(0x48);
    case 0x2725: return (i32)CONTEXT_FLOAT(0x4c);
    case 0x2726: return (i32)CONTEXT_FLOAT(0x50);
    case 0x2727: return (i32)CONTEXT_FLOAT(0x54);
    case 0x2728: return (i32)FLOAT_FIELD(0x2cc8);
    case 0x2729: return (i32)FLOAT_FIELD(0x2ccc);
    case 0x272a: return (i32)FLOAT_FIELD(0x2cd0);
    case 0x272b: return (i32)FLOAT_FIELD(0x2cd4);
    case 0x272c: return (i32)FLOAT_FIELD(0x2cd8);
    case 0x272d: return (i32)FLOAT_FIELD(0x2cdc);
    case 0x272e: return (i32)FLOAT_FIELD(0x2ce0);
    case 0x272f: return (i32)FLOAT_FIELD(0x2ce4);
    case 0x2749: return (i32)CONTEXT_FLOAT(0x80);
    case 0x274a: return (i32)CONTEXT_FLOAT(0x84);
    case 0x274b: return (i32)CONTEXT_FLOAT(0x88);
    case 0x274c: return (i32)CONTEXT_FLOAT(0x8c);
    case 0x274d: return g_TargetInt004ECE20;
    case 0x274e: return g_TargetInt004ECE24;
    case 0x274f: return g_TargetInt004ECE28;
    case 0x2750: return g_TargetInt004ECE2C;
    case 0x2751: return (i32)g_TargetFloat004ECE30;
    case 0x2752: return (i32)g_TargetFloat004ECE34;
    case 0x2753: return (i32)g_TargetFloat004ECE38;
    case 0x2754: return (i32)g_TargetFloat004ECE3C;
    case 0x273a: return (i32)FLOAT_FIELD(0x2d88);
    case 0x273b: return (i32)FLOAT_FIELD(0x2d8c);
    case 0x273c: return (i32)FLOAT_FIELD(0x2d90);
    case 0x273d: return (i32)g_TargetPlayerPosition017D61AC.x;
    case 0x273e: return (i32)g_TargetPlayerPosition017D61AC.y;
    case 0x273f: return (i32)g_TargetPlayerPosition017D61AC.z;
    case 0x275a: return (i32)FLOAT_FIELD(0x2dd0);
    case 0x275b: return (i32)FLOAT_FIELD(0x2dd4);
    case 0x275c: return (i32)FLOAT_FIELD(0x2dd8);
    case 0x2765: return (i32)FLOAT_FIELD(0x2d64);
    case 0x2766: return (i32)FLOAT_FIELD(0x2d68);
    case 0x2767: return (i32)FLOAT_FIELD(0x2d6c);
    case 0x2768: return INT_FIELD(0x3358);
    case 0x2769: return INT_FIELD(0x335c);
    case 0x276a: return INT_FIELD(0x3360);
    case 0x276b: return INT_FIELD(0x3364);
    case 0x2755: return (i32)FLOAT_FIELD(0x2d94);
    case 0x2756: return (i32)FLOAT_FIELD(0x2d98);
    case 0x2757: return (i32)FLOAT_FIELD(0x2da8);
    case 0x2758: return (i32)FLOAT_FIELD(0x2dac);
    case 0x2759: return (i32)FLOAT_FIELD(0x2db0);
    case 0x275d: return (i32)FLOAT_FIELD(0x2d9c);
    case 0x275e: return (i32)FLOAT_FIELD(0x2da0);
    case 0x2763: return INT_FIELD(0x3354);
    case 0x2764: return *(u8 *)(enemy->bytes + 0x3313);
    case 0x276c: return INT_FIELD(0x3304);
    case 0x276d: return INT_FIELD(0x2e08);

    case 0x2770:
        return ENEMY_HELPERS->HasParentChain()
                   ? ENEMY_HELPERS->CountParentChain()
                   : ENEMY_HELPERS->HasAttachedEnemy()
                         ? (*(TargetEnemyHelpersOverlay **)(enemy->bytes + 0x2da4))->CountParentChain()
                         : 0;
    case 0x2740:
        return (i32)g_TargetPlayer017D5EF8.AngleToPlayer(&VECTOR_FIELD(0x2d88));
    case 0x2742:
    {
        Vector3 delta = g_TargetPlayerPosition017D61AC - VECTOR_FIELD(0x2d88);
        return (i32)delta.Length();
    }
    case 0x2771: return g_TargetPlayer017D5EF8.IsYoukai();
    case 0x2772:
        return g_TargetGameManager0160F508.GetTimeOrbs() + g_TargetSpellcard004EA670.GetDemoFlag()
                       + g_TargetItemManager01653648.GetTimeOrbCount()
                   >= g_TargetGameManager0160F508.GetLastSpellTimeOrbThreshold()
               ? 2
               : 0;
    case 0x2773:
        return g_TargetSpellcard004EA670.IsActive() ? g_TargetSpellcard004EA670.GetActiveState()
                                                    : g_TargetSpellcard004EA670.GetInactiveState();
    case 0x2774: return g_TargetSpellcard004EA670.GetTimerFrames();
    default: return operand;
    }
}

// Observed: 0x0041FE10 is __fastcall for ECX/EDX and callee-cleans the two
// stack arguments (u16 flags, i32 flagIndex). Its selector table covers
// 0x2710..0x276d; every unlisted value returns the caller's operand pointer.
i32 *__fastcall ResolveIntLValue(EnemyOverlay *enemy, i32 *operand, u16 flags, i32 flagIndex)
{
    if (flagIndex >= 0 && !(flags & (1 << flagIndex)))
    {
        return operand;
    }

    switch (*operand)
    {
    case 0x2710: return &CONTEXT_INT(0x18);
    case 0x2711: return &CONTEXT_INT(0x1c);
    case 0x2712: return &CONTEXT_INT(0x20);
    case 0x2713: return &CONTEXT_INT(0x24);
    case 0x2714: return &CONTEXT_INT(0x28);
    case 0x2715: return &CONTEXT_INT(0x2c);
    case 0x2716: return &CONTEXT_INT(0x30);
    case 0x2717: return &CONTEXT_INT(0x34);
    case 0x2718: return &INT_FIELD(0x2ca8);
    case 0x2719: return &INT_FIELD(0x2cac);
    case 0x271a: return &INT_FIELD(0x2cb0);
    case 0x271b: return &INT_FIELD(0x2cb4);
    case 0x271c: return &INT_FIELD(0x2cb8);
    case 0x271d: return &INT_FIELD(0x2cbc);
    case 0x271e: return &INT_FIELD(0x2cc0);
    case 0x271f: return &INT_FIELD(0x2cc4);
    case 0x2745: return &CONTEXT_INT(0x70);
    case 0x2746: return &CONTEXT_INT(0x74);
    case 0x2747: return &CONTEXT_INT(0x78);
    case 0x2748: return &CONTEXT_INT(0x7c);
    case 0x2734: return &CONTEXT_INT(0x58);
    case 0x2735: return &CONTEXT_INT(0x5c);
    case 0x2736: return &CONTEXT_INT(0x60);
    case 0x2737: return &CONTEXT_INT(0x64);
    case 0x2738: return &g_TargetInt0160F538;
    case 0x2739: return &g_TargetInt0164D334;
    case 0x2741: return &INT_FIELD(0x2e1c);
    case 0x2743: return &INT_FIELD(0x2dfc);
    case 0x276c: return &INT_FIELD(0x3304);
    case 0x276d: return &INT_FIELD(0x2e08);
    case 0x274d: return &g_TargetInt004ECE20;
    case 0x274e: return &g_TargetInt004ECE24;
    case 0x274f: return &g_TargetInt004ECE28;
    case 0x2750: return &g_TargetInt004ECE2C;
    default: return operand;
    }
}

#undef ENEMY_HELPERS
#undef CONTEXT_FLOAT
#undef CONTEXT_INT
#undef ECL_CONTEXT
#undef VECTOR_FIELD
#undef FLOAT_FIELD
#undef INT_FIELD

} // namespace EclOperands
} // namespace th08
