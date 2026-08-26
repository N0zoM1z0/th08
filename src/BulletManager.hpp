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

    AnmVm sprite0;
    AnmVm sprite1;
    AnmVm sprite2;
    AnmVm sprite3;
    AnmVm sprite4;
    Float3 position;
    unknown_fields(0xd40, 4);
};
C_ASSERT(sizeof(BulletTypeSprites) == 0xd44);

struct BulletExState
{
    BulletExState();

    ZunTimer timer;
    unknown_fields(0xc, 8);
    Float3 position;
    unknown_fields(0x20, 0xc);
};
C_ASSERT(sizeof(BulletExState) == 0x2c);

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
    Float3 position0;
    Float3 position1;
    Float3 position2;
    unknown_fields(0xd68, 0x18);
    ZunTimer timer0;
    ZunTimer timer1;
    unknown_fields(0xd98, 0x1e8);
    BulletExState exStates[7];
    unknown_fields(0x10b4, 4);
};
C_ASSERT(sizeof(Bullet) == 0x10b8);

struct BulletManager
{
    BulletTypeSprites bulletTypeSprites[0x20];
    Bullet bullets[0x601];
    Laser lasers[0x100];
    unknown_fields(0x6ba538, 8);
    ZunTimer timer;
    unknown_fields(0x6ba54c, 4);
    char *bulletAnmPath;
    unknown_fields(0x6ba554, 0x18);
    u8 *bulletCursor;
    i32 unk6ba570;
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

DIFFABLE_EXTERN(BulletManager, g_BulletManager);

} /* namespace th08 */
