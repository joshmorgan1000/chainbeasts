#!/usr/bin/env node
import http from 'http';
import { readFileSync } from 'fs';
import { dirname, join } from 'path';
import { fileURLToPath } from 'url';

const port = process.env.PLUGIN_PORT ? Number(process.env.PLUGIN_PORT) : 8780;
const apiPath = '/api/plugins';

const __dirname = dirname(fileURLToPath(import.meta.url));
const pluginsFile = join(__dirname, '..', 'client', 'public', 'plugins.json');
let plugins = [];
try {
  plugins = JSON.parse(readFileSync(pluginsFile, 'utf8'));
} catch (err) {
  console.warn('failed to load plugins.json:', err);
}

const server = http.createServer((req, res) => {
  if (req.url === apiPath && req.method === 'GET') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify(plugins));
  } else {
    res.writeHead(404);
    res.end();
  }
});

server.listen(port, () => {
  console.log(`Plugin marketplace server listening on http://localhost:${port}${apiPath}`);
});
