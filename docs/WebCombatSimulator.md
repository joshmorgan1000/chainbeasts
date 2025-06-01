# Web Combat Simulator

This design outlines a minimal, deterministic combat simulator that runs entirely inside a web browser. It mirrors the on-chain rules so players can test creatures and developers can fine-tune the AI without blockchain transactions.

## 1. Feature Overview

### 1.1 Hatching Random Beasts
* Provide a **Hatch** button that creates a creature from a random seed.
* Sensors and appendages are drawn from the official catalog so the simulator matches mainnet behaviour.
* The creature summary (power, defense, stamina) appears in the top overlay.

### 1.2 Training
* Users may run local training steps using the INT8 WebAssembly kernel.
* Each step consumes a small amount of in-game energy and updates the creature stats in real time.
* Training progress is displayed next to the base stats for quick iteration.

### 1.3 Battles
* Two trained beasts can fight in a deterministic arena identical to the core engine.
* The simulator replays the turn sequence locally and renders the actions on a canvas.
* Results (remaining hit points, stamina) are shown in the bottom overlay at the end of the match.

### 1.4 Stats Overlay
* The top overlay lists each creature's current power, defense and stamina.
* The bottom overlay displays temporary status effects, training gains and battle logs.
* Overlays remain visible during all interactions so designers can tweak balance constants and instantly observe the outcome.

## 2. Technical Architecture

1. **WebAssembly Kernel** – built via the existing `build_wasm.sh` script. The module exposes the same INT8 math helpers and battle functions used in the native build.
2. **UI Layer** – a lightweight HTML page backed by a single canvas element for creature sprites and attack effects.
3. **State Management** – a JavaScript module maintains the creature objects, random seeds and current battle state. All randomness uses deterministic seeds so replays stay consistent.
4. **Event Loop** – user actions (hatch, train, battle) trigger kernel calls and update the overlays. The main loop only processes one action at a time to keep behaviour reproducible.

## 3. User Interface Layout

```
+--------------------------------------------------------------+
| Top Overlay: creature stats and training progress            |
+--------------------------------------------------------------+
|                                                              |
|                        Battle Canvas                         |
|                                                              |
+--------------------------------------------------------------+
| Bottom Overlay: battle log and temporary effects             |
+--------------------------------------------------------------+
```

* The overlays use fixed positioning so they remain visible on small screens.
* Sprite art and animations can be loaded from the existing assets used by the native client.

## 4. Local Development

1. Build the WebAssembly module as described in [`docs/WebAssembly.md`](WebAssembly.md).
2. Serve the `index.html`, `neuropet.js` and `neuropet.wasm` files via a local HTTP server.
3. Open the page in a modern browser with WebAssembly support and test creature actions directly.

This simulator focuses on reproducibility and fast iteration. By keeping the stats on screen and exposing the same kernel functions as the on-chain implementation, designers can quickly balance the game and verify AI behaviour before deploying new rules.

## 5. Implementation Details

The browser side is intentionally minimal. `neuropet.js` exposes a `Simulator`
class that wraps the WebAssembly kernel compiled via `build_wasm.sh`.  The
module provides deterministic functions for hatching, training steps and
battles.  JavaScript maintains creature state in typed arrays and forwards user
actions to the kernel one at a time so replays remain consistent.

Random seeds are generated with a small PRNG mirroring the on-chain logic.
All draws and battle outcomes therefore match the native engine when using the
same seed.  Frame updates and overlay text are driven by
`requestAnimationFrame` and no external frameworks are required.

The simulator performs all work in memory and leaves persistence to the
embedding page.  Assets such as sprites and sound effects may be loaded
externally but do not affect determinism or the core loop.
