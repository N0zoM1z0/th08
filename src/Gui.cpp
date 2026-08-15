#include "th_pch.h"

#include "BulletManager.hpp"
#include "Gui.hpp"

namespace th08
{

DIFFABLE_STATIC(Gui, g_Gui);
DIFFABLE_STATIC(ChainElem, g_GuiCalcChain);
DIFFABLE_STATIC(ChainElem, g_GuiDrawChain);
DIFFABLE_STATIC(i32, g_GuiAnmReleaseRequired);

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

// STUB: th08 0x437ad0
ZunResult Gui::RegisterChain()
{
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

// STUB: th08 0x439710
ZunResult Gui::LoadMsg(const char *path)
{
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4397d5
void Gui::FreeMsgFile(void)
{
    if (this->impl->msgFile != NULL)
    {
        ZUN_FREE(this->impl->msgFile);
    }
}

} /* namespace th08 */
