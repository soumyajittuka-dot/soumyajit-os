#ifndef SCHEDULER_H
#define SCHEDULER_H

//#include <stdint.h>
#include "types.h"

typedef struct {
    uint32_t esp;
    uint32_t eip;
    uint32_t pid;
    uint32_t state;
} task_t;

void init_scheduler();
void create_task(void (*entry_point)());
uint32_t* timer_handler(uint32_t* esp);
void ps_command();
void task1();
void task2();

#endif