#include "pci.h"
#include "ports.h"
#include "vga.h"
#include "e1000.h"

static unsigned int e1000_bar0_addr = 0;

unsigned int pci_config_read_dword(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned int address = (unsigned int)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

unsigned short pci_config_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned int tmp = pci_config_read_dword(bus, slot, func, offset);
    return (unsigned short)((tmp >> ((offset & 2) * 8)) & 0xFFFF);
}

void print_hex_word(unsigned short num) {
    char hex[] = "0123456789ABCDEF";
    char str[5];
    str[0] = hex[(num >> 12) & 0xF];
    str[1] = hex[(num >> 8) & 0xF];
    str[2] = hex[(num >> 4) & 0xF];
    str[3] = hex[num & 0xF];
    str[4] = '\0';
    vga_print_string(str, VGA_COLOR_WHITE);
}

void print_hex_byte(unsigned char num) {
    char hex[] = "0123456789ABCDEF";
    char str[3];
    str[0] = hex[(num >> 4) & 0xF];
    str[1] = hex[num & 0xF];
    str[2] = '\0';
    vga_print_string(str, VGA_COLOR_CYAN);
}

void print_hex_dword(unsigned int num) {
    char hex[] = "0123456789ABCDEF";
    char str[9];
    for(int i = 0; i < 8; i++) {
        str[i] = hex[(num >> (28 - i*4)) & 0xF];
    }
    str[8] = '\0';
    vga_print_string(str, VGA_COLOR_YELLOW);
}

unsigned int pci_get_bar0(unsigned char bus, unsigned char slot, unsigned char func) {
    return pci_config_read_dword(bus, slot, func, 0x10) & 0xFFFFFFF0;
}

unsigned int pci_get_e1000_bar0(void) {
    return e1000_bar0_addr;
}

void e1000_read_mac(unsigned int bar0) {
    volatile unsigned int* ptr = (volatile unsigned int*)bar0;
    unsigned int ral = ptr[0x5400 / 4];
    unsigned int rah = ptr[0x5404 / 4] & 0xFFFF;

    vga_print_string("MAC Address: ", VGA_COLOR_CYAN);
    unsigned char mac[6];
    mac[0] = ral & 0xFF;
    mac[1] = (ral >> 8) & 0xFF;
    mac[2] = (ral >> 16) & 0xFF;
    mac[3] = (ral >> 24) & 0xFF;
    mac[4] = rah & 0xFF;
    mac[5] = (rah >> 8) & 0xFF;

    for(int i = 0; i < 6; i++) {
        print_hex_byte(mac[i]);
        if(i < 5) vga_print_string(":", VGA_COLOR_CYAN);
    }
    vga_print_string("\n", VGA_COLOR_WHITE);
}

void pci_scan() {
    vga_print_string("Scanning PCI Bus...\n", VGA_COLOR_YELLOW);
    int found = 0;
    for(int bus = 0; bus < 4; bus++) {
        for(int slot = 0; slot < 32; slot++) {
            unsigned short vendor = pci_config_read_word(bus, slot, 0, 0);
            if(vendor!= 0xFFFF) {
                unsigned short device = pci_config_read_word(bus, slot, 0, 2);
                vga_print_string("PCI ", VGA_COLOR_WHITE);
                print_hex_word(bus);
                vga_print_string(":", VGA_COLOR_WHITE);
                print_hex_word(slot);
                vga_print_string(" = ", VGA_COLOR_WHITE);
                print_hex_word(vendor);
                vga_print_string(":", VGA_COLOR_WHITE);
                print_hex_word(device);

                if(vendor == 0x8086 && device == 0x100E) {
                    vga_print_string(" [Intel E1000 NET]", VGA_COLOR_GREEN);
                    found = 1;
                    e1000_bar0_addr = pci_get_bar0(bus, slot, 0);
                    vga_print_string("\nBAR0: ", VGA_COLOR_YELLOW);
                    print_hex_dword(e1000_bar0_addr);
                    vga_print_string("\n", VGA_COLOR_WHITE);
                    e1000_read_mac(e1000_bar0_addr);

                 //   e1000_init(e1000_bar0_addr); // CHANGE: UNCOMMENT KORLAM

                }
                vga_print_string("\n", VGA_COLOR_WHITE);
            }
        }
    }
    if(!found) vga_print_string("No E1000 found!\n", VGA_COLOR_RED);
}