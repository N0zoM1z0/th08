#include "th_pch.h"

#include "Player.hpp"

namespace th08
{

DIFFABLE_STATIC(Player, g_Player);
DIFFABLE_STATIC(ChainElem *, g_PlayerCalcChain);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainLowPrio);

// STUB: th08 0x44c230
ZunResult Player::RegisterChain(u32 param)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x44c390
ChainCallbackResult Player::OnUpdate(Player *player)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x44d530
ChainCallbackResult Player::OnDrawHighPrio(Player *player)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x44d630
ChainCallbackResult Player::OnDrawLowPrio(Player *player)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x44d650
ZunResult Player::AddedCallback(Player *player)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x44dc60
ZunResult Player::DeletedCallback(Player *player)
{
    return ZUN_SUCCESS;
}

void Player::CutChain()
{
    g_Chain.Cut(g_PlayerCalcChain);
    g_PlayerCalcChain = NULL;
    g_Chain.Cut(g_PlayerDrawChainHighPrio);
    g_PlayerDrawChainHighPrio = NULL;
    g_Chain.Cut(g_PlayerDrawChainLowPrio);
    g_PlayerDrawChainLowPrio = NULL;
}

// STUB: th08 0x44dd70
ZunResult Player::LoadShtFile(PlayerRawShtFile **header, const char *path)
{
    return ZUN_SUCCESS;
}

} /* namespace th08 */
