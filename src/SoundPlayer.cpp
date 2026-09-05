#include "th_pch.h"

#include "SoundPlayer.hpp"

#include "Global.hpp"
#include "Supervisor.hpp"
#include "dxutil.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th08
{

#define BGM_BUFFER_SIZE 0x8000
#define BGM_WAV_NUM_CHANNELS 2
#define BGM_WAV_BITS_PER_SAMPLE 16
#define BGM_WAV_BLOCK_ALIGN (BGM_WAV_BITS_PER_SAMPLE / 8 * BGM_WAV_NUM_CHANNELS)
#define BGM_WAV_SAMPLES_PER_SEC 44100

DIFFABLE_STATIC_ARRAY_ASSIGN(SoundBufferIdxVolume, 46, g_SoundBufferIdxVol) = {
    {0, -1900, 0},   {0, -2100, 0},   {1, -1200, 5},    {1, -1500, 5},   {2, -1100, 100}, {3, -700, 100},
    {4, -700, 100},  {5, -1900, 50},  {6, -2200, 50},   {7, -2400, 50},  {8, -1100, 100}, {9, -1100, 100},
    {10, -1500, 10}, {11, -1500, 10}, {12, -1000, 100}, {5, -1100, 50},  {13, -1300, 50}, {14, -1400, 50},
    {15, -900, 100}, {16, -400, 100}, {17, -880, 0},    {18, -1500, 0},  {5, -300, 20},   {6, -1800, 20},
    {7, -1800, 20},  {19, -1100, 50}, {20, -1300, 50},  {21, -1500, 50}, {22, -500, 140}, {23, -500, 100},
    {24, -1100, 20}, {25, -800, 90},  {24, -1200, 20},  {19, -500, 50},  {26, -800, 100}, {27, -800, 100},
    {28, -800, 100}, {29, -700, 0},   {30, -300, 100},  {31, -800, 100}, {32, -800, 100}, {33, -200, 100},
    {34, 0, 100},    {34, -600, 100}, {35, -800, 0},    {8, -100, 100},
};
DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 36, g_SFXList) = {
    "se_plst00.wav",   "se_enep00.wav",  "se_pldead00.wav", "se_power0.wav",   "se_power1.wav",   "se_tan00.wav",
    "se_tan01.wav",    "se_tan02.wav",   "se_ok00.wav",     "se_cancel00.wav", "se_select00.wav", "se_gun00.wav",
    "se_cat00.wav",    "se_lazer00.wav", "se_lazer01.wav",  "se_enep01.wav",   "se_nep00.wav",    "se_damage00.wav",
    "se_item00.wav",   "se_kira00.wav",  "se_kira01.wav",   "se_kira02.wav",   "se_extend.wav",   "se_timeout.wav",
    "se_graze.wav",    "se_powerup.wav", "se_pause.wav",    "se_cardget.wav",  "se_option.wav",   "se_damage01.wav",
    "se_timeout2.wav", "se_opshow.wav",  "se_ophide.wav",   "se_invalid.wav",  "se_slash.wav",    "se_item01.wav",
};
DIFFABLE_STATIC(SoundPlayer, g_SoundPlayer)

#pragma var_order(bufDesc, audioBuffer2Start, audioBuffer2Len, audioBuffer1Len, audioBuffer1Start, wavFormat)
ZunResult SoundPlayer::InitializeDSound(HWND gameWindow)
{
    DSBUFFERDESC bufDesc;
    WAVEFORMATEX wavFormat;
    LPVOID audioBuffer1Start;
    DWORD audioBuffer1Len;
    LPVOID audioBuffer2Start;
    DWORD audioBuffer2Len;

    ZeroMemory(this, sizeof(SoundPlayer));

    for (i32 i = 0; i < NUM_SOUND_BUFFERS; i++)
    {
        this->unconsumedMetadataBySound[i] = -1;
    }
    for (i32 i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        this->soundQueue[i] = -1;
    }

    this->manager = new CSoundManager();
    if (FAILED(this->manager->Initialize(gameWindow, 2, 2, BGM_WAV_SAMPLES_PER_SEC, BGM_WAV_BITS_PER_SAMPLE)))
    {
        g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT);
        SAFE_DELETE(this->manager);
        return ZUN_ERROR;
    }

    this->dsoundHdl = this->manager->GetDirectSound();
    this->bgmThreadHandle = NULL;
    ZeroMemory(&bufDesc, sizeof(DSBUFFERDESC));
    bufDesc.dwSize = sizeof(DSBUFFERDESC);
    bufDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    bufDesc.dwBufferBytes = BGM_BUFFER_SIZE;
    ZeroMemory(&wavFormat, sizeof(WAVEFORMATEX));
    wavFormat.cbSize = 0;
    wavFormat.wFormatTag = WAVE_FORMAT_PCM;
    wavFormat.nChannels = BGM_WAV_NUM_CHANNELS;
    wavFormat.nSamplesPerSec = BGM_WAV_SAMPLES_PER_SEC;
    wavFormat.nAvgBytesPerSec = BGM_WAV_SAMPLES_PER_SEC * sizeof(INT16) * BGM_WAV_NUM_CHANNELS;
    wavFormat.nBlockAlign = BGM_WAV_BLOCK_ALIGN;
    wavFormat.wBitsPerSample = BGM_WAV_BITS_PER_SAMPLE;
    bufDesc.lpwfxFormat = &wavFormat;
    if (FAILED(this->dsoundHdl->CreateSoundBuffer(&bufDesc, &this->initSoundBuffer, NULL)))
    {
        return ZUN_ERROR;
    }
    if (FAILED(this->initSoundBuffer->Lock(0, BGM_BUFFER_SIZE, &audioBuffer1Start, &audioBuffer1Len, &audioBuffer2Start,
                                           &audioBuffer2Len, 0)))
    {
        return ZUN_ERROR;
    }

    ZeroMemory(audioBuffer1Start, BGM_BUFFER_SIZE);
    this->initSoundBuffer->Unlock(audioBuffer1Start, audioBuffer1Len, audioBuffer2Start, audioBuffer2Len);
    this->initSoundBuffer->Play(0, 0, 1);
    this->bgmVolume = 100;
    this->sfxVolume = 100;
    /* 4 times per second */
    SetTimer(gameWindow, 0, 250, NULL);
    this->gameWindow = gameWindow;
    g_GameErrorContext.Log(TH_DBG_SOUNDPLAYER_INIT_SUCCESS);
    return ZUN_SUCCESS;
}

ZunResult SoundPlayer::Release()
{
    i32 i;

    if (this->bgmFmtData != NULL)
    {
        g_ZunMemory.Free(this->bgmFmtData);
    }
    for (i = 0; i < NUM_SOUND_BUFFERS; i++)
    {
        SAFE_RELEASE(this->duplicateSoundBuffers[i]);
        SAFE_RELEASE(this->soundBuffers[i]);
    }
    if (this->manager == NULL)
    {
        return ZUN_SUCCESS;
    }
    KillTimer(this->gameWindow, 1);
    this->StopBGM();
    this->dsoundHdl = NULL;
    this->initSoundBuffer->Stop();
    SAFE_RELEASE(this->initSoundBuffer);
    SAFE_DELETE(this->bgm);
    SAFE_DELETE(this->manager);
    for (i = 0; i < NUM_BGM_SLOTS; i++)
    {
        this->FreePreloadedBGM(i);
    }
    return ZUN_SUCCESS;
}

#pragma var_order(pos, i, buffer)
i32 SoundPlayer::GetFmtIndexByName(char *name)
{
    char *pos;
    i32 i = 0;
    char buffer[128];

    pos = strrchr(name, '/');
    if (pos == NULL)
    {
        pos = strrchr(name, '\\');
    }
    if (pos == NULL)
    {
        strcpy(buffer, name);
    }
    else
    {
        strcpy(buffer, pos + 1);
    }
    while (this->bgmFmtData[i].name[0] != '\0')
    {
        if (strcmp(this->bgmFmtData[i].name, buffer) == 0)
        {
            break;
        }
        i++;
    }
    if (this->bgmFmtData[i].name[0] == '\0')
    {
        i = 0;
    }
    return i;
}

#pragma var_order(sFDCursor, dsBuffer, wavDataPtr, formatSize, audioPtr2, audioSize2, audioSize1, audioPtr1,           \
                  soundFileData, wavData, fileSize)
ZunResult SoundPlayer::LoadSound(i32 idx, char *path)
{
    u8 *soundFileData;
    u8 *sFDCursor;
    i32 fileSize;
    WAVEFORMATEX *wavDataPtr;
    WAVEFORMATEX *audioPtr1;
    WAVEFORMATEX *audioPtr2;
    DWORD audioSize1;
    DWORD audioSize2;
    WAVEFORMATEX wavData;
    i32 formatSize;
    DSBUFFERDESC dsBuffer;

    if (this->manager == NULL)
    {
        return ZUN_SUCCESS;
    }
    SAFE_RELEASE(this->soundBuffers[idx]);
    soundFileData = (u8 *)FileSystem::OpenFile(path, NULL, FALSE);
    sFDCursor = soundFileData;
    if (sFDCursor == NULL)
    {
        return ZUN_ERROR;
    }
    if (strncmp((char *)sFDCursor, "RIFF", 4))
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE2, path);
        g_ZunMemory.Free(soundFileData);
        return ZUN_ERROR;
    }
    sFDCursor += 4;

    fileSize = *(i32 *)sFDCursor;
    sFDCursor += 4;

    if (strncmp((char *)sFDCursor, "WAVE", 4))
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        g_ZunMemory.Free(soundFileData);
        return ZUN_ERROR;
    }
    sFDCursor += 4;
    wavDataPtr = GetWavFormatData(sFDCursor, "fmt ", &formatSize, fileSize - 12);
    if (wavDataPtr == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        g_ZunMemory.Free(soundFileData);
        return ZUN_ERROR;
    }
    wavData = *wavDataPtr;

    wavDataPtr = GetWavFormatData(sFDCursor, "data", &formatSize, fileSize - 12);
    if (wavDataPtr == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        g_ZunMemory.Free(soundFileData);
        return ZUN_ERROR;
    }
    ZeroMemory(&dsBuffer, sizeof(dsBuffer));
    dsBuffer.dwSize = sizeof(dsBuffer);
    dsBuffer.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_LOCSOFTWARE;
    dsBuffer.dwBufferBytes = formatSize;
    dsBuffer.lpwfxFormat = &wavData;
    if (FAILED(this->dsoundHdl->CreateSoundBuffer(&dsBuffer, &this->soundBuffers[idx], NULL)))
    {
        g_ZunMemory.Free(soundFileData);
        return ZUN_ERROR;
    }
    if (FAILED(soundBuffers[idx]->Lock(0, formatSize, (LPVOID *)&audioPtr1, (LPDWORD)&audioSize1, (LPVOID *)&audioPtr2,
                                       (LPDWORD)&audioSize2, NULL)))
    {
        g_ZunMemory.Free(soundFileData);
        return ZUN_ERROR;
    }
    CopyMemory(audioPtr1, wavDataPtr, audioSize1);
    if (audioSize2 != 0)
    {
        CopyMemory(audioPtr2, (i8 *)wavDataPtr + audioSize1, audioSize2);
    }
    soundBuffers[idx]->Unlock((LPVOID *)audioPtr1, audioSize1, (LPVOID *)audioPtr2, audioSize2);
    g_ZunMemory.Free(soundFileData);
    return ZUN_SUCCESS;
}

WAVEFORMATEX *SoundPlayer::GetWavFormatData(u8 *soundData, char *formatString, i32 *formatSize,
                                            u32 fileSizeExcludingFormat)
{
    while (fileSizeExcludingFormat > 0)
    {
        *formatSize = *(i32 *)(soundData + 4);
        if (strncmp((char *)soundData, formatString, 4) == 0)
        {
            return (WAVEFORMATEX *)(soundData + 8);
        }
        fileSizeExcludingFormat -= (*formatSize + 8);
        soundData += *formatSize + 8;
    }
    return NULL;
}

ZunResult SoundPlayer::LoadFmt(char *path)
{
    this->bgmFmtData = (ThBgmFormat *)FileSystem::OpenFile(path, NULL, FALSE);
    return this->bgmFmtData != NULL ? ZUN_SUCCESS : ZUN_ERROR;
}

#pragma var_order(notifySize, fmtData, res, numSamplesPerSec, blockAlign)
ZunResult SoundPlayer::StartBGM(char *path)
{
    HRESULT res;
    ThBgmFormat *fmtData;
    DWORD blockAlign;
    DWORD numSamplesPerSec;
    DWORD notifySize;

    strcpy(this->currentBgmFileName, path);

    if (this->manager == NULL)
        return ZUN_ERROR;

    if (this->dsoundHdl == NULL)
        return ZUN_ERROR;

    utils::DebugPrint("Streming BGM Start\r\n");
    this->StopBGM();

    fmtData = this->bgmFmtData;
    blockAlign = fmtData->format.nBlockAlign;
    numSamplesPerSec = fmtData->format.nSamplesPerSec;
    notifySize = numSamplesPerSec * 4 * blockAlign / BGM_WAV_BITS_PER_SAMPLE;
    notifySize -= (notifySize % blockAlign);
    this->bgmUpdateEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    this->bgmThreadHandle =
        CreateThread(NULL, 0, SoundPlayer::BGMPlayerThread, g_Supervisor.hwndGameWindow, 0, &this->bgmThreadId);
    res = this->manager->CreateStreaming(&this->bgm, path, DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY,
                                         GUID_NULL, 16, notifySize, this->bgmUpdateEvent, fmtData);
    if (FAILED(res))
    {
        utils::DebugPrint(TH_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER);
        return ZUN_ERROR;
    }
    return ZUN_SUCCESS;
}

ZunResult SoundPlayer::ReopenBGM(char *path)
{
    if (this->bgm == NULL)
        return ZUN_ERROR;

    i32 idx = GetFmtIndexByName(path);

    this->bgm->GetWaveFile()->Reopen(&this->bgmFmtData[idx]);
    utils::DebugPrint("Streming BGM Reopen %d\r\n", idx);
    return ZUN_SUCCESS;
}

#pragma var_order(fmtIdx, numBytesRead, handle, bufferPtr)
ZunResult SoundPlayer::PreloadBGM(i32 idx, char *path)
{
    HANDLE handle;
    i32 fmtIdx;
    LPBYTE bufferPtr;
    DWORD numBytesRead;

    if (this->bgmPreloadAllocations[idx] != NULL)
    {
        if (strcmp(path, this->bgmFileNames[idx]) == 0)
            return ZUN_SUCCESS;
    }
    strcpy(g_SoundPlayer.bgmFileNames[idx], path);

    if (!g_Supervisor.IsMusicPreloadEnabled())
        return ZUN_SUCCESS;

    if (this->manager == NULL)
        return ZUN_SUCCESS;

    this->FreePreloadedBGM(idx);
    utils::DebugPrint("Streming BGM PreLoad %d\r\n", idx);

    handle = CreateFileA(this->currentBgmFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        utils::DebugPrint("error : bgmfile is not find %s\r\n", this->currentBgmFileName);
        return ZUN_ERROR;
    }

    fmtIdx = this->GetFmtIndexByName(path);
    SetFilePointer(handle, this->bgmFmtData[fmtIdx].startOffset, 0, FILE_BEGIN);

    bufferPtr = (u8 *)g_ZunMemory.Alloc(this->bgmFmtData[fmtIdx].preloadAllocSize,
                                        "d:\\cygwin\\home\\zun\\prog\\th08\\global.h");
    if (bufferPtr == NULL)
    {
        CloseHandle(handle);
        utils::DebugPrint("error : bgmfile is not find %s\r\n", this->currentBgmFileName);
        return ZUN_ERROR;
    }

    ReadFile(handle, bufferPtr, this->bgmFmtData[fmtIdx].preloadAllocSize, &numBytesRead, NULL);
    CloseHandle(handle);
    this->bgmPreloadFmtData[idx] = &this->bgmFmtData[fmtIdx];
    this->bgmPreloadAllocations[idx] = bufferPtr;
    this->bgmPreloadData[idx] = bufferPtr;
    this->bgmPreloadAllocSizes[idx] = this->bgmPreloadFmtData[idx]->preloadAllocSize;

    return ZUN_SUCCESS;
}

#pragma var_order(notifySize, hr, numSamplesPerSec, blockAlign)
ZunResult SoundPlayer::LoadBGM(i32 idx)
{
    HRESULT hr;
    DWORD blockAlign;
    DWORD numSamplesPerSec;
    DWORD notifySize;

    if (this->manager == NULL)
        return ZUN_ERROR;

    if (g_Supervisor.cfg.musicMode == OFF)
        return ZUN_ERROR;

    if (this->dsoundHdl == NULL)
        return ZUN_ERROR;

    if (!g_Supervisor.IsMusicPreloadEnabled())
        return this->ReopenBGM(this->bgmFileNames[idx]);

    if (this->bgmPreloadAllocations[idx] == NULL)
        return ZUN_ERROR;

    utils::DebugPrint("Streming BGM Load no %d\r\n", idx);

    blockAlign = this->bgmPreloadFmtData[idx]->format.nBlockAlign;
    numSamplesPerSec = this->bgmPreloadFmtData[idx]->format.nSamplesPerSec;
    notifySize = numSamplesPerSec * 4 * blockAlign / BGM_WAV_BITS_PER_SAMPLE;
    notifySize -= (notifySize % blockAlign);
    this->bgmUpdateEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    this->bgmThreadHandle =
        CreateThread(NULL, 0, SoundPlayer::BGMPlayerThread, g_Supervisor.hwndGameWindow, 0, &this->bgmThreadId);
    hr = this->manager->CreateStreamingFromMemory(
        &this->bgm, this->bgmPreloadData[idx], this->bgmPreloadAllocSizes[idx], this->bgmPreloadFmtData[idx],
        DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY, GUID_NULL, 16, notifySize, this->bgmUpdateEvent);
    if (FAILED(hr))
    {
        utils::DebugPrint(TH_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER);
        return ZUN_ERROR;
    }

    utils::DebugPrint("load comp\r\n");
    this->loadedBgmSlot = idx;
    return ZUN_SUCCESS;
}

void SoundPlayer::StopBGM()
{
    if (this->bgm != NULL)
    {
        utils::DebugPrint("Streming BGM stop\r\n");
        this->bgm->Stop();
        if (this->bgmThreadHandle != NULL)
        {
            PostThreadMessageA(this->bgmThreadId, WM_QUIT, 0, 0);
            utils::DebugPrint("stop m_dwNotifyThreadID\r\n");

            while (WaitForSingleObject(this->bgmThreadHandle, 256))
                PostThreadMessageA(this->bgmThreadId, WM_QUIT, 0, 0);

            utils::DebugPrint("stop comp\r\n");
            CloseHandle(this->bgmThreadHandle);
            CloseHandle(this->bgmUpdateEvent);
            this->bgmThreadHandle = NULL;
        }
        SAFE_DELETE(this->bgm);
    }
}

ZunResult SoundPlayer::InitSoundBuffers()
{
    i32 i;

    if (this->manager == NULL)
        return ZUN_ERROR;

    if (this->dsoundHdl == NULL)
        return ZUN_SUCCESS;

    for (i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        this->soundQueue[i] = -1;
    }
    for (i = 0; i < ARRAY_SIZE_SIGNED(g_SFXList); i++)
    {
        if (this->LoadSound(i, g_SFXList[i]) != ZUN_SUCCESS)
        {
            g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE, g_SFXList[i]);
            return ZUN_ERROR;
        }
    }
    for (i = 0; i < ARRAY_SIZE(g_SoundBufferIdxVol); i++)
    {
        this->dsoundHdl->DuplicateSoundBuffer(this->soundBuffers[g_SoundBufferIdxVol[i].bufferIdx],
                                              &this->duplicateSoundBuffers[i]);
        this->duplicateSoundBuffers[i]->SetCurrentPosition(0);
        this->duplicateSoundBuffers[i]->SetVolume(g_SoundBufferIdxVol[i].volume);
    }

    return ZUN_SUCCESS;
}

void SoundPlayer::PlaySoundByIdx(SoundIdx idx, i32 pan)
{
    i32 unconsumedMetadata;
    i32 i;

    unconsumedMetadata = g_SoundBufferIdxVol[idx].unconsumedMetadata;
    for (i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        if (this->soundQueue[i] < 0)
            break;

        if (this->soundQueue[i] == idx)
        {
            if (this->soundQueueRequestCounts[i] < 0x80)
                this->soundQueuePanData[i][this->soundQueueRequestCounts[i]++] = pan;

            return;
        }
    }

    if (i >= SFX_QUEUE_LENGTH)
        return;

    this->soundQueue[i] = idx;
    this->unconsumedMetadataBySound[idx] = unconsumedMetadata;
    this->soundQueuePanData[i][0] = pan;
    this->soundQueueRequestCounts[i]++;
}

void SoundPlayer::PlaySoundPositionedByIdx(SoundIdx idx, f32 pan)
{
    i32 unconsumedMetadata;
    i32 panAsInt;
    i32 i;

    unconsumedMetadata = g_SoundBufferIdxVol[idx].unconsumedMetadata;
    panAsInt = ((pan - 192) * 1000) / 192;

    for (i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        if (this->soundQueue[i] < 0)
            break;

        if (this->soundQueue[i] == idx)
        {
            if (this->soundQueueRequestCounts[i] < 0x80)
                this->soundQueuePanData[i][this->soundQueueRequestCounts[i]++] = panAsInt;

            return;
        }
    }

    if (i >= SFX_QUEUE_LENGTH)
        return;

    this->soundQueue[i] = idx;
    this->unconsumedMetadataBySound[idx] = unconsumedMetadata;
    this->soundQueuePanData[i][0] = panAsInt;
    this->soundQueueRequestCounts[i]++;
}

#pragma var_order(restartCommandProcessing, averagedPan, i, commandCursor, soundIndex, j, preloadBuffer, bgmPath,     \
                  bgmFormatIndex, reopenedBuffer, volumeScale)
i32 SoundPlayer::ProcessQueues()
{
    SoundPlayerCommand *commandCursor;
    BOOL restartCommandProcessing;
    LPDIRECTSOUNDBUFFER preloadBuffer;
    LPDIRECTSOUNDBUFFER reopenedBuffer;
    char *bgmPath;
    i32 bgmFormatIndex;
    i32 i;
    i32 j;
    i32 soundIndex;
    i32 averagedPan;
    f32 volumeScale;

    if (this->manager == NULL)
        return 0;

    commandCursor = this->commandQueue;

loop:
    restartCommandProcessing = FALSE;

    switch (commandCursor->opcode)
    {
    case SOUNDPLAYER_COMMAND_SET_VOLUME:
        if (this->bgm != NULL)
            this->bgm->SetVolume(this->bgmVolume);
        goto next_command;
    case SOUNDPLAYER_COMMAND_PRELOAD_BGM:
        if (g_Supervisor.IsMusicPreloadEnabled())
        {
            utils::DebugPrint("Sound : PreLoad Stage\r\n");
            if (commandCursor->step == 0)
            {
                this->StopBGM();
                this->PreloadBGM(commandCursor->argument, commandCursor->path);
                restartCommandProcessing = TRUE;
                goto next_command;
            }
        }
        else
        {
            utils::DebugPrint("Sound : PreLoad Stage\r\n");
            this->PreloadBGM(commandCursor->argument, commandCursor->path);
            restartCommandProcessing = TRUE;
            goto next_command;
        }
        commandCursor->step++;
        break;
    case SOUNDPLAYER_COMMAND_LOAD_BGM:
        if (g_Supervisor.IsMusicPreloadEnabled() && commandCursor->argument >= 0)
        {
            if (commandCursor->step == 0)
            {
                utils::DebugPrint("Sound : Load Stage\r\n");
                if (this->LoadBGM(commandCursor->argument) != ZUN_SUCCESS)
                {
                    goto next_command;
                }
            }
            else if (commandCursor->step == 2)
            {
                utils::DebugPrint("Sound : Reset Stage\r\n");
                if (this->bgm != NULL)
                {
                    if (FAILED(this->bgm->Reset()))
                    {
                        goto next_command;
                    }
                }
            }
            else if (commandCursor->step == 5)
            {
                utils::DebugPrint("Sound : Fill Buffer Stage\r\n");
                preloadBuffer = this->bgm->GetBuffer(0);
                commandCursor->argument = this->bgm->GetWaveFile()->GetFormat()->totalLength != 0;
                if (FAILED(this->bgm->FillBufferWithSound(preloadBuffer, commandCursor->argument)))
                {
                    goto next_command;
                }
            }
            else if (commandCursor->step == 7)
            {
                utils::DebugPrint("Sound : Play Stage\r\n");
                this->bgm->Play(0, DSBPLAY_LOOPING);
            }
            else
            {
                if (commandCursor->step >= 20)
                {
                    goto next_command;
                }
            }
        }
        else if (this->bgm == NULL)
        {
            goto next_command;
        }
        else if (commandCursor->step == 0)
        {
            utils::DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (commandCursor->step == 1)
        {
            if (this->bgm->m_bIsLocked)
            {
                break;
            }
            utils::DebugPrint("Sound : Recreate Stage\r\n");
            this->bgm->InitSoundBuffers();
        }
        else if (commandCursor->step == 2)
        {
            utils::DebugPrint("Sound : ReOpen Stage\r\n");
            bgmPath =
                commandCursor->argument >= 0 ? this->bgmFileNames[commandCursor->argument] : commandCursor->path;
            bgmFormatIndex = this->GetFmtIndexByName(bgmPath);
            this->bgm->GetWaveFile()->Reopen(&this->bgmFmtData[bgmFormatIndex]);
        }
        else if (commandCursor->step == 3)
        {
            utils::DebugPrint("Sound : Fill Buffer Stage\r\n");
            reopenedBuffer = this->bgm->GetBuffer(0);
            this->bgm->Reset();
            commandCursor->argument = this->bgm->GetWaveFile()->GetFormat()->totalLength != 0;
            if (FAILED(this->bgm->FillBufferWithSound(reopenedBuffer, commandCursor->argument)))
            {
                goto next_command;
            }
        }
        else if (commandCursor->step == 4)
        {
            utils::DebugPrint("Sound : Play Stage\r\n");
            this->bgm->Play(0, DSBPLAY_LOOPING);
        }
        else if (commandCursor->step >= 7)
        {
            goto next_command;
        }
        commandCursor->step++;
        break;
    case SOUNDPLAYER_COMMAND_RELEASE_BGM:
        if (this->bgm == NULL)
        {
            goto next_command;
        }
        if (commandCursor->step == 0)
        {
            utils::DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (commandCursor->step == 1)
        {
            utils::DebugPrint("Sound : Thread Stop Stage\r\n");
            if (this->bgmThreadHandle == NULL)
            {
                goto next_command;
            }
            PostThreadMessageA(this->bgmThreadId, WM_QUIT, 0, 0);
        }
        else if (commandCursor->step == 2)
        {
            if (WaitForSingleObject(this->bgmThreadHandle, 256))
            {
                utils::DebugPrint("Sound : Thread Stop Wait Stage\r\n");
                PostThreadMessageA(this->bgmThreadId, WM_QUIT, 0, 0);
                commandCursor->step--;
            }
            else
            {
                this->bgmThreadHandle = NULL;
            }
        }
        else if (commandCursor->step == 3)
        {
            utils::DebugPrint("Sound : Handle Close Stage\r\n");
            CloseHandle(this->bgmThreadHandle);
            CloseHandle(this->bgmUpdateEvent);
            this->bgmThreadHandle = NULL;
            SAFE_DELETE(this->bgm);
        }
        else if (commandCursor->step == 10)
        {
            goto next_command;
        }
        commandCursor->step++;
        break;
    case SOUNDPLAYER_COMMAND_STOP_BGM:
        if (this->bgm == NULL)
        {
            goto next_command;
        }
        if (commandCursor->step == 0)
        {
            utils::DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (commandCursor->step == 1)
        {
            goto next_command;
        }
        commandCursor->step++;
        break;
    case SOUNDPLAYER_COMMAND_FADE_OUT:
        utils::DebugPrint("Sound : Fade Out Stage %d\r\n", commandCursor->argument);
        g_SoundPlayer.FadeOut(commandCursor->argument);
        goto next_command;
    case SOUNDPLAYER_COMMAND_PAUSE:
        if (g_Supervisor.cfg.musicMode == WAV)
        {
            if (this->bgm->m_bIsLocked)
            {
                utils::DebugPrint("locked\n");
                break;
            }
            if (this->bgm != NULL)
            {
                this->bgm->Pause();
            }
        }
        goto next_command;
    case SOUNDPLAYER_COMMAND_UNPAUSE:
        if (g_Supervisor.cfg.musicMode == WAV)
        {
            if (this->bgm->m_bIsLocked)
            {
                break;
            }
            if (this->bgm != NULL)
            {
                this->bgm->Unpause();
            }
        }
        goto next_command;
    default:
        break;

    next_command:
        for (i = 0; i < BGM_QUEUE_LENGTH; i++, commandCursor++)
        {
            if (commandCursor->opcode == SOUNDPLAYER_COMMAND_NONE)
            {
                break;
            }
            CopyMemory(commandCursor, commandCursor + 1, sizeof(*commandCursor));
        }

        if (restartCommandProcessing)
        {
            goto loop;
        }
    };

    if (!g_Supervisor.cfg.playSounds)
    {
        return this->commandQueue[0].opcode;
    }
    for (i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        if (this->soundQueue[i] < 0)
        {
            break;
        }
        soundIndex = soundQueue[i];
        soundQueue[i] = -1;

        averagedPan = 0;
        for (j = 0; j < this->soundQueueRequestCounts[i]; j++)
        {
            averagedPan += this->soundQueuePanData[i][j];
        }
        averagedPan /= this->soundQueueRequestCounts[i];
        this->soundQueueRequestCounts[i] = 0;
        if (this->duplicateSoundBuffers[soundIndex] == NULL)
        {
            continue;
        }
        this->duplicateSoundBuffers[soundIndex]->Stop();
        this->duplicateSoundBuffers[soundIndex]->SetCurrentPosition(0);
        this->duplicateSoundBuffers[soundIndex]->SetPan(averagedPan);

        volumeScale = (f32)g_SoundPlayer.sfxVolume / 100.0f;

        if (g_SoundPlayer.sfxVolume != 0)
        {
            volumeScale = 1.0f - volumeScale;
            volumeScale = volumeScale * volumeScale * volumeScale;
            volumeScale = 1.0f - volumeScale;
            this->duplicateSoundBuffers[soundIndex]->SetVolume(
                (i32)((g_SoundBufferIdxVol[soundIndex].volume + 5000) * volumeScale) - 5000);
        }
        else
        {
            this->duplicateSoundBuffers[soundIndex]->SetVolume(SOUNDPLAYER_SILENT_VOLUME);
        }
        this->duplicateSoundBuffers[soundIndex]->Play(0, 0, 0);
    }

    return this->commandQueue[0].opcode;
}

#pragma var_order(msg, looped, lpThreadParameterCopy, waitObj, res, stopped)
DWORD WINAPI SoundPlayer::BGMPlayerThread(LPVOID lpThreadParameter)
{
    DWORD waitObj;
    MSG msg;
    u32 stopped;
    u32 looped;
    LPVOID lpThreadParameterCopy;
    HRESULT res;

    lpThreadParameterCopy = lpThreadParameter;
    stopped = false;
    looped = true;
    while (!stopped)
    {
        waitObj = MsgWaitForMultipleObjects(1, &g_SoundPlayer.bgmUpdateEvent, FALSE, INFINITE, QS_ALLEVENTS);
        if (g_SoundPlayer.bgm == NULL)
        {
            stopped = true;
        }
        switch (waitObj)
        {
        case WAIT_OBJECT_0:
            if (g_SoundPlayer.bgm != NULL && g_SoundPlayer.bgm->m_bIsPlaying)
            {
                g_SoundPlayer.bgm->m_bIsLocked = TRUE;
                res = g_SoundPlayer.bgm->HandleWaveStreamNotification(looped);
                g_SoundPlayer.bgm->m_bIsLocked = FALSE;
            }
            break;
        case WAIT_OBJECT_0 + 1:
            while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) != 0)
            {
                if (msg.message == WM_QUIT)
                {
                    stopped = true;
                }
            }
            break;
        }
    }
    utils::DebugPrint(TH_DBG_SOUNDPLAYER_BGM_THREAD_TERMINATED);
    return 0;
}

void SoundPlayer::QueueCommand(i32 opcode, i32 argument, char *path)
{
    i32 i;

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->commandQueue) - 1; i++)
    {
        if (this->commandQueue[i].opcode != SOUNDPLAYER_COMMAND_NONE)
            continue;

        this->commandQueue[i].opcode = opcode;
        this->commandQueue[i].argument = argument;
        strcpy(this->commandQueue[i].path, path);
        this->commandQueue[i].step = 0;

        break;
    }

    utils::DebugPrint("Sound Que Add %d\r\n", opcode);
    return;
}

ThBgmFormat *CWaveFile::GetFormat()
{
    return this->m_pzwf;
}

LPDIRECTSOUND CSoundManager::GetDirectSound()
{
#ifdef TH08_MODERN_PORT
    // MinGW models IDirectSound8 as a separate COM interface type rather than
    // a C++ subclass of IDirectSound. The DirectSound 8 vtable keeps the base
    // interface prefix used by this caller.
    return reinterpret_cast<LPDIRECTSOUND>(this->m_pDS);
#else
    return this->m_pDS;
#endif
}

CWaveFile *CSound::GetWaveFile()
{
    return this->m_pWaveFile;
}

SoundPlayer::SoundPlayer()
{
    ZeroMemory(this, sizeof(SoundPlayer));
    for (i32 i = 0; i < NUM_SOUND_BUFFERS; i++)
    {
        this->unconsumedMetadataBySound[i] = -1;
    }
}

void SoundPlayer::FreePreloadedBGM(i32 idx)
{
    if (this->bgmPreloadAllocations[idx] != NULL)
    {
        g_ZunMemory.Free(this->bgmPreloadAllocations[idx]);
        this->bgmPreloadAllocations[idx] = NULL;
    }
}

void SoundPlayer::FadeOut(f32 seconds)
{
    if (this->bgm != NULL)
    {
        this->bgm->FadeOut(seconds);
    }
}

}; // namespace th08
