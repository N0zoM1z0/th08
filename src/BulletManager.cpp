#include "th_pch.h"

#include "BulletManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"

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


DIFFABLE_STATIC(i32, g_BulletCancelItemType);
void __fastcall fsincos(f32 *sine, f32 *cosine, f32 angle) {}

// FUNCTION: th08 0x430830
#pragma var_order(position, playerCollisionResult, bulletIndex, sine, bullet, laser, cosine, radius, this)
void BulletManager::RemoveAllBullets(i32 mode)
{
    u8 *bullet = reinterpret_cast<u8 *>(&g_BulletManager) + 0x1A880;
    i32 bulletIndex;
    i32 playerCollisionResult;
    u8 *laser;
    f32 position[3];
    f32 sine;
    f32 cosine;
    f32 radius;

    for (bulletIndex = 0; bulletIndex < 0x600; bulletIndex++, bullet += 0x10B8)
    {
        if (*reinterpret_cast<u16 *>(bullet + 0xDB8) == 0 || *reinterpret_cast<u16 *>(bullet + 0xDB8) == 5)
        {
            continue;
        }

        playerCollisionResult = g_Player.FUN_00449ff0(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                      reinterpret_cast<Float3 *>(bullet + 0xD34));
        if (g_Player.FUN_00449ff0(reinterpret_cast<Float3 *>(bullet + 0xD44), reinterpret_cast<Float3 *>(bullet + 0xD34)) == 2)
        {
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), static_cast<ItemType>(g_BulletCancelItemType), 1);
            memset(bullet, 0, 0x10B8);
        }
        else if (mode != 4)
        {
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                    static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), mode);
            memset(bullet, 0, 0x10B8);
        }
        else
        {
            *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
        }
    }

    laser = reinterpret_cast<u8 *>(this) + 0x660938;
    reinterpret_cast<Float3 *>(position)->operator float *();
    for (bulletIndex = 0; bulletIndex < 0x100; bulletIndex++, laser += 0x59C)
    {
        if (*reinterpret_cast<i32 *>(laser + 0x584) == 0)
        {
            continue;
        }
        if ((*reinterpret_cast<u16 *>(laser + 0x594) & 4) != 0 && mode != 4)
        {
            continue;
        }

        if (*reinterpret_cast<u8 *>(laser + 0x598) < 2)
        {
            *reinterpret_cast<u8 *>(laser + 0x598) = 2;
            *reinterpret_cast<ZunTimer *>(laser + 0x588) = 0;
            *reinterpret_cast<i32 *>(laser + 0x564) = *reinterpret_cast<i32 *>(laser + 0x568);

            if (mode != 4)
            {
                radius = *reinterpret_cast<f32 *>(laser + 0x558);
                fsincos(&sine, &cosine, *reinterpret_cast<f32 *>(laser + 0x554));
                while (*reinterpret_cast<f32 *>(laser + 0x55C) > radius)
                {
                    position[0] = cosine * radius + *reinterpret_cast<f32 *>(laser + 0x548);
                    position[1] = sine * radius + *reinterpret_cast<f32 *>(laser + 0x54C);
                    position[2] = 0.0f;
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(position),
                                            static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), mode);
                    radius = radius + 1.0f;
                }
            }
        }

        *reinterpret_cast<i32 *>(laser + 0x580) = 0;
    }

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA53C) = 10;
}

// FUNCTION: th08 0x42f360
#pragma var_order(i, bullet, this)
void BulletManager::Initialize()
{
    u8 *bullet;
    i32 i;

    memset(this, 0, sizeof(BulletManager));
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x6BA56C) = reinterpret_cast<u8 *>(this) + 0x1A880;
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(this) + 0x660638) = 6;
    this->unk6ba570 = 6;

    bullet = reinterpret_cast<u8 *>(&g_BulletManager) + 0x1A880;
    for (i = 0; i < 0x600; i++, bullet += 0x10B8)
    {
        *reinterpret_cast<u16 *>(bullet + 0x21A) = 0xFFFF;
        *reinterpret_cast<u16 *>(bullet + 0xCAA) = 0xFFFF;
        *reinterpret_cast<u16 *>(bullet + 0x4BE) = 0xFFFF;
        *reinterpret_cast<u16 *>(bullet + 0x762) = 0xFFFF;
        *reinterpret_cast<u16 *>(bullet + 0xA06) = 0xFFFF;
    }
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
