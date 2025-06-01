# Per‑Battle Curriculum Reveal Protocol – Draft v0.1

---

## 1  Overview

This protocol lets a player “coach” a creature with a **private dataset** for a single duel while ensuring:

* The curriculum is **committed** up‑front (no late cherry‑picking).
* All validators and miners can replay gradients once the match block is final.
* Failure to reveal is slashable, so opponents aren’t stuck in unverifiable limbo.

All data lives **on‑chain**; off‑chain mirrors are optional caches.

---

## 2  Data Structures

| Field       | Bytes | When   | Purpose                                                    |
| ----------- | ----- | ------ | ---------------------------------------------------------- |
| `C_dataset` | 32    | Setup  | `keccak256(zstd(dataset))` commitment                      |
| `S_owner`   | 32    | Setup  | Owner‑chosen secret salt for PRNG                          |
| `sig`       | 65    | Setup  | `ecdsaSign(walletPriv, C_dataset ∥ S_owner ∥ battleBlock)` |
| `dataset`   | ≤512k | Reveal | Zstd‑compressed binary of samples                          |

*512 kB cap keeps calldata & gas bounded (<\$0.02 on typical L2).*
*Set `C_dataset = 0x0` for no custom curriculum.*

---

## 3  Transaction Flow

### 3.1 Challenge / Setup Tx

```text
function challenge(opponent, battleBlock, C_dataset, S_owner, sig) payable
```

* Verifies `sig` matches caller’s wallet.
* Escrows ENERGY stake and duel bond.
* Emits `DuelScheduled(duelID, …)`.

### 3.2 Reveal Tx (battleBlock ≤ now < battleBlock + Wr)

```text
function revealDataset(duelID, dataset, S_owner)
```

* Checks `keccak256(zstd(dataset)) == C_dataset`.
* Stores `dataset` bytes in contract storage + emits `DatasetRevealed` event.
* Moves duel to **ready** state; battle engine begins.

### 3.3 Forfeit Path

If `revealDataset` not called by `battleBlock + Wr`:

```text
function claimForfeit(duelID)
```

* Slashes challenger’s bond; opponent wins by default.
* ENERGY stake refunded to opponent.

`Wr` default = **3 blocks** (\~36 s on the L2 roll‑up).  Tunable via DAO.

---

## 4  Validator Logic

1. On `DatasetRevealed`, cache compressed bytes locally.
2. During checkpoint replay, PRNG seed = `seed_core ⊕ S_owner ⊕ globalStep`.
3. Generate labels → verify grads, loss‑drop, `h128`.
4. If mismatch, raise `FaultyCheckpoint` and trigger slashing.

Validators that **don’t** care about labels can still verify lineage hash; full checks cost ≈1 ms extra.

---

## 5  Gas & Storage Estimates

| Item                  | Gas (Arbitrum)                | Cost (@25 gwei L1, \$3k ETH) |
| --------------------- | ----------------------------- | ---------------------------- |
| Setup Tx (no dataset) | \~50k                         | < \$0.01                     |
| Reveal Tx (512 kB)    | \~350k + calldata             | \~\$0.02                     |
| Storage               | 512 kB × \$0.02/KB (one‑time) | \$10.24 (owner‑paid)         |

DAO could rebate 80 % of heavy storage to encourage innovation.

---

## 6  Security Properties

* **Censorship‑resistant** – dataset bytes live on‑chain; no external host needed.
* **No hidden dataset swap** – sig binds `C_dataset` to `battleBlock`; any later change invalidates signature.
* **Slash on non‑reveal** – economic penalty deters griefing.

---

## 7  Client UX

1. Player selects curriculum file → client zstd‑compresses + hashes.
2. Wallet signs & sends **challenge tx**.
3. Before `battleBlock`, client auto‑submits **reveal tx**.
4. Post‑battle, UI offers **publish dataset** toggle to share tactics.

---

© 2025 ChainBeasts Labs – draft
