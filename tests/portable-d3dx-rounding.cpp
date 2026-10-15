// Feed identical binary32 inputs to the portable and original D3DX8 APIs.
#include <d3dx8.h>
#include <stdio.h>
#include <string.h>
#if defined(_WIN32)
#include <float.h>
#elif defined(__i386__)
#include <fenv.h>
#endif

static bool Read(void *destination, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
    {
        unsigned word;
        if (scanf("%u", &word) != 1) return false;
        memcpy(static_cast<char *>(destination) + i * 4, &word, 4);
    }
    return true;
}

static void Print(unsigned index, const void *source, unsigned count)
{
    printf("%u", index);
    for (unsigned i = 0; i < count; ++i)
    {
        unsigned word;
        memcpy(&word, static_cast<const char *>(source) + i * 4, 4);
        printf(" %u", word);
    }
    puts("");
}

int main()
{
#if defined(_WIN32)
    _controlfp(_PC_24 | _RC_NEAR, _MCW_PC | _MCW_RC);
#elif defined(__i386__)
    fenv_t environment;
    if (fegetenv(&environment)) return 2;
    environment.__control_word &= ~0xF00;
    if (fesetenv(&environment)) return 2;
#endif
    unsigned count;
    if (scanf("%u", &count) != 1 || count == 0 || count > 2048) return 2;
    for (unsigned index = 0; index < count; ++index)
    {
        unsigned kind;
        if (scanf("%u", &kind) != 1) return 2;
        if (kind == 0)
        {
            D3DXQUATERNION rotation;
            D3DXMATRIX matrix;
            if (!Read(&rotation, 4)) return 2;
            if (D3DXMatrixRotationQuaternion(&matrix, &rotation) != &matrix) return 3;
            Print(index, &matrix, 16);
        }
        else if (kind == 1)
        {
            D3DXVECTOR3 vector, result;
            D3DXMATRIX matrix;
            if (!Read(&vector, 3) || !Read(&matrix, 16)) return 2;
            if (D3DXVec3TransformCoord(&result, &vector, &matrix) != &result) return 3;
            Print(index, &result, 3);
        }
        else if (kind == 2)
        {
            D3DXVECTOR3 vector, result;
            D3DVIEWPORT8 viewport;
            D3DXMATRIX projection, view, world;
            if (!Read(&vector, 3) || !Read(&viewport, 6) || !Read(&projection, 16) ||
                !Read(&view, 16) || !Read(&world, 16)) return 2;
            if (D3DXVec3Project(&result, &vector, &viewport, &projection, &view, &world) != &result) return 3;
            Print(index, &result, 3);
        }
        else if (kind == 3)
        {
            D3DXVECTOR3 eye, at, up;
            D3DXMATRIX matrix;
            if (!Read(&eye, 3) || !Read(&at, 3) || !Read(&up, 3)) return 2;
            if (D3DXMatrixLookAtLH(&matrix, &eye, &at, &up) != &matrix) return 3;
            Print(index, &matrix, 16);
        }
        else if (kind == 4)
        {
            float parameters[4];
            D3DXMATRIX matrix;
            if (!Read(parameters, 4)) return 2;
            if (D3DXMatrixPerspectiveFovLH(&matrix, parameters[0], parameters[1], parameters[2], parameters[3]) != &matrix) return 3;
            Print(index, &matrix, 16);
        }
        else return 2;
    }
    return 0;
}
