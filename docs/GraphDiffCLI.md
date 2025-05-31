# Graph Diff & Merge CLI

`graph_diff` provides a small command line interface for inspecting differences between two Harmonics graphs and merging updates. Diff files use a compact binary format with the `.hgrd` extension.
It is built along with the rest of the utilities and lives in the build directory after running `cmake`.

## Building

Run CMake and build the `graph_diff` target:

```bash
cmake -S . -B build
cmake --build build --target graph_diff -j$(nproc)
```

The resulting binary is placed in the build directory and can be run from there.

## Usage

The tool follows the pattern `graph_diff <command> [options]`. Run without
arguments to see the available commands:

```bash
./build/graph_diff --help
```

Each sub-command described below expects input graph files in the Harmonics
`*.hgr` format and optionally produces a `.hgrd` diff file.

## Commands

### `diff`

```
graph_diff diff base.hgr update.hgr -o changes.hgrd
```

Generates a binary description of the edits required to turn `base.hgr` into `update.hgr`.
If `-o` is omitted a short summary is printed to stdout.

### `apply`

```
graph_diff apply graph.hgr changes.hgrd -o new_graph.hgr
```

Applies a previously generated diff file and writes the result to `new_graph.hgr`.

### `merge`

```
graph_diff merge base.hgr patch.hgr -o merged.hgr
```

Merges `patch.hgr` into `base.hgr` and writes the merged graph to `merged.hgr`.

## Example workflow

1. Compute the diff between an old and a new graph:
   ```bash
   graph_diff diff old.hgr new.hgr -o update.hgrd
   ```
2. Review or version control `update.hgrd`.
3. Apply the diff on another machine:
   ```bash
   graph_diff apply old.hgr update.hgrd -o new.hgr
   ```
4. Alternatively merge a patch graph directly:
   ```bash
   graph_diff merge base.hgr tweaks.hgr -o final.hgr
```
5. Quickly inspect the differences without writing a file:
   ```bash
   graph_diff diff base.hgr updated.hgr
   ```
