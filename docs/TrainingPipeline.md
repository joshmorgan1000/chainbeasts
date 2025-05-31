# Production Training Pipeline

This guide outlines the steps required to run the official training pipeline that powers the on-chain creatures. It builds on the dataset caching and metrics streaming utilities already present in the project.

## 1. Build the Utilities

Compile the project as usual and make sure the example binaries are available:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

The `metrics_demo` program will be used for the examples below.

## 2. Start the Metrics Server

The training loop pushes step metrics over WebSocket. Launch the helper server from `scripts/`:

```bash
node scripts/metrics_server.js
```

Set the `METRICS_PORT` environment variable to change the port (default `8765`).

## 3. Enable Dataset Caching

When downloading or generating datasets wrap the base producer in
`DiskCacheProducer` to persist samples across runs. This avoids repeated
network I/O and preprocessing. Refer to
[`docs/DatasetCaching.md`](DatasetCaching.md) for the available helpers.
Cached files live under `.harmonics/cache` by default and you can override the
location with the `HARMONICS_CACHE_DIR` environment variable.

```cpp
auto base = std::make_shared<harmonics::HttpProducer>(
    "127.0.0.1", 8080, "/train");

neuropet::MetricsStreamer streamer{"127.0.0.1", 8765};
neuropet::run_dataset_pipeline(base, "train.cache", 10, streamer);
```

The first run downloads the dataset and writes `train.cache`. Subsequent runs
read directly from the cache.

## 4. Run the Training Loop

With the server running and the dataset prepared execute the demo program:

```bash
./build/metrics_demo
```

Each step emits a `TrainingMetrics` struct which is forwarded to all connected WebSocket clients. Production trainers follow the same pattern but operate on real models and datasets.

---

© 2025 Cognithesis Labs – Draft
