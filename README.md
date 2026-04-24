# 📦 List Loading and Binary Serialization

Console C++20 application that reads `inlet.in`, where each line has the form `<data>;<rand_index>`, builds a doubly linked list and serializes it to the binary file `outlet.out`.

## Output Format

During serialization, each node is assigned a sequential index. This makes the binary format suitable for later deserialization: instead of restoring raw pointer values, the reader can recreate all nodes first and then reconnect `rand` links by saved indices.

`outlet.out` contains the total number of nodes and, for each node, the string length, the `rand` target index or `-1` for `nullptr`, and the raw string bytes.

## Build

```bash
cmake -S . -B out/build
cmake --build out/build
```

## Run

Place `inlet.in` in the project root and run the executable. On success, the program writes `outlet.out` to the current working directory.
