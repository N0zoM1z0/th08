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

// Observed: this target cluster was compiled with inlining disabled.  Even
// __forceinline helpers become out-of-line COMDAT calls under the probe
// profile, whereas both target bodies access the layouts directly.
#define ECL_CONTEXT(owner) (reinterpret_cast<Enemy *>(owner)->activeEclContext)
#define ENEMY_OWNER(owner) (reinterpret_cast<Enemy *>(owner))

// Target globals not yet represented by an owner-lane type are deliberately
// named by address.  The addresses below are direct operands in 0x00420120.

// Observed: TH08 1.00d 0x00420120 is a thiscall float resolver.  It converts
// the incoming float to i32, dispatches all IDs 0x2710..0x2773, returns in
// ST(0), and returns the raw operand for out-of-range IDs and 0x2772.
f32 EnemyOverlay::ResolveFloat(f32 operand)
{
    switch ((i32)operand)
    {
    case 0x2710: return (f32)ECL_CONTEXT(this)->intVariables[0];
    case 0x2711: return (f32)ECL_CONTEXT(this)->intVariables[1];
    case 0x2712: return (f32)ECL_CONTEXT(this)->intVariables[2];
    case 0x2713: return (f32)ECL_CONTEXT(this)->intVariables[3];
    case 0x2714: return (f32)ECL_CONTEXT(this)->intVariables[4];
    case 0x2715: return (f32)ECL_CONTEXT(this)->intVariables[5];
    case 0x2716: return (f32)ECL_CONTEXT(this)->intVariables[6];
    case 0x2717: return (f32)ECL_CONTEXT(this)->intVariables[7];
    case 0x2718: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[0];
    case 0x2719: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[1];
    case 0x271A: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[2];
    case 0x271B: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[3];
    case 0x271C: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[4];
    case 0x271D: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[5];
    case 0x271E: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[6];
    case 0x271F: return (f32)reinterpret_cast<Enemy *>(this)->eclIntVariables[7];

    case 0x2745: return (f32)ECL_CONTEXT(this)->callParameterInts[0];
    case 0x2746: return (f32)ECL_CONTEXT(this)->callParameterInts[1];
    case 0x2747: return (f32)ECL_CONTEXT(this)->callParameterInts[2];
    case 0x2748: return (f32)ECL_CONTEXT(this)->callParameterInts[3];
    case 0x2734: return (f32)ECL_CONTEXT(this)->extraIntVariables[0];
    case 0x2735: return (f32)ECL_CONTEXT(this)->extraIntVariables[1];
    case 0x2736: return (f32)ECL_CONTEXT(this)->extraIntVariables[2];
    case 0x2737: return (f32)ECL_CONTEXT(this)->extraIntVariables[3];
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
    case 0x276C: return (f32)reinterpret_cast<Enemy *>(this)->itemDropType;
    case 0x276D: return (f32)reinterpret_cast<Enemy *>(this)->score;
    case 0x274D: return (f32)EclRunLow::g_EclCallParameters.ints[0];
    case 0x274E: return (f32)EclRunLow::g_EclCallParameters.ints[1];
    case 0x274F: return (f32)EclRunLow::g_EclCallParameters.ints[2];
    case 0x2750: return (f32)EclRunLow::g_EclCallParameters.ints[3];
    case 0x2751: return EclRunLow::g_EclCallParameters.floats[0];
    case 0x2752: return EclRunLow::g_EclCallParameters.floats[1];
    case 0x2753: return EclRunLow::g_EclCallParameters.floats[2];
    case 0x2754: return EclRunLow::g_EclCallParameters.floats[3];

    case 0x2720: return ECL_CONTEXT(this)->floatVariables[0];
    case 0x2721: return ECL_CONTEXT(this)->floatVariables[1];
    case 0x2722: return ECL_CONTEXT(this)->floatVariables[2];
    case 0x2723: return ECL_CONTEXT(this)->floatVariables[3];
    case 0x2724: return ECL_CONTEXT(this)->floatVariables[4];
    case 0x2725: return ECL_CONTEXT(this)->floatVariables[5];
    case 0x2726: return ECL_CONTEXT(this)->floatVariables[6];
    case 0x2727: return ECL_CONTEXT(this)->floatVariables[7];
    case 0x2728: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[0];
    case 0x2729: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[1];
    case 0x272A: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[2];
    case 0x272B: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[3];
    case 0x272C: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[4];
    case 0x272D: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[5];
    case 0x272E: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[6];
    case 0x272F: return reinterpret_cast<Enemy *>(this)->eclFloatVariables[7];
    case 0x2749: return ECL_CONTEXT(this)->callParameterFloats[0];
    case 0x274A: return ECL_CONTEXT(this)->callParameterFloats[1];
    case 0x274B: return ECL_CONTEXT(this)->callParameterFloats[2];
    case 0x274C: return ECL_CONTEXT(this)->callParameterFloats[3];
    case 0x273A: return reinterpret_cast<Enemy *>(this)->worldPosition.x;
    case 0x273B: return reinterpret_cast<Enemy *>(this)->worldPosition.y;
    case 0x273C: return reinterpret_cast<Enemy *>(this)->worldPosition.z;
    case 0x273D: return g_Player.position.x;
    case 0x273E: return g_Player.position.y;
    case 0x273F: return g_Player.position.z;
    case 0x276E: return ECL_CONTEXT(this)->extraFloatVariables[0];
    case 0x276F: return ECL_CONTEXT(this)->extraFloatVariables[1];
    case 0x275A: return reinterpret_cast<Enemy *>(this)->movementInterpolationOrigin.x;
    case 0x275B: return reinterpret_cast<Enemy *>(this)->movementInterpolationOrigin.y;
    case 0x275C: return reinterpret_cast<Enemy *>(this)->movementInterpolationOrigin.z;
    case 0x275F: return reinterpret_cast<Enemy *>(this)->movementInterpolationDelta.x;
    case 0x2760: return reinterpret_cast<Enemy *>(this)->movementInterpolationDelta.y;
    case 0x2761: return reinterpret_cast<Enemy *>(this)->movementInterpolationDelta.z;
    case 0x2765: return reinterpret_cast<Enemy *>(this)->lastFrameDisplacement.x;
    case 0x2766: return reinterpret_cast<Enemy *>(this)->lastFrameDisplacement.y;
    case 0x2767: return reinterpret_cast<Enemy *>(this)->lastFrameDisplacement.z;
    case 0x2768: return (f32)reinterpret_cast<Enemy *>(this)->lifeCallbackThresholds[0];
    case 0x2769: return (f32)reinterpret_cast<Enemy *>(this)->lifeCallbackThresholds[1];
    case 0x276A: return (f32)reinterpret_cast<Enemy *>(this)->lifeCallbackThresholds[2];
    case 0x276B: return (f32)reinterpret_cast<Enemy *>(this)->lifeCallbackThresholds[3];
    case 0x2740:
        return g_Player.AngleToPoint(&reinterpret_cast<Enemy *>(this)->worldPosition);
    case 0x2755: return reinterpret_cast<Enemy *>(this)->movementAngle;
    case 0x2756: return reinterpret_cast<Enemy *>(this)->angularVelocity;
    case 0x2757: return reinterpret_cast<Enemy *>(this)->speed;
    case 0x2758: return reinterpret_cast<Enemy *>(this)->acceleration;
    case 0x2759: return reinterpret_cast<Enemy *>(this)->orbitRadius;
    case 0x275D: return reinterpret_cast<Enemy *>(this)->orbitAngle;
    case 0x275E: return reinterpret_cast<Enemy *>(this)->orbitAngularVelocity;
    case 0x2764: return (f32)reinterpret_cast<Enemy *>(this)->bossSlot;
    case 0x2763: return (f32)reinterpret_cast<Enemy *>(this)->lastDamage;

    case 0x2770:
        return (f32)(ENEMY_OWNER(this)->HasParentChain()
                         ? ENEMY_OWNER(this)->CountParentChain()
                         : ENEMY_OWNER(this)->HasAttachedEnemy()
                               ? ENEMY_OWNER(this)->parentEnemy->CountParentChain()
                               : 0);

    case 0x2742:
    {
        Float3 delta = g_Player.position - reinterpret_cast<Enemy *>(this)->worldPosition;
        return D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&delta));
    }
    case 0x2771: return (f32)g_Player.IsYoukai();
    case 0x2773:
        return (f32)(g_Spellcard.IsActive()
                         ? g_Spellcard.IsCaptureValid()
                         : g_Spellcard.WasCaptured());

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
    case 0x2720: return &ECL_CONTEXT(enemy)->floatVariables[0];
    case 0x2721: return &ECL_CONTEXT(enemy)->floatVariables[1];
    case 0x2722: return &ECL_CONTEXT(enemy)->floatVariables[2];
    case 0x2723: return &ECL_CONTEXT(enemy)->floatVariables[3];
    case 0x2724: return &ECL_CONTEXT(enemy)->floatVariables[4];
    case 0x2725: return &ECL_CONTEXT(enemy)->floatVariables[5];
    case 0x2726: return &ECL_CONTEXT(enemy)->floatVariables[6];
    case 0x2727: return &ECL_CONTEXT(enemy)->floatVariables[7];
    case 0x2728: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[0];
    case 0x2729: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[1];
    case 0x272A: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[2];
    case 0x272B: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[3];
    case 0x272C: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[4];
    case 0x272D: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[5];
    case 0x272E: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[6];
    case 0x272F: return &reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[7];

    case 0x2749: return &ECL_CONTEXT(enemy)->callParameterFloats[0];
    case 0x274A: return &ECL_CONTEXT(enemy)->callParameterFloats[1];
    case 0x274B: return &ECL_CONTEXT(enemy)->callParameterFloats[2];
    case 0x274C: return &ECL_CONTEXT(enemy)->callParameterFloats[3];
    case 0x273A: return &reinterpret_cast<Enemy *>(enemy)->position.x;
    case 0x273B: return &reinterpret_cast<Enemy *>(enemy)->position.y;
    case 0x273C: return &reinterpret_cast<Enemy *>(enemy)->position.z;
    case 0x273D: return &g_Player.position.x;
    case 0x273E: return &g_Player.position.y;
    case 0x273F: return &g_Player.position.z;
    case 0x276E: return &ECL_CONTEXT(enemy)->extraFloatVariables[0];
    case 0x276F: return &ECL_CONTEXT(enemy)->extraFloatVariables[1];
    case 0x2751: return &EclRunLow::g_EclCallParameters.floats[0];
    case 0x2752: return &EclRunLow::g_EclCallParameters.floats[1];
    case 0x2753: return &EclRunLow::g_EclCallParameters.floats[2];
    case 0x2754: return &EclRunLow::g_EclCallParameters.floats[3];

    case 0x275A: return &reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin.x;
    case 0x275B: return &reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin.y;
    case 0x275C: return &reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin.z;
    case 0x275F: return &reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x;
    case 0x2760: return &reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.y;
    case 0x2761: return &reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.z;
    case 0x2755: return &reinterpret_cast<Enemy *>(enemy)->movementAngle;
    case 0x2756: return &reinterpret_cast<Enemy *>(enemy)->angularVelocity;
    case 0x2757: return &reinterpret_cast<Enemy *>(enemy)->speed;
    case 0x2758: return &reinterpret_cast<Enemy *>(enemy)->acceleration;
    case 0x2759: return &reinterpret_cast<Enemy *>(enemy)->orbitRadius;
    case 0x275D: return &reinterpret_cast<Enemy *>(enemy)->orbitAngle;
    case 0x275E: return &reinterpret_cast<Enemy *>(enemy)->orbitAngularVelocity;

    default: return operand;
    }
}

#undef ENEMY_OWNER
#undef ECL_CONTEXT

} // namespace EclOperands
} // namespace th08
