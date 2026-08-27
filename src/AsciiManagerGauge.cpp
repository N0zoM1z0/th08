#include "th_pch.h"

#include "AsciiManager.hpp"

namespace th08
{

// Target cluster 0x00406FD0..0x00407114 lies after the GameManager setters and
// before Background code. It cannot remain in the current early AsciiManager
// object if that object's 0x00402000 layout is to match.
// FUNCTION: th08 0x406fd0
void AsciiManager::UpdateVms()
{
    g_AnmManager->ExecuteScript(&this->youkaiGauge);
    g_AnmManager->ExecuteScript(&this->youkaiGaugeHumanIcon);
    g_AnmManager->ExecuteScript(&this->youkaiGaugeYoukaiIcon);
    g_AnmManager->ExecuteScript(&this->youkaiGaugeCursor);
    g_AnmManager->ExecuteScript(&this->percentageText);
    g_AnmManager->ExecuteScript(&this->bossMarkers[0]);
    g_AnmManager->ExecuteScript(&this->bossMarkers[1]);
    g_AnmManager->ExecuteScript(&this->bossMarkers[2]);
    g_AnmManager->ExecuteScript(&this->bossMarkers[3]);
    g_AnmManager->ExecuteScript(&this->unk_1520);
}

// FUNCTION: th08 0x4070b0
void AsciiManager::SetGaugeInterrupt(i32 interrupt)
{
    this->youkaiGauge.SetInterrupt(interrupt);
    this->youkaiGaugeHumanIcon.SetInterrupt(interrupt);
    this->youkaiGaugeYoukaiIcon.SetInterrupt(interrupt);
    this->youkaiGaugeCursor.SetInterrupt(interrupt);

    this->gaugeInterrupt = interrupt;
}

// FUNCTION: th08 0x407140
i32 AsciiManager::GetGaugeInterrupt()
{
    return this->gaugeInterrupt;
}

// FUNCTION: th08 0x407160
void AsciiManager::ResetStrings()
{
    this->numStrings = 0;
}

// FUNCTION: th08 0x407180
void AsciiManager::SetSpaceWidth(i32 spaceWidth)
{
    this->spaceWidth = spaceWidth;
}

} // namespace th08
