#include "th_pch.h"

#include <stdarg.h>
#include <stdio.h>

#include "Player.hpp"
#include "ScreenEffect.hpp"

namespace th08
{

// Placeholder for the unledgered global AnmLoaded* observed at 0x00577eb4.
DIFFABLE_STATIC(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);

namespace EclOperands
{
struct Vector3
{
    f32 x;
    f32 y;
    f32 z;

    Vector3 operator-(const Vector3 &other) const;
    f32 Length() const;
};
Vector3 g_TargetPlayerPosition017D61AC;
} // namespace EclOperands

DIFFABLE_STATIC(ChainElem, g_AsciiManagerDrawChainLowPrio);
DIFFABLE_STATIC(AsciiManager, g_AsciiManager);
DIFFABLE_STATIC(ChainElem, g_AsciiManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_AsciiManagerDrawChainHighPrio);

// FUNCTION: th08 0x402130
AsciiManagerString::AsciiManagerString()
{
}

// FUNCTION: th08 0x402150
PauseMenu::PauseMenu()
{
}

// FUNCTION: th08 0x402190
RetryMenu::RetryMenu()
{
}

// FUNCTION: th08 0x4021d0
AsciiManagerPopup::AsciiManagerPopup()
{
}

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

// FUNCTION: th08 0x402430
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

// FUNCTION: th08 0x4028c0
ZunResult AsciiManager::DeletedCallback(AsciiManager *ascii)
{
    g_AnmManager->ReleaseAnm(1);
    g_AnmManager->ReleaseAnm(3);

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4028f0
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
    popup->scale.x = this->scaleX;
    popup->scale.y = this->scaleY;
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
    popup->scale.x = this->scaleX;
    popup->scale.y = this->scaleY;
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

#pragma var_order(popup, alpha, dy, dx, i, j, charPtr, unused, rect, alphaColor, divisor)
// FUNCTION: th08 0x405420
void AsciiManager::OnDrawHighPrioImpl()
{
    AsciiManagerPopup *popup;
    u8 *charPtr;
    i32 alpha;
    f32 dx, dy;
    i32 i, j;
    ZunRect rect;
    ZunColor alphaColor;
    i32 divisor;

    popup = this->scorePopups;
    Float3 unused;

    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.DisableFog();
    }
    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

    for (j = 0; j < ASCII_MAX_SCORE_POPUPS + ASCII_MAX_PLAYER_POPUPS; j++, popup++)
    {
        if (!popup->inUse)
        {
            continue;
        }

        this->smallScoreText.pos.x = popup->position.x - (f32)(popup->characterCount * 4);
        this->smallScoreText.pos.y = popup->position.y;
        this->smallScoreText.color1.d3dColor = popup->color;

        dx = EclOperands::g_TargetPlayerPosition017D61AC.x - popup->position.x;
        dy = EclOperands::g_TargetPlayerPosition017D61AC.y - popup->position.y;
        alpha = (i32)(dx * dx + dy * dy);
        if (alpha > 4096)
        {
            alpha = 208;
        }
        else if (alpha > 1024)
        {
            alpha = ((alpha - 1024) << 7) / 3072 + 80;
        }
        else
        {
            alpha = 80;
        }

        this->smallScoreText.scale.x = this->scaleX;
        this->smallScoreText.scale.y = this->scaleY;

        charPtr = (u8 *)&popup->text[popup->characterCount - 1];
        for (i = popup->characterCount; i > 0; i--)
        {
            if (popup->timer < 52)
            {
                this->smallScoreText.loadedSprite = this->asciiAnm->GetSprite(*charPtr);
                this->smallScoreText.color1.a = alpha;
            }
            else if (popup->timer < 56)
            {
                this->smallScoreText.loadedSprite = this->asciiAnm->GetSprite(*charPtr + 11);
                this->smallScoreText.color1.a = alpha;
            }
            else
            {
                this->smallScoreText.loadedSprite = this->asciiAnm->GetSprite(*charPtr + 21);
                this->smallScoreText.color1.a = alpha;
            }
            this->smallScoreText.spriteSize.x = this->smallScoreText.loadedSprite->widthPx;
            g_AnmManager->DrawNoRotation(&this->smallScoreText);
            this->smallScoreText.pos.x += 8.0f;
            charPtr--;
        }
    }

    if (this->unk_16f08 > 0)
    {
        alphaColor.a = this->unk_16f08;
        alphaColor.r = 0;
        alphaColor.g = 0;
        alphaColor.b = 0;

        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = EclOperands::g_TargetPlayerPosition017D61AC.x + 32.0f - this->unk_16f04 + g_AnmManager->screenShakeOffset.x;
        rect.bottom = 464.0f;
        if (rect.right > rect.left)
        {
            ScreenEffect::DrawSquare(&rect, alphaColor.d3dColor);
        }

        rect.left = EclOperands::g_TargetPlayerPosition017D61AC.x + 32.0f + this->unk_16f04 + g_AnmManager->screenShakeOffset.x;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        if (rect.right > rect.left)
        {
            ScreenEffect::DrawSquare(&rect, alphaColor.d3dColor);
        }

        rect.left = EclOperands::g_TargetPlayerPosition017D61AC.x + 32.0f - this->unk_16f04 + g_AnmManager->screenShakeOffset.x;
        if (rect.left < 32.0f)
        {
            rect.left = 32.0f;
        }
        rect.top = 16.0f;
        rect.right = EclOperands::g_TargetPlayerPosition017D61AC.x + 32.0f + this->unk_16f04 + g_AnmManager->screenShakeOffset.x;
        if (rect.right > 416.0f)
        {
            rect.right = 416.0f;
        }
        rect.bottom = EclOperands::g_TargetPlayerPosition017D61AC.y + 16.0f - this->unk_16f04 + g_AnmManager->screenShakeOffset.y;
        if (rect.bottom > rect.top)
        {
            ScreenEffect::DrawSquare(&rect, alphaColor.d3dColor);
        }

        rect.top = EclOperands::g_TargetPlayerPosition017D61AC.y + 16.0f + this->unk_16f04 + g_AnmManager->screenShakeOffset.y;
        rect.bottom = 464.0f;
        if (rect.bottom > rect.top)
        {
            ScreenEffect::DrawSquare(&rect, alphaColor.d3dColor);
        }

        g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(&this->unk_16f0c, 105);
        this->unk_16f0c.scale.y = this->unk_16f04 / 63.0f;
        this->unk_16f0c.scale.x = this->unk_16f0c.scale.y;
        this->unk_16f0c.pos = *(Float3 *)&EclOperands::g_TargetPlayerPosition017D61AC;
        this->unk_16f0c.pos.x += 32.0f;
        this->unk_16f0c.pos.y += 16.0f;
        this->unk_16f0c.color1.a = this->unk_16f08;
        g_AnmManager->DrawNoRotation(&this->unk_16f0c);
    }

    popup = this->timePopups;
    for (j = 0; j < ASCII_MAX_TIME_POPUPS; j++, popup++)
    {
        if (!popup->inUse)
        {
            continue;
        }

        this->popupText.pos.x = popup->position.x - 3.5f * popup->characterCount;
        this->popupText.pos.y = popup->position.y;
        this->popupText.color1.d3dColor = popup->color;

        dx = EclOperands::g_TargetPlayerPosition017D61AC.x - popup->position.x;
        dy = EclOperands::g_TargetPlayerPosition017D61AC.y - popup->position.y;
        alpha = (i32)(dx * dx + dy * dy);
        if (alpha > 4096)
        {
            alpha = 208;
        }
        else if (alpha > 1024)
        {
            alpha = ((alpha - 1024) << 7) / 3072 + 80;
        }
        else
        {
            alpha = 80;
        }

        this->popupText.scale.x = popup->scale.x;
        this->popupText.scale.y = popup->scale.y;

        charPtr = (u8 *)&popup->text[popup->characterCount - 1];
        for (i = popup->characterCount; i > 0; i--)
        {
            this->popupText.loadedSprite = this->asciiAnm->GetSprite(*charPtr + 136);
            this->popupText.color1.a = alpha;
            this->popupText.spriteSize.x = this->popupText.loadedSprite->widthPx;
            g_AnmManager->DrawNoRotation(&this->popupText);
            this->popupText.pos.x += 7.0f * popup->scale.x;
            charPtr--;
        }
    }

    g_AnmManager->screenShakeOffset.y = 0.0f;
    g_AnmManager->screenShakeOffset.x = 0.0f;

    if (this->youkaiGauge.IsVisible())
    {
        this->youkaiGaugeCursor.pos.x =
            (f32)g_GameManager.GetYoukaiGauge() * 112.0f / 2.0f / 10000.0f + this->youkaiGauge.pos.x + 64.0f;
        g_AnmManager->FUN_00463470(&this->youkaiGaugeCursor);

        this->percentageText.pos.x =
            (f32)g_GameManager.GetYoukaiGauge() * 80.0f / 2.0f / 10000.0f + this->youkaiGauge.pos.x + 64.0f;
        this->percentageText.pos.y = this->youkaiGaugeCursor.pos.y - 7.0f;
        this->percentageText.pos.z = this->youkaiGaugeCursor.pos.z;
        this->percentageText.color1.a = this->youkaiGauge.color1.a;

        if (g_GameManager.GaugeIsExtremelyHuman())
        {
            this->percentageText.color1.r = 112;
            this->percentageText.color1.g = 112;
            this->percentageText.color1.b = 255;
        }
        else if (g_GameManager.GaugeIsModeratelyHuman())
        {
            this->percentageText.color1.r = 176;
            this->percentageText.color1.g = 176;
            this->percentageText.color1.b = 255;
        }
        else if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            this->percentageText.color1.r = 255;
            this->percentageText.color1.g = 112;
            this->percentageText.color1.b = 112;
        }
        else if (g_GameManager.GaugeIsModeratelyYoukai())
        {
            this->percentageText.color1.r = 255;
            this->percentageText.color1.g = 176;
            this->percentageText.color1.b = 176;
        }
        else
        {
            this->percentageText.color1.r = 255;
            this->percentageText.color1.g = 255;
            this->percentageText.color1.b = 255;
        }

        this->youkaiGauge.color1.d3dColor = this->percentageText.color1.d3dColor;

        g_AnmManager->DrawNoRotation(&this->youkaiGauge);
        g_AnmManager->DrawNoRotation(&this->youkaiGaugeHumanIcon);
        g_AnmManager->DrawNoRotation(&this->youkaiGaugeYoukaiIcon);

        this->DrawPercentage(&this->percentageText.pos, g_GameManager.GetYoukaiGauge(),
                             this->percentageText.color1.d3dColor);

        divisor = 10000000;
        i = g_GameManager.globals->pointItemValue;
        alpha = 0;
        this->percentageText.pos.x = this->youkaiGauge.pos.x + 62.0f - 14.0f;
        this->percentageText.pos.y = this->youkaiGauge.pos.y + 3.0f + 8.0f;

        for (j = 0; j < 8; j++)
        {
            alpha += i / divisor;
            if (alpha != 0)
            {
                this->asciiAnm->SetSprite(&this->percentageText, i / divisor + 136);
                g_AnmManager->DrawNoRotation(&this->percentageText);
                this->percentageText.pos.x += 7.0f;
            }
            i %= divisor;
            divisor /= 10;
        }
    }
}

#pragma var_order(xOffset, numDigits, absPercentage)
// FUNCTION: th08 0x405e10
void AsciiManager::DrawPercentage(Float3 *position, i32 percentage, D3DCOLOR color)
{
    f32 xOffset;
    i32 absPercentage;
    i32 numDigits;

    numDigits = 4;
    if (percentage < 0)
    {
        numDigits++;
    }

    absPercentage = abs(percentage);
    if (absPercentage >= 10000)
    {
        numDigits += 3;
    }
    else
    {
        if (absPercentage >= 1000)
        {
            numDigits += 2;
        }
        else
        {
            numDigits++;
        }
    }

    xOffset = (f32)numDigits * 3.5f - 3.5f - 4.0f;

    this->percentageText.pos = *position;
    this->percentageText.pos.x -= xOffset;
    this->percentageText.color1.d3dColor = color;

    if (percentage < 0)
    {
        this->asciiAnm->SetSprite(&this->percentageText, 148);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;
    }

    if (absPercentage >= 10000)
    {
        this->asciiAnm->SetSprite(&this->percentageText, 137);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 136);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 136);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 147);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 5.0f;
        this->percentageText.scale.y = 0.8f;
        this->percentageText.scale.x = 0.8f;
        this->percentageText.pos.y += 2.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 136);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 5.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 136);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;
    }
    else if (absPercentage >= 1000)
    {
        numDigits = absPercentage;
        this->asciiAnm->SetSprite(&this->percentageText, numDigits / 1000 + 136);
        numDigits = numDigits % 1000;
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;

        this->asciiAnm->SetSprite(&this->percentageText, numDigits / 100 + 136);
        numDigits = numDigits % 100;
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 147);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 5.0f;
        this->percentageText.scale.y = 0.8f;
        this->percentageText.scale.x = 0.8f;
        this->percentageText.pos.y += 2.0f;

        this->asciiAnm->SetSprite(&this->percentageText, numDigits / 10 + 136);
        numDigits = numDigits % 10;
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 5.0f;

        this->asciiAnm->SetSprite(&this->percentageText, numDigits + 136);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;
    }
    else
    {
        numDigits = absPercentage;
        this->asciiAnm->SetSprite(&this->percentageText, numDigits / 100 + 136);
        numDigits = numDigits % 100;
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;

        this->asciiAnm->SetSprite(&this->percentageText, 147);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 5.0f;
        this->percentageText.scale.y = 0.8f;
        this->percentageText.scale.x = 0.8f;
        this->percentageText.pos.y += 2.0f;

        this->asciiAnm->SetSprite(&this->percentageText, numDigits / 10 + 136);
        numDigits = numDigits % 10;
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 5.0f;

        this->asciiAnm->SetSprite(&this->percentageText, numDigits + 136);
        g_AnmManager->DrawNoRotation(&this->percentageText);
        this->percentageText.pos.x += 7.0f;
    }

    this->percentageText.scale.y = 1.0f;
    this->percentageText.scale.x = 1.0f;
    this->percentageText.pos.y -= 2.0f;
    this->asciiAnm->SetSprite(&this->percentageText, 146);
    g_AnmManager->DrawNoRotation(&this->percentageText);
}

} /* namespace th08 */
