#pragma once

#include <d3d8.h>
#include <math.h>

#define D3DX_DEFAULT 0xffffffffu
#define D3DX_FILTER_NONE 1

struct D3DXVECTOR2
{
    FLOAT x, y;
    D3DXVECTOR2() {}
    D3DXVECTOR2(FLOAT x_, FLOAT y_) : x(x_), y(y_) {}
    operator FLOAT *() { return &x; }
    operator const FLOAT *() const { return &x; }
    D3DXVECTOR2 operator+(const D3DXVECTOR2 &v) const { return D3DXVECTOR2(x + v.x, y + v.y); }
    D3DXVECTOR2 operator-(const D3DXVECTOR2 &v) const { return D3DXVECTOR2(x - v.x, y - v.y); }
    D3DXVECTOR2 operator*(FLOAT s) const { return D3DXVECTOR2(x * s, y * s); }
    D3DXVECTOR2 &operator+=(const D3DXVECTOR2 &v) { x += v.x; y += v.y; return *this; }
    D3DXVECTOR2 &operator-=(const D3DXVECTOR2 &v) { x -= v.x; y -= v.y; return *this; }
};

struct D3DXVECTOR3 : public D3DVECTOR
{
    D3DXVECTOR3() {}
    D3DXVECTOR3(FLOAT x_, FLOAT y_, FLOAT z_) { x = x_; y = y_; z = z_; }
    D3DXVECTOR3(const D3DVECTOR &v) { x = v.x; y = v.y; z = v.z; }
    operator FLOAT *() { return &x; }
    operator const FLOAT *() const { return &x; }
    D3DXVECTOR3 operator+() const { return *this; }
    D3DXVECTOR3 operator-() const { return D3DXVECTOR3(-x, -y, -z); }
    D3DXVECTOR3 operator+(const D3DXVECTOR3 &v) const { return D3DXVECTOR3(x + v.x, y + v.y, z + v.z); }
    D3DXVECTOR3 operator-(const D3DXVECTOR3 &v) const { return D3DXVECTOR3(x - v.x, y - v.y, z - v.z); }
    D3DXVECTOR3 operator*(FLOAT s) const { return D3DXVECTOR3(x * s, y * s, z * s); }
    D3DXVECTOR3 operator/(FLOAT s) const { return D3DXVECTOR3(x / s, y / s, z / s); }
    D3DXVECTOR3 &operator+=(const D3DXVECTOR3 &v) { x += v.x; y += v.y; z += v.z; return *this; }
    D3DXVECTOR3 &operator-=(const D3DXVECTOR3 &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    D3DXVECTOR3 &operator*=(FLOAT s) { x *= s; y *= s; z *= s; return *this; }
    D3DXVECTOR3 &operator/=(FLOAT s) { x /= s; y /= s; z /= s; return *this; }
};

inline D3DXVECTOR3 operator*(FLOAT s, const D3DXVECTOR3 &v) { return v * s; }

struct D3DXQUATERNION
{
    FLOAT x, y, z, w;
    D3DXQUATERNION() {}
    D3DXQUATERNION(FLOAT x_, FLOAT y_, FLOAT z_, FLOAT w_) : x(x_), y(y_), z(z_), w(w_) {}
};

struct D3DXMATRIX : public D3DMATRIX
{
    D3DXMATRIX() {}
    operator FLOAT *() { return &_11; }
    operator const FLOAT *() const { return &_11; }
};

typedef struct _D3DXIMAGE_INFO
{
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    D3DFORMAT Format;
    D3DRESOURCETYPE ResourceType;
    DWORD ImageFileFormat;
} D3DXIMAGE_INFO;

D3DXMATRIX *D3DXMatrixIdentity(D3DXMATRIX *);
D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *, const D3DXMATRIX *, const D3DXMATRIX *);
D3DXMATRIX *D3DXMatrixRotationX(D3DXMATRIX *, FLOAT);
D3DXMATRIX *D3DXMatrixRotationY(D3DXMATRIX *, FLOAT);
D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *, FLOAT);
D3DXMATRIX *D3DXMatrixRotationQuaternion(D3DXMATRIX *, const D3DXQUATERNION *);
D3DXMATRIX *D3DXMatrixLookAtLH(D3DXMATRIX *, const D3DXVECTOR3 *, const D3DXVECTOR3 *, const D3DXVECTOR3 *);
D3DXMATRIX *D3DXMatrixPerspectiveFovLH(D3DXMATRIX *, FLOAT, FLOAT, FLOAT, FLOAT);
D3DXVECTOR3 *D3DXVec3Cross(D3DXVECTOR3 *, const D3DXVECTOR3 *, const D3DXVECTOR3 *);
FLOAT D3DXVec3Dot(const D3DXVECTOR3 *, const D3DXVECTOR3 *);
FLOAT D3DXVec3Length(const D3DXVECTOR3 *);
FLOAT D3DXVec3LengthSq(const D3DXVECTOR3 *);
D3DXVECTOR3 *D3DXVec3Normalize(D3DXVECTOR3 *, const D3DXVECTOR3 *);
D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *, const D3DXVECTOR3 *, const D3DXMATRIX *);
D3DXVECTOR3 *D3DXVec3Project(D3DXVECTOR3 *, const D3DXVECTOR3 *, const D3DVIEWPORT8 *,
                             const D3DXMATRIX *, const D3DXMATRIX *, const D3DXMATRIX *);

HRESULT D3DXCreateTexture(IDirect3DDevice8 *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL,
                          IDirect3DTexture8 **);
HRESULT D3DXCreateTextureFromFileInMemoryEx(IDirect3DDevice8 *, LPCVOID, UINT, UINT, UINT, UINT, DWORD,
                                            D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR,
                                            D3DXIMAGE_INFO *, void *, IDirect3DTexture8 **);
HRESULT D3DXLoadSurfaceFromFileInMemory(IDirect3DSurface8 *, const void *, const RECT *, LPCVOID, UINT,
                                        const RECT *, DWORD, D3DCOLOR, D3DXIMAGE_INFO *);
HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *, const void *, const RECT *, IDirect3DSurface8 *,
                                   const void *, const RECT *, DWORD, D3DCOLOR);
