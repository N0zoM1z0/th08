#include "th_pch.h"

#include "EnemyManager.hpp"

namespace th08
{

DIFFABLE_STATIC(EnemyManager, g_EnemyManager);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainLowPrio);

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

// STUB: th08 0x42ebf0
ZunResult EnemyManager::AddedCallback(EnemyManager *enemyManager)
{
    return ZUN_ERROR;
}

// STUB: th08 0x42ee80
ZunResult EnemyManager::DeletedCallback(EnemyManager *enemyManager)
{
    return ZUN_ERROR;
}

// FUNCTION: th08 0x42ef70
void EnemyManager::CutChain()
{
    g_Chain.Cut(&g_EnemyManagerCalcChain);
    g_Chain.Cut(&g_EnemyManagerDrawChainHighPrio);
    g_Chain.Cut(&g_EnemyManagerDrawChainLowPrio);
}

} /* namespace th08 */
