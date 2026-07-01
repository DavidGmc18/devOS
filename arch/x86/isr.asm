[bits 64]

extern __isr_dispatch

__isr_handler:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, rsp ; Pass the stack pointer to C (struct regs*)
    call __isr_dispatch

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    add rsp, 16 ; Remove vector_id & error_code
    iretq

%macro ISR 1
    __isr_%1:
        push qword 0 ; Error code
        push qword %1 ; Vector ID
        jmp __isr_handler
%endmacro

%macro ISR_ERR 1
    __isr_%1:
        push qword %1 ; Vector ID
        jmp __isr_handler
%endmacro

ISR 0 ; Divide Error 
ISR 1 ; Debug Exception 
ISR 2 ; NMI Interrupt 
ISR 3 ; Breakpoint
ISR 4 ; Overflow
ISR 5 ; BOUND Range Exceeded
ISR 6 ; Invalid Opcode
ISR 7 ; Device Not Available
ISR_ERR 8 ; Double Fault 
ISR 9 ; Coprocessor Segment Overrun
ISR_ERR 10 ; Invalid TSS 
ISR_ERR 11 ; Segment Not Present
ISR_ERR 12 ; Stack-Segment Fault
ISR_ERR 13 ; General Protection 
ISR_ERR 14 ; Page Fault 
ISR 15 ; Reserved
ISR 16 ; x87 FPU Error
ISR_ERR 17 ; Alignment Check
ISR 18 ; Machine Check 
ISR 19 ; SIMD Floating-Point Exception
ISR 20 ; Virtualization Exception 
ISR_ERR 21 ; Control Protection Exception 
ISR 22
ISR 23
ISR 24
ISR 25
ISR 26
ISR 27
ISR 28
ISR_ERR 29
ISR_ERR 30
ISR 31

%assign i 32
%rep 224
    ISR i
    %assign i i+1
%endrep

section .data
align 8
global isr_table
isr_table:
%assign i 0
%rep 256
    dq __isr_%+i
    %assign i i+1
%endrep