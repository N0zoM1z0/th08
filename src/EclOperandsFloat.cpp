#include "EclOperands.hpp"

#include "EclManager.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Global.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"

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

// Observed: this target cluster was compiled with inlining disabled.  Even
// __forceinline helpers become out-of-line COMDAT calls under the probe
// profile, whereas both target bodies access the private layouts directly.
// Keep the overlay private while retaining that direct expression shape.
#define ECL_CONTEXT(owner) (*(u8 **)((owner)->bytes + 0x2CA0))
#define CONTEXT_INT(owner, offset) (*(i32 *)(ECL_CONTEXT(owner) + (offset)))
#define CONTEXT_FLOAT(owner, offset) (*(f32 *)(ECL_CONTEXT(owner) + (offset)))
#define ENEMY_INT(owner, offset) (*(i32 *)((owner)->bytes + (offset)))
#define ENEMY_FLOAT(owner, offset) (*(f32 *)((owner)->bytes + (offset)))
#define ENEMY_VECTOR(owner, offset) (*(TargetVector3 *)((owner)->bytes + (offset)))
#define ENEMY_HELPERS(owner) ((TargetEnemyHelpersOverlay *)(owner))

// Target globals not yet represented by an owner-lane type are deliberately
// named by address.  The addresses below are direct operands in 0x00420120.

// Observed: TH08 1.00d 0x00420120 is a thiscall float resolver.  It converts
// the incoming float to i32, dispatches all IDs 0x2710..0x2773, returns in
// ST(0), and returns the raw operand for out-of-range IDs and 0x2772.
f32 EnemyOverlay::ResolveFloat(f32 operand)
{
    switch ((i32)operand)
    {
    case 0x2710: return (f32)CONTEXT_INT(this, 0x18);
    case 0x2711: return (f32)CONTEXT_INT(this, 0x1C);
    case 0x2712: return (f32)CONTEXT_INT(this, 0x20);
    case 0x2713: return (f32)CONTEXT_INT(this, 0x24);
    case 0x2714: return (f32)CONTEXT_INT(this, 0x28);
    case 0x2715: return (f32)CONTEXT_INT(this, 0x2C);
    case 0x2716: return (f32)CONTEXT_INT(this, 0x30);
    case 0x2717: return (f32)CONTEXT_INT(this, 0x34);
    case 0x2718: return (f32)ENEMY_INT(this, 0x2CA8);
    case 0x2719: return (f32)ENEMY_INT(this, 0x2CAC);
    case 0x271A: return (f32)ENEMY_INT(this, 0x2CB0);
    case 0x271B: return (f32)ENEMY_INT(this, 0x2CB4);
    case 0x271C: return (f32)ENEMY_INT(this, 0x2CB8);
    case 0x271D: return (f32)ENEMY_INT(this, 0x2CBC);
    case 0x271E: return (f32)ENEMY_INT(this, 0x2CC0);
    case 0x271F: return (f32)ENEMY_INT(this, 0x2CC4);

    case 0x2745: return (f32)CONTEXT_INT(this, 0x70);
    case 0x2746: return (f32)CONTEXT_INT(this, 0x74);
    case 0x2747: return (f32)CONTEXT_INT(this, 0x78);
    case 0x2748: return (f32)CONTEXT_INT(this, 0x7C);
    case 0x2734: return (f32)CONTEXT_INT(this, 0x58);
    case 0x2735: return (f32)CONTEXT_INT(this, 0x5C);
    case 0x2736: return (f32)CONTEXT_INT(this, 0x60);
    case 0x2737: return (f32)CONTEXT_INT(this, 0x64);
    case 0x2730: return (f32)(g_Rng.GetRandomU32() & 0x7FFFFFFF);
    case 0x2731: return g_Rng.GetRandomF32();
    case 0x2732: return (f32)(i32)g_Rng.GetRandomU32();
    case 0x2733: return g_Rng.GetRandomF32Signed();
    case 0x2762: return g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    case 0x2738: return (f32)g_GameManager.difficulty;
    case 0x2739: return (f32)g_GameManager.rank;
    case 0x2741: return (f32)reinterpret_cast<Enemy *>(this)->bossTimer.current;
    case 0x2743: return (f32)reinterpret_cast<Enemy *>(this)->life;
    case 0x2744: return (f32)::th08::g_GameManager.shotType;
    case 0x276C: return (f32)ENEMY_INT(this, 0x3304);
    case 0x276D: return (f32)reinterpret_cast<Enemy *>(this)->score;
    case 0x274D: return (f32)EclRunLowProposal::g_EclCallParameters.ints[0];
    case 0x274E: return (f32)EclRunLowProposal::g_EclCallParameters.ints[1];
    case 0x274F: return (f32)EclRunLowProposal::g_EclCallParameters.ints[2];
    case 0x2750: return (f32)EclRunLowProposal::g_EclCallParameters.ints[3];
    case 0x2751: return EclRunLowProposal::g_EclCallParameters.floats[0];
    case 0x2752: return EclRunLowProposal::g_EclCallParameters.floats[1];
    case 0x2753: return EclRunLowProposal::g_EclCallParameters.floats[2];
    case 0x2754: return EclRunLowProposal::g_EclCallParameters.floats[3];

    case 0x2720: return CONTEXT_FLOAT(this, 0x38);
    case 0x2721: return CONTEXT_FLOAT(this, 0x3C);
    case 0x2722: return CONTEXT_FLOAT(this, 0x40);
    case 0x2723: return CONTEXT_FLOAT(this, 0x44);
    case 0x2724: return CONTEXT_FLOAT(this, 0x48);
    case 0x2725: return CONTEXT_FLOAT(this, 0x4C);
    case 0x2726: return CONTEXT_FLOAT(this, 0x50);
    case 0x2727: return CONTEXT_FLOAT(this, 0x54);
    case 0x2728: return ENEMY_FLOAT(this, 0x2CC8);
    case 0x2729: return ENEMY_FLOAT(this, 0x2CCC);
    case 0x272A: return ENEMY_FLOAT(this, 0x2CD0);
    case 0x272B: return ENEMY_FLOAT(this, 0x2CD4);
    case 0x272C: return ENEMY_FLOAT(this, 0x2CD8);
    case 0x272D: return ENEMY_FLOAT(this, 0x2CDC);
    case 0x272E: return ENEMY_FLOAT(this, 0x2CE0);
    case 0x272F: return ENEMY_FLOAT(this, 0x2CE4);
    case 0x2749: return CONTEXT_FLOAT(this, 0x80);
    case 0x274A: return CONTEXT_FLOAT(this, 0x84);
    case 0x274B: return CONTEXT_FLOAT(this, 0x88);
    case 0x274C: return CONTEXT_FLOAT(this, 0x8C);
    case 0x273A: return ENEMY_FLOAT(this, 0x2D88);
    case 0x273B: return ENEMY_FLOAT(this, 0x2D8C);
    case 0x273C: return ENEMY_FLOAT(this, 0x2D90);
    case 0x273D: return (*reinterpret_cast<TargetVector3 *>(&g_Player.position)).x;
    case 0x273E: return (*reinterpret_cast<TargetVector3 *>(&g_Player.position)).y;
    case 0x273F: return (*reinterpret_cast<TargetVector3 *>(&g_Player.position)).z;
    case 0x276E: return CONTEXT_FLOAT(this, 0x68);
    case 0x276F: return CONTEXT_FLOAT(this, 0x6C);
    case 0x275A: return ENEMY_FLOAT(this, 0x2DD0);
    case 0x275B: return ENEMY_FLOAT(this, 0x2DD4);
    case 0x275C: return ENEMY_FLOAT(this, 0x2DD8);
    case 0x275F: return ENEMY_FLOAT(this, 0x2DC4);
    case 0x2760: return ENEMY_FLOAT(this, 0x2DC8);
    case 0x2761: return ENEMY_FLOAT(this, 0x2DCC);
    case 0x2765: return ENEMY_FLOAT(this, 0x2D64);
    case 0x2766: return ENEMY_FLOAT(this, 0x2D68);
    case 0x2767: return ENEMY_FLOAT(this, 0x2D6C);
    case 0x2768: return (f32)ENEMY_INT(this, 0x3358);
    case 0x2769: return (f32)ENEMY_INT(this, 0x335C);
    case 0x276A: return (f32)ENEMY_INT(this, 0x3360);
    case 0x276B: return (f32)ENEMY_INT(this, 0x3364);
    case 0x2740:
        return g_Player.FUN_0044c1b0(reinterpret_cast<Float3 *>(&ENEMY_VECTOR(this, 0x2D88)));
    case 0x2755: return ENEMY_FLOAT(this, 0x2D94);
    case 0x2756: return ENEMY_FLOAT(this, 0x2D98);
    case 0x2757: return ENEMY_FLOAT(this, 0x2DA8);
    case 0x2758: return ENEMY_FLOAT(this, 0x2DAC);
    case 0x2759: return ENEMY_FLOAT(this, 0x2DB0);
    case 0x275D: return ENEMY_FLOAT(this, 0x2D9C);
    case 0x275E: return ENEMY_FLOAT(this, 0x2DA0);
    case 0x2764: return (f32)*(u8 *)(bytes + 0x3313);
    case 0x2763: return (f32)ENEMY_INT(this, 0x3354);

    case 0x2770:
        return (f32)(ENEMY_HELPERS(this)->HasParentChain()
                         ? ENEMY_HELPERS(this)->CountParentChain()
                         : ENEMY_HELPERS(this)->HasAttachedEnemy()
                               ? ((TargetEnemyHelpersOverlay *)*(void **)(bytes + 0x2DA4))->CountParentChain()
                               : 0);

    case 0x2742:
    {
        Float3 delta = g_Player.position - *reinterpret_cast<Float3 *>(&ENEMY_VECTOR(this, 0x2D88));
        return D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&delta));
    }
    case 0x2771: return (f32)g_Player.IsYoukai();
    case 0x2773:
        return (f32)(g_Spellcard.IsActive()
                         ? g_Spellcard.GetActiveState()
                         : g_Spellcard.GetInactiveState());

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
    case 0x2720: return &CONTEXT_FLOAT(enemy, 0x38);
    case 0x2721: return &CONTEXT_FLOAT(enemy, 0x3C);
    case 0x2722: return &CONTEXT_FLOAT(enemy, 0x40);
    case 0x2723: return &CONTEXT_FLOAT(enemy, 0x44);
    case 0x2724: return &CONTEXT_FLOAT(enemy, 0x48);
    case 0x2725: return &CONTEXT_FLOAT(enemy, 0x4C);
    case 0x2726: return &CONTEXT_FLOAT(enemy, 0x50);
    case 0x2727: return &CONTEXT_FLOAT(enemy, 0x54);
    case 0x2728: return &ENEMY_FLOAT(enemy, 0x2CC8);
    case 0x2729: return &ENEMY_FLOAT(enemy, 0x2CCC);
    case 0x272A: return &ENEMY_FLOAT(enemy, 0x2CD0);
    case 0x272B: return &ENEMY_FLOAT(enemy, 0x2CD4);
    case 0x272C: return &ENEMY_FLOAT(enemy, 0x2CD8);
    case 0x272D: return &ENEMY_FLOAT(enemy, 0x2CDC);
    case 0x272E: return &ENEMY_FLOAT(enemy, 0x2CE0);
    case 0x272F: return &ENEMY_FLOAT(enemy, 0x2CE4);

    case 0x2749: return &CONTEXT_FLOAT(enemy, 0x80);
    case 0x274A: return &CONTEXT_FLOAT(enemy, 0x84);
    case 0x274B: return &CONTEXT_FLOAT(enemy, 0x88);
    case 0x274C: return &CONTEXT_FLOAT(enemy, 0x8C);
    case 0x273A: return &ENEMY_FLOAT(enemy, 0x2D34);
    case 0x273B: return &ENEMY_FLOAT(enemy, 0x2D38);
    case 0x273C: return &ENEMY_FLOAT(enemy, 0x2D3C);
    case 0x273D: return &(*reinterpret_cast<TargetVector3 *>(&g_Player.position)).x;
    case 0x273E: return &(*reinterpret_cast<TargetVector3 *>(&g_Player.position)).y;
    case 0x273F: return &(*reinterpret_cast<TargetVector3 *>(&g_Player.position)).z;
    case 0x276E: return &CONTEXT_FLOAT(enemy, 0x68);
    case 0x276F: return &CONTEXT_FLOAT(enemy, 0x6C);
    case 0x2751: return &EclRunLowProposal::g_EclCallParameters.floats[0];
    case 0x2752: return &EclRunLowProposal::g_EclCallParameters.floats[1];
    case 0x2753: return &EclRunLowProposal::g_EclCallParameters.floats[2];
    case 0x2754: return &EclRunLowProposal::g_EclCallParameters.floats[3];

    case 0x275A: return &ENEMY_FLOAT(enemy, 0x2DD0);
    case 0x275B: return &ENEMY_FLOAT(enemy, 0x2DD4);
    case 0x275C: return &ENEMY_FLOAT(enemy, 0x2DD8);
    case 0x275F: return &ENEMY_FLOAT(enemy, 0x2DC4);
    case 0x2760: return &ENEMY_FLOAT(enemy, 0x2DC8);
    case 0x2761: return &ENEMY_FLOAT(enemy, 0x2DCC);
    case 0x2755: return &ENEMY_FLOAT(enemy, 0x2D94);
    case 0x2756: return &ENEMY_FLOAT(enemy, 0x2D98);
    case 0x2757: return &ENEMY_FLOAT(enemy, 0x2DA8);
    case 0x2758: return &ENEMY_FLOAT(enemy, 0x2DAC);
    case 0x2759: return &ENEMY_FLOAT(enemy, 0x2DB0);
    case 0x275D: return &ENEMY_FLOAT(enemy, 0x2D9C);
    case 0x275E: return &ENEMY_FLOAT(enemy, 0x2DA0);

    default: return operand;
    }
}

} // namespace EclOperands
} // namespace th08
