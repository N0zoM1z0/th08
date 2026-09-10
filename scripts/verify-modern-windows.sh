#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "usage: $0 path/to/th08-modern.exe"
    exit 0
fi
if [[ $# -ne 1 ]]; then
    echo "usage: $0 path/to/th08-modern.exe" >&2
    exit 2
fi

binary="$1"
runtime_dir="$(cd "$(dirname "${binary}")" && pwd)"
binary="${runtime_dir}/$(basename "${binary}")"
d3dx_dll="${runtime_dir}/d3dx8d.dll"

if [[ ! -f "${binary}" ]]; then
    echo "Windows executable not found: ${binary}" >&2
    exit 1
fi
if [[ ! -f "${d3dx_dll}" ]]; then
    echo "Required development runtime not found beside the executable: ${d3dx_dll}" >&2
    exit 1
fi

binary_kind="$(file -b "${binary}")"
if [[ "${binary_kind}" != *"PE32 executable (GUI) Intel 80386"* ]]; then
    echo "Expected a PE32 Intel 80386 Windows GUI executable, got: ${binary_kind}" >&2
    exit 1
fi

pe_headers="$(i686-w64-mingw32-objdump -p "${binary}")"
if ! grep -Eq '^ImageBase[[:space:]]+00400000$' <<<"${pe_headers}"; then
    echo "Windows executable does not use the required 0x00400000 image base." >&2
    exit 1
fi
if ! grep -Eq '^Subsystem[[:space:]]+00000002[[:space:]]+\(Windows GUI\)$' <<<"${pe_headers}"; then
    echo "Windows executable does not use the GUI subsystem." >&2
    exit 1
fi
if ! grep -Eqi 'DLL Name: d3dx8d\.dll' <<<"${pe_headers}"; then
    echo "Windows executable does not import the expected DirectX 8 debug runtime." >&2
    exit 1
fi
if grep -Eqi 'DLL Name: (libgcc|libstdc\+\+|libwinpthread).*\.dll' <<<"${pe_headers}"; then
    echo "Windows executable unexpectedly depends on a MinGW support DLL." >&2
    exit 1
fi

echo "Windows i386 executable OK: ${binary}"
echo "Development-only D3DX runtime present: ${d3dx_dll}"
