#include "th_pch.h"

#include "AnmManager.hpp"
#include "Background.hpp"
#include "Gui.hpp"
#include "ScreenEffect.hpp"
#include "EclManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"

namespace th08
{
ZunBool IsDisableResourceReload();
u8 MixColors(u8 color1, u8 color2);

struct RawStageHeader
{
    i16 nbObjects;
    i16 nbFaces;
    i32 facesOffset;
    i32 scriptOffset;
    i32 unkC;
    char stageName[128];
    char songNames[4][128];
    char songPaths[4][128];
};
C_ASSERT(sizeof(RawStageHeader) == 0x490);

struct RawStageQuadBasic
{
    i16 type;
    i16 byteSize;
    i16 anmScript;
    i16 vmIdx;
    D3DXVECTOR3 position;
    D3DXVECTOR2 size;
};
C_ASSERT(sizeof(RawStageQuadBasic) == 0x1c);

struct RawStageObject
{
    i16 id;
    i8 zLevel;
    i8 flags;
    D3DXVECTOR3 position;
    D3DXVECTOR3 size;
    RawStageQuadBasic firstQuad;
};
C_ASSERT(sizeof(RawStageObject) == 0x38);
DIFFABLE_STATIC(Background, g_Background);
DIFFABLE_STATIC(ChainElem, g_BackgroundCalcChain);
DIFFABLE_STATIC(ChainElem, g_BackgroundDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_BackgroundDrawChainLowPrio);

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageAnmFiles) = {
    "stg1bg.anm", "stg2bg.anm", "stg3bg.anm", "stg4abg.anm", "stg4abg.anm",
    "stg5bg.anm", "stg6bg.anm", "stg7bg.anm", "stg8bg.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageStdFiles) = {
    "stage1.std", "stage2.std", "stage3.std", "stage4a.std", "stage4b.std",
    "stage5.std", "stage6.std", "stage7.std", "stage8.std"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageStdFilesSpell) = {
    "stage1_s.std", "stage2_s.std", "stage3_s.std", "stage4a_s.std", "stage4b_s.std",
    "stage5_s.std", "stage6_s.std", "stage7_s.std", "stage8_s.std"};

// FUNCTION: th08 0x4071a0
Background::Background()
{
    memset(this, 0, sizeof(Background));
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[0]) = D3DXVECTOR3(0, 0, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[1]) = D3DXVECTOR3(0, 0, 0);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[2]) = D3DXVECTOR3(0, 1.0f, 0);
    this->unk6394.unk48 = 0.5235987901687622f;
    this->unk6264 = this->unk6394;
    this->unk62b0 = this->unk6394;
}

// STUB: th08 0x407400
ChainCallbackResult Background::OnUpdate(Background *background)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x409160
#pragma var_order(color2, this)
void Background::FUN_00409160(D3DCOLOR color)
{
    ZunColor color2;

    if (reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->a == 0)
    {
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->d3dColor = color;
    }
    else
    {
        color2.d3dColor = color;
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->r =
            ((u32)color2.r + reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->r) >> 1;
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->g =
            ((u32)color2.g + reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->g) >> 1;
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->b =
            ((u32)color2.b + reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->b) >> 1;
        reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->a =
            ((u32)color2.a + reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0x6468)->a) >> 1;
    }
}

// FUNCTION: th08 0x409200
#pragma var_order(i, viewport, effect, rect, fogColor, background)
ChainCallbackResult Background::OnDrawHighPrio(Background *background)
{
    i32 i;
    D3DVIEWPORT8 viewport;
    AnmVm *effect;
    ZunRect rect;
    ZunColor fogColor;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x6478) = 0;
    for (i = 0; i < 16; i++)
    {
        background->vectors6480[i] = Float3(0.0f, 0.0f, 0.0f);
    }

    g_Supervisor.viewport.X = 32;
    g_Supervisor.viewport.Y = 16;
    g_Supervisor.viewport.Width = 384;
    g_Supervisor.viewport.Height = 448;

    g_AnmManager->ClearVertexBuffer();
    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();
    g_AnmManager->ResetFrameDebugInfo();
    g_AnmManager->ClearCameraSettings();
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.DisableFog();
    }
    g_AnmManager->FlushVertexBuffer();

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB2C) != 0)
    {
        viewport.X = 32;
        viewport.Y = 16;
        viewport.Width = 384;
        viewport.Height = 448;
        g_Supervisor.d3dDevice->SetViewport(&viewport);
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, COLOR_BLACK, 1.0f, 0);
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB2C) = 0;
    }
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    if (reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + 0x6468)->a > 0)
    {
        g_AnmManager->SetMixColor(
            reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + 0x6468)->d3dColor);
    }
    reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + 0x6468)->a = 0;
    reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + 0x6468)->r = 0x80;
    reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + 0x6468)->g = 0x80;
    reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + 0x6468)->b = 0x80;

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) <= 1 && !g_Gui.IsDialogPresent())
    {
        if (background->stageVm0.activeSpriteIndex > 0)
        {
            g_AnmManager->Draw2DAndFlush(&background->stageVm0);
        }
        if (background->stageVm1.activeSpriteIndex > 0)
        {
            g_AnmManager->Draw2DAndFlush(&background->stageVm1);
        }
        if (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8) != NULL)
        {
            effect = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8);
            (*reinterpret_cast<void (__fastcall **)(AnmVm *)>(reinterpret_cast<u8 *>(effect) + 0x34C))(effect);
        }
    }

    if ((*reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0x830) & COLOR_ALPHA_MASK) ==
        COLOR_ALPHA_MASK)
    {
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                                      *reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0x830),
                                      1.0f, 0);
    }
    else if (*reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0x830) != 0)
    {
        rect.left = 32.0f;
        rect.top = 16.0f;
        rect.right = 416.0f;
        rect.bottom = 464.0f;
        ScreenEffect::DrawSquare(&rect,
                                 *reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0x830));
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER,
                                      *reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0x830),
                                      1.0f, 0);
    }
    else
    {
        g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER,
                                      *reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0x830),
                                      1.0f, 0);
    }

    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    if (!g_AnmManager->useMixColor)
    {
        g_Supervisor.SetRenderState(
            D3DRS_FOGCOLOR, *reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0xAF4));
    }
    else
    {
        fogColor.d3dColor = *reinterpret_cast<D3DCOLOR *>(reinterpret_cast<u8 *>(background) + 0xAF4);
        fogColor.r = MixColors(fogColor.r, g_AnmManager->color.r);
        fogColor.g = MixColors(fogColor.g, g_AnmManager->color.g);
        fogColor.b = MixColors(fogColor.b, g_AnmManager->color.b);
        g_Supervisor.SetRenderState(D3DRS_FOGCOLOR, fogColor.d3dColor);
    }
    g_Supervisor.SetRenderState(
        D3DRS_FOGSTART, *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(background) + 0xAEC));
    g_Supervisor.SetRenderState(
        D3DRS_FOGEND, *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(background) + 0xAF0));
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.EnableFog();
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) <= 1 && !g_Gui.IsDialogPresent())
    {
        background->RenderObjects(0);
        background->RenderObjects(1);
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}


// FUNCTION: th08 0x409640
#pragma var_order(zValue, alpha, rect, i, background)
ChainCallbackResult Background::OnDrawLowPrio(Background *background)
{
    ZunRect rect;
    i32 i;
    i32 alpha;
    f32 zValue;

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) <= 1 && !g_Gui.IsDialogPresent())
    {
        background->RenderObjects(2);
        background->RenderObjects(3);
        if (!g_Supervisor.IsFogDisabled())
        {
            g_Supervisor.DisableFog();
        }
        g_EffectManager.FUN_004281e0();
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) == 1)
        {
            rect.left = 32.0f;
            rect.top = 16.0f;
            rect.right = 416.0f;
            rect.bottom = 464.0f;
            alpha = (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB28) * 255) / 60;
            g_AnmManager->FlushVertexBuffer();
            g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
            if (!g_Supervisor.IsFogDisabled())
            {
                g_Supervisor.SetRenderState(D3DRS_FOGENABLE, FALSE);
            }
            ScreenEffect::DrawSquare(&rect, alpha << 24);
        }
    }

    g_AnmManager->FlushVertexBuffer();
    g_Supervisor.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.DisableFog();
    }

    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) >= 1)
    {
        for (i = 0; i < *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB30); i++)
        {
            g_AnmManager->Draw2DAndFlush(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(background) + 0xB38 + i * sizeof(AnmVm)));
        }
        if (background->onDrawLowPrioCallback != NULL)
        {
            background->onDrawLowPrioCallback();
        }
    }

    g_AnmManager->SetCameraMode(0);
    background->SetCamera1();
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    zValue = 1000.0f;
    g_Supervisor.SetRenderState(D3DRS_FOGSTART, *reinterpret_cast<u32 *>(&zValue));
    zValue = 2000.0f;
    g_Supervisor.SetRenderState(D3DRS_FOGEND, *reinterpret_cast<u32 *>(&zValue));
    if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x646C) == 0)
    {
        g_AnmManager->SetMixColorDefault();
    }
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x646C) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x647C) = 0;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x409850
#pragma var_order(i, vector0, vector1, vector2, vector3, background)
ZunResult Background::AddedCallback(Background *background)
{
    i32 i;

    background->timer80c = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0x818) = 0;
    background->vector824.x = 0.0f;
    background->vector824.y = 0.0f;
    background->vector824.z = 0.0f;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB24) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xB10) = 0;

    if (!IsDisableResourceReload())
    {
        background->stageAnmFile = g_AnmManager->PreloadAnm(4, g_StageAnmFiles[g_GameManager.currentStage]);
        if (background->stageAnmFile == NULL)
        {
            return ZUN_ERROR;
        }
    }
    else
    {
        background->stageAnmFile = g_AnmManager->GetAnm(4);
    }

    if (!g_GameManager.IsSpellPractice())
    {
        if (background->LoadStageData(g_StageStdFiles[g_GameManager.currentStage]) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
    }
    else
    {
        if (background->LoadStageData(g_StageStdFilesSpell[g_GameManager.currentStage]) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
    }

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + 0xAF4) = 0xFF000000;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(background) + 0xAEC) = 200.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(background) + 0xAF0) = 500.0f;

    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vectors[0]) = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vectors[1]) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vectors[5]) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vectors[2]) = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    background->unk6394.unk48 = 0.5235987901687622f;
    background->unk6264 = background->unk6394;
    background->unk62b0 = background->unk6394;

    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(background) + 0x6474) = 0;
    for (i = 0; i < 4; i++)
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + i * 4 + 0x63E0) = 0;
        background->timers63f4[i] = 0;
    }

    background->unk6260 = 0;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(background) + 0x6470) = 1322500.0f;
    if (g_GameManager.currentStage == 5)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(background) + 0x6470) = 1822500.0f;
    }
    else if (g_GameManager.currentStage == 6 || g_GameManager.currentStage == 7)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(background) + 0x6470) = 3240000.0f;
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409b20
#pragma var_order(stageAnmSecondary, background)
ZunResult Background::RegisterChain(i32 param)
{
    Background *background = &g_Background;
    void *stageAnmSecondary;

    if (IsDisableResourceReload())
    {
        stageAnmSecondary = background->stageAnmSecondary;
    }

    memset(background, 0, sizeof(Background));

    if (IsDisableResourceReload())
    {
        background->stageAnmSecondary = stageAnmSecondary;
    }

    *(i32 *)((u8 *)background + 0x81C) = 0;
    *(i32 *)((u8 *)background + 0x820) = param;

    g_BackgroundCalcChain.SetCallback((ChainCallback)Background::OnUpdate);
    g_BackgroundCalcChain.addedCallback = (ChainLifetimeCallback)Background::AddedCallback;
    g_BackgroundCalcChain.deletedCallback = (ChainLifetimeCallback)Background::DeletedCallback;
    g_BackgroundCalcChain.arg = background;
    if (g_Chain.AddToCalcChain(&g_BackgroundCalcChain, 8) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_BackgroundDrawChainHighPrio.SetCallback((ChainCallback)Background::OnDrawHighPrio);
    g_BackgroundDrawChainHighPrio.arg = background;
    g_Chain.AddToDrawChain(&g_BackgroundDrawChainHighPrio, 6);

    g_BackgroundDrawChainLowPrio.SetCallback((ChainCallback)Background::OnDrawLowPrio);
    g_BackgroundDrawChainLowPrio.arg = background;
    g_Chain.AddToDrawChain(&g_BackgroundDrawChainLowPrio, 7);

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x40b900
ZunBool IsDisableResourceReload()
{
    return g_Supervisor.unk16c;
}

// FUNCTION: th08 0x409c20
ZunResult Background::DeletedCallback(Background *background)
{
    if (!IsDisableResourceReload())
    {
        g_AnmManager->ReleaseAnm(4);
    }
    if (background->stageAnm != NULL)
    {
        g_ZunMemory.Free(background->stageAnm);
        background->stageAnm = NULL;
    }
    if (!IsDisableResourceReload() && background->stageAnmSecondary != NULL)
    {
        g_ZunMemory.Free(background->stageAnmSecondary);
        background->stageAnmSecondary = NULL;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409ca0
void Background::CutChain()
{
    g_Chain.Cut(&g_BackgroundCalcChain);
    g_Chain.Cut(&g_BackgroundDrawChainHighPrio);
    g_Chain.Cut(&g_BackgroundDrawChainLowPrio);
}

// STUB: th08 0x40a1b0
void Background::RenderObjects(i32 mode)
{
}

// FUNCTION: th08 0x409ce0
#pragma var_order(vmIdx, i, curObj, curQuad, this)
ZunResult Background::LoadStageData(const char *path)
{
    RawStageObject *curObj;
    RawStageQuadBasic *curQuad;
    i32 i;
    i32 vmIdx;

    if (!IsDisableResourceReload())
    {
        this->stageAnmSecondary = FileSystem::OpenFile(path, NULL, 0);
        if (this->stageAnmSecondary == NULL)
        {
            g_GameErrorContext.Log("ステージデータが見つかりません。データが壊れています\r\n");
            return ZUN_ERROR;
        }
    }

    this->stageObjectCount = ((RawStageHeader *)this->stageAnmSecondary)->nbObjects;
    this->stageVmCount = ((RawStageHeader *)this->stageAnmSecondary)->nbFaces;
    this->stageUnknown804 =
        (void *)(((RawStageHeader *)this->stageAnmSecondary)->facesOffset + (i32)this->stageAnmSecondary);
    this->stageUnknown808 =
        (void *)(((RawStageHeader *)this->stageAnmSecondary)->scriptOffset + (i32)this->stageAnmSecondary);
    this->stageOffsets = (u8 *)this->stageAnmSecondary + sizeof(RawStageHeader);

    if (!IsDisableResourceReload())
    {
        for (i = 0; i < this->stageObjectCount; i++)
        {
            ((RawStageObject **)this->stageOffsets)[i] =
                (RawStageObject *)((i32)((RawStageObject **)this->stageOffsets)[i] +
                                   (i32)this->stageAnmSecondary);
        }
    }

    this->stageAnm = g_ZunMemory.Alloc(this->stageVmCount * sizeof(AnmVm), "bgscroll");
    for (i = 0, vmIdx = 0; i < this->stageObjectCount; i++)
    {
        curObj = ((RawStageObject **)this->stageOffsets)[i];
        curObj->flags = 1;
        curQuad = &curObj->firstQuad;
        while (curQuad->type >= 0)
        {
            this->stageAnmFile->ExecuteAnmIdx(&((AnmVm *)this->stageAnm)[vmIdx], curQuad->anmScript);
            curQuad->vmIdx = vmIdx++;
            curQuad = (RawStageQuadBasic *)((u8 *)curQuad + curQuad->byteSize);
        }
    }

    switch (g_GameManager.currentStage)
    {
    case 2:
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->textAnmVm, 33);
        break;
    default:
        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->textAnmVm, 33);
        break;
    }
    this->textAnmVm.SetInterrupt(2);
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x834) = 0;
    this->timer838 = 0;
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x409f40
#pragma var_order(unusedQuad, activeVms, i, vm, curObj, curQuad, this)
u32 Background::FUN_00409f40()
{
    RawStageQuadBasic *curQuad;
    RawStageObject *curObj;
    AnmVm *vm;
    i32 i;
    i32 activeVms;
    RawStageQuadBasic *unusedQuad;

    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x834) != 0)
    {
        if (g_Player.IsHuman())
        {
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x834) = 0;
            this->timer838 = 0;
            this->textAnmVm.SetInterrupt(2);
        }
    }
    else if (g_Player.IsYoukai())
    {
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x834) = 1;
        this->timer838 = 0;
        this->textAnmVm.SetInterrupt(1);
    }

    this->timer838++;
    g_AnmManager->ExecuteScript(&this->textAnmVm);

    for (i = 0; i < this->stageObjectCount; i++)
    {
        curObj = ((RawStageObject **)this->stageOffsets)[i];
        if ((curObj->flags & 1) != 0)
        {
            activeVms = 0;
            curQuad = &curObj->firstQuad;
            while (curQuad->type >= 0)
            {
                vm = &((AnmVm *)this->stageAnm)[curQuad->vmIdx];
                switch (curQuad->type)
                {
                case 0:
                    g_AnmManager->ExecuteScript(vm);
                    break;
                case 1:
                    unusedQuad = curQuad;
                    g_AnmManager->ExecuteScript(vm);
                    break;
                }

                if (vm->currentInstruction != NULL)
                {
                    activeVms++;
                }
                curQuad = (RawStageQuadBasic *)((u8 *)curQuad + curQuad->byteSize);
            }

            if (vm->type == 1)
            {
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(vm) + 0x1F8) |= 0x20000;
                vm->color2.r = ((u32)vm->color1.r * reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0xA34)->r) >> 8;
                vm->color2.g = ((u32)vm->color1.g * reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0xA34)->g) >> 8;
                vm->color2.b = ((u32)vm->color1.b * reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0xA34)->b) >> 8;
                vm->color2.a = ((u32)vm->color1.a * reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(this) + 0xA34)->a) >> 8;
            }

            if (activeVms == 0)
            {
                curObj->flags &= ~1;
            }
        }
    }
    return 0;
}

// FUNCTION: th08 0x40b5a0
#pragma var_order(cameraDistance, viewportMiddleHeight, viewportMiddleWidth, aspectRatio, fov, this)
void Background::SetCamera1()
{
    f32 fov;
    f32 aspectRatio;
    f32 viewportMiddleWidth;
    f32 viewportMiddleHeight;
    f32 cameraDistance;

    viewportMiddleWidth = (f32)g_Supervisor.viewport.Width / 2.0f;
    viewportMiddleHeight = (f32)g_Supervisor.viewport.Height / 2.0f;
    aspectRatio = (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height;
    fov = ZUN_PI / 10.0f;
    cameraDistance = viewportMiddleHeight / (f32)tan(fov / 2.0f);

    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix,
                       &D3DXVECTOR3(viewportMiddleWidth, viewportMiddleHeight, cameraDistance),
                       &D3DXVECTOR3(viewportMiddleWidth, viewportMiddleHeight, 0.0f),
                       &D3DXVECTOR3(0.0f, -1.0f, 0.0f));
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 1.0f, 10000.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
}

// FUNCTION: th08 0x40b6d0
#pragma var_order(eyeVec, atVec, this)
void Background::SetCamera2()
{
    Float3 atVec = this->unk6394.vectors[1] + this->unk6394.vectors[0];
    Float3 eyeVec = this->unk6394.vectors[5] + this->unk6394.vectors[0];
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix, reinterpret_cast<D3DXVECTOR3 *>(&eyeVec),
                       reinterpret_cast<D3DXVECTOR3 *>(&atVec),
                       reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[2]));
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, this->unk6394.unk48,
                               (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height, 30.0f, 1800.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
    D3DXVec3Cross(reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[4]),
                  reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[1]),
                  reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[2]));
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[4]),
                      reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vectors[4]));
}

}; // Namespace th08
