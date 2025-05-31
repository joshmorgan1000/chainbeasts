# Matchmaker Contract Guide

This document describes the on-chain matchmaker contract and how it integrates with the rest of the contract suite. The matchmaker enforces the deterministic battle rules and records results on `MatchLedger` so rewards and rankings remain verifiable.

## 1. Overview

`Matchmaker` is a Solidity contract that queues battle requests and pairs them in a first-in, first-out fashion. Once two creatures are available the contract runs a deterministic fight using the same rules as the off-chain engine. Outcomes are emitted as events and pushed into `MatchLedger`.

The contract is designed to be minimal so verification is straightforward. All math is 32‑bit and the battle loop mirrors the reference implementation in [`include/neuropet/battle.hpp`](../include/neuropet/battle.hpp).

## 2. Interface

```solidity
// enqueue creature stats; runs a battle when two are waiting
function enqueue(
    uint32 creatureId,
    int32 power,
    int32 defense,
    int32 stamina
) external;

// number of creatures waiting in the queue
function pending() external view returns (uint256);

// emitted whenever a creature joins the queue
event Enqueued(uint256 indexed creatureId, address indexed owner);

// emitted after a match is resolved
event Matched(
    uint256 indexed battleId,
    uint256 indexed creatureA,
    uint256 indexed creatureB,
    uint256 winner
);
```

The constructor accepts the `MatchLedger` address. Results are recorded there using `record(creatureA, creatureB, winner, battleHash)`.

## 3. Battle Flow

1. Players call `enqueue` with their creature stats. The contract stores the request and emits `Enqueued`.
2. When two or more requests are waiting, `_runMatch()` pops the first two and executes `_fight()`.
3. The winner and loser IDs are written to `MatchLedger`. A `Matched` event signals the battle ID and winner.
4. External services watching `Matched` can reward participants or trigger additional game logic.

The random seed is derived from the XOR of both creature IDs to ensure determinism. Future versions may pass a `season_hash` from `SeasonRegistry` to incorporate balance changes.

## 4. Off-Chain Integration

`OnchainArena` in the C++ client sends battle requests via JSON-RPC to a node connected to the matchmaker. The arena mirrors local fights so results remain consistent with on-chain state. Validators can replay battles by reading `MatchLedger` events.


---

## 5. Deployment

Deploy `MatchLedger` first and pass its address to the matchmaker constructor.

### Example

```bash
chain-cli deploy contracts/Matchmaker.sol --rpc <RPC_URL> --key <KEY> \
    --constructor-args <MATCH_LEDGER_ADDR>
```

## 6. Basic Usage

Queue a creature for battle using `enqueue`:

```bash
chain-cli call <MATCHMAKER_ADDR> enqueue 17 10 8 6 --rpc <RPC_URL> --key <KEY>
```

Call `pending()` to see how many creatures are waiting. Monitor `Matched` events
to determine the winner and retrieve the battle ID.

---

The matchmaker contract is implemented and ready for use. The interface above is considered stable.

