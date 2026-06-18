#ifndef KERNEL_H
#define KERNEL_H

void gdt_install();
void mem_init();
void idt_init();
void init_keyboard();
void pci_scan();
void clear_screen();
void print_string(const char* str);
void print_char(char c);

#endif