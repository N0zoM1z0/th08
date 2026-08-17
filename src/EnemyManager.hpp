#pragma once
#include "Global.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"

namespace th08
{

struct Enemy
{
    // Target-observed RunEcl post-dispatch calls.  Both receive the current
    // enemy in ECX and take no explicit arguments.
    void FUN_0042a820();
    void FUN_00422c40();
    void FUN_00423150();
    void FUN_0042bc90();
    void enemy_fun_00415c80();
    void FUN_0042b2f0();
};

struct EnemyManager
{
    void Initialize();
    static ZunResult RegisterChain();
    static ChainCallbackResult OnUpdate();
    static ChainCallbackResult OnDrawHighPrio(EnemyManager *enemyManager);
    ChainCallbackResult __fastcall OnDrawImpl(i32 drawGroup, i32 chainPriority);
    static ChainCallbackResult OnDrawLowPrio(EnemyManager *enemyManager);
    static ZunResult AddedCallback(EnemyManager *enemyManager);
    static ZunResult DeletedCallback(EnemyManager *enemyManager);
    void *SpawnEnemy2(i32 type, const D3DXVECTOR3 *position, i32 a, i32 b, i32 c, i32 *outContext);
    i32 FUN_0042efb0(i32 maxScore, i32 totalScore);
    static void CutChain();
};

DIFFABLE_EXTERN(EnemyManager, g_EnemyManager);

} /* namespace th08 */
