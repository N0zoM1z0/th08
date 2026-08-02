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

ZunResult Ending::ReadEndFileParameter()
{
    i32 param;

    param = atol(this->cursorPtr);

    while (*this->cursorPtr != '\0')
    {
        this->cursorPtr++;
    }
    while (*this->cursorPtr == '\0')
    {
        this->cursorPtr++;
    }

    return (ZunResult)param;
}

#pragma var_order(rect, alpha)
void Ending::FadingEffect()
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
ZunResult Ending::ParseEndFile()
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

    if (this->timer3 > 0)
    {
        this->timer3--;

        if (this->minWaitResetFrames != 0)
        {
            this->minWaitResetFrames--;
        }
        else
        {
            if ((WAS_PRESSED(TH_BUTTON_SELECTMENU)) || ((this->hasSeenEnding && (IS_PRESSED(TH_BUTTON_SKIP) != 0))))
            {
                this->timer3 = 0;
            }
        }

        if (this->timer3 <= 0)
        {
            for (i = 0; i < ARRAY_SIZE_SIGNED(this->vms) - 1; i++)
            {
                this->vms[i].SetInterrupt(2);
            }
            this->timesFileParsed = 0;
        }
        else
        {
            goto end_of_parse;
        }
    }

    if (this->timer2 > 0)
    {
        this->timer2--;
        if (this->minWaitFrames != 0)
        {
            this->minWaitFrames--;
        }
        else
        {
            if ((WAS_PRESSED(TH_BUTTON_SELECTMENU)) || ((this->hasSeenEnding && (IS_PRESSED(TH_BUTTON_SKIP) != 0))))
            {
                this->timer2 = 0;
            }
        }
        goto end_of_parse;
    }

    while (true)
    {
        switch (*this->cursorPtr)
        {
        case END_READ_OPCODE:

            this->cursorPtr++;
            switch (*this->cursorPtr)
            {
            case END_OPCODE_BACKGROUND:
                if (g_AnmManager->LoadSurface(0, (this->cursorPtr + 1)))
                {
                    return ZUN_ERROR;
                }
                break;

            case END_OPCODE_EXECUTE_ANM:
                this->cursorPtr++;
                vmIndex = this->ReadEndFileParameter();
                anmScriptIdx = this->ReadEndFileParameter();
                anmSpriteIdx = this->ReadEndFileParameter();
                this->anmFile->ExecuteAnmIdx(&this->vms[vmIndex], anmScriptIdx);
                this->anmFile->SetSprite(&this->vms[vmIndex], anmSpriteIdx);
                break;

            case END_OPCODE_SCROLL_BACKGROUND:
                this->cursorPtr++;
                firstRead = this->ReadEndFileParameter();
                secondRead = this->ReadEndFileParameter();
                this->backgroundScrollSpeed = (f32)firstRead / (f32)secondRead;
                break;

            case END_OPCODE_SET_VERTICAL_SCROLL_POS:
                this->cursorPtr++;
                this->backgroundPos.y = (f32)this->ReadEndFileParameter();
                break;

            case END_OPCODE_EXEC_END_FILE:
                if (this->LoadEnding(this->cursorPtr + 1) != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
                index = 0;
                lineEndDisplayed = false;
                this->hasSeenEnding = this->unk2a5c;

            case END_OPCODE_ROLL_STAFF:
                for (spriteIdx = 0; spriteIdx < ARRAY_SIZE_SIGNED(this->vms); spriteIdx++)
                {
                    this->vms[spriteIdx].scriptIndex = 0;
                }
                break;

            case END_OPCODE_PLAY_MUSIC:
                g_Supervisor.LoadMusic(0, this->cursorPtr + 1);
                g_Supervisor.PlayMusic(0, 0);
                break;

            case END_OPCODE_FADE_MUSIC:
                this->cursorPtr++;
                fadeTime = (f32)this->ReadEndFileParameter();
                g_Supervisor.FadeOutMusic(fadeTime);
                break;

            case END_OPCODE_SET_DELAY:
                this->cursorPtr++;
                this->line2Delay = this->ReadEndFileParameter();
                this->topLineDelay = this->ReadEndFileParameter();
                break;

            case END_OPCODE_COLOR:
                this->cursorPtr++;
                this->textColor = this->ReadEndFileParameter();
                break;

            case END_OPCODE_WAIT_RESET:
                this->cursorPtr++;
                this->timer3 = this->ReadEndFileParameter();
                this->minWaitResetFrames = this->ReadEndFileParameter();
                this->timer2 = 0;
                this->minWaitFrames = 0;
                while (*this->cursorPtr != '\n' && *this->cursorPtr != '\r')
                {
                    this->cursorPtr++;
                }
                while (*this->cursorPtr == '\n' || *this->cursorPtr == '\r')
                {
                    this->cursorPtr++;
                }
                goto end_of_parse;

            case END_OPCODE_WAIT:
                this->cursorPtr++;
                this->timer2 = this->ReadEndFileParameter();
                this->minWaitFrames = this->ReadEndFileParameter();
                while (*this->cursorPtr != '\n' && *this->cursorPtr != '\r')
                {
                    this->cursorPtr++;
                }
                while (*this->cursorPtr == '\n' || *this->cursorPtr == '\r')
                {
                    this->cursorPtr++;
                }
                goto end_of_parse;

            case END_OPCODE_FADE_IN_BLACK:
                this->cursorPtr++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_IN_BLACK;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter();
                break;

            case END_OPCODE_FADE_OUT_BLACK:
                this->cursorPtr++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_OUT_BLACK;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter();
                break;

            case END_OPCODE_FADE_IN:
                this->cursorPtr++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_IN_WHITE;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter();
                break;

            case END_OPCODE_FADE_OUT:
                this->cursorPtr++;
                this->fadeMode = ENDING_FADE_TYPE_FADE_OUT_WHITE;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter();
                break;

            case END_OPCODE_END:
                return ZUN_ERROR;
            }
            while (*this->cursorPtr != '\n' && *this->cursorPtr != '\r')
            {
                this->cursorPtr++;
            }
            while (*this->cursorPtr == '\n' || *this->cursorPtr == '\r')
            {
                this->cursorPtr++;
            }
            break;

        case '\0':
        case '\n':
        case '\r':
            if (index != 0)
            {
                g_AnmManager->DrawTextLeft(&this->vms[this->timesFileParsed], this->textColor, COLOR_WHITE, textBuffer);
                this->vms[this->timesFileParsed].SetInterrupt(1);
            }
            while (*this->cursorPtr == '\n' || *this->cursorPtr == '\0' || *this->cursorPtr == '\r')
            {
                this->cursorPtr++;
            }
            if (IS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                this->timer2 = this->topLineDelay;
                this->minWaitFrames = this->topLineDelay;
            }
            else
            {
                this->timer2 = this->line2Delay;
                this->minWaitFrames = this->topLineDelay;
            }
            this->timesFileParsed++;
            goto end_of_parse;

        default:
            textBuffer[index] = *this->cursorPtr;
            textBuffer[index + 1] = *(this->cursorPtr + 1);
            index += 2;
            this->cursorPtr += 2;
            break;

        } // switch(*this->cursorPtr)
    } // while(true)

end_of_parse:
    this->timer1++;
    this->backgroundPos.y -= this->backgroundScrollSpeed;

    if (this->backgroundPos.y <= 0.0f)
    {
        this->backgroundPos.y = 0.0f;
        this->backgroundScrollSpeed = 0.0f;
    }
    return ZUN_SUCCESS;
}

ZunResult Ending::LoadEnding(const char *path)
{
    char *prevFile = this->fileData;
    this->fileData = (char *)FileSystem::OpenFile(path, NULL, 0);

    if (this->fileData == NULL)
    {
        this->fileData = prevFile;
        g_GameErrorContext.Log(TH_ERR_ENDING_FILE_UNREADABLE);
        return ZUN_ERROR;
    }

    this->cursorPtr = this->fileData;
    this->line2Delay = 8;
    this->timer2 = 0;
    this->timer1 = 0;

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
    this->line2Delay = 8;
    this->timer2 = 0;
    this->timer1 = 0;
    this->backgroundPos.x = 0;
    this->backgroundPos.y = 0;
    this->backgroundScrollSpeed = 0;
}

ChainCallbackResult Ending::OnUpdate(Ending *ending)
{
    i32 frameSkip = 0;

loop:
    if (ending->ParseEndFile() != ZUN_SUCCESS)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(ending->vms) - 1; i++)
    {
        g_AnmManager->ExecuteScript(&ending->vms[i]);
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
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(ending->vms) - 1; idx++)
    {
        g_AnmManager->Draw2D(&ending->vms[idx]);
    }
    ending->FadingEffect();
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
    g_Supervisor.unk178 = 1;

    ending->anmFile = g_AnmManager->LoadAnm(0x18, "staff01.anm");

    if (g_GameManager.flags.unk4)
    {

        shotType2 = g_GameManager.shotType;

        stageBit = (g_GameManager.currentStage != STAGE6B) ? SPELL_PRACTICE_UNLOCKED_FLAG : EXTRA_UNLOCKED_FLAG;

        g_GameManager.plst.playDataByDifficulty[g_GameManager.difficulty].clears++;
        g_GameManager.plst.playDataTotals.clears++;

        ending->hasSeenEnding = false;
        ending->unk2a5c = 0;

        if ((g_GameManager.currentStage == STAGE6B && g_GameManager.IsExtraUnlocked()) ||
            (g_GameManager.currentStage == STAGE6A && g_GameManager.IsSpellPracticeUnlocked()))
        {
            ending->unk2a5c = 1;
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
        ending->hasSeenEnding = g_GameManager.clrdData[g_GameManager.shotType].unk_20;
        g_GameManager.clrdData[g_GameManager.shotType].unk_20 = 0;
        g_GameManager.plst.bgmUnlocked[18] = 0x12;
    }

execute_anms:
    for (i = 0; i < ARRAY_SIZE_SIGNED(ending->vms) - 1; i++)
    {
        g_Supervisor.textAnm->ExecuteAnmIdx(&ending->vms[i], i + 0x12);
        ending->vms[i].pos = Float3(64.0f, i * 16.0f + 400.0f, 0.0f);
    }

    if (g_GameManager.flags.unk4 == 0)
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

    if (ending->LoadEnding(endingFile) != ZUN_SUCCESS)
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
    g_ZunMemory.Free(ending->fileData);
    g_Chain.Cut(ending->drawChain);
    ending->drawChain = NULL;
    ZUN_DELETE(ending);
    g_Supervisor.unk178 = 0;
    return ZUN_SUCCESS;
}

} /* namespace th08 */
