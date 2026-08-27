#pragma once

#include "inttypes.hpp"

namespace th08
{
struct Enemy;

namespace EclOperands
{

i32 __fastcall ResolveInt(Enemy *enemy, i32 operand);
i32 *__fastcall ResolveIntLValue(Enemy *enemy, i32 *operand, u16 flags, i32 flagIndex);
f32 *__fastcall ResolveFloatLValue(Enemy *enemy, f32 *operand, u16 flags, i32 flagIndex);

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
