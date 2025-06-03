#!/usr/bin/env node
import http from 'http';
import { promises as fs } from 'fs';
import { extname, join, dirname } from 'path';
import { fileURLToPath } from 'url';

const port = process.env.SIM_PORT ? Number(process.env.SIM_PORT) : 8080;
const __dirname = dirname(fileURLToPath(import.meta.url));
const root = join(__dirname, '..', 'examples', 'web_simulator');

const mime = {
  '.html': 'text/html',
  '.js': 'application/javascript',
  '.wasm': 'application/wasm',
  '.css': 'text/css',
};

const server = http.createServer(async (req, res) => {
  let path = req.url === '/' ? '/index.html' : req.url.split('?')[0];
  const file = join(root, path);
  try {
    const data = await fs.readFile(file);
    const type = mime[extname(file)] || 'application/octet-stream';
    res.writeHead(200, { 'Content-Type': type });
    res.end(data);
  } catch {
    res.writeHead(404);
    res.end();
  }
});

server.listen(port, () => {
  console.log(`Web simulator available at http://localhost:${port}/`);
});
