#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ZunMath.hpp"

#include <math.h>

namespace th08
{
namespace EclHelpers
{

using EclOperands::EnemyOverlay;

#define FloatField(enemy, offset) ((f32 *)((enemy)->bytes + (offset)))

struct EnemyMotionFlags
{
    u32 lowBits : 12;
    u32 interpolationMode : 2;
    u32 motionMode : 3;
    u32 bit17 : 1;
    u32 mirrorX : 1;
    u32 highBits : 13;
};

#define MotionFlags(enemy) (*reinterpret_cast<EnemyMotionFlags *>((enemy)->bytes + 0x3324))

#define ReadInt(enemy, instruction, index)                                                   \
    ((instruction)->operandFlags & (1 << (index))                                           \
         ? EclOperands::ResolveInt((enemy), ((i32 *)(instruction)->operands)[index])        \
         : ((i32 *)(instruction)->operands)[index])

#define ReadFloat(enemy, instruction, index)                                                 \
    ((instruction)->operandFlags & (1 << (index))                                           \
         ? (enemy)->ResolveFloat(((f32 *)(instruction)->operands)[index])                   \
         : ((f32 *)(instruction)->operands)[index])

// Target 0x00420D10. Establishes a finite polar-motion segment from an
// angle/speed pair, snapshots its origin, sets its timer to the resolved
// duration, and installs the
// target-observed easing/mirror bits in Enemy+0x3324.
void __fastcall ConfigurePolarMotion(EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    f32 angle = AddNormalizeAngle(ReadFloat(enemy, instruction, 2), 0.0f);

    *FloatField(enemy, 0x2DC4) = cosf(angle) *
                                 ReadFloat(enemy, instruction, 3) *
                                 ReadInt(enemy, instruction, 0);
    *FloatField(enemy, 0x2DC8) = sinf(angle) *
                                 ReadFloat(enemy, instruction, 3) *
                                 ReadInt(enemy, instruction, 0);
    *FloatField(enemy, 0x2DCC) = 0.0f;
    *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2DD0) =
        *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2D88);
    *(ZunTimer *)(enemy->bytes + 0x2DDC) =
        (*(i32 *)(enemy->bytes + 0x2DE8) = ReadInt(enemy, instruction, 0));

    MotionFlags(enemy).motionMode = ReadInt(enemy, instruction, 1);
    MotionFlags(enemy).interpolationMode = 2;
    if (MotionFlags(enemy).mirrorX)
        *FloatField(enemy, 0x2DC4) = -*FloatField(enemy, 0x2DC4);
}

// Target 0x00420F40. The requested point is converted to a displacement from
// the current position while the existing velocity becomes the interpolation
// origin; the remainder mirrors ConfigurePolarMotion's timer/easing setup.
void __fastcall ConfigureRelativeMotion(EnemyOverlay *enemy, EclRawInstruction *instruction)
{
    D3DXVECTOR3 target;
    target.x = ReadFloat(enemy, instruction, 2);
    target.y = ReadFloat(enemy, instruction, 3);
    target.z = 0.0f;

    *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2DC4) =
        target - *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2D88);
    *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2DD0) =
        *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2D34);
    *(ZunTimer *)(enemy->bytes + 0x2DDC) =
        (*(i32 *)(enemy->bytes + 0x2DE8) = ReadInt(enemy, instruction, 0));

    MotionFlags(enemy).motionMode = ReadInt(enemy, instruction, 1);
    MotionFlags(enemy).interpolationMode = 2;
    *reinterpret_cast<D3DXVECTOR3 *>(enemy->bytes + 0x2D4C) =
        D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (MotionFlags(enemy).mirrorX)
        *FloatField(enemy, 0x2DC4) = -*FloatField(enemy, 0x2DC4);
}

#undef ReadFloat
#undef ReadInt
#undef MotionFlags
#undef FloatField

} // namespace EclHelpers
} // namespace th08
