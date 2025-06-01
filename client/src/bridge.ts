import { sendTransaction } from './wallet';

const BRIDGE_ADDR = (window as any).BRIDGE_ADDR || '0x0000000000000000000000000000000000000000';
const NFT_ADDR = (window as any).CREATURE_NFT_ADDR || '0x0000000000000000000000000000000000000000';

const APPROVE_SELECTOR = '095ea7b3';
const BRIDGE_OUT_SELECTOR = '57c74f6e';
const BRIDGE_IN_SELECTOR = 'e21f3d77';

function encUint(v: number | bigint): string {
  return BigInt(v).toString(16).padStart(64, '0');
}

function encAddr(addr: string): string {
  return addr.toLowerCase().replace(/^0x/, '').padStart(64, '0');
}

function encBytes(data: Uint8Array): string {
  const hex = Array.from(data)
    .map(b => b.toString(16).padStart(2, '0'))
    .join('');
  const len = encUint(data.length);
  const padded = hex.padEnd(Math.ceil(data.length / 32) * 64, '0');
  return len + padded;
}

export class Bridge {
  private walletConnected = false;

  private async ensureWallet(): Promise<void> {
    if (!this.walletConnected) {
      const eth = (window as any).ethereum;
      if (!eth?.request) throw new Error('Wallet provider not found');
      await eth.request({ method: 'eth_requestAccounts' });
      this.walletConnected = true;
    }
  }

  async bridgeOut(tokenId: number, dstChainId: number): Promise<void> {
    await this.ensureWallet();
    const approveData =
      APPROVE_SELECTOR + encAddr(BRIDGE_ADDR) + encUint(tokenId);
    await sendTransaction(NFT_ADDR, '0x' + approveData);
    const data = BRIDGE_OUT_SELECTOR + encUint(tokenId) + encUint(dstChainId);
    await sendTransaction(BRIDGE_ADDR, '0x' + data);
  }

  async bridgeIn(
    tokenId: number,
    srcChainId: number,
    genesisWeights: Uint8Array,
    dna: string,
  ): Promise<void> {
    await this.ensureWallet();
    const eth = (window as any).ethereum;
    const [to] = await eth.request({ method: 'eth_requestAccounts' });
    const offset = encUint(160);
    const data =
      BRIDGE_IN_SELECTOR +
      encUint(tokenId) +
      encAddr(to) +
      encUint(srcChainId) +
      offset +
      dna.replace(/^0x/, '').padStart(64, '0') +
      encBytes(genesisWeights);
    await sendTransaction(BRIDGE_ADDR, '0x' + data);
  }
}
