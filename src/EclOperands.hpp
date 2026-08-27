#pragma once

#include "inttypes.hpp"

namespace th08
{
namespace EclOperands
{

struct Vector3
{
    f32 x;
    f32 y;
    f32 z;

    Vector3 operator-(const Vector3 &other) const;
    f32 Length() const;
};
extern Vector3 g_TargetPlayerPosition017D61AC;

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

namespace EclRunLowProposal
{
struct EclCallParameterCopy
{
    i32 ints[4];
    f32 floats[4];
};
extern EclCallParameterCopy g_EclCallParameters;
} // namespace EclRunLowProposal
} // namespace th08
