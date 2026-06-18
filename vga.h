#ifndef VGA_H
#define VGA_H

#define VGA_ADDRESS 0xA0000
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

// Mode 13h default colors
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GRAY 7
#define VGA_COLOR_DARK_GRAY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW 14
#define VGA_COLOR_WHITE 15

// Global State Variables
extern int vga_is_graphics_mode;
extern int vga_cursor_x;
extern int vga_cursor_y;

// Core VGA Functions
void vga_init();
void vga_init_mode_13h();
void vga_set_text_mode();
void vga_clear_gfx();
void vga_demo();

// Drawing Functions
void vga_put_pixel(int x, int y, unsigned char color);
void vga_draw_rect(int x, int y, int w, int h, unsigned char color);

// Text and Typing Functions
void vga_draw_char(char c, int x, int y, unsigned char color);
void vga_putchar(char c, unsigned char color);
void vga_print_string(const volatile char* str, unsigned char color);

// Command Shell and Input Functions
void vga_handle_keyboard_input(char c);

// Utility & Execution Functions
int string_compare(const volatile char* s1, const volatile char* s2);

#endif