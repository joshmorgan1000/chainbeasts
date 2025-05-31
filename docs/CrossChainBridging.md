# Cross-Chain Bridging Guide

This guide explains how to move **CreatureNFT** tokens between blockchains. It assumes the contracts from `ContractDeployment.md` are deployed on both the origin and destination chains.

## Overview

Bridging works by locking the token on the origin chain and minting a mirror token on the destination chain. The creature's stats and training history remain intact because they are stored in the token metadata and replicated across chains.

## Steps

1. **Approve the bridge** – The owner approves the `CreatureNFTBridge` contract to transfer their token.
2. **Lock on origin** – Call `bridgeOut(creatureId, dstChainId)` which transfers the token to the bridge and emits a proof event.
3. **Relay the event** – Off‑chain relayers monitor bridge events and submit the proof to the destination chain.
4. **Mint on destination** – The destination bridge verifies the proof and mints a new token representing the creature.
5. **Return trip** – To move back, call `bridgeIn` on the destination chain which burns the mirror token and unlocks the original.

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

