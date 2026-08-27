#pragma once

#include <stdint.h>

namespace th08
{
namespace modern
{

// Return false only when bridge mode was not requested.  In bridge mode every
// failure returns true with a neutral mask, so SDL input can never leak into a
// hard-no-bomb solver run.
bool SolverBridgeReadInput(uint16_t *inputMask);

// Mirror the retail analysis patch at 0x0044D0FA while bridge mode is active,
// unless the diagnostic replay-save mode explicitly restores life decrement.
// The patched instruction changes AddLives(-1) to AddLives(0), so callers must
// still execute AddLives to preserve its anti-tamper bookkeeping.
bool SolverBridgePreserveLives();

// Remove time spent waiting for the solver from the clock observed by the
// game.  The caller supplies the unmodified Linux wall-clock value.
uint64_t SolverBridgeVirtualMicroseconds(uint64_t realMicroseconds);

} // namespace modern
} // namespace th08
