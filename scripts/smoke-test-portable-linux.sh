#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "usage: $0 path/to/th08-modern path/to/TH08-data-dir [seconds]" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
binary="$1"
data_dir="$2"
duration="${3:-45}"

if [[ "${binary}" != /* ]]; then
    binary="${repo_root}/${binary}"
fi
if [[ "${data_dir}" != /* ]]; then
    data_dir="$(cd "${data_dir}" && pwd)"
fi
if [[ ! -x "${binary}" ]]; then
    echo "Portable Linux executable not found or not executable: ${binary}" >&2
    exit 1
fi
for archive in th08.dat thbgm.dat; do
    if [[ ! -f "${data_dir}/${archive}" ]]; then
        echo "Original data archive not found: ${data_dir}/${archive}" >&2
        exit 1
    fi
done
for command_name in file xvfb-run timeout; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Required smoke-test command not found: ${command_name}" >&2
        exit 1
    fi
done

runtime_command=("${binary}")
binary_description="$(LC_ALL=C file -L "${binary}")"
case "$(uname -m):${binary_description}" in
    x86_64:*'ARM aarch64'*|amd64:*'ARM aarch64'*)
        if ! command -v qemu-aarch64 >/dev/null 2>&1; then
            echo "qemu-aarch64 is required to smoke-test AArch64 on this host." >&2
            exit 1
        fi
        runtime_command=(qemu-aarch64 -L / "${binary}")
        ;;
esac

install -d "${repo_root}/build"
runtime_dir="$(mktemp -d "${repo_root}/build/portable-smoke.XXXXXX")"
cleanup_runtime_dir()
{
    if [[ "${TH08_KEEP_SMOKE_DIR:-0}" != 1 && "${runtime_dir}" == "${repo_root}"/build/portable-smoke.* ]]; then
        rm -rf -- "${runtime_dir}"
    fi
}
trap cleanup_runtime_dir EXIT

ln -s "${data_dir}/th08.dat" "${runtime_dir}/th08.dat"
ln -s "${data_dir}/thbgm.dat" "${runtime_dir}/thbgm.dat"

set +e
env SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe \
    xvfb-run -a -s '-screen 0 1024x768x24' \
    timeout --signal=TERM --kill-after=5s "${duration}s" \
    "${runtime_command[@]}" --data-dir "${runtime_dir}"
status=$?
set -e

if [[ -f "${runtime_dir}/modern-crash.txt" ]]; then
    echo "Portable runtime produced modern-crash.txt:" >&2
    sed -n '1,160p' "${runtime_dir}/modern-crash.txt" >&2
    exit 1
fi
if [[ ${status} -ne 124 ]]; then
    echo "Portable runtime exited before the ${duration}-second smoke window (status ${status})." >&2
    exit 1
fi
if [[ ! -f "${runtime_dir}/modern-files.txt" ]]; then
    echo "Portable runtime did not produce its archive request log." >&2
    exit 1
fi

required_requests=(
    title01.anm
    demo/demorpy0.rpy
    ply00a.sht
    stage5.std
    ecldata5.ecl
    msg5a.dat
)
for request in "${required_requests[@]}"; do
    if ! grep -Fq "path=${request}" "${runtime_dir}/modern-files.txt"; then
        echo "Smoke test did not reach required runtime resource: ${request}" >&2
        exit 1
    fi
done

cp "${runtime_dir}/modern-files.txt" "${repo_root}/build/portable-smoke-last-files.txt"
echo "Portable runtime smoke test passed for ${duration} seconds."
echo "  title -> demo replay -> Stage 5 resource load: OK"
echo "  request log: ${repo_root}/build/portable-smoke-last-files.txt"
