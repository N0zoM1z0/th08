#pragma once

#include "Global.hpp"
#include "Supervisor.hpp"

namespace th08
{

struct PlayerRawShtFile
{
    unknown_fields(0x0, 0x1c);
    f32 pointItemValueLine;
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
    PLAYER_STATE_DYING = 2,
    PLAYER_STATE_DEAD = 3,
};

struct PlayerStateEffect
{
    unknown_fields(0x0, 0x2a4);
    Float3 position;
    unknown_fields(0x2b0, 0xa0);
    i8 active;
};

struct Player
{
    i8 playerState;
    u8 playerType;
    unknown_fields(0x2, 0x2);
    i8 stateFlag;
    u8 isYoukai;
    unknown_fields(0x6, 0x1fa);
    i32 stateColor;
    unknown_fields(0x204, 0xb0);
    Float3 position;
    unknown_fields(0x2c0, 0xd1c);

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
    unknown_fields(0xbe834, 0x2423c);
    i32 playerStateSlotCooldown;
    PlayerRawShtFile *primaryShtFile;
    PlayerRawShtFile *secondaryShtFile;
    unknown_fields(0xe2a7c, 0x78);
    ZunTimer timer;
    unknown_fields(0xe2b00, 0x10);
    ChainElem *calcChain;
    ChainElem *drawChainHighPrio;
    ChainElem *drawChainLowPrio;
    PlayerStateEffect *stateEffect;
    unknown_fields(0xe2b20, 0x10);

    static ZunResult RegisterChain(u32 playerType);
    static ChainCallbackResult OnUpdate(Player *player);
    static ChainCallbackResult OnDrawHighPrio(Player *player);
    static ChainCallbackResult OnDrawLowPrio(Player *player);
    static ZunResult AddedCallback(Player *player);
    static ZunResult DeletedCallback(Player *player);
    static void CutChain();

    PlayerUnkStruct0x40 *FUN_0044de60(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerUnkStruct0x40 *FUN_0044df00(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerUnkStruct0x40 *FUN_0044dfa0(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerUnkStruct0x40 *FUN_0044e040(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    void FUN_0044c5b0();
    void FUN_0044d2c0();

    void FUN_0044c650();
    i32 FUN_0044cbf0();
    void FUN_0044d180();
    void FUN_0044aec0();
    void FUN_0044d420();
    i32 __fastcall FUN_0044fd80(u8 *slot, i32 value, u8 *entry);
    void __fastcall FUN_0044fb70(u8 *slot, u8 *entry);
    i32 __fastcall FUN_0044fdd0(u8 *slot, i32 value, u8 *entry);
    i32 __fastcall FUN_0044fe20(u8 *slot, i32 value, u8 *entry);
    i32 __fastcall FUN_0044ffa0(u8 *slot, i32 value, u8 *entry);
    void FUN_00451150();
    void FUN_004512f0();
    void FUN_00451400();
    i32 FUN_00451500();
    void __fastcall FUN_00450f60(i32 value);
    i32 FUN_00451d50();

    static ZunResult LoadShtFile(PlayerRawShtFile **header, const char *path);
    i32 IsHuman();
    i32 IsYoukai();
    f32 FUN_0044c1b0(Float3 *position);
    i32 FUN_00449ff0(Float3 *position, Float3 *position2);
};
C_ASSERT(sizeof(Player) == 0xe2b30);

DIFFABLE_EXTERN(Player, g_Player);
DIFFABLE_EXTERN(PlayerRawShtFile *, g_PlayerPrimaryShtFile);

} /* namespace th08 */
