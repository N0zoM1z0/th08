#include "th_pch.h"

#include "AsciiManager.hpp"
#include "Config.hpp"
#include "GameManager.hpp"
#include "ResultScreen.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "pbg/Lzss.hpp"

#include "i18n.hpp"

#include <direct.h>
#include <stdio.h>
#include <time.h>

// General constants
#define RESULT_KEYBOARD_COLUMNS 16
#define RESULT_KEYBOARD_ROWS 6

#define RESULT_KEYBOARD_CHARACTERS (RESULT_KEYBOARD_COLUMNS * RESULT_KEYBOARD_ROWS)
#define RESULT_KEYBOARD_KEY_SPACE (RESULT_KEYBOARD_CHARACTERS - 2)
#define RESULT_KEYBOARD_KEY_END (RESULT_KEYBOARD_CHARACTERS - 1)

// Script indices
#define RESULT_SCRIPT_CATEGORY_HIGHSCORE 0
#define RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE 3

#define RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY 4
#define RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EXTRA 8

#define RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY 9
#define RESULT_SCRIPT_SPELLCARD_DIFFICULTY_ALL 14

#define RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI 15
#define RESULT_SCRIPT_HIGHSCORE_CHARACTER_YUYUKO 26

#define RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI 27
#define RESULT_SCRIPT_SPELLCARD_CHARACTER_ALL 39

#define RESULT_SCRIPT_LISTING 40

#define RESULT_SCRIPT_REPLAY_SAVE_QUESTION 49

#define RESULT_SCRIPT_YES 50
#define RESULT_SCRIPT_REPLAY_LISTING_MAIN 55
#define RESULT_SCRIPT_REPLAY_LISTING_START 56
#define RESULT_SCRIPT_PLAYER_RESULTS 71

// TODO: when the ResultScreen is moved into this file, use this macro
// for the array size of ResultScreen::spriteVms
#define RESULT_NUM_SCRIPTS 72

// Interrupt numbers

// These two interrupt numbers have no functional difference. No script
// in result00.anm has an explicit 1 or 2 interrupt handler, the -1 interrupt
// handler is called. They both have the intended effect of making an AnmVm
// disappear.
#define RESULT_INTERRUPT_HIDE 1
#define RESULT_INTERRUPT_EXITING 2

#define RESULT_INTERRUPT_LISTING_APPEAR 3

#define RESULT_INTERRUPT_LISTING_MOVE_PAGE 10
#define RESULT_INTERRUPT_QUESTION_SAVE_REPLAY 11
#define RESULT_INTERRUPT_REPLAY_APPEAR 12
#define RESULT_INTERRUPT_REPLAY_OVERWRITE 13
#define RESULT_INTERRUPT_CANNOT_SAVE_REPLAY_RETRY 14
#define RESULT_INTERRUPT_SELECT_REPLAY 16

// Again, not a real interrupt in the scripts.
#define RESULT_INTERRUPT_HIDE_REPLAY 17
#define RESULT_INTERRUPT_PLAYER_RESULTS_SHOW 18

#define RESULT_INTERRUPT_CANNOT_SAVE_REPLAY_SLOW_MODE 19
#define RESULT_INTERRUPT_SPRITE_SELECTED 20
#define RESULT_INTERRUPT_SPRITE_NOT_SELECTED 21
#define RESULT_INTERRUPT_SPRITE_APPEAR 22
#define RESULT_INTERRUPT_SPRITE_CHOSEN 23
#define RESULT_INTERRUPT_CHARACTER_DISAPPEAR 24
#define RESULT_INTERRUPT_CHARACTER_APPEAR 25

namespace th08
{

DIFFABLE_STATIC_ASSIGN(i32, g_ResultStageNumbers[]) = {1, 2, 3, 4, 4, 5, 6, 6, 1, 1, 1};

DIFFABLE_STATIC_ASSIGN(const char *, g_ResultsStageList[]) = {
    "Stage 1", "Stage 2",       "Stage 3",        "Stage 4-uncanny", "Stage 4-powerful",
    "Stage 5", "Stage 6-Eirin", "Stage 6-Kaguya", "Extra Stage",     "Last Word",
};

DIFFABLE_STATIC_ASSIGN(float, g_SpellcardsWeightList[]) = {1.0f, 1.5f, 1.5f, 2.0f, 2.5f};

DIFFABLE_STATIC_ASSIGN(const char *, g_RightAlignedDifficultyList[]) = {
    "      Easy", "    Normal", "      Hard", "   Lunatic", "     Extra",
};

const char *g_AlphabetList = "ABCDEFGHIJKLMNOP"
                             "QRSTUVWXYZ.,:;_@"
                             "abcdefghijklmnop"
                             "qrstuvwxyz+-/*=%"
                             "0123456789#!?'\"$"
                             "(){}[]<>&\\|~^ --";

DIFFABLE_STATIC_ASSIGN(const char *, g_CharacterList[]) = {
    TH_RESULT_SHOT_REIMU_YUKARI, TH_RESULT_SHOT_MARISA_ALICE, TH_RESULT_SHOT_SAKUYA_REMILIA,
    TH_RESULT_SHOT_YOUMU_YUYUKO, TH_RESULT_SHOT_REIMU,        TH_RESULT_SHOT_YUKARI,
    TH_RESULT_SHOT_MARISA,       TH_RESULT_SHOT_ALICE,        TH_RESULT_SHOT_SAKUYA,
    TH_RESULT_SHOT_REMILIA,      TH_RESULT_SHOT_YOUMU,        TH_RESULT_SHOT_YUYUKO,
    TH_RESULT_SHOT_ALL};

DIFFABLE_STATIC_ASSIGN(const char *, g_ResultsCharacterNames[]) = {
    "Rm & Yk", "Ms & Al", "Sk & Rr", "Ym & Yy", "Reimu  ", "Yukari ",
    "Marisa ", "Alice  ", "Sakuya ", "Remilia", "Youmu  ", "Yuyuko ",
};

const char *ResultScreen::GetStageName(i32 stage)
{
    return (stage >= 9) ? "Clear" : g_ResultsStageList[stage];
}

const char *ResultScreen::GetCharacterName(i32 character)
{
    return g_CharacterList[character];
}

#pragma var_order(i, characterSlot, scoreData, encryptedData, currentOffset, currentCharacter, character, clrd, catk,  \
                  pscr, vrsm, compressedData, header, byteValue, byteIdx, xorValue, bytes, header2)
void ResultScreen::WriteScore(ResultScreen *result)
{
    i32 i;
    i32 character;
    i32 characterSlot;
    ScoreListNode *currentCharacter;
    ScoreDat *header;
    Clrd *clrd;
    Catk *catk;
    Pscr *pscr;
    u8 *scoreData;
    u8 *compressedData;
    u8 *encryptedData;
    ScoreDat *header2;
    Vrsm vrsm;
    i32 currentOffset;
    i32 byteIdx;
    u8 *bytes;
    u8 xorValue;
    u8 byteValue;

    currentOffset = 0;

    scoreData = (u8 *)g_ZunMemory.Alloc(0x640000);

#define COPY(data, size)                                                                                               \
    memcpy(scoreData + currentOffset, data, size);                                                                     \
    currentOffset += size;

    COPY(result->scoreDat, sizeof(*result->scoreDat));

    result->fileHeader.magic = TH8K_MAGIC;
    result->fileHeader.unkLen = sizeof(result->fileHeader);
    result->fileHeader.th8kLen = sizeof(result->fileHeader);
    result->fileHeader.version = SCORE_DAT_VERSION;

    COPY(&result->fileHeader, sizeof(result->fileHeader));

    for (i = 0; i < MAX_DIFFICULTIES; i++)
    {
        for (character = 0; character < SHOT_ALL; character++)
        {
            currentCharacter = result->scores[i][character].next;
            characterSlot = 0;

            while (currentCharacter != NULL)
            {
                if (currentCharacter->data->base.magic == HSCR_MAGIC)
                {
                    currentCharacter->data->character = character;
                    currentCharacter->data->difficulty = i;
                    currentCharacter->data->base.unkLen = sizeof(Hscr);
                    currentCharacter->data->base.th8kLen = sizeof(Hscr);
                    currentCharacter->data->base.version = HSCR_VERSION;
                    currentCharacter->data->base.unk_9 = 0;

                    COPY(currentCharacter->data, sizeof(Hscr));
                }

                currentCharacter = currentCharacter->next;
                characterSlot++;

                if (characterSlot >= 10)
                {
                    break;
                }
            }
        }
    }

    for (clrd = g_GameManager.clrdData, i = 0; i < SHOT_ALL + 1; i++, clrd++)
    {
        clrd->base.magic = CLRD_MAGIC;
        clrd->base.unkLen = sizeof(Clrd);
        clrd->base.th8kLen = sizeof(Clrd);
        clrd->base.version = CLRD_VERSION;

        COPY(clrd, sizeof(Clrd));
    }

    for (catk = g_GameManager.catkData, i = 0; i < SPELLCARD_COUNT_SPELLCARDS; i++, catk++)
    {
        if (catk->base.magic == CATK_MAGIC)
        {
            catk->spellcardNumber = i;
            catk->base.unkLen = sizeof(Catk);
            catk->base.th8kLen = sizeof(Catk);
            catk->base.version = CATK_VERSION;

            COPY(catk, sizeof(Catk));
        }
    }

    for (pscr = g_GameManager.pscrData, i = 0; i < SHOT_ALL; i++, pscr++)
    {
        if (pscr->unk0x175 != 0)
        {
            COPY(pscr, sizeof(Pscr));
        }
    }

    COPY(&result->lsnm, sizeof(Lsnm));

    g_GameManager.flsp.base.magic = FLSP_MAGIC;
    g_GameManager.flsp.base.version = FLSP_VERSION;
    g_GameManager.flsp.base.unkLen = g_GameManager.flsp.base.th8kLen = sizeof(Flsp);

    COPY(&g_GameManager.flsp, sizeof(Flsp));

    g_Supervisor.UpdatePlayTime();
    COPY(&g_GameManager.plst, sizeof(Plst));

    vrsm.base.magic = VRSM_MAGIC;
    vrsm.base.version = VRSM_VERSION;
    vrsm.base.unkLen = sizeof(Vrsm);
    vrsm.base.th8kLen = sizeof(Vrsm);
    vrsm.base.unk_9 = 0;

    strcpy(vrsm.version, CONFIG_VERSION_STRING);

    vrsm.exeSize = g_Supervisor.exeSize;
    vrsm.exeChecksum = g_Supervisor.exeChecksum;

    COPY(&vrsm, sizeof(Vrsm));
    header = (ScoreDat *)scoreData;

    header->decompressedFileSizeMinusHeader = currentOffset - sizeof(ScoreDat);
    header->decompressedFileSize = currentOffset;

    compressedData = Lzss::Encode(scoreData + sizeof(ScoreDat), header->decompressedFileSizeMinusHeader,
                                  (i32 *)&header->compressedFileSize);
    memcpy(scoreData + sizeof(ScoreDat), compressedData, header->compressedFileSize);
    GlobalFree(compressedData);

    currentOffset = header->compressedFileSize + sizeof(ScoreDat);

    header2 = (ScoreDat *)scoreData;
    header2->headerSize = sizeof(ScoreDat);
    header2->checksum = 0;
    header2->rngValue1 = g_Rng.GetRandomU16InRange(0x100);
    header2->rngValue2 = g_Rng.GetRandomU16InRange(0x100);
    header2->version = SCORE_DAT_VERSION;

    for (byteIdx = offsetof(ScoreDat, version); byteIdx < currentOffset; byteIdx++)
    {
        header2->checksum += scoreData[byteIdx];
    }

    xorValue = 0;
    byteValue = 0;

    bytes = (u8 *)header2 + 1;
    byteIdx = currentOffset;
    byteIdx -= offsetof(ScoreDat, checksum);
    xorValue = *bytes;

    while (byteIdx > 0)
    {
        byteValue = bytes[1];
        xorValue = (xorValue & 0xe0) >> 5 | (xorValue & 0x1f) << 3;
        bytes[1] ^= xorValue;
        xorValue += byteValue;

        bytes++;
        byteIdx--;
    }

    encryptedData = (u8 *)FileSystem::Encrypt(scoreData, currentOffset, SCORE_DAT_XOR_VALUE,
                                              SCORE_DAT_XOR_VALUE_INCREMENT, SCORE_DAT_CHUNK_SIZE, SCORE_DAT_MAX_BYTES);

    FileSystem::WriteDataToFile("score.dat", encryptedData, currentOffset);

    g_ZunMemory.Free(scoreData);
    g_ZunMemory.Free(encryptedData);

#undef COPY
}

// STUB: th08 0x454298
void ResultScreen::LogScoreDataToFile(ResultScreen *resultScreen)
{
}

i32 ResultScreen::LinkScoreEx(Hscr *out, i32 difficulty, i32 character)
{
    return ScoreDat::LinkScore(&this->scores[difficulty][character], out);
}

void ResultScreen::FreeScore(i32 difficulty, i32 character)
{
    ScoreDat::FreeAllScores(&this->scores[difficulty][character]);
}

i32 ResultScreen::HandleCategorySelectScreen()
{
    AnmVm *vm;
    i32 i;

    switch (this->menuDepth)
    {
    case 0:
        if (this->frameTimer2 == 0)
        {
            vm = this->spriteVms;

            for (i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
            {
                vm->pendingInterrupt = RESULT_INTERRUPT_HIDE;
            }

            for (i = RESULT_SCRIPT_CATEGORY_HIGHSCORE; i <= RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_APPEAR;
                g_AnmManager->ExecuteScript(&this->spriteVms[i]);
                if (i == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (this->frameTimer2 < 20)
        {
            break;
        }
        this->menuDepth++;
        this->frameTimer2 = 0;
    case 1:
        i = ResultScreen::MoveCursor(this, 4);
        if (i != 0)
        {
            for (i = RESULT_SCRIPT_CATEGORY_HIGHSCORE; i <= RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE; i++)
            {
                if (i == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }
        if (WAS_PRESSED(TH_BUTTON_D))
        {
            ResultScreen::LogScoreDataToFile(this);
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            if (this->cursor == 3)
            {
                goto exit;
            }

            this->cursor = 3;

            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);

            for (i = RESULT_SCRIPT_CATEGORY_HIGHSCORE; i <= RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE; i++)
            {
                if (i == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            // Useless assignment?
            vm = this->spriteVms;

            switch (this->cursor)
            {
            case 0:
                this->SetState(RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_DIFFICULTY);
                for (i = RESULT_SCRIPT_CATEGORY_HIGHSCORE; i <= RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE; i++)
                {
                    if (i == this->cursor)
                    {
                        this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                    }
                    else
                    {
                        this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                    }
                }
                break;
            case 1:
                this->SetState(RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_DIFFICULTY);
                for (i = RESULT_SCRIPT_CATEGORY_HIGHSCORE; i <= RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE; i++)
                {
                    if (i == this->cursor)
                    {
                        this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                    }
                    else
                    {
                        this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                    }
                }
                break;
            case 2:
                for (i = RESULT_SCRIPT_CATEGORY_HIGHSCORE; i <= RESULT_SCRIPT_CATEGORY_BACK_TO_TITLE; i++)
                {
                    if (i == this->cursor)
                    {
                        this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                    }
                    else
                    {
                        this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                    }
                }
                this->SetState(RESULT_SCREEN_STATE_OTHER_STATS_SCREEN_INIT);
                break;
            case 3:
            exit:
                this->SetState(RESULT_SCREEN_STATE_EXITING);
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
                return 1;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            return 1;
        }
    }

    this->frameTimer2++;

    return 0;
}

#pragma var_order(i, vm, i2, j)
i32 ResultScreen::HandleHighScoreDifficultySelect()
{
    AnmVm *vm;
    i32 i;
    i32 i2;
    i32 j;

    switch (this->menuDepth)
    {
    case 0:
        if (this->frameTimer2 == 0)
        {
            this->cursor = this->selectedDifficulty;

            for (i = RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY; i <= RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EXTRA; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_APPEAR;
                g_AnmManager->ExecuteScript(&this->spriteVms[i]);
                if ((i - RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (this->frameTimer2 < 6)
        {
            break;
        }
        this->menuDepth++;
        this->frameTimer2 = 0;
    case 1:
        i = ResultScreen::MoveCursor(this, MAX_DIFFICULTIES);
        if (i != 0)
        {
            for (i = RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY; i <= RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EXTRA; i++)
            {
                if ((i - RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->SetState(RESULT_SCREEN_STATE_CHOOSING_CATEGORY);
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->selectedDifficulty = this->cursor;
            this->cursor = 0;
            return 1;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            // Another useless assignment?
            vm = this->spriteVms;

            for (i = RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY; i <= RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EXTRA; i++)
            {
                if ((i - RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                }
            }

            this->selectedDifficulty = this->cursor;
            this->SetState(RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_CHARACTER);
            this->cursor = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            return 1;
        }

        break;
    }

    if (IS_PRESSED(TH_BUTTON_FOCUS) || IS_PRESSED(TH_BUTTON_SKIP))
    {
        if (this->cheatCodeStep < 4)
        {
            if (WAS_PRESSED(TH_BUTTON_RIGHT))
            {
                this->cheatCodeStep++;
            }
            else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
            {
                this->cheatCodeStep = 0;
            }
        }
        else if (this->cheatCodeStep < 5)
        {
            if (WAS_PRESSED(TH_BUTTON_LEFT))
            {
                this->cheatCodeStep++;
            }
            else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
            {
                this->cheatCodeStep = 0;
            }
        }
        else if (this->cheatCodeStep < 7)
        {
            if (WAS_PRESSED(TH_BUTTON_D))
            {
                this->cheatCodeStep++;
            }
            else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
            {
                this->cheatCodeStep = 0;
            }
        }
        else if (this->cheatCodeStep < 10)
        {
            if (WAS_PRESSED(TH_BUTTON_Q))
            {
                this->cheatCodeStep++;
            }
            else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
            {
                this->cheatCodeStep = 0;
            }
        }
        else
        {
            for (i2 = 0; i2 < SHOT_ALL + 1; i2++)
            {
                for (j = 0; j < MAX_DIFFICULTIES; j++)
                {
                    g_GameManager.clrdData[i2].difficultiesClearedWithoutRetries[j] |= 0xffff;
                    g_GameManager.clrdData[i2].difficultiesClearedWithRetries[j] |= 0xffff;
                }
            }

            for (i2 = 0; i2 < SPELLCARD_COUNT_SPELLCARDS; i2++)
            {
                for (j = 0; j < SHOT_ALL + 1; j++)
                {
                    g_GameManager.catkData[i2].inGameHistory.attempts[j]++;
                }
            }

            this->cheatCodeStep = 0;

            g_SoundPlayer.PlaySoundByIdx(SOUND_1UP, 0);
        }
    }
    else
    {
        this->cheatCodeStep = 0;
    }

    this->frameTimer2++;

    return 0;
}

i32 ResultScreen::HandleHighScoreCharacterSelect()
{
    AnmVm *vm;
    i32 i;

    switch (this->menuDepth)
    {
    case 0:
        if (this->frameTimer2 == 0)
        {
            this->cursor = this->selectedHighScoreCharacter;

            for (i = RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_HIGHSCORE_CHARACTER_YUYUKO; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_APPEAR;
                g_AnmManager->ExecuteScript(&this->spriteVms[i]);
                if ((i - RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (this->frameTimer2 < 6)
        {
            break;
        }
        this->menuDepth++;
        this->frameTimer2 = 0;
    case 1:
        i = ResultScreen::MoveCursor(this, SHOT_ALL);
        if (i != 0)
        {
            for (i = RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_HIGHSCORE_CHARACTER_YUYUKO; i++)
            {
                if ((i - RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->SetState(RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_DIFFICULTY);
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->selectedHighScoreCharacter = this->cursor;

            for (i = RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_HIGHSCORE_CHARACTER_YUYUKO; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
            }

            return 1;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            // I'm pretty sure ZUN copy pasted the code for each function and
            // didn't check.
            vm = this->spriteVms;

            for (i = RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_HIGHSCORE_CHARACTER_YUYUKO; i++)
            {
                if ((i - RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                }
            }

            this->spriteVms[RESULT_SCRIPT_LISTING].pendingInterrupt = RESULT_INTERRUPT_LISTING_APPEAR;
            this->selectedHighScoreCharacter = -1;
            this->SetState(RESULT_SCREEN_STATE_BEST_SCORES);
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            return 1;
        }

        break;
    }

    this->frameTimer2++;

    return 0;
}

i32 ResultScreen::HandleHighScoreScreen()
{
    i32 oldCursor;

    if (this->selectedHighScoreCharacter != this->cursor && this->frameTimer == 10)
    {
        this->selectedHighScoreCharacter = this->cursor;
    }

    if (this->frameTimer < 6)
    {
        return 0;
    }

    oldCursor = this->cursor;

    if (ResultScreen::MoveCursorHorizontally(this, SHOT_ALL) != 0)
    {
        this->frameTimer = 0;
        this->spriteVms[RESULT_SCRIPT_LISTING].pendingInterrupt =
            this->selectedDifficulty + RESULT_INTERRUPT_LISTING_APPEAR;
        this->spriteVms[oldCursor + RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI].pendingInterrupt =
            RESULT_INTERRUPT_CHARACTER_DISAPPEAR;
        this->spriteVms[this->cursor + RESULT_SCRIPT_HIGHSCORE_CHARACTER_REIMU_YUKARI].pendingInterrupt =
            RESULT_INTERRUPT_CHARACTER_APPEAR;
    }

    if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
    {
        this->selectedHighScoreCharacter = this->cursor;
        this->SetState(RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_CHARACTER);
        this->spriteVms[RESULT_SCRIPT_LISTING].pendingInterrupt = RESULT_INTERRUPT_HIDE;
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);

        return 1;
    }

    this->frameTimer2++;

    return 0;
}

i32 ResultScreen::HandleSpellCardDifficultySelect()
{
    AnmVm *vm;
    i32 i;

    switch (this->menuDepth)
    {
    case 0:
        if (this->frameTimer2 == 0)
        {
            this->cursor = this->selectedSpellcardDifficulty;

            for (i = RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY; i <= RESULT_SCRIPT_SPELLCARD_DIFFICULTY_ALL; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_APPEAR;
                g_AnmManager->ExecuteScript(&this->spriteVms[i]);
                if ((i - RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (this->frameTimer2 < 6)
        {
            break;
        }
        this->menuDepth++;
        this->frameTimer2 = 0;
    case 1:
        i = ResultScreen::MoveCursor(this, MAX_DIFFICULTIES + 1);
        if (i != 0)
        {
            for (i = RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY; i <= RESULT_SCRIPT_SPELLCARD_DIFFICULTY_ALL; i++)
            {
                if ((i - RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->SetState(RESULT_SCREEN_STATE_CHOOSING_CATEGORY);
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->selectedSpellcardDifficulty = this->cursor;
            this->cursor = 1;

            return 1;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            vm = this->spriteVms;

            for (i = RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY; i <= RESULT_SCRIPT_SPELLCARD_DIFFICULTY_ALL; i++)
            {
                if ((i - RESULT_SCRIPT_SPELLCARD_DIFFICULTY_EASY) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                }
            }

            this->selectedSpellcardDifficulty = this->cursor;
            this->SetState(RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_CHARACTER);
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            return 1;
        }

        break;
    }

    this->frameTimer2++;

    return 0;
}

i32 ResultScreen::HandleSpellCardCharacterSelect()
{
    AnmVm *vm;
    i32 i;

    switch (this->menuDepth)
    {
    case 0:
        if (this->frameTimer2 == 0)
        {
            this->cursor = this->shotTypeCursor;

            for (i = RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_SPELLCARD_CHARACTER_ALL; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_APPEAR;
                g_AnmManager->ExecuteScript(&this->spriteVms[i]);
                if ((i - RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (this->frameTimer2 < 6)
        {
            break;
        }
        this->menuDepth++;
        this->frameTimer2 = 0;
    case 1:
        i = ResultScreen::MoveCursor(this, SHOT_ALL + 1);
        if (i != 0)
        {
            for (i = RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_SPELLCARD_CHARACTER_ALL; i++)
            {
                if ((i - RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_SELECTED;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_NOT_SELECTED;
                }
            }
        }

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->SetState(RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_DIFFICULTY);
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->shotTypeCursor = this->cursor;

            for (i = RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_SPELLCARD_CHARACTER_ALL; i++)
            {
                this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
            }

            return 1;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            vm = this->spriteVms;

            for (i = RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI; i <= RESULT_SCRIPT_SPELLCARD_CHARACTER_ALL; i++)
            {
                if ((i - RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI) == this->cursor)
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_SPRITE_CHOSEN;
                }
                else
                {
                    this->spriteVms[i].pendingInterrupt = RESULT_INTERRUPT_HIDE;
                }
            }

            this->shotTypeCursor = this->cursor;
            this->SetState(RESULT_SCREEN_STATE_SPELLCARDS);

            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            this->spriteVms[RESULT_SCRIPT_LISTING].pendingInterrupt = RESULT_INTERRUPT_LISTING_APPEAR;
            this->cursor = 0;
            this->spellcardPage = -1;

            return 1;
        }

        break;
    }

    this->frameTimer2++;

    return 0;
}

#pragma var_order(spellCardNumber, i, spellCardCount)
i32 ResultScreen::HandleSpellCardScreen()
{
    i32 spellCardCount;
    i32 spellCardNumber;
    i32 i;

    if (this->exitingSpellcardResults && this->frameTimer >= 10)
    {
        this->SetState(RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_CHARACTER);
    }

    spellCardCount = g_SpellcardCountsPerDifficulty[this->selectedSpellcardDifficulty];

    if ((this->spellcardPage != this->cursor || this->previousShotType != this->shotTypeCursor) &&
        this->frameTimer == 10)
    {
        this->spellcardPage = this->cursor;
        this->previousShotType = this->shotTypeCursor;

        for (i = this->spellcardPage * 10; i < (this->spellcardPage * 10) + 10; i++)
        {
            if (i >= spellCardCount)
            {
                break;
            }

            spellCardNumber = g_SpellcardNumbersPerDifficulty[this->selectedSpellcardDifficulty][i];

            if (g_GameManager.catkData[spellCardNumber].inGameHistory.attempts[SHOT_ALL] == 0)
            {
                g_AnmManager->DrawTextLeft(&this->textVms[i % 10], COLOR_TEXT_WHITE, 0,
                                           TH_RESULT_SPELLCARD_NOT_UNLOCKED);
            }
            else
            {
                g_AnmManager->DrawTextLeft(&this->textVms[i % 10], COLOR_TEXT_WHITE, 0,
                                           g_GameManager.catkData[spellCardNumber].spellName);
            }

            this->textVms[i % 10].color1.a = 255;
        }

        g_AnmManager->DrawTextLeft(&this->textVms[10], COLOR_TEXT_WHITE, 0, TH_RESULT_SPELLCARD_NAME,
                                   this->capturedSpellCards[this->selectedSpellcardDifficulty][this->shotTypeCursor],
                                   spellCardCount);

        this->textVms[10].color1.a = 255;
    }

    if (this->frameTimer < 6)
    {
        return 0;
    }

    if (ResultScreen::MoveCursorHorizontally(this, (spellCardCount + 9) / 10) != 0)
    {
        this->frameTimer = 0;
        this->spriteVms[RESULT_SCRIPT_LISTING].pendingInterrupt = RESULT_INTERRUPT_LISTING_MOVE_PAGE;
    }
    else
    {
        if (ResultScreen::MoveShotTypeCursor(this, SHOT_ALL + 1) != 0)
        {
            this->frameTimer = 0;
            this->updateSpellcardResults = TRUE;

            // ZUN bug: if you press up or down really fast in the spellcard
            // record screen, there is a pile up of the shot type name sprites.
#ifdef FIX_REALLY_BAD_BUGS
            for (i = 0; i < SHOT_ALL + 1; i++)
            {
                this->spriteVms[i + RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI].pendingInterrupt =
                    RESULT_INTERRUPT_CHARACTER_DISAPPEAR;
            }
#else
            this->spriteVms[this->previousShotType + RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI].pendingInterrupt =
                RESULT_INTERRUPT_CHARACTER_DISAPPEAR;
#endif
            this->spriteVms[this->shotTypeCursor + RESULT_SCRIPT_SPELLCARD_CHARACTER_REIMU_YUKARI].pendingInterrupt =
                RESULT_INTERRUPT_CHARACTER_APPEAR;
        }
    }

    if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
    {
        this->exitingSpellcardResults = 1;
        this->frameTimer = 0;
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        this->spriteVms[RESULT_SCRIPT_LISTING].pendingInterrupt = RESULT_INTERRUPT_HIDE;

        return 1;
    }

    this->frameTimer2++;

    return 0;
}

i32 ResultScreen::HandleResultKeyboard()
{
    float slowdownRate;
    AnmVm *vm;
    i32 i;

    if (g_Supervisor.IsSlowModeEnabled() || g_Supervisor.IsSpeedhackDetected())
    {
        this->currentState = RESULT_SCREEN_STATE_STATS_SCREEN;
        this->frameTimer = 0;
        memcpy(g_GameManager.catkData, g_GameManager.catkData2, sizeof(g_GameManager.catkData2));
        return 0;
    }

    if (this->frameTimer == 0)
    {
        this->selectedHighScoreCharacter = g_GameManager.shotType + g_GameManager.fullShotType;
        this->selectedDifficulty = g_GameManager.difficulty;

        vm = this->spriteVms;

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
        {
            vm->pendingInterrupt = this->selectedDifficulty + RESULT_INTERRUPT_LISTING_APPEAR;
        }

        g_AnmManager->DrawTextCentered(&this->textVms[0], COLOR_TEXT_WHITE, 0,
                                       g_CharacterList[this->selectedHighScoreCharacter]);

        this->textVms[0].color1.a = 255;

        g_GameManager.hscr.playtimeFrames = g_GameManager.unk3DB94;

        g_GameManager.hscr.humanityRate = ((float)g_GameManager.unk3DBA0 / g_GameManager.unk3DBA4) * 10000.0f;

        this->hscr = g_GameManager.hscr;

        this->hscr.score = g_GameManager.globals->score;
        this->hscr.numRetries = g_GameManager.globals->numRetries;
        this->hscr.base.version = HSCR_VERSION;
        this->hscr.base.magic = HSCR_MAGIC;

        if (!g_GameManager.flags.unk4)
        {
            this->hscr.stage = g_GameManager.currentStage;
        }
        else
        {
            this->hscr.stage = 99;
        }

        this->hscr.base.unk_9 = 1;

        strcpy(this->hscr.name, this->lsnm.name);

        FormatDate(this->hscr.date);

        slowdownRate = ((g_Supervisor.lagNumerator / g_Supervisor.lagDenominator) - 0.5f) * 2.0f;
        if (slowdownRate < 0.0f)
        {
            slowdownRate = 0.0f;
        }
        else if (slowdownRate >= 1.0f)
        {
            slowdownRate = 1.0f;
        }

        this->hscr.lagPercentage = (1 - slowdownRate) * 100.0f;

        // Skip the result keyboard if the score doesn't come in the top 10 high scores
        if (ResultScreen::LinkScoreEx(&this->hscr, this->selectedDifficulty, this->selectedHighScoreCharacter) >= 10)
        {
            goto skip;
        }

        this->cursor = 0;

        if (this->lastNameSavedInScore)
        {
            this->selectedCharacter = RESULT_KEYBOARD_KEY_END;
        }

        strcpy(this->lastName, "");
    }

    if (this->frameTimer < 10)
    {
        return 0;
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_UP))
    {
    up_pressed:
        this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
        if (this->selectedCharacter < 0)
        {
            this->selectedCharacter += RESULT_KEYBOARD_CHARACTERS;
        }

        // ?! Use a do while loop!
        if (g_AlphabetList[this->selectedCharacter] == ' ')
        {
            goto up_pressed;
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_DOWN))
    {
    down_pressed:
        this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
        if (this->selectedCharacter >= RESULT_KEYBOARD_CHARACTERS)
        {
            this->selectedCharacter -= RESULT_KEYBOARD_CHARACTERS;
        }

        if (g_AlphabetList[this->selectedCharacter] == ' ')
        {
            goto down_pressed;
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_LEFT))
    {
    left_pressed:
        this->selectedCharacter--;
        if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == (RESULT_KEYBOARD_COLUMNS - 1))
        {
            this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
        }
        if (this->selectedCharacter < 0)
        {
            this->selectedCharacter = RESULT_KEYBOARD_COLUMNS - 1;
        }

        if (g_AlphabetList[this->selectedCharacter] == ' ')
        {
            goto left_pressed;
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_RIGHT))
    {
    right_pressed:
        this->selectedCharacter++;
        if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == 0)
        {
            this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
        }

        if (g_AlphabetList[this->selectedCharacter] == ' ')
        {
            goto right_pressed;
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_SELECTMENU))
    {
        i32 playerNameIdx = this->cursor >= 8 ? 7 : this->cursor;

        if (this->selectedCharacter < RESULT_KEYBOARD_KEY_SPACE)
        {
            this->hscr.name[playerNameIdx] = g_AlphabetList[this->selectedCharacter];
        }
        else if (this->selectedCharacter == RESULT_KEYBOARD_KEY_SPACE)
        {
            this->hscr.name[playerNameIdx] = ' ';
        }
        else
        {
            goto exit_keyboard;
        }

        if (this->cursor < 8)
        {
            this->cursor++;
            if (this->cursor == 8)
            {
                this->selectedCharacter = RESULT_KEYBOARD_KEY_END;
            }
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_RETURNMENU))
    {
        i32 playerNameIdx = this->cursor >= 8 ? 7 : this->cursor;

        if (this->cursor > 0)
        {
            this->cursor--;
            this->hscr.name[playerNameIdx] = ' ';
            this->hscr.name[this->cursor] = ' ';
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
    }

    if (WAS_PRESSED(TH_BUTTON_MENU))
    {
    exit_keyboard:
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
    skip:
        this->currentState = RESULT_SCREEN_STATE_STATS_SCREEN;
        this->frameTimer = 0;

        vm = this->spriteVms;

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
        {
            vm->pendingInterrupt = RESULT_INTERRUPT_EXITING;
        }

        strcpy(this->lastName, this->hscr.name);
        strcpy(this->lsnm.name, this->lastName);
    }

    return 0;
}

#pragma var_order(localTime, currentTime)
void ResultScreen::FormatDate(char *buffer)
{
    time_t currentTime;
    tm *localTime;

    time(&currentTime);

    localTime = localtime(&currentTime);

    strftime(buffer, 6, "%m/%d", localTime);
}

#pragma var_order(vm, interrupt, i, replayFileSize, replayFile)
i32 ResultScreen::HandleReplaySaveKeyboard()
{
    AnmVm *vm;
    i32 interrupt;
    i32 i;
    ReplayData *replayFile;
    i32 replayFileSize;

    switch (this->currentState)
    {
    case RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION:
        if (this->frameTimer == 10)
        {
            if (g_Supervisor.IsSlowModeEnabled() || g_Supervisor.IsSpeedhackDetected())
            {
                interrupt = RESULT_INTERRUPT_CANNOT_SAVE_REPLAY_SLOW_MODE;
            }
            else
            {
                if (g_GameManager.globals->numRetries != 0)
                {
                    interrupt = RESULT_INTERRUPT_CANNOT_SAVE_REPLAY_RETRY;
                }
                else
                {
                    interrupt = RESULT_INTERRUPT_QUESTION_SAVE_REPLAY;
                }
            }

            vm = &this->spriteVms[RESULT_SCRIPT_HIGHSCORE_DIFFICULTY_EASY];

            for (i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
            {
                vm->pendingInterrupt = interrupt;
            }

            if (interrupt != RESULT_INTERRUPT_QUESTION_SAVE_REPLAY)
            {
                this->currentState = RESULT_SCREEN_STATE_CANT_SAVE_REPLAY;
            }

            this->cursor = 0;
        }

        vm = &this->spriteVms[RESULT_SCRIPT_YES];

        if (this->cursor == 0)
        {
            vm[0].color1.d3dColor = COLOR_COMBINE_ALPHA(0xff6060, vm[0].color1.d3dColor);
            vm[1].color1.d3dColor = COLOR_COMBINE_ALPHA(0x606060, vm[1].color1.d3dColor);
        }
        else
        {
            vm[0].color1.d3dColor = COLOR_COMBINE_ALPHA(0x606060, vm[0].color1.d3dColor);
            vm[1].color1.d3dColor = COLOR_COMBINE_ALPHA(0xff6060, vm[1].color1.d3dColor);
        }

        if (this->frameTimer < 12)
        {
            return 0;
        }

        ResultScreen::MoveCursorHorizontally(this, 2);

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU) || WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto exit;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            if (cursor == 0)
            {
            choose_replay:
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

                while (g_SoundPlayer.ProcessQueues() != 0)
                    ;

                this->currentState = RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE;

                for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
                {
                    vm->pendingInterrupt = RESULT_INTERRUPT_REPLAY_APPEAR;
                }

                this->frameTimer = 0;

                goto replay;
            }

        exit:
            this->frameTimer = 0;

            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);

            this->currentState = RESULT_SCREEN_STATE_EXITING;

            for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
            {
                vm->pendingInterrupt = RESULT_INTERRUPT_EXITING;
            }
        }

        break;
    case RESULT_SCREEN_STATE_CANT_SAVE_REPLAY:
        if (this->frameTimer < 20)
        {
            return 0;
        }
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->frameTimer = 0;

            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);

            this->currentState = RESULT_SCREEN_STATE_EXITING;

            for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
            {
                vm->pendingInterrupt = RESULT_INTERRUPT_EXITING;
            }
        }
        break;
    case RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE:
    replay:
        if (this->frameTimer == 0)
        {
            char replayPath[64];

            _mkdir("replay");

            for (i = 0; i < RESULT_REPLAY_MAX_RESULTS; i++)
            {
                sprintf(replayPath, "./replay/th8_%.2d.rpy", i + 1);

                replayFile = (ReplayData *)FileSystem::OpenFile(replayPath, &replayFileSize, TRUE);

                if (replayFile == NULL)
                {
                    continue;
                }

                replayFile = ReplayManager::LoadReplayData(replayFile, replayFileSize);
                if (replayFile != NULL)
                {
                    this->replays[i] = *replayFile;
                    g_ZunMemory.Free(replayFile);
                }
            }
        }

        if (this->frameTimer < 20)
        {
            return 0;
        }

        ResultScreen::MoveCursor(this, RESULT_REPLAY_MAX_RESULTS);

        this->selectedReplay = this->cursor;

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            this->selectedReplay = this->cursor;
            this->frameTimer = 0;

            FormatDate(this->currentReplay.date);

            this->currentReplay.spellcardScore = g_GameManager.globals->score;

            if (this->replays[this->cursor].header.magic != *(i32 *)REPLAY_MAGIC ||
                (this->replays[this->cursor].header.version & 0xfff) != REPLAY_VERSION)
            {
                for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
                {
                    vm->pendingInterrupt = RESULT_INTERRUPT_HIDE_REPLAY;
                }

                vm = &this->spriteVms[RESULT_SCRIPT_REPLAY_LISTING_START + this->selectedReplay];
                vm->pendingInterrupt = RESULT_INTERRUPT_SELECT_REPLAY;

                this->currentState = RESULT_SCREEN_STATE_WRITING_REPLAY_NAME;
            }
            else
            {
                for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
                {
                    vm->pendingInterrupt = RESULT_INTERRUPT_REPLAY_OVERWRITE;
                }

                vm = &this->spriteVms[RESULT_SCRIPT_REPLAY_LISTING_START + this->selectedReplay];
                vm->pendingInterrupt = RESULT_INTERRUPT_SELECT_REPLAY;

                this->currentState = RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE;
            }

            this->cursor = 0;
            this->selectedCharacter = 0;
            if (this->lastNameSavedInScore)
            {
                this->selectedCharacter = RESULT_KEYBOARD_KEY_END;
            }
        }

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);

            this->currentState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;

            for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
            {
                vm->pendingInterrupt = RESULT_INTERRUPT_EXITING;
            }

            this->frameTimer = 0;
        }
        break;
    case RESULT_SCREEN_STATE_WRITING_REPLAY_NAME:
        if (this->frameTimer < 30)
        {
            return 0;
        }

        if (WAS_PRESSED_SCROLLING(TH_BUTTON_UP))
        {
        up_pressed:
            this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
            if (this->selectedCharacter < 0)
            {
                this->selectedCharacter += RESULT_KEYBOARD_CHARACTERS;
            }

            // ?! Use a do while loop!
            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                goto up_pressed;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }

        if (WAS_PRESSED_SCROLLING(TH_BUTTON_DOWN))
        {
        down_pressed:
            this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
            if (this->selectedCharacter >= RESULT_KEYBOARD_CHARACTERS)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_CHARACTERS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                goto down_pressed;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }

        if (WAS_PRESSED_SCROLLING(TH_BUTTON_LEFT))
        {
        left_pressed:
            this->selectedCharacter--;
            if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == (RESULT_KEYBOARD_COLUMNS - 1))
            {
                this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
            }
            if (this->selectedCharacter < 0)
            {
                this->selectedCharacter = RESULT_KEYBOARD_COLUMNS - 1;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                goto left_pressed;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }

        if (WAS_PRESSED_SCROLLING(TH_BUTTON_RIGHT))
        {
        right_pressed:
            this->selectedCharacter++;
            if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == 0)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                goto right_pressed;
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }

        if (WAS_PRESSED_SCROLLING(TH_BUTTON_SELECTMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

            while (g_SoundPlayer.ProcessQueues() != 0)
                ;

            i32 replayNameIdx = this->cursor >= 8 ? 7 : this->cursor;

            if (this->selectedCharacter < RESULT_KEYBOARD_KEY_SPACE)
            {
                this->lastName[replayNameIdx] = g_AlphabetList[this->selectedCharacter];
            }
            else if (this->selectedCharacter == RESULT_KEYBOARD_KEY_SPACE)
            {
                this->lastName[replayNameIdx] = ' ';
            }
            else
            {
                char replayPath[64];

                sprintf(replayPath, "./replay/th8_%.2d.rpy", this->selectedReplay + 1);

                ReplayManager::SaveReplay(replayPath, this->lastName);

                this->frameTimer = 0;
                this->currentState = RESULT_SCREEN_STATE_EXITING;

                vm = this->spriteVms;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
                {
                    vm->pendingInterrupt = RESULT_INTERRUPT_EXITING;
                }

                strcpy(this->lsnm.name, this->lastName);
            }

            if (this->cursor < 8)
            {
                this->cursor++;
                if (this->cursor == 8)
                {
                    this->selectedCharacter = RESULT_KEYBOARD_KEY_END;
                }
            }
        }

        if (WAS_PRESSED_SCROLLING(TH_BUTTON_RETURNMENU))
        {
            i32 replayNameIdx = this->cursor >= 8 ? 7 : this->cursor;

            if (this->cursor > 0)
            {
                this->cursor--;
                this->lastName[replayNameIdx] = ' ';
                this->lastName[this->cursor] = ' ';
            }

            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        }

        if (WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto choose_replay;
        }

        break;
    case RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE:
        vm = &this->spriteVms[RESULT_SCRIPT_YES];

        if (this->cursor == 0)
        {
            vm[0].color1.d3dColor = COLOR_COMBINE_ALPHA(0xff6060, vm[0].color1.d3dColor);
            vm[1].color1.d3dColor = COLOR_COMBINE_ALPHA(0x606060, vm[1].color1.d3dColor);
        }
        else
        {
            vm[0].color1.d3dColor = COLOR_COMBINE_ALPHA(0x606060, vm[0].color1.d3dColor);
            vm[1].color1.d3dColor = COLOR_COMBINE_ALPHA(0xff6060, vm[1].color1.d3dColor);
        }

        if (this->frameTimer < 20)
        {
            return 0;
        }

        ResultScreen::MoveCursorHorizontally(this, 2);

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU) || WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto choose_replay;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            this->frameTimer = 0;

            if (this->cursor == 0)
            {
                for (vm = this->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(this->spriteVms); i++, vm++)
                {
                    vm->pendingInterrupt = RESULT_INTERRUPT_HIDE_REPLAY;
                }

                vm = &this->spriteVms[this->selectedReplay + RESULT_SCRIPT_REPLAY_LISTING_START];
                vm->pendingInterrupt = RESULT_INTERRUPT_SELECT_REPLAY;

                this->currentState = RESULT_SCREEN_STATE_WRITING_REPLAY_NAME;
            }
            else
            {
                goto choose_replay;
            }
        }

        break;
    }

    return 0;
}

// Same code as in EoSD.
ZunResult ResultScreen::CheckConfirmButton()
{
    AnmVm *vm;

    switch (this->currentState)
    {
    case RESULT_SCREEN_STATE_STATS_SCREEN:
        if (this->frameTimer <= 30)
        {
            vm = &this->spriteVms[RESULT_SCRIPT_PLAYER_RESULTS];
            vm->pendingInterrupt = RESULT_INTERRUPT_PLAYER_RESULTS_SHOW;
        }
        if (this->frameTimer >= 90 && WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            vm = &this->spriteVms[RESULT_SCRIPT_PLAYER_RESULTS];
            vm->pendingInterrupt = RESULT_INTERRUPT_EXITING;
            this->frameTimer = 0;
            this->currentState = RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION;
        }
        break;
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        if (this->frameTimer >= 30)
        {
            this->frameTimer = 9;
            this->currentState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;
        }
        break;
    }

    return ZUN_SUCCESS;
}

i32 ResultScreen::HandleOtherStatsScreen()
{
    AnmVm *vm;

    switch (this->currentState)
    {
    case RESULT_SCREEN_STATE_OTHER_STATS_SCREEN_INIT:
        if (this->frameTimer == 1)
        {
            Float3 pos;

            pos.x = 56.0f;
            pos.y = 64.0f;
            pos.z = 0.0f;

            vm = &this->textVms[0];
            vm->pos = pos;

            g_Supervisor.UpdatePlayTime();

            g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, TH_RESULT_TOTAL_TIME, g_GameManager.plst.totalHours,
                                       g_GameManager.plst.totalMinutes, g_GameManager.plst.totalSeconds);

            g_Supervisor.UpdatePlayTime();

            this->totalSeconds = g_GameManager.plst.totalSeconds;

            vm++;
            pos.y += 17.0f;
            vm->pos = pos;

            g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, TH_RESULT_TOTAL_PLAYTIME, g_GameManager.plst.gameHours,
                                       g_GameManager.plst.gameMinutes, g_GameManager.plst.gameSeconds);

            vm++;
            pos.y += 17.0f;
            vm->pos = pos;

            g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, TH_RESULT_PLAYCOUNT_INFO);

            for (i32 i = 0; i < SHOT_ALL; i++)
            {
                vm++;
                pos.y += 17.0f;
                vm->pos = pos;

                g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, "%s %6d %6d %6d %6d %6d %6d", g_CharacterList[i],
                                           g_GameManager.plst.playDataByDifficulty[EASY].attemptsPerCharacter[i],
                                           g_GameManager.plst.playDataByDifficulty[NORMAL].attemptsPerCharacter[i],
                                           g_GameManager.plst.playDataByDifficulty[HARD].attemptsPerCharacter[i],
                                           g_GameManager.plst.playDataByDifficulty[LUNATIC].attemptsPerCharacter[i],
                                           g_GameManager.plst.playDataByDifficulty[EXTRA].attemptsPerCharacter[i],
                                           g_GameManager.plst.playDataTotals.attemptsPerCharacter[i]);
            }

            vm++;
            pos.y += 17.0f;
            vm->pos = pos;

            g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, "%s %6d %6d %6d %6d %6d %6d", g_CharacterList[SHOT_ALL],
                                       g_GameManager.plst.playDataByDifficulty[EASY].attemptsTotal,
                                       g_GameManager.plst.playDataByDifficulty[NORMAL].attemptsTotal,
                                       g_GameManager.plst.playDataByDifficulty[HARD].attemptsTotal,
                                       g_GameManager.plst.playDataByDifficulty[LUNATIC].attemptsTotal,
                                       g_GameManager.plst.playDataByDifficulty[EXTRA].attemptsTotal,
                                       g_GameManager.plst.playDataTotals.attemptsTotal);

            vm++;
            pos.y += 34.0f;
            vm->pos = pos;

            g_AnmManager->DrawTextLeft(
                vm, COLOR_TEXT_WHITE, 0, TH_RESULT_CLEAR_COUNT, g_GameManager.plst.playDataByDifficulty[EASY].clears,
                g_GameManager.plst.playDataByDifficulty[NORMAL].clears,
                g_GameManager.plst.playDataByDifficulty[HARD].clears,
                g_GameManager.plst.playDataByDifficulty[LUNATIC].clears,
                g_GameManager.plst.playDataByDifficulty[EXTRA].clears, g_GameManager.plst.playDataTotals.clears);

            vm++;
            pos.y += 17.0f;
            vm->pos = pos;

            g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, TH_RESULT_CONTINUE_COUNT,
                                       g_GameManager.plst.playDataByDifficulty[EASY].continues,
                                       g_GameManager.plst.playDataByDifficulty[NORMAL].continues,
                                       g_GameManager.plst.playDataByDifficulty[HARD].continues,
                                       g_GameManager.plst.playDataByDifficulty[LUNATIC].continues,
                                       g_GameManager.plst.playDataByDifficulty[EXTRA].continues,
                                       g_GameManager.plst.playDataByDifficulty[MAX_DIFFICULTIES]
                                           .continues); // ?! why not playDataTotals.continues?
            vm++;
            pos.y += 17.0f;
            vm->pos = pos;

            g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, TH_RESULT_PRACTICE_COUNT,
                                       g_GameManager.plst.playDataByDifficulty[EASY].practices,
                                       g_GameManager.plst.playDataByDifficulty[NORMAL].practices,
                                       g_GameManager.plst.playDataByDifficulty[HARD].practices,
                                       g_GameManager.plst.playDataByDifficulty[LUNATIC].practices,
                                       g_GameManager.plst.playDataByDifficulty[EXTRA].practices,
                                       g_GameManager.plst.playDataTotals.practices);

            vm++;
            pos.y += 17.0f;
            vm->pos = pos;
        }
        if (this->frameTimer < 40)
        {
            i32 i;

            for (vm = this->textVms, i = 0; i < 20; i++, vm++)
            {
                vm->color1.a = (this->frameTimer * 255) / 40;
            }
        }
        else
        {
            this->currentState = RESULT_SCREEN_STATE_OTHER_STATS_SCREEN;
        }
        break;
    case RESULT_SCREEN_STATE_OTHER_STATS_SCREEN:
        if ((this->frameTimer % 60) == 0)
        {
            g_Supervisor.UpdatePlayTime();

            if (g_GameManager.plst.totalSeconds != this->totalSeconds)
            {
                vm = &this->textVms[0];

                g_AnmManager->DrawTextLeft(vm, COLOR_TEXT_WHITE, 0, TH_RESULT_TOTAL_TIME, g_GameManager.plst.totalHours,
                                           g_GameManager.plst.totalMinutes, g_GameManager.plst.totalSeconds);

                this->totalSeconds = g_GameManager.plst.totalSeconds;
            }
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU | TH_BUTTON_RETURNMENU))
        {
            this->currentState = RESULT_SCREEN_STATE_OTHER_STATS_TO_INIT_TRANSITION;
            this->frameTimer = 0;
        }
        break;
    case RESULT_SCREEN_STATE_OTHER_STATS_TO_INIT_TRANSITION:
        if (this->frameTimer < 20)
        {
            i32 i;

            for (vm = this->textVms, i = 0; i < 20; i++, vm++)
            {
                vm->color1.a = 255 - (this->frameTimer * 255) / 20;
            }
        }
        else
        {
            this->currentState = RESULT_SCREEN_STATE_INIT;
            this->frameTimer = 0;
            return 1;
        }

        break;
    }

    return 0;
}

#pragma var_order(vm, strPos, unknownFloat, completion, slowdownRate)
i32 ResultScreen::DrawFinalStats()
{
    static const float g_DifficultyWeightList[] = {-30.0f, -10.0f, 20.0f, 30.0f, 30.0f};
    AnmVm *vm;
    Float3 strPos;
    float completion;
    float unknownFloat; // This variable also exists in EoSD, also unused
                        // there. Maybe a debugging variable?
    float slowdownRate;

    switch (this->currentState)
    {
    case RESULT_SCREEN_STATE_STATS_SCREEN:
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        vm = &this->spriteVms[RESULT_SCRIPT_PLAYER_RESULTS];
        g_AsciiManager.SetColor(vm->color1.d3dColor);
        unknownFloat = 0.0f;

        completion =
            g_GameManager.difficulty < EXTRA ? g_GameManager.unk3de04 / 195559.0f : g_GameManager.unk3de04 / 80000.0f;

        strPos = vm->pos;

        strPos.x += 210.0f;
        strPos.y += 32.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.globals->displayScore);

        strPos.x += g_AsciiManager.spaceWidth * 9;
        g_AsciiManager.AddFormatText(&strPos, "%1d",
                                     g_GameManager.globals->numRetries >= 10 ? 9 : g_GameManager.globals->numRetries);
        strPos.x -= g_AsciiManager.spaceWidth * 9;

        if (g_GameManager.globals->displayScore < 2000000)
        {
            unknownFloat -= 20.0f;
        }
        else if (g_GameManager.globals->displayScore < 200000000)
        {
            unknownFloat += (((g_GameManager.globals->displayScore - 2000000) / 198000000.0f) * 60.0f) - 20.0f;
        }
        else
        {
            unknownFloat += 40.0f;
        }

        strPos.y += 22.0f;
        g_AsciiManager.AddString(&strPos, g_RightAlignedDifficultyList[g_GameManager.difficulty]);

        unknownFloat += g_DifficultyWeightList[g_GameManager.difficulty];

        strPos.x += g_AsciiManager.spaceWidth;
        strPos.y += 22.0f;

        if (!g_GameManager.flags.unk4)
        {
            if (completion >= 1.0f)
            {
                completion = 0.99f;
            }
            g_AsciiManager.AddFormatText(&strPos, "    %3.2f%%", completion * 100.0f);

            unknownFloat += completion * 70.0f;
        }
        else
        {
            g_AsciiManager.AddFormatText(&strPos, "      100%%");

            unknownFloat += 70.0f;
        }

        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.globals->numRetries);
        unknownFloat -= (g_GameManager.globals->numRetries * 10.0f);

        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.GetDeaths());
        unknownFloat -= (g_GameManager.GetDeaths() * 5.0f) - 10.0f;

        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.GetBombsUsed());
        unknownFloat -= ((g_GameManager.GetBombsUsed() * 2.0f) - 10.0f);

        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.globals->spellcardsCaptured);
        unknownFloat += g_GameManager.globals->spellcardsCaptured * g_SpellcardsWeightList[g_GameManager.difficulty];

        slowdownRate = ((g_Supervisor.lagNumerator / g_Supervisor.lagDenominator) - 0.5f) * 2.0f;
        if (slowdownRate < 0.0f)
        {
            slowdownRate = 0.0f;
        }
        else if (slowdownRate >= 1.0f)
        {
            slowdownRate = 1.0f;
        }

        slowdownRate = (1 - slowdownRate) * 100.0f;

        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "    %3.2f%%", slowdownRate);

        if (slowdownRate < 50.0f)
        {
            unknownFloat -= 70.0f * slowdownRate / 100.0f;
        }
        else
        {
            unknownFloat = -999.0f;
        }

        if (g_GameManager.globals->pointItemsCollected < 800)
        {
            unknownFloat += 0.01f * g_GameManager.globals->pointItemsCollected;
        }
        else
        {
            unknownFloat += 8.0f;
        }

        if (g_GameManager.globals->graze < 5000)
        {
            unknownFloat += 0.0025f * g_GameManager.globals->graze;
        }
        else
        {
            unknownFloat += 12.5f;
        }

        g_AsciiManager.SetColor(COLOR_WHITE);

        break;
    }

    return 0;
}

ZunResult ResultScreen::RegisterChain(u32 unk)
{
    ResultScreen *resultScreen = ZUN_NEW(ResultScreen, "ResultSysInf");

    g_ScreenEffectCounter = 0;

    utils::GuiDebugPrint("Stg.PlayTimeAll = %d\r\n", g_GameManager.unk3de04);

    if (unk == 1) // When writing the score after a game
    {
        if (!g_GameManager.IsPracticeMode())
        {
            resultScreen->currentState = RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME;
        }
        else if (g_GameManager.flags.isSpellPractice)
        {
            resultScreen->currentState = RESULT_SCREEN_STATE_SPELL_PRACTICE;
        }
        else
        {
            resultScreen->currentState = RESULT_SCREEN_STATE_PRACTICE;
        }
    }
    else if (unk == 2) // Writing the score file for the first time
    {
        resultScreen->currentState = RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE;
        ResultScreen::AddedCallback(resultScreen);

        return ZUN_SUCCESS;
    }

    resultScreen->calcChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnUpdate);
    resultScreen->calcChain->addedCallback = (ChainLifetimeCallback)ResultScreen::AddedCallback;
    resultScreen->calcChain->deletedCallback = (ChainLifetimeCallback)ResultScreen::DeletedCallback;
    resultScreen->calcChain->arg = resultScreen;

    if (g_Chain.AddToCalcChain(resultScreen->calcChain, 16) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    resultScreen->drawChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnDraw);
    resultScreen->drawChain->arg = resultScreen;
    g_Chain.AddToDrawChain(resultScreen->drawChain, 18);

    return ZUN_SUCCESS;
}

#pragma var_order(vm1, i, vm2)
ChainCallbackResult ResultScreen::OnUpdate(ResultScreen *result)
{
    AnmVm *vm1;
    AnmVm *vm2;
    i32 i;

    switch (result->currentState)
    {
    case RESULT_SCREEN_STATE_PRACTICE:
        g_Supervisor.curState = SupervisorState_TitleScreen;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    case RESULT_SCREEN_STATE_SPELL_PRACTICE:
        g_Supervisor.curState = SupervisorState_TitleScreen;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    case RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE:
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    case RESULT_SCREEN_STATE_INIT:
    result_init:
        result->SetState(RESULT_SCREEN_STATE_CHOOSING_CATEGORY);
    case RESULT_SCREEN_STATE_CHOOSING_CATEGORY:
        result->HandleCategorySelectScreen();
        break;
    case RESULT_SCREEN_STATE_EXITING:
        if (result->frameTimer == 1)
        {
            Float3 pos(500.0f, 440.0f, 0.0f);

            g_Supervisor.SetupLoadingVms(&pos);

            for (vm1 = result->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(result->spriteVms); i++, vm1++)
            {
                vm1->pendingInterrupt = RESULT_INTERRUPT_EXITING;
            }
        }

        if (result->frameTimer < 20)
        {
            break;
        }

        g_Supervisor.curState = SupervisorState_TitleScreen;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    case RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_CHARACTER:
        result->HandleHighScoreCharacterSelect();
        break;
    case RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_DIFFICULTY:
        result->HandleHighScoreDifficultySelect();
        break;
    case RESULT_SCREEN_STATE_BEST_SCORES:
        result->HandleHighScoreScreen();
        break;
    case RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_CHARACTER:
        result->HandleSpellCardCharacterSelect();
        break;
    case RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_DIFFICULTY:
        result->HandleSpellCardDifficultySelect();
        break;
    case RESULT_SCREEN_STATE_SPELLCARDS:
        result->HandleSpellCardScreen();
        break;
    case RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME:
        result->HandleResultKeyboard();
        break;
    case RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION:
    case RESULT_SCREEN_STATE_CANT_SAVE_REPLAY:
    case RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE:
    case RESULT_SCREEN_STATE_WRITING_REPLAY_NAME:
    case RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE:
        result->HandleReplaySaveKeyboard();
        break;
    case RESULT_SCREEN_STATE_STATS_SCREEN:
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        result->CheckConfirmButton();
        break;

    case RESULT_SCREEN_STATE_OTHER_STATS_SCREEN_INIT:
    case RESULT_SCREEN_STATE_OTHER_STATS_SCREEN:
    case RESULT_SCREEN_STATE_OTHER_STATS_TO_INIT_TRANSITION:
        if (result->HandleOtherStatsScreen() != 0)
        {
            goto result_init;
        }
    }

    for (vm2 = result->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(result->spriteVms); i++, vm2++)
    {
        g_AnmManager->ExecuteScript(vm2);
    }

    result->frameTimer++;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#define RESULT_SCREEN_MAX_DISPLAYED_RESULTS 10

#pragma var_order(local_10, i, name, vm, node, local_2c, pos, x, spellCardIdx, difficulties, currentSpellcardNumber,   \
                  local_5c, local_60)
ChainCallbackResult ResultScreen::OnDraw(ResultScreen *result)
{
    AnmVm *vm = result->spriteVms;
    Float3 pos;
    Float3 local_10;
    i32 i;
    ScoreListNode *node;
    i32 local_2c;
    float x;
    char name[9];
    i32 spellCardIdx;
    i32 currentSpellcardNumber;
    const char *difficulties[5];
    float local_5c;
    float local_60;

    g_AnmManager->FlushVertexBuffer();
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    g_AnmManager->CopySurfaceToBackbuffer(0, 0, 0, 0, 0);

    for (i = 0; i < ARRAY_SIZE_SIGNED(result->spriteVms); i++, vm++)
    {
        pos = vm->pos;
        vm->pos += vm->pos2;

        g_AnmManager->DrawNoRotation(vm);

        vm->pos = pos;
    }

    vm = &result->spriteVms[RESULT_SCRIPT_LISTING];

    if (vm->pos[0] < 640.0f)
    {
        if (result->currentState != RESULT_SCREEN_STATE_SPELLCARDS)
        {
            pos = vm->pos;

            pos[1] += 18.0f;
            pos[0] += 24.0f;

            g_AsciiManager.SetColor(0xffe0e0ef);
            g_AsciiManager.AddFormatText(&pos, "No  Name       Score(Stage)   Date   Slow");

            pos[1] += 18.0f;

            node = result->scores[result->selectedDifficulty][result->selectedHighScoreCharacter].next;

            for (i = 0; i < RESULT_SCREEN_MAX_DISPLAYED_RESULTS; i++)
            {
                if (result->currentState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME)
                {
                    if (node->data->base.unk_9 != 0)
                    {
                        g_AsciiManager.SetColor(0xfff0f0ff);
                    }
                    else
                    {
                        g_AsciiManager.SetColor(0xc0ffc0c0);
                    }
                }
                else
                {
                    g_AsciiManager.SetColor(0xffffc0c0);
                }

                g_AsciiManager.AddFormatText(&pos, "%2d", i + 1);

                pos.x += 48.0f;

                if (result->currentState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME && node->data->base.unk_9 != 0)
                {
                    // Yes, I know this is so ugly, but there is no other way
                    // that I know of to get the codegen exact. ZUN must have
                    // written this.
                    *(i32 *)&name[0] = *(i32 *)"    ";
                    *(i32 *)&name[4] = *(i32 *)"    ";
                    name[8] = '\0';

                    name[result->cursor >= 8 ? 7 : result->cursor] = '_';

                    g_AsciiManager.AddFormatText(&pos, "%8s", name);
                }
                if (node->data->stage < 99)
                {
                    g_AsciiManager.AddFormatText(&pos, "%8s %9d%1d(%d)", node->data->name, node->data->score,
                                                 node->data->numRetries >= 10 ? 9 : node->data->numRetries,
                                                 g_ResultStageNumbers[node->data->stage]);
                }
                else
                {
                    g_AsciiManager.AddFormatText(&pos, "%8s %9d%1d(C)", node->data->name, node->data->score,
                                                 node->data->numRetries >= 10 ? 9 : node->data->numRetries);
                }

                pos.x += 320.0f;

                g_AsciiManager.AddFormatText(&pos, " %5s   %3.2f", node->data->date, node->data->lagPercentage);

                pos[1] += 18.0f;
                pos[0] -= 368.0f;

                node = node->next;
            }
        }
        else
        {
            // ZUN probably wrote this:
            // const char *difficulties[] = { "E", "N", "H", "L", "-" };

            difficulties[0] = "E";
            difficulties[1] = "N";
            difficulties[2] = "H";
            difficulties[3] = "L";
            difficulties[4] = "-";

            pos = vm->pos;

            result->textVms[10].pos = pos;
            result->textVms[10].pos.x += 320.0f;
            result->textVms[10].pos.y -= 16.0f;

            g_AnmManager->DrawNoRotation(&result->textVms[10]);

            pos[1] += 16.0f;

            for (i = 0; i < RESULT_SCREEN_MAX_DISPLAYED_RESULTS; i++)
            {
                spellCardIdx = (result->spellcardPage * RESULT_SCREEN_MAX_DISPLAYED_RESULTS) + i;

                if (spellCardIdx >= g_SpellcardCountsPerDifficulty[result->selectedSpellcardDifficulty])
                {
                    break;
                }

                x = pos[0];

                pos[0] += 320.0f;
                pos[1] += 16.0f;

                result->listingDividerSprite.pos = pos;
                result->listingDividerSprite.scale.x = 2.375f;
                g_AnmManager->DrawNoRotation(&result->listingDividerSprite);

                pos[1] -= 16.0f;
                pos[0] = x;

                result->textVms[i].pos = pos;

                currentSpellcardNumber =
                    g_SpellcardNumbersPerDifficulty[result->selectedSpellcardDifficulty][spellCardIdx];
                if (g_GameManager.catkData[currentSpellcardNumber].inGameHistory.attempts[result->previousShotType] ==
                    0)
                {
                    g_AsciiManager.SetColor(0xc0c0c0ff);
                }
                else if (g_GameManager.catkData[currentSpellcardNumber]
                             .inGameHistory.captures[result->previousShotType] == 0)
                {
                    g_AsciiManager.SetColor(0xffc0a0a0);
                }
                else
                {
                    g_AsciiManager.SetColor(0xfff0f0ff - (i * 0x80800));
                }

                g_AsciiManager.AddFormatText(&pos, "No.%.2d", currentSpellcardNumber + 1);

                result->textVms[i].pos[0] += 78.0f;
                g_AnmManager->DrawNoRotation(&result->textVms[i]);
                pos[0] += 446.0f;

                if (currentSpellcardNumber < SPELLCARD_LAST_WORD_START)
                {
                    if (g_GameManager.catkData[currentSpellcardNumber]
                            .inGameHistory.attempts[result->previousShotType] == 0)
                    {
                        g_AsciiManager.AddFormatText(&pos, "---/---(-)");
                    }
                    else
                    {
                        g_AsciiManager.AddFormatText(
                            &pos, "%3d/%3d(%s)",
                            g_GameManager.catkData[currentSpellcardNumber]
                                .inGameHistory.captures[result->previousShotType],
                            g_GameManager.catkData[currentSpellcardNumber]
                                .inGameHistory.attempts[result->previousShotType],
                            difficulties[g_GameManager.catkData[currentSpellcardNumber].difficulty]);
                    }
                }
                else if (g_GameManager.catkData[currentSpellcardNumber]
                             .spellPracticeHistory.attempts[result->previousShotType] == 0)
                {
                    g_AsciiManager.AddFormatText(&pos, "---/---(-)");
                }
                else
                {
                    g_AsciiManager.AddFormatText(
                        &pos, "%3d/%3d(%s)",
                        g_GameManager.catkData[currentSpellcardNumber]
                            .spellPracticeHistory.captures[result->previousShotType],
                        g_GameManager.catkData[currentSpellcardNumber]
                            .spellPracticeHistory.attempts[result->previousShotType],
                        difficulties[g_GameManager.catkData[currentSpellcardNumber].difficulty]);
                }

                pos[0] -= 446.0f;
                pos[0] += 424.0f;
                pos[1] -= 13.0f;

                g_AsciiManager.SetColor(0xffa08090);
                g_AsciiManager.SetScale(0.8f, 0.8f);

                if (g_GameManager.catkData[currentSpellcardNumber].inGameHistory.captures[result->previousShotType] !=
                    0)
                {
                    g_AsciiManager.AddFormatText(&pos, "MaxBonus %8d",
                                                 g_GameManager.catkData[currentSpellcardNumber]
                                                     .inGameHistory.maxBonus[result->previousShotType]);
                }

                pos[0] -= 424.0f;
                pos[1] += 13.0f;

                g_AsciiManager.SetScale(1.0f, 1.0f);

                if (!result->updateSpellcardResults)
                {
                    pos[1] += 33;
                }
                else if (result->frameTimer < 10)
                {
                    pos[1] += ((10 - result->frameTimer) * 33) / 10;
                }
                else
                {
                    pos[1] += ((result->frameTimer - 10) * 33) / 10;
                }
            }

            if (result->frameTimer >= 20)
            {
                result->updateSpellcardResults = FALSE;
            }
        }
    }

    if (result->currentState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME ||
        result->currentState == RESULT_SCREEN_STATE_WRITING_REPLAY_NAME)
    {
        pos = Float3(160.0f, 356.0f, 0.0f);
        char letter[16];

        for (i = 0; i < RESULT_KEYBOARD_ROWS; i++)
        {
            for (local_2c = 0; local_2c < RESULT_KEYBOARD_COLUMNS; local_2c++)
            {
                local_5c = 0;
                local_60 = 0;

                if (result->selectedCharacter == (i * RESULT_KEYBOARD_COLUMNS) + local_2c)
                {
                    g_AsciiManager.SetColor(0xffffffc0);

                    if (result->frameTimer % 64 < 32)
                    {
                        local_5c = ((result->frameTimer % 32) * 0.8f) / 32.0f + 1.2f;
                    }
                    else
                    {
                        local_5c = (2.0f - ((result->frameTimer % 32) * 0.8f) / 32.0f);
                    }

                    g_AsciiManager.SetScale(local_5c, local_5c);

                    local_60 = local_5c = -(local_5c - 1.0f) * 8.0f;
                }
                else
                {
                    g_AsciiManager.SetColor(0xc0c0c0c0);
                    g_AsciiManager.SetScale(1.0f, 1.0f);
                }

                local_10 = pos;
                local_10.x += local_5c;
                local_10.y += local_60;

                letter[0] = g_AlphabetList[(i * RESULT_KEYBOARD_COLUMNS) + local_2c];
                letter[1] = 0;

                if (i == 5)
                {
                    if (local_2c == 14)
                    {
                        letter[0] = 128;
                    }
                    else if (local_2c == 15)
                    {
                        letter[0] = 129;
                    }
                }

                g_AsciiManager.AddString(&local_10, letter);
                pos[0] += 20.0f;
            }

            pos[0] -= local_2c * 20;
            pos[1] += 18.0f;
        }
    }

    g_AsciiManager.SetScale(1.0f, 1.0f);

    if (result->currentState >= RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION &&
        result->currentState <= RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE)
    {
        vm = &result->spriteVms[RESULT_SCRIPT_REPLAY_SAVE_QUESTION];

        for (i = 0; i < 6; i++, vm++)
        {
            g_AnmManager->DrawNoRotation(vm);
        }

        vm = &result->spriteVms[RESULT_SCRIPT_REPLAY_LISTING_MAIN];

        pos = vm->pos;

        vm++;

        g_AsciiManager.AddFormatText(&pos, "No.   Name     Date   Player Score");

        for (i = 0; i < ARRAY_SIZE_SIGNED(result->replays); i++)
        {
            pos = vm->pos;
            vm++;

            if (i == result->selectedReplay)
            {
                g_AsciiManager.SetColor(0xffff8080);
            }
            else
            {
                g_AsciiManager.SetColor(0xff808080);
            }

            if (result->currentState == RESULT_SCREEN_STATE_WRITING_REPLAY_NAME)
            {
                g_AsciiManager.AddFormatText(
                    &pos, "No.%.2d %8s %5s  %7s %9d0", i + 1, result->lastName, result->currentReplay.date,
                    g_ResultsCharacterNames[g_GameManager.shotType + g_GameManager.fullShotType],
                    result->currentReplay.spellcardScore);

                g_AsciiManager.SetColor(0xfff0f0ff);

                // Yes, I know this is so ugly, but there is no other way
                // that I know of to get the codegen exact. ZUN must have
                // written this.
                *(i32 *)&name[0] = *(i32 *)"    ";
                *(i32 *)&name[4] = *(i32 *)"    ";
                name[8] = '\0';

                name[result->cursor >= 8 ? 7 : result->cursor] = '_';

                g_AsciiManager.AddFormatText(&pos, "      %8s", name);
            }
            else if (result->replays[i].header.magic != *(i32 *)REPLAY_MAGIC ||
                     (result->replays[i].header.version & 0xfff) != REPLAY_VERSION)
            {
                g_AsciiManager.AddFormatText(&pos, "No.%.2d -------- --/--  -------          0", i + 1);
            }
            else
            {
                g_AsciiManager.AddFormatText(
                    &pos, "No.%.2d %8s %5s  %7s %9d0", i + 1, result->replays[i].playerName, result->replays[i].date,
                    g_ResultsCharacterNames[result->replays[i].shotType], result->replays[i].spellcardScore);
            }
        }
    }

    g_AsciiManager.SetColor(COLOR_WHITE);
    result->DrawFinalStats();

    if (result->currentState == RESULT_SCREEN_STATE_OTHER_STATS_SCREEN_INIT ||
        result->currentState == RESULT_SCREEN_STATE_OTHER_STATS_SCREEN ||
        result->currentState == RESULT_SCREEN_STATE_OTHER_STATS_TO_INIT_TRANSITION)
    {
        vm = result->textVms;
        for (i = 0; i < 0x14; i++, vm++)
        {
            g_AnmManager->DrawNoRotation(vm);
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(i, vm, shotType, stage, catk, difficulty, spellcard)
ZunResult ResultScreen::AddedCallback(ResultScreen *result)
{
    AnmVm *vm;
    i32 i;
    i32 shotType;
    i32 stage;
    i32 difficulty;
    i32 spellcard;
    Catk *catk;

    // Even in PCB this doesn't make sense. It appears ZUN has an if
    // statement like this and commented out the body, and forgot to
    // remove the if statement
    if (g_GameManager.IsPhantasmUnlocked())
    {
    }

    for (i = 0; i < MAX_DIFFICULTIES; i++)
    {
        for (shotType = 0; shotType < SHOT_ALL; shotType++)
        {
            for (stage = 0; stage < MAX_STAGES_AND_LAST_WORD; stage++)
            {
                result->defaultScore[i][shotType][stage].score = 100000 - (10000 * stage);
                result->defaultScore[i][shotType][stage].lagPercentage = 0.0f;
                result->defaultScore[i][shotType][stage].base.magic = *(i32 *)"DMYS";
                result->defaultScore[i][shotType][stage].difficulty = i;
                result->defaultScore[i][shotType][stage].base.version = HSCR_VERSION;
                result->defaultScore[i][shotType][stage].base.unkLen = sizeof(Hscr);
                result->defaultScore[i][shotType][stage].base.th8kLen = sizeof(Hscr);
                result->defaultScore[i][shotType][stage].stage = STAGE1;
                result->defaultScore[i][shotType][stage].base.unk_9 = 0;
                result->defaultScore[i][shotType][stage].numRetries = 0;
                result->defaultScore[i][shotType][stage].unk0x166 = 1;

                result->LinkScoreEx(&result->defaultScore[i][shotType][stage], i, shotType);

                strcpy(result->defaultScore[i][shotType][stage].name, "--------");
                strcpy(result->defaultScore[i][shotType][stage].date, "--/--");
            }
        }
    }

    if (result->currentState != RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE)
    {
        if (g_AnmManager->LoadSurface(0, "result/result.jpg") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        result->resultAnm = g_AnmManager->LoadAnm(21, "result00.anm");
        if (result->resultAnm == NULL)
        {
            return ZUN_ERROR;
        }

        result->resultTextAnm = g_AnmManager->LoadAnm(22, "resulttext.anm");
        if (result->resultTextAnm == NULL)
        {
            return ZUN_ERROR;
        }

        for (vm = result->spriteVms, i = 0; i < ARRAY_SIZE_SIGNED(result->spriteVms); i++, vm++)
        {
            vm->pos = Float3(0.0f, 0.0f, 0.0f);
            vm->pos2 = Float3(0.0f, 0.0f, 0.0f);

            result->resultAnm->SetAndExecuteScriptIdx(vm, i);
        }

        result->resultAnm->InitializeAndSetSprite(&result->listingDividerSprite, 32);

        for (vm = result->textVms, i = 0; i < 14; i++, vm++)
        {
            g_Supervisor.textAnm->InitializeAndSetSprite(vm, i + 21);

            vm->pos = Float3(0.0f, 0.0f, 0.0f);
            vm->anchor = 3;
            vm->fontWidth = 15;
            vm->fontHeight = 15;
        }

        for (i = 0; i < 14; i++, vm++)
        {
            result->resultTextAnm->InitializeAndSetSprite(vm, i + 2);
            vm->pos = Float3(0.0f, 0.0f, 0.0f);
            vm->anchor = 3;
            vm->fontWidth = 15;
            vm->fontHeight = 15;
        }
    }

    result->unk0x20 = 0;
    result->scoreDat = ScoreDat::OpenScore("score.dat");

    for (i = 0; i < MAX_DIFFICULTIES; i++)
    {
        for (shotType = 0; shotType < SHOT_ALL; shotType++)
        {
            ScoreDat::GetHighScore(result->scoreDat, &result->scores[i][shotType], shotType, i, NULL);
        }
    }

    result->lsnm.base.magic = LSNM_MAGIC;
    result->lsnm.base.version = LSNM_VERSION;
    result->lsnm.base.unkLen = sizeof(Lsnm);
    result->lsnm.base.th8kLen = sizeof(Lsnm);

    strcpy(result->lsnm.name, "        ");

    result->lastNameSavedInScore = ScoreDat::ParseLSNM(result->scoreDat, &result->lsnm);

    if (result->currentState != RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME &&
        result->currentState != RESULT_SCREEN_STATE_PRACTICE &&
        result->currentState != RESULT_SCREEN_STATE_SPELL_PRACTICE &&
        result->currentState != RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE)
    {
        ScoreDat::ParseCATK(result->scoreDat, g_GameManager.catkData);
        ScoreDat::ParseCLRD(result->scoreDat, g_GameManager.clrdData);
        // Another of what appears to be an if statement with no block.
        if (g_GameManager.IsPhantasmUnlocked())
        {
        }
        ScoreDat::ParsePSCR(result->scoreDat, g_GameManager.pscrData);
    }

    if (result->currentState == RESULT_SCREEN_STATE_PRACTICE)
    {
        if (g_GameManager.pscrData[g_GameManager.shotType + g_GameManager.fullShotType]
                .highScores[g_GameManager.currentStage][g_GameManager.difficulty] < g_GameManager.globals->score)
        {
            g_GameManager.pscrData[g_GameManager.shotType + g_GameManager.fullShotType]
                .highScores[g_GameManager.currentStage][g_GameManager.difficulty] = g_GameManager.globals->score;
        }
        result->currentState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;

        strcpy(result->lastName, result->lsnm.name);

        if (g_Supervisor.IsSlowModeEnabled() || g_Supervisor.IsSpeedhackDetected())
        {
            memcpy(&g_GameManager.catkData, &g_GameManager.catkData2, sizeof(g_GameManager.catkData));
        }
    }

    if (result->currentState == RESULT_SCREEN_STATE_SPELL_PRACTICE)
    {
        result->currentState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;

        strcpy(result->lastName, result->lsnm.name);

        if (g_Supervisor.IsSlowModeEnabled() || g_Supervisor.IsSpeedhackDetected())
        {
            memcpy(&g_GameManager.catkData, &g_GameManager.catkData2, sizeof(g_GameManager.catkData));
        }
    }

    difficulty = 0;

    // Yes, difficulty is set to 0 twice.
    for (difficulty = 0; difficulty < MAX_DIFFICULTIES + 1; difficulty++)
    {
        for (i = 0; i < SHOT_ALL + 1; i++)
        {
            result->capturedSpellCards[difficulty][i] = 0;

            for (spellcard = 0; spellcard < g_SpellcardCountsPerDifficulty[difficulty]; spellcard++, catk++)
            {
                catk = &g_GameManager.catkData[g_SpellcardNumbersPerDifficulty[difficulty][spellcard]];

                if (catk->base.magic != CATK_MAGIC || catk->base.version != CATK_VERSION)
                {
                    continue;
                }

                if (catk->inGameHistory.captures[i] != 0 || catk->spellPracticeHistory.captures[i] != 0)
                {
                    result->capturedSpellCards[difficulty][i]++;
                }
            }
        }
    }

    result->selectedSpellcardDifficulty = 5;
    result->shotTypeCursor = SHOT_ALL;
    result->previousShotType = SHOT_ALL;
    result->updateSpellcardResults = FALSE;
    result->unk_10ef8.activeSpriteIndex = -1;

    g_GameManager.plst.playDataTotals.continues = g_GameManager.plst.playDataByDifficulty[EASY].continues +
                                                  g_GameManager.plst.playDataByDifficulty[NORMAL].continues +
                                                  g_GameManager.plst.playDataByDifficulty[HARD].continues +
                                                  g_GameManager.plst.playDataByDifficulty[LUNATIC].continues +
                                                  g_GameManager.plst.playDataByDifficulty[EXTRA].continues;

    g_GameManager.plst.playDataTotals.clears =
        g_GameManager.plst.playDataByDifficulty[EASY].clears + g_GameManager.plst.playDataByDifficulty[NORMAL].clears +
        g_GameManager.plst.playDataByDifficulty[HARD].clears + g_GameManager.plst.playDataByDifficulty[LUNATIC].clears +
        g_GameManager.plst.playDataByDifficulty[EXTRA].clears;

    if (result->currentState == RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE)
    {
        if (g_Supervisor.IsSlowModeEnabled() || g_Supervisor.IsSpeedhackDetected())
        {
            ScoreDat::ParseCATK(result->scoreDat, g_GameManager.catkData);
        }

        ResultScreen::DeletedCallback(result);

        return ZUN_ERROR;
    }

    result->selectedDifficulty = 1;
    result->selectedHighScoreCharacter = 0;

    return ZUN_SUCCESS;
}

#pragma var_order(difficulty, shotType)
ZunResult ResultScreen::DeletedCallback(ResultScreen *result)
{
    i32 difficulty;
    i32 shotType;

    if (result->scoreDat)
    {
        ResultScreen::WriteScore(result);
        ScoreDat::ReleaseScore(result->scoreDat);
    }

    result->scoreDat = NULL;

    for (difficulty = 0; difficulty < MAX_DIFFICULTIES; difficulty++)
    {
        for (shotType = 0; shotType < SHOT_ALL; shotType++)
        {
            result->FreeScore(difficulty, shotType);
        }
    }

    g_AnmManager->ReleaseAnm(21);
    g_AnmManager->ReleaseAnm(22);

    g_AnmManager->ReplaceSurface(8, 0);

    g_Chain.Cut(result->drawChain);
    result->drawChain = NULL;

    ZUN_DELETE(result);

    return ZUN_SUCCESS;
}

i32 ResultScreen::MoveCursor(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_SCROLLING(TH_BUTTON_UP))
    {
        resultScreen->cursor--;

        if (resultScreen->cursor < 0)
        {
            resultScreen->cursor += length;
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);

        return -1;
    }
    if (WAS_PRESSED_SCROLLING(TH_BUTTON_DOWN))
    {
        resultScreen->cursor++;

        if (resultScreen->cursor >= length)
        {
            resultScreen->cursor -= length;
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);

        return 1;
    }

    return 0;
}

i32 ResultScreen::MoveShotTypeCursor(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_SCROLLING(TH_BUTTON_UP))
    {
        resultScreen->shotTypeCursor--;
        if (resultScreen->shotTypeCursor < 0)
        {
            resultScreen->shotTypeCursor += length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        return -1;
    }
    else if (WAS_PRESSED_SCROLLING(TH_BUTTON_DOWN))
    {
        resultScreen->shotTypeCursor++;
        if (resultScreen->shotTypeCursor >= length)
        {
            resultScreen->shotTypeCursor -= length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        return 1;
    }

    return 0;
}

i32 ResultScreen::MoveCursorHorizontally(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_SCROLLING(TH_BUTTON_LEFT))
    {
        resultScreen->cursor--;
        if (resultScreen->cursor < 0)
        {
            resultScreen->cursor += length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        return -1;
    }
    else if (WAS_PRESSED_SCROLLING(TH_BUTTON_RIGHT))
    {
        resultScreen->cursor++;
        if (resultScreen->cursor >= length)
        {
            resultScreen->cursor -= length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        return 1;
    }

    return 0;
}

} /* namespace th08 */
