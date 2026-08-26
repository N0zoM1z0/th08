#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"

namespace th08
{

struct BulletTypeSprites;

struct BulletTransformRecord
{
    f32 float0;
    f32 float1;
    i32 int0;
    i32 int1;
    u32 kind;
    i32 allowWhileActive;
};
C_ASSERT(sizeof(BulletTransformRecord) == 0x18);

enum BulletTransformKind
{
    BULLET_TRANSFORM_NONE = 0,
    BULLET_TRANSFORM_DECELERATE = 0x1,
    BULLET_TRANSFORM_SPAWN_FAST = 0x2,
    BULLET_TRANSFORM_SPAWN_NORMAL = 0x4,
    BULLET_TRANSFORM_SPAWN_SLOW = 0x8,
    BULLET_TRANSFORM_ACCELERATE_VECTOR = 0x10,
    BULLET_TRANSFORM_ACCELERATE_POLAR = 0x20,
    BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE = 0x40,
    BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED = 0x80,
    BULLET_TRANSFORM_CHANGE_DIRECTION_ABSOLUTE = 0x100,
    BULLET_TRANSFORM_PLAY_SPAWN_SOUND = 0x200,
    BULLET_TRANSFORM_BOUNCE_ALL_EDGES = 0x400,
    BULLET_TRANSFORM_BOUNCE_EXCEPT_BOTTOM = 0x800,
    BULLET_TRANSFORM_CANCEL_IMMUNE = 0x1000,
    BULLET_TRANSFORM_SET_CULL_DELAY = 0x2000,
    BULLET_TRANSFORM_SET_SPRITE = 0x4000,
    BULLET_TRANSFORM_WAIT = 0x20000,
    BULLET_TRANSFORM_DESPAWN = 0x40000,
    BULLET_TRANSFORM_PLAY_SOUND = 0x80000,
    BULLET_TRANSFORM_WRAP_X = 0x400000,
    BULLET_TRANSFORM_WRAP_Y = 0x800000,
    BULLET_TRANSFORM_SPAWN_CHILD_PATTERN = 0x1000000,
};

struct BulletSpawnDescriptor
{
    i16 bulletType;
    i16 color;
    Float3 position;
    f32 angle;
    f32 angleStep;
    f32 speed1;
    f32 speed2;
    BulletTransformRecord transforms[18];
    f32 laserStartOffset;       // +0x1D0
    f32 laserEndOffset;         // +0x1D4
    f32 laserStartLength;       // +0x1D8
    f32 laserWidth;             // +0x1DC
    i32 laserStartTime;         // +0x1E0
    i32 laserDuration;          // +0x1E4
    i32 laserDespawnDuration;   // +0x1E8
    i32 laserHitboxStartTime;   // +0x1EC
    i32 laserHitboxEndDelay;    // +0x1F0
    i16 count1;
    i16 count2;
    u16 aimMode;
    u16 unknown1FA;
    u32 transformFlags;
    i32 spawnSound;
    i32 transformSound;
    i32 transformStartIndex;
    BulletTypeSprites *templateSprites;

    BulletSpawnDescriptor();
};
C_ASSERT(sizeof(BulletSpawnDescriptor) == 0x210);

i32 IsBulletManagerAnmReleaseRequired();

struct BulletTypeSprites
{
    BulletTypeSprites();

    AnmVm bulletVm;
    AnmVm spawnFastVm;
    AnmVm spawnNormalVm;
    AnmVm spawnSlowVm;
    AnmVm despawnVm;
    Float3 collisionSize;
    unknown_fields(0xd40, 1);
    u8 height;
    u8 drawBucketIndex;
    unknown_fields(0xd43, 1);
};
C_ASSERT(sizeof(BulletTypeSprites) == 0xd44);
C_ASSERT(offsetof(BulletTypeSprites, spawnFastVm) == 0x2a4);
C_ASSERT(offsetof(BulletTypeSprites, spawnNormalVm) == 0x548);
C_ASSERT(offsetof(BulletTypeSprites, spawnSlowVm) == 0x7ec);
C_ASSERT(offsetof(BulletTypeSprites, despawnVm) == 0xa90);
C_ASSERT(offsetof(BulletTypeSprites, collisionSize) == 0xd34);
C_ASSERT(offsetof(BulletTypeSprites, height) == 0xd41);
C_ASSERT(offsetof(BulletTypeSprites, drawBucketIndex) == 0xd42);

struct BulletExState
{
    BulletExState();

    ZunTimer timer;
    union
    {
        f32 float0;
        f32 accelerationMagnitude;
        f32 speedDelta;
        f32 directionChangeSpeed;
        f32 bounceSpeed;
    };
    union
    {
        f32 float1;
        f32 accelerationAngle;
        f32 angleDelta;
        f32 directionChangeAngle;
    };
    Float3 vector;
    union
    {
        i32 int0;
        i32 durationFrames;
        i32 directionChangeIntervalFrames;
        i32 bouncesCompleted;
    };
    union
    {
        i32 int1;
        i32 directionChangeRepeatCount;
        i32 bounceLimit;
    };
    union
    {
        i32 int2;
        i32 directionChangesCompleted;
    };
};
C_ASSERT(sizeof(BulletExState) == 0x2c);
C_ASSERT(offsetof(BulletExState, float0) == 0xc);
C_ASSERT(offsetof(BulletExState, float1) == 0x10);
C_ASSERT(offsetof(BulletExState, vector) == 0x14);
C_ASSERT(offsetof(BulletExState, int0) == 0x20);
C_ASSERT(offsetof(BulletExState, int1) == 0x24);
C_ASSERT(offsetof(BulletExState, int2) == 0x28);

enum BulletTransformStateSlot
{
    BULLET_TRANSFORM_STATE_DECELERATION = 0,
    BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION = 1,
    BULLET_TRANSFORM_STATE_POLAR_ACCELERATION = 2,
    BULLET_TRANSFORM_STATE_DIRECTION_CHANGE = 3,
    BULLET_TRANSFORM_STATE_BOUNDARY_BOUNCE = 4,
    BULLET_TRANSFORM_STATE_WAIT = 5,
    BULLET_TRANSFORM_STATE_WRAP = 6,
};

enum LaserState
{
    LASER_STATE_STARTING = 0,
    LASER_STATE_ACTIVE = 1,
    LASER_STATE_DESPAWNING = 2,
};

struct Laser
{
    Laser();

    AnmVm vm0;
    AnmVm vm1;
    Float3 position;
    f32 angle;                 // +0x554
    f32 startOffset;           // +0x558
    f32 endOffset;             // +0x55C
    f32 startLength;           // +0x560
    f32 width;                 // +0x564
    f32 currentWidth;          // +0x568
    f32 speed;                 // +0x56C
    i32 startTime;             // +0x570
    i32 hitboxStartTime;       // +0x574
    i32 duration;              // +0x578
    i32 despawnDuration;       // +0x57C
    i32 hitboxEndDelay;        // +0x580
    i32 inUse;                 // +0x584
    ZunTimer timer;            // +0x588
    u16 flags;                 // +0x594
    i16 color;                 // +0x596
    u8 state;                  // +0x598
    u8 hideCapDuringStartup;   // +0x599
    u8 unknown59A[2];
};
C_ASSERT(sizeof(Laser) == 0x59c);
C_ASSERT(offsetof(Laser, vm1) == 0x2a4);
C_ASSERT(offsetof(Laser, position) == 0x548);
C_ASSERT(offsetof(Laser, angle) == 0x554);
C_ASSERT(offsetof(Laser, startOffset) == 0x558);
C_ASSERT(offsetof(Laser, endOffset) == 0x55c);
C_ASSERT(offsetof(Laser, startLength) == 0x560);
C_ASSERT(offsetof(Laser, width) == 0x564);
C_ASSERT(offsetof(Laser, currentWidth) == 0x568);
C_ASSERT(offsetof(Laser, speed) == 0x56c);
C_ASSERT(offsetof(Laser, startTime) == 0x570);
C_ASSERT(offsetof(Laser, hitboxStartTime) == 0x574);
C_ASSERT(offsetof(Laser, duration) == 0x578);
C_ASSERT(offsetof(Laser, despawnDuration) == 0x57c);
C_ASSERT(offsetof(Laser, hitboxEndDelay) == 0x580);
C_ASSERT(offsetof(Laser, inUse) == 0x584);
C_ASSERT(offsetof(Laser, timer) == 0x588);
C_ASSERT(offsetof(Laser, flags) == 0x594);
C_ASSERT(offsetof(Laser, color) == 0x596);
C_ASSERT(offsetof(Laser, state) == 0x598);
C_ASSERT(offsetof(Laser, hideCapDuringStartup) == 0x599);

enum BulletState
{
    BULLET_STATE_UNUSED = 0,
    BULLET_STATE_FIRED = 1,
    BULLET_STATE_SPAWNING_FAST = 2,
    BULLET_STATE_SPAWNING_NORMAL = 3,
    BULLET_STATE_SPAWNING_SLOW = 4,
    BULLET_STATE_DESPAWNING = 5,
    BULLET_STATE_SENTINEL = 6,
};

struct Bullet
{
    Bullet();
    void FUN_00432170();
    void FUN_0042ffc0();
    void FUN_00432390();
    void FUN_004325a0();
    void FUN_004326e0();
    void FUN_00432830();
    void FUN_00432460();
    void FUN_00432210();
    void FUN_004322b0();
    void FUN_00432aa0();
    void FUN_004329f0();
    ZunResult DrawSingleBullet();

    BulletTypeSprites sprites;
    Float3 position;
    Float3 velocity;
    Float3 unknownVectorD5C;
    f32 speed;
    unknown_fields(0xd6c, 8);
    f32 angle;
    unknown_fields(0xd78, 8);
    ZunTimer stateTimer;
    ZunTimer activeTimer;
    unknown_fields(0xd98, 0x10);
    i32 offscreenCullDelayFrames;
    u32 activeTransformFlags;
    u32 transformFlags;
    i16 color;
    unknown_fields(0xdb6, 2);
    u16 state;
    u16 offscreenFrames;
    unknown_fields(0xdbc, 1);
    u8 isGrazed;
    u8 cancelledDuringSpawn;
    unknown_fields(0xdbf, 1);
    Bullet *nextInDrawBucket;
    i32 zoneTransitionCooldownFrames;
    i32 transformSound;
    i32 transformIndex;
    BulletTransformRecord transforms[18];
    BulletExState exStates[7];
    i8 collisionDisabled;
    unknown_fields(0x10b5, 3);
};
C_ASSERT(sizeof(Bullet) == 0x10b8);
C_ASSERT(offsetof(Bullet, position) == 0xd44);
C_ASSERT(offsetof(Bullet, velocity) == 0xd50);
C_ASSERT(offsetof(Bullet, unknownVectorD5C) == 0xd5c);
C_ASSERT(offsetof(Bullet, speed) == 0xd68);
C_ASSERT(offsetof(Bullet, angle) == 0xd74);
C_ASSERT(offsetof(Bullet, stateTimer) == 0xd80);
C_ASSERT(offsetof(Bullet, activeTimer) == 0xd8c);
C_ASSERT(offsetof(Bullet, offscreenCullDelayFrames) == 0xda8);
C_ASSERT(offsetof(Bullet, activeTransformFlags) == 0xdac);
C_ASSERT(offsetof(Bullet, transformFlags) == 0xdb0);
C_ASSERT(offsetof(Bullet, color) == 0xdb4);
C_ASSERT(offsetof(Bullet, state) == 0xdb8);
C_ASSERT(offsetof(Bullet, offscreenFrames) == 0xdba);
C_ASSERT(offsetof(Bullet, isGrazed) == 0xdbd);
C_ASSERT(offsetof(Bullet, cancelledDuringSpawn) == 0xdbe);
C_ASSERT(offsetof(Bullet, nextInDrawBucket) == 0xdc0);
C_ASSERT(offsetof(Bullet, zoneTransitionCooldownFrames) == 0xdc4);
C_ASSERT(offsetof(Bullet, transformSound) == 0xdc8);
C_ASSERT(offsetof(Bullet, transformIndex) == 0xdcc);
C_ASSERT(offsetof(Bullet, transforms) == 0xdd0);
C_ASSERT(offsetof(Bullet, exStates) == 0xf80);
C_ASSERT(offsetof(Bullet, collisionDisabled) == 0x10b4);

struct BulletManager
{
    BulletTypeSprites bulletTypeSprites[0x20];
    Bullet bullets[0x601];
    Laser lasers[0x100];
    i32 activeBulletCount;
    i32 spawnSuppressionFrames;
    ZunTimer timer;
    i32 frameCounter;
    char *bulletAnmPath;
    Bullet *drawBuckets[6];
    Bullet *bulletCursor;
    i32 cancelItemType;
    AnmLoaded *bulletAnm;

    BulletManager();

    void Initialize();
    void bulletmanager_fun_00415c60();
    void RemoveAllBullets(i32 mode);
    i32 DespawnBullets(i32 maxScore, i32 awardLaserItems);
    void FUN_004321b0();
    i32 FUN_0042f5f0(BulletSpawnDescriptor *descriptor, i32 index1, i32 index2, f32 angleToPlayer);
    void RemoveBulletsInRadius(const Float3 *position, f32 radius);
    i32 FUN_00430e10(BulletSpawnDescriptor *descriptor);
    Laser *SpawnLaserPattern(BulletSpawnDescriptor *descriptor);

    static ZunResult RegisterChain(char *bulletAnmPath);
    static ChainCallbackResult OnUpdate(BulletManager *bulletManager);
    static ChainCallbackResult OnDraw(BulletManager *bulletManager);
    static ZunResult AddedCallback(BulletManager *bulletManager);
    static ZunResult DeletedCallback(BulletManager *bulletManager);
    static void CutChain();
};
C_ASSERT(sizeof(BulletManager) == 0x6ba578);
C_ASSERT(offsetof(BulletManager, bullets) == 0x1a880);
C_ASSERT(offsetof(BulletManager, lasers) == 0x660938);
C_ASSERT(offsetof(BulletManager, activeBulletCount) == 0x6ba538);
C_ASSERT(offsetof(BulletManager, spawnSuppressionFrames) == 0x6ba53c);
C_ASSERT(offsetof(BulletManager, timer) == 0x6ba540);
C_ASSERT(offsetof(BulletManager, frameCounter) == 0x6ba54c);
C_ASSERT(offsetof(BulletManager, drawBuckets) == 0x6ba554);
C_ASSERT(offsetof(BulletManager, bulletCursor) == 0x6ba56c);
C_ASSERT(offsetof(BulletManager, cancelItemType) == 0x6ba570);
C_ASSERT(offsetof(BulletManager, bulletAnm) == 0x6ba574);

DIFFABLE_EXTERN(BulletManager, g_BulletManager);

} /* namespace th08 */
