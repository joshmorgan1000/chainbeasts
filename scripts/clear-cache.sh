#!/usr/bin/env bash
set -euo pipefail

CACHE_DIR="${HARMONICS_CACHE_DIR:-.harmonics/cache}"

if [ -d "$CACHE_DIR" ]; then
    echo "Removing cache directory: $CACHE_DIR"
    rm -rf "$CACHE_DIR"
else
    echo "Cache directory not found: $CACHE_DIR"
fi
