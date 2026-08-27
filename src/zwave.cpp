//-----------------------------------------------------------------------------
// File: DSUtil.cpp
//
// Desc: DirectSound framework classes for reading and writing wav files and
//       playing them in DirectSound buffers. Feel free to use this class
//       as a starting point for adding extra functionality.
//
// Copyright (c) 1999-2000 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT

#include "th_pch.h"

#include "SoundPlayer.hpp"
#include "dxutil.hpp"
#include "utils.hpp"
#include "zwave.hpp"
#include <dsound.h>
#include <dxerr8.h>
#include <mmsystem.h>
#include <windows.h>

namespace th08
{
//-----------------------------------------------------------------------------
// Name: CSoundManager::CSoundManager()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSoundManager::CSoundManager()
{
    m_pDS = NULL;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::~CSoundManager()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSoundManager::~CSoundManager()
{
    SAFE_RELEASE(m_pDS);
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::Initialize()
// Desc: Initializes the IDirectSound object and also sets the primary buffer
//       format.  This function must be called before any others.
//-----------------------------------------------------------------------------
HRESULT CSoundManager::Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                                  DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    SAFE_RELEASE(m_pDS);

    // Create IDirectSound using the primary sound device
    if (FAILED(hr = DirectSoundCreate8(NULL, &m_pDS, NULL)))
        return DXTRACE_ERR(TEXT("DirectSoundCreate8"), hr);

    // Set DirectSound coop level
    if (FAILED(hr = m_pDS->SetCooperativeLevel(hWnd, dwCoopLevel)))
        return DXTRACE_ERR(TEXT("SetCooperativeLevel"), hr);

    // Set primary buffer format
    SetPrimaryBufferFormat(dwPrimaryChannels, dwPrimaryFreq, dwPrimaryBitRate);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::SetPrimaryBufferFormat()
// Desc: Set primary buffer to a specified format
//       For example, to set the primary buffer format to 22kHz stereo, 16-bit
//       then:   dwPrimaryChannels = 2
//               dwPrimaryFreq     = 22050,
//               dwPrimaryBitRate  = 16
//-----------------------------------------------------------------------------
HRESULT CSoundManager::SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    // Get the primary buffer
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat = NULL;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBPrimary, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), hr);

    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)dwPrimaryChannels;
    wfx.nSamplesPerSec = dwPrimaryFreq;
    wfx.wBitsPerSample = (WORD)dwPrimaryBitRate;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (FAILED(hr = pDSBPrimary->SetFormat(&wfx)))
        return DXTRACE_ERR(TEXT("SetFormat"), hr);

    SAFE_RELEASE(pDSBPrimary);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::CreateStreaming()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundManager::CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName,
                                       DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount,
                                       DWORD dwNotifySize, HANDLE hNotifyEvent, ThBgmFormat *pzwf)
{
    HRESULT hr;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSBuffer = NULL;
    CWaveFile *pWaveFile = NULL;
    DSBPOSITIONNOTIFY *aPosNotify = NULL;
    LPDIRECTSOUNDNOTIFY pDSNotify = NULL;

    pWaveFile = new CWaveFile();

    if (pWaveFile->Open(strWaveFileName, pzwf, WAVEFILE_READ) != S_OK)
    {
        delete pWaveFile;
        return E_FAIL;
    }

    // Figure out how big the DSound buffer should be
    DWORD dwDSBufferSize = dwNotifySize * dwNotifyCount;

    // Set up the direct sound buffer.  Request the NOTIFY flag, so
    // that we are notified as the sound buffer plays.  Note, that using this flag
    // may limit the amount of hardware acceleration that can occur.
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = dwCreationFlags | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
    dsbd.dwBufferBytes = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat = &pWaveFile->m_pzwf->format;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBuffer, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), E_FAIL);

    // Create the notification events, so that we know when to fill
    // the buffer as the sound plays.
    if (FAILED(hr = pDSBuffer->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pDSNotify)))
        return DXTRACE_ERR(TEXT("QueryInterface"), E_FAIL);

    aPosNotify = new DSBPOSITIONNOTIFY[dwNotifyCount];
    if (aPosNotify == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < dwNotifyCount; i++)
    {
        aPosNotify[i].dwOffset = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;
    }

    // Tell DirectSound when to notify us. The notification will come in the from
    // of signaled events that are handled in WinMain()
    if (FAILED(hr = pDSNotify->SetNotificationPositions(dwNotifyCount, aPosNotify)))
    {
        SAFE_RELEASE(pDSNotify);
        SAFE_DELETE(aPosNotify);
        return DXTRACE_ERR(TEXT("SetNotificationPositions"), E_FAIL);
    }

    SAFE_RELEASE(pDSNotify);
    SAFE_DELETE(aPosNotify);

    // Create the sound
    *ppStreamingSound = new CStreamingSound(pDSBuffer, dwDSBufferSize, pWaveFile, dwNotifySize);

    CopyMemory(&(*ppStreamingSound)->m_dsbd, &dsbd, sizeof(DSBUFFERDESC));
    (*ppStreamingSound)->m_pSoundManager = this;
    (*ppStreamingSound)->m_hNotifyEvent = hNotifyEvent;
    (*ppStreamingSound)->m_bIsLocked = FALSE;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::CreateStreamingFromMemory()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundManager::CreateStreamingFromMemory(CStreamingSound **ppStreamingSound, BYTE *pbData, ULONG ulDataSize,
                                                 ThBgmFormat *pzwf, DWORD dwCreationFlags, GUID guid3DAlgorithm,
                                                 DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent)
{
    HRESULT hr;

    utils::DebugPrint("StreamingSound Create \r\n");

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSBuffer = NULL;
    CWaveFile *pWaveFile = NULL;
    DSBPOSITIONNOTIFY *aPosNotify = NULL;
    LPDIRECTSOUNDNOTIFY pDSNotify = NULL;

    pWaveFile = new CWaveFile();
    pWaveFile->OpenFromMemory(pbData, ulDataSize, pzwf, 0);

    // Figure out how big the DSound buffer should be
    DWORD dwDSBufferSize = dwNotifySize * dwNotifyCount;

    // Set up the direct sound buffer.  Request the NOTIFY flag, so
    // that we are notified as the sound buffer plays.  Note, that using this flag
    // may limit the amount of hardware acceleration that can occur.
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = dwCreationFlags | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
    dsbd.dwBufferBytes = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat = &pWaveFile->m_pzwf->format;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBuffer, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), E_FAIL);

    // Create the notification events, so that we know when to fill
    // the buffer as the sound plays.
    if (FAILED(hr = pDSBuffer->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pDSNotify)))
        return DXTRACE_ERR(TEXT("QueryInterface"), E_FAIL);

    aPosNotify = new DSBPOSITIONNOTIFY[dwNotifyCount];
    if (aPosNotify == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < dwNotifyCount; i++)
    {
        aPosNotify[i].dwOffset = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;
    }

    // Tell DirectSound when to notify us. The notification will come in the from
    // of signaled events that are handled in WinMain()
    if (FAILED(hr = pDSNotify->SetNotificationPositions(dwNotifyCount, aPosNotify)))
    {
        SAFE_RELEASE(pDSNotify);
        SAFE_DELETE(aPosNotify);
        return DXTRACE_ERR(TEXT("SetNotificationPositions"), E_FAIL);
    }

    SAFE_RELEASE(pDSNotify);
    SAFE_DELETE(aPosNotify);

    // Create the sound
    *ppStreamingSound = new CStreamingSound(pDSBuffer, dwDSBufferSize, pWaveFile, dwNotifySize);

    CopyMemory(&(*ppStreamingSound)->m_dsbd, &dsbd, sizeof(DSBUFFERDESC));
    (*ppStreamingSound)->m_pSoundManager = this;
    (*ppStreamingSound)->m_hNotifyEvent = hNotifyEvent;
    (*ppStreamingSound)->m_bIsLocked = FALSE;

    utils::DebugPrint("Success \r\n");

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSound::CSound()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSound::CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile)
{
    DWORD i;

    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    for (i = 0; i < dwNumBuffers; i++)
        m_apDSBuffer[i] = apDSBuffer[i];

    m_dwDSBufferSize = dwDSBufferSize;
    m_dwNumBuffers = dwNumBuffers;
    m_pWaveFile = pWaveFile;

    FillBufferWithSound(m_apDSBuffer[0], FALSE);

    // Make DirectSound do pre-processing on sound effects
    for (i = 0; i < dwNumBuffers; i++)
        m_apDSBuffer[i]->SetCurrentPosition(0);

    m_bIsPlaying = FALSE;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::InitSoundBuffer()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::InitSoundBuffers()
{
    DWORD i;

    m_bIsPlaying = FALSE;

    for (i = 0; i < m_dwNumBuffers; i++)
        SAFE_RELEASE(m_apDSBuffer[i]);

    SAFE_DELETE(m_apDSBuffer);

    DSBPOSITIONNOTIFY *aPosNotify = NULL;
    LPDIRECTSOUNDNOTIFY pDSNotify = NULL;

    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[m_dwNumBuffers];

    for (i = 0; i < m_dwNumBuffers; i++)
    {
        if (FAILED(m_pSoundManager->m_pDS->CreateSoundBuffer(&m_dsbd, &m_apDSBuffer[i], NULL)))
            return DXTRACE_ERR(TEXT("CreateSoundBuffer"), E_FAIL);

        if (FAILED(m_apDSBuffer[i]->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pDSNotify)))
            return DXTRACE_ERR(TEXT("QueryInterface"), E_FAIL);

        aPosNotify = new DSBPOSITIONNOTIFY[16];
        if (aPosNotify == NULL)
            return E_OUTOFMEMORY;

        for (DWORD j = 0; j < 16; j++)
        {
            aPosNotify[j].dwOffset = (m_dwNotifySize * j) + m_dwNotifySize - 1;
            aPosNotify[j].hEventNotify = m_hNotifyEvent;
        }

        if (FAILED(pDSNotify->SetNotificationPositions(16, aPosNotify)))
        {
            SAFE_RELEASE(pDSNotify);
            SAFE_DELETE(aPosNotify);
            return DXTRACE_ERR(TEXT("SetNotificationPositions"), E_FAIL);
        }

        SAFE_RELEASE(pDSNotify);
        SAFE_DELETE(aPosNotify);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSound::~CSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSound::~CSound()
{
    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        SAFE_RELEASE(m_apDSBuffer[i]);
    }

    SAFE_DELETE_ARRAY(m_apDSBuffer);
    SAFE_DELETE(m_pWaveFile);
}

//-----------------------------------------------------------------------------
// Name: CSound::FillBufferWithSound()
// Desc: Fills a DirectSound buffer with a sound file
//-----------------------------------------------------------------------------
HRESULT CSound::FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger)
{
    HRESULT hr;
    VOID *pDSLockedBuffer = NULL;   // Pointer to locked buffer memory
    DWORD dwDSLockedBufferSize = 0; // Size of the locked DirectSound buffer
    DWORD dwWavDataRead = 0;        // Amount of data read from the wav file

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if (FAILED(hr = RestoreBuffer(pDSB, NULL)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    // Lock the buffer down
    if (FAILED(hr = pDSB->Lock(0, m_dwDSBufferSize, &pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L)))
        return DXTRACE_ERR(TEXT("Lock"), hr);

    // Reset the wave file to the beginning
    m_pWaveFile->ResetFile(false);

    if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize, &dwWavDataRead)))
        return DXTRACE_ERR(TEXT("Read"), hr);

    if (dwWavDataRead == 0)
    {
        // Wav is blank, so just fill with silence
        FillMemory((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize,
                   (BYTE)(m_pWaveFile->m_pzwf->format.wBitsPerSample == 8 ? 128 : 0));
    }
    else if (dwWavDataRead < dwDSLockedBufferSize)
    {
        // If the wav file was smaller than the DirectSound buffer,
        // we need to fill the remainder of the buffer with data
        if (bRepeatWavIfBufferLarger)
        {
            // Reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwWavDataRead; // From previous call above.
            while (dwReadSoFar < dwDSLockedBufferSize)
            {
                // This will keep reading in until the buffer is full
                // for very short files
                if (FAILED(hr = m_pWaveFile->ResetFile(false)))
                    return DXTRACE_ERR(TEXT("ResetFile"), hr);

                hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer + dwReadSoFar, dwDSLockedBufferSize - dwReadSoFar,
                                       &dwWavDataRead);
                if (FAILED(hr))
                    return DXTRACE_ERR(TEXT("Read"), hr);

                dwReadSoFar += dwWavDataRead;
            }
        }
        else
        {
            // Don't repeat the wav file, just fill in silence
            FillMemory((BYTE *)pDSLockedBuffer + dwWavDataRead, dwDSLockedBufferSize - dwWavDataRead,
                       (BYTE)(m_pWaveFile->m_pzwf->format.wBitsPerSample == 8 ? 128 : 0));
        }
    }

    // Unlock the buffer, we don't need it anymore.
    pDSB->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSound::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT CSound::RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored)
{
    HRESULT hr;

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;
    if (pbWasRestored)
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if (FAILED(hr = pDSB->GetStatus(&dwStatus)))
        return DXTRACE_ERR(TEXT("GetStatus"), hr);

    if (dwStatus & DSBSTATUS_BUFFERLOST)
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so
        // the restoring the buffer may fail.
        // If it does, sleep until DirectSound gives us control.
        do
        {
            hr = pDSB->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
        } while (hr = pDSB->Restore());

        if (pbWasRestored != NULL)
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//-----------------------------------------------------------------------------
// Name: CSound::GetFreeBuffer()
// Desc: Checks to see if a buffer is playing and returns TRUE if it is.
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetFreeBuffer()
{
    BOOL bIsPlaying = FALSE;
#ifdef TH08_MODERN_PORT
    DWORD i;
#endif

    if (m_apDSBuffer == NULL)
        return FALSE;

#ifdef TH08_MODERN_PORT
    for (i = 0; i < m_dwNumBuffers; i++)
#else
    for (DWORD i = 0; i < m_dwNumBuffers; i++)
#endif
    {
        if (m_apDSBuffer[i])
        {
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus(&dwStatus);
            if ((dwStatus & DSBSTATUS_PLAYING) == 0)
                break;
        }
    }

    if (i != m_dwNumBuffers)
        return m_apDSBuffer[i];
    else
        return m_apDSBuffer[rand() % m_dwNumBuffers];
}

//-----------------------------------------------------------------------------
// Name: CSound::GetBuffer()
// Desc:
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetBuffer(DWORD dwIndex)
{
    if (m_apDSBuffer == NULL)
        return NULL;
    if (dwIndex >= m_dwNumBuffers)
        return NULL;

    return m_apDSBuffer[dwIndex];
}

//-----------------------------------------------------------------------------
// Name: CSound::Play()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT CSound::Play(DWORD dwPriority, DWORD dwFlags)
{
    HRESULT hr;
    BOOL bRestored;

    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();

    if (pDSB == NULL)
        return DXTRACE_ERR(TEXT("GetFreeBuffer"), E_FAIL);

    // Restore the buffer if it was lost
    if (FAILED(hr = RestoreBuffer(pDSB, &bRestored)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    if (bRestored)
    {
        // The buffer was restored, so we need to fill it with new data
        if (FAILED(hr = FillBufferWithSound(pDSB, FALSE)))
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);

        // Make DirectSound do pre-processing on sound effects
        Reset();
    }

    m_iFadeType = 0;
    m_iCurFadeProgress = 0;
    m_iTotalFade = 0;
    SetVolume(0);
    m_bIsPlaying = TRUE;
    m_dwPriority = dwPriority;
    m_dwFlags = dwFlags;
    m_unk2c = 0;

    return pDSB->Play(0, dwPriority, dwFlags);
}

//-----------------------------------------------------------------------------
// Name: CSound::SetVolume()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSound::SetVolume(i32 iVolume)
{
    f32 volumeF = g_SoundPlayer.bgmVolume / 100.0f;

    if (g_SoundPlayer.bgmVolume != 0)
    {
        volumeF = 1.0f - volumeF;
        volumeF = volumeF * volumeF;
        volumeF = 1.0f - volumeF;
        return m_apDSBuffer[0]->SetVolume((i32)((iVolume + 5000) * volumeF) - 5000);
    }
    else
    {
        return m_apDSBuffer[0]->SetVolume(DSBVOLUME_MIN);
    }
}

//-----------------------------------------------------------------------------
// Name: CSound::Stop()
// Desc: Stops the sound from playing
//-----------------------------------------------------------------------------
HRESULT CSound::Stop()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    m_bIsPlaying = FALSE;

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        hr |= m_apDSBuffer[i]->Stop();
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);
    }

    m_iFadeType = 0;

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CSound::Pause()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSound::Pause()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    m_bIsPlaying = FALSE;
    hr |= m_apDSBuffer[0]->Stop();

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CSound::Unpause()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSound::Unpause()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = m_apDSBuffer[0];

    m_bIsPlaying = TRUE;

    return pDSB->Play(0, m_dwPriority, m_dwFlags);
}

//-----------------------------------------------------------------------------
// Name: CSound::Reset()
// Desc: Reset all of the sound buffers
//-----------------------------------------------------------------------------
HRESULT CSound::Reset()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::CStreamingSound()
// Desc: Setups up a buffer so data can be streamed from the wave file into
//       buffer.  This is very useful for large wav files that would take a
//       while to load.  The buffer is initially filled with data, then
//       as sound is played the notification events are signaled and more data
//       is written into the buffer by calling HandleWaveStreamNotification()
//-----------------------------------------------------------------------------
CStreamingSound::CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile,
                                 DWORD dwNotifySize)
    : CSound(&pDSBuffer, dwDSBufferSize, 1, pWaveFile)
{
    m_dwLastPlayPos = 0;
    m_dwPlayProgress = 0;
    m_dwNotifySize = dwNotifySize;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::~CStreamingSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CStreamingSound::~CStreamingSound()
{
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::UpdateFadeOut()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::UpdateFadeOut()
{
    if (m_iFadeType == 1)
    {
        if (--m_iCurFadeProgress <= 0)
        {
            m_iFadeType = 0;
            m_apDSBuffer[0]->Stop();
            return S_FALSE;
        }

        i32 newVolume = m_iCurFadeProgress * 5000 / m_iTotalFade - 5000;
        HRESULT hr = SetVolume(newVolume);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::UpdateFadeIn()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::UpdateFadeIn()
{
    if (m_iFadeType == 2)
    {
        if (--m_iCurFadeProgress <= 0)
        {
            m_iFadeType = 0;
            return S_FALSE;
        }

        i32 newVolume = 0 - m_iCurFadeProgress * 5000 / m_iTotalFade;
        HRESULT hr = SetVolume(newVolume);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::UpdatePartialFadeIn()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::UpdatePartialFadeIn()
{
    if (m_iFadeType == 3)
    {
        if (--m_iCurFadeProgress <= 0)
        {
            m_iFadeType = 0;
            return S_FALSE;
        }

        i32 newVolume = 0 - m_iCurFadeProgress * 1000 / m_iTotalFade;
        HRESULT hr = SetVolume(newVolume);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::UpdatePartialFadeOut()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::UpdatePartialFadeOut()
{
    if (m_iFadeType == 4)
    {
        if (--m_iCurFadeProgress <= 0)
        {
            m_iFadeType = 0;
            return S_FALSE;
        }

        i32 newVolume = m_iCurFadeProgress * 1000 / m_iTotalFade - 1000;
        HRESULT hr = SetVolume(newVolume);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::HandleWaveStreamNotification()
// Desc: Handle the notification that tell us to put more wav data in the
//       circular buffer
//-----------------------------------------------------------------------------
#pragma var_order(dwDSLockedBufferSize2, pDSLockedBuffer, dwBytesWrittenToBuffer, currentPlayCursor,                 \
                  pDSLockedBuffer2, bRestored, dwPlayDelta, hr, dwDSLockedBufferSize, dwCurrentPlayPos,             \
                  currentWriteCursor, dwReadSoFar)
HRESULT CStreamingSound::HandleWaveStreamNotification(BOOL bLoopedPlay)
{
    HRESULT hr;
    DWORD dwCurrentPlayPos;
    DWORD dwPlayDelta;
    DWORD dwBytesWrittenToBuffer;
    VOID *pDSLockedBuffer;
    VOID *pDSLockedBuffer2;
    DWORD dwDSLockedBufferSize;
    DWORD dwDSLockedBufferSize2;
    DWORD currentPlayCursor;
    DWORD currentWriteCursor;

    if (m_apDSBuffer == NULL || m_pWaveFile == NULL)
        return CO_E_NOTINITIALIZED;

    m_apDSBuffer[0]->GetCurrentPosition(&currentPlayCursor, &currentWriteCursor);

    if ((m_dwNextWriteOffset >= currentWriteCursor - m_dwNotifySize &&
         m_dwNextWriteOffset < currentWriteCursor) ||
        (currentWriteCursor - m_dwNotifySize < 0 &&
         m_dwNextWriteOffset >= m_dwDSBufferSize - m_dwNotifySize))
    {
        utils::DebugPrint("Stream Skip\n");
        return CO_E_FIRST;
    }

    // Restore the buffer if it was lost
    BOOL bRestored;
    if (FAILED(hr = RestoreBuffer(m_apDSBuffer[0], &bRestored)))
    {
        utils::DebugPrint("error : RestoreBuffer in HandleWaveStreamNotification\r\n");
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);
    }

    if (bRestored)
    {
        // The buffer was restored, so we need to fill it with new data
        if (FAILED(hr = FillBufferWithSound(m_apDSBuffer[0], FALSE)))
        {
            utils::DebugPrint("error : FillBufferWithSound in HandleWaveStreamNotification\r\n");
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);
        }
        return S_OK;
    }

    // Lock the DirectSound buffer
    pDSLockedBuffer = NULL;
    pDSLockedBuffer2 = NULL;
    if (FAILED(hr = m_apDSBuffer[0]->Lock(m_dwNextWriteOffset, m_dwNotifySize, &pDSLockedBuffer, &dwDSLockedBufferSize,
                                          &pDSLockedBuffer2, &dwDSLockedBufferSize2, 0L)))
    {
        utils::DebugPrint("error : Buffer->Lock in HandleWaveStreamNotification\r\n");
        return DXTRACE_ERR(TEXT("Lock"), hr);
    }

    // m_dwDSBufferSize and m_dwNextWriteOffset are both multiples of m_dwNotifySize,
    // it should the second buffer should never be valid
    if (pDSLockedBuffer2 != NULL)
        return E_UNEXPECTED;

    if (!m_bFillNextNotificationWithSilence)
    {
        // Fill the DirectSound buffer with wav data
        if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize, &dwBytesWrittenToBuffer)))
        {
            utils::DebugPrint("error : m_pWaveFile->Read in HandleWaveStreamNotification\r\n");
            return DXTRACE_ERR(TEXT("Read"), hr);
        }
    }
    else
    {
        // Fill the DirectSound buffer with silence
        FillMemory(pDSLockedBuffer, dwDSLockedBufferSize,
                   (BYTE)(m_pWaveFile->m_pzwf->format.wBitsPerSample == 8 ? 128 : 0));
        dwBytesWrittenToBuffer = dwDSLockedBufferSize;
    }

    // If the number of bytes written is less than the
    // amount we requested, we have a short file.
    if (dwBytesWrittenToBuffer < dwDSLockedBufferSize)
    {
        if (!bLoopedPlay)
        {
            // Fill in silence for the rest of the buffer.
            FillMemory((BYTE *)pDSLockedBuffer + dwBytesWrittenToBuffer, dwDSLockedBufferSize - dwBytesWrittenToBuffer,
                       (BYTE)(m_pWaveFile->m_pzwf->format.wBitsPerSample == 8 ? 128 : 0));

            // Any future notifications should just fill the buffer with silence
            m_bFillNextNotificationWithSilence = TRUE;
        }
        else
        {
            // We are looping, so reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwBytesWrittenToBuffer; // From previous call above.
            while (dwReadSoFar < dwDSLockedBufferSize)
            {
                // This will keep reading in until the buffer is full (for very short files).
                if (FAILED(hr = m_pWaveFile->ResetFile(true)))
                {
                    utils::DebugPrint("error : m_pWaveFile->ResetFile in HandleWaveStreamNotification\r\n");
                    return DXTRACE_ERR(TEXT("ResetFile"), hr);
                }

                if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer + dwReadSoFar,
                                                  dwDSLockedBufferSize - dwReadSoFar, &dwBytesWrittenToBuffer)))
                {
                    utils::DebugPrint("error : m_pWaveFile->Read(+) in HandleWaveStreamNotification\r\n");
                    return DXTRACE_ERR(TEXT("Read"), hr);
                }

                dwReadSoFar += dwBytesWrittenToBuffer;
            }
        }
    }

    // Unlock the DirectSound buffer
    m_apDSBuffer[0]->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

    // Figure out how much data has been played so far.  When we have played
    // passed the end of the file, we will either need to start filling the
    // buffer with silence or starting reading from the beginning of the file,
    // depending if the user wants to loop the sound
    if (FAILED(hr = m_apDSBuffer[0]->GetCurrentPosition(&dwCurrentPlayPos, NULL)))
    {
        utils::DebugPrint("error : m_apDSBuffer[0]->GetCurrentPosition in HandleWaveStreamNotification\r\n");
        return DXTRACE_ERR(TEXT("GetCurrentPosition"), hr);
    }

    // Check to see if the position counter looped
    if (dwCurrentPlayPos < m_dwLastPlayPos)
        dwPlayDelta = (m_dwDSBufferSize - m_dwLastPlayPos) + dwCurrentPlayPos;
    else
        dwPlayDelta = dwCurrentPlayPos - m_dwLastPlayPos;

    m_dwPlayProgress += dwPlayDelta;
    m_dwLastPlayPos = dwCurrentPlayPos;

    // If we are now filling the buffer with silence, then we have found the end so
    // check to see if the entire sound has played, if it has then stop the buffer.
    if (m_bFillNextNotificationWithSilence)
    {
        // We don't want to cut off the sound before it's done playing.
        if (m_dwPlayProgress >= m_pWaveFile->GetSize())
        {
            m_apDSBuffer[0]->Stop();
        }
    }

    // Update where the buffer will lock (for next time)
    m_dwNextWriteOffset += dwDSLockedBufferSize;
    m_dwNextWriteOffset %= m_dwDSBufferSize; // Circular buffer

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::Reset()
// Desc: Resets the sound so it will begin playing at the beginning
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::Reset()
{
    HRESULT hr;

    if (m_apDSBuffer[0] == NULL || m_pWaveFile == NULL)
        return CO_E_NOTINITIALIZED;

    m_dwLastPlayPos = 0;
    m_dwPlayProgress = 0;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;

    // Restore the buffer if it was lost
    BOOL bRestored;
    if (FAILED(hr = RestoreBuffer(m_apDSBuffer[0], &bRestored)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    if (bRestored)
    {
        // The buffer was restored, so we need to fill it with new data
        if (FAILED(hr = FillBufferWithSound(m_apDSBuffer[0], FALSE)))
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);
    }

    m_pWaveFile->ResetFile(false);

    return m_apDSBuffer[0]->SetCurrentPosition(0L);
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.
//       Then call Read() as needed.  Calling the destructor or Close()
//       will close the file.
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pzwf = NULL;
    m_hmmio = NULL;
    m_dwSize = 0;
    m_bIsReadingFromMemory = FALSE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open(LPTSTR strFileName, ThBgmFormat *pzwf, DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

    if (m_dwFlags == WAVEFILE_READ)
    {
        if (strFileName == NULL)
            return E_INVALIDARG;

        utils::DebugPrint("Streaming File Open %s\r\n", strFileName);
        m_hWaveFile = CreateFileA(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (m_hWaveFile == INVALID_HANDLE_VALUE)
            return E_FAIL;

        m_pzwf = pzwf;
        ResetFile(false);
        m_dwSize = m_ck.cksize;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Reopen()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Reopen(ThBgmFormat *pzwf)
{
    if (m_bIsReadingFromMemory)
        return E_FAIL;

    if (m_hWaveFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    m_pzwf = pzwf;
    ResetFile(false);
    m_dwSize = m_ck.cksize;
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: copy data to CWaveFile member variable from memory
//-----------------------------------------------------------------------------
HRESULT CWaveFile::OpenFromMemory(BYTE *pbData, ULONG ulDataSize, ThBgmFormat *pzwf, DWORD dwFlags)
{
    m_pzwf = pzwf;
    m_ulDataSize = ulDataSize;
    m_pbData = pbData;
    m_pbDataCur = m_pbData;
    m_bIsReadingFromMemory = TRUE;

    if (dwFlags != WAVEFILE_READ)
        return E_NOTIMPL;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file
//-----------------------------------------------------------------------------
DWORD CWaveFile::GetSize()
{
    return m_dwSize;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the
//       beginning of the file again
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile(bool bLoop)
{
    DWORD unconsumedSeekResult;

    if (m_bIsReadingFromMemory)
    {
        m_pbDataCur = m_pbData;

        if (m_pzwf->totalLength > 0)
        {
            m_ulDataSize = m_pzwf->totalLength;
        }

        if (bLoop && m_pzwf->introLength > 0)
        {
            m_pbDataCur += m_pzwf->introLength;
        }
    }
    else
    {
        if (m_hWaveFile == NULL)
        {
            return CO_E_NOTINITIALIZED;
        }

        if (bLoop && m_pzwf->introLength > 0)
        {
            unconsumedSeekResult =
                SetFilePointer(m_hWaveFile,
                               g_SoundPlayer.bgmFileBaseOffset + m_pzwf->introLength + m_pzwf->startOffset, 0,
                               FILE_BEGIN);
            m_ck.cksize = m_pzwf->totalLength - m_pzwf->introLength;
        }
        else
        {
            unconsumedSeekResult =
                SetFilePointer(m_hWaveFile, g_SoundPlayer.bgmFileBaseOffset + m_pzwf->startOffset, 0, FILE_BEGIN);
            m_ck.cksize = m_pzwf->totalLength;
        }
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So
//       subsequent calls will be continue where the last left off unless
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead)
{
    if (m_bIsReadingFromMemory)
    {
        if (m_pbDataCur == NULL)
            return CO_E_NOTINITIALIZED;
        if (pdwSizeRead != NULL)
            *pdwSizeRead = 0;

        if ((BYTE *)(m_pbDataCur + dwSizeToRead) > (BYTE *)(m_pbData + m_ulDataSize))
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }

        CopyMemory(pBuffer, m_pbDataCur, dwSizeToRead);
        m_pbDataCur += dwSizeToRead;

        if (pdwSizeRead != NULL)
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else
    {
        if (m_hWaveFile == NULL)
            return CO_E_NOTINITIALIZED;
        if (pBuffer == NULL || pdwSizeRead == NULL)
            return E_INVALIDARG;

        UINT cbDataIn = dwSizeToRead;
        if (cbDataIn > m_ck.cksize)
            cbDataIn = m_ck.cksize;

        m_ck.cksize -= cbDataIn;

        DWORD dwSize;
        ReadFile(m_hWaveFile, pBuffer, cbDataIn, &dwSize, NULL);

        if (pdwSizeRead != NULL)
            *pdwSizeRead = dwSize;

        return S_OK;
    }
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Close()
{
    if (m_dwFlags == WAVEFILE_READ)
    {
        CloseHandle(m_hWaveFile);
        m_hWaveFile = INVALID_HANDLE_VALUE;
    }

    return S_OK;
}
}; // namespace th08
