#pragma once

#include "inttypes.hpp"

namespace th08
{
namespace EclOperands
{

// Private byte overlay for target-observed Enemy offsets. The public Enemy
// layout remains intentionally incomplete until its owning lane proves it.
struct EnemyOverlay
{
    u8 bytes[1];

    f32 ResolveFloat(f32 operand);
};

struct TargetEnemyHelpersOverlay
{
    i32 HasParentChain();
    i32 HasAttachedEnemy();
    i32 CountParentChain();
};

i32 __fastcall ResolveInt(EnemyOverlay *enemy, i32 operand);
i32 *__fastcall ResolveIntLValue(EnemyOverlay *enemy, i32 *operand, u16 flags, i32 flagIndex);
f32 *__fastcall ResolveFloatLValue(EnemyOverlay *enemy, f32 *operand, u16 flags, i32 flagIndex);

} // namespace EclOperands
} // namespace th08
