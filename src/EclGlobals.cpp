#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"

namespace th08
{

DIFFABLE_STATIC(i32, g_EclCallbackPublishedEnemyField24);
DIFFABLE_STATIC(i32, g_EclCallbackPublishedEnemyField56);
DIFFABLE_STATIC(void *, g_EclExUpdateCallback);
DIFFABLE_STATIC(EclExBarrierRenderState, g_EclExBarrierRenderState);
DIFFABLE_STATIC(i8, g_EclScriptedGlobalUpdateFreeze);
DIFFABLE_STATIC(f32, g_EclGameTimeScale);
DIFFABLE_STATIC(u32, g_EclGameTimeScaleFlags);
DIFFABLE_STATIC(u8, g_TargetByte0164D0B1);

namespace EclRunLowProposal
{
DIFFABLE_STATIC_ARRAY(EclOperands::EnemyOverlay *, 92, g_EclEnemyTableF54CC0);
} // namespace EclRunLowProposal

} // namespace th08
