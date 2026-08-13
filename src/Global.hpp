#pragma once

#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "pbg/PbgArchive.hpp"
#include "utils.hpp"
#include <d3dx8.h>
#include <windows.h>

namespace th08
{

#define IS_PRESSED(key) (g_CurFrameInput & (key))
#define WAS_PRESSED(key) (((g_CurFrameInput & (key)) != 0) && (g_CurFrameInput & (key)) != (g_LastFrameInput & (key)))
#define WAS_PRESSED_SCROLLING(key)                                                                                     \
    (WAS_PRESSED(key) || (((g_CurFrameInput & (key)) != 0) && (g_IsEighthFrameOfHeldInput != 0)))

/* zunName is ZUN's original name for this type */
#define ZUN_NEW(type, zunName) ((type *)g_ZunMemory.AddToRegistry(new type(), sizeof(type), zunName))
#define ZUN_NEW_ARRAY(type, number, zunName)                                                                           \
    ((type *)g_ZunMemory.AddToRegistry(new type[number], sizeof(type) * number, zunName))
#define ZUN_DELETE(p)                                                                                                  \
    g_ZunMemory.RemoveFromRegistry(p);                                                                                 \
    delete p;                                                                                                          \
    p = NULL;
#define ZUN_DELETE2(p)                                                                                                 \
    delete p;                                                                                                          \
    p = NULL;

#define ZUN_FREE(p)                                                                                                    \
    g_ZunMemory.Free(p);                                                                                               \
    p = NULL;

enum ChainCallbackResult
{
    CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB = (unsigned int)0,
    CHAIN_CALLBACK_RESULT_CONTINUE = (unsigned int)1,
    CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN = (unsigned int)2,
    CHAIN_CALLBACK_RESULT_BREAK = (unsigned int)3,
    CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS = (unsigned int)4,
    CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR = (unsigned int)5,
    CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB = (unsigned int)6,
};

typedef ChainCallbackResult (*ChainCallback)(void *);
typedef ZunResult (*ChainLifetimeCallback)(void *);

// TODO: rename to funcChainInf
class ChainElem
{
  public:
    ChainElem();
    ~ChainElem();

    void SetCallback(ChainCallback callback)
    {
        this->callback = callback;
        this->addedCallback = NULL;
        this->deletedCallback = NULL;
    }

    short priority;
    u16 isHeapAllocated : 1;
    ChainCallback callback;
    ChainLifetimeCallback addedCallback;
    ChainLifetimeCallback deletedCallback;
    struct ChainElem *prev;
    struct ChainElem *next;
    struct ChainElem *unkPtr;
    void *arg;
};

class Chain
{
  private:
    ChainElem calcChain;
    ChainElem drawChain;

    void ReleaseSingleChain(ChainElem *root);
    void CutImpl(ChainElem *to_remove);

  public:
    Chain();
    ~Chain();

    void Cut(ChainElem *to_remove);
    void Release();
    int AddToCalcChain(ChainElem *elem, int priority);
    int AddToDrawChain(ChainElem *elem, int priority);
    int RunDrawChain();
    int RunCalcChain();

    ChainElem *CreateElem(ChainCallback callback);
};

enum TouhouButton
{
    TH_BUTTON_SHOOT = 1 << 0,
    TH_BUTTON_BOMB = 1 << 1,
    TH_BUTTON_FOCUS = 1 << 2,
    TH_BUTTON_MENU = 1 << 3,
    TH_BUTTON_UP = 1 << 4,
    TH_BUTTON_DOWN = 1 << 5,
    TH_BUTTON_LEFT = 1 << 6,
    TH_BUTTON_RIGHT = 1 << 7,
    TH_BUTTON_SKIP = 1 << 8,
    TH_BUTTON_Q = 1 << 9,
    TH_BUTTON_S = 1 << 10,
    TH_BUTTON_HOME = 1 << 11,
    TH_BUTTON_ENTER = 1 << 12,
    TH_BUTTON_D = 1 << 13,
    TH_BUTTON_RESET = 1 << 14,

    TH_BUTTON_UP_LEFT = TH_BUTTON_UP | TH_BUTTON_LEFT,
    TH_BUTTON_UP_RIGHT = TH_BUTTON_UP | TH_BUTTON_RIGHT,
    TH_BUTTON_DOWN_LEFT = TH_BUTTON_DOWN | TH_BUTTON_LEFT,
    TH_BUTTON_DOWN_RIGHT = TH_BUTTON_DOWN | TH_BUTTON_RIGHT,
    TH_BUTTON_DIRECTION = TH_BUTTON_DOWN | TH_BUTTON_RIGHT | TH_BUTTON_UP | TH_BUTTON_LEFT,

    TH_BUTTON_SELECTMENU = TH_BUTTON_ENTER | TH_BUTTON_SHOOT,
    TH_BUTTON_RETURNMENU = TH_BUTTON_MENU | TH_BUTTON_BOMB,
    TH_BUTTON_WRONG_CHEATCODE =
        TH_BUTTON_SHOOT | TH_BUTTON_BOMB | TH_BUTTON_MENU | TH_BUTTON_Q | TH_BUTTON_S | TH_BUTTON_ENTER,
    TH_BUTTON_ANY = 0xFFFF,
};

namespace Controller
{
u16 GetJoystickCaps();
u32 SetButtonFromControllerInputs(u16 *outButtons, i16 controllerButtonToTest, u16 touhouButton, u32 inputButtons);

u32 SetButtonFromDirectInputJoystate(u16 *outButtons, i16 controllerButtonToTest, u16 touhouButton, u8 *inputButtons);

u16 GetControllerInput(u16 buttons);
u8 *GetControllerState();
u16 GetInput();
void ResetKeyboard();
}; // namespace Controller

namespace FileSystem
{
LPBYTE Decrypt(LPBYTE inData, i32 size, u8 xorValue, u8 xorValueInc, i32 chunkSize, i32 maxBytes);
LPBYTE TryDecryptFromTable(LPBYTE inData, LPINT unused, i32 size);
LPBYTE Encrypt(LPBYTE inData, i32 size, u8 xorValue, u8 xorValueInc, i32 chunkSize, i32 maxBytes);
LPBYTE OpenFile(LPCSTR path, i32 *fileSize, BOOL isExternalResource);
BOOL CheckIfFileAlreadyExists(LPCSTR path);
int WriteDataToFile(LPCSTR path, LPVOID data, size_t size);
}; // namespace FileSystem

class GameErrorContext
{
  public:
    GameErrorContext();
    ~GameErrorContext();

    void ResetContext()
    {
        this->bufferEnd = this->buffer;
        this->bufferEnd[0] = '\0';
    }

    void Flush()
    {
        if (this->bufferEnd != this->buffer)
        {
            Log("---------------------------------------------------------- \r\n");

            if (this->showMessageBox)
            {
                MessageBoxA(NULL, this->buffer, "log", MB_ICONSTOP);
            }

            FileSystem::WriteDataToFile("./log.txt", this->buffer, strlen(this->buffer));
        }
    }

    const char *Log(const char *fmt, ...);
    const char *Fatal(const char *fmt, ...);

  private:
    char buffer[0x2000];
    char *bufferEnd;
    i8 showMessageBox;
};

class Rng
{
  public:
    u16 GetRandomU16();
    u32 GetRandomU32();
    f32 GetRandomF32();
    f32 GetRandomF32Signed();

    void ResetGenerationCount()
    {
        this->generationCount = 0;
    }

    void SetSeed(u16 newSeed)
    {
        this->seed = newSeed;
    }

    u16 GetSeed()
    {
        return this->seed;
    }

    u16 GetRandomU16InRange(u16 range)
    {
        return range != 0 ? GetRandomU16() % range : 0;
    }

    u32 GetRandomU32InRange(u32 range)
    {
        return range != 0 ? GetRandomU32() % range : 0;
    }

    f32 GetRandomF32InRange(f32 range)
    {
        return GetRandomF32() * range;
    }

    f32 GetRandomF32SignedInRange(f32 range)
    {
        return GetRandomF32Signed() * range;
    }

  private:
    u16 seed, seedBackup;
    u32 generationCount;
};

class ZunMemory
{
  public:
    ZunMemory();
    ~ZunMemory();

    // NOTE: the default parameter for debugText is probably just __FILE__
    void *Alloc(size_t size, const char *debugText = "d:\\cygwin\\home\\zun\\prog\\th08\\global.h")
    {
        return malloc(size);
    }

    void Free(void *ptr)
    {
        free(ptr);
    }

    void *AddToRegistry(void *ptr, size_t size, char *name)
    {
#ifdef DEBUG
        this->bRegistryInUse = TRUE;
        for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->registry); i++)
        {
            if (this->registry[i] == NULL)
            {
                RegistryInfo *info = (RegistryInfo *)malloc(sizeof(*info));
                if (info != NULL)
                {
                    info->data = ptr;
                    info->size = size;
                    info->name = name;
                    this->registry[i] = info;
                }
                break;
            }
        }
#endif
        return ptr;
    }

    void RemoveFromRegistry(VOID *ptr)
    {
#ifdef DEBUG
        for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->registry); i++)
        {
            if (this->registry[i] == ptr)
            {
                free(this->registry[i]);
                this->registry[i] = NULL;
                break;
            }
        }
#endif
    }

  private:
    struct RegistryInfo
    {
        void *data;
        size_t size;
        char *name;
    };

    RegistryInfo *registry[0x1000];
    BOOL bRegistryInUse;
};

struct ControllerMapping
{
    i16 shotButton;
    i16 bombButton;
    i16 focusButton;
    i16 menuButton;
    i16 upButton;
    i16 downButton;
    i16 leftButton;
    i16 rightButton;
    i16 skipButton;
};

struct ZunGlobals
{
    u32 displayScore;
    i32 grazeInStage;
    u32 score;
    i32 graze;
    i32 unk0x10;
    u32 displayedHighScore;
    u8 continuesUsedInHighScore;
    /* 3 bytes pad */
    i32 spellcardsCaptured;
    i16 youkaiGaugeCopy;
    i16 youkaiGauge;
    i32 pointItemValue;
    u8 clockTime;
    u8 numRetries;
    /* 2 bytes pad */
    i32 pointItemsCollectedInStage;
    i32 pointItemsCollected;
    u32 pointItemExtendsSoFar;
    i32 nextPointItemExtendThreshold;
    i32 currentTimeOrbs;
    i32 lastSpellTimeOrbThreshold;
    i32 totalTimeOrbs;
    i32 rng1[7];
    f32 deaths;
    f32 deathInStage;
    f32 rng2[2];
    f32 livesRemaining;
    f32 rng3[2];
    f32 bombsRemaining;
    f32 bombsUsed;
    f32 bombsUsedInStage;
    f32 rng4[3];
    f32 playerPower;
    f32 rng5[2];
    i32 rng6;
    i32 rng7[8];
    u32 antiTamperValue;
    i32 antiTamperChecksum;
    i32 rng8[5];
};

C_ASSERT(sizeof(ZunGlobals) == 0xe4);

DIFFABLE_EXTERN(Rng, g_Rng);
DIFFABLE_EXTERN(u16, g_CurFrameInput);
DIFFABLE_EXTERN(u16, g_LastFrameInput);
DIFFABLE_EXTERN(u16, g_NumOfFramesInputsWereHeld);
DIFFABLE_EXTERN(u16, g_IsEighthFrameOfHeldInput);
DIFFABLE_EXTERN(GameErrorContext, g_GameErrorContext);
DIFFABLE_EXTERN(Chain, g_Chain);
DIFFABLE_EXTERN(PbgArchive, g_PbgArchive);
DIFFABLE_EXTERN(ZunMemory, g_ZunMemory);
DIFFABLE_EXTERN(ControllerMapping, g_ControllerMapping);

i32 IsResourceReloadEnabled();
}; // namespace th08
