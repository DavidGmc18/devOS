bits 32

extern main
global entry

section .entry
entry:
    jmp short start
    nop


section .start
start:

    pop eax ; (return adress)
    pop ebx ; ATA_drive_t boot_drive
    pop ecx ; uint8_t boot_partition
    pop edx ; MemoryInfo* mem_info

    mov esp, 0x200000

    push edx
    push ecx
    push ebx
    call main

    jmp start