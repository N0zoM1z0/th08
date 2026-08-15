#include "th_pch.h"

#include "BulletManager.hpp"

namespace th08
{

DIFFABLE_STATIC(BulletManager, g_BulletManager);
DIFFABLE_STATIC(ChainElem, g_BulletManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_BulletManagerDrawChain);
DIFFABLE_STATIC(i32, g_ResourceReloadEnabled);
DIFFABLE_STATIC(i32, g_BulletManagerAnmReleaseRequired);

// FUNCTION: th08 0x4338b0
i32 IsResourceReloadEnabled()
{
    return g_ResourceReloadEnabled;
}

// FUNCTION: th08 0x4338c0
i32 IsBulletManagerAnmReleaseRequired()
{
    return g_BulletManagerAnmReleaseRequired;
}

// FUNCTION: th08 0x415c60
void BulletManager::bulletmanager_fun_00415c60()
{
    this->RemoveAllBullets(1);
}

// STUB: th08 0x430830
void BulletManager::RemoveAllBullets(i32 mode)
{
}

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

// FUNCTION: th08 0x433820
ZunResult BulletManager::DeletedCallback(BulletManager *bulletManager)
{
    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(6);
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x433850
void BulletManager::CutChain()
{
    g_Chain.Cut(&g_BulletManagerCalcChain);
    g_Chain.Cut(&g_BulletManagerDrawChain);
}

} /* namespace th08 */
