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
#include "scheduler.h" // ADDED 1: হেডার অ্যাড
#include "vfs.h"       // ADDED: VFS এর জন্য

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
    paging_init(); // <-- COMMENT HATANO + AGE ANA
    print_string ("Paging Enabled\n"); // Debug

    // ========== FIX 2: TARPOR VGA MODE 13H ==========
    vga_init_mode_13h(); // <-- EKHON 0xA0000 MAP KORA ACHE
    vga_clear_gfx();
    vga_print_string("GFX OK\n", VGA_COLOR_GREEN);
    
    // ADDED: VFS INIT করো
    init_vfs(); // RAM File System চালু
    
    // ADDED 2: SCHEDULER INIT + 4টা টাস্ক বানাও
    init_scheduler();              // শিডিউলার চালু
    create_task(task1, "task1");   // টাস্ক 1
    create_task(task2, "task2");   // টাস্ক 2  
    create_task(task3, "task3");   // টাস্ক 3
    create_task(task4, "task4");   // টাস্ক 4
    
    vga_print_string("SOUMYAJIT OS v2.2 Ready\n", VGA_COLOR_WHITE);
    vga_print_string("SOC> ", VGA_COLOR_LIGHT_CYAN);

    __asm__ __volatile__("sti");
    command_loop();

    for(;;) {
        __asm__ __volatile__("hlt");
    }
}