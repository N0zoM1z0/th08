#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"

namespace th08
{

namespace EclRunLowProposal
{
struct InterpolationSlot;
void __fastcall InterpolateLinear(EclOperands::EnemyOverlay *enemy, InterpolationSlot *slot, f32 t);
void __fastcall InterpolateHermite(EclOperands::EnemyOverlay *enemy, InterpolationSlot *slot, f32 t);

DIFFABLE_STATIC_ARRAY_ASSIGN(void *, 8, g_EclInterpolatorCallbacks) = {
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateHermite),
};
} // namespace EclRunLowProposal

struct EclExInstruction;
namespace EclExIns
{
void __fastcall ReisenFreezeBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall MokouResurrection(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SetScriptedUpdateFreeze(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
}

void __fastcall ConfigureNightBlindness(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall TriggerShortScreenPulse(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall UpdateBouncingEnemyMotion(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall StartNarrowBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall WarpBulletsAcrossNarrowBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall StopBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall StartWideBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall WarpBulletsAcrossWideBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SynchronizeOrbitingChildFormation(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall UpdateNarrowRotatingLaserHitbox(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall TriggerScreenPulseAndShake(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall UpdateMediumRotatingLaserHitbox(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall ApplyRedBackgroundTint(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall AdvanceReisenBulletPhase(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall TriggerScreenShake(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall TriggerChildrenNearMarkedBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall TriggerLongScreenPulse(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SetFrameRateDivisor(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall PublishCurrentSpellCardNumber(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall StartMediumBulletWarpBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall WarpBulletsAcrossMediumBarrier(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall HideSpellCardPresentation(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall PublishCapturedSpellCardCount(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall UpdateWideRotatingLaserHitbox(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SpawnEnemiesFromMarkedBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall EnterScaledBulletTime(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall ExitScaledBulletTime(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SetScreenEffectCounter(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall SpawnBombOrExtendItem(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);

DIFFABLE_STATIC_ARRAY_ASSIGN(void *, 32, g_EclExInsn) = {
    reinterpret_cast<void *>(&ConfigureNightBlindness),
    reinterpret_cast<void *>(&TriggerShortScreenPulse),
    reinterpret_cast<void *>(&UpdateBouncingEnemyMotion),
    reinterpret_cast<void *>(&StartNarrowBulletWarpBarrier),
    reinterpret_cast<void *>(&WarpBulletsAcrossNarrowBarrier),
    reinterpret_cast<void *>(&StopBulletWarpBarrier),
    reinterpret_cast<void *>(&StartWideBulletWarpBarrier),
    reinterpret_cast<void *>(&WarpBulletsAcrossWideBarrier),
    reinterpret_cast<void *>(&SynchronizeOrbitingChildFormation),
    reinterpret_cast<void *>(&UpdateNarrowRotatingLaserHitbox),
    reinterpret_cast<void *>(&TriggerScreenPulseAndShake),
    reinterpret_cast<void *>(&UpdateMediumRotatingLaserHitbox),
    reinterpret_cast<void *>(&EclExIns::ReisenFreezeBullets),
    reinterpret_cast<void *>(&ApplyRedBackgroundTint),
    reinterpret_cast<void *>(&AdvanceReisenBulletPhase),
    reinterpret_cast<void *>(&TriggerScreenShake),
    reinterpret_cast<void *>(&TriggerChildrenNearMarkedBullets),
    reinterpret_cast<void *>(&TriggerLongScreenPulse),
    reinterpret_cast<void *>(&SetFrameRateDivisor),
    reinterpret_cast<void *>(&PublishCurrentSpellCardNumber),
    reinterpret_cast<void *>(&StartMediumBulletWarpBarrier),
    reinterpret_cast<void *>(&WarpBulletsAcrossMediumBarrier),
    reinterpret_cast<void *>(&EclExIns::MokouResurrection),
    reinterpret_cast<void *>(&HideSpellCardPresentation),
    reinterpret_cast<void *>(&PublishCapturedSpellCardCount),
    reinterpret_cast<void *>(&UpdateWideRotatingLaserHitbox),
    reinterpret_cast<void *>(&EclExIns::SetScriptedUpdateFreeze),
    reinterpret_cast<void *>(&SpawnEnemiesFromMarkedBullets),
    reinterpret_cast<void *>(&EnterScaledBulletTime),
    reinterpret_cast<void *>(&ExitScaledBulletTime),
    reinterpret_cast<void *>(&SetScreenEffectCounter),
    reinterpret_cast<void *>(&SpawnBombOrExtendItem),
};

DIFFABLE_STATIC(u32, g_EclGameTimeScaleFlags);
DIFFABLE_STATIC(EclManager, g_EclManager);

namespace EclRunLowProposal
{
DIFFABLE_STATIC(EclCallParameterCopy, g_EclCallParameters);
} // namespace EclRunLowProposal

} // namespace th08
