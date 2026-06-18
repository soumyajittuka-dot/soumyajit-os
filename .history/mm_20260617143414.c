#include "mm.h"
#include "types.h"
#include "screen.h"

extern void print_string(const char* str);
extern void print_int(int n);

#define MEM_START 0x200000 // 2MB er por theke heap shuru
#define MEM_SIZE 0x6400000 // 100MB
#define BLOCK_SIZE 4096 // 4KB blocks
#define BLOCK_COUNT 25600 // 100MB / 4KB

// Paging er jonno placement allocator - 1MB theke shuru
uint32_t placement_address = 0x200000;

// 0: Free, 1: Used End, 2: Used Continues
unsigned char mem_map[BLOCK_COUNT];

void mem_init() {
    for(int i = 0; i < BLOCK_COUNT; i++) {
        mem_map[i] = 0;
    }
    print_string("Memory: 100MB Initialized [4KB Blocks]\n");
}

void* kmalloc(uint32_t size) {
    if(size == 0) return 0;

    int blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    for(int i = 0; i < BLOCK_COUNT; i++) {
        int free_found = 1;

        // Check if continuous blocks are free
        for(int j = 0; j < blocks_needed; j++) {
            if(i + j >= BLOCK_COUNT || mem_map[i + j]!= 0) {
                free_found = 0;
                i = i + j;
                break;
            }
        }

        // Allocate the blocks
        if(free_found) {
            for(int j = 0; j < blocks_needed; j++) {
                if (j == blocks_needed - 1) {
                    mem_map[i + j] = 1; // end of allocation
                } else {
                    mem_map[i + j] = 2; // continues
                }
            }
            return (void*)(MEM_START + i * BLOCK_SIZE);
        }
    }
    return 0; // Out of memory
}

void kfree(void* ptr) {
    if(ptr == 0) return;

    uint32_t block_index = ((uint32_t)ptr - MEM_START) / BLOCK_SIZE;

    if(block_index < BLOCK_COUNT) {
        while(block_index < BLOCK_COUNT && mem_map[block_index]!= 0) {
            unsigned char status = mem_map[block_index];
            mem_map[block_index] = 0;
            block_index++;
            if(status == 1) break;
        }
    }
}

int get_free_mem() {
    int free_blocks = 0;
    for(int i = 0; i < BLOCK_COUNT; i++) {
        if(mem_map[i] == 0) free_blocks++;
    }
    return (free_blocks * BLOCK_SIZE) / 1024;
}

int get_used_mem() {
    int used_blocks = 0;
    for(int i = 0; i < BLOCK_COUNT; i++) {
        if(mem_map[i]!= 0) used_blocks++;
    }
    return (used_blocks * BLOCK_SIZE) / 1024;
}

void mem_stats() {
    int free_kb = get_free_mem();
    int used_kb = get_used_mem();
    int total_kb = (BLOCK_COUNT * BLOCK_SIZE) / 1024;

    print_string("\n=== Memory Stats ===\n");
    print_string("Total: "); print_int(total_kb / 1024); print_string(" MB\n");
    print_string("Used : "); print_int(used_kb / 1024); print_string(" MB ("); print_int(used_kb); print_string(" KB)\n");
    print_string("Free : "); print_int(free_kb / 1024); print_string(" MB ("); print_int(free_kb); print_string(" KB)\n");
    print_string("Block Size: "); print_int(BLOCK_SIZE / 1024); print_string(" KB\n");
}

// ==========================================
// PAGING SUPPORT FUNCTIONS - NEW
// ==========================================

uint32_t kmalloc_a(uint32_t sz) {
    if (placement_address & 0xFFF) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }
    uint32_t tmp = placement_address;
    placement_address += sz;
    return tmp;
}

uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys) {
    if (placement_address & 0xFFF) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }
    if (phys) *phys = placement_address;
    uint32_t tmp = placement_address;
    placement_address += sz;
    return tmp;
}
// void paging_init() {
   // print_string("Paging: Disabled for now\n");

// }