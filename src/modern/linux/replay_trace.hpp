#pragma once

namespace th08
{
struct ReplayManager;
namespace modern
{
void ReplayTraceBeginCalculation();
bool ReplayTraceRecord(ReplayManager *manager);
void ReplayTraceFinish();
bool ReplayTraceShouldExit();
}
}
