#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"

namespace th08
{

i32 IsBulletManagerAnmReleaseRequired();
DIFFABLE_EXTERN(i32, g_BulletManagerAnmReleaseRequired);

struct BulletTypeSprites
{
};

struct Laser
{
};

struct Bullet
{
};

struct BulletManager
{
    unknown_fields(0x0, 0x6BA570);
    i32 unk6ba570;
    AnmLoaded *bulletAnm;

    void Initialize();
    void bulletmanager_fun_00415c60();
    void RemoveAllBullets(i32 mode);

    static ZunResult RegisterChain();
    static ChainCallbackResult OnUpdate(BulletManager *bulletManager);
    static ChainCallbackResult OnDraw(BulletManager *bulletManager);
    static ZunResult AddedCallback(BulletManager *bulletManager);
    static ZunResult DeletedCallback(BulletManager *bulletManager);
    static void CutChain();
};

DIFFABLE_EXTERN(BulletManager, g_BulletManager);

} /* namespace th08 */
