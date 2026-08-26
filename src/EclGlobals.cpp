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
void __fastcall FUN_004233d0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423400(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423530(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423a60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424130(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424170(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004241e0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004244f0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424730(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004246e0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424820(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424e00(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424c40(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424e50(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424f60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424f90(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424fc0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423db0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425020(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425040(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424910(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004250d0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004251b0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425290(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424a00(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425390(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);

DIFFABLE_STATIC_ARRAY_ASSIGN(void *, 32, g_EclExInsn) = {
    reinterpret_cast<void *>(&ConfigureNightBlindness),
    reinterpret_cast<void *>(&FUN_004233d0),
    reinterpret_cast<void *>(&FUN_00423400),
    reinterpret_cast<void *>(&FUN_00423530),
    reinterpret_cast<void *>(&FUN_00423a60),
    reinterpret_cast<void *>(&FUN_00424130),
    reinterpret_cast<void *>(&FUN_00424170),
    reinterpret_cast<void *>(&FUN_004241e0),
    reinterpret_cast<void *>(&FUN_004244f0),
    reinterpret_cast<void *>(&FUN_00424730),
    reinterpret_cast<void *>(&FUN_004246e0),
    reinterpret_cast<void *>(&FUN_00424820),
    reinterpret_cast<void *>(&EclExIns::ReisenFreezeBullets),
    reinterpret_cast<void *>(&FUN_00424e00),
    reinterpret_cast<void *>(&FUN_00424c40),
    reinterpret_cast<void *>(&FUN_00424e20),
    reinterpret_cast<void *>(&FUN_00424e50),
    reinterpret_cast<void *>(&FUN_00424f60),
    reinterpret_cast<void *>(&FUN_00424f90),
    reinterpret_cast<void *>(&FUN_00424fc0),
    reinterpret_cast<void *>(&FUN_00423db0),
    reinterpret_cast<void *>(&FUN_00423e20),
    reinterpret_cast<void *>(&EclExIns::MokouResurrection),
    reinterpret_cast<void *>(&FUN_00425020),
    reinterpret_cast<void *>(&FUN_00425040),
    reinterpret_cast<void *>(&FUN_00424910),
    reinterpret_cast<void *>(&EclExIns::SetScriptedUpdateFreeze),
    reinterpret_cast<void *>(&FUN_004250d0),
    reinterpret_cast<void *>(&FUN_004251b0),
    reinterpret_cast<void *>(&FUN_00425290),
    reinterpret_cast<void *>(&FUN_00424a00),
    reinterpret_cast<void *>(&FUN_00425390),
};

#ifndef TH08_MODERN_PORT
DIFFABLE_STATIC(void *, g_EclExUpdateCallback);
#endif
DIFFABLE_STATIC(i32, g_EclGlobal004ECCA8);
#ifndef TH08_MODERN_PORT
DIFFABLE_STATIC(EclExBarrierRenderState, g_EclExBarrierRenderState);
#endif
DIFFABLE_STATIC(u32, g_EclGameTimeScaleFlags);
DIFFABLE_STATIC(EclManager, g_EclManager);

namespace EclRunLowProposal
{
DIFFABLE_STATIC(EclCallParameterCopy, g_EclCallParameters);
} // namespace EclRunLowProposal

} // namespace th08
