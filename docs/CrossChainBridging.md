# Cross-Chain Bridging Guide

This guide explains how to move **CreatureNFT** tokens between blockchains. It assumes the contracts from `ContractDeployment.md` are deployed on both the origin and destination chains.

## Overview

Bridging works by locking the token on the origin chain and minting a mirror token on the destination chain.  With proof‑based transfers the `bridgeOut` event now includes a `rootHash` that uniquely represents the creature's genesis weights and DNA.  Off‑chain validators generate a STARK proof for this hash which is checked by the destination chain before a mirror token can be minted.  The creature's stats and training history remain intact because they are stored in the token metadata and replicated across chains.

## Steps

1. **Approve the bridge** – The owner approves the `CreatureNFTBridge` contract to transfer their token.
2. **Lock on origin** – Call `bridgeOut(creatureId, dstChainId)` which transfers the token to the bridge and emits a `BridgeOut` event containing the creature's genesis weights, DNA and `rootHash`.
3. **Generate proof** – A relayer or validator uses `bridge_cli` to fetch the event data and produce a STARK proof of the `rootHash`.  The proof is submitted to the `ProofVerifier` contract on the destination chain.
4. **Mint on destination** – Once the proof is finalized, call `bridgeIn(tokenId, srcChainId, dstChainId, weights, dna, proof)` which verifies the proof and mints or releases the token.
5. **Return trip** – To move back, the same flow is used with the source and destination chains swapped.

## Considerations

* Only one instance of a creature can exist at any time. The bridge enforces this by keeping the original token locked while the mirror exists.
* Training and battling are paused while a creature is bridged.
* Gas fees apply on both chains for locking, relaying and minting operations.

## Client UI

The React client exposes a **Bridging** view under the navigation menu.
Connect your wallet and select the destination chain to initiate a transfer.
The UI performs the approval and `bridgeOut` call, then tracks the relay until
the creature appears on the target network. Returning follows the same flow via
the **Bridge In** button.

