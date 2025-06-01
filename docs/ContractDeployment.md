# Contract Deployment Guide

This guide explains how to deploy the Chain Beasts smart contracts to a local development network or any EVM compatible chain.

## Overview

The contract suite is composed of multiple modules:

* **Core** – staking and reward token.
* **Governance** – DAO and timelock controller.
* **CreatureNFT** – ERC‑721 creatures with training history.
* **Marketplace** – sales and leasing of creatures.
* **TrainingLedger** – stores training checkpoints.
* **MatchLedger** – records battle outcomes.
* **Matchmaker** – queues and resolves battles.
* **SeasonRegistry** – tracks active seasons.
* **ProofVerifier** – validates STARK proofs for checkpoints.

All contracts are written in Solidity 0.8 and built with Foundry.

## Prerequisites

* Foundry installed (`forge` and `cast` commands).
* An RPC endpoint for the target network.
* A private key funded with native tokens for deployment.

## Automated Deployment

When running the local devnet simply execute:

```bash
make deploy
```

This invokes `scripts/devnet/deploy.sh` which compiles the contracts and deploys them in order.  The script prints the addresses of each contract once finished.  Override the defaults using environment variables:

```bash
RPC_URL=http://localhost:8545 \
PRIVATE_KEY=<HEX_KEY> \
make deploy
```

## Manual Deployment

To deploy manually build the contracts with `forge` and create each one using `forge create`:

```bash
forge build
forge create contracts/Core.sol:Core --rpc-url <RPC_URL> --private-key <KEY>
```

Repeat the `forge create` command for every contract, passing the constructor arguments shown in `scripts/devnet/deploy.sh`. After deployment wire the addresses together with `cast send` (for example set the marketplace on `CreatureNFT`).

---

© 2025 ChainBeasts Labs
