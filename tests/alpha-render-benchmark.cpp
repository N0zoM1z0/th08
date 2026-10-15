#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    parameters.BackBufferWidth = 640; parameters.BackBufferHeight = 480;
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
    const bool alphaTest = true;
    check(device->SetRenderState(D3DRS_ALPHATESTENABLE, alphaTest));
    check(device->SetRenderState(D3DRS_ALPHAREF, 4));
    check(device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
    check(device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    check(device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
    check(device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
    check(device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
    check(device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
    check(device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
    check(device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
    IDirect3DTexture8 *texture; check(device->CreateTexture(2,2,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture));
    D3DLOCKED_RECT lock; check(texture->LockRect(0,&lock,NULL,0));
    const DWORD texels[]={0x00ffffff,0x20c03080,0xc0f08040,0xffe060a0};
    for (int y=0;y<2;++y) memcpy(static_cast<char *>(lock.pBits)+y*lock.Pitch,texels+y*2,8);
    check(texture->UnlockRect(0)); check(device->SetTexture(0,texture));
    check(device->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR));
    check(device->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR));
    check(device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE));
    check(device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA));
    const DWORD color=0x28ffffff;
    Vertex quad[]={{32,16,.5f,1,color,0,0},{416,16,.5f,1,color,1,0},
                   {32,464,.5f,1,color,0,1},{416,464,.5f,1,color,1,1}};
    timespec started,ended; double elapsed=0;
    for (int frame=-3;frame<20;++frame)
    {
        if (frame==0) { glFinish();clock_gettime(CLOCK_MONOTONIC,&started); }
        check(device->Clear(0,NULL,D3DCLEAR_TARGET,0xff183048,1,0));check(device->BeginScene());
        for (int layer=0;layer<12;++layer) check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,quad,sizeof(Vertex)));
        check(device->EndScene());
    }
    glFinish();clock_gettime(CLOCK_MONOTONIC,&ended);
    elapsed=(ended.tv_sec-started.tv_sec)+(ended.tv_nsec-started.tv_nsec)*1e-9;
    IDirect3DSurface8 *surface;check(device->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface));
    check(surface->LockRect(&lock,NULL,D3DLOCK_READONLY));
    unsigned long checksum=2166136261u;
    for (int y=0;y<480;++y) for (int x=0;x<640;++x)
    {
        DWORD pixel;memcpy(&pixel,static_cast<char *>(lock.pBits)+y*lock.Pitch+x*4,4);
        for (int shift=0;shift<24;shift+=8) checksum=((checksum^((pixel>>shift)&255))*16777619u)&0xffffffffu;
    }
    printf("{\"seconds\":%.9f,\"checksum\":%lu}\n",elapsed,checksum);
    check(surface->UnlockRect());surface->Release();texture->Release();device->Release();api->Release();return 0;
}
