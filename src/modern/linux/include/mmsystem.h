#pragma once

#include <windows.h>
#include <mmreg.h>

typedef UINT MMRESULT;
typedef UINT MMVERSION;
typedef void *HMMIO;
typedef UINT_PTR MMIOPROC;
typedef UINT(WINAPI *LPTIMECALLBACK)(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

typedef struct timecaps_tag { UINT wPeriodMin, wPeriodMax; } TIMECAPS;
typedef struct midi_hdr_tag {
    LPSTR lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD_PTR dwUser;
    DWORD dwFlags;
    struct midi_hdr_tag *lpNext;
    DWORD_PTR reserved;
    DWORD dwOffset;
    DWORD_PTR dwReserved[8];
} MIDIHDR, *LPMIDIHDR;

typedef struct _MMCKINFO { DWORD ckid, cksize, fccType, dwDataOffset, dwFlags; } MMCKINFO;
typedef struct _MMIOINFO {
    DWORD dwFlags, fccIOProc;
    MMIOPROC pIOProc;
    UINT wErrorRet;
    HANDLE htask;
    LONG cchBuffer;
    HPSTR pchBuffer, pchNext, pchEndRead, pchEndWrite;
    LONG lBufOffset, lDiskOffset;
    DWORD adwInfo[3];
    DWORD dwReserved1, dwReserved2;
    HMMIO hmmio;
} MMIOINFO;

typedef struct joyinfoex_tag {
    DWORD dwSize, dwFlags, dwXpos, dwYpos, dwZpos, dwRpos, dwUpos, dwVpos,
        dwButtons, dwButtonNumber, dwPOV, dwReserved1, dwReserved2;
} JOYINFOEX;
typedef struct tagJOYCAPSA {
    WORD wMid, wPid;
    char szPname[32];
    UINT wXmin, wXmax, wYmin, wYmax, wZmin, wZmax, wNumButtons, wPeriodMin, wPeriodMax,
        wRmin, wRmax, wUmin, wUmax, wVmin, wVmax;
    UINT wCaps, wMaxAxes, wNumAxes, wMaxButtons;
    char szRegKey[32];
    char szOEMVxD[260];
} JOYCAPSA;

#define MMSYSERR_NOERROR 0
#define JOYERR_NOERROR 0
#define JOY_RETURNALL 0xff
#define MIDI_MAPPER ((UINT)-1)
#define TIME_PERIODIC 1

MMRESULT timeGetDevCaps(TIMECAPS *, UINT);
MMRESULT timeBeginPeriod(UINT);
MMRESULT timeEndPeriod(UINT);
UINT timeSetEvent(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
MMRESULT timeKillEvent(UINT);
MMRESULT midiOutOpen(HMIDIOUT *, UINT, DWORD_PTR, DWORD_PTR, DWORD);
MMRESULT midiOutClose(HMIDIOUT);
MMRESULT midiOutReset(HMIDIOUT);
MMRESULT midiOutPrepareHeader(HMIDIOUT, LPMIDIHDR, UINT);
MMRESULT midiOutUnprepareHeader(HMIDIOUT, LPMIDIHDR, UINT);
MMRESULT midiOutLongMsg(HMIDIOUT, LPMIDIHDR, UINT);
MMRESULT midiOutShortMsg(HMIDIOUT, DWORD);
MMRESULT joyGetPosEx(UINT, JOYINFOEX *);
MMRESULT joyGetDevCapsA(UINT_PTR, JOYCAPSA *, UINT);
