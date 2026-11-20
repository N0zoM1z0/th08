#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "usage: $0 path/to/th08-modern"
    exit 0
fi
if [[ $# -ne 1 ]]; then
    echo "usage: $0 path/to/th08-modern" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
binary="$1"
if [[ "${binary}" != /* ]]; then
    binary="${repo_root}/${binary}"
fi

if [[ ! -x "${binary}" ]]; then
    echo "Linux executable not found or not executable: ${binary}" >&2
    exit 1
fi

for command_name in file readelf nm; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Required verification command not found: ${command_name}" >&2
        exit 1
    fi
done

file_description="$(LC_ALL=C file -L "${binary}")"
elf_header="$(LC_ALL=C readelf --file-header "${binary}")"
elf_sections="$(LC_ALL=C readelf --wide --section-headers "${binary}")"
elf_dynamic="$(LC_ALL=C readelf --dynamic "${binary}")"
symbols="$(LC_ALL=C nm --defined-only --demangle "${binary}")"

if [[ "${file_description}" != *"ELF 32-bit LSB pie executable, Intel 80386"* ]]; then
    echo "Expected a 32-bit i386 PIE executable:" >&2
    echo "  ${file_description}" >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Class:[[:space:]]+ELF32$' <<<"${elf_header}"; then
    echo "ELF header is not ELF32." >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Type:[[:space:]]+DYN ' <<<"${elf_header}"; then
    echo "Expected ELF type ET_DYN." >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Machine:[[:space:]]+Intel 80386$' <<<"${elf_header}"; then
    echo "ELF machine is not Intel 80386." >&2
    exit 1
fi

if grep -Eq '[[:space:]]\.th08_target_data[[:space:]]' <<<"${elf_sections}" ||
   grep -Eq '^[[:xdigit:]]+[[:space:]]+A[[:space:]]+th08::' <<<"${symbols}"; then
    echo "Found a fixed-address TH08 section or symbol." >&2
    exit 1
fi
if grep -Eq '(^|[[:space:](])TEXTREL([[:space:])]|$)' <<<"${elf_dynamic}"; then
    echo "Found text relocations in the Linux executable." >&2
    exit 1
fi

for symbol_name in g_Spellcard g_GameManager g_Chain g_AnmManager; do
    if ! grep -Eq "^[[:xdigit:]]+[[:space:]]+[BD][[:space:]]+th08::${symbol_name}$" <<<"${symbols}"; then
        echo "Required runtime global has no native storage: ${symbol_name}" >&2
        exit 1
    fi
done

echo "Verified portable Linux executable:"
echo "  ${file_description}"
echo "  ELF32 PIE and section-backed TH08 symbols: OK"
