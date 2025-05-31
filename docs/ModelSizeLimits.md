# Model Size Limits

To keep on-chain proofs tractable and prevent unbounded growth of creature models, Chain Beasts enforces a maximum combined size for the INT8 networks.

## Default Limit

The helper constant `neuropet::DEFAULT_MAX_MODEL_BYTES` is set to `64 * 1024` (64 KB). Validators reject any checkpoint whose total weights exceed this value.

## Runtime Enforcement

Call `neuropet::enforce_model_size()` before training or inference to throw a runtime error when the limit is exceeded:

```cpp
neuropet::CreatureModel model = ...;
neuropet::enforce_model_size(model); // throws if model too large
```

A custom limit can be passed to the function:

```cpp
std::size_t cap = 32 * 1024; // 32 KB
neuropet::enforce_model_size(model, cap);
```

## Motivation

Restricting model size keeps on-chain metadata light and bounds prover workload. Special leagues may raise or lower the cap but every block validator verifies compliance with the active limit.
