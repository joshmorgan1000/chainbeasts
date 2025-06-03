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

### Real Dataset Example

A small CSV file containing battle records is provided under `examples/battle_samples.csv`. Convert it to the HDF5 format using the `dataset_convert` tool:

```bash
dataset_convert --csv examples/battle_samples.csv -o battle.hdf5
```

Run the pipeline with the converted dataset:

```cpp
auto data = std::make_shared<harmonics::Hdf5Producer>("battle.hdf5");
neuropet::run_dataset_pipeline(data, "train.cache", 10, streamer);
```

On subsequent runs `train.cache` is loaded immediately and `battle.hdf5` is only consulted when the cache is missing.

## 4. Run the Training Loop

With the server running and the dataset prepared execute the demo program:

```bash
./build/metrics_demo
```
To demonstrate checkpoint caching use the companion example:

```bash
./build/checkpoint_demo    # creates or updates trainer.ckpt
```

Running the program again resumes from the previous checkpoint and continues
streaming metrics from the next step.

Each step emits a `TrainingMetrics` struct which is forwarded to all connected WebSocket clients. Production trainers follow the same pattern but operate on real models and datasets.

## 5. Production Pipeline Overview

The official trainer links together the dataset cache, incremental graph
cache and metrics streamer. When invoked with the same dataset URL and cache
name, subsequent runs skip both the HTTP download and kernel compilation
phases. See [IncrementalCache.md](IncrementalCache.md) for a detailed overview
of how the graph cache stores compiled kernels between runs.

```bash
# example invocation using the demo binaries
./build/metrics_demo           # reads train.cache and streams metrics
```

All caches reside under `.harmonics/cache` unless the
`HARMONICS_CACHE_DIR` environment variable is set.  Sharing this directory
across machines allows trainers to reuse compiled kernels and cached
datasets, dramatically reducing startup time.

## 6. Continuous Training Integration

Continuous training runs can resume seamlessly when both caches persist
between sessions. Invoke the trainer with the same dataset cache name and
graph cache key to pick up where the previous execution left off. Only new
graph variations trigger kernel compilation while existing objects are loaded
from the cache directory.

The `tools/graph_cache_cli` utility helps manage cached kernels:

```bash
tools/graph_cache_cli list    # inspect cached digests
tools/graph_cache_cli clear   # remove all cached kernels
```

Combined with dataset caching this allows workers to restart frequently
without incurring download or compilation costs, enabling a continuous
training loop.

---

# Combat & Training Logistics – Draft v0.1

---

## 1  Deterministic Combat Resolution

| Phase             | Rule                                                                                                                           | Integer math                                |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------- |
| **Start‑of‑turn** | `stamina -= 1` if last action was `attack` or `block`. Creature with `stamina = 0` can *only* `block` until it rests one turn. | 8‑bit clamp                                 |
| **Action commit** | Core net outputs logits `[move_N,S,E,W, atk_power, block_flag]`; appendage maps to legal action.                               | Argmax for move; `atk_power` saturates 0‑15 |
| **Movement**      | Resolve first; hitting `WALL` cancels; entering `HAZARD` costs +2 stamina.                                                     |                                             |
| **Attack**        | If adjacent: `damage = atk_power × (256 / (256 + enemy_defense))`                                                              | INT16 intermediate, floor                   |
| **Block**         | `temp_def += 32` (cap 96) lasts one enemy hit then resets.                                                                     |                                             |
| **Crit flag**     | If `critFlag == 1`, `damage ×= 2`.                                                                                             |                                             |
| **Victory**       | Enemy HP ≤ 0 **or** max 60 turns.                                                                                              |                                             |

*All constants stored in `SeasonRegistry` → tweakable without forking proof circuit.*

---

## 2  Training Labels

| Head             | Label                                                      | Loss          |
| ---------------- | ---------------------------------------------------------- | ------------- |
| **Move logits**  | One‑hot of A\* optimal step toward enemy else zero vector. | Cross‑entropy |
| **Attack power** | `(expected_damage × 15 / max_damage)` → int 0‑15.          | MSE           |
| **Block flag**   | 1 if `temp_def < enemy_atk × 0.8`, else 0.                 | BCE           |

### Curriculum

* Epoch 0‑2: train **move** only.
* Epoch 3‑5: unlock **attack power**.
* Epoch 6+: unlock **block** head.

LR schedule handles per‑epoch changes.

---

## 3  Sample Generation (Fully Deterministic)

| Source               | Key derivation                                                      | Storage                       |
| -------------------- | ------------------------------------------------------------------- | ----------------------------- |
| **Self‑play**        | `(genesis_seed, checkpoint_root)` → PRNG for enemy moves.           | Regenerates on demand         |
| **Real battles**     | Hash of board state from match log.                                 | Off‑chain cache pinned by key |
| **Curriculum tasks** | Hard‑coded board templates; index = `blockHash(epoch_start)` mod N. | Kernel ROM table              |

Trainer pulls sample → computes label via rules above → runs one SGD step. Validators/STARK reproduce exactly.

---

## 4  ENERGY & Growth‑Spurts

* Cost per micro‑step: `baseCost × activeParams/slabParams`.
* **Growth‑spurt** unlocking a dormant slot burns extra ENERGY; kernel flips the slot bit in `activeMask` inside that 128‑step window.
* Checkpoint payload: `{… energyBurn, activeMask, lrShift …}`. Contract locks ENERGY in the same tx.

---

## 5  Tunable Constants

* `temp_def += 32` and cap 96.
* Damage curve $currently hyperbola$.
* Curriculum epoch lengths.

*All tweakable via governance‑timelock; proof circuit unchanged.*

---

# Private Curriculum Sampling – Spec v0.1

> *Purpose* Let owners “coach” their beasts with a private dataset while every gradient update stays verifiable and deterministic.

---

## 1  One‑time Commitment at Hatch

| Field       | Size | Description                                             |
| ----------- | ---- | ------------------------------------------------------- |
| `C_dataset` | 32 B | `keccak256(zstd(dataset))` (owner‑supplied)             |
| `S_owner`   | 32 B | Random secret chosen by wallet; never revealed on‑chain |

Both values are embedded in the ERC‑721 token’s immutable metadata. `C_dataset = 0x0` means *no private curriculum*.

---

## 2  Sample Selection during Training

```text
# inside kernel, per micro‑step
sampleIdx = splitMix64( seed_core ⊕ S_owner ⊕ globalStep ) mod DATASET_LEN
```

* `seed_core` standard hatch seed (see HATCHING.md).
* `globalStep` monotonic counter stored in checkpoint.
* Trainer loads `dataset[sampleIdx]` from local storage.

Because `S_owner` never leaves the client, outsiders cannot predict sample order even if they guess the dataset.

---

## 3  Label Computation

Labels follow the deterministic rules in **TrainingPipeline.md §1** using the selected sample.
Validators that *do* possess the dataset can recompute labels; otherwise they skip label checks and only verify the lineage hash and loss‑drop flag.

### Dispute Path

1. Challenger posts a bond and supplies `{dataset, S_owner}`.
2. Contract checks `keccak256(zstd(dataset)) == C_dataset`.
3. Validators replay disputed checkpoints with full label verification.
4. If divergence found → offender slashed, challenger rewarded.

---

## 4  Checkpoint Payload Extension

Adds one new field:

| Field        | Bytes | Note                                           |
| ------------ | ----- | ---------------------------------------------- |
| `globalStep` | 4     | Total micro‑steps since genesis (wraps at 2³²) |

Other fields unchanged (`energyBurn, lrShift, activeMask, h128`).

---

## 5  ENERGY Economics

* **No extra fee** to use a private curriculum—players already pay per step.
* **Dispute filing bond** burns 2× the ENERGY window cost to deter spam.

---

## 6  Storage Notes

* Dataset is *never* uploaded on‑chain.
* Recommended max compressed size per beast: **512 kB** to keep audits tractable.

---

### Security & Research Angle

Reverse‑engineering the hidden dataset from observed gradients becomes a real‑world penetration test for model‑stealing defenses—exactly the incentive the game hopes to spark.

---

© 2025 ChainBeasts Labs – Draft
