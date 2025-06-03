# Proof Workflow

The project now integrates a production-grade STARK prover distributed as
`libstark_prover_v2.so`. `ZkProofSystem` loads this shared library at runtime
and uses its C API to derive a proof from the batch tensors. The prover
interprets the concatenated tensors as the coefficients of a polynomial in
the field \(2^{31}-1\) and evaluates it at \(x = 17\) using a lightweight
FRI folding routine. The resulting field element is encoded in
little-endian order to form the proof bytes and hashed with **Keccak‑256**
to obtain the `checkpoint_root` that is submitted on-chain.


To build the stub with CMake run:

```bash
cmake -S . -B build -DNEUROPET_BUILD_INTERNAL_PROVER=ON
cmake --build build -j
```

If the resulting `libstark_prover_v2.so` is not on the library search path set
the `ZK_PROVER_PATH` environment variable to its absolute location before
starting the validator.

OpenSSL's EVP API is used for all digest operations. The proof system
invokes `EVP_Digest` directly, for example:

```cpp
unsigned char digest[32];
unsigned int len = sizeof(digest);
EVP_Digest(proof_bytes, sizeof(proof_bytes), digest,
           &len, EVP_sha3_256(), nullptr);
```

Earlier versions relied on the now deprecated SHA‑256 helpers, which have
been removed.

## 1. Generating Proofs

1. The client trainer runs 128 steps of training locally.
2. All tensors produced during the batch are concatenated and passed to the prover which evaluates the polynomial and generates the proof element.
3. The prover computes `checkpoint_root = keccak256(proof)` and returns both values.
4. The root and proof are bundled together for submission on-chain.
5. The `Validator` library exposes `generate_stark_proof()` and `prove_and_submit()` helpers which call into the prover library.

## 2. Submitting Proofs

Validators call the `submit_proof` method on the `ProofVerifier` contract with the `checkpoint_root` and proof bytes.  The accompanying `ZkVerifier` contract executes the verification circuits to ensure the proof is valid and marks the batch as proven when the check succeeds.

## 3. Prover Network

Any participant may generate proofs. Dedicated provers can help keep up with network demand by producing proofs in parallel. The on-chain contract only accepts a proof once and ignores duplicates.

## 4. Fallback Attestations

While Phase 0 relies on quorum attestations from validators, once the proof system is live those attestations become optional. A valid proof supersedes manual attestations and allows faster finalization of checkpoints.

## 5. CLI Usage

`proof_cli` provides a simple command line interface around the validator
library. Build it with CMake and pass a binary tensor file to generate a proof:

```bash
proof_cli tensors.bin
```

To immediately submit the proof to an on-chain verifier supply the RPC
endpoint and contract address:

```bash
proof_cli tensors.bin --submit 127.0.0.1:8545 0x0000000000000000000000000000000000000000
```

## 6. STARK Implementation Details

The prover is distributed as the `libstark_prover_v2.so` shared library which
exports a small C API:
`zk_generate_proof(const int8_t* data, size_t len, zk_proof_raw* out)` and
`zk_verify_proof(...)`.  `ZkProofSystem` loads the library at startup (optionally
using the `ZK_PROVER_PATH` environment variable) and forwards tensor data to
these functions.  The on-chain verifier runs the matching verification circuit
to confirm the `checkpoint_root` and marks the batch as proven when it succeeds.

## 7. Integrating with `ZkProofSystem`

`ZkProofSystem` acts as the bridge between the trainer and the on-chain
verifier.  After each 128-step window the validator calls
`generate_stark_proof()` with the concatenated tensors.  The resulting proof and
root hash are forwarded to `submit_proof()` which relays them to the smart
contract.  Set `ZK_PROVER_PATH` to point at a custom prover if the default
`libstark_prover_v2.so` is not in the library search path.

---

© 2025 ChainBeasts Labs
