import { useEffect, useState } from 'react';

interface MarketEvent {
  id: number;
  tokenId: number;
  price: number;
  seller: string;
  type: 'listed' | 'sold';
}

export default function MarketplaceNotifications() {
  const [connected, setConnected] = useState(false);
  const [events, setEvents] = useState<MarketEvent[]>([]);

  useEffect(() => {
    if ('Notification' in window && Notification.permission === 'default') {
      Notification.requestPermission().catch(() => {
        /* ignore */
      });
    }

    const ws = new WebSocket('ws://localhost:8768/market');
    ws.onopen = () => setConnected(true);
    ws.onclose = () => setConnected(false);
    ws.onerror = () => setConnected(false);
    ws.onmessage = ev => {
      try {
        const e = JSON.parse(ev.data) as MarketEvent;
        setEvents(list => [...list.slice(-19), e]);
        if (
          'Notification' in window &&
          Notification.permission === 'granted'
        ) {
          const title = e.type === 'listed' ? 'New Listing' : 'Sale Completed';
          new Notification(title, {
            body: `Token ${e.tokenId} price ${e.price}`,
          });
        }
      } catch (err) {
        console.warn('Invalid marketplace message', err);
      }
    };
    return () => {
      ws.close();
    };
  }, []);

  return (
    <div>
      <h2>Marketplace Notifications</h2>
      <div>Status: {connected ? 'connected' : 'disconnected'}</div>
      <ul>
        {events.map(e => (
          <li key={e.id}>
            Event {e.id}: token {e.tokenId} price {e.price} –{' '}
            {e.type === 'listed' ? `listed by ${e.seller}` : 'sold'}
          </li>
        ))}
      </ul>
    </div>
  );
}
