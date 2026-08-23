#pragma once

#ifndef TH08_MODERN_PORT
#error "This header is only for the modern Windows build."
#endif

// Several original headers assume the monolithic VC7 PCH already supplied
// multimedia types before DirectSound and MIDI declarations.
#include <windows.h>
#include <mmsystem.h>
#include <d3d8.h>

// The original DirectX 8 SDK exported these aliases and constant. MinGW's
// compatible D3D8 header uses only the LP-prefixed aliases.
typedef LPDIRECT3D8 PDIRECT3D8;
typedef LPDIRECT3DDEVICE8 PDIRECT3DDEVICE8;

#ifndef D3DPRESENT_RATE_DEFAULT
#define D3DPRESENT_RATE_DEFAULT 0x00000000
#endif
