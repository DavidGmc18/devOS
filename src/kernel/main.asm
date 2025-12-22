%include "../bootloader/bpb.inc"
org 0x7E00
bits 16

msg_hello: db 'Hello world from kernel!', ENDL, 0

start:
    mov ax, cs
    mov ds, ax
    mov es, ax

    mov si, msg_hello
    call puts

.halt:
    cli
    hlt
    call .halt

;
; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si    
    ret