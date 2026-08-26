#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "EnemyManager.hpp"
#include "ZunMath.hpp"

#include <math.h>

namespace th08
{
namespace EclHelpers
{

using EclOperands::EnemyOverlay;

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

    reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x = cosf(angle) *
                                 ReadFloat(enemy, instruction, 3) *
                                 ReadInt(enemy, instruction, 0);
    reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.y = sinf(angle) *
                                 ReadFloat(enemy, instruction, 3) *
                                 ReadInt(enemy, instruction, 0);
    reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.z = 0.0f;
    *reinterpret_cast<D3DXVECTOR3 *>(
        &reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin) =
        *reinterpret_cast<D3DXVECTOR3 *>(
            &reinterpret_cast<Enemy *>(enemy)->worldPosition);
    reinterpret_cast<Enemy *>(enemy)->movementTimer =
        (reinterpret_cast<Enemy *>(enemy)->movementDuration = ReadInt(enemy, instruction, 0));

    MotionFlags(enemy).motionMode = ReadInt(enemy, instruction, 1);
    MotionFlags(enemy).interpolationMode = 2;
    if (MotionFlags(enemy).mirrorX)
        reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x =
            -reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x;
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

    *reinterpret_cast<D3DXVECTOR3 *>(
        &reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta) =
        target - *reinterpret_cast<D3DXVECTOR3 *>(
                     &reinterpret_cast<Enemy *>(enemy)->worldPosition);
    *reinterpret_cast<D3DXVECTOR3 *>(
        &reinterpret_cast<Enemy *>(enemy)->movementInterpolationOrigin) =
        *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position);
    reinterpret_cast<Enemy *>(enemy)->movementTimer =
        (reinterpret_cast<Enemy *>(enemy)->movementDuration = ReadInt(enemy, instruction, 0));

    MotionFlags(enemy).motionMode = ReadInt(enemy, instruction, 1);
    MotionFlags(enemy).interpolationMode = 2;
    *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->velocity) =
        D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (MotionFlags(enemy).mirrorX)
        reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x =
            -reinterpret_cast<Enemy *>(enemy)->movementInterpolationDelta.x;
}

#undef ReadFloat
#undef ReadInt
#undef MotionFlags
} // namespace EclHelpers
} // namespace th08
