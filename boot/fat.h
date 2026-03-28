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
} fat_dev_t;

void fat_init(BL_DiskRead disk_read_fn);

int fat_dev_init(fat_dev_t* dev, BL_Disk* disk);

int fat_read(fat_dev_t* dev, const char* path, void* buffer);