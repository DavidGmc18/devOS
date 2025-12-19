org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

jmp short init
nop

; FAT16 Header
bdb_oem:                    db 'DAVFAT16'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 256
bdb_total_sectors:          dw 4096
bdb_media_descriptor_type:  db 0xF8                 ; HDD
bdb_sectors_per_fat:        dw 128
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 32768                ; actual sector count
; extended boot record
ebr_drive_number:           db 0x80                 ;  0x80 hdd
ebr_reserved:               db 0                    ; reserved
ebr_signature:              db 0x29
ebr_volume_id:              dd 0x12345678           ; serial number, value doesn't matter
ebr_volume_label:           db 'DAVID OS   '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT16   '           ; 8 bytes


; disk adress packet
dap:
    db 16 ; size
    db 0  ; reserved
    dw 1  ; sectors to read (set dynamically)
    dw 0  ; offset (BX)
    dw 0  ; segment (ES)
    dq 0  ; LBA (QWORD)


init:
    mov ax, 0
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7C00

    mov [ebr_drive_number], dl

    mov ax, [bdb_sectors_per_fat]
    mov cx, [bdb_fat_count]
    ; xor dx, dx
    mul cx
    add ax, [bdb_reserved_sectors]
    mov [root_lba], ax

    mov ax, [bdb_dir_entries_count]
    shl ax, 5
    xor dx, dx
    div word [bdb_bytes_per_sector]
    mov [root_sectors], ax

    ; mov ax, [root_sectors]
    add ax, [root_lba] ; <- end of root
    sub ax, [bdb_sectors_per_cluster]
    sub ax, [bdb_sectors_per_cluster]
    mov [data_sector_start], ax

    ; mov si, msg_loading
    ; call puts


; Read root directory
; lba = sectors_per_fat * fat_count + reserved_sectors = 257
; size (Bytes) = dir_entries_count * 32 = 8192
; sectors = size (Bytes) / bytes_per_sector = 16
read_root_dir:
    mov ax, [root_lba]
    mov cx, [root_sectors]
    mov bx, buffer
    call disk_read


; Search tru root to find kernel.bin
    xor bx, bx
    mov di, buffer
search_kernel:
    mov si, file_kernel_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je found_kernel

    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl search_kernel
    jmp kernel_not_found_error


; Load kernel
found_kernel:
    mov si, msg_kernel_found
    call puts

    mov ax, [di + 26]
    mov [kernel_cluster], ax

    mov ax, [bdb_reserved_sectors]
    mov cx, [bdb_sectors_per_fat]
    mov bx, buffer
    ; ax = LBA
    ; cx = sectors
    ; bx:es = buffer
    call disk_read

    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET


; Loop over each sector of kernel.bin
load_kernel_loop:
    ; LBA = (kernel_cluster - 2) * sectors_per_cluster + start_sector
    ; LBA = kernel_cluster * sectors_per_cluster - (2*sectors_per_cluster) + start_sector
    ; Data is counted from root, not from disk start (also first 2 datasegments are reserved by FAT)
    ; start_sector = reserved + fats + root
    ; start_sector = reserved + (num_fats * clusters_per_fat) + (dir_entries_count * 32 / bytes_per_sector)
    mov ax, [kernel_cluster]
    mov cx, [bdb_sectors_per_cluster]
    xor dx, dx
    mul cx
    add ax, [data_sector_start]

    mov cx, [bdb_sectors_per_cluster]

    call disk_read

    
    mov ax, [bdb_bytes_per_sector]
    mov dx, [bdb_sectors_per_cluster]
    mul dx
    add bx, ax

    mov ax, [kernel_cluster]
    shl ax, 1 ; each entry in FAT fot FAT16 is 2 bytes

    mov si, buffer
    add si, ax
    mov ax, [ds:si] ; get entry for current kernel_cluster

    cmp ax, 0xFFF8
    jae read_finish

    mov [kernel_cluster], ax
    jmp load_kernel_loop

read_finish:
    ; mov dl, [ebr_drive_number]

    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET    

halt:
    cli
    hlt
    jmp halt





; Read from disk
; Params:
;   - ax: LBA
;   - cx: num sectors to read 
;   - bx:es: memory address where to store read data
disk_read:
    pusha

    mov word [dap + 2], cx       ; sectors to read
    mov word [dap + 4], bx       ; offset
    mov word [dap + 6], es       ; segment
    mov word [dap + 8], ax       ; LBA low 16 bits
    mov word [dap + 10], 0       ; rest of low DWORD
    mov dword [dap + 12], 0      ; high DWORD

    mov ah, 42h
    mov si, dap
    mov dl, [ebr_drive_number]
    mov di, 3                    ; retry count

.retry:
    stc
    int 13h
    jnc .done

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






; Prints a string to the screen
; Params:
;   - ds:si points to string
puts:
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






disk_error:
    mov si, msg_read_failed 
    call puts
    jmp halt


kernel_not_found_error:
    mov si, msg_kernel_not_found
    call puts
    jmp halt





; msg_loading:            db 'Loading...', ENDL, 0
msg_read_failed:        db 'Read from disk failed!', ENDL, 0
file_kernel_bin:        db 'KERNEL  BIN'
msg_kernel_not_found:   db 'KERNEL.BIN file not found!', ENDL, 0
msg_kernel_found:       db 'Found KERNEL.BIN file', ENDL, 0

root_lba:               dw 0
root_sectors:           dw 0
kernel_cluster:         dw 0
data_sector_start:      dw 0

KERNEL_LOAD_SEGMENT     equ 0x2000
KERNEL_LOAD_OFFSET      equ 0



times 510-($-$$) db 0
dw 0AA55h

buffer: