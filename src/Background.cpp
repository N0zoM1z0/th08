#include "th_pch.h"

#include "Background.hpp"

namespace th08
{
DIFFABLE_STATIC(Background, g_Background);

// STUB: th08 0x4071a0
Background::Background()
{
}

// STUB: th08 0x407400
ChainCallbackResult Background::OnUpdate(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x409200
ChainCallbackResult Background::OnDrawHighPrio(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x409640
ChainCallbackResult Background::OnDrawLowPrio(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// STUB: th08 0x409850
ZunResult Background::AddedCallback(Background *background)
{
    return ZUN_ERROR;
}

// STUB: th08 0x409b20
ZunResult Background::RegisterChain()
{
    return ZUN_ERROR;
}

// STUB: th08 0x409c20
ZunResult Background::DeletedCallback()
{
    return ZUN_ERROR;
}

// STUB: th08 0x409ca0
void Background::CutChain()
{
}

// STUB: th08 0x409ce0
ZunResult Background::LoadStageData()
{
    return ZUN_ERROR;
}

}; // Namespace th08
