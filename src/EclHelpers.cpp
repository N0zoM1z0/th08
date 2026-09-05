#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "EnemyManager.hpp"
#include "ZunMath.hpp"

#include <math.h>

namespace th08
{
namespace EclHelpers
{

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
void __fastcall ConfigurePolarMotion(Enemy *enemy, EclRawInstruction *instruction)
{
    f32 angle = AddNormalizeAngle(ReadFloat(enemy, instruction, 2), 0.0f);

    enemy->movementInterpolationDelta.x = cosf(angle) *
                                 ReadFloat(enemy, instruction, 3) *
                                 ReadInt(enemy, instruction, 0);
    enemy->movementInterpolationDelta.y = sinf(angle) *
                                 ReadFloat(enemy, instruction, 3) *
                                 ReadInt(enemy, instruction, 0);
    enemy->movementInterpolationDelta.z = 0.0f;
    *D3DXVECTOR3_PTR(
        &enemy->movementInterpolationOrigin) =
        *D3DXVECTOR3_PTR(
            &enemy->worldPosition);
    enemy->movementTimer =
        (enemy->movementDuration = ReadInt(enemy, instruction, 0));

    reinterpret_cast<EnemyFlag1Bits *>(
        &enemy->flags1)->movementEasing =
        ReadInt(enemy, instruction, 1);
    reinterpret_cast<EnemyFlag1Bits *>(
        &enemy->flags1)->movementMode = 2;
    if (reinterpret_cast<EnemyFlag1Bits *>(
            &enemy->flags1)->mirrorMovementX)
        enemy->movementInterpolationDelta.x =
            -enemy->movementInterpolationDelta.x;
}

// Target 0x00420F40. The requested point is converted to a displacement from
// the current position while the existing velocity becomes the interpolation
// origin; the remainder mirrors ConfigurePolarMotion's timer/easing setup.
void __fastcall ConfigureRelativeMotion(Enemy *enemy, EclRawInstruction *instruction)
{
    D3DXVECTOR3 target;
    target.x = ReadFloat(enemy, instruction, 2);
    target.y = ReadFloat(enemy, instruction, 3);
    target.z = 0.0f;

    *D3DXVECTOR3_PTR(
        &enemy->movementInterpolationDelta) =
        target - *D3DXVECTOR3_PTR(
                     &enemy->worldPosition);
    *D3DXVECTOR3_PTR(
        &enemy->movementInterpolationOrigin) =
        *D3DXVECTOR3_PTR(&enemy->position);
    enemy->movementTimer =
        (enemy->movementDuration = ReadInt(enemy, instruction, 0));

    reinterpret_cast<EnemyFlag1Bits *>(
        &enemy->flags1)->movementEasing =
        ReadInt(enemy, instruction, 1);
    reinterpret_cast<EnemyFlag1Bits *>(
        &enemy->flags1)->movementMode = 2;
    *D3DXVECTOR3_PTR(&enemy->velocity) =
        D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (reinterpret_cast<EnemyFlag1Bits *>(
            &enemy->flags1)->mirrorMovementX)
        enemy->movementInterpolationDelta.x =
            -enemy->movementInterpolationDelta.x;
}

#undef ReadFloat
#undef ReadInt
} // namespace EclHelpers
} // namespace th08
