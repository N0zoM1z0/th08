#pragma once

#include <d3d8.h>

struct LinuxSurfaceAccess
{
    BYTE *pixels;
    UINT width;
    UINT height;
    UINT pitch;
    D3DFORMAT format;
};

struct LinuxTextureRegionStats
{
    UINT sampledPixels;
    UINT visiblePixels;
    UINT colorfulPixels;
    UINT nearWhitePixels;
    UINT visibleEdgePixels;
    UINT modulatedVisiblePixels;
    UINT modulatedColorfulPixels;
    UINT modulatedNearWhitePixels;
};

struct LinuxFramebufferDeltaStats
{
    UINT sampledPixels;
    UINT changedPixels;
    UINT colorfulChangedPixels;
    UINT chromaticChangedPixels;
    UINT nearWhiteChangedPixels;
    unsigned long long absoluteRgbDifference;
};

bool th08_linux_surface_access(IDirect3DSurface8 *surface, LinuxSurfaceAccess *access, bool readBackbuffer);
void th08_linux_surface_changed(IDirect3DSurface8 *surface);
bool th08_linux_texture_region_stats(IDirect3DTexture8 *texture, float u0, float v0,
                                     float u1, float v1, D3DCOLOR diffuse,
                                     LinuxTextureRegionStats *stats);
bool th08_linux_begin_framebuffer_probe(IDirect3DDevice8 *device, int left, int top,
                                        int right, int bottom);
bool th08_linux_end_framebuffer_probe(IDirect3DDevice8 *device,
                                      LinuxFramebufferDeltaStats *stats);
