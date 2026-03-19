#pragma once
// FAT 32

#include <stdint.h>
#include <bl/types.h>

typedef struct {
    void* abar;
    uint8_t port;
    uint32_t lba;
    uint32_t sectors;
    uint32_t fat;
    uint32_t fat_sectors;
    uint32_t root_dir_cluster;
    uint32_t data;
    uint8_t sectors_per_cluster;
} FAT_dev;

void FAT_init(BL_DiskRead disk_read_fn);

int FAT_dev_init(FAT_dev* dev, BL_Disk* disk);

int FAT_read(FAT_dev* dev, const char* path, void* buffer);