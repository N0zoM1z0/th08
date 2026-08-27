//-----------------------------------------------------------------------------
// File: DSUtil.h
//
// Desc:
//
// Copyright (c) 1999-2000 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DSUTIL_H
#define DSUTIL_H

#include "inttypes.hpp"
#include <dsound.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <windows.h>

namespace th08
{
//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CSoundManager;
class CSound;
class CStreamingSound;
class CWaveFile;

//-----------------------------------------------------------------------------
// Structs used by this header
//-----------------------------------------------------------------------------
struct ThBgmFormat;

//-----------------------------------------------------------------------------
// Typing macros
//-----------------------------------------------------------------------------
#define WAVEFILE_READ 1
#define WAVEFILE_WRITE 2

#define DSUtil_StopSound(s)                                                                                            \
    {                                                                                                                  \
        if (s)                                                                                                         \
            s->Stop();                                                                                                 \
    }
#define DSUtil_PlaySound(s)                                                                                            \
    {                                                                                                                  \
        if (s)                                                                                                         \
            s->Play(0, 0);                                                                                             \
    }
#define DSUtil_PlaySoundLooping(s)                                                                                     \
    {                                                                                                                  \
        if (s)                                                                                                         \
            s->Play(0, DSBPLAY_LOOPING);                                                                               \
    }

//-----------------------------------------------------------------------------
// Name: class CSoundManager
// Desc:
//-----------------------------------------------------------------------------
class CSoundManager
{
    // This might not actually be true; it could be that zwave is compiled with inlining turned on
    friend class CStreamingSound;

  protected:
    LPDIRECTSOUND8 m_pDS;

  public:
    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                       DWORD dwPrimaryBitRate);
    LPDIRECTSOUND GetDirectSound();
    HRESULT SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate);

    HRESULT CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags,
                            GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent,
                            ThBgmFormat *pzwf);
    HRESULT CreateStreamingFromMemory(CStreamingSound **ppStreamingSound, BYTE *pbData, ULONG ulDataSize,
                                      ThBgmFormat *pzwf, DWORD dwCreationFlags, GUID guid3DAlgorithm,
                                      DWORD dwNumBuffers, DWORD dwNotifySize, HANDLE hNotifyEvent);
};

//-----------------------------------------------------------------------------
// Name: class CSound
// Desc: Encapsulates functionality of a DirectSound buffer.
//-----------------------------------------------------------------------------
class CSound
{
  protected:
    LPDIRECTSOUNDBUFFER *m_apDSBuffer;
    DWORD m_dwDSBufferSize;
    CWaveFile *m_pWaveFile;
    DWORD m_dwNumBuffers;

  public:
    // Modifications by ZUN to this class
    INT m_iCurFadeProgress;
    INT m_iTotalFade;
    INT m_iFadeType;
    DWORD m_dwPriority;
    DWORD m_dwFlags;
    DWORD unconsumedDword28;
    DWORD unconsumedDword2C;
    BOOL m_bIsPlaying;
    DSBUFFERDESC m_dsbd;
    CSoundManager *m_pSoundManager;

    HRESULT RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored);

    CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile);
    virtual ~CSound();

    HRESULT FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger);
    LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer(DWORD dwIndex);

    HRESULT Play(DWORD dwPriority, DWORD dwFlags);
    HRESULT Stop();
    HRESULT Reset();
    BOOL IsSoundPlaying();

    // Modifications made by ZUN to this class
    HRESULT SetVolume(i32 iVolume);
    HRESULT Pause();
    HRESULT Unpause();
    CWaveFile *GetWaveFile();
};

//-----------------------------------------------------------------------------
// Name: class CStreamingSound
// Desc: Encapsulates functionality to play a wave file with DirectSound.
//       The Create() method loads a chunk of wave file into the buffer,
//       and as sound plays more is written to the buffer by calling
//       HandleWaveStreamNotification() whenever hNotifyEvent is signaled.
//-----------------------------------------------------------------------------
class CStreamingSound : public CSound
{
    friend class CSoundManager;

  protected:
    DWORD m_dwLastPlayPos;
    DWORD m_dwPlayProgress;
    DWORD m_dwNextWriteOffset;
    BOOL m_bFillNextNotificationWithSilence;

  public:
    // Modifications by ZUN to this class
    DWORD m_dwNotifySize;
    HANDLE m_hNotifyEvent;
    BOOL m_bIsLocked;

    CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile, DWORD dwNotifySize);
    ~CStreamingSound();

    HRESULT HandleWaveStreamNotification(BOOL bLoopedPlay);
    HRESULT Reset();

    // Modifications by ZUN to this class
    HRESULT InitSoundBuffers();
    HRESULT UpdateFadeOut();
    HRESULT UpdateFadeIn();
    HRESULT UpdatePartialFadeIn();
    HRESULT UpdatePartialFadeOut();
    void FadeOut(f32 seconds)
    {
        m_iFadeType = 1;
        m_iTotalFade = m_iCurFadeProgress = seconds * 60;
    }
    void FadeIn(f32 seconds)
    {
        this->m_iFadeType = 2;
        this->m_iTotalFade = this->m_iCurFadeProgress = seconds * 60;
        this->SetVolume(DSBVOLUME_MIN);
    }
    void PartialFadeOut(f32 seconds)
    {
        this->m_iFadeType = 4;
        this->m_iTotalFade = this->m_iCurFadeProgress = seconds * 60;
    }
    void PartialFadeIn(f32 seconds)
    {
        this->m_iFadeType = 3;
        this->m_iTotalFade = this->m_iCurFadeProgress = seconds * 60;
        this->SetVolume(-1000);
    }
};

//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class CWaveFile
{
  public:
    // WAVEFORMATEX *m_pwfx; // Pointer to WAVEFORMATEX structure
    HMMIO m_hmmio;     // MM I/O handle for the WAVE
    MMCKINFO m_ck;     // Multimedia RIFF chunk
    MMCKINFO m_ckRiff; // Use in opening a WAVE file
    DWORD m_dwSize;    // The size of the wave file
    MMIOINFO m_mmioinfoOut;
    DWORD m_dwFlags;
    BOOL m_bIsReadingFromMemory;
    BYTE *m_pbData;
    BYTE *m_pbDataCur;
    ULONG m_ulDataSize;

    // Modifications by ZUN to this class
    HANDLE m_hWaveFile;
    ThBgmFormat *m_pzwf;

  protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO(WAVEFORMATEX *pwfxDest);

  public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open(LPTSTR strFileName, ThBgmFormat *pzwf, DWORD dwFlags);
    HRESULT OpenFromMemory(BYTE *pbData, ULONG ulDataSize, ThBgmFormat *pzwf, DWORD dwFlags);
    HRESULT Close();

    HRESULT Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead);
    HRESULT Write(UINT nSizeToWrite, BYTE *pbData, UINT *pnSizeWrote);

    DWORD GetSize();
    HRESULT ResetFile(bool bLoop);
    ThBgmFormat *GetFormat();

    // Modifications by ZUN to this class
    HRESULT Reopen(ThBgmFormat *pzwf);
};

//-----------------------------------------------------------------------------
// Name: struct ThBgmFormat
// Desc:
//-----------------------------------------------------------------------------
struct ThBgmFormat
{
    char name[16];
    i32 startOffset;
    DWORD preloadAllocSize;
    i32 introLength;
    i32 totalLength;
    WAVEFORMATEX format;
};
}; // namespace th08

#endif // DSUTIL_H
