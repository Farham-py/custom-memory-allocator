# Custom Vector Allocator (C++)

A hand-built dynamic array (`std::vector`-like) implementation backed by a single static memory arena, written from scratch in C++ without using `new`, `malloc`, or STL containers for storage.

This project was built to understand — at the byte level — how dynamic arrays manage memory: contiguous allocation, growth/reallocation strategy, and metadata layout, all implemented manually on top of a raw `unsigned char` heap.

## Why this exists

Most engineers use `std::vector` without ever seeing what happens underneath. This project reimplements the core mechanics of a dynamic array manager on a fixed 1MB memory pool, including:

- A custom heap with manual allocation and compaction (no OS heap calls)
- Per-vector headers storing size, capacity, type, and block size
- An ID table so vectors are referenced by handle, not raw pointer
- Automatic growth (reserve/reallocate) when capacity is exceeded
- Full compaction on destroy — freed memory is reclaimed and shifted, not leaked

## Features

- Create typed vectors (`int` or `double`) with a given initial capacity
- `push_back`, `push_at` (insert), `pop_back`, `erase_range`, `clear`
- Random access via `vector_at`
- Deep copy of a vector (`vector_copy`)
- Automatic capacity growth on overflow
- Manual heap compaction on `vector_destroy` (shifts remaining blocks, updates all handles)
- Heap usage reporting (`print_memory_status`)
- Exception-based error handling for invalid IDs, out-of-range access, and heap overflow

## How the heap is organized

```
[ control area ] [ vector 1 header | vector 1 data ] [ vector 2 header | vector 2 data ] ...
```

- **Control area**: tracks total heap usage, vector count, and an ID → offset lookup table
- **Header** (per vector): `size`, `capacity`, `type`, `block size`
- **Data**: contiguous typed elements immediately following the header

All vectors live in one 1MB static array (`HEAP`). There is no fragmentation-tolerant allocator here by design — this is meant to demonstrate the mechanics clearly, not to be production-grade.

## Build & Run

```bash
g++ -std=c++17 -O2 -o vector_test main.cpp
./vector_test
```

## Example output

The included `main.cpp` runs a full functional test: creating vectors, pushing/inserting/erasing elements, copying a vector, filling the heap to capacity, and gracefully catching a heap-overflow exception — followed by clean vector destruction.

## What I learned

- Designing a metadata layout that supports safe relocation of memory blocks
- Handling pointer/offset invalidation after reallocation or compaction (via an indirection table)
- Writing exception-safe C++ without relying on the standard allocator
- Trade-offs between a flat static heap vs. a real free-list allocator

## Possible extensions

- Free-list based allocation instead of full compaction on destroy
- Template-based type support instead of an `enum`-based type tag
- Alignment handling for arbitrary struct types

## License

MIT
