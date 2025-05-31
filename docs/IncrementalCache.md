# Incremental Graph Compilation Cache

This document describes the workflow for compiling computation graphs incrementally. The goal is to avoid recompiling unchanged kernels across runs while still generating deterministic artifacts.

## Overview

1. **Fingerprint** every node and configuration option in the graph. The resulting digest becomes the cache key.
2. **Check** the cache directory for an existing compiled object matching this key.
3. **Compile** only the kernels that are missing and store the results under the key.
4. **Link** the cached objects together to produce the final executable or WebAssembly module.

The cache lives in `.harmonics/cache/` by default but can be overridden with the `HARMONICS_CACHE_DIR` environment variable.

## Workflow Steps

### 1. Graph Fingerprinting

During graph construction, Harmonics serialises the structure and parameters deterministically and hashes the result using BLAKE3. Any change to layer order, weights, or code generation flags yields a new digest.

### 2. Cache Lookup

Before compilation starts, Harmonics queries the cache directory for a subfolder named after the digest. If all kernels for the current target exist, the build system reuses them directly.

### 3. Kernel Compilation

When a kernel is missing, the compiler backend generates the source, invokes the appropriate compiler (Clang, NVCC, etc.) and writes the output object file into the cache subfolder.

### 4. Final Linking

After all kernels are available, Harmonics links the cached objects into a single module. Because the objects remain unchanged between runs, only linking is required on subsequent compilations.

### 5. Cache Invalidation

Developers may invalidate the cache by deleting the digest folder or by changing build flags that influence code generation. A helper script `scripts/clear-cache.sh` is provided for convenience.

## Benefits

* Speeds up iterative development by skipping redundant compilations.
* Keeps builds deterministic because cache keys derive from the fully serialised graph.
* Works across machines when the cache directory is shared.

## Usage Example

The build system checks the incremental cache automatically when you
compile a graph. A typical workflow looks like:

```bash
cmake -S . -B build
cmake --build build --target my_graph -j$(nproc)
```

On the first run all kernels are compiled and stored under the cache
directory. Subsequent builds of the same graph reuse the cached objects
and simply link the final module.

Set `HARMONICS_CACHE_DIR=/custom/path` before building to override the
default location. Use `scripts/clear-cache.sh` to remove old entries
when the cache needs to be invalidated.

