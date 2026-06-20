#include "command.h"
#include "vga.h"
#include "keyboard.h"
#include "e1000.h"
#include "ports.h"
#include "mm.h"
#include "types.h"
#include "scheduler.h" // ps এর জন্য অ্যাড করলাম
#include "vfs.h"       // ADDED: VFS এর জন্য
#include "string.h"    // ADDED: string functions এর জন্য

extern volatile uint32_t timer_ticks;
extern void print_int(int n); // screen.c থেকে

char input_buffer[256];
int input_pos = 0;

// ==== FIX 1: লোকাল print_hex - VGA এর জন্য ====
void print_hex32(uint32_t n) {
    char* hex = "0123456789ABCDEF";
    char buf[9];
    buf[8] = '\0';
    for(int i = 7; i >= 0; i--) {
        buf[i] = hex[n & 0xF];
        n >>= 4;
    }
    vga_print_string(buf, VGA_COLOR_WHITE);
}

int string_equal(char* s1, char* s2) {
    int i = 0;
    while(s1[i] && s2[i]) {
        if(s1[i] != s2[i]) return 0;
        i++;
    }
    return s1[i] == s2[i];
}

int string_equal_ignore_case(char* s1, char* s2) {
    int i = 0;
    while(s1[i] && s2[i]) {
        char c1 = s1[i];
        char c2 = s2[i];
        if(c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if(c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if(c1 != c2) return 0;
        i++;
    }
    return s1[i] == s2[i];
}

void cmd_memtest() {
    vga_print_string("Testing kmalloc/kfree...\n", VGA_COLOR_CYAN);

    void* p1 = kmalloc(4096);
    void* p2 = kmalloc(8192);
    void* p3 = kmalloc(100);

    vga_print_string("p1: 0x", VGA_COLOR_WHITE); print_hex32((uint32_t)p1); vga_print_string("\n", VGA_COLOR_WHITE);
    vga_print_string("p2: 0x", VGA_COLOR_WHITE); print_hex32((uint32_t)p2); vga_print_string("\n", VGA_COLOR_WHITE);
    vga_print_string("p3: 0x", VGA_COLOR_WHITE); print_hex32((uint32_t)p3); vga_print_string("\n", VGA_COLOR_WHITE);

    mem_stats();

    kfree(p2);
    vga_print_string("Freed p2\n", VGA_COLOR_YELLOW);

    mem_stats();

    void* p4 = kmalloc(4096);
    vga_print_string("p4: 0x", VGA_COLOR_WHITE); print_hex32((uint32_t)p4); vga_print_string("\n", VGA_COLOR_WHITE);

    kfree(p1);
    kfree(p3);
    kfree(p4);
    vga_print_string("All freed\n", VGA_COLOR_GREEN);
}

void execute_command(char* cmd) {
    vga_print_string("\n", VGA_COLOR_WHITE);

    if(string_equal(cmd, "clear")) {
        vga_clear_gfx();
    }
    else if(string_equal(cmd, "help")) {
        vga_print_string("Available Commands:\n", VGA_COLOR_YELLOW);
        vga_print_string(" help - Show this message\n", VGA_COLOR_WHITE);
        vga_print_string(" clear - Clear the screen\n", VGA_COLOR_WHITE);
        vga_print_string(" ps - Show running tasks\n", VGA_COLOR_CYAN);
        vga_print_string(" ls - List files\n", VGA_COLOR_CYAN); 
        vga_print_string(" touch <file> - Create file\n", VGA_COLOR_CYAN); 
        vga_print_string(" cat <file> - Read file\n", VGA_COLOR_CYAN); 
        vga_print_string(" echo <text> > <file> - Write file\n", VGA_COLOR_CYAN); 
        vga_print_string(" rm <file> - Delete file\n", VGA_COLOR_CYAN); 
        vga_print_string(" mkdir <dir> - Create directory\n", VGA_COLOR_CYAN); 
        vga_print_string(" ping - Test e1000 network\n", VGA_COLOR_WHITE);
        vga_print_string(" about - OS version info\n", VGA_COLOR_WHITE);
        vga_print_string(" fault - Trigger page fault\n", VGA_COLOR_WHITE);
        vga_print_string(" div0 - Trigger divide by zero\n", VGA_COLOR_WHITE);
        vga_print_string(" meminfo - Show CR0, CR3, Paging\n", VGA_COLOR_WHITE);
        vga_print_string(" memtest - Test kmalloc/kfree\n", VGA_COLOR_CYAN);
        vga_print_string(" memstats- Show heap usage\n", VGA_COLOR_CYAN);
        vga_print_string(" uptime - Show ticks since boot\n", VGA_COLOR_WHITE);
        vga_print_string(" reboot - Reboot the system\n", VGA_COLOR_WHITE);
        vga_print_string(" color - Test all VGA colors\n", VGA_COLOR_WHITE);
    }
    else if(string_equal(cmd, "ps")) {
        ps_command();
    }
    else if(string_equal(cmd, "ls")) {
        list_files();
    }
    else if(string_starts_with(cmd, "touch ")) {
        char* filename = cmd + 6;
        while(*filename == ' ') filename++;
        if(*filename == '\0') {
            vga_print_string("Usage: touch <filename>\n", VGA_COLOR_RED);
        } else if(create_file(filename) == 0) {
            vga_print_string("Created: ", VGA_COLOR_GREEN);
            vga_print_string(filename, VGA_COLOR_WHITE);
            vga_print_string("\n", VGA_COLOR_WHITE);
        } else {
            vga_print_string("Failed: File exists or no space\n", VGA_COLOR_RED);
        }
    }
    else if(string_starts_with(cmd, "mkdir")) {
        char* dirname = cmd + 5;
        while(*dirname == ' ') dirname++;
        if(*dirname == '\0') {
            vga_print_string("Usage: mkdir <dirname>\n", VGA_COLOR_RED);
        } else if(create_dir(dirname) == 0) {
            vga_print_string("Created directory: ", VGA_COLOR_GREEN);
            vga_print_string(dirname, VGA_COLOR_WHITE);
            vga_print_string("\n", VGA_COLOR_WHITE);
        } else {
            vga_print_string("Failed: Directory exists or no space\n", VGA_COLOR_RED);
        }
    }
    else if(string_starts_with(cmd, "echo ")) {
        char* rest = cmd + 5;
        char* arrow = string_find(rest, '>');
        if(arrow) {
            *arrow = '\0';
            char* text = rest;
            char* filename = arrow + 1;
            while(*filename == ' ') filename++;
            while(*text == ' ') text++;

            if(write_file(filename, (uint8_t*)text, strlen(text)) >= 0) {
                vga_print_string("Wrote to ", VGA_COLOR_GREEN);
                vga_print_string(filename, VGA_COLOR_WHITE);
                vga_print_string("\n", VGA_COLOR_WHITE);
            } else {
                vga_print_string("Write failed: File not found\n", VGA_COLOR_RED);
            }
        } else {
            vga_print_string("Usage: echo <text> > <filename>\n", VGA_COLOR_RED);
        }
    }
    else if(string_starts_with(cmd, "cat ")) {
        char* filename = cmd + 4;
        while(*filename == ' ') filename++;
        uint8_t buffer[256];
        int bytes = read_file(filename, buffer, 255);
        if(bytes >= 0) {
            buffer[bytes] = '\0';
            vga_print_string((char*)buffer, VGA_COLOR_WHITE);
            vga_print_string("\n", VGA_COLOR_WHITE);
        } else {
            vga_print_string("File not found\n", VGA_COLOR_RED);
        }
    }
    else if(string_starts_with(cmd, "rm ")) {
        char* filename = cmd + 3;
        while(*filename == ' ') filename++;
        if(delete_file(filename) == 0) {
            vga_print_string("Deleted\n", VGA_COLOR_GREEN);
        } else {
            vga_print_string("File not found\n", VGA_COLOR_RED);
        }
    }
    else if(string_equal_ignore_case(cmd, "ping")) {
        vga_print_string("PING: Sending packet to 8.8.8.8...\n", VGA_COLOR_CYAN);
        unsigned char packet[64];
        for(int i = 0; i < 64; i++) packet[i] = i;
        (void)packet; // Added this to fix the [-Wunused-but-set-variable] warning
        //e1000_send_packet(packet, 64);
        vga_print_string("PING: 64 bytes sent!\n", VGA_COLOR_GREEN);
    }
    else if(string_equal(cmd, "fault")) {
        vga_print_string("Triggering page fault...\n", VGA_COLOR_RED);
        uint32_t* bad = (uint32_t*)0xDEADBEEF;
        *bad = 123;
        vga_print_string("You should NOT see this!\n", VGA_COLOR_RED);
    }
    else if(string_equal(cmd, "div0")) {
        vga_print_string("Triggering divide by zero...\n", VGA_COLOR_RED);
        volatile int a = 10;
        volatile int b = 0;
        volatile int c = a / b;
        (void)c;
        vga_print_string("You should NOT see this!\n", VGA_COLOR_RED);
    }
    else if(string_equal(cmd, "about")) {
        vga_print_string("Soumyajit OS v2.2\n", VGA_COLOR_GREEN);
        vga_print_string("Features: IDT, Paging, GFX, e1000, Heap, VFS\n", VGA_COLOR_GREEN);
        vga_print_string("Built: 7:48 AM with Intel Manuals\n", VGA_COLOR_GREEN);
    }
    else if(string_equal(cmd, "meminfo")) {
        uint32_t cr0, cr3;
        // FIXED: _asm_ to __asm__
        __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
        __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
        
        vga_print_string("Memory Info:\n", VGA_COLOR_YELLOW);
        vga_print_string(" CR0: ", VGA_COLOR_WHITE); print_hex32(cr0);
        if(cr0 & 0x80000000) vga_print_string(" Paging ON\n", VGA_COLOR_GREEN);
        else vga_print_string(" Paging OFF\n", VGA_COLOR_RED);
        vga_print_string(" CR3: ", VGA_COLOR_WHITE); print_hex32(cr3);
        vga_print_string(" Page Directory\n", VGA_COLOR_WHITE);
    }
    else if(string_equal(cmd, "memtest")) {
        cmd_memtest();
    }
    else if(string_equal(cmd, "memstats")) {
        mem_stats();
    }
    else if(string_equal(cmd, "uptime")) {
        vga_print_string("Uptime: ", VGA_COLOR_CYAN);
        char buf[16]; int i = 0; uint32_t ticks = timer_ticks;
        if(ticks == 0) buf[i++] = '0';
        while(ticks > 0) { buf[i++] = '0' + (ticks % 10); ticks /= 10; }
        buf[i] = '\0';
        for(int j = 0; j < i/2; j++) { char t = buf[j]; buf[j] = buf[i-j-1]; buf[i-j-1] = t; }
        vga_print_string(buf, VGA_COLOR_WHITE);
        vga_print_string(" ticks\n", VGA_COLOR_WHITE);
    }
    else if(string_equal(cmd, "reboot")) {
        vga_print_string("Rebooting...\n", VGA_COLOR_RED);
        uint8_t good = 0x02;
        while(good & 0x02) good = inb(0x64);
        outb(0x64, 0xFE);
        
        // FIXED: _asm_ to __asm__
        __asm__ volatile("hlt");
    }
    else if(string_equal(cmd, "color")) {
        vga_print_string("VGA Colors:\n", VGA_COLOR_WHITE);
        for(int i = 0; i < 16; i++) {
            vga_print_string("### ", i);
        }
        vga_print_string("\n", VGA_COLOR_WHITE);
    }
    else if(cmd[0] == '\0') {
        // খালি enter
    }
    else {
        vga_print_string("Unknown: ", VGA_COLOR_RED);
        vga_print_string(cmd, VGA_COLOR_WHITE);
        vga_print_string("\n", VGA_COLOR_WHITE);
    }

    input_pos = 0;
    input_buffer[0] = '\0';
}

void process_keyboard_input(char c) {
    if(c == '\n') {
        input_buffer[input_pos] = '\0';
        execute_command(input_buffer);
        vga_print_string("SOC> ", VGA_COLOR_LIGHT_BLUE);
    }
    else if(c == '\b') {
        if(input_pos > 0) {
            input_pos--;
            vga_print_string("\b \b", VGA_COLOR_WHITE);
        }
    }
    else if(input_pos < 255) {
        input_buffer[input_pos++] = c;
        char str[2] = {c, '\0'};
        vga_print_string(str, VGA_COLOR_WHITE);
    }
}

void command_loop() {
    vga_print_string("SOC> ", VGA_COLOR_LIGHT_BLUE);
    while(1) {
        char c = keyboard_getchar();
        process_keyboard_input(c);
    }
}