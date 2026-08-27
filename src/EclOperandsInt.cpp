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

// This target cluster was built with inlining disabled.  Keep direct typed
// expressions so helper calls and member access retain the retail shape.
#define ECL_CONTEXT (enemy->activeEclContext)
#define ENEMY_OWNER (enemy)



// Observed: 0x0041F420 is __fastcall, with Enemy in ECX and the raw integer
// operand in EDX. The primary table covers 0x2710 through 0x2774 inclusive.
// Values 0x275f..0x2762 deliberately share the raw-operand default handler.
i32 __fastcall ResolveInt(Enemy *enemy, i32 operand)
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
    case 0x2718: return enemy->eclIntVariables[0];
    case 0x2719: return enemy->eclIntVariables[1];
    case 0x271a: return enemy->eclIntVariables[2];
    case 0x271b: return enemy->eclIntVariables[3];
    case 0x271c: return enemy->eclIntVariables[4];
    case 0x271d: return enemy->eclIntVariables[5];
    case 0x271e: return enemy->eclIntVariables[6];
    case 0x271f: return enemy->eclIntVariables[7];

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
    case 0x2741: return enemy->bossTimer.current;
    case 0x2743: return enemy->life;
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
    case 0x2728: return (i32)enemy->eclFloatVariables[0];
    case 0x2729: return (i32)enemy->eclFloatVariables[1];
    case 0x272a: return (i32)enemy->eclFloatVariables[2];
    case 0x272b: return (i32)enemy->eclFloatVariables[3];
    case 0x272c: return (i32)enemy->eclFloatVariables[4];
    case 0x272d: return (i32)enemy->eclFloatVariables[5];
    case 0x272e: return (i32)enemy->eclFloatVariables[6];
    case 0x272f: return (i32)enemy->eclFloatVariables[7];
    case 0x2749: return (i32)ECL_CONTEXT->callParameterFloats[0];
    case 0x274a: return (i32)ECL_CONTEXT->callParameterFloats[1];
    case 0x274b: return (i32)ECL_CONTEXT->callParameterFloats[2];
    case 0x274c: return (i32)ECL_CONTEXT->callParameterFloats[3];
    case 0x274d: return EclRunLow::g_EclCallParameters.ints[0];
    case 0x274e: return EclRunLow::g_EclCallParameters.ints[1];
    case 0x274f: return EclRunLow::g_EclCallParameters.ints[2];
    case 0x2750: return EclRunLow::g_EclCallParameters.ints[3];
    case 0x2751: return (i32)EclRunLow::g_EclCallParameters.floats[0];
    case 0x2752: return (i32)EclRunLow::g_EclCallParameters.floats[1];
    case 0x2753: return (i32)EclRunLow::g_EclCallParameters.floats[2];
    case 0x2754: return (i32)EclRunLow::g_EclCallParameters.floats[3];
    case 0x273a: return (i32)enemy->worldPosition.x;
    case 0x273b: return (i32)enemy->worldPosition.y;
    case 0x273c: return (i32)enemy->worldPosition.z;
    case 0x273d: return (i32)g_Player.position.x;
    case 0x273e: return (i32)g_Player.position.y;
    case 0x273f: return (i32)g_Player.position.z;
    case 0x275a: return (i32)enemy->movementInterpolationOrigin.x;
    case 0x275b: return (i32)enemy->movementInterpolationOrigin.y;
    case 0x275c: return (i32)enemy->movementInterpolationOrigin.z;
    case 0x2765: return (i32)enemy->lastFrameDisplacement.x;
    case 0x2766: return (i32)enemy->lastFrameDisplacement.y;
    case 0x2767: return (i32)enemy->lastFrameDisplacement.z;
    case 0x2768: return enemy->lifeCallbackThresholds[0];
    case 0x2769: return enemy->lifeCallbackThresholds[1];
    case 0x276a: return enemy->lifeCallbackThresholds[2];
    case 0x276b: return enemy->lifeCallbackThresholds[3];
    case 0x2755: return (i32)enemy->movementAngle;
    case 0x2756: return (i32)enemy->angularVelocity;
    case 0x2757: return (i32)enemy->speed;
    case 0x2758: return (i32)enemy->acceleration;
    case 0x2759: return (i32)enemy->orbitRadius;
    case 0x275d: return (i32)enemy->orbitAngle;
    case 0x275e: return (i32)enemy->orbitAngularVelocity;
    case 0x2763: return enemy->lastDamage;
    case 0x2764: return enemy->bossSlot;
    case 0x276c: return enemy->itemDropType;
    case 0x276d: return enemy->score;

    case 0x2770:
        return ENEMY_OWNER->HasParentChain()
                   ? ENEMY_OWNER->CountParentChain()
                   : ENEMY_OWNER->HasAttachedEnemy()
                         ? ENEMY_OWNER->parentEnemy->CountParentChain()
                         : 0;
    case 0x2740:
        return (i32)g_Player.AngleToPoint(&enemy->worldPosition);
    case 0x2742:
    {
        Float3 delta = g_Player.position - enemy->worldPosition;
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
        return g_Spellcard.IsActive() ? g_Spellcard.IsCaptureValid()
                                                    : g_Spellcard.WasCaptured();
    case 0x2774: return g_Spellcard.GetTimerFrames();
    default: return operand;
    }
}

// Observed: 0x0041FE10 is __fastcall for ECX/EDX and callee-cleans the two
// stack arguments (u16 flags, i32 flagIndex). Its selector table covers
// 0x2710..0x276d; every unlisted value returns the caller's operand pointer.
i32 *__fastcall ResolveIntLValue(Enemy *enemy, i32 *operand, u16 flags, i32 flagIndex)
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
    case 0x2718: return &enemy->eclIntVariables[0];
    case 0x2719: return &enemy->eclIntVariables[1];
    case 0x271a: return &enemy->eclIntVariables[2];
    case 0x271b: return &enemy->eclIntVariables[3];
    case 0x271c: return &enemy->eclIntVariables[4];
    case 0x271d: return &enemy->eclIntVariables[5];
    case 0x271e: return &enemy->eclIntVariables[6];
    case 0x271f: return &enemy->eclIntVariables[7];
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
    case 0x2741: return &enemy->bossTimer.current;
    case 0x2743: return &enemy->life;
    case 0x276c: return &enemy->itemDropType;
    case 0x276d: return &enemy->score;
    case 0x274d: return &EclRunLow::g_EclCallParameters.ints[0];
    case 0x274e: return &EclRunLow::g_EclCallParameters.ints[1];
    case 0x274f: return &EclRunLow::g_EclCallParameters.ints[2];
    case 0x2750: return &EclRunLow::g_EclCallParameters.ints[3];
    default: return operand;
    }
}

#undef ENEMY_OWNER
#undef ECL_CONTEXT

} // namespace EclOperands
} // namespace th08
