org 0x7E00
bits 16

%include "../BPB.inc"

%define ENDL 0x0D, 0x0A

STAGE2_LOAD_SEGMENT     equ 0x0
STAGE2_LOAD_OFFSET      equ 0x500

init:
    ; ROOT_DIR_LBA = BPB_reserved_sectors + BPB_fat_count * BPB_sectors_per_fat
    mov ax, [BPB_sectors_per_fat]
    xor cx, cx
    mov cl, [BPB_fat_count]
    mul cx
    add ax, [BPB_reserved_sectors]
    mov [ROOT_DIR_LBA], ax

    ; ROOT_SIZE = BPB_root_dir_entries * 32 / 512 (round up)
    ;   = BPB_root_dir_entries / 16 (rounded up)
    ;   = (BPB_root_dir_entries + 15) / 16
    ; TODO hard coded BPB_bytes_per_sector (512)
    mov ax, [BPB_root_dir_entries]
    add ax, 15
    shr ax, 4
    mov [ROOT_DIR_SIZE], ax    
 
    ; DATA_REGION_LBA = ROOT_DIR_LBA + ROOT_SIZE
    ; mov ax, [ROOT_DIR_SIZE]
    add ax, [ROOT_DIR_LBA]
    mov [DATA_REGION_LBA], ax

detect_filesystem:
    ; CountofClusters = DataSec / BPB_SecPerClus
    mov ax, [BPB_total_sectors]
    sub ax, [DATA_REGION_LBA]
    xor cx, cx
    mov cl, [BPB_sectors_per_cluster]
    xor dx, dx
    div cx
    ; FAT12 volume cannot contain more than 4084 clusters
    cmp word ax, 4085
    jae .set_FAT16

.set_FAT12:
    mov byte [MDB_filesystem], FS_FAT12
    jmp load_root_dir

.set_FAT16:
    mov byte [MDB_filesystem], FS_FAT16

load_root_dir:
    mov ax, [ROOT_DIR_LBA]
    mov cl, [ROOT_DIR_SIZE]
    mov dl, [EBPB_drive_number]
    mov bx, buffer
    call disk_read

find_stage2:
    xor bx, bx 
    mov di, buffer
    
.loop:
    mov si, file_stage2_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found

    add di, 32
    inc bx
    cmp bx, [BPB_root_dir_entries]
    jl .loop
    
    jmp stage2_not_found_error

.found:
    mov ax, [di + 26]
    mov [STAGE2_CLUSTER], ax

load_fat:
    mov ax, [BPB_reserved_sectors] ; First FAT starts after reserved sectors
    mov cl, [BPB_sectors_per_fat]
    mov dl, [EBPB_drive_number]
    mov bx, buffer
    call disk_read

load_stage2:
    mov bx, STAGE2_LOAD_SEGMENT
    mov es, bx
    mov bx, STAGE2_LOAD_OFFSET

load_loop:
    ; Compute LBA
    ; LBA = (STAGE2_CLUSTER - 2) * BPB_sectors_per_cluster + DATA_REGION_LBA
    mov ax, [STAGE2_CLUSTER]
    sub ax, 2
    xor cx, cx
    mov cl, [BPB_sectors_per_cluster]
    mul cx
    add ax, [DATA_REGION_LBA]

    ; Load
    ; ax - LBA
    mov cl, [BPB_sectors_per_cluster]
    mov dl, [EBPB_drive_number]
    ; bx - memory address where to store data
    call disk_read

    ; Increment bx
    ; bx += BPB_bytes_per_sector * BPB_sectors_per_cluster
    mov ax, [BPB_bytes_per_sector]
    mov cx, [BPB_sectors_per_cluster]
    mul cx
    add bx, ax 

    ; FAT12 or FAT16?
    cmp byte [MDB_filesystem], FS_FAT16
    je next_cluster_FAT16

next_cluster_FAT12:
    ; NEXT_CLUSTER = FAT[STAGE2_CLUSTER * 1.5] (read 1.5byte)
    mov ax, [STAGE2_CLUSTER]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx ; check for remaineder => oven or odd
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster

.even:
    and ax, 0x0FFF

.next_cluster:
    cmp ax, 0x0FF8
    jae read_finish

    mov [STAGE2_CLUSTER], ax
    jmp load_loop

next_cluster_FAT16:
    ; NEXT_CLUSTER = FAT[STAGE2_CLUSTER * 2] (read word)
    mov ax, [STAGE2_CLUSTER]
    shl ax, 1

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    cmp ax, 0xFFF8
    jae read_finish

    mov [STAGE2_CLUSTER], ax
    jmp load_loop

read_finish:
    ; TODO remove this, use EBPB_drive_number
    ; jump to our kernel
    mov dl, [EBPB_drive_number]          ; boot device in dl

    mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
    mov ds, ax
    mov es, ax

    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

    jmp wait_key_and_reboot             ; should never happen

    cli                                 ; disable interrupts, this way CPU can't get out of "halt" state
    hlt


;
; Error handlers
;

floppy_error:
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

    push cx                             ; temporarily save CL (number of sectors to read)
    call lba_to_chs                     ; compute CHS
    pop ax                              ; AL = number of sectors to read
    
    mov ah, 02h
    mov di, 3                           ; retry count

.retry:
    pusha                               ; save all registers, we don't know what bios modifies
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
    jmp floppy_error

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
    jc floppy_error
    popa
    ret

ROOT_DIR_LBA: dw 0
ROOT_DIR_SIZE: dw 0
DATA_REGION_LBA: dw 0
STAGE2_CLUSTER: dw 0

msg_read_failed: db 'Read from disk failed!', ENDL, 0
file_stage2_bin: db 'STAGE2  BIN'
msg_stage2_not_found:   db 'STAGE2.BIN file not found!', ENDL, 0

buffer:

times 512-($-$$) db 0