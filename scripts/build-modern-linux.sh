#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
build_dir="${TH08_LINUX_BUILD_DIR:-build/modern-linux}"

if [[ "${build_dir}" = /* || "${build_dir}" == *..* ]]; then
    echo "TH08_LINUX_BUILD_DIR must be a repository-relative path without '..'." >&2
    exit 2
fi

case "$(uname -m)" in
    x86_64|amd64|i386|i486|i586|i686) ;;
    *)
        echo "The current Linux port requires an x86 or x86-64 host with i386 multilib support." >&2
        exit 1
        ;;
esac

missing_commands=()
for command_name in g++ pkg-config cmake ninja python3; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        missing_commands+=("${command_name}")
    fi
done
if (( ${#missing_commands[@]} != 0 )); then
    echo "Missing Linux build commands: ${missing_commands[*]}" >&2
    echo "Run scripts/setup-modern-linux.sh with your TH08 data directory." >&2
    exit 1
fi

pkg_config_libdir="${TH08_LINUX_PKG_CONFIG_LIBDIR:-/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig}"
export PKG_CONFIG_LIBDIR="${pkg_config_libdir}"

missing=()
for module in sdl2 SDL2_image SDL2_ttf fontconfig gl; do
    if ! pkg-config --exists "${module}"; then
        missing+=("${module}")
    fi
done
if (( ${#missing[@]} != 0 )); then
    echo "Missing i386 development modules: ${missing[*]}" >&2
    echo "On Debian/Ubuntu, enable i386 and install:" >&2
    echo "  g++-multilib libsdl2-dev:i386 libsdl2-image-dev:i386 libsdl2-ttf-dev:i386 libfontconfig1-dev:i386 libgl-dev:i386" >&2
    echo "Or run scripts/setup-modern-linux.sh with your TH08 data directory." >&2
    exit 1
fi

cd "${repo_root}"
cmake -S . -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=cmake/linux-i386-toolchain.cmake
cmake --build "${build_dir}" --parallel "${TH08_LINUX_BUILD_JOBS:-1}"

echo "Built ${repo_root}/${build_dir}/th08-modern"
