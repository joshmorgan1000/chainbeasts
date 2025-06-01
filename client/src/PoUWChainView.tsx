import { useEffect, useState } from 'react';
import { PoUWMonitor, MiningReward } from './pouw';
import { connect } from './wallet';

export default function PoUWChainView() {
  const [address, setAddress] = useState<string | null>(null);
  const [rewards, setRewards] = useState<MiningReward[]>([]);

  const connectWallet = async () => {
    try {
      await connect();
      const eth = (window as any).ethereum;
      const [addr] = await eth.request({ method: 'eth_requestAccounts' });
      setAddress(addr.toLowerCase());
    } catch (err) {
      console.warn('wallet connect failed:', err);
    }
  };

  useEffect(() => {
    if (!address) return;
    const monitor = new PoUWMonitor();
    monitor.start(address, r => setRewards(rs => [...rs, r]));
    return () => monitor.stop();
  }, [address]);

  return (
    <div>
      <h2>PoUW Chain</h2>
      {address ? (
        <div>Miner: {address}</div>
      ) : (
        <button onClick={connectWallet}>Connect Wallet</button>
      )}
      <ul>
        {rewards.map((r, i) => (
          <li key={i}>Block {r.block} – reward {r.amount.toString()}</li>
        ))}
      </ul>
    </div>
  );
}
