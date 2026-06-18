#include "screen.h"
//#include "io.h"

static int cursor_x = 0;
static int cursor_y = 0;
static char* video_memory = (char*)0xB8000;

void clear_screen() {
    for(int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void print_char(char c) {
    if(c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if(c == '\b') {
        if(cursor_x > 0) {
            cursor_x--;
            int offset = (cursor_y * 80 + cursor_x) * 2;
            video_memory[offset] = ' ';
            video_memory[offset + 1] = 0x07;
        }
    } else {
        int offset = (cursor_y * 80 + cursor_x) * 2;
        video_memory[offset] = c;
        video_memory[offset + 1] = 0x07;
        cursor_x++;
        if(cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    if(cursor_y >= 25) clear_screen();
}

void print_string(const char* str) {
    int i = 0;
    while(str[i]!= '\0') {
        print_char(str[i]);
        i++;
    }
}

void print_int(int n) {
    if(n == 0) { print_char('0'); return; }
    if(n < 0) { print_char('-'); n = -n; }
    char buffer[12]; int i = 0;
    while(n > 0) { buffer[i++] = '0' + (n % 10); n /= 10; }
    while(i > 0) print_char(buffer[--i]);
}
void print_hex(uint32_t n) {
    print_string("0x");
    char hex_chars[] = "0123456789ABCDEF";
    for(int i = 28; i >= 0; i -= 4) {
        print_char(hex_chars[(n >> i) & 0xF]);
    }
}