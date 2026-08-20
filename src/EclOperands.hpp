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

// Private byte overlay for target-observed Enemy offsets. The public Enemy
// layout remains intentionally incomplete until its owning lane proves it.
struct EnemyOverlay
{
    u8 bytes[1];

    f32 ResolveFloat(f32 operand);
    void FUN_0042adb0(i32 value);
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

namespace EclRunLowProposal
{
struct EclCallParameterCopy
{
    i32 ints[4];
    f32 floats[4];
};
extern EclCallParameterCopy g_EclCallParameters;
extern EclOperands::EnemyOverlay *g_EclEnemyTableF54CC0[];
} // namespace EclRunLowProposal
} // namespace th08
