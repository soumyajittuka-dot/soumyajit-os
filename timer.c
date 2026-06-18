#include "timer.h"
#include "ports.h"

volatile uint32_t timer_ticks = 0; // <-- EITA ADD KOR TOP E

void timer_handler(registers_t *r) {
    (void)r;
    timer_ticks++; // Prottek interrupt e 1 kore barbe
    // outb(0x20, 0x20); // EOI lagbe na, irq_handler pathay
}

void timer_init() {
    irq_install_handler(0, timer_handler); // IRQ0 = Timer
}