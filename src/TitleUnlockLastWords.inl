// Shared by the production TitleScreen owner and its strict VC7 probe.
// Keep this source-shaped implementation single-owned so the Linux port
// cannot silently fall back to the former production stub.
extern i32 g_SpellcardNumbersNormal[49];


struct TitleCatkView
{
    Catk catk;
    inline ZunBool SpellPracticeCaptured(i32 shotType) { return (this->catk.spellPracticeHistory.captures[shotType] > 0) ? TRUE : FALSE; }
    inline ZunBool CapturedAny(i32 shotType) { return (this->catk.inGameHistory.captures[shotType] > 0 || this->catk.spellPracticeHistory.captures[shotType] > 0) ? TRUE : FALSE; }
    inline ZunBool AttemptedAny(i32 shotType) { return (this->catk.inGameHistory.attempts[shotType] > 0 || this->catk.spellPracticeHistory.attempts[shotType] != 0) ? TRUE : FALSE; }
};
C_ASSERT(sizeof(TitleCatkView) == sizeof(Catk));

static inline void TitleUnlockLastWord(i32 spellCardNumber)
{
    g_GameManager.flsp.unlockedLastWordSpellCards[spellCardNumber - SPELLCARD_LAST_WORD_START] = (BYTE)spellCardNumber;
}

#pragma var_order(i, totalCaptures, extraClearCount2, extraClearCount3, k, lastSpellCaptures15, extraClearCount4, n, requiredNormalCaptures, ii, extraClearCount6, jj, extraStageClearCount, kk, lastSpellCaptures30)
void TitleScreen::UnlockLastWordSpellCards()
{
    i32 totalCaptures;
    i32 extraClearCount2;
    i32 extraClearCount3;
    i32 lastSpellCaptures15;
    i32 extraClearCount4;
    i32 requiredNormalCaptures;
    i32 extraClearCount6;
    i32 extraStageClearCount;
    i32 lastSpellCaptures30;
    i32 i;
    i32 k;
    i32 n;
    i32 ii;
    i32 jj;
    i32 kk;

    totalCaptures = 0;
    for (i = 0; i < SPELLCARD_COUNT_SPELLCARDS; i++)
    {
        if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[i])->SpellPracticeCaptured(SHOT_ALL))
            totalCaptures++;
    }

    extraClearCount2 = 0;
    for (i = 0; i < SHOT_ALL; i++)
    {
        if ((g_GameManager.clrdData[i].difficultiesClearedWithoutRetries[EASY] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[i].difficultiesClearedWithoutRetries[NORMAL] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[i].difficultiesClearedWithoutRetries[HARD] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[i].difficultiesClearedWithoutRetries[LUNATIC] & EXTRA_UNLOCKED_FLAG) != 0)
            extraClearCount2++;
    }
    if (extraClearCount2 >= 2)
        TitleUnlockLastWord(SPELLCARD_LW_WRIGGLE);

    extraClearCount3 = 0;
    for (k = 0; k < SHOT_ALL; k++)
    {
        if ((g_GameManager.clrdData[k].difficultiesClearedWithoutRetries[EASY] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[k].difficultiesClearedWithoutRetries[NORMAL] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[k].difficultiesClearedWithoutRetries[HARD] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[k].difficultiesClearedWithoutRetries[LUNATIC] & EXTRA_UNLOCKED_FLAG) != 0)
            extraClearCount3++;
    }
    if (extraClearCount3 >= 3)
        TitleUnlockLastWord(SPELLCARD_LW_MYSTIA);

    if (totalCaptures >= 50)
        TitleUnlockLastWord(SPELLCARD_LW_KEINE);

    lastSpellCaptures15 = 0;
    for (i = 0; i < g_LastSpellCount; i++)
    {
        if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[g_LastSpellNumbers[i]])->SpellPracticeCaptured(SHOT_ALL))
            lastSpellCaptures15++;
    }
    if (lastSpellCaptures15 >= 15)
        TitleUnlockLastWord(SPELLCARD_LW_KAGUYA);

    if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[137])->CapturedAny(SHOT_ALL))
        TitleUnlockLastWord(SPELLCARD_LW_EIRIN);

    extraClearCount4 = 0;
    for (n = 0; n < SHOT_ALL; n++)
    {
        if ((g_GameManager.clrdData[n].difficultiesClearedWithoutRetries[EASY] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[n].difficultiesClearedWithoutRetries[NORMAL] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[n].difficultiesClearedWithoutRetries[HARD] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[n].difficultiesClearedWithoutRetries[LUNATIC] & EXTRA_UNLOCKED_FLAG) != 0)
            extraClearCount4++;
    }
    if (extraClearCount4 >= 4)
        TitleUnlockLastWord(SPELLCARD_LW_REISEN);

    if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[195])->CapturedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[204])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[145])->CapturedAny(SHOT_ALL))
        TitleUnlockLastWord(SPELLCARD_LW_MOKOU);

    if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[208])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[209])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[210])->AttemptedAny(SHOT_ALL))
        TitleUnlockLastWord(SPELLCARD_LW_TEWI);

    if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[205])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[206])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[207])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[211])->AttemptedAny(SHOT_ALL))
        TitleUnlockLastWord(SPELLCARD_LW_KEINEEX);

    requiredNormalCaptures = 0;
    requiredNormalCaptures = 0;
    for (ii = 0; ii < g_SpellcardCountsPerDifficulty[NORMAL]; ii++)
    {
        if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[g_SpellcardNumbersNormal[ii]])->CapturedAny(SHOT_MARISA))
            requiredNormalCaptures++;
    }
    if (requiredNormalCaptures == g_SpellcardCountsPerDifficulty[NORMAL])
        TitleUnlockLastWord(SPELLCARD_LW_REIMU);

    if ((g_GameManager.clrdData[SHOT_REIMU].difficultiesClearedWithoutRetries[HARD] & EXTRA_UNLOCKED_FLAG) != 0 ||
        (g_GameManager.clrdData[SHOT_REIMU].difficultiesClearedWithoutRetries[LUNATIC] & EXTRA_UNLOCKED_FLAG) != 0)
        TitleUnlockLastWord(SPELLCARD_LW_MARISA);

    if (totalCaptures >= 120)
        TitleUnlockLastWord(SPELLCARD_LW_SAKUYA);

    extraClearCount6 = 0;
    for (jj = 0; jj < SHOT_ALL; jj++)
    {
        if ((g_GameManager.clrdData[jj].difficultiesClearedWithoutRetries[EASY] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[jj].difficultiesClearedWithoutRetries[NORMAL] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[jj].difficultiesClearedWithoutRetries[HARD] & EXTRA_UNLOCKED_FLAG) != 0 ||
            (g_GameManager.clrdData[jj].difficultiesClearedWithoutRetries[LUNATIC] & EXTRA_UNLOCKED_FLAG) != 0)
            extraClearCount6++;
    }
    if (extraClearCount6 >= 6)
        TitleUnlockLastWord(SPELLCARD_LW_YOUMU);

    extraStageClearCount = 0;
    for (kk = 0; kk < SHOT_ALL; kk++)
    {
        if ((g_GameManager.clrdData[kk].difficultiesClearedWithoutRetries[EXTRA] & ZUN_BIT(EXTRASTAGE)) != 0)
            extraStageClearCount++;
    }
    if (extraStageClearCount >= 3)
        TitleUnlockLastWord(SPELLCARD_LW_ALICE);

    lastSpellCaptures30 = 0;
    for (i = 0; i < g_LastSpellCount; i++)
    {
        if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[g_LastSpellNumbers[i]])->SpellPracticeCaptured(SHOT_ALL))
            lastSpellCaptures30++;
    }
    if (lastSpellCaptures30 >= 30)
        TitleUnlockLastWord(SPELLCARD_LW_REMILIA);

    if ((g_GameManager.clrdData[SHOT_ALL].difficultiesClearedWithRetries[LUNATIC] & 0xC000) != 0)
        TitleUnlockLastWord(SPELLCARD_LW_YUYUKO);

    if (reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[205])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[206])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[207])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[208])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[209])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[210])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[211])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[212])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[213])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[214])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[215])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[216])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[217])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[218])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[219])->AttemptedAny(SHOT_ALL) &&
        reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[220])->AttemptedAny(SHOT_ALL))
        TitleUnlockLastWord(SPELLCARD_LW_YUKARI);
}
