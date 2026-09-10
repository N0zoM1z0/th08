#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
build_dir="${TH08_WINDOWS_BUILD_DIR:-build/modern-windows}"

if [[ "${build_dir}" = /* || "${build_dir}" == *..* ]]; then
    echo "TH08_WINDOWS_BUILD_DIR must be a repository-relative path without '..'." >&2
    exit 2
fi

missing_commands=()
for command_name in cmake ninja python3 i686-w64-mingw32-g++ i686-w64-mingw32-objdump file; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        missing_commands+=("${command_name}")
    fi
done
if (( ${#missing_commands[@]} != 0 )); then
    echo "Missing Windows i386 build commands: ${missing_commands[*]}" >&2
    exit 1
fi

dx8_root="${TH08_DX8_SDK_ROOT:-${repo_root}/scripts/prefix/mssdk}"
for sdk_file in \
    "${dx8_root}/include/d3dx8.h" \
    "${dx8_root}/lib/d3dx8d.lib" \
    "${dx8_root}/samples/multimedia/support/d3dx8d.dll"; do
    if [[ ! -f "${sdk_file}" ]]; then
        echo "Required DirectX 8 SDK file not found: ${sdk_file}" >&2
        echo "Run the repository prefix setup or set TH08_DX8_SDK_ROOT." >&2
        exit 1
    fi
done

cd "${repo_root}"
cmake -S . -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32-toolchain.cmake \
    -DTH08_DX8_SDK_ROOT="${dx8_root}"
cmake --build "${build_dir}" --parallel "${TH08_WINDOWS_BUILD_JOBS:-1}"
"${script_dir}/verify-modern-windows.sh" "${repo_root}/${build_dir}/th08-modern.exe"

echo "Built ${repo_root}/${build_dir}/th08-modern.exe"
