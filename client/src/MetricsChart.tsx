import { useEffect, useRef, useState } from 'react';

interface Metric {
  step: number;
  loss: number;
}

/**
 * Simple real-time chart component showing training loss over time using the
 * metrics WebSocket stream.
 */
export default function MetricsChart() {
  const [data, setData] = useState<Metric[]>([]);
  const canvasRef = useRef<HTMLCanvasElement>(null);

  // Connect to metrics WebSocket on mount
  useEffect(() => {
    const ws = new WebSocket('ws://localhost:8765/metrics');
    ws.onmessage = ev => {
      try {
        const m = JSON.parse(ev.data) as Metric;
        setData(d => [...d.slice(-99), m]);
      } catch (err) {
        console.warn('Invalid metrics message', err);
      }
    };
    return () => {
      ws.close();
    };
  }, []);

  // Draw chart whenever data updates
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    const w = canvas.width;
    const h = canvas.height;
    ctx.clearRect(0, 0, w, h);
    if (data.length === 0) return;

    const maxLoss = Math.max(...data.map(m => m.loss), 1);
    ctx.beginPath();
    data.forEach((m, i) => {
      const x = (i / (data.length - 1 || 1)) * w;
      const y = h - (m.loss / maxLoss) * h;
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.strokeStyle = '#00f';
    ctx.stroke();
  }, [data]);

  return (
    <div>
      <canvas ref={canvasRef} width={300} height={150} />
    </div>
  );
}
