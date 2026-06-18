#include "ports.h"

uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a" (result) : "d" (port));
    return result;
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a" (result) : "d" (port));
    return result;
}

void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a" (val), "Nd" (port));
}

uint32_t inl(uint16_t port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a" (result) : "d" (port));
    return result;
}

void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a" (val), "Nd" (port));
}