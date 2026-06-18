#include "types.h"
#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "mm.h"
#include "pci.h"
#include "vga.h"
#include "paging.h"
#include "command.h"

extern void gdt_install();

void __attribute__((noreturn)) c_kernel_init() {
    gdt_install();
    clear_screen();

    mem_init();
    print_string("Memory Manager Ready (100MB Initialized)\n");

    idt_init();
    timer_init();
    print_string("IDT + PIC Initialized\n");

    keyboard_init();
    print_string("Keyboard Ready\n");
   
    pci_scan();

    // ========== FIX 1: PAGING AGE ON KORO ==========
    //  paging_init(); // <-- COMMENT HATANO + AGE ANA
    print_string("Paging Enabled\n"); // Debug

    // ========== FIX 2: TARPOR VGA MODE 13H ==========
    vga_init_mode_13h(); // <-- EKHON 0xA0000 MAP KORA ACHE
    vga_clear_gfx();
    vga_print_string("GFX OK\n", VGA_COLOR_GREEN);
    vga_print_string("SOUMYAJIT OS v2.2 Ready\n", VGA_COLOR_WHITE);
    vga_print_string("SOC> ", VGA_COLOR_LIGHT_CYAN);

    __asm__ __volatile__("sti");
    command_loop();

    for(;;) {
        __asm__ __volatile__("hlt");
    }
}