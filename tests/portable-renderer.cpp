#include "modern/linux/linux_compat.hpp"
#include "modern/linux/d3d8_internal.hpp"
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__i386__)
#include <fenv.h>
#include <fpu_control.h>
#endif

namespace
{
IDirect3DDevice8 *device;
IDirect3DSurface8 *backbuffer;

void Require(bool ok, const char *message)
{
    if (!ok) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}

void CheckDeviceFloatEnvironment(IDirect3D8 *d3d, HWND window, D3DPRESENT_PARAMETERS *parameters)
{
#if defined(__i386__)
    fenv_t original, environment;
    Require(fegetenv(&original) == 0, "read caller floating-point environment");
    environment = original;
    environment.__control_word = (environment.__control_word & ~_FPU_EXTENDED) | _FPU_EXTENDED;
    Require(fesetenv(&environment) == 0 && fesetround(FE_DOWNWARD) == 0, "set caller extended precision and rounding");
    IDirect3DDevice8 *created = NULL;
    Require(d3d->CreateDevice(0, D3DDEVTYPE_HAL, window, 0, parameters, &created) == S_OK, "default device for FPU contract");
    Require(fegetenv(&environment) == 0 && (environment.__control_word & _FPU_EXTENDED) == _FPU_SINGLE,
            "device uses retail single-precision x87 mode");
    Require(fegetround() == FE_TONEAREST && fegetexcept() == 0, "default device selects nearest rounding and masks exceptions");
    created->Release();
    environment = original;
    environment.__control_word = (environment.__control_word & ~_FPU_EXTENDED) | _FPU_EXTENDED;
    Require(fesetenv(&environment) == 0 && fesetround(FE_DOWNWARD) == 0, "reset caller environment for preservation");
    Require(d3d->CreateDevice(0, D3DDEVTYPE_HAL, window, D3DCREATE_FPU_PRESERVE, parameters, &created) == S_OK,
            "preserving device for FPU contract");
    Require(fegetenv(&environment) == 0 && (environment.__control_word & _FPU_EXTENDED) == _FPU_EXTENDED &&
            fegetround() == FE_DOWNWARD, "FPU_PRESERVE retains caller precision and rounding");
    created->Release();
    Require(fesetenv(&original) == 0, "restore caller environment after FPU contract");
    puts("PASS: device precision defaults and FPU_PRESERVE");
#else
    puts("SKIP: x87 device precision contracts require i386");
#endif
}

D3DCOLOR Pixel(UINT x, UINT y)
{
    D3DLOCKED_RECT lock;
    Require(backbuffer->LockRect(&lock, NULL, D3DLOCK_READONLY) == S_OK, "readback lock");
    D3DCOLOR pixel;
    memcpy(&pixel, static_cast<BYTE *>(lock.pBits) + y * lock.Pitch + x * 4, 4);
    Require(backbuffer->UnlockRect() == S_OK, "readback unlock");
    return pixel & 0xffffff;
}

void Color(UINT x, UINT y, D3DCOLOR expected, const char *message, int tolerance = 0)
{
    D3DCOLOR actual = Pixel(x, y);
    for (int shift = 0; shift < 24; shift += 8)
        Require(abs(int((actual >> shift) & 255) - int((expected >> shift) & 255)) <= tolerance, message);
}

void Reset(D3DCOLOR color)
{
    D3DVIEWPORT8 viewport = {0, 0, 128, 128, 0.0f, 1.0f};
    device->SetViewport(&viewport);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    device->SetTexture(0, NULL);
    device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
}

void Solid(float left, float top, float right, float bottom, D3DCOLOR color,
           float z = 0.5f, float rhw = 1.0f)
{
    struct Vertex { float x, y, z, rhw; D3DCOLOR color; };
    Vertex vertices[4] = {{left, top, z, rhw, color}, {right, top, z, rhw, color},
                          {left, bottom, z, rhw, color}, {right, bottom, z, rhw, color}};
    device->SetTexture(0, NULL);
    device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    Require(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex)) == S_OK, "solid draw");
}

void Projection()
{
    D3DMATRIX identity, projection;
    memset(&identity, 0, sizeof(identity));
    identity._11 = identity._22 = identity._33 = identity._44 = 1.0f;
    memset(&projection, 0, sizeof(projection));
    projection._11 = projection._22 = projection._34 = 1.0f;
    projection._33 = 0.5f;
    device->SetTransform(D3DTS_WORLD, &identity);
    device->SetTransform(D3DTS_VIEW, &identity);
    device->SetTransform(D3DTS_PROJECTION, &projection);
    D3DVIEWPORT8 viewport = {16, 16, 64, 64, 0.0f, 1.0f};
    device->SetViewport(&viewport);
}

void CheckLocks()
{
    Reset(0xff112233);
    D3DLOCKED_RECT lock;
    Require(backbuffer->LockRect(&lock, NULL, 0) == S_OK, "flag-0 lock");
    D3DCOLOR current;
    memcpy(&current, lock.pBits, 4);
    Require((current & 0xffffff) == 0x112233, "flag-0 lock sees current GPU pixels");
    Require(backbuffer->UnlockRect() == S_OK, "flag-0 unlock");
    Solid(0, 0, 32, 32, 0xff00ff00);
    device->Present(NULL, NULL, NULL, NULL);
    Color(8, 8, 0x00ff00, "flag-0 read does not overwrite the next draw");

    Require(backbuffer->LockRect(&lock, NULL, D3DLOCK_READONLY) == S_OK, "read-only lock");
    Require(backbuffer->UnlockRect() == S_OK, "read-only unlock");
    Solid(0, 0, 32, 32, 0xff0000ff);
    device->Present(NULL, NULL, NULL, NULL);
    Color(8, 8, 0x0000ff, "read-only unlock preserves subsequent rendering");

    RECT rect = {10, 20, 14, 24};
    Require(backbuffer->LockRect(&lock, &rect, 0) == S_OK, "partial write lock");
    D3DCOLOR red = 0xffff0000;
    memcpy(static_cast<BYTE *>(lock.pBits) + lock.Pitch + 4, &red, 4);
    Require(backbuffer->UnlockRect() == S_OK, "partial write unlock");
    Color(11, 21, 0xff0000, "unlocked CPU write is visible immediately");
    Solid(40, 40, 48, 48, 0xffffff00);
    device->Present(NULL, NULL, NULL, NULL);
    Color(11, 21, 0xff0000, "partial write survives presentation");
    Color(44, 44, 0xffff00, "draw after partial write survives presentation");
    Color(100, 100, 0x112233, "partial write preserves untouched pixels");
    puts("PASS: flag-0 reads, read-only locks, immediate partial writes, and later draws");
}

void CheckPerspective()
{
    IDirect3DTexture8 *texture;
    Require(device->CreateTexture(64, 64, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture) == S_OK,
            "gradient texture allocation");
    D3DLOCKED_RECT lock;
    texture->LockRect(0, &lock, NULL, 0);
    for (UINT y = 0; y < 64; ++y)
        for (UINT x = 0; x < 64; ++x)
        {
            D3DCOLOR color = 0xff004000 | (x * 4 << 16) | y * 4;
            memcpy(static_cast<BYTE *>(lock.pBits) + y * lock.Pitch + x * 4, &color, 4);
        }
    texture->UnlockRect(0);
    Reset(0);
    device->SetTexture(0, texture);
    device->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
    device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
    struct Transformed { float x, y, z, rhw, u, v; };
    Transformed screen[4] = {{16,16,0.5f,1,0,0}, {80,16,0.5f,1,1,0},
                             {16,80,0.5f,0.25f,0,1}, {80,80,0.5f,0.25f,1,1}};
    device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);
    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, screen, sizeof(Transformed));
    // At (32,40), barycentric weights are (3/8,1/4,3/8).
    // Reciprocal-depth weighting gives UV (8/23,3/23), hence texel (22,8).
    Color(32, 40, 0x584020, "XYZRHW uses reciprocal-depth texture interpolation");
    texture->LockRect(0, &lock, NULL, D3DLOCK_READONLY);
    texture->UnlockRect(0);
    Color(32, 40, 0x584020, "read-only texture lock preserves its rendered content");

    Reset(0);
    Projection();
    device->SetTexture(0, texture);
    struct Vertex { float x, y, z, u, v; };
    Vertex world[4] = {{-1,1,1,0,0}, {1,1,1,1,0}, {-4,-4,4,0,1}, {4,-4,4,1,1}};
    device->SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, world, sizeof(Vertex));
    Color(32, 40, 0x584020, "XYZ preserves clip W and integer pixel centers");
    device->SetTexture(0, NULL);
    texture->Release();
    puts("PASS: XYZ and XYZRHW perspective interpolation and pixel centers");
}

void CheckClippingAndDepth()
{
    Reset(0);
    Projection();
    struct Vertex { float x, y, z; D3DCOLOR color; };
    Vertex behind[3] = {{-0.8f,0.8f,-1,0xffff0000}, {0.8f,0.8f,-1,0xffff0000},
                        {0,-0.8f,-1,0xffff0000}};
    device->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE);
    device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, behind, sizeof(Vertex));
    for (UINT y = 16; y < 80; y += 8)
        for (UINT x = 16; x < 80; x += 8)
            Color(x, y, 0, "vertices behind the camera are clipped");

    Reset(0);
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
    Solid(16, 16, 80, 80, 0xffff0000, 0.25f);
    Solid(16, 16, 80, 80, 0xff0000ff, 0.75f, 0.25f);
    Color(32, 40, 0xff0000, "XYZRHW retains normalized depth with differing W");
    puts("PASS: negative clip W and reciprocal-depth Z testing");
}

void CheckFogAndBlit()
{
    Reset(0xff010203);
    Projection();
    float start = 2.0f, end = 6.0f;
    DWORD startBits, endBits;
    memcpy(&startBits, &start, 4); memcpy(&endBits, &end, 4);
    device->SetRenderState(D3DRS_FOGENABLE, TRUE);
    device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    device->SetRenderState(D3DRS_FOGCOLOR, 0xffff0000);
    device->SetRenderState(D3DRS_FOGSTART, startBits);
    device->SetRenderState(D3DRS_FOGEND, endBits);
    struct Vertex { float x, y, z; D3DCOLOR color; };
    Vertex vertices[4] = {{-4,4,4,0xffffffff}, {4,4,4,0xffffffff},
                          {-4,-4,4,0xffffffff}, {4,-4,4,0xffffffff}};
    device->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE);
    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
    Color(32, 40, 0xff8080, "linear fog retains view-space distance", 1);
    glViewport(3, 7, 30, 50);
    D3DLOCKED_RECT lock;
    RECT rect = {2, 2, 3, 3};
    backbuffer->LockRect(&lock, &rect, 0);
    D3DCOLOR green = 0xff00ff00;
    memcpy(lock.pBits, &green, 4);
    backbuffer->UnlockRect();
    Color(2, 2, 0x00ff00, "CPU upload ignores current fog and viewport");
    Color(32, 40, 0xff8080, "CPU upload preserves already fogged pixels", 1);
    device->Present(NULL, NULL, NULL, NULL);
    Color(2, 2, 0x00ff00, "presentation preserves CPU upload");
    puts("PASS: fog distance and viewport-independent backbuffer upload");
}
}

int main(int argc, char **argv)
{
    const char *test = argc == 2 ? argv[1] : "all";
    Require(argc <= 2 && (!strcmp(test, "all") || !strcmp(test, "locks") || !strcmp(test, "perspective") ||
                          !strcmp(test, "clipping") || !strcmp(test, "fog") || !strcmp(test, "fpu")), "unknown contract case");
    HWND window = CreateWindowExA(0, "TEST", "TH08 renderer contract", WS_VISIBLE,
                                 0, 0, 640, 480, NULL, NULL, NULL, NULL);
    Require(window != NULL, "SDL window");
    IDirect3D8 *d3d = Direct3DCreate8(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS parameters;
    memset(&parameters, 0, sizeof(parameters));
    parameters.BackBufferWidth = parameters.BackBufferHeight = 128;
    parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    parameters.Windowed = TRUE;
    parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    if (!strcmp(test, "fpu"))
    {
        CheckDeviceFloatEnvironment(d3d, window, &parameters);
        d3d->Release(); DestroyWindow(window);
        return 0;
    }
    Require(d3d->CreateDevice(0, D3DDEVTYPE_HAL, window, 0, &parameters, &device) == S_OK, "OpenGL device");
    device->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
    if (!strcmp(test, "all") || !strcmp(test, "locks")) CheckLocks();
    if (!strcmp(test, "all") || !strcmp(test, "perspective")) CheckPerspective();
    if (!strcmp(test, "all") || !strcmp(test, "clipping")) CheckClippingAndDepth();
    if (!strcmp(test, "all") || !strcmp(test, "fog")) CheckFogAndBlit();
    backbuffer->Release(); device->Release(); d3d->Release(); DestroyWindow(window);
    puts("Portable renderer contracts passed.");
    return 0;
}
