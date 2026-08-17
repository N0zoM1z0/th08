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
DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);
DIFFABLE_STATIC(Spellcard, g_Spellcard);
DIFFABLE_STATIC(ChainElem *, g_SpellcardCalcChain);
DIFFABLE_STATIC(i32, g_LastSpellCount);
DIFFABLE_STATIC(AnmLoaded *, g_SpellcardBackgroundAnm);

struct SpellcardFlagBits
{
    u32 lowBits : 5;
    u32 unk5 : 1;
    u32 highBits : 26;
};

struct SpellcardResetFlagBits
{
    u32 bit0 : 1;
    u32 bits1To6 : 6;
    u32 bit7 : 1;
    u32 rest : 24;
};

struct SpellEffectDword
{
    u32 value;
};

struct SpellEffectCopyOverlay
{
    u8 pad000[0x208];
    SpellEffectDword field208;
    SpellEffectDword field20C;
    u8 pad210[0x28];
    SpellEffectDword field238;
    SpellEffectDword field23C;
    u8 pad240[0x28];
    SpellEffectDword field268;
    u8 pad26C[0xC];
    SpellEffectDword field278;
    u8 pad27C[0x98];
    SpellEffectDword field314;
    u8 pad318[0x8];
    SpellEffectDword field320;
    u8 pad324[0x8];
    SpellEffectDword field32C;
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


// FUNCTION: th08 0x4143e0
Spellcard::Spellcard()
{
}


DIFFABLE_EXTERN(i32, g_GuiFullPowerModeFrames);

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

    this->flags &= ~0x200;
    this->flags |= 1;
    this->flags |= 4;
    this->flags &= ~0x10;
    this->flags &= ~8;
    this->flags &= ~0x40;
    this->flags &= ~0x80;
    this->flags &= ~0x400;
    this->spellCardNumber = spellCardNumber;
    this->activeEnemy = enemy;
    this->enemySpellFlagsSnapshot = *reinterpret_cast<u32 *>(this->activeEnemy + 0x2E0C);
    this->bonusProgress = bonus;
    this->scoreLimit = bonus;
    if (((*reinterpret_cast<u32 *>(this->activeEnemy + 0x3324) >> 27) & 1) != 0)
    {
        this->scoreLimit = 99999990;
    }
    this->bonusCounter =
        (this->bonusProgress - this->bonusProgress / 7u) / (*reinterpret_cast<i32 *>(this->activeEnemy + 0x3378) / 60);
    this->timer108 = *reinterpret_cast<i32 *>(this->activeEnemy + 0x3378);
    this->timer114 = *reinterpret_cast<i32 *>(this->activeEnemy + 0x3378);

    for (i = 0; i < 0x30; i++)
    {
        this->spellName[i] = encodedName[i] ^ 0xAA;
    }
    this->CutInEnemy(enemyFace, this->spellName, 0);
    g_BulletManager.bulletmanager_fun_00415c60();
    g_Background.background_fun_00415ce0();
    for (i = 0; (i32)i < g_Background.spellVmCount; i++)
    {
        g_SpellcardBackgroundAnm->SetAndExecuteScriptIdx(&g_Background.anmVmArray[i],
                                                         i + g_Background.spellVmScriptBase);
    }
    g_Background.onDrawLowPrioCallback = NULL;
    reinterpret_cast<Enemy *>(this->activeEnemy)->enemy_fun_00415c80();
    this->mixColor = 0x80808080;

    if (((this->flags >> 8) & 1) != 0)
    {
        this->rewardEffect = NULL;
        g_Gui.gui_fun_00437edc(this->bonusAward);
        g_GameManager.AddScore(this->bonusAward);
        this->flags &= ~0x100;
        if (this->pendingTimeOrbs > 0)
        {
            g_GameManager.AddTimeOrbs(this->pendingTimeOrbs);
            this->pendingTimeOrbs = 0;
        }
    }

    this->flags &= ~0x800;
    this->spellEffect = reinterpret_cast<u8 *>(g_EffectManager.FUN_00425870(
        (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0) ? 52 : 39,
        reinterpret_cast<D3DXVECTOR3 *>(this->activeEnemy + 0x2D34), 1, 1, -1));
    *reinterpret_cast<ZunTimer *>(this->spellEffect + 0x50) = 0;
    *reinterpret_cast<ZunTimer *>(this->spellEffect + 0xA4) = 100;
    *reinterpret_cast<u8 *>(this->spellEffect + 0xF8) = 6;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x238) = 8.0f;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x244) = 256.0f;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x23C) = 64.0f;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x248) = 0.0f;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x20C) = 64.0f;
    *reinterpret_cast<D3DXVECTOR3 *>(this->spellEffect + 0x2A4) = *reinterpret_cast<D3DXVECTOR3 *>(this->activeEnemy + 0x2D34);
    *reinterpret_cast<i32 *>(this->spellEffect + 0x324) = 64;
    *reinterpret_cast<i32 *>(this->spellEffect + 0x318) = 0;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x314) = 256.0f;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x320) = 15.0f;
    *reinterpret_cast<f32 *>(this->spellEffect + 0x334) = 6.0f;

    reinterpret_cast<SpellcardFlagBits *>(&this->flags)->unk5 =
        (*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3;
    g_Gui.flags.bombDisplayUpdateFrames = 3;
    g_Gui.flags.lifeDisplayUpdateFrames = 3;
    this->flags &= ~0x40;

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

        if (catk->unk0xe != (u8)checksum)
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
        catk->unk0xe = (u8)nameChecksum;
    }
}

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

    g_EclCallbackPublishedEnemyField24 = 0;
    if ((this->flags & 1) != 0)
    {
        captured = 0;
        this->flags &= ~1;
        this->spellcard_fun_00416160();

        if (((this->flags >> 3) & 1) == 0)
        {
            enemyScore = g_BulletManager.DespawnBullets(8000, 1);
            enemyScore = g_EnemyManager.FUN_0042efb0(8000, enemyScore);
            if (enemyScore != 0)
            {
                g_GameManager.AddScore(enemyScore);
                g_Gui.FUN_00437ddd(enemyScore);
            }

            if (((this->flags >> 2) & 1) != 0)
            {
                catk = &g_GameManager.catkData[this->spellCardNumber];
                this->bonusAward = this->bonusProgress;
                if (((*reinterpret_cast<u32 *>(this->activeEnemy + 0x3324) >> 27) & 1) != 0)
                {
                    this->pendingTimeOrbs = 700;
                }
                else
                {
                    i = (i32)this->timer114 - (i32)this->timer114 / 7;
                    if ((i32)this->timer108 >= i)
                    {
                        this->pendingTimeOrbs = 1000;
                    }
                    else if ((i32)this->timer108 >= 180)
                    {
                        this->pendingTimeOrbs = 900 * ((i32)this->timer108 - 180) / (i - 180) + 100;
                    }
                    else
                    {
                        this->pendingTimeOrbs = 100;
                    }
                }

                this->flags |= 0x200;
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

                    if (catk->unk0xe != (u8)checksum)
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
                    catk->unk0xe = (u8)baseChecksum;
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
                *reinterpret_cast<u8 *>(this->spellEffect + 0x350) = 0;
                this->spellEffect = NULL;
                g_Gui.FUN_00437e5d(0, (((this->flags >> 5) & 1) != 0) + 5);
            }
            else
            {
                this->flags |= 0x100;
                if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0)
                {
                    *reinterpret_cast<u32 *>(&g_GameManager.flags) =
                        (*reinterpret_cast<u32 *>(&g_GameManager.flags) & 0xFFFFFE7F) | 0x100;
                }

                *reinterpret_cast<ZunTimer *>(this->spellEffect + 0x50) = 0;
                *reinterpret_cast<ZunTimer *>(this->spellEffect + 0xA4) = 30;
                *reinterpret_cast<u8 *>(this->spellEffect + 0xF8) = 6;
                *reinterpret_cast<u32 *>(this->spellEffect + 0x238) =
                    *reinterpret_cast<u32 *>(this->spellEffect + 0x314);
                *reinterpret_cast<f32 *>(this->spellEffect + 0x244) = 256.0f;
                *reinterpret_cast<u32 *>(this->spellEffect + 0x23C) =
                    *reinterpret_cast<u32 *>(this->spellEffect + 0x32C);
                *reinterpret_cast<u32 *>(this->spellEffect + 0x248) = 0;
                *reinterpret_cast<u32 *>(this->spellEffect + 0x208) =
                    *reinterpret_cast<u32 *>(this->spellEffect + 0x314);
                *reinterpret_cast<u32 *>(this->spellEffect + 0x20C) =
                    *reinterpret_cast<u32 *>(this->spellEffect + 0x32C);

                *reinterpret_cast<ZunTimer *>(this->spellEffect + 0x5C) = 0;
                *reinterpret_cast<ZunTimer *>(this->spellEffect + 0xB0) = 60;
                *reinterpret_cast<u8 *>(this->spellEffect + 0xF9) = 3;
                reinterpret_cast<AnmVm *>(this->spellEffect)->color1Initial =
                    reinterpret_cast<AnmVm *>(this->spellEffect)->color1;
                *reinterpret_cast<u8 *>(this->spellEffect + 0x27E) = 0xD0;
                *reinterpret_cast<u8 *>(this->spellEffect + 0x27D) = 0x80;
                *reinterpret_cast<u8 *>(this->spellEffect + 0x27C) = 0xA0;
                *reinterpret_cast<u8 *>(this->spellEffect + 0x27F) = 0x20;
                *reinterpret_cast<f32 *>(this->spellEffect + 0x334) = 6.0f;
                *reinterpret_cast<ZunTimer *>(this->spellEffect + 0x338) = 0;

                this->rewardEffect = this->spellEffect;
                this->spellEffect = NULL;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SPELL_CAPTURE, 0);
            }
        }

        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xE2A6C) = 16;
        if (g_Player.playerState == PLAYER_STATE_ALIVE)
        {
            g_Player.timer = 70;
            g_Player.playerState = PLAYER_STATE_DEAD;
        }
        g_Gui.flags.bombDisplayUpdateFrames = 3;
        g_Gui.flags.lifeDisplayUpdateFrames = 3;
        g_SoundPlayer.PlaySoundByIdx((SoundIdx)15, 0);
        g_Background.background_fun_00416ad0();
    }

    if (this->activeEnemy != NULL)
    {
        *reinterpret_cast<u32 *>(this->activeEnemy + 0x3324) &= 0xF7FFFFFF;
    }
    this->activeEnemy = NULL;
    this->flags &= ~0x800;
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


// FUNCTION: th08 0x416b90
#pragma var_order(itemCount, i, this)
i32 Spellcard::OnUpdateImpl()
{
    i32 i;
    i32 itemCount;

    if (g_GameManager.flags.unk10 || g_EclScriptedGlobalUpdateFreeze)
    {
        return 1;
    }

    if ((this->flags & 1) != 0)
    {
        if ((*reinterpret_cast<u32 *>(this->activeEnemy + 0x3324) & 1) == 0 ||
            this->enemySpellFlagsSnapshot != *reinterpret_cast<u32 *>(this->activeEnemy + 0x2E0C))
        {
            this->spellcard_fun_00416af0();
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

        if (((this->flags >> 2) & 1) != 0)
        {
            if (((this->flags >> 11) & 1) == 0 &&
                ((*reinterpret_cast<u32 *>(this->activeEnemy + 0x3324) >> 27) & 1) == 0)
            {
                this->bonusProgress -=
                    (i32)((f64)((u32)this->bonusCounter / 60u) * g_EclGameTimeScale);
                this->bonusProgress -= (u32)this->bonusProgress % 10u;
            }
        }
        else if (*reinterpret_cast<i16 *>(this->spellEffect + 0x214) == 221)
        {
            g_AsciiManagerDemoAnm0577EB4->SetSprite(reinterpret_cast<AnmVm *>(this->spellEffect), 222);
            *reinterpret_cast<f32 *>(this->spellEffect + 0x270) = 4.0f;
            *reinterpret_cast<f32 *>(this->spellEffect + 0x18) = 4.0f;
        }

        if (*reinterpret_cast<f32 *>(this->spellEffect + 0x334) != 0.0f)
        {
            *reinterpret_cast<u32 *>(this->spellEffect + 0x32C) =
                *reinterpret_cast<u32 *>(this->spellEffect + 0x20C);
            if (*reinterpret_cast<f32 *>(this->spellEffect + 0x32C) == 0.0f)
            {
                *reinterpret_cast<f32 *>(this->spellEffect + 0x334) = 0.0f;
            }
        }

        if ((i32)*reinterpret_cast<ZunTimer *>(this->spellEffect + 0xA4) == 0)
        {
            *reinterpret_cast<ZunTimer *>(this->spellEffect + 0x50) = 0;
            *reinterpret_cast<ZunTimer *>(this->spellEffect + 0xA4) =
                *reinterpret_cast<i32 *>(this->activeEnemy + 0x3378) - 100;
            *reinterpret_cast<u8 *>(this->spellEffect + 0xF8) = 0;
            *reinterpret_cast<f32 *>(this->spellEffect + 0x238) = 256.0f;
            *reinterpret_cast<f32 *>(this->spellEffect + 0x244) = 8.0f;
            *reinterpret_cast<f32 *>(this->spellEffect + 0x23C) = 0.0f;
            *reinterpret_cast<f32 *>(this->spellEffect + 0x248) = 0.0f;
        }

        if (((this->flags >> 6) & 1) == 0)
        {
        *reinterpret_cast<Float3 *>(this->spellEffect + 0x2E0) =
            ((*reinterpret_cast<Float3 *>(this->activeEnemy + 0x2D34) +
              *reinterpret_cast<Float3 *>(this->activeEnemy + 0x2D40)) -
             *reinterpret_cast<Float3 *>(this->spellEffect + 0x2E0)) /
                16.0f +
            *reinterpret_cast<Float3 *>(this->spellEffect + 0x2E0);
        *reinterpret_cast<f32 *>(this->spellEffect + 0x2E8) = 0.0f;
        }

        *reinterpret_cast<f32 *>(this->spellEffect + 0x318) = AddNormalizeAngle(
            *reinterpret_cast<f32 *>(this->spellEffect + 0x318),
            this->FUN_00417860() ? -0.031415928f : 0.015707964f);
    }
    else if (this->rewardEffect != NULL)
    {
        if (*reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x338) == 30)
        {
            *reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x80) = 0;
            *reinterpret_cast<ZunTimer *>(this->rewardEffect + 0xD4) = 20;
            *reinterpret_cast<u8 *>(this->rewardEffect + 0xFC) = 1;
            *reinterpret_cast<u32 *>(this->rewardEffect + 0x268) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x320);
            *reinterpret_cast<f32 *>(this->rewardEffect + 0x270) = 64.0f;
            *reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x50) = 0;
            *reinterpret_cast<ZunTimer *>(this->rewardEffect + 0xA4) = 100;
            *reinterpret_cast<u8 *>(this->rewardEffect + 0xF8) = 4;
            *reinterpret_cast<u32 *>(this->rewardEffect + 0x238) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x314);
            *reinterpret_cast<f32 *>(this->rewardEffect + 0x244) = 0.0f;
            *reinterpret_cast<u32 *>(this->rewardEffect + 0x23C) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x32C);
            *reinterpret_cast<f32 *>(this->rewardEffect + 0x248) = 60.0f;
            *reinterpret_cast<u32 *>(this->rewardEffect + 0x208) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x314);
            *reinterpret_cast<u32 *>(this->rewardEffect + 0x20C) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x32C);
        }
        else if (*reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x338) == 60)
        {
            *reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x80) = 0;
            *reinterpret_cast<ZunTimer *>(this->rewardEffect + 0xD4) = 70;
            *reinterpret_cast<u8 *>(this->rewardEffect + 0xFC) = 1;
            *reinterpret_cast<u32 *>(this->rewardEffect + 0x268) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x320);
            *reinterpret_cast<f32 *>(this->rewardEffect + 0x270) = 0.0f;
        }
        else if (*reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x338) == 130)
        {
            *reinterpret_cast<u8 *>(this->rewardEffect + 0x350) = 0;
            this->rewardEffect = NULL;
            g_Gui.gui_fun_00437edc(this->bonusAward);
            g_GameManager.AddScore(this->bonusAward);
            this->flags &= ~0x100;
            if (this->pendingTimeOrbs > 0)
            {
                g_GameManager.AddTimeOrbs(this->pendingTimeOrbs);
                g_GameManager.globals->pointItemValue += this->pendingTimeOrbs * 10;
                this->pendingTimeOrbs = 0;
            }
        }

        if (this->rewardEffect != NULL)
        {
            if (*reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x338) <= 80)
            {
                *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0) =
                    (reinterpret_cast<const Float3 &>(EclOperands::g_TargetPlayerPosition017D61AC) -
                     *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0)) /
                        16.0f +
                    *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0);
                *reinterpret_cast<f32 *>(this->rewardEffect + 0x2E8) = 0.0f;
                *reinterpret_cast<f32 *>(this->rewardEffect + 0x318) = AddNormalizeAngle(
                    *reinterpret_cast<f32 *>(this->rewardEffect + 0x318), -0.015707964f);
            }
            else
            {
                *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0) =
                    (reinterpret_cast<const Float3 &>(EclOperands::g_TargetPlayerPosition017D61AC) -
                     *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0)) /
                        4.0f +
                    *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0);
                *reinterpret_cast<f32 *>(this->rewardEffect + 0x2E8) = 0.0f;
                *reinterpret_cast<f32 *>(this->rewardEffect + 0x318) = AddNormalizeAngle(
                    *reinterpret_cast<f32 *>(this->rewardEffect + 0x318), -0.05235988f);
            }

            *reinterpret_cast<u32 *>(this->rewardEffect + 0x32C) =
                *reinterpret_cast<u32 *>(this->rewardEffect + 0x20C);

            if (*reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x338) > 8 &&
                this->pendingTimeOrbs > 0)
            {
                D3DXVECTOR3 itemPosition;
                f32 angle =
                    ((f32)*reinterpret_cast<ZunTimer *>(this->rewardEffect + 0x338) - 10.0f) *
                        6.2831855f / 40.0f -
                    1.5707964f;
                angle = AddNormalizeAngle(angle, 0.0f);
                reinterpret_cast<Float3 *>(&itemPosition)->FromAngleMagnitude(angle, 128.0f);
                *reinterpret_cast<Float3 *>(&itemPosition) +=
                    *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0);
                itemPosition.z = 0.0f;

                itemCount = this->pendingTimeOrbs > 7 ? 7 : this->pendingTimeOrbs;
                for (i = 0; i < itemCount; i++)
                {
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&itemPosition), ITEM_TIME2,
                                            ITEM_STATE_DEFAULT);
                }
                this->pendingTimeOrbs -= itemCount;

                angle = AddNormalizeAngle(angle, 3.1415927f);
                reinterpret_cast<Float3 *>(&itemPosition)->FromAngleMagnitude(angle, 128.0f);
                *reinterpret_cast<Float3 *>(&itemPosition) +=
                    *reinterpret_cast<Float3 *>(this->rewardEffect + 0x2E0);
                itemPosition.z = 0.0f;

                itemCount = this->pendingTimeOrbs > 7 ? 7 : this->pendingTimeOrbs;
                for (i = 0; i < 6; i++)
                {
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(&itemPosition), ITEM_TIME2,
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

    g_AnmManager->ExecuteScript(&this->vm120);
    g_AnmManager->ExecuteScript(&this->vm668);
    g_AnmManager->ExecuteScript(&this->vmBB0);
    g_AnmManager->ExecuteScript(&this->vm10F8);
    g_AnmManager->ExecuteScript(&this->vm1B88);
    g_AnmManager->ExecuteScript(&this->vm3C4);
    g_AnmManager->ExecuteScript(&this->vm90C);
    g_AnmManager->ExecuteScript(&this->vmE54);
    g_AnmManager->ExecuteScript(&this->vm139C);
    g_AnmManager->ExecuteScript(&this->vm1640);
    g_AnmManager->ExecuteScript(&this->vm18E4);
    g_AnmManager->ExecuteScript(&this->vm1E2C);
    g_AnmManager->ExecuteScript(&this->vm2374);
    this->timer108--;

    return 1;
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
    g_Chain.AddToCalcChain(reinterpret_cast<ChainElem *>(spellcard->lifetimeObject), 12);
    g_Chain.AddToDrawChain(spellcard->lifetimeChain, 15);
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
        this->playerFaceAnm0->SetAndExecuteScriptIdx(&this->vm120, 0);
        this->playerFaceAnm0->SetAndExecuteScriptIdx(&this->vm3C4, 0);
    }

    (*reinterpret_cast<AnmLoaded **>(0x017CE8F4))->SetAndExecuteScriptIdx(&this->vm10F8, 4);
    (*reinterpret_cast<AnmLoaded **>(0x017CE8F4))->SetAndExecuteScriptIdx(&this->vm139C, 5);
    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->SetAndExecuteScriptIdx(&this->vm1B88, 1);
    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->SetAndExecuteScriptIdx(&this->vm1E2C, 0);
    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->SetAndExecuteScriptIdx(&this->vm2374, 2);
    (*reinterpret_cast<AnmLoaded **>(0x004D50A8))->SetAndExecuteScriptIdx(&this->vm20D0, 4);

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vm120) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vm668) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vmBB0) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vm10F8) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vm3C4) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vm90C) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vmE54) + 0x220) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&this->vm139C) + 0x220) = 0;

    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vm120) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vm668) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vmBB0) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vm10F8) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vm3C4) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vm90C) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vmE54) + 0x1F8) &= ~1u;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&this->vm139C) + 0x1F8) &= ~1u;

    this->vm10F8.fontWidth = 15;
    this->vm10F8.fontHeight = 15;
    this->vm139C.fontWidth = 15;
    this->vm139C.fontHeight = 15;
    this->pendingTimeOrbs = 0;
    return ZUN_SUCCESS;
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

// FUNCTION: th08 0x44cba0
void Spellcard::FUN_0044cba0()
{
    this->flags &= ~4u;
    this->bonusProgress = 0;
    reinterpret_cast<SpellcardResetFlagBits *>(&this->flags)->bit7 =
        reinterpret_cast<SpellcardResetFlagBits *>(&this->flags)->bit0;
}

// FUNCTION: th08 0x44d150
void Spellcard::FUN_0044d150()
{
    this->flags &= ~4u;
    this->bonusProgress = 0;
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
