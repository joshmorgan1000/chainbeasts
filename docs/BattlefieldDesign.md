# Deterministic Battlefield Design

This document outlines a simple deterministic arena for Chain Beasts. Creatures fight on a fixed grid so every turn is reproducible on-chain and in the client.

## 1. Map Layout

* **Board size** – 8×8 tiles. Each tile is addressed by `(x, y)` with `0 ≤ x,y < 8`.
* **Tile types** – `EMPTY`, `WALL`, `HAZARD`. Tiles are encoded as bytes and hashed along with the battle seed.
* **Isometric view** – the UI may render the grid in an isometric style, but the underlying coordinates remain a simple square grid.

## 2. Actions

Each turn a creature chooses one action:

1. **Move** – step to an adjacent tile (`N`, `S`, `E`, `W`) if not blocked.
2. **Attack** – strike the opposing creature if within one tile, using the existing power/defense rules.
3. **Block** – sacrifice the attack to gain one temporary defense point for the next enemy hit.

Sensors feed the current tile type and relative opponent position into the creature model so decision making remains deterministic.

## 3. Turn Order

* The starting turn is derived from `seed = creatureA_id XOR creatureB_id`.
* Turns alternate after each action. Randomness is limited to the starting seed so fights replay exactly across nodes.

## 4. Rewards

* **Victory reward** – winner receives `+5 $ENERGY` and gains one ranking point in `MatchLedger`.
* **Participation reward** – loser receives `+1 $ENERGY` to encourage frequent battles.
* Rewards are minted from the on-chain reward pool and emitted with the match event so off-chain clients can verify them.

## 5. Determinism Notes

The entire board state, creature stats and initial seed are hashed before the match begins. Validators replay the same sequence using the battle engine. Because movement and attacks use discrete coordinates and integer math, every node arrives at identical results.
