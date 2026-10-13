#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TH08_MODERN_LINUX
#include <SDL.h>
#include <SDL_opengl.h>
#endif
struct Vertex { float x,y,z,w; DWORD color; float u,v; };
static void check(HRESULT h) { if(FAILED(h)) { fprintf(stderr,"D3D call failed: %08lx\n",(unsigned long)h);exit(1); } }
int main(int argc,char **argv) {
 if(argc!=3)return 1;
 FILE *in=fopen(argv[1],"rb");if(!in)return 2;
 unsigned count;if(fread(&count,4,1,in)!=1||count==0||count>10000||sizeof(Vertex)!=28)return 3;
 Vertex *quads=(Vertex *)malloc(count*112);if(fread(quads,112,count,in)!=count)return 4;fclose(in);
 HWND window;
#ifdef TH08_MODERN_LINUX
 SDL_Init(SDL_INIT_VIDEO);window=reinterpret_cast<HWND>(SDL_CreateWindow("Subpixel quad probe",0,0,640,480,SDL_WINDOW_OPENGL));
#else
 window=CreateWindowA("STATIC","Subpixel quad probe",WS_OVERLAPPEDWINDOW,0,0,640,480,NULL,NULL,GetModuleHandle(NULL),NULL);ShowWindow(window,SW_SHOW);
#endif
 IDirect3D8 *api=Direct3DCreate8(D3D_SDK_VERSION);IDirect3DDevice8 *device;
 D3DPRESENT_PARAMETERS p;memset(&p,0,sizeof(p));p.Windowed=TRUE;p.hDeviceWindow=window;p.BackBufferWidth=640;p.BackBufferHeight=480;p.BackBufferCount=1;p.BackBufferFormat=D3DFMT_X8R8G8B8;p.SwapEffect=D3DSWAPEFFECT_COPY;p.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
 check(api->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&p,&device));
#ifdef TH08_MODERN_LINUX
 const char *version=reinterpret_cast<const char *>(glGetString(GL_VERSION));
 const bool clipControl=SDL_GL_ExtensionSupported("GL_ARB_clip_control")!=SDL_FALSE;
 printf("OpenGL: %s; ARB_clip_control: %s\n",version,clipControl?"available":"unavailable");
 if(getenv("TH08_RENDERER_REQUIRE_LEGACY_GL")&&(clipControl||strncmp(version,"2.1 ",4))) {
  fprintf(stderr,"Legacy probe requires OpenGL 2.1 without clip control\n");return 6;
 }
#endif
 check(device->SetVertexShader(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1));
 check(device->SetRenderState(D3DRS_ZENABLE,FALSE));check(device->SetRenderState(D3DRS_LIGHTING,FALSE));check(device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE));
 check(device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE));
 IDirect3DTexture8 *texture;check(device->CreateTexture(256,256,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture));
 D3DLOCKED_RECT lock;check(texture->LockRect(0,&lock,NULL,0));
 for(unsigned y=0;y<256;y++)for(unsigned x=0;x<256;x++){DWORD color=x>=33&&x<=35?0xffffffffu:0xff000000u;memcpy((char *)lock.pBits+y*lock.Pitch+x*4,&color,4);}check(texture->UnlockRect(0));
 check(device->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT));check(device->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT));
 check(device->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE));check(device->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE));
 for(int vp=0;vp<2;vp++)for(int material=0;material<2;material++)for(unsigned index=0;index<=count;index++) {
  D3DVIEWPORT8 v={vp?32u:0u,vp?16u:0u,vp?384u:640u,vp?448u:480u,0,1};check(device->SetViewport(&v));
  check(device->SetTextureStageState(0,D3DTSS_COLORARG1,material?D3DTA_TEXTURE:D3DTA_DIFFUSE));check(device->SetTexture(0,material?texture:NULL));check(device->SetTextureStageState(0,D3DTSS_COLOROP,material?D3DTOP_MODULATE:D3DTOP_SELECTARG1));
  check(device->Clear(0,NULL,D3DCLEAR_TARGET,0xff802080,1,0));check(device->BeginScene());
  for(unsigned q=0;q<count;q++)if(index==count||q==index)check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,quads+q*4,28));
  check(device->EndScene());IDirect3DSurface8 *surface;check(device->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface));check(surface->LockRect(&lock,NULL,D3DLOCK_READONLY));
  char filename[2048];snprintf(filename,sizeof(filename),"%s/v%d-m%d-q%u.ppm",argv[2],vp,material,index);FILE *out=fopen(filename,"wb");if(!out)return 5;fprintf(out,"P6\n64 16\n255\n");
  for(int y=232;y<248;y++)for(int x=192;x<256;x++){const unsigned char *b=(const unsigned char *)lock.pBits+y*lock.Pitch+x*4;unsigned char rgb[]={b[2],b[1],b[0]};fwrite(rgb,1,3,out);}fclose(out);check(surface->UnlockRect());surface->Release();
 }
 texture->Release();device->Release();api->Release();free(quads);printf("Captured %u subpixel quad cases\n",4*(count+1));return 0;
}
