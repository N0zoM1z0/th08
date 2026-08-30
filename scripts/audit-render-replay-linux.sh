#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 4 || $# -gt 5 ]]; then
    echo "usage: $0 path/to/th08-modern path/to/TH08-data-dir path/to/replay.rpy stage-index [seconds]" >&2
    exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
binary="$1"
data_dir="$2"
replay="$3"
stage_index="$4"
duration="${5:-90}"

if [[ ! "${stage_index}" =~ ^[0-9]+$ || ${stage_index} -gt 8 ]]; then
    echo "Stage index must be an integer from 0 through 8: ${stage_index}" >&2
    exit 2
fi
if [[ ! "${duration}" =~ ^[1-9][0-9]*$ ]]; then
    echo "Audit duration must be a positive integer number of seconds: ${duration}" >&2
    exit 2
fi

for variable_name in binary data_dir replay; do
    value="${!variable_name}"
    if [[ "${value}" != /* ]]; then
        printf -v "${variable_name}" '%s/%s' "${repo_root}" "${value}"
    fi
done
if [[ ! -x "${binary}" ]]; then
    echo "Portable Linux executable not found or not executable: ${binary}" >&2
    exit 1
fi
if [[ ! -f "${replay}" ]]; then
    echo "Replay not found: ${replay}" >&2
    exit 1
fi
for archive in th08.dat thbgm.dat; do
    if [[ ! -f "${data_dir}/${archive}" ]]; then
        echo "Original data archive not found: ${data_dir}/${archive}" >&2
        exit 1
    fi
done
for command_name in xvfb-run timeout gdb; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Required render-audit command not found: ${command_name}" >&2
        exit 1
    fi
done
keep_lives="${TH08_RENDER_AUDIT_KEEP_LIVES:-1}"
fast_replay="${TH08_RENDER_AUDIT_FAST:-1}"
if [[ "${keep_lives}" != 0 && "${keep_lives}" != 1 ]]; then
    echo "TH08_RENDER_AUDIT_KEEP_LIVES must be 0 or 1: ${keep_lives}" >&2
    exit 2
fi
if [[ "${fast_replay}" != 0 && "${fast_replay}" != 1 ]]; then
    echo "TH08_RENDER_AUDIT_FAST must be 0 or 1: ${fast_replay}" >&2
    exit 2
fi

install -d "${repo_root}/build"
runtime_dir="$(mktemp -d "${repo_root}/build/render-audit.XXXXXX")"
cleanup_runtime_dir()
{
    if [[ "${TH08_KEEP_RENDER_AUDIT_DIR:-0}" != 1 &&
          "${runtime_dir}" == "${repo_root}"/build/render-audit.* ]]; then
        rm -rf -- "${runtime_dir}"
    fi
}
trap cleanup_runtime_dir EXIT

install -d "${runtime_dir}/replay"
ln -s "${data_dir}/th08.dat" "${runtime_dir}/th08.dat"
ln -s "${data_dir}/thbgm.dat" "${runtime_dir}/thbgm.dat"
cp "${replay}" "${runtime_dir}/replay/audit.rpy"

gdb_script="${script_dir}/gdb/render-audit-normal-lives.gdb"
if [[ "${keep_lives}" == 1 ]]; then
    gdb_script="${script_dir}/gdb/render-audit-keep-lives.gdb"
fi
runtime_command=(
    gdb --batch --quiet
    -x "${gdb_script}"
    --args "${binary}" --data-dir "${runtime_dir}"
)

set +e
env SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe \
    TH08_AUTOPLAY_REPLAY=./replay/audit.rpy \
    TH08_AUTOPLAY_STAGE="${stage_index}" \
    TH08_RENDER_AUDIT=1 \
    TH08_RENDER_AUDIT_FAST="${fast_replay}" \
    xvfb-run -a -s '-screen 0 1024x768x24' \
    timeout --signal=TERM --kill-after=5s "${duration}s" \
    "${runtime_command[@]}" >"${runtime_dir}/audit-runtime.log" 2>&1
status=$?
set -e

if [[ -f "${runtime_dir}/modern-crash.txt" ]]; then
    echo "Render audit produced modern-crash.txt:" >&2
    sed -n '1,160p' "${runtime_dir}/modern-crash.txt" >&2
    exit 1
fi
if grep -Eq 'received signal SIG[A-Z0-9]+' "${runtime_dir}/audit-runtime.log"; then
    echo "Render audit stopped on a debugger-visible signal:" >&2
    sed -n '1,160p' "${runtime_dir}/audit-runtime.log" >&2
    exit 1
fi
if [[ ${status} -ne 0 && ${status} -ne 124 ]]; then
    echo "Portable runtime exited before the ${duration}-second audit window (status ${status})." >&2
    sed -n '1,160p' "${runtime_dir}/audit-runtime.log" >&2
    exit 1
fi
if [[ ${status} -eq 0 ]] &&
   ! grep -Eq 'exited normally|exited with code 0' "${runtime_dir}/audit-runtime.log"; then
    echo "Debugger exited without a clean inferior exit or timeout." >&2
    sed -n '1,160p' "${runtime_dir}/audit-runtime.log" >&2
    exit 1
fi

python3 "${script_dir}/check-render-audit.py" \
    "${runtime_dir}/modern-enemy-render.csv" \
    --require-stage "${stage_index}" --min-samples 5
cp "${runtime_dir}/modern-enemy-render.csv" "${repo_root}/build/render-audit-last.csv"
cp "${runtime_dir}/modern-files.txt" "${repo_root}/build/render-audit-last-files.txt"
cp "${runtime_dir}/audit-runtime.log" "${repo_root}/build/render-audit-last-runtime.log"
echo "Replay render audit passed for stage index ${stage_index}."
echo "  audit: ${repo_root}/build/render-audit-last.csv"
if [[ "${keep_lives}" == 1 ]]; then
    patch_hits="$(grep -c 'render-audit: suppressed AddLives' \
        "${runtime_dir}/audit-runtime.log" || true)"
    echo "  external no-life-decrement test hits: ${patch_hits}"
fi
