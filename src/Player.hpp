#pragma once

#include "Global.hpp"
#include "Supervisor.hpp"

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
    u8 playerType;
    unknown_fields(0x2, 0xfda);

    // Observed as g_Player + 0xFDC and through Player receivers at +0xFDC.
    // The target uses it to pause/alter gameplay updates.
    i32 frameStop;
    unknown_fields(0xfe0, 0xb7854);
    // Target slot allocators at 0x44DFA0 and 0x44E040 address this array as
    // Player + 0xB8834. Each element's active byte is at +0x3C.
    PlayerUnkStruct0x40 playerSlotsB[192];
    // Target slot allocators at 0x44DE60 and 0x44DF00 address this array as
    // Player + 0xBB834. Each element's active byte is at +0x3C.
    PlayerUnkStruct0x40 playerSlotsC[192];
    unknown_fields(0xbe834, 0x24240);
    PlayerRawShtFile *primaryShtFile;
    PlayerRawShtFile *secondaryShtFile;
    unknown_fields(0xe2a7c, 0x78);
    ZunTimer timer;
    unknown_fields(0xe2b00, 0x10);
    ChainElem *calcChain;
    ChainElem *drawChainHighPrio;
    ChainElem *drawChainLowPrio;
    unknown_fields(0xe2b1c, 0x14);

    static ZunResult RegisterChain(u32 playerType);
    static ChainCallbackResult OnUpdate(Player *player);
    static ChainCallbackResult OnDrawHighPrio(Player *player);
    static ChainCallbackResult OnDrawLowPrio(Player *player);
    static ZunResult AddedCallback(Player *player);
    static ZunResult DeletedCallback(Player *player);
    static void CutChain();

    PlayerUnkStruct0x40 *FUN_0044de60(const Float2 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerUnkStruct0x40 *FUN_0044df00(const Float2 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerUnkStruct0x40 *FUN_0044dfa0(const Float2 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerUnkStruct0x40 *FUN_0044e040(const Float2 *center, f32 value1, f32 value2, i32 value3, i32 value4);

    static ZunResult LoadShtFile(PlayerRawShtFile **header, const char *path);
    i32 IsYoukai();
};
C_ASSERT(sizeof(Player) == 0xe2b30);

DIFFABLE_EXTERN(Player, g_Player);

} /* namespace th08 */
