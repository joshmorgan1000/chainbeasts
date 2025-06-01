import { useState } from 'react';
import { lockTraits } from './traits';

export default function CustomizationView() {
  const [tokenId, setTokenId] = useState('');
  const [traits, setTraits] = useState('');
  const [nameHash, setNameHash] = useState('');
  const [log, setLog] = useState<string[]>([]);

  const append = (m: string) => setLog(l => [...l, m]);

  const lock = async () => {
    try {
      await lockTraits(Number(tokenId), traits, nameHash);
      append('Traits locked');
    } catch (err: any) {
      append('error: ' + err.message);
    }
  };

  return (
    <div>
      <h2>Trait Customisation</h2>
      <input
        placeholder="Token ID"
        value={tokenId}
        onChange={e => setTokenId(e.target.value)}
      />
      <input
        placeholder="Traits (hex)"
        value={traits}
        onChange={e => setTraits(e.target.value)}
      />
      <input
        placeholder="Name hash (hex32)"
        value={nameHash}
        onChange={e => setNameHash(e.target.value)}
      />
      <button onClick={lock}>Lock Traits</button>
      <pre>{log.join('\n')}</pre>
    </div>
  );
}
