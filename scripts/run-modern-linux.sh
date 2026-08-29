#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "usage: $0 /path/to/original-th08-directory"
    exit 0
fi
if [[ $# -ne 1 ]]; then
    echo "usage: $0 /path/to/original-th08-directory" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
data_dir="$1"

if [[ -n "${TH08_LINUX_BINARY:-}" ]]; then
    binary="${TH08_LINUX_BINARY}"
    if [[ "${binary}" != /* ]]; then
        binary="${repo_root}/${binary}"
    fi
elif [[ -x "${script_dir}/th08-modern" ]]; then
    # The CI package installs this launcher beside the ELF.
    binary="${script_dir}/th08-modern"
else
    binary="${repo_root}/build/modern-linux/th08-modern"
fi

if [[ ! -x "${binary}" ]]; then
    echo "Linux executable not found: ${binary}" >&2
    echo "Build it with scripts/build-modern-linux.sh, scripts/build-portable-linux.sh, or extract a complete CI package." >&2
    exit 1
fi

if [[ ! -d "${data_dir}" ]]; then
    echo "TH08 data directory not found: ${data_dir}" >&2
    exit 1
fi

data_dir="$(cd "${data_dir}" && pwd -P)"

if [[ ! -f "${data_dir}/th08.dat" ]]; then
    echo "Selected data directory does not contain th08.dat: ${data_dir}" >&2
    exit 1
fi
if [[ ! -f "${data_dir}/thbgm.dat" ]]; then
    echo "Selected data directory does not contain thbgm.dat: ${data_dir}" >&2
    exit 1
fi

exec "${binary}" --data-dir "${data_dir}"
