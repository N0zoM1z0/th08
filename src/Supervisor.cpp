#include "th_pch.h"

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Config.hpp"
#include "Ending.hpp"
#include "GameManager.hpp"
#include "Global.hpp"
#include "MusicRoom.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "ScoreDat.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Supervisor.hpp"
#include "TextHelper.hpp"
#include "TitleScreen.hpp"
#include "i18n.hpp"
#include "utils.hpp"
#include <winbase.h>
#include <d3dx8.h>
#include <direct.h>
#include <stdio.h>
#include <time.h>

namespace th08
{
#ifndef TH08_MODERN_PORT
inline Float3::Float3(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}
#endif

DIFFABLE_STATIC(ScreenEffect *, g_SupervisorScreenEffect);
DIFFABLE_STATIC(Supervisor, g_Supervisor);
DIFFABLE_STATIC_ARRAY(AnmVm, 3, g_SupervisorLoadingVms);

DIFFABLE_STATIC(u32, g_SupervisorFpsQpcSampleCount);
DIFFABLE_STATIC(LARGE_INTEGER, g_SupervisorFpsLastQpc);
DIFFABLE_STATIC_ARRAY(char, 256, g_SupervisorFpsDebugBuffer);
DIFFABLE_STATIC_ARRAY(char, 256, g_SupervisorFpsBuffer);
DIFFABLE_STATIC(DWORD, g_SupervisorFpsLastTime);
DIFFABLE_STATIC(u32, g_SupervisorFpsTimeInitialized);
DIFFABLE_STATIC(u32, g_SupervisorFpsFrameCount);

// FUNCTION: th08 0x438a29
ZunBool Supervisor::IsMinimumGraphicsMode()
{
    return this->cfg.opts.displayMinimumGraphics;
}

// FUNCTION: th08 0x438a41
ZunBool Supervisor::IsDepthTestDisabled()
{
    return this->cfg.opts.disableDepthTest;
}

// FUNCTION: th08 0x438a59
ZunBool Supervisor::IsColorCompositingDisabled()
{
    return this->cfg.opts.disableColorCompositing;
}

// FUNCTION: th08 0x438a71
ZunBool Supervisor::IsHUDRedrawEnabled()
{
    return this->cfg.opts.redrawHUDEveryFrame;
}

ChainCallbackResult Supervisor::OnUpdate(Supervisor *s)
{
    if (s->flags.receivedCloseMsg && !s->IsSubthreadRunning())
    {
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    }

    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();

    g_AnmManager->ResetFrameDebugInfo();
    g_AnmManager->ClearCameraSettings();
    g_AnmManager->SetMixColorDefault();
    g_AnmManager->screenShakeOffset.x = g_AnmManager->screenShakeOffset.y = 0.0f;

    g_AnmManager->ExecuteScriptArray(g_SupervisorLoadingVms, ARRAY_SIZE(g_SupervisorLoadingVms));

    if (g_AnmManager->ServicePreloadedAnims() != ZUN_SUCCESS)
    {
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    }

    if (s->unk294 != 0)
    {
        if (s->unk294 == 2)
        {
            return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
        }
        if (s->unk290 == 0)
        {
            s->unk294 = 0;
        }
        else
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE;
        }
    }

    g_Supervisor.ClearFogState();
    g_SoundPlayer.UpdateFades();

    if (!g_GameManager.IsUnknown())
    {
        g_LastFrameInput = g_CurFrameInput;
        g_CurFrameInput = Controller::GetInput();

        g_IsEighthFrameOfHeldInput = 0;
        if (g_LastFrameInput == g_CurFrameInput)
        {
            if (g_NumOfFramesInputsWereHeld >= 0x1e)
            {
                if (g_NumOfFramesInputsWereHeld % 8 == 0)
                {
                    g_IsEighthFrameOfHeldInput = 1;
                }
                if (0x26 <= g_NumOfFramesInputsWereHeld)
                {
                    g_NumOfFramesInputsWereHeld = 0x1e;
                }
            }

            g_NumOfFramesInputsWereHeld++;
        }
        else
        {
            g_NumOfFramesInputsWereHeld = 0;
        }
    }
    else
    {
        g_CurFrameInput |= Controller::GetInput();
    }

    if (s->wantedState != s->curState)
    {
        s->wantedState2 = s->wantedState;
        utils::GuiDebugPrint("scene %d -> %d\r\n", s->wantedState, s->curState);
        switch (s->wantedState)
        {
        case SupervisorState_Init:
        init_titlescreen:
            s->curState = SupervisorState_TitleScreen;
            g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
            if (TitleScreen::RegisterChain(0) != ZUN_SUCCESS)
            {
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            }
            break;
        case SupervisorState_TitleScreen:
            switch (s->curState)
            {
            case SupervisorState_ExitGame:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SupervisorState_GameManager:
                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            /* Seems to be completely unused */
            case SupervisorState_ExitGame2:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR;
            case SupervisorState_ResultScreen:
                if (ResultScreen::RegisterChain(0) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SupervisorState_MusicRoom:
                if (MusicRoom::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SupervisorState_Ending:
                GameManager::CutChain();
                if (Ending::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            }
            break;
        case SupervisorState_ResultScreen:
            switch (s->curState)
            {
            case SupervisorState_ExitGame:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SupervisorState_TitleScreen:
                s->curState = SupervisorState_Init;
                goto init_titlescreen;
            }
            break;
        case SupervisorState_GameManager:
            switch (s->curState)
            {
            case SupervisorState_ExitGame:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SupervisorState_TitleScreen:
                GameManager::CutChain();
                s->curState = SupervisorState_Init;
                ReplayManager::SaveReplay(NULL, NULL);

                goto init_titlescreen;
            case SupervisorState_ResultScreenFromGame:
                GameManager::CutChain();
                if (ResultScreen::RegisterChain(1) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SupervisorState_GameManagerRestartFromBeginning:
                GameManager::CutChain();
                if (!g_GameManager.IsPracticeMode() && g_GameManager.difficulty < 4)
                {
                    g_GameManager.currentStage = 0;
                }
                ReplayManager::SaveReplay(NULL, NULL);
                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                s->curState = SupervisorState_GameManager;
                break;
            case SupervisorState_SpellcardPracticeRestart:
                g_Supervisor.curState = SupervisorState_GameManagerReInit;
                g_Supervisor.unk16c = 1;

                GameManager::CutChain();

                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                s->curState = SupervisorState_GameManager;
                break;
            /* Also seems to be completely unused */
            case SupervisorState_GameManagerNextStageWeird:
                g_Supervisor.curState = SupervisorState_GameManagerReInit;
                GameManager::CutChain();
                g_GameManager.AdvanceToNextStage();

                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                s->curState = SupervisorState_GameManager;
                break;
            case SupervisorState_GameManagerReInit:
                GameManager::CutChain();

                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }

                s->curState = SupervisorState_GameManager;
                break;
            case SupervisorState_FinishReplay:
                GameManager::CutChain();

                s->curState = SupervisorState_Init;
                ReplayManager::SaveReplay(NULL, NULL);
                s->curState = SupervisorState_TitleScreen;

                g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);

                if (TitleScreen::RegisterChain(1) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SupervisorState_Ending:
                GameManager::CutChain();
                if (Ending::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            }
            break;
        case SupervisorState_ResultScreenFromGame:
            switch (s->curState)
            {
            case SupervisorState_ExitGame:
                ReplayManager::SaveReplay(NULL, NULL);
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SupervisorState_TitleScreen:
                s->curState = SupervisorState_Init;

                ReplayManager::SaveReplay(NULL, NULL);

                goto init_titlescreen;
            }
            break;
        case SupervisorState_MusicRoom:
            switch (s->curState)
            {
            case SupervisorState_ExitGame:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SupervisorState_TitleScreen:
                s->curState = SupervisorState_Init;

                goto init_titlescreen;
            }
            break;
        case SupervisorState_Ending:
            switch (s->curState)
            {
            case SupervisorState_ExitGame:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SupervisorState_TitleScreen:
                s->curState = SupervisorState_Init;

                goto init_titlescreen;
            case SupervisorState_ResultScreenFromGame:
                if (ResultScreen::RegisterChain(1) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            }
            break;
        }
        g_CurFrameInput = g_LastFrameInput = g_IsEighthFrameOfHeldInput = 0;
    }

    s->wantedState = s->curState;
    s->calcCount++;

    if ((s->calcCount % 4000) == 3999 &&
        g_Supervisor.CheckVersion(CONFIG_VERSION_STRING, g_Supervisor.exeSize, g_Supervisor.exeChecksum) != ZUN_SUCCESS)
    {
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    }

    if (g_ScreenEffectCounter != 0)
    {
        g_ScreenEffectCounter--;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

Supervisor::Supervisor()
{
    memset(this, 0, sizeof(Supervisor));

    this->flags.unk6 = true;
    this->flags.unk8 = true;
}

// FUNCTION: th08 0x445bc0
ChainCallbackResult Supervisor::DrawFpsCounter(Supervisor *s)
{
    Supervisor::CalculateFps(TRUE);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x445bd4
#pragma var_order(position, color1, color2, surface, s)
ChainCallbackResult Supervisor::OnDraw2(Supervisor *s)
{
    Float3 position;
    i32 color1;
    i32 color2;
    IDirect3DSurface8 *surface;

    if (s->loadingVmsHaveBeenSetup >= 2)
    {
        s->loadingVmsHaveBeenSetup++;
        if (s->loadingVmsHaveBeenSetup >= 5)
        {
            position.x = 288.0f;
            position.y = 454.0f;
            position.z = 0.0f;
            g_AsciiManager.scaleX = 0.5f;
            g_AsciiManager.scaleY = 0.5f;
            if (s->loadingVmsHaveBeenSetup < 35)
            {
                color1 = 255 - (((s->loadingVmsHaveBeenSetup - 5) << 7) / 30);
                g_AsciiManager.color.a = color1;
            }
            else
            {
                color2 = 255 - (((65 - s->loadingVmsHaveBeenSetup) << 7) / 30);
                g_AsciiManager.color.a = color2;
            }
            g_AsciiManager.AddFormatText(&position, "Press Shot Button");
            g_AsciiManager.scaleX = 1.0f;
            g_AsciiManager.scaleY = 1.0f;
            g_AsciiManager.OnDrawLowPrioImpl();
            g_AsciiManager.numStrings = 0;

            if (s->loadingVmsHaveBeenSetup >= 65)
            {
                s->loadingVmsHaveBeenSetup = 5;
            }
        }
    }

    if (s->loadingVmsHaveBeenSetup != 0)
    {
        g_AnmManager->CopySurfaceToBackbuffer(8, 0, 0, 0, 0);
    }
    else
    {
#ifdef TH08_MODERN_PORT
        surface = g_AnmManager->surfaces[8];
#else
        __asm
        {
            push 8
            pop eax
            shl eax, 2
            mov ecx, dword ptr [g_AnmManager]
            mov eax, dword ptr [ecx + eax + 0x2038]
            mov surface, eax
        }
#endif
        if (surface != NULL)
        {
            g_AnmManager->ReleaseSurface(8);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Supervisor::DrawLoadingVms(Supervisor *s)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(g_SupervisorLoadingVms); i++)
    {
        g_SupervisorLoadingVms[i].pos += g_SupervisorLoadingVms[i].pos2;

        g_AnmManager->Draw2D(&g_SupervisorLoadingVms[i]);

        g_SupervisorLoadingVms[i].pos -= g_SupervisorLoadingVms[i].pos2;
    }

    if (s->unk294 != 0)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x445e3d
BOOL CALLBACK Supervisor::ControllerCallback(LPCDIDEVICEOBJECTINSTANCEA lpddoi, LPVOID pvRef)
{
    DIPROPRANGE range;
    LPVOID context;

    context = pvRef;

    if ((lpddoi->dwType & 3) != 0)
    {
        range.diph.dwSize = sizeof(DIPROPRANGE);
        range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        range.diph.dwHow = DIPH_BYID;
        range.diph.dwObj = lpddoi->dwType;
        range.lMin = -1000;
        range.lMax = 1000;

        if (g_Supervisor.controller->SetProperty(DIPROP_RANGE, &range.diph) < 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

#pragma var_order(elem, result, supervisor)
ZunResult Supervisor::RegisterChain()
{
    Supervisor *supervisor = &g_Supervisor;

    supervisor->wantedState = SupervisorState_Init;
    supervisor->curState = SupervisorState_ExitGame;
    supervisor->calcCount = 0;

    ChainElem *elem = g_Chain.CreateElem((ChainCallback)Supervisor::OnUpdate);

    elem->arg = supervisor;
    elem->addedCallback = (ChainLifetimeCallback)Supervisor::AddedCallback;
    elem->deletedCallback = (ChainLifetimeCallback)Supervisor::DeletedCallback;

    ZunResult result = (ZunResult)g_Chain.AddToCalcChain(elem, 0);

    if (result != ZUN_SUCCESS)
    {
        return result;
    }

    elem = g_Chain.CreateElem((ChainCallback)Supervisor::DrawFpsCounter);
    elem->arg = supervisor;
    g_Chain.AddToDrawChain(elem, 16);

    elem = g_Chain.CreateElem((ChainCallback)Supervisor::OnDraw2);
    elem->arg = supervisor;
    g_Chain.AddToDrawChain(elem, 0);

    elem = g_Chain.CreateElem((ChainCallback)Supervisor::DrawLoadingVms);
    elem->arg = supervisor;
    g_Chain.AddToDrawChain(elem, 2);

    return ZUN_SUCCESS;
}

#pragma var_order(position, score, s)
int Supervisor::AddedCallback(Supervisor *s)
{
    g_Supervisor.framerateMultiplier = 1.0f;

    ScoreDat *score = ScoreDat::OpenScore("score.dat");

    memset(&g_GameManager.plst, 0, sizeof(g_GameManager.plst));
    g_GameManager.plst.base.unkLen = g_GameManager.plst.base.th8kLen = sizeof(Plst);
    g_GameManager.plst.base.magic = PLST_MAGIC;
    g_GameManager.plst.base.version = PLST_VERSION;

    ScoreDat::ParsePLST(score, &g_GameManager.plst);
    ScoreDat::ParseCLRD(score, g_GameManager.clrdData);
    ScoreDat::ParsePSCR(score, g_GameManager.pscrData);
    ScoreDat::ParseCATK(score, g_GameManager.catkData);
    ScoreDat::ParseFLSP(score, &g_GameManager.flsp);

    ScoreDat::ReleaseScore(score);

    g_GameManager.flags.isExtraUnlocked = g_GameManager.IsExtraUnlocked();
    g_GameManager.flags.isSpellPracticeUnlocked = g_GameManager.IsSpellPracticeUnlocked();
    g_GameManager.flags.isExtraUnlockedWithAllTeams = g_GameManager.IsExtraUnlockedWithAllTeams();

    if (Supervisor::LoadDat() != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_AnmManager->LoadSurface(8, "title/th08logo.jpg");
    s->loadingAnm = g_AnmManager->LoadAnm(2, "nowloading.anm");
    if (s->loadingAnm == NULL)
    {
        g_AnmManager->ReleaseSurface(0);
        return ZUN_ERROR;
    }

    g_Supervisor.unk178 = 1;

    if (!g_Supervisor.disableVsync && Supervisor::CheckFps() != ZUN_SUCCESS)
    {
        g_AnmManager->ReleaseSurface(0);
        return -2;
    }

    g_AnmManager->SetupVertexBuffer();
    TextHelper::CreateTextBuffer();

    Float3 position(500.0, 440.0f, 0.0f);

    g_Supervisor.SetupLoadingVms(&position);

    g_Supervisor.unk294 = 1;
    g_Supervisor.ThreadStart((LPTHREAD_START_ROUTINE)Supervisor::StartupThread, s);

    return ZUN_SUCCESS;
}

ZunResult Supervisor::LoadDat()
{
    if (g_PbgArchive.Load("th08.dat"))
    {
#pragma var_order(fileSize, versionFileName)
        i32 fileSize;
        char versionFileName[128];

        sprintf(versionFileName, "th08_%.4x%c.ver", 0x100, 'd');

        g_Supervisor.versionData = (char *)FileSystem::OpenFile(versionFileName, &fileSize, 0);
        g_Supervisor.versionDataSize = fileSize;
        if (g_Supervisor.versionData == NULL)
        {
            g_GameErrorContext.Fatal(TH_ERR_DAT_WRONG_VERSION);
            return ZUN_ERROR;
        }
    }
    else
    {
        g_GameErrorContext.Fatal(TH_ERR_DAT_NOT_FOUND);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x446232
#pragma var_order(frameIndex, framesInWindow, lastTime, samples, sampleCount, currentTime, deltaTime, fps, elapsedSeconds, averageIndex, average)
i32 Supervisor::CheckFps()
{
    i32 frameIndex;
    i32 framesInWindow;
    DWORD lastTime;
    f32 samples[29];
    i32 sampleCount;
    DWORD currentTime;
    i32 deltaTime;
    f32 fps;
    f32 elapsedSeconds;
    i32 averageIndex;
    f32 average;

    frameIndex = 0;
    framesInWindow = 0;
    sampleCount = 0;
    lastTime = 0;

    timeBeginPeriod(1);
    lastTime = timeGetTime();
    timeEndPeriod(1);

    while (frameIndex < 1800 && sampleCount < 8)
    {
        g_Supervisor.d3dDevice->BeginScene();
        g_AnmManager->CopySurfaceToBackbuffer(8, 0, 0, 0, 0);
        g_Supervisor.d3dDevice->EndScene();
        if (g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
            g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);

        frameIndex++;
        timeBeginPeriod(1);
        currentTime = timeGetTime();
        timeEndPeriod(1);
        framesInWindow++;
        deltaTime = currentTime - lastTime;

        if (deltaTime >= 700)
        {
            lastTime = currentTime;
            framesInWindow = 0;
        }
        else if (deltaTime >= 500)
        {
            elapsedSeconds = (f32)deltaTime / 1000.0f;
            fps = (f32)framesInWindow * 1000.0f / deltaTime;
            if (fps >= 57.0f)
            {
                samples[sampleCount] = fps;
                sampleCount++;
            }
            lastTime = currentTime;
            framesInWindow = 0;
        }
    }

    if (!g_Supervisor.cfg.opts.disableVsync)
    {
        average = 0.0f;
        if (sampleCount >= 2)
        {
            for (averageIndex = 0; averageIndex < sampleCount; averageIndex++)
                average = average + samples[averageIndex];
            average = average / averageIndex;
        }
        else
        {
            average = 1000.0f;
        }

        if (average > 160.0f)
        {
            g_GameErrorContext.Log("\x90\x82\x92\xBC\x93\xAF\x8A\xFA\x82\xAA\x8E\xE6\x82\xEA\x82\xC4\x82\xC8\x82\xA2\x82\xA9\x81\x41\x83\x8A\x83\x74\x83\x8C\x83\x62\x83\x56\x83\x85\x83\x8C\x81\x5B\x83\x67\x82\xAA\x8D\x82\x82\xB7\x82\xAC\x82\xDC\x82\xB7\x81\x42\r\n");
            g_GameErrorContext.Log("\x8B\xAD\x90\xA7\x82\x55\x82\x4F\x83\x74\x83\x8C\x81\x5B\x83\x80\x83\x82\x81\x5B\x83\x68\x82\xC5\x93\xAE\x8D\xEC\x82\xB5\x82\xDC\x82\xB7\r\n");
            g_Supervisor.disableVsync = true;
            return -2;
        }

        if (average >= 65.0f)
        {
            g_GameErrorContext.Log("\x90\x82\x92\xBC\x93\xAF\x8A\xFA\x82\xAA\x8E\xE6\x82\xEA\x82\xC4\x82\xC8\x82\xA2\x82\xA9\x81\x41\x83\x8A\x83\x74\x83\x8C\x83\x62\x83\x56\x83\x85\x83\x8C\x81\x5B\x83\x67\x82\xAA\x8D\x82\x82\xB7\x82\xAC\x82\xDC\x82\xB7\x81\x42\r\n");
            g_GameErrorContext.Log("\x8B\xAD\x90\xA7\x82\x55\x82\x4F\x83\x74\x83\x8C\x81\x5B\x83\x80\x83\x82\x81\x5B\x83\x68\x82\xC5\x93\xAE\x8D\xEC\x82\xB5\x82\xDC\x82\xB7\r\n");
            g_Supervisor.disableVsync = true;
            return -2;
        }
    }

    return ZUN_SUCCESS;

}

#pragma var_order(bgmVolume, scoreFileSize, scoreFile, findFile, i, fileNameBuffer, scoreBackupFileName, findData,     \
                  currentLocalTime, currentTime)
void Supervisor::StartupThread(Supervisor *s)
{
    float bgmVolume;
    u8 *scoreFile;
    i32 scoreFileSize;
    HANDLE findFile;
    int i;
    char fileNameBuffer[256]; /* yes I know the buffer might be too small, but it would not match otherwise. */
    const char *scoreBackupFileName;
    WIN32_FIND_DATAA findData;
    time_t currentTime;
    tm *currentLocalTime;

    g_Supervisor.unk178 = 0;
    g_Supervisor.unk174 = 0;
    g_Supervisor.totalPlayTime = timeGetTime();

    *reinterpret_cast<u16 *>(&g_Rng) = static_cast<u16>(g_Supervisor.totalPlayTime);

    g_Supervisor.SetupDInput();

    if (g_Supervisor.midiOutput == NULL)
    {
        g_Supervisor.midiOutput = new MidiOutput();
    }
    if (g_Supervisor.midiOutput != NULL)
    {
        g_Supervisor.midiOutput->ReadFileData(30, "bgm/init.mid");
    }
    g_SoundPlayer.InitSoundBuffers();
    g_Supervisor.textAnm = g_AnmManager->PreloadAnm(0, "text.anm");
    if (g_Supervisor.textAnm == NULL)
    {
        goto err;
    }

    if (AsciiManager::RegisterChain() != ZUN_SUCCESS)
    {
        if (g_Supervisor.subthreadCloseRequestActive)
        {
            return;
        }
        g_GameErrorContext.Log(TH_ERR_FAILED_TO_INIT_ASCII);
        goto err;
    }

    if (g_SoundPlayer.LoadFmt("bgm/thbgm.fmt") != ZUN_SUCCESS)
    {
        if (g_Supervisor.subthreadCloseRequestActive)
        {
            return;
        }
        g_GameErrorContext.Log(TH_ERR_FAILED_TO_INIT_BGM);
        goto err;
    }

    g_SoundPlayer.bgmVolume = g_Supervisor.cfg.musicVolume;
    g_SoundPlayer.sfxVolume = g_Supervisor.cfg.sfxVolume;

    bgmVolume = g_SoundPlayer.bgmVolume / 100.0f;

    if (g_SoundPlayer.sfxVolume != 0)
    {
        /* Strangely, the parentheses affect code generation, even
         * though they don't seem do anything?
         */

        bgmVolume = (1.0f - bgmVolume);
        bgmVolume *= bgmVolume;
        bgmVolume *= bgmVolume;
        bgmVolume = (1.0f - bgmVolume);

        g_SoundPlayer.unkVolume = ((int)(SOUNDPLAYER_VOLUME_RANGE * bgmVolume)) - SOUNDPLAYER_VOLUME_RANGE;
    }
    else
    {
        g_SoundPlayer.unkVolume = SOUNDPLAYER_SILENT_VOLUME;
    }

    if (g_SoundPlayer.unusedBgmSeekOffset == 0)
    {
        if (!g_Supervisor.IsMusicPreloadEnabled())
        {
            g_SoundPlayer.StartBGM("thbgm.dat");
        }
        else
        {
            strcpy(g_SoundPlayer.currentBgmFileName, "thbgm.dat");
        }
    }
    else if (!g_Supervisor.IsMusicPreloadEnabled())
    {
        g_SoundPlayer.StartBGM("th08.dat");
    }
    else
    {
        strcpy(g_SoundPlayer.currentBgmFileName, "th08.dat");
    }

    if (g_Supervisor.flags.unk8 && ((scoreFile = FileSystem::OpenFile("score.dat", &scoreFileSize, TRUE)) != NULL))
    {
        scoreBackupFileName = "score_4.??????.bak";

        i = 0;

        _mkdir("./backup");
        _chdir("./backup");

        findFile = FindFirstFileA("score_5.??????.bak", &findData);

        while (findFile != INVALID_HANDLE_VALUE)
        {
            DeleteFileA(findData.cFileName);

            if (!FindNextFileA(findFile, &findData))
            {
                break;
            }
        }

        FindClose(findFile);

        for (i = 4; i > 0; i--)
        {
            strcpy(fileNameBuffer, scoreBackupFileName);

            fileNameBuffer[6] = i + '0';

            findFile = FindFirstFileA(fileNameBuffer, &findData);

            while (findFile != INVALID_HANDLE_VALUE)
            {
                strcpy(fileNameBuffer, findData.cFileName);

                fileNameBuffer[6] = i + '1';

                rename(findData.cFileName, fileNameBuffer);

                if (!FindNextFileA(findFile, &findData))
                {
                    break;
                }
            }

            FindClose(findFile);
        }

        time(&currentTime);
        currentLocalTime = localtime(&currentTime);
        strftime(fileNameBuffer, 128, "score_1.%y%m%d.bak", currentLocalTime);

        FileSystem::WriteDataToFile(fileNameBuffer, scoreFile, scoreFileSize);
        free(scoreFile);
        _chdir("../");
    }

    if (g_Supervisor.flags.unk6)
    {
        g_Supervisor.dummyMidiTimer = new DummyMidiTimer();
        if (g_Supervisor.dummyMidiTimer != NULL)
        {
            g_Supervisor.dummyMidiTimer->StartTimer();
        }
    }

    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.unk290 = 0;
    g_Supervisor.unk294 = 0;
    g_Supervisor.flags.unk8 = false;

    return;

err:
    g_Supervisor.runningSubthreadHandle = NULL;
    g_Supervisor.subthreadCloseRequestActive = FALSE;
    g_Supervisor.unk290 = 0;
    g_Supervisor.unk294 = 2;
    g_Supervisor.flags.receivedCloseMsg = true;
}

// FUNCTION: th08 0x446a37
ZunResult Supervisor::SetupDInput()
{
    HINSTANCE instance = (HINSTANCE)GetWindowLongA(this->hwndGameWindow, GWL_HINSTANCE);

    if (this->cfg.opts.dontUseDirectInput != 0)
    {
        return ZUN_ERROR;
    }

    if (DirectInput8Create(instance, DIRECTINPUT_VERSION, IID_IDirectInput8A, (void **)&this->dInputIface, NULL) < 0)
    {
        this->dInputIface = NULL;
        g_GameErrorContext.Log("DirectInput create error\r\n");
        return ZUN_ERROR;
    }

    if (this->dInputIface->CreateDevice(GUID_SysKeyboard, &this->keyboard, NULL) < 0)
    {
        if (this->dInputIface != NULL)
        {
            this->dInputIface->Release();
            this->dInputIface = NULL;
        }
        g_GameErrorContext.Log("DirectInput create error\r\n");
        return ZUN_ERROR;
    }

    if (this->keyboard->SetDataFormat(&c_dfDIKeyboard) < 0)
    {
        if (this->keyboard != NULL)
        {
            this->keyboard->Release();
            this->keyboard = NULL;
        }
        if (this->dInputIface != NULL)
        {
            this->dInputIface->Release();
            this->dInputIface = NULL;
        }
        g_GameErrorContext.Log("keyboard data format error\r\n");
        return ZUN_ERROR;
    }

    if (this->keyboard->SetCooperativeLevel(this->hwndGameWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY) < 0)
    {
        if (this->keyboard != NULL)
        {
            this->keyboard->Release();
            this->keyboard = NULL;
        }
        if (this->dInputIface != NULL)
        {
            this->dInputIface->Release();
            this->dInputIface = NULL;
        }
        g_GameErrorContext.Log("keyboard cooperative level error\r\n");
        return ZUN_ERROR;
    }

    this->keyboard->Acquire();
    g_GameErrorContext.Log("KeyBoard OK\r\n");
    this->dInputIface->EnumDevices(4, Supervisor::EnumGameControllersCb, NULL, DIEDFL_ATTACHEDONLY);

    if (this->controller != NULL)
    {
        this->controller->SetDataFormat(&c_dfDIJoystick);
        this->controller->SetCooperativeLevel(this->hwndGameWindow, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
        g_Supervisor.controllerCaps.dwSize = sizeof(DIDEVCAPS);
        this->controller->GetCapabilities(&g_Supervisor.controllerCaps);
        this->controller->EnumObjects(Supervisor::ControllerCallback, NULL, NULL);
        g_GameErrorContext.Log("JoyStick OK\r\n");
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x446cc7
BOOL CALLBACK Supervisor::EnumGameControllersCb(LPCDIDEVICEINSTANCE pdidInstance, LPVOID pContext)
{
    HRESULT result;

    if (g_Supervisor.controller == NULL)
    {
        result = g_Supervisor.dInputIface->CreateDevice(pdidInstance->guidInstance, &g_Supervisor.controller, NULL);
        if (result < 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

ZunResult Supervisor::DeletedCallback(Supervisor *s)
{
    g_Supervisor.ThreadClose();
    if (g_Supervisor.versionData != NULL)
    {
        ZUN_FREE(g_Supervisor.versionData);
    }

    g_AnmManager->ReleaseVertexBuffer();
    g_AnmManager->ReleaseAnm(0);
    g_AnmManager->ReleaseAnm(2);
    g_AnmManager->ReleaseSurface(8);

    AsciiManager::CutChain();

    g_SoundPlayer.QueueCommand(4, 0, "dummy");
    if (g_Supervisor.cfg.musicMode == MIDI && g_Supervisor.midiOutput != NULL)
    {
        g_Supervisor.midiOutput->PlayFile(30);
    }

    ReplayManager::SaveReplay(NULL, NULL);
    TextHelper::ReleaseTextBuffer();

    if (s->keyboard != NULL)
    {
        s->keyboard->Unacquire();
    }

    SAFE_RELEASE(s->keyboard);

    if (s->controller != NULL)
    {
        s->controller->Unacquire();
    }

    SAFE_RELEASE(s->controller);
    SAFE_RELEASE(s->dInputIface);

    if (g_GameManager.globals != NULL)
    {
        ZUN_DELETE2(g_GameManager.globals);
    }

    if (g_GameManager.cfg != NULL)
    {
        ZUN_DELETE2(g_GameManager.cfg);
    }

    g_PbgArchive.Release();
    if (g_Supervisor.dummyMidiTimer != NULL)
    {
        g_Supervisor.dummyMidiTimer->StopTimer();
        ZUN_DELETE2(g_Supervisor.dummyMidiTimer);
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x446f53
#pragma var_order(fps, elapsed, currentTime, framerate, currentQpc, fpsCounterPos, debugCounterPos, shouldDraw)
void Supervisor::CalculateFps(ZunBool shouldDraw)
{
    f32 fps;
    f32 elapsed;
    DWORD currentTime;
    f32 framerate;
    LARGE_INTEGER currentQpc;
    Float3 fpsCounterPos;
    Float3 debugCounterPos;

    if ((i8)g_GameManager.unk2D == 0)
    {
        g_SupervisorFpsFrameCount += (u8)g_Supervisor.cfg.frameskipConfig + 1;

        if (g_Supervisor.fpsPerformanceFrequency == 0)
        {
            if ((g_SupervisorFpsTimeInitialized & 1) == 0)
            {
                g_SupervisorFpsTimeInitialized |= 1;
                g_SupervisorFpsLastTime = timeGetTime();
            }

            currentTime = timeGetTime();
            if (currentTime < g_SupervisorFpsLastTime)
            {
                g_SupervisorFpsLastTime = currentTime;
                g_SupervisorFpsFrameCount = 0;
            }

            if (currentTime - g_SupervisorFpsLastTime >= 500)
            {
                elapsed = (f32)(currentTime - g_SupervisorFpsLastTime) / 1000.0f;
                g_SupervisorFpsLastTime = currentTime;
calculateFps:
                fps = (f32)g_SupervisorFpsFrameCount / elapsed;
                g_SupervisorFpsFrameCount = 0;
                sprintf(g_SupervisorFpsBuffer, "%.02ffps", fps);

                if (g_GameManager.flags.unk2 && shouldDraw)
                {
                    framerate = 60.0f;
                    g_Supervisor.lagDenominator += framerate;

                    if (framerate * 0.89999998f < fps)
                        g_Supervisor.lagNumerator += framerate;
                    else if (framerate * 0.69999999f < fps)
                        g_Supervisor.lagNumerator += framerate * 0.80000001f;
                    else if (framerate * 0.5f < fps)
                        g_Supervisor.lagNumerator += framerate * 0.60000002f;
                    else
                        g_Supervisor.lagNumerator += framerate * 0.5f;

                    if (!g_GameManager.flags.isReplay)
                        *reinterpret_cast<i16 *>(&g_Supervisor.unk198) = (i16)(fps + 0.5f);
                    else
                        sprintf(g_SupervisorFpsDebugBuffer, "%2d", *reinterpret_cast<i16 *>(&g_Supervisor.unk198));
                }
            }
        }
        else
        {
            if (g_SupervisorFpsLastQpc.LowPart == 0)
                QueryPerformanceCounter(&g_SupervisorFpsLastQpc);

            QueryPerformanceCounter(&currentQpc);
            if (currentQpc.LowPart < g_SupervisorFpsLastQpc.LowPart)
            {
                g_SupervisorFpsLastQpc = currentQpc;
                g_SupervisorFpsFrameCount = 0;
            }

            if (currentQpc.LowPart >=
                g_SupervisorFpsLastQpc.LowPart +
                    (g_Supervisor.fpsPerformanceFrequency >> 1))
            {
                elapsed = (f32)(currentQpc.LowPart - g_SupervisorFpsLastQpc.LowPart) /
                          (f32)g_Supervisor.fpsPerformanceFrequency;
                g_SupervisorFpsLastQpc = currentQpc;
                g_SupervisorFpsQpcSampleCount++;
                goto calculateFps;
            }
        }
    }

    if (g_Supervisor.unk178 == 0 && shouldDraw)
    {
        fpsCounterPos.x = 512.0f;
        fpsCounterPos.y = 464.0f;
        fpsCounterPos.z = 0.0f;
        g_AsciiManager.AddString(&fpsCounterPos, g_SupervisorFpsBuffer);

        if (g_GameManager.flags.isReplay && g_GameManager.flags.unk2)
        {
            debugCounterPos.x = 384.0f;
            debugCounterPos.y = 448.0f;
            debugCounterPos.z = 0.0f;

            if (g_Supervisor.unk0x33c != 0)
                g_AsciiManager.color.d3dColor = 0xffff4040;
            else
                g_AsciiManager.color.d3dColor = 0xffffffd0;

            g_AsciiManager.AddString(&debugCounterPos, g_SupervisorFpsDebugBuffer);
            g_AsciiManager.color.d3dColor = 0xffffffff;
        }
    }
}

void ZunTimer::Increment(int value)
{
    if (g_Supervisor.flags.unk5 != 0)
    {
        this->current++;
        this->subFrame = 0.0f;
        this->previous = -999.0f;
    }

    if (g_Supervisor.framerateMultiplier > 0.99f)
    {
        this->current += value;
        return;
    }

    if (value < 0)
    {
        this->Decrement(-value);
        return;
    }

    this->previous = this->current;
    this->subFrame += value * g_Supervisor.framerateMultiplier;

    while (this->subFrame >= 1.0f)
    {
        this->current++;
        this->subFrame -= 1.0f;
    }
}

void ZunTimer::Decrement(int value)
{
    if (g_Supervisor.flags.unk5 != 0)
    {
        this->current--;
        this->subFrame = 0.0f;
        this->previous = -999.0f;
    }

    if (g_Supervisor.framerateMultiplier > 0.99f)
    {
        this->current -= value;
        return;
    }

    if (value < 0)
    {
        this->Increment(-value);
        return;
    }

    this->previous = this->current;
    this->subFrame -= value * g_Supervisor.framerateMultiplier;

    while (this->subFrame < 0.0f)
    {
        this->current--;
        this->subFrame += 1.0f;
    }
}

void Supervisor::TickTimer(int *frames, float *subframes)
{
    if (this->framerateMultiplier <= 0.99f)
    {
        *subframes += this->framerateMultiplier;
        if (*subframes >= 1.0f)
        {
            *frames = *frames + 1;
            *subframes -= 1.0f;
        }
    }
    else
    {
        *frames = *frames + 1;
    }
}

#pragma pack(push, 1)
struct SnapshotBitmapFileHeader
{
    u16 type;
    u32 size;
    u16 reserved1;
    u16 reserved2;
    u32 offBits;
};
#pragma pack(pop)

// FUNCTION: th08 0x44748f
#pragma var_order(fileHeader, pixels, infoHeader, backbuffer, widthBytes, src, dst, y, x, dstRow, lockedRect, bytesWritten, file, allocSize, this)
ZunBool Supervisor::TakeSnapshot(const char *filePath)
{
    SnapshotBitmapFileHeader fileHeader;
    BITMAPINFOHEADER *infoHeader = NULL;
    u8 *pixels = NULL;
    IDirect3DSurface8 *backbuffer = NULL;
    D3DLOCKED_RECT lockedRect;
    DWORD bytesWritten;
    HANDLE file;
    i32 widthBytes;
    i32 y;
    i32 x;
    i32 dstRow;
    u8 *dst;
    u8 *src;
    i32 allocSize;

    utils::GuiDebugPrint("SnapShot! %s\n", filePath);
    this->d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

    memset(&fileHeader, 0, sizeof(fileHeader));
    fileHeader.type = *reinterpret_cast<const u16 *>("BM");
    fileHeader.offBits = 0x36;
    fileHeader.size = fileHeader.offBits;

    switch (this->presentParameters.BackBufferFormat)
    {
    case D3DFMT_R5G6B5:
        utils::GuiDebugPrint("16bit は取り込めない\r\n");
        g_GameErrorContext.Log("16bit は取り込めない\r\n");
        goto cleanup;

    case D3DFMT_X8R8G8B8:
        allocSize = 0x2C;
        infoHeader = (BITMAPINFOHEADER *)malloc(allocSize);
        if (infoHeader == NULL)
        {
            g_GameErrorContext.Log("snapShotScreen : 確保しくり\r\n");
            goto cleanup;
        }
        memset(infoHeader, 0, 0x2C);
        widthBytes = 0x780;
        pixels = (u8 *)malloc(widthBytes * 0x1E0);
        if (pixels == NULL)
        {
            g_GameErrorContext.Log("snapShotScreen : 確保しくり\r\n");
            goto cleanup;
        }
        fileHeader.size += widthBytes * 0x1E0;
        infoHeader->biBitCount = 0x18;
        infoHeader->biSize = 0x28;
        infoHeader->biWidth = 0x280;
        infoHeader->biHeight = 0x1E0;
        infoHeader->biPlanes = 1;
        infoHeader->biCompression = 0;
        backbuffer->LockRect(&lockedRect, NULL, 0);
        dstRow = 0;
        for (y = 0x1DF; y > -1; y--, dstRow++)
        {
            dst = pixels + widthBytes * dstRow;
            src = (u8 *)lockedRect.pBits + lockedRect.Pitch * y;
            for (x = 0; x < 0x280; x++)
            {
                *dst = *src++;
                dst++;
                *dst = *src++;
                dst++;
                *dst = *src;
                src += 2;
                dst++;
            }
        }
        backbuffer->UnlockRect();
        file = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
        {
            goto cleanup;
        }
        WriteFile(file, &fileHeader, sizeof(fileHeader), &bytesWritten, NULL);
        WriteFile(file, infoHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
        WriteFile(file, pixels, widthBytes * 0x1E0, &bytesWritten, NULL);
        CloseHandle(file);
        goto cleanup;

    default:
        g_GameErrorContext.Log("error ? mother.cpp\r\n");
        return TRUE;
    }

cleanup:
    if (backbuffer != NULL)
    {
        backbuffer->Release();
        backbuffer = NULL;
    }
    free(infoHeader);
    free(pixels);
    return FALSE;
}

// FUNCTION: th08 0x447764
void SoundPlayer::UpdateFades()
{
    if (this->bgm != NULL)
    {
        this->bgm->UpdateFadeOut();
        this->bgm->UpdateFadeIn();
        this->bgm->UpdatePartialFadeOut();
        this->bgm->UpdatePartialFadeIn();
    }
}

#pragma var_order(fileSize, configFileBuffer, bgmHandle, bytesRead, bgmBuffer, bgmHandle2, bytesRead2, bgmBuffer2)
ZunResult Supervisor::LoadConfig(char *configFile)
{
    i32 bgmBuffer[4];
    i32 bgmBuffer2[4];

    HANDLE bgmHandle;
    HANDLE bgmHandle2;

    DWORD bytesRead;
    DWORD bytesRead2;

    u8 *configFileBuffer;
    i32 fileSize;

    memset(&g_Supervisor.cfg, 0, sizeof(GameConfiguration));
    configFileBuffer = FileSystem::OpenFile(configFile, &fileSize, true);
    if (configFileBuffer == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_CONFIG_NOT_FOUND);
    SET_DEFAULT:
        g_Supervisor.cfg.lifeCount = 2;
        g_Supervisor.cfg.bombCount = 3;
        g_Supervisor.cfg.colorMode16bit = 0;
        g_Supervisor.cfg.version = GAME_VERSION;
        g_Supervisor.cfg.padXAxis = 600;
        g_Supervisor.cfg.padYAxis = 600;
        bgmHandle = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (bgmHandle != INVALID_HANDLE_VALUE)
        {
            ReadFile(bgmHandle, bgmBuffer, 16, &bytesRead, NULL);
            CloseHandle(bgmHandle);
            if (bgmBuffer[0] != ZWAV_MAGIC || bgmBuffer[1] != 1 || bgmBuffer[2] != 0x800)
            {
                g_GameErrorContext.Fatal(TH_ERR_BGM_VERSION_MISMATCH);
                return ZUN_ERROR;
            }
            g_Supervisor.cfg.musicMode = WAV;
        }
        else
        {
            g_Supervisor.cfg.musicMode = MIDI;
            utils::GuiDebugPrint(TH_ERR_NO_WAVE_FILE);
        }
        g_Supervisor.cfg.playSounds = 1;
        g_Supervisor.cfg.defaultDifficulty = NORMAL;
        g_Supervisor.cfg.windowed = false;
        g_Supervisor.cfg.frameskipConfig = false;
        g_Supervisor.cfg.controllerMapping = g_ControllerMapping;
        g_Supervisor.cfg.effectQuality = MAXIMUM;
        g_Supervisor.cfg.slowMode = 0;
        g_Supervisor.cfg.shotSlow = 0;
        g_Supervisor.cfg.musicVolume = 100;
        g_Supervisor.cfg.sfxVolume = 80;
    }
    else
    {
        g_Supervisor.cfg = *(GameConfiguration *)configFileBuffer;
        free(configFileBuffer);
        bgmHandle2 = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (bgmHandle2 != INVALID_HANDLE_VALUE)
        {
            ReadFile(bgmHandle2, bgmBuffer2, 16, &bytesRead2, NULL);
            CloseHandle(bgmHandle2);
            if (bgmBuffer2[0] != ZWAV_MAGIC || bgmBuffer2[1] != 1 || bgmBuffer2[2] != 0x800)
            {
                g_GameErrorContext.Fatal(TH_ERR_BGM_VERSION_MISMATCH);
                return ZUN_ERROR;
            }
        }
        if (g_Supervisor.cfg.lifeCount >= 7 || g_Supervisor.cfg.bombCount >= 4 ||
            g_Supervisor.cfg.colorMode16bit >= 2 || g_Supervisor.cfg.musicMode >= 3 ||
            g_Supervisor.cfg.defaultDifficulty >= 6 || g_Supervisor.cfg.playSounds >= 2 ||
            g_Supervisor.cfg.windowed >= 2 || g_Supervisor.cfg.frameskipConfig >= 3 ||
            g_Supervisor.cfg.effectQuality >= 3 || g_Supervisor.cfg.slowMode >= 2 || g_Supervisor.cfg.shotSlow >= 2 ||
            g_Supervisor.cfg.version != GAME_VERSION || fileSize != 60)
        {

            g_GameErrorContext.Log(TH_ERR_CONFIG_ABNORMAL);
            memset(&g_Supervisor.cfg, 0, sizeof(GameConfiguration));
            goto SET_DEFAULT;
        }
        g_ControllerMapping = g_Supervisor.cfg.controllerMapping;
    }

    g_Supervisor.cfg.opts.useSwTextureBlending = true; // Bit ignored from PCB onwards (HW blending always used)
    if (this->cfg.opts.dontUseVertexBuf != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_VERTEX_BUFFER);
    }
    if (this->cfg.opts.disableFog != 0)
    {
        g_GameErrorContext.Log(TH_ERR_NO_FOG);
    }
    if (this->cfg.opts.force16bitTextures != false)
    {
        g_GameErrorContext.Log(TH_ERR_USE_16BIT_TEXTURES);
    }
    // This should be inlined
    if (this->ShouldForceBackbufferClear())
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_BACKBUFFER_CLEAR);
    }
    if (this->cfg.opts.displayMinimumGraphics != false)
    {
        g_GameErrorContext.Log(TH_ERR_DONT_RENDER_ITEMS);
    }
    if (this->cfg.opts.suppressUseOfGoroudShading != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_GOURAUD_SHADING);
    }
    if (this->cfg.opts.disableDepthTest != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_DEPTH_TESTING);
    }
    this->disableVsync = false;
    this->cfg.opts.force60Fps = false;

    if (this->cfg.opts.disableColorCompositing != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_TEXTURE_COLOR_COMPOSITING);
    }
    if (this->cfg.windowed != false)
    {
        g_GameErrorContext.Log(TH_ERR_LAUNCH_WINDOWED);
    }
    if (this->cfg.opts.referenceRasterizerMode != false)
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_REFERENCE_RASTERIZER);
    }
    if (this->cfg.opts.dontUseDirectInput != false)
    {
        g_GameErrorContext.Log(TH_ERR_DO_NOT_USE_DIRECTINPUT);
    }
    if (this->cfg.opts.redrawHUDEveryFrame != false)
    {
        g_GameErrorContext.Log(TH_ERR_REDRAW_HUD_EVERY_FRAME);
    }
    if (this->cfg.opts.preloadMusic != false)
    {
        g_GameErrorContext.Log(TH_ERR_PRELOAD_BGM);
    }
    if (this->cfg.opts.disableVsync != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_VSYNC);
        g_Supervisor.disableVsync = true;
    }
    if (this->cfg.opts.dontDetectTextDrawingBackground != false)
    {
        g_GameErrorContext.Log(TH_ERR_DONT_DETECT_TEXT_BG);
    }

    if (FileSystem::WriteDataToFile(configFile, &g_Supervisor.cfg, sizeof(GameConfiguration)) != 0)
    {
        g_GameErrorContext.Fatal(TH_ERR_FILE_CANNOT_BE_EXPORTED, configFile);
        g_GameErrorContext.Fatal(TH_ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

#pragma var_order(periodLoc, wavPathBuf)
// FUNCTION: th08 0x447d04
ZunBool GameManager::IsExtraUnlockedWithAllTeams()
{
    return this->IsExtraUnlockedForCharacter(SHOT_REIMU_YUKARI) &&
           this->IsExtraUnlockedForCharacter(SHOT_MARISA_ALICE) &&
           this->IsExtraUnlockedForCharacter(SHOT_SAKUYA_REMILIA) &&
           this->IsExtraUnlockedForCharacter(SHOT_YOUMU_YUYUKO);
}

ZunBool Supervisor::LoadMusic(int param_1, char *path)
{
    char wavPathBuf[256];
    char *periodLoc;

    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->ReadFileData(param_1, path);
        }

        return FALSE;
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        strcpy(wavPathBuf, path);

        periodLoc = strrchr(wavPathBuf, '.');
        periodLoc[1] = 'w';
        periodLoc[2] = 'a';
        periodLoc[3] = 'v';

        g_SoundPlayer.QueueCommand(1, param_1, wavPathBuf);
    }

    return TRUE;
}

// FUNCTION: th08 0x447e47
#pragma var_order(midiOutput, this)
ZunBool Supervisor::PlayMusic(int param_1, char *param_2)
{
    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            MidiOutput *midiOutput = g_Supervisor.midiOutput;
            midiOutput->StopPlayback();
            midiOutput->ParseFile(param_1);
            midiOutput->Play();
        }

        if (((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 3) & 1) == 0 &&
            ((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 1) & 1) == 0)
        {
            param_2[0x164CF14] = 1;
        }
        return FALSE;
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        if (g_Supervisor.cfg.opts.preloadMusic)
        {
            g_SoundPlayer.QueueCommand(4, 0, "dummy");
        }
        g_SoundPlayer.QueueCommand(2, param_1, "dummy");
        if (((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 3) & 1) == 0 &&
            ((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 1) & 1) == 0)
        {
            param_2[0x164CF14] = 1;
        }
    }

    return FALSE;
}

// FUNCTION: th08 0x447f21
#pragma var_order(periodLoc, wavPathBuf, midiOutput, this)
ZunResult Supervisor::PlayAudio(char *path, int param_2)
{
    char wavPathBuf[256];
    char *periodLoc;

    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            MidiOutput *midiOutput = g_Supervisor.midiOutput;
            midiOutput->StopPlayback();
            midiOutput->LoadFile(path);
            midiOutput->Play();
        }
        if (((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 3) & 1) == 0 &&
            ((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 1) & 1) == 0)
        {
            ((char *)param_2)[0x164CF14] = 1;
        }
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        strcpy(wavPathBuf, path);

        periodLoc = strrchr(wavPathBuf, '.');
        periodLoc[1] = 'w';
        periodLoc[2] = 'a';
        periodLoc[3] = 'v';

        g_SoundPlayer.QueueCommand(2, -1, wavPathBuf);
        if (((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 3) & 1) == 0 &&
            ((*(u32 *)((u8 *)&g_GameManager + 0x3DBAC) >> 1) & 1) == 0)
        {
            ((char *)param_2)[0x164CF14] = 1;
        }
    }
    else
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

ZunResult Supervisor::StopAudio()
{
    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->StopPlayback();
        }
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        if (g_Supervisor.IsMusicPreloadEnabled())
        {
            g_SoundPlayer.QueueCommand(4, 0, "dummy");
        }
        else
        {
            g_SoundPlayer.QueueCommand(3, 0, "dummy");
        }
    }
    else
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x4480f8
#pragma var_order(fadeTime, this)
ZunResult Supervisor::FadeOutMusic(float param_1)
{
    f32 fadeTime;

    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
            g_Supervisor.midiOutput->SetFadeOut((u32)(1000.0f * param_1));
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        if (this->framerateMultiplier == 0.0f)
            fadeTime = param_1;
        else if (this->framerateMultiplier > 1.0f)
            fadeTime = param_1;
        else
            fadeTime = param_1 / this->framerateMultiplier;

        g_SoundPlayer.QueueCommand(5, (i32)fadeTime, "");
    }
    else
    {
        return ZUN_ERROR;
    }
    return ZUN_SUCCESS;
}

ZunBool Supervisor::IsSlowModeEnabled()
{
    return g_GameManager.cfg != NULL && g_GameManager.cfg->slowMode;
}

i32 Supervisor::EnableFog()
{
    g_AnmManager->FlushVertexBuffer();

    if (this->fogState != FOG_ENABLED)
    {
        this->fogState = FOG_ENABLED;

        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }

    return 0;
}

i32 Supervisor::DisableFog()
{
    g_AnmManager->FlushVertexBuffer();

    if (this->fogState != FOG_DISABLED)
    {
        this->fogState = FOG_DISABLED;

        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }

    return 0;
}

void Supervisor::SetRenderState(D3DRENDERSTATETYPE renderStateType, int value)
{
    g_AnmManager->FlushVertexBuffer();

    this->d3dDevice->SetRenderState(renderStateType, value);
}

#pragma var_order(playTime, difference)
void Supervisor::UpdatePlayTime()
{
    DWORD playTime = timeGetTime();

    if (playTime < this->totalPlayTime)
    {
        this->totalPlayTime = playTime;
    }

    DWORD difference = playTime - this->totalPlayTime;

    g_GameManager.plst.totalHours += (difference / 3600000);
    difference %= 3600000;

    g_GameManager.plst.totalMinutes += (difference / 60000);
    difference %= 60000;

    g_GameManager.plst.totalSeconds += (difference / 1000);
    difference %= 1000;

    g_GameManager.plst.totalMilliseconds += difference;

    if (g_GameManager.plst.totalMilliseconds >= 1000)
    {
        g_GameManager.plst.totalSeconds += (g_GameManager.plst.totalMilliseconds / 1000);
        g_GameManager.plst.totalMilliseconds = (g_GameManager.plst.totalMilliseconds % 1000);
    }
    if (g_GameManager.plst.totalSeconds >= 60)
    {
        g_GameManager.plst.totalMinutes += (g_GameManager.plst.totalMilliseconds / 60);
        g_GameManager.plst.totalSeconds = (g_GameManager.plst.totalMilliseconds % 60);
    }
    if (g_GameManager.plst.totalMinutes >= 60)
    {
        g_GameManager.plst.totalHours += (g_GameManager.plst.totalMinutes / 60);
        g_GameManager.plst.totalMinutes = (g_GameManager.plst.totalMinutes % 60);
    }

    this->totalPlayTime = playTime;
}

#pragma var_order(gameTime, difference)
void Supervisor::UpdateGameTime()
{
    DWORD gameTime = timeGetTime();

    if (gameTime < this->systemTime)
    {
        this->systemTime = 0;
    }

    DWORD difference = gameTime - this->systemTime;

    g_GameManager.plst.gameHours += (difference / 3600000);
    difference %= 3600000;

    g_GameManager.plst.gameMinutes += (difference / 60000);
    difference %= 60000;

    g_GameManager.plst.gameSeconds += (difference / 1000);
    difference %= 1000;

    g_GameManager.plst.gameMilliseconds += difference;

    if (g_GameManager.plst.gameMilliseconds >= 1000)
    {
        g_GameManager.plst.gameSeconds += (g_GameManager.plst.gameMilliseconds / 1000);
        g_GameManager.plst.gameMilliseconds = (g_GameManager.plst.gameMilliseconds % 1000);
    }
    if (g_GameManager.plst.gameSeconds >= 60)
    {
        g_GameManager.plst.gameMinutes += (g_GameManager.plst.gameMilliseconds / 60);
        g_GameManager.plst.gameSeconds = (g_GameManager.plst.gameMilliseconds % 60);
    }
    if (g_GameManager.plst.gameMinutes >= 60)
    {
        g_GameManager.plst.gameHours += (g_GameManager.plst.gameMinutes / 60);
        g_GameManager.plst.gameMinutes = (g_GameManager.plst.gameMinutes % 60);
    }

    this->systemTime = gameTime;
}

#pragma var_order(versionData, versionDataExeChecksum, versionDataSize, oldPos, versionDataExeSize)
ZunResult Supervisor::CheckVersion(const char *version, i32 exeSize, i32 exeChecksum)
{
    const char *versionData;
    const char *oldPos;
    u32 versionDataSize;
    i32 versionDataExeSize;
    i32 versionDataExeChecksum;

#ifdef TH08_MODERN_PORT
    // A reconstructed executable cannot have one of the retail executable
    // sizes or checksums recorded in th08_0100d.ver. The serialized score and
    // replay formats are already structurally validated by their loaders. A
    // portable build has no retail executable identity to compare, including
    // for the bundled demo replays, so leave the integrity whitelist intact
    // only for the reconstruction build.
    (void)version;
    (void)exeSize;
    (void)exeChecksum;
    return ZUN_SUCCESS;
#endif

    if (this->versionData == NULL)
    {
        return ZUN_SUCCESS;
    }

    versionData = this->versionData;
    versionDataSize = this->versionDataSize;

    if (strncmp(version, CONFIG_DEBUG_VERSION_STRING, CONFIG_VERSION_STRING_LENGTH) == 0)
    {
        return ZUN_SUCCESS;
    }

    if (strcmp(CONFIG_VERSION_STRING, CONFIG_DEBUG_VERSION_STRING) == 0)
    {
        return ZUN_SUCCESS;
    }

    /* The version data file contains a list of all the accepted versions
     * for the game.
     * Each line in that file is in the following format:
     *
     * VERSION  EXE_SIZE EXE_CHECKSUM
     *
     * For example:
     *
     * 0100d 840704 2724749753
     */
    while (versionDataSize > 0)
    {
        if (strncmp(version, versionData, CONFIG_VERSION_STRING_LENGTH) == 0)
        {
            // As long as we do not have a fully matching binary, we cannot
            // get the same binary size and checksum of any of the valid
            // versions in the version data file. Also, the original logic
            // would be totally broken on any ports of this decompilation.
            // So if the version string matches, just take it.
#ifdef FIX_REALLY_BAD_BUGS

            return ZUN_SUCCESS;
#else
            /* ZUN bloat: the format string could have been "%*s %d %d", with
             * the %*s meaning ignore the first string.
             */
            versionData += CONFIG_VERSION_STRING_LENGTH + 1;
            sscanf(versionData, "%d %d", &versionDataExeSize, &versionDataExeChecksum);

            if (versionDataExeSize == exeSize && versionDataExeChecksum == exeChecksum)
            {
                return ZUN_SUCCESS;
            }
#endif
        }

        oldPos = versionData;
        versionData = strchr(versionData, '\n') + 1;
        versionDataSize -= versionData - oldPos;
    }

    return ZUN_ERROR;
}

ZunResult Supervisor::ThreadStart(LPTHREAD_START_ROUTINE startFunction, void *startParam)
{
    this->ThreadClose();

    utils::GuiDebugPrint("info : Sub Thread Start Request\n");

    this->runningSubthreadHandle = CreateThread(NULL, 0, startFunction, startParam, 0, &this->runningSubthreadID);

    this->unk290 = TRUE;

    return (this->runningSubthreadHandle != NULL) ? ZUN_SUCCESS : ZUN_ERROR;
}

void Supervisor::ThreadClose()
{
    if (this->runningSubthreadHandle != NULL)
    {
        utils::GuiDebugPrint("info : Sub Thread Close Request\n");
        this->subthreadCloseRequestActive = TRUE;

        while (WaitForSingleObject(this->runningSubthreadHandle, 1000) == WAIT_TIMEOUT)
            Sleep(1);

        CloseHandle(this->runningSubthreadHandle);
        this->runningSubthreadHandle = NULL;
        this->subthreadCloseRequestActive = FALSE;
    }
}

void Supervisor::SetupLoadingVms(Float3 *position)
{
    if (this->loadingVmsHaveBeenSetup == 0)
    {
        this->loadingAnm->ExecuteAnmIdx(&g_SupervisorLoadingVms[0], 0);
        this->loadingAnm->ExecuteAnmIdx(&g_SupervisorLoadingVms[1], 1);
        this->loadingAnm->ExecuteAnmIdx(&g_SupervisorLoadingVms[2], 2);

        this->loadingVmsHaveBeenSetup = 1;

        g_SupervisorLoadingVms[0].pos = *position;
        g_SupervisorLoadingVms[1].pos = *position;
        g_SupervisorLoadingVms[2].pos = *position;
    }
}

void Supervisor::HideLoadingVms(void)
{
    if (this->loadingVmsHaveBeenSetup == 1)
    {
        g_SupervisorLoadingVms[0].SetInterrupt(1);
        g_SupervisorLoadingVms[1].SetInterrupt(1);
        g_SupervisorLoadingVms[2].SetInterrupt(1);
        this->loadingVmsHaveBeenSetup = 0;
    }
    if (g_SupervisorScreenEffect != NULL)
    {
        g_SupervisorScreenEffect->FUN_0045c160();
        g_SupervisorScreenEffect = NULL;
    }
}

// FUNCTION: th08 0x448972
void Supervisor::FUN_00448972()
{
    if (this->loadingVmsHaveBeenSetup == 1)
    {
        g_SupervisorLoadingVms[0].SetInterrupt(2);
        g_SupervisorLoadingVms[1].SetInterrupt(2);
        g_SupervisorLoadingVms[2].SetInterrupt(2);
        this->loadingVmsHaveBeenSetup = 2;
    }
    if (g_SupervisorScreenEffect != NULL)
    {
        g_SupervisorScreenEffect->FUN_0045c160();
        g_SupervisorScreenEffect = NULL;
    }
}

void Supervisor::SetupLoadingVmsAndInitCapture(Float3 *position)
{
    if (this->loadingVmsHaveBeenSetup == 0)
    {
        this->loadingAnm->ExecuteAnmIdx(&g_SupervisorLoadingVms[0], 0);
        this->loadingAnm->ExecuteAnmIdx(&g_SupervisorLoadingVms[1], 1);
        this->loadingAnm->ExecuteAnmIdx(&g_SupervisorLoadingVms[2], 2);

        this->loadingVmsHaveBeenSetup = 1;

        g_SupervisorLoadingVms[0].pos = *position;
        g_SupervisorLoadingVms[1].pos = *position;
        g_SupervisorLoadingVms[2].pos = *position;
    }

    g_AnmManager->RequestCapture(8, 0, 0, 640, 480, 0, 0, 640, 480);
}

void Supervisor::StartEffect(i32 idx)
{
    if (g_SupervisorScreenEffect == NULL)
    {
        g_SupervisorScreenEffect =
            ScreenEffect::RegisterChain((ScreenEffectType)(idx + SCREEN_EFFECT_UNK5), 60, 0, 0, 0, 1);
    }
}

void Supervisor::InitializeCriticalSections()
{
    for (u32 i = 0; i < ARRAY_SIZE_SIGNED(this->criticalSections); i++)
    {
        InitializeCriticalSection(&this->criticalSections[i]);
    }
}

void Supervisor::DeleteCriticalSections()
{
    for (u32 i = 0; i < ARRAY_SIZE(this->criticalSections); i++)
    {
        DeleteCriticalSection(&this->criticalSections[i]);
    }
}

}; // namespace th08
