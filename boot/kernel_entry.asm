[bits 32]
global kernel64_entry

CR4_PAE_ENABLE equ 1 << 5

EFER_MSR equ 0xC0000080
EFER_LM_ENABLE equ 1 << 8

CR0_PM_ENABLE equ 1 << 0
CR0_PG_ENABLE equ 1 << 31

section .text
kernel64_entry:
    mov eax, [esp+4]
    mov [pml4_addr], eax

    mov eax, [esp+8]
    mov [kernel_addr], eax

    mov eax, [esp+12]
    mov [kernel_addr+4], eax

;   Paging
    mov eax, [pml4_addr]
    mov cr3, eax

    mov eax, cr4
    or eax, CR4_PAE_ENABLE
    mov cr4, eax

;   LME bit in EFER
    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_LM_ENABLE  ; Set LME (Long Mode Enable, bit 8)
    wrmsr

;   Enable Paging
    mov eax, cr0
    or eax, CR0_PG_ENABLE | CR0_PM_ENABLE   ; ensuring that PM is set will allow for jumping
    mov cr0, eax

;   GDT & jmp to long-mode
    lgdt [gdt_ptr]
    jmp 0x08:long_mode
    hlt

[bits 64]
long_mode:
    mov ax, 0x10    ; data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov rsp, 0x200000 ; TODO organize stack

    jmp [kernel_addr]
    hlt


section .data
pml4_addr: dd 0
kernel_addr: dq 0

GDT_USER_SEG equ 1<<44
GDT_PRESENT equ 1<<47
GDT_WRITEABLE equ 1<<41
GDT_CODE_SEG equ 1<<43
GDT_64BIT equ  1<<53

align 8
gdt:
    dq 0            ; null descriptor
.code: equ $ - gdt  ; offset 0x08
    dq GDT_USER_SEG|GDT_PRESENT|GDT_WRITEABLE|GDT_CODE_SEG|GDT_64BIT
.data: equ $ - gdt                  ; offset 0x10
    dq GDT_USER_SEG|GDT_PRESENT|GDT_WRITEABLE

gdt_ptr:
    dw $ - gdt - 1  ; limit
    dd gdt, 0       ; base (64-bit ptr, but fine to define here)                
