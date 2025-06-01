#!/usr/bin/env node
import { WebSocketServer } from 'ws';

const port = process.env.VALIDATOR_PORT ? Number(process.env.VALIDATOR_PORT) : 8766;
const path = '/validator';

const wss = new WebSocketServer({ port, path, perMessageDeflate: false });

wss.on('connection', ws => {
  // Disable Nagle's algorithm for lower latency
  ws._socket.setNoDelay(true);
  ws.on('message', data => {
    for (const client of wss.clients) {
      if (client.readyState === client.OPEN) {
        client.send(data);
      }
    }
  });
});

console.log(`Validator WebSocket server listening on ws://localhost:${port}${path}`);
