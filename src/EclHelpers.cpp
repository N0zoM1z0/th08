#include "EclManager.hpp"
#include "EclOperands.hpp"

#include <math.h>

namespace th08
{
namespace EclHelpers
{

using EclOperands::EnemyOverlay;

static __forceinline f32 *FloatField(EnemyOverlay *enemy, i32 offset)
{
    return (f32 *)(enemy->bytes + offset);
}

static __forceinline u32 *Flags(EnemyOverlay *enemy)
{
    return (u32 *)(enemy->bytes + 0x3324);
}

static __forceinline i32 ReadInt(EnemyOverlay *enemy, EclRawInstruction *instruction, i32 index)
{
    i32 *operands = (i32 *)instruction->operands;
    return instruction->operandFlags & (1 << index) ? EclOperands::ResolveInt(enemy, operands[index])
                                                    : operands[index];
}

static __forceinline f32 ReadFloat(EnemyOverlay *enemy, EclRawInstruction *instruction, i32 index)
{
    f32 *operands = (f32 *)instruction->operands;
    return instruction->operandFlags & (1 << index) ? enemy->ResolveFloat(operands[index]) : operands[index];
}

// Target 0x00420D10. Establishes a finite polar-motion segment from an
// angle/speed pair, snapshots its origin, resets its timer, and installs the
// target-observed easing/mirror bits in Enemy+0x3324.
void __fastcall ConfigurePolarMotion(EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    i32 duration = ReadInt(enemy, instruction, 0);
    i32 mode = ReadInt(enemy, instruction, 1);
    f32 angle = ReadFloat(enemy, instruction, 2);
    f32 speed = ReadFloat(enemy, instruction, 3);

    *FloatField(enemy, 0x2DC4) = cosf(angle) * speed * duration;
    *FloatField(enemy, 0x2DC8) = sinf(angle) * speed * duration;
    *FloatField(enemy, 0x2DCC) = 0.0f;
    *FloatField(enemy, 0x2DD0) = *FloatField(enemy, 0x2D88);
    *FloatField(enemy, 0x2DD4) = *FloatField(enemy, 0x2D8C);
    *FloatField(enemy, 0x2DD8) = *FloatField(enemy, 0x2D90);
    *(i32 *)(enemy->bytes + 0x2DE8) = duration;
    *(ZunTimer *)(enemy->bytes + 0x2DDC) = 0;

    *Flags(enemy) = (*Flags(enemy) & 0xFFFE3FFF) | ((mode & 7) << 14);
    *Flags(enemy) = (*Flags(enemy) & 0xFFFFCFFF) | 0x2000;
    if (*Flags(enemy) & (1 << 18))
        *FloatField(enemy, 0x2DC4) = -*FloatField(enemy, 0x2DC4);
}

// Target 0x00420F40. The requested point is converted to a displacement from
// the current position while the existing velocity becomes the interpolation
// origin; the remainder mirrors ConfigurePolarMotion's timer/easing setup.
void __fastcall ConfigureRelativeMotion(EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    i32 duration = ReadInt(enemy, instruction, 0);
    i32 mode = ReadInt(enemy, instruction, 1);
    f32 targetX = ReadFloat(enemy, instruction, 2);
    f32 targetY = ReadFloat(enemy, instruction, 3);

    *FloatField(enemy, 0x2DC4) = targetX - *FloatField(enemy, 0x2D88);
    *FloatField(enemy, 0x2DC8) = targetY - *FloatField(enemy, 0x2D8C);
    *FloatField(enemy, 0x2DCC) = 0.0f - *FloatField(enemy, 0x2D90);
    *FloatField(enemy, 0x2DD0) = *FloatField(enemy, 0x2D34);
    *FloatField(enemy, 0x2DD4) = *FloatField(enemy, 0x2D38);
    *FloatField(enemy, 0x2DD8) = *FloatField(enemy, 0x2D3C);
    *(i32 *)(enemy->bytes + 0x2DE8) = duration;
    *(ZunTimer *)(enemy->bytes + 0x2DDC) = 0;

    *Flags(enemy) = (*Flags(enemy) & 0xFFFE3FFF) | ((mode & 7) << 14);
    *Flags(enemy) = (*Flags(enemy) & 0xFFFFCFFF) | 0x2000;
    *FloatField(enemy, 0x2D4C) = 0.0f;
    *FloatField(enemy, 0x2D50) = 0.0f;
    *FloatField(enemy, 0x2D54) = 0.0f;
    if (*Flags(enemy) & (1 << 18))
        *FloatField(enemy, 0x2DC4) = -*FloatField(enemy, 0x2DC4);
}

} // namespace EclHelpers
} // namespace th08
