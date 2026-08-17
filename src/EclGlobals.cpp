#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"

namespace th08
{

DIFFABLE_STATIC(i32, g_EclCallbackPublishedEnemyField24);
DIFFABLE_STATIC(i8, g_EclScriptedGlobalUpdateFreeze);
DIFFABLE_STATIC(f32, g_EclGameTimeScale);

namespace EclRunLowProposal
{
DIFFABLE_STATIC_ARRAY(EclOperands::EnemyOverlay *, 92, g_EclEnemyTableF54CC0);
} // namespace EclRunLowProposal

} // namespace th08
