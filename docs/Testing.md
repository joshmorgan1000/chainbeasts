# Testing Workflow

This project uses CMake and GoogleTest for the native components and standard Solidity frameworks for the contracts.

## Running the Tests

Use the helper script provided in `scripts/`:

```bash
./scripts/run_tests.sh [Debug|Release]
```

The script configures a build directory (`build-Release` by default), compiles all targets and then runs the CMake `test` target via `ctest`. Pass `Debug` to build with debug symbols.

You can also invoke the target manually:

```bash
cmake --build build-Release --target test
```

Contract tests may be executed separately using either Foundry or Hardhat inside the `contracts/` folder:

```bash
forge test        # with Foundry
# or
npx hardhat test  # with Hardhat
```

---

© 2025 Cognithesis Labs – Draft
