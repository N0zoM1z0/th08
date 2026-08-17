#include "th_pch.h"

#include "EnemyManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ItemManager.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();
i32 IsResourceReloadEnabled();

DIFFABLE_STATIC(EnemyManager, g_EnemyManager);

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

// FUNCTION: th08 0x42e120
ChainCallbackResult EnemyManager::OnDrawHighPrio(EnemyManager *enemyManager)
{
    return enemyManager->OnDrawImpl(0, 2);
}

// STUB: th08 0x42e140
ChainCallbackResult __fastcall EnemyManager::OnDrawImpl(i32 drawGroup, i32 chainPriority)
{
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
