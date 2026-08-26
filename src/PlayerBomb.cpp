#include "th_pch.h"

#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "AnmManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"
#include "ReplayManager.hpp"
#include "EnemyManager.hpp"
#include "Spellcard.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ScreenEffect.hpp"
#include "utils.hpp"

// Repository production TU for the target-contiguous Player bomb/shot callback
// family at 0x0040BC20..0x004142C0. The filename describes the recovered
// cluster; it does not claim the original source filename.
namespace th08
{
// FUNCTION: th08 0x40bc20
i32 Player::IsHuman()
{
    return !this->isYoukai;
}

// FUNCTION: th08 0x40bc40
i32 Player::IsYoukai()
{
    return this->isYoukai;
}
// FUNCTION: th08 0x40bc60
#pragma var_order(sourceColor, mixedColor)
void __fastcall SetBombBackgroundTint(Player *player, D3DCOLOR color)
{
    ZunColor mixedColor;
    ZunColor sourceColor;
    sourceColor.d3dColor = color;

    if (player->bombState.timer < 60)
    {
        mixedColor.r = 0x80 - (0x80 - sourceColor.r) * (i32)player->bombState.timer / 60;
        mixedColor.g = 0x80 - (0x80 - sourceColor.g) * (i32)player->bombState.timer / 60;
        mixedColor.b = 0x80 - (0x80 - sourceColor.b) * (i32)player->bombState.timer / 60;
    }
    else if (player->bombState.timer >= player->bombState.duration - 60)
    {
        mixedColor.r = 0x80 - (0x80 - sourceColor.r) * (player->bombState.duration - (i32)player->bombState.timer) / 60;
        mixedColor.g = 0x80 - (0x80 - sourceColor.g) * (player->bombState.duration - (i32)player->bombState.timer) / 60;
        mixedColor.b = 0x80 - (0x80 - sourceColor.b) * (player->bombState.duration - (i32)player->bombState.timer) / 60;
    }
    else
    {
        mixedColor.d3dColor = sourceColor.d3dColor;
    }
    mixedColor.a = 0x80;
    g_AnmManager->SetMixColorDefault();
    g_Background.AccumulateTint(mixedColor.d3dColor);
    g_Background.retainTint = 1;
}

// FUNCTION: th08 0x40eb50
i32 AnmVm::UpdatePulsingRadialTrail()
{
    reinterpret_cast<Effect *>(this)->verticesDirty = 1;
    reinterpret_cast<Effect *>(this)->vertexSegmentCount = 48;
    reinterpret_cast<Effect *>(this)->shapeThickness = 32.0f;
    reinterpret_cast<Effect *>(this)->radius =
        64.0f + (f32)((reinterpret_cast<Effect *>(this)->timer.HasTicked() & 1) ? 8 : 0);
    reinterpret_cast<Effect *>(this)->angle = 0.0f;
    return 1;
}

// FUNCTION: th08 0x40ec30
void AnmVm::StartPositionInterpolation(i32 duration, i32 mode, Float3 *initial, Float3 *final)
{
    this->interpCurrentTimers[AnmInterp_Pos] = 0;
    this->interpEndTimers[AnmInterp_Pos] = duration;
    this->interpModes[AnmInterp_Pos] = mode;
    this->posInitial = *initial;
    this->posFinal = *final;
}

// FUNCTION: th08 0x40eca0
void AnmVm::StartColor1RgbInterpolation(i32 duration, i32 mode, u32 initial, u32 final)
{
    this->interpCurrentTimers[AnmInterp_RGB1] = 0;
    this->interpEndTimers[AnmInterp_RGB1] = duration;
    this->interpModes[AnmInterp_RGB1] = mode;
    this->color1Initial.r = (initial >> 16) & 0xFF;
    this->color1Initial.g = (initial >> 8) & 0xFF;
    this->color1Initial.b = initial & 0xFF;
    this->color1Final.r = (final >> 16) & 0xFF;
    this->color1Final.g = (final >> 8) & 0xFF;
    this->color1Final.b = final & 0xFF;
}

// FUNCTION: th08 0x40ed50
void AnmVm::StartColor1AlphaInterpolation(i32 duration, i32 mode, i32 initial, i32 final)
{
    this->interpCurrentTimers[AnmInterp_Alpha1] = 0;
    this->interpEndTimers[AnmInterp_Alpha1] = duration;
    this->interpModes[AnmInterp_Alpha1] = mode;
    this->color1Initial.a = initial;
    this->color1Final.a = final;
}

// FUNCTION: th08 0x40eda0
void AnmVm::StartScaleInterpolation(i32 duration, i32 mode, Float2 *initial, Float2 *final)
{
    this->interpCurrentTimers[AnmInterp_Scale] = 0;
    this->interpEndTimers[AnmInterp_Scale] = duration;
    this->interpModes[AnmInterp_Scale] = mode;
    this->scaleInitial = *initial;
    this->scaleFinal = *final;
}

// FUNCTION: th08 0x40ebc0
ZunBool ZunTimer::IsPeriodic(i32 interval)
{
    return this->current != this->previous && (this->current % interval) == 0;
}

// FUNCTION: th08 0x40e350
ZunBool ZunTimer::JustReached(i32 value)
{
    return this->current != this->previous && this->current == value;
}

// FUNCTION: th08 0x40bf00
void Player::SpawnBombStateEffect()
{
    AnmVm *effect;
    if (this->stateEffect != NULL)
        this->stateEffect->active = false;

    effect = g_EffectManager.SpawnEffectInFixedSlot(23, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 0, 1, -1);
    effect->interpCurrentTimers[AnmInterp_Scale] = 0;
    effect->interpEndTimers[AnmInterp_Scale] = this->timer;
    effect->interpModes[AnmInterp_Scale] = AnmInterpMode_Linear;
    effect->scaleInitial = effect->scale;
    effect->scaleFinal.x = 0.0625f;
    effect->scaleFinal.y = 0.0625f;
    effect->intVar0 = (i32)this->timer;
    effect->angleVel.z *= -1.0f;
    effect->color1.r = 0xFF;
    effect->color1.g = 0x40;
    effect->color1.b = 0x40;
    this->stateEffect = reinterpret_cast<Effect *>(effect);
}

// FUNCTION: th08 0x40be30
#pragma var_order(i, bomb, workItem)
void __fastcall BeginBombSpell(Player *player, i32 cutInType, const char *cutInText, i32 duration, i32 timer, i32 cutInArg)
{
    u32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    g_Spellcard.CutInPlayer(cutInType, cutInText, cutInArg);
    bomb->duration = duration;
    player->timer = timer;
    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->SpawnBombStateEffect();
    i = 0;
    workItem = bomb->workItems;
    for (; i < 128; i++, workItem++)
        workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
    g_ItemManager.AutoCollectAllItems();
    bomb->tailPosition = player->position;
}

// FUNCTION: th08 0x40c010
#pragma var_order(i, bomb, workItem, angle)
void __fastcall UpdateFantasyOrbBomb(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x97\xEC\x95\x84\x81\x75\x96\xB2\x91\x7A\x96\xAD\x8E\xEC\x81\x76",
                     200, 260, 0);
        g_EffectManager.SpawnEffect(12, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xFF4040FF);
        angle = -ZUN_PI;
        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[i].vms[0], 19);
            bomb->workItems[i].angle = angle;
            angle += ZUN_PI / 8.0f;
            workItem->position = player->position;
            workItem->pathPoints[0] = workItem->position;
            workItem->motionStep = 0.0f;
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            workItem->cancelRegion = player->CreateCircleCancelRegion(&player->position, 96.0f, 0.0f, 200, 6);
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 64.0f, 0.0f, 5, 200);
            workItem->damageRegion->collisionInterval = 2;
            workItem->damageRegion->hitCap = 200;
            workItem->damageRegion->mode = 1;
        }
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
    }

    if (bomb->timer < 40)
    {
        Float3 previousPosition;
        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->angle = AddNormalizeAngle(
                workItem->angle, (i & 1) ? 0.052359879016876221f : -0.052359879016876221f);
            previousPosition = workItem->position;
            workItem->position.x = cosf(workItem->angle) * workItem->motionStep + workItem->pathPoints[0].x;
            workItem->position.y = sinf(workItem->angle) * workItem->motionStep + workItem->pathPoints[0].y;
            workItem->motionStep += 3.2f;
            workItem->motion = workItem->position - previousPosition;
        }
    }
    else
    {
#pragma var_order(yDelta, xDelta, speed, targetPosition, slot)
        Float3 targetPosition;
        f32 speed;
        f32 xDelta;
        f32 yDelta;
        PlayerCollisionRegion *slot;
        if (bomb->timer.JustReached(40))
        {
            workItem = bomb->workItems;
            for (i = 0; i < 16; i++, workItem++)
            {
                workItem->speed = sqrtf(workItem->motion.x * workItem->motion.x +
                                        workItem->motion.y * workItem->motion.y);
                workItem->angle = VectorAngle(workItem->motion.x, workItem->motion.y);
                workItem->stateTimer = 0;
                workItem->motionStep = 8.0f;
            }
        }

        i = 0;
        workItem = bomb->workItems;
        for (; i < 16; i++, workItem++)
        {
            if (!workItem->state)
                continue;

            if (workItem->state == PLAYER_BOMB_WORK_ITEM_ACTIVE && bomb->timer.HasTicked())
            {
                if (player->tailPosition0.x > -100.0f)
                    targetPosition = player->tailPosition0;
                else
                    targetPosition = player->position;

                    xDelta = targetPosition.operator float *()[0] - workItem->position.x;
                    yDelta = targetPosition.operator float *()[1] - workItem->position.y;
                    speed = sqrtf(xDelta * xDelta + yDelta * yDelta) / (workItem->motionStep / 8.0f);
                    if (speed < 1.0f)
                        speed = 1.0f;

                    xDelta = xDelta / speed + workItem->motion.x;
                    yDelta = yDelta / speed + workItem->motion.y;
                    speed = sqrtf(xDelta * xDelta + yDelta * yDelta);
                    workItem->motionStep = speed > 10.0f ? 10.0f : speed;
                    if (workItem->motionStep < 1.0f)
                        workItem->motionStep = 1.0f;
                    workItem->motion.x = xDelta * workItem->motionStep / speed;
                    workItem->motion.y = yDelta * workItem->motionStep / speed;

                    player->CreateCircleCancelRegion(&workItem->position, 128.0f, 0.0f, 0, 6);
                    if (workItem->damageRegion->hitAccumulator >= workItem->damageRegion->hitCap ||
                        bomb->timer >= bomb->duration - 30)
                    {
                        workItem->cancelRegion->active = 0;
                        workItem->damageRegion->active = 0;
                        player->CreateCircleCancelRegion(&player->position, 64.0f, 4.266666889190674f, 30, 6);
                        slot = player->CreateCircleDamageRegion(&workItem->position, 64.0f, 12.800000190734863f, 500, 12);
                        slot->collisionInterval = 4;
                        slot->hitCap = 0;
                        g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position), 8, -1);
                        workItem->state = PLAYER_BOMB_WORK_ITEM_FINISHING;
                        workItem->vms[0].pendingInterrupt = 1;
                        workItem->motion / 8.0f;
                        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->position.x);
                        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
                    }
            }

            workItem->position.x += g_Supervisor.framerateMultiplier * workItem->motion.x;
            workItem->position.y += g_Supervisor.framerateMultiplier * workItem->motion.y;
        }
    }

    i = 0;
    workItem = bomb->workItems;
    for (; i < 16; i++, workItem++)
    {
        if (!workItem->state)
            continue;
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_ACTIVE)
        {
            workItem->cancelRegion->center.x = workItem->position.x;
            workItem->cancelRegion->center.y = workItem->position.y;
            workItem->damageRegion->center.x = workItem->position.x;
            workItem->damageRegion->center.y = workItem->position.y;
        }
        else if (workItem->state && bomb->timer.HasTicked())
        {
            if (++workItem->stateTimer >= 30)
                workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
        }
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x40c7b0
f32 VectorAngle(f32 y, f32 x)
{
    return (f32)atan2(y, x);
}

// FUNCTION: th08 0x40c7d0
#pragma var_order(inverse, this)
Float3 Float3::operator/(f32 scalar) const
{
    f32 inverse;

    inverse = 1.0f / scalar;
    return Float3(this->x * inverse, this->y * inverse, this->z * inverse);
}

// FUNCTION: th08 0x40c820
#pragma var_order(vm, i, workItem)
void __fastcall DrawFantasyOrbBomb(Player *player)
{
    PlayerBombWorkItem *workItem;
    i32 i;
    AnmVm *vm;

    SetBombBackgroundTint(player, 0x80404040);
    i = 0;
    workItem = player->bombState.workItems;
    for (; i < 16; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;
        vm = &workItem->vms[0];
        vm->pos = workItem->position + vm->pos2;
        vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        vm->pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
    }
}


DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 7, g_PlayerDreamSealColors) = {
    0x8FFFFFFF, 0x8F0000FF, 0x8FFF00FF, 0x8FFF0000, 0x8FFFFF00, 0x8F00FF00, 0x8F00FFFF,
};

// FUNCTION: th08 0x40c910
#pragma var_order(i, bomb, workItem, angle, previousPosition)
void __fastcall UpdateFantasySealBlinkDeathbomb(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x90\x5F\x97\xEC\x81\x75\x96\xB2\x91\x7A\x95\x95\x88\xF3\x81\x40\x8F\x75\x81\x76",
                     200, 260, 1);
        g_EffectManager.SpawnEffect(12, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xFF4040FF);
        angle = -ZUN_PI;
        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[i].vms[0], 19);
            bomb->workItems[i].angle = angle;
            angle += ZUN_PI / 8.0f;
            workItem->position = player->position;
            workItem->pathPoints[0] = workItem->position;
            workItem->motionStep = 0.0f;
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            workItem->cancelRegion = player->CreateCircleCancelRegion(&player->position, 96.0f, 0.0f, 200, 6);
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 64.0f, 0.0f, 5, 200);
            workItem->damageRegion->collisionInterval = 2;
            workItem->damageRegion->hitCap = 200;
            workItem->damageRegion->mode = 1;
        }
        bomb->secondaryWorkCursor = 0;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
    }

    Float3 previousPosition;
    workItem = bomb->workItems;
    for (i = 0; i < 16; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_ACTIVE)
        {
            workItem->angle = AddNormalizeAngle(
                workItem->angle, (i & 1) ? 0.052359879016876221f : -0.052359879016876221f);
            previousPosition = workItem->position;
            workItem->position.x = cosf(workItem->angle) * workItem->motionStep + workItem->pathPoints[0].x;
            workItem->position.y = sinf(workItem->angle) * workItem->motionStep + workItem->pathPoints[0].y;
            if (bomb->timer < 40)
            {
                if (i & 1)
                    workItem->motionStep += 1.2000000476837158f;
                else
                    workItem->motionStep += 2.4000000953674316f;
            }
            workItem->motion = workItem->position - previousPosition;

            if (bomb->timer >= bomb->duration - 40 - i)
            {
                workItem->cancelRegion->active = 0;
                workItem->damageRegion->active = 0;
                player->CreateCircleCancelRegion(&player->position, 64.0f, 4.266666889190674f, 30, 6);
                PlayerCollisionRegion *slot =
                    player->CreateCircleDamageRegion(&workItem->position, 64.0f, 8.533333778381348f, 25, 15);
                slot->collisionInterval = 5;
                slot->hitCap = 50;
                g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position), 8, -1);
                workItem->state = PLAYER_BOMB_WORK_ITEM_FINISHING;
                workItem->vms[0].pendingInterrupt = 1;
                workItem->motion / 8.0f;
                g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->position.x);
                ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
            }

            workItem->cancelRegion->center.x = workItem->position.x;
            workItem->cancelRegion->center.y = workItem->position.y;
            workItem->damageRegion->center.x = workItem->position.x;
            workItem->damageRegion->center.y = workItem->position.y;
        }
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
    }

    if (bomb->timer >= 40)
    {
#pragma var_order(color, spawnPosition, slot)
        Float3 spawnPosition;
        u32 color;
        PlayerCollisionRegion *slot;
        if (bomb->timer % 20 == 0)
        {
            workItem = &bomb->workItems[bomb->secondaryWorkCursor + 16];
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 20);
            bomb->secondaryWorkCursor = 1;
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            color = g_PlayerDreamSealColors[(u32)((i32)bomb->timer / 20) % 7];
            if (player->tailPosition0.x > -100.0f)
                spawnPosition = player->tailPosition0;
            else
            {
                spawnPosition.x = g_Rng.GetRandomF32InRange(320.0f) + 32.0f;
                spawnPosition.y = g_Rng.GetRandomF32InRange(384.0f) + 32.0f;
                spawnPosition.z = 0.0f;
            }
            workItem->position = spawnPosition;
            g_EffectManager.SpawnEffect(49, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position), 1, color);
            g_EffectManager.SpawnEffect(55, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position), 1, color);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&spawnPosition, 64.0f, 4.266666889190674f, 30, 6);
            workItem->damageRegion = player->CreateCircleDamageRegion(&spawnPosition, 64.0f, 8.533333778381348f, 400, 15);
            slot = workItem->damageRegion;
            slot->collisionInterval = 2;
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->position.x);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, color, 0, 21);
        }
    }

    workItem = &bomb->workItems[16];
    for (i = 16; i < ARRAY_SIZE(bomb->workItems); i++, workItem++)
    {
        if (!workItem->state)
            continue;
        workItem->cancelRegion->center.x = workItem->position.x;
        workItem->cancelRegion->center.y = workItem->position.y;
        workItem->damageRegion->center.x = workItem->position.x;
        workItem->damageRegion->center.y = workItem->position.y;
        if (g_AnmManager->ExecuteScript(&workItem->vms[0]))
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
    }
}

// FUNCTION: th08 0x40d010
#pragma var_order(vm, i, workItem)
void __fastcall DrawFantasySealBlinkDeathbomb(Player *player)
{
    PlayerBombWorkItem *workItem;
    u32 i;
    AnmVm *vm;

    SetBombBackgroundTint(player, 0x802020d0);
    i = 0;
    workItem = player->bombState.workItems;
    for (; i < 128; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;
        vm = &workItem->vms[0];
        vm->pos = workItem->position + vm->pos2;
        vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        vm->pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
    }
}

// FUNCTION: th08 0x40d100
#pragma var_order(bomb, effect, i)
void __fastcall UpdateDissolveSpell(Player *player)
{
    PlayerBombState *bomb;
    AnmVm *effect;
    u32 i;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        if (!g_GameManager.IsSpellPractice())
            BeginBombSpell(player, -1,
                         "\x81\x75\x83\x66\x83\x42\x83\x5D\x83\x8B\x83\x75\x83\x58\x83\x79\x83\x8B\x81\x76",
                         120, 200, 0);
        else
            BeginBombSpell(player, -1,
                         "\x81\x75\x83\x66\x83\x42\x83\x5D\x83\x8B\x83\x75\x83\x58\x83\x79\x83\x8B\x81\x76",
                         40, 200, 0);

        g_EffectManager.SpawnEffect(12, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xff4040ff);
        effect = g_EffectManager.SpawnEffectInFixedSlot(50, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 4, 1,
                                              0xff4040ff);
        effect->interpCurrentTimers[AnmInterp_Pos] = 0;
        if (!g_GameManager.IsSpellPractice())
            effect->interpEndTimers[AnmInterp_Pos] = 90;
        else
            effect->interpEndTimers[AnmInterp_Pos] = 30;
        effect->interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOutCubic;
        effect->posInitial.x = 8.0f;
        effect->posFinal.x = 128.0f;
        effect->posInitial.y = 64.0f;
        effect->posFinal.y = 0.0f;
        effect->pos.x = 8.0f;
        effect->pos.y = 64.0f;
        reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 64;
        reinterpret_cast<Effect *>(effect)->angle = 0.0f;
        reinterpret_cast<Effect *>(effect)->radius = 8.0f;
        reinterpret_cast<Effect *>(effect)->shapeThickness = 15.0f;
        reinterpret_cast<Effect *>(effect)->radialWaveCount = 6.0f;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        player->verticalSpeedMultiplier = 0.0f;
        player->horizontalSpeedMultiplier = 0.0f;

        for (i = 0; i < 8; i++)
        {
            if (g_EnemyManager.bosses[i] != NULL)
            {
                g_EnemyManager.bosses[i]->flags1 &= ~ENEMY_FLAG_ACCEPTS_DAMAGE;
            }
        }
    }
}

// FUNCTION: th08 0x40d310
#pragma var_order(bomb, colorValue)
void __fastcall DrawDissolveSpell(Player *player)
{
    PlayerBombState *bomb = &player->bombState;
    if (bomb->timer < 60)
    {
        SetBombBackgroundTint(player, 0x80404040);
    }
    else
    {
        i32 colorValue = ((i32)bomb->timer - 60) * 176 / 60 + 64;
        SetBombBackgroundTint(player, 0x80000000 | (colorValue << 16) | (colorValue << 8) | colorValue);
    }
}
// FUNCTION: th08 0x40d3d0
ZunBool ZunTimer::HasTicked()
{
    return this->current != this->previous;
}

// FUNCTION: th08 0x40d410
i32 ZunTimer::operator%(i32 value)
{
    return this->current % value;
}
// FUNCTION: th08 0x40d430
#pragma var_order(bomb, workItem)
void __fastcall UpdateArtfulSacrificeBomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 1,
                     "\x96\x82\x95\x84\x81\x75\x83\x41\x81\x5B\x83\x65\x83\x42\x83\x74\x83\x8B\x83\x54\x83\x4E\x83\x8A\x83\x74\x83\x40\x83\x43\x83\x58\x81\x76",
                     210, 250, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->position = player->optionStates[0].position;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
    }

    if (bomb->timer < 60)
    {
        Float3 target(192.0f, 224.0f, 0.0f);
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        player->optionStates[0].position =
            (target - workItem->position) * interp + workItem->position;
        player->optionStates[0].vm.rotation.z += -0.31415927410125732f;
        player->CreateCircleCancelRegion(&player->optionStates[0].position, 32.0f, 0.0f, 0, 6);
        player->CreateCircleDamageRegion(&player->optionStates[0].position, 32.0f, 0.0f, 40, 0);
    }
    else
    {
        player->optionStates[0].vm.rotation.z = 0.0f;
        player->optionStates[0].position.x = 192.0f;
        player->optionStates[0].position.y = 224.0f;
        if (bomb->timer >= 150)
            player->optionStates[0].vm.color1.a = 0;

        if (bomb->timer.JustReached(60))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.JustReached(64))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.JustReached(68))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -20304);
        }
        else if (bomb->timer.JustReached(72))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.JustReached(76))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -49088);
        }
        else if (bomb->timer.JustReached(90))
        {
#pragma var_order(effect, damageSlot)
            AnmVm *effect;
            PlayerCollisionRegion *damageSlot;
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(15), 0);
            effect = g_EffectManager.SpawnEffect(42, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(43, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(44, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            player->CreateCircleCancelRegion(&player->optionStates[0].position, 1.0f, 5.0f, 110, 6);
            damageSlot = player->CreateCircleDamageRegion(&player->optionStates[0].position, 1.0f, 5.0f, 70, 110);
            damageSlot->collisionInterval = 5;
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 24, 8, 0, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x8fffffff, 0, 21);
        }
        else if (bomb->timer.JustReached(100))
        {
            AnmVm *effect100 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.JustReached(110))
        {
            AnmVm *effect110 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.JustReached(120))
        {
            AnmVm *effect120 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.JustReached(130))
        {
            AnmVm *effect130 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -65536);
        }
        else if (bomb->timer.JustReached(150))
        {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x8fffffff, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 24, 8, 0, 0, 21);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(25), 0);
        }
        else if (bomb->timer.JustReached(209))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40d950
void __fastcall DrawDefaultBombTint(Player *player)
{
    SetBombBackgroundTint(player, 0x80404040);
}

// FUNCTION: th08 0x40d970
#pragma var_order(bomb, workItem)
void __fastcall UpdateReturnInanimatenessDeathbomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 1,
                     "\x96\x82\x91\x80\x81\x75\x83\x8A\x83\x5E\x81\x5B\x83\x93\x83\x43\x83\x69\x83\x6A\x83\x81\x83\x67\x83\x6C\x83\x58\x81\x76",
                     230, 280, 1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->position = player->position;
    }

    if (bomb->timer < 60)
    {
        Float3 target(192.0f, 224.0f, 0.0f);
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        player->optionStates[0].position =
            (target - workItem->position) * interp + workItem->position;
        player->optionStates[0].vm.rotation.z += -0.31415927410125732f;
        player->CreateCircleCancelRegion(&player->optionStates[0].position, 32.0f, 0.0f, 0, 6);
        player->CreateCircleDamageRegion(&player->optionStates[0].position, 32.0f, 0.0f, 40, 0);
    }
    else
    {
        player->optionStates[0].vm.rotation.z = 0.0f;
        player->optionStates[0].position.x = 192.0f;
        player->optionStates[0].position.y = 224.0f;
        if (bomb->timer >= 128)
            player->optionStates[0].vm.color1.a = 0;

        if (bomb->timer.JustReached(60))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.JustReached(64))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.JustReached(68))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -20304);
        }
        else if (bomb->timer.JustReached(72))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.JustReached(76))
        {
            g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -49088);
        }
        else if (bomb->timer.JustReached(120))
        {
#pragma var_order(effect, damageSlot, burstPosition)
            AnmVm *effect;
            PlayerCollisionRegion *damageSlot;
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(15), 0);
            effect = g_EffectManager.SpawnEffect(42, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(43, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            effect = g_EffectManager.SpawnEffect(44, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
            Float3 burstPosition(64.0f, 96.0f, 0.0f);
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xff0000f0);
            burstPosition.y = 352.0f;
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xfff00000);
            burstPosition.x = 320.0f;
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xff00f000);
            burstPosition.y = 96.0f;
            effect = g_EffectManager.SpawnEffect(45, reinterpret_cast<D3DXVECTOR3 *>(&burstPosition), 1, 0xff00f0f0);
            player->CreateCircleCancelRegion(&player->optionStates[0].position, 1.0f, 5.0f, 110, 6);
            damageSlot = player->CreateCircleDamageRegion(&player->optionStates[0].position, 1.0f, 5.0f, 70, 110);
            damageSlot->collisionInterval = 5;
        }
        else if (bomb->timer.JustReached(130))
        {
            AnmVm *effect130 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -1);
        }
        else if (bomb->timer.JustReached(140))
        {
            AnmVm *effect140 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -12080);
        }
        else if (bomb->timer.JustReached(150))
        {
            AnmVm *effect150 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -32640);
        }
        else if (bomb->timer.JustReached(160))
        {
            AnmVm *effect160 = g_EffectManager.SpawnEffect(
                45, reinterpret_cast<D3DXVECTOR3 *>(&player->optionStates[0].position), 1, -65536);
        }
        else if (bomb->timer.JustReached(180))
        {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, -1, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 24, 8, 0, 0, 21);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(25), 0);
        }
        else if (bomb->timer.JustReached(229))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40dee0
#pragma var_order(bomb, workItem, rect, fadeValue, color, color2, rect2)
void __fastcall DrawReturnInanimatenessDeathbomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    ZunRect rect;
    i32 fadeValue;
    ZunColor color;
    ZunColor color2;
    ZunRect rect2;

    bomb = &player->bombState;
    workItem = player->bombState.workItems;
    if (bomb->timer < 90)
    {
        SetBombBackgroundTint(player, 0x802020d0);
        return;
    }
    if (bomb->timer <= 120)
    {
        fadeValue = 208 * ((i32)bomb->timer - 90) / 30;
        color.r = fadeValue / 5 + 0xd0;
        color.g = fadeValue + 0x20;
        color.b = fadeValue + 0x20;
        color.a = 0x80;
        SetBombBackgroundTint(player, color.d3dColor);

        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        color.a = 255 * ((i32)bomb->timer - 90) / 30;
        ScreenEffect::DrawSquare(&rect, color.d3dColor);
        return;
    }
    if (bomb->timer <= 220)
    {
        rect2.left = 32.0f;
        rect2.top = 16.0f;
        rect2.right = 416.0f;
        rect2.bottom = 464.0f;
        color2.r = 0xff;
        color2.g = 0xff;
        color2.b = 0xff;
        color2.a = 0x70;
        ScreenEffect::DrawSquare(&rect2, color2.d3dColor);
        return;
    }
    SetBombBackgroundTint(player, 0x802020d0);
}

// FUNCTION: th08 0x40e040
i32 __fastcall FUN_0040e040(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)reinterpret_cast<Effect *>(effect)->timer / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    reinterpret_cast<Effect *>(effect)->radius = 256.0f * interp;
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 64;
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    reinterpret_cast<Effect *>(effect)->radialWaveCount = 5.0f;
    reinterpret_cast<Effect *>(effect)->secondaryAngle = 0.0f;
    if (reinterpret_cast<Effect *>(effect)->timer < 40)
    {
        reinterpret_cast<Effect *>(effect)->shapeThickness = 8.0f;
    }
    else
    {
        reinterpret_cast<Effect *>(effect)->secondaryRadius = 64.0f * interp;
        reinterpret_cast<Effect *>(effect)->shapeThickness += 2.0f;
    }
    return 1;
}

// FUNCTION: th08 0x40e120
i32 __fastcall FUN_0040e120(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)reinterpret_cast<Effect *>(effect)->timer / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    reinterpret_cast<Effect *>(effect)->radius = 256.0f * interp;
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 48;
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    reinterpret_cast<Effect *>(effect)->radialWaveCount = 0.0f;
    reinterpret_cast<Effect *>(effect)->secondaryRadius = 128.0f * interp;
    reinterpret_cast<Effect *>(effect)->secondaryAngle = ZUN_PI / 4.0f;
    if (reinterpret_cast<Effect *>(effect)->timer < 40)
        reinterpret_cast<Effect *>(effect)->shapeThickness = 8.0f;
    else
        reinterpret_cast<Effect *>(effect)->shapeThickness += 1.5f;
    return 1;
}

// FUNCTION: th08 0x40e200
i32 __fastcall FUN_0040e200(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)reinterpret_cast<Effect *>(effect)->timer / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    reinterpret_cast<Effect *>(effect)->radius = 256.0f * interp;
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 48;
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    reinterpret_cast<Effect *>(effect)->secondaryRadius = 128.0f * interp;
    reinterpret_cast<Effect *>(effect)->secondaryAngle = -ZUN_PI / 4.0f;
    if (reinterpret_cast<Effect *>(effect)->timer < 40)
        reinterpret_cast<Effect *>(effect)->shapeThickness = 8.0f;
    else
        reinterpret_cast<Effect *>(effect)->shapeThickness += 1.5f;
    return 1;
}

// FUNCTION: th08 0x40e2d0
i32 __fastcall FUN_0040e2d0(AnmVm *effect)
{
    f32 interp = 1.0f - (f32)reinterpret_cast<Effect *>(effect)->timer / 40.0f;
    interp *= interp;
    interp = 1.0f - interp;
    reinterpret_cast<Effect *>(effect)->radius = 192.0f * interp;
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 8;
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    reinterpret_cast<Effect *>(effect)->shapeThickness = 8.0f;
    return 1;
}

// FUNCTION: th08 0x40e3b0
#pragma var_order(bomb, workItem, angle, slot, position)
void __fastcall UpdateMasterSparkBomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;
    PlayerCollisionRegion *slot;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x97\xF6\x95\x84\x81\x75\x83\x7D\x83\x58\x83\x5E\x81\x5B\x83\x58\x83\x70\x81\x5B\x83\x4E\x81\x76",
                     300, 350, 0);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(19), 0);
        workItem->position = player->position;
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[0], 30);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[1], 31);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[2], 32);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[3], 33);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[4], 34);
        player->verticalSpeedMultiplier = 0.2f;
        player->horizontalSpeedMultiplier = 0.2f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE_ENVELOPE, 16, 120, 60, 120, 21);
    }

    if (player->bombState.timer.HasTicked() && ((i32)player->bombState.timer % 4) != 0)
    {
        Float3 position;
        position = player->position;
        position.x = 192.0f;
        position.y /= 2.0f;
        player->CreateRectCancelRegion(&position, 384.0f, position.y * 2.0f, 6, 0);

        position = player->position;
        position.y /= 2.0f;
        slot = player->CreateRectDamageRegion(&position, 128.0f, position.y * 2.0f, 12, 0);
        slot->mode = 1;
        position.x = 192.0f;
        slot = player->CreateRectDamageRegion(&position, 384.0f, position.y * 2.0f, 6, 0);
        slot->mode = 1;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 5);
}

// FUNCTION: th08 0x40e610
#pragma var_order(vm, i, motionStep, angle)
void __fastcall DrawMasterSparkBomb(Player *player)
{
    AnmVm *vm;
    i32 i;
    f32 motionStep;
    f32 angle;

    SetBombBackgroundTint(player, 0x80404040);
    motionStep = ZUN_PI / 15.0f;
    vm = &player->bombState.workItems[0].vms[0];
    for (i = 0; i < 5; i++, vm++)
    {
        angle = (f32)i * motionStep - ZUN_PI / 2.0f - motionStep * 2.0f;
        if (angle < -ZUN_PI)
            angle += ZUN_2PI;
        vm->pos = player->position;
        vm->pos.x += cosf(angle) * vm->loadedSprite->widthPx * vm->scale.x / 2.0f;
        vm->pos.y += sinf(angle) * vm->loadedSprite->widthPx * vm->scale.x / 2.0f;
        vm->SetZRotation(angle);
        vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        vm->pos.z = 0.0f;
        g_AnmManager->Draw2D(vm);
    }
}

// FUNCTION: th08 0x40e780
#pragma var_order(bomb, workItem, angle)
void __fastcall UpdateFinalSparkDeathbomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x96\x82\x96\x43\x81\x75\x83\x74\x83\x40\x83\x43\x83\x69\x83\x8B\x83\x58\x83\x70\x81\x5B\x83\x4E\x81\x76",
                     350, 380, 1);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(19), 0);
        workItem->position = player->position;
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[0], 35);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[1], 36);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[2], 37);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[3], 38);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[4], 39);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE_ENVELOPE, 16, 120, 60, 120, 21);
        player->verticalSpeedMultiplier = 0.2f;
        player->horizontalSpeedMultiplier = 0.2f;
        bomb->secondaryWorkCursor = 0;
    }

    if (bomb->timer.IsPeriodic(10))
    {
#pragma var_order(effect, position1, position0, scale1, scale0)
        AnmVm *effect = g_EffectManager.SpawnEffectInFixedSlot(
            53, reinterpret_cast<D3DXVECTOR3 *>(&player->position), bomb->secondaryWorkCursor % 4 + 4, 1, -1);
        if (bomb->secondaryWorkCursor & 1)
            g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 92);
        reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 32;
        reinterpret_cast<Effect *>(effect)->radialWaveCount = 0;

        Float3 position0;
        Float3 position1;
        position0.x = 0.0f;
        position0.y = 0.0f;
        position0.z = 0.0f;
        position1.x = 128.0f;
        position1.y = 0.0f;
        position1.z = 0.0f;
        effect->StartPositionInterpolation(30, AnmInterpMode_EaseOut, &position0, &position1);

        Float2 scale0;
        Float2 scale1;
        scale0.x = 32.0f;
        scale0.y = 0.0f;
        scale1.x = 64.0f;
        scale1.y = 0.0f;
        effect->StartScaleInterpolation(30, AnmInterpMode_EaseIn, &scale0, &scale1);
        effect->StartColor1AlphaInterpolation(30, AnmInterpMode_EaseInQuartic, 255, 0);
        effect->StartColor1RgbInterpolation(30, AnmInterpMode_Linear, -1, 0xffff0000);
        g_AnmManager->ExecuteScript(effect);
        bomb->secondaryWorkCursor++;
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(17), player->position.x);
    }

    if (player->bombState.timer.HasTicked() && ((i32)player->bombState.timer % 4) != 0)
    {
#pragma var_order(slot, position)
        Float3 position;
        PlayerCollisionRegion *slot;
        position = player->position;
        position.x = 192.0f;
        position.y /= 2.0f;
        player->CreateRectCancelRegion(&position, 384.0f, position.y * 2.0f, 6, 0);

        position = player->position;
        position.y /= 2.0f;
        slot = player->CreateRectDamageRegion(&position, 128.0f, position.y * 2.0f, 12, 0);
        slot->mode = 1;
        position.x = 192.0f;
        slot = player->CreateRectDamageRegion(&position, 384.0f, position.y * 2.0f, 7, 0);
        slot->mode = 1;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 5);
}

// FUNCTION: th08 0x40ee10
#pragma var_order(bomb, workItem)
void __fastcall UpdateRedNightlessCastleBomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 1,
                     "\x8D\x67\x95\x84\x81\x75\x95\x73\x96\xE9\x8F\xE9\x83\x8C\x83\x62\x83\x68\x81\x76",
                     240, 290, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem[0].position = player->optionStates[0].target;
        workItem[1].position = player->optionStates[1].target;
        workItem[2].position = player->optionStates[2].target;
        workItem[3].position = player->optionStates[3].target;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        bomb->secondaryWorkCursor = 0;
        player->verticalSpeedMultiplier = 0.0f;
        player->horizontalSpeedMultiplier = 0.0f;
    }

    if (bomb->timer < 60)
    {
#pragma var_order(interp, position)
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        Float3 position;
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target =
            (position - workItem[0].position) * interp + workItem[0].position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target =
            (position - workItem[1].position) * interp + workItem[1].position;
        position.y += 64.0f;
        player->optionStates[2].target =
            (position - workItem[2].position) * interp + workItem[2].position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target =
            (position - workItem[3].position) * interp + workItem[3].position;
    }
    else
    {
        if (bomb->timer.JustReached(60))
        {
            player->verticalSpeedMultiplier = 2.0f;
            player->horizontalSpeedMultiplier = 2.0f;
            player->optionStates[0].vm.SetInterrupt(2);
            player->optionStates[1].vm.SetInterrupt(2);
            player->optionStates[2].vm.SetInterrupt(2);
            player->optionStates[3].vm.SetInterrupt(2);
        }

        Float3 position;
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target = position;
        position.y += 64.0f;
        player->optionStates[2].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target = position;

        workItem->cancelRegion = player->CreateCircleCancelRegion(&player->position, 96.0f, 0.0f, 0, 6);

        if (bomb->timer.IsPeriodic(10))
        {
#pragma var_order(effect, position1, position0, scale1, scale0)
            AnmVm *effect = g_EffectManager.SpawnEffectInFixedSlot(
                53, reinterpret_cast<D3DXVECTOR3 *>(&player->position), bomb->secondaryWorkCursor % 4 + 4, 1, -1);
            reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 32;
            reinterpret_cast<Effect *>(effect)->radialWaveCount = 4.0f;

            Float3 position0;
            Float3 position1;
            position0.x = 0.0f;
            position0.y = 0.0f;
            position0.z = 0.0f;
            position1.x = 192.0f;
            position1.y = g_Rng.GetRandomF32InRange(128.0f);
            position1.z = 0.0f;
            effect->StartPositionInterpolation(30, AnmInterpMode_EaseOut, &position0, &position1);

            Float2 scale0;
            Float2 scale1;
            scale0.x = 64.0f;
            scale0.y = 0.0f;
            scale1.x = 64.0f;
            scale1.y = 0.0f;
            effect->StartScaleInterpolation(30, AnmInterpMode_EaseIn, &scale0, &scale1);
            effect->StartColor1AlphaInterpolation(30, AnmInterpMode_EaseInQuartic, 255, 0);
            effect->StartColor1RgbInterpolation(30, AnmInterpMode_Linear, -1, 0xffff0000);
            bomb->secondaryWorkCursor++;
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(17), player->position.x);
            g_AnmManager->ExecuteScript(effect);
        }

        if (player->shotTimer >= 5)
        {
            workItem->cancelRegion = player->CreateRectCancelRegion(&player->position, 96.0f, 800.0f, 6, 0);
            workItem->cancelRegion = player->CreateRectCancelRegion(&player->position, 800.0f, 96.0f, 6, 0);
        }

        if (bomb->timer.JustReached(239))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
            player->optionStates[1].state2C8 = 1;
            player->optionStates[1].timer = 0;
            player->optionStates[2].state2C8 = 1;
            player->optionStates[2].timer = 0;
            player->optionStates[3].state2C8 = 1;
            player->optionStates[3].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40f550
void __fastcall DrawRedNightlessCastleBomb(Player *player)
{
    SetBombBackgroundTint(player, 0x80d02020);
}

// FUNCTION: th08 0x40f570
#pragma var_order(bomb, workItem)
void __fastcall UpdateScarletDevilDeathbomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    workItem = &bomb->workItems[0];
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 1,
                     "\x8D\x67\x96\x82\x81\x75\x83\x58\x83\x4A\x81\x5B\x83\x8C\x83\x62\x83\x67\x83\x66\x83\x72\x83\x8B\x81\x76",
                     280, 320, 1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem[0].position = player->optionStates[0].target;
        workItem[1].position = player->optionStates[1].target;
        workItem[2].position = player->optionStates[2].target;
        workItem[3].position = player->optionStates[3].target;
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        bomb->secondaryWorkCursor = 0;
        player->verticalSpeedMultiplier = 0.0f;
        player->horizontalSpeedMultiplier = 0.0f;
    }

    if (bomb->timer < 60)
    {
#pragma var_order(interp, position)
        f32 interp = (f32)bomb->timer / 60.0f;
        interp *= interp;
        Float3 position;
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target =
            (position - workItem[0].position) * interp + workItem[0].position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target =
            (position - workItem[1].position) * interp + workItem[1].position;
        position.y += 64.0f;
        player->optionStates[2].target =
            (position - workItem[2].position) * interp + workItem[2].position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target =
            (position - workItem[3].position) * interp + workItem[3].position;
    }
    else
    {
        if (bomb->timer.JustReached(60))
        {
            player->verticalSpeedMultiplier = 3.0f;
            player->horizontalSpeedMultiplier = 3.0f;
            player->optionStates[0].vm.SetInterrupt(2);
            player->optionStates[1].vm.SetInterrupt(2);
            player->optionStates[2].vm.SetInterrupt(2);
            player->optionStates[3].vm.SetInterrupt(2);
        }

        Float3 position;
        workItem->cancelRegion = player->CreateCircleCancelRegion(&player->position, 96.0f, 0.0f, 0, 6);
        position = player->position;
        position.x -= 32.0f;
        player->optionStates[0].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[1].target = position;
        position.y += 64.0f;
        player->optionStates[2].target = position;
        position.x += 32.0f;
        position.y -= 32.0f;
        player->optionStates[3].target = position;

        if (bomb->timer.IsPeriodic(10))
        {
#pragma var_order(effect, position1, position0, scale1, scale0)
            AnmVm *effect = g_EffectManager.SpawnEffectInFixedSlot(
                53, reinterpret_cast<D3DXVECTOR3 *>(&player->position), bomb->secondaryWorkCursor % 4 + 4, 1, -1);
            reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 32;
            reinterpret_cast<Effect *>(effect)->radialWaveCount = 4.0f;

            Float3 position0;
            Float3 position1;
            position0.x = 0.0f;
            position0.y = 0.0f;
            position0.z = 0.0f;
            position1.x = 192.0f;
            position1.y = g_Rng.GetRandomF32InRange(128.0f);
            position1.z = 0.0f;
            effect->StartPositionInterpolation(30, AnmInterpMode_EaseOut, &position0, &position1);

            Float2 scale0;
            Float2 scale1;
            scale0.x = 64.0f;
            scale0.y = 0.0f;
            scale1.x = 128.0f;
            scale1.y = 0.0f;
            effect->StartScaleInterpolation(30, AnmInterpMode_EaseIn, &scale0, &scale1);
            effect->StartColor1AlphaInterpolation(30, AnmInterpMode_EaseInQuartic, 255, 0);
            effect->StartColor1RgbInterpolation(30, AnmInterpMode_Linear, -1, 0xffff0000);
            bomb->secondaryWorkCursor++;
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(17), player->position.x);
            g_AnmManager->ExecuteScript(effect);
        }

        if (player->shotTimer >= 5)
        {
            workItem->cancelRegion = player->CreateRectCancelRegion(&player->position, 96.0f, 800.0f, 6, 0);
            workItem->cancelRegion = player->CreateRectCancelRegion(&player->position, 800.0f, 96.0f, 6, 0);
        }

        if (bomb->timer.JustReached(279))
        {
            player->optionStates[0].state2C8 = 1;
            player->optionStates[0].timer = 0;
            player->optionStates[1].state2C8 = 1;
            player->optionStates[1].timer = 0;
            player->optionStates[2].state2C8 = 1;
            player->optionStates[2].timer = 0;
            player->optionStates[3].state2C8 = 1;
            player->optionStates[3].timer = 0;
        }
    }
}

// FUNCTION: th08 0x40fcb0
void __fastcall DrawScarletDevilDeathbomb(Player *player)
{
    SetBombBackgroundTint(player, 0x80f00000);
}

// FUNCTION: th08 0x40fcd0
#pragma var_order(i, bomb, workItem, vm, angle)
void __fastcall UpdateKillingDollBomb(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    AnmVm *vm;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x8C\xB6\x95\x84\x81\x75\x8E\x45\x90\x6C\x83\x68\x81\x5B\x83\x8B\x81\x76",
                     250, 290, 0);
        workItem = bomb->workItems;
        for (i = 0; i < 96; i++, workItem++)
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
        player->verticalSpeedMultiplier = 0.5f;
        player->horizontalSpeedMultiplier = 0.5f;
        bomb->workItems[0].effectVm =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
    }

    if (bomb->timer >= 0 && bomb->timer <= 60)
    {
        reinterpret_cast<Effect *>(bomb->workItems[0].effectVm)->position = player->position;
    }

    if (bomb->timer >= 20 && bomb->timer < 116)
    {
        workItem = player->bombState.workItems;
        for (i = 0; i < 96; i++, workItem++)
        {
            if (!player->bombState.timer.JustReached(2 * (i % 48) + 20))
                continue;
            if (workItem->state)
                return;
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
                vm = &workItem->vms[0];
                player->anmFile->ExecuteAnmIdx(vm, 22);
                angle = (f32)i * ZUN_2PI / 96.0f - ZUN_PI;
                workItem->angle = angle;
                workItem->speed = g_Rng.GetRandomF32InRange(1.0f) + 0.5f;
                workItem->motionStep = g_Rng.GetRandomF32InRange(0.1f) + 0.03f;
                workItem->auxiliaryMotion.x = g_Rng.GetRandomU16InRange(1)
                    ? 0.15707963705062866f : -0.15707963705062866f;
                workItem->motion.x = cosf(workItem->angle) * 24.0f;
                workItem->motion.y = sinf(workItem->angle) * 24.0f;
                workItem->position = player->position + workItem->motion;
                workItem->timer = 0;
                workItem->motion.z = 0.0f;
                workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 32.0f, 0.0f, 500, 6);
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 32.0f, 0.0f, 20, 500);
        }
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 120, 4, 1, 0, 21);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (!workItem->state)
            continue;

        if ((i32)workItem->timer < 30 || (i32)workItem->timer >= 70)
        {
            if (workItem->timer.JustReached(70))
            {
                if (player->tailPosition0.x > -100.0f)
                {
                    workItem->angle = AddNormalizeAngle(
                        VectorAngle(player->tailPosition0.y - workItem->position.y,
                                    player->tailPosition0.x - workItem->position.x),
                        0.0f);
                }
                workItem->speed = 14.0f;
            }
            workItem->speed += workItem->motionStep;
            workItem->motion.x = cosf(workItem->angle) * workItem->speed;
            workItem->motion.y = sinf(workItem->angle) * workItem->speed;
        }
        else
        {
            workItem->angle = AddNormalizeAngle(workItem->angle, workItem->auxiliaryMotion.x);
            workItem->motion.x = 0.0f;
            workItem->motion.y = 0.0f;
        }

        if (workItem->damageRegion != NULL)
        {
            workItem->damageRegion->center.x = workItem->position.x;
            workItem->damageRegion->center.y = workItem->position.y;
            workItem->cancelRegion->center.x = workItem->position.x;
            workItem->cancelRegion->center.y = workItem->position.y;
            if (workItem->timer >= 120)
            {
                workItem->damageRegion->active = 0;
                workItem->cancelRegion->active = 0;
                workItem->cancelRegion = NULL;
                workItem->damageRegion = NULL;
            }
            else if (workItem->damageRegion->hitAccumulator > 0)
            {
                player->anmFile->ExecuteAnmIdx(&workItem->vms[0], 23);
                g_EffectManager.SpawnEffect(
                    0, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[i].position), 1, 0xffff80ff);
                workItem->damageRegion->active = 0;
                workItem->cancelRegion->active = 0;
                workItem->cancelRegion = NULL;
                workItem->damageRegion = NULL;
                g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(43), workItem->position.x);
            }
        }

        workItem->position += workItem->motion;
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
        workItem->timer++;
    }
}

// FUNCTION: th08 0x4103f0
#pragma var_order(i, bomb, workItem, vm, angle)
void __fastcall UpdateNightMistPhantomKillerDeathbomb(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    AnmVm *vm;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x8C\xB6\x91\x92\x81\x75\x96\xE9\x96\xB6\x82\xCC\x8C\xB6\x89\x65\x8E\x45\x90\x6C\x8B\x53\x81\x76",
                     320, 350, 1);
        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
        player->verticalSpeedMultiplier = 0.5f;
        player->horizontalSpeedMultiplier = 0.5f;
        bomb->workItems[0].effectVm =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 50, 4, 1, 0, 21);
    }

    if (bomb->timer >= 0 && bomb->timer <= 60)
    {
        reinterpret_cast<Effect *>(bomb->workItems[0].effectVm)->position = player->position;
    }

    if (bomb->timer >= 20 && bomb->timer < 148)
    {
        workItem = player->bombState.workItems;
        for (i = 0; i < 128; i++, workItem++)
        {
            if (!player->bombState.timer.JustReached(2 * (i % 64) + 20))
                continue;
            if (workItem->state)
                return;
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            vm = &workItem->vms[0];
            player->anmFile->ExecuteAnmIdx(vm, 20);
            angle = AddNormalizeAngle((f32)i * ZUN_2PI / 64.0f - ZUN_PI, 0.0f);
            workItem->angle = angle;
            workItem->speed = g_Rng.GetRandomF32InRange(1.0f) + 0.5f;
            workItem->motionStep = g_Rng.GetRandomF32InRange(0.1f) + 0.03f;
            workItem->auxiliaryMotion.x = g_Rng.GetRandomU16InRange(1)
                ? 0.15707963705062866f : -0.15707963705062866f;
            workItem->motion.x = cosf(workItem->angle) * 24.0f;
            workItem->motion.y = sinf(workItem->angle) * 24.0f;
            workItem->position = player->position + workItem->motion;
            workItem->timer = 0;
            workItem->motion.z = 0.0f;
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 32.0f, 0.0f, 500, 6);
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 32.0f, 0.0f, 30, 500);
        }
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (!workItem->state)
            continue;

        if ((i32)workItem->timer < 30 || (i32)workItem->timer >= 70)
        {
            if (workItem->timer.JustReached(70))
            {
                if (player->tailPosition0.x > -100.0f)
                {
                    workItem->angle = AddNormalizeAngle(
                        VectorAngle(player->tailPosition0.y - workItem->position.y,
                                    player->tailPosition0.x - workItem->position.x),
                        0.0f);
                }
                workItem->speed = 14.0f;
                g_EffectManager.SpawnEffect(
                    46, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position), 1, -1);
            }
            workItem->speed += workItem->motionStep;
            workItem->motion.x = cosf(workItem->angle) * workItem->speed;
            workItem->motion.y = sinf(workItem->angle) * workItem->speed;
        }
        else
        {
            workItem->angle = AddNormalizeAngle(workItem->angle, workItem->auxiliaryMotion.x);
            workItem->motion.x = 0.0f;
            workItem->motion.y = 0.0f;
        }

        if (workItem->damageRegion != NULL)
        {
            workItem->damageRegion->center.x = workItem->position.x;
            workItem->damageRegion->center.y = workItem->position.y;
            workItem->cancelRegion->center.x = workItem->position.x;
            workItem->cancelRegion->center.y = workItem->position.y;
            if (workItem->timer >= 120)
            {
                workItem->damageRegion->active = 0;
                workItem->cancelRegion->active = 0;
                workItem->cancelRegion = NULL;
                workItem->damageRegion = NULL;
            }
            else if (workItem->damageRegion->hitAccumulator > 0)
            {
                if (i % 3 == 0)
                    ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 2, 1, 0x208080ff, 0, 21);
                g_EffectManager.SpawnEffect(
                    0, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position), 1, 0xffff80ff);
                player->anmFile->ExecuteAnmIdx(&workItem->vms[0], 21);
                workItem->damageRegion->active = 0;
                workItem->cancelRegion->active = 0;
                workItem->cancelRegion = NULL;
                workItem->damageRegion = NULL;
                g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(43), workItem->position.x);
            }
        }

        workItem->position += workItem->motion;
        g_AnmManager->ExecuteScript(&workItem->vms[0]);
        workItem->timer++;
    }
}

// FUNCTION: th08 0x410300
#pragma var_order(i, workItem)
void __fastcall DrawKillingDollBomb(Player *player)
{
    PlayerBombWorkItem *workItem;
    i32 i;

    SetBombBackgroundTint(player, 0x80404040);
    workItem = player->bombState.workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;
        workItem->vms[0].SetZRotation(workItem->angle);
        workItem->vms[0].pos = workItem->position;
        workItem->vms[0].pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        workItem->vms[0].pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x410ac0
#pragma var_order(i, workItem)
void __fastcall DrawNightMistPhantomKillerDeathbomb(Player *player)
{
    PlayerBombWorkItem *workItem;
    i32 i;

    SetBombBackgroundTint(player, 0x80202080);
    workItem = player->bombState.workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;
        workItem->vms[0].SetZRotation(workItem->angle);
        workItem->vms[0].pos = workItem->position;
        workItem->vms[0].pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        workItem->vms[0].pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x410c40
#pragma var_order(bomb, workItem, angle)
void __fastcall UpdateQuadrupleBarrierBomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        PlayerCollisionRegion *slot;

        BeginBombSpell(player, 1,
                     "\x8B\xAB\x95\x84\x81\x75\x8E\x6C\x8F\x64\x8C\x8B\x8A\x45\x81\x76",
                     150, 200, 0);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->position = player->position;
        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(ZUN_PI / 4.0f, 1.0f, 4.0f);
        g_EffectManager.SpawnEffectInFixedSlotWithVelocity(36, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position),
                                     reinterpret_cast<D3DXVECTOR3 *>(&velocity), 4, 1, -1);
    }

    if (bomb->timer.JustReached(10))
    {
        PlayerCollisionRegion *slot;
        AnmVm *effect;

        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(ZUN_PI * 3.0f / 8.0f, 1.0f, 4.0f);
        effect = g_EffectManager.SpawnEffectInFixedSlotWithVelocity(36, reinterpret_cast<D3DXVECTOR3 *>(&player->position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 5, 1, -1);
        g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 89);
        bomb->workItems[1].position = player->position;
    }

    if (bomb->timer.JustReached(20))
    {
        PlayerCollisionRegion *slot;
        AnmVm *effect;

        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(ZUN_PI / 2.0f, 1.0f, 4.0f);
        effect = g_EffectManager.SpawnEffectInFixedSlotWithVelocity(36, reinterpret_cast<D3DXVECTOR3 *>(&player->position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 6, 1, -1);
        g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 90);
        bomb->workItems[2].position = player->position;
    }

    if (bomb->timer.JustReached(30))
    {
        PlayerCollisionRegion *slot;
        AnmVm *effect;

        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(1.9634954929351807f, 1.0f, 4.0f);
        effect = g_EffectManager.SpawnEffectInFixedSlotWithVelocity(36, reinterpret_cast<D3DXVECTOR3 *>(&player->position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 7, 1, -1);
        g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 91);
        bomb->workItems[3].position = player->position;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 2);
}

// FUNCTION: th08 0x4114e0
#pragma var_order(interp, i, slot, angle)
i32 __fastcall FUN_004114e0(AnmVm *effect)
{
    f32 interp;
    i32 i;
    PlayerCollisionRegion *slot;
    f32 angle;

    if (reinterpret_cast<Effect *>(effect)->timer < 40)
    {
        interp = 1.0f - (f32)reinterpret_cast<Effect *>(effect)->timer / 40.0f;
        reinterpret_cast<Effect *>(effect)->shapeThickness =
            88.0f - (f32)reinterpret_cast<Effect *>(effect)->timer * 80.0f / 40.0f;
        reinterpret_cast<Effect *>(effect)->radius =
            192.0f - 384.0f * interp * interp;
        --reinterpret_cast<Effect *>(effect)->vertexSegmentCount;
        reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    }
    else
    {
        if (reinterpret_cast<Effect *>(effect)->timer == 40)
        {
#pragma var_order(position, radius)
            f32 radius;
            ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x8ff08080, 0, 21);
            angle = reinterpret_cast<Effect *>(effect)->angle + ZUN_PI / 4.0f;
            radius = reinterpret_cast<Effect *>(effect)->radius * 0.7071067094802856f;
            Float3 position;
            g_AnmManager->InitializeVerticalTextureStrip(
                effect,
                reinterpret_cast<Effect *>(effect)->vertices,
                2 * reinterpret_cast<Effect *>(effect)->vertexSegmentCount + 2);
            for (i = 0; i < 4; i++)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;
                position.FromAngleMagnitude(angle, radius);
                position += reinterpret_cast<Effect *>(effect)->vector5;
                slot = g_Player.CreateRectDamageRegion(
                    &position, radius * 8.0f,
                    reinterpret_cast<Effect *>(effect)->shapeThickness * 4.0f, 60, 70);
                slot->collisionInterval = 4;
                slot->angle = AddNormalizeAngle(angle, ZUN_PI / 2.0f);
                angle = slot->angle;
                slot = g_Player.CreateRectCancelRegion(
                    &position, radius * 4.0f,
                    reinterpret_cast<Effect *>(effect)->shapeThickness * 4.0f, 6, 100);
                slot->angle = angle;
            }
        }
        reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    }
    return 1;
}

// FUNCTION: th08 0x4123d0
#pragma var_order(unused0, unused1, bomb, workItem, interp)
void __fastcall UpdateSlashOfFutureEternityDeathbomb(Player *player)
{
    i32 unused0;
    i32 unused1;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 interp;

    bomb = &player->bombState;
    unused1 = 0;
    unused0 = 0;
    workItem = &bomb->workItems[0];

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x90\x6C\x8B\x53\x81\x75\x96\xA2\x97\x88\x89\x69\x8D\x85\x8E\x61\x81\x76",
                     250, 300, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        player->verticalSpeedMultiplier = 0.0f;
        player->horizontalSpeedMultiplier = 0.0f;
        player->anmFile->SetAndExecuteScriptIdx(&player->mainVm, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        workItem->position = player->position;
        workItem->motion = workItem->position;
        workItem->motion.y = 416.0f;
    }

    if (bomb->timer < 40)
    {
        interp = (f32)bomb->timer / 40.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->motion - workItem->position) * interp + workItem->position;
        return;
    }
    else if (bomb->timer.JustReached(40))
    {
        g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xffff8080);
        return;
    }
    else if (bomb->timer.JustReached(70))
    {
        player->position.y = 32.0f;
        Float3 position = player->position;
        position.y = 224.0f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xefffffff, 0, 21);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(42), 0);
        workItem->cancelRegion = player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 500, 0);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 60);
        workItem->damageRegion->collisionInterval = 5;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        return;
    }
    else if (bomb->timer.JustReached(80))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 32.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 64.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xcfffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(90))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 64.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 128.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xafffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(100))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 96.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 192.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x8fffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(110))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 128.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 60, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 256.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 60, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x6fffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(120))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 160.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 50, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 320.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 50, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x5fffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(130))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 192.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 40, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        position.x += 384.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 40, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xffff8080);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x5fffffff, 0, 21);
        workItem->motion = player->position;
        return;
    }
    else if (bomb->timer >= 150 && bomb->timer < 180)
    {
        interp = ((f32)bomb->timer - 150.0f) / 30.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->position - workItem->motion) * interp + workItem->motion;
        return;
    }
    else if (bomb->timer.JustReached(180))
    {
        player->verticalSpeedMultiplier = 1.0f;
        player->horizontalSpeedMultiplier = 1.0f;
    }
}

// FUNCTION: th08 0x411b10
#pragma var_order(unused0, unused1, bomb, workItem, interp)
void __fastcall UpdateSlashOfPresentWorldBomb(Player *player)
{
    i32 unused0;
    i32 unused1;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 interp;

    bomb = &player->bombState;
    unused1 = 0;
    unused0 = 0;
    workItem = &bomb->workItems[0];

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 0,
                     "\x90\x6C\x95\x84\x81\x75\x8C\xBB\x90\xA2\x8E\x61\x81\x76",
                     220, 270, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        player->verticalSpeedMultiplier = 0.0f;
        player->horizontalSpeedMultiplier = 0.0f;
        player->anmFile->SetAndExecuteScriptIdx(&player->mainVm, 0);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(6), 0);
        workItem->position = player->position;
        workItem->motion = workItem->position;
        workItem->motion.y = 416.0f;
    }

    if (bomb->timer < 40)
    {
        interp = (f32)bomb->timer / 40.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->motion - workItem->position) * interp + workItem->position;
        return;
    }
    else if (bomb->timer.JustReached(40))
    {
        g_EffectManager.SpawnEffect(40, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, 0xff8080ff);
        return;
    }
    else if (bomb->timer.JustReached(70))
    {
        player->position.y = 32.0f;
        Float3 position = player->position;
        position.y = 224.0f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xefffffff, 0, 21);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(42), 0);
        workItem->cancelRegion = player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 300, 10);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 80, 60);
        workItem->damageRegion->collisionInterval = 5;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        return;
    }
    else if (bomb->timer.JustReached(80))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 32.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        position.x += 64.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xcfffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(90))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 64.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        position.x += 128.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 100, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xbfffffff, 0, 21);
        return;
    }
    else if (bomb->timer.JustReached(100))
    {
        Float3 position = player->position;
        position.y = 224.0f;
        position.x -= 96.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageRegion->collisionInterval = 2;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        position.x += 192.0f;
        player->CreateRectCancelRegion(&position, 96.0f, 448.0f, 6, 60);
        workItem->damageRegion = player->CreateRectDamageRegion(&position, 96.0f, 448.0f, 80, 40);
        workItem->damageRegion->collisionInterval = 3;
        g_EffectManager.SpawnEffect(48, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, 0xff8080ff);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x8fffffff, 0, 21);
        workItem->motion = player->position;
        return;
    }
    else if (bomb->timer >= 120 && bomb->timer < 150)
    {
        interp = ((f32)bomb->timer - 120.0f) / 30.0f;
        interp = 1.0f - interp;
        interp *= interp;
        interp = 1.0f - interp;
        player->position = (workItem->position - workItem->motion) * interp + workItem->motion;
        return;
    }
    else if (bomb->timer.JustReached(150))
    {
        player->verticalSpeedMultiplier = 1.0f;
        player->horizontalSpeedMultiplier = 1.0f;
    }
}

// FUNCTION: th08 0x4117b0
#pragma var_order(interp, radialBase, i, slot, angle)
i32 __fastcall FUN_004117b0(AnmVm *effect)
{
    f32 interp;
    f32 radialBase;
    i32 i;
    PlayerCollisionRegion *slot;
    f32 angle;

    reinterpret_cast<Effect *>(effect)->angle =
        AddNormalizeAngle(reinterpret_cast<Effect *>(effect)->angle,
                          ((reinterpret_cast<Effect *>(effect)->slotIndex & 1) != 0)
                              ? 0.039269909f
                              : -0.039269909f);
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;

    if (reinterpret_cast<Effect *>(effect)->timer < 50)
    {
        interp = 1.0f - (f32)reinterpret_cast<Effect *>(effect)->timer / 50.0f;
        radialBase = (f32)(reinterpret_cast<Effect *>(effect)->slotIndex - 4) * 32.0f + 384.0f;
        reinterpret_cast<Effect *>(effect)->shapeThickness =
            88.0f - (f32)reinterpret_cast<Effect *>(effect)->timer * 80.0f / 50.0f;
        reinterpret_cast<Effect *>(effect)->radius =
            (f32)(reinterpret_cast<Effect *>(effect)->slotIndex - 4) * 32.0f + 192.0f -
            radialBase * interp * interp;
        --reinterpret_cast<Effect *>(effect)->vertexSegmentCount;
    }
    else
    {
        if (reinterpret_cast<Effect *>(effect)->timer == 50)
        {
#pragma var_order(position, radius)
            f32 radius;
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0, 21);
            ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0x8f6060f0, 0, 21);
            angle = reinterpret_cast<Effect *>(effect)->angle + ZUN_PI / 4.0f;
            radius = reinterpret_cast<Effect *>(effect)->radius * 0.7071067094802856f;
            Float3 position;
            g_AnmManager->InitializeVerticalTextureStrip(
                effect,
                reinterpret_cast<Effect *>(effect)->vertices,
                2 * reinterpret_cast<Effect *>(effect)->vertexSegmentCount + 2);
            for (i = 0; i < 4; i++)
            {
                if (angle >= ZUN_PI)
                    angle -= ZUN_2PI;
                position.FromAngleMagnitude(angle, radius);
                position += reinterpret_cast<Effect *>(effect)->vector5;
                slot = g_Player.CreateRectDamageRegion(
                    &position, radius * 8.0f,
                    reinterpret_cast<Effect *>(effect)->shapeThickness * 4.0f, 60, 100);
                slot->collisionInterval = 2;
                slot->angle = AddNormalizeAngle(angle, ZUN_PI / 2.0f);
                angle = slot->angle;
                slot = g_Player.CreateRectCancelRegion(
                    &position, radius * 4.0f,
                    reinterpret_cast<Effect *>(effect)->shapeThickness * 4.0f, 6, 150);
                slot->angle = angle;
            }
        }
        reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    }
    return 1;
}

// FUNCTION: th08 0x411720
#pragma var_order(velocity, position)
i32 __fastcall FUN_00411720(AnmVm *effect)
{
    Float3 position = reinterpret_cast<Effect *>(effect)->position;
    Float3 velocity = reinterpret_cast<Effect *>(effect)->vector1;

    g_EffectManager.SpawnEffectInFixedSlotWithVelocity(35, reinterpret_cast<D3DXVECTOR3 *>(&position),
                                 reinterpret_cast<D3DXVECTOR3 *>(&velocity),
                                 reinterpret_cast<Effect *>(effect)->slotIndex, 1, -1);
    reinterpret_cast<Effect *>(effect)->updateCallback = reinterpret_cast<void *>(FUN_004114e0);
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 44;
    reinterpret_cast<Effect *>(effect)->shapeThickness = 4.0f;
    return 0;
}

// FUNCTION: th08 0x411a80
#pragma var_order(velocity, position)
i32 __fastcall FUN_00411a80(AnmVm *effect)
{
    Float3 position = reinterpret_cast<Effect *>(effect)->position;
    Float3 velocity = reinterpret_cast<Effect *>(effect)->vector1;

    g_EffectManager.SpawnEffectInFixedSlotWithVelocity(35, reinterpret_cast<D3DXVECTOR3 *>(&position),
                                 reinterpret_cast<D3DXVECTOR3 *>(&velocity),
                                 reinterpret_cast<Effect *>(effect)->slotIndex, 1, -1);
    reinterpret_cast<Effect *>(effect)->updateCallback = reinterpret_cast<void *>(FUN_004117b0);
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 54;
    reinterpret_cast<Effect *>(effect)->shapeThickness = 6.0f;
    return 0;
}

// FUNCTION: th08 0x410fe0
#pragma var_order(bomb, workItem, angle)
void __fastcall UpdateEternalNightQuadrupleBarrierDeathbomb(Player *player)
{
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    f32 angle;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        PlayerCollisionRegion *slot;

        BeginBombSpell(player, 1,
                     "\x8B\xAB\x8A\x45\x81\x75\x89\x69\x96\xE9\x8E\x6C\x8F\x64\x8C\x8B\x8A\x45\x81\x76",
                     250, 300, 1);
        angle = -ZUN_PI;
        workItem = &bomb->workItems[0];
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(13), 0);
        workItem->position = player->position;
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[0], 21);
        player->anmFile->SetAndExecuteScriptIdx(&bomb->workItems[0].vms[1], 22);
        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 100, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(ZUN_PI / 4.0f, 1.0f, 4.0f);
        g_EffectManager.SpawnEffectInFixedSlotWithVelocity(37, reinterpret_cast<D3DXVECTOR3 *>(&workItem->position),
                                     reinterpret_cast<D3DXVECTOR3 *>(&velocity), 4, 1, -1);
    }

    if (bomb->timer.JustReached(10))
    {
        PlayerCollisionRegion *slot;
        AnmVm *effect;

        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 40, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(ZUN_PI * 3.0f / 8.0f, 1.0f, 4.0f);
        effect = g_EffectManager.SpawnEffectInFixedSlotWithVelocity(37, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[0].position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 5, 1, -1);
        g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 93);
        bomb->workItems[1].position = player->position;
    }

    if (bomb->timer.JustReached(20))
    {
        PlayerCollisionRegion *slot;
        AnmVm *effect;

        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 100, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(ZUN_PI / 2.0f, 1.0f, 4.0f);
        effect = g_EffectManager.SpawnEffectInFixedSlotWithVelocity(37, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[0].position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 6, 1, -1);
        g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 94);
        bomb->workItems[2].position = player->position;
    }

    if (bomb->timer.JustReached(30))
    {
        PlayerCollisionRegion *slot;
        AnmVm *effect;

        player->CreateCircleCancelRegion(&player->position, 100.0f, 1.0f, 100, 6);
        slot = player->CreateCircleDamageRegion(&player->position, 100.0f, 1.0f, 70, 40);
        slot->collisionInterval = 5;
        Float3 velocity(1.9634954929351807f, 1.0f, 4.0f);
        effect = g_EffectManager.SpawnEffectInFixedSlotWithVelocity(37, reinterpret_cast<D3DXVECTOR3 *>(&bomb->workItems[0].position),
                                              reinterpret_cast<D3DXVECTOR3 *>(&velocity), 7, 1, -1);
        g_EffectManager.effectAnm->SetAndExecuteScriptIdx(effect, 95);
        bomb->workItems[3].position = player->position;
    }

    g_AnmManager->ExecuteScriptArray(&bomb->workItems[0].vms[0], 2);
}

// FUNCTION: th08 0x4113a0
#pragma var_order(vm, workItem)
void __fastcall DrawEternalNightQuadrupleBarrierDeathbomb(Player *player)
{
    PlayerBombWorkItem *workItem;
    AnmVm *vm;

    workItem = player->bombState.workItems;
    SetBombBackgroundTint(player, 0x802020d0);
    vm = &workItem->vms[0];
    vm->pos = workItem->position + vm->pos2;
    vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
    vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
    vm->pos.z = 0.01f;
    g_AnmManager->Draw2D(vm);

    vm++;
    vm->pos = workItem->position + vm->pos2;
    vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
    vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
    vm->pos.z = 0.0f;
    g_AnmManager->Draw2D(vm);
}

// FUNCTION: th08 0x410bb0
i32 __fastcall FUN_00410bb0(AnmVm *effect)
{
    reinterpret_cast<Effect *>(effect)->radius += 8.0f;
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 12;
    reinterpret_cast<Effect *>(effect)->shapeThickness = 32.0f;
    return 1;
}

// FUNCTION: th08 0x413070
i32 __fastcall FUN_00413070(AnmVm *effect)
{
    f32 interp;
    if (reinterpret_cast<Effect *>(effect)->timer < 30)
    {
        reinterpret_cast<Effect *>(effect)->radius = 192.0f;
        reinterpret_cast<Effect *>(effect)->vertexSegmentCount = 48;
        reinterpret_cast<Effect *>(effect)->shapeThickness = 3.0f;
        reinterpret_cast<Effect *>(effect)->secondaryRadius = 0.0001f;
        reinterpret_cast<Effect *>(effect)->secondaryAngle = ZUN_PI / 2.0f;
    }
    else
    {
        interp = ((f32)reinterpret_cast<Effect *>(effect)->timer - 30.0f) / 30.0f;
        interp *= interp;
        interp *= interp;
        reinterpret_cast<Effect *>(effect)->secondaryRadius = 192.0f * interp + 0.0001f;
        reinterpret_cast<Effect *>(effect)->shapeThickness = 80.0f * interp + 3.0f;
    }
    reinterpret_cast<Effect *>(effect)->verticesDirty = 1;
    return 1;
}

// FUNCTION: th08 0x413140
#pragma var_order(i, bomb, workItem)
void __fastcall UpdateGhastlyDreamBomb(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 1,
                     "\x8E\x80\x95\x84\x81\x75\x83\x4D\x83\x83\x83\x58\x83\x67\x83\x8A\x83\x68\x83\x8A\x81\x5B\x83\x80\x81\x76",
                     300, 350, 0);

        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;

        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = 0.013089969754219055f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = -0.013089969754219055f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = 0.015707964077591896f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 1.5f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = -0.015707964077591896f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 1.5f;
        }

        player->verticalSpeedMultiplier = 0.8f;
        player->horizontalSpeedMultiplier = 0.8f;
        bomb->workItems[0].effectVm =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 120, 12, 0, 0, 21);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;

        workItem->pathPoints[1] = workItem->position;
        workItem->motion.x += workItem->motion.y;
        workItem->angle = AddNormalizeAngle(workItem->angle, workItem->motionStep);
        workItem->position.FromAngleMagnitude(workItem->angle, workItem->motion.x);
        workItem->position += workItem->pathPoints[0];
        workItem->pathPoints[1] = workItem->position - workItem->pathPoints[1];

        if (workItem->motion.x >= 500.0f)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
            workItem->damageRegion->active = 0;
            workItem->cancelRegion->active = 0;
            continue;
        }
        else
        {
            if (workItem->damageRegion != NULL)
            {
                workItem->damageRegion->center.x = workItem->position.x;
                workItem->damageRegion->center.y = workItem->position.y;
                workItem->cancelRegion->center.x = workItem->position.x;
                workItem->cancelRegion->center.y = workItem->position.y;
            }

            if (g_AnmManager->ExecuteScript(&workItem->vms[0]))
            {
                workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
                workItem->damageRegion->active = 0;
                workItem->cancelRegion->active = 0;
            }
        }
    }
}

// FUNCTION: th08 0x413990
#pragma var_order(i, bomb, workItem, spawned, signedScaled)
void __fastcall UpdateEternalSleepInDreamlandDeathbomb(Player *player)
{
    i32 i;
    PlayerBombState *bomb;
    PlayerBombWorkItem *workItem;
    i32 spawned;
    f32 signedScaled;

    bomb = &player->bombState;
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        BeginBombSpell(player, 1,
                     "\x8E\x80\x92\xB1\x81\x75\x89\xD8\xE3\xEF\x82\xCC\x89\x69\x96\xB0\x81\x76",
                     300, 350, 1);

        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;

        workItem = bomb->workItems;
        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = 0.013089969754219055f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = -0.013089969754219055f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 2.0f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 18);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = 0.015707964077591896f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 1.5f;
        }

        for (i = 0; i < 16; i++, workItem++)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 19);
            workItem->angle = (f32)i * ZUN_PI / 8.0f - ZUN_PI;
            workItem->motionStep = -0.015707964077591896f;
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 24.0f, 0.0f, 50, 500);
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 24.0f, 0.0f, 500, 6);
            workItem->damageRegion->hitCap = 800;
            workItem->motion.x = 0.0f;
            workItem->motion.y = 1.5f;
        }

        player->verticalSpeedMultiplier = 0.8f;
        player->horizontalSpeedMultiplier = 0.8f;
        bomb->workItems[0].effectVm =
            g_EffectManager.SpawnEffect(20, reinterpret_cast<D3DXVECTOR3 *>(&player->position), 1, -1);
        g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(5), 0);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 60, 16, 0, 0, 21);
    }

    if (bomb->timer >= 60 && bomb->timer < 200 && bomb->timer.IsPeriodic(20))
    {
        signedScaled = g_Rng.GetRandomF32SignedInRange(ZUN_PI);
        spawned = 0;
        workItem = bomb->workItems;
        for (i = 0; i < 128; i++, workItem++)
        {
            if (workItem->state != PLAYER_BOMB_WORK_ITEM_INACTIVE)
                continue;

            workItem->state = PLAYER_BOMB_WORK_ITEM_ACTIVE;
            player->anmFile->SetAndExecuteScriptIdx(&workItem->vms[0], 20);
            workItem->angle = AddNormalizeAngle((f32)i * ZUN_PI / 8.0f, signedScaled);
            workItem->pathPoints[0] = player->position;
            workItem->position = workItem->pathPoints[0];
            workItem->damageRegion = player->CreateCircleDamageRegion(&workItem->position, 64.0f, 0.0f, 100, 500);
            workItem->damageRegion->hitCap = 1200;
            workItem->cancelRegion = player->CreateCircleCancelRegion(&workItem->position, 64.0f, 0.0f, 500, 6);
            workItem->motion.x = 0.0f;
            workItem->motion.y = 8.0f;
            if (++spawned >= 16)
                break;
        }
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), workItem->position.x);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 30, 8, 0, 0, 21);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_ARCADE_PULSE, 8, 1, 0xe0f0f0f0, 0, 21);
    }

    workItem = bomb->workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;

        workItem->pathPoints[1] = workItem->position;
        workItem->motion.x += workItem->motion.y;
        workItem->angle = AddNormalizeAngle(workItem->angle, workItem->motionStep);
        workItem->position.FromAngleMagnitude(workItem->angle, workItem->motion.x);
        workItem->position += workItem->pathPoints[0];
        workItem->pathPoints[1] = workItem->position - workItem->pathPoints[1];

        if (workItem->motion.x >= 500.0f)
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
            workItem->damageRegion->active = 0;
            workItem->cancelRegion->active = 0;
            continue;
        }

        if (workItem->damageRegion != NULL)
        {
            workItem->damageRegion->center.x = workItem->position.x;
            workItem->damageRegion->center.y = workItem->position.y;
            workItem->cancelRegion->center.x = workItem->position.x;
            workItem->cancelRegion->center.y = workItem->position.y;
        }

        if (g_AnmManager->ExecuteScript(&workItem->vms[0]))
        {
            workItem->state = PLAYER_BOMB_WORK_ITEM_INACTIVE;
            workItem->damageRegion->active = 0;
            workItem->cancelRegion->active = 0;
        }
    }
}

// FUNCTION: th08 0x413890
#pragma var_order(i, workItem)
void __fastcall DrawGhastlyDreamBomb(Player *player)
{
    i32 i;
    PlayerBombWorkItem *workItem;

    SetBombBackgroundTint(player, 0x80404040);
    workItem = player->bombState.workItems;
    for (i = 0; i < 96; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;
        workItem->vms[0].SetZRotation(VectorAngle(workItem->pathPoints[1].y, workItem->pathPoints[1].x));
        workItem->vms[0].pos = workItem->position;
        workItem->vms[0].pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        workItem->vms[0].pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x4142c0
#pragma var_order(i, workItem)
void __fastcall DrawEternalSleepInDreamlandDeathbomb(Player *player)
{
    i32 i;
    PlayerBombWorkItem *workItem;

    SetBombBackgroundTint(player, 0x80802020);
    workItem = player->bombState.workItems;
    for (i = 0; i < 128; i++, workItem++)
    {
        if (workItem->state == PLAYER_BOMB_WORK_ITEM_INACTIVE)
            continue;
        workItem->vms[0].SetZRotation(VectorAngle(workItem->pathPoints[1].y, workItem->pathPoints[1].x));
        workItem->vms[0].pos = workItem->position;
        workItem->vms[0].pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        workItem->vms[0].pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        workItem->vms[0].pos.z = 0.0f;
        g_AnmManager->Draw2D(&workItem->vms[0]);
    }
}

// FUNCTION: th08 0x412300
#pragma var_order(bomb, rect, color)
void __fastcall DrawSlashOfPresentWorldBomb(Player *player)
{
    PlayerBombState *bomb = &player->bombState;
    SetBombBackgroundTint(player, 0x80404040);
    if (bomb->timer >= 70)
    {
        SetBombBackgroundTint(player, 0x80000030);
        ZunColor color;
        ZunRect rect;
        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        if (bomb->timer < 100)
        {
            color.a = 0xff;
        }
        else if (bomb->timer < 160)
        {
            color.a = 0xff - 0xff * ((i32)bomb->timer - 100) / 60;
        }
        else
        {
            return;
        }
        ScreenEffect::DrawSquare(&rect, color.d3dColor);
    }
}

// FUNCTION: th08 0x412fa0
#pragma var_order(bomb, rect, color)
void __fastcall DrawSlashOfFutureEternityDeathbomb(Player *player)
{
    PlayerBombState *bomb = &player->bombState;
    SetBombBackgroundTint(player, 0x80404040);
    if (bomb->timer >= 70)
    {
        SetBombBackgroundTint(player, 0x80000030);
        ZunRect rect;
        ZunColor color;
        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        color.r = 0xff;
        color.g = 0x00;
        color.b = 0x00;
        if (bomb->timer < 100)
        {
            color.a = 0xff;
        }
        else if (bomb->timer < 160)
        {
            color.a = 0xff - 0xff * ((i32)bomb->timer - 100) / 60;
        }
        else
        {
            return;
        }
        ScreenEffect::DrawSquare(&rect, color.d3dColor);
    }
}

} // namespace th08
