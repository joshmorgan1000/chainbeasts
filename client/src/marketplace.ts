export interface Listing {
  id: number;
  price: number;
  seller: string;
}

export interface Lease {
  id: number;
  price: number;
  duration: number;
  renter: string | null;
  expiry: number;
}

/**
 * Simple wrapper around the on-chain Marketplace contract.
 * Provides listing, buying and leasing helpers using a wallet provider.
 */
import { connect, sendTransaction } from './wallet';

// Placeholder contract address
const MARKETPLACE_ADDR =
  (window as any).MARKETPLACE_ADDR || '0x0000000000000000000000000000000000000000';
const CREATURE_NFT_ADDR =
  (window as any).CREATURE_NFT_ADDR || '0x0000000000000000000000000000000000000000';

// Precomputed function selectors
const LIST_SELECTOR = '0x50fd7367'; // list(uint256,uint256)
const BUY_SELECTOR = '0xd96a094a'; // buy(uint256)
const BREED_SELECTOR = '0x3039f6e9'; // breed(uint256,uint256,bytes)
const LIST_LEASE_SELECTOR = '0xaf775986'; // listForLease(uint256,uint256,uint256)
const CANCEL_LEASE_SELECTOR = '0xd6616f75'; // cancelLease(uint256)
const RENT_SELECTOR = '0x7456be7d'; // rent(uint256)
const NFT_SELECTOR = '0x47ccca02'; // nft()
const NEXT_ID_SELECTOR = '0x61b8ce8c'; // nextId()
const LISTINGS_SELECTOR = '0xde74e57b'; // listings(uint256)
const GET_LEASE_SELECTOR = '0x9f44657c'; // getLease(uint256)

function encUint(v: number | bigint): string {
  const n = BigInt(v);
  return n.toString(16).padStart(64, '0');
}

async function ethCall(to: string, data: string): Promise<string> {
  const eth = (window as any).ethereum;
  if (!eth?.request) throw new Error('Wallet provider not found');
  return eth.request({
    method: 'eth_call',
    params: [{ to, data }, 'latest'],
  });
}

export class Marketplace {
  private walletConnected = false;

  private async ensureWallet(): Promise<void> {
    if (!this.walletConnected) {
      await connect();
      this.walletConnected = true;
    }
  }

  async list(tokenId: number, price: number): Promise<void> {
    await this.ensureWallet();
    const data =
      LIST_SELECTOR + encUint(tokenId) + encUint(price);
    await sendTransaction(MARKETPLACE_ADDR, '0x' + data);
  }

  async buy(tokenId: number, price: number): Promise<void> {
    await this.ensureWallet();
    const data = BUY_SELECTOR + encUint(tokenId);
    await sendTransaction(
      MARKETPLACE_ADDR,
      '0x' + data,
      '0x' + BigInt(price).toString(16),
    );
  }

  async breed(parentA: number, parentB: number, weights: Uint8Array): Promise<void> {
    await this.ensureWallet();
    const data =
      BREED_SELECTOR +
      encUint(parentA) +
      encUint(parentB) +
      Buffer.from(weights).toString('hex');
    await sendTransaction(MARKETPLACE_ADDR, '0x' + data);
  }

  async getListings(): Promise<Listing[]> {
    const listings: Listing[] = [];
    try {
      // fetch total token count from NFT contract
      const nextIdHex = await ethCall(CREATURE_NFT_ADDR, NEXT_ID_SELECTOR);
      const nextId = Number(BigInt(nextIdHex));
      for (let id = 1; id < nextId; ++id) {
        const data = LISTINGS_SELECTOR + encUint(id);
        const res = await ethCall(MARKETPLACE_ADDR, '0x' + data);
        const seller = '0x' + res.slice(26, 66);
        const price = Number(BigInt('0x' + res.slice(66, 130)));
        if (seller !== '0x0000000000000000000000000000000000000000' && price > 0) {
          listings.push({ id, price, seller });
        }
      }
    } catch (err) {
      console.warn('getListings failed:', err);
    }
    return listings;
  }

  async listForLease(tokenId: number, price: number, duration: number): Promise<void> {
    await this.ensureWallet();
    const data =
      LIST_LEASE_SELECTOR +
      encUint(tokenId) +
      encUint(price) +
      encUint(duration);
    await sendTransaction(MARKETPLACE_ADDR, '0x' + data);
  }

  async cancelLease(tokenId: number): Promise<void> {
    await this.ensureWallet();
    const data = CANCEL_LEASE_SELECTOR + encUint(tokenId);
    await sendTransaction(MARKETPLACE_ADDR, '0x' + data);
  }

  async rent(tokenId: number, price: number): Promise<void> {
    await this.ensureWallet();
    const data = RENT_SELECTOR + encUint(tokenId);
    await sendTransaction(
      MARKETPLACE_ADDR,
      '0x' + data,
      '0x' + BigInt(price).toString(16),
    );
  }

  async getLeases(): Promise<Lease[]> {
    const leases: Lease[] = [];
    try {
      const nextIdHex = await ethCall(CREATURE_NFT_ADDR, NEXT_ID_SELECTOR);
      const nextId = Number(BigInt(nextIdHex));
      for (let id = 1; id < nextId; ++id) {
        const data = GET_LEASE_SELECTOR + encUint(id);
        const res = await ethCall(MARKETPLACE_ADDR, '0x' + data);
        const owner = '0x' + res.slice(26, 66);
        if (owner === '0x0000000000000000000000000000000000000000') continue;
        const price = Number(BigInt('0x' + res.slice(66, 130)));
        const duration = Number(BigInt('0x' + res.slice(130, 194)));
        const renterAddr = '0x' + res.slice(194 + 24, 258);
        const expiry = Number(BigInt('0x' + res.slice(258, 322)));
        const renter = renterAddr === '0x0000000000000000000000000000000000000000' ? null : renterAddr;
        leases.push({ id, price, duration, renter, expiry });
      }
    } catch (err) {
      console.warn('getLeases failed:', err);
    }
    return leases;
  }
}
