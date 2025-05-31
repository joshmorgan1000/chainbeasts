# Plugin Marketplace Guide

This document explains how to install community-made Harmonics plugins using the built-in marketplace shipped with the client.

## 1. Browsing Plugins

1. Start the React client:
   ```bash
   cd client && npm start
   ```
2. Open the **Plugin Marketplace** view.
3. The client fetches `plugins.json` which lists available archives in the following format:
   ```json
   [
     {
       "name": "sample_layer_plugin",
       "version": "0.1.0",
       "description": "Example plugin adding custom layers",
       "url": "https://example.com/plugins/sample_layer_plugin-0.1.0.tar.gz"
     }
   ]
   ```

## 2. Installing Plugins

Select a plugin and click **Install**. The archive is downloaded and saved to your machine. Extract it into a directory listed in `HARMONICS_PLUGIN_PATH`:

```bash
mkdir -p ~/.harmonics/plugins
tar -xf sample_layer_plugin-0.1.0.tar.gz -C ~/.harmonics/plugins/
```

Restart the application or call `load_plugins_from_path()` to load the new plugin.

The client remembers installed plugin versions and automatically downloads
updates when the marketplace lists a newer release.

## 3. Publishing Your Plugin

1. Package the plugin as described in [PluginPackaging.md](PluginPackaging.md).
2. Host the archive and add an entry to `plugins.json` with its name, version, description and download URL.
3. Clients will see the new plugin after refreshing the marketplace view.

---
