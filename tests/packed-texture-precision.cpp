// The 256x256 texture enumerates every A4R4G4B4 source word.
// Point sampling uses texel centers; linear sampling shifts them by one quarter.
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

int main(int argc, char **argv)
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
    parameters.BackBufferWidth = parameters.BackBufferHeight = 256;
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
    if (argc!=3 || (strcmp(argv[2],"point") && strcmp(argv[2],"linear"))) return 3;
    const bool linear = !strcmp(argv[2],"linear");
    FILE *file=fopen(argv[1],"wb");if (!file) return 4;
    IDirect3DTexture8 *texture;check(device->CreateTexture(256,256,1,0,D3DFMT_A4R4G4B4,D3DPOOL_MANAGED,&texture));
    D3DLOCKED_RECT lock;check(texture->LockRect(0,&lock,NULL,0));
    for (int y=0;y<256;++y) for (int x=0;x<256;++x) {
        WORD texel=y*256+x;memcpy(static_cast<char *>(lock.pBits)+y*lock.Pitch+x*2,&texel,2);
    }
    check(texture->UnlockRect(0));check(device->SetTexture(0,texture));
    check(device->SetTextureStageState(0,D3DTSS_MINFILTER,linear?D3DTEXF_LINEAR:D3DTEXF_POINT));
    check(device->SetTextureStageState(0,D3DTSS_MAGFILTER,linear?D3DTEXF_LINEAR:D3DTEXF_POINT));
    const DWORD colors[]={0xffffffff,0x28ffffff,0x80c0f080,0xffc0f080};
    for (int material=0;material<4;++material) for (int blend=0;blend<3;++blend) for (int repeated=0;repeated<2;++repeated) {
        const DWORD color=colors[material];
        check(device->SetRenderState(D3DRS_ALPHABLENDENABLE,blend!=0));
        check(device->SetRenderState(D3DRS_DESTBLEND,blend==2?D3DBLEND_ONE:D3DBLEND_INVSRCALPHA));
        check(device->Clear(0,NULL,D3DCLEAR_TARGET,0xff183048,1,0));
        const float left=linear?-.25f:-.5f, right=left+256;
        Vertex quad[]={{left,left,.5f,1,color,0,0},{right,left,.5f,1,color,1,0},
                       {left,right,.5f,1,color,0,1},{right,right,.5f,1,color,1,1}};
        check(device->BeginScene());for (int draw=0;draw<(repeated?4:1);++draw)
            check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,quad,sizeof(Vertex)));
        check(device->EndScene());IDirect3DSurface8 *surface;
        check(device->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface));check(surface->LockRect(&lock,NULL,D3DLOCK_READONLY));
        for (int y=0;y<256;++y) for (int x=0;x<256;++x) {
            DWORD pixel;memcpy(&pixel,static_cast<char *>(lock.pBits)+y*lock.Pitch+x*4,4);pixel&=0xffffff;
            if (fwrite(&pixel,4,1,file)!=1) return 5;
        }
        check(surface->UnlockRect());surface->Release();
    }
    fclose(file);texture->Release();device->Release();api->Release();puts("Captured 24 exhaustive packed-texture cases");return 0;
}
