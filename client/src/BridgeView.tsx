import { useState } from 'react';
import { Bridge } from './bridge';

export default function BridgeView() {
  const [bridge] = useState(() => new Bridge());
  const [outToken, setOutToken] = useState('');
  const [dstChain, setDstChain] = useState('');
  const [inToken, setInToken] = useState('');
  const [srcChain, setSrcChain] = useState('');
  const [weights, setWeights] = useState('');
  const [dna, setDna] = useState('');
  const [log, setLog] = useState<string[]>([]);

  const append = (m: string) => setLog(l => [...l, m]);

  const bridgeOut = async () => {
    try {
      await bridge.bridgeOut(Number(outToken), Number(dstChain));
      append('bridge out sent');
    } catch (err: any) {
      append('error: ' + err.message);
    }
  };

  const bridgeIn = async () => {
    try {
      const data = weights.startsWith('0x') ? weights.slice(2) : weights;
      const bytes = new Uint8Array(
        data.match(/.{1,2}/g)?.map(b => parseInt(b, 16)) || [],
      );
      await bridge.bridgeIn(Number(inToken), Number(srcChain), bytes, dna);
      append('bridge in sent');
    } catch (err: any) {
      append('error: ' + err.message);
    }
  };

  return (
    <div>
      <h2>Cross-Chain Bridge</h2>
      <div>
        <input
          placeholder="Token ID"
          value={outToken}
          onChange={e => setOutToken(e.target.value)}
        />
        <input
          placeholder="Destination Chain ID"
          value={dstChain}
          onChange={e => setDstChain(e.target.value)}
        />
        <button onClick={bridgeOut}>Bridge Out</button>
      </div>
      <div>
        <input
          placeholder="Token ID"
          value={inToken}
          onChange={e => setInToken(e.target.value)}
        />
        <input
          placeholder="Source Chain ID"
          value={srcChain}
          onChange={e => setSrcChain(e.target.value)}
        />
        <input
          placeholder="Genesis Weights (hex)"
          value={weights}
          onChange={e => setWeights(e.target.value)}
        />
        <input
          placeholder="DNA (hex32)"
          value={dna}
          onChange={e => setDna(e.target.value)}
        />
        <button onClick={bridgeIn}>Bridge In</button>
      </div>
      <pre>{log.join('\n')}</pre>
    </div>
  );
}
