#include "paging.h"
#include "mm.h"
#include "vga.h"
#include "idt.h"
#include "screen.h"
#include "string.h"
#include <stddef.h> // offsetof er jonno

extern void load_page_directory(uint32_t dir);
extern void enable_paging();

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;
uint32_t *frames;
uint32_t nframes;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static uint32_t first_frame() {
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i]!= 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(frames[i] & (0x1 << j))) {
                    return i*32+j;
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
    set_frame(idx*0x1000);
    page->present = 1;
    page->rw = is_writeable;
    page->user =!is_kernel;
    page->frame = idx;
}

// Identity mapping er jonno. Hardware address map korte lagbe.
void alloc_frame_physical(page_t *page, int is_kernel, int is_writeable, uint32_t phys_addr) {
    if (page->frame!= 0) return;
    page->present = 1;
    page->rw = is_writeable;
    page->user =!is_kernel;
    page->frame = phys_addr / 0x1000; // Direct physical address
    set_frame(phys_addr);
}

// ===== BOSS EITAI MAIN FIX =====
// RECURSION BHOOT TARANO VERSION
page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
    address /= 0x1000;
    uint32_t table_idx = address / 1024;

    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[address % 1024];
    } else if(make) {
        uint32_t phys;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &phys);
        memset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = phys | 0x7; // Present + RW + User

        // CRITICAL FIX: Recursion nai. Direct map.
        if (dir == kernel_directory) {
            uint32_t pt_idx = phys / 0x1000 / 1024;
            uint32_t pg_idx = (phys / 0x1000) % 1024;

            // Jodi page table er page table banate hoy
            if (!dir->tables[pt_idx]) {
                uint32_t pt_phys;
                dir->tables[pt_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &pt_phys);
                memset(dir->tables[pt_idx], 0, 0x1000);
                dir->tablesPhysical[pt_idx] = pt_phys | 0x7;
            }

            // Notun page table ke nije ke map kor
            page_t *pt_page = &dir->tables[pt_idx]->pages[pg_idx];
            pt_page->present = 1;
            pt_page->rw = 1;
            pt_page->user = 0;
            pt_page->frame = phys / 0x1000;
            set_frame(phys);
        }
        return &dir->tables[table_idx]->pages[address % 1024];
    }
    return 0;
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

    uint32_t mem_end_page = 0x400000; // 4MB
    nframes = mem_end_page / 0x1000;
    frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes) * 4);
    memset(frames, 0, INDEX_FROM_BIT(nframes) * 4);

    uint32_t phys_dir;
    kernel_directory = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys_dir);
    memset(kernel_directory, 0, sizeof(page_directory_t));

    // Page Directory ke nijer physical address e map kor
    alloc_frame_physical(get_page(phys_dir, 1, kernel_directory), 1, 1, phys_dir);

    kernel_directory->physicalAddr = phys_dir + offsetof(page_directory_t, tablesPhysical);

    print_string("Page Dir Phys: ");
    print_hex(kernel_directory->physicalAddr);
    print_string("\n");

    // 0-4MB Identity map
    for (uint32_t i = 0; i < 0x400000; i += 0x1000) {
        alloc_frame_physical(get_page(i, 1, kernel_directory), 1, 1, i);
    }

    // VGA Text Mode 0xB8000 - Identity map MUST
    alloc_frame_physical(get_page(0xB8000, 1, kernel_directory), 1, 1, 0xB8000);

    // VGA Mode 13h 0xA0000 - 0xAFFFF - Identity map MUST
    for (uint32_t i = 0xA0000; i < 0xB0000; i += 0x1000) {
        alloc_frame_physical(get_page(i, 1, kernel_directory), 1, 1, i);
    }

    irq_install_handler(14, page_fault);

    print_string("Loading CR3...\n");
    current_directory = kernel_directory;
    load_page_directory(kernel_directory->physicalAddr);

    print_string("Enabling Paging...\n");
    enable_paging();

    print_string("Paging Init Done\n");
}