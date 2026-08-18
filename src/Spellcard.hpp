#pragma once

#include "AnmManager.hpp"
#include "ZunBool.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "ZunMath.hpp"

namespace th08
{
// Naming scheme:
// SPELLCARD_[STAGE]_[ENEMY]_(LAST SPELL)(NUM)(DIFFICULTY)
// [] fields are mandatory
// () fields are optional
enum SpellcardNumber
{
    SPELLCARD_ST1_MBOSS_1H,
    SPELLCARD_ST1_MBOSS_1L,
    SPELLCARD_ST1_BOSS_1E,
    SPELLCARD_ST1_BOSS_1N,
    SPELLCARD_ST1_BOSS_1H,
    SPELLCARD_ST1_BOSS_1L,
    SPELLCARD_ST1_BOSS_2E,
    SPELLCARD_ST1_BOSS_2N,
    SPELLCARD_ST1_BOSS_2H,
    SPELLCARD_ST1_BOSS_2L,
    SPELLCARD_ST1_BOSS_LSN,
    SPELLCARD_ST1_BOSS_LSH,
    SPELLCARD_ST1_BOSS_LSL,

    SPELLCARD_ST2_MBOSS_1E,
    SPELLCARD_ST2_MBOSS_1N,
    SPELLCARD_ST2_MBOSS_1H,
    SPELLCARD_ST2_MBOSS_1L,
    SPELLCARD_ST2_BOSS_1E,
    SPELLCARD_ST2_BOSS_1N,
    SPELLCARD_ST2_BOSS_1H,
    SPELLCARD_ST2_BOSS_1L,
    SPELLCARD_ST2_BOSS_2E,
    SPELLCARD_ST2_BOSS_2N,
    SPELLCARD_ST2_BOSS_2H,
    SPELLCARD_ST2_BOSS_2L,
    SPELLCARD_ST2_BOSS_3E,
    SPELLCARD_ST2_BOSS_3N,
    SPELLCARD_ST2_BOSS_3H,
    SPELLCARD_ST2_BOSS_3L,
    SPELLCARD_ST2_BOSS_LSN,
    SPELLCARD_ST2_BOSS_LSH,
    SPELLCARD_ST2_BOSS_LSL,

    SPELLCARD_ST3_MBOSS_1E,
    SPELLCARD_ST3_MBOSS_1N,
    SPELLCARD_ST3_MBOSS_1H,
    SPELLCARD_ST3_MBOSS_1L,
    SPELLCARD_ST3_BOSS_1N,
    SPELLCARD_ST3_BOSS_1H,
    SPELLCARD_ST3_BOSS_1L,
    SPELLCARD_ST3_BOSS_2E,
    SPELLCARD_ST3_BOSS_2N,
    SPELLCARD_ST3_BOSS_2H,
    SPELLCARD_ST3_BOSS_2L,
    SPELLCARD_ST3_BOSS_3E,
    SPELLCARD_ST3_BOSS_3N,
    SPELLCARD_ST3_BOSS_3H,
    SPELLCARD_ST3_BOSS_3L,
    SPELLCARD_ST3_BOSS_4E,
    SPELLCARD_ST3_BOSS_4N,
    SPELLCARD_ST3_BOSS_4H,
    SPELLCARD_ST3_BOSS_4L,
    SPELLCARD_ST3_BOSS_LSN,
    SPELLCARD_ST3_BOSS_LSH,
    SPELLCARD_ST3_BOSS_LSL,

    SPELLCARD_ST4A_MBOSS_1E,
    SPELLCARD_ST4A_MBOSS_1N,
    SPELLCARD_ST4A_MBOSS_1H,
    SPELLCARD_ST4A_MBOSS_1L,
    SPELLCARD_ST4A_MBOSS_2E,
    SPELLCARD_ST4A_MBOSS_2N,
    SPELLCARD_ST4A_MBOSS_2H,
    SPELLCARD_ST4A_MBOSS_2L,
    SPELLCARD_ST4A_BOSS_1E,
    SPELLCARD_ST4A_BOSS_1N,
    SPELLCARD_ST4A_BOSS_1H,
    SPELLCARD_ST4A_BOSS_1L,
    SPELLCARD_ST4A_BOSS_2E,
    SPELLCARD_ST4A_BOSS_2N,
    SPELLCARD_ST4A_BOSS_2H,
    SPELLCARD_ST4A_BOSS_2L,
    SPELLCARD_ST4A_BOSS_3E,
    SPELLCARD_ST4A_BOSS_3N,
    SPELLCARD_ST4A_BOSS_3H,
    SPELLCARD_ST4A_BOSS_3L,
    SPELLCARD_ST4A_BOSS_LSN,
    SPELLCARD_ST4A_BOSS_LSH,
    SPELLCARD_ST4A_BOSS_LSL,

    SPELLCARD_ST4B_MBOSS_1E,
    SPELLCARD_ST4B_MBOSS_1N,
    SPELLCARD_ST4B_MBOSS_1H,
    SPELLCARD_ST4B_MBOSS_1L,
    SPELLCARD_ST4B_MBOSS_2E,
    SPELLCARD_ST4B_MBOSS_2N,
    SPELLCARD_ST4B_MBOSS_2H,
    SPELLCARD_ST4B_MBOSS_2L,
    SPELLCARD_ST4B_BOSS_1E,
    SPELLCARD_ST4B_BOSS_1N,
    SPELLCARD_ST4B_BOSS_1H,
    SPELLCARD_ST4B_BOSS_1L,
    SPELLCARD_ST4B_BOSS_2E,
    SPELLCARD_ST4B_BOSS_2N,
    SPELLCARD_ST4B_BOSS_2H,
    SPELLCARD_ST4B_BOSS_2L,
    SPELLCARD_ST4B_BOSS_3E,
    SPELLCARD_ST4B_BOSS_3N,
    SPELLCARD_ST4B_BOSS_3H,
    SPELLCARD_ST4B_BOSS_3L,
    SPELLCARD_ST4B_BOSS_LSN,
    SPELLCARD_ST4B_BOSS_LSH,
    SPELLCARD_ST4B_BOSS_LSL,

    SPELLCARD_ST5_BOSS_1E,
    SPELLCARD_ST5_BOSS_1N,
    SPELLCARD_ST5_BOSS_1H,
    SPELLCARD_ST5_BOSS_1L,
    SPELLCARD_ST5_BOSS_2E,
    SPELLCARD_ST5_BOSS_2N,
    SPELLCARD_ST5_BOSS_2H,
    SPELLCARD_ST5_BOSS_2L,
    SPELLCARD_ST5_BOSS_3E,
    SPELLCARD_ST5_BOSS_3N,
    SPELLCARD_ST5_BOSS_3H,
    SPELLCARD_ST5_BOSS_3L,
    SPELLCARD_ST5_BOSS_4E,
    SPELLCARD_ST5_BOSS_4N,
    SPELLCARD_ST5_BOSS_4H,
    SPELLCARD_ST5_BOSS_4L,
    SPELLCARD_ST5_BOSS_LSN,
    SPELLCARD_ST5_BOSS_LSH,
    SPELLCARD_ST5_BOSS_LSL,

    SPELLCARD_ST6A_MBOSS_1E,
    SPELLCARD_ST6A_MBOSS_1N,
    SPELLCARD_ST6A_MBOSS_1H,
    SPELLCARD_ST6A_MBOSS_1L,
    SPELLCARD_ST6A_BOSS_1E,
    SPELLCARD_ST6A_BOSS_1N,
    SPELLCARD_ST6A_BOSS_1H,
    SPELLCARD_ST6A_BOSS_1L,
    SPELLCARD_ST6A_BOSS_2E,
    SPELLCARD_ST6A_BOSS_2N,
    SPELLCARD_ST6A_BOSS_2H,
    SPELLCARD_ST6A_BOSS_2L,
    SPELLCARD_ST6A_BOSS_3E,
    SPELLCARD_ST6A_BOSS_3N,
    SPELLCARD_ST6A_BOSS_3H,
    SPELLCARD_ST6A_BOSS_3L,
    SPELLCARD_ST6A_BOSS_4E,
    SPELLCARD_ST6A_BOSS_4N,
    SPELLCARD_ST6A_BOSS_4H,
    SPELLCARD_ST6A_BOSS_4L,
    SPELLCARD_ST6A_BOSS_5E,
    SPELLCARD_ST6A_BOSS_5N,
    SPELLCARD_ST6A_BOSS_5H,
    SPELLCARD_ST6A_BOSS_5L,
    SPELLCARD_ST6A_BOSS_LSE,
    SPELLCARD_ST6A_BOSS_LSN,
    SPELLCARD_ST6A_BOSS_LSH,
    SPELLCARD_ST6A_BOSS_LSL,

    SPELLCARD_ST6B_MBOSS_1E,
    SPELLCARD_ST6B_MBOSS_1N,
    SPELLCARD_ST6B_MBOSS_1H,
    SPELLCARD_ST6B_MBOSS_1L,
    SPELLCARD_ST6B_BOSS_1E,
    SPELLCARD_ST6B_BOSS_1N,
    SPELLCARD_ST6B_BOSS_1H,
    SPELLCARD_ST6B_BOSS_1L,
    SPELLCARD_ST6B_BOSS_2E,
    SPELLCARD_ST6B_BOSS_2N,
    SPELLCARD_ST6B_BOSS_2H,
    SPELLCARD_ST6B_BOSS_2L,
    SPELLCARD_ST6B_BOSS_3E,
    SPELLCARD_ST6B_BOSS_3N,
    SPELLCARD_ST6B_BOSS_3H,
    SPELLCARD_ST6B_BOSS_3L,
    SPELLCARD_ST6B_BOSS_4E,
    SPELLCARD_ST6B_BOSS_4N,
    SPELLCARD_ST6B_BOSS_4H,
    SPELLCARD_ST6B_BOSS_4L,
    SPELLCARD_ST6B_BOSS_5E,
    SPELLCARD_ST6B_BOSS_5N,
    SPELLCARD_ST6B_BOSS_5H,
    SPELLCARD_ST6B_BOSS_5L,
    SPELLCARD_ST6B_BOSS_LS1E,
    SPELLCARD_ST6B_BOSS_LS1N,
    SPELLCARD_ST6B_BOSS_LS1H,
    SPELLCARD_ST6B_BOSS_LS1L,
    SPELLCARD_ST6B_BOSS_LS2E,
    SPELLCARD_ST6B_BOSS_LS2N,
    SPELLCARD_ST6B_BOSS_LS2H,
    SPELLCARD_ST6B_BOSS_LS2L,
    SPELLCARD_ST6B_BOSS_LS3E,
    SPELLCARD_ST6B_BOSS_LS3N,
    SPELLCARD_ST6B_BOSS_LS3H,
    SPELLCARD_ST6B_BOSS_LS3L,
    SPELLCARD_ST6B_BOSS_LS4E,
    SPELLCARD_ST6B_BOSS_LS4N,
    SPELLCARD_ST6B_BOSS_LS4H,
    SPELLCARD_ST6B_BOSS_LS4L,
    SPELLCARD_ST6B_BOSS_LS5E,
    SPELLCARD_ST6B_BOSS_LS5N,
    SPELLCARD_ST6B_BOSS_LS5H,
    SPELLCARD_ST6B_BOSS_LS5L,

    SPELLCARD_EX_MBOSS_1,
    SPELLCARD_EX_MBOSS_2,
    SPELLCARD_EX_MBOSS_3,
    SPELLCARD_EX_BOSS_1,
    SPELLCARD_EX_BOSS_2,
    SPELLCARD_EX_BOSS_3,
    SPELLCARD_EX_BOSS_4,
    SPELLCARD_EX_BOSS_5,
    SPELLCARD_EX_BOSS_6,
    SPELLCARD_EX_BOSS_7,
    SPELLCARD_EX_BOSS_8,
    SPELLCARD_EX_BOSS_9,
    SPELLCARD_EX_BOSS_10,
    SPELLCARD_EX_BOSS_LS,

    SPELLCARD_COUNT_IN_GAME_SPELLCARDS,
    SPELLCARD_LAST_WORD_START = SPELLCARD_COUNT_IN_GAME_SPELLCARDS,

    SPELLCARD_LW_WRIGGLE = SPELLCARD_LAST_WORD_START,
    SPELLCARD_LW_MYSTIA,
    SPELLCARD_LW_KEINE,
    SPELLCARD_LW_REISEN,
    SPELLCARD_LW_EIRIN,
    SPELLCARD_LW_KAGUYA,
    SPELLCARD_LW_MOKOU,
    SPELLCARD_LW_TEWI,
    SPELLCARD_LW_KEINEEX,
    SPELLCARD_LW_REIMU,
    SPELLCARD_LW_MARISA,
    SPELLCARD_LW_SAKUYA,
    SPELLCARD_LW_YOUMU,
    SPELLCARD_LW_ALICE,
    SPELLCARD_LW_REMILIA,
    SPELLCARD_LW_YUYUKO,
    SPELLCARD_LW_YUKARI,

    SPELLCARD_COUNT_SPELLCARDS,
    SPELLCARD_COUNT_LAST_WORD_SPELLCARDS = SPELLCARD_COUNT_SPELLCARDS - SPELLCARD_COUNT_IN_GAME_SPELLCARDS,
};

struct Spellcard
{
    Spellcard();
    ZunResult Init();

    u32 flags;                       // +0x000
    u8 *activeEnemy;                 // +0x004
    i32 spellCardNumber;             // +0x008
    u32 enemySpellFlagsSnapshot;     // +0x00C
    i32 pendingTimeOrbs;             // +0x010
    char spellName[48];              // +0x014
    u8 unknown_044[0x30];            // +0x044
    char spellCommentLine1[64];      // +0x074
    char spellCommentLine2[64];      // +0x0B4
    u8 *spellEffect;                 // +0x0F4
    u8 *rewardEffect;                // +0x0F8
    i32 bonusProgress;               // +0x0FC
    i32 bonusCounter;                // +0x100
    i32 bonusAward;                  // +0x104
    ZunTimer timer108;               // +0x108
    ZunTimer timer114;               // +0x114
    AnmVm vm120;                     // +0x120
    AnmVm vm3C4;                     // +0x3C4
    AnmVm vm668;                     // +0x668
    AnmVm vm90C;                     // +0x90C
    AnmVm vmBB0;                     // +0xBB0
    AnmVm vmE54;                     // +0xE54
    AnmVm vm10F8;                    // +0x10F8
    AnmVm vm139C;                    // +0x139C
    AnmVm vm1640;                    // +0x1640
    AnmVm vm18E4;                    // +0x18E4
    AnmVm vm1B88;                    // +0x1B88
    AnmVm vm1E2C;                    // +0x1E2C
    AnmVm vm20D0;                    // +0x20D0
    AnmVm vm2374;                    // +0x2374
    f32 playerSpellNameWidth;        // +0x2618
    f32 enemySpellNameWidth;         // +0x261C
    D3DCOLOR mixColor;                 // +0x2620
    AnmLoaded *playerFaceAnm0;       // +0x2624
    AnmLoaded *playerFaceAnm1;       // +0x2628
    AnmLoaded *enemyFaceAnm0;        // +0x262C
    AnmLoaded *enemyFaceAnm1;        // +0x2630
    AnmLoaded *commonFaceAnm;        // +0x2634
    i32 scoreLimit;                  // +0x2638
    void *lifetimeObject;            // +0x263C
    ChainElem *lifetimeChain;        // +0x2640

    void StartSpell(i32 spellCardNumber, const u8 *encodedName, i32 enemyFace, i32 bonus, u8 *enemy,
                    const u8 *encodedOwner, const char *commentLine1, const char *commentLine2);
    void CutInEnemyNoPortrait(const char *name, i32 unused);
    void CutInPlayer(i32 playerFace, const char *name, i32 sprite);
    void CutInEnemy(i32 enemyFace, const char *name, i32 sprite);
    void spellcard_fun_00416130();
    void spellcard_fun_00416160();
    void FUN_0044cba0();
    void FUN_0044d150();
    void EndSpell();
    void spellcard_fun_00416af0();
    void spellcard_fun_00416b10(i32 amount);
    i32 OnUpdateImpl();
    i32 OnDrawImpl();

    static ChainCallbackResult OnUpdate(Spellcard *spellcard);
    static ChainCallbackResult OnDraw(Spellcard *spellcard);

    void SetStoredVector(f32 x, f32 y, f32 z);
    void FUN_0041f0b0(i32 value);
    void FUN_0041f0e0(i32 value);
    i32 IsActive();
    i32 GetInactiveState();
    i32 GetActiveState();
    i32 GetTimerFrames();
    i32 FUN_00417860();
    i32 FUN_0042DFF0();

    static ZunResult RegisterChain();
    static ZunResult DeletedCallback(Spellcard *spellcard);
    static i32 __fastcall IsLastSpell(i32 spellCardNumber);

    static i32 GetDifficultyFromSpellCard(i32 spellcardNumber);
    static void CutChain();
};
C_ASSERT(sizeof(Spellcard) == 0x2644);

DIFFABLE_EXTERN_ARRAY(i32 *, 6, g_SpellcardNumbersPerDifficulty);
DIFFABLE_EXTERN_ARRAY(i32, 6, g_SpellcardCountsPerDifficulty);
DIFFABLE_EXTERN_ARRAY(i32, 43, g_LastSpellNumbers);
DIFFABLE_EXTERN(i32, g_LastSpellCount);
DIFFABLE_EXTERN(Spellcard, g_Spellcard);
DIFFABLE_EXTERN(ChainElem *, g_SpellcardCalcChain);
DIFFABLE_EXTERN_ARRAY(i32 *, 10, g_SpellcardNumbersPerStage)
DIFFABLE_EXTERN_ARRAY(i32, 10, g_SpellcardCountPerStage)

} /* namespace th08 */
