#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

// Memory layout: [ Block | User Data ]
// Linked list: global_base -> Block1 -> Block2 -> ...

// Structure to hold metadata for each memory block
// We use a linked list to keep track of allocated and free blocks
typedef struct Block {
    size_t size;        // How many bytes of data follow this header
    int is_free;        // Boolean flag: 1 if block is available, 0 if used
    struct Block *next; // Pointer to the next block in the list
} Block;

#define BLOCK_SIZE sizeof(Block)  // 24 bytes on 64-bit systems | 8 (size_t) + 4 (int) + 4 (padding) + 8 (pointer)

void *global_base = NULL;  // Head of the linked list

// Find a free block that fits the requested size (First Fit strategy)
Block *find_free_block(Block **last, size_t size) {
    Block *current = global_base;
    while (current && !(current->is_free && current->size >= size)) {
        *last = current;
        current = current->next;
    }
    return current;
}

// If no block is found, request more space from the OS using sbrk
Block *request_space(Block *last, size_t size) {
    Block *block;
    block = sbrk(0); // Get current break address
    void *request = sbrk(size + BLOCK_SIZE);
    
    assert(request != (void*) -1); // sbrk returns -1 on error

    if (last) { // If there was a previous block, link it
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->is_free = 0;
    return block;
}

void *tiny_malloc(size_t size) {
    Block *block;

    if (size <= 0) return NULL;

    // Round `size` up to next multiple of 8 for proper 64-bit memory alignment
    size = (size + 7) & ~7;

    if (!global_base) { // First call
        block = request_space(NULL, size);
        global_base = block;
    } else {
        Block *last = global_base;
        block = find_free_block(&last, size);
        if (!block) { // No free block found, ask OS for more
            block = request_space(last, size);
        } else {      // Found a block, mark it as used
            block->is_free = 0;
        }
    }

    // Return pointer to the actual data (immediately after the header)
    return (block + 1);
}

void tiny_free(void *ptr) {
    if (!ptr) return;

    // Get the block header by stepping back from the data pointer
    Block *block_ptr = (Block*)ptr - 1;
    block_ptr->is_free = 1;
}

// --- Test Program ---
int main() {
    printf("Allocating 128 bytes...\n");
    int *data = (int*)tiny_malloc(128);
    data[0] = 42;
    printf("Data stored: %d at %p\n", data[0], (void*)data);

    printf("Freeing data...\n");
    tiny_free(data);

    printf("Allocating 64 bytes (should reuse previous block)...\n");
    int *data2 = (int*)tiny_malloc(64);
    printf("New pointer: %p\n", (void*)data2);

    return 0;
}