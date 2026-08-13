#pragma once

#include "Global.hpp"

namespace th08
{

struct PlayerRawShtFile
{
};

struct PlayerUnkStruct0x40
{
    void Deactivate();
    void Reset();

    Float2 center;
    f32 radius;
    f32 radiusGrowth;
    Float2 size;
    Float2 sizeGrowth;
    f32 angle;
    i32 lifetime;
    i32 collisionValue;
    i32 damage;
    i32 hitAccumulator;
    i32 hitCap;
    i32 collisionInterval;
    u8 active;
    u8 mode;
    unknown_fields(0x3e, 2);
};
C_ASSERT(sizeof(PlayerUnkStruct0x40) == 0x40);

enum PlayerState
{
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_SPAWNING,
    PLAYER_STATE_DEAD,
};

struct Player
{
    i8 playerState;
    unknown_fields(0x1, 0xfdb);

    // Observed as g_Player + 0xFDC and through Player receivers at +0xFDC.
    // The target uses it to pause/alter gameplay updates.
    i32 frameStop;

    static ZunResult RegisterChain(u32 param);
    static ChainCallbackResult OnUpdate(Player *player);
    static ChainCallbackResult OnDrawHighPrio(Player *player);
    static ChainCallbackResult OnDrawLowPrio(Player *player);
    static ZunResult AddedCallback(Player *player);
    static ZunResult DeletedCallback(Player *player);
    static void CutChain();

    static ZunResult LoadShtFile(PlayerRawShtFile **header, const char *path);
    i32 IsYoukai();
};

DIFFABLE_EXTERN(Player, g_Player);

} /* namespace th08 */
