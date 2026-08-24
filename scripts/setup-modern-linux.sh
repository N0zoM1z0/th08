#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "usage: $0 /path/to/original-th08-directory"
    echo "Installs Debian/Ubuntu i386 dependencies when needed, builds, and runs TH08."
    exit 0
fi
if [[ $# -ne 1 ]]; then
    echo "usage: $0 /path/to/original-th08-directory" >&2
    echo "Installs Debian/Ubuntu i386 dependencies when needed, builds, and runs TH08." >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
data_dir="$1"

if [[ ! -d "${data_dir}" ]]; then
    echo "TH08 data directory not found: ${data_dir}" >&2
    exit 1
fi
data_dir="$(cd "${data_dir}" && pwd -P)"
for archive in th08.dat thbgm.dat; do
    if [[ ! -f "${data_dir}/${archive}" ]]; then
        echo "Selected data directory does not contain ${archive}: ${data_dir}" >&2
        exit 1
    fi
done

case "$(uname -m)" in
    x86_64|amd64|i386|i486|i586|i686) ;;
    *)
        echo "The current Linux port requires an x86 or x86-64 host with i386 multilib support." >&2
        exit 1
        ;;
esac

pkg_config_libdir="${TH08_LINUX_PKG_CONFIG_LIBDIR:-/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig}"
modules=(sdl2 SDL2_image SDL2_ttf fontconfig gl)

dependencies_ready=true
for command_name in g++ pkg-config cmake ninja python3; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        dependencies_ready=false
    fi
done
if [[ "${dependencies_ready}" == true ]]; then
    for module in "${modules[@]}"; do
        if ! PKG_CONFIG_LIBDIR="${pkg_config_libdir}" pkg-config --exists "${module}"; then
            dependencies_ready=false
        fi
    done
fi

if [[ "${dependencies_ready}" != true ]]; then
    if ! command -v apt-get >/dev/null 2>&1 || ! command -v dpkg >/dev/null 2>&1; then
        echo "Automatic dependency installation currently supports Debian and Ubuntu." >&2
        echo "Install an i386 multilib compiler plus SDL2, SDL2_image, SDL2_ttf, Fontconfig, and OpenGL development packages." >&2
        exit 1
    fi

    administrator=()
    if (( EUID != 0 )); then
        if ! command -v sudo >/dev/null 2>&1; then
            echo "sudo is required to install missing system packages." >&2
            exit 1
        fi
        administrator=(sudo)
    fi

    if ! dpkg --print-foreign-architectures | grep -qx i386; then
        "${administrator[@]}" dpkg --add-architecture i386
    fi
    "${administrator[@]}" apt-get update
    "${administrator[@]}" env DEBIAN_FRONTEND=noninteractive apt-get install -y \
        g++-multilib \
        cmake \
        ninja-build \
        pkg-config \
        python3 \
        fonts-vlgothic \
        libsdl2-dev:i386 \
        libsdl2-image-dev:i386 \
        libsdl2-ttf-dev:i386 \
        libfontconfig1-dev:i386 \
        libgl-dev:i386
fi

exec "${script_dir}/play-modern-linux.sh" "${data_dir}"
