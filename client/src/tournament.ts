export interface DuelResult {
  id: number;
  challenger: string;
  opponent: string;
  winner: 'challenger' | 'opponent';
}

const FASHION_DUEL_ADDR =
  (window as any).FASHION_DUEL_ADDR || '0x0000000000000000000000000000000000000000';

const NEXT_ID_SELECTOR = '0x61b8ce8c'; // nextId()
const DUELS_SELECTOR = '0x859a62d0'; // duels(uint256)
const WEIGHT_CH_SELECTOR = '0x42ade315'; // weightChallenger(uint256)
const WEIGHT_OP_SELECTOR = '0xeb4fc291'; // weightOpponent(uint256)

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

export class Tournament {
  async getResults(): Promise<DuelResult[]> {
    const results: DuelResult[] = [];
    try {
      const nextIdHex = await ethCall(FASHION_DUEL_ADDR, NEXT_ID_SELECTOR);
      const nextId = Number(BigInt(nextIdHex));
      for (let id = 1; id < nextId; ++id) {
        const duelData = await ethCall(
          FASHION_DUEL_ADDR,
          '0x' + DUELS_SELECTOR.slice(2) + encUint(id),
        );
        const challenger = '0x' + duelData.slice(26, 66);
        const opponent = '0x' + duelData.slice(90, 130);
        const resolved = BigInt('0x' + duelData.slice(322, 386)) !== 0n;
        if (!resolved) continue;
        const wcHex = await ethCall(
          FASHION_DUEL_ADDR,
          '0x' + WEIGHT_CH_SELECTOR.slice(2) + encUint(id),
        );
        const woHex = await ethCall(
          FASHION_DUEL_ADDR,
          '0x' + WEIGHT_OP_SELECTOR.slice(2) + encUint(id),
        );
        const wc = BigInt(wcHex);
        const wo = BigInt(woHex);
        const winner = wc >= wo ? 'challenger' : 'opponent';
        results.push({ id, challenger, opponent, winner });
      }
    } catch (err) {
      console.warn('getResults failed:', err);
    }
    return results;
  }
}
