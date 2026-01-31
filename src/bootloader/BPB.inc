%macro BiosParameterBlock 0
    times 25 db 0
%endmacro

BPB_bytes_per_sector:       equ 0x7C0B  ; 2B
BPB_sectors_per_cluster:    equ 0x7C0D  ; 1B
BPB_reserved_sectors:       equ 0x7C0E  ; 2B
BPB_fat_count:              equ 0x7C10  ; 1B
BPB_root_dir_entries:       equ 0x7C11  ; 2B
BPB_total_sectors:          equ 0x7C13  ; 2B
BPB_media_descriptor:       equ 0x7C15  ; 1B
BPB_sectors_per_fat:        equ 0x7C16  ; 2B
BPB_sectors_per_track:      equ 0x7C18  ; 2B
BPB_head_count:             equ 0x7C1A  ; 2B
BPB_hidden_sectors:         equ 0x7C1C  ; 4B
BPB_large_sector_count:     equ 0x7C20  ; 4B 


%macro ExtendedBiosParameterBlock 0
    times 7 db 0
%endmacro

EBPB_drive_number:          equ 0x7C24  ; 1B
EBPB_flags:                 equ 0x7C25  ; 1B
EBPB_boot_signature:        equ 0x7C26  ; 1B
EBPB_volume_id:             equ 0x7C27  ; 4B


%macro MetaDataBlock 0
    times 1 db 0
%endmacro

MDB_filesystem:             equ 0x7C2B  ; 1B
%define FS_FAT12 0
%define FS_FAT16 1