#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TH08_MODERN_LINUX
// D3D8 SDK values absent from the portable header's current subset.
#define D3DTOP_SELECTARG2 3
#define D3DTA_COMPLEMENT 0x10
#define D3DTA_ALPHAREPLICATE 0x20
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
    IDirect3DTexture8 *texture;
    check(device->CreateTexture(2,2,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture));
    D3DLOCKED_RECT lock; check(texture->LockRect(0,&lock,NULL,0));
    const DWORD texels[] = {0x00e050a0,0x2080c030,0xc03070f0,0xffa04060};
    for (int y=0;y<2;++y) memcpy(static_cast<char *>(lock.pBits)+y*lock.Pitch,texels+y*2,8);
    check(texture->UnlockRect(0));
    check(device->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR));
    check(device->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR));
    check(device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE));
    check(device->SetRenderState(D3DRS_TEXTUREFACTOR,0x80b040c0));
    check(device->SetRenderState(D3DRS_FOGVERTEXMODE,D3DFOG_LINEAR));
    check(device->SetRenderState(D3DRS_FOGCOLOR,0xff2060a0));
    float start=0, end=1; DWORD bits;
    memcpy(&bits,&start,4); check(device->SetRenderState(D3DRS_FOGSTART,bits));
    memcpy(&bits,&end,4); check(device->SetRenderState(D3DRS_FOGEND,bits));
    D3DMATRIX identity; memset(&identity,0,sizeof(identity));
    identity._11=identity._22=identity._33=identity._44=1;
    check(device->SetTransform(D3DTS_WORLD,&identity));
    check(device->SetTransform(D3DTS_VIEW,&identity));
    check(device->SetTransform(D3DTS_PROJECTION,&identity));
    const DWORD references[]={0,4,128,255};
    bool first=true; printf("[\n");
    for (int material=0;material<6;++material) for (int comparison=1;comparison<=8;++comparison)
    for (int ref=0;ref<4;++ref) for (int fog=0;fog<2;++fog)
    {
        check(device->SetTexture(0,material==2?NULL:texture));
        DWORD colorOp=D3DTOP_MODULATE,alphaOp=D3DTOP_MODULATE;
        DWORD color1=D3DTA_TEXTURE,color2=D3DTA_DIFFUSE,alpha1=D3DTA_TEXTURE,alpha2=D3DTA_DIFFUSE;
        if (material==1) color2=alpha2=D3DTA_TFACTOR;
        if (material==2) { colorOp=alphaOp=D3DTOP_SELECTARG1; color1=alpha1=D3DTA_DIFFUSE; }
        if (material==3) { colorOp=alphaOp=D3DTOP_SELECTARG1; color1=D3DTA_DIFFUSE; }
        if (material==4) { colorOp=alphaOp=D3DTOP_SELECTARG2; color2=alpha2=D3DTA_TFACTOR; }
        if (material==5) { color1|=D3DTA_ALPHAREPLICATE; alpha1|=D3DTA_COMPLEMENT; }
        check(device->SetTextureStageState(0,D3DTSS_COLOROP,colorOp));
        check(device->SetTextureStageState(0,D3DTSS_ALPHAOP,alphaOp));
        check(device->SetTextureStageState(0,D3DTSS_COLORARG1,color1));
        check(device->SetTextureStageState(0,D3DTSS_COLORARG2,color2));
        check(device->SetTextureStageState(0,D3DTSS_ALPHAARG1,alpha1));
        check(device->SetTextureStageState(0,D3DTSS_ALPHAARG2,alpha2));
        check(device->SetRenderState(D3DRS_ALPHAFUNC,comparison));
        check(device->SetRenderState(D3DRS_ALPHAREF,references[ref]));
        check(device->SetRenderState(D3DRS_FOGENABLE,fog));
        check(device->Clear(0,NULL,D3DCLEAR_TARGET,0xff183048,1,0));
        const DWORD color=material==2?0x0481c033:0x28c0f080;
        Vertex transformed[]={{3.1875f,4.375f,.5f,1,color,0,0},{27.1875f,4.375f,.5f,1,color,1,0},
                              {3.1875f,28.375f,.5f,1,color,0,1},{27.1875f,28.375f,.5f,1,color,1,1}};
        struct Untransformed { float x,y,z; DWORD color; float u,v; };
        Untransformed world[]={{-.8f,.8f,.5f,color,0,0},{.8f,.8f,.5f,color,1,0},
                               {-.8f,-.8f,.5f,color,0,1},{.8f,-.8f,.5f,color,1,1}};
        check(device->SetVertexShader((fog?D3DFVF_XYZ:D3DFVF_XYZRHW)|D3DFVF_DIFFUSE|D3DFVF_TEX1));
        check(device->BeginScene());
        check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,fog?static_cast<void *>(world):static_cast<void *>(transformed),
                                     fog?sizeof(Untransformed):sizeof(Vertex)));
        check(device->EndScene());
        IDirect3DSurface8 *surface; check(device->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface));
        check(surface->LockRect(&lock,NULL,D3DLOCK_READONLY));
        printf("%s{\"material\":%d,\"comparison\":%d,\"reference\":%lu,\"fog\":%d,\"pixels\":[",
               first?"":",\n",material,comparison,static_cast<unsigned long>(references[ref]),fog); first=false;
        for (int sample=0;sample<16;++sample)
        {
            DWORD pixel; memcpy(&pixel,static_cast<char *>(lock.pBits)+(10+sample/4)*lock.Pitch+(7+sample%4)*4,4);
            printf("%s%lu",sample?",":"",static_cast<unsigned long>(pixel&0xffffff));
        }
        printf("]}");check(surface->UnlockRect());surface->Release();
    }
    printf("\n]\n");texture->Release();device->Release();api->Release();return 0;
}
