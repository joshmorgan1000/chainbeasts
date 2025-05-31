# WebAssembly Backend and Browser Setup

This document explains how to build the Chain Beasts INT8 kernel and Harmonics runtime for WebAssembly and how to load the resulting module inside a browser.

## Building the Module

The project uses [Emscripten](https://emscripten.org/) to produce a `wasm32-unknown-unknown` build. Ensure `emcmake` is available in your `$PATH` and run:

```bash
(cd scripts && ./build_wasm.sh)
```

The script configures CMake with `emcmake` and builds into `build-wasm/`. When compiled this way the Harmonics library defines `HARMONICS_HAS_WASM=1` and the runtime automatically selects the WebAssembly backend when executed in a browser.

The directory will contain `neuropet.js` and `neuropet.wasm` which can be served alongside your client code.

## Loading in the Browser

Include the generated JavaScript loader using a module script:

```html
<script type="module">
import initNeuropet from './build-wasm/neuropet.js';

initNeuropet().then((exports) => {
    // Example: call the INT8 matmul helper
    const A = new Int8Array([1, 2, 3, 4]);
    const B = new Int8Array([5, 6, 7, 8]);
    const C = new Int8Array(4);
    exports.int8_matmul(A, B, C, 2, 2, 2);
    console.log('C:', [...C]);
});
</script>
```

Any exported kernel function can be invoked the same way. When the module runs outside of the browser (for example under Node.js) the runtime falls back to the CPU implementation.
