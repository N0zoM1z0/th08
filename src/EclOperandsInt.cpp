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
    case ECL_OPERAND_LOCAL_INT_0: return ECL_CONTEXT->intVariables[0];
    case ECL_OPERAND_LOCAL_INT_1: return ECL_CONTEXT->intVariables[1];
    case ECL_OPERAND_LOCAL_INT_2: return ECL_CONTEXT->intVariables[2];
    case ECL_OPERAND_LOCAL_INT_3: return ECL_CONTEXT->intVariables[3];
    case ECL_OPERAND_LOCAL_INT_4: return ECL_CONTEXT->intVariables[4];
    case ECL_OPERAND_LOCAL_INT_5: return ECL_CONTEXT->intVariables[5];
    case ECL_OPERAND_LOCAL_INT_6: return ECL_CONTEXT->intVariables[6];
    case ECL_OPERAND_LOCAL_INT_7: return ECL_CONTEXT->intVariables[7];
    case ECL_OPERAND_ENEMY_INT_0: return enemy->eclIntVariables[0];
    case ECL_OPERAND_ENEMY_INT_1: return enemy->eclIntVariables[1];
    case ECL_OPERAND_ENEMY_INT_2: return enemy->eclIntVariables[2];
    case ECL_OPERAND_ENEMY_INT_3: return enemy->eclIntVariables[3];
    case ECL_OPERAND_ENEMY_INT_4: return enemy->eclIntVariables[4];
    case ECL_OPERAND_ENEMY_INT_5: return enemy->eclIntVariables[5];
    case ECL_OPERAND_ENEMY_INT_6: return enemy->eclIntVariables[6];
    case ECL_OPERAND_ENEMY_INT_7: return enemy->eclIntVariables[7];

    // Case-body order follows the target jump-table emission order rather
    // than numeric selector order.
    case ECL_OPERAND_CALL_INT_0: return ECL_CONTEXT->callParameterInts[0];
    case ECL_OPERAND_CALL_INT_1: return ECL_CONTEXT->callParameterInts[1];
    case ECL_OPERAND_CALL_INT_2: return ECL_CONTEXT->callParameterInts[2];
    case ECL_OPERAND_CALL_INT_3: return ECL_CONTEXT->callParameterInts[3];
    case ECL_OPERAND_EXTRA_INT_0: return ECL_CONTEXT->extraIntVariables[0];
    case ECL_OPERAND_EXTRA_INT_1: return ECL_CONTEXT->extraIntVariables[1];
    case ECL_OPERAND_EXTRA_INT_2: return ECL_CONTEXT->extraIntVariables[2];
    case ECL_OPERAND_EXTRA_INT_3: return ECL_CONTEXT->extraIntVariables[3];
    case ECL_OPERAND_RANDOM_NONNEGATIVE_INT: return (i32)(g_Rng.GetRandomU32() & 0x7fffffff);
    case ECL_OPERAND_RANDOM_UNIT_FLOAT: return (i32)g_Rng.GetRandomF32();
    case ECL_OPERAND_RANDOM_RAW_INT: return (i32)g_Rng.GetRandomU32();
    case ECL_OPERAND_RANDOM_SIGNED_UNIT_FLOAT: return (i32)g_Rng.GetRandomF32Signed();
    case ECL_OPERAND_DIFFICULTY: return g_GameManager.difficulty;
    case ECL_OPERAND_RANK: return g_GameManager.rank;
    case ECL_OPERAND_BOSS_TIMER: return enemy->bossTimer.current;
    case ECL_OPERAND_LIFE: return enemy->life;
    case ECL_OPERAND_SHOT_TYPE: return ::th08::g_GameManager.shotType;
    case ECL_OPERAND_EXTRA_FLOAT_0: return (i32)ECL_CONTEXT->extraFloatVariables[0];
    case ECL_OPERAND_EXTRA_FLOAT_1: return (i32)ECL_CONTEXT->extraFloatVariables[1];

    case ECL_OPERAND_LOCAL_FLOAT_0: return (i32)ECL_CONTEXT->floatVariables[0];
    case ECL_OPERAND_LOCAL_FLOAT_1: return (i32)ECL_CONTEXT->floatVariables[1];
    case ECL_OPERAND_LOCAL_FLOAT_2: return (i32)ECL_CONTEXT->floatVariables[2];
    case ECL_OPERAND_LOCAL_FLOAT_3: return (i32)ECL_CONTEXT->floatVariables[3];
    case ECL_OPERAND_LOCAL_FLOAT_4: return (i32)ECL_CONTEXT->floatVariables[4];
    case ECL_OPERAND_LOCAL_FLOAT_5: return (i32)ECL_CONTEXT->floatVariables[5];
    case ECL_OPERAND_LOCAL_FLOAT_6: return (i32)ECL_CONTEXT->floatVariables[6];
    case ECL_OPERAND_LOCAL_FLOAT_7: return (i32)ECL_CONTEXT->floatVariables[7];
    case ECL_OPERAND_ENEMY_FLOAT_0: return (i32)enemy->eclFloatVariables[0];
    case ECL_OPERAND_ENEMY_FLOAT_1: return (i32)enemy->eclFloatVariables[1];
    case ECL_OPERAND_ENEMY_FLOAT_2: return (i32)enemy->eclFloatVariables[2];
    case ECL_OPERAND_ENEMY_FLOAT_3: return (i32)enemy->eclFloatVariables[3];
    case ECL_OPERAND_ENEMY_FLOAT_4: return (i32)enemy->eclFloatVariables[4];
    case ECL_OPERAND_ENEMY_FLOAT_5: return (i32)enemy->eclFloatVariables[5];
    case ECL_OPERAND_ENEMY_FLOAT_6: return (i32)enemy->eclFloatVariables[6];
    case ECL_OPERAND_ENEMY_FLOAT_7: return (i32)enemy->eclFloatVariables[7];
    case ECL_OPERAND_CALL_FLOAT_0: return (i32)ECL_CONTEXT->callParameterFloats[0];
    case ECL_OPERAND_CALL_FLOAT_1: return (i32)ECL_CONTEXT->callParameterFloats[1];
    case ECL_OPERAND_CALL_FLOAT_2: return (i32)ECL_CONTEXT->callParameterFloats[2];
    case ECL_OPERAND_CALL_FLOAT_3: return (i32)ECL_CONTEXT->callParameterFloats[3];
    case ECL_OPERAND_SHARED_CALL_INT_0: return EclRunLow::g_EclCallParameters.ints[0];
    case ECL_OPERAND_SHARED_CALL_INT_1: return EclRunLow::g_EclCallParameters.ints[1];
    case ECL_OPERAND_SHARED_CALL_INT_2: return EclRunLow::g_EclCallParameters.ints[2];
    case ECL_OPERAND_SHARED_CALL_INT_3: return EclRunLow::g_EclCallParameters.ints[3];
    case ECL_OPERAND_SHARED_CALL_FLOAT_0: return (i32)EclRunLow::g_EclCallParameters.floats[0];
    case ECL_OPERAND_SHARED_CALL_FLOAT_1: return (i32)EclRunLow::g_EclCallParameters.floats[1];
    case ECL_OPERAND_SHARED_CALL_FLOAT_2: return (i32)EclRunLow::g_EclCallParameters.floats[2];
    case ECL_OPERAND_SHARED_CALL_FLOAT_3: return (i32)EclRunLow::g_EclCallParameters.floats[3];
    case ECL_OPERAND_ENEMY_POSITION_X: return (i32)enemy->worldPosition.x;
    case ECL_OPERAND_ENEMY_POSITION_Y: return (i32)enemy->worldPosition.y;
    case ECL_OPERAND_ENEMY_POSITION_Z: return (i32)enemy->worldPosition.z;
    case ECL_OPERAND_PLAYER_POSITION_X: return (i32)g_Player.position.x;
    case ECL_OPERAND_PLAYER_POSITION_Y: return (i32)g_Player.position.y;
    case ECL_OPERAND_PLAYER_POSITION_Z: return (i32)g_Player.position.z;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_X: return (i32)enemy->movementInterpolationOrigin.x;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_Y: return (i32)enemy->movementInterpolationOrigin.y;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_Z: return (i32)enemy->movementInterpolationOrigin.z;
    case ECL_OPERAND_LAST_FRAME_DISPLACEMENT_X: return (i32)enemy->lastFrameDisplacement.x;
    case ECL_OPERAND_LAST_FRAME_DISPLACEMENT_Y: return (i32)enemy->lastFrameDisplacement.y;
    case ECL_OPERAND_LAST_FRAME_DISPLACEMENT_Z: return (i32)enemy->lastFrameDisplacement.z;
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_0: return enemy->lifeCallbackThresholds[0];
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_1: return enemy->lifeCallbackThresholds[1];
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_2: return enemy->lifeCallbackThresholds[2];
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_3: return enemy->lifeCallbackThresholds[3];
    case ECL_OPERAND_MOVEMENT_ANGLE: return (i32)enemy->movementAngle;
    case ECL_OPERAND_ANGULAR_VELOCITY: return (i32)enemy->angularVelocity;
    case ECL_OPERAND_SPEED: return (i32)enemy->speed;
    case ECL_OPERAND_ACCELERATION: return (i32)enemy->acceleration;
    case ECL_OPERAND_ORBIT_RADIUS: return (i32)enemy->orbitRadius;
    case ECL_OPERAND_ORBIT_ANGLE: return (i32)enemy->orbitAngle;
    case ECL_OPERAND_ORBIT_ANGULAR_VELOCITY: return (i32)enemy->orbitAngularVelocity;
    case ECL_OPERAND_LAST_DAMAGE: return enemy->lastDamage;
    case ECL_OPERAND_BOSS_SLOT: return enemy->bossSlot;
    case ECL_OPERAND_ITEM_DROP_TYPE: return enemy->itemDropType;
    case ECL_OPERAND_SCORE: return enemy->score;

    case ECL_OPERAND_PARENT_CHAIN_DEPTH:
        return ENEMY_OWNER->HasParentChain()
                   ? ENEMY_OWNER->CountParentChain()
                   : ENEMY_OWNER->HasAttachedEnemy()
                         ? ENEMY_OWNER->parentEnemy->CountParentChain()
                         : 0;
    case ECL_OPERAND_ANGLE_TO_PLAYER:
        return (i32)g_Player.AngleToPoint(&enemy->worldPosition);
    case ECL_OPERAND_DISTANCE_TO_PLAYER:
    {
        Float3 delta = g_Player.position - enemy->worldPosition;
        return (i32)D3DXVec3Length(D3DXVECTOR3_PTR(&delta));
    }
    case ECL_OPERAND_PLAYER_IS_YOUKAI: return g_Player.IsYoukai();
    case ECL_OPERAND_TIME_ORB_THRESHOLD_STATE:
        return g_GameManager.GetTimeOrbs() + g_Spellcard.GetPendingTimeOrbs()
                       + g_ItemManager.GetTimeOrbCount()
                   >= g_GameManager.GetLastSpellTimeOrbThreshold()
               ? 2
               : 0;
    case ECL_OPERAND_SPELL_CAPTURE_STATE:
        return g_Spellcard.IsActive() ? g_Spellcard.IsCaptureValid()
                                                    : g_Spellcard.WasCaptured();
    case ECL_OPERAND_SPELL_TIMER_FRAMES: return g_Spellcard.GetTimerFrames();
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
    case ECL_OPERAND_LOCAL_INT_0: return &ECL_CONTEXT->intVariables[0];
    case ECL_OPERAND_LOCAL_INT_1: return &ECL_CONTEXT->intVariables[1];
    case ECL_OPERAND_LOCAL_INT_2: return &ECL_CONTEXT->intVariables[2];
    case ECL_OPERAND_LOCAL_INT_3: return &ECL_CONTEXT->intVariables[3];
    case ECL_OPERAND_LOCAL_INT_4: return &ECL_CONTEXT->intVariables[4];
    case ECL_OPERAND_LOCAL_INT_5: return &ECL_CONTEXT->intVariables[5];
    case ECL_OPERAND_LOCAL_INT_6: return &ECL_CONTEXT->intVariables[6];
    case ECL_OPERAND_LOCAL_INT_7: return &ECL_CONTEXT->intVariables[7];
    case ECL_OPERAND_ENEMY_INT_0: return &enemy->eclIntVariables[0];
    case ECL_OPERAND_ENEMY_INT_1: return &enemy->eclIntVariables[1];
    case ECL_OPERAND_ENEMY_INT_2: return &enemy->eclIntVariables[2];
    case ECL_OPERAND_ENEMY_INT_3: return &enemy->eclIntVariables[3];
    case ECL_OPERAND_ENEMY_INT_4: return &enemy->eclIntVariables[4];
    case ECL_OPERAND_ENEMY_INT_5: return &enemy->eclIntVariables[5];
    case ECL_OPERAND_ENEMY_INT_6: return &enemy->eclIntVariables[6];
    case ECL_OPERAND_ENEMY_INT_7: return &enemy->eclIntVariables[7];
    case ECL_OPERAND_CALL_INT_0: return &ECL_CONTEXT->callParameterInts[0];
    case ECL_OPERAND_CALL_INT_1: return &ECL_CONTEXT->callParameterInts[1];
    case ECL_OPERAND_CALL_INT_2: return &ECL_CONTEXT->callParameterInts[2];
    case ECL_OPERAND_CALL_INT_3: return &ECL_CONTEXT->callParameterInts[3];
    case ECL_OPERAND_EXTRA_INT_0: return &ECL_CONTEXT->extraIntVariables[0];
    case ECL_OPERAND_EXTRA_INT_1: return &ECL_CONTEXT->extraIntVariables[1];
    case ECL_OPERAND_EXTRA_INT_2: return &ECL_CONTEXT->extraIntVariables[2];
    case ECL_OPERAND_EXTRA_INT_3: return &ECL_CONTEXT->extraIntVariables[3];
    case ECL_OPERAND_DIFFICULTY: return &g_GameManager.difficulty;
    case ECL_OPERAND_RANK: return &g_GameManager.rank;
    case ECL_OPERAND_BOSS_TIMER: return &enemy->bossTimer.current;
    case ECL_OPERAND_LIFE: return &enemy->life;
    case ECL_OPERAND_ITEM_DROP_TYPE: return &enemy->itemDropType;
    case ECL_OPERAND_SCORE: return &enemy->score;
    case ECL_OPERAND_SHARED_CALL_INT_0: return &EclRunLow::g_EclCallParameters.ints[0];
    case ECL_OPERAND_SHARED_CALL_INT_1: return &EclRunLow::g_EclCallParameters.ints[1];
    case ECL_OPERAND_SHARED_CALL_INT_2: return &EclRunLow::g_EclCallParameters.ints[2];
    case ECL_OPERAND_SHARED_CALL_INT_3: return &EclRunLow::g_EclCallParameters.ints[3];
    default: return operand;
    }
}

#undef ENEMY_OWNER
#undef ECL_CONTEXT

} // namespace EclOperands
} // namespace th08
