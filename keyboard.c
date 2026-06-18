#include "keyboard.h"
#include "screen.h"
#include "ports.h"
#include "vga.h"
#include "idt.h"

volatile int key_ready = 0;
volatile char last_key = 0;
volatile int shift_pressed = 0;

static unsigned char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static unsigned char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|','Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

void keyboard_irq_handler(registers_t *r) {
    (void)r;
    unsigned char scancode = inb(0x60);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return; // EOI REMOVE - irq_handler pathabe
    }

    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return; // EOI REMOVE
    }

    if (scancode & 0x80) {
        return; // EOI REMOVE - key release ignore
    }

    if (scancode >= sizeof(scancode_to_ascii)) {
        return; // EOI REMOVE
    }

    char c = 0;
    if (shift_pressed) {
        c = scancode_to_ascii_shift[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }

    if (c == 0) {
        return; // EOI REMOVE
    }

    if (scancode == 0x01) { // ESC key
        vga_set_text_mode();
        return; // EOI REMOVE
    }

    last_key = c;
    key_ready = 1;
    // outb(0x20, 0x20); EITA DELETE - irq_handler already EOI pathay
}

void keyboard_init() {
    key_ready = 0;
    last_key = 0;
    shift_pressed = 0;
    irq_install_handler(1, keyboard_irq_handler);
    print_string("Keyboard Driver Registered to IRQ1\n");
}

char keyboard_getchar() {
    while(key_ready == 0) {
        __asm__ __volatile__("hlt");
    }
    key_ready = 0;
    return last_key;
}