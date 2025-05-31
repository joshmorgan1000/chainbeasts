# Marketplace Guide

This document explains how to trade creatures, retrieve sale listings and use the leasing system provided by the `Marketplace` contract.

## 1. Listing Creatures for Sale

1. Approve the contract to transfer your token:
   ```bash
   cast send <NFT_ADDR> "approve(address,uint256)" <MARKET_ADDR> <TOKEN_ID> --rpc-url <RPC_URL> --private-key <KEY>
   ```
2. Create the listing by calling `list` with the asking price in wei:
   ```bash
   cast send <MARKET_ADDR> "list(uint256,uint256)" <TOKEN_ID> <PRICE> --rpc-url <RPC_URL> --private-key <KEY>
   ```
3. Update the price using `updatePrice` or cancel with `cancel` if needed.

## 2. Retrieving Listings

All active listings are stored in the public `listings` mapping. Query the tuple `(seller, price)` by token ID:

```bash
cast call <MARKET_ADDR> "listings(uint256)(address,uint256)" <TOKEN_ID> --rpc-url <RPC_URL>
```

If the seller address is zero the creature is not currently listed.

## 3. Leasing Creatures

The marketplace also supports temporary rentals.

1. List a creature for lease specifying the daily price and duration in seconds:
   ```bash
   cast send <MARKET_ADDR> "listForLease(uint256,uint256,uint256)" <TOKEN_ID> <PRICE> <DURATION> --rpc-url <RPC_URL> --private-key <KEY>
   ```
2. Renters call `rent` with the quoted price to obtain access until the expiry timestamp:
   ```bash
   cast send <MARKET_ADDR> "rent(uint256)" <TOKEN_ID> --value <PRICE> --rpc-url <RPC_URL> --private-key <KEY>
   ```
3. Retrieve lease information at any time:
   ```bash
   cast call <MARKET_ADDR> "getLease(uint256)((address,uint256,uint256,address,uint256))" <TOKEN_ID> --rpc-url <RPC_URL>
   ```
   The returned struct includes the owner, price, duration, current renter and expiry.
4. Owners may cancel using `cancelLease` when no active rental exists.

---

With these commands you can manage sales and rentals entirely on-chain. See `contracts/Marketplace.sol` for full interface details.
