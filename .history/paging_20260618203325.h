#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "idt.h" // registers_t এর জন্য

// বিটফিল্ড 4 বাইটে প্যাক করতে হবে
typedef struct page {
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed: 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;
} __attribute__((packed)) page_t;

// 1024 টা পেজের অ্যারে (4KB)
typedef struct page_table {
    page_t pages[1024];
} __attribute__((packed)) page_table_t;

// Directory-তেও 1024 টা পয়েন্টার এবং ফিজিক্যাল অ্যাড্রেসের অ্যারে থাকতে হবে
typedef struct page_directory {
    uint32_t tablesPhysical[1024]; // CPU সরাসরি এই অ্যারে রিড করবে
    page_table_t *tables[1024];    // আমাদের ট্র্যাক রাখার জন্য
    uint32_t physicalAddr;
} page_directory_t;

void paging_init(void);
void page_fault(registers_t *regs);

#endif