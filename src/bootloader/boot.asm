org 0x7C00
bits 16

jmp short init
nop

bdb_oem:                    db 'DAVFAT16'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 256
bdb_total_sectors:          dw 4096
bdb_media_descriptor_type:  db 0xF8                 ; HDD
bdb_sectors_per_fat:        dw 127
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 3276                 ; actual sector count

; extended boot record
ebr_drive_number:           db 0x80                 ;  0x80 hdd
ebr_reserved:               db 0                    ; reserved
ebr_signature:              db 0x29
ebr_volume_id:              dd 0x12345678           ; serial number, value doesn't matter
ebr_volume_label:           db 'DAVID OS   '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT16   '           ; 8 bytes

init:
    mov ax, 0
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7C00

halt:
    cli
    hlt
    jmp halt

times 510-($-$$) db 0
dw 0AA55h