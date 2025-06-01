import { useState } from 'react';
import { BracketManager } from './bracket';

export default function TournamentManageView() {
  const [tm] = useState(() => new BracketManager());
  const [players, setPlayers] = useState('');
  const [prize, setPrize] = useState('');
  const [bracketId, setBracketId] = useState('');
  const [winners, setWinners] = useState('');
  const [log, setLog] = useState<string[]>([]);

  const append = (m: string) => setLog(l => [...l, m]);

  const create = async () => {
    try {
      const addrs = players.split(',').map(a => a.trim()).filter(a => a);
      await tm.createBracket(addrs, BigInt(prize || '0'));
      append('created bracket');
    } catch (e: any) {
      append('error: ' + e.message);
    }
  };

  const report = async () => {
    try {
      const ws = winners.split(',').map(a => a.trim()).filter(a => a);
      await tm.reportWinners(Number(bracketId), ws);
      append('reported winners');
    } catch (e: any) {
      append('error: ' + e.message);
    }
  };

  return (
    <div>
      <h2>Tournament Management</h2>
      <div>
        <input
          placeholder="Players comma separated"
          value={players}
          onChange={e => setPlayers(e.target.value)}
        />
        <input
          placeholder="Prize (wei)"
          value={prize}
          onChange={e => setPrize(e.target.value)}
        />
        <button onClick={create}>Create Bracket</button>
      </div>
      <div>
        <input
          placeholder="Bracket ID"
          value={bracketId}
          onChange={e => setBracketId(e.target.value)}
        />
        <input
          placeholder="Winners comma separated"
          value={winners}
          onChange={e => setWinners(e.target.value)}
        />
        <button onClick={report}>Report Winners</button>
      </div>
      <pre>{log.join('\n')}</pre>
    </div>
  );
}
