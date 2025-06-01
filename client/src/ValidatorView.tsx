import { useEffect, useState } from 'react';

interface Proof {
  root: string;
  proof: string;
}

export default function ValidatorView() {
  const [connected, setConnected] = useState(false);
  const [proofs, setProofs] = useState<Proof[]>([]);

  useEffect(() => {
    const ws = new WebSocket('ws://localhost:8765/validator');
    ws.onopen = () => setConnected(true);
    ws.onclose = () => setConnected(false);
    ws.onerror = () => setConnected(false);
    ws.onmessage = e => {
      try {
        const data = JSON.parse(e.data) as Proof;
        setProofs(p => [...p, data]);
      } catch (err) {
        console.warn('Invalid proof message', err);
      }
    };
    return () => {
      ws.close();
    };
  }, []);

  return (
    <div>
      <h2>Validator</h2>
      <div>Status: {connected ? 'connected' : 'disconnected'}</div>
      <ul>
        {proofs.map((p, i) => (
          <li key={i}>
            <div>Root: {p.root}</div>
            <div>Proof: {p.proof}</div>
          </li>
        ))}
      </ul>
    </div>
  );
}
