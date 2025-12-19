org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

jmp short init
nop

; FAT16 Boot Parameter Block
bdb_oem:                    db 'DAVFAT16'
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 256
bdb_total_sectors:          dw 0          ; 0 = use large_sector_count
bdb_media_descriptor_type:  db 0xF8
bdb_sectors_per_fat:        dw 128
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 32768
ebr_drive_number:           db 0
ebr_reserved:               db 0
ebr_signature:              db 0x29
ebr_volume_id:              dd 0x12345678
ebr_volume_label:           db 'DAVID OS   '
ebr_system_id:              db 'FAT16   '

; Disk Address Packet
dap:    db 16, 0
        dw 0          ; sectors to read (filled later)
        dw 0          ; offset
        dw 0          ; segment
        dd 0          ; LBA low
        dd 0          ; LBA high

init:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [ebr_drive_number], dl

    ; Check extended read support
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc no_ext
    cmp bx, 0xAA55
    jne no_ext

    ; Compute root LBA
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx
    add ax, [bdb_reserved_sectors]
    mov [root_lba], ax

    ; Compute root sectors
    mov ax, [bdb_dir_entries_count]
    shl ax, 5               ; *32
    xor dx, dx
    div word [bdb_bytes_per_sector]
    mov [root_sectors], ax

    ; Compute first data sector
    add ax, [root_lba]
    sub ax, 2               ; subtract 2 reserved clusters
    mov [data_start], ax

    ; Read root directory
    mov ax, [root_lba]
    mov cx, [root_sectors]
    mov bx, buffer
    call disk_read

    ; Search for KERNEL.BIN
    mov di, buffer
    xor bx, bx
.search:
    mov si, kernel_name
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found
    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search
    mov si, msg_notfound
    call puts
    jmp halt

.found:
    mov si, msg_found
    call puts

    mov ax, [di+26]         ; first cluster
    mov [cluster], ax

    ; Load FAT
    mov ax, [bdb_reserved_sectors]
    mov cx, [bdb_sectors_per_fat]
    mov bx, buffer
    call disk_read

    ; Setup load address
    mov ax, 0x2000
    mov es, ax
    xor bx, bx              ; 0x2000:0000

load_loop:
    ; Compute LBA = (cluster - 2) * sec_per_cluster + data_start
    mov ax, [cluster]
    sub ax, 2
    xor dx, dx
    mul byte [bdb_sectors_per_cluster]
    add ax, [data_start]

    mov cx, [bdb_sectors_per_cluster]
    call disk_read

    ; Advance pointer by one cluster (512 bytes if spc=1)
    add bx, 512
    jnc .no_overflow
    mov ax, es
    add ax, 0x1000
    mov es, ax
.no_overflow:

    ; Get next cluster
    mov ax, [cluster]
    shl ax, 1
    mov si, buffer
    add si, ax
    mov ax, [si]

    cmp ax, 0xFFF8
    jae done_loading

    mov [cluster], ax
    jmp load_loop

done_loading:
    mov dl, [ebr_drive_number]
    mov ax, 0x2000
    mov ds, ax
    mov es, ax
    jmp 0x2000:0x0000

halt:
    cli
    hlt
    jmp halt

no_ext:
    mov si, msg_noext
    call puts
    jmp halt

disk_error:
    mov si, msg_diskerr
    call puts
    jmp halt

; ax = LBA, cx = sectors, es:bx = buffer
disk_read:
    pusha
    mov [dap+4], bx
    mov [dap+6], es
    mov [dap+8], ax
    mov word [dap+2], cx

    mov ah, 0x42
    mov si, dap
    mov dl, [ebr_drive_number]
    mov di, 3
.retry:
    int 0x13
    jnc .ok
    call disk_reset
    dec di
    jnz .retry
    jmp disk_error
.ok:
    popa
    ret

disk_reset:
    pusha
    xor ax, ax
    mov dl, [ebr_drive_number]
    stc
    int 0x13
    popa
    ret

puts:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    popa
    ret

kernel_name:     db 'KERNEL  BIN'
msg_found:       db 'K', ENDL, 0        ; Found
msg_notfound:    db 'N', ENDL, 0        ; Not found
msg_diskerr:     db 'D', ENDL, 0        ; Disk error
msg_noext:       db 'E', ENDL, 0        ; No ext support

root_lba:        dw 0
root_sectors:    dw 0
data_start:      dw 0
cluster:         dw 0

times 510-($-$$) db 0
dw 0xAA55

buffer: