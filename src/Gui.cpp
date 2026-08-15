#include "th_pch.h"

#include "BulletManager.hpp"
#include "Gui.hpp"

namespace th08
{

DIFFABLE_STATIC(Gui, g_Gui);
DIFFABLE_STATIC(ChainElem, g_GuiCalcChain);
DIFFABLE_STATIC(ChainElem, g_GuiDrawChain);
DIFFABLE_STATIC(i32, g_GuiAnmReleaseRequired);
DIFFABLE_STATIC(i32, g_GuiResourceReloadEnabled);
DIFFABLE_STATIC(i32, g_GuiFullPowerModeFrames);

// FUNCTION: th08 0x438fe9
i32 FUN_00438fe9()
{
    return g_GuiResourceReloadEnabled;
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


// FUNCTION: th08 0x437e5d
void Gui::FUN_00437e5d(i32 value, i32 isShown)
{
    this->impl->formatted1.position = Float3(416.0f, 168.0f, 0.0f);
    this->impl->formatted1.isShown = isShown;
    this->impl->formatted1.timer = 0;
    this->impl->formatted1.value = value;
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
