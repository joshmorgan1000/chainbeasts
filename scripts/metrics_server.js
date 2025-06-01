#!/usr/bin/env node
import { WebSocketServer } from 'ws';

const port = process.env.METRICS_PORT ? Number(process.env.METRICS_PORT) : 8765;
const path = '/metrics';

const wss = new WebSocketServer({ port, path, perMessageDeflate: false });

wss.on('connection', (ws) => {
  // Disable Nagle's algorithm for lower latency
  ws._socket.setNoDelay(true);
  ws.on('message', (data) => {
    // Broadcast received metrics to all connected clients
    for (const client of wss.clients) {
      if (client.readyState === client.OPEN) {
        client.send(data);
      }
    }
  });
});

console.log(`Metrics WebSocket server listening on ws://localhost:${port}${path}`);
