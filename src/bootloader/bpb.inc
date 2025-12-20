%ifndef BOOT_INFO_INC
%define BOOT_INFO_INC

; Constants
%define ENDL 0x0D, 0x0A

; BPB (BIOS Parameter Block)
%define BDB_OEM                     0x7C03      ; 8 bytes
%define BDB_BYTES_PER_SECTOR        0x7C0B      ; 2 bytes
%define BDB_SECTORS_PER_CLUSTER     0x7C0D      ; 1 byte
%define BDB_RESERVED_SECTORS        0x7C0E      ; 2 bytes
%define BDB_FAT_COUNT               0x7C10      ; 1 byte
%define BDB_DIR_ENTRIES_COUNT       0x7C11      ; 2 bytes
%define BDB_TOTAL_SECTORS           0x7C13      ; 2 bytes
%define BDB_MEDIA_DESCRIPTOR_TYPE   0x7C15      ; 1 byte
%define BDB_SECTORS_PER_FAT         0x7C16      ; 2 bytes
%define BDB_SECTORS_PER_TRACK       0x7C18      ; 2 bytes
%define BDB_HEADS                   0x7C1A      ; 2 bytes
%define BDB_HIDDEN_SECTORS          0x7C1C      ; 4 bytes
%define BDB_LARGE_SECTOR_COUNT      0x7C20      ; 4 bytes
%define BDB_SIZE                    BDB_LARGE_SECTOR_COUNT + 4 - BDB_OEM

; EBR (Extended Boot Record)
%define EBR_DRIVE_NUMBER            0x7C24      ; 1 byte
%define EBR_RESERVED                0x7C25      ; 1 byte
%define EBR_SIGNATURE               0x7C26      ; 1 byte
%define EBR_VOLUME_ID               0x7C27      ; 4 bytes
%define EBR_VOLUME_LABEL            0x7C2B      ; 11 bytes
%define EBR_SYSTEM_ID               0x7C36      ; 8 bytes
%define EBR_SIZE                    EBR_SYSTEM_ID + 8 - EBR_DRIVE_NUMBER

%endif