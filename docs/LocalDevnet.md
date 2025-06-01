# Local Devnet and Validator

This guide walks through running the local development network and starting a validator node. It assumes you have the toolchain described in the README installed.

## Prerequisites

* **Docker** and **posix make** – orchestrate the PoUW node and helper services.
* **Foundry** or **Hardhat** – deploy the contracts.
* **CMake** and **Clang ≥16** – build the native validator.
* **Node ≥20** – run the React client.

## Starting the network

1. Clone the repository and build the utilities:

   ```bash
   git clone https://github.com/your-org/neuropet.git
   cd neuropet
   cmake -S . -B build && cmake --build build -j$(nproc)
   ```

2. Launch the devnet services:

   ```bash
   make devnet
   ```

   This spins up a local PoUW node. Blocks advance using mock time and the
   JSON-RPC endpoint is available at `http://localhost:8545`. The script also
   launches WebSocket servers on ports `8765` and `8766` used by the React
   client for training metrics and validator proofs.

## Running the WebSocket servers separately

The devnet helper automatically starts two Node.js processes that forward
training metrics and validator proofs via WebSocket. If you need to run these
servers manually (for example when developing the client independently) use:

```bash
node scripts/metrics_server.js     # default ws://localhost:8765/metrics
node scripts/validator_server.js   # default ws://localhost:8766/validator
```

Set the `METRICS_PORT` or `VALIDATOR_PORT` environment variables to override the
default ports.

## Analytics Dashboard

Start the React client with `npm start` inside the `client` folder and open
`http://localhost:3000`. The **Analytics Dashboard** tab displays the real-time
loss chart and recent tournament results streamed from `metrics_server.js` and
`battle_server.js`. Ensure the devnet or the standalone servers are running so
the dashboard receives updates.

3. Deploy the contracts and seed faucet keys:

   ```bash
   make deploy
   ```

## Mining and validation

With the devnet running you can start a validator to attest checkpoints:

```bash
./build/validator --follow mempool
```

Running multiple instances simulates the network of miners securing the
chain. The validator listens to the devnet RPC and writes attestations
once a checkpoint reaches quorum.

## Shutting down

Press `Ctrl+C` in the terminal running `make devnet` to stop the
services. Temporary chain data lives in `./devnet/` and can be removed
with:

```bash
rm -rf devnet
```

---

© 2025 ChainBeasts Labs – Draft
