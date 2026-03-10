bits 32

extern start

section .entry
    jmp short setup_stack
    nop

section .text
setup_stack:
    pop eax ; discard return address
    pop eax ; BL_BootInfo*
    pop ebx ; BL_BootServices*

    mov esp, 0x08000
    xor ebp, ebp

    push ebx
    push eax
    call start

reboot:
    mov al, 0xFE
    out 0x64, al
    hlt
    jmp reboot