#define _WIN32_WINNT 0x0500

#include "th_pch.h"

#include "AnmManager.hpp"
#include "Background.hpp"
#include "GameManager.hpp"
#include "Global.hpp"
#include "ResultScreen.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Supervisor.hpp" // Official name: mother.hpp
#include "ZunBool.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "i18n.hpp"
#include "inttypes.hpp"
#include <d3dx8.h>
#include <direct.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winnls32.h>

namespace th08
{
enum RenderResult
{
    RENDER_RESULT_KEEP_RUNNING = 0,
    RENDER_RESULT_EXIT_SUCCESS = 1,
    RENDER_RESULT_EXIT_ERROR = 2,
    RENDER_RESULT_EXIT_SUCCESS_2 = -1
};

// MSVC tries to align 64-bit types even on 32-bit builds, so the pack is required
#pragma pack(4)
struct GameWindow
{
    HWND window;
    ZunBool windowIsClosing; // Kept from previous games, but never set to true in IN
    ZunBool windowIsActive;
    ZunBool windowIsInactive;
    i8 framesSinceRedraw;
    LARGE_INTEGER pcFrequency;
    u8 usesRelativePath; // Disables vsync when set
    ZunBool screenSaveActive;
    ZunBool lowPowerActive;
    ZunBool powerOffActive;
    f64 curTimestamp;
    f64 lastTimestamp;
    f64 lastFrameTime;

    GameWindow()
    {
        memset(this, 0, sizeof(*this));
    }

    RenderResult Render();
    static void Present();
    f64 GetTimestamp();
    static ZunBool InitD3DInterface();
    static ZunBool CreateGameWindow(HINSTANCE hInstance);
    static LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static ZunBool InitD3DRendering();
    static void FormatD3DCapabilities(D3DCAPS8 *caps, char *buf);
    static char *FormatCapability(char *capabilityName, u32 capabilityFlags, u32 mask, char *buf);
    static void ResetRenderState();
    static ZunResult CheckForRunningGameInstance(HINSTANCE hInstance);
    static void ActivateWindow(HWND hWnd);
    static i32 CalcExecutableChecksum();
    static ZunBool ResolveIt(char *shortcutPath, char *dstPath, i32 maxPathLen);
};
C_ASSERT(sizeof(GameWindow) == 0x44);

DIFFABLE_STATIC(HANDLE, g_ExclusiveMutex);
DIFFABLE_STATIC(GameWindow, g_GameWindow);
}; // namespace th08

using namespace th08;

#pragma var_order(d3dDeviceStatus, msg, renderResult, i)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR pCmdLine, int nCmdShow)
{
    HRESULT d3dDeviceStatus;
    i32 i;
    MSG msg;
    i32 renderResult;

    renderResult = RENDER_RESULT_KEEP_RUNNING;

    g_Supervisor.hInstance = hInstance;

    SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfoA(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfoA(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, (LPVOID *)false, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, 0, (LPVOID *)false, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, 0, (LPVOID *)false, SPIF_SENDCHANGE);

    g_Supervisor.InitializeCriticalSections();
    g_GameErrorContext.Log(TH_ERR_LOGGER_START);

    if (GameWindow::CheckForRunningGameInstance(hInstance) == ZUN_ERROR)
    {
        goto stop;
    }

    if (g_Supervisor.LoadConfig("th08.cfg") != ZUN_SUCCESS)
    {
        goto stop;
    }

    GameWindow::CalcExecutableChecksum();
    QueryPerformanceFrequency(&g_GameWindow.pcFrequency);

restart:
    if (GameWindow::InitD3DInterface())
    {
        goto stop;
    }

    if (GameWindow::CreateGameWindow(hInstance))
    {
        goto stop;
    }

    if (GameWindow::InitD3DRendering())
    {
        goto stop;
    }

    g_SoundPlayer.InitializeDSound(g_GameWindow.window);
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();

    g_AnmManager = ZUN_NEW(AnmManager, "SprtCtrlInf");

    if (!g_Supervisor.IsWindowed())
    {
        WINNLSEnableIME(NULL, FALSE);
        ShowCursor(FALSE);
        SetCursor(NULL);
    }

    renderResult = Supervisor::RegisterChain();

    if (renderResult != RENDER_RESULT_KEEP_RUNNING)
    {
        // This seems to be the only way to match assembly? But also why would anyone write code this way
        if (renderResult == RENDER_RESULT_EXIT_SUCCESS_2)
        {
            goto awfulConditionalBreak;
        }

        renderResult = RENDER_RESULT_EXIT_ERROR;
    }
    else
    {
        renderResult = RENDER_RESULT_KEEP_RUNNING;

        g_GameWindow.framesSinceRedraw = -4;
        g_GameWindow.lastFrameTime = 0;
        g_GameWindow.lastTimestamp = g_GameWindow.lastFrameTime;
        g_GameWindow.curTimestamp = g_GameWindow.lastTimestamp;

        while (!g_GameWindow.windowIsClosing)
        {
            if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
            else
            {
                d3dDeviceStatus = g_Supervisor.d3dDevice->TestCooperativeLevel();

                if (d3dDeviceStatus == D3D_OK)
                {
                    renderResult = g_GameWindow.Render();

                    if (renderResult != RENDER_RESULT_KEEP_RUNNING)
                    {
                        break;
                    }

                    g_Supervisor.flags.d3dDevDisconnectFlag = 0;
                }
                else if (d3dDeviceStatus == D3DERR_DEVICENOTRESET)
                {
                    g_AnmManager->ReleaseSurfaces();

                    if (g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters) != D3D_OK)
                    {
                        break;
                    }

                    GameWindow::ResetRenderState();
                    g_Supervisor.unk174 = 3;
                    g_Supervisor.flags.d3dDevDisconnectFlag = 1;
                }
            }
        }
    }

awfulConditionalBreak:
    if (g_GameManager.plst.base.magic != 0)
    {
        ResultScreen::RegisterChain(2);
    }

    g_Chain.Release();

    while (g_SoundPlayer.ProcessQueues() != 0)
        ;

stop:
    Sleep(1000);

    g_SoundPlayer.Release();
    ZUN_DELETE(g_AnmManager);

    if (g_Supervisor.d3dDevice != NULL)
    {
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
    }

    if (g_Supervisor.d3dDevice != NULL)
    {
        g_Supervisor.d3dDevice->Release();
        g_Supervisor.d3dDevice = NULL;
    }

    if (g_Supervisor.d3dIface != NULL)
    {
        g_Supervisor.d3dIface->Release();
        g_Supervisor.d3dIface = NULL;
    }

    if (g_GameWindow.window != NULL)
    {
        ShowWindow(g_GameWindow.window, SW_HIDE);
        MoveWindow(g_GameWindow.window, 0, 0, 0, 0, FALSE);
        DestroyWindow(g_GameWindow.window);
        g_GameWindow.window = NULL;
    }

    ShowCursor(TRUE);

    if (renderResult == RENDER_RESULT_EXIT_ERROR)
    {
        g_GameErrorContext.ResetContext();
        g_GameErrorContext.Log(TH_ERR_OPTION_CHANGED_RESTART);

        if (!g_Supervisor.IsWindowed())
        {
            WINNLSEnableIME(NULL, TRUE);
        }

        for (i = 0; i < 60;)
        {
            if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }

            i++;
        }

        goto restart;
    }

    FileSystem::WriteDataToFile("th08.cfg", &g_Supervisor.cfg, 60);

    if (g_Supervisor.midiOutput != NULL)
    {
        g_Supervisor.midiOutput->StopPlayback();
        g_ZunMemory.RemoveFromRegistry(g_Supervisor.midiOutput);
        delete g_Supervisor.midiOutput;
        g_Supervisor.midiOutput = NULL;
    }

    g_GameErrorContext.Flush();
    g_Supervisor.DeleteCriticalSections();

    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);
    WINNLSEnableIME(NULL, TRUE);

    return 0;
}

RenderResult GameWindow::Render()
{
    i32 calcChainResult;

    this->curTimestamp = this->GetTimestamp();

    // Safeguard in case of timestamp overflow or other weirdness
    if (this->lastTimestamp > this->curTimestamp)
    {
        this->lastFrameTime = this->curTimestamp;
    }

    this->lastTimestamp = this->curTimestamp;

    if (this->lastFrameTime < this->curTimestamp)
    {

        while (this->lastFrameTime < this->curTimestamp)
        {
            this->lastFrameTime += (1.0f / 60);
        }

        g_AnmManager->FlushVertexBuffer();

        g_Supervisor.viewport.X = 0;
        g_Supervisor.viewport.Y = 0;
        g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
        g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;

        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

        calcChainResult = g_Chain.RunCalcChain();
        g_SoundPlayer.ProcessQueues();

        if (calcChainResult == 0)
        {
            g_Supervisor.ThreadClose();
            return RENDER_RESULT_EXIT_SUCCESS;
        }
        else if (calcChainResult == -1)
        {
            g_Supervisor.ThreadClose();
            return RENDER_RESULT_EXIT_ERROR;
        }

        this->framesSinceRedraw++;

        if (g_Supervisor.cfg.frameskipConfig <= this->framesSinceRedraw)
        {
            g_Supervisor.d3dDevice->BeginScene();
            g_AnmManager->ClearVertexBuffer();
            g_Supervisor.fogState = FOG_UNSET;
            g_Supervisor.DisableFog();
            g_Chain.RunDrawChain();
            g_AnmManager->FlushVertexBuffer();
            g_Supervisor.d3dDevice->SetTexture(0, NULL);
            g_Supervisor.d3dDevice->EndScene();
            this->framesSinceRedraw = 0;
        }

        this->curTimestamp = this->GetTimestamp();
        Present();
    }
    else
    {
        Sleep(0);
    }

    return RENDER_RESULT_KEEP_RUNNING;
}

#pragma var_order(i, snapshotPath)
void GameWindow::Present()
{
    i32 i;
    char snapshotPath[0x100];

    if (g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_AnmManager->ReleaseSurfaces();
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
        ResetRenderState();

        g_Supervisor.unk174 = 2;
    }

    g_AnmManager->TakeScreencaptures();

    if (WAS_PRESSED(TH_BUTTON_HOME))
    {
        _mkdir("snapshot");

        for (i = 0; i < 1000; i++)
        {
            sprintf(snapshotPath, "snapshot/th%.3d.bmp", i);

            if (!FileSystem::CheckIfFileAlreadyExists(snapshotPath))
            {
                break;
            }
        }

        if (i < 1000)
        {
            g_Supervisor.TakeSnapshot(snapshotPath);
        }
    }

    if (g_Supervisor.unk174 != 0 && !g_GameManager.isInGameMenu)
    {
        g_Supervisor.unk174--;
    }
}

#pragma var_order(performanceCounterValue, timestamp)
f64 GameWindow::GetTimestamp()
{
    LARGE_INTEGER performanceCounterValue;
    f64 timestamp;

    if (g_GameWindow.pcFrequency.LowPart != 0)
    {
        QueryPerformanceCounter(&performanceCounterValue);
        return (f64)performanceCounterValue.LowPart / (f64)g_GameWindow.pcFrequency.LowPart;
    }

    timeBeginPeriod(1);
    timestamp = timeGetTime();
    timeEndPeriod(1);

    return timestamp;
}

ZunBool GameWindow::InitD3DInterface()
{
    g_Supervisor.d3dIface = Direct3DCreate8(D3D_SDK_VERSION);

    if (g_Supervisor.d3dIface == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_D3D_ERR_COULD_NOT_CREATE_OBJ);
        return true;
    }

    return false;
}

#pragma var_order(height, width, baseClass)
ZunBool GameWindow::CreateGameWindow(HINSTANCE hInstance)
{
    WNDCLASS baseClass;
    i32 height;
    i32 width;

    memset(&baseClass, 0, sizeof(baseClass));

    baseClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    baseClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    baseClass.hInstance = hInstance;
    baseClass.lpfnWndProc = WindowProc;
    g_GameWindow.windowIsActive = true;
    g_GameWindow.windowIsInactive = false;
    baseClass.lpszClassName = "BASE";
    RegisterClassA(&baseClass);

    if (!g_Supervisor.IsWindowed())
    {
        width = GAME_WINDOW_WIDTH;
        height = GAME_WINDOW_HEIGHT;

        g_GameWindow.window = CreateWindowExA(0, "BASE", TH_WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 0, 0, width, height,
                                              NULL, NULL, hInstance, NULL);
    }
    else
    {
        width = GetSystemMetrics(SM_CXDLGFRAME) * 2 + GAME_WINDOW_WIDTH;
        height = GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + GAME_WINDOW_HEIGHT;

        g_GameWindow.window = CreateWindowExA(0, "BASE", TH_WINDOW_TITLE, WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU,
                                              CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
    }

    g_Supervisor.hwndGameWindow = g_GameWindow.window;

    if (g_GameWindow.window == NULL)
    {
        return true;
    }

    ActivateWindow(g_GameWindow.window);
    return false;
}

LRESULT __stdcall GameWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1; // Indicates that IN can erase the background
    case MM_MOM_DONE:
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->UnprepareHeader((LPMIDIHDR)lParam);
        }

        break;
    case WM_ACTIVATEAPP:
        g_GameWindow.windowIsActive = wParam;

        if (g_GameWindow.windowIsActive)
        {
            g_GameWindow.windowIsInactive = false;
        }
        else
        {
            g_GameWindow.windowIsInactive = true;
        }

        break;
    case WM_SETCURSOR:
        if (!g_Supervisor.IsWindowed())
        {
            if (g_GameWindow.windowIsInactive)
            {
                SetCursor(LoadCursorA(NULL, IDC_ARROW));
                ShowCursor(TRUE);
            }
            else
            {
                ShowCursor(NULL);
                SetCursor(NULL);
            }
        }
        else
        {
            SetCursor(LoadCursorA(NULL, IDC_ARROW));
            ShowCursor(TRUE);
        }

        return TRUE;
    case WM_CLOSE:
        g_Supervisor.flags.receivedCloseMsg = true;
        return 1;
    }

    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

#pragma var_order(failedToSetFramerate, usingHardwareRenderer, displayMode, presentParams, cameraDistance, halfHeight, \
                  halfWidth, aspectRatio, fov, capabilitiesBuf)
ZunBool GameWindow::InitD3DRendering()
{
    f32 aspectRatio;
    f32 cameraDistance;
    char capabilitiesBuf[0x2000];
    D3DDISPLAYMODE displayMode;
    ZunBool failedToSetFramerate;
    f32 fov;
    f32 halfHeight;
    f32 halfWidth;
    D3DPRESENT_PARAMETERS presentParams;
    u8 usingHardwareRenderer;

    usingHardwareRenderer = true;

    memset(&presentParams, 0, sizeof(presentParams));

    g_Supervisor.d3dIface->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
    if (!g_Supervisor.cfg.windowed)
    {
        if (g_Supervisor.Is16bitColorMode() == TRUE)
        {
            presentParams.BackBufferFormat = D3DFMT_R5G6B5;
            g_Supervisor.cfg.colorMode16bit = TRUE;
        }
        // Used in cases of corrupt or missing config files in earlier games. Dead code in IN
        else if (g_Supervisor.cfg.colorMode16bit == 0xff)
        {
            presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
            g_Supervisor.cfg.colorMode16bit = 0;
            g_GameErrorContext.Log(TH_DBG_SCREEN_INIT_32BITS);
        }
        else if (g_Supervisor.cfg.colorMode16bit == 0)
        {
            presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
        }
        else
        {
            presentParams.BackBufferFormat = D3DFMT_R5G6B5;
        }

        // ?????? Not sure why this is here
        if (g_GameWindow.usesRelativePath)
        {
            g_Supervisor.disableVsync = true;
        }

        if (!g_Supervisor.disableVsync)
        {
            presentParams.FullScreen_RefreshRateInHz = 60;
            presentParams.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
            g_GameErrorContext.Log(TH_DBG_SET_REFRESH_RATE_60HZ);

            if (g_Supervisor.cfg.frameskipConfig == 0)
            {
                presentParams.SwapEffect = D3DSWAPEFFECT_FLIP;
            }
            else
            {
                presentParams.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
            }
        }
        else
        {
            presentParams.FullScreen_RefreshRateInHz = 0;
            presentParams.SwapEffect = D3DSWAPEFFECT_COPY;
            presentParams.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            g_GameErrorContext.Log(TH_DBG_TRY_ASYNC_VSYNC);
        }
    }
    else
    {
        presentParams.BackBufferFormat = displayMode.Format;
        presentParams.SwapEffect = D3DSWAPEFFECT_COPY;
        presentParams.Windowed = TRUE;
    }

    presentParams.BackBufferWidth = GAME_WINDOW_WIDTH;
    presentParams.BackBufferHeight = GAME_WINDOW_HEIGHT;
    presentParams.EnableAutoDepthStencil = TRUE;
    presentParams.AutoDepthStencilFormat = D3DFMT_D16;
    presentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    g_Supervisor.flags.unk1 = true;
    g_Supervisor.couldSetRefreshRate = true;
    failedToSetFramerate = false;

    for (;;)
    {
        if (g_Supervisor.IsReferenceRasterizerMode())
        {
            goto REFERENCE_RASTERIZER_MODE;
        }
        else
        {

            if (g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, g_GameWindow.window,
                                                    D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams,
                                                    &g_Supervisor.d3dDevice) < 0)
            {
                if (failedToSetFramerate)
                {
                    g_GameErrorContext.Log(TH_DBG_TL_HAL_UNAVAILABLE);
                }

                if (g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, g_GameWindow.window,
                                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams,
                                                        &g_Supervisor.d3dDevice) < 0)
                {
                    if (failedToSetFramerate)
                    {
                        g_GameErrorContext.Log(TH_DBG_HAL_UNAVAILABLE);
                    }

                REFERENCE_RASTERIZER_MODE:
                    if (g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_REF, g_GameWindow.window,
                                                            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams,
                                                            &g_Supervisor.d3dDevice) < 0)
                    {
                        if (!g_Supervisor.disableVsync)
                        {
                            g_GameErrorContext.Log(TH_DBG_CANT_SET_REFRESH_RATE);
                            presentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
                            g_Supervisor.couldSetRefreshRate = false;
                            failedToSetFramerate = true;

                            continue;
                        }
                        else
                        {
                            if (presentParams.FullScreen_PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE)
                            {
                                g_GameErrorContext.Log(TH_ERR_ASYNC_VSYNC_UNSUPPORTED);
                                g_GameErrorContext.Fatal(TH_ERR_CHANGE_REFRESH_RATE);
                                presentParams.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
                                presentParams.SwapEffect = D3DSWAPEFFECT_COPY;

                                continue;
                            }
                            else
                            {
                                g_GameErrorContext.Fatal(TH_ERR_D3D_INIT_FAILED);

                                if (g_Supervisor.d3dIface != NULL)
                                {
                                    g_Supervisor.d3dIface->Release();
                                    g_Supervisor.d3dIface = NULL;
                                }

                                return true;
                            }
                        }
                    }
                    else
                    {
                        g_GameErrorContext.Log(TH_DBG_USING_REF_MODE);
                        g_Supervisor.flags.usingHardwareTL = false;
                        usingHardwareRenderer = false;
                    }
                }
                else
                {
                    g_GameErrorContext.Log(TH_DBG_USING_HAL_MODE);
                    g_Supervisor.flags.usingHardwareTL = false;
                }
            }
            else
            {
                g_GameErrorContext.Log(TH_DBG_USING_TL_HAL_MODE);
                g_Supervisor.flags.usingHardwareTL = true;
            }

            break;
        }
    }

    memcpy(&g_Supervisor.presentParameters, &presentParams, sizeof(presentParams));

    halfWidth = GAME_WINDOW_WIDTH / 2.0f;
    halfHeight = GAME_WINDOW_HEIGHT / 2.0f;
    aspectRatio = (f32)GAME_WINDOW_WIDTH / (f32)GAME_WINDOW_HEIGHT;
    fov = ZUN_PI / 6;
    cameraDistance = halfHeight / (f32)tan(fov / 2.0f);

    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix, &D3DXVECTOR3(halfWidth, -halfHeight, -cameraDistance),
                       &D3DXVECTOR3(halfWidth, -halfHeight, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 100.0f, 10000.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
    g_Supervisor.d3dDevice->GetViewport(&g_Supervisor.viewport);
    g_Supervisor.d3dDevice->GetDeviceCaps(&g_Supervisor.d3dCaps);

    // This is dead code because from PCB onwards the bit that indicated software texture blending in EoSD
    //   is set true unconditionally in the config load function and then ignored (HW blending is always used)
    //   Therefore IsHardwareBlendingDisabled will always return true here
    if (!g_Supervisor.IsHardwareBlendingDisabled() && !(g_Supervisor.d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADD))
    {
        g_GameErrorContext.Log(TH_ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD);
        g_Supervisor.cfg.opts.useSwTextureBlending = true;
    }

    if (g_Supervisor.d3dCaps.MaxTextureWidth <= 256)
    {
        g_GameErrorContext.Log(TH_ERR_NO_LARGE_TEXTURE_SUPPORT);
    }

    FormatD3DCapabilities(&g_Supervisor.d3dCaps, capabilitiesBuf);
    g_GameErrorContext.Log(capabilitiesBuf);

    if (!g_Supervisor.Is16bitColorMode() && usingHardwareRenderer)
    {
        if (g_Supervisor.d3dIface->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, presentParams.BackBufferFormat,
                                                     0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == D3D_OK)
        {
            g_Supervisor.flags.using32BitGraphics = true;
        }
        else
        {
            g_Supervisor.flags.using32BitGraphics = false;
            g_Supervisor.cfg.opts.force16bitTextures = true;
            g_GameErrorContext.Log(TH_ERR_D3DFMT_A8R8G8B8_UNSUPPORTED);
        }
    }

    ResetRenderState();
    ScreenEffect::SetViewport(COLOR_BLACK);
    g_GameWindow.windowIsClosing = false;
    g_Supervisor.lastFrameTime = 0;
    return false;
}

void GameWindow::FormatD3DCapabilities(D3DCAPS8 *caps, char *buf)
{
    char *strPos;

    strPos = buf;

    strPos += sprintf(strPos, TH_DBG_D3D_CAPS_START);
    strPos = FormatCapability(TH_DBG_CAPS_READ_SCANLINE, caps->Caps, D3DCAPS_READ_SCANLINE, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_WINDOW_MODE_RENDERING, caps->Caps2, D3DCAPS2_CANRENDERWINDOWED, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_IMMEDIATE_PRESENTATION_SWAP, caps->PresentationIntervals,
                              D3DPRESENT_INTERVAL_IMMEDIATE, strPos);
    strPos =
        FormatCapability(TH_DBG_CAPS_PRESENTATION_VSYNC, caps->PresentationIntervals, D3DPRESENT_INTERVAL_ONE, strPos);

    strPos += sprintf(strPos, TH_DBG_CAPS_DEVICE_START);
    strPos = FormatCapability(TH_DBG_CAPS_NONLOCAL_VRAM_BLIT, caps->DevCaps, D3DDEVCAPS_CANBLTSYSTONONLOCAL, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_HARDWARE_TL, caps->DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_TEXTURES_FROM_NONLOCAL_VRAM, caps->DevCaps, D3DDEVCAPS_TEXTURENONLOCALVIDMEM,
                              strPos);
    strPos =
        FormatCapability(TH_DBG_CAPS_TEXTURES_FROM_MAIN_MEMORY, caps->DevCaps, D3DDEVCAPS_TEXTURESYSTEMMEMORY, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_TEXTURES_FROM_VRAM, caps->DevCaps, D3DDEVCAPS_TEXTUREVIDEOMEMORY, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_VERTEX_BUFFER_IN_RAM, caps->DevCaps, D3DDEVCAPS_TLVERTEXSYSTEMMEMORY, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_VERTEX_BUFFER_IN_VRAM, caps->DevCaps, D3DDEVCAPS_TLVERTEXVIDEOMEMORY, strPos);

    strPos += sprintf(strPos, TH_DBG_CAPS_PRIMITIVES_START);
    strPos = FormatCapability(TH_DBG_CAPS_ALPHA_BLENDING, caps->PrimitiveMiscCaps, D3DPMISCCAPS_BLENDOP, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_POINT_CLIPPING, caps->PrimitiveMiscCaps, D3DPMISCCAPS_CLIPPLANESCALEDPOINTS,
                              strPos);
    strPos = FormatCapability(TH_DBG_CAPS_VERTEX_CLIPPING, caps->PrimitiveMiscCaps, D3DPMISCCAPS_CLIPTLVERTS, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_CULL_CCW, caps->PrimitiveMiscCaps, D3DPMISCCAPS_CULLCCW, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_CULL_CW, caps->PrimitiveMiscCaps, D3DPMISCCAPS_CULLCW, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_CULL_NONE, caps->PrimitiveMiscCaps, D3DPMISCCAPS_CULLNONE, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_DEPTH_TEST_TOGGLE, caps->PrimitiveMiscCaps, D3DPMISCCAPS_MASKZ, strPos);

    strPos += sprintf(strPos, TH_DBG_CAPS_RASTERIZER_START);
    strPos = FormatCapability(TH_DBG_CAPS_ANISOTROPIC_FILTERING, caps->RasterCaps, D3DPRASTERCAPS_ANISOTROPY, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_ANTIALIASING, caps->RasterCaps, D3DPRASTERCAPS_ANTIALIASEDGES, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_DITHERING, caps->RasterCaps, D3DPRASTERCAPS_DITHER, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_RANGE_BASED_FOG, caps->RasterCaps, D3DPRASTERCAPS_FOGRANGE, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_Z_BASED_FOG, caps->RasterCaps, D3DPRASTERCAPS_ZFOG, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_TABLE_BASED_FOG, caps->RasterCaps, D3DPRASTERCAPS_FOGTABLE, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_VERTEX_BASED_FOG, caps->RasterCaps, D3DPRASTERCAPS_FOGVERTEX, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_DEPTH_TEST, caps->RasterCaps, D3DPRASTERCAPS_ZTEST, strPos);

    strPos += sprintf(strPos, TH_DBG_CAPS_SHADING_START);
    strPos = FormatCapability(TH_DBG_CAPS_GOURAUD_SHADING, caps->ShadeCaps, D3DPSHADECAPS_COLORGOURAUDRGB, strPos);
    strPos =
        FormatCapability(TH_DBG_CAPS_ALPHA_GOURAUD_SHADING, caps->ShadeCaps, D3DPSHADECAPS_ALPHAGOURAUDBLEND, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_GOURAUD_SHADED_FOG, caps->ShadeCaps, D3DPSHADECAPS_FOGGOURAUD, strPos);

    strPos += sprintf(strPos, TH_DBG_CAPS_TEXTURE_START);
    strPos += sprintf(strPos, TH_DBG_CAPS_LARGEST_TEXTURE, caps->MaxTextureWidth, caps->MaxTextureHeight);
    strPos = FormatCapability(TH_DBG_CAPS_ALPHA_IN_TEXTURE, caps->TextureCaps, D3DPTEXTURECAPS_ALPHA, strPos);
    strPos = FormatCapability(TH_DBG_CAPS_TEXTURE_TRANSFORM, caps->TextureCaps, D3DPTEXTURECAPS_PROJECTED, strPos);
    strPos =
        FormatCapability(TH_DBG_CAPS_BILINEAR_FILTER_MAG, caps->TextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR, strPos);
    strPos =
        FormatCapability(TH_DBG_CAPS_BILINEAR_FILTER_MIN, caps->TextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR, strPos);

    strPos += sprintf(strPos, TH_DBG_CAPS_END);
}

char *GameWindow::FormatCapability(char *capabilityName, u32 capabilityFlags, u32 mask, char *buf)
{
    // Who needs strcpy when you have sprintf?
    buf += sprintf(buf, capabilityName);

    if ((capabilityFlags & mask) == 0)
    {
        buf += sprintf(buf, TH_DBG_CAPABILITY_NOT_PRESENT);
    }
    else
    {
        buf += sprintf(buf, TH_DBG_CAPABILITY_PRESENT);
    }

    return buf;
}

#pragma var_order(fogVal, fogDensity)
void GameWindow::ResetRenderState()
{
    // Required to pass floats to SetTextureStageState without implicit conversion to int
    f32 fogDensity;
    f32 fogVal;

    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZENABLE, true);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    }

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHAREF, 4);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGENABLE, true);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGENABLE, false);
    }

    fogDensity = 1.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(u32 *)&fogDensity);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, COLOR_LIGHT_GREY);

    fogVal = 1000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)&fogVal);
    fogVal = 5000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)&fogVal);

    // Always evaluates true because Zun mistakenly used bitwise or instead of and
    // Doesn't matter, because it disables what it tried to test for anyway
    if (g_Supervisor.d3dCaps.RasterCaps | D3DPRASTERCAPS_ANTIALIASEDGES)
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_EDGEANTIALIAS, false);
    }

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);

    // Alpha texture settings
    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }

    // Color texture settings
    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    }

    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

    if (g_AnmManager != NULL)
    {
        g_AnmManager->ClearBlendMode();
        g_AnmManager->ClearColorOp();
        g_AnmManager->ClearVertexShader();
        g_AnmManager->ClearTexture();
        g_AnmManager->ClearCameraSettings();
    }

    g_Background.skyFogNeedsSetup = true;
}

#pragma var_order(moduleFilenameBuf, startupInfo, consoleTitleBuf, fileExtension)
ZunResult GameWindow::CheckForRunningGameInstance(HINSTANCE hInstance)
{
    char consoleTitleBuf[MAX_PATH + 1];
    char *fileExtension;
    char moduleFilenameBuf[MAX_PATH + 1];
    STARTUPINFO startupInfo;

    g_ExclusiveMutex = CreateMutexA(NULL, TRUE, "Touhou 08 App");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        g_GameErrorContext.Fatal(TH_ERR_ALREADY_RUNNING);
        return ZUN_ERROR;
    }

    startupInfo.cb = sizeof(startupInfo);
    memset(&startupInfo.lpReserved, 0, sizeof(startupInfo) - 4); // Fill remaining struct members

    // GetModuleFileNameA will get the absolute path of the TH08 executable
    // GetConsoleTitleA will always fail(?) because TH08 isn't a console application. consoleTitleBuf is clobbered
    // before being read anyway GetStartupInfoA will return in lpTitle the absolute path of the executable/shortcut when
    // launched from a graphical shell
    //   or the relative path the TH08 executable was launched with when launched from a console
    GetModuleFileNameA(NULL, moduleFilenameBuf, ARRAY_SIZE(moduleFilenameBuf));
    GetConsoleTitleA(consoleTitleBuf, ARRAY_SIZE(consoleTitleBuf));
    GetStartupInfoA(&startupInfo);

    if (startupInfo.lpTitle != NULL)
    {
        fileExtension = strrchr(startupInfo.lpTitle, '.');
        if (FileSystem::CheckIfFileAlreadyExists(startupInfo.lpTitle) && fileExtension != NULL)
        {
            if (_stricmp(fileExtension, ".lnk") == 0)
            {
                do
                {
                    ResolveIt(startupInfo.lpTitle, consoleTitleBuf, MAX_PATH);
                    fileExtension = strrchr(consoleTitleBuf, '.');
                } while ((_stricmp(fileExtension, ".lnk") == 0));
            }
            else
            {
                strcpy(consoleTitleBuf, startupInfo.lpTitle);
            }

            if (strcmp(moduleFilenameBuf, consoleTitleBuf) != 0)
            {
                g_GameWindow.usesRelativePath = true;
            }
        }

        g_Supervisor.flags.unk6 = false;
    }
    else
    {
        g_Supervisor.flags.unk6 = true;
    }

    if (g_ExclusiveMutex == NULL)
    {
        return ZUN_ERROR;
    }
    else
    {
        return ZUN_SUCCESS;
    }
}

#pragma var_order(touhouWinThread, lockoutTime, foregroundWinThread)
void GameWindow::ActivateWindow(HWND hWnd)
{
    DWORD foregroundWinThread;
    u32 lockoutTime;
    DWORD touhouWinThread;

    foregroundWinThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    touhouWinThread = GetWindowThreadProcessId(hWnd, NULL);
    AttachThreadInput(touhouWinThread, foregroundWinThread, true);
    SystemParametersInfoA(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockoutTime, 0);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)0, 0);
    SetActiveWindow(hWnd);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lockoutTime, 0);
    AttachThreadInput(touhouWinThread, foregroundWinThread, false);
}

#pragma var_order(moduleFilenameBuf, i, fileSize, dataCursor, checksum, dataBase)
i32 GameWindow::CalcExecutableChecksum()
{
    u32 checksum;
    u32 *dataBase;
    u32 *dataCursor;
    i32 fileSize;
    i32 i;
    char moduleFilenameBuf[MAX_PATH + 1];

    if (GetModuleFileNameA(NULL, moduleFilenameBuf, ARRAY_SIZE(moduleFilenameBuf)))
    {
        checksum = 0;
        dataCursor = (u32 *)FileSystem::OpenFile(moduleFilenameBuf, &fileSize, true);
        dataBase = dataCursor;

        if (dataCursor == NULL)
        {
            return -1;
        }

        for (i = 0; i < (fileSize / 4) - 1; i++, dataCursor++)
        {
            checksum += *dataCursor;
        }

        utils::DebugPrint("main sum %d\r\n", checksum);
        g_ZunMemory.Free(dataBase);
        g_Supervisor.exeChecksum = checksum;
        g_Supervisor.exeSize = fileSize;

        return checksum;
    }

    return -1;
}

// Modified version of ResolveIt function used as an example in Microsoft's documentation
// https://web.archive.org/web/20250210164627/https://learn.microsoft.com/en-us/windows/win32/shell/links#resolving-a-shortcut
#pragma var_order(hres, retValue, psl, ppf, wPath, wfd)
ZunBool GameWindow::ResolveIt(char *shortcutPath, char *dstPath, i32 maxPathLen)
{
    HRESULT hres;
    IPersistFile *ppf;
    IShellLink *psl;
    ZunBool retValue;
    WIN32_FIND_DATA wfd;
    LPWSTR wPath;

    if (dstPath == NULL)
    {
        return false;
    }

    retValue = false;

    CoInitialize(NULL);
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
    if (SUCCEEDED(hres))
    {
        hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
        if (SUCCEEDED(hres))
        {
            wPath = new WCHAR[maxPathLen];
            // Presumably something that set hres was deleted here
            if (SUCCEEDED(hres))
            {
                MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wPath, maxPathLen);
                hres = ppf->Load(wPath, STGM_READ);
                if (SUCCEEDED(hres))
                {
                    hres = psl->GetPath(dstPath, maxPathLen, &wfd, 0);
                    if (SUCCEEDED(hres))
                    {
                        retValue = true;
                    }
                }
            }

            delete wPath;
            ppf->Release();
        }

        psl->Release();
    }

    CoUninitialize();
    return retValue;
}

// FUNCTION: th08 0x406d10
ZunBool GameManager::GaugeIsExtremelyHuman()
{
    return this->globals->youkaiGauge <= this->youkaiGaugeHumanEffectsThreshold;
}

// FUNCTION: th08 0x406d40
ZunBool GameManager::GaugeIsModeratelyHuman()
{
    return this->globals->youkaiGauge <= this->youkaiGaugeHumanTintThreshold;
}

// FUNCTION: th08 0x406d70
ZunBool GameManager::GaugeIsExtremelyYoukai()
{
    return this->globals->youkaiGauge >= this->youkaiGaugeYoukaiEffectsThreshold;
}

// FUNCTION: th08 0x406da0
ZunBool GameManager::GaugeIsModeratelyYoukai()
{
    return this->globals->youkaiGauge >= this->youkaiGaugeYoukaiTintThreshold;
}

// FUNCTION: th08 0x406c70
ZunBool GameManager::IsSpellPractice()
{
    return this->flags.isSpellPractice;
}

// FUNCTION: th08 0x406c90
ZunBool GameManager::IsReplay()
{
    return this->flags.isReplay;
}

// FUNCTION: th08 0x406cb0
ZunBool GameManager::IsDemoMode()
{
    return this->flags.isDemoMode;
}

// FUNCTION: th08 0x406cf0
i32 GameManager::GetYoukaiGauge()
{
    return this->globals->youkaiGauge;
}

// FUNCTION: th08 0x406dd0
u8 GameManager::GetClockTime()
{
    return this->globals->clockTime;
}

// FUNCTION: th08 0x406df0
void GameManager::AddToClockTime(i8 value)
{
    this->globals->clockTime += value;
}

// FUNCTION: th08 0x453c60
void GameManager::SetClockTime(u8 value)
{
    this->globals->clockTime = value;
}


// FUNCTION: th08 0x41fdf0
void ZunTimer::operator+=(int value)
{
    this->Increment(value);
}

// FUNCTION: th08 0x4066a0
ZunBool ZunTimer::operator<(int value)
{
    return this->current < value;
}

// FUNCTION: th08 0x4066c0
ZunBool ZunTimer::operator>(int value)
{
    return this->current > value;
}
