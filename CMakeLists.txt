cmake_minimum_required(VERSION 3.20)

project(th08-modern LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "The first modern port target is Windows. Use a Windows or MinGW toolchain.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "TH08 currently requires a 32-bit Windows toolchain (-A Win32 or i686-w64-mingw32).")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(TH08_DX8_SDK_ROOT "${CMAKE_SOURCE_DIR}/scripts/prefix/mssdk" CACHE PATH
    "DirectX 8 SDK root containing include/ and lib/")
if(NOT EXISTS "${TH08_DX8_SDK_ROOT}/include/d3dx8.h")
    message(FATAL_ERROR
        "DirectX 8 SDK headers were not found. Set TH08_DX8_SDK_ROOT or run the existing prefix setup first.")
endif()

set(TH08_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
add_custom_command(
    OUTPUT "${TH08_GENERATED_DIR}/i18n.hpp"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${TH08_GENERATED_DIR}"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/generate_i18n.py"
            -o "${TH08_GENERATED_DIR}/i18n.hpp" --shift-jis
            "${CMAKE_SOURCE_DIR}/config/i18n.csv"
    DEPENDS
        "${CMAKE_SOURCE_DIR}/config/i18n.csv"
        "${CMAKE_SOURCE_DIR}/scripts/generate_i18n.py"
    VERBATIM
)
add_custom_target(th08-modern-generated DEPENDS "${TH08_GENERATED_DIR}/i18n.hpp")

set(TH08_GAME_SOURCES
    src/AnmManager.cpp
    src/AsciiManager.cpp
    src/AsciiManagerBossMarker.cpp
    src/AsciiManagerGauge.cpp
    src/AsciiManagerGuiMode.cpp
    src/AsciiManagerScale.cpp
    src/Background.cpp
    src/BulletManager.cpp
    src/EclDependencies.cpp
    src/EclExIns.cpp
    src/EclGlobals.cpp
    src/EclHelpers.cpp
    src/EclManager.cpp
    src/EclOperandsFloat.cpp
    src/EclOperandsInt.cpp
    src/EclRun.cpp
    src/EffectManager.cpp
    src/Ending.cpp
    src/EnemyManager.cpp
    src/EnemyManagerUpdate.cpp
    src/EnemyTimeline.cpp
    src/GameManager.cpp
    src/Global.cpp
    src/Gui.cpp
    src/ItemManager.cpp
    src/Midi.cpp
    src/MusicRoom.cpp
    src/Player.cpp
    src/PlayerBomb.cpp
    src/ReplayManager.cpp
    src/ResultScreen.cpp
    src/ScoreDat.cpp
    src/ScreenEffect.cpp
    src/SoundPlayer.cpp
    src/Spellcard.cpp
    src/Supervisor.cpp
    src/TextHelper.cpp
    src/TitleScreen.cpp
    src/main.cpp
    src/modern/windows_runtime.cpp
    src/utils.cpp
    src/zwave.cpp
    src/pbg/Lzss.cpp
    src/pbg/PbgArchive.cpp
    src/pbg/PbgFile.cpp
)

add_executable(th08-modern WIN32 ${TH08_GAME_SOURCES})
add_dependencies(th08-modern th08-modern-generated)
target_compile_features(th08-modern PRIVATE cxx_std_98)
target_compile_definitions(th08-modern PRIVATE
    TH08_MODERN_PORT
    WIN32_LEAN_AND_MEAN
)
target_include_directories(th08-modern PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${TH08_GENERATED_DIR}"
)

if(MINGW)
    # MinGW ships compatible D3D8/DirectInput/DirectSound headers and import
    # libraries, but not D3DX8. Search the legacy SDK only after the toolchain
    # headers so its old Windows SDK does not shadow modern system headers.
    set(TH08_DX8_SANITIZED_INCLUDE "${CMAKE_BINARY_DIR}/dx8-include")
    file(COPY "${TH08_DX8_SDK_ROOT}/include/" DESTINATION "${TH08_DX8_SANITIZED_INCLUDE}")
    file(GLOB TH08_DX8_HEADERS "${TH08_DX8_SANITIZED_INCLUDE}/*")
    string(ASCII 26 TH08_DOS_EOF)
    foreach(TH08_DX8_HEADER IN LISTS TH08_DX8_HEADERS)
        if(NOT IS_DIRECTORY "${TH08_DX8_HEADER}")
            file(READ "${TH08_DX8_HEADER}" TH08_DX8_HEADER_CONTENT)
            string(REPLACE "${TH08_DOS_EOF}" "" TH08_DX8_HEADER_CONTENT "${TH08_DX8_HEADER_CONTENT}")
            file(WRITE "${TH08_DX8_HEADER}" "${TH08_DX8_HEADER_CONTENT}")
        endif()
    endforeach()

    target_compile_options(th08-modern PRIVATE
        -fpermissive
        -mms-bitfields
        -Wno-write-strings
        "-include${CMAKE_SOURCE_DIR}/src/modern/windows_compat.hpp"
        "-idirafter${TH08_DX8_SANITIZED_INCLUDE}"
    )
    target_link_options(th08-modern PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_libraries(th08-modern PRIVATE
        # The retail D3DX8 library is a static MSVC C++ archive and cannot be
        # mixed with MinGW's C++ ABI. The SDK's debug DLL exposes the same C
        # API through an import library, which is suitable for bring-up builds.
        "${TH08_DX8_SDK_ROOT}/lib/d3dx8d.lib"
        dxguid dinput8 dsound d3d8 winmm
        kernel32 user32 gdi32 advapi32 shell32 ole32 oleaut32 uuid dbghelp
    )
    set(TH08_D3DX8_DEBUG_DLL
        "${TH08_DX8_SDK_ROOT}/samples/multimedia/support/d3dx8d.dll")
    if(EXISTS "${TH08_D3DX8_DEBUG_DLL}")
        add_custom_command(TARGET th08-modern POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${TH08_D3DX8_DEBUG_DLL}" "$<TARGET_FILE_DIR:th08-modern>"
            VERBATIM
        )
    else()
        message(WARNING "d3dx8d.dll was not found; th08-modern.exe will build but needs that SDK DLL to start.")
    endif()
elseif(MSVC)
    target_include_directories(th08-modern SYSTEM PRIVATE "${TH08_DX8_SDK_ROOT}/include")
    target_link_directories(th08-modern PRIVATE "${TH08_DX8_SDK_ROOT}/lib")
    target_compile_options(th08-modern PRIVATE /W3)
    target_link_libraries(th08-modern PRIVATE
        dxguid d3dx8 dinput8 dsound d3d8 winmm
        kernel32 user32 gdi32 advapi32 shell32 ole32 oleaut32 uuid dbghelp
    )
else()
    message(FATAL_ERROR "The modern Windows target currently supports MSVC and MinGW.")
endif()

set_target_properties(th08-modern PROPERTIES OUTPUT_NAME "th08-modern")
