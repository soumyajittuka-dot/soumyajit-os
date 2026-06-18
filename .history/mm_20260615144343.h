#ifndef MM_H
#define MM_H

#include "types.h" // uint32_t er jonno

// Basic Memory Functions
void mem_init();
void* kmalloc(uint32_t size);
void kfree(void* ptr);
int get_free_mem();
int get_used_mem();
void mem_stats();

// Paging Support Functions - EIGULO ADD KORO
uint32_t kmalloc_a(uint32_t size); // Aligned malloc
uint32_t kmalloc_ap(uint32_t size, uint32_t *phys); // Aligned + Physical addr
void paging_init();
#endif