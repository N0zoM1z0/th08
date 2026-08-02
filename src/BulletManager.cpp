#include "th_pch.h"

#include "BulletManager.hpp"

namespace th08
{

DIFFABLE_STATIC(BulletManager, g_BulletManager);
DIFFABLE_STATIC(ChainElem, g_BulletManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_BulletManagerDrawChain);

// STUB: th08 0x42f360
void BulletManager::Initialize()
{
}

// STUB: th08 0x4311a0
ZunResult BulletManager::RegisterChain()
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x431240
ChainCallbackResult BulletManager::OnUpdate(BulletManager *bulletManager)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x432b50
ChainCallbackResult BulletManager::OnDraw(BulletManager *bulletManager)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x433070
ZunResult BulletManager::AddedCallback(BulletManager *bulletManager)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x433820
ZunResult BulletManager::DeletedCallback(BulletManager *bulletManager)
{
    return ZUN_SUCCESS;
}

} /* namespace th08 */
