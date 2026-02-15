#pragma once

#include <stdint.h>
#include <driver/ata/ata.h>

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} MemoryBlock;

typedef struct {
    uint32_t block_count;
    MemoryBlock* blocks;
} MemoryInfo;

typedef struct {
    ATA_drive_t drive;
    uint8_t partition;
} BootDevice;

typedef struct {
    BootDevice boot_device;
    MemoryInfo memory_info;
} BootParams;
