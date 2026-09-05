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

// Observed: this target cluster was compiled with inlining disabled.  Even
// __forceinline helpers become out-of-line COMDAT calls under the probe
// profile, whereas both target bodies access the layouts directly.
#define ECL_CONTEXT(owner) ((owner)->activeEclContext)
#define ENEMY_OWNER(owner) (owner)

// Observed: TH08 1.00d 0x00420120 is a thiscall float resolver.  It converts
// the incoming float to i32, dispatches all IDs 0x2710..0x2773, returns in
// ST(0), and returns the raw operand for out-of-range IDs and 0x2772.
f32 Enemy::ResolveFloat(f32 operand)
{
    switch ((i32)operand)
    {
    case ECL_OPERAND_LOCAL_INT_0: return (f32)ECL_CONTEXT(this)->intVariables[0];
    case ECL_OPERAND_LOCAL_INT_1: return (f32)ECL_CONTEXT(this)->intVariables[1];
    case ECL_OPERAND_LOCAL_INT_2: return (f32)ECL_CONTEXT(this)->intVariables[2];
    case ECL_OPERAND_LOCAL_INT_3: return (f32)ECL_CONTEXT(this)->intVariables[3];
    case ECL_OPERAND_LOCAL_INT_4: return (f32)ECL_CONTEXT(this)->intVariables[4];
    case ECL_OPERAND_LOCAL_INT_5: return (f32)ECL_CONTEXT(this)->intVariables[5];
    case ECL_OPERAND_LOCAL_INT_6: return (f32)ECL_CONTEXT(this)->intVariables[6];
    case ECL_OPERAND_LOCAL_INT_7: return (f32)ECL_CONTEXT(this)->intVariables[7];
    case ECL_OPERAND_ENEMY_INT_0: return (f32)this->eclIntVariables[0];
    case ECL_OPERAND_ENEMY_INT_1: return (f32)this->eclIntVariables[1];
    case ECL_OPERAND_ENEMY_INT_2: return (f32)this->eclIntVariables[2];
    case ECL_OPERAND_ENEMY_INT_3: return (f32)this->eclIntVariables[3];
    case ECL_OPERAND_ENEMY_INT_4: return (f32)this->eclIntVariables[4];
    case ECL_OPERAND_ENEMY_INT_5: return (f32)this->eclIntVariables[5];
    case ECL_OPERAND_ENEMY_INT_6: return (f32)this->eclIntVariables[6];
    case ECL_OPERAND_ENEMY_INT_7: return (f32)this->eclIntVariables[7];

    case ECL_OPERAND_CALL_INT_0: return (f32)ECL_CONTEXT(this)->callParameterInts[0];
    case ECL_OPERAND_CALL_INT_1: return (f32)ECL_CONTEXT(this)->callParameterInts[1];
    case ECL_OPERAND_CALL_INT_2: return (f32)ECL_CONTEXT(this)->callParameterInts[2];
    case ECL_OPERAND_CALL_INT_3: return (f32)ECL_CONTEXT(this)->callParameterInts[3];
    case ECL_OPERAND_EXTRA_INT_0: return (f32)ECL_CONTEXT(this)->extraIntVariables[0];
    case ECL_OPERAND_EXTRA_INT_1: return (f32)ECL_CONTEXT(this)->extraIntVariables[1];
    case ECL_OPERAND_EXTRA_INT_2: return (f32)ECL_CONTEXT(this)->extraIntVariables[2];
    case ECL_OPERAND_EXTRA_INT_3: return (f32)ECL_CONTEXT(this)->extraIntVariables[3];
    case ECL_OPERAND_RANDOM_NONNEGATIVE_INT: return (f32)(g_Rng.GetRandomU32() & 0x7FFFFFFF);
    case ECL_OPERAND_RANDOM_UNIT_FLOAT: return g_Rng.GetRandomF32();
    case ECL_OPERAND_RANDOM_RAW_INT: return (f32)(i32)g_Rng.GetRandomU32();
    case ECL_OPERAND_RANDOM_SIGNED_UNIT_FLOAT: return g_Rng.GetRandomF32Signed();
    case ECL_OPERAND_RANDOM_ANGLE: return g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    case ECL_OPERAND_DIFFICULTY: return (f32)g_GameManager.difficulty;
    case ECL_OPERAND_RANK: return (f32)g_GameManager.rank;
    case ECL_OPERAND_BOSS_TIMER: return (f32)this->bossTimer.current;
    case ECL_OPERAND_LIFE: return (f32)this->life;
    case ECL_OPERAND_SHOT_TYPE: return (f32)::th08::g_GameManager.shotType;
    case ECL_OPERAND_ITEM_DROP_TYPE: return (f32)this->itemDropType;
    case ECL_OPERAND_SCORE: return (f32)this->score;
    case ECL_OPERAND_SHARED_CALL_INT_0: return (f32)EclRunLow::g_EclCallParameters.ints[0];
    case ECL_OPERAND_SHARED_CALL_INT_1: return (f32)EclRunLow::g_EclCallParameters.ints[1];
    case ECL_OPERAND_SHARED_CALL_INT_2: return (f32)EclRunLow::g_EclCallParameters.ints[2];
    case ECL_OPERAND_SHARED_CALL_INT_3: return (f32)EclRunLow::g_EclCallParameters.ints[3];
    case ECL_OPERAND_SHARED_CALL_FLOAT_0: return EclRunLow::g_EclCallParameters.floats[0];
    case ECL_OPERAND_SHARED_CALL_FLOAT_1: return EclRunLow::g_EclCallParameters.floats[1];
    case ECL_OPERAND_SHARED_CALL_FLOAT_2: return EclRunLow::g_EclCallParameters.floats[2];
    case ECL_OPERAND_SHARED_CALL_FLOAT_3: return EclRunLow::g_EclCallParameters.floats[3];

    case ECL_OPERAND_LOCAL_FLOAT_0: return ECL_CONTEXT(this)->floatVariables[0];
    case ECL_OPERAND_LOCAL_FLOAT_1: return ECL_CONTEXT(this)->floatVariables[1];
    case ECL_OPERAND_LOCAL_FLOAT_2: return ECL_CONTEXT(this)->floatVariables[2];
    case ECL_OPERAND_LOCAL_FLOAT_3: return ECL_CONTEXT(this)->floatVariables[3];
    case ECL_OPERAND_LOCAL_FLOAT_4: return ECL_CONTEXT(this)->floatVariables[4];
    case ECL_OPERAND_LOCAL_FLOAT_5: return ECL_CONTEXT(this)->floatVariables[5];
    case ECL_OPERAND_LOCAL_FLOAT_6: return ECL_CONTEXT(this)->floatVariables[6];
    case ECL_OPERAND_LOCAL_FLOAT_7: return ECL_CONTEXT(this)->floatVariables[7];
    case ECL_OPERAND_ENEMY_FLOAT_0: return this->eclFloatVariables[0];
    case ECL_OPERAND_ENEMY_FLOAT_1: return this->eclFloatVariables[1];
    case ECL_OPERAND_ENEMY_FLOAT_2: return this->eclFloatVariables[2];
    case ECL_OPERAND_ENEMY_FLOAT_3: return this->eclFloatVariables[3];
    case ECL_OPERAND_ENEMY_FLOAT_4: return this->eclFloatVariables[4];
    case ECL_OPERAND_ENEMY_FLOAT_5: return this->eclFloatVariables[5];
    case ECL_OPERAND_ENEMY_FLOAT_6: return this->eclFloatVariables[6];
    case ECL_OPERAND_ENEMY_FLOAT_7: return this->eclFloatVariables[7];
    case ECL_OPERAND_CALL_FLOAT_0: return ECL_CONTEXT(this)->callParameterFloats[0];
    case ECL_OPERAND_CALL_FLOAT_1: return ECL_CONTEXT(this)->callParameterFloats[1];
    case ECL_OPERAND_CALL_FLOAT_2: return ECL_CONTEXT(this)->callParameterFloats[2];
    case ECL_OPERAND_CALL_FLOAT_3: return ECL_CONTEXT(this)->callParameterFloats[3];
    case ECL_OPERAND_ENEMY_POSITION_X: return this->worldPosition.x;
    case ECL_OPERAND_ENEMY_POSITION_Y: return this->worldPosition.y;
    case ECL_OPERAND_ENEMY_POSITION_Z: return this->worldPosition.z;
    case ECL_OPERAND_PLAYER_POSITION_X: return g_Player.position.x;
    case ECL_OPERAND_PLAYER_POSITION_Y: return g_Player.position.y;
    case ECL_OPERAND_PLAYER_POSITION_Z: return g_Player.position.z;
    case ECL_OPERAND_EXTRA_FLOAT_0: return ECL_CONTEXT(this)->extraFloatVariables[0];
    case ECL_OPERAND_EXTRA_FLOAT_1: return ECL_CONTEXT(this)->extraFloatVariables[1];
    case ECL_OPERAND_INTERPOLATION_ORIGIN_X: return this->movementInterpolationOrigin.x;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_Y: return this->movementInterpolationOrigin.y;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_Z: return this->movementInterpolationOrigin.z;
    case ECL_OPERAND_INTERPOLATION_DELTA_X: return this->movementInterpolationDelta.x;
    case ECL_OPERAND_INTERPOLATION_DELTA_Y: return this->movementInterpolationDelta.y;
    case ECL_OPERAND_INTERPOLATION_DELTA_Z: return this->movementInterpolationDelta.z;
    case ECL_OPERAND_LAST_FRAME_DISPLACEMENT_X: return this->lastFrameDisplacement.x;
    case ECL_OPERAND_LAST_FRAME_DISPLACEMENT_Y: return this->lastFrameDisplacement.y;
    case ECL_OPERAND_LAST_FRAME_DISPLACEMENT_Z: return this->lastFrameDisplacement.z;
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_0: return (f32)this->lifeCallbackThresholds[0];
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_1: return (f32)this->lifeCallbackThresholds[1];
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_2: return (f32)this->lifeCallbackThresholds[2];
    case ECL_OPERAND_LIFE_CALLBACK_THRESHOLD_3: return (f32)this->lifeCallbackThresholds[3];
    case ECL_OPERAND_ANGLE_TO_PLAYER:
        return g_Player.AngleToPoint(&this->worldPosition);
    case ECL_OPERAND_MOVEMENT_ANGLE: return this->movementAngle;
    case ECL_OPERAND_ANGULAR_VELOCITY: return this->angularVelocity;
    case ECL_OPERAND_SPEED: return this->speed;
    case ECL_OPERAND_ACCELERATION: return this->acceleration;
    case ECL_OPERAND_ORBIT_RADIUS: return this->orbitRadius;
    case ECL_OPERAND_ORBIT_ANGLE: return this->orbitAngle;
    case ECL_OPERAND_ORBIT_ANGULAR_VELOCITY: return this->orbitAngularVelocity;
    case ECL_OPERAND_BOSS_SLOT: return (f32)this->bossSlot;
    case ECL_OPERAND_LAST_DAMAGE: return (f32)this->lastDamage;

    case ECL_OPERAND_PARENT_CHAIN_DEPTH:
        return (f32)(ENEMY_OWNER(this)->HasParentChain()
                         ? ENEMY_OWNER(this)->CountParentChain()
                         : ENEMY_OWNER(this)->HasAttachedEnemy()
                               ? ENEMY_OWNER(this)->parentEnemy->CountParentChain()
                               : 0);

    case ECL_OPERAND_DISTANCE_TO_PLAYER:
    {
        Float3 delta = g_Player.position - this->worldPosition;
        return D3DXVec3Length(D3DXVECTOR3_PTR(&delta));
    }
    case ECL_OPERAND_PLAYER_IS_YOUKAI: return (f32)g_Player.IsYoukai();
    case ECL_OPERAND_SPELL_CAPTURE_STATE:
        return (f32)(g_Spellcard.IsActive()
                         ? g_Spellcard.IsCaptureValid()
                         : g_Spellcard.WasCaptured());

    case ECL_OPERAND_TIME_ORB_THRESHOLD_STATE:
    default: return operand;
    }
}

namespace EclOperands
{

// Observed: TH08 1.00d 0x00420950 receives Enemy in ECX and operand in EDX.
// A clear operand flag returns the raw pointer; enabled operands dispatch the
// writable subset selected by the target's 80-byte sparse switch table.
f32 *__fastcall ResolveFloatLValue(Enemy *enemy, f32 *operand, u16 flags, i32 flagIndex)
{
    if (flagIndex >= 0 && !(flags & (1 << flagIndex)))
        return operand;

    switch ((i32)*operand)
    {
    case ECL_OPERAND_LOCAL_FLOAT_0: return &ECL_CONTEXT(enemy)->floatVariables[0];
    case ECL_OPERAND_LOCAL_FLOAT_1: return &ECL_CONTEXT(enemy)->floatVariables[1];
    case ECL_OPERAND_LOCAL_FLOAT_2: return &ECL_CONTEXT(enemy)->floatVariables[2];
    case ECL_OPERAND_LOCAL_FLOAT_3: return &ECL_CONTEXT(enemy)->floatVariables[3];
    case ECL_OPERAND_LOCAL_FLOAT_4: return &ECL_CONTEXT(enemy)->floatVariables[4];
    case ECL_OPERAND_LOCAL_FLOAT_5: return &ECL_CONTEXT(enemy)->floatVariables[5];
    case ECL_OPERAND_LOCAL_FLOAT_6: return &ECL_CONTEXT(enemy)->floatVariables[6];
    case ECL_OPERAND_LOCAL_FLOAT_7: return &ECL_CONTEXT(enemy)->floatVariables[7];
    case ECL_OPERAND_ENEMY_FLOAT_0: return &enemy->eclFloatVariables[0];
    case ECL_OPERAND_ENEMY_FLOAT_1: return &enemy->eclFloatVariables[1];
    case ECL_OPERAND_ENEMY_FLOAT_2: return &enemy->eclFloatVariables[2];
    case ECL_OPERAND_ENEMY_FLOAT_3: return &enemy->eclFloatVariables[3];
    case ECL_OPERAND_ENEMY_FLOAT_4: return &enemy->eclFloatVariables[4];
    case ECL_OPERAND_ENEMY_FLOAT_5: return &enemy->eclFloatVariables[5];
    case ECL_OPERAND_ENEMY_FLOAT_6: return &enemy->eclFloatVariables[6];
    case ECL_OPERAND_ENEMY_FLOAT_7: return &enemy->eclFloatVariables[7];

    case ECL_OPERAND_CALL_FLOAT_0: return &ECL_CONTEXT(enemy)->callParameterFloats[0];
    case ECL_OPERAND_CALL_FLOAT_1: return &ECL_CONTEXT(enemy)->callParameterFloats[1];
    case ECL_OPERAND_CALL_FLOAT_2: return &ECL_CONTEXT(enemy)->callParameterFloats[2];
    case ECL_OPERAND_CALL_FLOAT_3: return &ECL_CONTEXT(enemy)->callParameterFloats[3];
    case ECL_OPERAND_ENEMY_POSITION_X: return &enemy->position.x;
    case ECL_OPERAND_ENEMY_POSITION_Y: return &enemy->position.y;
    case ECL_OPERAND_ENEMY_POSITION_Z: return &enemy->position.z;
    case ECL_OPERAND_PLAYER_POSITION_X: return &g_Player.position.x;
    case ECL_OPERAND_PLAYER_POSITION_Y: return &g_Player.position.y;
    case ECL_OPERAND_PLAYER_POSITION_Z: return &g_Player.position.z;
    case ECL_OPERAND_EXTRA_FLOAT_0: return &ECL_CONTEXT(enemy)->extraFloatVariables[0];
    case ECL_OPERAND_EXTRA_FLOAT_1: return &ECL_CONTEXT(enemy)->extraFloatVariables[1];
    case ECL_OPERAND_SHARED_CALL_FLOAT_0: return &EclRunLow::g_EclCallParameters.floats[0];
    case ECL_OPERAND_SHARED_CALL_FLOAT_1: return &EclRunLow::g_EclCallParameters.floats[1];
    case ECL_OPERAND_SHARED_CALL_FLOAT_2: return &EclRunLow::g_EclCallParameters.floats[2];
    case ECL_OPERAND_SHARED_CALL_FLOAT_3: return &EclRunLow::g_EclCallParameters.floats[3];

    case ECL_OPERAND_INTERPOLATION_ORIGIN_X: return &enemy->movementInterpolationOrigin.x;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_Y: return &enemy->movementInterpolationOrigin.y;
    case ECL_OPERAND_INTERPOLATION_ORIGIN_Z: return &enemy->movementInterpolationOrigin.z;
    case ECL_OPERAND_INTERPOLATION_DELTA_X: return &enemy->movementInterpolationDelta.x;
    case ECL_OPERAND_INTERPOLATION_DELTA_Y: return &enemy->movementInterpolationDelta.y;
    case ECL_OPERAND_INTERPOLATION_DELTA_Z: return &enemy->movementInterpolationDelta.z;
    case ECL_OPERAND_MOVEMENT_ANGLE: return &enemy->movementAngle;
    case ECL_OPERAND_ANGULAR_VELOCITY: return &enemy->angularVelocity;
    case ECL_OPERAND_SPEED: return &enemy->speed;
    case ECL_OPERAND_ACCELERATION: return &enemy->acceleration;
    case ECL_OPERAND_ORBIT_RADIUS: return &enemy->orbitRadius;
    case ECL_OPERAND_ORBIT_ANGLE: return &enemy->orbitAngle;
    case ECL_OPERAND_ORBIT_ANGULAR_VELOCITY: return &enemy->orbitAngularVelocity;

    default: return operand;
    }
}

#undef ENEMY_OWNER
#undef ECL_CONTEXT

} // namespace EclOperands
} // namespace th08
