#pragma once

#include "inttypes.hpp"

namespace th08
{
namespace EclOperands
{

// ABI adapter retained for the target resolver and detach-chain symbols.
// Implementations immediately recover the asserted public Enemy owner.
struct EnemyOverlay
{
    u8 bytes[1];

    f32 ResolveFloat(f32 operand);
    void DetachEnemyChain(i32 awardRewards);
};

i32 __fastcall ResolveInt(EnemyOverlay *enemy, i32 operand);
i32 *__fastcall ResolveIntLValue(EnemyOverlay *enemy, i32 *operand, u16 flags, i32 flagIndex);
f32 *__fastcall ResolveFloatLValue(EnemyOverlay *enemy, f32 *operand, u16 flags, i32 flagIndex);

} // namespace EclOperands

namespace EclRunLow
{
struct EclCallParameterCopy
{
    i32 ints[4];
    f32 floats[4];
};
extern EclCallParameterCopy g_EclCallParameters;
} // namespace EclRunLow
} // namespace th08
