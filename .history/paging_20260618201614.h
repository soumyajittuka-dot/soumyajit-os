#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// বিটফিল্ড 4 বাইটে প্যাক করতে হবে
typedef struct page {
   uint32_t present : 1;
   uint32_t rw : 1;
   uint32_t user : 1;
   uint32_t accessed: 1;
   uint32_t dirty : 1;
   uint32_t unused : 7;
   uint32_t frame : 20;
} __attribute__((packed)) page_t; // <-- এটা মিসিং ছিল

// সিম্পল রাখো। 1024 পয়েন্টারের দরকার নাই এখন
typedef struct page_table {
   page_t pages;
} __attribute__((packed)) page_table_t;

typedef struct page_directory {
   page_table_t *tables;
   uint32_t tablesPhysical;
   uint32_t physicalAddr;
} page_directory_t;

void paging_init(void);
void page_fault(registers_t *regs);

#endif