#ifndef KMALLOC_H
#define KMALLOC_H

#include "types.h"

void init_heap();

/**
 * Allocate specified size of memory.
 * The Allocator can't be guaranteed.
 * 128MB for the allocator
 */
void* kmalloc(uint32_t size);

/**
 * Free a chunk of memory.
 */
void kfree(void* pos);

void test_kmalloc();

#endif
