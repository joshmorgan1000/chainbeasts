# INT8 Kernel Benchmark Results

This document contains baseline numbers for the `int8_matmul` kernel measured on a
64×64×64 matrix multiply. Values were collected using the `scripts/run_benchmarks.sh`
helper on an Intel Xeon processor running in Release mode.

```
$ ./scripts/run_benchmarks.sh
INT8 matmul 64x64x64
Iterations: 1000
Total time (s): 0.0239901
Ops/s: 1.09272e+10
```

On the reference system the benchmark completes around **10.9** GFLOP/s. These
numbers provide a starting point for future optimisations.
