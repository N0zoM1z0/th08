#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
image_name="${TH08_LINUX_BUILD_IMAGE:-th08-modern-linux32}"
build_dir="${TH08_LINUX_BUILD_DIR:-build/modern-linux-container}"

if [[ "${build_dir}" = /* || "${build_dir}" == *..* ]]; then
    echo "TH08_LINUX_BUILD_DIR must be a repository-relative path without '..'." >&2
    exit 2
fi

docker build \
    --platform linux/386 \
    --file "${repo_root}/containers/linux32.Dockerfile" \
    --tag "${image_name}" \
    "${repo_root}"

docker run --rm \
    --platform linux/386 \
    --user "$(id -u):$(id -g)" \
    --volume "${repo_root}:/src" \
    --workdir /src \
    "${image_name}" \
    cmake -S . -B "${build_dir}" -G Ninja -DCMAKE_BUILD_TYPE=Debug

docker run --rm \
    --platform linux/386 \
    --user "$(id -u):$(id -g)" \
    --volume "${repo_root}:/src" \
    --workdir /src \
    "${image_name}" \
    cmake --build "${build_dir}" --parallel 1

echo "Built ${repo_root}/${build_dir}/th08-modern"
