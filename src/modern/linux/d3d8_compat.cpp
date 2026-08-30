#include "d3d8_internal.hpp"
#include "Gui.hpp"

#include <SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace
{
class LinuxTexture;

typedef void (APIENTRY *GenFramebuffersFunction)(GLsizei, GLuint *);
typedef void (APIENTRY *BindFramebufferFunction)(GLenum, GLuint);
typedef void (APIENTRY *FramebufferTexture2DFunction)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRY *CheckFramebufferStatusFunction)(GLenum);
typedef void (APIENTRY *DeleteFramebuffersFunction)(GLsizei, const GLuint *);
typedef void (APIENTRY *GenRenderbuffersFunction)(GLsizei, GLuint *);
typedef void (APIENTRY *BindRenderbufferFunction)(GLenum, GLuint);
typedef void (APIENTRY *RenderbufferStorageFunction)(GLenum, GLenum, GLsizei, GLsizei);
typedef void (APIENTRY *FramebufferRenderbufferFunction)(GLenum, GLenum, GLenum, GLuint);
typedef void (APIENTRY *DeleteRenderbuffersFunction)(GLsizei, const GLuint *);
typedef void (APIENTRY *FogCoordfFunction)(GLfloat);

struct FramebufferApi
{
    FramebufferApi()
        : genFramebuffers(NULL), bindFramebuffer(NULL), framebufferTexture2D(NULL),
          checkFramebufferStatus(NULL), deleteFramebuffers(NULL), genRenderbuffers(NULL),
          bindRenderbuffer(NULL), renderbufferStorage(NULL), framebufferRenderbuffer(NULL),
          deleteRenderbuffers(NULL)
    {
    }

    void *Load(const char *coreName, const char *extensionName)
    {
        void *procedure = SDL_GL_GetProcAddress(coreName);
        return procedure != NULL ? procedure : SDL_GL_GetProcAddress(extensionName);
    }

    bool Initialize()
    {
        genFramebuffers = reinterpret_cast<GenFramebuffersFunction>(
            Load("glGenFramebuffers", "glGenFramebuffersEXT"));
        bindFramebuffer = reinterpret_cast<BindFramebufferFunction>(
            Load("glBindFramebuffer", "glBindFramebufferEXT"));
        framebufferTexture2D = reinterpret_cast<FramebufferTexture2DFunction>(
            Load("glFramebufferTexture2D", "glFramebufferTexture2DEXT"));
        checkFramebufferStatus = reinterpret_cast<CheckFramebufferStatusFunction>(
            Load("glCheckFramebufferStatus", "glCheckFramebufferStatusEXT"));
        deleteFramebuffers = reinterpret_cast<DeleteFramebuffersFunction>(
            Load("glDeleteFramebuffers", "glDeleteFramebuffersEXT"));
        genRenderbuffers = reinterpret_cast<GenRenderbuffersFunction>(
            Load("glGenRenderbuffers", "glGenRenderbuffersEXT"));
        bindRenderbuffer = reinterpret_cast<BindRenderbufferFunction>(
            Load("glBindRenderbuffer", "glBindRenderbufferEXT"));
        renderbufferStorage = reinterpret_cast<RenderbufferStorageFunction>(
            Load("glRenderbufferStorage", "glRenderbufferStorageEXT"));
        framebufferRenderbuffer = reinterpret_cast<FramebufferRenderbufferFunction>(
            Load("glFramebufferRenderbuffer", "glFramebufferRenderbufferEXT"));
        deleteRenderbuffers = reinterpret_cast<DeleteRenderbuffersFunction>(
            Load("glDeleteRenderbuffers", "glDeleteRenderbuffersEXT"));
        return genFramebuffers != NULL && bindFramebuffer != NULL && framebufferTexture2D != NULL &&
               checkFramebufferStatus != NULL && deleteFramebuffers != NULL &&
               genRenderbuffers != NULL && bindRenderbuffer != NULL &&
               renderbufferStorage != NULL && framebufferRenderbuffer != NULL &&
               deleteRenderbuffers != NULL;
    }

    GenFramebuffersFunction genFramebuffers;
    BindFramebufferFunction bindFramebuffer;
    FramebufferTexture2DFunction framebufferTexture2D;
    CheckFramebufferStatusFunction checkFramebufferStatus;
    DeleteFramebuffersFunction deleteFramebuffers;
    GenRenderbuffersFunction genRenderbuffers;
    BindRenderbufferFunction bindRenderbuffer;
    RenderbufferStorageFunction renderbufferStorage;
    FramebufferRenderbufferFunction framebufferRenderbuffer;
    DeleteRenderbuffersFunction deleteRenderbuffers;
};

FramebufferApi g_framebufferApi;
FogCoordfFunction g_fogCoordf;

UINT BytesPerPixel(D3DFORMAT format)
{
    switch (format)
    {
    case D3DFMT_R8G8B8: return 3;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4: return 2;
    default: return 4;
    }
}

void DecodePixel(const BYTE *source, D3DFORMAT format, BYTE *rgba)
{
    WORD pixel;
    switch (format)
    {
    case D3DFMT_R8G8B8:
        rgba[0] = source[2]; rgba[1] = source[1]; rgba[2] = source[0]; rgba[3] = 255; break;
    case D3DFMT_R5G6B5:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = static_cast<BYTE>(((pixel >> 11) & 31) * 255 / 31);
        rgba[1] = static_cast<BYTE>(((pixel >> 5) & 63) * 255 / 63);
        rgba[2] = static_cast<BYTE>((pixel & 31) * 255 / 31); rgba[3] = 255; break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = static_cast<BYTE>(((pixel >> 10) & 31) * 255 / 31);
        rgba[1] = static_cast<BYTE>(((pixel >> 5) & 31) * 255 / 31);
        rgba[2] = static_cast<BYTE>((pixel & 31) * 255 / 31);
        rgba[3] = format == D3DFMT_A1R5G5B5 && !(pixel & 0x8000) ? 0 : 255; break;
    case D3DFMT_A4R4G4B4:
        memcpy(&pixel, source, sizeof(pixel));
        rgba[0] = static_cast<BYTE>(((pixel >> 8) & 15) * 17);
        rgba[1] = static_cast<BYTE>(((pixel >> 4) & 15) * 17);
        rgba[2] = static_cast<BYTE>((pixel & 15) * 17);
        rgba[3] = static_cast<BYTE>(((pixel >> 12) & 15) * 17); break;
    default:
        rgba[0] = source[2]; rgba[1] = source[1]; rgba[2] = source[0];
        rgba[3] = format == D3DFMT_X8R8G8B8 ? 255 : source[3]; break;
    }
}

void EncodePixel(BYTE *destination, D3DFORMAT format, const BYTE *rgba)
{
    WORD pixel;
    switch (format)
    {
    case D3DFMT_R8G8B8:
        destination[0] = rgba[2]; destination[1] = rgba[1]; destination[2] = rgba[0]; break;
    case D3DFMT_R5G6B5:
        pixel = static_cast<WORD>(((rgba[0] * 31 / 255) << 11) |
                                  ((rgba[1] * 63 / 255) << 5) | (rgba[2] * 31 / 255));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        pixel = static_cast<WORD>(((format == D3DFMT_X1R5G5B5 || rgba[3] >= 128) ? 0x8000 : 0) |
                                  ((rgba[0] * 31 / 255) << 10) |
                                  ((rgba[1] * 31 / 255) << 5) | (rgba[2] * 31 / 255));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    case D3DFMT_A4R4G4B4:
        pixel = static_cast<WORD>(((rgba[3] >> 4) << 12) | ((rgba[0] >> 4) << 8) |
                                  ((rgba[1] >> 4) << 4) | (rgba[2] >> 4));
        memcpy(destination, &pixel, sizeof(pixel)); break;
    default:
        destination[0] = rgba[2]; destination[1] = rgba[1]; destination[2] = rgba[0];
        destination[3] = format == D3DFMT_X8R8G8B8 ? 255 : rgba[3]; break;
    }
}

void Identity(D3DMATRIX *matrix)
{
    memset(matrix, 0, sizeof(*matrix));
    matrix->_11 = matrix->_22 = matrix->_33 = matrix->_44 = 1.0f;
}

class LinuxSurface : public IDirect3DSurface8
{
  public:
    LinuxSurface(UINT width_, UINT height_, D3DFORMAT format_, bool backbuffer_, LinuxTexture *owner_)
        : refs(1), width(width_), height(height_), format(format_), backbuffer(backbuffer_), owner(owner_),
          dirty(!backbuffer_)
    {
        pitch = width * BytesPerPixel(format);
        pixels.resize(pitch * height);
    }
    ULONG AddRef() { return ++refs; }
    ULONG Release() { ULONG value = --refs; if (value == 0) delete this; return value; }
    HRESULT GetDesc(D3DSURFACE_DESC *description)
    {
        if (description == NULL) return E_INVALIDARG;
        memset(description, 0, sizeof(*description));
        description->Format = format; description->Type = D3DRTYPE_SURFACE;
        description->Pool = backbuffer ? D3DPOOL_DEFAULT : D3DPOOL_SYSTEMMEM;
        description->Size = static_cast<UINT>(pixels.size());
        description->Width = width; description->Height = height; return S_OK;
    }
    HRESULT LockRect(D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags)
    {
        if (locked == NULL) return E_INVALIDARG;
        if (backbuffer && (flags & D3DLOCK_READONLY)) ReadBackbuffer();
        UINT left = rect != NULL && rect->left > 0 ? static_cast<UINT>(rect->left) : 0;
        UINT top = rect != NULL && rect->top > 0 ? static_cast<UINT>(rect->top) : 0;
        if (left >= width || top >= height) return E_INVALIDARG;
        locked->Pitch = pitch;
        locked->pBits = &pixels[top * pitch + left * BytesPerPixel(format)]; return S_OK;
    }
    HRESULT UnlockRect() { dirty = true; return S_OK; }
    HRESULT GetDC(HDC *dc)
    { if (dc == NULL) return E_INVALIDARG; *dc = CreateCompatibleDC(NULL); return *dc ? S_OK : E_FAIL; }
    HRESULT ReleaseDC(HDC dc) { return DeleteDC(dc) ? S_OK : E_FAIL; }
    void ReadBackbuffer()
    {
        if (!backbuffer || width == 0 || height == 0) return;
        std::vector<BYTE> rgba(width * height * 4);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &rgba[0]);
        const UINT bytes = BytesPerPixel(format);
        for (UINT y = 0; y < height; ++y)
            for (UINT x = 0; x < width; ++x)
                EncodePixel(&pixels[y * pitch + x * bytes], format,
                            &rgba[((height - 1 - y) * width + x) * 4]);
        dirty = false;
    }
    void FlushBackbuffer()
    {
        if (!backbuffer || !dirty || width == 0 || height == 0) return;
        std::vector<BYTE> rgba(width * height * 4);
        const UINT bytes = BytesPerPixel(format);
        for (UINT y = 0; y < height; ++y)
            for (UINT x = 0; x < width; ++x)
                DecodePixel(&pixels[y * pitch + x * bytes], format, &rgba[(y * width + x) * 4]);

        GLuint name = 0;
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_ALPHA_TEST); glDisable(GL_BLEND); glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST); glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &name); glBindTexture(GL_TEXTURE_2D, name);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgba[0]);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glColor4ub(255, 255, 255, 255);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(static_cast<float>(width), 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, static_cast<float>(height));
        glTexCoord2f(1.0f, 1.0f); glVertex2f(static_cast<float>(width), static_cast<float>(height));
        glEnd();
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
        glDeleteTextures(1, &name);
        glPopAttrib();
        dirty = false;
    }
    ULONG refs;
    UINT width, height, pitch;
    D3DFORMAT format;
    bool backbuffer;
    LinuxTexture *owner;
    bool dirty;
    std::vector<BYTE> pixels;
};

class LinuxTexture : public IDirect3DTexture8
{
  public:
    LinuxTexture(UINT width, UINT height, D3DFORMAT format)
        : refs(1), priority(0), glName(0), uploaded(false)
    { surface = new LinuxSurface(width, height, format, false, this); }
    ~LinuxTexture()
    {
        surface->owner = NULL; surface->Release();
        if (glName != 0) glDeleteTextures(1, &glName);
    }
    ULONG AddRef() { return ++refs; }
    ULONG Release() { ULONG value = --refs; if (value == 0) delete this; return value; }
    DWORD SetPriority(DWORD value) { DWORD old = priority; priority = value; return old; }
    void PreLoad() { Upload(); }
    HRESULT GetLevelDesc(UINT level, D3DSURFACE_DESC *description)
    { return level == 0 ? surface->GetDesc(description) : E_INVALIDARG; }
    HRESULT GetSurfaceLevel(UINT level, IDirect3DSurface8 **result)
    {
        if (level != 0 || result == NULL) return E_INVALIDARG;
        surface->AddRef(); *result = surface; return S_OK;
    }
    HRESULT LockRect(UINT level, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags)
    { return level == 0 ? surface->LockRect(locked, rect, flags) : E_INVALIDARG; }
    HRESULT UnlockRect(UINT level)
    { if (level != 0) return E_INVALIDARG; uploaded = false; return surface->UnlockRect(); }
    void Upload()
    {
        if (uploaded && !surface->dirty) return;
        if (glName == 0) glGenTextures(1, &glName);
        glBindTexture(GL_TEXTURE_2D, glName);
        std::vector<BYTE> rgba(surface->width * surface->height * 4);
        UINT bytes = BytesPerPixel(surface->format);
        for (UINT y = 0; y < surface->height; ++y)
            for (UINT x = 0; x < surface->width; ++x)
                DecodePixel(&surface->pixels[y * surface->pitch + x * bytes], surface->format,
                            &rgba[(y * surface->width + x) * 4]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->width, surface->height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgba.empty() ? NULL : &rgba[0]);
        uploaded = true; surface->dirty = false;
    }
    ULONG refs;
    DWORD priority;
    GLuint glName;
    bool uploaded;
    LinuxSurface *surface;
};

class LinuxVertexBuffer : public IDirect3DVertexBuffer8
{
  public:
    explicit LinuxVertexBuffer(UINT size) : refs(1), bytes(size) {}
    ULONG AddRef() { return ++refs; }
    ULONG Release() { ULONG value = --refs; if (value == 0) delete this; return value; }
    HRESULT Lock(UINT offset, UINT size, BYTE **data, DWORD)
    {
        if (data == NULL || offset > bytes.size()) return E_INVALIDARG;
        if (size == 0) size = static_cast<UINT>(bytes.size() - offset);
        if (offset + size > bytes.size()) return E_INVALIDARG;
        *data = bytes.empty() ? NULL : &bytes[offset]; return S_OK;
    }
    HRESULT Unlock() { return S_OK; }
    ULONG refs;
    std::vector<BYTE> bytes;
};

GLenum PrimitiveMode(D3DPRIMITIVETYPE type)
{
    switch (type)
    {
    case D3DPT_POINTLIST: return GL_POINTS;
    case D3DPT_LINELIST: return GL_LINES;
    case D3DPT_LINESTRIP: return GL_LINE_STRIP;
    case D3DPT_TRIANGLESTRIP: return GL_TRIANGLE_STRIP;
    case D3DPT_TRIANGLEFAN: return GL_TRIANGLE_FAN;
    default: return GL_TRIANGLES;
    }
}

UINT VertexCount(D3DPRIMITIVETYPE type, UINT primitiveCount)
{
    if (type == D3DPT_POINTLIST) return primitiveCount;
    if (type == D3DPT_LINELIST) return primitiveCount * 2;
    if (type == D3DPT_LINESTRIP) return primitiveCount + 1;
    if (type == D3DPT_TRIANGLELIST) return primitiveCount * 3;
    return primitiveCount + 2;
}

GLenum CompareFunction(DWORD function)
{
    switch (function)
    {
    case D3DCMP_NEVER: return GL_NEVER;
    case D3DCMP_LESS: return GL_LESS;
    case D3DCMP_EQUAL: return GL_EQUAL;
    case D3DCMP_LESSEQUAL: return GL_LEQUAL;
    case D3DCMP_GREATER: return GL_GREATER;
    case D3DCMP_NOTEQUAL: return GL_NOTEQUAL;
    case D3DCMP_GREATEREQUAL: return GL_GEQUAL;
    default: return GL_ALWAYS;
    }
}

GLenum BlendFunction(DWORD function)
{
    switch (function)
    {
    case D3DBLEND_ZERO: return GL_ZERO;
    case D3DBLEND_ONE: return GL_ONE;
    case D3DBLEND_SRCCOLOR: return GL_SRC_COLOR;
    case D3DBLEND_INVSRCCOLOR: return GL_ONE_MINUS_SRC_COLOR;
    case D3DBLEND_SRCALPHA: return GL_SRC_ALPHA;
    case D3DBLEND_INVSRCALPHA: return GL_ONE_MINUS_SRC_ALPHA;
    case D3DBLEND_DESTALPHA: return GL_DST_ALPHA;
    case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
    case D3DBLEND_DESTCOLOR: return GL_DST_COLOR;
    case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
    case D3DBLEND_SRCALPHASAT: return GL_SRC_ALPHA_SATURATE;
    default: return GL_ONE;
    }
}

bool TextureOperationUsesTexture(DWORD operation, DWORD argument1, DWORD argument2)
{
    if (operation == D3DTOP_DISABLE) return false;
    if (operation == D3DTOP_SELECTARG1)
        return (argument1 & D3DTA_SELECTMASK) == D3DTA_TEXTURE;
    return (argument1 & D3DTA_SELECTMASK) == D3DTA_TEXTURE ||
           (argument2 & D3DTA_SELECTMASK) == D3DTA_TEXTURE;
}

GLenum TextureArgumentSource(DWORD argument)
{
    switch (argument & D3DTA_SELECTMASK)
    {
    case D3DTA_TEXTURE: return GL_TEXTURE;
    case D3DTA_TFACTOR: return GL_CONSTANT;
    default: return GL_PRIMARY_COLOR;
    }
}

void ConfigureTextureComponent(GLenum combineParameter, GLenum source0Parameter,
                               GLenum source1Parameter, GLenum operand0Parameter,
                               GLenum operand1Parameter, DWORD operation,
                               DWORD argument1, DWORD argument2, GLenum operand)
{
    glTexEnvi(GL_TEXTURE_ENV, combineParameter,
              operation == D3DTOP_SELECTARG1 ? GL_REPLACE : GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, source0Parameter, TextureArgumentSource(argument1));
    glTexEnvi(GL_TEXTURE_ENV, operand0Parameter, operand);
    glTexEnvi(GL_TEXTURE_ENV, source1Parameter, TextureArgumentSource(argument2));
    glTexEnvi(GL_TEXTURE_ENV, operand1Parameter, operand);
}

class LinuxDevice : public IDirect3DDevice8
{
  public:
    LinuxDevice(SDL_Window *window_, const D3DPRESENT_PARAMETERS &parameters)
        : refs(1), window(window_), context(NULL), backbuffer(NULL), texture(NULL), vertexBuffer(NULL),
          fvf(0), streamStride(0), renderFramebuffer(0), renderColorTexture(0), renderDepthBuffer(0),
          dialogueSnapshotTexture(0), framebufferReady(false), dialogueSnapshotReady(false),
          wasDialogPresent(false), presentCount(0), probeLeft(0), probeBottom(0),
          probeWidth(0), probeHeight(0)
    {
        memset(renderStates, 0, sizeof(renderStates)); memset(textureStates, 0, sizeof(textureStates));
        Identity(&world); Identity(&view); Identity(&projection); Identity(&textureTransform);
        context = SDL_GL_CreateContext(window);
        if (context == NULL) return;
        SDL_GL_MakeCurrent(window, context);
        g_fogCoordf = reinterpret_cast<FogCoordfFunction>(SDL_GL_GetProcAddress("glFogCoordf"));
        if (g_fogCoordf == NULL)
            g_fogCoordf = reinterpret_cast<FogCoordfFunction>(SDL_GL_GetProcAddress("glFogCoordfEXT"));
        SDL_GL_SetSwapInterval(parameters.FullScreen_PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ? 0 : 1);
        framebufferReady = ResetInternal(parameters);
        renderStates[D3DRS_TEXTUREFACTOR] = 0xffffffffu;
        renderStates[D3DRS_SRCBLEND] = D3DBLEND_SRCALPHA;
        renderStates[D3DRS_DESTBLEND] = D3DBLEND_INVSRCALPHA;
        renderStates[D3DRS_ZWRITEENABLE] = TRUE;
        renderStates[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL; renderStates[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
        textureStates[D3DTSS_COLOROP] = D3DTOP_MODULATE;
        textureStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE; textureStates[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        textureStates[D3DTSS_ALPHAOP] = D3DTOP_MODULATE;
        textureStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE; textureStates[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        textureStates[D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP; textureStates[D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
        textureStates[D3DTSS_MINFILTER] = D3DTEXF_POINT; textureStates[D3DTSS_MAGFILTER] = D3DTEXF_POINT;
        glDisable(GL_CULL_FACE); glDisable(GL_LIGHTING);
    }
    ~LinuxDevice()
    {
        if (context != NULL) SDL_GL_MakeCurrent(window, context);
        if (texture != NULL) texture->Release();
        if (vertexBuffer != NULL) vertexBuffer->Release();
        if (backbuffer != NULL) backbuffer->Release();
        DestroyRenderTarget();
        if (context != NULL) SDL_GL_DeleteContext(context);
    }
    bool Ready() const { return context != NULL && backbuffer != NULL && framebufferReady; }
    ULONG AddRef() { return ++refs; }
    ULONG Release() { ULONG value = --refs; if (value == 0) delete this; return value; }
    HRESULT TestCooperativeLevel() { return S_OK; }
    HRESULT Reset(D3DPRESENT_PARAMETERS *parameters)
    {
        if (parameters == NULL) return E_INVALIDARG;
        framebufferReady = ResetInternal(*parameters);
        return framebufferReady ? S_OK : E_FAIL;
    }
    HRESULT Present(const RECT *, const RECT *, HWND, const RGNDATA *)
    {
        backbuffer->FlushBackbuffer();
        presentCount++;

        int drawableWidth, drawableHeight;
        SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
        g_framebufferApi.bindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glViewport(0, 0, drawableWidth, drawableHeight);

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_ALPHA_TEST); glDisable(GL_BLEND); glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING); glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, renderColorTexture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrtho(0.0, drawableWidth, drawableHeight, 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glColor4ub(255, 255, 255, 255);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(static_cast<float>(drawableWidth), 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, static_cast<float>(drawableHeight));
        glTexCoord2f(1.0f, 0.0f); glVertex2f(static_cast<float>(drawableWidth),
                                            static_cast<float>(drawableHeight));
        glEnd();
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
        glPopAttrib();

        glFlush();
        SDL_GL_SwapWindow(window);

        g_framebufferApi.bindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, backbuffer->width, backbuffer->height);
        return S_OK;
    }
    HRESULT GetBackBuffer(UINT index, D3DBACKBUFFER_TYPE, IDirect3DSurface8 **result)
    {
        if (index != 0 || result == NULL || backbuffer == NULL) return E_INVALIDARG;
        backbuffer->AddRef(); *result = backbuffer; return S_OK;
    }
    HRESULT CreateTexture(UINT width, UINT height, UINT, DWORD, D3DFORMAT format, D3DPOOL,
                          IDirect3DTexture8 **result)
    {
        if (result == NULL || width == 0 || height == 0) return E_INVALIDARG;
        if (format == D3DFMT_UNKNOWN) format = D3DFMT_A8R8G8B8;
        *result = new(std::nothrow) LinuxTexture(width, height, format);
        return *result != NULL ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT CreateVertexBuffer(UINT size, DWORD, DWORD, D3DPOOL, IDirect3DVertexBuffer8 **result)
    {
        if (result == NULL) return E_INVALIDARG;
        *result = new(std::nothrow) LinuxVertexBuffer(size); return *result != NULL ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT CreateRenderTarget(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, BOOL,
                               IDirect3DSurface8 **result)
    { return CreateSurface(width, height, format, result); }
    HRESULT CreateImageSurface(UINT width, UINT height, D3DFORMAT format, IDirect3DSurface8 **result)
    { return CreateSurface(width, height, format, result); }
    HRESULT CopyRects(IDirect3DSurface8 *sourceRaw, const RECT *sourceRects, UINT count,
                      IDirect3DSurface8 *destinationRaw, const POINT *destinationPoints)
    {
        LinuxSurfaceAccess source, destination;
        if (!th08_linux_surface_access(sourceRaw, &source, true) ||
            !th08_linux_surface_access(destinationRaw, &destination, false)) return E_INVALIDARG;
        if (source.format != destination.format) return E_NOTIMPL;
        if (count == 0) count = 1;
        UINT bytes = BytesPerPixel(source.format);
        for (UINT index = 0; index < count; ++index)
        {
            RECT rect;
            if (sourceRects != NULL) rect = sourceRects[index];
            else { rect.left = 0; rect.top = 0; rect.right = source.width; rect.bottom = source.height; }
            POINT point; point.x = destinationPoints != NULL ? destinationPoints[index].x : 0;
            point.y = destinationPoints != NULL ? destinationPoints[index].y : 0;
            UINT copyWidth = rect.right > rect.left ? rect.right - rect.left : 0;
            UINT copyHeight = rect.bottom > rect.top ? rect.bottom - rect.top : 0;
            if (point.x < 0 || point.y < 0 || rect.left < 0 || rect.top < 0) continue;
            if (static_cast<UINT>(point.x) + copyWidth > destination.width) copyWidth = destination.width - point.x;
            if (static_cast<UINT>(point.y) + copyHeight > destination.height) copyHeight = destination.height - point.y;
            for (UINT y = 0; y < copyHeight; ++y)
                memcpy(destination.pixels + (point.y + y) * destination.pitch + point.x * bytes,
                       source.pixels + (rect.top + y) * source.pitch + rect.left * bytes, copyWidth * bytes);
        }
        th08_linux_surface_changed(destinationRaw); return S_OK;
    }
    HRESULT BeginScene()
    {
        const bool dialogPresent = th08::g_Gui.IsDialoguePresent() != 0;
        if (dialogPresent && !wasDialogPresent)
            CaptureDialogueSnapshot();
        if (dialogPresent && dialogueSnapshotReady)
            RestoreDialogueSnapshot();
        wasDialogPresent = dialogPresent;
        return S_OK;
    }
    HRESULT EndScene() { return S_OK; }
    HRESULT Clear(DWORD, const D3DRECT *, DWORD flags, D3DCOLOR color, float depth, DWORD)
    {
        if ((flags & D3DCLEAR_TARGET) && getenv("TH08_LINUX_RENDER_TRACE") != NULL)
        {
            FILE *trace = fopen("modern-render.txt", "ab");
            if (trace != NULL)
            {
                fprintf(trace,
                        "frame=%lu flags=%08lx color=%08lx viewport=%lu,%lu,%lu,%lu caller=%p\n",
                        presentCount, static_cast<unsigned long>(flags), static_cast<unsigned long>(color),
                        static_cast<unsigned long>(viewport.X), static_cast<unsigned long>(viewport.Y),
                        static_cast<unsigned long>(viewport.Width), static_cast<unsigned long>(viewport.Height),
                        __builtin_return_address(0));
                fclose(trace);
            }
        }

        GLbitfield mask = 0;
        if (flags & D3DCLEAR_TARGET)
        {
            glClearColor(((color >> 16) & 255) / 255.0f, ((color >> 8) & 255) / 255.0f,
                         (color & 255) / 255.0f, ((color >> 24) & 255) / 255.0f);
            mask |= GL_COLOR_BUFFER_BIT;
        }
        if (flags & D3DCLEAR_ZBUFFER) { glClearDepth(depth); mask |= GL_DEPTH_BUFFER_BIT; }
        if (flags & D3DCLEAR_STENCIL) mask |= GL_STENCIL_BUFFER_BIT;
        const int targetWidth = backbuffer != NULL ? backbuffer->width : viewport.Width;
        const int targetHeight = backbuffer != NULL ? backbuffer->height : viewport.Height;
        const int left = static_cast<int>(viewport.X);
        const int bottom = targetHeight - static_cast<int>(viewport.Y + viewport.Height);
        const int width = static_cast<int>(viewport.Width);
        const int height = static_cast<int>(viewport.Height);
        glViewport(0, 0, targetWidth, targetHeight);
        glEnable(GL_SCISSOR_TEST);
        glScissor(left, bottom, width, height);
        if (flags & D3DCLEAR_ZBUFFER) glDepthMask(GL_TRUE);
        glClear(mask);
        if (flags & D3DCLEAR_ZBUFFER)
            glDepthMask(renderStates[D3DRS_ZWRITEENABLE] ? GL_TRUE : GL_FALSE);
        glDisable(GL_SCISSOR_TEST);
        return S_OK;
    }
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix)
    {
        if (matrix == NULL) return E_INVALIDARG;
        if (state == D3DTS_WORLD) world = *matrix;
        else if (state == D3DTS_VIEW) view = *matrix;
        else if (state == D3DTS_PROJECTION) projection = *matrix;
        else if (state == D3DTS_TEXTURE0) textureTransform = *matrix;
        return S_OK;
    }
    HRESULT SetViewport(const D3DVIEWPORT8 *value)
    { if (value == NULL) return E_INVALIDARG; viewport = *value; return S_OK; }
    HRESULT GetViewport(D3DVIEWPORT8 *value)
    { if (value == NULL) return E_INVALIDARG; *value = viewport; return S_OK; }
    HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
    { if (static_cast<UINT>(state) < 256) renderStates[state] = value; return S_OK; }
    HRESULT SetTexture(DWORD stage, IDirect3DTexture8 *value)
    {
        if (stage != 0) return S_OK;
        LinuxTexture *next = static_cast<LinuxTexture *>(value);
        if (next != NULL) next->AddRef();
        if (texture != NULL) texture->Release(); texture = next; return S_OK;
    }
    HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
    { if (stage == 0 && static_cast<UINT>(state) < 32) textureStates[state] = value; return S_OK; }
    HRESULT SetVertexShader(DWORD value) { fvf = value; return S_OK; }
    HRESULT SetStreamSource(UINT stream, IDirect3DVertexBuffer8 *value, UINT stride)
    {
        if (stream != 0) return E_INVALIDARG;
        LinuxVertexBuffer *next = static_cast<LinuxVertexBuffer *>(value);
        if (next != NULL) next->AddRef();
        if (vertexBuffer != NULL) vertexBuffer->Release();
        vertexBuffer = next; streamStride = stride; return S_OK;
    }
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount)
    {
        if (vertexBuffer == NULL || streamStride == 0) return E_FAIL;
        UINT offset = startVertex * streamStride, count = VertexCount(type, primitiveCount);
        if (offset + count * streamStride > vertexBuffer->bytes.size()) return E_INVALIDARG;
        return Draw(type, primitiveCount, &vertexBuffer->bytes[offset], streamStride);
    }
    HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT primitiveCount, const void *vertices, UINT stride)
    { return vertices != NULL ? Draw(type, primitiveCount, static_cast<const BYTE *>(vertices), stride) : E_INVALIDARG; }
    HRESULT GetDeviceCaps(D3DCAPS8 *caps)
    {
        if (caps == NULL) return E_INVALIDARG;
        memset(caps, 0, sizeof(*caps)); caps->DeviceType = D3DDEVTYPE_HAL;
        caps->Caps2 = D3DCAPS2_CANRENDERWINDOWED;
        caps->PresentationIntervals = D3DPRESENT_INTERVAL_ONE | D3DPRESENT_INTERVAL_IMMEDIATE;
        caps->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_HWRASTERIZATION |
                        D3DDEVCAPS_TEXTURESYSTEMMEMORY | D3DDEVCAPS_TEXTUREVIDEOMEMORY |
                        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY | D3DDEVCAPS_TLVERTEXVIDEOMEMORY;
        caps->MaxTextureWidth = caps->MaxTextureHeight = 4096;
        caps->MaxTextureBlendStages = 1; caps->MaxSimultaneousTextures = 1;
        caps->MaxPrimitiveCount = 0x100000; caps->MaxStreams = 1; caps->MaxStreamStride = 256;
        caps->TextureOpCaps = D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_SELECTARG1;
        return S_OK;
    }
    HRESULT ResourceManagerDiscardBytes(DWORD) { return S_OK; }

    bool BeginFramebufferProbe(int left, int top, int right, int bottom)
    {
        if (backbuffer == NULL || left >= right || top >= bottom)
            return false;
        if (left < 0) left = 0;
        if (top < 0) top = 0;
        if (right > static_cast<int>(backbuffer->width)) right = backbuffer->width;
        if (bottom > static_cast<int>(backbuffer->height)) bottom = backbuffer->height;
        if (left >= right || top >= bottom)
            return false;

        probeLeft = left;
        probeBottom = static_cast<int>(backbuffer->height) - bottom;
        probeWidth = right - left;
        probeHeight = bottom - top;
        probePixels.resize(probeWidth * probeHeight * 4);
        glReadPixels(probeLeft, probeBottom, probeWidth, probeHeight, GL_RGBA,
                     GL_UNSIGNED_BYTE, probePixels.empty() ? NULL : &probePixels[0]);
        return !probePixels.empty();
    }

    bool EndFramebufferProbe(LinuxFramebufferDeltaStats *stats)
    {
        if (stats == NULL || probePixels.empty() || probeWidth <= 0 || probeHeight <= 0)
            return false;

        std::vector<BYTE> after(probePixels.size());
        glReadPixels(probeLeft, probeBottom, probeWidth, probeHeight, GL_RGBA,
                     GL_UNSIGNED_BYTE, after.empty() ? NULL : &after[0]);
        memset(stats, 0, sizeof(*stats));
        stats->sampledPixels = probeWidth * probeHeight;
        for (UINT index = 0; index < stats->sampledPixels; ++index)
        {
            const BYTE *beforePixel = &probePixels[index * 4];
            const BYTE *afterPixel = &after[index * 4];
            const int redDifference = abs(static_cast<int>(afterPixel[0]) - beforePixel[0]);
            const int greenDifference = abs(static_cast<int>(afterPixel[1]) - beforePixel[1]);
            const int blueDifference = abs(static_cast<int>(afterPixel[2]) - beforePixel[2]);
            const int alphaDifference = abs(static_cast<int>(afterPixel[3]) - beforePixel[3]);
            if (redDifference == 0 && greenDifference == 0 && blueDifference == 0 && alphaDifference == 0)
                continue;

            ++stats->changedPixels;
            stats->absoluteRgbDifference += redDifference + greenDifference + blueDifference;
            const BYTE maximum = afterPixel[0] > afterPixel[1]
                ? (afterPixel[0] > afterPixel[2] ? afterPixel[0] : afterPixel[2])
                : (afterPixel[1] > afterPixel[2] ? afterPixel[1] : afterPixel[2]);
            const BYTE minimum = afterPixel[0] < afterPixel[1]
                ? (afterPixel[0] < afterPixel[2] ? afterPixel[0] : afterPixel[2])
                : (afterPixel[1] < afterPixel[2] ? afterPixel[1] : afterPixel[2]);
            if (maximum - minimum >= 32)
                ++stats->colorfulChangedPixels;
            const int maximumDifference = redDifference > greenDifference
                ? (redDifference > blueDifference ? redDifference : blueDifference)
                : (greenDifference > blueDifference ? greenDifference : blueDifference);
            const int minimumDifference = redDifference < greenDifference
                ? (redDifference < blueDifference ? redDifference : blueDifference)
                : (greenDifference < blueDifference ? greenDifference : blueDifference);
            if (maximumDifference - minimumDifference >= 8)
                ++stats->chromaticChangedPixels;
            if (afterPixel[0] >= 240 && afterPixel[1] >= 240 && afterPixel[2] >= 240)
                ++stats->nearWhiteChangedPixels;
        }
        probePixels.clear();
        probeWidth = probeHeight = 0;
        return true;
    }

  private:
    HRESULT CreateSurface(UINT width, UINT height, D3DFORMAT format, IDirect3DSurface8 **result)
    {
        if (result == NULL || width == 0 || height == 0) return E_INVALIDARG;
        if (format == D3DFMT_UNKNOWN) format = D3DFMT_A8R8G8B8;
        *result = new(std::nothrow) LinuxSurface(width, height, format, false, NULL);
        return *result != NULL ? S_OK : E_OUTOFMEMORY;
    }
    void DestroyRenderTarget()
    {
        if (dialogueSnapshotTexture != 0)
            glDeleteTextures(1, &dialogueSnapshotTexture);
        if (renderDepthBuffer != 0 && g_framebufferApi.deleteRenderbuffers != NULL)
            g_framebufferApi.deleteRenderbuffers(1, &renderDepthBuffer);
        if (renderFramebuffer != 0 && g_framebufferApi.deleteFramebuffers != NULL)
            g_framebufferApi.deleteFramebuffers(1, &renderFramebuffer);
        if (renderColorTexture != 0)
            glDeleteTextures(1, &renderColorTexture);
        renderDepthBuffer = renderFramebuffer = renderColorTexture = dialogueSnapshotTexture = 0;
        dialogueSnapshotReady = false;
        wasDialogPresent = false;
    }
    bool CreateRenderTarget(UINT width, UINT height)
    {
        if (!g_framebufferApi.Initialize())
        {
            fprintf(stderr, "th08-modern: OpenGL framebuffer objects are unavailable\n");
            return false;
        }

        glGenTextures(1, &renderColorTexture);
        glBindTexture(GL_TEXTURE_2D, renderColorTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glGenTextures(1, &dialogueSnapshotTexture);
        glBindTexture(GL_TEXTURE_2D, dialogueSnapshotTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        g_framebufferApi.genRenderbuffers(1, &renderDepthBuffer);
        g_framebufferApi.bindRenderbuffer(GL_RENDERBUFFER, renderDepthBuffer);
        g_framebufferApi.renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

        g_framebufferApi.genFramebuffers(1, &renderFramebuffer);
        g_framebufferApi.bindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
        g_framebufferApi.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_TEXTURE_2D, renderColorTexture, 0);
        g_framebufferApi.framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                 GL_RENDERBUFFER, renderDepthBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        GLenum status = g_framebufferApi.checkFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            fprintf(stderr, "th08-modern: unable to create OpenGL framebuffer (status 0x%04x)\n",
                    static_cast<unsigned int>(status));
            DestroyRenderTarget();
            return false;
        }
        return true;
    }
    void CaptureDialogueSnapshot()
    {
        if (dialogueSnapshotTexture == 0 || backbuffer == NULL)
            return;
        glBindTexture(GL_TEXTURE_2D, dialogueSnapshotTexture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, backbuffer->width, backbuffer->height);
        dialogueSnapshotReady = true;
    }
    void RestoreDialogueSnapshot()
    {
        const UINT width = backbuffer->width;
        const UINT height = backbuffer->height;
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_ALPHA_TEST); glDisable(GL_BLEND); glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING); glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, dialogueSnapshotTexture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glColor4ub(255, 255, 255, 255);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(static_cast<float>(width), 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, static_cast<float>(height));
        glTexCoord2f(1.0f, 0.0f); glVertex2f(static_cast<float>(width), static_cast<float>(height));
        glEnd();
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
        glPopAttrib();
    }
    bool ResetInternal(const D3DPRESENT_PARAMETERS &parameters)
    {
        UINT width = parameters.BackBufferWidth != 0 ? parameters.BackBufferWidth : 640;
        UINT height = parameters.BackBufferHeight != 0 ? parameters.BackBufferHeight : 480;
        D3DFORMAT format = parameters.BackBufferFormat;
        if (format == D3DFMT_UNKNOWN) format = D3DFMT_X8R8G8B8;
        DestroyRenderTarget();
        if (backbuffer != NULL) backbuffer->Release();
        backbuffer = new LinuxSurface(width, height, format, true, NULL);
        if (backbuffer == NULL || !CreateRenderTarget(width, height)) return false;
        viewport.X = viewport.Y = 0; viewport.Width = width; viewport.Height = height;
        viewport.MinZ = 0.0f; viewport.MaxZ = 1.0f;
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return true;
    }
    void TransformPosition(const float *position, bool transformed, float *xOut, float *yOut,
                           float *zOut, float *fogCoordinateOut)
    {
        if (transformed)
        {
            // D3D8 pre-transformed vertices use integer pixel centers, while
            // OpenGL samples at half-integer centers.
            *xOut = position[0] + 0.5f; *yOut = position[1] + 0.5f; *zOut = position[2];
            *fogCoordinateOut = 0.0f;
            return;
        }
        float vector[4] = {position[0], position[1], position[2], 1.0f};
        const D3DMATRIX *matrices[3] = {&world, &view, &projection};
        for (int index = 0; index < 3; ++index)
        {
            const D3DMATRIX &m = *matrices[index]; float next[4];
            next[0] = vector[0] * m._11 + vector[1] * m._21 + vector[2] * m._31 + vector[3] * m._41;
            next[1] = vector[0] * m._12 + vector[1] * m._22 + vector[2] * m._32 + vector[3] * m._42;
            next[2] = vector[0] * m._13 + vector[1] * m._23 + vector[2] * m._33 + vector[3] * m._43;
            next[3] = vector[0] * m._14 + vector[1] * m._24 + vector[2] * m._34 + vector[3] * m._44;
            memcpy(vector, next, sizeof(vector));
            if (index == 1)
                *fogCoordinateOut = fabsf(vector[2]);
        }
        float reciprocal = fabsf(vector[3]) > 1.0e-8f ? 1.0f / vector[3] : 1.0f;
        *xOut = viewport.X + (vector[0] * reciprocal + 1.0f) * viewport.Width * 0.5f;
        *yOut = viewport.Y + (1.0f - vector[1] * reciprocal) * viewport.Height * 0.5f;
        *zOut = viewport.MinZ + vector[2] * reciprocal * (viewport.MaxZ - viewport.MinZ);
    }
    void PrepareState()
    {
        const UINT width = backbuffer != NULL ? backbuffer->width : viewport.Width;
        const UINT height = backbuffer != NULL ? backbuffer->height : viewport.Height;
        const int drawableWidth = width;
        const int drawableHeight = height;
        glEnable(GL_SCISSOR_TEST);
        glScissor(static_cast<int>(viewport.X * drawableWidth / width),
                  drawableHeight - static_cast<int>((viewport.Y + viewport.Height) * drawableHeight / height),
                  static_cast<int>(viewport.Width * drawableWidth / width),
                  static_cast<int>(viewport.Height * drawableHeight / height));
        glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        if (renderStates[D3DRS_ALPHABLENDENABLE])
        { glEnable(GL_BLEND); glBlendFunc(BlendFunction(renderStates[D3DRS_SRCBLEND]), BlendFunction(renderStates[D3DRS_DESTBLEND])); }
        else glDisable(GL_BLEND);
        if (renderStates[D3DRS_ALPHATESTENABLE])
        { glEnable(GL_ALPHA_TEST); glAlphaFunc(CompareFunction(renderStates[D3DRS_ALPHAFUNC]), (renderStates[D3DRS_ALPHAREF] & 255) / 255.0f); }
        else glDisable(GL_ALPHA_TEST);
        if (renderStates[D3DRS_ZENABLE]) { glEnable(GL_DEPTH_TEST); glDepthFunc(CompareFunction(renderStates[D3DRS_ZFUNC])); }
        else glDisable(GL_DEPTH_TEST);
        glDepthMask(renderStates[D3DRS_ZWRITEENABLE] ? GL_TRUE : GL_FALSE);
        if (renderStates[D3DRS_FOGENABLE] &&
            renderStates[D3DRS_FOGVERTEXMODE] == D3DFOG_LINEAR && g_fogCoordf != NULL)
        {
            const DWORD color = renderStates[D3DRS_FOGCOLOR];
            const GLfloat fogColor[4] = {
                ((color >> 16) & 255) / 255.0f,
                ((color >> 8) & 255) / 255.0f,
                (color & 255) / 255.0f,
                1.0f
            };
            GLfloat fogStart, fogEnd;
            memcpy(&fogStart, &renderStates[D3DRS_FOGSTART], sizeof(fogStart));
            memcpy(&fogEnd, &renderStates[D3DRS_FOGEND], sizeof(fogEnd));
            glEnable(GL_FOG);
            glFogi(GL_FOG_MODE, GL_LINEAR);
            glFogi(GL_FOG_COORDINATE_SOURCE, GL_FOG_COORDINATE);
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogf(GL_FOG_START, fogStart);
            glFogf(GL_FOG_END, fogEnd);
        }
        else
        {
            glDisable(GL_FOG);
        }
        const bool colorUsesTexture = TextureOperationUsesTexture(
            textureStates[D3DTSS_COLOROP], textureStates[D3DTSS_COLORARG1],
            textureStates[D3DTSS_COLORARG2]);
        const bool alphaUsesTexture = TextureOperationUsesTexture(
            textureStates[D3DTSS_ALPHAOP], textureStates[D3DTSS_ALPHAARG1],
            textureStates[D3DTSS_ALPHAARG2]);
        if (texture != NULL && (colorUsesTexture || alphaUsesTexture))
        {
            texture->Upload(); glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, texture->glName);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureStates[D3DTSS_MINFILTER] == D3DTEXF_LINEAR ? GL_LINEAR : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureStates[D3DTSS_MAGFILTER] == D3DTEXF_LINEAR ? GL_LINEAR : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureStates[D3DTSS_ADDRESSU] == D3DTADDRESS_CLAMP ? GL_CLAMP : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureStates[D3DTSS_ADDRESSV] == D3DTADDRESS_CLAMP ? GL_CLAMP : GL_REPEAT);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            ConfigureTextureComponent(GL_COMBINE_RGB, GL_SOURCE0_RGB, GL_SOURCE1_RGB,
                                      GL_OPERAND0_RGB, GL_OPERAND1_RGB,
                                      textureStates[D3DTSS_COLOROP],
                                      textureStates[D3DTSS_COLORARG1],
                                      textureStates[D3DTSS_COLORARG2], GL_SRC_COLOR);
            ConfigureTextureComponent(GL_COMBINE_ALPHA, GL_SOURCE0_ALPHA, GL_SOURCE1_ALPHA,
                                      GL_OPERAND0_ALPHA, GL_OPERAND1_ALPHA,
                                      textureStates[D3DTSS_ALPHAOP],
                                      textureStates[D3DTSS_ALPHAARG1],
                                      textureStates[D3DTSS_ALPHAARG2], GL_SRC_ALPHA);
            const DWORD factor = renderStates[D3DRS_TEXTUREFACTOR];
            const GLfloat constantColor[4] = {
                ((factor >> 16) & 255) / 255.0f,
                ((factor >> 8) & 255) / 255.0f,
                (factor & 255) / 255.0f,
                ((factor >> 24) & 255) / 255.0f
            };
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constantColor);
        }
        else glDisable(GL_TEXTURE_2D);
    }
    D3DCOLOR EffectiveColor(D3DCOLOR diffuse)
    {
        if (texture != NULL &&
            (TextureOperationUsesTexture(textureStates[D3DTSS_COLOROP],
                                         textureStates[D3DTSS_COLORARG1],
                                         textureStates[D3DTSS_COLORARG2]) ||
             TextureOperationUsesTexture(textureStates[D3DTSS_ALPHAOP],
                                         textureStates[D3DTSS_ALPHAARG1],
                                         textureStates[D3DTSS_ALPHAARG2])))
            return diffuse;
        DWORD factor = renderStates[D3DRS_TEXTUREFACTOR];
        DWORD colorOp = textureStates[D3DTSS_COLOROP], alphaOp = textureStates[D3DTSS_ALPHAOP];
        DWORD colorArg1 = textureStates[D3DTSS_COLORARG1] & D3DTA_SELECTMASK;
        DWORD colorArg2 = textureStates[D3DTSS_COLORARG2] & D3DTA_SELECTMASK;
        DWORD alphaArg1 = textureStates[D3DTSS_ALPHAARG1] & D3DTA_SELECTMASK;
        DWORD alphaArg2 = textureStates[D3DTSS_ALPHAARG2] & D3DTA_SELECTMASK;
        DWORD modifier = colorArg2 == D3DTA_TFACTOR ? factor : diffuse;
        DWORD alphaModifier = alphaArg2 == D3DTA_TFACTOR ? factor : diffuse;
        DWORD rgb = modifier & 0x00ffffffu, alpha = alphaModifier & 0xff000000u;
        if (colorOp == D3DTOP_SELECTARG1 && colorArg1 == D3DTA_TEXTURE) rgb = 0x00ffffffu;
        else if (colorOp == D3DTOP_SELECTARG1 && colorArg1 == D3DTA_DIFFUSE) rgb = diffuse & 0x00ffffffu;
        if (alphaOp == D3DTOP_SELECTARG1 && alphaArg1 == D3DTA_TEXTURE) alpha = 0xff000000u;
        else if (alphaOp == D3DTOP_SELECTARG1 && alphaArg1 == D3DTA_DIFFUSE) alpha = diffuse & 0xff000000u;
        return alpha | rgb;
    }
    HRESULT Draw(D3DPRIMITIVETYPE type, UINT primitiveCount, const BYTE *data, UINT stride)
    {
        UINT count = VertexCount(type, primitiveCount);
        bool transformed = (fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW;
        UINT offset = transformed ? 16 : 12;
        if (fvf & D3DFVF_NORMAL) offset += 12;
        if (fvf & D3DFVF_PSIZE) offset += 4;
        bool hasDiffuse = (fvf & D3DFVF_DIFFUSE) != 0; UINT colorOffset = offset;
        if (hasDiffuse) offset += 4;
        if (fvf & D3DFVF_SPECULAR) offset += 4;
        bool hasTexture = (fvf & D3DFVF_TEXCOUNT_MASK) != 0; UINT textureOffset = offset;
        PrepareState(); glBegin(PrimitiveMode(type));
        for (UINT index = 0; index < count; ++index)
        {
            const BYTE *vertex = data + index * stride; float x, y, z, fogCoordinate;
            TransformPosition(reinterpret_cast<const float *>(vertex), transformed, &x, &y, &z,
                              &fogCoordinate);
            D3DCOLOR color = hasDiffuse ? *reinterpret_cast<const D3DCOLOR *>(vertex + colorOffset) : 0xffffffffu;
            color = EffectiveColor(color);
            if (hasTexture)
            {
                const float *uv = reinterpret_cast<const float *>(vertex + textureOffset);
                float u = uv[0], v = uv[1];
                if (!transformed)
                {
                    u = uv[0] * textureTransform._11 + uv[1] * textureTransform._21 + textureTransform._31;
                    v = uv[0] * textureTransform._12 + uv[1] * textureTransform._22 + textureTransform._32;
                }
                glTexCoord2f(u, v);
            }
            glColor4ub((color >> 16) & 255, (color >> 8) & 255, color & 255, (color >> 24) & 255);
            if (g_fogCoordf != NULL)
                g_fogCoordf(fogCoordinate);
            glVertex3f(x, y, 1.0f - 2.0f * z);
        }
        glEnd(); return S_OK;
    }
    ULONG refs;
    SDL_Window *window;
    SDL_GLContext context;
    LinuxSurface *backbuffer;
    LinuxTexture *texture;
    LinuxVertexBuffer *vertexBuffer;
    DWORD fvf;
    UINT streamStride;
    GLuint renderFramebuffer, renderColorTexture, renderDepthBuffer, dialogueSnapshotTexture;
    bool framebufferReady, dialogueSnapshotReady, wasDialogPresent;
    unsigned long presentCount;
    std::vector<BYTE> probePixels;
    int probeLeft, probeBottom, probeWidth, probeHeight;
    DWORD renderStates[256], textureStates[32];
    D3DMATRIX world, view, projection, textureTransform;
    D3DVIEWPORT8 viewport;
};

class LinuxDirect3D : public IDirect3D8
{
  public:
    LinuxDirect3D() : refs(1) {}
    ULONG AddRef() { return ++refs; }
    ULONG Release() { ULONG value = --refs; if (value == 0) delete this; return value; }
    HRESULT GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *mode)
    {
        if (mode == NULL) return E_INVALIDARG;
        mode->Width = 640; mode->Height = 480; mode->RefreshRate = 60; mode->Format = D3DFMT_X8R8G8B8; return S_OK;
    }
    HRESULT CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT) { return S_OK; }
    HRESULT CreateDevice(UINT, D3DDEVTYPE, HWND window, DWORD, D3DPRESENT_PARAMETERS *parameters,
                         IDirect3DDevice8 **result)
    {
        if (window == NULL || parameters == NULL || result == NULL) return E_INVALIDARG;
        LinuxDevice *device = new(std::nothrow) LinuxDevice(reinterpret_cast<SDL_Window *>(window), *parameters);
        if (device == NULL) return E_OUTOFMEMORY;
        if (!device->Ready()) { delete device; *result = NULL; return E_FAIL; }
        *result = device; return S_OK;
    }
  private: ULONG refs;
};
} // namespace

bool th08_linux_surface_access(IDirect3DSurface8 *surfaceRaw, LinuxSurfaceAccess *access, bool readBackbuffer)
{
    if (surfaceRaw == NULL || access == NULL) return false;
    LinuxSurface *surface = static_cast<LinuxSurface *>(surfaceRaw);
    if (surface->backbuffer || readBackbuffer) surface->ReadBackbuffer();
    access->pixels = surface->pixels.empty() ? NULL : &surface->pixels[0];
    access->width = surface->width; access->height = surface->height;
    access->pitch = surface->pitch; access->format = surface->format; return true;
}

void th08_linux_surface_changed(IDirect3DSurface8 *surfaceRaw)
{
    if (surfaceRaw == NULL) return;
    LinuxSurface *surface = static_cast<LinuxSurface *>(surfaceRaw);
    surface->dirty = true;
    if (surface->owner != NULL) surface->owner->uploaded = false;
    surface->FlushBackbuffer();
}

bool th08_linux_texture_region_stats(IDirect3DTexture8 *textureRaw, float u0, float v0,
                                     float u1, float v1, D3DCOLOR diffuse,
                                     LinuxTextureRegionStats *stats)
{
    if (textureRaw == NULL || stats == NULL)
        return false;
    LinuxTexture *texture = static_cast<LinuxTexture *>(textureRaw);
    LinuxSurface *surface = texture->surface;
    if (surface == NULL || surface->pixels.empty() || surface->width == 0 || surface->height == 0)
        return false;
    if (u0 != u0 || v0 != v0 || u1 != u1 || v1 != v1 ||
        fabsf(u0) > 16.0f || fabsf(v0) > 16.0f ||
        fabsf(u1) > 16.0f || fabsf(v1) > 16.0f)
        return false;

    if (u0 > u1) { const float swap = u0; u0 = u1; u1 = swap; }
    if (v0 > v1) { const float swap = v0; v0 = v1; v1 = swap; }
    int left = static_cast<int>(floorf(u0 * surface->width));
    int top = static_cast<int>(floorf(v0 * surface->height));
    int right = static_cast<int>(ceilf(u1 * surface->width));
    int bottom = static_cast<int>(ceilf(v1 * surface->height));
    if (left >= right || top >= bottom || right - left > static_cast<int>(surface->width) * 4 ||
        bottom - top > static_cast<int>(surface->height) * 4)
        return false;

    memset(stats, 0, sizeof(*stats));
    const UINT bytes = BytesPerPixel(surface->format);
    const BYTE diffuseAlpha = (diffuse >> 24) & 0xff;
    const BYTE diffuseRed = (diffuse >> 16) & 0xff;
    const BYTE diffuseGreen = (diffuse >> 8) & 0xff;
    const BYTE diffuseBlue = diffuse & 0xff;
    for (int y = top; y < bottom; ++y)
    {
        for (int x = left; x < right; ++x)
        {
            const int textureWidth = static_cast<int>(surface->width);
            const int textureHeight = static_cast<int>(surface->height);
            const int wrappedX = ((x % textureWidth) + textureWidth) % textureWidth;
            const int wrappedY = ((y % textureHeight) + textureHeight) % textureHeight;
            BYTE rgba[4];
            DecodePixel(&surface->pixels[wrappedY * surface->pitch + wrappedX * bytes],
                        surface->format, rgba);
            ++stats->sampledPixels;
            if (rgba[3] <= 8)
                continue;
            ++stats->visiblePixels;
            const BYTE maximum = rgba[0] > rgba[1]
                ? (rgba[0] > rgba[2] ? rgba[0] : rgba[2])
                : (rgba[1] > rgba[2] ? rgba[1] : rgba[2]);
            const BYTE minimum = rgba[0] < rgba[1]
                ? (rgba[0] < rgba[2] ? rgba[0] : rgba[2])
                : (rgba[1] < rgba[2] ? rgba[1] : rgba[2]);
            if (maximum - minimum >= 32)
                ++stats->colorfulPixels;
            if (rgba[0] >= 240 && rgba[1] >= 240 && rgba[2] >= 240)
                ++stats->nearWhitePixels;
            if (x == left || x == right - 1 || y == top || y == bottom - 1)
                ++stats->visibleEdgePixels;

            const BYTE modulatedAlpha = static_cast<BYTE>(rgba[3] * diffuseAlpha / 255U);
            if (modulatedAlpha <= 8)
                continue;
            ++stats->modulatedVisiblePixels;
            const BYTE modulatedRed = static_cast<BYTE>(rgba[0] * diffuseRed / 255U);
            const BYTE modulatedGreen = static_cast<BYTE>(rgba[1] * diffuseGreen / 255U);
            const BYTE modulatedBlue = static_cast<BYTE>(rgba[2] * diffuseBlue / 255U);
            const BYTE contributionRed =
                static_cast<BYTE>(modulatedRed * modulatedAlpha / 255U);
            const BYTE contributionGreen =
                static_cast<BYTE>(modulatedGreen * modulatedAlpha / 255U);
            const BYTE contributionBlue =
                static_cast<BYTE>(modulatedBlue * modulatedAlpha / 255U);
            const BYTE contributionMaximum = contributionRed > contributionGreen
                ? (contributionRed > contributionBlue ? contributionRed : contributionBlue)
                : (contributionGreen > contributionBlue ? contributionGreen : contributionBlue);
            const BYTE contributionMinimum = contributionRed < contributionGreen
                ? (contributionRed < contributionBlue ? contributionRed : contributionBlue)
                : (contributionGreen < contributionBlue ? contributionGreen : contributionBlue);
            if (contributionMaximum - contributionMinimum >= 8)
                ++stats->modulatedColorfulPixels;
            if (modulatedRed >= 240 && modulatedGreen >= 240 && modulatedBlue >= 240)
                ++stats->modulatedNearWhitePixels;
        }
    }
    return true;
}

bool th08_linux_begin_framebuffer_probe(IDirect3DDevice8 *deviceRaw, int left, int top,
                                        int right, int bottom)
{
    if (deviceRaw == NULL)
        return false;
    return static_cast<LinuxDevice *>(deviceRaw)->BeginFramebufferProbe(left, top, right, bottom);
}

bool th08_linux_end_framebuffer_probe(IDirect3DDevice8 *deviceRaw,
                                      LinuxFramebufferDeltaStats *stats)
{
    if (deviceRaw == NULL)
        return false;
    return static_cast<LinuxDevice *>(deviceRaw)->EndFramebufferProbe(stats);
}

extern "C" IDirect3D8 *Direct3DCreate8(UINT sdkVersion)
{ return sdkVersion == D3D_SDK_VERSION ? new(std::nothrow) LinuxDirect3D() : NULL; }
