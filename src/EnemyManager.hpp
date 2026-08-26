#pragma once
#include "Global.hpp"
#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"

namespace th08
{

struct EnemyEclContext;

struct EnemyUnkStruct3
{
    void *callback;
    ZunTimer timer;
    i32 duration;
    i32 callbackIndex;
    i32 easing;
    f32 parameters[4];
    f32 affectedVariable;

    EnemyUnkStruct3();
};
C_ASSERT(sizeof(EnemyUnkStruct3) == 0x30);
C_ASSERT(offsetof(EnemyUnkStruct3, duration) == 0x10);
C_ASSERT(offsetof(EnemyUnkStruct3, affectedVariable) == 0x2c);

struct EnemyUnkStruct2
{
    EclRawInstruction *currentInstr;
    ZunTimer time;
    EnemyEclContextCallback callback;
    void *callbackArgument;
    i32 intVariables[8];
    f32 floatVariables[8];
    i32 extraIntVariables[4];
    f32 extraFloatVariables[2];
    i32 callParameterInts[4];
    f32 callParameterFloats[4];
    ZunTimer secondaryTime;
    EnemyUnkStruct3 interpolationSlots[8];
    i32 unknown21c;
    i32 childContextSlot;
    i16 subId;
    unknown_fields(0x226, 2);

    EnemyUnkStruct2();
};
C_ASSERT(sizeof(EnemyUnkStruct2) == 0x228);
C_ASSERT(offsetof(EnemyUnkStruct2, callback) == 0x10);
C_ASSERT(offsetof(EnemyUnkStruct2, intVariables) == 0x18);
C_ASSERT(offsetof(EnemyUnkStruct2, floatVariables) == 0x38);
C_ASSERT(offsetof(EnemyUnkStruct2, extraIntVariables) == 0x58);
C_ASSERT(offsetof(EnemyUnkStruct2, extraFloatVariables) == 0x68);
C_ASSERT(offsetof(EnemyUnkStruct2, callParameterInts) == 0x70);
C_ASSERT(offsetof(EnemyUnkStruct2, secondaryTime) == 0x90);
C_ASSERT(offsetof(EnemyUnkStruct2, interpolationSlots) == 0x9c);
C_ASSERT(offsetof(EnemyUnkStruct2, childContextSlot) == 0x220);
C_ASSERT(offsetof(EnemyUnkStruct2, subId) == 0x224);

struct EnemyUnkStruct0x1c
{
    Float3 position;
    Float3 velocity;
    i32 unknown18;

    EnemyUnkStruct0x1c();
};
C_ASSERT(sizeof(EnemyUnkStruct0x1c) == 0x1c);

struct EnemyBulletRankInfluence
{
    f32 speedLow;
    f32 speedHigh;
    i16 count1Low;
    i16 count1High;
    i16 count2Low;
    i16 count2High;
};
C_ASSERT(sizeof(EnemyBulletRankInfluence) == 0x10);

void __fastcall FUN_0042bc50(void *self);

struct Enemy
{
    Enemy();
    void FUN_0042b370(i32 amount);
    void FUN_0042bcf0();
    void FUN_0042e010();

    unknown_fields(0x0, 0xc);
    AnmVm vm;
    AnmVm secondaryVms[2];
    EnemyUnkStruct2 mainEclContextStorage;
    EnemyUnkStruct2 mainEclCallStackStorage[16];
    EnemyEclContext *activeEclContext;
    EnemyEclContext *activeEclCallStack;
    i32 eclIntVariables[8];
    f32 eclFloatVariables[8];
    i16 mainEclCallStackDepth;
    i16 activeEclCallStackDepth;
    unknown_fields(0x2cec, 2);
    i16 deathCallbackSubId;
    i16 eclSubroutineIds[32];
    i16 pendingEclSubroutineIndex;
    unknown_fields(0x2d32, 2);
    Float3 position;
    Float3 positionOffset;
    Float3 velocity;
    Float3 previousPosition;
    Float3 lastFrameDisplacement;
    Float3 hitboxDimensions;
    Float3 secondaryHitboxDimensions;
    Float3 worldPosition;
    f32 movementAngle;
    f32 angularVelocity;
    f32 orbitAngle;
    f32 orbitAngularVelocity;
    Enemy *parentEnemy;
    f32 speed;
    f32 acceleration;
    f32 orbitRadius;
    f32 radialVelocity;
    Float3 shootOffset;
    Float3 movementInterpolationDelta;
    Float3 movementInterpolationOrigin;
    ZunTimer movementTimer;
    i32 movementDuration;
    EnemyBulletRankInfluence bulletRankInfluence;
    i32 life;
    i32 maxLife;
    i32 phaseStartingLife;
    i32 score;
    i32 enemyIndex;
    i32 playerShotHitAccumulator;
    ZunTimer bossTimer;
    D3DCOLOR displayColor;
    BulletSpawnDescriptor bulletSpawnDescriptor;
    u8 pendingShotInstruction[0x2c];
    i32 shootIntervalFrames;
    ZunTimer shootIntervalTimer;
    BulletSpawnDescriptor laserSpawnDescriptor;
    unknown_fields(0x3280, 0x98);
    ZunTimer timer3318;
    unknown_fields(0x3324, 0x30);
    i32 lastDamage;
    i32 lifeCallbackThresholds[4];
    i32 lifeCallbackSubIds[4];
    i32 timerCallbackThresholdFrames;
    i32 timerCallbackSubId;
    i32 linkedChildCount;
    u8 *childEclBlocks[4];
    EnemyUnkStruct0x1c trail[96];
    VertexTex1DiffuseXyzrhw vertices[194];
    unknown_fields(0x534c, 8);
    ZunTimer timer5354;
    unknown_fields(0x5360, 0x70);

    // Target-observed RunEcl post-dispatch calls.  Both receive the current
    // enemy in ECX and take no explicit arguments.
    void FUN_0042a820();
    void UpdateMovement();
    void FUN_00423150();
    void FUN_0042bc90();
    void enemy_fun_00415c80();
    void FUN_0042b2f0();
    void FUN_0042bea0(i32 mode);
    i32 FUN_0042b490();
    i32 FUN_0042b930();
    void ClampPosition();
    void FUN_0042c290(Float3 *position, Float3 *size);
    void FUN_0042c420();
    void IntegrateVelocity();
};
C_ASSERT(sizeof(Enemy) == 0x53d0);
C_ASSERT(offsetof(Enemy, mainEclContextStorage) == 0x7f8);
C_ASSERT(offsetof(Enemy, mainEclCallStackStorage) == 0xa20);
C_ASSERT(offsetof(Enemy, activeEclContext) == 0x2ca0);
C_ASSERT(offsetof(Enemy, activeEclCallStack) == 0x2ca4);
C_ASSERT(offsetof(Enemy, eclIntVariables) == 0x2ca8);
C_ASSERT(offsetof(Enemy, eclFloatVariables) == 0x2cc8);
C_ASSERT(offsetof(Enemy, mainEclCallStackDepth) == 0x2ce8);
C_ASSERT(offsetof(Enemy, activeEclCallStackDepth) == 0x2cea);
C_ASSERT(offsetof(Enemy, deathCallbackSubId) == 0x2cee);
C_ASSERT(offsetof(Enemy, eclSubroutineIds) == 0x2cf0);
C_ASSERT(offsetof(Enemy, pendingEclSubroutineIndex) == 0x2d30);
C_ASSERT(offsetof(Enemy, position) == 0x2d34);
C_ASSERT(offsetof(Enemy, positionOffset) == 0x2d40);
C_ASSERT(offsetof(Enemy, velocity) == 0x2d4c);
C_ASSERT(offsetof(Enemy, previousPosition) == 0x2d58);
C_ASSERT(offsetof(Enemy, lastFrameDisplacement) == 0x2d64);
C_ASSERT(offsetof(Enemy, hitboxDimensions) == 0x2d70);
C_ASSERT(offsetof(Enemy, secondaryHitboxDimensions) == 0x2d7c);
C_ASSERT(offsetof(Enemy, worldPosition) == 0x2d88);
C_ASSERT(offsetof(Enemy, movementAngle) == 0x2d94);
C_ASSERT(offsetof(Enemy, angularVelocity) == 0x2d98);
C_ASSERT(offsetof(Enemy, orbitAngle) == 0x2d9c);
C_ASSERT(offsetof(Enemy, orbitAngularVelocity) == 0x2da0);
C_ASSERT(offsetof(Enemy, parentEnemy) == 0x2da4);
C_ASSERT(offsetof(Enemy, speed) == 0x2da8);
C_ASSERT(offsetof(Enemy, acceleration) == 0x2dac);
C_ASSERT(offsetof(Enemy, orbitRadius) == 0x2db0);
C_ASSERT(offsetof(Enemy, radialVelocity) == 0x2db4);
C_ASSERT(offsetof(Enemy, shootOffset) == 0x2db8);
C_ASSERT(offsetof(Enemy, movementInterpolationDelta) == 0x2dc4);
C_ASSERT(offsetof(Enemy, movementInterpolationOrigin) == 0x2dd0);
C_ASSERT(offsetof(Enemy, movementTimer) == 0x2ddc);
C_ASSERT(offsetof(Enemy, movementDuration) == 0x2de8);
C_ASSERT(offsetof(Enemy, bulletRankInfluence) == 0x2dec);
C_ASSERT(offsetof(Enemy, life) == 0x2dfc);
C_ASSERT(offsetof(Enemy, maxLife) == 0x2e00);
C_ASSERT(offsetof(Enemy, phaseStartingLife) == 0x2e04);
C_ASSERT(offsetof(Enemy, score) == 0x2e08);
C_ASSERT(offsetof(Enemy, enemyIndex) == 0x2e0c);
C_ASSERT(offsetof(Enemy, playerShotHitAccumulator) == 0x2e10);
C_ASSERT(offsetof(Enemy, bossTimer) == 0x2e14);
C_ASSERT(offsetof(Enemy, displayColor) == 0x2e20);
C_ASSERT(offsetof(Enemy, bulletSpawnDescriptor) == 0x2e24);
C_ASSERT(offsetof(Enemy, pendingShotInstruction) == 0x3034);
C_ASSERT(offsetof(Enemy, shootIntervalFrames) == 0x3060);
C_ASSERT(offsetof(Enemy, shootIntervalTimer) == 0x3064);
C_ASSERT(offsetof(Enemy, laserSpawnDescriptor) == 0x3070);
C_ASSERT(offsetof(Enemy, lastDamage) == 0x3354);
C_ASSERT(offsetof(Enemy, lifeCallbackThresholds) == 0x3358);
C_ASSERT(offsetof(Enemy, lifeCallbackSubIds) == 0x3368);
C_ASSERT(offsetof(Enemy, timerCallbackThresholdFrames) == 0x3378);
C_ASSERT(offsetof(Enemy, timerCallbackSubId) == 0x337c);
C_ASSERT(offsetof(Enemy, linkedChildCount) == 0x3380);
C_ASSERT(offsetof(Enemy, childEclBlocks) == 0x3384);
C_ASSERT(offsetof(Enemy, trail) == 0x3394);

struct EclTimelineInstruction
{
    i32 time;
    i16 opcode;
    u8 size;
    u8 difficultyMask;
    union
    {
        i32 ints[7];
        f32 floats[7];
    } args;
};

struct EclTimeline
{
    ZunTimer timer;
    EclTimelineInstruction *instruction;
    EclTimeline();
    void Run();
};
C_ASSERT(sizeof(EclTimeline) == 0x10);

struct EnemyManager
{
    EnemyManager();

    Enemy firstEnemy;
    Enemy enemies[481];
    unknown_fields(0x9dcda0, 0x30);
    EclTimeline timelines[16];
    ZunTimer timer;
    unknown_fields(0x9dcedc, 0x34);

    void Initialize();
    void FUN_0042c3b0();
    static ZunResult RegisterChain();
    static ChainCallbackResult OnUpdate(EnemyManager *enemyManager);
    static ChainCallbackResult OnDrawHighPrio(EnemyManager *enemyManager);
    ChainCallbackResult __fastcall OnDrawImpl(i32 drawGroup, i32 chainPriority);
    static ChainCallbackResult OnDrawLowPrio(EnemyManager *enemyManager);
    static ZunResult AddedCallback(EnemyManager *enemyManager);
    static ZunResult DeletedCallback(EnemyManager *enemyManager);
    void *SpawnEnemy1(i32 type, const D3DXVECTOR3 *position, i32 a, i32 b, i32 c, i32 flags);
    void *SpawnEnemy2(i32 type, const D3DXVECTOR3 *position, i32 a, i32 b, i32 c, i32 *contextInts);
    i32 FUN_0042efb0(i32 maxScore, i32 totalScore);
    i32 FUN_0042f1f0();
    static void CutChain();
};
C_ASSERT(sizeof(EnemyManager) == 0x9dcf10);

DIFFABLE_EXTERN(EnemyManager, g_EnemyManager);

} /* namespace th08 */
