#include "th_pch.h"

#include "AsciiManager.hpp"
#include "Gui.hpp"

namespace th08
{

// Target cluster 0x0042F2D0..0x0042F315. Detailed whole-image anchors place
// it between the GameManager solo-character predicates and the final
// EnemyManager helper, so it cannot remain in the early AsciiManager object.
// FUNCTION: th08 0x0042F2D0
void AsciiManager::FUN_0042f2d0(i32 index, u32 value)
{
    *(u32 *)((u8 *)this + 0x2254 + index * 4) = value;
}

// FUNCTION: th08 0x0042F2F0
void AsciiManager::SetScale(float scaleX, float scaleY)
{
    this->scaleX = scaleX;
    this->scaleY = scaleY;
}

// FUNCTION: th08 0x42f320
bool Gui::IsBossPresent()
{
    return this->bossPresent;
}

// FUNCTION: th08 0x42f340
void Gui::FUN_0042f340(i32 value)
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x24) = value;
}

} // namespace th08
