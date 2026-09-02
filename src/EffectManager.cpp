#include "th_pch.h"

#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "AnmManager.hpp"
#include "Background.hpp"
#include "ItemManager.hpp"
#include "ReplayManager.hpp"
#include "GameManager.hpp"
#include "EnemyManager.hpp"
#include "Player.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();

void __fastcall AdjustStageEffectDrawPosition(AnmVm *effect, D3DXVECTOR3 *base);
i32 __fastcall HasAnimationEnded(Effect *effect);
i32 __fastcall DrawRadialTrail(Effect *effect);


























DIFFABLE_STATIC(EffectManager, g_EffectManager);
DIFFABLE_STATIC(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EffectManagerDrawChain);

// Target 0x004E4B64 is owned by Gui.cpp but participates in effect-resource setup.
extern i32 g_GuiMessageStageMode;





















struct EffectTemplate
{
    i32 scriptIdx;
    EffectUpdateCallback updateCallback;
    EffectInitializeCallback initializeCallback;
};
C_ASSERT(sizeof(EffectTemplate) == 0xc);
C_ASSERT(offsetof(EffectTemplate, updateCallback) == 0x4);
C_ASSERT(offsetof(EffectTemplate, initializeCallback) == 0x8);
DIFFABLE_STATIC_ARRAY(EffectTemplate, 66, g_EffectTemplates);

// FUNCTION: th08 0x423d70
Float3 *Float3::operator*=(f32 scalar)
{
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    return this;
}

// FUNCTION: th08 0x4253e0
Effect *EffectManager::GetFixedSlotEffect(i32 index)
{
    return &this->effects[index + 0x280];
}

// FUNCTION: th08 0x425410
void EffectManager::ResetEffects()
{
    memset(this, 0, 0x8B05C);
}

// FUNCTION: th08 0x425430
#pragma var_order(effect, i)
Effect *EffectManager::SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color)
{
    Effect *effect = this->effects + this->nextEffectIndex;
    i32 i;

    for (i = 0; i < 0x200; i++)
    {
        this->nextEffectIndex = this->nextEffectIndex + 1;
        if (this->nextEffectIndex >= 0x200)
        {
            this->nextEffectIndex = 0;
        }

        if (effect->active != 0)
        {
            if (this->nextEffectIndex == 0)
            {
                effect = this->effects;
            }
            else
            {
                effect++;
            }
            continue;
        }

        if (effect->vertices != NULL)
        {
            g_ZunMemory.Free(effect->vertices);
        }

        memset(effect, 0, sizeof(Effect));
        effect->active = 1;
        effect->effectId = id;
        effect->position = *reinterpret_cast<Float3 *>(position);
        this->effectAnm->SetAndExecuteScriptIdx(&effect->vm, g_EffectTemplates[id].scriptIdx);
        *reinterpret_cast<u32 *>(&effect->vm.flags) |= 0x2000;
        effect->vm.color1.d3dColor = color;
        effect->vm.pos2.x = 0.0f;
        effect->vm.pos2.y = 0.0f;
        effect->vm.pos2.z = 0.0f;
        effect->updateCallback = g_EffectTemplates[id].updateCallback;

        if (g_EffectTemplates[id].initializeCallback != NULL)
        {
            if (g_EffectTemplates[id].initializeCallback(effect) != 0)
            {
                effect->active = 0;
            }
        }

        count--;
        if (count == 0)
        {
            break;
        }

        if (this->nextEffectIndex == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }

    g_ReplayManager->frameEventFlags |= 0x400;
    return i >= 0x200 ? &this->effects[653] : effect;
}

// FUNCTION: th08 0x425650
#pragma var_order(effect, i)
Effect *EffectManager::SpawnEffectWithVelocity(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 count, i32 color)
{
    Effect *effect = this->effects + this->nextEffectIndex;
    i32 i;

    for (i = 0; i < 0x200; i++)
    {
        this->nextEffectIndex = this->nextEffectIndex + 1;
        if (this->nextEffectIndex >= 0x200)
        {
            this->nextEffectIndex = 0;
        }

        if (effect->active != 0)
        {
            if (this->nextEffectIndex == 0)
            {
                effect = this->effects;
            }
            else
            {
                effect++;
            }
            continue;
        }

        if (effect->vertices != NULL)
        {
            g_ZunMemory.Free(effect->vertices);
        }

        memset(effect, 0, sizeof(Effect));
        effect->active = 1;
        effect->effectId = id;
        effect->position = *reinterpret_cast<Float3 *>(position);
        this->effectAnm->SetAndExecuteScriptIdx(&effect->vm, g_EffectTemplates[id].scriptIdx);
        effect->vm.color1.d3dColor = color;
        effect->vm.pos2.x = 0.0f;
        effect->vm.pos2.y = 0.0f;
        effect->vm.pos2.z = 0.0f;
        effect->updateCallback = g_EffectTemplates[id].updateCallback;
        effect->vector1 = *reinterpret_cast<Float3 *>(velocity);

        if (g_EffectTemplates[id].initializeCallback != NULL)
        {
            if (g_EffectTemplates[id].initializeCallback(effect) != 0)
            {
                effect->active = 0;
            }
        }

        count--;
        if (count == 0)
        {
            break;
        }

        if (this->nextEffectIndex == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }

    g_ReplayManager->frameEventFlags |= 0x400;
    return i >= 0x200 ? &this->effects[653] : effect;
}

// FUNCTION: th08 0x425870
#pragma var_order(effect)
Effect *EffectManager::SpawnEffectInFixedSlot(i32 id, D3DXVECTOR3 *position, i32 slotIndex, i32 unused, i32 color)
{
    Effect *effect = &this->effects[slotIndex + 0x280];

    if (effect->vertices != NULL)
    {
        g_ZunMemory.Free(effect->vertices);
    }

    memset(effect, 0, sizeof(Effect));
    effect->slotIndex = slotIndex;
    effect->active = 1;
    effect->effectId = id;
    effect->position = *reinterpret_cast<Float3 *>(position);

    if (g_EffectTemplates[id].scriptIdx >= 0)
    {
        this->effectAnm->SetAndExecuteScriptIdx(&effect->vm, g_EffectTemplates[id].scriptIdx);
    }

    *reinterpret_cast<u32 *>(&effect->vm.flags) |= 0x2000;
    effect->vm.color1.d3dColor = color;
    effect->vm.pos2.x = 0.0f;
    effect->vm.pos2.y = 0.0f;
    effect->vm.pos2.z = 0.0f;
    effect->updateCallback = g_EffectTemplates[id].updateCallback;

    if (g_EffectTemplates[id].initializeCallback != NULL &&
        g_EffectTemplates[id].initializeCallback(effect) != 0)
    {
        effect->active = 0;
    }

    g_ReplayManager->frameEventFlags |= 0x400;
    return effect;
}

// FUNCTION: th08 0x4259e0
#pragma var_order(effect)
Effect *EffectManager::SpawnEffectInFixedSlotWithVelocity(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 slotIndex,
                                   i32 unused, i32 color)
{
    Effect *effect = &this->effects[slotIndex + 0x280];

    if (effect->vertices != NULL)
        g_ZunMemory.Free(effect->vertices);

    memset(effect, 0, sizeof(Effect));
    effect->slotIndex = slotIndex;
    effect->vector1 = *reinterpret_cast<Float3 *>(velocity);
    effect->active = 1;
    effect->effectId = id;
    effect->position = *reinterpret_cast<Float3 *>(position);

    if (g_EffectTemplates[id].scriptIdx >= 0)
    {
        this->effectAnm->SetAndExecuteScriptIdx(&effect->vm, g_EffectTemplates[id].scriptIdx);
    }

    *reinterpret_cast<u32 *>(&effect->vm.flags) |= 0x2000;
    effect->vm.color1.d3dColor = color;
    effect->vm.pos2.x = 0.0f;
    effect->vm.pos2.y = 0.0f;
    effect->vm.pos2.z = 0.0f;
    effect->updateCallback = g_EffectTemplates[id].updateCallback;

    if (g_EffectTemplates[id].initializeCallback != NULL &&
        g_EffectTemplates[id].initializeCallback(effect) != 0)
    {
        effect->active = 0;
    }

    g_ReplayManager->frameEventFlags |= 0x400;
    return effect;
}

// FUNCTION: th08 0x425b70
#pragma var_order(effect, i, zeroVector)
Effect *EffectManager::SpawnEffectInSecondaryPool(i32 id, D3DXVECTOR3 *position, i32 count, i32 color)
{
    Effect *effect = this->effects + 0x200;
    i32 i;

    for (i = 0; i < 0x80; i++, effect++)
    {
        if (effect->active != 0)
        {
            continue;
        }

        if (effect->vertices != NULL)
        {
            g_ZunMemory.Free(effect->vertices);
        }
        effect->vertices = NULL;
        effect->drawCallback = NULL;
        effect->drawGroup = 0;
        effect->active = 1;
        effect->effectId = id;
        effect->position = *reinterpret_cast<Float3 *>(position);
        this->effectAnm->SetAndExecuteScriptIdx(&effect->vm, g_EffectTemplates[id].scriptIdx);
        effect->vm.color1.d3dColor = color;
        effect->vm.pos2.x = 0.0f;
        effect->vm.pos2.y = 0.0f;
        effect->vm.pos2.z = 0.0f;
        effect->updateCallback = g_EffectTemplates[id].updateCallback;
        effect->timer = 0;
        effect->releaseRequested = 0;
        effect->releaseTimer = 0;
        *reinterpret_cast<D3DXVECTOR3 *>(&effect->vector1) = D3DXVECTOR3(0, 0, 0);

        if (g_EffectTemplates[id].initializeCallback != NULL)
        {
            if (g_EffectTemplates[id].initializeCallback(effect) != 0)
            {
                effect->active = 0;
            }
        }

        count--;
        if (count == 0)
        {
            break;
        }
    }

    g_ReplayManager->frameEventFlags |= 0x400;
    return i >= 0x80 ? &this->effects[653] : effect;
}

// FUNCTION: th08 0x425d70
i32 __fastcall EffectRandomSplashInit(Effect *effect)
{
    effect->vector2.operator float *()[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
    effect->vector2.operator float *()[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
    effect->vector2.operator float *()[2] = 0.0f;
    effect->vector3 = -effect->vector2 / 19.0f;
    effect->vector2 *= g_Supervisor.framerateMultiplier;
    effect->vector3 *= g_Supervisor.framerateMultiplier;
    return 0;
}

// FUNCTION: th08 0x425e60
i32 __fastcall EffectRandomSplashUpdate(Effect *effect)
{
    effect->position += effect->vector2;
    effect->vector2 += effect->vector3;
    return 1;
}

// FUNCTION: th08 0x425ea0
i32 __fastcall EffectRandomSplashBigInit(Effect *effect)
{
    effect->vector2.operator float *()[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
    effect->vector2.operator float *()[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
    effect->vector2.operator float *()[2] = 0.0f;
    effect->vector3 = -effect->vector2 / 20.0f;
    effect->vector2 *= g_Supervisor.framerateMultiplier;
    effect->vector3 *= g_Supervisor.framerateMultiplier;
    return 0;
}

// FUNCTION: th08 0x425fa0
Float3 Float3::operator-() const
{
    return Float3(-this->x, -this->y, -this->z);
}

// FUNCTION: th08 0x425fe0
i32 __fastcall EffectOrbitInit(Effect *effect)
{
    effect->drawGroup = 2;
    effect->vector6.x = 0.0f;
    effect->vector6.y = 0.0f;
    effect->vector6.z = 0.0f;
    effect->radius = 0.0f;
    return 0;
}

// FUNCTION: th08 0x426030
#pragma var_order(posOffset, verticalAngle, localMatrix, horizontalAngle, normalizedPos, alpha, this)
i32 __fastcall EffectOrbitUpdate(Effect *effect)
{
    Float3 posOffset;
    f32 verticalAngle;
    Float3 normalizedPos;
    D3DXMATRIX localMatrix;
    f32 horizontalAngle;
    f32 alpha;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&normalizedPos),
                      reinterpret_cast<D3DXVECTOR3 *>(&effect->vector6));
    verticalAngle = sinf(effect->angle);
    horizontalAngle = cosf(effect->angle);
    effect->orientationAxis.x = normalizedPos.x * verticalAngle;
    effect->orientationAxis.y = normalizedPos.y * verticalAngle;
    effect->orientationAxis.z = normalizedPos.z * verticalAngle;
    effect->orientationW = horizontalAngle;
    D3DXMatrixRotationQuaternion(
        &localMatrix, reinterpret_cast<D3DXQUATERNION *>(&effect->orientationAxis));
    posOffset.x = normalizedPos.y * 1.0f - normalizedPos.z * 0.0f;
    posOffset.y = normalizedPos.z * 0.0f - normalizedPos.x * 1.0f;
    posOffset.z = normalizedPos.x * 0.0f - normalizedPos.y * 0.0f;
    if (D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&posOffset)) < 0.00001f)
        normalizedPos = Float3(1.0f, 0.0f, 0.0f);
    else
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&posOffset), reinterpret_cast<D3DXVECTOR3 *>(&posOffset));
    posOffset *= effect->radius;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3 *>(&posOffset), reinterpret_cast<D3DXVECTOR3 *>(&posOffset), &localMatrix);
    posOffset.z *= 6.0f;
    effect->position = posOffset + effect->vector5;
    effect->position.z = 0.0f;
    if (effect->releaseRequested != 0)
    {
        ++effect->releaseTimer;
        if (effect->releaseTimer >= 16)
            return 0;
        alpha = 1.0f - (f32)effect->releaseTimer / 16.0f;
        effect->vm.color1.d3dColor = (effect->vm.color1.d3dColor & 0xffffff) |
            ((i32)(alpha * 255.0f) << 24);
        effect->vm.scale.y = 2.0f - alpha;
        effect->vm.scale.x = effect->vm.scale.y;
    }
    return 1;
}

// FUNCTION: th08 0x426280
#pragma var_order(backgroundOffset, effect)
i32 __fastcall InitializeTintedBossTrackingCameraParticle(Effect *effect)
{
    Float3 backgroundOffset;

    backgroundOffset = -g_Background.cameraCurrent.lookAtOffset;
    effect->vector4 = g_Background.cameraCurrent.lookAtOffset + g_Background.cameraCurrent.position;
    effect->vector4.x += g_Rng.GetRandomF32SignedInRange(60.0f) + backgroundOffset.x / 2.0f;
    effect->vector4.y += g_Rng.GetRandomF32SignedInRange(100.0f) - 50.0f + backgroundOffset.y / 2.0f;
    effect->vector4.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + backgroundOffset.z / 2.0f;

    effect->vector2.x = g_Rng.GetRandomF32SignedInRange(0.001f) + effect->vector1.x;
    effect->vector2.y = g_Rng.GetRandomF32SignedInRange(0.03f) + effect->vector1.y;
    effect->vector2.z = -g_Rng.GetRandomF32InRange(0.1f) - 0.3f + effect->vector1.z;
    effect->vector3.x = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.y = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.z = -0.0003f;
    effect->vector2 = effect->vector2 * g_Supervisor.framerateMultiplier;
    effect->vector3 = effect->vector3 * g_Supervisor.framerateMultiplier;
    effect->drawGroup = 1;
    effect->vm.pos2.x = -9999.0f;
    effect->vm.posInitial.x = 0.0f;
    effect->vm.posFinal.x = 0.0f;
    effect->vm.posFinal.y = 0.0f;
    effect->vm.posFinal.z = 0.0f;
    effect->vm.rotateInitial.x = 0.0f;
    effect->vm.rotateInitial.y = 0.0f;
    effect->vm.rotateInitial.z = 0.0f;
    return 0;
}

// FUNCTION: th08 0x4264f0
#pragma var_order(delta, dot, effect)
i32 __fastcall UpdateTintedBossTrackingCameraParticle(Effect *effect)
{
    f32 dot;

    effect->vector2 += effect->vector3;
    effect->vector4 += effect->vector2;
    effect->position = effect->vector4;

    Float3 delta;
    delta = effect->position - g_Background.cameraCurrent.position;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&delta), reinterpret_cast<D3DXVECTOR3 *>(&delta));
    dot = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&g_Background.cameraCurrent.forward),
                      reinterpret_cast<D3DXVECTOR3 *>(&delta));
    if (dot < 0.94f)
        return 0;

    if (g_EnemyManager.HasBoss())
    {
        if (((g_EnemyManager.bosses[0]->flags1 >>
              ENEMY_FLAG_DAMAGEABLE_SHIFT) & 1) != 0)
        {
            if (effect->vm.pos2.x <= -9999.0f)
            {
                effect->vm.pos2 = g_EnemyManager.bosses[0]->position;
            }
            else
            {
                effect->vm.pos2 =
                    (g_EnemyManager.bosses[0]->position -
                     effect->vm.pos2) * 0.1f + effect->vm.pos2;
            }
        }
    }

    *reinterpret_cast<u32 *>(&effect->vm.flags) |= 0x20000;
    effect->vm.color2.r = ((u32)effect->vm.color1.r * g_Background.stageTextVm.color1.r) >> 8;
    effect->vm.color2.g = ((u32)effect->vm.color1.g * g_Background.stageTextVm.color1.g) >> 8;
    effect->vm.color2.b = ((u32)effect->vm.color1.b * g_Background.stageTextVm.color1.b) >> 8;
    effect->vm.color2.a = ((u32)effect->vm.color1.a * g_Background.stageTextVm.color1.a) >> 8;
    return 1;
}

// FUNCTION: th08 0x426720
#pragma var_order(backgroundOffset, effect)
i32 __fastcall InitializeRisingBossTrackingCameraParticle(Effect *effect)
{
    Float3 backgroundOffset;

    backgroundOffset = -g_Background.cameraCurrent.lookAtOffset;
    effect->vector4 = g_Background.cameraCurrent.lookAtOffset + g_Background.cameraCurrent.position;
    effect->vector4.x += g_Rng.GetRandomF32SignedInRange(60.0f) + backgroundOffset.x / 2.0f;
    effect->vector4.y += g_Rng.GetRandomF32SignedInRange(200.0f) - 200.0f + backgroundOffset.y / 2.0f;
    effect->vector4.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + backgroundOffset.z / 2.0f;

    effect->vector2.x = g_Rng.GetRandomF32SignedInRange(0.001f) + effect->vector1.x;
    effect->vector2.y = g_Rng.GetRandomF32SignedInRange(0.03f) + 0.4f;
    effect->vector2.z = -g_Rng.GetRandomF32InRange(0.1f) - 0.3f + effect->vector1.z;
    effect->vector3.x = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.y = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->vector3.z = -0.0003f;
    effect->vector2 = effect->vector2 * g_Supervisor.framerateMultiplier;
    effect->vector3 = effect->vector3 * g_Supervisor.framerateMultiplier;
    effect->drawGroup = 1;
    effect->vm.pos2.x = -9999.0f;
    effect->vm.posInitial.x = 0.0f;
    effect->vm.posFinal.x = 0.0f;
    effect->vm.posFinal.y = 0.0f;
    effect->vm.posFinal.z = 0.0f;
    effect->vm.rotateInitial.x = 0.0f;
    effect->vm.rotateInitial.y = 0.0f;
    effect->vm.rotateInitial.z = 0.0f;
    return 0;
}

// FUNCTION: th08 0x426990
#pragma var_order(delta, dot, effect)
i32 __fastcall UpdateRisingBossTrackingCameraParticle(Effect *effect)
{
    f32 dot;

    effect->vector2 += effect->vector3;
    effect->vector4 += effect->vector2;
    effect->position = effect->vector4;

    Float3 delta;
    delta = effect->position - g_Background.cameraCurrent.position;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&delta), reinterpret_cast<D3DXVECTOR3 *>(&delta));
    dot = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&g_Background.cameraCurrent.forward),
                      reinterpret_cast<D3DXVECTOR3 *>(&delta));
    if (dot < 0.94f)
        return 0;

    if (g_EnemyManager.bosses[0] != NULL)
    {
        if (((g_EnemyManager.bosses[0]->flags1 >>
              ENEMY_FLAG_DAMAGEABLE_SHIFT) & 1) != 0)
        {
            if (effect->vm.pos2.x <= -9999.0f)
            {
                effect->vm.pos2 = g_EnemyManager.bosses[0]->position;
            }
            else
            {
                effect->vm.pos2 =
                    (g_EnemyManager.bosses[0]->position -
                     effect->vm.pos2) * 0.1f + effect->vm.pos2;
            }
        }
    }
    return 1;
}

// FUNCTION: th08 0x426b20
#pragma var_order(angle, effect)
i32 __fastcall InitializeRandomDirectionalOffset(Effect *effect)
{
    f32 angle;

    effect->vector5 = effect->position;
    effect->vector5.z = 0.0f;
    angle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
    effect->vector6.x = cosf(angle);
    effect->vector6.y = sinf(angle);
    effect->vector6.z = 0.0f;
    return 0;
}

// FUNCTION: th08 0x426bb0
#pragma var_order(alpha, effect)
i32 __fastcall UpdateDirectionalOffset60(Effect *effect)
{
    f32 alpha;

    alpha = 256.0f - (f32)effect->timer * 256.0f / 60.0f;
    effect->position = effect->vector6 * alpha + effect->vector5;
    effect->position.z = 0.0f;
    return 1;
}

// FUNCTION: th08 0x426c40
i32 __fastcall TrackPlayerUntilAnimationEnds(Effect *effect)
{
    if (HasAnimationEnded(effect))
        return 0;

    effect->position = g_Player.position;
    return 1;
}

// FUNCTION: th08 0x426c90
#pragma var_order(alpha, effect)
i32 __fastcall UpdateDirectionalOffset240(Effect *effect)
{
    f32 alpha;

    alpha = 256.0f - (f32)effect->timer * 256.0f / 240.0f;
    effect->position = effect->vector6 * alpha + effect->vector5;
    return 1;
}

// FUNCTION: th08 0x426d10
#pragma var_order(effect, i, delta)
void __fastcall ShiftStageEffectOrigins(Float3 *delta)
{
    Effect *effect = g_EffectManager.effects;
    i32 i;

    for (i = 0; i < 0x200; i++, effect++)
    {
        if (effect->effectId == 0x33)
        {
            effect->vector4 += *delta;
        }
    }
}

// FUNCTION: th08 0x426d70
#pragma var_order(delta, dot, effect)
i32 __fastcall UpdateSpinningCameraParticle(Effect *effect)
{
    f32 dot;

    effect->vector2 += effect->vector3;
    effect->vector4 += effect->vector2;
    effect->position = effect->vector4;

    Float3 delta;
    delta = effect->position - g_Background.cameraCurrent.position;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&delta), reinterpret_cast<D3DXVECTOR3 *>(&delta));
    dot = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&g_Background.cameraCurrent.forward),
                      reinterpret_cast<D3DXVECTOR3 *>(&delta));
    if (dot < 0.94f)
        return 0;

    effect->vm.SetZRotation(AddNormalizeAngle(effect->vm.rotation.z, effect->vm.rotation.x));
    if (effect->position.z >= 0.0f)
        return 0;
    return 1;
}

// FUNCTION: th08 0x426e70
#pragma var_order(backgroundOffset, effect)
i32 __fastcall InitializeSpinningCameraParticle(Effect *effect)
{
    Float3 backgroundOffset;

    backgroundOffset = -g_Background.cameraCurrent.lookAtOffset;
    effect->vector4 = g_Background.cameraCurrent.lookAtOffset + g_Background.cameraCurrent.position;
    effect->vector4.x += g_Rng.GetRandomF32InRange(120.0f) - 60.0f + backgroundOffset.x / 2.0f;
    effect->vector4.y += g_Rng.GetRandomF32InRange(200.0f) - 100.0f + backgroundOffset.y / 2.0f;
    effect->vector4.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + backgroundOffset.z / 2.0f;

    effect->vector2.x = g_Rng.GetRandomF32InRange(0.06f) - 0.03f + effect->vector1.x;
    effect->vector2.y = g_Rng.GetRandomF32InRange(0.06f) - 0.03f + effect->vector1.y;
    effect->vector2.z = g_Rng.GetRandomF32InRange(0.1f) + 0.03f + effect->vector1.z;
    effect->vector3.x = g_Rng.GetRandomF32InRange(0.0002f) - 0.0001f;
    effect->vector3.y = g_Rng.GetRandomF32InRange(0.0002f) - 0.0001f;
    effect->vector2 = effect->vector2 * g_Supervisor.framerateMultiplier;
    effect->vector3 = effect->vector3 * g_Supervisor.framerateMultiplier;
    effect->drawGroup = 1;
    effect->vm.rotation.z = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
    effect->vm.rotation.x = g_Rng.GetRandomF32InRange(0.03141592815518379f) - 0.015707964077591896f;
    return 0;
}

// FUNCTION: th08 0x4270c0
#pragma var_order(angle, effect)
i32 __fastcall InitializeDirectionalOffset(Effect *effect)
{
    f32 angle;

    if (effect->vector1.x > -990.0)
        angle = AddNormalizeAngle(effect->vector1.x, 0.0f);
    else
        angle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;

    effect->vector5 = effect->position;
    effect->vector5.z = 0.0f;
    effect->vector6.x = cosf(angle);
    effect->vector6.y = sinf(angle);
    effect->vector6.z = 0.0f;
    effect->vector6 *= g_Rng.GetRandomF32InRange(1.5f) + 1.0f;
    return 0;
}

// FUNCTION: th08 0x4271a0
#pragma var_order(alpha, effect)
i32 __fastcall UpdateEasedDirectionalOffset(Effect *effect)
{
    f32 alpha;

    alpha = (f32)effect->timer / 90.0f;
    alpha = 1.0f - (1.0f - alpha) * (1.0f - alpha);
    effect->position = effect->vector6 * alpha * 128.0f + effect->vector5;
    effect->position.z = 0.0f;
    return 1;
}

// FUNCTION: th08 0x427250
i32 __fastcall KeepTrailAlive(Effect *effect)
{
    return 1;
}

// FUNCTION: th08 0x427260
#pragma var_order(offset, effect)
i32 __fastcall InitializeTrailOffset(Effect *effect)
{
    Float3 offset;

    offset.FromAngleMagnitude(effect->vector1.x, 256.0f);
    effect->position.x += offset.x;
    effect->position.y += offset.y;
    effect->vm.rotation.z = AddNormalizeAngle(effect->vector1.x, ZUN_PI / 2.0f);
    return 0;
}

// FUNCTION: th08 0x4272e0
i32 __fastcall InitializeRadialTrail(Effect *effect)
{
    effect->vertices = static_cast<VertexTex1DiffuseXyzrhw *>(g_ZunMemory.Alloc(0x1c38, "Effect"));
    if (effect->vertices == NULL)
        return -1;

    effect->vertexSegmentCount = 3;
    effect->vector5 = effect->position;
    effect->vector6.x = 0.0f;
    effect->vector6.y = 0.0f;
    effect->vector6.z = 1.0f;
    effect->vector7.x = 0.0f;
    effect->vector7.y = -1.0f;
    effect->vector7.z = 0.0f;
    effect->angle = effect->vector1.x;
    effect->radius = effect->vector1.y;
    effect->shapeThickness = effect->vector1.z;

    g_AnmManager->InitializeHorizontalTextureStrip(&effect->vm, effect->vertices, effect->vertexSegmentCount * 2);
    effect->verticesDirty = 1;
    effect->drawCallback = DrawRadialTrail;
    effect->secondaryRadius = 0.0f;
    effect->secondaryAngle = 0.0f;
    effect->radialWaveCount = 0.0f;
    effect->vertexSegmentCount = 24;
    return 0;
}

// FUNCTION: th08 0x427450
#pragma var_order(i, innerRadius, vertex, angleStep, radius)
i32 __fastcall DrawRadialTrail(Effect *effect)
{
    i32 i;
    f32 innerRadius;
    VertexTex1DiffuseXyzrhw *vertex;
    f32 angleStep;
    f32 radius;

    if (effect->verticesDirty)
    {
        angleStep = ZUN_2PI / effect->vertexSegmentCount;
        radius = effect->shapeThickness /
                 sinf((ZUN_PI - angleStep) / 2.0f);
        vertex = effect->vertices;
        g_AnmManager->InitializeVerticalTextureStrip(
            &effect->vm, effect->vertices, effect->vertexSegmentCount * 2 + 2);

        if (effect->secondaryRadius == 0.0f)
        {
            f32 angle;
            angle = effect->angle;
            innerRadius = effect->radius - radius;
            radius += effect->radius;
            for (i = effect->vertexSegmentCount + 1; i > 0; --i)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;

                vertex->pos.z = 0.0f;
                vertex->pos.FromAngleMagnitude(angle, radius);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex++;

                vertex->pos.z = 0.0f;
                vertex->pos.FromAngleMagnitude(angle, innerRadius);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex++;

                angle += angleStep;
            }
        }
        else if (effect->radialWaveCount == 0.0f)
        {
#pragma var_order(innerEllipseRadius, outerEllipseRadius, angle, point)
            f32 angle = 0.0f;
            Float3 point;
            f32 outerEllipseRadius;
            f32 innerEllipseRadius;

            outerEllipseRadius = radius + effect->secondaryRadius;
            innerEllipseRadius = effect->secondaryRadius - radius;
            innerRadius = effect->radius - radius;
            radius += effect->radius;

            for (i = effect->vertexSegmentCount + 1; i > 0; --i)
            {
                point.FromRotatedVec2(angle, radius, outerEllipseRadius);
                Rotate(&vertex->pos, &point, effect->secondaryAngle);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                point.FromRotatedVec2(angle, innerRadius, innerEllipseRadius);
                Rotate(&vertex->pos, &point, effect->secondaryAngle);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                angle += angleStep;
            }
        }
        else
        {
#pragma var_order(secondAngleStep, secondAngle, radialOffset, angle, unused)
            f32 secondAngle;
            f32 angle;
            f32 secondAngleStep;
            f32 radialOffset;

            secondAngle = effect->secondaryAngle;
            angle = effect->angle;
            secondAngleStep = ZUN_2PI * effect->radialWaveCount / effect->vertexSegmentCount;
            Float3 unused;
            innerRadius = effect->radius - radius;
            radius += effect->radius;

            for (i = effect->vertexSegmentCount + 1; i > 0; --i)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;
                if (secondAngle >= ZUN_PI)
                    secondAngle -= ZUN_2PI;

                radialOffset = cosf(secondAngle) * effect->secondaryRadius;
                vertex->pos.FromAngleMagnitude(angle, radius + radialOffset);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                vertex->pos.FromAngleMagnitude(angle, innerRadius + radialOffset);
                vertex->pos += effect->vector5;
                vertex->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                vertex->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                vertex->pos.z = 0.0f;
                vertex++;

                angle += angleStep;
                secondAngle += secondAngleStep;
            }
        }
        effect->verticesDirty = 0;
    }

    g_AnmManager->DrawVertices(&effect->vm, effect->vertices, effect->vertexSegmentCount * 2 + 2);
    return 1;
}

// FUNCTION: th08 0x427970
i32 __fastcall InitializeAlternateLayerRadialTrail(Effect *effect)
{
    InitializeRadialTrail(effect);
    effect->alternateDrawGroup = 1;
    return 0;
}

// FUNCTION: th08 0x427990
i32 __fastcall SyncRadialTrailRadius(Effect *effect)
{
    effect->verticesDirty = 1;
    effect->shapeThickness = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    return 1;
}

// FUNCTION: th08 0x4279d0
i32 __fastcall SyncRadialTrailShape(Effect *effect)
{
    effect->vertexSegmentCount = effect->vm.intVar0;
    effect->radialWaveCount = (f32)effect->vm.intVar1;
    effect->shapeThickness = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    effect->secondaryRadius = effect->vm.pos.y;
    effect->angle = effect->vm.rotation.z;
    effect->secondaryAngle = effect->vm.rotation.y;
    effect->verticesDirty = 1;
    return 1;
}

// FUNCTION: th08 0x427a60
i32 __fastcall UpdateTimedRadialTrail(Effect *effect)
{
    effect->vertexSegmentCount = 32;
    effect->shapeThickness = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    effect->secondaryRadius = effect->vm.pos.y;
    effect->verticesDirty = 1;
    if (effect->timer >= 120)
        return 0;
    return 1;
}

// FUNCTION: th08 0x427ae0
i32 __fastcall UpdateFadingRadialTrail(Effect *effect)
{
    effect->verticesDirty = 1;
    effect->shapeThickness = effect->vm.scale.x;
    effect->radius = effect->vm.pos.x;
    effect->secondaryRadius = effect->vm.pos.y;
    effect->angle = effect->vm.rotation.z;
    if (effect->vm.color1.a == 0)
        return 0;
    return 1;
}

// FUNCTION: th08 0x427b50
i32 __fastcall SyncAnchoredRadialTrail(Effect *effect)
{
    effect->vertexSegmentCount = effect->vm.intVar0;
    effect->radialWaveCount = (f32)effect->vm.intVar1;
    effect->shapeThickness = effect->vm.scale.x;
    effect->radius = effect->vm.floatVar1;
    effect->angle = effect->vm.rotation.z;
    effect->secondaryAngle = effect->vm.rotation.y;
    effect->verticesDirty = 1;
    effect->vector5 = effect->vm.pos;
    return 1;
}

// FUNCTION: th08 0x427bf0
#pragma var_order(effect, i)
ChainCallbackResult EffectManager::OnUpdate(EffectManager *effectManager)
{
    Effect *effect = effectManager->effects;
    i32 i;

    effectManager->activeCount = 0;
    effectManager->drawGroupTails[0] = &effectManager->drawGroupSentinel0;
    effectManager->drawGroupTails[1] = &effectManager->drawGroupSentinel1;
    effectManager->drawGroupTails[2] = &effectManager->drawGroupSentinel2;
    effectManager->drawGroupTails[3] = &effectManager->drawGroupSentinel3;
    effectManager->drawGroupTails[4] = &effectManager->drawGroupSentinel4;

    effectManager->drawGroupSentinel0.nextInDrawGroup = NULL;
    effectManager->drawGroupSentinel1.nextInDrawGroup = NULL;
    effectManager->drawGroupSentinel2.nextInDrawGroup = NULL;
    effectManager->drawGroupSentinel3.nextInDrawGroup = NULL;
    effectManager->drawGroupSentinel4.nextInDrawGroup = NULL;

    for (i = 0; i < 653; i++, effect++)
    {
        if (effect->active == 0)
        {
            if (effect->vertices != NULL)
            {
                g_ZunMemory.Free(effect->vertices);
                effect->vertices = NULL;
            }
            continue;
        }

        effectManager->activeCount++;
        if (!g_GameManager.flags.deathbombFreezeActive ||
            effect->updateDuringFreeze != 0)
        {
            if (effect->updateCallback != NULL && effect->updateCallback(effect) != 1)
            {
                effect->active = 0;
                continue;
            }
            if (g_AnmManager->ExecuteScript(&effect->vm))
            {
                effect->active = 0;
                continue;
            }
            effect->timer++;
        }

        effect->nextInDrawGroup = NULL;
        if (effect->effectId == 0x40)
            continue;

        if (effect->drawGroup == 1 || effect->drawGroup >= 3)
        {
            effectManager->drawGroupTails[1]->nextInDrawGroup = effect;
            effectManager->drawGroupTails[1] = effect;
        }
        else if (effect->drawGroup == 0)
        {
            if (effect->alternateDrawGroup != 0)
            {
                effectManager->drawGroupTails[3]->nextInDrawGroup = effect;
                effectManager->drawGroupTails[3] = effect;
            }
            else if (((*reinterpret_cast<u32 *>(&effect->vm.flags) >> 4) & 3) == 1)
            {
                effectManager->drawGroupTails[4]->nextInDrawGroup = effect;
                effectManager->drawGroupTails[4] = effect;
            }
            else
            {
                effectManager->drawGroupTails[0]->nextInDrawGroup = effect;
                effectManager->drawGroupTails[0] = effect;
            }
        }
        else
        {
            effectManager->drawGroupTails[2]->nextInDrawGroup = effect;
            effectManager->drawGroupTails[2] = effect;
        }
    }

    if (++effectManager->tamperCheckCounter % 300 == 100 &&
        g_GameManager.IsTampered())
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x427f00
#pragma var_order(effect)
ChainCallbackResult EffectManager::OnDraw(EffectManager *effectManager)
{
    Effect *effect;

    effect = effectManager->drawGroupSentinel0.nextInDrawGroup;
    while (effect != NULL)
    {
        if (effect->drawCallback != NULL)
        {
            effect->drawCallback(effect);
        }
        else
        {
            effect->vm.pos = effect->position;
            effect->vm.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
            effect->vm.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
            effect->vm.pos.z = 0.07f;
            effect->vm.pos += effect->vm.pos2;
            g_AnmManager->Draw2D(&effect->vm);
        }
        effect = effect->nextInDrawGroup;
    }

    effect = effectManager->drawGroupSentinel2.nextInDrawGroup;
    while (effect != NULL)
    {
        effect->vm.pos = effect->position;
        g_AnmManager->DrawCameraFacingQuad(&effect->vm);
        effect = effect->nextInDrawGroup;
    }

    effect = effectManager->drawGroupSentinel4.nextInDrawGroup;
    while (effect != NULL)
    {
        if (effect->drawCallback != NULL)
        {
            effect->drawCallback(effect);
        }
        else
        {
            effect->vm.pos = effect->position;
            effect->vm.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
            effect->vm.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
            effect->vm.pos.z = 0.07f;
            effect->vm.pos += effect->vm.pos2;
            g_AnmManager->Draw2D(&effect->vm);
        }
        effect = effect->nextInDrawGroup;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x428100
#pragma var_order(effect, this)
i32 EffectManager::DrawBulletLayerEffects()
{
    Effect *effect = this->drawGroupSentinel3.nextInDrawGroup;

    while (effect != NULL)
    {
        if (effect->drawCallback != NULL)
        {
            effect->drawCallback(effect);
        }
        else
        {
            effect->vm.pos = effect->position;
            effect->vm.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
            effect->vm.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
            effect->vm.pos += effect->vm.pos2;
            effect->vm.pos.z = 0.04f;
            g_AnmManager->Draw2D(&effect->vm);
        }

        effect = effect->nextInDrawGroup;
    }

    return 1;
}

// FUNCTION: th08 0x4281e0
#pragma var_order(effect, i, this)
i32 EffectManager::DrawBackgroundEffects()
{
    Effect *effect = this->drawGroupSentinel1.nextInDrawGroup;
    i32 i = 0;

    if (g_Supervisor.cfg.effectQuality == MINIMUM)
    {
        return 1;
    }

    while (effect != NULL)
    {
        i++;
        if (g_Supervisor.cfg.effectQuality == MODERATE && (i & 1) != 0)
        {
            return 1;
        }

        effect->vm.pos = effect->position;
        if (effect->drawGroup == 4)
        {
            g_AnmManager->Draw2D(&effect->vm);
        }
        else if (effect->drawGroup == 1)
        {
            if (effect->effectId == 0x33 || effect->effectId == 0x3F)
            {
                g_AnmManager->DrawWithCallback(
                    &effect->vm, AdjustStageEffectDrawPosition);
            }
            else
            {
                g_AnmManager->DrawCameraFacingQuad(&effect->vm);
            }
        }
        else
        {
            g_AnmManager->DrawProjected3DQuad(&effect->vm);
        }

        effect = effect->nextInDrawGroup;
    }
    return 1;
}

// FUNCTION: th08 0x428310
#pragma var_order(delta, point)
void __fastcall AdjustStageEffectDrawPosition(AnmVm *effect, D3DXVECTOR3 *base)
{
    D3DXVECTOR3 delta;
    D3DXVECTOR3 point;

    if (!g_GameManager.isInGameMenu && !g_GameManager.showRetryMenu)
    {
        point = *base + *reinterpret_cast<D3DXVECTOR3 *>(&effect->posFinal);
        delta = *reinterpret_cast<D3DXVECTOR3 *>(&effect->pos2) - point;
        if (effect->pos2.x > -9999.0f)
        {
            delta.x += 32.0f;
            delta.y += 16.0f;
            delta.z = 0.0f;
            if (D3DXVec3LengthSq(&delta) < 25600.0f)
            {
                effect->posInitial.x += 0.0005000000237487257f;
                *reinterpret_cast<D3DXVECTOR3 *>(&effect->posFinal) += delta * effect->posInitial.x;
            }
        }

        delta = point - reinterpret_cast<const D3DXVECTOR3 &>(g_Player.position);
        delta.x -= 32.0f;
        delta.y -= 16.0f;
        delta.z = 0.0f;
        if (D3DXVec3LengthSq(&delta) < 7744.0f)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(&effect->posFinal) += delta * 0.019999999552965164f;
        }
    }
    *base += *reinterpret_cast<D3DXVECTOR3 *>(&effect->posFinal);
}

// FUNCTION: th08 0x4284b0
ZunResult EffectManager::LoadEffectResources(EffectManager *effectManager)
{
    effectManager->ResetEffects();
    effectManager->effectAnm = g_AnmManager->GetAnm(6);
    g_GuiMessageStageMode = 0;
    g_Background.spellVmCount = 2;

    if (!IsDisableResourceReload())
    {
        if (!g_GameManager.IsSpellPractice() || g_GameManager.currentSpellCardNumber < 216)
        {
            effectManager->stageEffectAnm = g_AnmManager->PreloadAnm(9, g_EffectAnms[g_GameManager.currentStage]);
        }
        else
        {
            effectManager->stageEffectAnm =
                g_AnmManager->PreloadAnm(9, g_EffectAnms[g_GameManager.currentSpellCardNumber - 216 + 9]);
        }
        if (effectManager->stageEffectAnm == NULL)
            return ZUN_ERROR;
    }
    else
    {
        effectManager->stageEffectAnm = g_AnmManager->GetAnm(9);
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x428590
#pragma var_order(effect, i)
ZunResult EffectManager::ReleaseEffectResources(EffectManager *effectManager)
{
    Effect *effect = effectManager->effects;
    i32 i;
    for (i = 0; i < 653; i++, effect++)
    {
        if (effect->vertices != NULL)
        {
            g_ZunMemory.Free(effect->vertices);
            effect->vertices = NULL;
        }
    }
    if (!IsDisableResourceReload())
        g_AnmManager->ReleaseAnm(9);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x428620
ZunResult EffectManager::RegisterChain()
{
    EffectManager *effectManager = &g_EffectManager;
    effectManager->ResetEffects();
    g_EffectManagerCalcChain.SetCallback((ChainCallback)EffectManager::OnUpdate);
    g_EffectManagerCalcChain.addedCallback = (ChainLifetimeCallback)EffectManager::LoadEffectResources;
    g_EffectManagerCalcChain.deletedCallback = (ChainLifetimeCallback)EffectManager::ReleaseEffectResources;
    g_EffectManagerCalcChain.arg = effectManager;
    if (g_Chain.AddToCalcChain(&g_EffectManagerCalcChain, CHAIN_PRIO_CALC_EFFECTMANAGER) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_EffectManagerDrawChain.SetCallback((ChainCallback)EffectManager::OnDraw);
    g_EffectManagerDrawChain.arg = effectManager;
    g_Chain.AddToDrawChain(&g_EffectManagerDrawChain, CHAIN_PRIO_DRAW_EFFECTMANAGER);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4286b0
void EffectManager::CutChain()
{
    g_Chain.Cut(&g_EffectManagerCalcChain);
    g_Chain.Cut(&g_EffectManagerDrawChain);
}

// FUNCTION: th08 0x428720
i32 __fastcall HasAnimationEnded(Effect *effect)
{
    return effect->vm.currentInstruction == NULL;
}

// FUNCTION: th08 0x428740
EffectManager::EffectManager()
{
    this->ResetEffects();
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->scaleZ = 1.0f;
    this->scaleW = 1.0f;
}

// FUNCTION: th08 0x4287e0
Effect::Effect()
{
}


} // namespace th08
