#pragma once

#include "ZunColor.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

#include <d3d8.h>

namespace th08
{
struct FormatInfo
{
    D3DFORMAT format;
    i32 bitCount;
    u32 alphaMask;
    u32 redMask;
    u32 greenMask;
    u32 blueMask;
};

class TextHelper
{
  public:
    TextHelper();
    ~TextHelper();

    bool ReleaseBuffer();
    bool AllocateBufferWithFallback(i32 width, i32 height, D3DFORMAT format);
    bool TryAllocateBuffer(i32 width, i32 height, D3DFORMAT format);
    FormatInfo *GetFormatInfo(D3DFORMAT format);
    bool InvertAlpha(i32 unusedX, i32 y, i32 spriteWidth, i32 fontHeight, BOOL useGentleColorFalloff);
    u8 *GetBuffer();
    u32 GetImageWidthInBytes();
    i32 GetHeight();
    HDC GetHDC();
    i32 GetWidth();
    D3DFORMAT GetFormat();
    bool IsAllocated();
    bool CopyTextToSurface(IDirect3DSurface8 *outSurface);

    static void CreateTextBuffer();
    static void ReleaseTextBuffer();
    static void RenderTextToTextureBold(i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontHeight,
                                        i32 fontWidth, COLORREF textColor, COLORREF outlineType, const char *string,
                                        IDirect3DTexture8 *outTexture);
    static void RenderTextToTexture(i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontHeight,
                                    i32 fontWidth, COLORREF textColor, COLORREF outlineType, const char *string,
                                    IDirect3DTexture8 *outTexture);

  private:
    D3DFORMAT format;
    i32 width;
    i32 height;
    u32 imageSizeInBytes;
    i32 imageWidthInBytes;
    HDC hdc;
    HGDIOBJ gdiObj;
    HGDIOBJ gdiObj2;
    u8 *buffer;
};

DIFFABLE_EXTERN(IDirect3DSurface8 *, g_TextBufferSurface)
}; // namespace th08
