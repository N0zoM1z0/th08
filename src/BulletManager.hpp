#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"

namespace th08
{

struct BulletSpawnDescriptor;

i32 IsBulletManagerAnmReleaseRequired();
DIFFABLE_EXTERN(i32, g_BulletManagerAnmReleaseRequired);

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

struct Laser
{
    Laser();

    AnmVm vm0;
    AnmVm vm1;
    Float3 position;
    unknown_fields(0x554, 0x34);
    ZunTimer timer;
    unknown_fields(0x594, 8);
};
C_ASSERT(sizeof(Laser) == 0x59c);

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
    i32 FUN_00430e10(BulletSpawnDescriptor *descriptor);

    static ZunResult RegisterChain(char *bulletAnmPath);
    static ChainCallbackResult OnUpdate(BulletManager *bulletManager);
    static ChainCallbackResult OnDraw(BulletManager *bulletManager);
    static ZunResult AddedCallback(BulletManager *bulletManager);
    static ZunResult DeletedCallback(BulletManager *bulletManager);
    static void CutChain();
};

DIFFABLE_EXTERN(BulletManager, g_BulletManager);

} /* namespace th08 */
