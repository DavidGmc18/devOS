%macro BiosParameterBlock 0
    times 25 db 0
%endmacro

BPB_bytes_per_sector:       equ 0x7C0B
BPB_sectors_per_cluster:    equ 0x7C0D
BPB_reserved_sectors:       equ 0x7C0E
BPB_fat_count:              equ 0x7C10
BPB_root_dir_entries:       equ 0x7C11
BPB_total_sectors:          equ 0x7C13
BPB_media_descriptor:       equ 0x7C15
BPB_sectors_per_fat:        equ 0x7C16
BPB_sectors_per_track:      equ 0x7C18
BPB_head_count:             equ 0x7C1A
BPB_hidden_sectors:         equ 0x7C1c
BPB_large_sector_count:     equ 0x7C20


%macro ExtendedBiosParameterBlock 0
    times 7 db 0
%endmacro

EBPB_drive_number:          equ 0x7C24
EBPB_flags:                 equ 0x7C25
EBPB_boot_signature:        equ 0x7C26
EBPB_volume_id:             equ 0x7C27