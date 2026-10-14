#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TH08_MODERN_LINUX
#include <SDL.h>
#include <d3dx8.h>
#else
extern "C" HRESULT WINAPI D3DXLoadSurfaceFromSurface(
    IDirect3DSurface8 *, const PALETTEENTRY *, const RECT *,
    IDirect3DSurface8 *, const PALETTEENTRY *, const RECT *, DWORD, D3DCOLOR);
#endif

static void require(HRESULT result, const char *step)
{
    if (FAILED(result)) {
        fprintf(stderr, "Failed %s: %08lx\n", step, (unsigned long)result);
        exit(1);
    }
}

static DWORD sample(unsigned x, unsigned y, unsigned width, unsigned height, int pattern)
{
    if (pattern == 0) return ((x + y) & 1) ? 0xffffffffu : 0;
    if (pattern == 1)
        return ((x * 255 / (width - 1)) << 24) |
               ((y * 255 / (height - 1)) << 16) |
               (((x + y) * 255 / (width + height - 2)) << 8) | 0x35;
    if (pattern == 2) return x == width / 2 && y == height / 2 ? 0xffffffffu : 0;
    return x < width / 2 ? 0xffe34197u : 0x0027be51u;
}

static void fill(IDirect3DSurface8 *surface, unsigned width, unsigned height,
                 bool shortFormat, int pattern)
{
    D3DLOCKED_RECT lock;
    require(surface->LockRect(&lock, NULL, 0), "source lock");
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            DWORD rgba = sample(x, y, width, height, pattern);
            WORD word = (WORD)(((rgba >> 16) & 0x8000) |
                               ((rgba >> 9) & 0x7c00) |
                               ((rgba >> 6) & 0x03e0) | ((rgba >> 3) & 0x001f));
            memcpy((char *)lock.pBits + y * lock.Pitch + x * (shortFormat ? 2 : 4),
                   shortFormat ? (void *)&word : (void *)&rgba, shortFormat ? 2 : 4);
        }
    }
    require(surface->UnlockRect(), "source unlock");
}

int main()
{
    HWND window;
#ifdef TH08_MODERN_LINUX
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    window = (HWND)SDL_CreateWindow("Surface scaling probe", 0, 0, 64, 64,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
#else
    window = CreateWindowA("STATIC", "Surface scaling probe", WS_OVERLAPPEDWINDOW,
                           0, 0, 64, 64, NULL, NULL, GetModuleHandle(NULL), NULL);
#endif
    if (!window) return 1;
    IDirect3D8 *api = Direct3DCreate8(D3D_SDK_VERSION);
    if (!api) return 1;
    D3DPRESENT_PARAMETERS parameters;
    memset(&parameters, 0, sizeof(parameters));
    parameters.Windowed = TRUE;
    parameters.hDeviceWindow = window;
    parameters.BackBufferWidth = 32;
    parameters.BackBufferHeight = 32;
    parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    parameters.BackBufferCount = 1;
    parameters.SwapEffect = D3DSWAPEFFECT_COPY;
    IDirect3DDevice8 *device;
    require(api->CreateDevice(0, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                              &parameters, &device), "device");
    const unsigned sizes[4][4] = {{8,8,8,8}, {8,8,4,4}, {4,4,8,8}, {9,7,4,3}};
    printf("[\n");
    bool first = true;
    for (int format = 0; format < 2; ++format) {
        bool shortFormat = format == 0;
        D3DFORMAT dxFormat = shortFormat ? D3DFMT_A1R5G5B5 : D3DFMT_A8R8G8B8;
        for (int shape = 0; shape < 4; ++shape) {
            unsigned sw = sizes[shape][0], sh = sizes[shape][1];
            unsigned dw = sizes[shape][2], dh = sizes[shape][3];
            IDirect3DSurface8 *source, *destination;
            require(device->CreateImageSurface(sw, sh, dxFormat, &source), "source surface");
            require(device->CreateImageSurface(dw, dh, dxFormat, &destination), "destination surface");
            for (int pattern = 0; pattern < 4; ++pattern) {
                fill(source, sw, sh, shortFormat, pattern);
                for (DWORD filter = 1; filter <= 5; ++filter) {
                    fill(destination, dw, dh, shortFormat, 1);
                    HRESULT result = D3DXLoadSurfaceFromSurface(destination, NULL, NULL,
                                                                source, NULL, NULL, filter, 0);
                    printf("%s{\"format\":%d,\"shape\":%d,\"pattern\":%d,\"filter\":%lu,"
                           "\"hresult\":%lu,\"pixels\":[", first ? "" : ",\n",
                           format, shape, pattern, (unsigned long)filter, (unsigned long)(DWORD)result);
                    first = false;
                    D3DLOCKED_RECT lock;
                    require(destination->LockRect(&lock, NULL, D3DLOCK_READONLY), "destination lock");
                    for (unsigned y = 0; y < dh; ++y) {
                        for (unsigned x = 0; x < dw; ++x) {
                            DWORD pixel = 0;
                            memcpy(&pixel, (char *)lock.pBits + y * lock.Pitch + x * (shortFormat ? 2 : 4),
                                   shortFormat ? 2 : 4);
                            printf("%s%lu", x || y ? "," : "", (unsigned long)pixel);
                        }
                    }
                    require(destination->UnlockRect(), "destination unlock");
                    printf("]}");
                }
            }
            source->Release();
            destination->Release();
        }
    }
    printf("\n]\n");
    device->Release();
    api->Release();
    return 0;
}
