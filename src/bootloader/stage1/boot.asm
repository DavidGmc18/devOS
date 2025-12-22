org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

start:
    ; 1. FORCE CS:IP to 0x0000:0x7C00
    ; Some BIOSes load to 0x07C0:0x0000. This fixes that.
    jmp 0x0000:entry

entry:
    ; 2. Setup Segments (DS, ES, SS) to 0
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; 3. Save Boot Drive (Passed by BIOS in DL)
    mov [boot_drive], dl

    ; 4. Feedback
    mov si, msg_load
    call puts

    ; 5. SETUP DISK PACKET (DAP)
    ; We write directly to the structure to be safe
    mov si, dap
    mov word [si], 0x0010   ; Size of packet (16 bytes)
    mov word [si+2], 1      ; Number of sectors to read
    mov word [si+4], 0x7E00 ; Offset (Buffer)
    mov word [si+6], 0x0000 ; Segment (Buffer)
    mov dword [si+8], 1     ; LBA Low (Sector 1)
    mov dword [si+12], 0    ; LBA High

    ; 6. READ DISK (INT 13h, AH=42h)
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc disk_error           ; Jump if Carry Flag is set

    ; 7. JUMP TO KERNEL
    ; Far jump to ensuring we are at 0000:7E00
    mov si, msg_succ
    call puts
    
    jmp 0x0000:0x7E00

; --- ERROR HANDLING ---
disk_error:
    mov si, msg_err
    call puts
    cli
    hlt

; --- HELPERS ---
puts:
    push ax
    push si
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    pop si
    pop ax
    ret

; --- DATA ---
boot_drive: db 0
dap: times 16 db 0

msg_load: db 'Loading...', ENDL, 0
msg_succ: db 'Jump!', ENDL, 0
msg_err:  db 'Disk Error!', ENDL, 0

times 510-($-$$) db 0
dw 0xAA55



; %include "../bpb.inc"
; org 0x7C00
; bits 16

; %define ENDL 0x0D, 0x0A

; jmp short start
; nop

; times BDB_SIZE + EBR_SIZE db 0


; dap:                    
;     db 16        ; size of DAP structure in bytes
;     db 0         ; reserved
;     dw 0         ; number of sectors to read (fill before INT 13h)
;     dw 0         ; offset (BX) to load
;     dw 0         ; segment (ES) to load
;     dd 0         ; starting LBA (low DWORD)
;     dd 0         ; starting LBA (high DWORD)


; load_boot_sectors: dw 1 ; number of reserved boot sectors, that needs to be loaded (we don't need to load first one as it it handled by BIOS)


; start:
;     xor ax, ax
;     mov ds, ax
;     mov es, ax
;     mov ss, ax
;     mov sp, 0x7C00

;     mov si, msg_loading
;     call puts

;     mov [EBR_DRIVE_NUMBER], dl

;     mov ax, 1
;     mov cx, [load_boot_sectors]
;     mov bx, 0x7E00
;     call disk_read
;     jmp 0x7E00


; ; Prints a string to the screen
; ; Params:
; ;   - ds:si points to string
; puts:
;     push si
;     push ax
;     push bx

; .loop:
;     lodsb               ; loads next character in al
;     or al, al           ; verify if next character is null?
;     jz .done

;     mov ah, 0x0E        ; call bios interrupt
;     mov bh, 0           ; set page number to 0
;     int 0x10

;     jmp .loop

; .done:
;     pop bx
;     pop ax
;     pop si    
;     ret


; ; Read from disk
; ; Params:
; ;   - ax: LBA
; ;   - cx: num sectors to read 
; ;   - bx:es: memory address where to store read data
; disk_read:
;     pusha

;     mov word [dap + 2], cx       ; sectors to read
;     mov word [dap + 4], bx       ; offset
;     mov word [dap + 8], ax       ; LBA low 16 bits

;     mov ah, 42h
;     mov si, dap
;     mov dl, [EBR_DRIVE_NUMBER]
;     mov di, 3                    ; retry count

; .retry:
;     stc
;     int 13h
;     jnc .done

;     call disk_reset
;     dec di
;     test di, di
;     jnz .retry
;     jmp disk_error

; .done:
;     popa
;     ret


; ; Resets disk controller
; disk_reset:
;     pusha
;     mov dl, [EBR_DRIVE_NUMBER]
;     mov ah, 0
;     stc
;     int 13h
;     jc disk_error
;     popa
;     ret


; disk_error:
;     mov si, msg_read_failed 
;     call puts
;     jmp halt



; halt:
;     cli
;     hlt
;     jmp halt


; msg_loading:        db 'Loading...', ENDL, 0
; msg_read_failed:        db 'Read from disk failed!', ENDL, 0


; times 510-($-$$) db 0
; dw 0xAA55