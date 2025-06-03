# Deterministic Kernel & Neural‑Net Spec

*Draft v0.1 – May 31 2025*

---

## 1  Numerics

| Item               | Value                                                                   |
| ------------------ | ----------------------------------------------------------------------- |
| **Forward domain** | `int8_t` fixed‑point, scale = 2<sup>‑7</sup>                            |
| **Accumulator**    | `int32_t`, saturating add/mul                                           |
| **Learning rate**  | `LR_SHIFT ∈ {4,5,6,…}` → integer right‑shift                            |
| **Overflow**       | Wrap‑around disabled (`‑fwrapv`); explicit clamp to `INT8_MIN/INT8_MAX` |

All arithmetic is **branch‑free** and reproduces bit‑exactly across x86, ARM, WASM.

---

## 2  Topology (Variable Heads)

### 2.0 Core

Fixed dense MLP: `Input → 128 → 128 → 64 → Output(6)`.

### 2.1 Sensor & Appendage Slots

* `MAX_SENSORS = 4`, `MAX_APPENDAGES = 4`
* Each slot may be **inactive** (`active_flag = 0`) or take one of four preset hidden sizes drawn from the table `SIZE_TABLE = {32, 48, 64, 96}`.
* Per‑creature anatomy is derived *deterministically* at hatch time:

```text
topo_bits   = keccak256(seed)
sensor_count  = 1 + (topo_bits[0..1] % MAX_SENSORS)      # 1‑4
append_count  = 1 + (topo_bits[2..3] % MAX_APPENDAGES)   # 1‑4
sensor_size[i]  = SIZE_TABLE[ topo_bits[4+2i]  & 3 ]
append_size[i]  = SIZE_TABLE[ topo_bits[20+2i] & 3 ]
```

### 2.2 Fixed‑Slab Memory Layout

The kernel pre‑allocates a **constant slab** large enough for `4 × 96` hidden neurons per sensor *and* appendage. Unused rows/cols are multiplied by `active_flag = 0`, so the arithmetic circuit size never changes.

### 2.3 Masks in Forward/Backward (branch‑free)

```cpp
int8_t masked_input  = input  & flag;   // flag ∈ {0, 255}
int8_t masked_weight = weight & flag;
acc += masked_weight * masked_input;
```

---

## 3  Activations (INT8 LUT)  Activations (INT8 LUT)

| Name         | Index | Table size | Comment                            |
| ------------ | ----- | ---------- | ---------------------------------- |
| ReLU         | 0     | 256        | `max(0,x)`                         |
| Hard‑Sigmoid | 1     | 256        | Piecewise linear                   |
| Softmax      | 2     | 256        | Log‑sum‑exp pre‑baked & normalised |

Tables live on‑chain in `SeasonRegistry`; hash feeds into kernel ID.

---

## 4  Initialisation (Genesis)

```text
splitMix64(seed) → 64‑bit stream
w_i = ((r >> 2) mod 17) − 8   # uniform INT8 in [‑8,8]
b_i = 0
```

Seed = `keccak256(finalBlockHeader ∥ walletAddress)`.

---

## 5  Optimiser

Deterministic SGD

```text
w ← w − (grad >> LR_SHIFT)
```

*No momentum, no Adam.*  `LR_SHIFT` schedule per epoch: `[4,4,5,6,…]` (on‑chain).

---

## 6  Batching & Hash Lineage

* **Micro‑batch size** = 1 decision step (deterministic).
* **Checkpoint window** = 128 steps; one on‑chain tx per window.
* **Checkpoint payload**

  ```text
  {
    creatureID,
    epoch,
    h128,          // lineage hash after 128 steps
    energyBurn,    // ENERGY spent this window
    lrShift,       // integer LR schedule index
    activeMask     // 8‑bit bitmap of live sensor/append slots
  }
  ```
* **Lineage hash update**

  ```text
  h₀ = creatureDNA
  h_{t+1} = keccak256(activeMask ∥ liveWeights ∥ liveBiases ∥ h_t)
  ```

  `activeMask` flips 0→1 when a **growth‑spurt** pays ENERGY to awaken a dormant slot; validators reject a checkpoint if the mask bit changes without a matching `energyBurn`.
* **Energy double‑spend guard** – contract locks the claimed ENERGY within the tx, preventing reuse across two checkpoints in the same block.

---

## 7  Interfaces

### 7.1 Sensor Input

```
[tileType(3b), dx, dy, power, defense, stamina] → INT8[6]
```

### 7.2 Core Output

```
[move_N, move_S, move_E, move_W, atk_power, block_flag] → INT8[6]
```

### 7.3 C ABI

```c
void cb_forward(const int8_t* input, int8_t* action_out);
void cb_train(const int8_t* input,
              const int8_t* target,
              uint8_t lr_shift,
              uint8_t* energy_spent);
void cb_hash(uint8_t* out32);  // lineage hash
```

Pointers 32‑byte aligned; heap‑free inside kernel.

---

## 8  Parameter Caps

| Section             | Per‑slot max weights | Total creature max |
| ------------------- | -------------------- | ------------------ |
| Core                | 65 536               | 65 536             |
| Sensor slot (×4)    | 9 856                | 39 424             |
| Appendage slot (×4) | 9 856                | 39 424             |

*Overall snapshot stays < 144 kB even at full anatomy.*

\---------|-------------|
\| Core           | 65 536 |
\| Each sensor    | 4 096 |
\| Each appendage | 4 096 |

Snapshot ≤ 80 kB; easy to replay.

---

## 9  Training Loop (reference)

```cpp
for (uint32_t step = 0; step < 128; ++step) {
    auto a1 = relu(matmul_int8(w1, x));
    auto a2 = relu(matmul_int8(w2, a1));
    auto logits = matmul_int8(w3, a2);
    auto y_hat  = softmax_int8(logits);

    int32_t grad_logits[64];
    for (int i = 0; i < 64; ++i)
        grad_logits[i] = (int32_t)y_hat[i] - (int32_t)y_true[i];

    backprop_and_update(w3, w2, w1,
                        grad_logits,
                        a2, a1, x,
                        LR_SHIFT);
    h = keccak256(w1, w2, w3, h);
}
```

---

## 10  Tool‑chain & Determinism Guard

* **Compiler** `clang ≥16`, `-std=c++17` (baseline). C++20 may be enabled once protobuf/macOS issues are resolved; any change bumps `kernel_id`.
* Flags: `-O3 -fno-fast-math -fwrapv -fno-exceptions -fno-rtti`
* Cross‑targets: `x86_64`, `aarch64`, `wasm32-unknown-unknown` (Emscripten 3.x).
* CI asserts **object‑file SHA256** and **state‑hash** equality across targets. Divergence means determinism breach.

## 11  Proof & Governance  Proof & Governance

* **Phase 0** – quorum replay.
* **Phase 1** – STARK verifies: `loss↓ ∧ h_{t+128}` matches.
* Any change to object file → new `kernel_id` → governed 30‑day timelock before accept.
