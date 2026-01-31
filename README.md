# tiny-malloc

A proof-of-concept custom memory allocator for Linux/macOS.

## What It Teaches

- **The Program Break:** In Unix-like systems, the "heap" is an area of memory that ends at a pointer called the "break." We use `sbrk()` to move this break higher, effectively asking the kernel to map more physical RAM to our process.
- **Metadata Management:** Every time you call `malloc`, the system doesn't just give you raw bytes; it stores a "header" (the `Block` struct) right before the memory it gives you. This is how `free()` knows how much memory it's supposed to release.
- **Memory Alignment:** CPUs are more efficient (and sometimes require) data to start at addresses that are multiples of $8$ or $16$. This project demonstrates a simple bitwise alignment: `(size + 7) & ~7`.

## How It Works Internally

1. **Linked List:** The allocator maintains a single-linked list of blocks.
2. **Allocation:** When `tiny_malloc` is called, it traverses the list to find a block marked `is_free` with enough size. If none is found, it calls `sbrk` to extend the heap.
3. **Freeing:** `tiny_free` simply takes the pointer provided by the user, subtracts the size of the header to find the `Block` struct, and sets `is_free` to $1$.

## How to Run

1. Compile with GCC:

   ```bash
   gcc -o tiny_malloc malloc.c
   ```

2. Run the binary:

   ```bash
   ./tiny_malloc
   ```

## Limitations (For further exploration)

- **No Coalescing:** If you free two adjacent blocks, this allocator doesn't merge them into one big block, leading to fragmentation.

- **No Splitting:** If you ask for $8$ bytes and find a free block of $1024$ bytes, it gives you the whole $1024$.