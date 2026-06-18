#ifndef MM_H
#define MM_H

#include "types.h"

// Basic Memory Functions - Bitmap Heap
void mem_init();
void* kmalloc(uint32_t size);
void kfree(void* ptr);
int get_free_mem();
int get_used_mem();
void mem_stats();

// Paging Support - Placement Allocator
uint32_t kmalloc_a(uint32_t size);
uint32_t kmalloc_ap(uint32_t size, uint32_t *phys);
void paging_init();

#endif