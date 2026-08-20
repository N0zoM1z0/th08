#include "th_pch.h"

#include "AsciiManager.hpp"
#include "GameManager.hpp"

namespace th08
{

// Target cluster 0x00422B80..0x00422C13. Detailed whole-image anchors place
// it immediately before the EnemyManager motion code at 0x00422C40.
// FUNCTION: th08 0x422b80
#pragma optimize("t", on)
f32 GameManager::ScaleFloatBasedOnRank(f32 upper, f32 lower)
{
    return upper + ((f32)this->rank * (lower - upper)) / 32.0f;
}
#pragma optimize("", on)

// FUNCTION: th08 0x422bb0
void AsciiManager::FUN_00422bb0(i32 slot, i16 state)
{
    this->bossMarkers[slot].SetInterrupt(state);
}

// FUNCTION: th08 0x422be0
void AsciiManager::SetBossMarkerPosition(i32 slot, D3DXVECTOR3 *position)
{
    this->bossMarkers[slot].pos = *(Float3 *)position;
}

} // namespace th08
