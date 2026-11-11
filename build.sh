#!/bin/bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${root_dir}/build"

cmake -S "${root_dir}" -B "${build_dir}"
cmake --build "${build_dir}" --config Release --target surge-xt_Standalone surge-xt_VST3 --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-4}"
