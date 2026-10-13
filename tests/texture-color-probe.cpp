#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TH08_MODERN_LINUX
#include <SDL.h>
#include <d3dx8.h>
#endif

struct Vertex { float x, y, z, rhw; DWORD color; float u, v; };
static IDirect3DDevice8 *device;
static void require(HRESULT result, const char *step) {
    if (FAILED(result)) { fprintf(stderr, "Failed %s: %08lx\n", step, (unsigned long)result); exit(1); }
}
static DWORD draw(IDirect3DTexture8 *texture, DWORD diffuse, DWORD background, bool blend) {
    require(device->SetTexture(0, texture), "texture");
    require(device->SetRenderState(D3DRS_ALPHABLENDENABLE, blend), "blend");
    require(device->SetTextureStageState(0, D3DTSS_COLORARG1, texture ? D3DTA_TEXTURE : D3DTA_DIFFUSE), "color argument");
    require(device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE), "color operation");
    require(device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE), "color modifier");
    require(device->SetTextureStageState(0, D3DTSS_ALPHAARG1, texture ? D3DTA_TEXTURE : D3DTA_DIFFUSE), "alpha argument");
    require(device->SetTextureStageState(0, D3DTSS_ALPHAOP, texture ? D3DTOP_MODULATE : D3DTOP_SELECTARG1), "alpha operation");
    require(device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE), "alpha modifier");
    // Untextured RGB uses a single diffuse argument.
    if (!texture) require(device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1), "untextured color");
    Vertex vertices[4] = {{-.5f,-.5f,0,1,diffuse,0,0},{31.5f,-.5f,0,1,diffuse,1,0},
                          {-.5f,31.5f,0,1,diffuse,0,1},{31.5f,31.5f,0,1,diffuse,1,1}};
    require(device->Clear(0, NULL, D3DCLEAR_TARGET, background, 1, 0), "clear");
    require(device->BeginScene(), "begin");
    require(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex)), "draw");
    require(device->EndScene(), "end");
    IDirect3DSurface8 *surface;
    require(device->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surface), "backbuffer");
    D3DLOCKED_RECT locked;
    require(surface->LockRect(&locked, NULL, D3DLOCK_READONLY), "readback");
    DWORD pixel;
    memcpy(&pixel, (char *)locked.pBits + 16 * locked.Pitch + 16 * 4, 4);
    require(surface->UnlockRect(), "unlock"); surface->Release();
    return pixel & 0xffffff;
}
int main(int argc, char **argv) {
    HWND window;
#ifdef TH08_MODERN_LINUX
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    window = (HWND)SDL_CreateWindow("D3D8 color probe",0,0,64,64,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
    if (!window) return 1;
#else
    window = CreateWindowA("STATIC", "D3D8 color probe", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                           0,0,64,64,NULL,NULL,GetModuleHandle(NULL),NULL);
#endif
    IDirect3D8 *api = Direct3DCreate8(D3D_SDK_VERSION);
    if (!api) return 1;
    D3DPRESENT_PARAMETERS parameters; memset(&parameters,0,sizeof(parameters));
    parameters.Windowed=TRUE; parameters.hDeviceWindow=window;
    parameters.BackBufferWidth=32; parameters.BackBufferHeight=32;
    parameters.BackBufferFormat=D3DFMT_X8R8G8B8; parameters.BackBufferCount=1;
    parameters.SwapEffect=D3DSWAPEFFECT_COPY; parameters.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    require(api->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&parameters,&device),"device");
    device->SetRenderState(D3DRS_ZENABLE,FALSE); device->SetRenderState(D3DRS_LIGHTING,FALSE);
    device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE); device->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
    device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
    device->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT); device->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT);
    require(device->SetVertexShader(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1),"fvf");
    const D3DFORMAT formats[]={D3DFMT_A1R5G5B5,D3DFMT_R5G6B5,D3DFMT_A4R4G4B4,D3DFMT_A8R8G8B8};
    printf("[\n"); bool first=true;
    for (int kind=0;kind<4;++kind) {
        IDirect3DTexture8 *texture;
        require(device->CreateTexture(1,1,1,0,formats[kind],D3DPOOL_MANAGED,&texture),"create texture");
        IDirect3DSurface8 *source, *destination;
        require(device->CreateImageSurface(1,1,formats[kind],&source),"copy source");
        require(texture->GetSurfaceLevel(0,&destination),"copy destination");
        int count=kind==1?64:kind==2?16:32;
        for (int value=0;value<count;++value) {
            D3DLOCKED_RECT locked; require(texture->LockRect(0,&locked,NULL,0),"lock texture");
            WORD shortPixel;
            if (kind==0) shortPixel=0x8000|((value&31)<<10)|((value&31)<<5)|(value&31);
            else if (kind==1) shortPixel=((value&31)<<11)|((value&63)<<5)|(value&31);
            else shortPixel=0xf000|(value<<8)|(value<<4)|value;
            DWORD longPixel=0xff000000u|((value*8)<<16)|((value*8)<<8)|(value*8);
            memcpy(locked.pBits,kind==3?(void *)&longPixel:(void *)&shortPixel,kind==3?4:2);
            require(texture->UnlockRect(0),"unlock texture");
            DWORD result=draw(texture,0xffffffffu,0xff000000u,false);
            printf("%s{\"kind\":%d,\"value\":%d,\"rgb\":%lu}",first?"":",\n",kind,value,(unsigned long)result);first=false;
            require(source->LockRect(&locked,NULL,0),"copy source lock");
            memcpy(locked.pBits,kind==3?(void *)&longPixel:(void *)&shortPixel,kind==3?4:2);
            require(source->UnlockRect(),"copy source unlock");
#ifdef TH08_MODERN_LINUX
            if (argc > 1 && strcmp(argv[1],"d3dx-copies") == 0)
                require(D3DXLoadSurfaceFromSurface(destination,NULL,NULL,source,NULL,NULL,3,0),"D3DX copy identical format");
            else
#endif
                require(device->CopyRects(source,NULL,0,destination,NULL),"copy identical format");
            require(destination->LockRect(&locked,NULL,D3DLOCK_READONLY),"copy destination lock");
            DWORD copied=0; memcpy(&copied,locked.pBits,kind==3?4:2);
            require(destination->UnlockRect(),"copy destination unlock");
            printf(",\n{\"kind\":%d,\"value\":%d,\"rgb\":%lu}",kind+10,value,(unsigned long)copied);
        }
        source->Release();destination->Release();
        texture->Release();
    }
    const DWORD alphas[]={1,17,31,64,127,128,191,254,255};
    for (unsigned i=0;i<sizeof(alphas)/sizeof(alphas[0]);++i) {
        DWORD result=draw(NULL,(alphas[i]<<24)|0x8951d5u,0xff8040f0u,true);
        printf(",\n{\"kind\":4,\"value\":%lu,\"rgb\":%lu}",(unsigned long)alphas[i],(unsigned long)result);
    }
    printf("\n]\n");device->Release();api->Release();return 0;
}
