#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE=${1:-Release}
M=${2:-64}
N=${3:-64}
K=${4:-64}
ITER=${5:-1000}
BUILD_DIR=build-bench-${BUILD_TYPE}

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE
if command -v nproc >/dev/null 2>&1; then
    PARALLEL=$(nproc)
else
    PARALLEL=${NUMBER_OF_PROCESSORS:-1}
fi
cmake --build "$BUILD_DIR" --target int8_kernel_bench -j "$PARALLEL"
"$BUILD_DIR"/int8_kernel_bench "$M" "$N" "$K" "$ITER"
