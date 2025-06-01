import { useEffect, useState } from 'react';

interface Battle {
  id: number;
  creatureA: number;
  creatureB: number;
  winner: number;
}

export default function BattleNotifications() {
  const [connected, setConnected] = useState(false);
  const [battles, setBattles] = useState<Battle[]>([]);

  useEffect(() => {
    if ('Notification' in window && Notification.permission === 'default') {
      Notification.requestPermission().catch(() => {
        /* ignore */
      });
    }

    const ws = new WebSocket('ws://localhost:8767/battles');
    ws.onopen = () => setConnected(true);
    ws.onclose = () => setConnected(false);
    ws.onerror = () => setConnected(false);
    ws.onmessage = ev => {
      try {
        const b = JSON.parse(ev.data) as Battle;
        setBattles(list => [...list.slice(-19), b]);

        if (
          'Notification' in window &&
          Notification.permission === 'granted'
        ) {
          new Notification(`Battle ${b.id} winner ${b.winner}`, {
            body: `${b.creatureA} vs ${b.creatureB}`,
          });
        }
      } catch (err) {
        console.warn('Invalid battle message', err);
      }
    };
    return () => {
      ws.close();
    };
  }, []);

  return (
    <div>
      <h2>Battle Notifications</h2>
      <div>Status: {connected ? 'connected' : 'disconnected'}</div>
      <ul>
        {battles.map(b => (
          <li key={b.id}>
            Battle {b.id}: {b.creatureA} vs {b.creatureB} – winner {b.winner}
          </li>
        ))}
      </ul>
    </div>
  );
}
