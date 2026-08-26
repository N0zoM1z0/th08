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
    unknown_fields(0x0, 0x4);
    f32 initialBombCount;
    i32 deathbombWindowFrames;
    f32 hurtboxSize;
    f32 grazeBoxSize;
    f32 itemAutoCollectSpeed;
    f32 itemCollectionBoxSize;
    f32 pointItemValueLine;
    unknown_fields(0x20, 0x4);
    f32 normalAxisSpeed;
    f32 focusedAxisSpeed;
    f32 normalDiagonalSpeed;
    f32 focusedDiagonalSpeed;
};
C_ASSERT(offsetof(PlayerRawShtFile, initialBombCount) == 0x4);
C_ASSERT(offsetof(PlayerRawShtFile, deathbombWindowFrames) == 0x8);
C_ASSERT(offsetof(PlayerRawShtFile, hurtboxSize) == 0xC);
C_ASSERT(offsetof(PlayerRawShtFile, grazeBoxSize) == 0x10);
C_ASSERT(offsetof(PlayerRawShtFile, itemAutoCollectSpeed) == 0x14);
C_ASSERT(offsetof(PlayerRawShtFile, itemCollectionBoxSize) == 0x18);
C_ASSERT(offsetof(PlayerRawShtFile, pointItemValueLine) == 0x1C);
C_ASSERT(offsetof(PlayerRawShtFile, normalAxisSpeed) == 0x24);
C_ASSERT(offsetof(PlayerRawShtFile, focusedAxisSpeed) == 0x28);
C_ASSERT(offsetof(PlayerRawShtFile, normalDiagonalSpeed) == 0x2C);
C_ASSERT(offsetof(PlayerRawShtFile, focusedDiagonalSpeed) == 0x30);
C_ASSERT(sizeof(PlayerRawShtFile) == 0x34);

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
    unknown_fields(0x0, 0x1F8);
    u32 vmFlags;
    unknown_fields(0x1FC, 0xA8);
    Float3 position;
    unknown_fields(0x2b0, 0xa0);
    i8 active;
};
C_ASSERT(offsetof(PlayerStateEffect, vmFlags) == 0x1F8);
C_ASSERT(offsetof(PlayerStateEffect, position) == 0x2A4);
C_ASSERT(offsetof(PlayerStateEffect, active) == 0x350);

struct Player;
struct PlayerOptionState;
typedef i32 (__fastcall *PlayerOptionCallback)(Player *, PlayerOptionState *);

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
    PlayerOptionCallback updateCallback;
    PlayerOptionCallback renderCallback;

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

enum PlayerMovementDirection
{
    PLAYER_DIRECTION_NONE,
    PLAYER_DIRECTION_UP,
    PLAYER_DIRECTION_DOWN,
    PLAYER_DIRECTION_LEFT,
    PLAYER_DIRECTION_RIGHT,
    PLAYER_DIRECTION_UP_LEFT,
    PLAYER_DIRECTION_UP_RIGHT,
    PLAYER_DIRECTION_DOWN_LEFT,
    PLAYER_DIRECTION_DOWN_RIGHT,
};
C_ASSERT(sizeof(PlayerBombWorkItem) == 0x16F0);

typedef void (__fastcall *PlayerBombCallback)(Player *player);

struct PlayerBombCallbacks
{
    PlayerBombCallback callbacks[5];
};
C_ASSERT(sizeof(PlayerBombCallbacks) == 0x14);

struct PlayerBombState
{
    i32 isInUse;
    i32 callbackSetIndex;
    i32 duration;
    unknown_fields(0x00000C, 0x4);
    i32 bombsConsumed;
    i32 secondaryWorkIndex;
    ZunTimer timer;
    PlayerBombCallbacks calcCallbacks;
    PlayerBombCallbacks drawCallbacks;
    PlayerBombWorkItem workItems[128];
    Float3 tailPosition;

    PlayerBombState();
};
C_ASSERT(sizeof(PlayerBombState) == 0xB7858);
C_ASSERT(offsetof(PlayerBombState, isInUse) == 0x0);
C_ASSERT(offsetof(PlayerBombState, callbackSetIndex) == 0x4);
C_ASSERT(offsetof(PlayerBombState, duration) == 0x8);
C_ASSERT(offsetof(PlayerBombState, bombsConsumed) == 0x10);
C_ASSERT(offsetof(PlayerBombState, timer) == 0x18);
C_ASSERT(offsetof(PlayerBombState, calcCallbacks) == 0x24);
C_ASSERT(offsetof(PlayerBombState, drawCallbacks) == 0x38);
C_ASSERT(offsetof(PlayerBombState, workItems) == 0x4C);

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
    unknown_fields(0x6, 0x2);
    i32 focusTransitionFrames;
    AnmLoaded *anmFile;
    AnmVm mainVm;
    Float3 position;
    Float3 position2;
    Float3 positionHistory[16];
    Float3 hurtboxBoundsMin;
    Float3 hurtboxBoundsMax;
    Float3 grazeBoundsMin;
    Float3 grazeBoundsMax;
    Float3 itemCollectionBoundsMin;
    Float3 itemCollectionBoundsMax;
    Float3 hurtboxHalfSize;
    Float3 grazeHalfSize;
    Float3 itemCollectionHalfSize;
    Float3 velocity;
    f32 horizontalSpeedMultiplier;
    f32 verticalSpeedMultiplier;
    PlayerOptionState optionStates[4];
    PlayerBombState bombState;
    PlayerUnkStruct0x40 playerSlotsB[192];
    PlayerUnkStruct0x40 playerSlotsC[192];
    PlayerStateEffect *focusEffect;
    PlayerShot shots[128];
    EclTimeline timelines[3];
    i32 deathbombWindowFrames;
    i32 bombInputLockFrames;
    i32 playerStateSlotCooldown;
    PlayerRawShtFile *primaryShtFile;
    PlayerRawShtFile *secondaryShtFile;
    i32 itemTimeOrbMode;
    unknown_fields(0xE2A80, 0x10);
    i32 bulletCancelItemType;
    unknown_fields(0xE2A94, 0x4);
    PlayerMovementDirection movementDirection;
    f32 currentHorizontalSpeed;
    f32 currentVerticalSpeed;
    Float3 tailPosition0;
    Float3 tailPosition1;
    Enemy *optionHomingTarget;
    i32 enemyTrackedPositionValid;
    ZunTimer shotTimer;
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
    unknown_fields(0xE2B20, 0x4);
    PlayerStateEffect *extremeGaugeEffect;
    AnmVm *deathbombEffectVm;
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
    i32 UpdateMovementAndOptions();
    void FUN_0044d420();
    i32 __fastcall FUN_0044fd80(u8 *slot, i32 value, u8 *entry);
    void __fastcall FUN_0044fb70(u8 *slot, u8 *entry);
    i32 __fastcall FUN_0044fdd0(u8 *slot, i32 value, u8 *entry);
    i32 __fastcall FUN_0044fe20(u8 *slot, i32 value, u8 *entry);
    i32 __fastcall FUN_0044ffa0(u8 *slot, i32 value, u8 *entry);
    void FUN_00451150();
    void FUN_004512f0();
    void FUN_00451400();
    i32 UpdateShooting();
    void StartShooting();
    i32 FUN_00451670(Float3 *enemyPosition, Float3 *enemySize, i32 *hitAccumulator, i32 *bombHit);
    void __fastcall SpawnShots(i32 value);
    i32 IsBombShotSuppressed();

    static ZunResult LoadShtFile(PlayerRawShtFile **header, const char *path);
    i32 IsHuman();
    i32 IsYoukai();
    f32 AngleToPoint(Float3 *position);
    i32 CheckBulletCollision(Float3 *position, Float3 *size);
    i32 CheckLethalCollision(Float3 *position, Float3 *size);
    i32 CheckGrazeCollision(Float3 *position, Float3 *size);
    u32 CalcItemBoxCollision(Float3 *position, Float3 *size);
    u32 CalcLaserHitbox(Float3 *position, Float3 *size, Float3 *origin, f32 angle, i32 graze);
    void AwardGraze(Float3 *position, i32 suppressExtraItems);
    void Die();
    i32 CheckBulletCancelCollision(Float3 *position, Float3 *position2);
};
C_ASSERT(sizeof(Player) == 0xe2b30);
C_ASSERT(offsetof(Player, optionModeFlag) == 0x3);
C_ASSERT(offsetof(Player, focusTransitionFrames) == 0x8);
C_ASSERT(offsetof(Player, mainVm) == 0x10);
C_ASSERT(offsetof(Player, position) == 0x2B4);
C_ASSERT(offsetof(Player, positionHistory) == 0x2CC);
C_ASSERT(offsetof(Player, hurtboxBoundsMin) == 0x38C);
C_ASSERT(offsetof(Player, hurtboxBoundsMax) == 0x398);
C_ASSERT(offsetof(Player, grazeBoundsMin) == 0x3A4);
C_ASSERT(offsetof(Player, grazeBoundsMax) == 0x3B0);
C_ASSERT(offsetof(Player, itemCollectionBoundsMin) == 0x3BC);
C_ASSERT(offsetof(Player, itemCollectionBoundsMax) == 0x3C8);
C_ASSERT(offsetof(Player, hurtboxHalfSize) == 0x3D4);
C_ASSERT(offsetof(Player, grazeHalfSize) == 0x3E0);
C_ASSERT(offsetof(Player, itemCollectionHalfSize) == 0x3EC);
C_ASSERT(offsetof(Player, velocity) == 0x3F8);
C_ASSERT(offsetof(Player, horizontalSpeedMultiplier) == 0x404);
C_ASSERT(offsetof(Player, verticalSpeedMultiplier) == 0x408);
C_ASSERT(offsetof(Player, optionStates) == 0x40C);
C_ASSERT(offsetof(PlayerOptionState, facingAngle) == 0x2DC);
C_ASSERT(offsetof(Player, bombState) == 0xFDC);
C_ASSERT(offsetof(Player, playerSlotsB) == 0xB8834);
C_ASSERT(offsetof(Player, playerSlotsC) == 0xBB834);
C_ASSERT(offsetof(Player, focusEffect) == 0xBE834);
C_ASSERT(offsetof(Player, shots) == 0xBE838);
C_ASSERT(offsetof(Player, timelines) == 0xE2A38);
C_ASSERT(offsetof(Player, deathbombWindowFrames) == 0xE2A68);
C_ASSERT(offsetof(Player, bombInputLockFrames) == 0xE2A6C);
C_ASSERT(offsetof(Player, playerStateSlotCooldown) == 0xE2A70);
C_ASSERT(offsetof(Player, primaryShtFile) == 0xE2A74);
C_ASSERT(offsetof(Player, secondaryShtFile) == 0xE2A78);
C_ASSERT(offsetof(Player, itemTimeOrbMode) == 0xE2A7C);
C_ASSERT(offsetof(Player, bulletCancelItemType) == 0xE2A90);
C_ASSERT(offsetof(Player, movementDirection) == 0xE2A98);
C_ASSERT(offsetof(Player, currentHorizontalSpeed) == 0xE2A9C);
C_ASSERT(offsetof(Player, currentVerticalSpeed) == 0xE2AA0);
C_ASSERT(offsetof(Player, tailPosition0) == 0xE2AA4);
C_ASSERT(offsetof(Player, optionHomingTarget) == 0xE2ABC);
C_ASSERT(offsetof(Player, enemyTrackedPositionValid) == 0xE2AC0);
C_ASSERT(offsetof(Player, shotTimer) == 0xE2AC4);
C_ASSERT(offsetof(Player, timer) == 0xE2AF4);
C_ASSERT(offsetof(Player, calcChain) == 0xE2B10);
C_ASSERT(offsetof(Player, extremeGaugeEffect) == 0xE2B24);
C_ASSERT(offsetof(Player, deathbombEffectVm) == 0xE2B28);
C_ASSERT(offsetof(Player, damageAccumulatorThreshold) == 0xE2B2C);

DIFFABLE_EXTERN(Player, g_Player);

} /* namespace th08 */
