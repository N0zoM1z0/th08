#pragma once
#include "Supervisor.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "dxutil.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <d3d8.h>

#define GAME_WINDOW_WIDTH 640
#define GAME_WINDOW_HEIGHT 480

namespace th08
{
struct VertexDiffuseXyzrhw
{
    VertexDiffuseXyzrhw();

    Float3 pos;
    f32 w;
    D3DCOLOR diffuse;
};

struct VertexTex1DiffuseXyzrhw
{
    Float3 pos;
    float w;
    D3DCOLOR diffuse;
    Float2 textureUV;
};

// Target-observed 4-vertex textured quad without a diffuse color field.
struct VertexTex0Xyzrhw
{
    Float3 pos;
    float w;
    Float2 textureUV;
};
C_ASSERT(sizeof(VertexTex0Xyzrhw) == 0x18);

// Touhou 8 uses DirectX 8.1, but evidently Zun used some mismatched DirectX 8 headers as well
// D3DXIMAGE_INFO changed from 20 to 28 bytes between DX8 and DX8.1, but somehow IN uses the the DX8 version
// This struct is a redefinition of the DX8 D3DXIMAGE_INFO for that
// The only reason this ABI mismatch doesn't cause issues is because no surface indices are ever loaded other than 0
struct ZunImageInfo
{
    u32 Width;
    u32 Height;
    u32 Depth;
    u32 MipLevels;
    D3DFORMAT Format;
};
C_ASSERT(sizeof(ZunImageInfo) == 0x14);

enum AnmBlendMode
{
    AnmBlendMode_Unset = -1,
    AnmBlendMode_Normal,
    AnmBlendMode_Additive
};

enum AnmColorOp
{
    AnmColorOp_Unset = -1
};

enum AnmVertexShader
{
    AnmVertexShader_Unset = -1
};

enum AnmCameraMode
{
    AnmCameraMode_Unset = -1
};

enum AnmZWriteMode
{
    AnmZWriteMode_Unset = -1
};

enum AnmVariable
{
    AnmVariable_I0 = 10000,
    AnmVariable_I1,
    AnmVariable_I2,
    AnmVariable_I3,
    AnmVariable_F0,
    AnmVariable_F1,
    AnmVariable_F2,
    AnmVariable_F3,
    AnmVariable_IC0,
    AnmVariable_IC1,
};

enum AnmInterp
{
    AnmInterp_Pos,
    AnmInterp_RGB1,
    AnmInterp_Alpha1,
    AnmInterp_Rotate,
    AnmInterp_Scale,
    AnmInterp_RGB2,
    AnmInterp_Alpha2,
    AnmInterp_Last
};

enum AnmInterpMode
{
    AnmInterpMode_Linear = 0,
    AnmInterpMode_EaseIn = 1,
    AnmInterpMode_EaseInCubic = 2,
    AnmInterpMode_EaseInQuartic = 3,
    AnmInterpMode_EaseOut = 4,
    AnmInterpMode_EaseOutCubic = 5,
    AnmInterpMode_EaseOutQuartic = 6
};

enum AnmOpcode
{
    AnmOpcode_EndOfScript = -1,
    AnmOpcode_Nop = 0,
    AnmOpcode_Delete = 1,
    AnmOpcode_Static = 2,
    AnmOpcode_Sprite = 3,
    AnmOpcode_Jmp = 4,
    AnmOpcode_JmpDec = 5,
    AnmOpcode_Pos = 6,
    AnmOpcode_Scale = 7,
    AnmOpcode_Alpha = 8,
    AnmOpcode_Color = 9,
    AnmOpcode_FlipX = 10,
    AnmOpcode_FlipY = 11,
    AnmOpcode_Rotate = 12,
    AnmOpcode_AngularVelocity = 13,
    AnmOpcode_ScaleGrowth = 14,
    AnmOpcode_AlphaTimeLinear = 15,
    AnmOpcode_AdditiveBlendMode = 16,
    AnmOpcode_PosTimeLinear = 17,
    AnmOpcode_PosTimeDecel = 18,
    AnmOpcode_PosTimeDecel2 = 19,
    AnmOpcode_Stop = 20,
    AnmOpcode_InterruptLabel = 21,
    AnmOpcode_AnchorTopLeft = 22,
    AnmOpcode_StopHide = 23,
    AnmOpcode_PosMode = 24,
    AnmOpcode_Ins25 = 25,
    AnmOpcode_AddU = 26,
    AnmOpcode_AddV = 27,
    AnmOpcode_Visible = 28,
    AnmOpcode_ScaleTimeLinear = 29,
    AnmOpcode_ZWriteDisable = 30,
    AnmOpcode_Ins31 = 31,
    AnmOpcode_PosTime = 32,
    AnmOpcode_ColorTime = 33,
    AnmOpcode_AlphaTime = 34,
    AnmOpcode_RotateTime = 35,
    AnmOpcode_ScaleTime = 36,
    AnmOpcode_ISet = 37,
    AnmOpcode_FSet = 38,
    AnmOpcode_IAdd = 39,
    AnmOpcode_FAdd = 40,
    AnmOpcode_ISub = 41,
    AnmOpcode_FSub = 42,
    AnmOpcode_IMul = 43,
    AnmOpcode_FMul = 44,
    AnmOpcode_IDiv = 45,
    AnmOpcode_FDiv = 46,
    AnmOpcode_IMod = 47,
    AnmOpcode_FMod = 48,
    AnmOpcode_ISetAdd = 49,
    AnmOpcode_FSetAdd = 50,
    AnmOpcode_ISetSub = 51,
    AnmOpcode_FSetSub = 52,
    AnmOpcode_ISetMul = 53,
    AnmOpcode_FSetMul = 54,
    AnmOpcode_ISetDiv = 55,
    AnmOpcode_FSetDiv = 56,
    AnmOpcode_ISetMod = 57,
    AnmOpcode_FSetMod = 58,
    AnmOpcode_ISetRand = 59,
    AnmOpcode_FSetRand = 60,
    AnmOpcode_FSin = 61,
    AnmOpcode_FCos = 62,
    AnmOpcode_FTan = 63,
    AnmOpcode_FAcos = 64,
    AnmOpcode_FAtan = 65,
    AnmOpcode_NormalizeAngle = 66,
    AnmOpcode_IJmpEq = 67,
    AnmOpcode_FJmpEq = 68,
    AnmOpcode_IJmpNeq = 69,
    AnmOpcode_FJmpNeq = 70,
    AnmOpcode_IJmpLess = 71,
    AnmOpcode_FJmpLess = 72,
    AnmOpcode_IJmpLessOrEq = 73,
    AnmOpcode_FJmpLessOrEq = 74,
    AnmOpcode_IJmpGreater = 75,
    AnmOpcode_FJmpGreater = 76,
    AnmOpcode_IJmpGreaterOrEq = 77,
    AnmOpcode_FJmpGreaterOrEq = 78,
    AnmOpcode_Wait = 79,
    AnmOpcode_UScroll = 80,
    AnmOpcode_VScroll = 81,
    AnmOpcode_BlendMode = 82,
    AnmOpcode_Ins83 = 83,
    AnmOpcode_Color2 = 84,
    AnmOpcode_Alpha2 = 85,
    AnmOpcode_Color2Time = 86,
    AnmOpcode_Alpha2Time = 87,
    AnmOpcode_Ins88 = 88,
    AnmOpcode_ReturnFromInterrupt = 89
};

struct AnmEntry
{
    IDirect3DTexture8 *texture;
    u8 *rawData;
    i32 size;
};

C_ASSERT(sizeof(AnmEntry) == 0xc);

struct AnmRawEntry
{
    i32 numSprites;
    i32 numScripts;
    u32 textureIdx;
    i32 width;
    i32 height;
    u32 format;
    u32 colorKey;
    u32 nameOffset;
    u32 spriteIdxOffset;
    u32 mipmapNameOffset;
    u32 version;
    u32 priority;
    u32 textureOffset;
    u8 hasData;
    /* 3 bytes pad for alignment */
    u32 nextOffset;
    u32 unk2;
};

C_ASSERT(sizeof(AnmRawEntry) == 0x40);

struct AnmTextureHeader
{
    char magic[4]; /* THTX */
    u16 unk0x4;
    i16 format;
    i16 width;
    i16 height;
    u16 unk0x14;
    u16 unk0x18;
};

struct AnmLoadedSprite
{
    i32 anmIdx;
    IDirect3DTexture8 *texture;
    Float2 startPixelInclusive;
    Float2 endPixelInclusive;
    float height;
    float width;
    Float2 uvStart;
    Float2 uvEnd;
    float heightPx;
    float widthPx;
    Float2 scaleFactor;
    u32 unk0x40;
};

C_ASSERT(sizeof(AnmLoadedSprite) == 0x44);

#define ANM_MAX_ARGS 10

struct AnmRawInstr
{
    i16 opcode;
    u16 instructionSize;
    i16 time;
    u16 varMask;
    union {
        i32 intArgs[ANM_MAX_ARGS];
        f32 floatArgs[ANM_MAX_ARGS];
        u8 byteArgs[ANM_MAX_ARGS * sizeof(i32)];
    };
};

struct AnmVmBase
{
    void SetBlendModeAdditive();
    void SetBlendModeNormal();

    void Initialize();

    ZunBool IsVisible()
    {
        return this->visible;
    }

    void ClearVisible();

    void SetInterrupt(i16 interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    Float3 rotation;
    Float3 angleVel;
    Float2 scale;
    Float2 scaleGrowth;
    Float2 spriteSize;
    Float2 uvScrollPos;
    ZunTimer currentTimeInScript;
    ZunTimer waitTimer;
    ZunTimer interpCurrentTimers[AnmInterp_Last];
    ZunTimer interpEndTimers[AnmInterp_Last];
    u8 interpModes[AnmInterp_Last];
    i32 intVar0;
    i32 intVar1;
    i32 intVar2;
    i32 intVar3;
    f32 floatVar0;
    f32 floatVar1;
    f32 floatVar2;
    f32 floatVar3;
    i32 counterVar0;
    i32 counterVar1;
    Float2 uvScrollVel;
    D3DXMATRIX matrix1;
    D3DXMATRIX matrix2;
    D3DXMATRIX matrix3;
    ZunColor color1;
    ZunColor color2;
    union {
        u16 flags;
        struct
        {
            u32 visible : 1;
            u32 flag1 : 1;
            u32 updateRotation : 1;
            u32 updateScale : 1;
            u32 blendMode : 2;
            u32 flag6 : 1;
            u32 flag7 : 1;
            u32 usePosOffset : 1;
            u32 flip : 2;
            u32 anchor : 2;
            u32 zWriteDisabled : 1;
            u32 stopped : 1;
            u32 flag15 : 1;
            u32 flag16 : 1;
            u32 flag17 : 1;
            u32 flag18 : 1;
            u32 flag19 : 1;
        };
    };
    i16 type;
    i16 pendingInterrupt;
    i32 playerBulletHitAnimationType;
    AnmLoaded *anmFile;
};

C_ASSERT(sizeof(AnmVmBase) == 0x208);

struct AnmVm : AnmVmBase
{
    void SetZRotation(f32 z);
    void ClearVisible();

    Float3 pos;
    i16 activeSpriteIndex;
    i16 anmFileIndex;
    i16 baseSpriteIndex;
    i16 scriptIndex;
    AnmRawInstr *beginningOfScript;
    AnmRawInstr *currentInstruction;
    AnmLoadedSprite *loadedSprite;
    ZunTimer interruptReturnTime;
    AnmRawInstr *interruptReturnInstruction;

    Float3 posInitial;
    Float3 posFinal;
    Float3 rotateInitial;
    Float3 rotateFinal;
    Float2 scaleInitial;
    Float2 scaleFinal;
    ZunColor color1Initial;
    ZunColor color1Final;
    ZunColor color2Initial;
    ZunColor color2Final;

    Float3 pos2;
    i32 timeOfLastSpriteSet;
    u8 fontWidth;
    u8 fontHeight;
    unknown_fields(0x29a, 0xa);

    AnmVm()
    {
        memset(this, 0, sizeof(AnmVm));
        this->activeSpriteIndex = -1;
    }

    u32 FUN_004396f8();

    f32 GetFloatVar(f32 varId);
    i32 GetIntVar(i32 varId);
    f32 *GetFloatVarPtr(f32 *varPtr, u16 varMask, u32 variableNumber);
    i32 *GetIntVarPtr(i32 *varPtr, u16 varMask, u32 variableNumber);
};

C_ASSERT(sizeof(AnmVm) == 0x2a4);

struct AnmLoaded
{
    i32 anmIdx;
    AnmRawEntry *rawData;
    i32 totalEntries;
    AnmLoadedSprite *sprites;
    AnmRawInstr **scripts;
    AnmEntry *textures;
    int numberEntriesToBeLoaded;

    void LoadSprite(i32 spriteIdx, AnmLoadedSprite *loadedSprite);

    void ExecuteAnmIdx(AnmVm *vm, int scriptIdx)
    {
        vm->scriptIndex = scriptIdx;

        vm->pos = Float3(0, 0, 0);
        vm->pos2 = Float3(0, 0, 0);

        vm->fontHeight = 15;
        vm->fontWidth = 15;

        this->SetAndExecuteScript(vm, this->scripts[scriptIdx]);
    }

    AnmLoadedSprite *GetSprite(int sprite)
    {
        return &this->sprites[sprite];
    }

    void InitializeAndSetSprite(AnmVm *vm, i32 sprite)
    {
        vm->Initialize();
        vm->anmFile = this;
        this->SetSprite(vm, sprite);
    }

    void SetAndExecuteScriptIdx(AnmVm *vm, int scriptIdx)
    {
        vm->anmFile = this;
        vm->scriptIndex = scriptIdx;
        this->SetAndExecuteScript(vm, this->scripts[scriptIdx]);
    }

    void ExecuteAnmIdxArray(AnmVm *vm, i32 scriptIdx, i32 count);
    ZunResult SetSprite(AnmVm *vm, int spriteIdx);
    void SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginningOfScript);
};

C_ASSERT(sizeof(AnmLoaded) == 0x1c);

struct AnmRawSprite
{
    u32 id;
    float x;
    float y;
    float width;
    float height;
};

struct AnmRawScript
{
    u32 id;
    u32 offset;
};

struct AnmManager
{
    AnmManager();
    void SetupVertexBuffer();

    // FUNCTION: th08 0x43ef40 FOLDED
    ~AnmManager()
    {
    }
    ZunBool ExecuteScript(AnmVm *vm);
    void ExecuteScriptArray(AnmVm *sprites, int count);
    void SetRenderStateForVm(AnmVm *vm);
    ZunResult FUN_004639e0(AnmVm *vm);
    ZunResult FUN_00463d60(AnmVm *vm);
    ZunResult FUN_004640e0(AnmVm *vm, void *callback);
    ZunResult DrawInner(AnmVm *vm, i32 flags);
    ZunResult AddSpriteToDrawBuffer(VertexTex1DiffuseXyzrhw *vertices);
    ZunResult DrawNoRotation(AnmVm *vm);
    u8 FUN_00463470(AnmVm *vm);
    void TranslateRotation(VertexTex1DiffuseXyzrhw *vertex, float x, float y, float sine, float cosine, float xOffset,
                           float yOffset);
    ZunResult Draw2D(AnmVm *vm);
    void DrawPlayerBullet(AnmVm *vm);
    ZunResult FUN_00463cf0(AnmVm *vm);
    ZunResult FUN_00464070(AnmVm *vm);
    ZunResult FUN_004649a0(AnmVm *vm, void *state, i32 count);
    ZunResult DrawWithCallback(AnmVm *vm, void *callback);
    void Draw2DAndFlush(AnmVm *vm);
    ZunResult DrawNoRotationNoRound(AnmVm *vm);
    ZunResult DrawTriangleFan(AnmVm *vm, VertexDiffuseXyzrhw *vertices, i32 vertexCount);
    ZunResult CreateTextureFromFile(AnmEntry *entry, i32 format, i32 colorKey);
    ZunResult CreateTextureFromAnm(IDirect3DTexture8 **outTexture, AnmTextureHeader *textureData, i32 format);
    ZunResult CreateEmptyTexture(IDirect3DTexture8 **outTexture, i32 width, i32 height, i32 format);
    AnmLoaded *LoadAnm(i32 anmIdx, const char *filename);
    AnmLoaded *ReadAnmEntries(i32 anmIdx, const char *filename);
    AnmLoaded *GetAnm(i32 anmIdx);
    AnmLoaded *PreloadAnm(i32 anmIdx, const char *filename);
    i32 LoadExternalTextureData(AnmLoaded *anmLoaded, i32 entryNumber, i32 *sprites, i32 *scripts,
                                AnmRawEntry *rawEntry);
    AnmLoaded *PostloadAnmEntry(AnmLoaded *anm);
    BOOL LoadTextureData(AnmLoaded *anmLoaded, i32 entryNumber, i32 sprites, i32 scripts, AnmRawEntry *rawEntry);
    ZunResult ServicePreloadedAnims();
    void ReleaseAnm(i32 anmIdx);
    void ReleaseAnmEntry(AnmEntry *anmEntry);
    void SetCameraMode(i32 mode);

    void DrawTextInner(IDirect3DTexture8 *outTexture, i32 x, i32 y, i32 width, i32 height, i32 fontWidth,
                       i32 fontHeight, COLORREF textColor, COLORREF outlineColor, const char *buffer,
                       float scaleFactorX, float scaleFactorY);
    void DrawTextLeft(AnmVm *vm, COLORREF textColor, COLORREF shadowColor, const char *fmt, ...);
    void DrawTextCentered(AnmVm *vm, COLORREF textColor, COLORREF shadowColor, const char *fmt, ...);
    ZunResult LoadSurface(i32 surfaceIdx, const char *path);
    ZunResult PreloadSurface(i32 surfaceIdx, const char *path);
    void ReleaseSurface(i32 surfaceIdx);
    void CopySurfaceToBackbuffer(int surfaceIdx, int left, int top, int x, int y);
    void CopySurfaceToBackbuffer2(i32 surfaceIdx, i32 destX, i32 destY, i32 srcX, i32 srcY, i32 width, i32 height);

    void ReleaseVertexBuffer()
    {
        SAFE_RELEASE(this->quadVertexBuffer);
    }

    void ClearBlendMode()
    {
        this->currentBlendMode = 3;
    }

    void ClearColorOp()
    {
        this->currentColorOp = AnmColorOp_Unset;
    }

    void ClearSprite()
    {
        this->currentSprite = NULL;
    }

    void ClearVertexShader()
    {
        this->currentVertexShader = AnmVertexShader_Unset;
    }

    void ClearTexture()
    {
        this->currentTexture = NULL;
    }

    void ClearCameraSettings()
    {
        this->cameraMode = AnmCameraMode_Unset;
    }

    void ClearZWrite()
    {
        this->disableZWrite = AnmZWriteMode_Unset;
    }

    void ResetFrameDebugInfo()
    {
        this->scriptsExecutedThisFrame = 0;
        this->renderStateChangesThisFrame = 0;
        this->unk0xc = 0;
        this->flushesThisFrame = 0;
    }

    void SetInterruptArray(AnmVm *vm, int count, i16 interrupt);

    ZunBool SpriteHasTexture(AnmVm *vm)
    {
        if (vm->loadedSprite == NULL)
        {
            return FALSE;
        }

        if (vm->loadedSprite->anmIdx < 0)
        {
            return FALSE;
        }

        return this->anmFiles[vm->loadedSprite->anmIdx].textures != NULL;
    }

    void ReleaseSurfaces()
    {
        i32 i;

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->surfaces); i++)
        {
            if (this->surfaces[i] != NULL)
            {
                this->surfaces[i]->Release();
                this->surfaces[i] = NULL;
            }
        }
    }

    void TakeScreencaptures()
    {
        if (this->captureAnmIdx >= 0)
        {
            CaptureToTexture(this->captureAnmIdx, this->textureCaptureSrcX, this->textureCaptureSrcY,
                             this->textureCaptureSrcW, this->textureCaptureSrcH, this->textureCaptureDstX,
                             this->textureCaptureDstY, this->textureCaptureDstW, this->textureCaptureDstH);
            this->captureAnmIdx = -1;
        }

        if (this->captureSurfaceIdx >= 0)
        {
            CaptureToSurface(this->captureSurfaceIdx, this->surfaceCaptureSrcX, this->surfaceCaptureSrcY,
                             this->surfaceCaptureSrcW, this->surfaceCaptureSrcH, this->surfaceCaptureDstX,
                             this->surfaceCaptureDstY, this->surfaceCaptureDstW, this->surfaceCaptureDstH);
            this->captureSurfaceIdx = -1;
        }
    }

    void SetMixColorDefault()
    {
        this->useMixColor = FALSE;
        this->color.d3dColor = 0x80808080;
    }

    void SetMixColor(D3DCOLOR color)
    {
        this->useMixColor = TRUE;
        this->color.d3dColor = color;
    }

    void RequestCapture(i32 captureSurfaceIdx, i32 srcX, i32 srcY, i32 srcW, i32 srcH, i32 dstX, i32 dstY, i32 dstW,
                        i32 dstH)
    {
        if (this->captureSurfaceIdx >= 0)
        {
            return;
        }

        this->captureSurfaceIdx = captureSurfaceIdx;
        this->surfaceCaptureSrcX = srcX;
        this->surfaceCaptureSrcY = srcY;
        this->surfaceCaptureSrcW = srcW;
        this->surfaceCaptureSrcH = srcH;
        this->surfaceCaptureDstX = dstX;
        this->surfaceCaptureDstY = dstY;
        this->surfaceCaptureDstW = dstW;
        this->surfaceCaptureDstH = dstH;
    }

    void ReplaceSurface(i32 destIndex, i32 srcIndex)
    {
        if (this->surfaces[srcIndex] != NULL)
        {
            this->ReleaseSurface(destIndex);

            this->surfaces[destIndex] = this->surfaces[srcIndex];
            this->surfacesBis[destIndex] = this->surfacesBis[srcIndex];

            this->surfaces[srcIndex] = NULL;
            this->surfacesBis[srcIndex] = NULL;

            this->surfaceInfo[destIndex] = this->surfaceInfo[srcIndex];
        }
    }

    void CaptureToTexture(i32 captureAnmIdx, i32 srcX, i32 srcY, i32 srcW, i32 srcH, i32 dstX, i32 dstY, i32 dstW,
                          i32 dstH);
    void CaptureToSurface(i32 captureSurfaceIdx, i32 srcX, i32 srcY, i32 srcW, i32 srcH, i32 dstX, i32 dstY, i32 dstW,
                          i32 dstH);

    void ClearVertexBuffer();
    void FlushVertexBuffer();

    ZunColor color;
    ZunBool useMixColor;
    i32 captureSurfaceIdx;
    u32 unk0xc;
    u32 scriptsExecutedThisFrame;
    u32 renderStateChangesThisFrame;
    u32 flushesThisFrame;
    Float2 screenShakeOffset;
    AnmLoaded anmFiles[256];
    Float3 unk0x1c24;
    unknown_fields(0x1c30, 0x34);
    AnmVm unk0x1c64;
    unknown_fields(0x1f08, 0x130);

    IDirect3DSurface8 *surfaces[32];
    IDirect3DSurface8 *surfacesBis[32];
    u8 *surfaceData[32];
    u32 surfaceDataSizes[32];
    ZunImageInfo surfaceInfo[32];

    unknown_fields(0x24b8, 0x4);

    IDirect3DTexture8 *currentTexture;
    u8 currentBlendMode;
    u8 currentColorOp;
    u8 currentVertexShader;
    u8 disableZWrite;
    u8 cameraMode;
    unknown_fields(0x24c5, 3); // Padding?
    void *currentSprite;
    IDirect3DVertexBuffer8 *quadVertexBuffer;
    VertexDiffuseXyzrhw untexturedVector[4];
    u32 spritesToDraw;
    VertexTex1DiffuseXyzrhw vertexBuffer[0x18000];
    VertexTex1DiffuseXyzrhw *vertexBufferEndPtr;
    VertexTex1DiffuseXyzrhw *vertexBufferStartPtr;
    i32 captureAnmIdx;
    i32 textureCaptureSrcX;
    i32 textureCaptureSrcY;
    i32 textureCaptureSrcW;
    i32 textureCaptureSrcH;
    i32 textureCaptureDstX;
    i32 textureCaptureDstY;
    i32 textureCaptureDstW;
    i32 textureCaptureDstH;
    i32 surfaceCaptureSrcX;
    i32 surfaceCaptureSrcY;
    i32 surfaceCaptureSrcW;
    i32 surfaceCaptureSrcH;
    i32 surfaceCaptureDstX;
    i32 surfaceCaptureDstY;
    i32 surfaceCaptureDstW;
    i32 surfaceCaptureDstH;
};
C_ASSERT(sizeof(AnmManager) == 0x2a2570);

DIFFABLE_EXTERN(AnmManager *, g_AnmManager);

}; // namespace th08
