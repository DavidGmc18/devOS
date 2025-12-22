bits 16

section _TEXT class=CODE

global _x86_Video_WriteCharReletype
_x86_Video_WriteCharReletype:

    push bp         ; save old call fram
    mov bp, sp      ; initialize new call frame

    ; save bx
    push bx

    ; [bp + 0] - old call frame
    ; [bp + 2] - return adress (small memory model => 2 bytes)
    ; [bp + 4] - first argument (character)
    ; [bp + 6] - second argument (page)
    ; bytes are cinverted to words (you can't push a single byte on the stack)
    mov ah, 0x0E
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    ; restore bx
    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret
