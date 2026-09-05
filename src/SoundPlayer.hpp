#pragma once

#include <dsound.h>
#include <windows.h>

#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "zwave.hpp"

#define SOUNDPLAYER_SILENT_VOLUME (DSBVOLUME_MIN)
#define SOUNDPLAYER_MAX_VOLUME (DSBVOLUME_MIN / 2)
#define SOUNDPLAYER_VOLUME_RANGE (SOUNDPLAYER_MAX_VOLUME - SOUNDPLAYER_SILENT_VOLUME)

namespace th08
{
enum SoundIdx
{
    NO_SOUND = -1,
    SOUND_SHOOT = 0,
    SOUND_SHOOT_QUIET = 1,
    SOUND_ENEMY_SHOOT = 2,
    SOUND_ENEMY_SHOOT_QUIET = 3,
    SOUND_PICHUN = 4,
    SOUND_POWER_0 = 5,
    SOUND_POWER_1 = 6,
    SOUND_BULLET_0 = 7,
    SOUND_BULLET_1 = 8,
    SOUND_BULLET_2 = 9,
    SOUND_SELECT = 10,
    SOUND_BACK = 11,
    SOUND_MOVE_MENU = 12,
    SOUND_GUN = 13,
    SOUND_CAT = 14,
    SOUND_BULLET_0_LOUD = 15,
    SOUND_LASER_0 = 16,
    SOUND_LASER_1 = 17,
    SOUND_TOTAL_BOSS_DEATH = 18,
    SOUND_MASTER_SPARK = 19,
    SOUND_DAMAGE = 20,
    SOUND_ITEM = 21,
    SOUND_BULLET_0_LOUDEST = 22,
    SOUND_BULLET_1_LOUD = 23,
    SOUND_BULLET_2_LOUD = 24,
    SOUND_SPARKLE_0 = 25,
    SOUND_SPARKLE_1 = 26,
    SOUND_SPARKLE_2 = 27,
    SOUND_1UP = 28,
    SOUND_TIMEOUT = 29,
    SOUND_GRAZE = 30,
    SOUND_POWERUP = 31,
    SOUND_GRAZE_QUIET = 32,
    SOUND_SPARKLE_0_LOUD = 33,
    SOUND_PAUSE = 34,
    SOUND_SPELL_CAPTURE = 35,
    SOUND_FAMILIAR_SPAWN = 36,
    SOUND_DAMAGE_LOW_HEALTH = 37,
    SOUND_TIMEOUT_2 = 38,
    SOUND_FAMILIAR_UNHIDE = 39,
    SOUND_FAMILIAR_HIDE = 40,
    SOUND_INVALID_ACTION = 41,
    SOUND_SLASH = 42,
    SOUND_SLASH_QUIET = 43,
    SOUND_ITEM_MAX_VALUE = 44,
    SOUND_SELECT_LOUD = 45,
};

struct SoundBufferIdxVolume
{
    i32 bufferIdx;
    i16 volume;
    i16 unconsumedMetadata;
};
C_ASSERT(sizeof(SoundBufferIdxVolume) == 0x8);

struct SoundPlayerCommand
{
    i32 opcode;
    i32 argument;
    i32 step;
    char path[256];
};
C_ASSERT(sizeof(SoundPlayerCommand) == 0x10c);
C_ASSERT(offsetof(SoundPlayerCommand, step) == 0x8);
C_ASSERT(offsetof(SoundPlayerCommand, path) == 0xc);

enum SoundPlayerCommandOpcode
{
    SOUNDPLAYER_COMMAND_NONE = 0,
    SOUNDPLAYER_COMMAND_PRELOAD_BGM = 1,
    SOUNDPLAYER_COMMAND_LOAD_BGM = 2,
    SOUNDPLAYER_COMMAND_STOP_BGM = 3,
    SOUNDPLAYER_COMMAND_RELEASE_BGM = 4,
    SOUNDPLAYER_COMMAND_FADE_OUT = 5,
    SOUNDPLAYER_COMMAND_PAUSE = 6,
    SOUNDPLAYER_COMMAND_UNPAUSE = 7,
    SOUNDPLAYER_COMMAND_SET_VOLUME = 8,
};

#define NUM_SOUND_BUFFERS 128
#define NUM_BGM_SLOTS 16
#define SFX_QUEUE_LENGTH 12
#define BGM_QUEUE_LENGTH 31

class SoundPlayer
{
  public:
    SoundPlayer();

    ZunResult InitializeDSound(HWND window);
    ZunResult InitSoundBuffers();
    ZunResult Release();

    ZunResult LoadSound(i32 idx, char *path);
    static WAVEFORMATEX *GetWavFormatData(u8 *soundData, char *formatString, i32 *formatSize,
                                          u32 fileSizeExcludingFormat);

    void QueueCommand(i32 opcode, i32 argument, char *path);
    i32 ProcessQueues();
    void PlaySoundByIdx(SoundIdx idx, i32 pan);
    void PlaySoundPositionedByIdx(SoundIdx idx, f32 pan);
    ZunResult StartBGM(char *path);
    ZunResult ReopenBGM(char *path);
    ZunResult PreloadBGM(i32 idx, char *path);
    ZunResult LoadBGM(i32 idx);
    void FreePreloadedBGM(i32 idx);
    void StopBGM();
    void FadeOut(f32 seconds);
    void FadeIn(f32 seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeIn(seconds);
        }
    }
    void PartialFadeOut(f32 seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->PartialFadeOut(seconds);
        }
    }
    void PartialFadeIn(f32 seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->PartialFadeIn(seconds);
        }
    }
    void Pause()
    {
        this->QueueCommand(SOUNDPLAYER_COMMAND_PAUSE, 0, "Pause");
    }
    void UnPause()
    {
        this->QueueCommand(SOUNDPLAYER_COMMAND_UNPAUSE, 0, "UnPause");
    }

    void UpdateFades();

    void QueueSetVolumeCommand()
    {
        this->QueueCommand(SOUNDPLAYER_COMMAND_SET_VOLUME, 0, "SetVol");
    }

    static DWORD WINAPI BGMPlayerThread(LPVOID lpThreadParameter);

    i32 GetFmtIndexByName(char *name);
    ZunResult LoadFmt(char *path);

    LPDIRECTSOUND dsoundHdl;
    i32 unconsumedDword04;
    LPDIRECTSOUNDBUFFER soundBuffers[NUM_SOUND_BUFFERS];
    LPDIRECTSOUNDBUFFER duplicateSoundBuffers[NUM_SOUND_BUFFERS];
    i32 unconsumedMetadataBySound[NUM_SOUND_BUFFERS];
    LPDIRECTSOUNDBUFFER initSoundBuffer;
    HWND gameWindow;
    CSoundManager *manager;
    DWORD bgmThreadId;
    HANDLE bgmThreadHandle;
    i32 unconsumedDword61C;
    i32 soundQueue[SFX_QUEUE_LENGTH];
    i32 soundQueueRequestCounts[SFX_QUEUE_LENGTH];
    u32 soundQueuePanData[SFX_QUEUE_LENGTH][128];
    ThBgmFormat *bgmPreloadFmtData[NUM_BGM_SLOTS];
    LPBYTE bgmPreloadAllocations[NUM_BGM_SLOTS];
    LPBYTE bgmPreloadData[NUM_BGM_SLOTS];
    DWORD bgmPreloadAllocSizes[NUM_BGM_SLOTS];
    u32 loadedBgmSlot;
    ThBgmFormat *bgmFmtData;
    SoundPlayerCommand commandQueue[BGM_QUEUE_LENGTH + 1];
    char bgmFileNames[NUM_BGM_SLOTS][256];
    char currentBgmFileName[256];
    CStreamingSound *bgm;
    HANDLE bgmUpdateEvent;
    i32 unconsumedDword5210;
    u32 bgmFileBaseOffset;
    i32 bgmVolume;
    i32 sfxVolume;
    i32 unconsumedBgmAttenuation;
};
C_ASSERT(sizeof(SoundPlayer) == 0x5224);
C_ASSERT(offsetof(SoundPlayer, unconsumedDword04) == 0x4);
C_ASSERT(offsetof(SoundPlayer, unconsumedMetadataBySound) == 0x408);
C_ASSERT(offsetof(SoundPlayer, unconsumedDword61C) == 0x61c);
C_ASSERT(offsetof(SoundPlayer, soundQueue) == 0x620);
C_ASSERT(offsetof(SoundPlayer, soundQueueRequestCounts) == 0x650);
C_ASSERT(offsetof(SoundPlayer, soundQueuePanData) == 0x680);
C_ASSERT(offsetof(SoundPlayer, bgmPreloadFmtData) == 0x1e80);
C_ASSERT(offsetof(SoundPlayer, bgmPreloadAllocations) == 0x1ec0);
C_ASSERT(offsetof(SoundPlayer, bgmPreloadData) == 0x1f00);
C_ASSERT(offsetof(SoundPlayer, bgmPreloadAllocSizes) == 0x1f40);
C_ASSERT(offsetof(SoundPlayer, loadedBgmSlot) == 0x1f80);
C_ASSERT(offsetof(SoundPlayer, unconsumedDword5210) == 0x5210);
C_ASSERT(offsetof(SoundPlayer, bgmFileBaseOffset) == 0x5214);
C_ASSERT(offsetof(SoundPlayer, unconsumedBgmAttenuation) == 0x5220);

DIFFABLE_EXTERN(SoundPlayer, g_SoundPlayer)
}; // namespace th08
