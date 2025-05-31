# Governance Contract Guide

This guide covers the minimal DAO used to manage on-chain parameters. The `Governance` contract relies on staked \$CORE for voting power and queues successful proposals behind a seven‑day timelock.

## 1. Staking

1. Lock \$CORE to gain voting weight:
   ```bash
   govcli stake --amount <AMOUNT> --rpc <RPC_URL> --key <KEY>
   ```
2. Withdraw at any time:
   ```bash
   govcli withdraw --amount <AMOUNT> --rpc <RPC_URL> --key <KEY>
   ```

## 2. Creating Proposals

1. Encode the function call on a governed contract.
2. Submit it with `propose`:
   ```bash
   govcli propose <TARGET> <DATA> --rpc <RPC_URL> --key <KEY>
   ```
   Proposals remain open for voting for **3 days**.

## 3. Voting

Vote for or against while the proposal is active:
```bash
govcli vote <ID> <true|false> --rpc <RPC_URL> --key <KEY>
```
Voting power equals the \$CORE you currently have staked.

## 4. Execution

After the voting period ends, successful proposals must wait an additional **7 days** before execution:
```bash
govcli execute <ID> --rpc <RPC_URL> --key <KEY>
```
The call is made on the target contract and reverts if it fails.

---

The governance contract controls modules like `SeasonRegistry` and `Marketplace`. Stakeholders can propose updates and, after passing the vote and timelock, execute them on-chain.

## 5. Updating Parameters

Many game constants such as energy curves and marketplace fees live in
`SeasonRegistry` and `Marketplace`. To change a parameter you must
submit the function call through governance.

1. Encode the call data for the target contract. For example, updating a
   season table might look like:
   ```bash
   cast calldata "setEnergyCurve(uint256[])" curve.json > data.bin
   ```
2. Propose the update using the encoded bytes:
   ```bash
   govcli propose <TARGET_ADDR> $(cat data.bin) --rpc <RPC_URL> --key <KEY>
   ```
3. Once the vote succeeds and the 7‑day timelock expires, execute it:
   ```bash
   govcli execute <ID> --rpc <RPC_URL> --key <KEY>
   ```
   The governed contract will update its state and emit an event if
   successful.

Regular parameter reviews keep the game balanced while preserving full
on-chain transparency.

## 6. Governance Hooks

`SeasonRegistry` and `Marketplace` call back into the `Governance` contract
whenever a parameter update succeeds. These callbacks record the target address
and function selector so off‑chain services can easily audit on‑chain changes.
Deployments should set the governance hook on both contracts to the DAO address:

```bash
cast send $MARKETPLACE "setGovernanceHook(address)" $GOVERNANCE
cast send $SEASONS "setGovernanceHook(address)" $GOVERNANCE
```

The last action can be read using `lastTarget()`, `lastSelector()` and
`lastData()` on the governance contract.
