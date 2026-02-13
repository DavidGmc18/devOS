#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <driver/ata/ata.h>

#define MBR_ERRC_SUCCESS 0

typedef struct {
    uint8_t attributes;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t lba;
    uint32_t sectors;
} __attribute__((packed)) MBR_Partition;

typedef struct {
    MBR_Partition partitions[4];
} __attribute__((packed)) MBR_Table;

int MBR_get_table(ATA_drive_t drive, MBR_Table* table);

#ifdef __cplusplus
}
#endif