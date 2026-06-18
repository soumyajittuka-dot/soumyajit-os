[BITS 32]

; ==========================================
; Multiboot Header - FIXED FLAGS
; ==========================================
section.multiboot
align 4
    MAGIC equ 0x1BADB002
    FLAGS equ 1 | 2 ; ALIGN + MEMINFO - MUST HAVE
    CHECKSUM equ -(MAGIC + FLAGS)

    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section.text
global _start ; _start NOT start
global gdt_flush
global load_idt
global load_page_directory
global enable_paging
global switch_task

extern c_kernel_init ; <-- FIX: c_kernel_main -> c_kernel_init
extern isr_handler
extern irq_handler

; ==========================================
; Kernel Entry - FIXED LABEL
; ==========================================
_start: ; _start NOT start
    cli
    mov esp, stack_top
    call c_kernel_init ; <-- FIX: c_kernel_main -> c_kernel_init
    jmp $

; ==========================================
; GDT Flush
; ==========================================
gdt_flush:
    extern gdt_ptr
    lgdt [gdt_ptr]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2
flush2:
    ret

; ==========================================
; Load IDT
; ==========================================
load_idt:
    extern idt_ptr
    lidt [idt_ptr]
    ret

; ==========================================
; Paging Functions
; ==========================================
load_page_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

enable_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

; ==========================================
; ISR Macros
; ==========================================
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push dword %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
global irq%1
irq%1:
    cli
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

; CPU Exceptions 0-31
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE 30
ISR_NOERRCODE 31

; Hardware IRQs 0-15 = Int 32-47
IRQ 0, 32
IRQ 1, 33 ; Keyboard
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; ==========================================
; ISR Common Stub
; ==========================================
isr_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call isr_handler
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

; ==========================================
; IRQ Common Stub
; ==========================================
irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call irq_handler
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

; ==========================================
; Context Switch
; ==========================================
switch_task:
    mov eax, [esp + 4]
    mov edx, [esp + 8]
    push ebx
    push esi
    push edi
    push ebp
    mov [eax], esp
    mov esp, edx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

; ==========================================
; Stack 16KB
; ==========================================
section.bss
align 16
stack_bottom:
    resb 16384
stack_top: