#include "mm.h"
#include "types.h"
#include "vga.h" // <-- VGA অ্যাড করলাম
#include "screen.h"




#define MEM_START 0x200000 // 2MB থেকে হিপ শুরু
#define MEM_SIZE 0x6400000 // 100MB
#define BLOCK_SIZE 4096 // 4KB blocks
#define BLOCK_COUNT 25600 // 100MB / 4KB

// Placement allocator - পেজিং স্ট্রাকচারের জন্য
uint32_t placement_address = 0x200000;

// Bitmap: 0=Free, 1=Used End, 2=Used Continues
unsigned char mem_map[BLOCK_COUNT];

void mem_init() {
    for(int i = 0; i < BLOCK_COUNT; i++) {
        mem_map[i] = 0;
    }
    vga_print_string("Heap: 100MB @ 0x200000 [4KB Blocks]\n", VGA_COLOR_CYAN);
}

void* kmalloc(uint32_t size) {
    if(size == 0) return 0;

    int blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    for(int i = 0; i < BLOCK_COUNT; i++) {
        int free_found = 1;

        for(int j = 0; j < blocks_needed; j++) {
            if(i + j >= BLOCK_COUNT || mem_map[i + j]!= 0) {
                free_found = 0;
                i = i + j;
                break;
            }
        }

        if(free_found) {
            for(int j = 0; j < blocks_needed; j++) {
                mem_map[i + j] = (j == blocks_needed - 1)? 1 : 2;
            }
            return (void*)(MEM_START + i * BLOCK_SIZE);
        }
    }
    vga_print_string("kmalloc: OOM!\n", VGA_COLOR_RED);
    return 0;
}

void kfree(void* ptr) {
    if(ptr == 0) return;

    uint32_t addr = (uint32_t)ptr;
    if(addr < MEM_START || addr >= MEM_START + MEM_SIZE) return;

    uint32_t block_index = (addr - MEM_START) / BLOCK_SIZE;

    while(block_index < BLOCK_COUNT && mem_map[block_index]!= 0) {
        unsigned char status = mem_map[block_index];
        mem_map[block_index] = 0;
        if(status == 1) break;
        block_index++;
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

// ==== এই ফাংশনটাই মেইন ফিক্স ====
void mem_stats() {
    int free_kb = get_free_mem();
    int used_kb = get_used_mem();
    int total_kb = (BLOCK_COUNT * BLOCK_SIZE) / 1024;

    vga_print_string("\n=== Memory Stats ===\n", VGA_COLOR_YELLOW);
    vga_print_string("Total: ", VGA_COLOR_WHITE); print_int(total_kb / 1024); vga_print_string(" MB\n", VGA_COLOR_WHITE);
    vga_print_string("Used : ", VGA_COLOR_WHITE); print_int(used_kb / 1024); vga_print_string(" MB (", VGA_COLOR_WHITE); print_int(used_kb); vga_print_string(" KB)\n", VGA_COLOR_WHITE);
    vga_print_string("Free : ", VGA_COLOR_WHITE); print_int(free_kb / 1024); vga_print_string(" MB (", VGA_COLOR_WHITE); print_int(free_kb); vga_print_string(" KB)\n", VGA_COLOR_WHITE);
}

// ==========================================
// Paging Support - placement allocator
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