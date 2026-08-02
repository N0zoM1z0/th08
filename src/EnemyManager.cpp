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

// STUB: th08 0x42c590
ZunResult EnemyManager::RegisterChain()
{
    return ZUN_ERROR;
}

// STUB: th08 0x42c660
ChainCallbackResult EnemyManager::OnUpdate()
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x42e120
ChainCallbackResult EnemyManager::OnDrawHighPrio(EnemyManager *enemyManager)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x42e140
ChainCallbackResult EnemyManager::OnDrawImpl()
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x42eb90
ChainCallbackResult EnemyManager::OnDrawLowPrio(EnemyManager *enemyManager)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
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

// STUB: th08 0x42ef70
void EnemyManager::CutChain()
{
}

} /* namespace th08 */
