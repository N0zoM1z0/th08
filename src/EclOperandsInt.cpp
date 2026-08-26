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

// These private overlays expose only behavior and offsets observed in the
// hash-attested TH08 1.00d target. Their owning subsystem lanes can replace
// the address-based names once the corresponding public layouts are proven.




// This target cluster was built with inlining disabled.  Helper functions,
// including __forceinline helpers, therefore become out-of-line COMDAT calls
// and change both owned function bodies.  The original functions access their
// private Enemy/ECL layouts directly; these expression macros retain that
// target-observed shape without introducing a public Enemy ABI.
#define INT_FIELD(offset) (*(i32 *)(enemy->bytes + (offset)))
#define FLOAT_FIELD(offset) (*(f32 *)(enemy->bytes + (offset)))
#define VECTOR_FIELD(offset) (*(Vector3 *)(enemy->bytes + (offset)))
#define ECL_CONTEXT (reinterpret_cast<Enemy *>(enemy)->activeEclContext)
#define ENEMY_HELPERS ((TargetEnemyHelpersOverlay *)enemy)



// Observed: 0x0041F420 is __fastcall, with Enemy in ECX and the raw integer
// operand in EDX. The primary table covers 0x2710 through 0x2774 inclusive.
// Values 0x275f..0x2762 deliberately share the raw-operand default handler.
i32 __fastcall ResolveInt(EnemyOverlay *enemy, i32 operand)
{
    switch (operand)
    {
    case 0x2710: return ECL_CONTEXT->intVariables[0];
    case 0x2711: return ECL_CONTEXT->intVariables[1];
    case 0x2712: return ECL_CONTEXT->intVariables[2];
    case 0x2713: return ECL_CONTEXT->intVariables[3];
    case 0x2714: return ECL_CONTEXT->intVariables[4];
    case 0x2715: return ECL_CONTEXT->intVariables[5];
    case 0x2716: return ECL_CONTEXT->intVariables[6];
    case 0x2717: return ECL_CONTEXT->intVariables[7];
    case 0x2718: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[0];
    case 0x2719: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[1];
    case 0x271a: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[2];
    case 0x271b: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[3];
    case 0x271c: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[4];
    case 0x271d: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[5];
    case 0x271e: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[6];
    case 0x271f: return reinterpret_cast<Enemy *>(enemy)->eclIntVariables[7];

    // Case-body order follows the target jump-table emission order rather
    // than numeric selector order.
    case 0x2745: return ECL_CONTEXT->callParameterInts[0];
    case 0x2746: return ECL_CONTEXT->callParameterInts[1];
    case 0x2747: return ECL_CONTEXT->callParameterInts[2];
    case 0x2748: return ECL_CONTEXT->callParameterInts[3];
    case 0x2734: return ECL_CONTEXT->extraIntVariables[0];
    case 0x2735: return ECL_CONTEXT->extraIntVariables[1];
    case 0x2736: return ECL_CONTEXT->extraIntVariables[2];
    case 0x2737: return ECL_CONTEXT->extraIntVariables[3];
    case 0x2730: return (i32)(g_Rng.GetRandomU32() & 0x7fffffff);
    case 0x2731: return (i32)g_Rng.GetRandomF32();
    case 0x2732: return (i32)g_Rng.GetRandomU32();
    case 0x2733: return (i32)g_Rng.GetRandomF32Signed();
    case 0x2738: return g_GameManager.difficulty;
    case 0x2739: return g_GameManager.rank;
    case 0x2741: return reinterpret_cast<Enemy *>(enemy)->bossTimer.current;
    case 0x2743: return reinterpret_cast<Enemy *>(enemy)->life;
    case 0x2744: return ::th08::g_GameManager.shotType;
    case 0x276e: return (i32)ECL_CONTEXT->extraFloatVariables[0];
    case 0x276f: return (i32)ECL_CONTEXT->extraFloatVariables[1];

    case 0x2720: return (i32)ECL_CONTEXT->floatVariables[0];
    case 0x2721: return (i32)ECL_CONTEXT->floatVariables[1];
    case 0x2722: return (i32)ECL_CONTEXT->floatVariables[2];
    case 0x2723: return (i32)ECL_CONTEXT->floatVariables[3];
    case 0x2724: return (i32)ECL_CONTEXT->floatVariables[4];
    case 0x2725: return (i32)ECL_CONTEXT->floatVariables[5];
    case 0x2726: return (i32)ECL_CONTEXT->floatVariables[6];
    case 0x2727: return (i32)ECL_CONTEXT->floatVariables[7];
    case 0x2728: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[0];
    case 0x2729: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[1];
    case 0x272a: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[2];
    case 0x272b: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[3];
    case 0x272c: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[4];
    case 0x272d: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[5];
    case 0x272e: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[6];
    case 0x272f: return (i32)reinterpret_cast<Enemy *>(enemy)->eclFloatVariables[7];
    case 0x2749: return (i32)ECL_CONTEXT->callParameterFloats[0];
    case 0x274a: return (i32)ECL_CONTEXT->callParameterFloats[1];
    case 0x274b: return (i32)ECL_CONTEXT->callParameterFloats[2];
    case 0x274c: return (i32)ECL_CONTEXT->callParameterFloats[3];
    case 0x274d: return EclRunLowProposal::g_EclCallParameters.ints[0];
    case 0x274e: return EclRunLowProposal::g_EclCallParameters.ints[1];
    case 0x274f: return EclRunLowProposal::g_EclCallParameters.ints[2];
    case 0x2750: return EclRunLowProposal::g_EclCallParameters.ints[3];
    case 0x2751: return (i32)EclRunLowProposal::g_EclCallParameters.floats[0];
    case 0x2752: return (i32)EclRunLowProposal::g_EclCallParameters.floats[1];
    case 0x2753: return (i32)EclRunLowProposal::g_EclCallParameters.floats[2];
    case 0x2754: return (i32)EclRunLowProposal::g_EclCallParameters.floats[3];
    case 0x273a: return (i32)reinterpret_cast<Enemy *>(enemy)->worldPosition.x;
    case 0x273b: return (i32)reinterpret_cast<Enemy *>(enemy)->worldPosition.y;
    case 0x273c: return (i32)reinterpret_cast<Enemy *>(enemy)->worldPosition.z;
    case 0x273d: return (i32)(*reinterpret_cast<Vector3 *>(&g_Player.position)).x;
    case 0x273e: return (i32)(*reinterpret_cast<Vector3 *>(&g_Player.position)).y;
    case 0x273f: return (i32)(*reinterpret_cast<Vector3 *>(&g_Player.position)).z;
    case 0x275a: return (i32)reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin.x;
    case 0x275b: return (i32)reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin.y;
    case 0x275c: return (i32)reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin.z;
    case 0x2765: return (i32)reinterpret_cast<Enemy *>(enemy)->lastFrameDisplacement.x;
    case 0x2766: return (i32)reinterpret_cast<Enemy *>(enemy)->lastFrameDisplacement.y;
    case 0x2767: return (i32)reinterpret_cast<Enemy *>(enemy)->lastFrameDisplacement.z;
    case 0x2768: return reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[0];
    case 0x2769: return reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[1];
    case 0x276a: return reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[2];
    case 0x276b: return reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[3];
    case 0x2755: return (i32)reinterpret_cast<Enemy *>(enemy)->movementAngle;
    case 0x2756: return (i32)reinterpret_cast<Enemy *>(enemy)->angularVelocity;
    case 0x2757: return (i32)reinterpret_cast<Enemy *>(enemy)->speed;
    case 0x2758: return (i32)reinterpret_cast<Enemy *>(enemy)->acceleration;
    case 0x2759: return (i32)reinterpret_cast<Enemy *>(enemy)->orbitRadius;
    case 0x275d: return (i32)reinterpret_cast<Enemy *>(enemy)->orbitAngle;
    case 0x275e: return (i32)reinterpret_cast<Enemy *>(enemy)->orbitAngularVelocity;
    case 0x2763: return reinterpret_cast<Enemy *>(enemy)->lastDamage;
    case 0x2764: return reinterpret_cast<Enemy *>(enemy)->bossSlot;
    case 0x276c: return reinterpret_cast<Enemy *>(enemy)->itemDropType;
    case 0x276d: return reinterpret_cast<Enemy *>(enemy)->score;

    case 0x2770:
        return ENEMY_HELPERS->HasParentChain()
                   ? ENEMY_HELPERS->CountParentChain()
                   : ENEMY_HELPERS->HasAttachedEnemy()
                         ? reinterpret_cast<TargetEnemyHelpersOverlay *>(
                               reinterpret_cast<Enemy *>(enemy)->parentEnemy)->CountParentChain()
                         : 0;
    case 0x2740:
        return (i32)g_Player.FUN_0044c1b0(&reinterpret_cast<Enemy *>(enemy)->worldPosition);
    case 0x2742:
    {
        Float3 delta = g_Player.position - reinterpret_cast<Enemy *>(enemy)->worldPosition;
        return (i32)D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&delta));
    }
    case 0x2771: return g_Player.IsYoukai();
    case 0x2772:
        return g_GameManager.GetTimeOrbs() + g_Spellcard.GetPendingTimeOrbs()
                       + g_ItemManager.GetTimeOrbCount()
                   >= g_GameManager.GetLastSpellTimeOrbThreshold()
               ? 2
               : 0;
    case 0x2773:
        return g_Spellcard.IsActive() ? g_Spellcard.GetActiveState()
                                                    : g_Spellcard.GetInactiveState();
    case 0x2774: return g_Spellcard.GetTimerFrames();
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
    case 0x2710: return &ECL_CONTEXT->intVariables[0];
    case 0x2711: return &ECL_CONTEXT->intVariables[1];
    case 0x2712: return &ECL_CONTEXT->intVariables[2];
    case 0x2713: return &ECL_CONTEXT->intVariables[3];
    case 0x2714: return &ECL_CONTEXT->intVariables[4];
    case 0x2715: return &ECL_CONTEXT->intVariables[5];
    case 0x2716: return &ECL_CONTEXT->intVariables[6];
    case 0x2717: return &ECL_CONTEXT->intVariables[7];
    case 0x2718: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[0];
    case 0x2719: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[1];
    case 0x271a: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[2];
    case 0x271b: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[3];
    case 0x271c: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[4];
    case 0x271d: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[5];
    case 0x271e: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[6];
    case 0x271f: return &reinterpret_cast<Enemy *>(enemy)->eclIntVariables[7];
    case 0x2745: return &ECL_CONTEXT->callParameterInts[0];
    case 0x2746: return &ECL_CONTEXT->callParameterInts[1];
    case 0x2747: return &ECL_CONTEXT->callParameterInts[2];
    case 0x2748: return &ECL_CONTEXT->callParameterInts[3];
    case 0x2734: return &ECL_CONTEXT->extraIntVariables[0];
    case 0x2735: return &ECL_CONTEXT->extraIntVariables[1];
    case 0x2736: return &ECL_CONTEXT->extraIntVariables[2];
    case 0x2737: return &ECL_CONTEXT->extraIntVariables[3];
    case 0x2738: return &g_GameManager.difficulty;
    case 0x2739: return &g_GameManager.rank;
    case 0x2741: return &reinterpret_cast<Enemy *>(enemy)->bossTimer.current;
    case 0x2743: return &reinterpret_cast<Enemy *>(enemy)->life;
    case 0x276c: return &reinterpret_cast<Enemy *>(enemy)->itemDropType;
    case 0x276d: return &reinterpret_cast<Enemy *>(enemy)->score;
    case 0x274d: return &EclRunLowProposal::g_EclCallParameters.ints[0];
    case 0x274e: return &EclRunLowProposal::g_EclCallParameters.ints[1];
    case 0x274f: return &EclRunLowProposal::g_EclCallParameters.ints[2];
    case 0x2750: return &EclRunLowProposal::g_EclCallParameters.ints[3];
    default: return operand;
    }
}

#undef ENEMY_HELPERS
#undef ECL_CONTEXT
#undef VECTOR_FIELD
#undef FLOAT_FIELD
#undef INT_FIELD

} // namespace EclOperands
} // namespace th08
