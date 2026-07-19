#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include "ReplayManager.hpp"
#include "ScoreDat.hpp"

namespace th08
{

enum ResultScreenState
{
    RESULT_SCREEN_STATE_INIT                            = 0,
    RESULT_SCREEN_STATE_CHOOSING_CATEGORY               = 1,
    RESULT_SCREEN_STATE_EXITING                         = 2,
    RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_DIFFICULTY = 3,
    RESULT_SCREEN_STATE_BEST_SCORES_CHOOSING_CHARACTER  = 4,
    RESULT_SCREEN_STATE_BEST_SCORES                     = 5,
    RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_DIFFICULTY  = 6,
    RESULT_SCREEN_STATE_SPELLCARDS_CHOOSING_CHARACTER   = 7,
    RESULT_SCREEN_STATE_SPELLCARDS                      = 8,
    RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME          = 9,
    RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION            = 10,
    RESULT_SCREEN_STATE_CANT_SAVE_REPLAY                = 11,
    RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE            = 12,
    RESULT_SCREEN_STATE_WRITING_REPLAY_NAME             = 13,
    RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE           = 14,
    RESULT_SCREEN_STATE_STATS_SCREEN                    = 15,
    RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION        = 16,
    RESULT_SCREEN_STATE_PRACTICE                        = 17,
    RESULT_SCREEN_STATE_INITIAL_SCORE_SAVE              = 18,
    RESULT_SCREEN_STATE_OTHER_STATS_SCREEN_INIT         = 19,
    RESULT_SCREEN_STATE_OTHER_STATS_SCREEN              = 20,
    RESULT_SCREEN_STATE_OTHER_STATS_TO_INIT_TRANSITION  = 21,
    RESULT_SCREEN_STATE_SPELL_PRACTICE                  = 22,
};

#define RESULT_REPLAY_MAX_RESULTS                           15

struct ResultScreen
{
    ResultScreen()
    {
        memset(this, 0, sizeof(ResultScreen));
    }

    ~ResultScreen()
    {
        g_ZunMemory.Free(this->scoreDat);
    }

    static const char *GetStageName(i32 stage);
    static const char *GetCharacterName(i32 character);
    static void WriteScore(ResultScreen *resultScreen);
    static void LogScoreDataToFile(ResultScreen *resultScreen);
    i32 LinkScoreEx(Hscr *out, i32 difficulty, i32 character);
    void FreeScore(i32 difficulty, i32 character);
    i32 HandleCategorySelectScreen();

    void SetState(ResultScreenState state)
    {
        this->previousState = this->currentState;
        this->currentState = state;
        this->currentState2 = state;
        this->menuDepth = 0;
        this->frameTimer2 = 0;
        this->frameTimer = 0;
        this->exitingSpellcardResults = 0;
    }

    i32 HandleHighScoreDifficultySelect();
    i32 HandleHighScoreCharacterSelect();
    i32 HandleHighScoreScreen();
    i32 HandleSpellCardDifficultySelect();
    i32 HandleSpellCardCharacterSelect();
    i32 HandleSpellCardScreen();
    i32 HandleResultKeyboard();

    static void FormatDate(char *buffer);

    i32 HandleReplaySaveKeyboard();
    ZunResult CheckConfirmButton();
    i32 HandleOtherStatsScreen();
    i32 DrawFinalStats();

    static ZunResult RegisterChain(u32 unk);
    static ChainCallbackResult OnUpdate(ResultScreen *resultScreen);
    static ChainCallbackResult OnDraw(ResultScreen *resultScreen);
    static ZunResult AddedCallback(ResultScreen *resultScreen);
    static ZunResult DeletedCallback(ResultScreen *resultScreen);

    static i32 MoveCursor(ResultScreen *resultScreen, i32 length);
    static i32 MoveShotTypeCursor(ResultScreen *resultScreen, i32 length);
    static i32 MoveCursorHorizontally(ResultScreen *resultScreen, i32 length);

    ScoreDat *scoreDat;
    i32 frameTimer;
    ResultScreenState currentState;

    ResultScreenState currentState2;
    i32 menuDepth;
    ResultScreenState previousState;
    i32 frameTimer2;
    i32 cursor;
    i32 unk0x20;                        // is set to 0 when loading the score, never used
    i32 unk0x24;                        // unused
    i32 selectedReplay;
    i32 selectedCharacter;
    i32 shotTypeCursor;
    i32 previousShotType;
    ZunBool updateSpellcardResults;
    i32 selectedHighScoreCharacter;
    i32 spellcardPage;
    i32 selectedDifficulty;
    i32 selectedSpellcardDifficulty;

    i32 cheatCodeStep;
    ZunBool lastNameSavedInScore;

    ZunBool exitingSpellcardResults;

    char lastName[9];

    i32 capturedSpellCards[MAX_DIFFICULTIES + 1][SHOT_ALL + 1];

    u8 totalSeconds;

    AnmVm spriteVms[72];
    AnmVm textVms[30];
    AnmVm unk_10ef8;            // unused here and in PCB
    AnmVm listingDividerSprite;

    AnmLoaded *resultAnm;
    AnmLoaded *resultTextAnm;

    u32 unk0x11448;             // unused

    ScoreListNode scores[MAX_DIFFICULTIES][SHOT_ALL];
    Hscr defaultScore[MAX_DIFFICULTIES][SHOT_ALL][MAX_STAGES_AND_LAST_WORD];
    Hscr hscr;
    Th8k fileHeader;
    Lsnm lsnm;

    ChainElem *calcChain;
    ChainElem *drawChain;

    ReplayData replays[RESULT_REPLAY_MAX_RESULTS];
    ReplayData currentReplay;
};

C_ASSERT(sizeof(ResultScreen) == 0x477b0);

}; // namespace th08
