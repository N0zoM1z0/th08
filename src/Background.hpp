#pragma once
#include "Global.hpp"
#include "AnmManager.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <windows.h>

namespace th08
{
struct BackgroundUnkVectors
{
    Float3 vectors[6];
    f32 unk48;
};
C_ASSERT(sizeof(BackgroundUnkVectors) == 0x4c);

struct Background
{
    Background();

    static ChainCallbackResult OnUpdate(Background *background);
    static ChainCallbackResult OnDrawHighPrio(Background *background);
    static ChainCallbackResult OnDrawLowPrio(Background *background);
    static ZunResult AddedCallback(Background *background);
    static ZunResult RegisterChain(i32 param);
    static ZunResult DeletedCallback(Background *background);
    static void CutChain();
    ZunResult LoadStageData(const char *path);
    void RenderObjects(i32 mode);

    void SetCamera1();
    void SetCamera2();
    void FUN_00409160(D3DCOLOR color);
    u32 FUN_00409f40();

    void *stageAnm;
    AnmVm stageVm0;
    AnmVm stageVm1;
    AnmVm stageVm2;
    AnmLoaded *stageAnmFile;
    void *stageAnmSecondary;
    i32 stageVmCount;
    i32 stageObjectCount;
    void *stageOffsets;
    void *stageUnknown804;
    void *stageUnknown808;
    ZunTimer timer80c;
    unknown_fields(0x818, 0xc);
    Float3 vector824;
    unknown_fields(0x830, 0x8);
    ZunTimer timer838;
    AnmVm textAnmVm;
    unknown_fields(0xae8, 0x2c);
    ZunTimer timerB14;
    u8 skyFogNeedsSetup;
    unknown_fields(0xb21, 0x17);
    AnmVm anmVmArray[0x20];
    AnmVm anmVm5fb8;
    void (*onDrawLowPrioCallback)();
    i32 unk6260;
    BackgroundUnkVectors unk6264;
    BackgroundUnkVectors unk62b0;
    BackgroundUnkVectors unk62fc;
    BackgroundUnkVectors unk6348;
    BackgroundUnkVectors unk6394;
    unknown_fields(0x63e0, 0x14);
    ZunTimer timers63f4[5];
    unknown_fields(0x6430, 0x14);
    Float3 vector6444;
    unknown_fields(0x6450, 0x4);
    Float3 vector6454;
    unknown_fields(0x6460, 0x20);
    Float3 vectors6480[0x20];
};
C_ASSERT(sizeof(Background) == 0x6600);

DIFFABLE_EXTERN(Background, g_Background);
}; // Namespace th08
