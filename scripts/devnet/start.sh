#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(realpath "$SCRIPT_DIR/../..")"
DEVNET_DIR="$ROOT_DIR/devnet"
mkdir -p "$DEVNET_DIR"

cleanup() {
    if [[ -n "${ANVIL_PID:-}" ]]; then
        kill "$ANVIL_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${METRICS_PID:-}" ]]; then
        kill "$METRICS_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${BATTLE_PID:-}" ]]; then
        kill "$BATTLE_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${VALIDATOR_PID:-}" ]]; then
        kill "$VALIDATOR_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${PLUGIN_PID:-}" ]]; then
        kill "$PLUGIN_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${MARKET_PID:-}" ]]; then
        kill "$MARKET_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${AGG_PID:-}" ]]; then
        kill "$AGG_PID" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

echo "[devnet] starting Anvil on http://localhost:8545"
if command -v anvil >/dev/null 2>&1; then
    anvil --host 0.0.0.0 --port 8545 --block-time 1 \
        --data-dir "$DEVNET_DIR/anvil" > "$DEVNET_DIR/anvil.log" 2>&1 &
    ANVIL_PID=$!
else
    docker run --rm -p 8545:8545 -v "$DEVNET_DIR/anvil:/data" \
        ghcr.io/foundry-rs/foundry:latest anvil --host 0.0.0.0 --block-time 1 \
        > "$DEVNET_DIR/anvil.log" 2>&1 &
    ANVIL_PID=$!
fi

if command -v node >/dev/null 2>&1; then
    echo "[devnet] starting metrics, battle, validator, marketplace and plugin marketplace servers"
    node "$ROOT_DIR/scripts/metrics_server.js" > "$DEVNET_DIR/metrics.log" 2>&1 &
    METRICS_PID=$!
    node "$ROOT_DIR/scripts/battle_server.js" > "$DEVNET_DIR/battles.log" 2>&1 &
    BATTLE_PID=$!
    node "$ROOT_DIR/scripts/validator_server.js" > "$DEVNET_DIR/validator.log" 2>&1 &
    VALIDATOR_PID=$!
    node "$ROOT_DIR/scripts/marketplace_server.js" > "$DEVNET_DIR/marketplace.log" 2>&1 &
    MARKET_PID=$!
    node "$ROOT_DIR/scripts/plugin_marketplace_server.js" > "$DEVNET_DIR/plugin_marketplace.log" 2>&1 &
    PLUGIN_PID=$!
else
    echo "[devnet] warning: node not found, client sockets disabled" >&2
fi

if [[ -x "$ROOT_DIR/build/aggregator_server" ]]; then
    echo "[devnet] starting proof aggregator on port 8781"
    "$ROOT_DIR/build/aggregator_server" --port 8781 \
        > "$DEVNET_DIR/aggregator.log" 2>&1 &
    AGG_PID=$!
else
    echo "[devnet] warning: build/aggregator_server not found" >&2
fi

echo "[devnet] Press Ctrl+C to stop"
wait "$ANVIL_PID"
