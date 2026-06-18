#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "ports.h"

// ----- TYPEDEFS FIRST -----
typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// Handler function pointer types
typedef void (*isr_handler_t)(registers_t*);
typedef void (*irq_handler_t)(registers_t*);

// ----- CORE IDT FUNCTIONS -----
void idt_init();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void enable_interrupts();
void disable_interrupts();

// ----- HANDLERS -----
void isr_handler(registers_t *regs);
void irq_handler(registers_t *regs);

// ----- DYNAMIC IRQ -----
void irq_install_handler(int irq, irq_handler_t handler);
void irq_uninstall_handler(int irq);

// ----- DYNAMIC ISR (NEW) -----
void isr_install_handler(int isr, isr_handler_t handler);
void isr_uninstall_handler(int isr);

// ----- EXTERN ASM -----
extern void load_idt();

// ----- CPU EXCEPTIONS 0-31 -----
extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

// ----- HARDWARE IRQS 32-47 -----
extern void irq0(); extern void irq1(); extern void irq2(); extern void irq3();
extern void irq4(); extern void irq5(); extern void irq6(); extern void irq7();
extern void irq8(); extern void irq9(); extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

#endif