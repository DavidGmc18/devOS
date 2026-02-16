#pragma once

#include <stdint.h>
#include <driver/ata/ata.h>

enum MemoryBlockType {
    E820_USABLE = 1,
    E820_RESERVED = 2,
    E820_ACPI_RECLAIMABLE = 3,
    E820_ACPI_NVS = 4,
    E820_BAD_MEMORY = 5,
};

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
