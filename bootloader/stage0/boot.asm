org 0x7C00
bits 16

%include "../BPB.inc"

%define ENDL 0x0D, 0x0A

jmp short start
nop

OEM_ID: db 'DAVIDAK '
BiosParameterBlock
ExtendedBiosParameterBlock
MetaDataBlock

start:
    ; setup data segments
    mov ax, 0           ; can't set ds/es directly
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00              ; stack grows downwards from where we are loaded in memory

    ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
    ; expected location
    push es
    push word .after
    retf

.after:
    ; read something from disk
    ; BIOS should set DL to drive number
    mov [EBPB_drive_number], dl

    ; read drive parameters (sectors per track and head count),
    ; instead of relying on data on formatted disk
    push es
    mov ah, 08h
    int 13h
    jc disk_error
    pop es

    and cl, 0x3F                        ; remove top 2 bits
    xor ch, ch
    mov [BPB_sectors_per_track], cx     ; sector count

    inc dh
    mov [BPB_head_count], dh                 ; head count

detect_disk_type:
    mov ah, 41h
    mov dl, [EBPB_drive_number]
    mov bx, 0x55AA
    int 13h

    cmp bx, 0xAA55
    jne .no_DAP

    and cx, 1
    cmp cx, 1
    jne .no_DAP

    ; DAP
    mov byte [MDB_disk_address_packet], 1
    jmp load_stage1

.no_DAP:
    mov byte [MDB_disk_address_packet], 0

load_stage1:
    mov ax, 1
    mov cl, 2
    mov bx, STAGE1_LOAD_SEGMENT
    mov es, bx
    mov bx, STAGE1_LOAD_OFFSET
    call disk_read

.read_finish:
    mov ax, STAGE1_LOAD_SEGMENT         ; set segment registers
    mov ds, ax
    mov es, ax

    jmp STAGE1_LOAD_SEGMENT:STAGE1_LOAD_OFFSET

    jmp wait_key_and_reboot             ; should never happen

    cli                                 ; disable interrupts, this way CPU can't get out of "halt" state
    hlt


;
; Error handlers
;

disk_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

kernel_not_found_error:
    mov si, msg_stage1_not_found
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h                     ; wait for keypress
    jmp 0FFFFh:0                ; jump to beginning of BIOS, should reboot

.halt:
    cli                         ; disable interrupts, this way CPU can't get out of "halt" state
    hlt


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

;
; Disk routines
;

;
; Converts an LBA address to a CHS address
; Parameters:
;   - ax: LBA address
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
;

lba_to_chs:

    push ax
    push dx

    xor dx, dx                          ; dx = 0
    div word [BPB_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack

    inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                          ; cx = sector

    xor dx, dx                          ; dx = 0
    div word [BPB_head_count]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                        ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                          ; dh = head
    mov ch, al                          ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                           ; put upper 2 bits of cylinder in CL

    pop ax
    mov dl, al                          ; restore DL
    pop ax
    ret


;
; Reads sectors from a disk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
disk_read:
    push ax                             ; save registers we will modify
    push bx
    push cx
    push dx
    push di

    cmp byte [MDB_disk_address_packet], 1
    jne .no_DAP

    mov word [DAP_LBA], ax
    mov word [DAP_LBA+2], 0
    mov dword [DAP_LBA+4], 0

    mov byte [DAP_sectors], cl
    mov byte [DAP_sectors+2], 0

    mov word [DAP_offset], bx
    mov word [DAP_segment], es 

    mov ah, 42h

    jmp .read

.no_DAP:
    push cx                             ; temporarily save CL (number of sectors to read)
    call lba_to_chs                     ; compute CHS
    pop ax                              ; AL = number of sectors to read

    mov ah, 02h

.read:
    mov dl, [EBPB_drive_number]
    mov di, 3                           ; retry count

.retry:
    pusha                               ; save all registers, we don't know what bios modifies
    mov si, DAP
    stc                                 ; set carry flag, some BIOS'es don't set it
    int 13h                             ; carry flag cleared = success
    jnc .done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp disk_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax                             ; restore registers modified
    ret

;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc disk_error
    popa
    ret

msg_read_failed:        db 'Read from disk failed', ENDL, 0
msg_stage1_not_found:   db 'STAGE1.BIN file not found', ENDL, 0
file_stage1_bin:        db 'STAGE1  BIN'
stage1_cluster:         dw 0

DAP:
    db 0x10
    db 0
    DAP_sectors: dw 0
    DAP_offset: dw 0
    DAP_segment: dw 0
    DAP_LBA: dq 0

STAGE1_LOAD_SEGMENT     equ 0x0
STAGE1_LOAD_OFFSET      equ 0x7E00

times 446-($-$$) db 0

; MBR
times 64 db 0

dw 0xAA55

buffer: