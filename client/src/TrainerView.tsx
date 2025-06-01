import { useState } from 'react';
import { Trainer } from './trainer';
import { Marketplace } from './marketplace';
import MetricsChart from './MetricsChart';

export default function TrainerView() {
  const [trainer] = useState(() => new Trainer());
  const [marketplace] = useState(() => new Marketplace());
  const [log, setLog] = useState<string[]>([]);
  const [steps, setSteps] = useState('1');
  const [parentA, setParentA] = useState('');
  const [parentB, setParentB] = useState('');

  const append = (msg: string) => setLog(l => [...l, msg]);

  const connect = async () => {
    try {
      await trainer.ensureWallet();
      append('Wallet connected');
    } catch {
      append('Wallet provider not found');
    }
  };

  const hatch = async () => {
    await trainer.hatch();
    append('Hatched creature');
  };

  const train = async () => {
    await trainer.train(Number(steps) || 1, m => {
      append(`Training step ${m.step} result: ${m.loss}`);
    });
  };

  const breed = async () => {
    await marketplace.breed(Number(parentA), Number(parentB), new Uint8Array());
    append('Breeding transaction sent');
    setParentA('');
    setParentB('');
  };

  return (
    <div>
      <h2>Trainer</h2>
      <button onClick={connect}>Connect Wallet</button>
      <button onClick={hatch}>Hatch Creature</button>
      <div>
        <input
          placeholder="Steps"
          value={steps}
          onChange={e => setSteps(e.target.value)}
        />
        <button onClick={train}>Run Training</button>
      </div>
      <div>
        <input
          placeholder="Parent A"
          value={parentA}
          onChange={e => setParentA(e.target.value)}
        />
        <input
          placeholder="Parent B"
          value={parentB}
          onChange={e => setParentB(e.target.value)}
        />
        <button onClick={breed}>Breed</button>
      </div>
      <MetricsChart />
      <pre>{log.join('\n')}</pre>
    </div>
  );
}
