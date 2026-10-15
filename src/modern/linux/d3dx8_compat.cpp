#include "d3d8_internal.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <d3dx8.h>

#include <math.h>
#include <fenv.h>
#include <string.h>
#include <vector>
#if defined(__i386__) || defined(__x86_64__)
#include <xmmintrin.h>
#endif

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

// Expansion and high-bit packing preserve every 16-bit surface pixel.
BYTE Expand5(UINT value)
{
    return static_cast<BYTE>((value << 3) | (value >> 2));
}

BYTE Expand6(UINT value)
{
    return static_cast<BYTE>((value << 2) | (value >> 4));
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
        rgba[0] = Expand5((pixel >> 11) & 31);
        rgba[1] = Expand6((pixel >> 5) & 63);
        rgba[2] = Expand5(pixel & 31); rgba[3] = 255; break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = Expand5((pixel >> 10) & 31);
        rgba[1] = Expand5((pixel >> 5) & 31);
        rgba[2] = Expand5(pixel & 31);
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
        pixel = static_cast<WORD>(((rgba[0] >> 3) << 11) |
                                  ((rgba[1] >> 2) << 5) | (rgba[2] >> 3));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        pixel = static_cast<WORD>(((format == D3DFMT_X1R5G5B5 || rgba[3] >= 128) ? 0x8000 : 0) |
                                  ((rgba[0] >> 3) << 10) |
                                  ((rgba[1] >> 3) << 5) | (rgba[2] >> 3));
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

struct SurfaceColor
{
    FLOAT channel[4];
    SurfaceColor() { memset(channel, 0, sizeof(channel)); }
};

struct SurfaceWeight
{
    UINT index;
    FLOAT value;
    SurfaceWeight(UINT i, FLOAT v) : index(i), value(v) {}
};

typedef std::vector<SurfaceWeight> SurfaceWeights;

// D3DX8 integrates each source texel's triangular footprint in destination space.
// Canonical TF_SetupTriangle is at 0x0047D7B9; unflagged edges wrap.
std::vector<SurfaceWeights> TriangleWeights(UINT sourceCount, UINT destinationCount, bool wrap)
{
    FLOAT ratio = static_cast<FLOAT>(destinationCount) / sourceCount;
    FLOAT reciprocal = 0.5f / ratio;
    std::vector<SurfaceWeights> result(sourceCount);
    for (UINT source = 0; source < sourceCount; ++source)
    {
        SurfaceWeights &weights = result[source];
        for (UINT phase = 0; phase < 2; ++phase)
        {
            FLOAT base = static_cast<FLOAT>(source + phase) - 0.5f;
            FLOAT left = base * ratio;
            FLOAT right = left + ratio;
            if (!wrap) { if (left < 0) left = 0; if (right > destinationCount) right = destinationCount; }
            for (LONG pixel = static_cast<LONG>(floorf(left)); pixel < right; ++pixel)
            {
                FLOAT begin = left > pixel ? left : static_cast<FLOAT>(pixel);
                FLOAT end = right < pixel + 1 ? right : static_cast<FLOAT>(pixel + 1);
                if (begin >= end) continue;
                LONG index = pixel;
                if (index < 0) index += destinationCount;
                else if (index >= static_cast<LONG>(destinationCount)) index -= destinationCount;
                FLOAT weight = (begin + end) * reciprocal - base;
                if (!wrap && base < 0) weight = 1;
                else if (!wrap && base + 1 >= sourceCount) weight = 0;
                if (phase != 0) weight = 1 - weight;
                weight *= end - begin;
                if (!weights.empty() && weights.back().index == static_cast<UINT>(index))
                    weights.back().value += weight;
                else weights.push_back(SurfaceWeight(index, weight));
            }
        }
    }
    return result;
}

void SurfaceChannelLayout(D3DFORMAT format, UINT *levels, UINT *shift)
{
    for (UINT c = 0; c < 4; ++c) { levels[c] = 255; shift[c] = 0; }
    if (format == D3DFMT_R5G6B5 || format == D3DFMT_X1R5G5B5 || format == D3DFMT_A1R5G5B5)
    {
        levels[0] = levels[2] = 31; shift[0] = shift[2] = 3;
        levels[1] = format == D3DFMT_R5G6B5 ? 63 : 31;
        shift[1] = format == D3DFMT_R5G6B5 ? 2 : 3;
        levels[3] = 1; shift[3] = 7;
    }
    else if (format == D3DFMT_A4R4G4B4)
        for (UINT c = 0; c < 4; ++c) { levels[c] = 15; shift[c] = 4; }
}

SurfaceColor DecodeSurfaceColor(const BYTE *rgba, D3DFORMAT format, D3DCOLOR colorKey)
{
    UINT levels[4], shift[4]; SurfaceChannelLayout(format, levels, shift);
    SurfaceColor result;
    for (UINT c = 0; c < 4; ++c)
    {
        FLOAT reciprocal = 1.0f / levels[c];
        result.channel[c] = (rgba[c] >> shift[c]) * reciprocal;
    }
    if (colorKey != 0 && (colorKey & 0x00ffffffu) ==
        ((static_cast<DWORD>(rgba[0]) << 16) | (static_cast<DWORD>(rgba[1]) << 8) | rgba[2]))
        result.channel[3] = 0;
    return result;
}

UINT QuantizeSurfaceChannel(FLOAT value, UINT maximum, FLOAT bias)
{
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    // Canonical encoders set RC=truncate and store both arithmetic results as float.
    volatile FLOAT scaled = value * static_cast<FLOAT>(maximum);
    volatile FLOAT rounded = scaled + bias;
    UINT result = static_cast<UINT>(rounded);
    return result > maximum ? maximum : result;
}

void EncodeSurfaceColor(BYTE *destination, D3DFORMAT format, const SurfaceColor &color,
                        DWORD filter, UINT x, UINT y)
{
    UINT levels[4], shift[4], quantized[4]; SurfaceChannelLayout(format, levels, shift);
    // Canonical codec rounding matrices at 0x004C8E58 and 0x004C8ED8.
    static const FLOAT dither[16] = {
        0.96875f,0.46875f,0.84375f,0.34375f, 0.21875f,0.71875f,0.09375f,0.59375f,
        0.78125f,0.28125f,0.90625f,0.40625f, 0.03125f,0.53125f,0.15625f,0.65625f
    };
    FLOAT bias = (filter & 0x80000) != 0 ? dither[(y & 3) * 4 + (x & 3)] : 0.5f;
    for (UINT c = 0; c < 4; ++c) quantized[c] = QuantizeSurfaceChannel(color.channel[c], levels[c], bias);
    WORD word;
    switch (format)
    {
    case D3DFMT_R5G6B5:
        word = static_cast<WORD>((quantized[0] << 11) | (quantized[1] << 5) | quantized[2]);
        memcpy(destination, &word, 2); break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        word = static_cast<WORD>(((format == D3DFMT_X1R5G5B5 ? 1 : quantized[3]) << 15) |
                                (quantized[0] << 10) | (quantized[1] << 5) | quantized[2]);
        memcpy(destination, &word, 2); break;
    case D3DFMT_A4R4G4B4:
        word = static_cast<WORD>((quantized[3] << 12) | (quantized[0] << 8) |
                                (quantized[1] << 4) | quantized[2]);
        memcpy(destination, &word, 2); break;
    default:
        destination[0] = quantized[2]; destination[1] = quantized[1]; destination[2] = quantized[0];
        if (format != D3DFMT_R8G8B8) destination[3] = format == D3DFMT_X8R8G8B8 ? 255 : quantized[3];
        break;
    }
}

class SurfaceRoundingScope
{
    int previous;
public:
    SurfaceRoundingScope() : previous(fegetround()) { fesetround(FE_TOWARDZERO); }
    ~SurfaceRoundingScope() { fesetround(previous); }
};

void FilterSurfaceColors(const std::vector<SurfaceColor> &source, UINT sw, UINT sh,
                         std::vector<SurfaceColor> &destination, UINT dw, UINT dh, DWORD filter)
{
    UINT kind = filter & 0xffff;
    if (kind == 3)
    {
        FLOAT xRatio = static_cast<FLOAT>(sw) / dw;
        FLOAT yRatio = static_cast<FLOAT>(sh) / dh;
        for (UINT y = 0; y < dh; ++y)
            for (UINT x = 0; x < dw; ++x)
            {
                FLOAT px = x * xRatio - 0.5f, py = y * yRatio - 0.5f;
                LONG ix = static_cast<LONG>(floorf(px)), iy = static_cast<LONG>(floorf(py));
                FLOAT fx = px - ix, fy = py - iy;
                LONG xs[2] = {ix, ix + 1}, ys[2] = {iy, iy + 1};
                for (UINT i = 0; i < 2; ++i)
                {
                    if (xs[i] < 0) xs[i] = (filter & 0x10000) == 0 ? sw - 1 : 0;
                    if (xs[i] >= static_cast<LONG>(sw)) xs[i] = (filter & 0x10000) == 0 ? 0 : sw - 1;
                    if (ys[i] < 0) ys[i] = (filter & 0x20000) == 0 ? sh - 1 : 0;
                    if (ys[i] >= static_cast<LONG>(sh)) ys[i] = (filter & 0x20000) == 0 ? 0 : sh - 1;
                }
                for (UINT c = 0; c < 4; ++c)
                {
                    FLOAT topLeft = source[ys[0] * sw + xs[0]].channel[c] * (1 - fx);
                    FLOAT topRight = source[ys[0] * sw + xs[1]].channel[c] * fx;
                    FLOAT bottomLeft = source[ys[1] * sw + xs[0]].channel[c] * (1 - fx);
                    FLOAT bottomRight = source[ys[1] * sw + xs[1]].channel[c] * fx;
                    FLOAT top = (topLeft + topRight) * (1 - fy);
                    FLOAT bottom = (bottomLeft + bottomRight) * fy;
                    destination[y * dw + x].channel[c] = top + bottom;
                }
            }
    }
    else if (kind == 4 || kind == 5)
    {
        std::vector<SurfaceWeights> xWeights = TriangleWeights(sw, dw, (filter & 0x10000) == 0);
        std::vector<SurfaceWeights> yWeights = TriangleWeights(sh, dh, (filter & 0x20000) == 0);
        for (UINT y = 0; y < sh; ++y)
            for (UINT x = 0; x < sw; ++x)
                for (UINT dx = 0; dx < xWeights[x].size(); ++dx)
                    for (UINT dy = 0; dy < yWeights[y].size(); ++dy)
                    {
                        FLOAT weight = xWeights[x][dx].value * yWeights[y][dy].value;
                        if (xWeights[x][dx].value <= 1.0e-5f || yWeights[y][dy].value <= 1.0e-5f) continue;
                        SurfaceColor &pixel = destination[yWeights[y][dy].index * dw + xWeights[x][dx].index];
                        for (UINT c = 0; c < 4; ++c)
                        {
                            FLOAT contribution = weight * source[y * sw + x].channel[c];
                            pixel.channel[c] += contribution;
                        }
                    }
    }
    else for (UINT y = 0; y < dh; ++y)
        for (UINT x = 0; x < dw; ++x)
        {
            UINT sx = kind == 1 ? x : static_cast<UINT>((static_cast<unsigned long long>(x) * sw) / dw);
            UINT sy = kind == 1 ? y : static_cast<UINT>((static_cast<unsigned long long>(y) * sh) / dh);
            if (sx < sw && sy < sh) destination[y * dw + x] = source[sy * sw + sx];
        }
}

HRESULT CopyRgbaToSurface(IDirect3DSurface8 *destinationRaw, const RECT *destinationRectRaw,
                          const BYTE *source, UINT sourceWidth, UINT sourceHeight, UINT sourcePitch,
                          const RECT *sourceRectRaw, D3DCOLOR colorKey,
                          DWORD filter = 2, D3DFORMAT sourceFormat = D3DFMT_A8R8G8B8)
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
    if (!ValidRect(destinationRect) || !ValidRect(sourceRect)) return E_INVALIDARG;
    if (filter == D3DX_DEFAULT) filter = 0x80004;
    UINT filterKind = filter & 0xffff;
    if (filterKind < 1 || filterKind > 5) return E_INVALIDARG;
    UINT destinationWidth = destinationRect.right - destinationRect.left;
    UINT destinationHeight = destinationRect.bottom - destinationRect.top;
    UINT sourceRectWidth = sourceRect.right - sourceRect.left;
    UINT sourceRectHeight = sourceRect.bottom - sourceRect.top;
    UINT destinationBytes = BytesPerPixel(destination.format);
    bool scaled = sourceRectWidth != destinationWidth || sourceRectHeight != destinationHeight;
    if (sourceFormat != destination.format || (filterKind != 2 && scaled))
    {
        std::vector<SurfaceColor> colors(sourceRectWidth * sourceRectHeight);
        for (UINT y = 0; y < sourceRectHeight; ++y)
            for (UINT x = 0; x < sourceRectWidth; ++x)
                colors[y * sourceRectWidth + x] = DecodeSurfaceColor(
                    source + (sourceRect.top + y) * sourcePitch + (sourceRect.left + x) * 4, sourceFormat, colorKey);
        std::vector<SurfaceColor> filtered(destinationWidth * destinationHeight);
        bool box = filterKind == 5 && scaled && colorKey == 0 &&
                   (destinationWidth == sourceRectWidth / 2 || (sourceRectWidth == 1 && destinationWidth == 1)) &&
                   (destinationHeight == sourceRectHeight / 2 || (sourceRectHeight == 1 && destinationHeight == 1));
        if (box)
        {
            UINT levels[4], shift[4]; SurfaceChannelLayout(sourceFormat, levels, shift);
            UINT xCount = sourceRectWidth > 1 ? 2 : 1;
            UINT yCount = sourceRectHeight > 1 ? 2 : 1;
            UINT count = xCount * yCount;
            for (UINT y = 0; y < destinationHeight; ++y)
                for (UINT x = 0; x < destinationWidth; ++x)
                    for (UINT c = 0; c < 4; ++c)
                    {
                        if (sourceFormat == destination.format)
                        {
                            UINT sum = 0;
                            for (UINT dy = 0; dy < yCount; ++dy)
                                for (UINT dx = 0; dx < xCount; ++dx)
                                    sum += source[(sourceRect.top + y * yCount + dy) * sourcePitch +
                                                  (sourceRect.left + x * xCount + dx) * 4 + c] >> shift[c];
                            UINT value = (sum + count / 2) / count;
                            FLOAT reciprocal = 1.0f / levels[c];
                            filtered[y * destinationWidth + x].channel[c] = value * reciprocal;
                        }
                        else
                        {
                            FLOAT rows[2] = {0,0};
                            for (UINT dy = 0; dy < yCount; ++dy)
                                for (UINT dx = 0; dx < xCount; ++dx)
                                    rows[dy] += colors[(y * yCount + dy) * sourceRectWidth + x * xCount + dx].channel[c];
                            filtered[y * destinationWidth + x].channel[c] = (rows[0] + rows[1]) / count;
                        }
                    }
        }
        else FilterSurfaceColors(colors, sourceRectWidth, sourceRectHeight, filtered, destinationWidth, destinationHeight,
                                 scaled ? filter : ((filter & 0xffff0000) | 2));
        {
            SurfaceRoundingScope rounding;
            for (UINT y = 0; y < destinationHeight; ++y)
                for (UINT x = 0; x < destinationWidth; ++x)
                    EncodeSurfaceColor(destination.pixels + (destinationRect.top + y) * destination.pitch +
                        (destinationRect.left + x) * destinationBytes, destination.format, filtered[y * destinationWidth + x], filter, x, y);
        }
        th08_linux_surface_changed(destinationRaw); return S_OK;
    }
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
                    IDirect3DSurface8 *sourceRaw, const RECT *sourceRectRaw, D3DCOLOR colorKey, DWORD filter)
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
                             source.width, source.height, source.width * 4, sourceRectRaw, colorKey, filter, source.format);
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
    // Target D3DX8 doubles the components before the products and subtracts
    // each diagonal term separately. Keep those float rounding steps.
    const FLOAT x2 = 2.0f * q->x, y2 = 2.0f * q->y, z2 = 2.0f * q->z;
    const FLOAT xx = x2 * q->x, xy = y2 * q->x, xz = z2 * q->x;
    const FLOAT yy = y2 * q->y, yz = z2 * q->y, zz = z2 * q->z;
    const FLOAT xw = x2 * q->w, yw = y2 * q->w, zw = z2 * q->w;
    out->_11 = 1.0f - yy - zz; out->_12 = xy + zw; out->_13 = xz - yw; out->_14 = 0;
    out->_21 = xy - zw; out->_22 = 1.0f - xx - zz; out->_23 = yz + xw; out->_24 = 0;
    out->_31 = xz + yw; out->_32 = yz - xw; out->_33 = 1.0f - xx - yy; out->_34 = 0;
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

#if defined(__i386__) || defined(__x86_64__)
__attribute__((target("sse")))
#endif
D3DXVECTOR3 *D3DXVec3Normalize(D3DXVECTOR3 *out, const D3DXVECTOR3 *value)
{
    if (out == NULL || value == NULL) return NULL;
    const D3DXVECTOR3 copy = *value;
#if defined(__i386__) || defined(__x86_64__)
    const __m128 x = _mm_set_ss(copy.x), y = _mm_set_ss(copy.y), z = _mm_set_ss(copy.z);
    const __m128 square = _mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)), _mm_mul_ss(z, z));
    if (_mm_cvtss_f32(square) < 1.4210854715202004e-14f)
        out->x = out->y = out->z = 0.0f;
    else
    {
        // Retail D3DX refines RSQRT once, rounding each SSE operation to float.
        const __m128 estimate = _mm_rsqrt_ss(square);
        const __m128 correction = _mm_sub_ss(_mm_set_ss(3.0f), _mm_mul_ss(_mm_mul_ss(square, estimate), estimate));
        const __m128 scale = _mm_mul_ss(_mm_mul_ss(_mm_set_ss(0.5f), estimate), correction);
        out->x = _mm_cvtss_f32(_mm_mul_ss(scale, x));
        out->y = _mm_cvtss_f32(_mm_mul_ss(scale, y));
        out->z = _mm_cvtss_f32(_mm_mul_ss(scale, z));
    }
#else
    const FLOAT square = D3DXVec3LengthSq(&copy);
    if (square < 1.4210854715202004e-14f)
        out->x = out->y = out->z = 0.0f;
    else
        *out = copy / sqrtf(square);
#endif
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

#if defined(__i386__) || defined(__x86_64__)
__attribute__((target("sse")))
#endif
D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *out, const D3DXVECTOR3 *value, const D3DXMATRIX *matrix)
{
    if (out == NULL || value == NULL || matrix == NULL) return NULL;
#if defined(__i386__) || defined(__x86_64__)
    const __m128 xy = _mm_add_ps(
        _mm_mul_ps(_mm_set1_ps(value->x), _mm_loadu_ps(&matrix->_11)),
        _mm_mul_ps(_mm_set1_ps(value->y), _mm_loadu_ps(&matrix->_21)));
    const __m128 zw = _mm_add_ps(
        _mm_mul_ps(_mm_set1_ps(value->z), _mm_loadu_ps(&matrix->_31)),
        _mm_loadu_ps(&matrix->_41));
    const __m128 transformed = _mm_add_ps(xy, zw);
    // Target D3DX8 refines RCPPS once, then multiplies by the W reciprocal.
    // Preserve the same reciprocal rounding for affine and projective inputs.
    const __m128 estimate = _mm_rcp_ps(transformed);
    const __m128 reciprocal = _mm_sub_ps(_mm_add_ps(estimate, estimate),
        _mm_mul_ps(_mm_mul_ps(estimate, transformed), estimate));
    const __m128 result = _mm_mul_ps(transformed,
        _mm_shuffle_ps(reciprocal, reciprocal, _MM_SHUFFLE(3, 3, 3, 3)));
    FLOAT coordinates[4];
    _mm_storeu_ps(coordinates, result);
    out->x = coordinates[0]; out->y = coordinates[1]; out->z = coordinates[2];
#else
    FLOAT x = value->x * matrix->_11 + value->y * matrix->_21 + value->z * matrix->_31 + matrix->_41;
    FLOAT y = value->x * matrix->_12 + value->y * matrix->_22 + value->z * matrix->_32 + matrix->_42;
    FLOAT z = value->x * matrix->_13 + value->y * matrix->_23 + value->z * matrix->_33 + matrix->_43;
    FLOAT w = value->x * matrix->_14 + value->y * matrix->_24 + value->z * matrix->_34 + matrix->_44;
    if (fabsf(w) > 1.0e-8f) { x /= w; y /= w; z /= w; }
    out->x = x; out->y = y; out->z = z;
#endif
    return out;
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
                                            D3DFORMAT format, D3DPOOL pool, DWORD filter, DWORD, D3DCOLOR colorKey,
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
                                       image->pitch, NULL, colorKey, filter);
            surface->Release();
        }
        if (FAILED(status)) { (*result)->Release(); *result = NULL; }
    }
    SDL_FreeSurface(image); return status;
}

HRESULT D3DXLoadSurfaceFromFileInMemory(IDirect3DSurface8 *destination, const void *, const RECT *destinationRect,
                                        LPCVOID data, UINT size, const RECT *sourceRect, DWORD filter, D3DCOLOR colorKey,
                                        D3DXIMAGE_INFO *info)
{
    SDL_Surface *image = LoadImage(data, size); if (image == NULL) return E_FAIL;
    SetImageInfo(info, image->w, image->h, D3DFMT_A8R8G8B8);
    HRESULT status = CopyRgbaToSurface(destination, destinationRect, static_cast<const BYTE *>(image->pixels),
                                       image->w, image->h, image->pitch, sourceRect, colorKey, filter);
    SDL_FreeSurface(image); return status;
}

HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *destination, const void *, const RECT *destinationRect,
                                   IDirect3DSurface8 *source, const void *, const RECT *sourceRect,
                                   DWORD filter, D3DCOLOR colorKey)
{ return CopySurface(destination, destinationRect, source, sourceRect, colorKey, filter); }
