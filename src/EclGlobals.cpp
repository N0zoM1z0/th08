#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"

namespace th08
{

namespace EclRunLow
{
void __fastcall InterpolateLinear(
    Enemy *enemy, EnemyEclInterpolationSlot *slot, f32 t);
void __fastcall InterpolateHermite(
    Enemy *enemy, EnemyEclInterpolationSlot *slot, f32 t);

DIFFABLE_STATIC_ARRAY_ASSIGN(EnemyEclInterpolatorCallback, 8, g_EclInterpolatorCallbacks) = {
    &InterpolateLinear,
    &InterpolateLinear,
    &InterpolateLinear,
    &InterpolateLinear,
    &InterpolateLinear,
    &InterpolateLinear,
    &InterpolateLinear,
    &InterpolateHermite,
};
} // namespace EclRunLow

namespace EclExIns
{
void __fastcall ReisenFreezeBullets(Enemy *enemy, EclExInstruction *instruction);
void __fastcall MokouResurrection(Enemy *enemy, EclExInstruction *instruction);
void __fastcall SetScriptedUpdateFreeze(Enemy *enemy, EclExInstruction *instruction);
}

void __fastcall ConfigureNightBlindness(Enemy *enemy, EclExInstruction *instruction);
void __fastcall TriggerShortScreenPulse(Enemy *enemy, EclExInstruction *instruction);
void __fastcall UpdateBouncingEnemyMotion(Enemy *enemy, EclExInstruction *instruction);
void __fastcall StartNarrowBulletWarpBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall WarpBulletsAcrossNarrowBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall StopBulletWarpBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall StartWideBulletWarpBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall WarpBulletsAcrossWideBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall SynchronizeOrbitingChildFormation(Enemy *enemy, EclExInstruction *instruction);
void __fastcall UpdateNarrowRotatingLaserHitbox(Enemy *enemy, EclExInstruction *instruction);
void __fastcall TriggerScreenPulseAndShake(Enemy *enemy, EclExInstruction *instruction);
void __fastcall UpdateMediumRotatingLaserHitbox(Enemy *enemy, EclExInstruction *instruction);
void __fastcall ApplyRedBackgroundTint(Enemy *enemy, EclExInstruction *instruction);
void __fastcall AdvanceReisenBulletPhase(Enemy *enemy, EclExInstruction *instruction);
void __fastcall TriggerScreenShake(Enemy *enemy, EclExInstruction *instruction);
void __fastcall TriggerChildrenNearMarkedBullets(Enemy *enemy, EclExInstruction *instruction);
void __fastcall TriggerLongScreenPulse(Enemy *enemy, EclExInstruction *instruction);
void __fastcall SetFrameRateDivisor(Enemy *enemy, EclExInstruction *instruction);
void __fastcall PublishCurrentSpellCardNumber(Enemy *enemy, EclExInstruction *instruction);
void __fastcall StartMediumBulletWarpBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall WarpBulletsAcrossMediumBarrier(Enemy *enemy, EclExInstruction *instruction);
void __fastcall HideSpellCardPresentation(Enemy *enemy, EclExInstruction *instruction);
void __fastcall PublishCapturedSpellCardCount(Enemy *enemy, EclExInstruction *instruction);
void __fastcall UpdateWideRotatingLaserHitbox(Enemy *enemy, EclExInstruction *instruction);
void __fastcall SpawnEnemiesFromMarkedBullets(Enemy *enemy, EclExInstruction *instruction);
void __fastcall EnterScaledBulletTime(Enemy *enemy, EclExInstruction *instruction);
void __fastcall ExitScaledBulletTime(Enemy *enemy, EclExInstruction *instruction);
void __fastcall SetScreenEffectCounter(Enemy *enemy, EclExInstruction *instruction);
void __fastcall SpawnBombOrExtendItem(Enemy *enemy, EclExInstruction *instruction);

DIFFABLE_STATIC_ARRAY_ASSIGN(EclExInstructionCallback, 32, g_EclExInsn) = {
    &ConfigureNightBlindness,
    &TriggerShortScreenPulse,
    &UpdateBouncingEnemyMotion,
    &StartNarrowBulletWarpBarrier,
    &WarpBulletsAcrossNarrowBarrier,
    &StopBulletWarpBarrier,
    &StartWideBulletWarpBarrier,
    &WarpBulletsAcrossWideBarrier,
    &SynchronizeOrbitingChildFormation,
    &UpdateNarrowRotatingLaserHitbox,
    &TriggerScreenPulseAndShake,
    &UpdateMediumRotatingLaserHitbox,
    &EclExIns::ReisenFreezeBullets,
    &ApplyRedBackgroundTint,
    &AdvanceReisenBulletPhase,
    &TriggerScreenShake,
    &TriggerChildrenNearMarkedBullets,
    &TriggerLongScreenPulse,
    &SetFrameRateDivisor,
    &PublishCurrentSpellCardNumber,
    &StartMediumBulletWarpBarrier,
    &WarpBulletsAcrossMediumBarrier,
    &EclExIns::MokouResurrection,
    &HideSpellCardPresentation,
    &PublishCapturedSpellCardCount,
    &UpdateWideRotatingLaserHitbox,
    &EclExIns::SetScriptedUpdateFreeze,
    &SpawnEnemiesFromMarkedBullets,
    &EnterScaledBulletTime,
    &ExitScaledBulletTime,
    &SetScreenEffectCounter,
    &SpawnBombOrExtendItem,
};

#ifndef TH08_PORTABLE_NATIVE_LAYOUT
DIFFABLE_STATIC(u32, g_EclGameTimeScaleFlags);
#endif
DIFFABLE_STATIC(EclManager, g_EclManager);

namespace EclRunLow
{
DIFFABLE_STATIC(EclCallParameterCopy, g_EclCallParameters);
} // namespace EclRunLow

} // namespace th08
