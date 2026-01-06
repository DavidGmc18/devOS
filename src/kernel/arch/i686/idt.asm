[bits 32]

; void __attribute__((cdecl)) i686_IDT_Load(IDT_Descriptor* descriptor);
global i686_IDT_Load
i686_IDT_Load:
; make new call frame
    push ebp        ; save old call frame
    mov ebp, esp    ; initialize new call frame

    ; load gdt
    mov eax, [ebp + 8]
    lidt [eax]

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret