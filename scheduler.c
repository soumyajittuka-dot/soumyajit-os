#include "scheduler.h"
#include "mm.h"
//#include "io.h"
#include "screen.h"

//extern void print_string(const char* str);
//extern void print_char(char c);
//extern void print_int(int n);

#define MAX_TASKS 16

task_t tasks[MAX_TASKS];
int current_task = 0;
int num_tasks = 0;
int next_pid = 1;
char spinner[] = {'-', '\\', '|', '/'};
int spinner_idx = 0;

void task1() {
    while(1) {
        for(volatile int i = 0; i < 10000000; i++);
    }
}

void task2() {
    while(1) {
        for(volatile int i = 0; i < 10000000; i++);
    }
}

void init_scheduler() {
    num_tasks = 0;
    current_task = 0;
    for(int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = 0;
        tasks[i].esp = 0;
        tasks[i].eip = 0;
    }
}

void create_task(void (*entry_point)()) {
    if(num_tasks >= MAX_TASKS) return;
    task_t* t = &tasks[num_tasks];
    t->pid = next_pid++;
    t->state = 1;
    uint32_t* stack = (uint32_t*)kmalloc(4096);
    if(stack == 0) return;
    stack = (uint32_t*)((uint32_t)stack + 4096);
    *(--stack) = 0x202;
    *(--stack) = 0x08;
    *(--stack) = (uint32_t)entry_point;
    for(int i = 0; i < 8; i++) *(--stack) = 0;
    t->esp = (uint32_t)stack;
    t->eip = (uint32_t)entry_point;
    num_tasks++;
}

uint32_t*scheduler_timer_handler(uint32_t* esp) {
    tasks[current_task].esp = (uint32_t)esp;
    current_task++;
    if(current_task >= num_tasks) current_task = 0;

    // VGA TEXT MODE er code - Graphics mode e kaj korbe na
    // uint32_t pos = 160 * 0 + 156;
    // char* vidmem = (char*)0xB8000;
    // vidmem[pos] = spinner[spinner_idx];
    // vidmem[pos+1] = 0x0E;
    // vidmem[pos+2] = spinner[spinner_idx];
    // vidmem[pos+3] = 0x0C;
    spinner_idx = (spinner_idx + 1) % 4;

    return (uint32_t*)tasks[current_task].esp;
}

void ps_command() {
    print_string("\n=== Running Tasks ===\n");
    print_string("PID NAME STATUS\n");
    for(int i = 0; i < num_tasks; i++) {
        if(tasks[i].state == 1) {
            if(i == current_task) print_string("-> ");
            else print_string(" ");
            print_int(tasks[i].pid);
            print_string(" Task");
            print_int(i+1);
            print_string("-Spinner Running\n");
        }
    }
    print_string("=====================\n");
}