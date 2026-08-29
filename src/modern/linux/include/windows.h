#pragma once

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

#define WINAPI
#define CALLBACK
#define APIENTRY
#define __stdcall
#define __cdecl
#define __fastcall
#define __int64 long long
#define CONST const
#define FAR
#define TEXT(x) x
#define TRUE 1
#define FALSE 0
#define MAX_PATH 260

typedef void VOID;
typedef int BOOL;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef uint8_t byte;
typedef uint32_t UINT;
typedef int INT;
typedef int16_t INT16;
typedef uintptr_t UINT_PTR;
typedef uintptr_t DWORD_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;
typedef intptr_t LRESULT;
typedef float FLOAT;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef void *PVOID;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef BYTE *LPBYTE;
typedef DWORD *LPDWORD;
typedef int *LPINT;
typedef char *LPSTR;
typedef char *HPSTR;
typedef const char *LPCSTR;
typedef wchar_t *LPWSTR;
typedef const wchar_t *LPCWSTR;
typedef char *LPTSTR;
typedef long HRESULT;
typedef void *HANDLE;
typedef HANDLE HGLOBAL;
typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HDC;
typedef HANDLE HGDIOBJ;
typedef HANDLE HBITMAP;
typedef HANDLE HFONT;
typedef HANDLE HBRUSH;
typedef HANDLE HCURSOR;
typedef HANDLE HWND;
typedef HANDLE HMIDIOUT;
typedef HANDLE HMONITOR;
typedef DWORD COLORREF;
typedef DWORD MMRESULT;
typedef DWORD MMVERSION;
typedef uint64_t ULONGLONG;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    long long QuadPart;
} LARGE_INTEGER;

typedef struct tagRECT {
    LONG left, top, right, bottom;
} RECT, *LPRECT;
typedef const RECT *LPCRECT;

typedef struct tagPOINT {
    LONG x, y;
} POINT;

typedef struct tagSIZE {
    LONG cx, cy;
} SIZE;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagRGBQUAD { BYTE rgbBlue, rgbGreen, rgbRed, rgbReserved; } RGBQUAD;
typedef struct tagBITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO;

typedef struct _RGNDATA {
    char unused[32];
} RGNDATA;

typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG;

typedef LRESULT(CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef struct tagWNDCLASSA {
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HANDLE hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASSA, WNDCLASS;

typedef struct _STARTUPINFOA {
    DWORD cb;
    LPSTR lpReserved;
    LPSTR lpDesktop;
    LPSTR lpTitle;
    DWORD dwX, dwY, dwXSize, dwYSize;
    DWORD dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
    WORD wShowWindow, cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput, hStdOutput, hStdError;
} STARTUPINFOA, STARTUPINFO;

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    DWORD ftCreationTime[2];
    DWORD ftLastAccessTime[2];
    DWORD ftLastWriteTime[2];
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, WIN32_FIND_DATA;

typedef pthread_mutex_t CRITICAL_SECTION;
typedef DWORD(WINAPI *LPTHREAD_START_ROUTINE)(LPVOID);

typedef struct _GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
} GUID;
typedef const GUID &REFGUID;
typedef const GUID &REFIID;

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define S_OK ((HRESULT)0)
#define S_FALSE ((HRESULT)1)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000eL)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_UNEXPECTED ((HRESULT)0x8000ffffL)
#define CO_E_FIRST ((HRESULT)0x800401f0L)
#define CO_E_NOTINITIALIZED ((HRESULT)0x800401f0L)
#define ZeroMemory(d, n) memset((d), 0, (n))
#define CopyMemory(d, s, n) memcpy((d), (s), (n))
#define FillMemory(d, n, v) memset((d), (v), (n))
#ifdef TH08_PORTABLE_NATIVE_LAYOUT
// Runtime classes intentionally use native pointer alignment in the portable
// 64-bit build. The exact/i386 build keeps checking the original VC7 layout.
#define C_ASSERT(e)
#else
#define C_ASSERT(e) typedef char __C_ASSERT__[(e) ? 1 : -1]
#endif
// Serialized file structures keep their original byte contract on every host.
#define TH08_FILE_ASSERT(e) typedef char __TH08_FILE_ASSERT__[(e) ? 1 : -1]
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | ((WORD)((BYTE)(g)) << 8) | ((DWORD)(BYTE)(b) << 16)))

#define GENERIC_READ 0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_APPEND_DATA 0x00000004u
#define FILE_SHARE_READ 0x00000001u
#define FILE_SHARE_WRITE 0x00000002u
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define GMEM_FIXED 0
#define LPTR 0x40
#define ERROR_ALREADY_EXISTS 183
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define INFINITE 0xffffffffu
#define QS_ALLEVENTS 0x04bf

#define PM_REMOVE 1
#define WM_CLOSE 0x0010
#define WM_SETCURSOR 0x0020
#define WM_ERASEBKGND 0x0014
#define WM_ACTIVATEAPP 0x001c
#define WM_QUIT 0x0012
#define MM_MOM_DONE 0x3c9

#define MB_OK 0
#define MB_ICONERROR 0x10
#define MB_ICONSTOP 0x10
#define SW_HIDE 0
#define WS_OVERLAPPEDWINDOW 0x00cf0000u
#define WS_VISIBLE 0x10000000u
#define WS_MINIMIZEBOX 0x00020000u
#define WS_SYSMENU 0x00080000u
#define CW_USEDEFAULT 0
#define BLACK_BRUSH 4
#define IDC_ARROW ((LPCSTR)32512)
#define GWL_HINSTANCE (-6)
#define SM_CXDLGFRAME 7
#define SM_CYDLGFRAME 8
#define SM_CYCAPTION 4

#define SPI_GETSCREENSAVEACTIVE 16
#define SPI_SETSCREENSAVEACTIVE 17
#define SPI_GETLOWPOWERACTIVE 83
#define SPI_SETLOWPOWERACTIVE 85
#define SPI_GETPOWEROFFACTIVE 84
#define SPI_SETPOWEROFFACTIVE 86
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#define SPIF_SENDCHANGE 2

#define CP_ACP 0
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x100
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x200
#define FORMAT_MESSAGE_FROM_SYSTEM 0x1000
#define LANG_USER_DEFAULT 0x400
#define STGM_READ 0
#define CLSCTX_INPROC_SERVER 1
#define CALLBACK_WINDOW 0x10000

#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateFileA(LPCSTR, DWORD, DWORD, LPVOID, DWORD, DWORD, HANDLE);
HANDLE CreateFileW(LPCWSTR, DWORD, DWORD, LPVOID, DWORD, DWORD, HANDLE);
BOOL ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPVOID);
BOOL WriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPVOID);
DWORD SetFilePointer(HANDLE, LONG, LONG *, DWORD);
DWORD GetFileSize(HANDLE, LPDWORD);
BOOL CloseHandle(HANDLE);
BOOL FlushFileBuffers(HANDLE);
BOOL DeleteFileA(LPCSTR);
DWORD GetFileAttributesW(LPCWSTR);
BOOL SetCurrentDirectoryW(LPCWSTR);
HANDLE FindFirstFileA(LPCSTR, WIN32_FIND_DATAA *);
BOOL FindNextFileA(HANDLE, WIN32_FIND_DATAA *);
BOOL FindClose(HANDLE);

void Sleep(DWORD);
DWORD timeGetTime(void);
BOOL QueryPerformanceFrequency(LARGE_INTEGER *);
BOOL QueryPerformanceCounter(LARGE_INTEGER *);
DWORD GetCurrentThreadId(void);
HANDLE CreateThread(LPVOID, size_t, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
BOOL PostThreadMessageA(DWORD, UINT, WPARAM, LPARAM);
DWORD MsgWaitForMultipleObjects(DWORD, const HANDLE *, BOOL, DWORD, DWORD);
DWORD WaitForSingleObject(HANDLE, DWORD);
HANDLE CreateEventA(LPVOID, BOOL, BOOL, LPCSTR);
BOOL SetEvent(HANDLE);
UINT_PTR SetTimer(HWND, UINT_PTR, UINT, void *);
BOOL KillTimer(HWND, UINT_PTR);
HANDLE CreateMutexA(LPVOID, BOOL, LPCSTR);
DWORD GetLastError(void);

void InitializeCriticalSection(CRITICAL_SECTION *);
void DeleteCriticalSection(CRITICAL_SECTION *);
void EnterCriticalSection(CRITICAL_SECTION *);
void LeaveCriticalSection(CRITICAL_SECTION *);

BOOL PeekMessageA(MSG *, HWND, UINT, UINT, UINT);
BOOL TranslateMessage(const MSG *);
LRESULT DispatchMessageA(const MSG *);
LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterClassA(const WNDCLASSA *);
HWND CreateWindowExA(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HANDLE, HINSTANCE, LPVOID);
BOOL DestroyWindow(HWND);
BOOL ShowWindow(HWND, int);
BOOL MoveWindow(HWND, int, int, int, int, BOOL);
int ShowCursor(BOOL);
HCURSOR SetCursor(HCURSOR);
HCURSOR LoadCursorA(HINSTANCE, LPCSTR);
HGDIOBJ GetStockObject(int);
int GetSystemMetrics(int);
BOOL SystemParametersInfoA(UINT, UINT, PVOID, UINT);
HWND GetForegroundWindow(void);
DWORD GetWindowThreadProcessId(HWND, LPDWORD);
BOOL AttachThreadInput(DWORD, DWORD, BOOL);
HWND SetActiveWindow(HWND);
LONG GetWindowLongA(HWND, int);
BOOL WINNLSEnableIME(HWND, BOOL);
BOOL GetKeyboardState(BYTE *);
BOOL SetKeyboardState(const BYTE *);

int MessageBoxA(HWND, LPCSTR, LPCSTR, UINT);
int MessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT);
DWORD GetModuleFileNameA(HMODULE, LPSTR, DWORD);
DWORD GetConsoleTitleA(LPSTR, DWORD);
void GetStartupInfoA(STARTUPINFOA *);
int MultiByteToWideChar(UINT, DWORD, LPCSTR, int, LPWSTR, int);
DWORD FormatMessageA(DWORD, LPCVOID, DWORD, DWORD, LPSTR, DWORD, va_list *);
LPVOID LocalFree(LPVOID);

HGLOBAL GlobalAlloc(UINT, size_t);
HGLOBAL GlobalFree(HGLOBAL);
HMODULE LoadLibraryA(LPCSTR);
void *GetProcAddress(HMODULE, LPCSTR);

HDC CreateCompatibleDC(HDC);
BOOL DeleteDC(HDC);
HGDIOBJ SelectObject(HDC, HGDIOBJ);
BOOL DeleteObject(HGDIOBJ);
BOOL TextOutA(HDC, int, int, LPCSTR, int);
int SetBkMode(HDC, int);
COLORREF SetTextColor(HDC, COLORREF);
HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR);
HBITMAP CreateDIBSection(HDC, const void *, UINT, VOID **, HANDLE, DWORD);

HRESULT CoInitialize(LPVOID);
void CoUninitialize(void);
HRESULT CoCreateInstance(REFGUID, LPVOID, DWORD, REFIID, LPVOID *);

#ifdef __cplusplus
}
#endif

#define GetStartupInfo GetStartupInfoA
#define lstrcpynW(dst, src, n) (wcsncpy((dst), (src), (n)), (dst)[(n)-1] = 0, (dst))
#define _stricmp strcasecmp
