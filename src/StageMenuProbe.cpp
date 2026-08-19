#include "th_pch.h"
#include "AnmManager.hpp"
#include "EclManager.hpp"
#include "GameManager.hpp"
#include "Supervisor.hpp"

namespace th08
{
struct PauseMenuProbe
{
    u32 curState;
    i32 numFrames;
    AnmVm menuSprites[10];
    AnmVm menuBackground;
    void OnDraw();
};
C_ASSERT(sizeof(PauseMenuProbe) == 0x1d14);

struct RetryMenuProbe
{
    u32 curState;
    i32 numFrames;
    AnmVm menuSprites[6];
    AnmVm menuBackground;
    void OnDraw();
};
C_ASSERT(sizeof(RetryMenuProbe) == 0x1284);

#pragma var_order(menuBackground, vmIdx)
void PauseMenuProbe::OnDraw()
{
    u32 vmIdx;
    if (g_GameManager.isInGameMenu)
    {
        g_AnmManager->FlushVertexBuffer();
        g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
        g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
        g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        if (((g_EclGameTimeScaleFlags >> 1) & 1) != 0 && this->curState != 0)
        {
            AnmVm menuBackground = this->menuBackground;
            menuBackground.zWriteDisabled = TRUE;
            g_AnmManager->DrawNoRotation(&menuBackground);
        }
        for (vmIdx = 0; vmIdx < 10; vmIdx++)
        {
            if (this->menuSprites[vmIdx].IsVisible())
                g_AnmManager->DrawNoRotation(&this->menuSprites[vmIdx]);
        }
    }
}

#pragma var_order(vmIdx)
void RetryMenuProbe::OnDraw()
{
    i32 vmIdx;
    if (g_GameManager.showRetryMenu)
    {
        g_AnmManager->FlushVertexBuffer();
        g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
        g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
        g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        if (((g_EclGameTimeScaleFlags >> 1) & 1) != 0 && (this->curState != 0 || this->numFrames > 2))
            g_AnmManager->DrawNoRotation(&this->menuBackground);

        if (!g_GameManager.IsPracticeMode() && g_GameManager.difficulty < EXTRA)
        {
            for (vmIdx = 0; vmIdx < 4; vmIdx++)
                if (this->menuSprites[vmIdx].IsVisible())
                    g_AnmManager->DrawNoRotation(&this->menuSprites[vmIdx]);
        }
        else
        {
            for (vmIdx = 0; vmIdx < 3; vmIdx++)
                if (this->menuSprites[vmIdx].IsVisible())
                    g_AnmManager->DrawNoRotation(&this->menuSprites[vmIdx]);
        }
    }
}
} // namespace th08
