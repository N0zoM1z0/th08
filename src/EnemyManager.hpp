#pragma once
#include "Global.hpp"
#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"

namespace th08
{

struct EnemyUnkStruct3
{
    i32 unknown0;
    ZunTimer timer;
    unknown_fields(0x10, 0x20);

    EnemyUnkStruct3();
};
C_ASSERT(sizeof(EnemyUnkStruct3) == 0x30);

struct EnemyUnkStruct2
{
    i32 unknown0;
    ZunTimer timer4;
    unknown_fields(0x10, 0x80);
    ZunTimer timer90;
    EnemyUnkStruct3 rows[8];
    unknown_fields(0x21c, 0xc);

    EnemyUnkStruct2();
};
C_ASSERT(sizeof(EnemyUnkStruct2) == 0x228);

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
    EnemyUnkStruct2 firstContext;
    EnemyUnkStruct2 contexts[16];
    unknown_fields(0x2ca0, 0x94);
    Float3 vector2d34;
    Float3 vector2d40;
    Float3 vector2d4c;
    Float3 vector2d58;
    Float3 vector2d64;
    Float3 vector2d70;
    Float3 vector2d7c;
    Float3 vector2d88;
    unknown_fields(0x2d94, 0x24);
    Float3 vector2db8;
    Float3 vector2dc4;
    Float3 vector2dd0;
    ZunTimer timer2ddc;
    unknown_fields(0x2de8, 4);
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
    void FUN_00422c40();
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
    void FUN_0042deb0();
};
C_ASSERT(sizeof(Enemy) == 0x53d0);
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
