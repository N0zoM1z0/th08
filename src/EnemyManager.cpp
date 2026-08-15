#include "th_pch.h"

#include "EnemyManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();
i32 IsResourceReloadEnabled();

DIFFABLE_STATIC(EnemyManager, g_EnemyManager);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainLowPrio);

// STUB: th08 0x42bc90
void Enemy::FUN_0042bc90()
{
}

// STUB: th08 0x429e00
void EnemyManager::Initialize()
{
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
