import initNeuropet from '../build-wasm/neuropet.js';
import { connect, sendTransaction } from './wallet';

// Placeholder on-chain contract addresses
const CREATURE_NFT_ADDR = (window as any).CREATURE_NFT_ADDR || '0x0000000000000000000000000000000000000000';
const TRAINING_LEDGER_ADDR = (window as any).TRAINING_LEDGER_ADDR || '0x0000000000000000000000000000000000000000';

// Precomputed function selectors (keccak256 hashes)
const HATCH_SELECTOR = '0xd106adbb'; // hatch(bytes)
const SUBMIT_SELECTOR = '0x5df4059d'; // submitCheckpoint(uint256,uint256,bytes32,bool,uint32,bytes32)

export class Trainer {
  private modulePromise = initNeuropet();
  private wasm: any = null;
  private walletConnected = false;
  private socket: WebSocket | null = null;
  private hatchedId: number | null = null;

  async ensureWallet(): Promise<void> {
    if (!this.walletConnected) {
      await connect();
      this.walletConnected = true;
    }
  }

  private async initModule(): Promise<void> {
    if (!this.wasm) {
      this.wasm = await this.modulePromise;
    }
  }

  async hatch(weights: Uint8Array = new Uint8Array()): Promise<void> {
    await this.ensureWallet();
    const data =
      HATCH_SELECTOR + Buffer.from(weights).toString('hex');
    await sendTransaction(CREATURE_NFT_ADDR, '0x' + data);
  }

  private async submitCheckpoint(root: string): Promise<void> {
    const data = SUBMIT_SELECTOR + root.replace(/^0x/, '');
    await sendTransaction(TRAINING_LEDGER_ADDR, '0x' + data);
  }

  private ensureSocket(): void {
    if (this.socket) return;
    try {
      this.socket = new WebSocket('ws://localhost:8765/metrics');
    } catch (err) {
      console.warn('Failed to connect metrics socket:', err);
      this.socket = null;
    }
  }

  async train(steps = 1, cb?: (metrics: { step: number; loss: number }) => void): Promise<void> {
    await this.ensureWallet();
    await this.initModule();
    this.ensureSocket();
    const A = new Int8Array([1, 2, 3, 4]);
    const B = new Int8Array([5, 6, 7, 8]);
    const C = new Int8Array(4);
    for (let i = 0; i < steps; ++i) {
      (this.wasm as any).int8_matmul(A, B, C, 2, 2, 2);
      const metric = { step: i + 1, loss: C[0] };
      if (this.socket && this.socket.readyState === WebSocket.OPEN) {
        this.socket.send(JSON.stringify(metric));
      }
      if (cb) cb(metric);
      const root = '0x' + C[0].toString(16).padStart(2, '0');
      await this.submitCheckpoint(root);
    }
  }
}
