set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i386)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS_INIT "-m32")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-m32")

# CMake derives the i386 library architecture from the compiler probe. The
# build script separately constrains pkg-config so amd64 .pc files cannot leak
# into a 32-bit link.
