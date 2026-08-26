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
    Bullet *bullet;
    i32 i;

    memset(this, 0, sizeof(BulletManager));
    this->bulletCursor = &this->bullets[0];
    this->bullets[0x600].state = BULLET_STATE_SENTINEL;
    this->cancelItemType = 6;

    bullet = &g_BulletManager.bullets[0];
    for (i = 0; i < 0x600; i++, bullet++)
    {
        bullet->sprites.bulletVm.scriptIndex = -1;
        bullet->sprites.despawnVm.scriptIndex = -1;
        bullet->sprites.spawnFastVm.scriptIndex = -1;
        bullet->sprites.spawnNormalVm.scriptIndex = -1;
        bullet->sprites.spawnSlowVm.scriptIndex = -1;
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
    Bullet *bullet;
    f32 angle;
    u32 transformFlags;

    i = 0;
    bullet = this->bulletCursor;
    for (i = 0; i < 0x600; i++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            break;
        bullet++;
        if (bullet->state == BULLET_STATE_SENTINEL)
            bullet = &this->bullets[0];
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

    bullet->state = BULLET_STATE_FIRED;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(bullet) + 0xDBC) = 1;
    bullet->isGrazed = 0;
    bullet->stateTimer = 0;
    bullet->collisionDisabled = 0;
    bullet->activeTimer = 0;
    bullet->speed = speed;
    bullet->angle = AddNormalizeAngle(angle, 0.0f);
    bullet->position = descriptor->position;
    bullet->position.operator float *()[2] = 0.1f;
    bullet->velocity.FromAngleMagnitude(
        angle, speed * g_EclGameTimeScale);

    bullet->activeTransformFlags = descriptor->transformFlags;
    bullet->color = descriptor->color;
    bullet->zoneTransitionCooldownFrames = 0;
    bullet->cancelledDuringSpawn = 0;

    CopyBulletAnmVmCore(&bullet->sprites.bulletVm, &descriptor->templateSprites->bulletVm);
    CopyBulletAnmVmCore(&bullet->sprites.despawnVm, &descriptor->templateSprites->despawnVm);
    bullet->sprites.collisionSize = descriptor->templateSprites->collisionSize;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&bullet->sprites) + 0xD40) =
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(descriptor->templateSprites) + 0xD40);
    bullet->sprites.height = descriptor->templateSprites->height;
    bullet->sprites.drawBucketIndex = descriptor->templateSprites->drawBucketIndex;
    bullet->transformSound = descriptor->transformSound;
    bullet->offscreenCullDelayFrames = 0;

    if (bullet->sprites.bulletVm.activeSpriteIndex !=
        descriptor->templateSprites->bulletVm.activeSpriteIndex + descriptor->color)
    {
        this->bulletAnm->SetSprite(&bullet->sprites.bulletVm,
            descriptor->templateSprites->bulletVm.activeSpriteIndex + descriptor->color);
    }

    if (bullet->sprites.despawnVm.activeSpriteIndex !=
        descriptor->templateSprites->despawnVm.activeSpriteIndex + descriptor->color)
    {
        if (bullet->sprites.bulletVm.loadedSprite->heightPx <= 16.0f)
        {
            this->bulletAnm->SetSprite(&bullet->sprites.despawnVm,
                descriptor->templateSprites->despawnVm.activeSpriteIndex +
                    g_BulletSpriteOffsetSmall[descriptor->color]);
        }
        else if (bullet->sprites.bulletVm.loadedSprite->heightPx <= 32.0f)
        {
            this->bulletAnm->SetSprite(&bullet->sprites.despawnVm,
                descriptor->templateSprites->despawnVm.activeSpriteIndex +
                    g_BulletSpriteOffsetMedium[descriptor->color]);
        }
        else
        {
            this->bulletAnm->SetSprite(&bullet->sprites.despawnVm,
                descriptor->templateSprites->despawnVm.activeSpriteIndex + descriptor->color);
        }
    }

    transformFlags = descriptor->transformFlags;
    if ((descriptor->transformFlags & BULLET_TRANSFORM_SPAWN_FAST) != 0)
    {
        CopyBulletAnmVmCore(&bullet->sprites.spawnFastVm, &descriptor->templateSprites->spawnFastVm);
        SelectBulletSprite(&bullet->sprites.spawnFastVm,
                           &descriptor->templateSprites->spawnFastVm,
                           &bullet->sprites.bulletVm, descriptor->color);
        bullet->state = BULLET_STATE_SPAWNING_FAST;
        bullet->position -= bullet->velocity * 4.0f;
    }
    else if ((descriptor->transformFlags & BULLET_TRANSFORM_SPAWN_NORMAL) != 0)
    {
        CopyBulletAnmVmCore(&bullet->sprites.spawnNormalVm, &descriptor->templateSprites->spawnNormalVm);
        SelectBulletSprite(&bullet->sprites.spawnNormalVm,
                           &descriptor->templateSprites->spawnNormalVm,
                           &bullet->sprites.bulletVm, descriptor->color);
        bullet->state = BULLET_STATE_SPAWNING_NORMAL;
        bullet->position -= bullet->velocity * 4.0f;
    }
    else if ((descriptor->transformFlags & BULLET_TRANSFORM_SPAWN_SLOW) != 0)
    {
        CopyBulletAnmVmCore(&bullet->sprites.spawnSlowVm, &descriptor->templateSprites->spawnSlowVm);
        SelectBulletSprite(&bullet->sprites.spawnSlowVm,
                           &descriptor->templateSprites->spawnSlowVm,
                           &bullet->sprites.bulletVm, descriptor->color);
        bullet->state = BULLET_STATE_SPAWNING_SLOW;
        bullet->position -= bullet->velocity * 4.0f;
    }

    memcpy(bullet->transforms, descriptor->transforms, sizeof(descriptor->transforms));
    bullet->transformFlags = descriptor->transformFlags;
    bullet->activeTransformFlags = 0;
    bullet->transformIndex = descriptor->transformStartIndex;
    bullet->FUN_0042ffc0();

    if (this->spawnSuppressionFrames != 0 &&
        (bullet->transformFlags & BULLET_TRANSFORM_CANCEL_IMMUNE) == 0)
        bullet->state = BULLET_STATE_DESPAWNING;

    bullet++;
    if (bullet->state == BULLET_STATE_SENTINEL)
        this->bulletCursor = &this->bullets[0];
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
    if (this->transformIndex >= 18)
        return;

    record = &this->transforms[this->transformIndex];
    if (record->kind == BULLET_TRANSFORM_NONE)
        return;
    if (record->allowWhileActive == 0 && this->activeTransformFlags != 0)
        return;
    if ((this->transformFlags & record->kind) == 0)
    {
        ++this->transformIndex;
        goto nextRecord;
    }

    switch (record->kind)
    {
    case BULLET_TRANSFORM_DECELERATE:
        this->activeTransformFlags |= BULLET_TRANSFORM_DECELERATE;
        this->exStates[BULLET_TRANSFORM_STATE_DECELERATION].timer = 0;
        *reinterpret_cast<i32 *>(
            &this->exStates[BULLET_TRANSFORM_STATE_DECELERATION].vector.z) = 0;
        break;

    case BULLET_TRANSFORM_ACCELERATE_VECTOR:
        this->activeTransformFlags |= BULLET_TRANSFORM_ACCELERATE_VECTOR;
        this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].accelerationMagnitude =
            record->float0;
        this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].accelerationAngle =
            record->float1 > -990.0f ? record->float1
                                    : this->angle;
        this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].timer = 0;
        this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].durationFrames =
            record->int0;
        this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].vector.FromAngleMagnitude(
            this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].accelerationAngle,
            g_EclGameTimeScale *
                this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].accelerationMagnitude);
        if (this->transformIndex != 0 && this->transformSound >= 0)
            g_SoundPlayer.PlaySoundByIdx(
                static_cast<SoundIdx>(this->transformSound), 0);
        break;

    case BULLET_TRANSFORM_ACCELERATE_POLAR:
        this->activeTransformFlags |= BULLET_TRANSFORM_ACCELERATE_POLAR;
        this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].speedDelta = record->float0;
        this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].angleDelta = record->float1;
        this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].timer = 0;
        this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].durationFrames = record->int0;
        if (this->transformIndex != 0 && this->transformSound >= 0)
            g_SoundPlayer.PlaySoundByIdx(
                static_cast<SoundIdx>(this->transformSound), 0);
        break;

    case BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE:
    case BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED:
    case BULLET_TRANSFORM_CHANGE_DIRECTION_ABSOLUTE:
        this->activeTransformFlags |= record->kind;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeAngle =
            record->float0;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeSpeed =
            record->float1 > -999.0f ? record->float1
                                    : this->speed;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer = 0;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames =
            record->int0;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeRepeatCount =
            record->int1;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted = 0;
        break;

    case BULLET_TRANSFORM_BOUNCE_ALL_EDGES:
    case BULLET_TRANSFORM_BOUNCE_EXCEPT_BOTTOM:
        this->activeTransformFlags |= record->kind;
        if (record->float0 >= 0.0f)
            this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bounceSpeed = record->float0;
        else
            this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bounceSpeed =
                this->speed;
        this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bounceLimit = record->int0;
        this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bouncesCompleted = 0;
        break;

    case BULLET_TRANSFORM_WRAP_X:
        this->activeTransformFlags |= record->kind;
        this->exStates[BULLET_TRANSFORM_STATE_WRAP].timer = record->int0;
        break;

    case BULLET_TRANSFORM_WRAP_Y:
        this->activeTransformFlags |= record->kind;
        this->exStates[BULLET_TRANSFORM_STATE_WRAP].timer = record->int0;
        break;

    case BULLET_TRANSFORM_WAIT:
        this->activeTransformFlags |= record->kind;
        this->exStates[BULLET_TRANSFORM_STATE_WAIT].timer = record->int0;
        break;

    case BULLET_TRANSFORM_SET_CULL_DELAY:
        this->offscreenCullDelayFrames = record->int0;
        ++this->transformIndex;
        goto nextRecord;

    case BULLET_TRANSFORM_SET_SPRITE:
        this->sprites = g_BulletManager.bulletTypeSprites[record->int0];
        g_BulletManager.bulletAnm->SetSprite(
            &this->sprites.bulletVm,
            this->sprites.bulletVm.activeSpriteIndex + record->int1);
        ++this->transformIndex;
        goto nextRecord;

    case BULLET_TRANSFORM_DESPAWN:
        this->state = BULLET_STATE_DESPAWNING;
        break;

    case BULLET_TRANSFORM_PLAY_SOUND:
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(record->int0), this->position.x);
        ++this->transformIndex;
        goto nextRecord;

    case BULLET_TRANSFORM_SPAWN_CHILD_PATTERN:
        {
            BulletSpawnDescriptor pattern;
            i32 fadeParent;
            pattern.position = this->position;
            fadeParent = record->int0 & 0x80000000;
            pattern.aimMode = (static_cast<u32>(record->int0) & 0x7F000000) >> 24;
            pattern.bulletType = (static_cast<u32>(record->int0) & 0x00FF0000) >> 16;
            pattern.color = (static_cast<u32>(record->int0) & 0x0000FF00) >> 8;
            pattern.transformStartIndex = record->int0 & 0xFF;
            pattern.count1 = static_cast<i16>(record->int1);
            pattern.speed1 = record->float0;
            pattern.speed2 = record->float1;

            ++record;
            ++this->transformIndex;
            pattern.count2 = static_cast<i16>(record->int0);
            pattern.transformFlags = record->int1;
            pattern.angle = record->float0;
            pattern.angleStep = record->float1;
            memcpy(pattern.transforms, this->transforms, sizeof(pattern.transforms));
            g_BulletManager.FUN_00430e10(&pattern);
            ++this->transformIndex;
            if (fadeParent != 0)
                this->state = BULLET_STATE_DESPAWNING;
            else
                goto nextRecord;
        }
        break;

    default:
        break;
    }

    ++this->transformIndex;
}

// FUNCTION: th08 0x430830
#pragma var_order(position, playerCollisionResult, bulletIndex, sine, bullet, laser, cosine, radius, this)
void BulletManager::RemoveAllBullets(i32 mode)
{
    Bullet *bullet = &g_BulletManager.bullets[0];
    i32 bulletIndex;
    i32 playerCollisionResult;
    Laser *laser;
    f32 position[3];
    f32 sine;
    f32 cosine;
    f32 radius;

    for (bulletIndex = 0; bulletIndex < 0x600; bulletIndex++, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED || bullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        playerCollisionResult = g_Player.FUN_00449ff0(&bullet->position,
                                                      &bullet->sprites.collisionSize);
        if (g_Player.FUN_00449ff0(&bullet->position, &bullet->sprites.collisionSize) == 2)
        {
            g_ItemManager.SpawnItem(&bullet->position, static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
            memset(bullet, 0, 0x10B8);
        }
        else if (mode != 4)
        {
            g_ItemManager.SpawnItem(&bullet->position,
                                    static_cast<ItemType>(this->cancelItemType), mode);
            memset(bullet, 0, 0x10B8);
        }
        else
        {
            bullet->state = BULLET_STATE_DESPAWNING;
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
                                            static_cast<ItemType>(this->cancelItemType), mode);
                    radius = radius + 1.0f;
                }
            }
        }

        laser->hitboxEndDelay = 0;
    }

    this->spawnSuppressionFrames = 10;
}


// FUNCTION: th08 0x430aa0
#pragma var_order(score, totalScore, bulletCount, bulletIndex, sine, bullet, position, laser, cosine, radius, this)
i32 BulletManager::DespawnBullets(i32 maxScore, i32 awardLaserItems)
{
    f32 radius;
    f32 cosine;
    Laser *laser;
    f32 position[3];
    Bullet *bullet;
    f32 sine;
    i32 bulletIndex;
    i32 bulletCount;
    i32 totalScore;
    i32 score;

    totalScore = 0;
    score = 2000;
    bulletCount = 0;
    bullet = &g_BulletManager.bullets[0];
    for (bulletIndex = 0; bulletIndex < 0x600; bulletIndex++, bullet++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
        {
            continue;
        }

        if (g_Player.FUN_00449ff0(&bullet->position,
                                 &bullet->sprites.collisionSize) == 2)
        {
            g_ItemManager.SpawnItem(&bullet->position,
                                    static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
        }
        else
        {
            g_ItemManager.SpawnItem(
                &bullet->position,
                static_cast<ItemType>(this->cancelItemType), 1);
        }

        g_AsciiManager.CreateScorePopup(&bullet->position, score,
                                        score >= maxScore ? -256 : -1);
        totalScore += score;
        bulletCount++;
        score += 20;
        if (score > maxScore)
        {
            score = maxScore;
        }
        bullet->state = BULLET_STATE_DESPAWNING;
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
                    static_cast<ItemType>(this->cancelItemType), 1);
                radius = laser->startOffset;
                fsincos(&sine, &cosine, laser->angle);
                while (laser->endOffset > radius)
                {
                    position[0] = cosine * radius + laser->position.x;
                    position[1] = sine * radius + laser->position.y;
                    position[2] = 0.0f;
                    g_ItemManager.SpawnItem(
                        reinterpret_cast<Float3 *>(position),
                        static_cast<ItemType>(this->cancelItemType), 1);
                    radius += 32.0f;
                }
            }
        }

        laser->hitboxEndDelay = 0;
    }

    this->spawnSuppressionFrames = 10;
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
        if (bullet->state == BULLET_STATE_UNUSED ||
            bullet->state == BULLET_STATE_DESPAWNING)
            continue;
        delta = bullet->position - *position;
        if (D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&delta)) > radius)
            continue;
        g_ItemManager.SpawnItem(&bullet->position, static_cast<ItemType>(6), 1);
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
    if (g_BulletManager.activeBulletCount >= 0x600)
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
    if ((descriptor->transformFlags & BULLET_TRANSFORM_PLAY_SPAWN_SOUND) != 0)
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
    if (this->spawnSuppressionFrames != 0 &&
        (descriptor->transformFlags & BULLET_TRANSFORM_SPAWN_NORMAL) == 0)
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
    Bullet *bullet;
    i32 alpha;
    Laser *laser;
    f32 laserCenter[3];
    i32 rampWindow;

    bucketIndex = 0;
    bullet = &bulletManager->bullets[0];
    if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 10) & 1) != 0)
        return CHAIN_CALLBACK_RESULT_CONTINUE;

    g_ItemManager.OnUpdate();
    bulletManager->activeBulletCount = 0;
    bulletManager->FUN_004321b0();

    for (i = 0; i < 0x600; i++)
    {
        if (bullet->state == BULLET_STATE_UNUSED)
            goto nextBullet;

        ++bulletManager->activeBulletCount;
            switch (bullet->state)
            {
activateBullet:
            bullet->state = BULLET_STATE_FIRED;
            bullet->stateTimer = 0;
            case BULLET_STATE_FIRED:
updateBullet:
            bullet->FUN_0042ffc0();
            if (bullet->activeTransformFlags != 0)
            {
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_DECELERATE) != 0)
                    bullet->FUN_00432210();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_ACCELERATE_VECTOR) != 0)
                    bullet->FUN_004322b0();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_ACCELERATE_POLAR) != 0)
                    bullet->FUN_00432390();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE) != 0)
                    bullet->FUN_00432460();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_CHANGE_DIRECTION_ABSOLUTE) != 0)
                    bullet->FUN_004325a0();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED) != 0)
                    bullet->FUN_004326e0();
                if ((bullet->activeTransformFlags &
                     (BULLET_TRANSFORM_BOUNCE_ALL_EDGES |
                      BULLET_TRANSFORM_BOUNCE_EXCEPT_BOTTOM)) != 0)
                    bullet->FUN_00432830();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_WRAP_X) != 0)
                    bullet->FUN_004329f0();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_WRAP_Y) != 0)
                    bullet->FUN_00432aa0();
                if ((bullet->activeTransformFlags & BULLET_TRANSFORM_WAIT) != 0)
                {
                    if (bullet->exStates[BULLET_TRANSFORM_STATE_WAIT].timer <= 0)
                        bullet->activeTransformFlags ^= BULLET_TRANSFORM_WAIT;
                    else
                        bullet->exStates[BULLET_TRANSFORM_STATE_WAIT].timer--;
                }
            }

            if (bullet->offscreenCullDelayFrames != 0)
                --bullet->offscreenCullDelayFrames;
            if (!g_EclScriptedGlobalUpdateFreeze)
                bullet->position += bullet->velocity;

            if (bullet->offscreenCullDelayFrames == 0)
            {
                if (!g_GameManager.IsWithinPlayfield(
                        bullet->position.operator float *()[0],
                        bullet->position.operator float *()[1],
                        bullet->sprites.bulletVm.loadedSprite->widthPx,
                        bullet->sprites.bulletVm.loadedSprite->heightPx))
                {
                    if ((bullet->activeTransformFlags &
                         (BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE |
                          BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED |
                          BULLET_TRANSFORM_CHANGE_DIRECTION_ABSOLUTE |
                          BULLET_TRANSFORM_BOUNCE_ALL_EDGES |
                          BULLET_TRANSFORM_BOUNCE_EXCEPT_BOTTOM)) != 0)
                    {
                        ++bullet->offscreenFrames;
                        if (bullet->offscreenFrames >= 0x80)
                        {
                            bullet->FUN_00432170();
                            goto nextBullet;
                        }
                    }
                    else
                    {
                        if (bullet->offscreenFrames == 0)
                        {
                            bullet->FUN_00432170();
                            goto nextBullet;
                        }
                        --bullet->offscreenFrames;
                    }
                }
                else
                    bullet->offscreenFrames = 0;
            }

            if (bullet->collisionDisabled == 0)
            {
                if (bullet->isGrazed == 0 &&
                    (i32)bullet->activeTimer >= 16)
                {
                    collisionResult = g_Player.FUN_0044a470(&bullet->position,
                                                            &bullet->sprites.collisionSize);
                    if (collisionResult == 1)
                    {
                        bullet->isGrazed = 1;
                        goto lethalCollision;
                    }
                    if (collisionResult == 2 &&
                        (bullet->transformFlags & BULLET_TRANSFORM_CANCEL_IMMUNE) == 0)
                    {
                        bullet->state = BULLET_STATE_DESPAWNING;
                        if (g_Player.bulletCancelItemType == 9)
                        {
                            g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                            g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                        }
                        else if (g_Player.bulletCancelItemType >= 0)
                            g_ItemManager.SpawnItem(&bullet->position,
                                                    static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                    }
                    goto executeBulletScript;
                }

lethalCollision:
                collisionResult = g_Player.FUN_0044a230(&bullet->position,
                                                        &bullet->sprites.collisionSize);
                if (collisionResult != 0 &&
                    (collisionResult != 2 ||
                     (bullet->transformFlags & BULLET_TRANSFORM_CANCEL_IMMUNE) == 0))
                {
                    bullet->state = BULLET_STATE_DESPAWNING;
                    if (collisionResult == 2)
                    {
                        if (g_Player.bulletCancelItemType == 9)
                        {
                            g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                            g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                        }
                        else if (g_Player.bulletCancelItemType >= 0)
                            g_ItemManager.SpawnItem(&bullet->position,
                                                    static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                    }
                }
            }
executeBulletScript:
            if (bullet->sprites.bulletVm.currentInstruction != NULL)
                g_AnmManager->ExecuteScript(&bullet->sprites.bulletVm);
                break;
            case BULLET_STATE_SPAWNING_FAST:
                bullet->activeTimer--;
                bullet->position += bullet->velocity / 2.0f;
                if ((bullet->transformFlags & BULLET_TRANSFORM_CANCEL_IMMUNE) == 0 &&
                    g_Player.FUN_00449ff0(&bullet->position,
                                         &bullet->sprites.collisionSize) == 2)
                    bullet->cancelledDuringSpawn = 1;
                if (g_AnmManager->ExecuteScript(&bullet->sprites.spawnFastVm) == 0)
                    break;
                if (bullet->cancelledDuringSpawn != 0)
                {
                    bullet->state = BULLET_STATE_DESPAWNING;
                    if (g_Player.bulletCancelItemType == 9)
                    {
                        g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                        g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                    }
                    else if (g_Player.bulletCancelItemType >= 0)
                        g_ItemManager.SpawnItem(&bullet->position,
                                                static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                }
                goto activateBullet;
            case BULLET_STATE_SPAWNING_NORMAL:
                bullet->activeTimer--;
                bullet->position += bullet->velocity / 2.5f;
                if ((bullet->transformFlags & BULLET_TRANSFORM_CANCEL_IMMUNE) == 0 &&
                    g_Player.FUN_00449ff0(&bullet->position,
                                         &bullet->sprites.collisionSize) == 2)
                    bullet->cancelledDuringSpawn = 1;
                if (g_AnmManager->ExecuteScript(&bullet->sprites.spawnNormalVm) == 0)
                    break;
                if (bullet->cancelledDuringSpawn != 0)
                {
                    bullet->state = BULLET_STATE_DESPAWNING;
                    if (g_Player.bulletCancelItemType == 9)
                    {
                        g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                        g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                    }
                    else if (g_Player.bulletCancelItemType >= 0)
                        g_ItemManager.SpawnItem(&bullet->position,
                                                static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                }
                goto activateBullet;
            case BULLET_STATE_SPAWNING_SLOW:
                bullet->activeTimer--;
                bullet->position += bullet->velocity / 3.0f;
                if ((bullet->transformFlags & BULLET_TRANSFORM_CANCEL_IMMUNE) == 0 &&
                    g_Player.FUN_00449ff0(&bullet->position,
                                         &bullet->sprites.collisionSize) == 2)
                    bullet->cancelledDuringSpawn = 1;
                if (g_AnmManager->ExecuteScript(&bullet->sprites.spawnSlowVm) == 0)
                    break;
                if (bullet->cancelledDuringSpawn != 0)
                {
                    bullet->state = BULLET_STATE_DESPAWNING;
                    if (g_Player.bulletCancelItemType == 9)
                    {
                        g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                        g_ItemManager.SpawnItem(&bullet->position, ITEM_TIME, 1);
                    }
                    else if (g_Player.bulletCancelItemType >= 0)
                        g_ItemManager.SpawnItem(&bullet->position,
                                                static_cast<ItemType>(g_Player.bulletCancelItemType), 1);
                }
                goto activateBullet;
            case BULLET_STATE_DESPAWNING:
                bullet->position += bullet->velocity / 2.0f;
                if (g_AnmManager->ExecuteScript(&bullet->sprites.despawnVm) != 0)
                {
                    bullet->FUN_00432170();
                    goto nextBullet;
                }
                break;
            default:
                break;
            }

updateTimers:
            bullet->stateTimer++;
            bullet->activeTimer++;
            bullet->nextInDrawBucket =
                bulletManager->drawBuckets[bullet->sprites.drawBucketIndex];
            bulletManager->drawBuckets[bullet->sprites.drawBucketIndex] = bullet;
nextBullet:
        --bucketIndex;
        if (bucketIndex < 0)
        {
            bucketIndex = 0x5FF;
            bullet += 0x600;
        }
        bullet--;
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

    if (bulletManager->spawnSuppressionFrames != 0)
        --bulletManager->spawnSuppressionFrames;
    bulletManager->timer++;
    ++bulletManager->frameCounter;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x432170
void Bullet::FUN_00432170()
{
    this->state = BULLET_STATE_UNUSED;
    this->stateTimer = 0;
    this->activeTimer = 0;
}

// FUNCTION: th08 0x4321b0
void BulletManager::FUN_004321b0()
{
    this->drawBuckets[5] = NULL;
    this->drawBuckets[4] = NULL;
    this->drawBuckets[3] = NULL;
    this->drawBuckets[2] = NULL;
    this->drawBuckets[1] = NULL;
    this->drawBuckets[0] = NULL;
}

// FUNCTION: th08 0x432210
#pragma var_order(magnitude, this)
void Bullet::FUN_00432210()
{
    f32 magnitude;

    if (this->exStates[BULLET_TRANSFORM_STATE_DECELERATION].timer <= 16)
    {
        magnitude =
            5.0f -
            ((f32)this->exStates[BULLET_TRANSFORM_STATE_DECELERATION].timer * 5.0f) /
                16.0f;
        this->velocity.FromAngleMagnitude(this->angle,
                                 (magnitude + this->speed) *
                                     *reinterpret_cast<f32 *>(0x17CE8E0));
    }
    else
    {
        this->activeTransformFlags ^= BULLET_TRANSFORM_DECELERATE;
    }

    this->exStates[BULLET_TRANSFORM_STATE_DECELERATION].timer++;
}


// FUNCTION: th08 0x4322b0
#pragma var_order(delta, this)
void Bullet::FUN_004322b0()
{
    if (this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].timer >=
        this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].durationFrames)
    {
        this->activeTransformFlags &= ~BULLET_TRANSFORM_ACCELERATE_VECTOR;
    }
    else
    {
        this->velocity +=
            this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].vector *
            *reinterpret_cast<f32 *>(0x17CE8E0);

        if (fabsf(this->velocity.x) > 0.0001f ||
            fabsf(this->velocity.y) > 0.0001f)
        {
            this->angle = VectorAngle(this->velocity.y, this->velocity.x);
        }
    }

    this->exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION].timer++;
}

// FUNCTION: th08 0x432390
void Bullet::FUN_00432390()
{
    if (this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].timer >=
        this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].durationFrames)
    {
        this->activeTransformFlags &= ~BULLET_TRANSFORM_ACCELERATE_POLAR;
    }
    else
    {
        this->angle =
            AddNormalizeAngle(this->angle,
                              *reinterpret_cast<f32 *>(0x17CE8E0) *
                                  this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].angleDelta);
        this->speed +=
            *reinterpret_cast<f32 *>(0x17CE8E0) *
            this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].speedDelta;
        this->velocity.FromAngleMagnitude(this->angle,
                                 *reinterpret_cast<f32 *>(0x17CE8E0) *
                                     this->speed);
    }

    this->exStates[BULLET_TRANSFORM_STATE_POLAR_ACCELERATION].timer++;
}

// FUNCTION: th08 0x432460
#pragma var_order(magnitude, this)
void Bullet::FUN_00432460()
{
    f32 magnitude;

    if (this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer >=
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames)
    {
        if (this->transformSound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(this->transformSound), 0);
        }
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted += 1;
        if (this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted >=
            this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeRepeatCount)
        {
            this->activeTransformFlags &= ~BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE;
        }
        this->angle +=
            this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeAngle;
        *reinterpret_cast<i32 *>(&this->speed) =
            *reinterpret_cast<i32 *>(
                &this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeSpeed);
        magnitude = this->speed;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer = 0;
    }
    else
    {
        magnitude = this->speed -
                    ((f32)this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer *
                     this->speed) /
                        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames;
    }

    this->velocity.FromAngleMagnitude(this->angle,
                             magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
    this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer++;
}

// FUNCTION: th08 0x4325a0
#pragma var_order(magnitude, this)
void Bullet::FUN_004325a0()
{
    f32 magnitude;

    if (this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer >=
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames)
    {
        if (this->transformSound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(this->transformSound), 0);
        }
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted += 1;
        if (this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted >=
            this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeRepeatCount)
        {
            this->activeTransformFlags &= ~BULLET_TRANSFORM_CHANGE_DIRECTION_ABSOLUTE;
        }
        *reinterpret_cast<i32 *>(&this->angle) =
            *reinterpret_cast<i32 *>(
                &this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeAngle);
        *reinterpret_cast<i32 *>(&this->speed) =
            *reinterpret_cast<i32 *>(
                &this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeSpeed);
        magnitude = this->speed;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer = 0;
    }
    else
    {
        magnitude = this->speed -
                    ((f32)this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer *
                     this->speed) /
                        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames;
    }

    this->velocity.FromAngleMagnitude(this->angle,
                             magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
    this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer++;
}

// FUNCTION: th08 0x4326e0
#pragma var_order(magnitude, this)
void Bullet::FUN_004326e0()
{
    f32 magnitude;

    if (this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer >=
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames)
    {
        if (this->transformSound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(this->transformSound), 0);
        }
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted += 1;
        if (this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangesCompleted >=
            this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeRepeatCount)
        {
            this->activeTransformFlags &= ~BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED;
        }
        this->angle =
            AddNormalizeAngle(g_Player.FUN_0044c1b0(&this->position),
                              this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeAngle);
        *reinterpret_cast<i32 *>(&this->speed) =
            *reinterpret_cast<i32 *>(
                &this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeSpeed);
        magnitude = this->speed;
        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer = 0;
    }
    else
    {
        magnitude = this->speed -
                    ((f32)this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer *
                     this->speed) /
                        this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].directionChangeIntervalFrames;
    }

    this->velocity.FromAngleMagnitude(this->angle,
                             magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
    this->exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE].timer++;
}


// FUNCTION: th08 0x432830
#pragma var_order(magnitude, this)
void Bullet::FUN_00432830()
{
    f32 magnitude;

    if (!g_GameManager.IsWithinPlayfield(this->position.operator float *()[0],
                                         this->position.operator float *()[1],
                                         this->sprites.bulletVm.loadedSprite->widthPx,
                                         this->sprites.bulletVm.loadedSprite->heightPx))
    {
        if (this->transformSound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(this->transformSound), 0);
        }

        if (this->position.x < 0.0f || this->position.x >= 384.0f)
        {
            this->angle = -this->angle - ZUN_PI;
            this->angle = AddNormalizeAngle(this->angle, 0.0f);
        }

        if (this->position.y < 0.0f ||
            (this->position.y >= 448.0f &&
             (this->activeTransformFlags & BULLET_TRANSFORM_BOUNCE_ALL_EDGES) != 0))
        {
            this->angle = -this->angle;
        }

        *reinterpret_cast<i32 *>(&this->speed) =
            *reinterpret_cast<i32 *>(
                &this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bounceSpeed);
        magnitude = this->speed;
        this->velocity.FromAngleMagnitude(this->angle,
                                 magnitude * *reinterpret_cast<f32 *>(0x17CE8E0));
        this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bouncesCompleted += 1;
        if (this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bouncesCompleted >=
            this->exStates[BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE].bounceLimit)
        {
            this->activeTransformFlags &=
                ~(BULLET_TRANSFORM_BOUNCE_ALL_EDGES |
                  BULLET_TRANSFORM_BOUNCE_EXCEPT_BOTTOM);
        }
    }
}

// FUNCTION: th08 0x4329f0
void Bullet::FUN_004329f0()
{
    if (this->position.x < 0.0)
    {
        this->position.x += 384.0f;
    }
    else if (this->position.x > 384.0)
    {
        this->position.x -= 384.0f;
    }

    if (this->exStates[BULLET_TRANSFORM_STATE_WRAP].timer <= 0)
    {
        this->activeTransformFlags ^= BULLET_TRANSFORM_WRAP_X;
    }
    else
    {
        this->exStates[BULLET_TRANSFORM_STATE_WRAP].timer--;
    }
}

// FUNCTION: th08 0x432aa0
void Bullet::FUN_00432aa0()
{
    if (this->position.y < 0.0)
    {
        this->position.y += 448.0f;
    }
    else if (this->position.y > 448.0)
    {
        this->position.y -= 448.0f;
    }

    if (this->exStates[BULLET_TRANSFORM_STATE_WRAP].timer <= 0)
    {
        this->activeTransformFlags ^= BULLET_TRANSFORM_WRAP_Y;
    }
    else
    {
        this->exStates[BULLET_TRANSFORM_STATE_WRAP].timer--;
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
        node = bulletManager->drawBuckets[i];
        while (node != NULL)
        {
            node->DrawSingleBullet();
            node = node->nextInDrawBucket;
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

    switch (this->state)
    {
    case BULLET_STATE_SPAWNING_FAST:
        vm = &this->sprites.spawnFastVm;
        break;
    case BULLET_STATE_SPAWNING_NORMAL:
        vm = &this->sprites.spawnNormalVm;
        break;
    case BULLET_STATE_SPAWNING_SLOW:
        vm = &this->sprites.spawnSlowVm;
        break;
    case BULLET_STATE_DESPAWNING:
        vm = &this->sprites.despawnVm;
        break;
    default:
        vm = &this->sprites.bulletVm;
        break;
    }

    vm->pos.operator float *()[0] =
        g_GameManager.arcadeRegionTopLeftPos.x + this->position.operator float *()[0];
    vm->pos.operator float *()[1] =
        g_GameManager.arcadeRegionTopLeftPos.y + this->position.operator float *()[1];
    vm->pos.operator float *()[2] = 0.05f;
    vm->color1.d3dColor = (vm->color1.d3dColor & 0xff000000) | 0xffffff;

    if (vm->type != 0)
    {
        vm->SetZRotation(AddNormalizeAngle(
            ZUN_PI / 2.0f + this->angle, 0.0f));
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
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].bulletVm, g_BulletSpriteScripts[i].scripts[0]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].spawnFastVm, g_BulletSpriteScripts[i].scripts[1]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].spawnNormalVm, g_BulletSpriteScripts[i].scripts[2]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].spawnSlowVm, g_BulletSpriteScripts[i].scripts[3]);
        bulletManager->bulletAnm->SetAndExecuteScriptIdx(&bulletManager->bulletTypeSprites[i].despawnVm, g_BulletSpriteScripts[i].scripts[4]);

        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].bulletVm) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].spawnFastVm) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].spawnNormalVm) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].spawnSlowVm) + 0x1f8) |= 0x2000;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&bulletManager->bulletTypeSprites[i].despawnVm) + 0x1f8) |= 0x2000;

        bulletManager->bulletTypeSprites[i].bulletVm.baseSpriteIndex =
            bulletManager->bulletTypeSprites[i].bulletVm.activeSpriteIndex;
        bulletManager->bulletTypeSprites[i].height =
            (u8)bulletManager->bulletTypeSprites[i].bulletVm.loadedSprite->heightPx;

        if (bulletManager->bulletTypeSprites[i].bulletVm.loadedSprite->heightPx <= 8.0f)
        {
            bulletManager->bulletTypeSprites[i].collisionSize.x = 4.0f;
            bulletManager->bulletTypeSprites[i].collisionSize.y = 4.0f;
            bulletManager->bulletTypeSprites[i].drawBucketIndex = 5;
        }
        else if (bulletManager->bulletTypeSprites[i].bulletVm.loadedSprite->heightPx <= 16.0f)
        {
            switch (g_BulletSpriteScripts[i].scripts[0])
            {
            case 2:
            case 111:
            case 112:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 4.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 4.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 4;
                break;
            case 4:
            case 6:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 4.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 4.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 4;
                break;
            case 5:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 4.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 4.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 3;
            case 106:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 4.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 4.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 4;
                break;
            case 107:
            case 108:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 4.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 4.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 4;
                break;
            default:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 6.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 6.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 3;
                break;
            }
        }
        else if (bulletManager->bulletTypeSprites[i].bulletVm.loadedSprite->heightPx <= 32.0f)
        {
            switch (g_BulletSpriteScripts[i].scripts[0])
            {
            case 8:
            case 113:
            case 114:
            case 115:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 5.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 5.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 2;
                break;
            case 9:
            case 109:
            case 110:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 8.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 8.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 1;
                break;
            default:
                bulletManager->bulletTypeSprites[i].collisionSize.x = 10.0f;
                bulletManager->bulletTypeSprites[i].collisionSize.y = 10.0f;
                bulletManager->bulletTypeSprites[i].drawBucketIndex = 1;
                break;
            }
        }
        else
        {
            bulletManager->bulletTypeSprites[i].drawBucketIndex = 0;
            bulletManager->bulletTypeSprites[i].collisionSize.x = 24.0f;
            bulletManager->bulletTypeSprites[i].collisionSize.y = 24.0f;
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
