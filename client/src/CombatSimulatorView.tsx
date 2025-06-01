import { useState } from 'react';
import initNeuropet from '../build-wasm/neuropet.js';

interface Creature {
  id: number;
  power: number;
  defense: number;
  stamina: number;
}

function randInt(min: number, max: number): number {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function randomCreature(id: number): Creature {
  return {
    id,
    power: randInt(1, 10),
    defense: randInt(1, 10),
    stamina: randInt(1, 10),
  };
}

async function trainStep(wasm: any, creature: Creature): Promise<void> {
  const A = new Int8Array([1, 2, 3, 4]);
  const B = new Int8Array([5, 6, 7, 8]);
  const C = new Int8Array(4);
  wasm.int8_matmul(A, B, C, 2, 2, 2);
  creature.power += C[0] & 1;
  creature.defense += C[1] & 1;
  creature.stamina += C[2] & 1;
}

function fight(a: Creature, b: Creature): number {
  let hpA = a.defense;
  let hpB = b.defense;
  let staA = a.stamina;
  let staB = b.stamina;
  let aTurn = true;
  while (hpA > 0 && hpB > 0 && (staA > 0 || staB > 0)) {
    if (aTurn) {
      if (staA > 0) {
        hpB -= a.power;
        staA--;
      }
    } else {
      if (staB > 0) {
        hpA -= b.power;
        staB--;
      }
    }
    aTurn = !aTurn;
  }
  return hpA >= hpB ? a.id : b.id;
}

export default function CombatSimulatorView() {
  const [modulePromise] = useState(() => initNeuropet());
  const [nextId, setNextId] = useState(1);
  const [creatureA, setCreatureA] = useState<Creature | null>(null);
  const [creatureB, setCreatureB] = useState<Creature | null>(null);
  const [log, setLog] = useState<string[]>([]);

  const append = (msg: string) => setLog(l => [...l, msg]);

  const hatchA = () => {
    const c = randomCreature(nextId);
    setNextId(nextId + 1);
    setCreatureA(c);
    append(`Hatched creature A ${c.id}`);
  };

  const hatchB = () => {
    const c = randomCreature(nextId);
    setNextId(nextId + 1);
    setCreatureB(c);
    append(`Hatched creature B ${c.id}`);
  };

  const trainA = async () => {
    if (!creatureA) return;
    const wasm = await modulePromise;
    const c = { ...creatureA };
    await trainStep(wasm, c);
    setCreatureA(c);
    append(`Trained creature A ${c.id}`);
  };

  const trainB = async () => {
    if (!creatureB) return;
    const wasm = await modulePromise;
    const c = { ...creatureB };
    await trainStep(wasm, c);
    setCreatureB(c);
    append(`Trained creature B ${c.id}`);
  };

  const battle = () => {
    if (!creatureA || !creatureB) return;
    const winner = fight(creatureA, creatureB);
    append(`Battle result: winner ${winner}`);
  };

  return (
    <div>
      <h2>Combat Simulator</h2>
      <div className="overlay top">
        {creatureA && (
          <div>
            A – P{creatureA.power} D{creatureA.defense} S{creatureA.stamina}
          </div>
        )}
        {creatureB && (
          <div>
            B – P{creatureB.power} D{creatureB.defense} S{creatureB.stamina}
          </div>
        )}
      </div>
      <canvas id="battleCanvas" width="480" height="240"></canvas>
      <div className="overlay bottom">
        <pre>{log.join('\n')}</pre>
      </div>
      <div>
        <button onClick={hatchA}>Hatch A</button>
        <button onClick={hatchB}>Hatch B</button>
        <button onClick={trainA}>Train A</button>
        <button onClick={trainB}>Train B</button>
        <button onClick={battle}>Battle</button>
      </div>
    </div>
  );
}
