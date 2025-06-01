import { sendTransaction } from './wallet';

const TOURNAMENT_ADDR = (window as any).TOURNAMENT_ADDR || '0x0000000000000000000000000000000000000000';

const CREATE_BRACKET_SELECTOR = '86e5c892';
const REPORT_WINNERS_SELECTOR = '773ee7d0';
const PLAYERS_SELECTOR = 'ff583f66';
const WINNER_SELECTOR = 'f56de84f';

function encUint(v: number | bigint): string {
  return BigInt(v).toString(16).padStart(64, '0');
}

function encAddr(addr: string): string {
  return addr.toLowerCase().replace(/^0x/, '').padStart(64, '0');
}

function encArrayAddrs(addrs: string[]): string {
  const len = encUint(addrs.length);
  const elems = addrs.map(a => encAddr(a)).join('');
  return len + elems.padEnd(Math.ceil(addrs.length / 32) * 64, '0');
}

export class BracketManager {
  private async wallet(): Promise<string> {
    const eth = (window as any).ethereum;
    if (!eth?.request) throw new Error('Wallet provider not found');
    const [addr] = await eth.request({ method: 'eth_requestAccounts' });
    return addr;
  }

  async createBracket(players: string[], prize: bigint): Promise<void> {
    await this.wallet();
    const head = CREATE_BRACKET_SELECTOR + encUint(32 * 2) + encUint(players.length);
    const data = head + players.map(a => encAddr(a)).join('');
    await sendTransaction(
      TOURNAMENT_ADDR,
      '0x' + data,
      '0x' + prize.toString(16),
    );
  }

  async reportWinners(id: number, winners: string[]): Promise<void> {
    await this.wallet();
    const head =
      REPORT_WINNERS_SELECTOR +
      encUint(id) +
      encUint(32 * 2) +
      encUint(winners.length);
    const data = head + winners.map(a => encAddr(a)).join('');
    await sendTransaction(TOURNAMENT_ADDR, '0x' + data);
  }

  async getPlayers(id: number): Promise<string[]> {
    const eth = (window as any).ethereum;
    if (!eth?.request) throw new Error('Wallet provider not found');
    const data = PLAYERS_SELECTOR + encUint(id);
    const res: string = await eth.request({
      method: 'eth_call',
      params: [{ to: TOURNAMENT_ADDR, data: '0x' + data }, 'latest'],
    });
    const len = Number(BigInt('0x' + res.slice(2, 66)));
    const players: string[] = [];
    for (let i = 0; i < len; ++i) {
      const start = 66 + i * 64 + 24;
      players.push('0x' + res.slice(start, start + 40));
    }
    return players;
  }

  async getWinner(id: number): Promise<string | null> {
    const eth = (window as any).ethereum;
    if (!eth?.request) throw new Error('Wallet provider not found');
    const data = WINNER_SELECTOR + encUint(id);
    const res: string = await eth.request({
      method: 'eth_call',
      params: [{ to: TOURNAMENT_ADDR, data: '0x' + data }, 'latest'],
    });
    const addr = '0x' + res.slice(26);
    if (addr === '0x0000000000000000000000000000000000000000') return null;
    return addr;
  }
}
