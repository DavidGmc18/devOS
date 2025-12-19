; src/kernel/main.asm
bits 16

%define ENDL 0x0D, 0x0A

; NO org directive needed (or org 0x0000 is also fine, but not org 0x0 with a value)

start:
    mov si, msg_hello
    call puts

    cli
    hlt
    jmp $-2     ; infinite loop just in case

puts:
    push ax
    push bx
    push si

.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop

.done:
    pop si
    pop bx
    pop ax
    ret

msg_hello: db 'Hello world from KERNEL!', ENDL, 0

; Pad kernel if needed, but not required
times 1024-($-$$) db 0   ; optional: make it at least 1KB for testing