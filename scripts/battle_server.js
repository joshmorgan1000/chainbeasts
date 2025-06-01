#!/usr/bin/env node
import { WebSocketServer } from 'ws';

const port = process.env.BATTLE_PORT ? Number(process.env.BATTLE_PORT) : 8767;
const path = '/battles';

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

console.log(`Battle WebSocket server listening on ws://localhost:${port}${path}`);
