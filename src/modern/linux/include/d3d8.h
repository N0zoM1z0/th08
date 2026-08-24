#pragma once

#include <windows.h>
#include <d3d8types.h>
#include <d3d8caps.h>

#define D3D_SDK_VERSION 120
#define D3DADAPTER_DEFAULT 0
#define D3DCREATE_HARDWARE_VERTEXPROCESSING 0x40
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING 0x20
#define D3D_OK S_OK
#define D3DERR_DEVICENOTRESET ((HRESULT)0x88760869L)
#ifndef D3DPRESENT_RATE_DEFAULT
#define D3DPRESENT_RATE_DEFAULT 0
#endif

class IDirect3DSurface8
{
  public:
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetDesc(D3DSURFACE_DESC *) = 0;
    virtual HRESULT LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) = 0;
    virtual HRESULT UnlockRect() = 0;
    virtual HRESULT GetDC(HDC *) = 0;
    virtual HRESULT ReleaseDC(HDC) = 0;

  protected:
    virtual ~IDirect3DSurface8() {}
};

class IDirect3DTexture8
{
  public:
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual DWORD SetPriority(DWORD) = 0;
    virtual void PreLoad() = 0;
    virtual HRESULT GetLevelDesc(UINT, D3DSURFACE_DESC *) = 0;
    virtual HRESULT GetSurfaceLevel(UINT, IDirect3DSurface8 **) = 0;
    virtual HRESULT LockRect(UINT, D3DLOCKED_RECT *, const RECT *, DWORD) = 0;
    virtual HRESULT UnlockRect(UINT) = 0;

  protected:
    virtual ~IDirect3DTexture8() {}
};

class IDirect3DVertexBuffer8
{
  public:
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT Lock(UINT, UINT, BYTE **, DWORD) = 0;
    virtual HRESULT Unlock() = 0;

  protected:
    virtual ~IDirect3DVertexBuffer8() {}
};

class IDirect3DDevice8
{
  public:
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT TestCooperativeLevel() = 0;
    virtual HRESULT Reset(D3DPRESENT_PARAMETERS *) = 0;
    virtual HRESULT Present(const RECT *, const RECT *, HWND, const RGNDATA *) = 0;
    virtual HRESULT GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface8 **) = 0;
    virtual HRESULT CreateTexture(UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture8 **) = 0;
    virtual HRESULT CreateVertexBuffer(UINT, DWORD, DWORD, D3DPOOL, IDirect3DVertexBuffer8 **) = 0;
    virtual HRESULT CreateRenderTarget(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL,
                                       IDirect3DSurface8 **) = 0;
    virtual HRESULT CreateImageSurface(UINT, UINT, D3DFORMAT, IDirect3DSurface8 **) = 0;
    virtual HRESULT CopyRects(IDirect3DSurface8 *, const RECT *, UINT, IDirect3DSurface8 *, const POINT *) = 0;
    virtual HRESULT BeginScene() = 0;
    virtual HRESULT EndScene() = 0;
    virtual HRESULT Clear(DWORD, const D3DRECT *, DWORD, D3DCOLOR, float, DWORD) = 0;
    virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX *) = 0;
    virtual HRESULT SetViewport(const D3DVIEWPORT8 *) = 0;
    virtual HRESULT GetViewport(D3DVIEWPORT8 *) = 0;
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE, DWORD) = 0;
    virtual HRESULT SetTexture(DWORD, IDirect3DTexture8 *) = 0;
    virtual HRESULT SetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) = 0;
    virtual HRESULT SetVertexShader(DWORD) = 0;
    virtual HRESULT SetStreamSource(UINT, IDirect3DVertexBuffer8 *, UINT) = 0;
    virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE, UINT, UINT) = 0;
    virtual HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT, const void *, UINT) = 0;
    virtual HRESULT GetDeviceCaps(D3DCAPS8 *) = 0;
    virtual HRESULT ResourceManagerDiscardBytes(DWORD) = 0;

  protected:
    virtual ~IDirect3DDevice8() {}
};

class IDirect3D8
{
  public:
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *) = 0;
    virtual HRESULT CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT) = 0;
    virtual HRESULT CreateDevice(UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, IDirect3DDevice8 **) = 0;

  protected:
    virtual ~IDirect3D8() {}
};

typedef IDirect3D8 *LPDIRECT3D8;
typedef IDirect3D8 *PDIRECT3D8;
typedef IDirect3DDevice8 *LPDIRECT3DDEVICE8;
typedef IDirect3DDevice8 *PDIRECT3DDEVICE8;
typedef IDirect3DTexture8 *LPDIRECT3DTEXTURE8;
typedef IDirect3DTexture8 *PDIRECT3DTEXTURE8;
typedef IDirect3DTexture8 *LPDIRECT3DBASETEXTURE8;
typedef IDirect3DSurface8 *LPDIRECT3DSURFACE8;
typedef IDirect3DVertexBuffer8 *LPDIRECT3DVERTEXBUFFER8;

extern "C" IDirect3D8 *Direct3DCreate8(UINT sdkVersion);
