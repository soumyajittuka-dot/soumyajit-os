#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "idt.h"

extern volatile int key_ready;
extern volatile char last_key;

// Functions
void keyboard_init(); // <-- NAME CHANGE KORO: init_keyboard -> keyboard_init

void keyboard_irq_handler(registers_t *r);

char keyboard_getchar();

#endif