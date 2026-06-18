#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "idt.h"

// stddef.h er bodole nije define korlam
typedef unsigned int size_t;
#define NULL ((void*)0)

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

typedef struct page {
   uint32_t present : 1;
   uint32_t rw : 1;
   uint32_t user : 1;
   uint32_t accessed : 1;
   uint32_t dirty : 1;
   uint32_t unused : 7;
   uint32_t frame : 20;
} page_t;

typedef struct page_table {
   page_t pages[PAGE_ENTRIES];
} page_table_t;

typedef struct page_directory {
   page_table_t *tables[PAGE_ENTRIES];
   uint32_t tablesPhysical[PAGE_ENTRIES];
   uint32_t physicalAddr;
} page_directory_t;

extern void enable_paging();
extern void *memset(void*, int, size_t);
extern uint32_t kmalloc_a(uint32_t);
extern uint32_t kmalloc_ap(uint32_t, uint32_t*);

void paging_init();
void switch_page_directory(page_directory_t *new);
page_t *get_page(uint32_t address, int make, page_directory_t *dir);
void page_fault(registers_t *regs);

#endif