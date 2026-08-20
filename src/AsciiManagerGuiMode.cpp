#include "th_pch.h"

#include "AsciiManager.hpp"

namespace th08
{

// Target body 0x004398FF..0x00439915 lies between Gui/GameManager clusters, so
// it cannot remain in the current early AsciiManager object.
#pragma optimize("s", on)
// FUNCTION: th08 0x004398FF
void AsciiManager::SetIsGuiMode(u32 value)
{
    this->isGui = value;
}
#pragma optimize("", on)

} // namespace th08
