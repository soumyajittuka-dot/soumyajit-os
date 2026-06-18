#ifndef TIMER_H
#define TIMER_H

#include "types.h"
#include "idt.h"

extern volatile uint32_t timer_ticks; // EITA LAGBE

void timer_init();
void timer_handler(registers_t *r);

#endif