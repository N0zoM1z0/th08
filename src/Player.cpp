#include "th_pch.h"

#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "AnmManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"

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

// FUNCTION: th08 0x40d3d0
ZunBool ZunTimer::FUN_0040d3d0()
{
    return this->current != this->previous;
}

// FUNCTION: th08 0x40bc40
i32 Player::IsYoukai()
{
    return this->isYoukai;
}

// FUNCTION: th08 0x44c1b0
#pragma var_order(yDelta, xDelta, this)
f32 Player::FUN_0044c1b0(Float3 *position)
{
    f32 yDelta;
    f32 xDelta;

    xDelta = reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4)->operator float *()[0] - position->x;
    yDelta = reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4)->operator float *()[1] - position->y;

    if (yDelta == 0.0f && xDelta == 0.0f)
    {
        return ZUN_PI / 2.0f;
    }

    return VectorAngle(yDelta, xDelta);
}

// STUB: th08 0x449ff0
i32 Player::FUN_00449ff0(Float3 *position, Float3 *position2)
{
    return 0;
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


// STUB: th08 0x44c650
void Player::FUN_0044c650()
{
}
// STUB: th08 0x44cbf0
i32 Player::FUN_0044cbf0()
{
    return 0;
}
// STUB: th08 0x44d180
void Player::FUN_0044d180()
{
}
// STUB: th08 0x44aec0
void Player::FUN_0044aec0()
{
}
// FUNCTION: th08 0x451150
#pragma var_order(i, slot, this)
void Player::FUN_00451150()
{
    u8 *slot;
    i32 i;

    if ((*reinterpret_cast<u32 *>(0x164D0B4) >> 10) & 1)
    {
        return;
    }

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) == 0)
        {
            continue;
        }

        if (*reinterpret_cast<u32 *>(slot + 0x474) != 0)
        {
            if (reinterpret_cast<i32 (__fastcall *)(Player *, u8 *)>(*reinterpret_cast<u32 *>(slot + 0x474))(this, slot) != 0)
            {
                *reinterpret_cast<i16 *>(slot + 0x462) = 0;
                continue;
            }
        }

        reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0] +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(slot + 0x43C);
        reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1] +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(slot + 0x440);

        if (*reinterpret_cast<i16 *>(slot + 0x464) != 4 && *reinterpret_cast<i16 *>(slot + 0x464) != 5)
        {
            if (!g_GameManager.IsWithinPlayfield(
                    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0],
                    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1],
                    *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(slot + 0x224) + 0x34),
                    *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(slot + 0x224) + 0x30)))
            {
                *reinterpret_cast<i16 *>(slot + 0x462) = 0;
            }
        }

        if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(slot)) != ZUN_SUCCESS)
        {
            *reinterpret_cast<i16 *>(slot + 0x462) = 0;
        }
        (*reinterpret_cast<ZunTimer *>(slot + 0x454))++;
    }
}
// FUNCTION: th08 0x451400
#pragma var_order(i, slot, this)
void Player::FUN_00451400()
{
    u8 *slot;
    i32 i;

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 2)
        {
            continue;
        }
        if (*reinterpret_cast<i16 *>(slot + 0x1FC) != 0)
        {
            reinterpret_cast<AnmVm *>(slot)->SetZRotation(*reinterpret_cast<f32 *>(slot + 0x450));
        }
        *reinterpret_cast<f32 *>(slot + 0x208) = g_ItemAnmManagerScreenShakeOffset.x + *reinterpret_cast<f32 *>(slot + 0x2A4);
        *reinterpret_cast<f32 *>(slot + 0x20C) = g_ItemAnmManagerScreenShakeOffset.y + *reinterpret_cast<f32 *>(slot + 0x2A8);
        *reinterpret_cast<f32 *>(slot + 0x210) = 0.2f;
        if (*reinterpret_cast<i8 *>(slot + 0x470) != 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x1F2) = 0xff;
            *reinterpret_cast<u8 *>(slot + 0x1F1) = 0x40;
            *reinterpret_cast<u8 *>(slot + 0x1F0) = 0x40;
        }
        g_AnmManager->DrawPlayerBullet(reinterpret_cast<AnmVm *>(slot));
    }
}
// FUNCTION: th08 0x451500
i32 Player::FUN_00451500()
{
    if (*reinterpret_cast<i32 *>(0x164D2C8) < 20)
    {
        return 0;
    }

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
    {
        return 0;
    }

    if (this->FUN_00451d50())
    {
        return 0;
    }

    if (reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4)->FUN_0040d3d0())
    {
        if (*reinterpret_cast<i32 *>(0x17D6ED4) == 0 ||
            (*reinterpret_cast<u8 *>(0x164D0B1) != 1 && *reinterpret_cast<u8 *>(0x164D0B1) != 7 &&
             *reinterpret_cast<u8 *>(0x164D0B1) != 6))
        {
            this->FUN_00450f60((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4));
        }
    }

    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4))++;

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) >= 20)
    {
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = -1;
    }

    if ((*reinterpret_cast<u16 *>(0x164D52C) & 1) != 0)
    {
        if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
        {
            if (!g_Gui.IsDialogPresent())
            {
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = 0;
            }
        }
    }

    if (*reinterpret_cast<i8 *>(this) == 2 || *reinterpret_cast<i8 *>(this) == 1)
    {
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = -1;
    }

    return 0;
}

// FUNCTION: th08 0x44fb70
void __fastcall Player::FUN_0044fb70(u8 *slot, u8 *entry)
{
    if (*reinterpret_cast<i16 *>(entry + 0x20) == 0)
    {
        *reinterpret_cast<Float3 *>(slot + 0x2A4) =
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4);
    }
    else
    {
        *reinterpret_cast<Float3 *>(slot + 0x2A4) = *reinterpret_cast<Float3 *>(
            reinterpret_cast<u8 *>(this) + ((*reinterpret_cast<i16 *>(entry + 0x20) - 1) * 0x2F4) + 0x6B0);
    }

    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0] += *reinterpret_cast<f32 *>(entry + 0x4);
    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1] += *reinterpret_cast<f32 *>(entry + 0x8);
    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[2] = 0.495f;

    *reinterpret_cast<u32 *>(slot + 0x430) = *reinterpret_cast<u32 *>(entry + 0x0C);
    *reinterpret_cast<u32 *>(slot + 0x434) = *reinterpret_cast<u32 *>(entry + 0x10);
    *reinterpret_cast<f32 *>(slot + 0x438) = 1.0f;
    *reinterpret_cast<u32 *>(slot + 0x450) = *reinterpret_cast<u32 *>(entry + 0x14);
    *reinterpret_cast<u32 *>(slot + 0x44C) = *reinterpret_cast<u32 *>(entry + 0x18);
    *reinterpret_cast<f32 *>(slot + 0x43C) = cosf(*reinterpret_cast<f32 *>(entry + 0x14)) * *reinterpret_cast<f32 *>(entry + 0x18);
    *reinterpret_cast<f32 *>(slot + 0x440) = sinf(*reinterpret_cast<f32 *>(entry + 0x14)) * *reinterpret_cast<f32 *>(entry + 0x18);

    *reinterpret_cast<ZunTimer *>(slot + 0x454) = 0;
    *reinterpret_cast<u8 *>(slot + 0x46C) = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3);
    *reinterpret_cast<i16 *>(slot + 0x464) = *reinterpret_cast<i16 *>(entry + 0x22);
    *reinterpret_cast<i16 *>(slot + 0x460) = *reinterpret_cast<i16 *>(entry + 0x1C);
    *reinterpret_cast<i16 *>(slot + 0x46E) = *reinterpret_cast<i16 *>(entry + 0x24);

    if (*reinterpret_cast<i16 *>(entry + 0x26) >= 0)
    {
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(*reinterpret_cast<i16 *>(entry + 0x26)),
                                               *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4));
    }

    reinterpret_cast<AnmLoaded *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xC))
        ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(slot), *reinterpret_cast<i16 *>(entry + 0x24) + 10);

    *reinterpret_cast<u8 *>(slot + 0x470) = 0;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        if (*reinterpret_cast<i16 *>(entry + 0x1E) > 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x470) = 1;
        }
    }
}

// FUNCTION: th08 0x44fd80
#pragma var_order(slot, this)
i32 __fastcall Player::FUN_0044fd80(u8 *slot, i32 value, u8 *entry)
{
    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x450f60
#pragma var_order(i, table, slot, result, entry, this, value)
void __fastcall Player::FUN_00450f60(i32 value)
{
    unsigned __int64 *table;
    u8 *entry;
    u8 *slot;
    i32 result;
    i32 i;

    table = (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
                ? reinterpret_cast<unsigned __int64 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A74) + 0x38)
                : reinterpret_cast<unsigned __int64 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A78) + 0x38);

    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFDC) != 0 &&
        ((*reinterpret_cast<u8 *>(0x164D0B1) == 2 &&
          (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) & 1) != 0) ||
         *reinterpret_cast<u8 *>(0x164D0B1) == 9) &&
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFF4) >= 60)
    {
        table += ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xFE0) & 2) ? 7 : 6);
    }
    else
    {
        while (g_GameManager.GetPower() >= *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(table) + 4))
        {
            table++;
        }
    }

    entry = *reinterpret_cast<u8 **>(table);
    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 0)
        {
            continue;
        }

processEntry:
        if (*reinterpret_cast<u32 *>(entry + 0x28) != 0)
        {
            result = reinterpret_cast<i32 (__fastcall *)(Player *, u8 *, i32, u8 *)>(
                *reinterpret_cast<u32 *>(entry + 0x28))(this, slot, value, entry);
        }
        else
        {
            result = this->FUN_0044fd80(slot, value, entry);
        }

        if (result == 1)
        {
            *reinterpret_cast<u32 *>(slot + 0x1F8) |= 0x2000;
            *reinterpret_cast<i16 *>(slot + 0x462) = 1;
            *reinterpret_cast<u8 **>(slot + 0x480) = entry;
            *reinterpret_cast<u32 *>(slot + 0x474) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x2C);
            *reinterpret_cast<u32 *>(slot + 0x478) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x30);
            *reinterpret_cast<u32 *>(slot + 0x47C) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x34);
        }

        entry += 0x38;
        if (*reinterpret_cast<i16 *>(entry) < 0)
        {
            return;
        }
        if (result == 0)
        {
            goto processEntry;
        }
    }
}

// STUB: th08 0x451d50
i32 Player::FUN_00451d50()
{
    return 0;
}
// STUB: th08 0x44d420
void Player::FUN_0044d420()
{
}
// FUNCTION: th08 0x44c390
ChainCallbackResult Player::OnUpdate(Player *player)
{
    if (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x2C) != 0)
    {
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xBE834) != 0)
        {
            *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xBE834) + 0x1F8) |= 0x80000;
        }
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B24) != 0)
        {
            *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xE2B24) + 0x1F8) |= 0x80000;
        }
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xBE834) != 0)
    {
        *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xBE834) + 0x1F8) &= 0xfff7ffff;
    }
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B24) != 0)
    {
        *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xE2B24) + 0x1F8) &= 0xfff7ffff;
    }
    player->FUN_0044c5b0();
    player->FUN_0044c650();
    if (player->playerState == PLAYER_STATE_DYING)
    {
        if (player->FUN_0044cbf0() != 0)
        {
            goto updateD180;
        }
    }
    else if (player->playerState == PLAYER_STATE_SPAWNING)
    {
updateD180:
        player->FUN_0044d180();
    }
    player->FUN_0044d2c0();
    if (player->playerState != PLAYER_STATE_DYING && player->playerState != PLAYER_STATE_SPAWNING)
    {
        player->FUN_0044aec0();
    }
    g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10));
    player->FUN_00451150();
    player->FUN_00451500();
    player->FUN_0044d420();
    if (!g_Gui.IsDialogPresent())
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE10) += 1;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE14) += 1;
        if (g_GameManager.GaugeIsExtremelyHuman())
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE1C) += 1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE24) += 1;
            g_GameManager.AddScore(100);
        }
        else if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE18) += 1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE20) += 1;
            g_GameManager.AddScore(100);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(index, slot)
// FUNCTION: th08 0x44c5b0
void Player::FUN_0044c5b0()
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 384; index++, slot++)
    {
        if (slot->lifetime < 0)
            continue;

        slot->lifetime--;
        slot->radius += slot->radiusGrowth;
        slot->size.x += slot->sizeGrowth.x;
        slot->size.y += slot->sizeGrowth.y;

        if (slot->lifetime <= 0)
            slot->Deactivate();
    }
}

// STUB: th08 0x44d530
ChainCallbackResult Player::OnDrawHighPrio(Player *player)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d630
ChainCallbackResult Player::OnDrawLowPrio(Player *player)
{
    player->FUN_00451400();
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
