#include "d3d8_internal.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <d3dx8.h>

#include <math.h>
#include <string.h>
#include <vector>

namespace
{
UINT BytesPerPixel(D3DFORMAT format)
{
    switch (format)
    {
    case D3DFMT_R8G8B8: return 3;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4: return 2;
    default: return 4;
    }
}

void DecodePixel(const BYTE *source, D3DFORMAT format, BYTE *rgba)
{
    WORD pixel;
    switch (format)
    {
    case D3DFMT_R8G8B8:
        rgba[0] = source[2]; rgba[1] = source[1]; rgba[2] = source[0]; rgba[3] = 255; break;
    case D3DFMT_R5G6B5:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = static_cast<BYTE>(((pixel >> 11) & 31) * 255 / 31);
        rgba[1] = static_cast<BYTE>(((pixel >> 5) & 63) * 255 / 63);
        rgba[2] = static_cast<BYTE>((pixel & 31) * 255 / 31); rgba[3] = 255; break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = static_cast<BYTE>(((pixel >> 10) & 31) * 255 / 31);
        rgba[1] = static_cast<BYTE>(((pixel >> 5) & 31) * 255 / 31);
        rgba[2] = static_cast<BYTE>((pixel & 31) * 255 / 31);
        rgba[3] = format == D3DFMT_A1R5G5B5 && !(pixel & 0x8000) ? 0 : 255; break;
    case D3DFMT_A4R4G4B4:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = static_cast<BYTE>(((pixel >> 8) & 15) * 17);
        rgba[1] = static_cast<BYTE>(((pixel >> 4) & 15) * 17);
        rgba[2] = static_cast<BYTE>((pixel & 15) * 17);
        rgba[3] = static_cast<BYTE>(((pixel >> 12) & 15) * 17); break;
    default:
        rgba[0] = source[2]; rgba[1] = source[1]; rgba[2] = source[0];
        rgba[3] = format == D3DFMT_X8R8G8B8 ? 255 : source[3]; break;
    }
}

void EncodePixel(BYTE *destination, D3DFORMAT format, const BYTE *rgba)
{
    WORD pixel;
    switch (format)
    {
    case D3DFMT_R8G8B8:
        destination[0] = rgba[2]; destination[1] = rgba[1]; destination[2] = rgba[0]; break;
    case D3DFMT_R5G6B5:
        pixel = static_cast<WORD>(((rgba[0] * 31 / 255) << 11) |
                                  ((rgba[1] * 63 / 255) << 5) | (rgba[2] * 31 / 255));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        pixel = static_cast<WORD>(((format == D3DFMT_X1R5G5B5 || rgba[3] >= 128) ? 0x8000 : 0) |
                                  ((rgba[0] * 31 / 255) << 10) |
                                  ((rgba[1] * 31 / 255) << 5) | (rgba[2] * 31 / 255));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    case D3DFMT_A4R4G4B4:
        pixel = static_cast<WORD>(((rgba[3] >> 4) << 12) | ((rgba[0] >> 4) << 8) |
                                  ((rgba[1] >> 4) << 4) | (rgba[2] >> 4));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    default:
        destination[0] = rgba[2]; destination[1] = rgba[1]; destination[2] = rgba[0];
        destination[3] = format == D3DFMT_X8R8G8B8 ? 255 : rgba[3]; break;
    }
}

RECT FullRect(UINT width, UINT height)
{
    RECT rect; rect.left = 0; rect.top = 0; rect.right = width; rect.bottom = height; return rect;
}

bool ValidRect(const RECT &rect)
{ return rect.left >= 0 && rect.top >= 0 && rect.right > rect.left && rect.bottom > rect.top; }

HRESULT CopyRgbaToSurface(IDirect3DSurface8 *destinationRaw, const RECT *destinationRectRaw,
                          const BYTE *source, UINT sourceWidth, UINT sourceHeight, UINT sourcePitch,
                          const RECT *sourceRectRaw, D3DCOLOR colorKey)
{
    LinuxSurfaceAccess destination;
    if (!th08_linux_surface_access(destinationRaw, &destination, false) || destination.pixels == NULL)
        return E_INVALIDARG;
    RECT destinationRect = destinationRectRaw != NULL ? *destinationRectRaw : FullRect(destination.width, destination.height);
    RECT sourceRect = sourceRectRaw != NULL ? *sourceRectRaw : FullRect(sourceWidth, sourceHeight);
    if (!ValidRect(destinationRect) || !ValidRect(sourceRect)) return E_INVALIDARG;
    if (destinationRect.right > static_cast<LONG>(destination.width)) destinationRect.right = destination.width;
    if (destinationRect.bottom > static_cast<LONG>(destination.height)) destinationRect.bottom = destination.height;
    if (sourceRect.right > static_cast<LONG>(sourceWidth)) sourceRect.right = sourceWidth;
    if (sourceRect.bottom > static_cast<LONG>(sourceHeight)) sourceRect.bottom = sourceHeight;
    UINT destinationWidth = destinationRect.right - destinationRect.left;
    UINT destinationHeight = destinationRect.bottom - destinationRect.top;
    UINT sourceRectWidth = sourceRect.right - sourceRect.left;
    UINT sourceRectHeight = sourceRect.bottom - sourceRect.top;
    UINT destinationBytes = BytesPerPixel(destination.format);
    for (UINT y = 0; y < destinationHeight; ++y)
    {
        UINT sourceY = sourceRect.top + static_cast<UINT>((static_cast<unsigned long long>(y) * sourceRectHeight) / destinationHeight);
        for (UINT x = 0; x < destinationWidth; ++x)
        {
            UINT sourceX = sourceRect.left + static_cast<UINT>((static_cast<unsigned long long>(x) * sourceRectWidth) / destinationWidth);
            BYTE rgba[4]; memcpy(rgba, source + sourceY * sourcePitch + sourceX * 4, 4);
            if (colorKey != 0 && (colorKey & 0x00ffffffu) ==
                                 ((static_cast<DWORD>(rgba[0]) << 16) | (static_cast<DWORD>(rgba[1]) << 8) | rgba[2]))
                rgba[3] = 0;
            EncodePixel(destination.pixels + (destinationRect.top + y) * destination.pitch +
                        (destinationRect.left + x) * destinationBytes, destination.format, rgba);
        }
    }
    th08_linux_surface_changed(destinationRaw); return S_OK;
}

HRESULT CopySurface(IDirect3DSurface8 *destinationRaw, const RECT *destinationRectRaw,
                    IDirect3DSurface8 *sourceRaw, const RECT *sourceRectRaw, D3DCOLOR colorKey)
{
    LinuxSurfaceAccess source;
    if (!th08_linux_surface_access(sourceRaw, &source, true) || source.pixels == NULL) return E_INVALIDARG;
    std::vector<BYTE> rgba(source.width * source.height * 4);
    UINT sourceBytes = BytesPerPixel(source.format);
    for (UINT y = 0; y < source.height; ++y)
        for (UINT x = 0; x < source.width; ++x)
            DecodePixel(source.pixels + y * source.pitch + x * sourceBytes, source.format,
                        &rgba[(y * source.width + x) * 4]);
    return CopyRgbaToSurface(destinationRaw, destinationRectRaw, rgba.empty() ? NULL : &rgba[0],
                             source.width, source.height, source.width * 4, sourceRectRaw, colorKey);
}

SDL_Surface *LoadImage(LPCVOID data, UINT size)
{
    if (data == NULL || size == 0) return NULL;
    SDL_RWops *stream = SDL_RWFromConstMem(data, size);
    if (stream == NULL) return NULL;
    SDL_Surface *loaded = IMG_Load_RW(stream, 1);
    if (loaded == NULL) return NULL;
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded); return rgba;
}

void SetImageInfo(D3DXIMAGE_INFO *info, UINT width, UINT height, D3DFORMAT format)
{
    if (info == NULL) return;
    info->Width = width; info->Height = height; info->Depth = 1; info->MipLevels = 1; info->Format = format;
}
} // namespace

D3DXMATRIX *D3DXMatrixIdentity(D3DXMATRIX *out)
{
    if (out == NULL) return NULL;
    memset(out, 0, sizeof(*out)); out->_11 = out->_22 = out->_33 = out->_44 = 1.0f; return out;
}

D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *out, const D3DXMATRIX *left, const D3DXMATRIX *right)
{
    if (out == NULL || left == NULL || right == NULL) return NULL;
    D3DXMATRIX result;
    const FLOAT *a = *left, *b = *right;
    FLOAT *r = result;
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            r[row * 4 + column] = a[row * 4] * b[column] + a[row * 4 + 1] * b[4 + column] +
                                  a[row * 4 + 2] * b[8 + column] + a[row * 4 + 3] * b[12 + column];
    *out = result; return out;
}

D3DXMATRIX *D3DXMatrixRotationX(D3DXMATRIX *out, FLOAT angle)
{
    D3DXMatrixIdentity(out); if (out == NULL) return NULL;
    FLOAT cosine = cosf(angle), sine = sinf(angle);
    out->_22 = cosine; out->_23 = sine; out->_32 = -sine; out->_33 = cosine; return out;
}

D3DXMATRIX *D3DXMatrixRotationY(D3DXMATRIX *out, FLOAT angle)
{
    D3DXMatrixIdentity(out); if (out == NULL) return NULL;
    FLOAT cosine = cosf(angle), sine = sinf(angle);
    out->_11 = cosine; out->_13 = -sine; out->_31 = sine; out->_33 = cosine; return out;
}

D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *out, FLOAT angle)
{
    D3DXMatrixIdentity(out); if (out == NULL) return NULL;
    FLOAT cosine = cosf(angle), sine = sinf(angle);
    out->_11 = cosine; out->_12 = sine; out->_21 = -sine; out->_22 = cosine; return out;
}

D3DXMATRIX *D3DXMatrixRotationQuaternion(D3DXMATRIX *out, const D3DXQUATERNION *q)
{
    if (out == NULL || q == NULL) return NULL;
    out->_11 = 1 - 2 * (q->y * q->y + q->z * q->z);
    out->_12 = 2 * (q->x * q->y + q->z * q->w); out->_13 = 2 * (q->x * q->z - q->y * q->w); out->_14 = 0;
    out->_21 = 2 * (q->x * q->y - q->z * q->w);
    out->_22 = 1 - 2 * (q->x * q->x + q->z * q->z); out->_23 = 2 * (q->y * q->z + q->x * q->w); out->_24 = 0;
    out->_31 = 2 * (q->x * q->z + q->y * q->w); out->_32 = 2 * (q->y * q->z - q->x * q->w);
    out->_33 = 1 - 2 * (q->x * q->x + q->y * q->y); out->_34 = 0;
    out->_41 = out->_42 = out->_43 = 0; out->_44 = 1; return out;
}

D3DXVECTOR3 *D3DXVec3Cross(D3DXVECTOR3 *out, const D3DXVECTOR3 *left, const D3DXVECTOR3 *right)
{
    if (out == NULL || left == NULL || right == NULL) return NULL;
    D3DXVECTOR3 result(left->y * right->z - left->z * right->y,
                       left->z * right->x - left->x * right->z,
                       left->x * right->y - left->y * right->x);
    *out = result; return out;
}

FLOAT D3DXVec3Dot(const D3DXVECTOR3 *left, const D3DXVECTOR3 *right)
{ return left != NULL && right != NULL ? left->x * right->x + left->y * right->y + left->z * right->z : 0.0f; }

FLOAT D3DXVec3LengthSq(const D3DXVECTOR3 *value) { return D3DXVec3Dot(value, value); }
FLOAT D3DXVec3Length(const D3DXVECTOR3 *value) { return sqrtf(D3DXVec3LengthSq(value)); }

D3DXVECTOR3 *D3DXVec3Normalize(D3DXVECTOR3 *out, const D3DXVECTOR3 *value)
{
    if (out == NULL || value == NULL) return NULL;
    FLOAT length = D3DXVec3Length(value);
    if (length > 1.0e-8f) { D3DXVECTOR3 copy = *value; *out = copy / length; }
    else out->x = out->y = out->z = 0.0f;
    return out;
}

D3DXMATRIX *D3DXMatrixLookAtLH(D3DXMATRIX *out, const D3DXVECTOR3 *eye,
                               const D3DXVECTOR3 *at, const D3DXVECTOR3 *up)
{
    if (out == NULL || eye == NULL || at == NULL || up == NULL) return NULL;
    D3DXVECTOR3 zaxis, xaxis, yaxis, direction = *at - *eye;
    D3DXVec3Normalize(&zaxis, &direction); D3DXVec3Cross(&xaxis, up, &zaxis);
    D3DXVec3Normalize(&xaxis, &xaxis); D3DXVec3Cross(&yaxis, &zaxis, &xaxis);
    out->_11 = xaxis.x; out->_12 = yaxis.x; out->_13 = zaxis.x; out->_14 = 0;
    out->_21 = xaxis.y; out->_22 = yaxis.y; out->_23 = zaxis.y; out->_24 = 0;
    out->_31 = xaxis.z; out->_32 = yaxis.z; out->_33 = zaxis.z; out->_34 = 0;
    out->_41 = -D3DXVec3Dot(&xaxis, eye); out->_42 = -D3DXVec3Dot(&yaxis, eye);
    out->_43 = -D3DXVec3Dot(&zaxis, eye); out->_44 = 1; return out;
}

D3DXMATRIX *D3DXMatrixPerspectiveFovLH(D3DXMATRIX *out, FLOAT fov, FLOAT aspect, FLOAT nearZ, FLOAT farZ)
{
    if (out == NULL || aspect == 0 || farZ == nearZ) return NULL;
    memset(out, 0, sizeof(*out)); FLOAT yScale = 1.0f / tanf(fov * 0.5f);
    out->_11 = yScale / aspect; out->_22 = yScale; out->_33 = farZ / (farZ - nearZ);
    out->_34 = 1.0f; out->_43 = -nearZ * farZ / (farZ - nearZ); return out;
}

D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *out, const D3DXVECTOR3 *value, const D3DXMATRIX *matrix)
{
    if (out == NULL || value == NULL || matrix == NULL) return NULL;
    FLOAT x = value->x * matrix->_11 + value->y * matrix->_21 + value->z * matrix->_31 + matrix->_41;
    FLOAT y = value->x * matrix->_12 + value->y * matrix->_22 + value->z * matrix->_32 + matrix->_42;
    FLOAT z = value->x * matrix->_13 + value->y * matrix->_23 + value->z * matrix->_33 + matrix->_43;
    FLOAT w = value->x * matrix->_14 + value->y * matrix->_24 + value->z * matrix->_34 + matrix->_44;
    if (fabsf(w) > 1.0e-8f) { x /= w; y /= w; z /= w; }
    out->x = x; out->y = y; out->z = z; return out;
}

D3DXVECTOR3 *D3DXVec3Project(D3DXVECTOR3 *out, const D3DXVECTOR3 *value, const D3DVIEWPORT8 *viewport,
                             const D3DXMATRIX *projection, const D3DXMATRIX *view, const D3DXMATRIX *world)
{
    if (out == NULL || value == NULL || viewport == NULL || projection == NULL || view == NULL || world == NULL) return NULL;
    D3DXMATRIX worldView, transform; D3DXVECTOR3 projected;
    D3DXMatrixMultiply(&worldView, world, view); D3DXMatrixMultiply(&transform, &worldView, projection);
    D3DXVec3TransformCoord(&projected, value, &transform);
    out->x = viewport->X + (projected.x + 1.0f) * viewport->Width * 0.5f;
    out->y = viewport->Y + (1.0f - projected.y) * viewport->Height * 0.5f;
    out->z = viewport->MinZ + projected.z * (viewport->MaxZ - viewport->MinZ); return out;
}

HRESULT D3DXCreateTexture(IDirect3DDevice8 *device, UINT width, UINT height, UINT levels, DWORD usage,
                          D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8 **result)
{
    if (device == NULL) return E_INVALIDARG;
    return device->CreateTexture(width, height, levels, usage, format, pool, result);
}

HRESULT D3DXCreateTextureFromFileInMemoryEx(IDirect3DDevice8 *device, LPCVOID data, UINT size,
                                            UINT width, UINT height, UINT levels, DWORD usage,
                                            D3DFORMAT format, D3DPOOL pool, DWORD, DWORD, D3DCOLOR colorKey,
                                            D3DXIMAGE_INFO *info, void *, IDirect3DTexture8 **result)
{
    if (device == NULL || result == NULL) return E_INVALIDARG;
    SDL_Surface *image = LoadImage(data, size); if (image == NULL) return E_FAIL;
    UINT textureWidth = width == 0 || width == D3DX_DEFAULT ? image->w : width;
    UINT textureHeight = height == 0 || height == D3DX_DEFAULT ? image->h : height;
    if (format == D3DFMT_UNKNOWN) format = D3DFMT_A8R8G8B8;
    SetImageInfo(info, image->w, image->h, format);
    HRESULT status = device->CreateTexture(textureWidth, textureHeight, levels, usage, format, pool, result);
    if (SUCCEEDED(status))
    {
        IDirect3DSurface8 *surface = NULL; status = (*result)->GetSurfaceLevel(0, &surface);
        if (SUCCEEDED(status))
        {
            status = CopyRgbaToSurface(surface, NULL, static_cast<const BYTE *>(image->pixels), image->w, image->h,
                                       image->pitch, NULL, colorKey);
            surface->Release();
        }
        if (FAILED(status)) { (*result)->Release(); *result = NULL; }
    }
    SDL_FreeSurface(image); return status;
}

HRESULT D3DXLoadSurfaceFromFileInMemory(IDirect3DSurface8 *destination, const void *, const RECT *destinationRect,
                                        LPCVOID data, UINT size, const RECT *sourceRect, DWORD, D3DCOLOR colorKey,
                                        D3DXIMAGE_INFO *info)
{
    SDL_Surface *image = LoadImage(data, size); if (image == NULL) return E_FAIL;
    SetImageInfo(info, image->w, image->h, D3DFMT_A8R8G8B8);
    HRESULT status = CopyRgbaToSurface(destination, destinationRect, static_cast<const BYTE *>(image->pixels),
                                       image->w, image->h, image->pitch, sourceRect, colorKey);
    SDL_FreeSurface(image); return status;
}

HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *destination, const void *, const RECT *destinationRect,
                                   IDirect3DSurface8 *source, const void *, const RECT *sourceRect,
                                   DWORD, D3DCOLOR colorKey)
{ return CopySurface(destination, destinationRect, source, sourceRect, colorKey); }
