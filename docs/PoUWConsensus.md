# Proof-of-Useful-Work Consensus

This guide describes how the sovereign PoUW chain replaces traditional mining with deterministic training batches. Validators (miners) run the official INT8 kernel and submit the resulting checkpoints to produce new blocks.

## 1. Overview

Each block on the PoUW chain commits the root hash of a training batch. Miners execute a fixed number of kernel steps (128 by default) on the latest checkpoint and compute a proof using the integrated STARK prover. The proof bytes are hashed with **Keccak‑256** to derive the `checkpoint_root` submitted on-chain. Rewards are minted directly to the miner once the batch is verified.

## 2. Mining a Block

1. Sync the latest checkpoint from the chain.
2. Run the reference kernel for `128` steps to produce the next state.
3. Pass the batch tensors to `ZkProofSystem` which outputs the proof bytes and
   computes `checkpoint_root = keccak256(proof)`.
4. Sign the root with your wallet key.
5. Submit `{creature_id, epoch, checkpoint_root, signature}` to the `TrainingLedger` contract using `chain-cli`.

Validators watching the mempool replay the same batch. When a quorum of attestations is reached—or a STARK proof is provided—the block finalizes and the miner receives \$CORE and \$ENERGY rewards.

Example submission:

```bash
chain-cli submit-checkpoint <CREATURE_ID> <EPOCH> <ROOT> --rpc <RPC_URL> --key <KEY>
```

## 3. Rewards

The `TrainingLedger` emits a reward event once verification succeeds. The block header includes the checkpoint root and miner address:

```text
parent_hash      = <prev>
checkpoint_root  = <ROOT>
miner_address    = <ADDR>
```

When the block finalizes the `Energy` contract mints \$ENERGY to the miner via
`rewardMiner` and the `Core` contract mints \$CORE. These transferable tokens can
be sent to other players or spent on future training batches, battles and
governance participation.

Miners can immediately begin the next batch using the new root.

## 4. Consensus Rules

The PoUW chain finalizes a checkpoint once it meets the following criteria:

1. **Quorum or Proof** – a supermajority of validators attest to the same
   `checkpoint_root` *or* a valid STARK proof for the batch is submitted.
2. **Pending Entry** – the checkpoint must have been submitted via
   `submit_checkpoint` and not finalized already.
3. **Sequential Epochs** – `epoch_id` must match the next expected epoch for the
   creature. Out-of-order checkpoints are rejected.
4. **Monotonic Loss** – the reported loss cannot be higher than the previous
   finalized checkpoint for that creature.

When these rules pass the block is appended to the chain and the miner receives
`ENERGY_REWARD` and `CORE_REWARD` as defined in the protocol.

## 5. Finalization Rules

A checkpoint becomes final once one of the following conditions is met:

1. At least two thirds of active validators submit matching attestations.
2. A STARK proof generated by `ZkProofSystem` verifies on chain.

If competing roots exist for the same parent block, the chain selects the root
with the greater total attestation weight or any root accompanied by a valid
proof. The `TrainingLedger` enforces strict epoch ordering so miners cannot
skip ahead.

## 6. Reward Math

Let `energy_spent` denote the amount of \$ENERGY burned when running a batch.
The base reward is calculated as:

```text
core_reward   = energy_spent * CORE_PER_ENERGY
energy_reward = energy_spent
```

Where `CORE_PER_ENERGY` is a governance controlled constant. A fraction
`VALIDATOR_SPLIT` of `core_reward` is distributed among attesting validators in
proportion to their stake. The remainder goes to the miner who produced the
batch:

```text
miner_core        = core_reward * (1 - VALIDATOR_SPLIT)
validator_core_i  = core_reward * VALIDATOR_SPLIT * (stake_i / total_stake)
```

The \$ENERGY reward refunds the energy spent so training remains cost neutral.

## 7. Further Reading

* [`docs/LocalDevnet.md`](LocalDevnet.md) – run a local PoUW node.
* [`docs/ProofWorkflow.md`](ProofWorkflow.md) – generate proofs for batches using the integrated STARK-based workflow.
