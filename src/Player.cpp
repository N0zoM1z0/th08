#include "th_pch.h"

#include "Player.hpp"

namespace th08
{

DIFFABLE_STATIC(Player, g_Player);
DIFFABLE_STATIC(ChainElem *, g_PlayerCalcChain);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainLowPrio);

// FUNCTION: th08 0x44e0e0
ZunBool IsResourceReloadDisabled()
{
    return !IsResourceReloadEnabled();
}

/* The target emits these cross-subsystem definitions in the Player translation unit. */
// FUNCTION: th08 0x44e0f0
void AnmVmBase::SetBlendModeAdditive()
{
    this->blendMode = AnmBlendMode_Additive;
}

// FUNCTION: th08 0x44e120
void AnmVmBase::SetBlendModeNormal()
{
    this->blendMode = AnmBlendMode_Normal;
}

// FUNCTION: th08 0x44e140
void GameManager::SetYoukaiGauge(u16 value)
{
    this->globals->youkaiGauge = value;
}

// FUNCTION: th08 0x44e350
void PlayerUnkStruct0x40::Deactivate()
{
    this->active = false;
}

// FUNCTION: th08 0x44e370
void PlayerUnkStruct0x40::Reset()
{
    memset(this, 0, sizeof(*this));
    this->collisionInterval = 1;
}

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
