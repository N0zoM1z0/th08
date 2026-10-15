#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TH08_MODERN_LINUX
#include <SDL.h>
#include <SDL_opengl.h>
#endif

struct Vertex { float x, y, z, rhw; DWORD color; float u, v; };

static void check(HRESULT result)
{
    if (FAILED(result)) { fprintf(stderr, "D3D call failed: %08lx\n", (unsigned long)result); exit(1); }
}

int main()
{
    HWND window;
#ifdef TH08_MODERN_LINUX
    if (SDL_Init(SDL_INIT_VIDEO)) return 1;
    window = reinterpret_cast<HWND>(SDL_CreateWindow("Texture filter probe", 0, 0, 64, 64,
                                                    SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN));
#else
    window = CreateWindowA("STATIC", "Texture filter probe", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           0, 0, 64, 64, NULL, NULL, GetModuleHandle(NULL), NULL);
#endif
    if (!window) return 1;
    IDirect3D8 *api = Direct3DCreate8(D3D_SDK_VERSION);
    if (!api) return 1;
    D3DPRESENT_PARAMETERS parameters; memset(&parameters, 0, sizeof(parameters));
    parameters.Windowed = TRUE; parameters.hDeviceWindow = window;
    parameters.BackBufferWidth = parameters.BackBufferHeight = 32;
    parameters.BackBufferFormat = D3DFMT_X8R8G8B8; parameters.BackBufferCount = 1;
    parameters.SwapEffect = D3DSWAPEFFECT_COPY; parameters.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    IDirect3DDevice8 *device;
    check(api->CreateDevice(0, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &device));
#ifdef TH08_MODERN_LINUX
    const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    const bool clipControl = SDL_GL_ExtensionSupported("GL_ARB_clip_control") != SDL_FALSE;
    printf("OpenGL: %s; ARB_clip_control: %s\n", version, clipControl ? "available" : "unavailable");
    if (getenv("TH08_RENDERER_REQUIRE_LEGACY_GL") && (clipControl || strncmp(version, "2.1 ", 4))) return 2;
#endif
    check(device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1));
    check(device->SetRenderState(D3DRS_ZENABLE, FALSE));
    check(device->SetRenderState(D3DRS_LIGHTING, FALSE));
    check(device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    check(device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    check(device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    check(device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
    check(device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
    check(device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
    check(device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
    check(device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
    check(device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
    const D3DFORMAT formats[] = {D3DFMT_A1R5G5B5, D3DFMT_R5G6B5, D3DFMT_A4R4G4B4, D3DFMT_A8R8G8B8};
    bool first = true;
    printf("[\n");
    for (int kind = 0; kind < 4; ++kind)
    {
        IDirect3DTexture8 *texture;
        check(device->CreateTexture(4, 4, 1, 0, formats[kind], D3DPOOL_MANAGED, &texture));
        D3DLOCKED_RECT lock; check(texture->LockRect(0, &lock, NULL, 0));
        for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 4; ++x)
        {
            unsigned r = (x * 9 + y * 3) & 31, g = (x * 5 + y * 11) & 31;
            unsigned b = (x * 13 + y * 7) & 31, a = (x * 3 + y * 5) & 15;
            DWORD word = kind == 0 ? ((a >= 8 ? 0x8000 : 0) | r << 10 | g << 5 | b) :
                         kind == 1 ? (r << 11 | ((g << 1) | (g >> 4)) << 5 | b) :
                         kind == 2 ? (a << 12 | (r >> 1) << 8 | (g >> 1) << 4 | (b >> 1)) :
                         (a * 17 << 24 | ((r << 3) | (r >> 2)) << 16 |
                          ((g << 3) | (g >> 2)) << 8 | (b << 3) | (b >> 2));
            unsigned bytes = kind == 3 ? 4 : 2;
            memcpy(static_cast<char *>(lock.pBits) + y * lock.Pitch + x * bytes, &word, bytes);
        }
        check(texture->UnlockRect(0)); check(device->SetTexture(0, texture));
        for (int filter = 1; filter <= 2; ++filter) for (int blend = 0; blend < 3; ++blend)
        for (int phase = 0; phase < 4; ++phase) for (int repeated = 0; repeated < 2; ++repeated)
        {
            check(device->SetTextureStageState(0, D3DTSS_MINFILTER, filter));
            check(device->SetTextureStageState(0, D3DTSS_MAGFILTER, filter));
            check(device->SetRenderState(D3DRS_ALPHABLENDENABLE, blend != 0));
            check(device->SetRenderState(D3DRS_DESTBLEND, blend == 2 ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA));
            check(device->Clear(0, NULL, D3DCLEAR_TARGET, 0xff183048, 1, 0));
            float x = 3.125f + phase * 0.0625f, y = 4.25f + phase * 0.125f;
            const DWORD color = 0xd0c0f080;
            Vertex quad[] = {{x,y,0.5f,1,color,0,0}, {x+24,y,0.5f,1,color,1,0},
                             {x,y+24,0.5f,1,color,0,1}, {x+24,y+24,0.5f,1,color,1,1}};
            check(device->BeginScene());
            int draws = repeated ? 4 : 1;
            for (int i = 0; i < draws; ++i) check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(Vertex)));
            check(device->EndScene());
            IDirect3DSurface8 *surface; check(device->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surface));
            check(surface->LockRect(&lock, NULL, D3DLOCK_READONLY));
            printf("%s{\"format\":%d,\"filter\":%d,\"blend\":%d,\"phase\":%d,\"draws\":%d,\"pixels\":[",
                   first ? "" : ",\n", kind, filter, blend, phase, draws); first = false;
            for (int sample = 0; sample < 32; ++sample)
            {
                DWORD pixel; memcpy(&pixel, static_cast<char *>(lock.pBits) + (8 + sample / 8) * lock.Pitch + (8 + sample % 8) * 4, 4);
                printf("%s%lu", sample ? "," : "", static_cast<unsigned long>(pixel & 0xffffff));
            }
            printf("]}"); check(surface->UnlockRect()); surface->Release();
        }
        texture->Release();
    }
    printf("\n]\n"); device->Release(); api->Release();
    return 0;
}
