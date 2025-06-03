# INT8 Kernel Benchmark Results

This document contains baseline numbers for the `int8_matmul` kernel measured on a
64×64×64 matrix multiply. Values were collected using the `scripts/run_benchmarks.sh`
helper on an Intel Xeon processor running in Release mode. The script now accepts
optional `M N K ITER` arguments to benchmark different sizes.

```
$ ./scripts/run_benchmarks.sh Release
INT8 matmul 64x64x64
Iterations: 1000
Total time (s): 0.22616
Ops/s: 1.15911e+09
Time/iter (ms): 0.22616
```

On the reference system the benchmark completes around **10.9** GFLOP/s. These
numbers provide a starting point for future optimisations.

### GPU Notes

Enable the Vulkan backend to benchmark the GPU kernels:

```bash
export HARMONICS_ENABLE_VULKAN=1
./scripts/run_benchmarks.sh Release
```

Specify `HARMONICS_VULKAN_DEVICE=<index>` when more than one GPU is installed.
With a discrete GPU the `int8_matmul` kernel usually runs 5–10× faster than the
CPU baseline.
