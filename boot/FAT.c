#include "FAT.h"
#include "BPB.h"
#include <memory.h>
#include <math.h>
#include <stdbool.h>

static ATA_drive_t drive;

static uint32_t fat_lba;
static uint32_t root_dir_lba;
static uint32_t root_dir_sectors;
static uint32_t data_sector_lba;

static uint8_t fat_type;
#define FAT12 0
#define FAT16 1
#define FAT32 2

static uint8_t* buffer512 = (uint8_t*)0x200000;
static uint8_t* fat_table = (uint8_t*)0x200200;

int FAT_initialize(ATA_drive_t boot_drive, uint32_t sector_offset) {
    drive = boot_drive;

    fat_lba = sector_offset + BPB.reserved_sectors;
    root_dir_lba = fat_lba + BPB.fat_count * BPB.sectors_per_fat;

    root_dir_sectors = (BPB.root_dir_entries * 32 + BPB.bytes_per_sector - 1) / BPB.bytes_per_sector;
    data_sector_lba = root_dir_lba + root_dir_sectors;

    if (BPB.total_sectors < 4085) {
        fat_type = FAT12;
    } else if (BPB.total_sectors < 65525) {
        fat_type = FAT16;
    } else {
        fat_type = FAT32;
    }

    // TODO check if tehr eis enough mem
    uint16_t remaining_sectors = BPB.sectors_per_fat;
    uint32_t current_fat_lba = fat_lba;
    uint8_t* current_fat_table_ptr = fat_table;
    while (remaining_sectors > 0) {
        uint16_t chunk_size = MIN(remaining_sectors, 256);

        ATA_read28(drive, current_fat_lba, (uint8_t)chunk_size, current_fat_table_ptr);

        current_fat_lba += chunk_size;
        current_fat_table_ptr += chunk_size * 512;
        remaining_sectors -= chunk_size;
    }
}

static uint16_t FAT_find_file(const char* name) {
    uint32_t current_entry = 0;
    for (uint32_t s = 0; s < root_dir_sectors; s++) {
        ATA_read28(drive, root_dir_lba + s, 1, buffer512); // TODO handle errors

        for (uint16_t i = 0; i < 16; i++) {
            if (memcmp(buffer512 + i*32, name, 11) == 0)
                goto found;
 
            current_entry++;
            if (current_entry >= BPB.root_dir_entries)
                goto not_found;
        }

    }

    not_found:
        return 0;

    found:
        uint16_t start_cluster = buffer512[(current_entry % 16) * 32 + 26];
        return start_cluster;
}

static uint16_t FAT_next_cluster(uint16_t current_cluster) {
    switch (fat_type) {
        case FAT12:
            uint16_t value = *(uint16_t*)(fat_table + (current_cluster*3)/2);
            bool even = !(current_cluster & 1);
            if (even) {
                return value & 0x0FFF;
            } else {
                return (value >> 4) & 0x0FFF;
            }

        case FAT16:
            return *(uint16_t*)(fat_table + current_cluster*2); 

        default:
            0xFFFF;
    }
}

int FAT_load_file(const char* path, void* buffer) {
    uint32_t current_cluster = FAT_find_file(path);

    while ((fat_type == FAT12 && current_cluster < 0x0FF8) || (fat_type == FAT16 && current_cluster < 0xFFF8)) {
        ATA_read28(drive, data_sector_lba + (current_cluster - 2) * BPB.sectors_per_cluster, BPB.sectors_per_cluster, buffer); // TODO handle errors
        buffer += 512;

        current_cluster = FAT_next_cluster(current_cluster);
    }

    return 0;
}