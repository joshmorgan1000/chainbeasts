export interface MiningReward {
  block: number;
  amount: bigint;
}

/**
 * Simple helper to poll the TrainingLedger reward events via JSON-RPC.
 */
export class PoUWMonitor {
  private rpcUrl: string;
  private ledgerAddr: string;
  private timer: any = null;
  private lastBlock = 0n;

  constructor(
    rpcUrl: string = 'http://localhost:8545',
    ledgerAddr: string = (window as any).TRAINING_LEDGER_ADDR ||
      '0x0000000000000000000000000000000000000000',
  ) {
    this.rpcUrl = rpcUrl;
    this.ledgerAddr = ledgerAddr;
  }

  private async rpc(method: string, params: any[]): Promise<any> {
    const res = await fetch(this.rpcUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jsonrpc: '2.0', id: 1, method, params }),
    });
    const json = await res.json();
    if (json.error) throw new Error(json.error.message);
    return json.result;
  }

  start(miner: string, cb: (r: MiningReward) => void, interval = 5000): void {
    this.stop();
    this.timer = setInterval(async () => {
      try {
        const logs = await this.rpc('eth_getLogs', [
          {
            address: this.ledgerAddr,
            fromBlock: '0x' + this.lastBlock.toString(16),
            toBlock: 'latest',
          },
        ]);
        for (const log of logs) {
          const topics: string[] = log.topics;
          if (topics.length === 2 &&
              topics[1].toLowerCase().endsWith(miner.slice(2).toLowerCase())) {
            const amount = BigInt(log.data);
            const block = Number(BigInt(log.blockNumber));
            cb({ block, amount });
          }
          this.lastBlock = BigInt(log.blockNumber) + 1n;
        }
      } catch (err) {
        console.warn('reward poll failed:', err);
      }
    }, interval);
  }

  stop(): void {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
  }
}
