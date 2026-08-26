#include "th_pch.h"

#include "AnmManager.hpp"
#include "Ending.hpp"
#include "GameManager.hpp"
#include "ZunColor.hpp"
#include "i18n.hpp"
#include <cstdlib>

namespace th08
{

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_EndingFiles[3]) = {
    {"end00a.end", "end01a.end", "end02a.end", "end03a.end", "end00a.end", "end00a.end", "end01a.end", "end01a.end",
     "end02a.end", "end02a.end", "end03a.end", "end03a.end"},

    {"end00b.end", "end01b.end", "end02b.end", "end03b.end", "end00b.end", "end00b.end", "end01b.end", "end01b.end",
     "end02b.end", "end02b.end", "end03b.end", "end03b.end"},

    {"end00c.end", "end01c.end", "end02c.end", "end03c.end", "end00c.end", "end00c.end", "end01c.end", "end01c.end",
     "end02c.end", "end02c.end", "end03c.end", "end03c.end"}};

ZunResult Ending::ReadScriptParameter()
{
    i32 param;

    param = atol(this->scriptCursor);

    while (*this->scriptCursor != '\0')
    {
        this->scriptCursor++;
    }
    while (*this->scriptCursor == '\0')
    {
        this->scriptCursor++;
    }

    return (ZunResult)param;
}

#pragma var_order(rect, alpha)
void Ending::UpdateAndDrawFade()
{
    ZunRect rect;
    i32 alpha;

    rect.left = 0.0f;
    rect.top = 0.0f;
    rect.right = (f32)GAME_WINDOW_WIDTH;
    rect.bottom = (f32)GAME_WINDOW_HEIGHT;

    switch (this->fadeMode)
    {
    case ENDING_FADE_TYPE_FADE_IN_BLACK:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeMode = ENDING_FADE_TYPE_NO_FADE;
            this->fadeColor = 0;
            break;
        }
        else
        {
            alpha = 255 - (this->fadeTimer * 255) / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA(COLOR_BLACK, alpha);
            this->fadeTimer++;
            break;
        }

    case ENDING_FADE_TYPE_FADE_OUT_BLACK:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = COLOR_BLACK;
            break;
        }
        else
        {
            alpha = (this->fadeTimer * 255) / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA(COLOR_BLACK, alpha);
            this->fadeTimer++;
            break;
        }

    case ENDING_FADE_TYPE_FADE_IN_WHITE:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeMode = ENDING_FADE_TYPE_NO_FADE;
            this->fadeColor = 0;
            break;
        }
        else
        {
            alpha = 255 - (this->fadeTimer * 255) / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA3(COLOR_WHITE, alpha);
            this->fadeTimer++;
            break;
        }

    case ENDING_FADE_TYPE_FADE_OUT_WHITE:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = COLOR_WHITE;
            break;
        }
        else
        {
            alpha = (this->fadeTimer * 255) / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA3(COLOR_WHITE, alpha);
            this->fadeTimer++;
            break;
        }

    case ENDING_FADE_TYPE_NO_FADE:
        this->fadeColor = 0;
        break;
    }

    if ((this->fadeColor & COLOR_ALPHA_MASK) != 0)
    {
        ScreenEffect::DrawSquare(&rect, this->fadeColor);
    }
}

#pragma var_order(lineEndDisplayed, textBuffer, index, i, anmScriptIdx, vmIndex, anmSpriteIdx, firstRead, secondRead,  \
                  spriteIdx, fadeTime)
ZunResult Ending::RunEndingScript()
{
    char textBuffer[68];
    i32 index;
    ZunBool lineEndDisplayed;
    i32 i;

    i32 vmIndex;
    i32 anmScriptIdx;
    i32 anmSpriteIdx;
    i32 firstRead;
    i32 secondRead;
    i32 spriteIdx;
    f32 fadeTime;

    lineEndDisplayed = false;
    index = 0;

    memset(textBuffer, 0, sizeof(textBuffer));

    if (this->pageWaitTimer > 0)
    {
        this->pageWaitTimer--;

        if (this->pageSkipLockFrames != 0)
        {
            this->pageSkipLockFrames--;
        }
        else
        {
            if ((WAS_PRESSED(TH_BUTTON_SELECTMENU)) || ((this->hasSeenEnding && (IS_PRESSED(TH_BUTTON_SKIP) != 0))))
            {
                this->pageWaitTimer = 0;
            }
        }

        if (this->pageWaitTimer <= 0)
        {
            for (i = 0; i < ARRAY_SIZE_SIGNED(this->endingVms) - 1; i++)
            {
                this->endingVms[i].SetInterrupt(2);
            }
            this->nextTextVmIndex = 0;
        }
        else
        {
            goto end_of_parse;
        }
    }

    if (this->lineWaitTimer > 0)
    {
        this->lineWaitTimer--;
        if (this->lineSkipLockFrames != 0)
        {
            this->lineSkipLockFrames--;
        }
        else
        {
            if ((WAS_PRESSED(TH_BUTTON_SELECTMENU)) || ((this->hasSeenEnding && (IS_PRESSED(TH_BUTTON_SKIP) != 0))))
            {
                this->lineWaitTimer = 0;
            }
        }
        goto end_of_parse;
    }

    while (true)
    {
        switch (*this->scriptCursor)
        {
        case END_READ_OPCODE:

            this->scriptCursor++;
            switch (*this->scriptCursor)
            {
            case END_OPCODE_BACKGROUND:
                if (g_AnmManager->LoadSurface(0, (this->scriptCursor + 1)))
                {
                    return ZUN_ERROR;
                }
                break;

            case END_OPCODE_EXECUTE_ANM:
                this->scriptCursor++;
                vmIndex = this->ReadScriptParameter();
                anmScriptIdx = this->ReadScriptParameter();
                anmSpriteIdx = this->ReadScriptParameter();
                this->endingAnm->ExecuteAnmIdx(&this->endingVms[vmIndex], anmScriptIdx);
                this->endingAnm->SetSprite(&this->endingVms[vmIndex], anmSpriteIdx);
                break;

            case END_OPCODE_SCROLL_BACKGROUND:
                this->scriptCursor++;
                firstRead = this->ReadScriptParameter();
                secondRead = this->ReadScriptParameter();
                this->backgroundScrollSpeed = (f32)firstRead / (f32)secondRead;
                break;

            case END_OPCODE_SET_VERTICAL_SCROLL_POS:
                this->scriptCursor++;
                this->backgroundPos.y = (f32)this->ReadScriptParameter();
                break;

            case END_OPCODE_EXEC_END_FILE:
                if (this->LoadEndingScript(this->scriptCursor + 1) != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
                index = 0;
                lineEndDisplayed = false;
                this->hasSeenEnding = this->canSkipChainedEnding;

            case END_OPCODE_ROLL_STAFF:
                for (spriteIdx = 0; spriteIdx < ARRAY_SIZE_SIGNED(this->endingVms); spriteIdx++)
                {
                    this->endingVms[spriteIdx].scriptIndex = 0;
                }
                break;

            case END_OPCODE_PLAY_MUSIC:
                g_Supervisor.LoadMusic(0, this->scriptCursor + 1);
                g_Supervisor.PlayMusic(0, 0);
                break;

            case END_OPCODE_FADE_MUSIC:
                this->scriptCursor++;
                fadeTime = (f32)this->ReadScriptParameter();
                g_Supervisor.FadeOutMusic(fadeTime);
                break;

            case END_OPCODE_SET_DELAY:
                this->scriptCursor++;
                this->defaultLineWaitFrames = this->ReadScriptParameter();
                this->minimumLineWaitFrames = this->ReadScriptParameter();
                break;

            case END_OPCODE_COLOR:
                this->scriptCursor++;
                this->textColor = this->ReadScriptParameter();
                break;

            case END_OPCODE_WAIT_RESET:
                this->scriptCursor++;
                this->pageWaitTimer = this->ReadScriptParameter();
                this->pageSkipLockFrames = this->ReadScriptParameter();
                this->lineWaitTimer = 0;
                this->lineSkipLockFrames = 0;
                while (*this->scriptCursor != '\n' && *this->scriptCursor != '\r')
                {
                    this->scriptCursor++;
                }
                while (*this->scriptCursor == '\n' || *this->scriptCursor == '\r')
                {
                    this->scriptCursor++;
                }
                goto end_of_parse;

            case END_OPCODE_WAIT:
                this->scriptCursor++;
                this->lineWaitTimer = this->ReadScriptParameter();
                this->lineSkipLockFrames = this->ReadScriptParameter();
                while (*this->scriptCursor != '\n' && *this->scriptCursor != '\r')
                {
                    this->scriptCursor++;
                }
                while (*this->scriptCursor == '\n' || *this->scriptCursor == '\r')
                {
                    this->scriptCursor++;
                }
                goto end_of_parse;

            case END_OPCODE_FADE_IN_BLACK:
                this->scriptCursor++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_IN_BLACK;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadScriptParameter();
                break;

            case END_OPCODE_FADE_OUT_BLACK:
                this->scriptCursor++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_OUT_BLACK;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadScriptParameter();
                break;

            case END_OPCODE_FADE_IN:
                this->scriptCursor++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_IN_WHITE;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadScriptParameter();
                break;

            case END_OPCODE_FADE_OUT:
                this->scriptCursor++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_OUT_WHITE;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadScriptParameter();
                break;

            case END_OPCODE_END:
                return ZUN_ERROR;
            }
            while (*this->scriptCursor != '\n' && *this->scriptCursor != '\r')
            {
                this->scriptCursor++;
            }
            while (*this->scriptCursor == '\n' || *this->scriptCursor == '\r')
            {
                this->scriptCursor++;
            }
            break;

        case '\0':
        case '\n':
        case '\r':
            if (index != 0)
            {
                g_AnmManager->DrawTextLeft(&this->endingVms[this->nextTextVmIndex], this->textColor, COLOR_WHITE,
                                           textBuffer);
                this->endingVms[this->nextTextVmIndex].SetInterrupt(1);
            }
            while (*this->scriptCursor == '\n' || *this->scriptCursor == '\0' || *this->scriptCursor == '\r')
            {
                this->scriptCursor++;
            }
            if (IS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                this->lineWaitTimer = this->minimumLineWaitFrames;
                this->lineSkipLockFrames = this->minimumLineWaitFrames;
            }
            else
            {
                this->lineWaitTimer = this->defaultLineWaitFrames;
                this->lineSkipLockFrames = this->minimumLineWaitFrames;
            }
            this->nextTextVmIndex++;
            goto end_of_parse;

        default:
            textBuffer[index] = *this->scriptCursor;
            textBuffer[index + 1] = *(this->scriptCursor + 1);
            index += 2;
            this->scriptCursor += 2;
            break;

        } // switch(*this->scriptCursor)
    } // while(true)

end_of_parse:
    this->elapsedTimer++;
    this->backgroundPos.y -= this->backgroundScrollSpeed;

    if (this->backgroundPos.y <= 0.0f)
    {
        this->backgroundPos.y = 0.0f;
        this->backgroundScrollSpeed = 0.0f;
    }
    return ZUN_SUCCESS;
}

ZunResult Ending::LoadEndingScript(const char *path)
{
    char *prevFile = this->scriptData;
    this->scriptData = (char *)FileSystem::OpenFile(path, NULL, 0);

    if (this->scriptData == NULL)
    {
        this->scriptData = prevFile;
        g_GameErrorContext.Log(TH_ERR_ENDING_FILE_UNREADABLE);
        return ZUN_ERROR;
    }

    this->scriptCursor = this->scriptData;
    this->defaultLineWaitFrames = 8;
    this->lineWaitTimer = 0;
    this->elapsedTimer = 0;

    if (prevFile != NULL)
    {
        g_ZunMemory.Free(prevFile);
    }
    return ZUN_SUCCESS;
}

ZunResult Ending::RegisterChain()
{
    Ending *ending = ZUN_NEW(Ending, "EndingInf");

    ending->calcChain = g_Chain.CreateElem((ChainCallback)Ending::OnUpdate);
    ending->calcChain->arg = ending;
    ending->calcChain->addedCallback = (ChainLifetimeCallback)Ending::AddedCallback;
    ending->calcChain->deletedCallback = (ChainLifetimeCallback)Ending::DeletedCallback;

    if (g_Chain.AddToCalcChain(ending->calcChain, 5) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    ending->drawChain = g_Chain.CreateElem((ChainCallback)Ending::OnDraw);
    ending->drawChain->arg = ending;
    g_Chain.AddToDrawChain(ending->drawChain, 4);
    return ZUN_SUCCESS;
}

Ending::Ending()
{
    memset(this, 0, sizeof(Ending));
    this->defaultLineWaitFrames = 8;
    this->lineWaitTimer = 0;
    this->elapsedTimer = 0;
    this->backgroundPos.x = 0;
    this->backgroundPos.y = 0;
    this->backgroundScrollSpeed = 0;
}

ChainCallbackResult Ending::OnUpdate(Ending *ending)
{
    i32 frameSkip = 0;

loop:
    if (ending->RunEndingScript() != ZUN_SUCCESS)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(ending->endingVms) - 1; i++)
    {
        g_AnmManager->ExecuteScript(&ending->endingVms[i]);
    }

    if (ending->hasSeenEnding)
    {
        if (IS_PRESSED(TH_BUTTON_SKIP) != 0)
        {
            if (frameSkip < 8)
            {
                frameSkip++;
                goto loop;
            }
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Ending::OnDraw(Ending *ending)
{
    i32 idx;

    g_AnmManager->CopySurfaceToBackbuffer2(0, 0, 0, ending->backgroundPos.x, ending->backgroundPos.y, GAME_WINDOW_WIDTH,
                                           GAME_WINDOW_HEIGHT);
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(ending->endingVms) - 1; idx++)
    {
        g_AnmManager->Draw2D(&ending->endingVms[idx]);
    }
    ending->UpdateAndDrawFade();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(endingFile, shotType1, shotType2, stageBit, i)
ZunResult Ending::AddedCallback(Ending *ending)
{
    const char *endingFile;
    i32 shotType1;
    i32 shotType2;
    u32 stageBit;
    i32 i;

    shotType1 = g_GameManager.shotType;

    g_AnmManager->ClearTexture();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearVertexShader();
    ScreenEffect::Clear(COLOR_WHITE);
    g_Supervisor.suppressFpsDisplay = TRUE;

    ending->endingAnm = g_AnmManager->LoadAnm(0x18, "staff01.anm");

    if (g_GameManager.flags.gameCleared)
    {

        shotType2 = g_GameManager.shotType;

        stageBit = (g_GameManager.currentStage != STAGE6B) ? SPELL_PRACTICE_UNLOCKED_FLAG : EXTRA_UNLOCKED_FLAG;

        g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].clears++;
        g_GameManager.plst.playDataTotals.clears++;

        ending->hasSeenEnding = false;
        ending->canSkipChainedEnding = 0;

        if ((g_GameManager.currentStage == STAGE6B && g_GameManager.IsExtraUnlocked()) ||
            (g_GameManager.currentStage == STAGE6A && g_GameManager.IsSpellPracticeUnlocked()))
        {
            ending->canSkipChainedEnding = 1;
        }

        if ((g_GameManager.globals)->numRetries == 0)
        {
            if ((g_GameManager.clrdData[shotType2].difficultiesClearedWithoutRetries[EASY] & stageBit) ||
                (g_GameManager.clrdData[shotType2].difficultiesClearedWithoutRetries[NORMAL] & stageBit) ||
                (g_GameManager.clrdData[shotType2].difficultiesClearedWithoutRetries[HARD] & stageBit) ||
                (g_GameManager.clrdData[shotType2].difficultiesClearedWithoutRetries[LUNATIC] & stageBit))
            {
                ending->hasSeenEnding = true;
            }
            g_GameManager.clrdData[shotType2].difficultiesClearedWithoutRetries[g_GameManager.difficulty] |= stageBit;
            g_GameManager.clrdData[SHOT_ALL].difficultiesClearedWithoutRetries[g_GameManager.difficulty] |= stageBit;
        }
        else
        {
            if ((g_GameManager.clrdData[shotType2].difficultiesClearedWithRetries[EASY] & stageBit) ||
                (g_GameManager.clrdData[shotType2].difficultiesClearedWithRetries[NORMAL] & stageBit) ||
                (g_GameManager.clrdData[shotType2].difficultiesClearedWithRetries[HARD] & stageBit) ||
                (g_GameManager.clrdData[shotType2].difficultiesClearedWithRetries[LUNATIC] & stageBit))
            {
                ending->hasSeenEnding = true;
            }
        }

        g_GameManager.clrdData[shotType2].difficultiesClearedWithRetries[g_GameManager.difficulty] |= stageBit;
        g_GameManager.clrdData[SHOT_ALL].difficultiesClearedWithRetries[g_GameManager.difficulty] |= stageBit;

        g_GameManager.plst.bgmUnlocked[18] = 1;
        g_GameManager.plst.bgmUnlocked[19] = 1;
    }
    else
    {
        ending->hasSeenEnding = g_GameManager.clrdData[g_GameManager.shotType].pendingEndingSkip;
        g_GameManager.clrdData[g_GameManager.shotType].pendingEndingSkip = 0;
        g_GameManager.plst.bgmUnlocked[18] = 0x12;
    }

execute_anms:
    for (i = 0; i < ARRAY_SIZE_SIGNED(ending->endingVms) - 1; i++)
    {
        g_Supervisor.textAnm->ExecuteAnmIdx(&ending->endingVms[i], i + 0x12);
        ending->endingVms[i].pos = Float3(64.0f, i * 16.0f + 400.0f, 0.0f);
    }

    if (g_GameManager.flags.gameCleared == 0)
    {
        endingFile = g_EndingFiles[0][g_GameManager.shotType];
    }
    else if (g_GameManager.currentStage != STAGE6B)
    {
        endingFile = g_EndingFiles[1][g_GameManager.shotType];
    }
    else
    {
        endingFile = g_EndingFiles[2][g_GameManager.shotType];
    }

    if (ending->LoadEndingScript(endingFile) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    else
    {
        return ZUN_SUCCESS;
    }
}

ZunResult Ending::DeletedCallback(Ending *ending)
{
    g_AnmManager->ReleaseAnm(24);
    g_Supervisor.curState = SupervisorState_ResultScreenFromGame;
    g_AnmManager->ReleaseSurface(0);
    g_ZunMemory.Free(ending->scriptData);
    g_Chain.Cut(ending->drawChain);
    ending->drawChain = NULL;
    ZUN_DELETE(ending);
    g_Supervisor.suppressFpsDisplay = FALSE;
    return ZUN_SUCCESS;
}

} /* namespace th08 */
