import initNeuropet from '../build-wasm/neuropet.js';
import { connect, sendTransaction } from './wallet.js';

const CREATURE_NFT_ADDR = window.CREATURE_NFT_ADDR || '0x0000000000000000000000000000000000000000';
const TRAINING_LEDGER_ADDR = window.TRAINING_LEDGER_ADDR || '0x0000000000000000000000000000000000000000';
const HATCH_SELECTOR = '0xd106adbb';
const SUBMIT_SELECTOR = '0x5df4059d';

export class Trainer {
    constructor() {
        this.modulePromise = initNeuropet();
        this.wasm = null;
        this.walletConnected = false;
        this.socket = null;
        this.hatchedId = null;
    }

    async ensureWallet() {
        if (!this.walletConnected) {
            await connect();
            this.walletConnected = true;
        }
    }

    async initModule() {
        if (!this.wasm) {
            this.wasm = await this.modulePromise;
        }
    }

    async hatch(weights = new Uint8Array()) {
        await this.ensureWallet();
        const data = HATCH_SELECTOR + Buffer.from(weights).toString('hex');
        await sendTransaction(CREATURE_NFT_ADDR, '0x' + data);
    }

    async submitCheckpoint(root) {
        const data = SUBMIT_SELECTOR + root.replace(/^0x/, '');
        await sendTransaction(TRAINING_LEDGER_ADDR, '0x' + data);
    }

    ensureSocket() {
        if (this.socket)
            return;
        try {
            this.socket = new WebSocket('ws://localhost:8765/metrics');
        }
        catch (err) {
            console.warn('Failed to connect metrics socket:', err);
            this.socket = null;
        }
    }

    /**
     * Run a number of training steps using the WASM kernel.
     * @param {number} steps Number of steps to run
     * @param {(metrics: {step:number, loss:number}) => void} cb Callback for step metrics
     */
    async train(steps = 1, cb = null) {
        await this.ensureWallet();
        await this.initModule();
        this.ensureSocket();
        const A = new Int8Array([1, 2, 3, 4]);
        const B = new Int8Array([5, 6, 7, 8]);
        const C = new Int8Array(4);
        for (let i = 0; i < steps; ++i) {
            this.wasm.int8_matmul(A, B, C, 2, 2, 2);
            const metric = { step: i + 1, loss: C[0] };
            if (this.socket && this.socket.readyState === WebSocket.OPEN) {
                this.socket.send(JSON.stringify(metric));
            }
            if (cb)
                cb(metric);
            const root = '0x' + C[0].toString(16).padStart(2, '0');
            await this.submitCheckpoint(root);
        }
    }
}
