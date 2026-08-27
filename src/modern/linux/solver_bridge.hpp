#pragma once

#include <stdint.h>

// Export the current physical input epoch for generation-safe /proc capture.
// A route is far shorter than the uint32 wrap interval at 60 Hz.
extern "C" volatile uint32_t th08_solver_input_epoch;

namespace th08
{
namespace modern
{

// Return false only when bridge mode was not requested. This path never waits.
// While a solver is connected, an exact-epoch action is consumed when ready
// and a deadline miss preserves the complete held mask. No client, disconnect,
// or bridge failure selects neutral Shot+Focus, so SDL input cannot leak into a
// hard-no-Bomb run and a stale directional command cannot remain latched.
bool SolverBridgeReadInput(uint16_t *inputMask);

// Publish one completed-update notification for the next input epoch. The
// packet send is non-blocking; a saturated client queue drops this publication.
void SolverBridgePublishSnapshot();

// Mirror the retail analysis patch at 0x0044D0FA while bridge mode is active,
// unless the diagnostic replay-save mode explicitly restores life decrement.
// The patched instruction changes AddLives(-1) to AddLives(0), so callers must
// still execute AddLives to preserve its anti-tamper bookkeeping.
bool SolverBridgePreserveLives();

} // namespace modern
} // namespace th08
