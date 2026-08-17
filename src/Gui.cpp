#include "th_pch.h"

#include "BulletManager.hpp"
#include "Gui.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "ReplayManager.hpp"

namespace th08
{

DIFFABLE_STATIC(Gui, g_Gui);
DIFFABLE_STATIC(ChainElem, g_GuiCalcChain);
DIFFABLE_STATIC(ChainElem, g_GuiDrawChain);
DIFFABLE_STATIC(i32, g_GuiAnmReleaseRequired);
DIFFABLE_STATIC(i32, g_GuiResourceReloadEnabled);
DIFFABLE_STATIC(i32, g_GuiFullPowerModeFrames);
DIFFABLE_STATIC(i32, g_GuiMessageStageMode);
DIFFABLE_STATIC(AnmLoaded *, g_GuiPortraitAnm0);
DIFFABLE_STATIC(AnmLoaded *, g_GuiPortraitAnm1);
DIFFABLE_STATIC(AnmLoaded *, g_GuiPortraitAnm2);
struct GuiMessageTextColorSet
{
    u32 colors[4];
};
DIFFABLE_STATIC_ARRAY(GuiMessageTextColorSet, SHOT_ALL, g_GuiMessageTextColors);

struct GuiMessageStateOverlay
{
    void *msgFile;                  // +0x000
    u8 *currentInstr;               // +0x004
    i32 currentMsgIdx;              // +0x008
    ZunTimer timer;                 // +0x00C
    i32 framesElapsedDuringPause;   // +0x018
    i32 waitThreshold;              // +0x01C
    AnmVm portraits[4];             // +0x020
    AnmVm dialogueLines[2];         // +0xAB0
    AnmVm extraVms[2];              // +0xFF8
    u32 textColors[4];              // +0x1540
    u32 shadowColors[4];            // +0x1550
    i32 fontSize;                   // +0x1560
    i32 ignoreWaitCounter;          // +0x1564
    u8 dialogueSkippable;           // +0x1568
    u8 currentSide;                 // +0x1569
    u8 textPending;                 // +0x156A
    u8 currentLine;                 // +0x156B
    u8 currentPortrait;             // +0x156C
    u8 messageFlag;                 // +0x156D
    u8 routeChoice;                 // +0x156E
    u8 pad156F;                     // +0x156F
    i32 resultState;                // +0x1570
    i32 unknown1574;                // +0x1574
};
C_ASSERT(sizeof(GuiMessageStateOverlay) == 0x1578);
C_ASSERT(offsetof(GuiMessageStateOverlay, portraits) == 0x20);
C_ASSERT(offsetof(GuiMessageStateOverlay, dialogueLines) == 0xAB0);
C_ASSERT(offsetof(GuiMessageStateOverlay, extraVms) == 0xFF8);
C_ASSERT(offsetof(GuiMessageStateOverlay, textColors) == 0x1540);


// FUNCTION: th08 0x4353ec
#pragma var_order(i, decoded)
void __fastcall FUN_004353ec(char *out, const char *encoded)
{
    char decoded;
    i32 i = 0;
    do
    {
        decoded = *encoded ^ 0x77;
        out[i] = decoded;
        i++;
        encoded++;
    } while (decoded != '\0');
}

// FUNCTION: th08 0x438fe9
i32 FUN_00438fe9()
{
    return g_GuiResourceReloadEnabled;
}


// FUNCTION: th08 0x437f5c
#pragma var_order(srcRect, destRect)
void __fastcall FUN_00437f5c(i32 spriteIdx)
{
    RECT destRect;
    RECT srcRect;

    destRect.left = (i32)g_Gui.stageTextAnm->GetSprite(10)->startPixelInclusive.x;
    destRect.top = (i32)g_Gui.stageTextAnm->GetSprite(10)->startPixelInclusive.y;
    destRect.right = (i32)g_Gui.stageTextAnm->GetSprite(10)->endPixelInclusive.x;
    destRect.bottom = (i32)g_Gui.stageTextAnm->GetSprite(10)->endPixelInclusive.y;

    srcRect.left = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->startPixelInclusive.x;
    srcRect.top = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->startPixelInclusive.y;
    srcRect.right = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->endPixelInclusive.x;
    srcRect.bottom = (i32)g_Gui.stageTextAnm->GetSprite(spriteIdx)->endPixelInclusive.y;

    g_AnmManager->CopyTextureRect(10, 0, 10, 1, &destRect, &srcRect);
}

// FUNCTION: th08 0x43396d
void GuiImpl::FUN_0043396d(i32 value)
{
    void *msgFile;

    utils::GuiDebugPrint("msg start %d\n\r", value);
    msgFile = reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->msgFile;
    memset(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm), 0, 0x1570);
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->msgFile = msgFile;

    if (value == 0)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE5:
            FUN_00437f5c(22);
            break;
        case STAGE6A:
            g_GuiMessageStageMode = 2;
            break;
        case STAGE6B:
        {
            AnmLoaded *tmp = g_GuiPortraitAnm1;
            g_GuiPortraitAnm1 = g_GuiPortraitAnm2;
            g_GuiPortraitAnm2 = tmp;
            g_GuiMessageStageMode = 2;
            FUN_00437f5c(24);
            break;
        }
        case EXTRASTAGE:
        {
            AnmLoaded *tmp = g_GuiPortraitAnm1;
            g_GuiPortraitAnm1 = g_GuiPortraitAnm2;
            g_GuiPortraitAnm2 = tmp;
            g_GuiMessageStageMode = 2;
            FUN_00437f5c(25);
            break;
        }
        default:
            break;
        }
    }
    else if (value == 10)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE5:
            if (g_GameManager.globals->numRetries > 0)
            {
                value = 1;
                reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
            }
            else if (!g_GameManager.IsReplay())
            {
                if (g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, EASY) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, NORMAL) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, HARD) ||
                    g_GameManager.IsStageClearedWithoutRetries(STAGE6B, g_GameManager.shotType, LUNATIC) ||
                    g_GameManager.shotType > SHOT_YOUMU_YUYUKO)
                {
                    value = 3;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else if (g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, EASY) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, NORMAL) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, HARD) ||
                         g_GameManager.IsStageClearedWithRetries(STAGE6A, g_GameManager.shotType, LUNATIC))
                {
                    value = 2;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else
                {
                    value = 1;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
                }
            }
            else
            {
                if ((i8)g_ReplayManager->replayData->clearState == 2)
                {
                    value = 3;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else if ((i8)g_ReplayManager->replayData->clearState == 1)
                {
                    value = 2;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 1;
                }
                else
                {
                    value = 1;
                    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice = 0;
                }
            }
            g_GameManager.flags.isGoingToFinalB = reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->routeChoice;
            break;
        default:
            break;
        }
    }
    else if (value >= 6)
    {
        switch (g_GameManager.currentStage)
        {
        case STAGE6B:
            if ((i8)g_GameManager.GetClockTime() >= 12)
            {
                value = 5;
            }
            break;
        default:
            break;
        }
    }

    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentMsgIdx = value;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentInstr = reinterpret_cast<u8 **>(reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->msgFile)[value + 1];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[0].scriptIndex = -1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueLines[1].scriptIndex = -1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->messageFlag = 1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->fontSize = 15;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[0] = g_GuiMessageTextColors[g_GameManager.shotType].colors[0];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[1] = g_GuiMessageTextColors[g_GameManager.shotType].colors[1];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[2] = g_GuiMessageTextColors[g_GameManager.shotType].colors[2];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textColors[3] = g_GuiMessageTextColors[g_GameManager.shotType].colors[3];
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[0] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[1] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[2] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->shadowColors[3] = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->dialogueSkippable = 1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->waitThreshold = 6;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentSide = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->textPending = 1;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentLine = 0;
    reinterpret_cast<GuiMessageStateOverlay *>(&this->msgVm)->currentPortrait = 0xff;

    g_BulletManager.bulletmanager_fun_00415c60();
    g_EnemyManager.FUN_0042efb0(0, 0);
    g_ItemManager.AutoCollectAllItems();
}

// FUNCTION: th08 0x439810
void Gui::FUN_00439810(i32 value)
{
    this->impl->FUN_0043396d(value);
}

// FUNCTION: th08 0x439050
ZunResult Gui::FUN_00439050()
{
    this->timesAnm->SetSprite(
        reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C),
        static_cast<i8>(g_GameManager.GetClockTime()));
    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C)->SetInterrupt(1);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x439093
ZunResult Gui::FUN_00439093()
{
    this->timesAnm->SetSprite(
        reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C),
        static_cast<i8>(g_GameManager.GetClockTime()));
    reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this->impl) + 0x2156C)->SetInterrupt(2);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x438ff3
i32 FUN_00438ff3()
{
    return g_BulletManagerAnmReleaseRequired;
}

// FUNCTION: th08 0x438ffd
i32 FUN_00438ffd()
{
    return g_GuiAnmReleaseRequired;
}


// FUNCTION: th08 0x437ddd
void Gui::FUN_00437ddd(i32 value)
{
    this->impl->formatted0.position = Float3(416.0f, 48.0f, 0.0f);
    this->impl->formatted0.isShown = 1;
    this->impl->formatted0.timer = 0;
    this->impl->formatted0.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437e5d
void Gui::FUN_00437e5d(i32 value, i32 isShown)
{
    this->impl->formatted1.position = Float3(416.0f, 168.0f, 0.0f);
    this->impl->formatted1.isShown = isShown;
    this->impl->formatted1.timer = 0;
    this->impl->formatted1.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437edc
void Gui::gui_fun_00437edc(i32 value)
{
    this->impl->formatted2.position = Float3(224.0f, 16.0f, 0.0f);
    this->impl->formatted2.isShown = 1;
    this->impl->formatted2.timer = 0;
    this->impl->formatted2.value = value;
    g_GuiFullPowerModeFrames = 2;
}

// FUNCTION: th08 0x437bc4
GuiImpl::GuiImpl()
{
}

// FUNCTION: th08 0x437ce2
GuiMsgVm::GuiMsgVm()
{
}

// FUNCTION: th08 0x437d45
GuiFormattedText::GuiFormattedText()
{
}

// STUB: th08 0x4338ca
ChainCallbackResult Gui::OnUpdate(Gui *gui)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x433927
ChainCallbackResult Gui::OnDraw(Gui *gui)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x437a2f
ZunResult Gui::AddedCallback(Gui *gui)
{
    return gui->ActualAddedCallback();
}

// FUNCTION: th08 0x437a40
ZunResult Gui::DeletedCallback(Gui *gui)
{
    if (!FUN_00438ffd())
    {
        g_AnmManager->ReleaseAnm(13);
    }

    gui->FreeMsgFile();

    if (FUN_00438ff3())
    {
        g_AnmManager->ReleaseAnm(10);
        g_AnmManager->ReleaseAnm(12);
        g_AnmManager->ReleaseAnm(11);
        g_AnmManager->ReleaseAnm(14);
        ZUN_DELETE(gui->impl);
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x43587e
i32 Gui::MsgWait()
{
    if (this->impl == NULL)
        return 0;
    if (*(u32 *)((u8 *)this->impl + 0x22D78) > 0)
        return 0;
    return *(i32 *)((u8 *)this->impl + 0x2181C) >= 0;
}

// FUNCTION: th08 0x4358bb
i32 Gui::IsDialogPresent()
{
    if (this->impl == NULL)
        return 0;
    return *(i32 *)((u8 *)this->impl + 0x2181C) >= 0 || *(i32 *)((u8 *)this->impl + 0x2181C) == -2;
}

// FUNCTION: th08 0x437ad0
ZunResult Gui::RegisterChain()
{
    Gui *gui = &g_Gui;

    if (FUN_00438fe9())
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = ZUN_NEW(GuiImpl, "GUI");
    }

    g_GuiCalcChain.SetCallback((ChainCallback)Gui::OnUpdate);
    g_GuiCalcChain.addedCallback = (ChainLifetimeCallback)Gui::AddedCallback;
    g_GuiCalcChain.deletedCallback = (ChainLifetimeCallback)Gui::DeletedCallback;
    g_GuiCalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcChain, 15) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_GuiDrawChain.SetCallback((ChainCallback)Gui::OnDraw);
    g_GuiDrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawChain, 17);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x437d64
void Gui::CutChain()
{
    g_Chain.Cut(&g_GuiCalcChain);
    g_Chain.Cut(&g_GuiDrawChain);
}

// STUB: th08 0x4390ee
ZunResult Gui::ActualAddedCallback()
{
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x439710
ZunResult Gui::LoadMsg(const char *path)
{
    this->FreeMsgFile();
    this->impl->msgVm.msgFile = FileSystem::OpenFile(path, NULL, 0);
    if (this->impl->msgVm.msgFile == NULL)
    {
        g_GameErrorContext.Log("\x65\x72\x72\x6f\x72\x20\x3a\x20\x83\x81\x83\x62\x83\x5a\x81\x5b\x83\x57\x83\x74\x83\x40\x83\x43\x83\x8b\x20\x25\x73\x20\x82\xaa\x93\xc7\x82\xdd\x8d\x9e\x82\xdf\x82\xdc\x82\xb9\x82\xf1\x82\xc5\x82\xb5\x82\xbd\x0d\x0a", path);
        return ZUN_ERROR;
    }
    *(i32 *)((u8 *)this->impl + 0x2181C) = -1;
    *(i32 *)((u8 *)this->impl + 0x21818) = 0;
    for (i32 i = 0; i < *(i32 *)this->impl->msgVm.msgFile; ++i)
    {
        ((i32 *)this->impl->msgVm.msgFile)[i + 1] += (i32)this->impl->msgVm.msgFile;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4397d5
void Gui::FreeMsgFile(void)
{
    if (this->impl->msgVm.msgFile != NULL)
    {
        ZUN_FREE(this->impl->msgVm.msgFile);
    }
}

} /* namespace th08 */
