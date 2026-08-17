#include "th_pch.h"

#include "ScoreDat.hpp"
#include "Spellcard.hpp"
#include "Global.hpp"
#include "utils.hpp"

namespace th08
{
DIFFABLE_STATIC(Spellcard, g_Spellcard);
DIFFABLE_STATIC(ChainElem *, g_SpellcardCalcChain);
DIFFABLE_STATIC(i32, g_LastSpellCount);
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


// FUNCTION: th08 0x4143e0
Spellcard::Spellcard()
{
}


DIFFABLE_EXTERN(i32, g_GuiFullPowerModeFrames);

// FUNCTION: th08 0x415d10
void Spellcard::CutInEnemyNoPortrait(const char *name, i32 unused)
{
    this->flags |= 0x400;
    this->flags &= ~1;
    this->flags &= ~0x10;
    this->CutInEnemy(-1, name, 1);
}

// FUNCTION: th08 0x415d60
void Spellcard::CutInPlayer(i32 playerFace, const char *name, i32 sprite)
{
    if (playerFace == 0)
    {
        this->playerFaceAnm0->SetAndExecuteScriptIdx(&this->vm120, 0);
        this->playerFaceAnm0->SetSprite(&this->vm120, 0);
    }
    else if (playerFace == 1)
    {
        this->playerFaceAnm1->SetAndExecuteScriptIdx(&this->vm120, 0);
        this->playerFaceAnm1->SetSprite(&this->vm120, 0);
    }

    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->vm668, 0);
    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->vmBB0, 2);
    this->commonFaceAnm->SetSprite(&this->vm668, sprite);
    this->commonFaceAnm->SetSprite(&this->vmBB0, sprite);
    g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->vm10F8, 4);
    g_AnmManager->DrawTextLeft(&this->vm10F8, 0x00F0F0FF, 0, name);
    this->playerSpellNameWidth = strlen(name) * 0xf / 2.0f + 16;
    this->vm1B88.SetInterrupt(1);
    g_SoundPlayer.PlaySoundByIdx((SoundIdx)14, 0);
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x415f00
void Spellcard::CutInEnemy(i32 enemyFace, const char *name, i32 sprite)
{
    if (enemyFace >= 0)
    {
        this->enemyFaceAnm0->SetAndExecuteScriptIdx(&this->vm3C4, 0);
        this->enemyFaceAnm0->SetSprite(&this->vm3C4, enemyFace);
    }

    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->vm668, 1);
    this->commonFaceAnm->SetSprite(&this->vm668, sprite);
    this->commonFaceAnm->SetAndExecuteScriptIdx(&this->vmBB0, 3);
    this->commonFaceAnm->SetSprite(&this->vmBB0, sprite);

    if (g_GameManager.IsSpellNumberInRange(205, 221))
    {
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->vm139C, 6);
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->vm1640, 7);
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->vm18E4, 8);
        g_AnmManager->DrawTextRight(&this->vm1640, 0x00fff0f0, 0, name);
        g_AnmManager->DrawTextRight(&this->vm18E4, 0x00fff0f0, 0, name);
    }
    else
    {
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->vm139C, 5);
    }
    g_AnmManager->DrawTextRight(&this->vm139C, 0x00fff0f0, 0, name);

    this->enemySpellNameWidth = strlen(name) * 0xf / 2.0f + 16;
    this->vm1E2C.SetInterrupt(1);
    if (((this->flags >> 10) & 1) == 0)
    {
        this->vm2374.SetInterrupt(1);
    }
    g_SoundPlayer.PlaySoundByIdx((SoundIdx)14, 0);
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x416130
void Spellcard::spellcard_fun_00416130()
{
    this->vm10F8.pendingInterrupt = 1;
    this->vm1B88.SetInterrupt(2);
}

// FUNCTION: th08 0x416160
void Spellcard::spellcard_fun_00416160()
{
    this->vm139C.pendingInterrupt = 1;
    this->vm1E2C.SetInterrupt(2);
    if (((this->flags >> 10) & 1) == 0)
    {
        this->vm2374.SetInterrupt(2);
    }
}

// FUNCTION: th08 0x416af0
void Spellcard::spellcard_fun_00416af0()
{
    this->flags &= ~1;
    this->spellcard_fun_00416160();
}

// FUNCTION: th08 0x416b10
void Spellcard::spellcard_fun_00416b10(i32 amount)
{
    if (((this->flags >> 11) & 1) == 0)
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

    if (this->vm120.IsVisible())
    {
        g_AnmManager->DrawNoRotation(&this->vm120);
        g_AnmManager->DrawNoRotation(&this->vm668);
        g_AnmManager->Draw2D(&this->vmBB0);
    }

    if (this->vm3C4.IsVisible())
    {
        savedPos = *reinterpret_cast<SavedPosition *>(&this->vm3C4.pos);
        this->vm3C4.pos += this->vm3C4.pos2;
        g_AnmManager->DrawNoRotation(&this->vm3C4);
        *reinterpret_cast<SavedPosition *>(&this->vm3C4.pos) = savedPos;
        g_AnmManager->DrawNoRotation(&this->vm90C);
        g_AnmManager->Draw2D(&this->vmE54);
    }

    if (this->vm10F8.IsVisible())
    {
        this->vm1B88.pos = this->vm10F8.pos;
        this->vm1B88.pos.x -= 32.0f;
        g_AnmManager->DrawNoRotation(&this->vm1B88);
        g_AnmManager->Draw2D(&this->vm10F8);
    }

    if (this->vm139C.IsVisible())
    {
        g_AnmManager->SetMixColor(this->mixColor);
        this->vm1E2C.pos = this->vm139C.pos;
        g_AnmManager->DrawNoRotation(&this->vm1E2C);
        g_AnmManager->Draw2D(&this->vm139C);
        g_AnmManager->Draw2D(&this->vm1640);
        g_AnmManager->Draw2D(&this->vm18E4);
        g_AnmManager->DrawNoRotation(&this->vm2374);

        if (((this->flags >> 10) & 1) == 0)
        {
            score = this->bonusProgress;
            divisor = 10000000;
            leading = 0;
            catk = &g_GameManager.catkData[this->spellCardNumber];
            if (((this->flags >> 2) & 1) == 0)
            {
                score = 0;
            }

            this->vm20D0.pos = this->vm2374.pos;
            this->vm20D0.pos.x -= 40.0f;
            this->vm20D0.pos.y += 1.0f;
            for (i = 0; i < 8; i++)
            {
                value = score / divisor;
                if (value != 0)
                {
                    leading = 1;
                }
                if (leading != 0 || divisor == 1)
                {
                    this->vm20D0.loadedSprite =
                        (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value + 136);
                    g_AnmManager->DrawNoRotation(&this->vm20D0);
                }
                this->vm20D0.pos.x += 7.0f;
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
            this->vm20D0.pos.x += 32.0f;
            leading = 0;
            if (value / 100 != 0)
            {
                this->vm20D0.loadedSprite =
                    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value / 100 + 136);
                g_AnmManager->DrawNoRotation(&this->vm20D0);
                value %= 100;
                leading = 1;
            }
            this->vm20D0.pos.x += 7.0f;
            if (value / 10 != 0 || leading != 0)
            {
                this->vm20D0.loadedSprite =
                    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value / 10 + 136);
                g_AnmManager->DrawNoRotation(&this->vm20D0);
                value %= 10;
            }
            this->vm20D0.pos.x += 7.0f;
            this->vm20D0.loadedSprite = (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value + 136);
            g_AnmManager->DrawNoRotation(&this->vm20D0);

            value = g_GameManager.IsSpellPractice()
                        ? catk->spellPracticeHistory.attempts[g_GameManager.shotType]
                        : catk->inGameHistory.attempts[g_GameManager.shotType];
            if (value > 999)
            {
                value = 999;
            }
            this->vm20D0.pos.x += 13.0f;
            if (value / 100 != 0)
            {
                this->vm20D0.loadedSprite =
                    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value / 100 + 136);
                g_AnmManager->DrawNoRotation(&this->vm20D0);
                value %= 100;
                leading = 1;
            }
            this->vm20D0.pos.x += 7.0f;
            if (value / 10 != 0 || leading != 0)
            {
                this->vm20D0.loadedSprite =
                    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value / 10 + 136);
                g_AnmManager->DrawNoRotation(&this->vm20D0);
                value %= 10;
            }
            this->vm20D0.pos.x += 7.0f;
            this->vm20D0.loadedSprite =
                (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->GetSprite(value % 10 + 136);
            g_AnmManager->DrawNoRotation(&this->vm20D0);
        }
        g_AnmManager->SetMixColorDefault();
    }
    return 1;
}


// FUNCTION: th08 0x0041F040
void Spellcard::SetStoredVector(f32 x, f32 y, f32 z)
{
    *(f32 *)(this->spellEffect + 0x2A4) = x;
    *(f32 *)(this->spellEffect + 0x2A8) = y;
    *(f32 *)(this->spellEffect + 0x2AC) = z;
}



// FUNCTION: th08 0x004178A0
i32 Spellcard::IsActive()
{
    return this->flags & 1;
}

// FUNCTION: th08 0x00405260
i32 Spellcard::GetInactiveState()
{
    return (this->flags >> 9) & 1;
}


// FUNCTION: th08 0x0041FDD0
i32 Spellcard::GetTimerFrames()
{
    return *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x108);
}

// FUNCTION: th08 0x0041FD90
i32 Spellcard::GetActiveState()
{
    return this->IsActive() && ((this->flags >> 2) & 1);
}

// FUNCTION: th08 0x00417860
i32 Spellcard::FUN_00417860()
{
    return this->IsActive() && ((this->flags >> 5) & 1);
}

// FUNCTION: th08 0x0042DFF0
i32 Spellcard::FUN_0042DFF0()
{
    return (this->flags >> 7) & 1;
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

// FUNCTION: th08 0x4180f0
void Spellcard::CutChain()
{
    if (g_SpellcardCalcChain != NULL)
    {
        g_Chain.Cut(g_SpellcardCalcChain);
    }
}
} /* namespace th08 */
