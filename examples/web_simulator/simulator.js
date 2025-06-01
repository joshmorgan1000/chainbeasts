import initNeuropet from '../../build-wasm/neuropet.js';

class PRNG {
  constructor(seed = 1) { this.seed = seed >>> 0; }
  next() {
    this.seed = (this.seed * 1103515245 + 12345) >>> 0;
    return (this.seed >>> 1) / 0x7fffffff;
  }
  nextInt(min, max) { return Math.floor(this.next() * (max - min + 1)) + min; }
}

class Simulator {
  constructor(canvas, top, bottom) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d');
    this.top = top;
    this.bottom = bottom;
    this.prng = new PRNG(1);
    this.modulePromise = initNeuropet();
    this.nextId = 1;
    this.creatureA = null;
    this.creatureB = null;
    this.logLines = [];
  }

  log(msg) {
    this.logLines.push(msg);
    this.bottom.textContent = this.logLines.join('\n');
  }

  randomCreature() {
    return {
      id: this.nextId++,
      power: this.prng.nextInt(1, 10),
      defense: this.prng.nextInt(1, 10),
      stamina: this.prng.nextInt(1, 10)
    };
  }

  async train(creature) {
    const wasm = await this.modulePromise;
    const A = new Int8Array([1,2,3,4]);
    const B = new Int8Array([5,6,7,8]);
    const C = new Int8Array(4);
    wasm.int8_matmul(A, B, C, 2, 2, 2);
    creature.power += C[0] & 1;
    creature.defense += C[1] & 1;
    creature.stamina += C[2] & 1;
  }

  fight(a, b) {
    let hpA = a.defense;
    let hpB = b.defense;
    let staA = a.stamina;
    let staB = b.stamina;
    let aTurn = true;
    while (hpA > 0 && hpB > 0 && (staA > 0 || staB > 0)) {
      if (aTurn) {
        if (staA > 0) { hpB -= a.power; staA--; }
      } else {
        if (staB > 0) { hpA -= b.power; staB--; }
      }
      aTurn = !aTurn;
    }
    return hpA >= hpB ? a.id : b.id;
  }

  renderStats() {
    const a = this.creatureA;
    const b = this.creatureB;
    const parts = [];
    if (a) parts.push(`A \u2013 P${a.power} D${a.defense} S${a.stamina}`);
    if (b) parts.push(`B \u2013 P${b.power} D${b.defense} S${b.stamina}`);
    this.top.textContent = parts.join('    ');
  }

  hatchA() { this.creatureA = this.randomCreature(); this.log(`Hatched A ${this.creatureA.id}`); this.renderStats(); }
  hatchB() { this.creatureB = this.randomCreature(); this.log(`Hatched B ${this.creatureB.id}`); this.renderStats(); }
  async trainA() { if (!this.creatureA) return; await this.train(this.creatureA); this.log(`Trained A ${this.creatureA.id}`); this.renderStats(); }
  async trainB() { if (!this.creatureB) return; await this.train(this.creatureB); this.log(`Trained B ${this.creatureB.id}`); this.renderStats(); }
  battle() {
    if (!this.creatureA || !this.creatureB) return;
    const winner = this.fight(this.creatureA, this.creatureB);
    this.log(`Battle result: winner ${winner}`);
    this.renderStats();
  }
}

window.addEventListener('load', () => {
  const canvas = document.getElementById('battleCanvas');
  const top = document.getElementById('topOverlay');
  const bottom = document.getElementById('bottomOverlay');
  const sim = new Simulator(canvas, top, bottom);
  document.getElementById('hatchA').onclick = () => sim.hatchA();
  document.getElementById('hatchB').onclick = () => sim.hatchB();
  document.getElementById('trainA').onclick = () => sim.trainA();
  document.getElementById('trainB').onclick = () => sim.trainB();
  document.getElementById('battle').onclick = () => sim.battle();
});
