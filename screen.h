#ifndef SCREEN_H
#define SCREEN_H
#include "ports.h"
#include "types.h"

void clear_screen();
void print_char(char c);
void print_string(const char* str);
void print_int(int num);
void print_hex(uint32_t num);
void set_color(char fg, char bg);

#endif