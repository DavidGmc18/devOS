org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

; FAT12 header
jmp short start
nop

bdb_oem: db 'MSWIN4.1' ; 8 bytes
bdb_bytes_per_sector: dw 512
bdb_sectors_per_cluster: db 1
bdb_reserved_sectors: dw 1
bdb_fat_count: db 2
bdb_dir_entries_count: dw 0E0h
bdb_total_sectors: dw 2880 ; 2880 * 512 = 1.44MB
bdb_media_descriptor_type: db 0F0h ; F0 = 3.5" floppy disk
bdb_sectors_per_fat: dw 9
bdb_sectors_per_track: dw 18
bdb_heads: dw 2
bdb_hidden_sectors: dd 0
bdb_large_sector_count: dd 0

; extended boot record
ebr_drive_number: db 0, 0 ; type (0x00 floppy, 0x80 hdd, useless), reserved
ebr_signature: db 29h
ebr_volume_id: db 12h, 34h, 56h, 78h ; serial number, value doesn't matter
ebr_volume_label: db 'DAVID OS   ' ; 11 bytes, padded with spaces
ebr_system_id: db 'FAT12   ' ; 8 bytes


start:
    ; setup data segments
    mov ax, 0 ; can't write to ds/es directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00 ; stack grows downwards, we do not want it to override our code

    ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the expected location
    push es
    push word .after
    retf
.after:
    ; ; read something from floppy disk
    ; ; BIOS should set dl to drive number
    mov [ebr_drive_number], dl
    ; mov ax, 1 ; LBA=1, second sector from disk
    ; mov cl, 1 ; read 1 sector
    ; mov bx, 0x7E00 ; data should be after the bootloader
    ; call disk_read

    ; print message
    mov si, msg_loading
    call puts

    ; read drive parameters
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3F
    xor ch, ch
    mov [bdb_sectors_per_track], cx

    inc dh
    mov [bdb_heads], dh

    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx
    add ax, [bdb_reserved_sectors]
    push ax

    mov ax, [bdb_sectors_per_fat]
    shl ax, 5
    xor dx, dx
    div word [bdb_bytes_per_sector]

    test dx, dx
    jz .root_dir_after
    inc ax

.root_dir_after:
    mov cl, al
    pop ax
    mov dl, [ebr_drive_number]
    mov bx, buffer
    call disk_read

    ; search for kenrel.bin
    xor bx, bx
    mov di, buffer

.search_kernel:
    mov si, file_kernel_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found_kernel

    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel

    jmp kernel_not_found_error


.found_kernel:
    mov ax, [di + 26]
    mov [Kernel_cluster], ax

    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET

.load_kernel_loop:
    mov ax, [Kernel_cluster]
    add ax, 31

    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]

    mov ax, [Kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after

.even:
    and ax, 0x0FFF

.next_cluster_after:
    cmp ax, 0x0FF8
    jae .read_finish

    mov [Kernel_cluster], ax
    jmp .load_kernel_loop

.read_finish:
    mov dl, [ebr_drive_number]

    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET

    jmp wait_key_and_reboot


    cli
    hlt


; Error Handlers

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

kernel_not_found_error:
    mov si, msg_kernel_not_found
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h ; wait for keypress
    jmp 0FFFFH:0 ; jump to beginning of BIOS, should reboot

.halt:
    cli ; disable interrupts
    jmp .halt


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


; Disk routines

; Converts an LBA adress to CHS adress
; Parameters:
;   - ax: LBA adress
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
lba_to_chs:
    push ax
    push dx

    xor dx, dx; dx = 0
    div word [bdb_sectors_per_track] ; ax = LBA / SectorsPerTrack ; dx = LBA % SectorsPerTrack

    inc dx ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx ; cx = sector

    xor dx, dx ; dx = 0
    div word [bdb_heads] ; ax = (LBA / SectorsPerTrack) / Heads ; dx = (LBA / SectorsPerTrack) % Heads

    mov dh, dl ; dh = head
    mov ch, al ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah ; put upper 2 bits of cylinder

    pop ax
    mov dl, al
    pop ax
    ret


; Reads sectors from a disk
; Parameters:
;   - ax: LBA adress
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory adress where to store read data
disk_read:
    push ax
    push bx
    push cx
    push dx
    push di

    push cx ; temporarily save cl
    call lba_to_chs ; cx [bits 0-5]: sector number ; cx [bits 6-15]: cylinder
    pop ax

    mov ah, 02h
    mov di, 3 ; retry count

.retry:
    pusha ; save all registers, we don't know what BIOS modifies
    stc ; set carry flag, some BIOS'es don't set it
    int 13h ; interrupt 13,2 (ah = 02h) - read disk sectors ; carry flag cleared = success
    jnc .done ; jmp if carry no set (success)

    ; read failed
    popa ; pop all
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp floppy_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax


; Reset disk controller
; Parameters:
;   - dl: drive number
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret


msg_loading: db 'Loading...', ENDL, 0 ; 0 = null terminator
msg_read_failed: db 'Read from disk failed!', ENDL, 0
msg_kernel_not_found: db 'KERNEL.BIN file not found!', ENDL, 0
file_kernel_bin: db 'KERNEL  BIN'
Kernel_cluster: dw 0

KERNEL_LOAD_SEGMENT equ 0x2000
KERNEL_LOAD_OFFSET equ 0x

times 510-($-$$) db 0
dw 0AA55h

buffer: