# Dataset Caching Layer

The runtime includes a very small caching layer for datasets that are fetched over HTTP or generated on the fly. Use the provided producers to persist samples between runs and skip expensive I/O or preprocessing steps.

## Enabling the Cache

Pass a file path when constructing an `HttpProducer` to enable caching:

```cpp
harmonics::HttpProducer loader("127.0.0.1", 8080, "/data", /*cache=*/true,
                               "dataset.cache");
```

If the cache file exists the records are loaded directly from disk. Otherwise the producer performs the HTTP request and stores the received tensors in the file for the next run. Set the `cache` argument to `false` to disable automatic reuse.

`AsyncHttpProducer` exposes the same parameters and behaves identically.

For arbitrary producers you can wrap them in `DiskCacheProducer`:

```cpp
auto csv = std::make_shared<harmonics::CsvProducer>("train.csv");
harmonics::DiskCacheProducer cached(csv, "train.cache");
```

The first run materialises all samples from `csv` and stores them in `train.cache`. Subsequent runs read directly from the cache file.

## Use Cases

* **Repeated integration tests** – store a small fixture dataset locally so that CI runs do not perform network traffic.
* **Preprocessed snapshots** – cache the output of expensive preprocessing stages to keep training loops fast.

## Example Training Loop

Below is a minimal example that downloads a dataset over HTTP, caches the
records to disk and iterates over the samples in a training loop:

```cpp
#include <harmonics/dataset.hpp>

auto loader = std::make_shared<harmonics::HttpProducer>(
    "127.0.0.1", 8080, "/data", /*cache=*/true, "dataset.cache");
harmonics::DiskCacheProducer cached(loader, "train.cache");

for (std::size_t i = 0; i < cached.size(); ++i) {
    auto sample = cached.next();
    // feed `sample` into the network here
}
```

The cache format simply concatenates serialized `HTensor` blobs and does not include any metadata. Delete the file whenever the dataset contents change.

## Incremental Updates and Remote Invalidation

`DiskCacheProducer` now supports appending new samples to an existing cache
file. When the wrapped dataset grows only the additional records are written to
disk which avoids rewriting the entire cache. A custom token callback can be
provided to detect remote invalidation:

```cpp
int version = 0;
auto token = [&]() { return std::to_string(version); };

auto base = std::make_shared<harmonics::CsvProducer>("train.csv");
harmonics::DiskCacheProducer cached(base, "train.cache", token);
```

When the value returned by `token` changes the cache file is discarded and
repopulated from the wrapped dataset.

## Cache Directory

All dataset caches are stored in `.harmonics/cache` by default.  Set the
environment variable `HARMONICS_CACHE_DIR` to override this location.  The
`cache_cli` utility and `scripts/clear-cache.sh` script respect the same
environment variable so you can inspect or purge caches globally:

```bash
export HARMONICS_CACHE_DIR=/tmp/hcache
tools/cache_cli list
scripts/clear-cache.sh
```
