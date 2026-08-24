#pragma once

#include <windows.h>
#include <mmreg.h>

#define DS_OK S_OK
#define DSERR_BUFFERLOST ((HRESULT)0x88780096L)
#define DSBPLAY_LOOPING 1
#define DSBSTATUS_PLAYING 1
#define DSBSTATUS_BUFFERLOST 2
#define DSBVOLUME_MIN (-10000)
#define DSBCAPS_PRIMARYBUFFER 0x1
#define DSBCAPS_CTRLPAN 0x40
#define DSBCAPS_CTRLVOLUME 0x80
#define DSBCAPS_CTRLPOSITIONNOTIFY 0x100
#define DSBCAPS_GLOBALFOCUS 0x8000
#define DSBCAPS_GETCURRENTPOSITION2 0x10000
#define DSBCAPS_LOCSOFTWARE 0x8
#define DSSCL_PRIORITY 2

typedef struct _DSBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID guid3DAlgorithm;
} DSBUFFERDESC;
typedef struct _DSBPOSITIONNOTIFY { DWORD dwOffset; HANDLE hEventNotify; } DSBPOSITIONNOTIFY;

class IDirectSoundNotify
{
  public:
    virtual ULONG Release() = 0;
    virtual HRESULT SetNotificationPositions(DWORD, const DSBPOSITIONNOTIFY *) = 0;
  protected:
    virtual ~IDirectSoundNotify() {}
};

class IDirectSoundBuffer
{
  public:
    virtual ULONG Release() = 0;
    virtual HRESULT QueryInterface(REFIID, void **) = 0;
    virtual HRESULT GetCurrentPosition(LPDWORD, LPDWORD) = 0;
    virtual HRESULT GetStatus(LPDWORD) = 0;
    virtual HRESULT Initialize(void *, const DSBUFFERDESC *) = 0;
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD) = 0;
    virtual HRESULT Play(DWORD, DWORD, DWORD) = 0;
    virtual HRESULT SetCurrentPosition(DWORD) = 0;
    virtual HRESULT SetFormat(const WAVEFORMATEX *) = 0;
    virtual HRESULT SetVolume(LONG) = 0;
    virtual HRESULT SetPan(LONG) = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD) = 0;
    virtual HRESULT Restore() = 0;
  protected:
    virtual ~IDirectSoundBuffer() {}
};

class IDirectSound8
{
  public:
    virtual ULONG Release() = 0;
    virtual HRESULT CreateSoundBuffer(const DSBUFFERDESC *, IDirectSoundBuffer **, LPVOID) = 0;
    virtual HRESULT DuplicateSoundBuffer(IDirectSoundBuffer *, IDirectSoundBuffer **) = 0;
    virtual HRESULT SetCooperativeLevel(HWND, DWORD) = 0;
  protected:
    virtual ~IDirectSound8() {}
};

typedef IDirectSound8 *LPDIRECTSOUND8;
typedef IDirectSound8 *LPDIRECTSOUND;
typedef IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;
typedef IDirectSoundNotify *LPDIRECTSOUNDNOTIFY;

extern const GUID IID_IDirectSoundNotify;
extern const GUID GUID_NULL;
HRESULT DirectSoundCreate8(const GUID *, LPDIRECTSOUND8 *, LPVOID);
