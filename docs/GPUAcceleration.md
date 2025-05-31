# GPU Acceleration and Vulkan Backend

This guide covers building Chain Beasts with GPU support enabled. The runtime uses Vulkan for high performance training and falls back to the CPU path when no compatible device is detected. On macOS the Vulkan loader is typically provided by MoltenVK.

## 1. Prerequisites

1. Install the [Vulkan SDK](https://vulkan.lunarg.com/) (1.3 or newer). On macOS install MoltenVK via the SDK or through Homebrew.
2. Ensure `vulkaninfo` prints your device details and that CMake can locate the headers and loader library.

## 2. Build Flags

* `-DNEUROPET_ENABLE_VULKAN=ON` – compiles Chain Beasts with GPU kernels and passes `HARMONICS_HAS_VULKAN=1` to the underlying library.
* Set `HARMONICS_ENABLE_VULKAN=1` at runtime to activate the Vulkan backend. Optionally specify `HARMONICS_VULKAN_DEVICE=<index>` to select a GPU.

## 3. Building with Vulkan

Configure CMake with the Vulkan backend enabled:
```bash
cmake -S . -B build-vulkan \
    -DCMAKE_BUILD_TYPE=Release \
    -DNEUROPET_ENABLE_VULKAN=ON
cmake --build build-vulkan -j $(nproc)
```
This compiles the Harmonics library with GPU kernels and produces the standard executables under `build-vulkan/`.

## 4. Runtime Setup

Set the environment variable `HARMONICS_ENABLE_VULKAN=1` to allow the runtime to initialise the GPU backend:
```bash
export HARMONICS_ENABLE_VULKAN=1
```
When multiple devices are available you can select one via `HARMONICS_VULKAN_DEVICE=<index>` or programmatically using `harmonics::set_vulkan_device_index()`.

If the loader or device is unavailable the runtime automatically uses the CPU implementation, so the build works on machines without Vulkan.

## 5. Metal Backend

On Apple hardware the easiest path is MoltenVK which translates Vulkan calls to Metal. Install the SDK and follow the same build steps above. No additional flags are required.

## 6. Example Usage

After compiling Chain Beasts with Vulkan enabled you can run the demo programs on
the GPU. `metrics_demo` executes a short training loop and prints the running
loss. Enable the backend with:

```bash
export HARMONICS_ENABLE_VULKAN=1
./build-vulkan/metrics_demo
```

When several devices are present select one via `HARMONICS_VULKAN_DEVICE=<index>`
or by calling `harmonics::set_vulkan_device_index()` before creating a
`CycleRuntime`. The helper `int8_gpu_available()` reports whether the Vulkan
runtime was successfully initialised.

---

With GPU acceleration enabled training and inference run significantly faster, especially for large creature models.
