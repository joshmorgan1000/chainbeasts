import { useEffect, useState } from 'react';
import { Tournament, DuelResult } from './tournament';

export default function TournamentResults() {
  const [tournament] = useState(() => new Tournament());
  const [results, setResults] = useState<DuelResult[]>([]);

  const refresh = async () => {
    setResults(await tournament.getResults());
  };

  useEffect(() => {
    refresh();
    const id = setInterval(refresh, 10000);
    return () => clearInterval(id);
  }, []);

  return (
    <div>
      <h2>Tournament Results</h2>
      <button onClick={refresh}>Refresh</button>
      <ul>
        {results.map(r => (
          <li key={r.id}>
            Duel {r.id}: {r.challenger} vs {r.opponent} – winner {r.winner}
          </li>
        ))}
      </ul>
    </div>
  );
}
