export interface PluginInfo {
  name: string;
  version: string;
  description: string;
  url: string;
}

/**
 * Client-side helper for fetching and installing Harmonics plugins
 * from the community marketplace.
 */
export class PluginMarketplace {
  async getAvailablePlugins(): Promise<PluginInfo[]> {
    try {
      const res = await fetch('/api/plugins');
      if (!res.ok) throw new Error('failed');
      return (await res.json()) as PluginInfo[];
    } catch (err) {
      console.warn('getAvailablePlugins failed:', err);
      return [];
    }
  }

  async install(plugin: PluginInfo): Promise<void> {
    try {
      const res = await fetch(plugin.url);
      if (!res.ok) throw new Error('download failed');
      const blob = await res.blob();
      const a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = plugin.url.split('/').pop() || plugin.name;
      a.style.display = 'none';
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      this.recordInstall(plugin);
    } catch (err) {
      console.warn('install failed:', err);
    }
  }

  private versionCompare(a: string, b: string): number {
    const pa = a.split('.').map(n => parseInt(n, 10));
    const pb = b.split('.').map(n => parseInt(n, 10));
    for (let i = 0; i < Math.max(pa.length, pb.length); ++i) {
      const na = pa[i] || 0;
      const nb = pb[i] || 0;
      if (na < nb) return -1;
      if (na > nb) return 1;
    }
    return 0;
  }

  private getInstalled(): Record<string, string> {
    try {
      const data = localStorage.getItem('installed_plugins');
      if (data) return JSON.parse(data) as Record<string, string>;
    } catch (err) {
      console.warn('failed to load installed plugins:', err);
    }
    return {};
  }

  private recordInstall(plugin: PluginInfo) {
    const installed = this.getInstalled();
    installed[plugin.name] = plugin.version;
    try {
      localStorage.setItem('installed_plugins', JSON.stringify(installed));
    } catch (err) {
      console.warn('failed to record install:', err);
    }
  }

  async autoUpdateInstalled(): Promise<void> {
    const available = await this.getAvailablePlugins();
    const installed = this.getInstalled();
    for (const p of available) {
      const current = installed[p.name];
      if (current && this.versionCompare(current, p.version) < 0) {
        await this.install(p);
      }
    }
  }
}
