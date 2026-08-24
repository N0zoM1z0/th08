#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "usage: $0 path/to/th08-modern [output.tar.gz]"
    exit 0
fi
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "usage: $0 path/to/th08-modern [output.tar.gz]" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
binary="$1"
output="${2:-build/th08-modern-linux-i386.tar.gz}"

if [[ "${binary}" != /* ]]; then
    binary="${repo_root}/${binary}"
fi
if [[ "${output}" != /* ]]; then
    output="${repo_root}/${output}"
fi

"${script_dir}/verify-modern-linux.sh" "${binary}"

mkdir -p "$(dirname "${output}")" "${repo_root}/build"
staging_root="$(mktemp -d "${repo_root}/build/linux-package.XXXXXX")"
trap 'rm -rf "${staging_root}"' EXIT
package_dir="${staging_root}/th08-modern-linux-i386"
mkdir -p "${package_dir}"

install -m 0755 "${binary}" "${package_dir}/th08-modern"
install -m 0755 "${script_dir}/run-modern-linux.sh" "${package_dir}/run-th08.sh"
install -m 0644 "${repo_root}/resources/modern-icon.png" "${package_dir}/th08-modern.png"
install -m 0644 "${repo_root}/docs/LINUX_PACKAGE.md" "${package_dir}/README.md"
install -m 0644 "${repo_root}/docs/LINUX_PORTING.md" "${package_dir}/PORTING.md"
install -m 0644 "${repo_root}/LICENSE" "${package_dir}/LICENSE"

tar -czf "${output}" -C "${staging_root}" th08-modern-linux-i386
echo "Packaged ${output}"
