#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=build-wasm
EM_CMAKE=${EM_CMAKE:-emcmake}

$EM_CMAKE cmake -S .. -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"
