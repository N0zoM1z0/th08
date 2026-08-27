// Shared by the production TitleScreen owner and its strict VC7 probe.
// The probe remains the canonical byte oracle for this source-shaped body.
struct TitleLastWordCommentFormat
{
    const char *format1;
    i32 args1[5];
    const char *format2;
    i32 args2[5];
};
C_ASSERT(sizeof(TitleLastWordCommentFormat) == 0x30);
extern TitleLastWordCommentFormat g_TitleLastWordCommentFormats[];
extern const char *g_TitleSpellDifficultyNames[];

#pragma var_order(spellCardNumber, i, totalAttempts, commentLine1, commentLine2)
void TitleScreen::FormatSpellCardInfo()
{
    i32 spellCardNumber;
    i32 i;
    i32 totalAttempts;
    char commentLine2[128];
    char commentLine1[128];

    if (this->currentScreenState == TitleCurrentScreenState_Ready && this->spellCardInfoRevealCountdown == 0)
        return;

    spellCardNumber = g_SpellcardNumbersPerStage[g_GameManager.currentStage][this->cursor];
    totalAttempts = g_GameManager.catkData[spellCardNumber].spellPracticeHistory.attempts[SHOT_ALL] +
                    g_GameManager.catkData[spellCardNumber].inGameHistory.attempts[SHOT_ALL];

    if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 11)
    {
        g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[0], COLOR_TEXT_WHITE, 0,
            "\x82\x6D\x82\x8F\x81\x44%s\x81\x40\x81\x40%s",
            ConvertToFullWidthDigits(spellCardNumber + 1, 3),
            totalAttempts == 0 ? "\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48"
                               : g_GameManager.catkData[spellCardNumber].spellName);
    }

    if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 9)
    {
        g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[1], COLOR_TEXT_WHITE, 0,
            "\x8E\x67\x97\x70\x8E\xD2  %s\x81\x40\x81\x40%s %s",
            totalAttempts == 0 ? "\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48"
                               : g_GameManager.catkData[spellCardNumber].spellOwnerName,
            g_TitleSpellDifficultyNames[Spellcard::GetDifficultyFromSpellCard(spellCardNumber)],
            Spellcard::IsLastSpell(spellCardNumber) ? "Last" : " ");
    }

    if (this->currentScreenState == TitleCurrentScreenState_Init)
    {
        g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[2], COLOR_TEXT_WHITE, 0,
            "\x8E\xE6\x93\xBE\x90\x94/\x92\xA7\x90\xED\x90\x94[\x8D\xC5\x8D\x82\x93\x5F]\x81\x40%s\x81\x40\x91\x53\x8E\xE5\x90\x6C\x8C\x87\x8C\x76",
            ResultScreen::GetCharacterName(g_GameManager.shotType));
    }

    if (Spellcard::GetDifficultyFromSpellCard(spellCardNumber) <= EXTRA)
    {
        if (!reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[spellCardNumber])->AttemptedAny(SHOT_ALL))
        {
            if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 7)
                g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[3], COLOR_TEXT_WHITE, 0,
                    "\x81\x40\x81\x40---/---(---/---)[--------]\x81\x40\x81\x40---/---(---/---)[--------]");
        }
        else if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 7)
        {
            g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[3], COLOR_TEXT_WHITE, 0,
                "\x81\x40\x81\x40%3d/%3d(%3d/%3d)[%.8d]\x81\x40\x81\x40%3d/%3d(%3d/%3d)[%.8d]",
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.captures[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.attempts[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].inGameHistory.captures[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].inGameHistory.attempts[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.maxBonus[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.captures[SHOT_ALL],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.attempts[SHOT_ALL],
                g_GameManager.catkData[spellCardNumber].inGameHistory.captures[SHOT_ALL],
                g_GameManager.catkData[spellCardNumber].inGameHistory.attempts[SHOT_ALL],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.maxBonus[SHOT_ALL]);
        }
    }
    else
    {
        if (!reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[spellCardNumber])->AttemptedAny(SHOT_ALL))
        {
            if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 7)
                g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[3], COLOR_TEXT_WHITE, 0,
                    "\x81\x40\x81\x40---/---(---/---)[--------]\x81\x40\x81\x40---/---(---/---)[--------]");
        }
        else if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 7)
        {
            g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[3], COLOR_TEXT_WHITE, 0,
                "\x81\x40\x81\x40%3d/%3d(---/---)[%.8d]\x81\x40\x81\x40%3d/%3d(---/---)[%.8d]",
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.captures[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.attempts[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.maxBonus[g_GameManager.shotType],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.captures[SHOT_ALL],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.attempts[SHOT_ALL],
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.maxBonus[SHOT_ALL]);
        }
    }

    if (this->currentScreenState == TitleCurrentScreenState_Init)
        g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[4], COLOR_TEXT_WHITE, 0,
            "\x83\x4A\x81\x5B\x83\x68\x82\xCC\x95\xE2\x91\xAB");

    if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 5)
    {
        if (!reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[spellCardNumber])->AttemptedAny(SHOT_ALL) &&
            spellCardNumber >= SPELLCARD_LAST_WORD_START - 1 && spellCardNumber <= SPELLCARD_LW_YUKARI &&
            !g_GameManager.IsLastWordSpellCardAttempted(spellCardNumber))
        {
            g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[5], COLOR_TEXT_WHITE, 0, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].format1,
                g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args1[0] + 1, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args1[1] + 1, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args1[2] + 1,
                g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args1[3] + 1, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args1[4] + 1);
        }
        else
        {
            memset(commentLine1, 0, sizeof(commentLine1));
            memcpy(commentLine1, g_GameManager.catkData[spellCardNumber].spellCommentLine1, 64);
            g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[5], COLOR_TEXT_WHITE, 0,
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.captures[SHOT_ALL] == 0
                    ? "\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48"
                    : commentLine1);
        }
    }

    if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 3)
    {
        if (!reinterpret_cast<TitleCatkView *>(&g_GameManager.catkData[spellCardNumber])->AttemptedAny(SHOT_ALL) &&
            spellCardNumber >= SPELLCARD_LAST_WORD_START - 1 && spellCardNumber <= SPELLCARD_LW_YUKARI &&
            !g_GameManager.IsLastWordSpellCardAttempted(spellCardNumber))
        {
            g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[6], COLOR_TEXT_WHITE, 0, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].format2,
                g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args2[0] + 1, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args2[1] + 1, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args2[2] + 1,
                g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args2[3] + 1, g_TitleLastWordCommentFormats[spellCardNumber - (SPELLCARD_LAST_WORD_START - 1)].args2[4] + 1);
        }
        else
        {
            memset(commentLine2, 0, sizeof(commentLine2));
            memcpy(commentLine2, g_GameManager.catkData[spellCardNumber].spellCommentLine2, 64);
            g_AnmManager->DrawTextLeft(&this->spellCardInfoVms[6], COLOR_TEXT_WHITE, 0,
                g_GameManager.catkData[spellCardNumber].spellPracticeHistory.captures[SHOT_ALL] == 0
                    ? "\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48"
                    : commentLine2);
        }
    }

    this->spellCardInfoVms[5].pos.x = 96.0f;
    this->spellCardInfoVms[6].pos.x = 96.0f;

    if (this->currentScreenState == TitleCurrentScreenState_Init || this->spellCardInfoRevealCountdown == 3)
    {
        for (i = 0; i < 7; i++)
            this->spellCardInfoVms[i].color1.a = 255;
    }

    if (this->spellCardInfoRevealCountdown != 0)
        this->spellCardInfoRevealCountdown--;
}
