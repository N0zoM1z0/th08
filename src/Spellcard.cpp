#include "th_pch.h"

#include "ScoreDat.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "EnemyManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "Global.hpp"
#include "utils.hpp"

namespace th08
{
ZunBool IsDisableResourceReload();
DIFFABLE_STATIC(Spellcard, g_Spellcard);
DIFFABLE_STATIC(ChainElem *, g_SpellcardCalcChain);
DIFFABLE_STATIC(i32, g_LastSpellCount);
DIFFABLE_STATIC(AnmLoaded *, g_SpellcardBackgroundAnm);

struct SpellcardFlagBits
{
    u32 bits0To4 : 5;
    u32 alternateEffectStyle : 1;
    u32 highBits : 26;
};

struct SpellcardResetFlagBits
{
    u32 active : 1;
    u32 bits1To6 : 6;
    u32 bombDamageEnabled : 1;
    u32 rest : 24;
};

// clang-format off
// TODO: stop clang-format from fucking with whitespace formatting

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 42, g_SpellcardNumbersEasy) = {
    // Stage 1
    SPELLCARD_ST1_BOSS_1E,    SPELLCARD_ST1_BOSS_2E,
    // Stage 2
    SPELLCARD_ST2_MBOSS_1E,   SPELLCARD_ST2_BOSS_1E,    SPELLCARD_ST2_BOSS_2E,    SPELLCARD_ST2_BOSS_3E,
    // Stage 3
    SPELLCARD_ST3_MBOSS_1E,   SPELLCARD_ST3_BOSS_2E,    SPELLCARD_ST3_BOSS_3E,    SPELLCARD_ST3_BOSS_4E,
    // Stage 4A
    SPELLCARD_ST4A_MBOSS_1E,  SPELLCARD_ST4A_MBOSS_2E,  SPELLCARD_ST4A_BOSS_1E,   SPELLCARD_ST4A_BOSS_2E,
    SPELLCARD_ST4A_BOSS_3E,
    // Stage 4B
    SPELLCARD_ST4B_MBOSS_1E,  SPELLCARD_ST4B_MBOSS_2E,  SPELLCARD_ST4B_BOSS_1E,   SPELLCARD_ST4B_BOSS_2E,
    SPELLCARD_ST4B_BOSS_3E,
    // Stage 5
    SPELLCARD_ST5_BOSS_1E,    SPELLCARD_ST5_BOSS_2E,    SPELLCARD_ST5_BOSS_3E,    SPELLCARD_ST5_BOSS_4E,
    // Stage 6A
    SPELLCARD_ST6A_MBOSS_1E,  SPELLCARD_ST6A_BOSS_1E,   SPELLCARD_ST6A_BOSS_2E,   SPELLCARD_ST6A_BOSS_3E,
    SPELLCARD_ST6A_BOSS_4E,   SPELLCARD_ST6A_BOSS_5E,   SPELLCARD_ST6A_BOSS_LSE,
    // Stage 6B
    SPELLCARD_ST6B_MBOSS_1E,  SPELLCARD_ST6B_BOSS_1E,   SPELLCARD_ST6B_BOSS_2E,   SPELLCARD_ST6B_BOSS_3E,
    SPELLCARD_ST6B_BOSS_4E,   SPELLCARD_ST6B_BOSS_5E,   SPELLCARD_ST6B_BOSS_LS1E, SPELLCARD_ST6B_BOSS_LS2E,
    SPELLCARD_ST6B_BOSS_LS3E, SPELLCARD_ST6B_BOSS_LS4E, SPELLCARD_ST6B_BOSS_LS5E,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 49, g_SpellcardNumbersNormal) = {
    // Stage 1
    SPELLCARD_ST1_BOSS_1N,    SPELLCARD_ST1_BOSS_2N,    SPELLCARD_ST1_BOSS_LSN,
    // Stage 2
    SPELLCARD_ST2_MBOSS_1N,   SPELLCARD_ST2_BOSS_1N,    SPELLCARD_ST2_BOSS_2N,    SPELLCARD_ST2_BOSS_3N,
    SPELLCARD_ST2_BOSS_LSN,
    // Stage 3
    SPELLCARD_ST3_MBOSS_1N,   SPELLCARD_ST3_BOSS_1N,    SPELLCARD_ST3_BOSS_2N,    SPELLCARD_ST3_BOSS_3N,
    SPELLCARD_ST3_BOSS_4N,    SPELLCARD_ST3_BOSS_LSN,
    // Stage 4A
    SPELLCARD_ST4A_MBOSS_1N,  SPELLCARD_ST4A_MBOSS_2N,  SPELLCARD_ST4A_BOSS_1N,   SPELLCARD_ST4A_BOSS_2N,
    SPELLCARD_ST4A_BOSS_3N,   SPELLCARD_ST4A_BOSS_LSN,
    // Stage 4B
    SPELLCARD_ST4B_MBOSS_1N,  SPELLCARD_ST4B_MBOSS_2N,  SPELLCARD_ST4B_BOSS_1N,   SPELLCARD_ST4B_BOSS_2N,
    SPELLCARD_ST4B_BOSS_3N,   SPELLCARD_ST4B_BOSS_LSN,
    // Stage 5
    SPELLCARD_ST5_BOSS_1N,    SPELLCARD_ST5_BOSS_2N,    SPELLCARD_ST5_BOSS_3N,    SPELLCARD_ST5_BOSS_4N,
    SPELLCARD_ST5_BOSS_LSN,
    // Stage 6A
    SPELLCARD_ST6A_MBOSS_1N,  SPELLCARD_ST6A_BOSS_1N,   SPELLCARD_ST6A_BOSS_2N,   SPELLCARD_ST6A_BOSS_3N,
    SPELLCARD_ST6A_BOSS_4N,   SPELLCARD_ST6A_BOSS_5N,   SPELLCARD_ST6A_BOSS_LSN,
    // Stage 6B
    SPELLCARD_ST6B_MBOSS_1N,  SPELLCARD_ST6B_BOSS_1N,   SPELLCARD_ST6B_BOSS_2N,   SPELLCARD_ST6B_BOSS_3N,
    SPELLCARD_ST6B_BOSS_4N,   SPELLCARD_ST6B_BOSS_5N,   SPELLCARD_ST6B_BOSS_LS1N, SPELLCARD_ST6B_BOSS_LS2N,
    SPELLCARD_ST6B_BOSS_LS3N, SPELLCARD_ST6B_BOSS_LS4N, SPELLCARD_ST6B_BOSS_LS5N,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 50, g_SpellcardNumbersHard) = {
    // Stage 1
    SPELLCARD_ST1_MBOSS_1H,   SPELLCARD_ST1_BOSS_1H,    SPELLCARD_ST1_BOSS_2H,    SPELLCARD_ST1_BOSS_LSH,
    // Stage 2
    SPELLCARD_ST2_MBOSS_1H,   SPELLCARD_ST2_BOSS_1H,    SPELLCARD_ST2_BOSS_2H,    SPELLCARD_ST2_BOSS_3H,
    SPELLCARD_ST2_BOSS_LSH,
    // Stage 3
    SPELLCARD_ST3_MBOSS_1H,   SPELLCARD_ST3_BOSS_1H,    SPELLCARD_ST3_BOSS_2H,    SPELLCARD_ST3_BOSS_3H,
    SPELLCARD_ST3_BOSS_4H,    SPELLCARD_ST3_BOSS_LSH,
    // Stage 4A
    SPELLCARD_ST4A_MBOSS_1H,  SPELLCARD_ST4A_MBOSS_2H,  SPELLCARD_ST4A_BOSS_1H,   SPELLCARD_ST4A_BOSS_2H,
    SPELLCARD_ST4A_BOSS_3H,   SPELLCARD_ST4A_BOSS_LSH,
    // Stage 4B
    SPELLCARD_ST4B_MBOSS_1H,  SPELLCARD_ST4B_MBOSS_2H,  SPELLCARD_ST4B_BOSS_1H,   SPELLCARD_ST4B_BOSS_2H,
    SPELLCARD_ST4B_BOSS_3H,   SPELLCARD_ST4B_BOSS_LSH,
    // Stage 5
    SPELLCARD_ST5_BOSS_1H,    SPELLCARD_ST5_BOSS_2H,    SPELLCARD_ST5_BOSS_3H,    SPELLCARD_ST5_BOSS_4H,
    SPELLCARD_ST5_BOSS_LSH,
    // Stage 6A
    SPELLCARD_ST6A_MBOSS_1H,  SPELLCARD_ST6A_BOSS_1H,   SPELLCARD_ST6A_BOSS_2H,   SPELLCARD_ST6A_BOSS_3H,
    SPELLCARD_ST6A_BOSS_4H,   SPELLCARD_ST6A_BOSS_5H,   SPELLCARD_ST6A_BOSS_LSH,
    // Stage 6B
    SPELLCARD_ST6B_MBOSS_1H,  SPELLCARD_ST6B_BOSS_1H,   SPELLCARD_ST6B_BOSS_2H,   SPELLCARD_ST6B_BOSS_3H,
    SPELLCARD_ST6B_BOSS_4H,   SPELLCARD_ST6B_BOSS_5H,   SPELLCARD_ST6B_BOSS_LS1H, SPELLCARD_ST6B_BOSS_LS2H,
    SPELLCARD_ST6B_BOSS_LS3H, SPELLCARD_ST6B_BOSS_LS4H, SPELLCARD_ST6B_BOSS_LS5H,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 50, g_SpellcardNumbersLunatic) = {
    // Stage 1
    SPELLCARD_ST1_MBOSS_1L,   SPELLCARD_ST1_BOSS_1L,    SPELLCARD_ST1_BOSS_2L,    SPELLCARD_ST1_BOSS_LSL,
    // Stage 2
    SPELLCARD_ST2_MBOSS_1L,   SPELLCARD_ST2_BOSS_1L,    SPELLCARD_ST2_BOSS_2L,    SPELLCARD_ST2_BOSS_3L,
    SPELLCARD_ST2_BOSS_LSL,
    // Stage 3
    SPELLCARD_ST3_MBOSS_1L,   SPELLCARD_ST3_BOSS_1L,    SPELLCARD_ST3_BOSS_2L,    SPELLCARD_ST3_BOSS_3L,
    SPELLCARD_ST3_BOSS_4L,    SPELLCARD_ST3_BOSS_LSL,
    // Stage 4A
    SPELLCARD_ST4A_MBOSS_1L,  SPELLCARD_ST4A_MBOSS_2L,  SPELLCARD_ST4A_BOSS_1L,   SPELLCARD_ST4A_BOSS_2L,
    SPELLCARD_ST4A_BOSS_3L,   SPELLCARD_ST4A_BOSS_LSL,
    // Stage 4B
    SPELLCARD_ST4B_MBOSS_1L,  SPELLCARD_ST4B_MBOSS_2L,  SPELLCARD_ST4B_BOSS_1L,   SPELLCARD_ST4B_BOSS_2L,
    SPELLCARD_ST4B_BOSS_3L,   SPELLCARD_ST4B_BOSS_LSL,
    // Stage 5
    SPELLCARD_ST5_BOSS_1L,    SPELLCARD_ST5_BOSS_2L,    SPELLCARD_ST5_BOSS_3L,    SPELLCARD_ST5_BOSS_4L,
    SPELLCARD_ST5_BOSS_LSL,
    // Stage 6A
    SPELLCARD_ST6A_MBOSS_1L,  SPELLCARD_ST6A_BOSS_1L,   SPELLCARD_ST6A_BOSS_2L,   SPELLCARD_ST6A_BOSS_3L,
    SPELLCARD_ST6A_BOSS_4L,   SPELLCARD_ST6A_BOSS_5L,   SPELLCARD_ST6A_BOSS_LSL,
    // Stage 6B
    SPELLCARD_ST6B_MBOSS_1L,  SPELLCARD_ST6B_BOSS_1L,   SPELLCARD_ST6B_BOSS_2L,   SPELLCARD_ST6B_BOSS_3L,
    SPELLCARD_ST6B_BOSS_4L,   SPELLCARD_ST6B_BOSS_5L,   SPELLCARD_ST6B_BOSS_LS1L, SPELLCARD_ST6B_BOSS_LS2L,
    SPELLCARD_ST6B_BOSS_LS3L, SPELLCARD_ST6B_BOSS_LS4L, SPELLCARD_ST6B_BOSS_LS5L,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 14, g_SpellcardNumbersExtra) = {
    SPELLCARD_EX_MBOSS_1, SPELLCARD_EX_MBOSS_2, SPELLCARD_EX_MBOSS_3,
    SPELLCARD_EX_BOSS_1,  SPELLCARD_EX_BOSS_2,  SPELLCARD_EX_BOSS_3,  SPELLCARD_EX_BOSS_4,
    SPELLCARD_EX_BOSS_5,  SPELLCARD_EX_BOSS_6,  SPELLCARD_EX_BOSS_7,  SPELLCARD_EX_BOSS_8,
    SPELLCARD_EX_BOSS_9,  SPELLCARD_EX_BOSS_10, SPELLCARD_EX_BOSS_LS,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, SPELLCARD_COUNT_SPELLCARDS, g_SpellcardNumbersAllDifficulties) = {
    // Stage 1
                                                        SPELLCARD_ST1_MBOSS_1H,   SPELLCARD_ST1_MBOSS_1L,
    SPELLCARD_ST1_BOSS_1E,    SPELLCARD_ST1_BOSS_1N,    SPELLCARD_ST1_BOSS_1H,    SPELLCARD_ST1_BOSS_1L,
    SPELLCARD_ST1_BOSS_2E,    SPELLCARD_ST1_BOSS_2N,    SPELLCARD_ST1_BOSS_2H,    SPELLCARD_ST1_BOSS_2L,
                              SPELLCARD_ST1_BOSS_LSN,   SPELLCARD_ST1_BOSS_LSH,   SPELLCARD_ST1_BOSS_LSL,
    // Stage 2
    SPELLCARD_ST2_MBOSS_1E,   SPELLCARD_ST2_MBOSS_1N,   SPELLCARD_ST2_MBOSS_1H,   SPELLCARD_ST2_MBOSS_1L,
    SPELLCARD_ST2_BOSS_1E,    SPELLCARD_ST2_BOSS_1N,    SPELLCARD_ST2_BOSS_1H,    SPELLCARD_ST2_BOSS_1L,
    SPELLCARD_ST2_BOSS_2E,    SPELLCARD_ST2_BOSS_2N,    SPELLCARD_ST2_BOSS_2H,    SPELLCARD_ST2_BOSS_2L,
    SPELLCARD_ST2_BOSS_3E,    SPELLCARD_ST2_BOSS_3N,    SPELLCARD_ST2_BOSS_3H,    SPELLCARD_ST2_BOSS_3L,
                              SPELLCARD_ST2_BOSS_LSN,   SPELLCARD_ST2_BOSS_LSH,   SPELLCARD_ST2_BOSS_LSL,
    // Stage 3
    SPELLCARD_ST3_MBOSS_1E,   SPELLCARD_ST3_MBOSS_1N,   SPELLCARD_ST3_MBOSS_1H,   SPELLCARD_ST3_MBOSS_1L,
                              SPELLCARD_ST3_BOSS_1N,    SPELLCARD_ST3_BOSS_1H,    SPELLCARD_ST3_BOSS_1L,
    SPELLCARD_ST3_BOSS_2E,    SPELLCARD_ST3_BOSS_2N,    SPELLCARD_ST3_BOSS_2H,    SPELLCARD_ST3_BOSS_2L,
    SPELLCARD_ST3_BOSS_3E,    SPELLCARD_ST3_BOSS_3N,    SPELLCARD_ST3_BOSS_3H,    SPELLCARD_ST3_BOSS_3L,
    SPELLCARD_ST3_BOSS_4E,    SPELLCARD_ST3_BOSS_4N,    SPELLCARD_ST3_BOSS_4H,    SPELLCARD_ST3_BOSS_4L,
                              SPELLCARD_ST3_BOSS_LSN,   SPELLCARD_ST3_BOSS_LSH,   SPELLCARD_ST3_BOSS_LSL,
    // Stage 4A
    SPELLCARD_ST4A_MBOSS_1E,  SPELLCARD_ST4A_MBOSS_1N,  SPELLCARD_ST4A_MBOSS_1H,  SPELLCARD_ST4A_MBOSS_1L,
    SPELLCARD_ST4A_MBOSS_2E,  SPELLCARD_ST4A_MBOSS_2N,  SPELLCARD_ST4A_MBOSS_2H,  SPELLCARD_ST4A_MBOSS_2L,
    SPELLCARD_ST4A_BOSS_1E,   SPELLCARD_ST4A_BOSS_1N,   SPELLCARD_ST4A_BOSS_1H,   SPELLCARD_ST4A_BOSS_1L,
    SPELLCARD_ST4A_BOSS_2E,   SPELLCARD_ST4A_BOSS_2N,   SPELLCARD_ST4A_BOSS_2H,   SPELLCARD_ST4A_BOSS_2L,
    SPELLCARD_ST4A_BOSS_3E,   SPELLCARD_ST4A_BOSS_3N,   SPELLCARD_ST4A_BOSS_3H,   SPELLCARD_ST4A_BOSS_3L,
                              SPELLCARD_ST4A_BOSS_LSN,  SPELLCARD_ST4A_BOSS_LSH,  SPELLCARD_ST4A_BOSS_LSL,
    // Stage 4B
    SPELLCARD_ST4B_MBOSS_1E,  SPELLCARD_ST4B_MBOSS_1N,  SPELLCARD_ST4B_MBOSS_1H,  SPELLCARD_ST4B_MBOSS_1L,
    SPELLCARD_ST4B_MBOSS_2E,  SPELLCARD_ST4B_MBOSS_2N,  SPELLCARD_ST4B_MBOSS_2H,  SPELLCARD_ST4B_MBOSS_2L,
    SPELLCARD_ST4B_BOSS_1E,   SPELLCARD_ST4B_BOSS_1N,   SPELLCARD_ST4B_BOSS_1H,   SPELLCARD_ST4B_BOSS_1L,
    SPELLCARD_ST4B_BOSS_2E,   SPELLCARD_ST4B_BOSS_2N,   SPELLCARD_ST4B_BOSS_2H,   SPELLCARD_ST4B_BOSS_2L,
    SPELLCARD_ST4B_BOSS_3E,   SPELLCARD_ST4B_BOSS_3N,   SPELLCARD_ST4B_BOSS_3H,   SPELLCARD_ST4B_BOSS_3L,
                              SPELLCARD_ST4B_BOSS_LSN,  SPELLCARD_ST4B_BOSS_LSH,  SPELLCARD_ST4B_BOSS_LSL,
    // Stage 5
    SPELLCARD_ST5_BOSS_1E,    SPELLCARD_ST5_BOSS_1N,    SPELLCARD_ST5_BOSS_1H,    SPELLCARD_ST5_BOSS_1L,
    SPELLCARD_ST5_BOSS_2E,    SPELLCARD_ST5_BOSS_2N,    SPELLCARD_ST5_BOSS_2H,    SPELLCARD_ST5_BOSS_2L,
    SPELLCARD_ST5_BOSS_3E,    SPELLCARD_ST5_BOSS_3N,    SPELLCARD_ST5_BOSS_3H,    SPELLCARD_ST5_BOSS_3L,
    SPELLCARD_ST5_BOSS_4E,    SPELLCARD_ST5_BOSS_4N,    SPELLCARD_ST5_BOSS_4H,    SPELLCARD_ST5_BOSS_4L,
                              SPELLCARD_ST5_BOSS_LSN,   SPELLCARD_ST5_BOSS_LSH,   SPELLCARD_ST5_BOSS_LSL,
    // Stage 6A
    SPELLCARD_ST6A_MBOSS_1E,  SPELLCARD_ST6A_MBOSS_1N,  SPELLCARD_ST6A_MBOSS_1H,  SPELLCARD_ST6A_MBOSS_1L,
    SPELLCARD_ST6A_BOSS_1E,   SPELLCARD_ST6A_BOSS_1N,   SPELLCARD_ST6A_BOSS_1H,   SPELLCARD_ST6A_BOSS_1L,
    SPELLCARD_ST6A_BOSS_2E,   SPELLCARD_ST6A_BOSS_2N,   SPELLCARD_ST6A_BOSS_2H,   SPELLCARD_ST6A_BOSS_2L,
    SPELLCARD_ST6A_BOSS_3E,   SPELLCARD_ST6A_BOSS_3N,   SPELLCARD_ST6A_BOSS_3H,   SPELLCARD_ST6A_BOSS_3L,
    SPELLCARD_ST6A_BOSS_4E,   SPELLCARD_ST6A_BOSS_4N,   SPELLCARD_ST6A_BOSS_4H,   SPELLCARD_ST6A_BOSS_4L,
    SPELLCARD_ST6A_BOSS_5E,   SPELLCARD_ST6A_BOSS_5N,   SPELLCARD_ST6A_BOSS_5H,   SPELLCARD_ST6A_BOSS_5L,
    SPELLCARD_ST6A_BOSS_LSE,  SPELLCARD_ST6A_BOSS_LSN,  SPELLCARD_ST6A_BOSS_LSH,  SPELLCARD_ST6A_BOSS_LSL,
    // Stage 6B
    SPELLCARD_ST6B_MBOSS_1E,  SPELLCARD_ST6B_MBOSS_1N,  SPELLCARD_ST6B_MBOSS_1H,  SPELLCARD_ST6B_MBOSS_1L,
    SPELLCARD_ST6B_BOSS_1E,   SPELLCARD_ST6B_BOSS_1N,   SPELLCARD_ST6B_BOSS_1H,   SPELLCARD_ST6B_BOSS_1L,
    SPELLCARD_ST6B_BOSS_2E,   SPELLCARD_ST6B_BOSS_2N,   SPELLCARD_ST6B_BOSS_2H,   SPELLCARD_ST6B_BOSS_2L,
    SPELLCARD_ST6B_BOSS_3E,   SPELLCARD_ST6B_BOSS_3N,   SPELLCARD_ST6B_BOSS_3H,   SPELLCARD_ST6B_BOSS_3L,
    SPELLCARD_ST6B_BOSS_4E,   SPELLCARD_ST6B_BOSS_4N,   SPELLCARD_ST6B_BOSS_4H,   SPELLCARD_ST6B_BOSS_4L,
    SPELLCARD_ST6B_BOSS_5E,   SPELLCARD_ST6B_BOSS_5N,   SPELLCARD_ST6B_BOSS_5H,   SPELLCARD_ST6B_BOSS_5L,
    SPELLCARD_ST6B_BOSS_LS1E, SPELLCARD_ST6B_BOSS_LS1N, SPELLCARD_ST6B_BOSS_LS1H, SPELLCARD_ST6B_BOSS_LS1L,
    SPELLCARD_ST6B_BOSS_LS2E, SPELLCARD_ST6B_BOSS_LS2N, SPELLCARD_ST6B_BOSS_LS2H, SPELLCARD_ST6B_BOSS_LS2L,
    SPELLCARD_ST6B_BOSS_LS3E, SPELLCARD_ST6B_BOSS_LS3N, SPELLCARD_ST6B_BOSS_LS3H, SPELLCARD_ST6B_BOSS_LS3L,
    SPELLCARD_ST6B_BOSS_LS4E, SPELLCARD_ST6B_BOSS_LS4N, SPELLCARD_ST6B_BOSS_LS4H, SPELLCARD_ST6B_BOSS_LS4L,
    SPELLCARD_ST6B_BOSS_LS5E, SPELLCARD_ST6B_BOSS_LS5N, SPELLCARD_ST6B_BOSS_LS5H, SPELLCARD_ST6B_BOSS_LS5L,
    // Extra Stage
    SPELLCARD_EX_MBOSS_1,     SPELLCARD_EX_MBOSS_2,     SPELLCARD_EX_MBOSS_3,
    SPELLCARD_EX_BOSS_1,      SPELLCARD_EX_BOSS_2,      SPELLCARD_EX_BOSS_3,      SPELLCARD_EX_BOSS_4,
    SPELLCARD_EX_BOSS_5,      SPELLCARD_EX_BOSS_6,      SPELLCARD_EX_BOSS_7,      SPELLCARD_EX_BOSS_8,
    SPELLCARD_EX_BOSS_9,      SPELLCARD_EX_BOSS_10,     SPELLCARD_EX_BOSS_LS,
    // Last Word
    SPELLCARD_LW_WRIGGLE,     SPELLCARD_LW_MYSTIA,      SPELLCARD_LW_KEINE,       SPELLCARD_LW_REISEN,
    SPELLCARD_LW_EIRIN,       SPELLCARD_LW_KAGUYA,      SPELLCARD_LW_MOKOU,       SPELLCARD_LW_TEWI,
    SPELLCARD_LW_KEINEEX,     SPELLCARD_LW_REIMU,       SPELLCARD_LW_MARISA,      SPELLCARD_LW_SAKUYA,
    SPELLCARD_LW_YOUMU,       SPELLCARD_LW_ALICE,       SPELLCARD_LW_REMILIA,     SPELLCARD_LW_YUYUKO,
    SPELLCARD_LW_YUKARI,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32 *, 6, g_SpellcardNumbersPerDifficulty) = {
    g_SpellcardNumbersEasy,    g_SpellcardNumbersNormal, g_SpellcardNumbersHard,
    g_SpellcardNumbersLunatic, g_SpellcardNumbersExtra,  g_SpellcardNumbersAllDifficulties};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 6, g_SpellcardCountsPerDifficulty) = {
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersEasy),  ARRAY_SIZE_SIGNED(g_SpellcardNumbersNormal),
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersHard),  ARRAY_SIZE_SIGNED(g_SpellcardNumbersLunatic),
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersExtra), ARRAY_SIZE_SIGNED(g_SpellcardNumbersAllDifficulties)};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 13, g_SpellcardNumbersStage1) = {
                                                   SPELLCARD_ST1_MBOSS_1H, SPELLCARD_ST1_MBOSS_1L,
    SPELLCARD_ST1_BOSS_1E, SPELLCARD_ST1_BOSS_1N,  SPELLCARD_ST1_BOSS_1H,  SPELLCARD_ST1_BOSS_1L,
    SPELLCARD_ST1_BOSS_2E, SPELLCARD_ST1_BOSS_2N,  SPELLCARD_ST1_BOSS_2H,  SPELLCARD_ST1_BOSS_2L,
                           SPELLCARD_ST1_BOSS_LSN, SPELLCARD_ST1_BOSS_LSH, SPELLCARD_ST1_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 19, g_SpellcardNumbersStage2) = {
    SPELLCARD_ST2_MBOSS_1E, SPELLCARD_ST2_MBOSS_1N, SPELLCARD_ST2_MBOSS_1H, SPELLCARD_ST2_MBOSS_1L,
    SPELLCARD_ST2_BOSS_1E,  SPELLCARD_ST2_BOSS_1N,  SPELLCARD_ST2_BOSS_1H,  SPELLCARD_ST2_BOSS_1L,
    SPELLCARD_ST2_BOSS_2E,  SPELLCARD_ST2_BOSS_2N,  SPELLCARD_ST2_BOSS_2H,  SPELLCARD_ST2_BOSS_2L,
    SPELLCARD_ST2_BOSS_3E,  SPELLCARD_ST2_BOSS_3N,  SPELLCARD_ST2_BOSS_3H,  SPELLCARD_ST2_BOSS_3L,
                            SPELLCARD_ST2_BOSS_LSN, SPELLCARD_ST2_BOSS_LSH, SPELLCARD_ST2_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 22, g_SpellcardNumbersStage3) = {
    SPELLCARD_ST3_MBOSS_1E, SPELLCARD_ST3_MBOSS_1N, SPELLCARD_ST3_MBOSS_1H, SPELLCARD_ST3_MBOSS_1L,
                            SPELLCARD_ST3_BOSS_1N,  SPELLCARD_ST3_BOSS_1H,  SPELLCARD_ST3_BOSS_1L,
    SPELLCARD_ST3_BOSS_2E,  SPELLCARD_ST3_BOSS_2N,  SPELLCARD_ST3_BOSS_2H,  SPELLCARD_ST3_BOSS_2L,
    SPELLCARD_ST3_BOSS_3E,  SPELLCARD_ST3_BOSS_3N,  SPELLCARD_ST3_BOSS_3H,  SPELLCARD_ST3_BOSS_3L,
    SPELLCARD_ST3_BOSS_4E,  SPELLCARD_ST3_BOSS_4N,  SPELLCARD_ST3_BOSS_4H,  SPELLCARD_ST3_BOSS_4L,
                            SPELLCARD_ST3_BOSS_LSN, SPELLCARD_ST3_BOSS_LSH, SPELLCARD_ST3_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 23, g_SpellcardNumbersStage4A) = {
    SPELLCARD_ST4A_MBOSS_1E, SPELLCARD_ST4A_MBOSS_1N, SPELLCARD_ST4A_MBOSS_1H, SPELLCARD_ST4A_MBOSS_1L,
    SPELLCARD_ST4A_MBOSS_2E, SPELLCARD_ST4A_MBOSS_2N, SPELLCARD_ST4A_MBOSS_2H, SPELLCARD_ST4A_MBOSS_2L,
    SPELLCARD_ST4A_BOSS_1E,  SPELLCARD_ST4A_BOSS_1N,  SPELLCARD_ST4A_BOSS_1H,  SPELLCARD_ST4A_BOSS_1L,
    SPELLCARD_ST4A_BOSS_2E,  SPELLCARD_ST4A_BOSS_2N,  SPELLCARD_ST4A_BOSS_2H,  SPELLCARD_ST4A_BOSS_2L,
    SPELLCARD_ST4A_BOSS_3E,  SPELLCARD_ST4A_BOSS_3N,  SPELLCARD_ST4A_BOSS_3H,  SPELLCARD_ST4A_BOSS_3L,
                             SPELLCARD_ST4A_BOSS_LSN, SPELLCARD_ST4A_BOSS_LSH, SPELLCARD_ST4A_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 23, g_SpellcardNumbersStage4B) = {
    SPELLCARD_ST4B_MBOSS_1E, SPELLCARD_ST4B_MBOSS_1N, SPELLCARD_ST4B_MBOSS_1H, SPELLCARD_ST4B_MBOSS_1L,
    SPELLCARD_ST4B_MBOSS_2E, SPELLCARD_ST4B_MBOSS_2N, SPELLCARD_ST4B_MBOSS_2H, SPELLCARD_ST4B_MBOSS_2L,
    SPELLCARD_ST4B_BOSS_1E,  SPELLCARD_ST4B_BOSS_1N,  SPELLCARD_ST4B_BOSS_1H,  SPELLCARD_ST4B_BOSS_1L,
    SPELLCARD_ST4B_BOSS_2E,  SPELLCARD_ST4B_BOSS_2N,  SPELLCARD_ST4B_BOSS_2H,  SPELLCARD_ST4B_BOSS_2L,
    SPELLCARD_ST4B_BOSS_3E,  SPELLCARD_ST4B_BOSS_3N,  SPELLCARD_ST4B_BOSS_3H,  SPELLCARD_ST4B_BOSS_3L,
                             SPELLCARD_ST4B_BOSS_LSN, SPELLCARD_ST4B_BOSS_LSH, SPELLCARD_ST4B_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 19, g_SpellcardNumbersStage5) = {
    SPELLCARD_ST5_BOSS_1E, SPELLCARD_ST5_BOSS_1N,  SPELLCARD_ST5_BOSS_1H,  SPELLCARD_ST5_BOSS_1L,
    SPELLCARD_ST5_BOSS_2E, SPELLCARD_ST5_BOSS_2N,  SPELLCARD_ST5_BOSS_2H,  SPELLCARD_ST5_BOSS_2L,
    SPELLCARD_ST5_BOSS_3E, SPELLCARD_ST5_BOSS_3N,  SPELLCARD_ST5_BOSS_3H,  SPELLCARD_ST5_BOSS_3L,
    SPELLCARD_ST5_BOSS_4E, SPELLCARD_ST5_BOSS_4N,  SPELLCARD_ST5_BOSS_4H,  SPELLCARD_ST5_BOSS_4L,
                           SPELLCARD_ST5_BOSS_LSN, SPELLCARD_ST5_BOSS_LSH, SPELLCARD_ST5_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 28, g_SpellcardNumbersStage6A) = {
    SPELLCARD_ST6A_MBOSS_1E, SPELLCARD_ST6A_MBOSS_1N, SPELLCARD_ST6A_MBOSS_1H, SPELLCARD_ST6A_MBOSS_1L,
    SPELLCARD_ST6A_BOSS_1E,  SPELLCARD_ST6A_BOSS_1N,  SPELLCARD_ST6A_BOSS_1H,  SPELLCARD_ST6A_BOSS_1L,
    SPELLCARD_ST6A_BOSS_2E,  SPELLCARD_ST6A_BOSS_2N,  SPELLCARD_ST6A_BOSS_2H,  SPELLCARD_ST6A_BOSS_2L,
    SPELLCARD_ST6A_BOSS_3E,  SPELLCARD_ST6A_BOSS_3N,  SPELLCARD_ST6A_BOSS_3H,  SPELLCARD_ST6A_BOSS_3L,
    SPELLCARD_ST6A_BOSS_4E,  SPELLCARD_ST6A_BOSS_4N,  SPELLCARD_ST6A_BOSS_4H,  SPELLCARD_ST6A_BOSS_4L,
    SPELLCARD_ST6A_BOSS_5E,  SPELLCARD_ST6A_BOSS_5N,  SPELLCARD_ST6A_BOSS_5H,  SPELLCARD_ST6A_BOSS_5L,
    SPELLCARD_ST6A_BOSS_LSE, SPELLCARD_ST6A_BOSS_LSN, SPELLCARD_ST6A_BOSS_LSH, SPELLCARD_ST6A_BOSS_LSL,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 44, g_SpellcardNumbersStage6B) = {
    SPELLCARD_ST6B_MBOSS_1E,  SPELLCARD_ST6B_MBOSS_1N,  SPELLCARD_ST6B_MBOSS_1H,  SPELLCARD_ST6B_MBOSS_1L,
    SPELLCARD_ST6B_BOSS_1E,   SPELLCARD_ST6B_BOSS_1N,   SPELLCARD_ST6B_BOSS_1H,   SPELLCARD_ST6B_BOSS_1L,
    SPELLCARD_ST6B_BOSS_2E,   SPELLCARD_ST6B_BOSS_2N,   SPELLCARD_ST6B_BOSS_2H,   SPELLCARD_ST6B_BOSS_2L,
    SPELLCARD_ST6B_BOSS_3E,   SPELLCARD_ST6B_BOSS_3N,   SPELLCARD_ST6B_BOSS_3H,   SPELLCARD_ST6B_BOSS_3L,
    SPELLCARD_ST6B_BOSS_4E,   SPELLCARD_ST6B_BOSS_4N,   SPELLCARD_ST6B_BOSS_4H,   SPELLCARD_ST6B_BOSS_4L,
    SPELLCARD_ST6B_BOSS_5E,   SPELLCARD_ST6B_BOSS_5N,   SPELLCARD_ST6B_BOSS_5H,   SPELLCARD_ST6B_BOSS_5L,
    SPELLCARD_ST6B_BOSS_LS1E, SPELLCARD_ST6B_BOSS_LS1N, SPELLCARD_ST6B_BOSS_LS1H, SPELLCARD_ST6B_BOSS_LS1L,
    SPELLCARD_ST6B_BOSS_LS2E, SPELLCARD_ST6B_BOSS_LS2N, SPELLCARD_ST6B_BOSS_LS2H, SPELLCARD_ST6B_BOSS_LS2L,
    SPELLCARD_ST6B_BOSS_LS3E, SPELLCARD_ST6B_BOSS_LS3N, SPELLCARD_ST6B_BOSS_LS3H, SPELLCARD_ST6B_BOSS_LS3L,
    SPELLCARD_ST6B_BOSS_LS4E, SPELLCARD_ST6B_BOSS_LS4N, SPELLCARD_ST6B_BOSS_LS4H, SPELLCARD_ST6B_BOSS_LS4L,
    SPELLCARD_ST6B_BOSS_LS5E, SPELLCARD_ST6B_BOSS_LS5N, SPELLCARD_ST6B_BOSS_LS5H, SPELLCARD_ST6B_BOSS_LS5L,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 14, g_SpellcardNumbersStageExtra) = {
    SPELLCARD_EX_MBOSS_1, SPELLCARD_EX_MBOSS_2, SPELLCARD_EX_MBOSS_3,
    SPELLCARD_EX_BOSS_1,  SPELLCARD_EX_BOSS_2,  SPELLCARD_EX_BOSS_3,  SPELLCARD_EX_BOSS_4,
    SPELLCARD_EX_BOSS_5,  SPELLCARD_EX_BOSS_6,  SPELLCARD_EX_BOSS_7,  SPELLCARD_EX_BOSS_8,
    SPELLCARD_EX_BOSS_9,  SPELLCARD_EX_BOSS_10, SPELLCARD_EX_BOSS_LS,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 17, g_SpellcardNumbersLastWord) = {
    SPELLCARD_LW_WRIGGLE,
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
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 43, g_LastSpellNumbers) = {
    // Wriggle Nightbug
    SPELLCARD_ST1_BOSS_LSN,   SPELLCARD_ST1_BOSS_LSH,   SPELLCARD_ST1_BOSS_LSL,
    // Mystia Lorelei
    SPELLCARD_ST2_BOSS_LSN,   SPELLCARD_ST2_BOSS_LSH,   SPELLCARD_ST2_BOSS_LSL,
    // Keine Kamishirasawa
    SPELLCARD_ST3_BOSS_LSN,   SPELLCARD_ST3_BOSS_LSH,   SPELLCARD_ST3_BOSS_LSL,
    // Reimu Hakurei
    SPELLCARD_ST4A_BOSS_LSN,  SPELLCARD_ST4A_BOSS_LSH,  SPELLCARD_ST4A_BOSS_LSL,
    // Marisa Kirisame
    SPELLCARD_ST4B_BOSS_LSN,  SPELLCARD_ST4B_BOSS_LSH,  SPELLCARD_ST4B_BOSS_LSL,
    // Reisen U. Inaba
    SPELLCARD_ST5_BOSS_LSN,   SPELLCARD_ST5_BOSS_LSH,   SPELLCARD_ST5_BOSS_LSL,
    // Eirin Yagokoro
    SPELLCARD_ST6A_BOSS_LSE,  SPELLCARD_ST6A_BOSS_LSN,  SPELLCARD_ST6A_BOSS_LSH,  SPELLCARD_ST6A_BOSS_LSL,

    // Kaguya Houraisan, Spell 1
    SPELLCARD_ST6B_BOSS_LS1E, SPELLCARD_ST6B_BOSS_LS1N, SPELLCARD_ST6B_BOSS_LS1H, SPELLCARD_ST6B_BOSS_LS1L,
    // Kaguya Houraisan, Spell 2
    SPELLCARD_ST6B_BOSS_LS2E, SPELLCARD_ST6B_BOSS_LS2N, SPELLCARD_ST6B_BOSS_LS2H, SPELLCARD_ST6B_BOSS_LS2L,
    // Kaguya Houraisan, Spell 3
    SPELLCARD_ST6B_BOSS_LS3E, SPELLCARD_ST6B_BOSS_LS3N, SPELLCARD_ST6B_BOSS_LS3H, SPELLCARD_ST6B_BOSS_LS3L,
    // Kaguya Houraisan, Spell 4
    SPELLCARD_ST6B_BOSS_LS4E, SPELLCARD_ST6B_BOSS_LS4N, SPELLCARD_ST6B_BOSS_LS4H, SPELLCARD_ST6B_BOSS_LS4L,
    // Kaguya Houraisan, Spell 5
    SPELLCARD_ST6B_BOSS_LS5E, SPELLCARD_ST6B_BOSS_LS5N, SPELLCARD_ST6B_BOSS_LS5H, SPELLCARD_ST6B_BOSS_LS5L,

    // Fujiwara no Mokou
    SPELLCARD_EX_BOSS_LS,
};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32 *, 10, g_SpellcardNumbersPerStage) = {
    g_SpellcardNumbersStage1,     g_SpellcardNumbersStage2,  g_SpellcardNumbersStage3,  g_SpellcardNumbersStage4A,
    g_SpellcardNumbersStage4B,    g_SpellcardNumbersStage5,  g_SpellcardNumbersStage6A, g_SpellcardNumbersStage6B,
    g_SpellcardNumbersStageExtra, g_SpellcardNumbersLastWord};

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 10, g_SpellcardCountPerStage) = {
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage1),  ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage2),
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage3),  ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage4A),
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage4B), ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage5),
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage6A), ARRAY_SIZE_SIGNED(g_SpellcardNumbersStage6B),
    ARRAY_SIZE_SIGNED(g_SpellcardNumbersExtra),   ARRAY_SIZE_SIGNED(g_SpellcardNumbersLastWord)};

// clang-format on
// TODO: stop clang-format from fucking with whitespace formatting


// FUNCTION: th08 0x00405260
i32 Spellcard::WasCaptured()
{
    return (this->flags >> SPELLCARD_FLAG_CAPTURED_SHIFT) & 1;
}


// FUNCTION: th08 0x4143e0
Spellcard::Spellcard()
{
}


DIFFABLE_EXTERN(i32, g_GuiFullPowerModeFrames);

// FUNCTION: th08 0x4144d0
#pragma var_order(difficulty, i)
i32 Spellcard::GetDifficultyFromSpellCard(i32 spellCardNumber)
{
    i32 difficulty;
    i32 i;
    for (difficulty = 0; difficulty < MAX_DIFFICULTIES; difficulty++)
    {
        for (i = 0; i < g_SpellcardCountsPerDifficulty[difficulty]; i++)
        {
            if (g_SpellcardNumbersPerDifficulty[difficulty][i] == spellCardNumber)
            {
                return difficulty;
            }
        }
    }
    return MAX_DIFFICULTIES;
}

// FUNCTION: th08 0x00414540
i32 __fastcall Spellcard::IsLastSpell(i32 spellCardNumber)
{
    for (i32 i = 0; i < g_LastSpellCount; ++i)
    {
        if (g_LastSpellNumbers[i] == spellCardNumber)
            return TRUE;
    }
    return FALSE;
}

// FUNCTION: th08 0x414590
ZunResult Spellcard::Init()
{
    if (IsResourceReloadEnabled())
    {
        memset(this, 0, sizeof(Spellcard));
        this->commonFaceAnm = g_AnmManager->PreloadAnm(15, "face_cdbg.anm");
        if (this->commonFaceAnm == NULL)
            return ZUN_ERROR;

        if (!g_GameManager.IsSpellPractice())
        {
            switch (g_GameManager.shotType)
            {
            default:
                this->playerFaceAnm0 = g_AnmManager->PreloadAnm(16, "face_rm00.anm");
                if (this->playerFaceAnm0 == NULL)
                    return ZUN_ERROR;
                this->playerFaceAnm1 = g_AnmManager->PreloadAnm(17, "face_yk00.anm");
                if (this->playerFaceAnm1 == NULL)
                    return ZUN_ERROR;
                break;

            case SHOT_MARISA_ALICE:
            case SHOT_MARISA:
            case SHOT_ALICE:
                this->playerFaceAnm0 = g_AnmManager->PreloadAnm(16, "face_mr00.anm");
                if (this->playerFaceAnm0 == NULL)
                    return ZUN_ERROR;
                this->playerFaceAnm1 = g_AnmManager->PreloadAnm(17, "face_al00.anm");
                if (this->playerFaceAnm1 == NULL)
                    return ZUN_ERROR;
                break;

            case SHOT_SAKUYA_REMILIA:
            case SHOT_SAKUYA:
            case SHOT_REMILIA:
                this->playerFaceAnm0 = g_AnmManager->PreloadAnm(16, "face_sk00.anm");
                if (this->playerFaceAnm0 == NULL)
                    return ZUN_ERROR;
                this->playerFaceAnm1 = g_AnmManager->PreloadAnm(17, "face_rs00.anm");
                if (this->playerFaceAnm1 == NULL)
                    return ZUN_ERROR;
                break;

            case SHOT_YOUMU_YUYUKO:
            case SHOT_YOUMU:
            case SHOT_YUYUKO:
                this->playerFaceAnm0 = g_AnmManager->PreloadAnm(16, "face_ym00.anm");
                if (this->playerFaceAnm0 == NULL)
                    return ZUN_ERROR;
                this->playerFaceAnm1 = g_AnmManager->PreloadAnm(17, "face_yy00.anm");
                if (this->playerFaceAnm1 == NULL)
                    return ZUN_ERROR;
                break;
            }
        }
    }
    else
    {
        memset(this, 0, sizeof(Spellcard));
        this->commonFaceAnm = g_AnmManager->GetAnm(15);
        if (!g_GameManager.IsSpellPractice())
        {
            this->playerFaceAnm0 = g_AnmManager->GetAnm(16);
            this->playerFaceAnm1 = g_AnmManager->GetAnm(17);
        }
    }

    if (!IsDisableResourceReload())
    {
        if (!g_GameManager.IsSpellPractice())
        {
            switch (g_GameManager.currentStage)
            {
            case STAGE1:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st01.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE2:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st02.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE3:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st03.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE4A:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st04a.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE4B:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st04b.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE5:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st05.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                this->enemyFaceAnm1 = g_AnmManager->PreloadAnm(19, "face_st05b.anm");
                if (this->enemyFaceAnm1 == NULL) return ZUN_ERROR;
                break;
            case STAGE6A:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st06.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE6B:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st06.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                this->enemyFaceAnm1 = g_AnmManager->PreloadAnm(19, "face_st07.anm");
                if (this->enemyFaceAnm1 == NULL) return ZUN_ERROR;
                break;
            case EXTRASTAGE:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st08m.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                this->enemyFaceAnm1 = g_AnmManager->PreloadAnm(19, "face_st08.anm");
                if (this->enemyFaceAnm1 == NULL) return ZUN_ERROR;
                break;
            default:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st03.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            }
        }
        else
        {
            switch (g_GameManager.currentStage)
            {
            case STAGE1:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st01sp.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE2:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st02sp.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE3:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st03sp.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE4A:
                if (!g_GameManager.IsSpellPractice() || g_GameManager.IsSpellNumberEqualTo(214))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st04asp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberEqualTo(216))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_sksp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberEqualTo(217))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_ymsp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberEqualTo(218))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_alsp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberEqualTo(219))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_rssp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberEqualTo(220))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_yysp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberEqualTo(221))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_yksp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st04asp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                break;
            case STAGE4B:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st04bsp.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE5:
                if (g_GameManager.IsSpellNumberEqualTo(212))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st05msp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st05sp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                break;
            case STAGE6A:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st06sp.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            case STAGE6B:
                if (g_GameManager.IsSpellNumberInRange(147, 150))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st06sp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st07sp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                break;
            case EXTRASTAGE:
                if (g_GameManager.IsSpellNumberInRange(191, 193) || g_GameManager.IsSpellNumberEqualTo(213))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st08msp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                else if (g_GameManager.IsSpellNumberInRange(194, 204) || g_GameManager.IsSpellNumberEqualTo(211))
                {
                    this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st08sp.anm");
                    if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                }
                break;
            default:
                this->enemyFaceAnm0 = g_AnmManager->PreloadAnm(18, "face_st03.anm");
                if (this->enemyFaceAnm0 == NULL) return ZUN_ERROR;
                break;
            }
        }
    }
    else
    {
        this->enemyFaceAnm0 = g_AnmManager->GetAnm(18);
        this->enemyFaceAnm1 = g_AnmManager->GetAnm(19);
    }

    if (this->playerFaceAnm0 != NULL)
    {
        this->playerFaceAnm0->SetAndExecuteScriptIdx(&this->playerPortraitVm, 0);
        this->playerFaceAnm0->SetAndExecuteScriptIdx(&this->enemyPortraitVm, 0);
    }

    g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->playerSpellNameVm, 4);
    g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->enemySpellNameVm, 5);
    g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->playerSpellNameFrameVm, 1);
    g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->enemySpellNameFrameVm, 0);
    g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->spellBonusFrameVm, 2);
    g_AsciiManager.asciiAnm->SetAndExecuteScriptIdx(&this->spellBonusDigitsVm, 4);

    this->playerPortraitVm.currentInstruction = NULL;
    this->portraitBackdropVm.currentInstruction = NULL;
    this->portraitOverlayVm.currentInstruction = NULL;
    this->playerSpellNameVm.currentInstruction = NULL;
    this->enemyPortraitVm.currentInstruction = NULL;
    this->vm90C.currentInstruction = NULL;
    this->vmE54.currentInstruction = NULL;
    this->enemySpellNameVm.currentInstruction = NULL;

    this->playerPortraitVm.flagsWord &= ~1u;
    this->portraitBackdropVm.flagsWord &= ~1u;
    this->portraitOverlayVm.flagsWord &= ~1u;
    this->playerSpellNameVm.flagsWord &= ~1u;
    this->enemyPortraitVm.flagsWord &= ~1u;
    this->vm90C.flagsWord &= ~1u;
    this->vmE54.flagsWord &= ~1u;
    this->enemySpellNameVm.flagsWord &= ~1u;

    this->playerSpellNameVm.fontWidth = 15;
    this->playerSpellNameVm.fontHeight = 15;
    this->enemySpellNameVm.fontWidth = 15;
    this->enemySpellNameVm.fontHeight = 15;
    this->pendingTimeOrbs = 0;
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4152a0
#pragma var_order(i, catk, j, checksum, nameChecksum, ownerName, this)
void Spellcard::StartSpell(i32 spellCardNumber, const u8 *encodedName, i32 enemyFace, i32 bonus, u8 *enemy,
                           const u8 *encodedOwner, const char *commentLine1, const char *commentLine2)
{
    char ownerName[128];
    i32 nameChecksum;
    i32 checksum;
    i32 j;
    Catk *catk;
    u32 i;

    this->flags &= ~SPELLCARD_FLAG_CAPTURED;
    this->flags |= SPELLCARD_FLAG_ACTIVE;
    this->flags |= SPELLCARD_FLAG_CAPTURE_VALID;
    this->flags &= ~0x10;
    this->flags &= ~SPELLCARD_FLAG_TIMER_CALLBACK_TRANSITION;
    this->flags &= ~SPELLCARD_FLAG_EFFECT_TRACKING_DISABLED;
    this->flags &= ~SPELLCARD_FLAG_BOMB_DAMAGE_ENABLED;
    this->flags &= ~SPELLCARD_FLAG_SUPPRESS_BONUS_PRESENTATION;
    this->spellCardNumber = spellCardNumber;
    this->activeEnemy = enemy;
    this->activeEnemyIndexSnapshot = reinterpret_cast<Enemy *>(this->activeEnemy)->enemyIndex;
    this->bonusProgress = bonus;
    this->scoreLimit = bonus;
    if (((reinterpret_cast<Enemy *>(this->activeEnemy)->flags1 >>
          ENEMY_FLAG_TIMEOUT_SPELL_SHIFT) & 1) != 0)
    {
        this->scoreLimit = 99999990;
    }
    this->bonusCounter =
        (this->bonusProgress - this->bonusProgress / 7u) /
        (reinterpret_cast<Enemy *>(this->activeEnemy)->timerCallbackThresholdFrames / 60);
    this->timeRemaining = reinterpret_cast<Enemy *>(this->activeEnemy)->timerCallbackThresholdFrames;
    this->timeLimit = reinterpret_cast<Enemy *>(this->activeEnemy)->timerCallbackThresholdFrames;

    for (i = 0; i < 0x30; i++)
    {
        this->spellName[i] = encodedName[i] ^ 0xAA;
    }
    this->CutInEnemy(enemyFace, this->spellName, 0);
    g_BulletManager.ClearBulletsForTransition();
    g_Background.StartSpellBackground();
    for (i = 0; (i32)i < g_Background.spellVmCount; i++)
    {
        g_SpellcardBackgroundAnm->SetAndExecuteScriptIdx(&g_Background.spellVms[i],
                                                         i + g_Background.spellVmScriptBase);
    }
    g_Background.spellBackgroundDrawCallback = NULL;
    reinterpret_cast<Enemy *>(this->activeEnemy)->ResetBulletRankInfluence();
    this->mixColor = 0x80808080;

    if (((this->flags >> SPELLCARD_FLAG_CAPTURE_REWARD_PENDING_SHIFT) & 1) != 0)
    {
        this->rewardEffect = NULL;
        g_Gui.ShowSpellcardBonus(this->bonusAward);
        g_GameManager.AddScore(this->bonusAward);
        this->flags &= ~SPELLCARD_FLAG_CAPTURE_REWARD_PENDING;
        if (this->pendingTimeOrbs > 0)
        {
            g_GameManager.AddTimeOrbs(this->pendingTimeOrbs);
            this->pendingTimeOrbs = 0;
        }
    }

    this->flags &= ~SPELLCARD_FLAG_BONUS_UPDATES_DISABLED;
    this->spellEffect = reinterpret_cast<Effect *>(g_EffectManager.SpawnEffectInFixedSlot(
        (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0) ? 52 : 39,
        reinterpret_cast<D3DXVECTOR3 *>(
            &reinterpret_cast<Enemy *>(this->activeEnemy)->position), 1, 1, -1));
    this->spellEffect->vm.interpCurrentTimers[AnmInterp_Pos] = 0;
    this->spellEffect->vm.interpEndTimers[AnmInterp_Pos] = 100;
    this->spellEffect->vm.interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOutQuartic;
    this->spellEffect->vm.posInitial.x = 8.0f;
    this->spellEffect->vm.posFinal.x = 256.0f;
    this->spellEffect->vm.posInitial.y = 64.0f;
    this->spellEffect->vm.posFinal.y = 0.0f;
    this->spellEffect->vm.pos.y = 64.0f;
    this->spellEffect->position = reinterpret_cast<Enemy *>(this->activeEnemy)->position;
    this->spellEffect->vertexSegmentCount = 64;
    this->spellEffect->angle = 0.0f;
    this->spellEffect->radius = 256.0f;
    this->spellEffect->shapeThickness = 15.0f;
    this->spellEffect->radialWaveCount = 6.0f;

    reinterpret_cast<SpellcardFlagBits *>(&this->flags)->alternateEffectStyle =
        (*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3;
    g_Gui.flags.bombDisplayUpdateFrames = 3;
    g_Gui.flags.lifeDisplayUpdateFrames = 3;
    this->flags &= ~SPELLCARD_FLAG_EFFECT_TRACKING_DISABLED;

    if (!g_GameManager.IsReplay())
    {
        catk = &g_GameManager.catkData[this->spellCardNumber];
        checksum = 0;
        strcpy(catk->spellName, this->spellName);

        memset(ownerName, 0, sizeof(ownerName));
        for (j = 0; (u32)j < 0x30; j++)
        {
            ownerName[j] = encodedOwner[j] ^ 0xBB;
        }
        strcpy(catk->spellOwnerName, ownerName);

        if (g_GameManager.flags.isSpellPractice)
        {
            memcpy(this->spellCommentLine1, commentLine1, sizeof(this->spellCommentLine1));
            memcpy(this->spellCommentLine2, commentLine2, sizeof(this->spellCommentLine2));
        }

        for (j = strlen(catk->spellName); j > 0;)
        {
            --j;
            checksum += catk->spellName[j];
        }
        nameChecksum = checksum;

        for (j = 0; j < SHOT_ALL + 1; j++)
        {
            checksum += catk->inGameHistory.captures[j];
            checksum += catk->inGameHistory.attempts[j];
            checksum += catk->inGameHistory.maxBonus[j];
            checksum += catk->spellPracticeHistory.captures[j];
            checksum += catk->spellPracticeHistory.attempts[j];
            checksum += catk->spellPracticeHistory.maxBonus[j];
        }

        if (catk->historyChecksum != (u8)checksum)
        {
            for (j = 0; j < SHOT_ALL + 1; j++)
            {
                catk->inGameHistory.captures[j] = 0;
                catk->inGameHistory.attempts[j] = 0;
                catk->inGameHistory.maxBonus[j] = 0;
                catk->spellPracticeHistory.captures[j] = 0;
                catk->spellPracticeHistory.attempts[j] = 0;
                catk->spellPracticeHistory.maxBonus[j] = 0;
            }
        }

        if (!g_GameManager.flags.isSpellPractice)
        {
            if (catk->inGameHistory.attempts[g_GameManager.shotType] < 9999)
            {
                catk->inGameHistory.attempts[g_GameManager.shotType]++;
            }
            if (catk->inGameHistory.attempts[SHOT_ALL] < 9999)
            {
                catk->inGameHistory.attempts[SHOT_ALL]++;
            }
        }
        else
        {
            if (catk->spellPracticeHistory.attempts[g_GameManager.shotType] < 9999)
            {
                catk->spellPracticeHistory.attempts[g_GameManager.shotType]++;
            }
            if (catk->spellPracticeHistory.attempts[SHOT_ALL] < 9999)
            {
                catk->spellPracticeHistory.attempts[SHOT_ALL]++;
            }
        }

        for (j = 0; j < SHOT_ALL + 1; j++)
        {
            nameChecksum += catk->inGameHistory.captures[j];
            nameChecksum += catk->inGameHistory.attempts[j];
            nameChecksum += catk->inGameHistory.maxBonus[j];
            nameChecksum += catk->spellPracticeHistory.captures[j];
            nameChecksum += catk->spellPracticeHistory.attempts[j];
            nameChecksum += catk->spellPracticeHistory.maxBonus[j];
        }
        catk->difficulty = (u8)g_GameManager.difficulty;
        catk->historyChecksum = (u8)nameChecksum;
    }
}

// FUNCTION: th08 0x415c60
void BulletManager::ClearBulletsForTransition()
{
    this->RemoveAllBullets(1);
}

// FUNCTION: th08 0x415c80
void Enemy::ResetBulletRankInfluence()
{
    this->bulletRankInfluence.speedLow = -0.5f;
    this->bulletRankInfluence.speedHigh = 0.5f;
    this->bulletRankInfluence.count1Low = 0;
    this->bulletRankInfluence.count1High = 0;
    this->bulletRankInfluence.count2Low = 0;
    this->bulletRankInfluence.count2High = 0;
}

// FUNCTION: th08 0x415ce0
void Background::StartSpellBackground()
{
    this->spellBackgroundState = 1;
    this->spellBackgroundTimer = 0;
}

// FUNCTION: th08 0x415d10
void Spellcard::CutInEnemyNoPortrait(const char *name, i32 unused)
{
    this->flags |= SPELLCARD_FLAG_SUPPRESS_BONUS_PRESENTATION;
    this->flags &= ~SPELLCARD_FLAG_ACTIVE;
    this->flags &= ~0x10;
    this->CutInEnemy(-1, name, 1);
}

// FUNCTION: th08 0x415d60
void Spellcard::CutInPlayer(i32 playerFace, const char *name, i32 sprite)
{
    if (playerFace == 0)
    {
        this->playerFaceAnm0->SetAndExecuteScriptIdx(&this->playerPortraitVm, 0);
        this->playerFaceAnm0->SetSprite(&this->playerPortraitVm, 0);
    }
    else if (playerFace == 1)
    {
        this->playerFaceAnm1->SetAndExecuteScriptIdx(&this->playerPortraitVm, 0);
        this->playerFaceAnm1->SetSprite(&this->playerPortraitVm, 0);
    }

    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->portraitBackdropVm, 0);
    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->portraitOverlayVm, 2);
    this->commonFaceAnm->SetSprite(&this->portraitBackdropVm, sprite);
    this->commonFaceAnm->SetSprite(&this->portraitOverlayVm, sprite);
    g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->playerSpellNameVm, 4);
    g_AnmManager->DrawTextLeft(&this->playerSpellNameVm, 0x00F0F0FF, 0, name);
    this->playerSpellNameWidth = strlen(name) * 0xf / 2.0f + 16;
    this->playerSpellNameFrameVm.SetInterrupt(1);
    g_SoundPlayer.PlaySoundByIdx((SoundIdx)14, 0);
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x415f00
void Spellcard::CutInEnemy(i32 enemyFace, const char *name, i32 sprite)
{
    if (enemyFace >= 0)
    {
        this->enemyFaceAnm0->SetAndExecuteScriptIdx(&this->enemyPortraitVm, 0);
        this->enemyFaceAnm0->SetSprite(&this->enemyPortraitVm, enemyFace);
    }

    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->portraitBackdropVm, 1);
    this->commonFaceAnm->SetSprite(&this->portraitBackdropVm, sprite);
    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->portraitOverlayVm, 3);
    this->commonFaceAnm->SetSprite(&this->portraitOverlayVm, sprite);

    if (g_GameManager.IsSpellNumberInRange(205, 221))
    {
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->enemySpellNameVm, 6);
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->enemySpellNameLayer1Vm, 7);
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->enemySpellNameLayer2Vm, 8);
        g_AnmManager->DrawTextRight(&this->enemySpellNameLayer1Vm, 0x00fff0f0, 0, name);
        g_AnmManager->DrawTextRight(&this->enemySpellNameLayer2Vm, 0x00fff0f0, 0, name);
    }
    else
    {
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->enemySpellNameVm, 5);
    }
    g_AnmManager->DrawTextRight(&this->enemySpellNameVm, 0x00fff0f0, 0, name);

    this->enemySpellNameWidth = strlen(name) * 0xf / 2.0f + 16;
    this->enemySpellNameFrameVm.SetInterrupt(1);
    if (((this->flags >> SPELLCARD_FLAG_SUPPRESS_BONUS_PRESENTATION_SHIFT) & 1) == 0)
    {
        this->spellBonusFrameVm.SetInterrupt(1);
    }
    g_SoundPlayer.PlaySoundByIdx((SoundIdx)14, 0);
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x416130
void Spellcard::HidePlayerSpellPresentation()
{
    this->playerSpellNameVm.pendingInterrupt = 1;
    this->playerSpellNameFrameVm.SetInterrupt(2);
}

// FUNCTION: th08 0x416160
void Spellcard::HideEnemySpellPresentation()
{
    this->enemySpellNameVm.pendingInterrupt = 1;
    this->enemySpellNameFrameVm.SetInterrupt(2);
    if (((this->flags >> SPELLCARD_FLAG_SUPPRESS_BONUS_PRESENTATION_SHIFT) & 1) == 0)
    {
        this->spellBonusFrameVm.SetInterrupt(2);
    }
}

// FUNCTION: th08 0x4161b0
#pragma var_order(enemyScore, captured, catk, i, route, checksum, baseChecksum, this)
void Spellcard::EndSpell()
{
    i32 baseChecksum;
    i32 checksum;
    i32 route;
    i32 i;
    Catk *catk;
    i32 captured;
    i32 enemyScore;

    g_AsciiManager.nightBlindnessAlpha = 0;
    if ((this->flags & SPELLCARD_FLAG_ACTIVE) != 0)
    {
        captured = 0;
        this->flags &= ~SPELLCARD_FLAG_ACTIVE;
        this->HideEnemySpellPresentation();

        if (((this->flags >> SPELLCARD_FLAG_TIMER_CALLBACK_TRANSITION_SHIFT) & 1) == 0)
        {
            enemyScore = g_BulletManager.DespawnBullets(8000, 1);
            enemyScore = g_EnemyManager.KillAllNonBossEnemies(8000, enemyScore);
            if (enemyScore != 0)
            {
                g_GameManager.AddScore(enemyScore);
                g_Gui.ShowBonusScore(enemyScore);
            }

            if (((this->flags >> SPELLCARD_FLAG_CAPTURE_VALID_SHIFT) & 1) != 0)
            {
                catk = &g_GameManager.catkData[this->spellCardNumber];
                this->bonusAward = this->bonusProgress;
                if (((reinterpret_cast<Enemy *>(this->activeEnemy)->flags1 >>
                      ENEMY_FLAG_TIMEOUT_SPELL_SHIFT) & 1) != 0)
                {
                    this->pendingTimeOrbs = 700;
                }
                else
                {
                    i = (i32)this->timeLimit - (i32)this->timeLimit / 7;
                    if ((i32)this->timeRemaining >= i)
                    {
                        this->pendingTimeOrbs = 1000;
                    }
                    else if ((i32)this->timeRemaining >= 180)
                    {
                        this->pendingTimeOrbs = 900 * ((i32)this->timeRemaining - 180) / (i - 180) + 100;
                    }
                    else
                    {
                        this->pendingTimeOrbs = 100;
                    }
                }

                this->flags |= SPELLCARD_FLAG_CAPTURED;
                if (!g_GameManager.IsReplay())
                {
                    checksum = 0;
                    for (i = strlen(catk->spellName); i > 0;)
                    {
                        --i;
                        checksum += catk->spellName[i];
                    }
                    baseChecksum = checksum;

                    for (i = 0; i < SHOT_ALL + 1; i++)
                    {
                        checksum += catk->inGameHistory.captures[i];
                        checksum += catk->inGameHistory.attempts[i];
                        checksum += catk->inGameHistory.maxBonus[i];
                        checksum += catk->spellPracticeHistory.captures[i];
                        checksum += catk->spellPracticeHistory.attempts[i];
                        checksum += catk->spellPracticeHistory.maxBonus[i];
                    }

                    if (catk->historyChecksum != (u8)checksum)
                    {
                        for (i = 0; i < SHOT_ALL + 1; i++)
                        {
                            catk->inGameHistory.captures[i] = 0;
                            catk->inGameHistory.attempts[i] = 0;
                            catk->inGameHistory.maxBonus[i] = 0;
                            catk->spellPracticeHistory.captures[i] = 0;
                            catk->spellPracticeHistory.attempts[i] = 0;
                            catk->spellPracticeHistory.maxBonus[i] = 0;
                        }
                    }

                    catk->difficulty = (u8)g_GameManager.difficulty;
                    if (!g_GameManager.IsSpellPractice())
                    {
                        route = g_GameManager.shotType;
                        if ((u32)catk->inGameHistory.maxBonus[route] < (u32)this->bonusProgress)
                        {
                            catk->inGameHistory.maxBonus[route] = this->bonusProgress;
                        }
                        if ((u32)catk->inGameHistory.maxBonus[SHOT_ALL] < (u32)this->bonusProgress)
                        {
                            catk->inGameHistory.maxBonus[SHOT_ALL] = this->bonusProgress;
                        }
                        if (catk->inGameHistory.captures[route] < 9999)
                        {
                            catk->inGameHistory.captures[route]++;
                        }
                        if (catk->inGameHistory.captures[SHOT_ALL] < 9999)
                        {
                            catk->inGameHistory.captures[SHOT_ALL]++;
                        }
                    }
                    else
                    {
                        for (i = 0; (u32)i < sizeof(catk->spellCommentLine1); i++)
                        {
                            catk->spellCommentLine1[i] = (u8)this->spellCommentLine1[i] ^ 0xDD;
                        }
                        for (i = 0; (u32)i < sizeof(catk->spellCommentLine2); i++)
                        {
                            catk->spellCommentLine2[i] = (u8)this->spellCommentLine2[i] ^ 0xEE;
                        }

                        route = g_GameManager.shotType;
                        if ((u32)catk->spellPracticeHistory.maxBonus[route] < (u32)this->bonusProgress)
                        {
                            catk->spellPracticeHistory.maxBonus[route] = this->bonusProgress;
                        }
                        if ((u32)catk->spellPracticeHistory.maxBonus[SHOT_ALL] < (u32)this->bonusProgress)
                        {
                            catk->spellPracticeHistory.maxBonus[SHOT_ALL] = this->bonusProgress;
                        }
                        if (catk->spellPracticeHistory.captures[route] < 9999)
                        {
                            catk->spellPracticeHistory.captures[route]++;
                        }
                        if (catk->spellPracticeHistory.captures[SHOT_ALL] < 9999)
                        {
                            catk->spellPracticeHistory.captures[SHOT_ALL]++;
                        }
                    }

                    for (i = 0; i < SHOT_ALL + 1; i++)
                    {
                        baseChecksum += catk->inGameHistory.captures[i];
                        baseChecksum += catk->inGameHistory.attempts[i];
                        baseChecksum += catk->inGameHistory.maxBonus[i];
                        baseChecksum += catk->spellPracticeHistory.captures[i];
                        baseChecksum += catk->spellPracticeHistory.attempts[i];
                        baseChecksum += catk->spellPracticeHistory.maxBonus[i];
                    }
                    catk->historyChecksum = (u8)baseChecksum;
                    g_GameManager.hscr.spellCounters[this->spellCardNumber]++;
                }

                g_GameManager.globals->spellcardsCaptured++;
                captured = 1;
                g_GameManager.UpdateAntiTamper();
            }
        }

        if (this->spellEffect != NULL)
        {
            if (captured == 0)
            {
                if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0)
                {
                    *reinterpret_cast<u32 *>(&g_GameManager.flags) &= 0xFFFFFE7F;
                }
                this->spellEffect->active = 0;
                this->spellEffect = NULL;
                g_Gui.ShowPopupText(
                    0,
                    (((this->flags >> SPELLCARD_FLAG_ALTERNATE_EFFECT_STYLE_SHIFT) & 1) != 0) + 5);
            }
            else
            {
                this->flags |= SPELLCARD_FLAG_CAPTURE_REWARD_PENDING;
                if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0)
                {
                    *reinterpret_cast<u32 *>(&g_GameManager.flags) =
                        (*reinterpret_cast<u32 *>(&g_GameManager.flags) & 0xFFFFFE7F) | 0x100;
                }

                this->spellEffect->vm.interpCurrentTimers[AnmInterp_Pos] = 0;
                this->spellEffect->vm.interpEndTimers[AnmInterp_Pos] = 30;
                this->spellEffect->vm.interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOutQuartic;
                *reinterpret_cast<u32 *>(&this->spellEffect->vm.posInitial.x) =
                    *reinterpret_cast<u32 *>(&this->spellEffect->radius);
                this->spellEffect->vm.posFinal.x = 256.0f;
                *reinterpret_cast<u32 *>(&this->spellEffect->vm.posInitial.y) =
                    *reinterpret_cast<u32 *>(&this->spellEffect->secondaryRadius);
                *reinterpret_cast<u32 *>(&this->spellEffect->vm.posFinal.y) = 0;
                *reinterpret_cast<u32 *>(&this->spellEffect->vm.pos.x) =
                    *reinterpret_cast<u32 *>(&this->spellEffect->radius);
                *reinterpret_cast<u32 *>(&this->spellEffect->vm.pos.y) =
                    *reinterpret_cast<u32 *>(&this->spellEffect->secondaryRadius);

                this->spellEffect->vm.interpCurrentTimers[AnmInterp_RGB1] = 0;
                this->spellEffect->vm.interpEndTimers[AnmInterp_RGB1] = 60;
                this->spellEffect->vm.interpModes[AnmInterp_RGB1] = AnmInterpMode_EaseInQuartic;
                this->spellEffect->vm.color1Initial = this->spellEffect->vm.color1;
                this->spellEffect->vm.color1Final.r = 0xD0;
                this->spellEffect->vm.color1Final.g = 0x80;
                this->spellEffect->vm.color1Final.b = 0xA0;
                this->spellEffect->vm.color1Final.a = 0x20;
                this->spellEffect->radialWaveCount = 6.0f;
                this->spellEffect->timer = 0;

                this->rewardEffect = this->spellEffect;
                this->spellEffect = NULL;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SPELL_CAPTURE, 0);
            }
        }

        g_Player.bombInputLockFrames = 16;
        if (g_Player.playerState == PLAYER_STATE_ALIVE)
        {
            g_Player.timer = 70;
            g_Player.playerState = PLAYER_STATE_INVULNERABLE;
        }
        g_Gui.flags.bombDisplayUpdateFrames = 3;
        g_Gui.flags.lifeDisplayUpdateFrames = 3;
        g_SoundPlayer.PlaySoundByIdx((SoundIdx)15, 0);
        g_Background.StopSpellBackground();
    }

    if (this->activeEnemy != NULL)
    {
        reinterpret_cast<Enemy *>(this->activeEnemy)->flags1 &= ~ENEMY_FLAG_TIMEOUT_SPELL;
    }
    this->activeEnemy = NULL;
    this->flags &= ~SPELLCARD_FLAG_BONUS_UPDATES_DISABLED;
}

// FUNCTION: th08 0x416ad0
void Background::StopSpellBackground()
{
    this->spellBackgroundState = 0;
}

// FUNCTION: th08 0x416af0
void Spellcard::DeactivateWithoutCleanup()
{
    this->flags &= ~SPELLCARD_FLAG_ACTIVE;
    this->HideEnemySpellPresentation();
}

// FUNCTION: th08 0x416b10
void Spellcard::AddBonusProgress(i32 amount)
{
    if (((this->flags >> SPELLCARD_FLAG_BONUS_UPDATES_DISABLED_SHIFT) & 1) == 0)
    {
        this->bonusProgress += amount;
        if ((u32)this->bonusProgress >= (u32)this->scoreLimit)
        {
            this->bonusProgress = this->scoreLimit;
        }
        else
        {
            this->bonusCounter += amount / 120;
        }
    }
}


// FUNCTION: th08 0x416b90
#pragma var_order(itemCount, i, this)
i32 Spellcard::OnUpdateImpl()
{
    i32 i;
    i32 itemCount;

    if (g_GameManager.flags.deathbombFreezeActive || g_GameManager.scriptedUpdateFreeze)
    {
        return 1;
    }

    if ((this->flags & SPELLCARD_FLAG_ACTIVE) != 0)
    {
        if ((reinterpret_cast<Enemy *>(this->activeEnemy)->flags1 & ENEMY_FLAG_ACTIVE) == 0 ||
            this->activeEnemyIndexSnapshot != reinterpret_cast<Enemy *>(this->activeEnemy)->enemyIndex)
        {
            this->DeactivateWithoutCleanup();
        }

        if (EclOperands::g_TargetPlayerPosition017D61AC.x >= 64.0f &&
            EclOperands::g_TargetPlayerPosition017D61AC.y < 64.0f)
        {
            if (reinterpret_cast<u8 *>(&this->mixColor)[3] > 0x20)
            {
                reinterpret_cast<u8 *>(&this->mixColor)[3] -= 4;
            }
        }
        else if (reinterpret_cast<u8 *>(&this->mixColor)[3] < 0x80)
        {
            reinterpret_cast<u8 *>(&this->mixColor)[3] += 4;
        }

        if (((this->flags >> SPELLCARD_FLAG_CAPTURE_VALID_SHIFT) & 1) != 0)
        {
            if (((this->flags >> SPELLCARD_FLAG_BONUS_UPDATES_DISABLED_SHIFT) & 1) == 0 &&
                ((reinterpret_cast<Enemy *>(this->activeEnemy)->flags1 >>
                  ENEMY_FLAG_TIMEOUT_SPELL_SHIFT) & 1) == 0)
            {
                this->bonusProgress -=
                    (i32)((f64)((u32)this->bonusCounter / 60u) * g_Supervisor.framerateMultiplier);
                this->bonusProgress -= (u32)this->bonusProgress % 10u;
            }
        }
        else if (this->spellEffect->vm.activeSpriteIndex == 221)
        {
            g_EffectManager.effectAnm->SetSprite(&this->spellEffect->vm, 222);
            this->spellEffect->vm.scaleFinal.x = 4.0f;
            this->spellEffect->vm.scale.x = 4.0f;
        }

        if (this->spellEffect->radialWaveCount != 0.0f)
        {
            *reinterpret_cast<u32 *>(&this->spellEffect->secondaryRadius) =
                *reinterpret_cast<u32 *>(&this->spellEffect->vm.pos.y);
            if (this->spellEffect->secondaryRadius == 0.0f)
            {
                this->spellEffect->radialWaveCount = 0.0f;
            }
        }

        if ((i32)this->spellEffect->vm.interpEndTimers[AnmInterp_Pos] == 0)
        {
            this->spellEffect->vm.interpCurrentTimers[AnmInterp_Pos] = 0;
            this->spellEffect->vm.interpEndTimers[AnmInterp_Pos] =
                reinterpret_cast<Enemy *>(this->activeEnemy)->timerCallbackThresholdFrames - 100;
            this->spellEffect->vm.interpModes[AnmInterp_Pos] = AnmInterpMode_Linear;
            this->spellEffect->vm.posInitial.x = 256.0f;
            this->spellEffect->vm.posFinal.x = 8.0f;
            this->spellEffect->vm.posInitial.y = 0.0f;
            this->spellEffect->vm.posFinal.y = 0.0f;
        }

        if (((this->flags >> SPELLCARD_FLAG_EFFECT_TRACKING_DISABLED_SHIFT) & 1) == 0)
        {
            this->spellEffect->vector5 =
                ((reinterpret_cast<Enemy *>(this->activeEnemy)->position +
                  reinterpret_cast<Enemy *>(this->activeEnemy)->positionOffset) -
                 this->spellEffect->vector5) /
                    16.0f +
                this->spellEffect->vector5;
            this->spellEffect->vector5.z = 0.0f;
        }

        this->spellEffect->angle = AddNormalizeAngle(
            this->spellEffect->angle,
            this->UsesAlternateEffectStyle() ? -0.031415928f : 0.015707964f);
    }
    else if (this->rewardEffect != NULL)
    {
        if (this->rewardEffect->timer == 30)
        {
            this->rewardEffect->vm.interpCurrentTimers[AnmInterp_Scale] = 0;
            this->rewardEffect->vm.interpEndTimers[AnmInterp_Scale] = 20;
            this->rewardEffect->vm.interpModes[AnmInterp_Scale] = AnmInterpMode_EaseIn;
            *reinterpret_cast<u32 *>(&this->rewardEffect->vm.scaleInitial.x) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->shapeThickness);
            this->rewardEffect->vm.scaleFinal.x = 64.0f;
            this->rewardEffect->vm.interpCurrentTimers[AnmInterp_Pos] = 0;
            this->rewardEffect->vm.interpEndTimers[AnmInterp_Pos] = 100;
            this->rewardEffect->vm.interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOut;
            *reinterpret_cast<u32 *>(&this->rewardEffect->vm.posInitial.x) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->radius);
            this->rewardEffect->vm.posFinal.x = 0.0f;
            *reinterpret_cast<u32 *>(&this->rewardEffect->vm.posInitial.y) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->secondaryRadius);
            this->rewardEffect->vm.posFinal.y = 60.0f;
            *reinterpret_cast<u32 *>(&this->rewardEffect->vm.pos.x) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->radius);
            *reinterpret_cast<u32 *>(&this->rewardEffect->vm.pos.y) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->secondaryRadius);
        }
        else if (this->rewardEffect->timer == 60)
        {
            this->rewardEffect->vm.interpCurrentTimers[AnmInterp_Scale] = 0;
            this->rewardEffect->vm.interpEndTimers[AnmInterp_Scale] = 70;
            this->rewardEffect->vm.interpModes[AnmInterp_Scale] = AnmInterpMode_EaseIn;
            *reinterpret_cast<u32 *>(&this->rewardEffect->vm.scaleInitial.x) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->shapeThickness);
            this->rewardEffect->vm.scaleFinal.x = 0.0f;
        }
        else if (this->rewardEffect->timer == 130)
        {
            this->rewardEffect->active = 0;
            this->rewardEffect = NULL;
            g_Gui.ShowSpellcardBonus(this->bonusAward);
            g_GameManager.AddScore(this->bonusAward);
            this->flags &= ~SPELLCARD_FLAG_CAPTURE_REWARD_PENDING;
            if (this->pendingTimeOrbs > 0)
            {
                g_GameManager.AddTimeOrbs(this->pendingTimeOrbs);
                g_GameManager.globals->pointItemValue += this->pendingTimeOrbs * 10;
                this->pendingTimeOrbs = 0;
            }
        }

        if (this->rewardEffect != NULL)
        {
            if (this->rewardEffect->timer <= 80)
            {
                this->rewardEffect->vector5 =
                    (reinterpret_cast<const Float3 &>(EclOperands::g_TargetPlayerPosition017D61AC) -
                     this->rewardEffect->vector5) /
                        16.0f +
                    this->rewardEffect->vector5;
                this->rewardEffect->vector5.z = 0.0f;
                this->rewardEffect->angle = AddNormalizeAngle(
                    this->rewardEffect->angle, -0.015707964f);
            }
            else
            {
                this->rewardEffect->vector5 =
                    (reinterpret_cast<const Float3 &>(EclOperands::g_TargetPlayerPosition017D61AC) -
                     this->rewardEffect->vector5) /
                        4.0f +
                    this->rewardEffect->vector5;
                this->rewardEffect->vector5.z = 0.0f;
                this->rewardEffect->angle = AddNormalizeAngle(
                    this->rewardEffect->angle, -0.05235988f);
            }

            *reinterpret_cast<u32 *>(&this->rewardEffect->secondaryRadius) =
                *reinterpret_cast<u32 *>(&this->rewardEffect->vm.pos.y);

            if (this->rewardEffect->timer > 8 &&
                this->pendingTimeOrbs > 0)
            {
                D3DXVECTOR3 itemPosition;
                f32 angle =
                    ((f32)this->rewardEffect->timer - 10.0f) *
                        6.2831855f / 40.0f -
                    1.5707964f;
                angle = AddNormalizeAngle(angle, 0.0f);
                reinterpret_cast<Float3 *>(&itemPosition)->FromAngleMagnitude(angle, 128.0f);
                *reinterpret_cast<Float3 *>(&itemPosition) += this->rewardEffect->vector5;
                itemPosition.z = 0.0f;

                itemCount = this->pendingTimeOrbs > 7 ? 7 : this->pendingTimeOrbs;
                for (i = 0; i < itemCount; i++)
                {
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&itemPosition),
                                            ITEM_TIME_APEX_AUTOCOLLECT_REQUEST,
                                            ITEM_STATE_DEFAULT);
                }
                this->pendingTimeOrbs -= itemCount;

                angle = AddNormalizeAngle(angle, 3.1415927f);
                reinterpret_cast<Float3 *>(&itemPosition)->FromAngleMagnitude(angle, 128.0f);
                *reinterpret_cast<Float3 *>(&itemPosition) += this->rewardEffect->vector5;
                itemPosition.z = 0.0f;

                itemCount = this->pendingTimeOrbs > 7 ? 7 : this->pendingTimeOrbs;
                for (i = 0; i < 6; i++)
                {
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&itemPosition),
                                            ITEM_TIME_APEX_AUTOCOLLECT_REQUEST,
                                            ITEM_STATE_DEFAULT);
                }
                this->pendingTimeOrbs -= itemCount;
                if (this->pendingTimeOrbs < 0)
                {
                    this->pendingTimeOrbs = 0;
                }
            }
        }
    }

    g_AnmManager->ExecuteScript(&this->playerPortraitVm);
    g_AnmManager->ExecuteScript(&this->portraitBackdropVm);
    g_AnmManager->ExecuteScript(&this->portraitOverlayVm);
    g_AnmManager->ExecuteScript(&this->playerSpellNameVm);
    g_AnmManager->ExecuteScript(&this->playerSpellNameFrameVm);
    g_AnmManager->ExecuteScript(&this->enemyPortraitVm);
    g_AnmManager->ExecuteScript(&this->vm90C);
    g_AnmManager->ExecuteScript(&this->vmE54);
    g_AnmManager->ExecuteScript(&this->enemySpellNameVm);
    g_AnmManager->ExecuteScript(&this->enemySpellNameLayer1Vm);
    g_AnmManager->ExecuteScript(&this->enemySpellNameLayer2Vm);
    g_AnmManager->ExecuteScript(&this->enemySpellNameFrameVm);
    g_AnmManager->ExecuteScript(&this->spellBonusFrameVm);
    this->timeRemaining--;

    return 1;
}

// FUNCTION: th08 0x00417860
i32 Spellcard::UsesAlternateEffectStyle()
{
    return this->IsActive() &&
           ((this->flags >> SPELLCARD_FLAG_ALTERNATE_EFFECT_STYLE_SHIFT) & 1);
}

// FUNCTION: th08 0x004178A0
i32 Spellcard::IsActive()
{
    return this->flags & SPELLCARD_FLAG_ACTIVE;
}

// FUNCTION: th08 0x4178c0
#pragma var_order(savedPos, catk, i, value, score, divisor, leading, this)
i32 Spellcard::OnDrawImpl()
{
    i32 leading;
    i32 divisor;
    i32 score;
    i32 value;
    i32 i;
    Catk *catk;
    struct SavedPosition
    {
        f32 x;
        f32 y;
        f32 z;
    } savedPos;

    if (this->playerPortraitVm.IsVisible())
    {
        g_AnmManager->DrawNoRotation(&this->playerPortraitVm);
        g_AnmManager->DrawNoRotation(&this->portraitBackdropVm);
        g_AnmManager->Draw2D(&this->portraitOverlayVm);
    }

    if (this->enemyPortraitVm.IsVisible())
    {
        savedPos = *reinterpret_cast<SavedPosition *>(&this->enemyPortraitVm.pos);
        this->enemyPortraitVm.pos += this->enemyPortraitVm.pos2;
        g_AnmManager->DrawNoRotation(&this->enemyPortraitVm);
        *reinterpret_cast<SavedPosition *>(&this->enemyPortraitVm.pos) = savedPos;
        g_AnmManager->DrawNoRotation(&this->vm90C);
        g_AnmManager->Draw2D(&this->vmE54);
    }

    if (this->playerSpellNameVm.IsVisible())
    {
        this->playerSpellNameFrameVm.pos = this->playerSpellNameVm.pos;
        this->playerSpellNameFrameVm.pos.x -= 32.0f;
        g_AnmManager->DrawNoRotation(&this->playerSpellNameFrameVm);
        g_AnmManager->Draw2D(&this->playerSpellNameVm);
    }

    if (this->enemySpellNameVm.IsVisible())
    {
        g_AnmManager->SetMixColor(this->mixColor);
        this->enemySpellNameFrameVm.pos = this->enemySpellNameVm.pos;
        g_AnmManager->DrawNoRotation(&this->enemySpellNameFrameVm);
        g_AnmManager->Draw2D(&this->enemySpellNameVm);
        g_AnmManager->Draw2D(&this->enemySpellNameLayer1Vm);
        g_AnmManager->Draw2D(&this->enemySpellNameLayer2Vm);
        g_AnmManager->DrawNoRotation(&this->spellBonusFrameVm);

        if (((this->flags >> SPELLCARD_FLAG_SUPPRESS_BONUS_PRESENTATION_SHIFT) & 1) == 0)
        {
            score = this->bonusProgress;
            divisor = 10000000;
            leading = 0;
            catk = &g_GameManager.catkData[this->spellCardNumber];
            if (((this->flags >> SPELLCARD_FLAG_CAPTURE_VALID_SHIFT) & 1) == 0)
            {
                score = 0;
            }

            this->spellBonusDigitsVm.pos = this->spellBonusFrameVm.pos;
            this->spellBonusDigitsVm.pos.x -= 40.0f;
            this->spellBonusDigitsVm.pos.y += 1.0f;
            for (i = 0; i < 8; i++)
            {
                value = score / divisor;
                if (value != 0)
                {
                    leading = 1;
                }
                if (leading != 0 || divisor == 1)
                {
                    this->spellBonusDigitsVm.loadedSprite =
                        g_AsciiManager.asciiAnm->GetSprite(value + 136);
                    g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);
                }
                this->spellBonusDigitsVm.pos.x += 7.0f;
                score %= divisor;
                divisor /= 10;
            }

            value = g_GameManager.IsSpellPractice()
                        ? catk->spellPracticeHistory.captures[g_GameManager.shotType]
                        : catk->inGameHistory.captures[g_GameManager.shotType];
            if (value > 999)
            {
                value = 999;
            }
            this->spellBonusDigitsVm.pos.x += 32.0f;
            leading = 0;
            if (value / 100 != 0)
            {
                this->spellBonusDigitsVm.loadedSprite =
                    g_AsciiManager.asciiAnm->GetSprite(value / 100 + 136);
                g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);
                value %= 100;
                leading = 1;
            }
            this->spellBonusDigitsVm.pos.x += 7.0f;
            if (value / 10 != 0 || leading != 0)
            {
                this->spellBonusDigitsVm.loadedSprite =
                    g_AsciiManager.asciiAnm->GetSprite(value / 10 + 136);
                g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);
                value %= 10;
            }
            this->spellBonusDigitsVm.pos.x += 7.0f;
            this->spellBonusDigitsVm.loadedSprite = g_AsciiManager.asciiAnm->GetSprite(value + 136);
            g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);

            value = g_GameManager.IsSpellPractice()
                        ? catk->spellPracticeHistory.attempts[g_GameManager.shotType]
                        : catk->inGameHistory.attempts[g_GameManager.shotType];
            if (value > 999)
            {
                value = 999;
            }
            this->spellBonusDigitsVm.pos.x += 13.0f;
            if (value / 100 != 0)
            {
                this->spellBonusDigitsVm.loadedSprite =
                    g_AsciiManager.asciiAnm->GetSprite(value / 100 + 136);
                g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);
                value %= 100;
                leading = 1;
            }
            this->spellBonusDigitsVm.pos.x += 7.0f;
            if (value / 10 != 0 || leading != 0)
            {
                this->spellBonusDigitsVm.loadedSprite =
                    g_AsciiManager.asciiAnm->GetSprite(value / 10 + 136);
                g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);
                value %= 10;
            }
            this->spellBonusDigitsVm.pos.x += 7.0f;
            this->spellBonusDigitsVm.loadedSprite =
                g_AsciiManager.asciiAnm->GetSprite(value % 10 + 136);
            g_AnmManager->DrawNoRotation(&this->spellBonusDigitsVm);
        }
        g_AnmManager->SetMixColorDefault();
    }
    return 1;
}


// FUNCTION: th08 0x00417F60
ZunResult Spellcard::RegisterChain()
{
    Spellcard *spellcard = &g_Spellcard;

    if (spellcard->Init() != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    spellcard->lifetimeObject = g_Chain.CreateElem((ChainCallback)Spellcard::OnUpdate);
    spellcard->lifetimeChain = g_Chain.CreateElem((ChainCallback)Spellcard::OnDraw);
    reinterpret_cast<ChainElem *>(spellcard->lifetimeObject)->deletedCallback =
        (ChainLifetimeCallback)Spellcard::DeletedCallback;
    reinterpret_cast<ChainElem *>(spellcard->lifetimeObject)->arg = spellcard;
    spellcard->lifetimeChain->arg = spellcard;
    g_Chain.AddToCalcChain(
        reinterpret_cast<ChainElem *>(spellcard->lifetimeObject), CHAIN_PRIO_CALC_SPELLCARD);
    g_Chain.AddToDrawChain(spellcard->lifetimeChain, CHAIN_PRIO_DRAW_SPELLCARD);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x00418010
ChainCallbackResult Spellcard::OnUpdate(Spellcard *spellcard)
{
    return static_cast<ChainCallbackResult>(spellcard->OnUpdateImpl());
}

// FUNCTION: th08 0x00418030
ChainCallbackResult Spellcard::OnDraw(Spellcard *spellcard)
{
    return static_cast<ChainCallbackResult>(spellcard->OnDrawImpl());
}

// FUNCTION: th08 0x00418050
ZunResult Spellcard::DeletedCallback(Spellcard *spellcard)
{
    if (!IsDisableResourceReload())
    {
        g_AnmManager->ReleaseAnm(18);
        g_AnmManager->ReleaseAnm(19);
    }

    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(15);
        g_AnmManager->ReleaseAnm(16);
        g_AnmManager->ReleaseAnm(17);
    }

    if (spellcard->lifetimeObject != NULL)
    {
        reinterpret_cast<ChainElem *>(spellcard->lifetimeObject)->deletedCallback = NULL;
    }
    g_Chain.Cut(spellcard->lifetimeChain);
    spellcard->lifetimeChain = NULL;
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4180f0
void Spellcard::CutChain()
{
    if (g_SpellcardCalcChain != NULL)
    {
        g_Chain.Cut(g_SpellcardCalcChain);
    }
}









// FUNCTION: th08 0x44cba0
void Spellcard::InvalidateCaptureAndEnableBombDamage()
{
    this->flags &= ~SPELLCARD_FLAG_CAPTURE_VALID;
    this->bonusProgress = 0;
    reinterpret_cast<SpellcardResetFlagBits *>(&this->flags)->bombDamageEnabled =
        reinterpret_cast<SpellcardResetFlagBits *>(&this->flags)->active;
}

// FUNCTION: th08 0x44d150
void Spellcard::InvalidateCapture()
{
    this->flags &= ~SPELLCARD_FLAG_CAPTURE_VALID;
    this->bonusProgress = 0;
}

} /* namespace th08 */
