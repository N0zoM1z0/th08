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

bool th08_linux_surface_access(IDirect3DSurface8 *surface, LinuxSurfaceAccess *access, bool readBackbuffer);
void th08_linux_surface_changed(IDirect3DSurface8 *surface);
