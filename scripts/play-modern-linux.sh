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

"${script_dir}/build-modern-linux.sh"
exec "${script_dir}/run-modern-linux.sh" "$1"
