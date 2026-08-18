#include "th_pch.h"

#include "EnemyManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();
i32 IsResourceReloadEnabled();

DIFFABLE_STATIC(EnemyManager, g_EnemyManager);
DIFFABLE_STATIC(u16, g_EnemyDropCounter);
DIFFABLE_STATIC(ItemTimeOrbTimerStorage, g_EnemyAttachedTimer30);
DIFFABLE_STATIC(ItemTimeOrbTimerStorage, g_EnemyAttachedTimer0);
DIFFABLE_STATIC(u16, g_EnemyDropScheduleIndex);
DIFFABLE_STATIC_ARRAY_ASSIGN(u8, 32, g_EnemyDropSchedule) = {
    0, 0, 1, 0, 1, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 0, 1, 1, 1, 0, 0,
};

// FUNCTION: th08 0x42bc50
void __fastcall FUN_0042bc50(void *self)
{
    *reinterpret_cast<u32 *>(self) &= ~4u;
    *reinterpret_cast<u32 *>(self) |= 8u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(self) + 0xfc) = 0;
}

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

// FUNCTION: th08 0x42a4c0
EnemyUnkStruct3::EnemyUnkStruct3() {}

// FUNCTION: th08 0x42a450
EnemyUnkStruct2::EnemyUnkStruct2() {}

// FUNCTION: th08 0x42a490
EnemyUnkStruct0x1c::EnemyUnkStruct0x1c() {}

// FUNCTION: th08 0x42a280
Enemy::Enemy() {}

// FUNCTION: th08 0x449f50
EclTimeline::EclTimeline() {}

// FUNCTION: th08 0x42a210
EnemyManager::EnemyManager()
{
    this->Initialize();
}

namespace EclOperands
{

// FUNCTION: th08 0x0041F000
i32 TargetEnemyHelpersOverlay::HasParentChain()
{
    return *(void **)((u8 *)this + 0x2DA4) == 0 &&
           *(void **)((u8 *)this + 8) != 0;
}

// FUNCTION: th08 0x0041FD20
i32 TargetEnemyHelpersOverlay::HasAttachedEnemy()
{
    return *(void **)((u8 *)this + 0x2DA4) != 0;
}

// FUNCTION: th08 0x0041FD40
i32 TargetEnemyHelpersOverlay::CountParentChain()
{
    TargetEnemyHelpersOverlay *cursor = this;
    i32 count = 0;
    if (this->HasParentChain())
    {
        while (*(void **)((u8 *)cursor + 8) != 0)
        {
            cursor = *(TargetEnemyHelpersOverlay **)((u8 *)cursor + 8);
            count++;
        }
    }
    return count;
}

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

                if (g_Player.bombState.frameStop != 0)
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
            *reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage) = 0;
        }
    }

    if (reinterpret_cast<TargetEnemyHelpersOverlay *>(this)->HasAttachedEnemy() && mode != 0)
    {
        Float3 attachedPosition;
        g_GameManager.AddToYoukaiGauge(-g_GameManager.GetYoukaiGauge() / 12, 0);
        *reinterpret_cast<ZunTimer *>(&g_EnemyAttachedTimer0) = 0;
        *reinterpret_cast<ZunTimer *>(&g_EnemyAttachedTimer30) = 30;
        *reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage) = 50;
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

// FUNCTION: th08 0x415c80
void Enemy::enemy_fun_00415c80()
{
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2DEC) = -0.5f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2DF0) = 0.5f;
    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2DF4) = 0;
    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2DF6) = 0;
    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2DF8) = 0;
    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(this) + 0x2DFA) = 0;
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

        *reinterpret_cast<i32 *>(enemy + 0x2DFC) = 0;
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
            reinterpret_cast<EclManager *>(0x4ECCB8)->CallEclSub(
                reinterpret_cast<EnemyEclContext *>(enemy + 0x7F8), *reinterpret_cast<i16 *>(enemy + 0x2CEE));
            *reinterpret_cast<i16 *>(enemy + 0x2CEE) = -1;
        }
    }

    return totalScore;
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
    *reinterpret_cast<ZunTimer *>(enemy + 0x2E14) = 0;
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
    *reinterpret_cast<i32 *>(enemy + 0x2DFC) = 1;
    *reinterpret_cast<i32 *>(enemy + 0x2E08) = 100;
    *reinterpret_cast<u8 *>(enemy + 0x3310) = 0;
    *reinterpret_cast<u8 *>(enemy + 0x3311) = 0;
    *reinterpret_cast<u8 *>(enemy + 0x3312) = 0;
    *reinterpret_cast<i32 *>(enemy + 0x3060) = 0;
    *reinterpret_cast<ZunTimer *>(enemy + 0x3064) = 0;
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
    *reinterpret_cast<u32 *>(enemy + 0x2DEC) = 0xBE19999A;
    *reinterpret_cast<u32 *>(enemy + 0x2DF0) = 0x3E19999A;
    *reinterpret_cast<i32 *>(enemy + 0x3024) = 7;
    *reinterpret_cast<i32 *>(enemy + 0x3028) = 25;
    *reinterpret_cast<u32 *>(enemy + 0x3350) = 0x44800000;
    *reinterpret_cast<i32 *>(enemy + 0x2E10) = *reinterpret_cast<i32 *>(0x18B8A24);
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

// STUB: th08 0x42c660
ChainCallbackResult EnemyManager::OnUpdate()
{
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
                    vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
                    vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
                    g_AnmManager->Draw2D(vm);
                }
            }

            if (((*reinterpret_cast<u32 *>(enemy + 0x3324) >> 25) & 1) != 0)
                reinterpret_cast<AnmVm *>(enemy + 0xC)->SetZRotation(*reinterpret_cast<f32 *>(enemy + 0x2D94));

            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos =
                *reinterpret_cast<Float3 *>(enemy + 0x2D88) + *reinterpret_cast<Float3 *>(enemy + 0x294);
            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
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
                            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
                            reinterpret_cast<AnmVm *>(enemy + 0xC)->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
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
                    vm->pos.x += g_ItemAnmManagerScreenShakeOffset.x;
                    vm->pos.y += g_ItemAnmManagerScreenShakeOffset.y;
                    g_AnmManager->Draw2D(vm);
                }
            }

            enemy = *reinterpret_cast<u8 **>(enemy);
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
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
                g_AnmManager->PreloadAnm(8, reinterpret_cast<const char **>(0x4C7364)[g_GameManager.currentStage]);
            if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) == NULL)
            {
                return ZUN_ERROR;
            }
        }
        else
        {
            *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(enemyManager) + 0x9DCEF0) =
                g_AnmManager->PreloadAnm(
                    8, reinterpret_cast<const char **>(
                           0x4C7054)[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0)]);
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
        memset(reinterpret_cast<void *>(0x4ECCB8), 0, 0x188);
        if (((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBAC) >> 14) & 1) == 0)
        {
            if (reinterpret_cast<EclManager *>(0x4ECCB8)->Load(
                    const_cast<char *>(reinterpret_cast<const char **>(0x4C73CC)[g_GameManager.currentStage])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
        else if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0) >= 0xCD)
        {
            if (reinterpret_cast<EclManager *>(0x4ECCB8)->Load(
                    const_cast<char *>(reinterpret_cast<const char **>(
                        0x4C70E4)[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBB0)])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
        else
        {
            if (reinterpret_cast<EclManager *>(0x4ECCB8)->Load(
                    const_cast<char *>(reinterpret_cast<const char **>(0x4C73F0)[g_GameManager.currentStage])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
    }
    else
    {
        savedEcl0 = *reinterpret_cast<i32 *>(0x4ECCB8);
        savedEcl1 = *reinterpret_cast<i32 *>(0x4ECCBC);
        memset(reinterpret_cast<void *>(0x4ECCB8), 0, 0x188);
        *reinterpret_cast<i32 *>(0x4ECCB8) = savedEcl0;
        *reinterpret_cast<i32 *>(0x4ECCBC) = savedEcl1;
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
        reinterpret_cast<EclManager *>(0x4ECCB8)->Unload();
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

} /* namespace th08 */
