#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
target_arch="${1:-$(uname -m)}"

case "${target_arch}" in
    amd64) target_arch=x86_64 ;;
    arm64) target_arch=aarch64 ;;
esac

build_dir="${TH08_PORTABLE_BUILD_DIR:-build/portable-linux-${target_arch}}"
if [[ "${build_dir}" = /* || "${build_dir}" == *..* ]]; then
    echo "TH08_PORTABLE_BUILD_DIR must be a repository-relative path without '..'." >&2
    exit 2
fi

cmake_args=(
    -S .
    -B "${build_dir}"
    -G Ninja
    -DCMAKE_BUILD_TYPE=Debug
)

case "${target_arch}" in
    x86_64)
        if [[ "$(uname -m)" != x86_64 && "$(uname -m)" != amd64 ]]; then
            echo "The x86_64 portable build currently requires an x86_64 host." >&2
            exit 1
        fi
        compiler=g++
        default_pkg_config_libdir=/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig
        ;;
    aarch64)
        if [[ "$(uname -m)" == aarch64 || "$(uname -m)" == arm64 ]]; then
            compiler=g++
        else
            compiler=aarch64-linux-gnu-g++
            cmake_args+=(
                -DCMAKE_TOOLCHAIN_FILE=cmake/linux-aarch64-toolchain.cmake
            )
        fi
        default_pkg_config_libdir=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
        ;;
    *)
        echo "Unsupported portable Linux architecture: ${target_arch}" >&2
        echo "Supported architectures: x86_64, aarch64" >&2
        exit 2
        ;;
esac

missing_commands=()
for command_name in "${compiler}" pkg-config cmake ninja python3; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        missing_commands+=("${command_name}")
    fi
done
if (( ${#missing_commands[@]} != 0 )); then
    echo "Missing portable Linux build commands: ${missing_commands[*]}" >&2
    exit 1
fi

export PKG_CONFIG_LIBDIR="${TH08_PORTABLE_PKG_CONFIG_LIBDIR:-${default_pkg_config_libdir}}"

missing_modules=()
for module in sdl2 SDL2_image SDL2_ttf fontconfig gl; do
    if ! pkg-config --exists "${module}"; then
        missing_modules+=("${module}")
    fi
done
if (( ${#missing_modules[@]} != 0 )); then
    echo "Missing ${target_arch} development modules: ${missing_modules[*]}" >&2
    if [[ "${target_arch}" == aarch64 ]]; then
        echo "On Debian/Ubuntu, enable arm64 and install:" >&2
        echo "  libsdl2-dev:arm64 libsdl2-image-dev:arm64 libsdl2-ttf-dev:arm64 libfontconfig1-dev:arm64 libgl-dev:arm64" >&2
    fi
    exit 1
fi

cd "${repo_root}"
cmake "${cmake_args[@]}"
cmake --build "${build_dir}" --parallel "${TH08_PORTABLE_BUILD_JOBS:-1}"
"${repo_root}/scripts/verify-portable-linux.sh" "${build_dir}/th08-modern" "${target_arch}"

echo "Built ${repo_root}/${build_dir}/th08-modern"
