#pragma once

#include "Global.hpp"
#include "Supervisor.hpp"
#include "AnmManager.hpp"
#include "EnemyManager.hpp"

namespace th08
{

struct AnmLoaded;

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

struct PlayerOptionState
{
    AnmVm vm;
    Float3 position;
    Float3 target;
    Float3 velocity;
    i32 state2C8;
    i32 substate2CC;
    i32 optionIndex;
    unknown_fields(0x2D4, 4);
    f32 orbitAngle;
    f32 facingAngle;
    ZunTimer timer;
    void *updateCallback;
    void *renderCallback;

    PlayerOptionState();
};
C_ASSERT(sizeof(PlayerOptionState) == 0x2F4);

struct PlayerBombWorkItem
{
    i32 active;
    unknown_fields(0x0004, 4);
    f32 rotationStep;
    f32 speed;
    f32 rotation;
    Float3 anchor;
    Float3 points[32];
    Float3 position;
    Float3 velocity;
    AnmVm vms[8];
    AnmVm *effect;
    ZunTimer timer;
    PlayerUnkStruct0x40 *damageSlot;
    PlayerUnkStruct0x40 *cancelSlot;

    PlayerBombWorkItem();
};
C_ASSERT(sizeof(PlayerBombWorkItem) == 0x16F0);

struct PlayerBombState
{
    i32 frameStop;
    i32 unknown4;
    i32 duration;
    unknown_fields(0x00000C, 0x8);
    i32 secondaryWorkIndex;
    ZunTimer timer;
    unknown_fields(0x000024, 0x28);
    PlayerBombWorkItem workItems[128];
    Float3 tailPosition;

    PlayerBombState();
};
C_ASSERT(sizeof(PlayerBombState) == 0xB7858);

struct Player;
struct PlayerShot;
typedef i32 (__fastcall *PlayerShotCollisionCallback)(Player *player, PlayerShot *shot, Float3 *enemyPosition);

struct PlayerShotVelocity
{
    f32 x;
    f32 y;
    f32 z;
};

struct PlayerShot
{
    AnmVm vm;
    Float3 position;
    Float3 vectors[32];
    Float3 hitboxSize;
    PlayerShotVelocity velocity;
    unknown_fields(0x448, 0xC);
    ZunTimer timer;
    i16 damage;
    i16 state;
    i16 type;
    unknown_fields(0x466, 8);
    i16 animationIndex;
    unknown_fields(0x470, 0xC);
    PlayerShotCollisionCallback collisionCallback;
    void *shtEntry;

    PlayerShot();
};
C_ASSERT(sizeof(PlayerShot) == 0x484);

struct Player
{
    Player();

    i8 playerState;
    u8 playerType;
    unknown_fields(0x2, 1);
    u8 optionModeFlag;
    i8 stateFlag;
    u8 isYoukai;
    unknown_fields(0x6, 0x6);
    AnmLoaded *anmFile;
    AnmVm mainVm;
    Float3 position;
    Float3 position2;
    Float3 vectors2CC[16];
    Float3 vector38C;
    Float3 vector398;
    Float3 vector3A4;
    Float3 vector3B0;
    Float3 vector3BC;
    Float3 vector3C8;
    Float3 vector3D4;
    Float3 vector3E0;
    Float3 vector3EC;
    Float3 vector3F8;
    unknown_fields(0x404, 8);
    PlayerOptionState optionStates[4];
    PlayerBombState bombState;
    PlayerUnkStruct0x40 playerSlotsB[192];
    PlayerUnkStruct0x40 playerSlotsC[192];
    unknown_fields(0xBE834, 4);
    PlayerShot shots[128];
    EclTimeline timelines[3];
    unknown_fields(0xE2A68, 8);
    i32 playerStateSlotCooldown;
    PlayerRawShtFile *primaryShtFile;
    PlayerRawShtFile *secondaryShtFile;
    unknown_fields(0xE2A7C, 0x1C);
    i32 movementDirection;
    unknown_fields(0xE2A9C, 8);
    Float3 tailPosition0;
    Float3 tailPosition1;
    Enemy *optionHomingTarget;
    unknown_fields(0xE2AC0, 4);
    ZunTimer timerE2AC4;
    ZunTimer timerE2AD0;
    ZunTimer timerE2ADC;
    ZunTimer timerE2AE8;
    ZunTimer timer;
    ZunTimer timerE2B00;
    unknown_fields(0xE2B0C, 4);
    ChainElem *calcChain;
    ChainElem *drawChainHighPrio;
    ChainElem *drawChainLowPrio;
    PlayerStateEffect *stateEffect;
    unknown_fields(0xE2B20, 0xC);
    i32 damageAccumulatorThreshold;

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
    void FUN_0040bf00();
    void FUN_0044c5b0();
    void FUN_0044d2c0();

    void FUN_0044c650();
    i32 FUN_0044cbf0();
    void FUN_0044d180();
    i32 FUN_0044aec0();
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
    void FUN_00451640();
    i32 FUN_00451670(Float3 *enemyPosition, Float3 *enemySize, i32 *hitAccumulator, i32 *bombHit);
    void __fastcall FUN_00450f60(i32 value);
    i32 FUN_00451d50();

    static ZunResult LoadShtFile(PlayerRawShtFile **header, const char *path);
    i32 IsHuman();
    i32 IsYoukai();
    f32 FUN_0044c1b0(Float3 *position);
    i32 FUN_0044a230(Float3 *position, Float3 *size);
    i32 FUN_0044a360(Float3 *position, Float3 *size);
    i32 FUN_0044a470(Float3 *position, Float3 *size);
    u32 CalcItemBoxCollision(Float3 *position, Float3 *size);
    u32 CalcLaserHitbox(Float3 *position, Float3 *size, Float3 *origin, f32 angle, i32 graze);
    void FUN_0044a930(Float3 *position, i32 suppressExtraItems);
    void Die();
    i32 FUN_00449ff0(Float3 *position, Float3 *position2);
};
C_ASSERT(sizeof(Player) == 0xe2b30);
C_ASSERT(offsetof(Player, optionModeFlag) == 0x3);
C_ASSERT(offsetof(Player, mainVm) == 0x10);
C_ASSERT(offsetof(Player, position) == 0x2B4);
C_ASSERT(offsetof(Player, optionStates) == 0x40C);
C_ASSERT(offsetof(PlayerOptionState, facingAngle) == 0x2DC);
C_ASSERT(offsetof(Player, bombState) == 0xFDC);
C_ASSERT(offsetof(Player, playerSlotsB) == 0xB8834);
C_ASSERT(offsetof(Player, playerSlotsC) == 0xBB834);
C_ASSERT(offsetof(Player, shots) == 0xBE838);
C_ASSERT(offsetof(Player, timelines) == 0xE2A38);
C_ASSERT(offsetof(Player, playerStateSlotCooldown) == 0xE2A70);
C_ASSERT(offsetof(Player, movementDirection) == 0xE2A98);
C_ASSERT(offsetof(Player, timer) == 0xE2AF4);
C_ASSERT(offsetof(Player, damageAccumulatorThreshold) == 0xE2B2C);

DIFFABLE_EXTERN(Player, g_Player);
DIFFABLE_EXTERN(PlayerRawShtFile *, g_PlayerPrimaryShtFile);
DIFFABLE_EXTERN(PlayerRawShtFile *, g_PlayerSecondaryShtFile);

} /* namespace th08 */
