#!/usr/bin/env node
import initNeuropet from '../build-wasm/neuropet.js';

(async () => {
    const wasm = await initNeuropet();
    const A = new Int8Array([1, 2, 3, 4]);
    const B = new Int8Array([5, 6, 7, 8]);
    const C = new Int8Array(4);
    wasm.int8_matmul(A, B, C, 2, 2, 2);
    console.log('Training step result:', Array.from(C).join(', '));
})();
