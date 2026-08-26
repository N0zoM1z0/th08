#include "th_pch.h"

#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "EclManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "SoundPlayer.hpp"
#include "Supervisor.hpp"

namespace th08
{

DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);

DIFFABLE_STATIC(BulletManager, g_BulletManager);
DIFFABLE_STATIC(ChainElem, g_BulletManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_BulletManagerDrawChain);

void __fastcall CopyBulletAnmVmCore(AnmVm *dst, const AnmVm *src);
void __fastcall SelectBulletSprite(AnmVm *dst, AnmVm *base, AnmVm *sizeSource, i32 offset);


void __fastcall fsincos(f32 *sine, f32 *cosine, f32 angle) {}

// FUNCTION: th08 0x42a410
BulletSpawnDescriptor::BulletSpawnDescriptor()
{
    memset(this, 0, sizeof(*this));
    this->transformSound = -1;
}

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 16, g_BulletSpriteOffsetSmall) = {
    0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 0,
};
DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 8, g_BulletSpriteOffsetMedium) = {
    0, 1, 1, 2, 2, 3, 4, 0,
};

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

// FUNCTION: th08 0x42f5f0
#pragma var_order(speed, i, bullet, angle, transformFlags, this)
i32 BulletManager::FUN_0042f5f0(BulletSpawnDescriptor *descriptor, i32 index1, i32 index2, f32 angleToPlayer)
{
    f32 speed;
    i32 i;
    u8 *bullet;
    f32 angle;
    u32 transformFlags;

    i = 0;
    bullet = this->bulletCursor;
    for (i = 0; i < 0x600; i++)
    {
        if (*reinterpret_cast<u16 *>(bullet + 0xDB8) == 0)
            break;
        bullet += 0x10B8;
        if (*reinterpret_cast<u16 *>(bullet + 0xDB8) == 6)
            bullet = reinterpret_cast<u8 *>(this) + 0x1A880;
    }
    if (i >= 0x600)
        return 1;

    angle = 0.0f;
    if (descriptor->count2 > 1)
        speed = descriptor->speed1 -
                (descriptor->speed1 - descriptor->speed2) * (f32)index2 / (f32)descriptor->count2;
    else
        speed = descriptor->speed1;

    switch (descriptor->aimMode)
    {
    case 0:
    case 1:
        if ((descriptor->count1 & 1) != 0)
            angle += (f32)((index1 + 1) / 2) * descriptor->angleStep;
        else
            angle += (f32)(index1 / 2) * descriptor->angleStep + descriptor->angleStep * 0.5f;
        if ((index1 & 1) != 0)
            angle *= -1.0f;
        if (descriptor->aimMode == 0)
            angle += angleToPlayer;
        angle += descriptor->angle;
        break;
    case 2:
        angle += angleToPlayer;
    case 3:
        angle += (f32)index1 * (ZUN_PI * 2.0f) / (f32)descriptor->count1;
        angle += (f32)index2 * descriptor->angleStep + descriptor->angle;
        break;
    case 4:
        angle += angleToPlayer;
    case 5:
        angle += ZUN_PI / (f32)descriptor->count1;
        angle += (f32)index1 * (ZUN_PI * 2.0f) / (f32)descriptor->count1;
        angle += descriptor->angle;
        break;
    case 6:
        angle = g_Rng.GetRandomF32InRange(descriptor->angle - descriptor->angleStep) + descriptor->angleStep;
        break;
    case 7:
        speed = g_Rng.GetRandomF32InRange(descriptor->speed1 - descriptor->speed2) + descriptor->speed2;
        angle += (f32)index1 * (ZUN_PI * 2.0f) / (f32)descriptor->count1;
        angle += (f32)index2 * descriptor->angleStep + descriptor->angle;
        break;
    case 8:
        angle = g_Rng.GetRandomF32InRange(descriptor->angle - descriptor->angleStep) + descriptor->angleStep;
        speed = g_Rng.GetRandomF32InRange(descriptor->speed1 - descriptor->speed2) + descriptor->speed2;
        break;
    default:
        break;
    }

    *reinterpret_cast<u16 *>(bullet + 0xDB8) = 1;
    *reinterpret_cast<u8 *>(bullet + 0xDBC) = 1;
    *reinterpret_cast<u8 *>(bullet + 0xDBD) = 0;
    *reinterpret_cast<ZunTimer *>(bullet + 0xD80) = 0;
    *reinterpret_cast<u8 *>(bullet + 0x10B4) = 0;
    *reinterpret_cast<ZunTimer *>(bullet + 0xD8C) = 0;
    *reinterpret_cast<f32 *>(bullet + 0xD68) = speed;
    *reinterpret_cast<f32 *>(bullet + 0xD74) = AddNormalizeAngle(angle, 0.0f);
    *reinterpret_cast<Float3 *>(bullet + 0xD44) = descriptor->position;
    reinterpret_cast<Float3 *>(bullet + 0xD44)->operator float *()[2] = 0.1f;
    reinterpret_cast<Float3 *>(bullet + 0xD50)->FromAngleMagnitude(
        angle, speed * g_EclGameTimeScale);

    *reinterpret_cast<u32 *>(bullet + 0xDAC) = descriptor->transformFlags;
    *reinterpret_cast<i16 *>(bullet + 0xDB4) = descriptor->color;
    *reinterpret_cast<i32 *>(bullet + 0xDC4) = 0;
    *reinterpret_cast<u8 *>(bullet + 0xDBE) = 0;

    CopyBulletAnmVmCore(reinterpret_cast<AnmVm *>(bullet), reinterpret_cast<AnmVm *>(descriptor->templateSprites));
    CopyBulletAnmVmCore(reinterpret_cast<AnmVm *>(bullet + 0xA90), reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xA90));
    *reinterpret_cast<Float3 *>(bullet + 0xD34) = descriptor->templateSprites->position;
    *reinterpret_cast<u8 *>(bullet + 0xD40) = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xD40);
    *reinterpret_cast<u8 *>(bullet + 0xD41) = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xD41);
    *reinterpret_cast<u8 *>(bullet + 0xD42) = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xD42);
    *reinterpret_cast<i32 *>(bullet + 0xDC8) = descriptor->transformSound;
    *reinterpret_cast<i32 *>(bullet + 0xDA8) = 0;

    if (*reinterpret_cast<i16 *>(bullet + 0x214) !=
        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x214) + descriptor->color)
    {
        this->bulletAnm->SetSprite(reinterpret_cast<AnmVm *>(bullet),
            *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x214) + descriptor->color);
    }

    if (*reinterpret_cast<i16 *>(bullet + 0xCA4) !=
        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xCA4) + descriptor->color)
    {
        if (*reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(bullet + 0x224) + 0x30) <= 16.0f)
        {
            this->bulletAnm->SetSprite(reinterpret_cast<AnmVm *>(bullet + 0xA90),
                *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xCA4) +
                    g_BulletSpriteOffsetSmall[descriptor->color]);
        }
        else if (*reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(bullet + 0x224) + 0x30) <= 32.0f)
        {
            this->bulletAnm->SetSprite(reinterpret_cast<AnmVm *>(bullet + 0xA90),
                *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xCA4) +
                    g_BulletSpriteOffsetMedium[descriptor->color]);
        }
        else
        {
            this->bulletAnm->SetSprite(reinterpret_cast<AnmVm *>(bullet + 0xA90),
                *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xCA4) + descriptor->color);
        }
    }

    transformFlags = descriptor->transformFlags;
    if ((descriptor->transformFlags & 2) != 0)
    {
        CopyBulletAnmVmCore(reinterpret_cast<AnmVm *>(bullet + 0x2A4), reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x2A4));
        SelectBulletSprite(reinterpret_cast<AnmVm *>(bullet + 0x2A4),
                           reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x2A4),
                           reinterpret_cast<AnmVm *>(bullet), descriptor->color);
        *reinterpret_cast<u16 *>(bullet + 0xDB8) = 2;
        *reinterpret_cast<Float3 *>(bullet + 0xD44) -=
            *reinterpret_cast<Float3 *>(bullet + 0xD50) * 4.0f;
    }
    else if ((descriptor->transformFlags & 4) != 0)
    {
        CopyBulletAnmVmCore(reinterpret_cast<AnmVm *>(bullet + 0x548), reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x548));
        SelectBulletSprite(reinterpret_cast<AnmVm *>(bullet + 0x548),
                           reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x548),
                           reinterpret_cast<AnmVm *>(bullet), descriptor->color);
        *reinterpret_cast<u16 *>(bullet + 0xDB8) = 3;
        *reinterpret_cast<Float3 *>(bullet + 0xD44) -=
            *reinterpret_cast<Float3 *>(bullet + 0xD50) * 4.0f;
    }
    else if ((descriptor->transformFlags & 8) != 0)
    {
        CopyBulletAnmVmCore(reinterpret_cast<AnmVm *>(bullet + 0x7EC), reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x7EC));
        SelectBulletSprite(reinterpret_cast<AnmVm *>(bullet + 0x7EC),
                           reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0x7EC),
                           reinterpret_cast<AnmVm *>(bullet), descriptor->color);
        *reinterpret_cast<u16 *>(bullet + 0xDB8) = 4;
        *reinterpret_cast<Float3 *>(bullet + 0xD44) -=
            *reinterpret_cast<Float3 *>(bullet + 0xD50) * 4.0f;
    }

    memcpy(bullet + 0xDD0, descriptor->transforms, sizeof(descriptor->transforms));
    *reinterpret_cast<u32 *>(bullet + 0xDB0) = descriptor->transformFlags;
    *reinterpret_cast<u32 *>(bullet + 0xDAC) = 0;
    *reinterpret_cast<i32 *>(bullet + 0xDCC) = descriptor->transformStartIndex;
    reinterpret_cast<Bullet *>(bullet)->FUN_0042ffc0();

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA53C) != 0 &&
        (*reinterpret_cast<u32 *>(bullet + 0xDB0) & 0x1000) == 0)
        *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;

    bullet += 0x10B8;
    if (*reinterpret_cast<u16 *>(bullet + 0xDB8) == 6)
        this->bulletCursor = reinterpret_cast<u8 *>(this) + 0x1A880;
    else
        this->bulletCursor = bullet;
    return 0;
}

// FUNCTION: th08 0x42fe70
void __fastcall CopyBulletAnmVmCore(AnmVm *dst, const AnmVm *src)
{
    *dst = *src;
}

// FUNCTION: th08 0x42fea0
void __fastcall SelectBulletSprite(AnmVm *dst, AnmVm *base, AnmVm *sizeSource, i32 offset)
{
    if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(dst) + 0x214) !=
        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(base) + 0x214) + offset)
    {
        if (*reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(sizeSource) + 0x224) + 0x30) <= 16.0f)
        {
            g_BulletManager.bulletAnm->SetSprite(
                dst, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(base) + 0x214) + g_BulletSpriteOffsetSmall[offset]);
        }
        else if (*reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(sizeSource) + 0x224) + 0x30) <= 32.0f)
        {
            g_BulletManager.bulletAnm->SetSprite(
                dst, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(base) + 0x214) + g_BulletSpriteOffsetMedium[offset]);
        }
        else
        {
            g_BulletManager.bulletAnm->SetSprite(
                dst, *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(base) + 0x214) + offset);
        }
    }
}

struct BulletSpriteScriptRow
{
    i32 scripts[5];
};

static BulletSpriteScriptRow g_BulletSpriteScripts[21] = {
    {{0, 18, 19, 20, 15}},   {{1, 21, 22, 23, 16}},   {{2, 21, 22, 23, 16}},
    {{3, 21, 22, 23, 16}},   {{4, 21, 22, 23, 16}},   {{5, 21, 22, 23, 16}},
    {{6, 21, 22, 23, 16}},   {{7, 24, 24, 24, 17}},   {{8, 24, 24, 24, 17}},
    {{9, 24, 24, 24, 17}},   {{25, 27, 27, 27, 26}}, {{106, 21, 22, 23, 16}},
    {{107, 21, 22, 23, 16}}, {{108, 21, 22, 23, 16}}, {{109, 24, 24, 24, 17}},
    {{110, 24, 24, 24, 17}}, {{111, 21, 22, 23, 16}}, {{112, 21, 22, 23, 16}},
    {{113, 24, 24, 24, 17}}, {{114, 24, 24, 24, 17}}, {{115, 24, 24, 24, 17}},
};

// FUNCTION: th08 0x42ffc0
void Bullet::FUN_0042ffc0()
{
    BulletTransformRecord *record;

nextRecord:
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC) >= 18)
        return;

    record = reinterpret_cast<BulletTransformRecord *>(
        reinterpret_cast<u8 *>(this) + 0xDD0 +
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC) * sizeof(BulletTransformRecord));
    if (record->kind == 0)
        return;
    if (record->allowWhileActive == 0 && *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) != 0)
        return;
    if ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDB0) & record->kind) == 0)
    {
        ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
        goto nextRecord;
    }

    switch (record->kind)
    {
    case 1:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= 1;
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xF80) = 0;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xF9C) = 0;
        break;

    case 0x10:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= 0x10;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFB8) = record->float0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFBC) =
            record->float1 > -990.0f ? record->float1
                                    : *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD74);
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFAC) = 0;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFCC) = record->int0;
        reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xFC0)->FromAngleMagnitude(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFBC),
            g_EclGameTimeScale * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFB8));
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC) != 0 &&
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8) >= 0)
            g_SoundPlayer.PlaySoundByIdx(
                static_cast<SoundIdx>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8)), 0);
        break;

    case 0x20:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= 0x20;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFE4) = record->float0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xFE8) = record->float1;
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xFD8) = 0;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xFF8) = record->int0;
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC) != 0 &&
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8) >= 0)
            g_SoundPlayer.PlaySoundByIdx(
                static_cast<SoundIdx>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDC8)), 0);
        break;

    case 0x40:
    case 0x80:
    case 0x100:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= record->kind;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x1014) = record->float0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x1010) =
            record->float1 > -999.0f ? record->float1
                                    : *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68);
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1004) = 0;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1024) = record->int0;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1028) = record->int1;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x102C) = 0;
        break;

    case 0x400:
    case 0x800:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= record->kind;
        if (record->float0 >= 0.0f)
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x103C) = record->float0;
        else
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x103C) =
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xD68);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1054) = record->int0;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x1050) = 0;
        break;

    case 0x400000:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= record->kind;
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1088) = record->int0;
        break;

    case 0x800000:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= record->kind;
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x1088) = record->int0;
        break;

    case 0x20000:
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xDAC) |= record->kind;
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x105C) = record->int0;
        break;

    case 0x2000:
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDA8) = record->int0;
        ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
        goto nextRecord;

    case 0x4000:
        this->sprites = g_BulletManager.bulletTypeSprites[record->int0];
        g_BulletManager.bulletAnm->SetSprite(
            &this->sprites.sprite0,
            *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(&this->sprites.sprite0) + 0x214) + record->int1);
        ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
        goto nextRecord;

    case 0x40000:
        *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(this) + 0xDB8) = 5;
        break;

    case 0x80000:
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(record->int0), this->position0.x);
        ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
        goto nextRecord;

    case 0x1000000:
        {
            BulletSpawnDescriptor pattern;
            i32 fadeParent;
            pattern.position = this->position0;
            fadeParent = record->int0 & 0x80000000;
            pattern.aimMode = (static_cast<u32>(record->int0) & 0x7F000000) >> 24;
            pattern.bulletType = (static_cast<u32>(record->int0) & 0x00FF0000) >> 16;
            pattern.color = (static_cast<u32>(record->int0) & 0x0000FF00) >> 8;
            pattern.transformStartIndex = record->int0 & 0xFF;
            pattern.count1 = static_cast<i16>(record->int1);
            pattern.speed1 = record->float0;
            pattern.speed2 = record->float1;

            ++record;
            ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
            pattern.count2 = static_cast<i16>(record->int0);
            pattern.transformFlags = record->int1;
            pattern.angle = record->float0;
            pattern.angleStep = record->float1;
            memcpy(pattern.transforms, reinterpret_cast<u8 *>(this) + 0xDD0, sizeof(pattern.transforms));
            g_BulletManager.FUN_00430e10(&pattern);
            ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
            if (fadeParent != 0)
                *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(this) + 0xDB8) = 5;
            else
                goto nextRecord;
        }
        break;

    default:
        break;
    }

    ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xDCC);
}

// FUNCTION: th08 0x430830
#pragma var_order(position, playerCollisionResult, bulletIndex, sine, bullet, laser, cosine, radius, this)
void BulletManager::RemoveAllBullets(i32 mode)
{
    u8 *bullet = reinterpret_cast<u8 *>(&g_BulletManager) + 0x1A880;
    i32 bulletIndex;
    i32 playerCollisionResult;
    Laser *laser;
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
            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
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

    laser = &this->lasers[0];
    reinterpret_cast<Float3 *>(position)->operator float *();
    for (bulletIndex = 0; bulletIndex < 0x100; bulletIndex++, laser++)
    {
        if (laser->inUse == 0)
        {
            continue;
        }
        if ((laser->flags & 4) != 0 && mode != 4)
        {
            continue;
        }

        if (laser->state < LASER_STATE_DESPAWNING)
        {
            laser->state = LASER_STATE_DESPAWNING;
            laser->timer = 0;
            *reinterpret_cast<i32 *>(&laser->width) =
                *reinterpret_cast<i32 *>(&laser->currentWidth);

            if (mode != 4)
            {
                radius = laser->startOffset;
                fsincos(&sine, &cosine, laser->angle);
                while (laser->endOffset > radius)
                {
                    position[0] = cosine * radius + laser->position.x;
                    position[1] = sine * radius + laser->position.y;
                    position[2] = 0.0f;
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(position),
                                            static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), mode);
                    radius = radius + 1.0f;
                }
            }
        }

        laser->hitboxEndDelay = 0;
    }

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA53C) = 10;
}


// FUNCTION: th08 0x430aa0
#pragma var_order(score, totalScore, bulletCount, bulletIndex, sine, bullet, position, laser, cosine, radius, this)
i32 BulletManager::DespawnBullets(i32 maxScore, i32 awardLaserItems)
{
    f32 radius;
    f32 cosine;
    Laser *laser;
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
                                    static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
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

    laser = &this->lasers[0];
    reinterpret_cast<Float3 *>(position)->operator float *();
    for (bulletIndex = 0; bulletIndex < 0x100; bulletIndex++, laser++)
    {
        if (laser->inUse == 0)
        {
            continue;
        }

        if (laser->state < LASER_STATE_DESPAWNING)
        {
            laser->state = LASER_STATE_DESPAWNING;
            laser->timer = 0;
            *reinterpret_cast<i32 *>(&laser->width) =
                *reinterpret_cast<i32 *>(&laser->currentWidth);

            if (awardLaserItems)
            {
                g_ItemManager.SpawnItem(
                    &laser->position,
                    static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), 1);
                radius = laser->startOffset;
                fsincos(&sine, &cosine, laser->angle);
                while (laser->endOffset > radius)
                {
                    position[0] = cosine * radius + laser->position.x;
                    position[1] = sine * radius + laser->position.y;
                    position[2] = 0.0f;
                    g_ItemManager.SpawnItem(
                        reinterpret_cast<Float3 *>(position),
                        static_cast<ItemType>(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA570)), 1);
                    radius += 32.0f;
                }
            }
        }

        laser->hitboxEndDelay = 0;
    }

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA53C) = 10;
    return totalScore;
}

// FUNCTION: th08 0x430d30
#pragma var_order(delta, i, bullet, this)
void BulletManager::RemoveBulletsInRadius(const Float3 *position, f32 radius)
{
    i32 i;
    Bullet *bullet;

    bullet = &g_BulletManager.bullets[0];
    Float3 delta;
    radius *= radius;
    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (*reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(bullet) + 0xdb8) == 0 ||
            *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(bullet) + 0xdb8) == 5)
            continue;
        delta = bullet->position0 - *position;
        if (D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&delta)) > radius)
            continue;
        g_ItemManager.SpawnItem(&bullet->position0, static_cast<ItemType>(6), 1);
        memset(bullet, 0, sizeof(Bullet));
    }
}

// FUNCTION: th08 0x430e10
#pragma var_order(i, angleToPlayer, j, this)
i32 BulletManager::FUN_00430e10(BulletSpawnDescriptor *descriptor)
{
    i32 i;
    f32 angleToPlayer;
    i32 j;

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x800;
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_BulletManager) + 0x6BA538) >= 0x600)
        return 0;

    descriptor->templateSprites = &this->bulletTypeSprites[descriptor->bulletType];
    angleToPlayer = g_Player.FUN_0044c1b0(&descriptor->position);
    for (j = 0; j < descriptor->count2; j++)
    {
        for (i = 0; i < descriptor->count1; i++)
        {
            if (this->FUN_0042f5f0(descriptor, i, j, angleToPlayer) != 0)
                goto doneSpawning;
        }
    }

doneSpawning:
    if ((descriptor->transformFlags & 0x200) != 0)
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(descriptor->spawnSound), descriptor->position.x);
    return 0;
}

// FUNCTION: th08 0x430f20
#pragma var_order(i, laser, this)
Laser *BulletManager::SpawnLaserPattern(BulletSpawnDescriptor *descriptor)
{
    Laser *laser;
    i32 i;

    laser = &this->lasers[0];
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6BA53C) != 0 &&
        (descriptor->transformFlags & 4) == 0)
        return laser;

    for (i = 0; i < 0x100; i++, laser++)
    {
        if (laser->inUse)
            continue;

        this->bulletAnm->SetAndExecuteScriptIdx(&laser->vm0, descriptor->bulletType + 10);
        this->bulletAnm->SetSprite(&laser->vm0, laser->vm0.activeSpriteIndex + descriptor->color);
        this->bulletAnm->InitializeAndSetSprite(
            &laser->vm1, g_BulletSpriteOffsetSmall[descriptor->color] + 0x92);
        laser->vm1.blendMode = 1;
        laser->position = descriptor->position;
        laser->color = descriptor->color;
        laser->inUse = 1;
        laser->angle = descriptor->angle;
        if (descriptor->aimMode == 0)
            laser->angle = g_Player.FUN_0044c1b0(&descriptor->position) + laser->angle;
        laser->flags = static_cast<u16>(descriptor->transformFlags);
        laser->timer = 0;
        laser->startOffset = descriptor->laserStartOffset;
        laser->endOffset = descriptor->laserEndOffset;
        laser->startLength = descriptor->laserStartLength;
        laser->width = descriptor->laserWidth;
        laser->speed = descriptor->speed1;
        laser->startTime = descriptor->laserStartTime;
        laser->duration = descriptor->laserDuration;
        laser->despawnDuration = descriptor->laserDespawnDuration;
        laser->hitboxStartTime = descriptor->laserHitboxStartTime;
        laser->hitboxEndDelay = descriptor->laserHitboxEndDelay;
        laser->hideCapDuringStartup = 0;
        if (laser->startTime == 0)
            laser->state = LASER_STATE_ACTIVE;
        else
            laser->state = LASER_STATE_STARTING;
        break;
    }
    return laser;
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

// FUNCTION: th08 0x431240
#pragma var_order(collisionResult, i, currentWidth, bucketIndex, laserSize, bullet, alpha, laser, laserCenter, rampWindow, bulletManager)
ChainCallbackResult BulletManager::OnUpdate(BulletManager *bulletManager)
{
    i32 collisionResult;
    i32 i;
    f32 currentWidth;
    i32 bucketIndex;
    f32 laserSize[3];
    u8 *bullet;
    i32 alpha;
    Laser *laser;
    f32 laserCenter[3];
    i32 rampWindow;

    bucketIndex = 0;
    bullet = reinterpret_cast<u8 *>(bulletManager) + 0x1A880;
    if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 10) & 1) != 0)
        return CHAIN_CALLBACK_RESULT_CONTINUE;

    g_ItemManager.OnUpdate();
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA538) = 0;
    bulletManager->FUN_004321b0();

    for (i = 0; i < 0x600; i++)
    {
        if (*reinterpret_cast<u16 *>(bullet + 0xDB8) == 0)
            goto nextBullet;

        ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA538);
            switch (*reinterpret_cast<u16 *>(bullet + 0xDB8))
            {
activateBullet:
            *reinterpret_cast<u16 *>(bullet + 0xDB8) = 1;
            *reinterpret_cast<ZunTimer *>(bullet + 0xD80) = 0;
            case 1:
updateBullet:
            reinterpret_cast<Bullet *>(bullet)->FUN_0042ffc0();
            if (*reinterpret_cast<u32 *>(bullet + 0xDAC) != 0)
            {
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 1) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_00432210();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x10) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_004322b0();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x20) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_00432390();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x40) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_00432460();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x100) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_004325a0();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x80) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_004326e0();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0xC00) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_00432830();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x400000) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_004329f0();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x800000) != 0)
                    reinterpret_cast<Bullet *>(bullet)->FUN_00432aa0();
                if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0x20000) != 0)
                {
                    if (*reinterpret_cast<ZunTimer *>(bullet + 0x105C) <= 0)
                        *reinterpret_cast<u32 *>(bullet + 0xDAC) ^= 0x20000;
                    else
                        (*reinterpret_cast<ZunTimer *>(bullet + 0x105C))--;
                }
            }

            if (*reinterpret_cast<i32 *>(bullet + 0xDA8) != 0)
                --*reinterpret_cast<i32 *>(bullet + 0xDA8);
            if (!g_EclScriptedGlobalUpdateFreeze)
                *reinterpret_cast<Float3 *>(bullet + 0xD44) += *reinterpret_cast<Float3 *>(bullet + 0xD50);

            if (*reinterpret_cast<i32 *>(bullet + 0xDA8) == 0)
            {
                if (!g_GameManager.IsWithinPlayfield(
                        reinterpret_cast<Float3 *>(bullet + 0xD44)->operator float *()[0],
                        reinterpret_cast<Float3 *>(bullet + 0xD44)->operator float *()[1],
                        *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(bullet + 0x224) + 0x34),
                        *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(bullet + 0x224) + 0x30)))
                {
                    if ((*reinterpret_cast<u32 *>(bullet + 0xDAC) & 0xDC0) != 0)
                    {
                        ++*reinterpret_cast<u16 *>(bullet + 0xDBA);
                        if (*reinterpret_cast<u16 *>(bullet + 0xDBA) >= 0x80)
                        {
                            reinterpret_cast<Bullet *>(bullet)->FUN_00432170();
                            goto nextBullet;
                        }
                    }
                    else
                    {
                        if (*reinterpret_cast<u16 *>(bullet + 0xDBA) == 0)
                        {
                            reinterpret_cast<Bullet *>(bullet)->FUN_00432170();
                            goto nextBullet;
                        }
                        --*reinterpret_cast<u16 *>(bullet + 0xDBA);
                    }
                }
                else
                    *reinterpret_cast<u16 *>(bullet + 0xDBA) = 0;
            }

            if (*reinterpret_cast<i8 *>(bullet + 0x10B4) == 0)
            {
                if (*reinterpret_cast<u8 *>(bullet + 0xDBD) == 0 &&
                    (i32)*reinterpret_cast<ZunTimer *>(bullet + 0xD8C) >= 16)
                {
                    collisionResult = g_Player.FUN_0044a470(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                            reinterpret_cast<Float3 *>(bullet + 0xD34));
                    if (collisionResult == 1)
                    {
                        *reinterpret_cast<u8 *>(bullet + 0xDBD) = 1;
                        goto lethalCollision;
                    }
                    if (collisionResult == 2 && (*reinterpret_cast<u32 *>(bullet + 0xDB0) & 0x1000) == 0)
                    {
                        *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
                        if (g_Player.bulletCancelItemType == 9)
                        {
                            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                        }
                        else if (g_Player.bulletCancelItemType >= 0)
                            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                    static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                    }
                    goto executeBulletScript;
                }

lethalCollision:
                collisionResult = g_Player.FUN_0044a230(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                        reinterpret_cast<Float3 *>(bullet + 0xD34));
                if (collisionResult != 0 &&
                    (collisionResult != 2 || (*reinterpret_cast<u32 *>(bullet + 0xDB0) & 0x1000) == 0))
                {
                    *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
                    if (collisionResult == 2)
                    {
                        if (g_Player.bulletCancelItemType == 9)
                        {
                            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                        }
                        else if (g_Player.bulletCancelItemType >= 0)
                            g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                    static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                    }
                }
            }
executeBulletScript:
            if (*reinterpret_cast<void **>(bullet + 0x220) != NULL)
                g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(bullet));
                break;
            case 2:
                (*reinterpret_cast<ZunTimer *>(bullet + 0xD8C))--;
                *reinterpret_cast<Float3 *>(bullet + 0xD44) +=
                    *reinterpret_cast<Float3 *>(bullet + 0xD50) / 2.0f;
                if ((*reinterpret_cast<u32 *>(bullet + 0xDB0) & 0x1000) == 0 &&
                    g_Player.FUN_00449ff0(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                         reinterpret_cast<Float3 *>(bullet + 0xD34)) == 2)
                    *reinterpret_cast<u8 *>(bullet + 0xDBE) = 1;
                if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(bullet + 0x2A4)) == 0)
                    break;
                if (*reinterpret_cast<u8 *>(bullet + 0xDBE) != 0)
                {
                    *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
                    if (g_Player.bulletCancelItemType == 9)
                    {
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                    }
                    else if (g_Player.bulletCancelItemType >= 0)
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                }
                goto activateBullet;
            case 3:
                (*reinterpret_cast<ZunTimer *>(bullet + 0xD8C))--;
                *reinterpret_cast<Float3 *>(bullet + 0xD44) +=
                    *reinterpret_cast<Float3 *>(bullet + 0xD50) / 2.5f;
                if ((*reinterpret_cast<u32 *>(bullet + 0xDB0) & 0x1000) == 0 &&
                    g_Player.FUN_00449ff0(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                         reinterpret_cast<Float3 *>(bullet + 0xD34)) == 2)
                    *reinterpret_cast<u8 *>(bullet + 0xDBE) = 1;
                if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(bullet + 0x548)) == 0)
                    break;
                if (*reinterpret_cast<u8 *>(bullet + 0xDBE) != 0)
                {
                    *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
                    if (g_Player.bulletCancelItemType == 9)
                    {
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                    }
                    else if (g_Player.bulletCancelItemType >= 0)
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                }
                goto activateBullet;
            case 4:
                (*reinterpret_cast<ZunTimer *>(bullet + 0xD8C))--;
                *reinterpret_cast<Float3 *>(bullet + 0xD44) +=
                    *reinterpret_cast<Float3 *>(bullet + 0xD50) / 3.0f;
                if ((*reinterpret_cast<u32 *>(bullet + 0xDB0) & 0x1000) == 0 &&
                    g_Player.FUN_00449ff0(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                         reinterpret_cast<Float3 *>(bullet + 0xD34)) == 2)
                    *reinterpret_cast<u8 *>(bullet + 0xDBE) = 1;
                if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(bullet + 0x7EC)) == 0)
                    break;
                if (*reinterpret_cast<u8 *>(bullet + 0xDBE) != 0)
                {
                    *reinterpret_cast<u16 *>(bullet + 0xDB8) = 5;
                    if (g_Player.bulletCancelItemType == 9)
                    {
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44), ITEM_TIME, 1);
                    }
                    else if (g_Player.bulletCancelItemType >= 0)
                        g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(bullet + 0xD44),
                                                static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                }
                goto activateBullet;
            case 5:
                *reinterpret_cast<Float3 *>(bullet + 0xD44) +=
                    *reinterpret_cast<Float3 *>(bullet + 0xD50) / 2.0f;
                if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(bullet + 0xA90)) != 0)
                {
                    reinterpret_cast<Bullet *>(bullet)->FUN_00432170();
                    goto nextBullet;
                }
                break;
            default:
                break;
            }

updateTimers:
            (*reinterpret_cast<ZunTimer *>(bullet + 0xD80))++;
            (*reinterpret_cast<ZunTimer *>(bullet + 0xD8C))++;
            *reinterpret_cast<void **>(bullet + 0xDC0) =
                *reinterpret_cast<void **>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA554 +
                                            *reinterpret_cast<u8 *>(bullet + 0xD42) * 4);
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA554 +
                                       *reinterpret_cast<u8 *>(bullet + 0xD42) * 4) = bullet;
nextBullet:
        --bucketIndex;
        if (bucketIndex < 0)
        {
            bucketIndex = 0x5FF;
            bullet += 0x645000;
        }
        bullet -= 0x10B8;
    }

    laser = &bulletManager->lasers[0];
    reinterpret_cast<Float3 *>(laserCenter)->operator float *();
    reinterpret_cast<Float3 *>(laserSize)->operator float *();
    for (i = 0; i < 0x100; i++, laser++)
    {
            if (laser->inUse == 0)
                continue;

            laser->endOffset += g_EclGameTimeScale * laser->speed;
            if (laser->endOffset - laser->startOffset > laser->startLength)
                laser->startOffset = laser->endOffset - laser->startLength;
            if (laser->startOffset < 0.0f)
                laser->startOffset = 0.0f;

            laserSize[1] = laser->width / 2.0f;
            if (laser->startOffset <= 0.0f)
                laserSize[0] = laser->endOffset - laser->startOffset;
            else
                laserSize[0] = (laser->endOffset - laser->startOffset) * 0.7f;
            laserCenter[0] = (laser->endOffset - laser->startOffset) / 2.0f +
                            laser->startOffset + laser->position.x;
            laserCenter[1] = laser->position.y;
            laser->vm0.scale.x = laser->width / laser->vm0.loadedSprite->widthPx;
            currentWidth = laser->endOffset - laser->startOffset;
            laser->vm0.scale.y = currentWidth / laser->vm0.loadedSprite->heightPx;
            laser->vm0.SetZRotation(
                AddNormalizeAngle(ZUN_PI / 2.0f + laser->angle, 0.0f));

            switch (laser->state)
            {
            case LASER_STATE_STARTING:
                if ((laser->flags & 1) != 0)
                {
                    alpha = (i32)((f32)laser->timer * 255.0f / laser->startTime);
                    if (alpha > 255)
                        alpha = 255;
                    laser->vm0.color1.d3dColor = alpha << 24;
                }
                else
                {
                    rampWindow = laser->startTime > 30
                                     ? 30
                                     : laser->startTime;
                    if (laser->startTime - rampWindow < (i32)laser->timer)
                        currentWidth = (f32)laser->timer * laser->width / laser->startTime;
                    else
                        currentWidth = 1.2f;
                    laser->currentWidth = currentWidth;
                    laser->vm0.scale.x = currentWidth / 16.0f;
                    laserSize[0] = currentWidth / 2.0f;
                }
                if (laser->timer >= laser->hitboxStartTime)
                    g_Player.CalcLaserHitbox(reinterpret_cast<Float3 *>(laserCenter), reinterpret_cast<Float3 *>(laserSize),
                                             &laser->position, laser->angle, 0);
                if (laser->timer < laser->startTime)
                    break;
                laser->timer = 0;
                ++laser->state;
                laser->currentWidth = laser->width;
            case LASER_STATE_ACTIVE:
                g_Player.CalcLaserHitbox(reinterpret_cast<Float3 *>(laserCenter), reinterpret_cast<Float3 *>(laserSize),
                                         &laser->position, laser->angle,
                                         ((i32)laser->timer) % 20 == 0);
                if (laser->timer < laser->duration)
                    break;
                laser->timer = 0;
                ++laser->state;
                if (laser->despawnDuration == 0)
                {
                    laser->inUse = 0;
                    continue;
                }
            case LASER_STATE_DESPAWNING:
                if ((laser->flags & 1) != 0)
                {
                    alpha = (i32)((f32)laser->timer * 255.0f / laser->startTime);
                    if (alpha > 255)
                        alpha = 255;
                    laser->vm0.color1.d3dColor = alpha << 24;
                }
                else if (laser->despawnDuration > 0)
                {
                    currentWidth = laser->width -
                                   (f32)laser->timer * laser->width / laser->despawnDuration;
                    laser->vm0.scale.x = currentWidth / 16.0f;
                    laserSize[0] = currentWidth / 2.0f;
                }
                if (laser->timer < laser->hitboxEndDelay)
                    g_Player.CalcLaserHitbox(reinterpret_cast<Float3 *>(laserCenter), reinterpret_cast<Float3 *>(laserSize),
                                             &laser->position, laser->angle, 0);
                if (laser->timer < laser->despawnDuration)
                    break;
                laser->inUse = 0;
                continue;
            }

            if (laser->startOffset >= 640.0f)
                laser->inUse = 0;
            laser->timer++;
            g_AnmManager->ExecuteScript(&laser->vm0);
        }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA53C) != 0)
        --*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA53C);
    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA540))++;
    ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(bulletManager) + 0x6BA54C);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
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

// FUNCTION: th08 0x432b50
#pragma var_order(i, sine, laser, halfLength, cosine, node, bulletManager)
ChainCallbackResult BulletManager::OnDraw(BulletManager *bulletManager)
{
    i32 i;
    f32 sine;
    Laser *laser;
    f32 halfLength;
    f32 cosine;
    Bullet *node;

    if (g_GameManager.flags.unk10)
        g_AnmManager->SetMixColor(0xfff01010);

    laser = bulletManager->lasers;
    g_ItemManager.OnDraw();

    for (i = 0; i < ARRAY_SIZE_SIGNED(bulletManager->lasers); i++, laser++)
    {
        if (laser->inUse == 0)
            continue;

        fsincos(&sine, &cosine, laser->angle);
        halfLength = (laser->endOffset - laser->startOffset) / 2.0f + laser->startOffset;

        laser->vm0.pos.operator float *()[0] =
            laser->position.operator float *()[0] + cosine * halfLength;
        laser->vm0.pos.operator float *()[1] =
            laser->position.operator float *()[1] + sine * halfLength;
        laser->vm0.pos.operator float *()[2] = 0.06f;
        laser->color = (laser->color & 0xff000000) | 0xffffff;
        laser->vm0.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        laser->vm0.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        g_AnmManager->Draw2D(&laser->vm0);

        if (laser->startOffset < 16.0f || laser->speed == 0.0f)
        {
            if (!laser->hideCapDuringStartup || laser->state != LASER_STATE_STARTING)
            {
                laser->vm1.pos.operator float *()[0] =
                    laser->position.operator float *()[0] +
                    cosine * laser->startOffset;
                laser->vm1.pos.operator float *()[1] =
                    laser->position.operator float *()[1] +
                    sine * laser->startOffset;
                laser->vm1.pos.operator float *()[2] = 0.05f;
                laser->vm1.color1.d3dColor = laser->vm0.color1.d3dColor;
                *reinterpret_cast<u32 *>(&laser->vm1.flags) |= 0x40;
                laser->vm1.color1.d3dColor = (laser->vm1.color1.d3dColor & 0xffffff) | 0xff000000;
                laser->vm1.scale.x =
                    laser->width / 10.0f * ((16.0f - laser->startOffset) / 16.0f);
                laser->vm1.scale.y = laser->vm1.scale.x;
                if (laser->vm1.scale.y <= 0.0f)
                {
                    laser->vm1.scale.x = laser->width / 10.0f;
                    laser->vm1.scale.y = laser->vm1.scale.x;
                }
                laser->vm1.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                laser->vm1.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                g_AnmManager->Draw2D(&laser->vm1);
            }
        }
    }

    for (i = 0; i < 6; i++)
    {
        node = *reinterpret_cast<Bullet **>(reinterpret_cast<u8 *>(bulletManager) + 0x6ba554 + i * 4);
        while (node != NULL)
        {
            node->DrawSingleBullet();
            node = *reinterpret_cast<Bullet **>(reinterpret_cast<u8 *>(node) + 0xdc0);
        }
    }

    g_EffectManager.DrawUnkTypeEffects();
    if (g_GameManager.flags.unk10)
        g_AnmManager->SetMixColorDefault();

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x432f20
ZunResult Bullet::DrawSingleBullet()
{
    AnmVm *vm;

    switch (*reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(this) + 0xdb8))
    {
    case 2:
        vm = &this->sprites.sprite1;
        break;
    case 3:
        vm = &this->sprites.sprite2;
        break;
    case 4:
        vm = &this->sprites.sprite3;
        break;
    case 5:
        vm = &this->sprites.sprite4;
        break;
    default:
        vm = &this->sprites.sprite0;
        break;
    }

    vm->pos.operator float *()[0] =
        g_GameManager.arcadeRegionTopLeftPos.x + this->position0.operator float *()[0];
    vm->pos.operator float *()[1] =
        g_GameManager.arcadeRegionTopLeftPos.y + this->position0.operator float *()[1];
    vm->pos.operator float *()[2] = 0.05f;
    vm->color1.d3dColor = (vm->color1.d3dColor & 0xff000000) | 0xffffff;

    if (vm->type != 0)
    {
        vm->SetZRotation(AddNormalizeAngle(
            ZUN_PI / 2.0f + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xd74), 0.0f));
    }

    return g_AnmManager->Draw2D(vm);
}

// FUNCTION: th08 0x433070
#pragma var_order(i)
ZunResult BulletManager::AddedCallback(BulletManager *bulletManager)
{
    u32 i;

    if (IsResourceReloadEnabled())
    {
        g_AsciiManagerDemoAnm0577EB4 = g_AnmManager->PreloadAnm(6, "etama.anm");
        bulletManager->bulletAnm = g_AsciiManagerDemoAnm0577EB4;
        if (bulletManager->bulletAnm == NULL)
            return ZUN_ERROR;
    }
    else
    {
        bulletManager->bulletAnm = g_AnmManager->GetAnm(6);
    }

    for (i = 0; i < 21; i++)
    {
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].sprite0, g_BulletSpriteScripts[i].scripts[0]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].sprite1, g_BulletSpriteScripts[i].scripts[1]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].sprite2, g_BulletSpriteScripts[i].scripts[2]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].sprite3, g_BulletSpriteScripts[i].scripts[3]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].sprite4, g_BulletSpriteScripts[i].scripts[4]);

        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].sprite0) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].sprite1) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].sprite2) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].sprite3) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].sprite4) + 0x1f8) |= 0x2000;

        bulletManager->bulletTypeSprites[i].sprite0.baseSpriteIndex =
            bulletManager->bulletTypeSprites[i].sprite0.activeSpriteIndex;
        *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd41) =
            (u8)bulletManager->bulletTypeSprites[i].sprite0.loadedSprite->heightPx;

        if (bulletManager->bulletTypeSprites[i].sprite0.loadedSprite->heightPx <= 8.0f)
        {
            bulletManager->bulletTypeSprites[i].position.x = 4.0f;
            bulletManager->bulletTypeSprites[i].position.y = 4.0f;
            *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 5;
        }
        else if (bulletManager->bulletTypeSprites[i].sprite0.loadedSprite->heightPx <= 16.0f)
        {
            switch (g_BulletSpriteScripts[i].scripts[0])
            {
            case 2:
            case 111:
            case 112:
                bulletManager->bulletTypeSprites[i].position.x = 4.0f;
                bulletManager->bulletTypeSprites[i].position.y = 4.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 4;
                break;
            case 4:
            case 6:
                bulletManager->bulletTypeSprites[i].position.x = 4.0f;
                bulletManager->bulletTypeSprites[i].position.y = 4.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 4;
                break;
            case 5:
                bulletManager->bulletTypeSprites[i].position.x = 4.0f;
                bulletManager->bulletTypeSprites[i].position.y = 4.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 3;
            case 106:
                bulletManager->bulletTypeSprites[i].position.x = 4.0f;
                bulletManager->bulletTypeSprites[i].position.y = 4.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 4;
                break;
            case 107:
            case 108:
                bulletManager->bulletTypeSprites[i].position.x = 4.0f;
                bulletManager->bulletTypeSprites[i].position.y = 4.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 4;
                break;
            default:
                bulletManager->bulletTypeSprites[i].position.x = 6.0f;
                bulletManager->bulletTypeSprites[i].position.y = 6.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 3;
                break;
            }
        }
        else if (bulletManager->bulletTypeSprites[i].sprite0.loadedSprite->heightPx <= 32.0f)
        {
            switch (g_BulletSpriteScripts[i].scripts[0])
            {
            case 8:
            case 113:
            case 114:
            case 115:
                bulletManager->bulletTypeSprites[i].position.x = 5.0f;
                bulletManager->bulletTypeSprites[i].position.y = 5.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 2;
                break;
            case 9:
            case 109:
            case 110:
                bulletManager->bulletTypeSprites[i].position.x = 8.0f;
                bulletManager->bulletTypeSprites[i].position.y = 8.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 1;
                break;
            default:
                bulletManager->bulletTypeSprites[i].position.x = 10.0f;
                bulletManager->bulletTypeSprites[i].position.y = 10.0f;
                *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 1;
                break;
            }
        }
        else
        {
            *(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i]) + 0xd42) = 0;
            bulletManager->bulletTypeSprites[i].position.x = 24.0f;
            bulletManager->bulletTypeSprites[i].position.y = 24.0f;
        }
    }

    g_ItemManager.Initialize();
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

// FUNCTION: th08 0x4338b0
i32 IsResourceReloadEnabled()
{
    return g_Supervisor.unk164;
}

// FUNCTION: th08 0x4338c0
i32 IsBulletManagerAnmReleaseRequired()
{
    return g_Supervisor.unk168;
}

} /* namespace th08 */
