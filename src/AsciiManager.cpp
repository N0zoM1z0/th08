#include "th_pch.h"

#include <stdarg.h>
#include <stdio.h>

namespace th08
{

DIFFABLE_STATIC(ChainElem, g_AsciiManagerDrawChainLowPrio);
DIFFABLE_STATIC(AsciiManager, g_AsciiManager);
DIFFABLE_STATIC(ChainElem, g_AsciiManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_AsciiManagerDrawChainHighPrio);

ChainCallbackResult AsciiManager::OnUpdate(AsciiManager *ascii)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult AsciiManager::OnDrawLowPrio(AsciiManager *ascii)
{
    ascii->OnDrawLowPrioImpl();
    ascii->ResetStrings();
    ascii->pauseMenu.OnDraw();
    ascii->retryMenu.OnDraw();
    if (ascii->demoIcon.scriptIndex != 0)
    {
        g_AnmManager->DrawNoRotation(&ascii->demoIcon);
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult AsciiManager::OnDrawHighPrio(AsciiManager *ascii)
{
    ascii->OnDrawHighPrioImpl();

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void AsciiManager::Reset()
{
    memset(&this->smallScoreText, 0, sizeof(AnmVm));
    memset(&this->popupText, 0, sizeof(AnmVm));
    memset(&this->largeText, 0, sizeof(AnmVm));
    memset(&this->strings, 0, sizeof(this->strings));
    memset(&this->pauseMenu, 0, sizeof(PauseMenu));
    memset(&this->retryMenu, 0, sizeof(RetryMenu));
    memset(&this->scorePopups, 0, sizeof(this->scorePopups));
    memset(&this->timePopups, 0, sizeof(this->timePopups));

    this->numStrings = 0;
    this->isGui = FALSE;
    this->isSelected = FALSE;
    this->nextScorePopupIndex = 0;
    this->nextPlayerPointPopupIndex = 0;
    /* nextTimePopupIndex is not set to 0?  */
    this->unk0x829c = 0;
    this->color.d3dColor = 0xffffffff;
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->smallScoreText.anchor = 3;
    this->popupText.anchor = 3;
    this->asciiAnm->InitializeAndSetSprite(&this->smallScoreText, 0);
    this->asciiAnm->InitializeAndSetSprite(&this->popupText, 136);
    this->asciiAnm->InitializeAndSetSprite(&this->largeText, 32);
    this->smallScoreText.pos.z = 0.1f;
    /* This was already set to FALSE ? */
    this->isSelected = FALSE;
    this->SetSpaceWidth(13);
}

void AsciiManager::InitializeVms()
{
    this->asciiAnm->SetAndExecuteScriptIdx(&this->youkaiGauge, 5);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->youkaiGaugeYoukaiIcon, 7);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->youkaiGaugeHumanIcon, 6);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->youkaiGaugeCursor, 8);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->percentageText, 4);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->unk_1520, 9);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->bossMarkers[0], 10);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->bossMarkers[1], 10);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->bossMarkers[2], 10);
    this->asciiAnm->SetAndExecuteScriptIdx(&this->bossMarkers[3], 10);

    this->youkaiGaugeHumanIcon.pos.x -= (g_GameManager.youkaiGaugeHumanLimit * 56.0f) / -10000.0f;
    this->youkaiGaugeYoukaiIcon.pos.x += (g_GameManager.youkaiGaugeYoukaiLimit * 56.0f) / 10000.0f;

    this->SetGaugeInterrupt(this->GetGaugeInterrupt());
}

ZunResult AsciiManager::RegisterChain()
{
    AsciiManager *ascii = &g_AsciiManager;

    g_AsciiManagerCalcChain.SetCallback((ChainCallback)AsciiManager::OnUpdate);
    g_AsciiManagerCalcChain.addedCallback = (ChainLifetimeCallback)AsciiManager::AddedCallback;
    g_AsciiManagerCalcChain.deletedCallback = (ChainLifetimeCallback)AsciiManager::DeletedCallback;
    g_AsciiManagerCalcChain.arg = ascii;
    if (g_Chain.AddToCalcChain(&g_AsciiManagerCalcChain, 1) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_AsciiManagerDrawChainLowPrio.SetCallback((ChainCallback)AsciiManager::OnDrawLowPrio);
    g_AsciiManagerDrawChainLowPrio.arg = ascii;
    g_Chain.AddToDrawChain(&g_AsciiManagerDrawChainLowPrio, 20);

    g_AsciiManagerDrawChainHighPrio.SetCallback((ChainCallback)AsciiManager::OnDrawHighPrio);
    g_AsciiManagerDrawChainHighPrio.arg = ascii;
    g_Chain.AddToDrawChain(&g_AsciiManagerDrawChainHighPrio, 14);

    return ZUN_SUCCESS;
}

ZunResult AsciiManager::AddedCallback(AsciiManager *ascii)
{
    memset(ascii, 0, sizeof(AsciiManager));

    ascii->asciiAnm = g_AnmManager->PreloadAnm(1, "ascii.anm");
    if (ascii->asciiAnm == NULL)
    {
        return ZUN_ERROR;
    }

    ascii->captureAnm = g_AnmManager->PreloadAnm(3, "capture.anm");
    if (ascii->captureAnm == NULL)
    {
        return ZUN_ERROR;
    }

    ascii->Reset();
    ascii->InitializeVms();

    return ZUN_SUCCESS;
}

ZunResult AsciiManager::DeletedCallback(AsciiManager *ascii)
{
    g_AnmManager->ReleaseAnm(1);
    g_AnmManager->ReleaseAnm(3);

    return ZUN_SUCCESS;
}

void AsciiManager::CutChain()
{
    g_Chain.Cut(&g_AsciiManagerCalcChain);
    g_Chain.Cut(&g_AsciiManagerDrawChainLowPrio);
    /* ZUN seemingly forgot this: g_Chain.Cut(&g_AsciiManagerDrawChainHighPrio); */
}

#pragma var_order(nextString)
void AsciiManager::AddString(Float3 *position, const char *string)
{
    AsciiManagerString *nextString;

    if (this->numStrings >= ARRAY_SIZE_SIGNED(this->strings))
    {
        return;
    }

    nextString = &this->strings[this->numStrings];
    this->numStrings++;

    strcpy(nextString->text, string);

    nextString->position = *position;

    nextString->color = this->color.d3dColor;
    nextString->scaleX = this->scaleX;
    nextString->scaleY = this->scaleY;
    nextString->isGui = this->isGui;

    if (g_Supervisor.IsSoftwareTexturing())
    {
        nextString->isSelected = this->isSelected;
    }
    else
    {
        nextString->isSelected = FALSE;
    }
}

void AsciiManager::AddFormatText(Float3 *position, const char *fmt, ...)
{
    char buf[512];
    va_list va;

    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    this->AddString(position, buf);
    va_end(va);
}

int AsciiManager::AddFormatText2(Float3 *position, const char *fmt, ...)
{
    char buf[512];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->AddString(position, buf);
    va_end(args);

    /* Did you know that vsprintf returns the number of characters added to the
     * buffer? So ZUN did not have to call strlen here.
     */
    return strlen(buf);
}

#pragma var_order(spaceWidth, i, curString, text, isGui, vector)
void AsciiManager::OnDrawLowPrioImpl()
{
    Float3 vector;
    ZunBool isGui = TRUE;
    int i;
    AsciiManagerString *curString = &this->strings[0];
    u8 *text;
    float spaceWidth;

    this->largeText.visible = true;
    this->largeText.anchor = 3;

    for (i = 0; i < this->numStrings; i++, curString++)
    {
        this->largeText.pos = curString->position;

        text = (u8 *)curString->text;

        this->largeText.scale.x = curString->scaleX;
        this->largeText.scale.y = curString->scaleY;
        spaceWidth = this->spaceWidth * curString->scaleX;

        if (isGui != curString->isGui)
        {
            isGui = curString->isGui;

            g_AnmManager->FlushVertexBuffer();

            if (isGui)
            {
                g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
                g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
                g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
                g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
            else
            {
                g_Supervisor.viewport.X = 0;
                g_Supervisor.viewport.Y = 0;
                g_Supervisor.viewport.Width = 640;
                g_Supervisor.viewport.Height = 480;
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
        }

        while (*text)
        {
            if (*text == '\n')
            {
                this->largeText.pos.y += 16.0f * curString->scaleY;
                this->largeText.pos.x = curString->position.x;
            }
            else if (*text == ' ')
            {
                this->largeText.pos.x += spaceWidth;
            }
            else
            {
                if (!curString->isSelected)
                {
                    this->largeText.loadedSprite = this->asciiAnm->GetSprite(*text + (31 - ' '));
                    this->largeText.color1.d3dColor = curString->color;
                }
                else
                {
                    this->largeText.loadedSprite = this->asciiAnm->GetSprite(*text + (170 - ' '));
                    this->largeText.color1.d3dColor = 0xffffffff;
                }

                g_AnmManager->DrawNoRotation(&this->largeText);
                this->largeText.pos.x += spaceWidth;
            }

            text++;
        }
    }

    if (isGui)
    {
        g_AnmManager->FlushVertexBuffer();
        g_Supervisor.viewport.X = 0;
        g_Supervisor.viewport.Y = 0;
        g_Supervisor.viewport.Width = 640;
        g_Supervisor.viewport.Height = 480;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    }

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->bossMarkers); i++)
    {
        if (this->bossMarkers[i].pos.x >= 56.0f && this->bossMarkers[i].pos.x <= 392.0f)
        {
            // TODO: This line is not done! The player position is needed in this calculation
            spaceWidth = fabsf(this->bossMarkers[i].pos.x - 32.0f);

            this->bossMarkers[i].loadedSprite = this->asciiAnm->GetSprite(157);

            switch (this->bossMarkerStates[i])
            {
            case 0:
            no_flicker:
                this->bossMarkers[i].color1.r = 255;
                this->bossMarkers[i].color1.g = 255;
                this->bossMarkers[i].color1.b = 255;
                if (spaceWidth < 64.0f)
                {
                    this->bossMarkers[i].color1.a = (spaceWidth * 64.0f) / 64.0f + 96.0f;
                }
                else
                {
                    this->bossMarkers[i].color1.a = 160;
                }
                break;
            case 1:
                this->bossMarkers[i].color1.a = 128;
                this->bossMarkers[i].color1.r = 255;
                this->bossMarkers[i].color1.g = 64;
                this->bossMarkers[i].color1.b = 64;
                break;
            case 2:
                if (this->unk_8284 % 8 == 0)
                {
                    this->bossMarkers[i].loadedSprite = this->asciiAnm->GetSprite(158);
                    this->bossMarkers[i].color1.a = 255;
                    this->bossMarkers[i].color1.r = 255;
                    this->bossMarkers[i].color1.g = 255;
                    this->bossMarkers[i].color1.b = 255;
                }
                else
                {
                    goto no_flicker;
                }
                break;
            case 3:
                if (this->unk_8284 % 4 == 0)
                {
                    this->bossMarkers[i].loadedSprite = this->asciiAnm->GetSprite(158);
                    this->bossMarkers[i].color1.a = 255;
                    this->bossMarkers[i].color1.r = 255;
                    this->bossMarkers[i].color1.g = 255;
                    this->bossMarkers[i].color1.b = 255;
                }
                else
                {
                    goto no_flicker;
                }
                break;
            case 4:
                if (this->unk_8284 % 2 == 0)
                {
                    this->bossMarkers[i].loadedSprite = this->asciiAnm->GetSprite(158);
                    this->bossMarkers[i].color1.a = 255;
                    this->bossMarkers[i].color1.r = 255;
                    this->bossMarkers[i].color1.g = 255;
                    this->bossMarkers[i].color1.b = 255;
                }
                else
                {
                    goto no_flicker;
                }
                break;
            }

            g_AnmManager->DrawNoRotation(&this->bossMarkers[i]);
        }
    }
}

void AsciiManager::CreateScorePopup(Float3 *position, i32 number, D3DCOLOR color)
{
    AsciiManagerPopup *popup;
    int characterCount;

    if (this->nextScorePopupIndex >= ASCII_MAX_SCORE_POPUPS)
    {
        this->nextScorePopupIndex = 0;
    }
    popup = &this->scorePopups[nextScorePopupIndex];
    popup->inUse = true;

    characterCount = 0;
    if (number >= 0)
    {
        while (number != 0)
        {
            popup->text[characterCount] = number % 10;
            characterCount++;
            number /= 10;
        }
    }
    else
    {
        popup->text[characterCount] = 10;
        characterCount++;
    }

    if (characterCount == 0)
    {
        popup->text[characterCount] = 0;
        characterCount++;
    }

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer = 0;
    popup->position = *position;
    popup->position.x += g_GameManager.arcadeRegionTopLeftPos.x;
    popup->position.y += g_GameManager.arcadeRegionTopLeftPos.y;
    this->nextScorePopupIndex++;
}

void AsciiManager::CreatePlayerPointPopup(Float3 *position, i32 number, D3DCOLOR color)
{
    AsciiManagerPopup *popup;
    int characterCount;

    if (this->nextPlayerPointPopupIndex >= ASCII_MAX_PLAYER_POPUPS)
    {
        this->nextPlayerPointPopupIndex = 0;
    }
    popup = &this->scorePopups[ASCII_MAX_SCORE_POPUPS + nextPlayerPointPopupIndex];
    popup->inUse = true;

    characterCount = 0;
    if (number >= 0)
    {
        while (number != 0)
        {
            popup->text[characterCount] = number % 10;
            characterCount++;
            number /= 10;
        }
    }
    else
    {
        popup->text[characterCount] = 10;
        characterCount++;
    }

    if (characterCount == 0)
    {
        popup->text[characterCount] = 0;
        characterCount++;
    }

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer = 0;
    popup->position = *position;
    popup->position.x += g_GameManager.arcadeRegionTopLeftPos.x;
    popup->position.y += g_GameManager.arcadeRegionTopLeftPos.y;
    this->nextPlayerPointPopupIndex++;
}

void AsciiManager::CreateTimePopup(Float3 *position, i32 number, i32 param3, D3DCOLOR color)
{
    AsciiManagerPopup *popup;
    int characterCount;

    if (this->nextTimePopupIndex >= ASCII_MAX_TIME_POPUPS)
    {
        this->nextTimePopupIndex = 0;
    }
    popup = &this->timePopups[nextTimePopupIndex];
    popup->inUse = true;

    characterCount = 0;
    if (param3 > 0)
    {
        popup->text[characterCount] = 15;
        characterCount++;
        while (param3 != 0)
        {
            popup->text[characterCount] = param3 % 10;
            characterCount++;
            param3 /= 10;
        }
        popup->text[characterCount] = 14;
        characterCount++;
    }

    if (number > 0)
    {
        while (number != 0)
        {
            popup->text[characterCount] = number % 10;
            characterCount++;
            number /= 10;
        }
    }
    else
    {
        popup->text[characterCount] = 0;
        characterCount++;
    }

    popup->text[characterCount] = 13;
    characterCount++;

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer = 0;
    popup->position = *position;
    popup->position.x += g_GameManager.arcadeRegionTopLeftPos.x;
    popup->position.y += g_GameManager.arcadeRegionTopLeftPos.y;
    popup->scaleX = this->scaleX;
    popup->scaleY = this->scaleY;
    this->nextTimePopupIndex++;
}

void AsciiManager::CreateFamiliarPopup(Float3 *position, i32 number, i32 param3, D3DCOLOR color)
{
    AsciiManagerPopup *popup;
    int characterCount;

    if (this->nextTimePopupIndex >= ASCII_MAX_TIME_POPUPS)
    {
        this->nextTimePopupIndex = 0;
    }
    popup = &this->timePopups[nextTimePopupIndex];
    popup->inUse = true;

    characterCount = 0;
    if (param3 > 0)
    {
        popup->text[characterCount] = 15;
        characterCount++;
        while (param3 != 0)
        {
            popup->text[characterCount] = param3 % 10;
            characterCount++;
            param3 /= 10;
        }
        popup->text[characterCount] = 14;
        characterCount++;
    }

    if (number > 0)
    {
        while (number != 0)
        {
            popup->text[characterCount] = number % 10;
            characterCount++;
            number /= 10;
        }
    }
    else
    {
        popup->text[characterCount] = 0;
        characterCount++;
    }

    popup->text[characterCount] = 13;
    characterCount++;

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer = 88;
    popup->position = *position;
    popup->position.x += g_GameManager.arcadeRegionTopLeftPos.x + 3.5f * characterCount;
    popup->position.y += g_GameManager.arcadeRegionTopLeftPos.y;
    popup->scaleX = this->scaleX;
    popup->scaleY = this->scaleY;
    this->nextTimePopupIndex++;
}

// STUB: th08 0x4037b0
i32 PauseMenu::OnUpdate()
{
    return 0;
}

// STUB: th08 0x404750
i32 PauseMenu::OnDraw()
{
    return 0;
}

// STUB: th08 0x404890
i32 RetryMenu::OnUpdate()
{
    return 0;
}

// STUB: th08 0x4052b0
i32 RetryMenu::OnDraw()
{
    return 0;
}

// STUB: th08 0x405420
void AsciiManager::OnDrawHighPrioImpl()
{
}

// STUB: th08 0x405e10
void AsciiManager::DrawPercentage(Float3 *position, i32 percentage, D3DCOLOR color)
{
}

} /* namespace th08 */
