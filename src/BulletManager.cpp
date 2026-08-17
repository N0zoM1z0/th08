#include "th_pch.h"

#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"

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


// FUNCTION: th08 0x430aa0
#pragma var_order(score, totalScore, bulletCount, bulletIndex, sine, bullet, position, laser, cosine, radius, this)
i32 BulletManager::DespawnBullets(i32 maxScore, i32 awardLaserItems)
{
    f32 radius;
    f32 cosine;
    u8 *laser;
    f32 position[3];
    u8 *bullet;
    f32 sine;
    i32 bulletIndex;
    i32 bulletCount;
    i32 totalScore;
    i32 score;

    totalScore = 0;
    score = 2000;
    bulletCount = 0;
    bullet = reinterpret_cast<u8 *>(&g_BulletManager) + 0x1A880;
    for (bulletIndex = 0; bulletIndex < 0x600; bulletIndex++, bullet += 0x10B8)
    {
        if (*reinterpret_cast<u16 *>(bullet + 0xDB8) == 0)
        {
            continue;
        }

        if (g_Player.FUN_00449ff0(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                 reinterpret_cast<Float3 *>(bullet + 0xD34)) == 2)
        {
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                    static_cast<ItemType>(g_BulletCancelItemType), 1);
        }
        else
        {
            g_ItemManager.SpawnItem(
                reinterpret_cast<Float3 *>(bullet + 0xD44),
                static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), 1);
        }

        g_AsciiManager.CreateScorePopup(reinterpret_cast<Float3 *>(bullet + 0xD44), score,
                                        score >= maxScore ? -256 : -1);
        totalScore += score;
        bulletCount++;
        score += 20;
        if (score > maxScore)
        {
            score = maxScore;
        }
        *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
    }

    laser = reinterpret_cast<u8 *>(this) + 0x660938;
    reinterpret_cast<Float3 *>(position)->operator float *();
    for (bulletIndex = 0; bulletIndex < 0x100; bulletIndex++, laser += 0x59C)
    {
        if (*reinterpret_cast<i32 *>(laser + 0x584) == 0)
        {
            continue;
        }

        if (*reinterpret_cast<u8 *>(laser + 0x598) < 2)
        {
            *reinterpret_cast<u8 *>(laser + 0x598) = 2;
            *reinterpret_cast<ZunTimer *>(laser + 0x588) = 0;
            *reinterpret_cast<i32 *>(laser + 0x564) = *reinterpret_cast<i32 *>(laser + 0x568);

            if (awardLaserItems)
            {
                g_ItemManager.SpawnItem(
                    reinterpret_cast<Float3 *>(laser + 0x548),
                    static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), 1);
                radius = *reinterpret_cast<f32 *>(laser + 0x558);
                fsincos(&sine, &cosine, *reinterpret_cast<f32 *>(laser + 0x554));
                while (*reinterpret_cast<f32 *>(laser + 0x55C) > radius)
                {
                    position[0] = cosine * radius + *reinterpret_cast<f32 *>(laser + 0x548);
                    position[1] = sine * radius + *reinterpret_cast<f32 *>(laser + 0x54C);
                    position[2] = 0.0f;
                    g_ItemManager.SpawnItem(
                        reinterpret_cast<Float3 *>(position),
                        static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), 1);
                    radius += 32.0f;
                }
            }
        }

        *reinterpret_cast<i32 *>(laser + 0x580) = 0;
    }

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA53C) = 10;
    return totalScore;
}

// FUNCTION: th08 0x42f420
BulletManager::BulletManager()
{
    this->Initialize();
}

// FUNCTION: th08 0x42f4a0
BulletTypeSprites::BulletTypeSprites()
{
}

// FUNCTION: th08 0x42f500
Bullet::Bullet()
{
}

// FUNCTION: th08 0x42f580
Laser::Laser()
{
}

// FUNCTION: th08 0x42f5c0
BulletExState::BulletExState()
{
}

// FUNCTION: th08 0x432170
void Bullet::FUN_00432170()
{
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(this) + 0xDB8) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xD80) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xD8C) = 0;
}

// FUNCTION: th08 0x4321b0
void BulletManager::FUN_004321b0()
{
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA568) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA564) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA560) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA55C) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA558) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA554) = 0;
}

// FUNCTION: th08 0x432210
#pragma var_order(magnitude, this)
void Bullet::FUN_00432210()
{
    f32 magnitude;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xF80) <= 16)
    {
        magnitude =
            5.0f - ((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xF80) * 5.0f) / 16.0f;
        reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50)
            ->FromAngleMagnitude(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                                 (magnitude + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68)) *
                                     *reinterpret_cast<f32 *>(0x17CE8E0));
    }
    else
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) ^= 0x1;
    }

    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xF80))++;
}


// FUNCTION: th08 0x4322b0
#pragma var_order(delta, this)
void Bullet::FUN_004322b0()
{
    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFAC) >=
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFCC))
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) &= ~0x10;
    }
    else
    {
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50) +=
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xFC0) *
            *reinterpret_cast<f32 *>(0x17CE8E0);

        if (fabsf(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD50)) > 0.0001f ||
            fabsf(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD54)) > 0.0001f)
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
                VectorAngle(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD54),
                            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD50));
        }
    }

    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFAC))++;
}

// FUNCTION: th08 0x432390
void Bullet::FUN_00432390()
{
    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFD8) >=
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFF8))
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) &= ~0x20;
    }
    else
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
            AddNormalizeAngle(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                              *reinterpret_cast<f32 *>(0x17CE8E0) *
                                  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFE8));
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68) +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFE4);
        reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50)
            ->FromAngleMagnitude(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                                 *reinterpret_cast<f32 *>(0x17CE8E0) *
                                     *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68));
    }

    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFD8))++;
}

// FUNCTION: th08 0x432460
#pragma var_order(magnitude, this)
void Bullet::FUN_00432460()
{
    f32 magnitude;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) >=
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024))
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8) >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8)), 0);
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) += 1;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) >=
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1028))
        {
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) &= ~0x40;
        }
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) +=
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x1014);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xD68) =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1010);
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68);
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) = 0;
    }
    else
    {
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68) -
                    ((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) *
                     *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68)) /
                        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024);
    }

    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50)
        ->FromAngleMagnitude(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                             magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004))++;
}

// FUNCTION: th08 0x4325a0
#pragma var_order(magnitude, this)
void Bullet::FUN_004325a0()
{
    f32 magnitude;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) >=
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024))
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8) >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8)), 0);
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) += 1;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) >=
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1028))
        {
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) &= ~0x100;
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1014);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xD68) =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1010);
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68);
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) = 0;
    }
    else
    {
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68) -
                    ((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) *
                     *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68)) /
                        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024);
    }

    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50)
        ->FromAngleMagnitude(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                             magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004))++;
}

// FUNCTION: th08 0x4326e0
#pragma var_order(magnitude, this)
void Bullet::FUN_004326e0()
{
    f32 magnitude;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) >=
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024))
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8) >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8)), 0);
        }
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) += 1;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) >=
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1028))
        {
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) &= ~0x80;
        }
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
            AddNormalizeAngle(g_Player.FUN_0044c1b0(reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD44)),
                              *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x1014));
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xD68) =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1010);
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68);
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) = 0;
    }
    else
    {
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68) -
                    ((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) *
                     *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68)) /
                        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024);
    }

    reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50)
        ->FromAngleMagnitude(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                             magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004))++;
}


// FUNCTION: th08 0x432830
#pragma var_order(magnitude, this)
void Bullet::FUN_00432830()
{
    f32 magnitude;

    if (!g_GameManager.IsWithinPlayfield((reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD44)->operator float *())[0],
                                         (reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD44)->operator float *())[1],
                                         *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x224) + 0x34),
                                         *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0x224) + 0x30)))
    {
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8) >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8)), 0);
        }

        if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD44) < 0.0f ||
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD44) >= 384.0f)
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
                -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) - ZUN_PI;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
                AddNormalizeAngle(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74), 0.0f);
        }

        if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD48) < 0.0f ||
            (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD48) >= 448.0f &&
             (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) & 0x400) != 0))
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74) =
                -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74);
        }

        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xD68) =
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x103C);
        magnitude = *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68);
        reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xD50)
            ->FromAngleMagnitude(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74),
                                 magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1050) += 1;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1050) >=
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1054))
        {
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) &= ~0xC00;
        }
    }
}

// FUNCTION: th08 0x4329f0
void Bullet::FUN_004329f0()
{
    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD44) < 0.0)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD44) += 384.0f;
    }
    else if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD44) > 384.0)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD44) -= 384.0f;
    }

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1088) <= 0)
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) ^= 0x400000;
    }
    else
    {
        (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1088))--;
    }
}

// FUNCTION: th08 0x432aa0
void Bullet::FUN_00432aa0()
{
    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD48) < 0.0)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD48) += 448.0f;
    }
    else if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD48) > 448.0)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD48) -= 448.0f;
    }

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1088) <= 0)
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) ^= 0x800000;
    }
    else
    {
        (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1088))--;
    }
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

// FUNCTION: th08 0x4311a0
#pragma var_order(bulletManager, bulletAnmPath)
ZunResult BulletManager::RegisterChain(char *bulletAnmPath)
{
    BulletManager *bulletManager = &g_BulletManager;

    bulletManager->Initialize();
    bulletManager->bulletAnmPath = bulletAnmPath;

    g_BulletManagerCalcChain.SetCallback((ChainCallback)BulletManager::OnUpdate);
    g_BulletManagerCalcChain.addedCallback = (ChainLifetimeCallback)BulletManager::AddedCallback;
    g_BulletManagerCalcChain.deletedCallback = (ChainLifetimeCallback)BulletManager::DeletedCallback;
    g_BulletManagerCalcChain.arg = bulletManager;
    if (g_Chain.AddToCalcChain(&g_BulletManagerCalcChain, 14))
    {
        return ZUN_ERROR;
    }

    g_BulletManagerDrawChain.SetCallback((ChainCallback)BulletManager::OnDraw);
    g_BulletManagerDrawChain.arg = bulletManager;
    g_Chain.AddToDrawChain(&g_BulletManagerDrawChain, 13);

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
