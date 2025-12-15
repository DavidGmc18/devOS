org 0x7C00
bits 16

jmp short init

oem_name: db 'DAVFAT16'

bytes_per_sector: dw 512
sectors_per_cluster: db 1
reserved_sectors: dw 1
num_fats: db 2
num_dir_entries: dw 256
total_sectors_small: dw 32768
total_sectors_large: dd 0
media_descriptor: db 0xF8
hidden_sectors: dd 0

init:
    mov ax, 0
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7C00  

halt:
    cli
    hlt

times 510-($-$$) db 0
dw 0AA55h