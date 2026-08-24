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
symbols="$(LC_ALL=C nm --defined-only --demangle "${binary}")"

if [[ "${file_description}" != *"ELF 32-bit LSB executable, Intel 80386"* ]]; then
    echo "Expected a 32-bit i386 ELF executable:" >&2
    echo "  ${file_description}" >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Class:[[:space:]]+ELF32$' <<<"${elf_header}"; then
    echo "ELF header is not ELF32." >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Type:[[:space:]]+EXEC ' <<<"${elf_header}"; then
    echo "ELF is not a fixed-address executable (ET_EXEC)." >&2
    exit 1
fi
if ! grep -Eq '^[[:space:]]*Machine:[[:space:]]+Intel 80386$' <<<"${elf_header}"; then
    echo "ELF machine is not Intel 80386." >&2
    exit 1
fi

check_absolute_symbol() {
    local expected_address="$1"
    local symbol_name="$2"
    local symbol_line
    local actual_address

    symbol_line="$(grep -E "^[0-9a-fA-F]+[[:space:]]+A[[:space:]]+${symbol_name}$" <<<"${symbols}" || true)"
    if [[ -z "${symbol_line}" ]]; then
        echo "Required absolute layout symbol is missing: ${symbol_name}" >&2
        exit 1
    fi
    actual_address="${symbol_line%% *}"
    if [[ "${actual_address,,}" != "${expected_address,,}" ]]; then
        echo "Unexpected address for ${symbol_name}: ${actual_address} (expected ${expected_address})" >&2
        exit 1
    fi
}

check_absolute_symbol 004ea670 'th08::g_Spellcard'
check_absolute_symbol 004eccac 'th08::g_SpellcardCalcChain'
check_absolute_symbol 0160f508 'th08::g_GameManager'
check_absolute_symbol 0164f548 'th08::g_Chain'
check_absolute_symbol 018bdc90 'th08::g_AnmManager'

echo "Verified portable Linux executable:"
echo "  ${file_description}"
echo "  fixed target-owned layout symbols: OK"
