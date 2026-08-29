#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "usage: $0 path/to/th08-modern x86_64|aarch64 [output.tar.gz]" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
binary="$1"
target_arch="$2"

case "${target_arch}" in
    amd64) target_arch=x86_64 ;;
    arm64) target_arch=aarch64 ;;
esac
case "${target_arch}" in
    x86_64|aarch64) ;;
    *)
        echo "Unsupported portable package architecture: ${target_arch}" >&2
        exit 2
        ;;
esac

output="${3:-build/th08-portable-linux-${target_arch}.tar.gz}"
if [[ "${binary}" != /* ]]; then
    binary="${repo_root}/${binary}"
fi
if [[ "${output}" != /* ]]; then
    output="${repo_root}/${output}"
fi

"${script_dir}/verify-portable-linux.sh" "${binary}" "${target_arch}"

install -d "$(dirname "${output}")" "${repo_root}/build"
staging_root="$(mktemp -d "${repo_root}/build/portable-package.XXXXXX")"
cleanup_staging_root()
{
    if [[ "${staging_root}" == "${repo_root}"/build/portable-package.* ]]; then
        rm -rf -- "${staging_root}"
    fi
}
trap cleanup_staging_root EXIT

package_name="th08-portable-linux-${target_arch}"
package_dir="${staging_root}/${package_name}"
install -d "${package_dir}"

install -m 0755 "${binary}" "${package_dir}/th08-modern"
install -m 0755 "${script_dir}/run-modern-linux.sh" "${package_dir}/run-th08.sh"
install -m 0644 "${repo_root}/resources/modern-icon.png" "${package_dir}/th08-modern.png"
install -m 0644 "${repo_root}/docs/PORTABLE_64BIT.md" "${package_dir}/README.md"
install -m 0644 "${repo_root}/LICENSE" "${package_dir}/LICENSE"

tar -czf "${output}" -C "${staging_root}" "${package_name}"
echo "Packaged ${output}"
