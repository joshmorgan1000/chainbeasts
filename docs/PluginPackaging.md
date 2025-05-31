# Packaging Harmonics Plugins

This guide outlines how to build, package and distribute plugins for the Harmonics runtime. Plugins extend the library with custom layers, activations or losses and are loaded dynamically at runtime.

## 1. Build as a shared library

Create a CMake project that compiles your plugin source files into a shared library. The library must define the entry point

```cpp
extern "C" void harmonics_register(harmonics::FunctionRegistry&);
```

Use `add_library(my_plugin SHARED files.cpp)` and link against Harmonics if needed. Keep the library self‑contained by avoiding unnecessary external dependencies.

## 2. Version and metadata

Set the plugin version with `project(my_plugin VERSION x.y.z)` and optionally export metadata such as supported Harmonics version in a small `plugin.json` file alongside the library. Tools can inspect this file to verify compatibility before loading the plugin.

## 3. Packaging layout

Place the compiled library and manifest inside a directory named after the plugin:

```
my_plugin/
├── plugin.json
└── libmy_plugin.so
```

Compress the directory into a `.tar.gz` or `.zip` archive for distribution. On Windows the library should use the `.dll` extension.

### Packaging with the helper script

Run `scripts/package_plugin.js <dir>` to create the archive automatically. The
script reads `plugin.json`, verifies that the optional `harmonics_version` field
matches the current Harmonics library and produces `<name>-<version>.tar.gz`.

## 4. Distributing plugins

Distribute the archive through your preferred channel (website, package registry or direct download). Users extract the archive into any folder listed in the `HARMONICS_PLUGIN_PATH` environment variable. The runtime scans these paths and loads all discovered plugins via `load_plugins_from_path()`.

For projects using CPack you can create a package target with:

```cmake
include(CPack)
set(CPACK_GENERATOR "TGZ")
install(TARGETS my_plugin DESTINATION .)
install(FILES plugin.json DESTINATION .)
```

Running `cpack` produces `my_plugin-x.y.z-Linux.tar.gz` ready for upload.

## 5. Updating and unloading

Plugins remain loaded until `unload_plugin()` is called. Ship updates under a new version directory so that old projects can keep using previous builds. Loading two versions simultaneously is supported as long as symbol names do not clash.

## 6. Automation and versioning

Automate package creation in your CI pipeline so that every tagged commit
produces a versioned archive. A typical workflow is:

1. Set the plugin version with `project()` as shown above and emit the same
   number in `plugin.json`.
2. Configure CPack to generate a `.tar.gz` archive when `cpack` is run.
3. In CI, build the plugin, invoke `cpack` and upload the resulting file to your
   release page or package registry.

Use [semantic versioning](https://semver.org/) for both the CMake project and
the `plugin.json` metadata. Bump the major version only for incompatible API
changes. Minor versions may add functionality and patch releases should remain
backwards compatible.

