#pragma once
#include "AnmManager.hpp"
#include "Global.hpp"
#include "Supervisor.hpp"
#include "ZunColor.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

#include <d3dx8.h>

#define ASCII_MAX_STRINGS 256
#define ASCII_MAX_SCORE_POPUPS 720
#define ASCII_MAX_PLAYER_POPUPS 3
#define ASCII_MAX_TIME_POPUPS 128

namespace th08
{

struct PauseMenu
{
    PauseMenu();

    i32 OnUpdate();
    i32 OnDraw();

    u32 curState;
    i32 numFrames;
    AnmVm menuSprites[10];
    AnmVm menuBackground;
};

C_ASSERT(sizeof(PauseMenu) == 0x1d14);

struct RetryMenu
{
    RetryMenu();

    i32 OnUpdate();
    i32 OnDraw();

    u32 curState;
    i32 numFrames;

    AnmVm menuSprites[6];
    AnmVm menuBackground;
};

C_ASSERT(sizeof(RetryMenu) == 0x1284);

struct AsciiManagerString
{
    AsciiManagerString();

    char text[64];
    Float3 position;
    D3DCOLOR color;
    f32 scaleX;
    f32 scaleY;
    ZunBool isSelected;
    ZunBool isGui;
};

C_ASSERT(sizeof(AsciiManagerString) == 0x60);

struct AsciiManagerPopup
{
    AsciiManagerPopup();

    char text[12];
    Float3 position;
    D3DCOLOR color;
    ZunTimer timer;
    f32 scaleX;
    f32 scaleY;
    bool inUse;
    BYTE characterCount;
    u32 unk0x34;
};

C_ASSERT(sizeof(AsciiManagerPopup) == 0x38);

struct AsciiManager
{
    static ChainCallbackResult OnUpdate(AsciiManager *mgr);
    static ChainCallbackResult OnDrawLowPrio(AsciiManager *mgr);
    static ChainCallbackResult OnDrawHighPrio(AsciiManager *mgr);
    static ZunResult RegisterChain();
    static ZunResult AddedCallback(AsciiManager *mgr);
    static ZunResult DeletedCallback(AsciiManager *mgr);
    static void CutChain();
    void AddString(Float3 *position, const char *string);
    void AddFormatText(Float3 *position, const char *fmt, ...);
    int AddFormatText2(Float3 *position, const char *fmt, ...);
    void OnDrawLowPrioImpl();
    void CreateScorePopup(Float3 *position, i32 number, D3DCOLOR color);
    void CreatePlayerPointPopup(Float3 *position, i32 number, D3DCOLOR color);
    void CreateTimePopup(Float3 *position, i32 number, i32 param3, D3DCOLOR color);
    void CreateFamiliarPopup(Float3 *position, i32 number, i32 param3, D3DCOLOR color);
    void OnDrawHighPrioImpl();
    void DrawPercentage(Float3 *position, i32 percentage, D3DCOLOR color);

    void Reset();
    void InitializeVms();

    void SetColor(D3DCOLOR color)
    {
        this->color.d3dColor = color;
    }

    void SetIsSelected(ZunBool selected)
    {
        this->isSelected = selected;
    }

    void SetScale(float scaleX, float scaleY)
    {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
    }

    void UpdateVms()
    {
        g_AnmManager->ExecuteScript(&this->youkaiGauge);
        g_AnmManager->ExecuteScript(&this->youkaiGaugeHumanIcon);
        g_AnmManager->ExecuteScript(&this->youkaiGaugeYoukaiIcon);
        g_AnmManager->ExecuteScript(&this->youkaiGaugeCursor);
        g_AnmManager->ExecuteScript(&this->percentageText);
        g_AnmManager->ExecuteScript(&this->bossMarkers[0]);
        g_AnmManager->ExecuteScript(&this->bossMarkers[1]);
        g_AnmManager->ExecuteScript(&this->bossMarkers[2]);
        g_AnmManager->ExecuteScript(&this->bossMarkers[3]);
        g_AnmManager->ExecuteScript(&this->unk_1520);
    }

    void SetGaugeInterrupt(i32 interrupt)
    {
        this->youkaiGauge.SetInterrupt(interrupt);
        this->youkaiGaugeHumanIcon.SetInterrupt(interrupt);
        this->youkaiGaugeYoukaiIcon.SetInterrupt(interrupt);
        this->youkaiGaugeCursor.SetInterrupt(interrupt);

        this->gaugeInterrupt = interrupt;
    }

    i32 GetGaugeInterrupt()
    {
        return this->gaugeInterrupt;
    }

    void ResetStrings()
    {
        this->numStrings = 0;
    }

    void SetSpaceWidth(i32 spaceWidth)
    {
        this->spaceWidth = spaceWidth;
    }

    AnmVm largeText;
    AnmVm smallScoreText;
    AnmVm popupText;
    AnmVm youkaiGauge;
    AnmVm youkaiGaugeHumanIcon;
    AnmVm youkaiGaugeYoukaiIcon;
    AnmVm youkaiGaugeCursor;
    AnmVm percentageText;
    AnmVm unk_1520;

    AnmVm bossMarkers[4];
    i32 bossMarkerStates[4];

    AsciiManagerString strings[ASCII_MAX_STRINGS];
    i32 numStrings;

    ZunColor color;
    f32 scaleX;
    f32 scaleY;
    ZunBool isGui;
    ZunBool isSelected;

    i32 gaugeInterrupt;
    i32 spaceWidth;
    u32 unk_8284;

    AnmLoaded *asciiAnm;
    AnmLoaded *captureAnm;

    i32 nextScorePopupIndex;
    i32 nextPlayerPointPopupIndex;
    i32 nextTimePopupIndex;

    ZunBool unk0x829c;

    PauseMenu pauseMenu;
    RetryMenu retryMenu;

    AnmVm demoIcon;

    AsciiManagerPopup scorePopups[ASCII_MAX_SCORE_POPUPS + ASCII_MAX_PLAYER_POPUPS];
    AsciiManagerPopup timePopups[ASCII_MAX_TIME_POPUPS];

    f32 unk_16f04;
    i32 unk_16f08;

    AnmVm unk_16f0c;
};

C_ASSERT(sizeof(AsciiManager) == 0x171b0);
DIFFABLE_EXTERN(AsciiManager, g_AsciiManager);

} // namespace th08
