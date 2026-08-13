#include "th_pch.h"

#include "Gui.hpp"

namespace th08
{

DIFFABLE_STATIC(Gui, g_Gui);
DIFFABLE_STATIC(ChainElem, g_GuiCalcChain);
DIFFABLE_STATIC(ChainElem, g_GuiDrawChain);

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

// STUB: th08 0x437a40
ZunResult Gui::DeletedCallback(Gui *gui)
{
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

// STUB: th08 0x4397d5
void Gui::FreeMsgFile(void)
{
}

} /* namespace th08 */
