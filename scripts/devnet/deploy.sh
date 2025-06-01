#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(realpath "$SCRIPT_DIR/../..")"
RPC_URL=${RPC_URL:-http://localhost:8545}
PRIVATE_KEY=${PRIVATE_KEY:-0x59c6995e998f97a5a0044966f094538e313704ff6d6d39f42ebd8d0a7ebfb4c9}

cd "$ROOT_DIR"

if ! command -v forge >/dev/null 2>&1; then
    echo "[deploy] forge not found. Please install Foundry" >&2
    exit 1
fi

forge build

echo "[deploy] deploying Core"
CORE=$(forge create contracts/Core.sol:Core \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying Governance"
GOV=$(forge create contracts/Governance.sol:Governance \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" \
    --constructor-args "$CORE" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying CreatureNFT"
CREATURE_NFT=$(forge create contracts/CreatureNFT.sol:CreatureNFT \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying Marketplace"
MARKETPLACE=$(forge create contracts/Marketplace.sol:Marketplace \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" \
    --constructor-args "$CREATURE_NFT" "$GOV" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying Energy"
ENERGY=$(forge create contracts/Energy.sol:Energy \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying ZkVerifier"
ZK=$(forge create contracts/ZkVerifier.sol:ZkVerifier \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying ProofVerifier"
PROOF=$(forge create contracts/ProofVerifier.sol:ProofVerifier \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" \
    --constructor-args "$ZK" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying TrainingLedger"
TRAINING_LEDGER=$(forge create contracts/TrainingLedger.sol:TrainingLedger \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" \
    --constructor-args "$ENERGY" "$CORE" "$PROOF" "$SEASONS" | awk '/Deployed to:/ {print $3}')
echo "[deploy] deploying MatchLedger"
MATCH_LEDGER=$(forge create contracts/MatchLedger.sol:MatchLedger \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying Matchmaker"
MATCHMAKER=$(forge create contracts/Matchmaker.sol:Matchmaker \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" \
    --constructor-args "$MATCH_LEDGER" | awk '/Deployed to:/ {print $3}')

echo "[deploy] deploying SeasonRegistry"
SEASONS=$(forge create contracts/SeasonRegistry.sol:SeasonRegistry \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" \
    --constructor-args "$GOV" | awk '/Deployed to:/ {print $3}')
forge create contracts/ProofVerifier.sol:ProofVerifier \
    --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" >/dev/null

if command -v cast >/dev/null 2>&1; then
    cast send "$CREATURE_NFT" "setMarketplace(address)" "$MARKETPLACE" \
        --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" >/dev/null
    cast send "$CREATURE_NFT" "setSeasonRegistry(address)" "$SEASONS" \
        --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" >/dev/null
    cast send "$MARKETPLACE" "setGovernanceHook(address)" "$GOV" \
        --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" >/dev/null
    cast send "$SEASONS" "setGovernanceHook(address)" "$GOV" \
        --rpc-url "$RPC_URL" --private-key "$PRIVATE_KEY" >/dev/null
fi

echo "[deploy] done"
