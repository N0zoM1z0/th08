#include "th_pch.h"

#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "Player.hpp"

namespace th08
{

DIFFABLE_STATIC(Player, g_Player);
DIFFABLE_STATIC(ChainElem *, g_PlayerCalcChain);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem *, g_PlayerDrawChainLowPrio);
// The callback releases and clears these two independently allocated SHT files.
DIFFABLE_STATIC(PlayerRawShtFile *, g_PlayerPrimaryShtFile);
DIFFABLE_STATIC(PlayerRawShtFile *, g_PlayerSecondaryShtFile);

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

#pragma var_order(primaryShtFile, player, secondaryShtFile)
// FUNCTION: th08 0x44c230
ZunResult Player::RegisterChain(u32 playerType)
{
    Player *player = &g_Player;
    PlayerRawShtFile *secondaryShtFile;
    PlayerRawShtFile *primaryShtFile;

    if (IsResourceReloadDisabled())
    {
        primaryShtFile = player->primaryShtFile;
        secondaryShtFile = player->secondaryShtFile;
    }

    memset(player, 0, sizeof(*player));

    if (IsResourceReloadDisabled())
    {
        player->primaryShtFile = primaryShtFile;
        player->secondaryShtFile = secondaryShtFile;
    }

    player->timer = 0;
    player->playerType = playerType;

    player->calcChain = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->calcChain->arg = player;
    player->calcChain->addedCallback = (ChainLifetimeCallback)Player::AddedCallback;
    player->calcChain->deletedCallback = (ChainLifetimeCallback)Player::DeletedCallback;
    if (g_Chain.AddToCalcChain(player->calcChain, 9))
        return ZUN_ERROR;

    player->drawChainHighPrio = g_Chain.CreateElem((ChainCallback)Player::OnDrawHighPrio);
    player->drawChainLowPrio = g_Chain.CreateElem((ChainCallback)Player::OnDrawLowPrio);
    player->drawChainHighPrio->arg = player;
    player->drawChainLowPrio->arg = player;
    g_Chain.AddToDrawChain(player->drawChainHighPrio, 9);
    g_Chain.AddToDrawChain(player->drawChainLowPrio, 10);

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

// FUNCTION: th08 0x44d2c0
void Player::FUN_0044d2c0()
{
    if (this->playerStateSlotCooldown != 0)
    {
        this->playerStateSlotCooldown--;
        this->FUN_0044de60(&this->position, 768.0f, 896.0f, -1, 0);
    }

    if (this->playerState == PLAYER_STATE_DEAD)
    {
        this->stateFlag = false;

        if (this->stateEffect != NULL)
            this->stateEffect->position = this->position;

        this->timer--;
        if ((i32)this->timer <= 0)
        {
            if (this->stateEffect != NULL)
            {
                this->stateEffect->active = false;
                this->stateEffect = NULL;
            }

            this->playerState = PLAYER_STATE_ALIVE;
            this->timer = 0;
            this->stateColor = -1;
        }
        else if ((i32)this->timer % 8 < 2)
        {
            this->stateColor = 0xfff02020;
        }
        else
        {
            this->stateColor = -1;
        }
    }
    else
    {
        this->timer++;
    }
}

// STUB: th08 0x44d650
ZunResult Player::AddedCallback(Player *player)
{
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x44dc60
ZunResult Player::DeletedCallback(Player *player)
{
    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(5);
        g_AsciiManager.SetGaugeInterrupt(99);
        g_AsciiManager.FUN_00422bb0(0, 99);
        g_AsciiManager.FUN_00422bb0(1, 99);
        g_AsciiManager.FUN_00422bb0(2, 99);

        if (g_PlayerPrimaryShtFile != NULL)
        {
            g_ZunMemory.Free(g_PlayerPrimaryShtFile);
            g_PlayerPrimaryShtFile = NULL;
        }

        if (g_PlayerSecondaryShtFile != NULL)
        {
            g_ZunMemory.Free(g_PlayerSecondaryShtFile);
            g_PlayerSecondaryShtFile = NULL;
        }
    }

    return ZUN_SUCCESS;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44de60
PlayerUnkStruct0x40 *Player::FUN_0044de60(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->size.x = value1;
    slot->size.y = value2;
    slot->lifetime = value4;
    slot->collisionValue = value3;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44df00
PlayerUnkStruct0x40 *Player::FUN_0044df00(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->radius = value1;
    slot->radiusGrowth = value2;
    slot->lifetime = value3;
    slot->collisionValue = value4;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44dfa0
PlayerUnkStruct0x40 *Player::FUN_0044dfa0(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->size.x = value1;
    slot->size.y = value2;
    slot->lifetime = value4;
    slot->damage = value3;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44e040
PlayerUnkStruct0x40 *Player::FUN_0044e040(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->radius = value1;
    slot->radiusGrowth = value2;
    slot->lifetime = value4;
    slot->damage = value3;

    return slot;
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
