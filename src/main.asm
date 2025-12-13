org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

start:
    jmp main


; Prints a string to the screen
; Params:
;   - ds:si points to string
puts:
    push si
    push ax

.loop:
    lodsb ; loads next character in al and increments si
    or al, al ; verify if next character is null
    jz .done ; jump if zero

    ; call bios interrupt that draws character from al
    mov ah, 0x0e
    mov bh, 0 ; page number
    int 0x10 ; interrrupt (not integer)

    jmp .loop

.done:
    pop ax
    pop si
    ret


main:
    ; setup data segments
    mov ax, 0 ; can't write to ds/es directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00 ; stack grows downwards, we do not want it to override our code

    ; print message
    mov si, msg_hello_world
    call puts

    hlt

.halt:
    jmp .halt

msg_hello_world: db 'Hello world!', ENDL, 0 ; 0 = null terminator

times 510-($-$$) db 0
dw 0AA55h