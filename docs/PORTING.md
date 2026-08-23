# Playable reconstruction ports

The binary-exact VC7 build and the playable modern ports are separate build
products. The existing Ninja/VC7 path remains the evidence path for the
original Japanese 1.00d executable. CMake builds `th08-modern` from the same
authored game sources without making an exact-code claim.

## Current milestone: Windows x86

The first port keeps the original Win32, Direct3D 8, DirectInput 8,
DirectSound, and WinMM backends. This limits the first milestone to compiler
and runtime bring-up; it avoids changing gameplay and rendering semantics at
the same time. A 32-bit build is required because reconstructed layouts and
some behavior still depend on the original x86 pointer width.

On Linux, after creating the repository's existing DirectX 8 prefix:

```bash
cmake -S . -B build/modern-windows \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32-toolchain.cmake
cmake --build build/modern-windows --parallel
```

The MinGW bring-up build uses the SDK-only `d3dx8d.dll` because the retail
`d3dx8.lib` is a static MSVC C++ archive and cannot be linked into MinGW C++.
This debug DLL is copied only from the developer's local SDK into `build/`; it
is not a redistributable project artifact. Replacing the remaining D3DX calls
is required before a distributable MinGW build.

On 32-bit Windows with MSVC, configure with a Win32 generator and point
`TH08_DX8_SDK_ROOT` at a DirectX 8 SDK containing `include/` and `lib/`.

The executable still needs the legally obtained TH08 game data in its working
directory. The original executable is never embedded or copied into the
modern build.

## Port sequence

1. Compile and link all production-authored sources as Windows x86.
2. Boot through archive/config initialization to the title screen and record
   remaining runtime failures.
3. Isolate window, input, rendering, audio, timing, filesystem, and registry
   boundaries without changing gameplay code.
4. Replace platform backends for Linux and macOS while keeping replay-visible
   simulation behavior deterministic.
