#pragma once

#include "Global.hpp"
#include "Supervisor.hpp"
#include "AnmManager.hpp"
#include "EnemyManager.hpp"

namespace th08
{

struct AnmLoaded;
struct PlayerShotDescriptor;

struct PlayerShotPowerLevel
{
    PlayerShotDescriptor *descriptors;
    i32 minimumPower;
};
C_ASSERT(sizeof(PlayerShotPowerLevel) == 0x8);

struct PlayerRawShtFile
{
    u16 unknownHeaderWord;
    u16 shotPowerLevelCount;
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
    f32 itemMovementSpeed;
    PlayerShotPowerLevel shotPowerLevels[1];
};
C_ASSERT(offsetof(PlayerRawShtFile, shotPowerLevelCount) == 0x2);
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
C_ASSERT(offsetof(PlayerRawShtFile, itemMovementSpeed) == 0x34);
C_ASSERT(offsetof(PlayerRawShtFile, shotPowerLevels) == 0x38);

struct PlayerCollisionRegion
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
C_ASSERT(sizeof(PlayerCollisionRegion) == 0x40);

enum PlayerState
{
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_SPAWNING,
    PLAYER_STATE_DYING = 2,
    PLAYER_STATE_INVULNERABLE = 3,
};

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

enum PlayerBombWorkItemState
{
    PLAYER_BOMB_WORK_ITEM_INACTIVE,
    PLAYER_BOMB_WORK_ITEM_ACTIVE,
    PLAYER_BOMB_WORK_ITEM_FINISHING,
};

struct PlayerBombWorkItem
{
    i32 state;
    i32 stateTimer;
    f32 motionStep;
    f32 speed;
    f32 angle;
    Float3 position;
    Float3 pathPoints[32];
    Float3 motion;
    Float3 auxiliaryMotion;
    AnmVm vms[8];
    AnmVm *effectVm;
    ZunTimer timer;
    PlayerCollisionRegion *damageRegion;
    PlayerCollisionRegion *cancelRegion;

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
C_ASSERT(offsetof(PlayerBombWorkItem, stateTimer) == 0x4);
C_ASSERT(offsetof(PlayerBombWorkItem, motionStep) == 0x8);
C_ASSERT(offsetof(PlayerBombWorkItem, speed) == 0xC);
C_ASSERT(offsetof(PlayerBombWorkItem, angle) == 0x10);
C_ASSERT(offsetof(PlayerBombWorkItem, position) == 0x14);
C_ASSERT(offsetof(PlayerBombWorkItem, pathPoints) == 0x20);
C_ASSERT(offsetof(PlayerBombWorkItem, motion) == 0x1A0);
C_ASSERT(offsetof(PlayerBombWorkItem, auxiliaryMotion) == 0x1AC);
C_ASSERT(offsetof(PlayerBombWorkItem, vms) == 0x1B8);
C_ASSERT(offsetof(PlayerBombWorkItem, effectVm) == 0x16D8);
C_ASSERT(offsetof(PlayerBombWorkItem, timer) == 0x16DC);
C_ASSERT(offsetof(PlayerBombWorkItem, damageRegion) == 0x16E8);
C_ASSERT(offsetof(PlayerBombWorkItem, cancelRegion) == 0x16EC);

typedef void (__fastcall *PlayerBombCallback)(Player *player);

struct PlayerBombCallbackSet
{
    PlayerBombCallback callbacks[5];
};
C_ASSERT(sizeof(PlayerBombCallbackSet) == 0x14);

enum PlayerBombCallbackVariant
{
    PLAYER_BOMB_CALLBACK_PRIMARY,
    PLAYER_BOMB_CALLBACK_SECONDARY,
    PLAYER_BOMB_CALLBACK_PRIMARY_DEATHBOMB,
    PLAYER_BOMB_CALLBACK_SECONDARY_DEATHBOMB,
    PLAYER_BOMB_CALLBACK_SPECIAL,
};

struct PlayerBombState
{
    i32 isInUse;
    i32 callbackVariant;
    i32 duration;
    unknown_fields(0x00000C, 0x4);
    i32 bombsConsumed;
    i32 secondaryWorkCursor;
    ZunTimer timer;
    PlayerBombCallbackSet updateCallbacks;
    PlayerBombCallbackSet drawCallbacks;
    PlayerBombWorkItem workItems[128];
    Float3 tailPosition;

    PlayerBombState();
};
C_ASSERT(sizeof(PlayerBombState) == 0xB7858);
C_ASSERT(offsetof(PlayerBombState, isInUse) == 0x0);
C_ASSERT(offsetof(PlayerBombState, callbackVariant) == 0x4);
C_ASSERT(offsetof(PlayerBombState, duration) == 0x8);
C_ASSERT(offsetof(PlayerBombState, bombsConsumed) == 0x10);
C_ASSERT(offsetof(PlayerBombState, timer) == 0x18);
C_ASSERT(offsetof(PlayerBombState, updateCallbacks) == 0x24);
C_ASSERT(offsetof(PlayerBombState, drawCallbacks) == 0x38);
C_ASSERT(offsetof(PlayerBombState, workItems) == 0x4C);

struct PlayerShot;
typedef i32 (__fastcall *PlayerShotSpawnCallback)(Player *player, PlayerShot *shot,
                                                  i32 shotTimer,
                                                  PlayerShotDescriptor *descriptor);
typedef i32 (__fastcall *PlayerShotUpdateCallback)(Player *player, PlayerShot *shot);
typedef i32 (__fastcall *PlayerShotDrawCallback)(Player *player, PlayerShot *shot);
typedef i32 (__fastcall *PlayerShotCollisionCallback)(Player *player, PlayerShot *shot, Float3 *enemyPosition);

struct PlayerShotDescriptor
{
    i16 fireInterval;
    i16 fireFrame;
    Float2 positionOffset;
    Float2 hitboxSize;
    f32 angle;
    f32 speed;
    i16 damage;
    i16 extremeGaugeBehavior;
    i16 sourceOptionIndex;
    i16 shotType;
    i16 animationIndex;
    i16 soundIndex;
    PlayerShotSpawnCallback spawnCallback;
    PlayerShotUpdateCallback updateCallback;
    PlayerShotDrawCallback drawCallback;
    PlayerShotCollisionCallback collisionCallback;
};
C_ASSERT(sizeof(PlayerShotDescriptor) == 0x38);
C_ASSERT(offsetof(PlayerShotDescriptor, positionOffset) == 0x4);
C_ASSERT(offsetof(PlayerShotDescriptor, hitboxSize) == 0xC);
C_ASSERT(offsetof(PlayerShotDescriptor, angle) == 0x14);
C_ASSERT(offsetof(PlayerShotDescriptor, speed) == 0x18);
C_ASSERT(offsetof(PlayerShotDescriptor, damage) == 0x1C);
C_ASSERT(offsetof(PlayerShotDescriptor, sourceOptionIndex) == 0x20);
C_ASSERT(offsetof(PlayerShotDescriptor, spawnCallback) == 0x28);
C_ASSERT(offsetof(PlayerShotDescriptor, updateCallback) == 0x2C);
C_ASSERT(offsetof(PlayerShotDescriptor, drawCallback) == 0x30);
C_ASSERT(offsetof(PlayerShotDescriptor, collisionCallback) == 0x34);

enum PlayerShotState
{
    PLAYER_SHOT_INACTIVE,
    PLAYER_SHOT_ACTIVE,
    PLAYER_SHOT_HIT,
};

struct PlayerShotVelocity
{
    f32 x;
    f32 y;
    f32 z;
};
C_ASSERT(sizeof(PlayerShotVelocity) == 0xC);

struct PlayerShot
{
    AnmVm vm;
    Float3 position;
    Float3 positionHistory[32];
    Float3 hitboxSize;
    PlayerShotVelocity velocity;
    f32 auxiliaryValue;
    f32 speed;
    f32 angle;
    ZunTimer timer;
    i16 damage;
    i16 state;
    i16 shotType;
    i16 timelineIndex;
    i16 sourceOptionIndex;
    i16 trailSegmentCount;
    u8 optionModeFlag;
    u8 unknown46D;
    i16 animationIndex;
    i8 tintInExtremeYoukai;
    u8 unknown471[3];
    PlayerShotUpdateCallback updateCallback;
    PlayerShotDrawCallback drawCallback;
    PlayerShotCollisionCallback collisionCallback;
    PlayerShotDescriptor *descriptor;

    PlayerShot();
};
C_ASSERT(sizeof(PlayerShot) == 0x484);
C_ASSERT(offsetof(PlayerShot, position) == 0x2A4);
C_ASSERT(offsetof(PlayerShot, positionHistory) == 0x2B0);
C_ASSERT(offsetof(PlayerShot, hitboxSize) == 0x430);
C_ASSERT(offsetof(PlayerShot, velocity) == 0x43C);
C_ASSERT(offsetof(PlayerShot, auxiliaryValue) == 0x448);
C_ASSERT(offsetof(PlayerShot, speed) == 0x44C);
C_ASSERT(offsetof(PlayerShot, angle) == 0x450);
C_ASSERT(offsetof(PlayerShot, timer) == 0x454);
C_ASSERT(offsetof(PlayerShot, damage) == 0x460);
C_ASSERT(offsetof(PlayerShot, state) == 0x462);
C_ASSERT(offsetof(PlayerShot, shotType) == 0x464);
C_ASSERT(offsetof(PlayerShot, timelineIndex) == 0x466);
C_ASSERT(offsetof(PlayerShot, sourceOptionIndex) == 0x468);
C_ASSERT(offsetof(PlayerShot, trailSegmentCount) == 0x46A);
C_ASSERT(offsetof(PlayerShot, optionModeFlag) == 0x46C);
C_ASSERT(offsetof(PlayerShot, animationIndex) == 0x46E);
C_ASSERT(offsetof(PlayerShot, tintInExtremeYoukai) == 0x470);
C_ASSERT(offsetof(PlayerShot, updateCallback) == 0x474);
C_ASSERT(offsetof(PlayerShot, drawCallback) == 0x478);
C_ASSERT(offsetof(PlayerShot, collisionCallback) == 0x47C);
C_ASSERT(offsetof(PlayerShot, descriptor) == 0x480);

struct Player
{
    Player();

    i8 playerState;
    u8 playerType;
    u8 unknown2;
    u8 optionModeFlag;
    u8 deathbombPending;
    u8 isYoukai;
    u8 forceDeathbombAtWindowEnd;
    u8 unknown7;
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
    PlayerCollisionRegion damageRegions[192];
    PlayerCollisionRegion cancelRegions[192];
    Effect *focusEffect;
    PlayerShot shots[128];
    EclTimeline timelines[3];
    i32 deathbombWindowFrames;
    i32 bombInputLockFrames;
    i32 playerStateSlotCooldown;
    PlayerRawShtFile *primaryShtFile;
    PlayerRawShtFile *secondaryShtFile;
    i32 itemTimeOrbMode;
    PlayerShotDescriptor *persistentShotDescriptors[4];
    i32 bulletCancelItemType;
    u8 shotHitEffectCounter;
    u8 unknownE2A95[3];
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
    f32 baseShotAngle;
    ChainElem *calcChain;
    ChainElem *drawChainHighPrio;
    ChainElem *drawChainLowPrio;
    Effect *stateEffect;
    unknown_fields(0xE2B20, 0x4);
    Effect *extremeGaugeEffect;
    AnmVm *deathbombEffectVm;
    i32 damageAccumulatorThreshold;

    static ZunResult RegisterChain(u32 playerType);
    static ChainCallbackResult OnUpdate(Player *player);
    static ChainCallbackResult OnDrawHighPrio(Player *player);
    static ChainCallbackResult OnDrawLowPrio(Player *player);
    static ZunResult AddedCallback(Player *player);
    static ZunResult DeletedCallback(Player *player);
    static void CutChain();

    PlayerCollisionRegion *CreateRectCancelRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerCollisionRegion *CreateCircleCancelRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerCollisionRegion *CreateRectDamageRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    PlayerCollisionRegion *CreateCircleDamageRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4);
    void SpawnBombStateEffect();
    void UpdateCollisionRegions();
    void UpdateInvulnerability();

    void UpdateBombState();
    i32 UpdateDeathAndRespawn();
    void UpdateRespawnAnimation();
    i32 UpdateMovementAndOptions();
    void UpdateGaugePosition();
    i32 __fastcall SpawnShotOnSchedule(PlayerShot *shot, i32 value,
                                       PlayerShotDescriptor *descriptor);
    void __fastcall InitializeShot(PlayerShot *shot, PlayerShotDescriptor *descriptor);
    i32 __fastcall SpawnShotOnScheduleUnlessBombing(PlayerShot *shot, i32 value,
                                                    PlayerShotDescriptor *descriptor);
    i32 __fastcall SpawnPersistentShot(PlayerShot *shot, i32 value,
                                       PlayerShotDescriptor *descriptor);
    i32 __fastcall SpawnShotAimedAtTrackedPoint(PlayerShot *shot, i32 value,
                                                PlayerShotDescriptor *descriptor);
    void UpdateShots();
    void DrawActiveShots();
    void DrawHitShots();
    i32 UpdateShooting();
    void StartShooting();
    i32 CalcDamageToEnemy(Float3 *enemyPosition, Float3 *enemySize, i32 *hitAccumulator, i32 *bombHit);
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
C_ASSERT(offsetof(Player, unknown2) == 0x2);
C_ASSERT(offsetof(Player, optionModeFlag) == 0x3);
C_ASSERT(offsetof(Player, deathbombPending) == 0x4);
C_ASSERT(offsetof(Player, forceDeathbombAtWindowEnd) == 0x6);
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
C_ASSERT(offsetof(Player, damageRegions) == 0xB8834);
C_ASSERT(offsetof(Player, cancelRegions) == 0xBB834);
C_ASSERT(offsetof(Player, focusEffect) == 0xBE834);
C_ASSERT(offsetof(Player, shots) == 0xBE838);
C_ASSERT(offsetof(Player, timelines) == 0xE2A38);
C_ASSERT(offsetof(Player, deathbombWindowFrames) == 0xE2A68);
C_ASSERT(offsetof(Player, bombInputLockFrames) == 0xE2A6C);
C_ASSERT(offsetof(Player, playerStateSlotCooldown) == 0xE2A70);
C_ASSERT(offsetof(Player, primaryShtFile) == 0xE2A74);
C_ASSERT(offsetof(Player, secondaryShtFile) == 0xE2A78);
C_ASSERT(offsetof(Player, itemTimeOrbMode) == 0xE2A7C);
C_ASSERT(offsetof(Player, persistentShotDescriptors) == 0xE2A80);
C_ASSERT(offsetof(Player, bulletCancelItemType) == 0xE2A90);
C_ASSERT(offsetof(Player, shotHitEffectCounter) == 0xE2A94);
C_ASSERT(offsetof(Player, movementDirection) == 0xE2A98);
C_ASSERT(offsetof(Player, currentHorizontalSpeed) == 0xE2A9C);
C_ASSERT(offsetof(Player, currentVerticalSpeed) == 0xE2AA0);
C_ASSERT(offsetof(Player, tailPosition0) == 0xE2AA4);
C_ASSERT(offsetof(Player, optionHomingTarget) == 0xE2ABC);
C_ASSERT(offsetof(Player, enemyTrackedPositionValid) == 0xE2AC0);
C_ASSERT(offsetof(Player, shotTimer) == 0xE2AC4);
C_ASSERT(offsetof(Player, baseShotAngle) == 0xE2B0C);
C_ASSERT(offsetof(Player, timer) == 0xE2AF4);
C_ASSERT(offsetof(Player, calcChain) == 0xE2B10);
C_ASSERT(offsetof(Player, extremeGaugeEffect) == 0xE2B24);
C_ASSERT(offsetof(Player, deathbombEffectVm) == 0xE2B28);
C_ASSERT(offsetof(Player, damageAccumulatorThreshold) == 0xE2B2C);

DIFFABLE_EXTERN(Player, g_Player);

} /* namespace th08 */
