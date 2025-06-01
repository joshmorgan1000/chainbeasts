import { useEffect, useState } from 'react';
import { PluginMarketplace, PluginInfo } from './plugin_marketplace';

export default function PluginMarketplaceView() {
  const [mp] = useState(() => new PluginMarketplace());
  const [plugins, setPlugins] = useState<PluginInfo[]>([]);

  const refresh = async () => {
    setPlugins(await mp.getAvailablePlugins());
  };

  const install = async (p: PluginInfo) => {
    await mp.install(p);
  };

  useEffect(() => {
    refresh();
    mp.autoUpdateInstalled();
  }, []);

  return (
    <div>
      <h2>Plugin Marketplace</h2>
      <button onClick={refresh}>Refresh</button>
      <ul>
        {plugins.map(p => (
          <li key={p.name}>
            {p.name} {p.version} - {p.description}
            <button onClick={() => install(p)}>Install</button>
          </li>
        ))}
      </ul>
    </div>
  );
}
