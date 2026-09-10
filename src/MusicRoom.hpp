#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

namespace th08
{

struct TrackDescriptor
{
    char path[64];
    char title[66];
    char descriptors[8][66];

    TrackDescriptor()
    {
        memset(this, 0, sizeof(TrackDescriptor));
    }
};

C_ASSERT(sizeof(TrackDescriptor) == 0x292);

enum MusicRoomInputState
{
    MUSIC_ROOM_INPUT_WAITING_FOR_ENABLE = 0,
    MUSIC_ROOM_INPUT_ACTIVE = 1,
};

struct MusicRoom
{
    MusicRoom()
    {
        memset(this, 0, sizeof(MusicRoom));
    }

    ZunResult CheckInputEnable();
    i32 ProcessInput();

    static ZunResult RegisterChain();

    static ChainCallbackResult OnUpdate(MusicRoom *musicRoom);
    static ChainCallbackResult OnDraw(MusicRoom *musicRoom);
    static ZunResult AddedCallback(MusicRoom *musicRoom);
    static ZunResult DeletedCallback(MusicRoom *musicRoom);

    ChainElem *calcChain;
    ChainElem *drawChain;

    AnmLoaded *musicAnm;

    ZunBool bgmUnlocked[24];

    i32 frameCount;
    i32 inputState;
    i32 cursor;
    i32 selectedSongIndex;
    i32 listingOffset;

    i32 numDescriptors;
    TrackDescriptor *trackDescriptors;

    AnmVm mainVms[1];
    AnmVm songNameVms[31];
    AnmVm descriptionVms[8];
};

C_ASSERT(sizeof(MusicRoom) == 0x6a28);

} // namespace th08
