#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 path/to/th08-modern x86_64|aarch64" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
binary="$1"
expected_arch="$2"

case "${expected_arch}" in
    amd64) expected_arch=x86_64 ;;
    arm64) expected_arch=aarch64 ;;
esac
if [[ "${binary}" != /* ]]; then
    binary="${repo_root}/${binary}"
fi
if [[ ! -x "${binary}" ]]; then
    echo "Portable Linux executable not found or not executable: ${binary}" >&2
    exit 1
fi

for command_name in file readelf nm; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Required verification command not found: ${command_name}" >&2
        exit 1
    fi
done

case "${expected_arch}" in
    x86_64) expected_machine='Advanced Micro Devices X86-64' ;;
    aarch64) expected_machine='AArch64' ;;
    *)
        echo "Unsupported verification architecture: ${expected_arch}" >&2
        exit 2
        ;;
esac

file_description="$(LC_ALL=C file -L "${binary}")"
elf_header="$(LC_ALL=C readelf --file-header "${binary}")"
symbols="$(LC_ALL=C nm --defined-only --demangle "${binary}")"

if [[ "${file_description}" != *"ELF 64-bit LSB pie executable"* ]]; then
    echo "Expected a 64-bit little-endian PIE executable:" >&2
    echo "  ${file_description}" >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Class:[[:space:]]+ELF64$' <<<"${elf_header}"; then
    echo "ELF header is not ELF64." >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Type:[[:space:]]+DYN ' <<<"${elf_header}"; then
    echo "Portable ELF is not position-independent (ET_DYN)." >&2
    exit 1
fi
if ! grep -Eq "^[[:space:]]*Machine:[[:space:]]+${expected_machine}$" <<<"${elf_header}"; then
    echo "Unexpected ELF machine; expected ${expected_machine}." >&2
    exit 1
fi

if grep -Eq '^[0-9a-fA-F]+[[:space:]]+A[[:space:]]+th08::g_' <<<"${symbols}"; then
    echo "Portable ELF still contains a fixed-address TH08 global:" >&2
    grep -E '^[0-9a-fA-F]+[[:space:]]+A[[:space:]]+th08::g_' <<<"${symbols}" >&2
    exit 1
fi

for runtime_global in 'th08::g_GameManager' 'th08::g_Supervisor' 'th08::g_EffectManager'; do
    if ! grep -Fq " ${runtime_global}" <<<"${symbols}"; then
        echo "Required native runtime global is missing: ${runtime_global}" >&2
        exit 1
    fi
done

for retired_alias in \
    'th08::g_EclGameTimeScaleFlags' \
    'th08::g_GuiMessageScreenEffectDuration' \
    'th08::g_SpellcardBackgroundAnm'; do
    if grep -Fq " ${retired_alias}" <<<"${symbols}"; then
        echo "Fixed-layout alias survived as independent native storage: ${retired_alias}" >&2
        exit 1
    fi
done

echo "Verified portable Linux ${expected_arch} executable:"
echo "  ${file_description}"
echo "  ELF64 PIE and native global ownership: OK"
