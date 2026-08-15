#pragma once

#include <d3d8.h>
#include <d3dx8math.h>
#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>

#include "Global.hpp"
#include "Midi.hpp"
#include "ZunBool.hpp"
#include "ZunMath.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "utils.hpp"

namespace th08
{
#define GAME_VERSION 0x80001
#define ZWAV_MAGIC 'VAWZ'

enum MusicMode
{
    OFF = 0,
    WAV = 1,
    MIDI = 2
};

enum EffectQuality
{
    MINIMUM,
    MODERATE,
    MAXIMUM
};

enum FogState
{
    FOG_DISABLED = 0,
    FOG_ENABLED = 1,
    FOG_UNSET = 0xff
};

struct GameConfigOpts
{
    u32 useSwTextureBlending : 1;
    u32 dontUseVertexBuf : 1;
    u32 force16bitTextures : 1;
    u32 clearBackBufferOnRefresh : 1;
    u32 displayMinimumGraphics : 1;
    u32 suppressUseOfGoroudShading : 1;
    u32 disableDepthTest : 1;
    u32 force60Fps : 1;
    u32 disableColorCompositing : 1;
    u32 referenceRasterizerMode : 1;
    u32 disableFog : 1;
    u32 dontUseDirectInput : 1;
    u32 redrawHUDEveryFrame : 1;
    u32 preloadMusic : 1;
    u32 disableVsync : 1;
    u32 dontDetectTextDrawingBackground : 1;
};

struct GameConfiguration
{
    ControllerMapping controllerMapping;
    i32 version;
    i16 padXAxis;
    i16 padYAxis;
    u8 lifeCount;
    u8 bombCount;
    u8 colorMode16bit;
    u8 musicMode;
    u8 playSounds;
    u8 defaultDifficulty;
    u8 windowed;
    // 0 = fullspeed, 1 = 1/2 speed, 2 = 1/4 speed.
    u8 frameskipConfig;
    u8 effectQuality;
    u8 slowMode;
    u8 shotSlow;
    i8 musicVolume;
    i8 sfxVolume;
    i8 unk29[15];
    GameConfigOpts opts;
};

struct SupervisorFlags
{
    u32 usingHardwareTL : 1;
    u32 unk1 : 1; // Unconditionally set in InitD3DRendering. Never cleared?
    u32 using32BitGraphics : 1;
    u32 speedhackDetected : 1; // Leftover from PCB. Is never set in IN, but is used.
    u32 d3dDevDisconnectFlag : 1;
    u32 unk5 : 1;
    u32 unk6 : 1; // Set if LPTITLE is NULL in the startup info, which seems to never be true?
    u32 receivedCloseMsg : 1;
    u32 unk8 : 1;
};

enum SupervisorState
{
    SupervisorState_ExitGame = -1,
    SupervisorState_Init = 0,
    SupervisorState_TitleScreen = 1,
    SupervisorState_GameManager = 2,
    SupervisorState_GameManagerReInit = 3,
    SupervisorState_ExitGame2 = 4,
    SupervisorState_ResultScreen = 5,
    SupervisorState_ResultScreenFromGame = 6,
    SupervisorState_FinishReplay = 7,
    SupervisorState_MusicRoom = 8,
    SupervisorState_Ending = 9,
    SupervisorState_GameManagerRestartFromBeginning = 10,
    SupervisorState_SpellcardPracticeRestart = 11,
    SupervisorState_GameManagerNextStageWeird = 12,
};

/* This forward declaration is to prevent including AnmManager.hpp */
struct AnmLoaded;

struct Supervisor
{
    Supervisor();
    static ZunResult RegisterChain();

    static ChainCallbackResult OnUpdate(Supervisor *s);
    static BOOL CALLBACK ControllerCallback(LPCDIDEVICEOBJECTINSTANCEA lpddoi, LPVOID pvRef);
    static int AddedCallback(Supervisor *s);
    static ZunResult LoadDat();
    static i32 CheckFps();
    static void StartupThread(Supervisor *s);
    ZunResult SetupDInput();
    static BOOL CALLBACK EnumGameControllersCb(LPCDIDEVICEINSTANCE pdidInstance, LPVOID pContext);
    static ZunResult DeletedCallback(Supervisor *s);
    static ChainCallbackResult DrawFpsCounter(Supervisor *s);
    static ChainCallbackResult OnDraw2(Supervisor *s);
    static ChainCallbackResult DrawLoadingVms(Supervisor *s);
    static void CalculateFps(ZunBool shouldDraw);
    ZunResult CheckVersion(const char *version, i32 exeSize, i32 exeChecksum);

    ZunResult LoadConfig(char *configFile);
    ZunBool LoadMusic(int param_1, char *param_2);
    ZunBool PlayMusic(int param_1, char *param_2);
    ZunResult PlayAudio(char *path, int param_2);
    ZunResult StopAudio();
    ZunBool IsSlowModeEnabled();
    ZunResult FadeOutMusic(float param_1);

    void ThreadClose();
    void SetupLoadingVms(Float3 *position);
    void HideLoadingVms(void);
    void SetupLoadingVmsAndInitCapture(Float3 *position);
    void StartEffect(i32 idx);
    void InitializeCriticalSections();
    void DeleteCriticalSections();
    void TickTimer(i32 *frames, float *subframes);
    ZunBool TakeSnapshot(const char *filePath);
    void SetRenderState(D3DRENDERSTATETYPE renderStateType, int value);
    i32 DisableFog();
    i32 EnableFog();
    void UpdatePlayTime();
    void UpdateGameTime();

    ZunResult ThreadStart(LPTHREAD_START_ROUTINE startFunction, void *startParam);

    void ResetUnknownStuff()
    {
        this->unk0x338 = 0;
        this->unk0x340 = 0;
        this->unk0x34c = 0;
        this->unk0x344 = 0;
        this->unk0x348 = 0;
    }

    ZunBool IsShotSlowEnabled()
    {
        return this->cfg.shotSlow;
    }

    ZunBool IsSpeedhackDetected()
    {
        return this->flags.speedhackDetected;
    }

    ZunBool ShouldForceBackbufferClear()
    {
        return this->cfg.opts.clearBackBufferOnRefresh | this->cfg.opts.displayMinimumGraphics;
    }

    ZunBool IsHardwareBlendingDisabled()
    {
        return this->cfg.opts.useSwTextureBlending;
    }

    ZunBool IsVertexBufferDisabled()
    {
        return this->cfg.opts.dontUseVertexBuf;
    }

    ZunBool Is16bitColorMode()
    {
        return this->cfg.opts.force16bitTextures;
    }

    ZunBool IsDepthTestDisabled();

    ZunBool IsColorCompositingDisabled();

    ZunBool IsFogDisabled();

    ZunBool IsHUDRedrawEnabled();

    ZunBool IsReferenceRasterizerMode()
    {
        return this->cfg.opts.referenceRasterizerMode;
    }

    ZunBool IsMusicPreloadEnabled()
    {
        return this->cfg.opts.preloadMusic;
    }

    ZunBool IsWindowed()
    {
        return this->cfg.windowed;
    }

    ZunBool IsSoftwareTexturing()
    {
        return this->cfg.opts.disableColorCompositing | this->cfg.opts.useSwTextureBlending;
    }

    ZunBool IsMinimumGraphicsMode();

    ZunBool IsSubthreadRunning()
    {
        return this->runningSubthreadHandle != NULL;
    }

    void EnterCriticalSectionWrapper(int id)
    {
        EnterCriticalSection(&this->criticalSections[id]);
        this->lockCounts[id]++;
    }

    void LeaveCriticalSectionWrapper(int id)
    {
        LeaveCriticalSection(&this->criticalSections[id]);
        this->lockCounts[id]--;
    }

    void ClearFogState()
    {
        this->fogState = FOG_UNSET;
    }

    HINSTANCE hInstance;
    PDIRECT3D8 d3dIface;
    PDIRECT3DDEVICE8 d3dDevice;
    LPDIRECTINPUT8A dInputIface;
    LPDIRECTINPUTDEVICE8A keyboard;
    LPDIRECTINPUTDEVICE8A controller;
    DIDEVCAPS controllerCaps;
    HWND hwndGameWindow;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    D3DPRESENT_PARAMETERS presentParameters;
    DummyMidiTimer *dummyMidiTimer;
    GameConfiguration cfg;
    i32 calcCount;
    i32 wantedState;
    i32 curState;
    i32 wantedState2;
    i32 unk164;
    i32 unk168;
    i32 unk16c;
    i32 unk170;
    i32 unk174; // Commonly set for screen transitions and decremented once per frame, but never actually used for
                // anything
    i32 unk178;
    BOOL disableVsync;
    ZunBool couldSetRefreshRate;
    i32 lastFrameTime; // Unused in IN
    f32 framerateMultiplier;
    MidiOutput *midiOutput;
    float lagNumerator;
    float lagDenominator;
    u32 unk198;
    AnmLoaded *textAnm;
    AnmLoaded *loadingAnm;
    SupervisorFlags flags;
    DWORD totalPlayTime;
    DWORD systemTime;
    D3DCAPS8 d3dCaps;
    HANDLE runningSubthreadHandle;
    DWORD runningSubthreadID;
    BOOL subthreadCloseRequestActive;
    BOOL unk290;
    u32 unk294;
    CRITICAL_SECTION criticalSections[4];
    u8 lockCounts[4];
    i32 loadingVmsHaveBeenSetup;

    unknown_fields(0x300, 0x38);

    u32 unk0x338;
    u32 unk0x33c;
    u32 unk0x340;
    u32 unk0x344;
    u32 unk0x348;
    u32 unk0x34c;

    FogState fogState;
    u32 exeChecksum;
    u32 exeSize;

    i32 versionDataSize;
    char *versionData;
};
C_ASSERT(sizeof(Supervisor) == 0x364);
DIFFABLE_EXTERN(Supervisor, g_Supervisor);

#define CRASH_GAME() memset(&g_Supervisor, -1, sizeof(g_Supervisor))

struct ZunTimer
{
    int previous;
    float subFrame;
    int current;

    ZunTimer()
    {
        Initialize();
    }

    void Initialize()
    {
        this->current = 0;
        this->previous = -999;
        this->subFrame = 0.0;
    }

    operator int()
    {
        return this->current;
    }

    operator float()
    {
        return (float)this->current + (float)this->subFrame;
    }

    void operator++(int)
    {
        Tick();
    }

    i32 Tick()
    {
        this->previous = this->current;
        g_Supervisor.TickTimer(&this->current, &this->subFrame);
        return this->current;
    }

    void operator--(int)
    {
        this->Decrement(1);
    }

    ZunBool operator==(int value)
    {
        return this->current == value;
    }

    void operator+=(int value);

    ZunBool operator-=(int value)
    {
        this->Decrement(value);
    }

    ZunBool operator<(int value);

    ZunBool operator<=(int value)
    {
        return this->current <= value;
    }

    ZunBool operator>(int value);

    ZunBool operator>=(int value)
    {
        return this->current >= value;
    }

    void Increment(i32 value);
    void Decrement(i32 value);

    void operator=(i32 value)
    {
        SetCurrent(value);
    }

    void SetCurrent(i32 value)
    {
        this->current = value;
        this->subFrame = 0.0;
        this->previous = -999;
    }
};
}; // namespace th08
