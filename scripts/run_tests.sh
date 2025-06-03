#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE=${1:-Release}
BUILD_DIR=build-${BUILD_TYPE}

# Ensure a clean graph cache for deterministic test runs.
"$(dirname "$0")/clear-cache.sh"
export HARMONICS_KERNEL_CACHE_DISABLE=1

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if command -v nproc >/dev/null 2>&1; then
    PARALLEL=$(nproc)
else
    PARALLEL=${NUMBER_OF_PROCESSORS:-1}
fi
cmake --build "$BUILD_DIR" -j "$PARALLEL"
ctest --test-dir "$BUILD_DIR" --output-on-failure
