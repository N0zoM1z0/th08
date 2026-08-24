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
f32 __stdcall FUN_00408fc0(f32 value0, f32 value1, f32 value2, f32 value3, f32 time);
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

struct RawStageObjectInstance
{
    i16 id;
    i16 unk2;
    Float3 position;
};
C_ASSERT(sizeof(RawStageObjectInstance) == 0x10);

struct RawStageInstr
{
    i32 frame;
    i16 opcode;
    i16 size;
    i32 args[3];
};
C_ASSERT(sizeof(RawStageInstr) == 0x14);

struct BackgroundAnmVmSnapshot
{
    u32 words[0xA9];
};
C_ASSERT(sizeof(BackgroundAnmVmSnapshot) == sizeof(AnmVm));

struct RawStageQuadType1
{
    i16 type;
    i16 byteSize;
    i16 anmScript;
    i16 vmIdx;
    Float3 position1;
    Float3 position2;
    f32 width;
};
C_ASSERT(sizeof(RawStageQuadType1) == 0x24);

struct BackgroundStageVertex
{
    BackgroundStageVertex()
    {
    }

    Float3 pos;
    f32 w;
    ZunColor diffuse;
    Float2 textureUV;
};
C_ASSERT(sizeof(BackgroundStageVertex) == 0x1c);
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
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageEnemyAnms) = {
    "stg1enm.anm", "stg2enm.anm", "stg3enm.anm", "stg4aenm.anm", "stg4benm.anm",
    "stg5enm.anm", "stg6enm.anm", "stg7enm.anm", "stg8enm.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 17, g_SpellEnemyAnms) = {
    "stg1enm.anm", "stg2enm.anm", "stg3enm.anm", "stg5enm.anm", "stg6enm.anm",
    "stg7enm.anm", "stg8enm.anm", "stg5enm.anm", "stg8enm.anm", "stg4aenm.anm",
    "stg4benm.anm", "stgenm_sk.anm", "stgenm_ym.anm", "stgenm_al.anm", "stgenm_rm.anm",
    "stgenm_yy.anm", "stgenm_yk.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageEclFiles) = {
    "ecldata1.ecl", "ecldata2.ecl", "ecldata3.ecl", "ecldata4a.ecl", "ecldata4b.ecl",
    "ecldata5.ecl", "ecldata6.ecl", "ecldata7.ecl", "ecldata8.ecl"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_StageSpellEclFiles) = {
    "ecldata1sp.ecl", "ecldata2sp.ecl", "ecldata3sp.ecl", "ecldata4asp.ecl", "ecldata4bsp.ecl",
    "ecldata5sp.ecl", "ecldata6sp.ecl", "ecldata7sp.ecl", "ecldata8sp.ecl"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 17, g_SpellEclFiles) = {
    "ecldata1sp.ecl", "ecldata2sp.ecl", "ecldata3sp.ecl", "ecldata5sp.ecl", "ecldata6sp.ecl",
    "ecldata7sp.ecl", "ecldata8sp.ecl", "ecldata5sp.ecl", "ecldata8sp.ecl", "ecldata4asp.ecl",
    "ecldata4bsp.ecl", "ecldata_sk.ecl", "ecldata_ym.ecl", "ecldata_al.ecl", "ecldata_rm.ecl",
    "ecldata_yy.ecl", "ecldata_yk.ecl"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 9, g_GuiStageTextAnmPaths) = {
    "stg1txt.anm", "stg2txt.anm", "stg3txt.anm", "stg4atxt.anm", "stg4btxt.anm", "stg5txt.anm",
    "stg6txt.anm", "stg7txt.anm", "stg8txt.anm"};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 15, g_EffectAnms) = {
    "eff01.anm", "eff02.anm", "eff03.anm", "eff04a.anm", "eff04b.anm",
    "eff05.anm", "eff06.anm", "eff07.anm", "eff08.anm", "eff09sk.anm",
    "eff09ym.anm", "eff09al.anm", "eff09rm.anm", "eff09yy.anm", "eff09yk.anm"};

// FUNCTION: th08 0x4071a0
Background::Background()
{
    memset(this, 0, sizeof(Background));
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector0) = D3DXVECTOR3(0, 0, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector1) = D3DXVECTOR3(0, 0, 0);
    *reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector2) = D3DXVECTOR3(0, 1.0f, 0);
    this->unk6394.unk48 = 0.5235987901687622f;
    this->unk6264 = this->unk6394;
    this->unk62b0 = this->unk6394;
}

// FUNCTION: th08 0x4073b0
BackgroundUnkVectors::BackgroundUnkVectors()
{
}

// FUNCTION: th08 0x407400
#pragma var_order(curInsn, pos, spawnedStageEffect)
ChainCallbackResult Background::OnUpdate(Background *background)
{
    RawStageInstr *curInsn;
    D3DXVECTOR3 pos;
    AnmVm *spawnedStageEffect;

#define BG_I32(off) (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + (off)))
#define BG_U8(off) (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(background) + (off)))
#define BG_COLOR(off) (*reinterpret_cast<ZunColor *>(reinterpret_cast<u8 *>(background) + (off)))

    if (background->stageAnmSecondary == NULL)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_GameManager.flags.unk10)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    if (*reinterpret_cast<i32 *>(0x164D2CC) == 7)
    {
        if (*reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8) == NULL)
        {
            Float3 zeroVector(0.0f, 0.0f, 0.0f);
            *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8) =
                g_EffectManager.FUN_00425870(0x40, reinterpret_cast<D3DXVECTOR3 *>(&zeroVector), 0xC, 1, -1);
            spawnedStageEffect = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8);
            background->stageAnmFile->SetAndExecuteScriptIdx(spawnedStageEffect, 11);
        }
        else if (background->unk6260 == 1)
        {
            AnmVm *stageEffect1 = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8);
            background->stageAnmFile->SetAndExecuteScriptIdx(stageEffect1, 11);
        }
        else if (background->unk6260 == 2)
        {
            AnmVm *stageEffect2 = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8);
            BackgroundAnmVmSnapshot savedStageVm2 = *reinterpret_cast<BackgroundAnmVmSnapshot *>(stageEffect2);
            background->stageAnmFile->SetAndExecuteScriptIdx(stageEffect2, 12);
            stageEffect2->SetInterrupt(2);
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(stageEffect2) + 0x244) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(&savedStageVm2) + 0x244);
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(stageEffect2) + 0x238) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(&savedStageVm2) + 0x238);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(stageEffect2) + 0x50) =
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(&savedStageVm2) + 0x50);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(stageEffect2) + 0xA4) =
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(&savedStageVm2) + 0xA4);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(stageEffect2) + 0xF8) =
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&savedStageVm2) + 0xF8);
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(stageEffect2) + 0x1F0) =
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&savedStageVm2) + 0x1F0);
        }
        else if (background->unk6260 == 3)
        {
            AnmVm *stageEffect3 = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8);
            BackgroundAnmVmSnapshot savedStageVm3 = *reinterpret_cast<BackgroundAnmVmSnapshot *>(stageEffect3);
            stageEffect3->SetInterrupt(3);
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(stageEffect3) + 0x244) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(&savedStageVm3) + 0x244);
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(stageEffect3) + 0x238) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(&savedStageVm3) + 0x238);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(stageEffect3) + 0x50) =
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(&savedStageVm3) + 0x50);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(stageEffect3) + 0xA4) =
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(&savedStageVm3) + 0xA4);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(stageEffect3) + 0xF8) =
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&savedStageVm3) + 0xF8);
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(stageEffect3) + 0x1F0) =
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&savedStageVm3) + 0x1F0);
        }
        else if (background->unk6260 == 4)
        {
            AnmVm *stageEffect4 = *reinterpret_cast<AnmVm **>(reinterpret_cast<u8 *>(background) + 0xAE8);
            BackgroundAnmVmSnapshot savedStageVm4 = *reinterpret_cast<BackgroundAnmVmSnapshot *>(stageEffect4);
            stageEffect4->SetInterrupt(4);
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(stageEffect4) + 0x244) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(&savedStageVm4) + 0x244);
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(stageEffect4) + 0x238) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(&savedStageVm4) + 0x238);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(stageEffect4) + 0x50) =
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(&savedStageVm4) + 0x50);
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(stageEffect4) + 0xA4) =
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(&savedStageVm4) + 0xA4);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(stageEffect4) + 0xF8) =
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&savedStageVm4) + 0xF8);
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(stageEffect4) + 0x1F0) =
                *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&savedStageVm4) + 0x1F0);
        }
    }

    if (background->unk6260 != 0)
    {
        i32 seekIndex = 0;
        curInsn = reinterpret_cast<RawStageInstr *>(background->stageUnknown808);
        BG_I32(0x818) = 0;
        while ((curInsn->opcode != 0x1F || background->unk6260 != curInsn->args[0]) && curInsn->frame != -1)
        {
            curInsn++;
            seekIndex++;
        }
        if (curInsn->frame != -1)
        {
            BG_I32(0x818) = seekIndex + 1;
            background->timer80c = curInsn->frame;
            background->unk6260 = 0;
        }
    }

read_instruction:
    curInsn = reinterpret_cast<RawStageInstr *>(background->stageUnknown808) + BG_I32(0x818);
    if (background->timer80c >= curInsn->frame)
    {
        if (curInsn->frame != -1)
        {
    switch (curInsn->opcode)
    {
    case 0:
        if (curInsn->frame == -1)
        {
            background->vector6454 = *reinterpret_cast<Float3 *>(curInsn->args);
            background->vector824.x = background->vector6454.x;
            background->vector824.y = background->vector6454.y;
            background->vector824.z = background->vector6454.z;
        }
        else
        {
            pos = *reinterpret_cast<D3DXVECTOR3 *>(curInsn->args);
            background->vector824.x = pos.x;
            background->vector824.y = pos.y;
            background->vector824.z = pos.z;
            background->vector6454 = *reinterpret_cast<Float3 *>(&pos);
            BG_I32(0x6460) = curInsn->frame;
            curInsn++;
            BG_I32(0x6450) = curInsn->frame;
            background->vector6444 = *reinterpret_cast<Float3 *>(curInsn->args);
        }
        break;
    case 1:
        BG_I32(0xAF4) = curInsn->args[0];
        BG_I32(0xAEC) = curInsn->args[1];
        BG_I32(0xAF0) = curInsn->args[2];
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(background) + 0xB04) =
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(background) + 0xAEC);
        break;
    case 2:
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(background) + 0xAF8) =
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(background) + 0xAEC);
        BG_I32(0xB10) = curInsn->args[0];
        background->timerB14 = 0;
        break;
    case 5:
        if (BG_U8(0x6464))
        {
            Float3 cameraDelta = *reinterpret_cast<Float3 *>(curInsn->args) - background->unk6264.vector0;
            FUN_00426d10(&cameraDelta);
            BG_U8(0x6464) = 0;
        }
        background->unk62b0.vector0 = background->unk6264.vector0;
        background->unk6264.vector0 = *reinterpret_cast<Float3 *>(curInsn->args);
        if (background->interpolationDuration[0] == 0)
            background->unk6394.vector0 = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 6:
        background->interpolationDuration[0] = curInsn->args[0];
        background->interpolationTimers[0] = 0;
        background->interpolationMode[0] = curInsn->args[1];
        break;
    case 7:
        background->unk62b0.vector1 = background->unk6264.vector1;
        background->unk6264.vector1 = *reinterpret_cast<Float3 *>(curInsn->args);
        if (background->interpolationDuration[1] == 0)
            background->unk6394.vector1 = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 8:
        background->interpolationDuration[1] = curInsn->args[0];
        background->interpolationTimers[1] = 0;
        background->interpolationMode[1] = curInsn->args[1];
        break;
    case 9:
        background->unk62b0.vector2 = background->unk6264.vector2;
        background->unk6264.vector2 = *reinterpret_cast<Float3 *>(curInsn->args);
        if (background->interpolationDuration[2] == 0)
            background->unk6394.vector2 = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 10:
        background->interpolationDuration[2] = curInsn->args[0];
        background->interpolationMode[2] = curInsn->args[1];
        background->interpolationTimers[2] = 0;
        break;
    case 11:
        background->unk62b0.unk48 = background->unk6264.unk48;
        background->unk6264.unk48 = *reinterpret_cast<f32 *>(&curInsn->args[0]);
        if (background->interpolationDuration[3] == 0)
            background->unk6394.unk48 = *reinterpret_cast<f32 *>(&curInsn->args[0]);
        break;
    case 12:
        background->interpolationDuration[3] = curInsn->args[0];
        background->interpolationTimers[3] = 0;
        background->interpolationMode[3] = curInsn->args[1];
        break;
    case 13:
        BG_I32(0x830) = curInsn->args[0];
        break;
    case 3:
        if (background->unk6260 != 0)
        {
            background->unk6260 = 0;
            break;
        }
        goto instructions_done;
    case 4:
        BG_I32(0x818) = curInsn->args[0];
        background->timer80c = curInsn->args[1];
        background->interpolationDuration[0] = 0;
        BG_U8(0x6464) = 1;
        goto read_instruction;
    case 14: background->unk62b0.vector0 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 15: background->unk6264.vector0 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 16: background->unk6348.vector0 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 17: background->unk62fc.vector0 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 18:
        background->interpolationDuration[0] = curInsn->args[0];
        background->interpolationTimers[0] = 0;
        background->interpolationMode[0] = 7;
        break;
    case 19: background->unk62b0.vector1 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 20: background->unk6264.vector1 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 21: background->unk6348.vector1 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 22: background->unk62fc.vector1 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 23:
        background->interpolationDuration[1] = curInsn->args[0];
        background->interpolationTimers[1] = 0;
        background->interpolationMode[1] = 7;
        break;
    case 24: background->unk62b0.vector2 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 25: background->unk6264.vector2 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 26: background->unk6348.vector2 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 27: background->unk62fc.vector2 = *reinterpret_cast<Float3 *>(curInsn->args); break;
    case 28:
        background->interpolationDuration[2] = curInsn->args[0];
        background->interpolationTimers[2] = 0;
        background->interpolationMode[2] = 7;
        break;
    case 29:
        if (curInsn->args[0] >= 0) background->stageAnmFile->ExecuteAnmIdx(&background->stageVm0, curInsn->args[0]);
        else background->stageVm0.activeSpriteIndex = -1;
        break;
    case 30:
        if (curInsn->args[0] >= 0) background->stageAnmFile->ExecuteAnmIdx(&background->stageVm1, curInsn->args[0]);
        else background->stageVm0.activeSpriteIndex = -1;
        break;
    case 33:
        BG_U8(0x6474) = *reinterpret_cast<u8 *>(&curInsn->args[0]);
        background->interpolationDuration[4] = 0;
        background->interpolationTimers[4] = 0;
        background->interpolationMode[4] = 0;
        break;
    case 32:
        background->unk6394.vector5 = *reinterpret_cast<Float3 *>(curInsn->args);
        break;
    case 34:
        if (curInsn->args[0] >= 0) background->stageAnmFile->ExecuteAnmIdx(&background->stageVm2, curInsn->args[0]);
        else background->stageVm2.activeSpriteIndex = -1;
        break;
    case 31:
        break;
    default:
        break;
    }

    BG_I32(0x818)++;
    goto read_instruction;
        }
    }

instructions_done:
    {
#pragma var_order(interpolationIndex, interpolationDelta, interpolationTime, angle1, angle2, angle3, fogInterpRatio, i, j, spawnedEffect, k)
    i32 interpolationIndex;
    f32 interpolationDelta;
    f32 interpolationTime;
    f32 angle1;
    f32 angle2;
    f32 angle3;
    f32 fogInterpRatio;
    i32 i;
    i32 j;
    AnmVm *spawnedEffect;
    i32 k;

    interpolationIndex = 0;
    if (background->interpolationDuration[interpolationIndex] != 0)
        background->FUN_00408d60(interpolationIndex, &background->unk6394.vector0, &background->unk62b0.vector0,
                                 &background->unk6264.vector0, &background->unk6348.vector0, &background->unk62fc.vector0);
    interpolationIndex = 1;
    if (background->interpolationDuration[interpolationIndex] != 0)
        background->FUN_00408d60(interpolationIndex, &background->unk6394.vector1, &background->unk62b0.vector1,
                                 &background->unk6264.vector1, &background->unk6348.vector1, &background->unk62fc.vector1);
    interpolationIndex = 2;
    if (background->interpolationDuration[interpolationIndex] != 0)
        background->FUN_00408d60(interpolationIndex, &background->unk6394.vector2, &background->unk62b0.vector2,
                                 &background->unk6264.vector2, &background->unk6348.vector2, &background->unk62fc.vector2);
    interpolationIndex = 3;
    if (background->interpolationDuration[interpolationIndex] != 0)
    {
        if (background->interpolationTimers[interpolationIndex] < background->interpolationDuration[interpolationIndex])
        {
            background->interpolationTimers[interpolationIndex]++;
            interpolationTime = (f32)background->interpolationTimers[interpolationIndex] /
                                background->interpolationDuration[interpolationIndex];
        }
        else
        {
            background->interpolationTimers[interpolationIndex] = background->interpolationDuration[interpolationIndex];
            interpolationTime = 1.0f;
            background->interpolationDuration[interpolationIndex] = 0;
        }
        switch (background->interpolationMode[interpolationIndex])
        {
        case 1: interpolationTime = 1.0f - interpolationTime; interpolationTime = 1.0f - interpolationTime * interpolationTime; break;
        case 2: interpolationTime = 1.0f - interpolationTime; interpolationTime = 1.0f - interpolationTime * interpolationTime * interpolationTime; break;
        case 3: interpolationTime = 1.0f - interpolationTime; interpolationTime = 1.0f - interpolationTime * interpolationTime * interpolationTime * interpolationTime; break;
        case 4: interpolationTime = interpolationTime * interpolationTime; break;
        case 5: interpolationTime = interpolationTime * interpolationTime * interpolationTime; break;
        case 6: interpolationTime = interpolationTime * interpolationTime * interpolationTime * interpolationTime; break;
        }
        interpolationDelta = background->unk6264.unk48 - background->unk62b0.unk48;
        background->unk6394.unk48 = interpolationDelta * interpolationTime + background->unk62b0.unk48;
    }

    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vector3),
                      reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vector1));

    if (BG_U8(0x6474) != 0)
    {
        switch (BG_U8(0x6474))
        {
        case 1:
        {
            angle1 = (f32)background->interpolationTimers[4] * ZUN_PI * 2.0f / 480.0f - ZUN_PI;
            background->unk6394.vector5.x = sinf(angle1) * 40.0f;
            background->interpolationTimers[4]++;
            if (background->interpolationTimers[4] >= 480) background->interpolationTimers[4] = 0;
            break;
        }
        case 2:
        {
            angle2 = (f32)background->interpolationTimers[4] * ZUN_PI * 2.0f / 480.0f - ZUN_PI;
            background->unk6394.vector5.x = sinf(angle2) * 70.0f;
            background->unk6394.vector2.x = -sinf(angle2) * 0.1f;
            background->interpolationTimers[4]++;
            if (background->interpolationTimers[4] >= 480) background->interpolationTimers[4] = 0;
            break;
        }
        case 3:
        {
            angle3 = (f32)background->interpolationTimers[4] * ZUN_PI * 2.0f / 4800.0f - ZUN_PI;
            background->unk6394.vector2.x = sinf(angle3) * 1.0f;
            background->unk6394.vector2.z = cosf(angle3) * 1.0f;
            background->interpolationTimers[4]++;
            if (background->interpolationTimers[4] >= 4800) background->interpolationTimers[4] = 0;
            break;
        }
        }
    }

    if (BG_I32(0xB10) != 0)
    {
        background->timerB14++;
        fogInterpRatio = (f32)background->timerB14 / BG_I32(0xB10);
        if (fogInterpRatio >= 1.0f) fogInterpRatio = 1.0f;
        for (i = 0; i < 4; i++)
        {
            reinterpret_cast<u8 *>(&BG_COLOR(0xAF4))[i] =
                (u8)(((f32)reinterpret_cast<u8 *>(&BG_COLOR(0xB0C))[i] -
                      (f32)reinterpret_cast<u8 *>(&BG_COLOR(0xB00))[i]) * fogInterpRatio +
                     (f32)reinterpret_cast<u8 *>(&BG_COLOR(0xB00))[i]);
        }
        background->skyFog.nearPlane =
            (background->skyFogInterpFinal.nearPlane - background->skyFogInterpInitial.nearPlane) *
                fogInterpRatio +
            background->skyFogInterpInitial.nearPlane;
        background->skyFog.farPlane =
            (background->skyFogInterpFinal.farPlane - background->skyFogInterpInitial.farPlane) * fogInterpRatio +
            background->skyFogInterpInitial.farPlane;
        if (background->timerB14 >= background->skyFogInterpDuration) background->skyFogInterpDuration = 0;
    }

    if (curInsn->opcode != 3)
        background->timer80c++;
    background->FUN_00409f40();

    if (BG_I32(0xB24) >= 1)
    {
        if (BG_I32(0xB28) == 60) BG_I32(0xB24)++;
        BG_I32(0xB28)++;
        for (j = 0; j < BG_I32(0xB30); j++)
            g_AnmManager->ExecuteScript(&background->anmVmArray[j]);
    }
    if (background->stageVm0.activeSpriteIndex > 0) g_AnmManager->ExecuteScript(&background->stageVm0);
    if (background->stageVm1.activeSpriteIndex > 0) g_AnmManager->ExecuteScript(&background->stageVm1);
    if (background->stageVm2.activeSpriteIndex > 0)
    {
        g_AnmManager->ExecuteScript(&background->stageVm2);
        BG_I32(0x830) = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&background->stageVm2) + 0x1F0);
    }

    if (BG_I32(0x81C) % 3 == 0 && (BG_I32(0x81C) >= 700 || g_GameManager.IsSpellPractice()) && BG_I32(0xB24) < 2)
    {
        for (k = 0; k < 12; k++)
        {
            spawnedEffect = g_EffectManager.SpawnEffect(62, reinterpret_cast<D3DXVECTOR3 *>(&background->vectors6480[k]),
                                                        1, 0x20FFFFFF);
            *reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(spawnedEffect) + 0x354) = 4;
        }
    }
    BG_I32(0x647C) = 1;
    if (BG_I32(0xB24) >= 2) BG_I32(0x6478) = 0;

    BG_I32(0x81C)++;
    if (BG_I32(0x81C) % 500 == 250 && g_GameManager.IsTampered())
    {
        return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

#undef BG_I32
#undef BG_U8
#undef BG_COLOR
}

// FUNCTION: th08 0x408d60
void __fastcall Background::FUN_00408d60(i32 index, Float3 *out, const Float3 *start, const Float3 *end,
                                         const Float3 *control2, const Float3 *control3)
{
    f32 time;

    if (this->interpolationTimers[index] < this->interpolationDuration[index])
    {
        this->interpolationTimers[index]++;
        time = (f32)this->interpolationTimers[index] / this->interpolationDuration[index];
    }
    else
    {
        this->interpolationTimers[index] = this->interpolationDuration[index];
        time = 1.0f;
        this->interpolationDuration[index] = 0;
    }

    switch (this->interpolationMode[index])
    {
    case 1:
        time = 1.0f - time;
        time = 1.0f - time * time;
        break;
    case 2:
        time = 1.0f - time;
        time = 1.0f - time * time * time;
        break;
    case 3:
        time = 1.0f - time;
        time = 1.0f - time * time * time * time;
        break;
    case 4:
        time = time * time;
        break;
    case 5:
        time = time * time * time;
        break;
    case 6:
        time = time * time * time * time;
        break;
    }

    if (this->interpolationMode[index] != 7)
    {
        *out = *end - *start;
        *out = (*out * time) + *start;
    }
    else
    {
        out->x = FUN_00408fc0(start->x, end->x, control2->x, control3->x, time);
        out->y = FUN_00408fc0(start->y, end->y, control2->y, control3->y, time);
        out->z = FUN_00408fc0(start->z, end->z, control2->z, control3->z, time);
    }
}

// FUNCTION: th08 0x408fc0
#pragma var_order(weight3, weight1, weight2, weight0)
f32 __stdcall FUN_00408fc0(f32 value0, f32 value1, f32 value2, f32 value3, f32 time)
{
    f32 weight0;
    f32 weight1;
    f32 weight2;
    f32 weight3;

    weight0 = (time - 1.0f) * (time - 1.0f) * (2.0f * time + 1.0f);
    weight1 = time * time * (3.0f - 2.0f * time);
    weight2 = (1.0f - time) * (1.0f - time) * time;
    weight3 = (time - 1.0f) * time * time;
    return weight0 * value0 + weight1 * value1 + weight2 * value2 + weight3 * value3;
}

// FUNCTION: th08 0x409080
Float3 Float3::operator+(const Float3 &other) const
{
    return Float3(this->x + other.x, this->y + other.y, this->z + other.z);
}

// FUNCTION: th08 0x4090d0
Float3 Float3::operator-(const Float3 &other) const
{
    return Float3(this->x - other.x, this->y - other.y, this->z - other.z);
}

// FUNCTION: th08 0x409120
Float3 Float3::operator*(f32 scalar) const
{
    return Float3(this->x * scalar, this->y * scalar, this->z * scalar);
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

    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vector0) = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vector1) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vector5) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&background->unk6394.vector2) = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    background->unk6394.unk48 = 0.5235987901687622f;
    background->unk6264 = background->unk6394;
    background->unk62b0 = background->unk6394;

    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(background) + 0x6474) = 0;
    for (i = 0; i < 4; i++)
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(background) + i * 4 + 0x63E0) = 0;
        background->interpolationTimers[i] = 0;
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

// FUNCTION: th08 0x40a1b0
#pragma var_order(objQuadType1, curQuadVm, instancesDrawn, instance, fogState, worldMatrix, obj, objectDistance, cameraVec, quadPos, projectDest, curQuad, didDraw, radius, projectSrc, quadWidth, originalColor, this)
ZunResult Background::RenderObjects(i32 mode)
{
    RawStageQuadType1 *objQuadType1;
    AnmVm *curQuadVm;
    i32 instancesDrawn;
    RawStageObjectInstance *instance;
    i32 fogState;
    RawStageObject *obj;
    f32 objectDistance;
    RawStageQuadBasic *curQuad;
    i32 didDraw;
    f32 radius;
    f32 quadWidth;
    ZunColor originalColor;

    instance = reinterpret_cast<RawStageObjectInstance *>(this->stageUnknown804);
    instancesDrawn = 0;
    didDraw = 0;

    Float3 quadPos;
    Float3 cameraVec;
    Float3 projectDest;
    Float3 projectSrc(0.0f, 0.0f, 0.0f);
    D3DXMATRIX worldMatrix;

    fogState = 255;

    this->SetCamera2();
    g_AnmManager->SetCameraMode(1);
    D3DXMatrixIdentity(&worldMatrix);
    cameraVec = *reinterpret_cast<Float3 *>(&g_Supervisor.viewMatrix);
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&cameraVec),
                      reinterpret_cast<D3DXVECTOR3 *>(&cameraVec));

    while (instance->id >= 0)
    {
        obj = reinterpret_cast<RawStageObject **>(this->stageOffsets)[instance->id];
        if (obj->zLevel == mode)
        {
            curQuad = &obj->firstQuad;

            quadPos.x = obj->position.x + instance->position.x - this->vector824.x + obj->size.x / 2.0f;
            quadPos.y = obj->position.y + instance->position.y - this->vector824.y + obj->size.y / 2.0f;
            quadPos.z = obj->position.z + instance->position.z - this->vector824.z + obj->size.z / 2.0f;
            quadPos = quadPos - (this->unk6394.vector0 + this->unk6394.vector5);

            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x6470) <
                D3DXVec3LengthSq(reinterpret_cast<D3DXVECTOR3 *>(&quadPos)))
            {
                goto skip;
            }

            objectDistance = D3DXVec3Dot(reinterpret_cast<D3DXVECTOR3 *>(&quadPos),
                                         reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector3));
            radius = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&obj->size)) / 2.0f + 960.0f;
            if ((objectDistance > radius) || (objectDistance < 80.0f))
            {
                goto skip;
            }

            obj->flags |= 2;
            didDraw = 1;
            while (curQuad->type >= 0)
            {
                        curQuadVm = &reinterpret_cast<AnmVm *>(this->stageAnm)[curQuad->vmIdx];
                        switch (curQuad->type)
                        {
                        case 0:
                            curQuadVm->pos.x = curQuadVm->pos2.x + curQuad->position.x + instance->position.x -
                                                 this->vector824.x;
                            curQuadVm->pos.y = curQuadVm->pos2.y + curQuad->position.y + instance->position.y -
                                                 this->vector824.y;
                            curQuadVm->pos.z = curQuadVm->pos2.z + curQuad->position.z + instance->position.z -
                                                 this->vector824.z;
                            if (curQuad->size.x != 0.0f)
                            {
                                curQuadVm->scale.x = curQuad->size.x / curQuadVm->loadedSprite->widthPx;
                            }
                            if (curQuad->size.y != 0.0f)
                            {
                                curQuadVm->scale.y = curQuad->size.y / curQuadVm->loadedSprite->heightPx;
                            }

                            if ((curQuadVm->type & 0xF) == 2)
                            {
                                worldMatrix._41 = curQuadVm->pos[0];
                                worldMatrix._42 = curQuadVm->pos[1];
                                worldMatrix._43 = curQuadVm->pos[2];
                                D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&quadPos),
                                                reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                                &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);

                                if (curQuad->size.x != 0.0f)
                                {
                                    quadWidth = curQuad->size.x;
                                }
                                else
                                {
                                    quadWidth = curQuadVm->loadedSprite->widthPx;
                                }

                                worldMatrix._41 = cameraVec.x * quadWidth * curQuadVm->scale.x + worldMatrix._41;
                                worldMatrix._42 = cameraVec.y * quadWidth * curQuadVm->scale.x + worldMatrix._42;
                                worldMatrix._43 = cameraVec.z * quadWidth * curQuadVm->scale.x + worldMatrix._43;
                                D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectDest),
                                                reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                                &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                                projectDest = projectDest - quadPos;
                                curQuadVm->scale.x =
                                    D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest)) / quadWidth;
                                curQuadVm->scale.y = curQuadVm->scale.x;
                                if (quadWidth < 0.0f)
                                {
                                    curQuadVm->scale.y = -curQuadVm->scale.y;
                                }

                                projectDest = curQuadVm->pos - (this->unk6394.vector0 + this->unk6394.vector5);
                                quadWidth = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest));
                                originalColor = curQuadVm->color1;
                                if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) < quadWidth)
                                {
                                    quadWidth = (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) -
                                                 quadWidth) /
                                                (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) -
                                                 *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAF0));
                                    if (quadWidth >= 1.0f)
                                    {
                                        break;
                                    }
                                    curQuadVm->color1.b = curQuadVm->color1.b - static_cast<u8>(
                                        (curQuadVm->color1.b - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->b) *
                                        quadWidth);
                                    curQuadVm->color1.g = curQuadVm->color1.g - static_cast<u8>(
                                        (curQuadVm->color1.g - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->g) *
                                        quadWidth);
                                    curQuadVm->color1.r = curQuadVm->color1.r - static_cast<u8>(
                                        (curQuadVm->color1.r - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->r) *
                                        quadWidth);
                                    curQuadVm->color1.a =
                                        static_cast<u8>(curQuadVm->color1.a * (1.0f - quadWidth));
                                }

                                curQuadVm->pos = quadPos;
                                if ((curQuadVm->pos.z < 0.0f) || (curQuadVm->pos.z > 1.0f))
                                {
                                    goto restore_color;
                                }

                                if (fogState != 0)
                                {
                                    if (!g_Supervisor.IsFogDisabled())
                                    {
                                        g_Supervisor.DisableFog();
                                    }
                                    fogState = 0;
                                }
                                g_AnmManager->DrawNoRotationNoRound(curQuadVm);
                                if ((curQuadVm->type & 0xF0) == 0x10 &&
                                    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) > quadWidth &&
                                    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x647C) != 0)
                                {
                                    this->vectors6480[*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) +
                                                                              0x6478)] = quadPos;
                                    this->vectors6480[*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) +
                                                                              0x6478)].z = 0.0f;
                                    (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x6478))++;
                                }

                            restore_color:
                                curQuadVm->color1 = originalColor;
                            }
                            else
                            {
                                if (!g_Supervisor.IsFogDisabled() && fogState != 1)
                                {
                                    if (!g_Supervisor.IsFogDisabled())
                                    {
                                        g_Supervisor.EnableFog();
                                    }
                                    fogState = 1;
                                }
                                g_AnmManager->Draw3D(curQuadVm);
                            }
                            break;

                        case 1:
                        {
                            objQuadType1 = reinterpret_cast<RawStageQuadType1 *>(curQuad);
#pragma var_order(type1World, halfWidthSecond, type1Width, vertices, projectedSecond, halfWidthFirst)
                            Float3 type1World;
                            Float3 projectedSecond;
                            BackgroundStageVertex vertices[4];
                            f32 halfWidthFirst;
                            f32 halfWidthSecond;
                            f32 type1Width;

                            type1World.x = objQuadType1->position1.x + instance->position.x - this->vector824.x;
                            type1World.y = objQuadType1->position1.y + instance->position.y - this->vector824.y;
                            type1World.z = objQuadType1->position1.z + instance->position.z - this->vector824.z;
                            worldMatrix._41 = type1World.x;
                            worldMatrix._42 = type1World.y;
                            worldMatrix._43 = type1World.z;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&quadPos),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);

                            if (objQuadType1->width != 0.0f)
                            {
                                type1Width = objQuadType1->width;
                            }
                            else
                            {
                                type1Width = curQuadVm->loadedSprite->widthPx;
                            }
                            worldMatrix._41 = cameraVec.x * type1Width + worldMatrix._41;
                            worldMatrix._42 = cameraVec.y * type1Width + worldMatrix._42;
                            worldMatrix._43 = cameraVec.z * type1Width + worldMatrix._43;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectDest),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                            projectDest = projectDest - quadPos;
                            halfWidthFirst =
                                D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest)) / 2.0f;

                            projectDest = type1World - (this->unk6394.vector0 + this->unk6394.vector5);
                            type1Width = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest));
                            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) < type1Width)
                            {
                                type1Width = (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) -
                                              type1Width) /
                                             (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) -
                                              *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAF0));
                                if (type1Width < 1.0f)
                                {
                                    vertices[1].diffuse.b = curQuadVm->color1.b - static_cast<u8>(
                                        (curQuadVm->color1.b - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->b) *
                                        type1Width);
                                    vertices[0].diffuse.b = vertices[1].diffuse.b;
                                    vertices[1].diffuse.g = curQuadVm->color1.g - static_cast<u8>(
                                        (curQuadVm->color1.g - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->g) *
                                        type1Width);
                                    vertices[0].diffuse.g = vertices[1].diffuse.g;
                                    vertices[1].diffuse.r = curQuadVm->color1.r - static_cast<u8>(
                                        (curQuadVm->color1.r - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->r) *
                                        type1Width);
                                    vertices[0].diffuse.r = vertices[1].diffuse.r;
                                    vertices[1].diffuse.a =
                                        static_cast<u8>(curQuadVm->color1.a * (1.0f - type1Width));
                                    vertices[0].diffuse.a = vertices[1].diffuse.a;
                                }
                                else
                                {
                                    vertices[1].diffuse.a = 0;
                                    vertices[0].diffuse.a = vertices[1].diffuse.a;
                                }
                            }
                            else
                            {
                                vertices[1].diffuse.d3dColor = curQuadVm->color1.d3dColor;
                                vertices[0].diffuse.d3dColor = vertices[1].diffuse.d3dColor;
                            }

                            type1World.x = objQuadType1->position2.x + instance->position.x - this->vector824.x;
                            type1World.y = objQuadType1->position2.y + instance->position.y - this->vector824.y;
                            type1World.z = objQuadType1->position2.z + instance->position.z - this->vector824.z;
                            worldMatrix._41 = type1World.x;
                            worldMatrix._42 = type1World.y;
                            worldMatrix._43 = type1World.z;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectedSecond),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);

                            if (objQuadType1->width != 0.0f)
                            {
                                type1Width = objQuadType1->width;
                            }
                            else
                            {
                                type1Width = curQuadVm->loadedSprite->widthPx;
                            }
                            worldMatrix._41 = cameraVec.x * type1Width + worldMatrix._41;
                            worldMatrix._42 = cameraVec.y * type1Width + worldMatrix._42;
                            worldMatrix._43 = cameraVec.z * type1Width + worldMatrix._43;
                            D3DXVec3Project(reinterpret_cast<D3DXVECTOR3 *>(&projectDest),
                                            reinterpret_cast<D3DXVECTOR3 *>(&projectSrc), &g_Supervisor.viewport,
                                            &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                            projectDest = projectDest - projectedSecond;
                            halfWidthSecond =
                                D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest)) / 2.0f;

                            projectDest = type1World - (this->unk6394.vector0 + this->unk6394.vector5);
                            type1Width = D3DXVec3Length(reinterpret_cast<D3DXVECTOR3 *>(&projectDest));
                            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) < type1Width)
                            {
                                type1Width = (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) -
                                              type1Width) /
                                             (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAEC) -
                                              *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xAF0));
                                if (type1Width < 1.0f)
                                {
                                    vertices[3].diffuse.b = curQuadVm->color1.b - static_cast<u8>(
                                        (curQuadVm->color1.b - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->b) *
                                        type1Width);
                                    vertices[2].diffuse.b = vertices[3].diffuse.b;
                                    vertices[3].diffuse.g = curQuadVm->color1.g - static_cast<u8>(
                                        (curQuadVm->color1.g - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->g) *
                                        type1Width);
                                    vertices[2].diffuse.g = vertices[3].diffuse.g;
                                    vertices[3].diffuse.r = curQuadVm->color1.r - static_cast<u8>(
                                        (curQuadVm->color1.r - reinterpret_cast<ZunColor *>(
                                                                      reinterpret_cast<u8 *>(this) + 0xAF4)->r) *
                                        type1Width);
                                    vertices[2].diffuse.r = vertices[3].diffuse.r;
                                    vertices[3].diffuse.a =
                                        static_cast<u8>(curQuadVm->color1.a * (1.0f - type1Width));
                                    vertices[2].diffuse.a = vertices[3].diffuse.a;
                                }
                                else
                                {
                                    vertices[3].diffuse.a = 0;
                                    vertices[2].diffuse.a = vertices[3].diffuse.a;
                                }
                            }
                            else
                            {
                                vertices[3].diffuse.d3dColor = curQuadVm->color1.d3dColor;
                                vertices[2].diffuse.d3dColor = vertices[3].diffuse.d3dColor;
                            }

                            projectSrc = projectedSecond - quadPos;
                            type1Width = sqrtf(projectSrc.x * projectSrc.x + projectSrc.y * projectSrc.y);
                            if (type1Width < 0.00001f)
                            {
                                goto advance_quad;
                            }
                            projectSrc /= type1Width;

                            if ((quadPos.z < 0.0f) || (quadPos.z > 1.0f))
                            {
                                goto advance_quad;
                            }
                            if ((projectedSecond.z < 0.0f) || (projectedSecond.z > 1.0f))
                            {
                                goto advance_quad;
                            }

                            vertices[0].pos.x = projectSrc.y * halfWidthFirst + quadPos.x;
                            vertices[0].pos.y = quadPos.y - projectSrc.x * halfWidthFirst;
                            vertices[0].pos.z = quadPos.z;
                            vertices[1].pos.x = quadPos.x - projectSrc.y * halfWidthFirst;
                            vertices[1].pos.y = projectSrc.x * halfWidthFirst + quadPos.y;
                            vertices[1].pos.z = quadPos.z;
                            vertices[2].pos.x = projectSrc.y * halfWidthSecond + projectedSecond.x;
                            vertices[2].pos.y = projectedSecond.y - projectSrc.x * halfWidthSecond;
                            vertices[2].pos.z = projectedSecond.z;
                            vertices[3].pos.x = projectedSecond.x - projectSrc.y * halfWidthSecond;
                            vertices[3].pos.y = projectSrc.x * halfWidthSecond + projectedSecond.y;
                            vertices[3].pos.z = projectedSecond.z;

                            vertices[2].textureUV.x = curQuadVm->loadedSprite->uvStart.x +
                                                      curQuadVm->uvScrollPos.x;
                            vertices[0].textureUV.x = vertices[2].textureUV.x;
                            vertices[3].textureUV.x = curQuadVm->loadedSprite->uvEnd.x +
                                                      curQuadVm->uvScrollPos.x;
                            vertices[1].textureUV.x = vertices[3].textureUV.x;
                            vertices[1].textureUV.y = curQuadVm->loadedSprite->uvStart.y +
                                                      curQuadVm->uvScrollPos.y;
                            vertices[0].textureUV.y = vertices[1].textureUV.y;
                            vertices[3].textureUV.y = curQuadVm->loadedSprite->uvEnd.y +
                                                      curQuadVm->uvScrollPos.y;
                            vertices[2].textureUV.y = vertices[3].textureUV.y;
                            vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;

                            if (fogState != 0)
                            {
                                if (!g_Supervisor.IsFogDisabled())
                                {
                                    g_Supervisor.DisableFog();
                                }
                                fogState = 0;
                            }
                            g_AnmManager->FUN_00464dd0(curQuadVm, reinterpret_cast<VertexTex1DiffuseXyzrhw *>(vertices));
                            break;
                        }
                        }
                    advance_quad:
                        curQuad = reinterpret_cast<RawStageQuadBasic *>(reinterpret_cast<u8 *>(curQuad) +
                                                                       curQuad->byteSize);
            }
            instancesDrawn++;
        }
    skip:
        instance++;
    }
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x40b470
#pragma var_order(inverse, this)
Float3 *Float3::operator/=(f32 scalar)
{
    f32 inverse;

    inverse = 1.0f / scalar;
    this->x *= inverse;
    this->y *= inverse;
    this->z *= inverse;
    return this;
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
    Float3 atVec = this->unk6394.vector1 + this->unk6394.vector0;
    Float3 eyeVec = this->unk6394.vector5 + this->unk6394.vector0;
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix, reinterpret_cast<D3DXVECTOR3 *>(&eyeVec),
                       reinterpret_cast<D3DXVECTOR3 *>(&atVec),
                       reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector2));
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, this->unk6394.unk48,
                               (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height, 30.0f, 1800.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
    D3DXVec3Cross(reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector4),
                  reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector1),
                  reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector2));
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector4),
                      reinterpret_cast<D3DXVECTOR3 *>(&this->unk6394.vector4));
}

// FUNCTION: th08 0x40b900
ZunBool IsDisableResourceReload()
{
    return g_Supervisor.unk16c;
}

}; // Namespace th08
