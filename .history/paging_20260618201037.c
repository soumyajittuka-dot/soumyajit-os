#include "paging.h"
#include "mm.h"
#include "vga.h"
#include "idt.h"
#include "screen.h"
#include "string.h"

extern void load_page_directory(uint32_t dir);
extern void enable_paging();

// ফিক্স 1: সব স্ট্যাটিক বানাও, kmalloc বাদ। 1MB এর নিচে থাকবে তাই অটো ম্যাপড
__attribute__((aligned(4096))) page_directory_t kernel_directory_data;
__attribute__((aligned(4096))) page_table_t first_page_table; // 0-4MB এর জন্য
__attribute__((aligned(4096))) uint32_t frames[32768]; // 128MB পর্যন্ত ফ্রেম ট্র্যাক

page_directory_t *kernel_directory = &kernel_directory_data;
page_directory_t *current_directory = 0;
uint32_t nframes;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static uint32_t first_frame() {
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i]!= 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(frames[i] & (0x1 << j))) {
                    return i*32 + j;
                }
            }
        }
    }
    return (uint32_t)-1;
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    if (page->frame!= 0) return;
    uint32_t idx = first_frame();
    if (idx == (uint32_t)-1) {
        print_string("NO FREE FRAMES!\n");
        while(1);
    }
    set_frame(idx * 0x1000);
    page->present = 1;
    page->rw = is_writeable? 1 : 0;
    page->user = is_kernel? 0 : 1;
    page->frame = idx;
}

void alloc_frame_physical(page_t *page, int is_kernel, int is_writeable, uint32_t phys_addr) {
    if (page->frame!= 0) return;
    page->present = 1;
    page->rw = is_writeable? 1 : 0;
    page->user = is_kernel? 0 : 1;
    page->frame = phys_addr / 0x1000;
    set_frame(phys_addr);
}

// ফিক্স 2: get_page() সিম্পল করো। kmalloc বাদ
page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
    address /= 0x1000;
    uint32_t table_idx = address / 1024;

    // শুধু প্রথম টেবিল ইউজ করছি - 0-4MB
    if (table_idx == 0) {
        return &((page_table_t*)dir->tables[0])->pages[address % 1024];
    }
    return 0; // আপাতত 4MB এর বাইরে সাপোর্ট করছি না
}

void page_fault(registers_t *regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    print_string("PAGE FAULT! at 0x");
    print_hex(faulting_address);
    print_string(" err:");
    print_hex(regs->err_code);
    print_string("\n");
    while(1);
}

void paging_init() {
    print_string("Paging Init Start\n");

    uint32_t mem_end_page = 0x1000000; // 16MB
    nframes = mem_end_page / 0x1000;
    memset(frames, 0, sizeof(frames)); // ফিক্স: স্ট্যাটিক অ্যারে ক্লিয়ার

    // ফিক্স 3: স্ট্যাটিক স্ট্রাকচার ইউজ করো
    memset(kernel_directory, 0, sizeof(page_directory_t));
    memset(&first_page_table, 0, sizeof(page_table_t));

    kernel_directory->physicalAddr = (uint32_t)&kernel_directory_data;
    kernel_directory->tables[0] = &first_page_table;
    kernel_directory->tablesPhysical[0] = (uint32_t)&first_page_table | 0x3; // Present, RW, Supervisor

    set_frame((uint32_t)&kernel_directory_data);
    set_frame((uint32_t)&first_page_table);

    print_string("Page Dir Phys: 0x");
    print_hex(kernel_directory->physicalAddr);
    print_string("\n");

    // ফিক্স 4: 0-4MB Identity Map - Kernel + Stack + Heap + VGA সব কভার
    for (uint32_t i = 0; i < 0x400000; i += 0x1000) {
        uint32_t page_idx = i / 0x1000;
        first_page_table.pages[page_idx].present = 1;
        first_page_table.pages[page_idx].rw = 1;
        first_page_table.pages[page_idx].user = 0;
        first_page_table.pages[page_idx].frame = page_idx;
        set_frame(i);
    }

    // VGA Text Buffer 0xB8000 - অলরেডি উপরের লুপে কভার, তাও সেফটি
    first_page_table.pages[0xB8].present = 1;
    first_page_table.pages[0xB8].rw = 1;
    first_page_table.pages[0xB8].user = 0;
    first_page_table.pages[0xB8].frame = 0xB8;

    isr_install_handler(14, page_fault);

    print_string("Loading CR3...\n");
    current_directory = kernel_directory;
    asm volatile("mov %0, %%cr3" :: "r"(kernel_directory->physicalAddr) : "memory");

    print_string("Enabling Paging...\n");
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");

    print_string("Paging Enabled\n");
}