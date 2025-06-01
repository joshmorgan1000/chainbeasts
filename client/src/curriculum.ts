import { sendTransaction } from './wallet';

const CURRICULUM_ADDR = (window as any).CURRICULUM_ADDR ||
  '0x0000000000000000000000000000000000000000';

const CHALLENGE_SELECTOR = 'd59b8f5f';
const REVEAL_SELECTOR = '7eddbbdd';
const FORFEIT_SELECTOR = '334f9ad5';

function encUint(v: number | bigint): string {
  return BigInt(v).toString(16).padStart(64, '0');
}

function encAddr(addr: string): string {
  return addr.toLowerCase().replace(/^0x/, '').padStart(64, '0');
}

function encBytes(data: Uint8Array): string {
  const len = encUint(data.length);
  const hex = Array.from(data)
    .map(b => b.toString(16).padStart(2, '0'))
    .join('');
  const padded = hex.padEnd(Math.ceil(data.length / 32) * 64, '0');
  return len + padded;
}

function encHexBytes(hex: string): string {
  const clean = hex.replace(/^0x/, '');
  const bytes = new Uint8Array(clean.length / 2);
  for (let i = 0; i < bytes.length; ++i) {
    bytes[i] = parseInt(clean.substr(i * 2, 2), 16);
  }
  return encBytes(bytes);
}

export class CurriculumDuelClient {
  async challenge(
    opponent: string,
    battleBlock: number,
    commit: string,
    secret: string,
    v: number,
    r: string,
    s: string,
  ): Promise<void> {
    const offset = encUint(224); // 7 * 32 bytes static params
    const data =
      CHALLENGE_SELECTOR +
      encAddr(opponent) +
      encUint(battleBlock) +
      commit.replace(/^0x/, '').padStart(64, '0') +
      secret.replace(/^0x/, '').padStart(64, '0') +
      encUint(v) +
      r.replace(/^0x/, '').padStart(64, '0') +
      s.replace(/^0x/, '').padStart(64, '0') +
      encHexBytes('0x'); // empty bytes placeholder
    await sendTransaction(CURRICULUM_ADDR, '0x' + data);
  }

  async reveal(duelId: number, dataset: Uint8Array, secret: string): Promise<void> {
    const offset = encUint(96); // 3 static params
    const data =
      REVEAL_SELECTOR +
      encUint(duelId) +
      offset +
      secret.replace(/^0x/, '').padStart(64, '0') +
      encBytes(dataset);
    await sendTransaction(CURRICULUM_ADDR, '0x' + data);
  }

  async claimForfeit(duelId: number): Promise<void> {
    const data = FORFEIT_SELECTOR + encUint(duelId);
    await sendTransaction(CURRICULUM_ADDR, '0x' + data);
  }
}
