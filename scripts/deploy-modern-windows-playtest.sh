#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "usage: $0 /path/to/original-th08-directory /path/to/playtest-directory"
    exit 0
fi
if [[ $# -ne 2 ]]; then
    echo "usage: $0 /path/to/original-th08-directory /path/to/playtest-directory" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
data_dir="$1"
playtest_dir="$2"
build_dir="${TH08_WINDOWS_BUILD_DIR:-build/modern-windows}"

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

mkdir -p "${playtest_dir}"
playtest_dir="$(cd "${playtest_dir}" && pwd -P)"
if [[ "${playtest_dir}" == "${data_dir}" ]]; then
    echo "The playtest directory must be separate from the original game directory." >&2
    exit 1
fi

"${script_dir}/build-modern-windows.sh"

binary="${repo_root}/${build_dir}/th08-modern.exe"
d3dx_dll="${repo_root}/${build_dir}/d3dx8d.dll"
install -m 0755 "${binary}" "${playtest_dir}/th08-reconstructed.exe"
install -m 0644 "${d3dx_dll}" "${playtest_dir}/d3dx8d.dll"
install -m 0644 "${script_dir}/run-modern-windows.bat" "${playtest_dir}/run-th08-reconstruction.bat"

for archive in th08.dat thbgm.dat; do
    if [[ ! -e "${playtest_dir}/${archive}" ]]; then
        cp "${data_dir}/${archive}" "${playtest_dir}/${archive}"
    elif ! cmp -s "${data_dir}/${archive}" "${playtest_dir}/${archive}"; then
        echo "Refusing to replace a different existing archive: ${playtest_dir}/${archive}" >&2
        exit 1
    fi
done

for mutable_file in th08.cfg score.dat; do
    if [[ -f "${data_dir}/${mutable_file}" && ! -e "${playtest_dir}/${mutable_file}" ]]; then
        cp "${data_dir}/${mutable_file}" "${playtest_dir}/${mutable_file}"
    fi
done

if [[ -d "${data_dir}/replay" && ! -e "${playtest_dir}/replay" ]]; then
    cp -a "${data_dir}/replay" "${playtest_dir}/replay"
else
    mkdir -p "${playtest_dir}/replay"
fi
mkdir -p "${playtest_dir}/backup"

if command -v wslpath >/dev/null 2>&1; then
    windows_path="$(wslpath -w "${playtest_dir}")"
    echo "Prepared native Windows playtest directory: ${windows_path}"
    echo "Double-click ${windows_path}\\run-th08-reconstruction.bat"
else
    echo "Prepared native Windows playtest directory: ${playtest_dir}"
fi
echo "This local bring-up directory contains the non-redistributable SDK d3dx8d.dll; do not publish it."
