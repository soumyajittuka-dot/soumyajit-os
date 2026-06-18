#include "command.h"
#include "vga.h"
#include "keyboard.h"
#include "e1000.h"
#include "ports.h"
#include "mm.h" // <-- kmalloc এর জন্য
#include "types.h"

extern volatile uint32_t timer_ticks; // timer.c থেকে
extern void print_hex(uint32_t n); // screen.c থেকে
extern void print_int(int n); // screen.c থেকে

char input_buffer[256];
int input_pos = 0;

int string_equal(char* s1, char* s2) {
    int i = 0;
    while(s1[i] && s2[i]) {
        if(s1[i]!= s2[i]) return 0;
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
        if(c1!= c2) return 0;
        i++;
    }
    return s1[i] == s2[i];
}

void cmd_memtest() {
    vga_print_string("Testing kmalloc/kfree...\n", VGA_COLOR_CYAN);

    void* p1 = kmalloc(4096); // 1 ব্লক
    void* p2 = kmalloc(8192); // 2 ব্লক
    void* p3 = kmalloc(100); // 1 ব্লক

    vga_print_string("p1: 0x", VGA_COLOR_WHITE); print_hex((uint32_t)p1); vga_print_string("\n", VGA_COLOR_WHITE);
    vga_print_string("p2: 0x", VGA_COLOR_WHITE); print_hex((uint32_t)p2); vga_print_string("\n", VGA_COLOR_WHITE);
    vga_print_string("p3: 0x", VGA_COLOR_WHITE); print_hex((uint32_t)p3); vga_print_string("\n", VGA_COLOR_WHITE);

    mem_stats(); // আগের স্টেট

    kfree(p2);
    vga_print_string("Freed p2\n", VGA_COLOR_YELLOW);

    mem_stats(); // ফ্রি করার পর

    void* p4 = kmalloc(4096); // p2 এর জায়গায় আসা উচিত
    vga_print_string("p4: 0x", VGA_COLOR_WHITE); print_hex((uint32_t)p4); vga_print_string("\n", VGA_COLOR_WHITE);

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
        vga_print_string(" ping - Test e1000 network\n", VGA_COLOR_WHITE);
        vga_print_string(" about - OS version info\n", VGA_COLOR_WHITE);
        vga_print_string(" fault - Trigger page fault\n", VGA_COLOR_WHITE);
        vga_print_string(" div0 - Trigger divide by zero\n", VGA_COLOR_WHITE);
        vga_print_string(" meminfo - Show CR0, CR3, Paging\n", VGA_COLOR_WHITE);
        vga_print_string(" memtest - Test kmalloc/kfree\n", VGA_COLOR_CYAN); // <-- NEW
        vga_print_string(" memstats- Show heap usage\n", VGA_COLOR_CYAN); // <-- NEW
        vga_print_string(" uptime - Show ticks since boot\n", VGA_COLOR_WHITE);
        vga_print_string(" reboot - Reboot the system\n", VGA_COLOR_WHITE);
        vga_print_string(" color - Test all VGA colors\n", VGA_COLOR_WHITE);
    }
    else if(string_equal_ignore_case(cmd, "ping")) {
        vga_print_string("PING: Sending packet to 8.8.8.8...\n", VGA_COLOR_CYAN);
        unsigned char packet[64];
        for(int i = 0; i < 64; i++) packet[i] = i;
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
        vga_print_string("Features: IDT, Paging, GFX, e1000, Heap\n", VGA_COLOR_GREEN); // Heap অ্যাড করলাম
        vga_print_string("Built: 7:48 AM with Intel Manuals\n", VGA_COLOR_GREEN);
    }
    else if(string_equal(cmd, "meminfo")) {
        uint32_t cr0, cr3;
        __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
        __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
        vga_print_string("Memory Info:\n", VGA_COLOR_YELLOW);
        vga_print_string(" CR0: ", VGA_COLOR_WHITE); print_hex(cr0);
        if(cr0 & 0x80000000) vga_print_string(" Paging ON\n", VGA_COLOR_GREEN);
        else vga_print_string(" Paging OFF\n", VGA_COLOR_RED);
        vga_print_string(" CR3: ", VGA_COLOR_WHITE); print_hex(cr3);
        vga_print_string(" Page Directory\n", VGA_COLOR_WHITE);
    }
    else if(string_equal(cmd, "memtest")) { // <-- NEW COMMAND
        cmd_memtest();
    }
    else if(string_equal(cmd, "memstats")) { // <-- NEW COMMAND
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