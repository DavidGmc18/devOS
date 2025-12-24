#pragma once

#include "stdint.h"

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

uint8_t DISK_Initialize(DISK* disk, uint8_t driveNumber);
uint8_t DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t* dataOut);