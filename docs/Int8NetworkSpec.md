# Deterministic INT8 Network Specification

This document describes the binary format used to store deterministic quantised neural networks. The same file loaded on any platform will produce identical results when evaluated with the INT8 kernel.

## 1. Overview

Networks consist of a sequence of layers executed in order. The reference implementation supports `Dense` and `ReLU` operations and can be extended in future versions. All numeric values are signed INT8 little‑endian unless otherwise noted.

## 2. File Layout

Each file begins with the ASCII magic `N8NW` followed by a 32‑bit little‑endian version number. Version `1` corresponds to the layout below.

```
N8NW             -- magic bytes
u32  version     -- currently 1
u32  layer_count
for each layer {
    u8   op      -- 0 = Dense, 1 = ReLU
    u32  input   -- number of input units
    u32  output  -- number of output units
    u32  weights -- byte count of weight array
    int8[weights]
    u32  bias    -- byte count of bias array
    int8[bias]
}
```

All integers use little‑endian byte order. Weights for `Dense` layers are stored in row‑major order with size `input * output`.

## 3. Deterministic Kernel

The runtime multiplies INT8 matrices using saturating arithmetic and accumulates in INT32. Activations such as ReLU are implemented with lookup tables of 256 entries stored on chain. The helper functions `save_network` and `load_network` in [`include/neuropet/int8_spec.hpp`](../include/neuropet/int8_spec.hpp) handle serialization.

## 4. Example

```cpp
neuropet::Int8Network net;
net.layers.push_back({neuropet::Int8Op::Dense, 4, 8,
                      {/* weights */}, {/* bias */}});
net.layers.push_back({neuropet::Int8Op::ReLU, 8, 8});
neuropet::save_network(net, "creature.n8");
```

Loading the saved file on another machine yields the exact same structure and outputs when evaluated.

