#include "th_pch.h"

#include "EnemyManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ItemManager.hpp"
#include "Gui.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();
i32 IsResourceReloadEnabled();
f32 __stdcall FUN_0042eb10(f32 angle1, f32 angle2, f32 factor);

DIFFABLE_STATIC(EnemyManager, g_EnemyManager);
DIFFABLE_STATIC(u16, g_EnemyDropCounter);
DIFFABLE_STATIC(u16, g_EnemyDropScheduleIndex);
DIFFABLE_STATIC_ARRAY_ASSIGN(u8, 32, g_EnemyDropSchedule) = {
    0, 0, 1, 0, 1, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 0, 1, 1, 1, 0, 0,
};

namespace EclOperands
{




} // namespace EclOperands

// FUNCTION: th08 0x422c40
void Enemy::FUN_00422c40()
{
    switch ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 12) & 3)
    {
    case 3:
    {
        {
            Float3 legacyWork;
        }
        Float3 polarVelocity;

        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d9c) =
            AddNormalizeAngle(
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d9c),
                g_EclGameTimeScale * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2da0));
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2db0) =
            g_EclGameTimeScale * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2db4) +
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2db0);
        polarVelocity.FromAngleMagnitude(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d9c),
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2db0));
        this->vector2d4c.x = polarVelocity.x + this->vector2dd0.x - this->vector2d34.x;
        this->vector2d4c.y = polarVelocity.y + this->vector2dd0.y - this->vector2d34.y;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d94) =
            VectorAngle(this->vector2d4c.y, this->vector2d4c.x);
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x2de8) > 0)
        {
            this->timer2ddc--;
            if (this->timer2ddc <= 0)
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) &= 0xffffcfff;
        }
        break;
    }

    case 1:
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d94) =
            AddNormalizeAngle(
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d94),
                g_EclGameTimeScale * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d98));
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2da8) =
            g_EclGameTimeScale * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2dac) +
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2da8);
        this->vector2d4c.FromAngleMagnitude(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d94),
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2da8));
        this->vector2d4c.operator float *()[2] = 0.0f;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x2de8) > 0)
        {
            this->timer2ddc--;
            if (this->timer2ddc <= 0)
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) &= 0xffffcfff;
        }
        break;

    case 2:
    {
        f32 progress;

        this->timer2ddc--;
        progress = 1.0f - (f32)this->timer2ddc /
                              *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x2de8);
        if (progress < 0.0f)
            progress = 0.0f;
        switch ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 14) & 7)
        {
        case 1: progress *= progress; break;
        case 2: progress = progress * progress * progress; break;
        case 3: progress = progress * progress * progress * progress; break;
        case 4:
            progress = 1.0f - progress;
            progress *= progress;
            progress = 1.0f - progress;
            break;
        case 5:
            progress = 1.0f - progress;
            progress = progress * progress * progress;
            progress = 1.0f - progress;
            break;
        case 6:
            progress = 1.0f - progress;
            progress = progress * progress * progress * progress;
            progress = 1.0f - progress;
            break;
        }

        this->vector2d4c = this->vector2dd0 + this->vector2dc4 * progress - this->vector2d34;
        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 18) & 1) != 0)
            this->vector2d4c.x = -this->vector2d4c.x;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2d94) =
            VectorAngle(this->vector2d4c.y, this->vector2d4c.x);
        if (this->timer2ddc <= 0)
        {
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) &= 0xffffcfff;
            this->vector2d34 = this->vector2dd0 + this->vector2dc4;
            this->vector2d4c = Float3(0.0f, 0.0f, 0.0f);
        }
        break;
    }
    }
}

// FUNCTION: th08 0x429e00
#pragma var_order(i, enemy, this)
void EnemyManager::Initialize()
{
    u8 *enemy = reinterpret_cast<u8 *>(this) + 0x53D0;
    i32 i;

    memset(this, 0, 0x9DCF10);
    for (i = 0; (u32)i < 4; i++)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x9DCEFC + i * 4) = -1;

    enemy = reinterpret_cast<u8 *>(this);
    memset(enemy, 0, 0x53D0);
    for (i = 0; i < 2; i++)
        *reinterpret_cast<i16 *>(enemy + i * 0x2A4 + 0x4CA) = -1;
    for (i = 0; i < 0x60; i++)
        *reinterpret_cast<i32 *>(enemy + i * 0x1C + 0x3394) = 0xC479C000;

    *reinterpret_cast<u32 *>(enemy + 0x3324) |= 1;
    reinterpret_cast<Enemy *>(enemy)->bossTimer = 0;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFEFFFFFF;

    *reinterpret_cast<D3DXVECTOR3 *>(enemy + 0x2D70) = D3DXVECTOR3(24.0f, 24.0f, 24.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(enemy + 0x2D4C) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<i32 *>(enemy + 0x2D98) = 0;
    *reinterpret_cast<i32 *>(enemy + 0x2D94) = 0;
    *reinterpret_cast<i32 *>(enemy + 0x2DAC) = 0;
    *reinterpret_cast<i32 *>(enemy + 0x2DA8) = 0;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFFFCFFF;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFFDFFFF;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFFBFFFF;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFFFFFFD;
    *reinterpret_cast<i16 *>(enemy + 0x2CEA) = 0;
    reinterpret_cast<Enemy *>(enemy)->life = 1;
    reinterpret_cast<Enemy *>(enemy)->score = 100;
    *reinterpret_cast<u8 *>(enemy + 0x3310) = 0;
    *reinterpret_cast<u8 *>(enemy + 0x3311) = 0;
    *reinterpret_cast<u8 *>(enemy + 0x3312) = 0;
    reinterpret_cast<Enemy *>(enemy)->shootIntervalFrames = 0;
    reinterpret_cast<Enemy *>(enemy)->shootIntervalTimer = 0;
    *reinterpret_cast<D3DXVECTOR3 *>(enemy + 0x2DB8) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<i16 *>(enemy + 0x3338) = -1;
    *reinterpret_cast<i16 *>(enemy + 0x333A) = -1;
    *reinterpret_cast<i16 *>(enemy + 0x3332) = -1;
    *reinterpret_cast<u32 *>(enemy + 0x3324) |= 4;
    *reinterpret_cast<u32 *>(enemy + 0x3324) |= 8;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFFFFFEF;
    *reinterpret_cast<u32 *>(enemy + 0x3324) |= 0x40;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFFFFF7F;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFF8FFFFF;
    *reinterpret_cast<i16 *>(enemy + 0x2CEE) = -1;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFFF7FFFF;
    *reinterpret_cast<i32 *>(enemy + 0x53C0) = 0;
    *reinterpret_cast<i16 *>(enemy + 0x2D30) = -1;
    for (i = 0; i < 4; i++)
        *reinterpret_cast<i32 *>(enemy + 0x3358 + i * 4) = -1;
    *reinterpret_cast<i32 *>(enemy + 0x3378) = -1;
    *reinterpret_cast<i32 *>(enemy + 0x3300) = 0;
    *reinterpret_cast<u8 *>(enemy + 0x3314) = 0;
    *reinterpret_cast<u32 *>(enemy + 0x3324) &= 0xFDFFFFFF;
    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedLow = -0.15f;
    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedHigh = 0.15f;
    reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor.spawnSound = 7;
    reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor.transformSound = 25;
    *reinterpret_cast<u32 *>(enemy + 0x3350) = 0x44800000;
    reinterpret_cast<Enemy *>(enemy)->playerShotHitAccumulator = *reinterpret_cast<i32 *>(0x18B8A24);
}

// FUNCTION: th08 0x42a210
EnemyManager::EnemyManager()
{
    this->Initialize();
}

// FUNCTION: th08 0x42a280
Enemy::Enemy() {}

// FUNCTION: th08 0x42a450
EnemyUnkStruct2::EnemyUnkStruct2() {}

// FUNCTION: th08 0x42a490
EnemyUnkStruct0x1c::EnemyUnkStruct0x1c() {}

// FUNCTION: th08 0x42a4c0
EnemyUnkStruct3::EnemyUnkStruct3() {}

// FUNCTION: th08 0x42a820
#pragma var_order(i, this)
void Enemy::FUN_0042a820()
{
    i32 i;

    for (i = 0; i < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x53c0); i++)
    {
        if (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x5360 + i * 4) == NULL)
            continue;
        *reinterpret_cast<u8 *>(
            reinterpret_cast<u8 *>(*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x5360 + i * 4)) + 0x352) = 1;
        *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x5360 + i * 4) = NULL;
    }
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x53c0) = 0;
}

namespace EclOperands
{

// FUNCTION: th08 0x42adb0
#pragma var_order(j, nextEnemy, enemy, popupColor, chainIndex, position, dropLocals, itemType, attachedPosition)
void EnemyOverlay::FUN_0042adb0(i32 mode)
{
    i32 j;
    EnemyOverlay *nextEnemy;
    EnemyOverlay *enemy;
    i32 popupColor;
    i32 chainIndex;

    j = reinterpret_cast<TargetEnemyHelpersOverlay *>(this)->CountParentChain();
    if (j != 0)
    {
        chainIndex = 0;
        Float3 position;
        struct DropLocals { i32 itemCount; i32 i; } dropLocals;
        i32 itemType;
        enemy = *reinterpret_cast<EnemyOverlay **>(this->bytes + 8);
        popupColor = j < 2 ? -1 : (j < 6 ? -48 : (j < 10 ? -80 : -128));

        while (enemy != NULL)
        {
            if (((*reinterpret_cast<u32 *>(enemy->bytes + 0x3324) >> 9) & 1) != 0)
                *reinterpret_cast<Float3 *>(enemy->bytes + 0x2D40) =
                    *reinterpret_cast<Float3 *>(this->bytes + 0x2D34);

            nextEnemy = *reinterpret_cast<EnemyOverlay **>(enemy->bytes + 8);
            *reinterpret_cast<u32 *>(enemy->bytes + 0x3324) |= 0x400;
            *reinterpret_cast<f32 *>(enemy->bytes + 0x2DA4) = 0.0f;
            *reinterpret_cast<void **>(enemy->bytes + 8) = NULL;
            *reinterpret_cast<void **>(enemy->bytes + 4) = NULL;

            if (mode != 0)
            {
                itemType = (((*reinterpret_cast<u32 *>(this->bytes + 0x3324) >> 1) & 1) != 0) ? 7 : 9;
                if (g_GameManager.IsSoloYoukai())
                    dropLocals.itemCount = j >= 10 ? 26 : j * 2 + 6;
                else if (g_GameManager.IsSoloHuman())
                    dropLocals.itemCount = j >= 4 ? 40 : j * 6 + 16;
                else
                    dropLocals.itemCount = j >= 8 ? 26 : j * 2 + 10;

                if (g_Player.bombState.isInUse != 0)
                    j /= 3;

                g_AsciiManager.CreateTimePopup(
                    reinterpret_cast<Float3 *>(enemy->bytes + 0x2D88), j, 0, popupColor);
                *reinterpret_cast<Float3 *>(enemy->bytes + 0x2D88) =
                    *reinterpret_cast<Float3 *>(enemy->bytes + 0x2D34) +
                    *reinterpret_cast<Float3 *>(enemy->bytes + 0x2D40);
                g_Player.FUN_0044df00(reinterpret_cast<Float3 *>(enemy->bytes + 0x2D88),
                                      32.0f, 2.0f, 8, itemType);

                for (dropLocals.i = 0; dropLocals.i < dropLocals.itemCount; dropLocals.i++)
                {
                    position.FromAngleMagnitude(
                        g_Rng.GetRandomF32SignedInRange(ZUN_PI),
                        g_Rng.GetRandomF32InRange((f32)dropLocals.itemCount * 2.0f));
                    position.z = 0.0f;
                    position += *reinterpret_cast<Float3 *>(enemy->bytes + 0x2D88);
                    g_ItemManager.SpawnItem(&position, ITEM_TIME, 3);
                }

                if (!g_EnemyManager.FUN_0042f1f0() || g_Spellcard.IsActive())
                {
                    *reinterpret_cast<i32 *>(enemy->bytes + 0x3304) = 8;
                    reinterpret_cast<Enemy *>(enemy)->FUN_0042bea0(0);
                }
                g_SoundPlayer.PlaySoundPositionedByIdx(
                    static_cast<SoundIdx>(chainIndex % 2 + 2),
                    *reinterpret_cast<f32 *>(enemy->bytes + 0x2D88));
            }

            *reinterpret_cast<f32 *>(enemy->bytes + 0x330C) = 0.0f;
            *reinterpret_cast<f32 *>(enemy->bytes + 0x3308) = 0.0f;
            *reinterpret_cast<i32 *>(enemy->bytes + 0x3304) = -2;
            enemy = nextEnemy;
            ++chainIndex;
        }

        if (mode != 0)
        {
            g_AsciiManager.SetScale(2.0f, 2.0f);
            g_AsciiManager.CreateTimePopup(
                reinterpret_cast<Float3 *>(this->bytes + 0x2D88),
                *reinterpret_cast<i32 *>(this->bytes + 0x3380), 0, 0xFFF0F00F);
            g_AsciiManager.SetScale(1.0f, 1.0f);

            for (j = 0; j < 2 * *reinterpret_cast<i32 *>(this->bytes + 0x3380); j++)
            {
                position.FromAngleMagnitude(
                    g_Rng.GetRandomF32SignedInRange(ZUN_PI),
                    g_Rng.GetRandomF32InRange(128.0f));
                position.z = 0.0f;
                position += *reinterpret_cast<Float3 *>(this->bytes + 0x2D88);
                g_ItemManager.SpawnItem(&position, ITEM_TIME, 1);
            }
            g_Player.FUN_0044df00(reinterpret_cast<Float3 *>(this->bytes + 0x2D88),
                                  32.0f, 1.0f, 16, 7);
            *reinterpret_cast<ZunTimer *>(&g_Player.timerE2ADC) = 0;
        }
    }

    if (reinterpret_cast<TargetEnemyHelpersOverlay *>(this)->HasAttachedEnemy() && mode != 0)
    {
        Float3 attachedPosition;
        g_GameManager.AddToYoukaiGauge(-g_GameManager.GetYoukaiGauge() / 12, 0);
        *reinterpret_cast<ZunTimer *>(&g_Player.timerE2AE8) = 0;
        *reinterpret_cast<ZunTimer *>(&g_Player.timerE2AD0) = 30;
        *reinterpret_cast<ZunTimer *>(&g_Player.timerE2ADC) = 50;
        *reinterpret_cast<Float3 *>(this->bytes + 0x2D88) =
            *reinterpret_cast<Float3 *>(this->bytes + 0x2D34) +
            *reinterpret_cast<Float3 *>(this->bytes + 0x2D40);
        g_AsciiManager.CreateTimePopup(
            reinterpret_cast<Float3 *>(this->bytes + 0x2D88), 1, 0, 0xFFFFFFFF);
        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(this->bytes + 0x2D88), ITEM_TIME, 1);
        *reinterpret_cast<i32 *>(this->bytes + 0x330C) = 0;
        *reinterpret_cast<i32 *>(this->bytes + 0x3308) = 0;
        *reinterpret_cast<i32 *>(this->bytes + 0x3304) = -2;
    }

    reinterpret_cast<Enemy *>(this)->FUN_0042b2f0();
}

} // namespace EclOperands

// FUNCTION: th08 0x42b2f0
void Enemy::FUN_0042b2f0()
{
    if (reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(this)->HasAttachedEnemy())
    {
        *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 4) + 8) =
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 8);
        if (*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 8) != NULL)
        {
            *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 8) + 4) =
                *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 4);
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x2DA4) = 0;
        *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 8) = NULL;
        *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 4) = NULL;
    }
    else
    {
        *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 4) = NULL;
        *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 8) = NULL;
    }
}

DIFFABLE_STATIC(ChainElem, g_EnemyManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainLowPrio);

// FUNCTION: th08 0x42b370
#pragma var_order(damage, i, maxHp, this)
void Enemy::FUN_0042b370(i32 amount)
{
    i32 damage;
    i32 i;
    i32 maxHp;

    if (!reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(this)->HasAttachedEnemy())
        return;
    if (g_Player.bombState.isInUse != 0)
        return;

    maxHp = 0;
    for (i = 0; i < 4; ++i)
    {
        if (maxHp < *reinterpret_cast<i32 *>(
                        *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4) +
                        0x3358 + i * 4))
        {
            maxHp = *reinterpret_cast<i32 *>(
                *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4) +
                0x3358 + i * 4);
        }
    }

    damage = amount / 2;
    if (*reinterpret_cast<ZunTimer *>(
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4) + 0x5354) > 0)
    {
        if (((*reinterpret_cast<u32 *>(
                  *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4) + 0x3324) >>
              1) &
             1) != 0)
            damage /= 9;
        else
            damage = 0;
    }

    if (damage == 0)
        return;

    reinterpret_cast<Enemy *>(
        *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4))->life -= damage;
    if (reinterpret_cast<Enemy *>(
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4))->life <= maxHp)
    {
        reinterpret_cast<Enemy *>(
            *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x2da4))->life = maxHp;
    }
}

// FUNCTION: th08 0x42b490
#pragma var_order(state, phaseCount, i, work, enemyCursor, k, this)
i32 Enemy::FUN_0042b490()
{
    u32 state;
    i32 phaseCount;
    i32 i;
    i32 work;
    Enemy *enemyCursor;
    i32 k;

    phaseCount = 0;
    state = 0;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) &= 0xffffffcf;
    for (i = 0; i < 4; i++)
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4) < 0)
            continue;

        phaseCount++;
        if (this->life <
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4))
        {
            this->life =
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4);
            this->phaseStartingLife = this->life;
            g_EclManager.CallEclSub(
                reinterpret_cast<EnemyEclContext *>(reinterpret_cast<u8 *>(this) + 0x7f8),
                *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x3368 + i * 4));
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4) = -1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x53cc) =
                (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3378) - (i32)this->bossTimer) / 60;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3378) = -1;

            for (work = 0; work < 4; work++)
            {
                if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + work * 4) != NULL)
                {
                    g_ZunMemory.Free(*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + work * 4));
                    *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + work * 4) = NULL;
                }
            }

            this->enemy_fun_00415c80();
            *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2cea) = 0;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) &= 0xffffffcf;
            this->bulletSpawnDescriptor = g_EnemyManager.firstEnemy.bulletSpawnDescriptor;
            this->shootIntervalFrames = 0;
            reinterpret_cast<EclOperands::EnemyOverlay *>(this)->FUN_0042adb0(1);

            enemyCursor = &g_EnemyManager.enemies[0];
            for (k = 0; k < 480; k++, enemyCursor++)
            {
                if ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x3324) & 1) == 0)
                    continue;
                if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x3324) >> 1) & 1) != 0)
                    continue;

                enemyCursor->life = 0;
                if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x2cee) >= 0)
                {
                    g_EclManager.CallEclSub(
                        reinterpret_cast<EnemyEclContext *>(reinterpret_cast<u8 *>(enemyCursor) + 0x7f8),
                        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x2cee));
                    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x2cee) = -1;
                }
            }

            if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) != 0 &&
                g_Player.playerState == 0)
            {
                g_Player.timer = 70;
                g_Player.playerState = 3;
            }
            return 1;
        }

        work = this->life -
               *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4);
        if (g_Spellcard.IsActive())
        {
            if (work < 120)
                state = 3;
            else if (work < 200)
                state = 2;
            else if (work < 300)
                state = 1;
            else
                state = 0;
        }
        else
        {
            if (work < 500)
                state = 3;
            else if (work < 1500)
                state = 2;
            else if (work < 2200)
                state = 1;
            else
                state = 0;
        }

        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) >> 4) & 3) < state)
        {
            struct EnemyPhaseBits
            {
                u32 pad0 : 4;
                u32 state : 2;
                u32 pad6 : 26;
            };
            reinterpret_cast<EnemyPhaseBits *>(reinterpret_cast<u8 *>(this) + 0x3328)->state = state;
        }
    }

    if (phaseCount == 0)
    {
        work = this->life;
        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) != 0)
        {
            if (g_Spellcard.IsActive())
            {
                if (work < 120)
                    state = 3;
                else if (work < 300)
                    state = 2;
                else if (work < 400)
                    state = 1;
                else
                    state = 0;
            }
            else
            {
                if (work < 600)
                    state = 3;
                else if (work < 1600)
                    state = 2;
                else if (work < 2400)
                    state = 1;
                else
                    state = 0;
            }
        }
        else if (g_Spellcard.IsActive())
        {
            if (work < 10)
                state = 3;
            else
                state = 0;
        }
        else
        {
            if (work < 50)
                state = 3;
            else
                state = 0;
        }

        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) >> 4) & 3) < state)
        {
            struct EnemyPhaseBits
            {
                u32 pad0 : 4;
                u32 state : 2;
                u32 pad6 : 26;
            };
            reinterpret_cast<EnemyPhaseBits *>(reinterpret_cast<u8 *>(this) + 0x3328)->state = state;
        }
    }
    return 0;
}

// FUNCTION: th08 0x42b930
#pragma var_order(i, maxThreshold, selectedOrK, enemyCursor, j, this)
i32 Enemy::FUN_0042b930()
{
    i32 i;
    i32 maxThreshold;
    i32 selectedOrK;
    Enemy *enemyCursor;
    i32 j;

    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) != 0 &&
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3313) == 0)
    {
        g_Gui.FUN_0042f340(
            (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3378) - (i32)this->bossTimer) / 60);
    }

    if (this->bossTimer >= *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3378))
    {
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x53cc) = 0;
    maxThreshold = 0;
    for (i = 0; i < 4; i++)
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4) < 0)
            continue;
        if (maxThreshold < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4))
        {
            maxThreshold = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4);
            selectedOrK = i;
        }
    }

    if (maxThreshold > 0)
    {
        this->life =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + selectedOrK * 4);
        this->phaseStartingLife = this->life;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + selectedOrK * 4) = -1;
    }

    g_EclManager.CallEclSub(
        reinterpret_cast<EnemyEclContext *>(reinterpret_cast<u8 *>(this) + 0x7f8),
        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x337c));
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3378) = -1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x337c) =
        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2cee);
    this->bossTimer = 0;

    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 27) & 1) == 0)
    {
        FUN_0042bc50(&g_Spellcard);
        g_BulletManager.RemoveAllBullets(4);
    }

    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) != 0 &&
        g_Player.playerState == 0)
    {
        g_Player.timer = 70;
        g_Player.playerState = 3;
    }

    reinterpret_cast<EclOperands::EnemyOverlay *>(this)->FUN_0042adb0(0);
    enemyCursor = &g_EnemyManager.enemies[0];
    for (j = 0; j < 480; j++, enemyCursor++)
    {
        if ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x3324) & 1) == 0)
            continue;
        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x3324) >> 1) & 1) != 0)
            continue;
        enemyCursor->life = 0;
        if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x2cee) >= 0)
        {
            g_EclManager.CallEclSub(
                reinterpret_cast<EnemyEclContext *>(reinterpret_cast<u8 *>(enemyCursor) + 0x7f8),
                *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x2cee));
            *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemyCursor) + 0x2cee) = -1;
        }
    }

    for (selectedOrK = 0; selectedOrK < 4; selectedOrK++)
    {
        if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + selectedOrK * 4) != NULL)
        {
            g_ZunMemory.Free(*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + selectedOrK * 4));
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + selectedOrK * 4) = NULL;
        }
    }

    this->bulletSpawnDescriptor = g_EnemyManager.firstEnemy.bulletSpawnDescriptor;
    this->shootIntervalFrames = 0;
    this->enemy_fun_00415c80();
    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2cea) = 0;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) &= 0xffffffcf;
    return 1;
    }
    return 0;
}

// FUNCTION: th08 0x42bc50
void __fastcall FUN_0042bc50(void *self)
{
    *reinterpret_cast<u32 *>(self) &= ~4u;
    *reinterpret_cast<u32 *>(self) |= 8u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(self) + 0xfc) = 0;
}

// FUNCTION: th08 0x42bc90
void Enemy::FUN_0042bc90()
{
    for (i32 i = 0; i < 4; i++)
    {
        if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + i * 4) != NULL)
        {
            g_ZunMemory.Free(*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + i * 4));
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x3384 + i * 4) = NULL;
        }
    }
}

// FUNCTION: th08 0x42bcf0
#pragma var_order(i, this)
void Enemy::FUN_0042bcf0()
{
    i32 i;

    reinterpret_cast<EclOperands::EnemyOverlay *>(this)->FUN_0042adb0(0);

    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 20) & 7) == 0)
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) &= ~1U;

    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) != 0 &&
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3313) < 4)
    {
        g_Gui.SetBossPresent(false);
        EclRunLowProposal::g_EclEnemyTableF54CC0[
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3313)] = NULL;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) &= ~2U;
        g_AsciiManager.FUN_00422bb0(
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3313), 2);
        g_AsciiManager.SetBossMarkerPosition(
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3313),
            reinterpret_cast<D3DXVECTOR3 *>(&Float3(-999.0f, -999.0f, 0.0f)));
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x53c0) != 0)
        this->FUN_0042a820();

    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) != 0)
        EclRunLowProposal::g_EclEnemyTableF54CC0[
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3313)] = NULL;

    g_ReplayManager->flags |= 0x20;

    if (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8) != NULL)
    {
        (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8))->SetInterrupt(3);
        *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8) = NULL;
    }

    for (i = 0; i < 4; ++i)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3358 + i * 4) = -1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3378) = -1;

    this->FUN_0042bc90();
    if (g_Player.optionHomingTarget == this)
        g_Player.optionHomingTarget = NULL;
}

// FUNCTION: th08 0x42bea0
#pragma var_order(i, position)
void Enemy::FUN_0042bea0(i32 mode)
{
    Float3 position;
    i32 i;
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3304) >= 0)
    {
        g_EffectManager.SpawnEffect(
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3311) + 4,
            reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D88), 3, -1);
        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2D88),
                                static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3304)),
                                mode != 0);
    }
    else if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3304) == -1)
    {
        if ((g_EnemyDropCounter % 3) == 0)
        {
            g_EffectManager.SpawnEffect(
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3311) + 4,
                reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D88), 6, -1);
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2D88),
                                    static_cast<ItemType>(g_EnemyDropSchedule[g_EnemyDropScheduleIndex]),
                                    mode != 0);
            ++g_EnemyDropScheduleIndex;
            if (g_EnemyDropScheduleIndex >= 32)
                g_EnemyDropScheduleIndex = 0;
        }
        ++g_EnemyDropCounter;
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x330C) != 0)
    {
        for (i = 0; i < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x330C); i++)
        {
            position = *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2D88);
            position.operator float *()[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            position.operator float *()[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            if (g_GameManager.GetPower() < 128)
                g_ItemManager.SpawnItem(&position, ITEM_POWER_SMALL, 0);
            else
                g_ItemManager.SpawnItem(&position, ITEM_POINT, 0);
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x330C) = 0;
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3308) != 0)
    {
        for (i = 0; i < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3308); i++)
        {
            position = *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2D88);
            position.operator float *()[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            position.operator float *()[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(&position, ITEM_POINT, 0);
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3308) = 0;
    }
}

// FUNCTION: th08 0x42c180
void Enemy::ClampPosition()
{
    if ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 19) & 1)
    {
        if ((*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[0] < *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3340))
            (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[0] = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3340);
        else if ((*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[0] > *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3348))
            (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[0] = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3348);

        if ((*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[1] < *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3344))
            (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[1] = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3344);
        else if ((*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[1] > *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x334C))
            (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(this) + 0x2D34))[1] = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x334C);
    }
}

// FUNCTION: th08 0x42c290
#pragma var_order(collisionSize)
void Enemy::FUN_0042c290(Float3 *position, Float3 *size)
{
    Float3 collisionSize;

    collisionSize = *size / 0.7f;
    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 7) & 1) != 0 &&
        this->bossTimer.FUN_0040d3d0() && this->bossTimer % 6 == 0)
    {
        g_Player.FUN_0044a470(position, &collisionSize);
    }

    if (g_GameManager.shotType == 0 || g_GameManager.shotType == 4)
    {
        if (reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(this)->HasAttachedEnemy())
            return;
    }

    {
        collisionSize = *size / 1.5f;
        if (g_Player.FUN_0044a360(position, &collisionSize) == 1)
        {
            if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 1) & 1) == 0 &&
                ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 7) & 1) == 0)
            {
                this->life -= 10;
            }
        }
    }
}

// FUNCTION: th08 0x42c3b0
#pragma var_order(interval, this)
void EnemyManager::FUN_0042c3b0()
{
    i32 interval;

    if (!g_Gui.IsDialogPresent())
    {
        interval = 2400;
        interval -= g_GameManager.GetLives() * 4 * 60;
        if (this->timer.FUN_0040d3d0())
        {
            if ((i32)this->timer % interval == 0)
                g_GameManager.IncreaseSubrank(100);
        }
    }
}

// FUNCTION: th08 0x42c420
void Enemy::FUN_0042c420()
{
    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 11) & 1) == 0)
    {
        if (g_Player.IsYoukai())
        {
            g_EffectManager.SpawnEffect(31, reinterpret_cast<D3DXVECTOR3 *>(&this->vector2d88), 1, 0x80303080);
            if (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8) != NULL)
                (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8))->SetInterrupt(2);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(40), 0);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x332f) = 0;
        }

        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3328) >> 1) & 1) != 0 &&
            this->bossTimer.FUN_0040ebc0(2))
        {
            g_EffectManager.SpawnEffect(38, reinterpret_cast<D3DXVECTOR3 *>(&this->vector2d88), 1, -1);
        }
    }
    else
    {
        if (!g_Player.IsYoukai())
        {
            g_EffectManager.SpawnEffect(30, reinterpret_cast<D3DXVECTOR3 *>(&this->vector2d88), 1, 0x80803030);
            if (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8) != NULL)
                (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(this) + 0x53c8))->SetInterrupt(1);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(39), 0);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x332f) = 2;
        }
    }

    struct EnemyYoukaiFlagBits
    {
        u32 pad0 : 11;
        u32 isYoukai : 1;
        u32 pad12 : 20;
    };
    reinterpret_cast<EnemyYoukaiFlagBits *>(reinterpret_cast<u8 *>(this) + 0x3324)->isYoukai = g_Player.IsYoukai();
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3330) = g_Player.IsYoukai() ? 64 : 32;
}

// FUNCTION: th08 0x42c590
#pragma var_order(result, enemyManager)
ZunResult EnemyManager::RegisterChain()
{
    EnemyManager *enemyManager = &g_EnemyManager;
    i32 result = 0;

    enemyManager->Initialize();

    g_EnemyManagerCalcChain.SetCallback((ChainCallback)EnemyManager::OnUpdate);
    g_EnemyManagerCalcChain.addedCallback = (ChainLifetimeCallback)EnemyManager::AddedCallback;
    g_EnemyManagerCalcChain.deletedCallback = (ChainLifetimeCallback)EnemyManager::DeletedCallback;
    g_EnemyManagerCalcChain.arg = enemyManager;
    if (g_Chain.AddToCalcChain(&g_EnemyManagerCalcChain, 11) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_EnemyManagerDrawChainHighPrio.SetCallback((ChainCallback)EnemyManager::OnDrawHighPrio);
    g_EnemyManagerDrawChainHighPrio.arg = enemyManager;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChainHighPrio, 8) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_EnemyManagerDrawChainLowPrio.SetCallback((ChainCallback)EnemyManager::OnDrawLowPrio);
    g_EnemyManagerDrawChainLowPrio.arg = enemyManager;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChainLowPrio, 11) != ZUN_SUCCESS)
        return ZUN_ERROR;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42deb0
void Enemy::FUN_0042deb0()
{
    this->vector2d64 = this->vector2d34 - this->vector2d58;
    this->vector2d58 = this->vector2d34;
    if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 18) & 1) == 0)
        this->vector2d34.x += g_EclGameTimeScale * this->vector2d4c.x;
    else
        this->vector2d34.x -= g_EclGameTimeScale * this->vector2d4c.x;
    this->vector2d34.y += g_EclGameTimeScale * this->vector2d4c.y;
    this->vector2d34.z += g_EclGameTimeScale * this->vector2d4c.z;
}

// FUNCTION: th08 0x42e010
#pragma var_order(effect, i, this)
void Enemy::FUN_0042e010()
{
    u8 *effect;
    i32 i;

    for (i = 0; i < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x53c0); ++i)
    {
        effect = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x5360 + i * 4);
        if (effect == NULL)
            continue;

        reinterpret_cast<AnmVmBase *>(effect)->flag1 =
            ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x3324) >> 4) & 1) == 0;
        *reinterpret_cast<Float3 *>(effect + 0x2e0) = this->vector2d34;

        if (*reinterpret_cast<f32 *>(effect + 0x314) <
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x53c4))
        {
            *reinterpret_cast<f32 *>(effect + 0x314) += 0.3f;
        }
        else
        {
            *reinterpret_cast<f32 *>(effect + 0x314) =
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x53c4);
        }

        *reinterpret_cast<f32 *>(effect + 0x318) =
            AddNormalizeAngle(*reinterpret_cast<f32 *>(effect + 0x318), 0.031415928f);
    }
}

// FUNCTION: th08 0x42e120
ChainCallbackResult EnemyManager::OnDrawHighPrio(EnemyManager *enemyManager)
{
    return enemyManager->OnDrawImpl(0, 2);
}

// FUNCTION: th08 0x42e140
#pragma var_order(savedScaleY, savedScaleX, i, savedColor, vm, k, enemy, halfWidth, halfCenter, vertexCount, sinAngle, uv, previousAngle, vertices, uvStep, angle, cosAngle, uvSpan, this, drawGroup)
ChainCallbackResult __fastcall EnemyManager::OnDrawImpl(i32 drawGroup, i32 chainPriority)
{
    f32 savedScaleY;
    f32 savedScaleX;
    i32 i;
    u32 savedColor;
    AnmVm *vm;
    i32 k;
    u8 *enemy;
    f32 halfWidth;
    f32 halfCenter;
    i32 vertexCount;
    f32 sinAngle;
    f32 uv;
    f32 previousAngle;
    VertexTex1DiffuseXyzrhw *vertices;
    f32 uvStep;
    f32 angle;
    f32 cosAngle;
    f32 uvSpan;

    for (i = drawGroup; i < chainPriority; ++i)
    {
        enemy = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x9DCEDC + i * 4);
        while (enemy != NULL)
        {
            vm = reinterpret_cast<AnmVm *>(enemy + 0x2B0);
            for (k = 0; k < 1; ++k, ++vm)
            {
                if (vm->scriptIndex >= 0)
                {
                    if (vm->type)
                        vm->SetZRotation(*reinterpret_cast<f32 *>(enemy + 0x2D94));

                    if (((*reinterpret_cast<u32 *>(enemy + 0x3328) >> 8) & 1) == 0)
                        vm->pos = *reinterpret_cast<Float3 *>(enemy + 0x2D88) + vm->pos2;
                    else
                        vm->pos = *reinterpret_cast<Float3 *>(enemy + 0x2D88) +
                                  *reinterpret_cast<Float3 *>(enemy + 0x294);

                    vm->pos.z = 0.3f;
                    vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                    vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                    g_AnmManager->Draw2D(vm);
                }
            }

            if (((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 25) & 1) != 0)
                reinterpret_cast<AnmVm *>(enemy + 0xC)->SetZRotation(*reinterpret_cast<f32 *>(enemy + 0x2D94));

            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos =
                *reinterpret_cast<Float3 *>(enemy + 0x2D88) + *reinterpret_cast<Float3 *>(enemy + 0x294);
            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.z = 0.25f;

            if (*reinterpret_cast<u8 *>(enemy + 0x534C))
            {
                *reinterpret_cast<Float2 *>(&savedScaleX) = reinterpret_cast<AnmVm *>(enemy + 0xC)->scale;
                savedColor = reinterpret_cast<AnmVm *>(enemy + 0xC)->color1.d3dColor;

                if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 8) == 0)
                {
                    for (k = *reinterpret_cast<i16 *>(enemy + 0x534E) - 1; k > 0;
                         k -= *reinterpret_cast<i16 *>(enemy + 0x5352))
                    {
                        if (*reinterpret_cast<f32 *>(enemy + 0x3394 + k * 0x1C) < -990.0f)
                            continue;

                        if (((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 25) & 1) != 0)
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->SetZRotation(
                                    *reinterpret_cast<f32 *>(enemy + 0x33AC + k * 0x1C));

                            if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 2) != 0)
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->scale.x =
                                    savedScaleX - (f32)k * savedScaleX /
                                                      (f32)*reinterpret_cast<i16 *>(enemy + 0x534E);

                            if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 4) != 0)
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->color1.a =
                                    reinterpret_cast<u8 *>(&savedColor)[3] -
                                    reinterpret_cast<u8 *>(&savedColor)[3] * k /
                                        *reinterpret_cast<i16 *>(enemy + 0x534E);

                            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos =
                                *reinterpret_cast<Float3 *>(enemy + 0x3394 + k * 0x1C) +
                                *reinterpret_cast<Float3 *>(enemy + 0x294);
                            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.z = 0.3f;
                            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                        g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(enemy + 0xC));
                    }
                }
                else
                {
                    vertexCount = 0;
                    for (k = 0; k < *reinterpret_cast<i16 *>(enemy + 0x534E);
                         k += *reinterpret_cast<i16 *>(enemy + 0x5352))
                    {
                        if (*reinterpret_cast<f32 *>(enemy + 0x3394 + k * 0x1C) < -990.0f)
                            break;
                        vertexCount += 2;
                    }

                    if (vertexCount > 2)
                    {
                        uvSpan = reinterpret_cast<AnmVm *>(enemy + 0xC)->loadedSprite->uvEnd.x -
                                 reinterpret_cast<AnmVm *>(enemy + 0xC)->loadedSprite->uvStart.x;
                        uvStep = uvSpan / ((vertexCount + 1) / 2 - 1);
                        uv = reinterpret_cast<AnmVm *>(enemy + 0xC)->loadedSprite->uvEnd.x +
                             reinterpret_cast<AnmVm *>(enemy + 0xC)->uvScrollPos.x;
                        vertices = reinterpret_cast<VertexTex1DiffuseXyzrhw *>(enemy + 0x3E14);

                        for (k = 0; k < *reinterpret_cast<i16 *>(enemy + 0x534E);
                             k += *reinterpret_cast<i16 *>(enemy + 0x5352), uv -= uvStep)
                        {
                            if (*reinterpret_cast<f32 *>(enemy + 0x3394 + k * 0x1C) < -990.0f)
                                break;

                            if (k == 0)
                            {
                                angle = *reinterpret_cast<f32 *>(enemy + 0x33AC);
                            }
                            else
                            {
                                angle = FUN_0042eb10(
                                    *reinterpret_cast<f32 *>(enemy + 0x33AC + (k - 1) * 0x1C),
                                    *reinterpret_cast<f32 *>(enemy + 0x33AC + k * 0x1C), 0.5f);
                            }

                            if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 2) != 0 && k > 0 &&
                                k + *reinterpret_cast<i16 *>(enemy + 0x5352) <
                                    *reinterpret_cast<i16 *>(enemy + 0x534E))
                            {
                                sinAngle = FUN_0042eb10(
                                    *reinterpret_cast<f32 *>(
                                        enemy + 0x33AC +
                                        (k + *reinterpret_cast<i16 *>(enemy + 0x5352) - 1) * 0x1C),
                                    *reinterpret_cast<f32 *>(
                                        enemy + 0x33AC + *reinterpret_cast<i16 *>(enemy + 0x5352) * 0x1C),
                                    0.5f);
                                if (fabsf(previousAngle - angle) < 0.00001f &&
                                    fabsf(angle - sinAngle) < 0.00001f)
                                {
                                    vertexCount -= 2;
                                    continue;
                                }
                            }

                            previousAngle = angle;
                            sinAngle = sinf(angle);
                            cosAngle = cosf(angle);
                            halfCenter = 0.0f;
                            halfWidth = savedScaleY *
                                        reinterpret_cast<AnmVm *>(enemy + 0xC)->loadedSprite->heightPx / 2.0f;
                            if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 2) != 0)
                            {
                                angle = 1.0f - (f32)k / (f32)*reinterpret_cast<i16 *>(enemy + 0x534E);
                                halfCenter *= angle;
                                halfWidth *= angle;
                            }

                            vertices[1].diffuse = reinterpret_cast<AnmVm *>(enemy + 0xC)->color1.d3dColor;
                            vertices[0].diffuse = vertices[1].diffuse;
                            if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 4) != 0)
                            {
                                reinterpret_cast<u8 *>(&vertices[1].diffuse)[3] =
                                    reinterpret_cast<u8 *>(&savedColor)[3] -
                                    reinterpret_cast<u8 *>(&savedColor)[3] * k /
                                        *reinterpret_cast<i16 *>(enemy + 0x534E);
                                reinterpret_cast<u8 *>(&vertices[0].diffuse)[3] =
                                    reinterpret_cast<u8 *>(&vertices[1].diffuse)[3];
                            }

                            vertices[0].pos = *reinterpret_cast<Float3 *>(enemy + 0x3394 + k * 0x1C);
                            vertices[0].pos.x += cosAngle * halfCenter - sinAngle * halfWidth + 32.0f;
                            vertices[0].pos.y += sinAngle * halfCenter + cosAngle * halfWidth + 16.0f;
                            vertices[0].textureUV.x = uv;
                            vertices[0].textureUV.y =
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->loadedSprite->uvStart.y +
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->uvScrollPos.y;
                            ++vertices;

                            vertices[0].pos = *reinterpret_cast<Float3 *>(enemy + 0x3394 + k * 0x1C);
                            vertices[0].pos.x += cosAngle * halfCenter + sinAngle * halfWidth + 32.0f;
                            vertices[0].pos.y += sinAngle * halfCenter - cosAngle * halfWidth + 16.0f;
                            vertices[0].textureUV.x = uv;
                            vertices[0].textureUV.y =
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->loadedSprite->uvEnd.y +
                                reinterpret_cast<AnmVm *>(enemy + 0xC)->uvScrollPos.y;
                            ++vertices;
                        }

                        if (vertexCount > 2)
                            g_AnmManager->DrawVertices(
                                reinterpret_cast<AnmVm *>(enemy + 0xC),
                                reinterpret_cast<VertexTex1DiffuseXyzrhw *>(enemy + 0x3E14), vertexCount);
                    }
                }

                reinterpret_cast<AnmVm *>(enemy + 0xC)->scale = *reinterpret_cast<Float2 *>(&savedScaleX);
                reinterpret_cast<AnmVm *>(enemy + 0xC)->color1.d3dColor = savedColor;
            }

            if ((*reinterpret_cast<u8 *>(enemy + 0x534C) & 0x10) == 0 &&
                ((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 5) & 1) == 0)
            {
                g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(enemy + 0xC));
            }

            for (k = 1; k < 2; ++k, ++vm)
            {
                if (vm->scriptIndex >= 0)
                {
                    if (vm->type)
                        vm->SetZRotation(-*reinterpret_cast<f32 *>(enemy + 0x2D94));

                    if (((*reinterpret_cast<u32 *>(enemy + 0x3328) >> 8) & 1) == 0)
                        vm->pos = *reinterpret_cast<Float3 *>(enemy + 0x2D88) + vm->pos2;
                    else
                        vm->pos = *reinterpret_cast<Float3 *>(enemy + 0x2D88) +
                                  *reinterpret_cast<Float3 *>(enemy + 0x294);

                    vm->pos.z = 0.3f;
                    vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                    vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                    g_AnmManager->Draw2D(vm);
                }
            }

            enemy = *reinterpret_cast<u8 **>(enemy);
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x42eb10
#pragma var_order(wrapDelta, shortDelta)
f32 __stdcall FUN_0042eb10(f32 angle1, f32 angle2, f32 factor)
{
    f32 shortDelta;
    f32 wrapDelta;

    if (angle1 < angle2)
    {
        shortDelta = angle2 - angle1;
        wrapDelta = angle1 + ZUN_2PI - angle2;
    }
    else
    {
        shortDelta = angle1 - angle2;
        wrapDelta = angle2 + ZUN_2PI - angle1;
        angle1 = angle2;
    }

    if (shortDelta < wrapDelta)
        return shortDelta * factor + angle1;
    return wrapDelta * factor + angle1;
}

// FUNCTION: th08 0x42eb90
ChainCallbackResult EnemyManager::OnDrawLowPrio(EnemyManager *enemyManager)
{
    ChainCallbackResult result;

    if (g_GameManager.flags.unk10)
    {
        g_AnmManager->SetMixColor(0xfff01010);
    }

    result = enemyManager->OnDrawImpl(2, 4);

    if (g_GameManager.flags.unk10)
    {
        g_AnmManager->SetMixColorDefault();
    }

    return result;
}

// FUNCTION: th08 0x42ebf0
#pragma var_order(enemy, savedEcl0, savedEcl1, markerPosition, enemyManager)
ZunResult EnemyManager::AddedCallback(EnemyManager *enemyManager)
{
    Enemy *enemy = reinterpret_cast<Enemy *>(reinterpret_cast<u8 *>(enemyManager) + 0x53D0);
    i32 savedEcl0;
    i32 savedEcl1;

    if (IsResourceReloadEnabled())
    {
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEEC) =
            g_AnmManager->PreloadAnm(7, "enemy.anm");
        if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEEC) == NULL)
        {
            return ZUN_ERROR;
        }
    }
    else
    {
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEEC) = g_AnmManager->GetAnm(7);
    }

    if (!IsDisableResourceReload())
    {
        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBAC) >> 14) & 1) == 0 ||
            *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0) < 0xCD)
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) =
                g_AnmManager->PreloadAnm(8, g_StageEnemyAnms[g_GameManager.currentStage]);
            if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) == NULL)
            {
                return ZUN_ERROR;
            }
        }
        else
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) =
                g_AnmManager->PreloadAnm(
                    8, g_SpellEnemyAnms[
                           *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0) - 0xCD]);
            if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) == NULL)
            {
                return ZUN_ERROR;
            }
        }
    }
    else
    {
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) = g_AnmManager->GetAnm(8);
    }

    if (!IsDisableResourceReload())
    {
#ifdef TH08_MODERN_PORT
        memset(&g_EclManager, 0, sizeof(g_EclManager));
        memset(&EclRunLowProposal::g_EclCallParameters, 0,
               sizeof(EclRunLowProposal::g_EclCallParameters));
#else
        memset(&g_EclManager, 0,
               sizeof(g_EclManager) + sizeof(EclRunLowProposal::g_EclCallParameters));
#endif
        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBAC) >> 14) & 1) == 0)
        {
            if (g_EclManager.Load(const_cast<char *>(g_StageEclFiles[g_GameManager.currentStage])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
        else if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0) >= 0xCD)
        {
            if (g_EclManager.Load(const_cast<char *>(g_SpellEclFiles[
                    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0) - 0xCD])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
        else
        {
            if (g_EclManager.Load(const_cast<char *>(g_StageSpellEclFiles[g_GameManager.currentStage])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
    }
    else
    {
        savedEcl0 = reinterpret_cast<i32 *>(&g_EclManager)[0];
        savedEcl1 = reinterpret_cast<i32 *>(&g_EclManager)[1];
#ifdef TH08_MODERN_PORT
        memset(&g_EclManager, 0, sizeof(g_EclManager));
        memset(&EclRunLowProposal::g_EclCallParameters, 0,
               sizeof(EclRunLowProposal::g_EclCallParameters));
#else
        memset(&g_EclManager, 0,
               sizeof(g_EclManager) + sizeof(EclRunLowProposal::g_EclCallParameters));
#endif
        reinterpret_cast<i32 *>(&g_EclManager)[0] = savedEcl0;
        reinterpret_cast<i32 *>(&g_EclManager)[1] = savedEcl1;
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCDC0) = g_Rng.GetRandomU16InRange(3);
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCDC2) = g_Rng.GetRandomU16InRange(8);
    D3DXVECTOR3 markerPosition(-999.0f, -999.0f, -999.0f);
    g_AsciiManager.SetBossMarkerPosition(0, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(1, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(2, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(3, &markerPosition);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42ee80
#pragma var_order(i, enemy, markerPosition, enemyManager)
ZunResult EnemyManager::DeletedCallback(EnemyManager *enemyManager)
{
    Enemy *enemy = reinterpret_cast<Enemy *>(reinterpret_cast<u8 *>(enemyManager) + 0x53D0);
    i32 i = 0;

    for (; i < 0x1E0; ++i, enemy = reinterpret_cast<Enemy *>(reinterpret_cast<u8 *>(enemy) + 0x53D0))
    {
        enemy->FUN_0042bc90();
    }

    if (!IsDisableResourceReload())
    {
        g_AnmManager->ReleaseAnm(8);
    }
    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(7);
    }
    if (!IsDisableResourceReload())
    {
        g_EclManager.Unload();
    }

    D3DXVECTOR3 markerPosition(-999.0f, -999.0f, -999.0f);
    g_AsciiManager.SetBossMarkerPosition(0, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(1, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(2, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(3, &markerPosition);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42ef70
void EnemyManager::CutChain()
{
    g_Chain.Cut(&g_EnemyManagerCalcChain);
    g_Chain.Cut(&g_EnemyManagerDrawChainHighPrio);
    g_Chain.Cut(&g_EnemyManagerDrawChainLowPrio);
}

// FUNCTION: th08 0x42efb0
#pragma var_order(score, totalScore, enemy, enemyIndex, itemIndex, this)
i32 EnemyManager::FUN_0042efb0(i32 maxScore, i32 initialScore)
{
    i32 itemIndex;
    i32 enemyIndex;
    u8 *enemy;
    i32 totalScore;
    i32 score;

    enemy = reinterpret_cast<u8 *>(this) + 0x53D0;
    totalScore = initialScore;
    score = 2000;
    for (enemyIndex = 0; enemyIndex < 480; enemyIndex++, enemy += 0x53D0)
    {
        if ((*reinterpret_cast<u32 *>(enemy + 0x3324) & 1) == 0)
        {
            continue;
        }
        if (((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 1) & 1) != 0)
        {
            continue;
        }
        if (((*reinterpret_cast<u32 *>(enemy + 0x3328) >> 6) & 1) != 0)
        {
            continue;
        }

        reinterpret_cast<Enemy *>(enemy)->life = 0;
        if (((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 7) & 1) != 0)
        {
            *reinterpret_cast<Float3 *>(enemy + 0x2D88) =
                *reinterpret_cast<Float3 *>(enemy + 0x2D34) + *reinterpret_cast<Float3 *>(enemy + 0x2D40);
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(enemy + 0x2D88), ITEM_POINT_STAR,
                                    ITEM_STATE_AUTOCOLLECT);
            g_AsciiManager.CreateScorePopup(reinterpret_cast<Float3 *>(enemy + 0x2D88), score,
                                            score >= maxScore ? -256 : -1);
            totalScore += score;
            score += 30;
            if (score > maxScore)
            {
                score = maxScore;
            }

            if (*reinterpret_cast<u8 *>(enemy + 0x534C) != 0)
            {
                for (itemIndex = 0; itemIndex < *reinterpret_cast<i16 *>(enemy + 0x534E); itemIndex += 6)
                {
                    g_ItemManager.SpawnItem(
                        reinterpret_cast<Float3 *>(enemy + itemIndex * 0x1C + 0x3394), ITEM_POINT_STAR,
                        ITEM_STATE_AUTOCOLLECT);
                    g_AsciiManager.CreateScorePopup(
                        reinterpret_cast<Float3 *>(enemy + itemIndex * 0x1C + 0x3394), score,
                        score >= maxScore ? -256 : -1);
                    totalScore += score;
                    score += 30;
                    if (score > maxScore)
                    {
                        score = maxScore;
                    }
                }
            }
        }

        reinterpret_cast<Enemy *>(enemy)->FUN_0042b2f0();
        if (*reinterpret_cast<i16 *>(enemy + 0x2CEE) >= 0)
        {
            g_EclManager.CallEclSub(
                reinterpret_cast<EnemyEclContext *>(enemy + 0x7F8), *reinterpret_cast<i16 *>(enemy + 0x2CEE));
            *reinterpret_cast<i16 *>(enemy + 0x2CEE) = -1;
        }
    }

    return totalScore;
}

// FUNCTION: th08 0x42f1f0
i32 EnemyManager::FUN_0042f1f0()
{
    i32 i;
    for (i = 0; i < 8; i++)
    {
        if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0x9DCDA0 + i * 4) != NULL)
            return 1;
    }
    return 0;
}

// FUNCTION: th08 0x449f50
EclTimeline::EclTimeline() {}

} /* namespace th08 */
