#pragma once

#include <windows.h>

#define DIERR_INPUTLOST ((HRESULT)0x8007001eL)
#define DISCL_FOREGROUND 1
#define DISCL_BACKGROUND 2
#define DISCL_NONEXCLUSIVE 4
#define DISCL_NOWINKEY 8
#define DIEDFL_ATTACHEDONLY 1
#define DIPH_BYID 1
#define DIENUM_STOP 0
#define DIENUM_CONTINUE 1

#define DIK_ESCAPE 0x01
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_5 0x06
#define DIK_6 0x07
#define DIK_7 0x08
#define DIK_8 0x09
#define DIK_9 0x0a
#define DIK_0 0x0b
#define DIK_Q 0x10
#define DIK_R 0x13
#define DIK_P 0x19
#define DIK_RETURN 0x1c
#define DIK_LCONTROL 0x1d
#define DIK_RCONTROL 0x9d
#define DIK_S 0x1f
#define DIK_D 0x20
#define DIK_LSHIFT 0x2a
#define DIK_Z 0x2c
#define DIK_X 0x2d
#define DIK_RSHIFT 0x36
#define DIK_NUMPAD7 0x47
#define DIK_NUMPAD8 0x48
#define DIK_NUMPAD9 0x49
#define DIK_NUMPAD4 0x4b
#define DIK_NUMPAD6 0x4d
#define DIK_NUMPAD1 0x4f
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_HOME 0xc7
#define DIK_UP 0xc8
#define DIK_LEFT 0xcb
#define DIK_RIGHT 0xcd
#define DIK_DOWN 0xd0

typedef struct _DIDATAFORMAT { DWORD dwSize; } DIDATAFORMAT;
typedef struct _DIDEVCAPS {
    DWORD dwSize, dwFlags, dwDevType, dwAxes, dwButtons, dwPOVs, dwFFSamplePeriod,
        dwFFMinTimeResolution, dwFirmwareRevision, dwHardwareRevision, dwFFDriverVersion;
} DIDEVCAPS;
typedef struct _DIDEVICEINSTANCEA {
    DWORD dwSize;
    GUID guidInstance;
    GUID guidProduct;
    DWORD dwDevType;
    CHAR tszInstanceName[MAX_PATH];
    CHAR tszProductName[MAX_PATH];
    GUID guidFFDriver;
    WORD wUsagePage, wUsage;
} DIDEVICEINSTANCEA, *LPDIDEVICEINSTANCEA;
typedef const DIDEVICEINSTANCEA *LPCDIDEVICEINSTANCEA;
typedef LPCDIDEVICEINSTANCEA LPCDIDEVICEINSTANCE;
typedef struct _DIDEVICEOBJECTINSTANCEA {
    DWORD dwSize;
    GUID guidType;
    DWORD dwOfs, dwType, dwFlags;
    CHAR tszName[MAX_PATH];
} DIDEVICEOBJECTINSTANCEA, *LPDIDEVICEOBJECTINSTANCEA;
typedef const DIDEVICEOBJECTINSTANCEA *LPCDIDEVICEOBJECTINSTANCEA;
typedef struct _DIPROPHEADER { DWORD dwSize, dwHeaderSize, dwObj, dwHow; } DIPROPHEADER;
typedef struct _DIPROPRANGE { DIPROPHEADER diph; LONG lMin, lMax; } DIPROPRANGE;

typedef struct _DIJOYSTATE2 {
    LONG lX, lY, lZ, lRx, lRy, lRz;
    LONG rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE rgbButtons[128];
    LONG lVX, lVY, lVZ, lVRx, lVRy, lVRz;
    LONG rglVSlider[2];
    LONG lAX, lAY, lAZ, lARx, lARy, lARz;
    LONG rglASlider[2];
    LONG lFX, lFY, lFZ, lFRx, lFRy, lFRz;
    LONG rglFSlider[2];
} DIJOYSTATE2;

typedef BOOL(CALLBACK *LPDIENUMDEVICESCALLBACKA)(LPCDIDEVICEINSTANCEA, LPVOID);
typedef BOOL(CALLBACK *LPDIENUMDEVICEOBJECTSCALLBACKA)(LPCDIDEVICEOBJECTINSTANCEA, LPVOID);

class IDirectInputDevice8A
{
  public:
    virtual ULONG Release() = 0;
    virtual HRESULT GetCapabilities(DIDEVCAPS *) = 0;
    virtual HRESULT EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) = 0;
    virtual HRESULT GetDeviceState(DWORD, LPVOID) = 0;
    virtual HRESULT SetDataFormat(const DIDATAFORMAT *) = 0;
    virtual HRESULT SetCooperativeLevel(HWND, DWORD) = 0;
    virtual HRESULT SetProperty(REFGUID, const DIPROPHEADER *) = 0;
    virtual HRESULT Acquire() = 0;
    virtual HRESULT Unacquire() = 0;
    virtual HRESULT Poll() = 0;

  protected:
    virtual ~IDirectInputDevice8A() {}
};

class IDirectInput8A
{
  public:
    virtual ULONG Release() = 0;
    virtual HRESULT CreateDevice(REFGUID, IDirectInputDevice8A **, LPVOID) = 0;
    virtual HRESULT EnumDevices(DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD) = 0;

  protected:
    virtual ~IDirectInput8A() {}
};

typedef IDirectInputDevice8A *LPDIRECTINPUTDEVICE8A;
typedef IDirectInput8A *LPDIRECTINPUT8A;

extern const GUID IID_IDirectInput8A;
extern const GUID GUID_SysKeyboard;
extern const GUID DIPROP_RANGE;
extern const DIDATAFORMAT c_dfDIKeyboard;
extern const DIDATAFORMAT c_dfDIJoystick;

HRESULT DirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPVOID);
