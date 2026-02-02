#pragma once

#include <stdint.h>

typedef struct {
    const uint16_t bytes_per_sector;    // 0x7C0B
    const uint8_t sectors_per_cluster;  // 0x7C0D
    const uint16_t reserved_sectors;    // 0x7C0E
    const uint8_t fat_count;            // 0x7C10
    const uint16_t root_dir_entries;    // 0x7C11
    const uint16_t total_sectors;       // 0x7C13
    const uint8_t media_descriptor;     // 0x7C15
    const uint16_t sectors_per_fat;     // 0x7C16
    const uint16_t sectors_per_track;   // 0x7C18
    const uint16_t head_count;          // 0x7C1A
    const uint32_t hidden_sectors;      // 0x7C1C
    const uint32_t large_sector_count;  // 0x7C20
} __attribute__((packed)) _BPB;
static const _BPB* const BPB = (const _BPB*)0x7C00;

typedef struct {
    const uint8_t drive_number;         // 0x7C24
    const uint8_t flags;                // 0x7C25
    const uint8_t boot_signature;       // 0x7C26
    const uint32_t volume_id;           // 0x7C27
} __attribute__((packed)) _EBPB;
static const _EBPB* const EBPB = (const _EBPB*)0x7C24;

typedef struct {
    const uint8_t filesystem;           // 0x7C2B
    const uint8_t disk_address_packet;  // 0x7C2C
} __attribute__((packed)) _MDB;
static const _MDB* const MDB = (const _MDB*)0x7C2B;

#define FS_FAT12 0
#define FS_FAT16 1