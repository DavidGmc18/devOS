org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

jmp short start
nop

bdb_oem:                    db '        ' ; 8 bytes
bdb_bytes_per_sector:       dw 0
bdb_sectors_per_cluster:    db 0
bdb_reserved_sectors:       dw 0
bdb_fat_count:              db 0
bdb_dir_entries_count:      dw 0
bdb_total_sectors:          dw 0
bdb_media_descriptor_type:  db 0
bdb_sectors_per_fat:        dw 0
bdb_sectors_per_track:      dw 0
bdb_heads:                  dw 0
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0

; extended boot record
ebr_drive_number:           db 0
ebr_reserved:               db 0
ebr_signature:              db 0
ebr_volume_id:              dd 0
ebr_volume_label:           db '           ' ; 11 bytes, padded with spaces
ebr_system_id:              db '        ' ; 8 bytes


start:
    mov ax, 0           ; can't set ds/es directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00

.after:
    mov [ebr_drive_number], dl

    ; read drive parameters (sectors per track and head count),
    ; instead of relying on data on formatted disk
    push es
    mov ah, 08h
    int 13h
    jc disk_error
    pop es

    and cl, 0x3F                        ; remove top 2 bits
    xor ch, ch
    mov [bdb_sectors_per_track], cx

    inc dh
    mov [bdb_heads], dh

    ; compute LBA of root directory = reserved + fats * sectors_per_fat
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx                              ; ax = (fats * sectors_per_fat)
    add ax, [bdb_reserved_sectors]      ; ax = LBA of root directory
    mov [root_dir_lba], ax

    ; compute size of root directory = (32 * number_of_entries) / bytes_per_sector
    mov ax, [bdb_dir_entries_count]
    shl ax, 5                           ; ax *= 32
    xor dx, dx                          ; dx = 0
    div word [bdb_bytes_per_sector]     ; number of sectors we need to read

    test dx, dx                         ; if dx != 0, add 1
    jz .root_dir_after
    inc ax

.root_dir_after:
    mov [root_dir_sectors], ax

    ; read root directory
    mov cx, ax                          ; cx = number of sectors to read = size of root directory
    mov ax, [root_dir_lba]                              ; ax = LBA of root directory
    mov bx, buffer                      ; es:bx = buffer
    call disk_read

    ; search for stage2.bin
    xor bx, bx
    mov di, buffer

.search_stage2:
    mov si, file_stage2_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found_kernel

    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_stage2

    jmp stage2_not_found_error

.found_kernel:
    mov ax, [di + 26]
    mov [stage2_cluster], ax

    mov ax, 18
    call print_word_hex
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;     mov ax, [bdb_reserved_sectors]
;     mov bx, buffer
;     mov cl, [bdb_sectors_per_fat]
;     call disk_read

;     mov bx, STAGE2_LOAD_SEGMENT
;     mov es, bx
;     mov bx, STAGE2_LOAD_OFFSET

; .load_stage2_loop:
;     ; Compute LBA
;     mov ax, [stage2_cluster]

;     sub ax, 2
;     mov cx, [bdb_sectors_per_cluster]
;     mul cx
;     add ax, [root_dir_lba]
;     add ax, [root_dir_sectors]
    
;     mov cx, [bdb_sectors_per_cluster]
;     call disk_read
    
;     mov ax, [bdb_bytes_per_sector]
;     mov cx, [bdb_sectors_per_cluster]
;     mul cx
;     add bx, ax

;     mov ax, [stage2_cluster]
;     mov cx, 2
;     mul cx

;     mov si, buffer
;     add si, ax
;     mov ax, [ds:si]

;     cmp ax, 0xFFF8
;     jae .read_finish

;     mov [stage2_cluster], ax
;     jmp .load_stage2_loop

; .read_finish:
;     mov dl, [ebr_drive_number]

;     mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
;     mov ds, ax
;     mov es, ax

;     jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

;     jmp wait_key_and_reboot

.halt:
    cli ; disable interrupts, this way CPU can't get out of "halt" state
    hlt


disk_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

stage2_not_found_error:
    mov si, msg_stage2_not_found
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h                     ; wait for keypress
    jmp 0FFFFh:0


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


; input: lower 4 bits in AL
; output: ASCII character in AL
nibble_to_hex:
    cmp al, 10
    jb .digit
    add al, 'A' - 10
    ret
.digit:
    add al, '0'
    ret


; input: AL = byte to print
; uses: AH, AL
print_byte_hex:
    push ax
    mov ah, al
    shr ah, 4           ; high nibble
    call nibble_to_hex
    mov ah, 0x0E
    int 10h             ; print high nibble
    pop ax
    and al, 0x0F        ; low nibble
    call nibble_to_hex
    mov ah, 0x0E
    int 10h             ; print low nibble
    ret


; input: AX = word to print
; prints high byte then low byte
print_word_hex:
    push ax
    mov al, ah
    call print_byte_hex
    pop ax
    mov al, al
    call print_byte_hex
    ret


; Read from disk
; Params:
;   - ax: LBA
;   - cx: num sectors to read 
;   - es:bx: memory address where to store read data
disk_read:
    pusha

    mov word [dap + 2], cx      ; sectors to read
    mov word [dap + 4], bx      ; offset
    mov word [dap + 6], es      ; segment    
    mov word [dap + 8], ax      ; LBA low 16 bits

    mov ah, 42h
    mov si, dap
    mov dl, [ebr_drive_number]
    mov di, 3                    ; retry count

.retry:
    stc
    int 13h

    cmp ah, 0
    jc .fail

    cmp ah, 0
    jne .fail

    ; cmp al, cl
    ; jne .fail

    jmp .done

.fail:
    call disk_reset
    dec di
    test di, di
    jnz .retry
    jmp disk_error

.done:
    popa
    ret


; Resets disk controller
disk_reset:
    pusha
    mov dl, [ebr_drive_number]
    mov ah, 0
    stc
    int 13h
    jc disk_error
    popa
    ret


dap:                    
    db 16        ; size of DAP structure in bytes
    db 0         ; reserved
    dw 0         ; number of sectors to read (fill before INT 13h)
    dw 0         ; offset (BX) to load
    dw 0         ; segment (ES) to load
    dd 0         ; starting LBA (low DWORD)
    dd 0         ; starting LBA (high DWORD)


msg_read_failed:        db 'Read from disk failed!', ENDL, 0
msg_stage2_not_found:   db 'STAGE2.BIN file not found!', ENDL, 0
file_stage2_bin:        db 'KERNEL  BIN'

; msg_true:               db 'T', 0
; msg_false:               db 'F', 0
    ; cmp ax, 1128
    ; je .true
    ; mov si, msg_false
    ; call puts
    ; jmp .end
    ; .true:
    ; mov si, msg_true
    ; call puts
    ; .end:
    ; mov ax, 0x3
    ; mov cx, 1

root_dir_lba:           dw 0
root_dir_sectors:       dw 0
stage2_cluster:         dw 0

STAGE2_LOAD_SEGMENT     equ 0x2000
STAGE2_LOAD_OFFSET      equ 0


times 510-($-$$) db 0
dw 0AA55h

buffer: