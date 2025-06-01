#!/usr/bin/env node
import { WebSocketServer } from 'ws';

const port = process.env.MARKET_PORT ? Number(process.env.MARKET_PORT) : 8768;
const path = '/market';

const wss = new WebSocketServer({ port, path });

wss.on('connection', ws => {
  ws.on('message', data => {
    for (const client of wss.clients) {
      if (client.readyState === client.OPEN) {
        client.send(data);
      }
    }
  });
});

console.log(`Marketplace WebSocket server listening on ws://localhost:${port}${path}`);
