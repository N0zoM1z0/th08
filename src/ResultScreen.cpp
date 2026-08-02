#include "th_pch.h"

#include "ResultScreen.hpp"

namespace th08
{

// STUB: th08 0x453cd1
const char *ResultScreen::GetStageName(i32 stage)
{
    return NULL;
}

// STUB: th08 0x453cfa
const char *ResultScreen::GetCharacterName(i32 character)
{
    return NULL;
}

// STUB: th08 0x453d0d
void ResultScreen::WriteScore(ResultScreen *resultScreen)
{
}

// STUB: th08 0x454298
void ResultScreen::LogScoreDataToFile(ResultScreen *resultScreen)
{
}

// STUB: th08 0x454c59
void ResultScreen::LinkScoreEx(void *out, int difficulty, i32 character)
{
}

// STUB: th08 0x454c87
void ResultScreen::FreeScore(i32 difficulty, i32 character)
{
}

// STUB: th08 0x454cb2
void ResultScreen::HandleCategorySelectScreen()
{
}

// STUB: th08 0x4550b7
void ResultScreen::SetState(ResultScreenState state)
{
}

// STUB: th08 0x4550fc
i32 ResultScreen::HandleHighScoreDifficultySelect()
{
    return 0;
}

// STUB: th08 0x45567d
i32 ResultScreen::HandleHighScoreCharacterSelect()
{
    return 0;
}

// STUB: th08 0x455925
i32 ResultScreen::HandleHighScoreScreen()
{
    return 0;
}

// STUB: th08 0x455a33
i32 ResultScreen::HandleSpellCardDifficultySelect()
{
    return 0;
}

// STUB: th08 0x455cb0
i32 ResultScreen::HandleSpellCardCharacterSelect()
{
    return 0;
}

// STUB: th08 0x455f6b
i32 ResultScreen::HandleSpellCardScreen()
{
    return 0;
}

// STUB: th08 0x45621e
i32 ResultScreen::HandleResultKeyboard()
{
    return 0;
}

// STUB: th08 0x456938
void ResultScreen::FormatDate(char *buffer)
{
}

// STUB: th08 0x45696f
i32 ResultScreen::HandleReplaySaveKeyboard()
{
    return 0;
}

// STUB: th80 0x4577e2
ZunResult ResultScreen::CheckConfirmButton()
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x4578aa
i32 ResultScreen::HandleOtherStatsScreen()
{
    return 0;
}

// STUB: th08 0x457e00
i32 ResultScreen::DrawFinalStats()
{
    return 0;
}

// STUB: th08 0x4582a0
ZunResult ResultScreen::RegisterChain(u32 unk)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x4584b0
ChainCallbackResult ResultScreen::OnUpdate(ResultScreen *resultScreen)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x4586b4
ChainCallbackResult ResultScreen::OnDraw(ResultScreen *resultScreen)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x45964d
ZunResult ResultScreen::AddedCallback(ResultScreen *resultScreen)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x459fd2
ZunResult ResultScreen::DeletedCallback(ResultScreen *resultScreen)
{
    return ZUN_SUCCESS;
}

// STUB: th08 0x45a0f4
i32 ResultScreen::MoveCursor(ResultScreen *resultScreen, i32 length)
{
    return 0;
}

// STUB: th08 0x45a1f3
i32 ResultScreen::MoveShotTypeCursor(ResultScreen *resultScreen, i32 length)
{
    return 0;
}

// STUB: th08 0x45a2f2
i32 ResultScreen::MoveCursorHorizontally(ResultScreen *resultScreen, int length)
{
    return 0;
}

} /* namespace th08 */
