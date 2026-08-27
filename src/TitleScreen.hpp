#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"
#include "ReplayManager.hpp"
#include "ZunResult.hpp"

#define TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE 15
#define TITLE_REPLAYS_PER_PAGE 15

namespace th08
{

enum TitleScreenRegistrationReason
{
    TITLE_SCREEN_REGISTER_STANDARD = 0,
    TITLE_SCREEN_REGISTER_AFTER_REPLAY = 1,
};

enum TitleCurrentScreen
{
    TitleCurrentScreen_StartMenu = 0,
    TitleCurrentScreen_Option = 2,
    TitleCurrentScreen_KeyConfig = 3,
    TitleCurrentScreen_DifficultySelect = 4,
    TitleCurrentScreen_CharacterSelect = 5,
    TitleCurrentScreen_ShotSelect = 6, /* Leftover from PCB */
    TitleCurrentScreen_Replay = 7,
    TitleCurrentScreen_DifficultySelectPractice = 8,
    TitleCurrentScreen_CharacterSelectPractice = 9,
    TitleCurrentScreen_ShotSelectPractice = 10, /* Leftover from PCB */
    TitleCurrentScreen_PracticeStageSelect = 11,
    TitleCurrentScreen_DifficultySelectExtra = 12,
    TitleCurrentScreen_CharacterSelectExtra = 13,
    TitleCurrentScreen_ShotSelectExtra = 14,      /* Leftover from PCB */
    TitleCurrentScreen_CharacterSelectSpell = 15, /* Seems to be an earlier feature, the current scene
                                                   * is never set to this value, but it checks against
                                                   * this value.
                                                   */
    TitleCurrentScreen_SpellStageSelect = 16,
    TitleCurrentScreen_SpellCardSelect = 17,
};

enum TitleScreenState
{
    TitleScreenState_Ready = 0,
    TitleScreenState_Loading = 1,
    TitleScreenState_Close = 2,
};

enum TitleCurrentScreenState
{
    TitleCurrentScreenState_Init = 0,
    TitleCurrentScreenState_Ready = 1,
    TitleCurrentScreenState_Exit = 2,
    TitleCurrentScreenState_Changing = 3,
};

#define TITLE_MAX_REPLAYS 60

struct TitleScreen
{
    TitleScreen()
    {
        memset(this, 0, sizeof(TitleScreen));
    }

    static ChainCallbackResult OnUpdate(TitleScreen *titleScreen);
    ChainCallbackResult OnUpdateStartMenu();
    ChainCallbackResult OnUpdateOptions();
    ChainCallbackResult OnUpdateKeyConfig();
    ChainCallbackResult OnUpdateDifficultySelect();
    ChainCallbackResult OnUpdateCharacterSelect();
    ChainCallbackResult OnUpdatePracticeStageSelect();
    ChainCallbackResult OnUpdateSpellStageSelect();
    ChainCallbackResult OnUpdateSpellCardSelect();

    void UnlockLastWordSpellCards();

    void FormatSpellCardInfo();
    ChainCallbackResult OnUpdateReplayMenu();

    ChainCallbackResult DrawReplayMenu();
    ChainCallbackResult DrawPracticeStageSelect();
    ChainCallbackResult DrawSpellStageSelect();
    ChainCallbackResult DrawSpellCardSelect();

    i32 MoveCursorVertical(i32 menuLength);
    i32 MoveCursorHorizontal(i32 menuLength);

    ChainCallbackResult DrawCompletionStatusText();

    static ChainCallbackResult OnDraw(TitleScreen *titleScreen);
    ZunResult ActualAddedCallback();
    static void TitleSetupThread(TitleScreen *titleScreen);
    static void DisplayInfoImage(const char *path);
    // The retail function homes this reason but never reads it. Keep the i32
    // parameter for the original VC7 ABI and to document its two callers.
    static ZunResult RegisterChain(i32 registrationReason);

    static ZunResult AddedCallback(TitleScreen *titleScreen);
    static ZunResult DeletedCallback(TitleScreen *titleScreen);

    ZunResult Release();

    void ChangeCurrentScreen(TitleCurrentScreen newScreen)
    {
        this->previousScreen = this->currentScreen;
        this->currentScreen = newScreen;
        this->stateTimer = 0;
        this->stateTimer2 = 0;
        this->currentScreenState = TitleCurrentScreenState_Init;
        this->idleFrames = 0;
    }

    ZunResult SetKeyNumberSprite(AnmVm *vms, i16 key);

    void SetKeyConfigKey(i16 keyToChange, i16 newKey, i32 unused)
    {
        if (this->controllerMapping.shotButton == keyToChange)
        {
            this->controllerMapping.shotButton = newKey;
        }
        if (this->controllerMapping.bombButton == keyToChange)
        {
            this->controllerMapping.bombButton = newKey;
        }
        if (this->controllerMapping.focusButton == keyToChange)
        {
            this->controllerMapping.focusButton = newKey;
        }
        if (this->controllerMapping.upButton == keyToChange)
        {
            this->controllerMapping.upButton = newKey;
        }
        if (this->controllerMapping.downButton == keyToChange)
        {
            this->controllerMapping.downButton = newKey;
        }
        if (this->controllerMapping.leftButton == keyToChange)
        {
            this->controllerMapping.leftButton = newKey;
        }
        if (this->controllerMapping.rightButton == keyToChange)
        {
            this->controllerMapping.rightButton = newKey;
        }
        if (this->controllerMapping.menuButton == keyToChange)
        {
            this->controllerMapping.menuButton = newKey;
        }
        if (this->controllerMapping.skipButton == keyToChange)
        {
            this->controllerMapping.skipButton = newKey;
        }
    }

    i32 cursor;
    i32 currentPageSpellCardSelect;
    i32 cursor2;
    i32 currentScreenState;
    i32 stateTimer;
    i32 unk0x14;
    u8 padding[80];

    TitleCurrentScreen previousScreen;
    u32 practiceState;

    char replayFilePaths[TITLE_MAX_REPLAYS][512];
    char replayNumbers[TITLE_MAX_REPLAYS][8];
    ReplayData replays[TITLE_MAX_REPLAYS];
    ReplayData *currentReplay;
    i32 unk0xc284;
    i32 replayCount;
    i32 selectedReplay;
    i32 selectedReplayStage;
    i32 idleFrames;
    i32 currentNumberOfSpellCards;
    i32 spellCardInfoRevealCountdown;
    float percentageCapturedSpellPracticePerShot;
    float percentageCapturedInGamePerShot;
    float percentageCapturedSpellPractice;
    float percentageCapturedInGame;

    AnmLoaded *titleAnm;
    AnmLoaded *resultTextAnm;
    AnmVm *vms;
    AnmVm *currentHelpTextVm;
    AnmVm helpTextVms[14];
    AnmVm spellCardNameVms[TITLE_SPELL_CARD_SPELLCARDS_PER_PAGE + 6];
    AnmVm spellCardInfoVms[14];
    i32 vmCount;

    TitleCurrentScreen currentScreen;
    i32 stateTimer2;
    i32 startMenuIdleFrames;
    TitleScreenState state;
    ChainElem *calcChain;
    ChainElem *drawChain;
    ControllerMapping controllerMapping;
    GameConfiguration currentGameConfig;
};
C_ASSERT(offsetof(TitleScreen, spellCardInfoRevealCountdown) == 0xc29c);

} // namespace th08
